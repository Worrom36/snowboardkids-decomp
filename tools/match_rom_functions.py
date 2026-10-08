#!/usr/bin/env python3
"""Map US functions to JP and propose corroborated assembly file boundaries.

Run both build-and-verify variants first. This tool reads verified ROMs and ELF
symbol tables; it never rewrites extraction configuration. Addresses in reports
are ROM offsets unless explicitly labeled VRAM.
"""
from __future__ import annotations

import argparse
from bisect import bisect_right
from collections import Counter, defaultdict
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess

import yaml

ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class Function:
    name: str
    rom: int
    vram: int
    size: int
    file: str
    body: bytes


def code_layout(config):
    """Return explicit CPU text intervals and their segment address mappings."""
    ranges = []
    for segment_index, segment in enumerate(config['segments']):
        if not isinstance(segment, dict) or segment.get('type') != 'code':
            continue
        subs = segment['subsegments']
        for i, sub in enumerate(subs):
            if not isinstance(sub, list) or sub[1] not in ('c', 'asm', 'hasm'):
                continue
            following = (subs[i + 1] if i + 1 < len(subs)
                         else config['segments'][segment_index + 1])
            end = following[0] if isinstance(following, list) else following['start']
            ranges.append((sub[0], end, sub[2], segment['vram'] - segment['start']))
    return ranges


def read_region(root, version):
    suffix = '.jp' if version == 'jp' else ''
    config = yaml.safe_load((root / f'snowboardkids{suffix}.yaml').read_text())
    rom = (root / config['options']['target_path']).read_bytes()
    expected = config['sha1']
    if hashlib.sha1(rom).hexdigest() != expected:
        raise ValueError(f'{version}: original ROM checksum differs from configuration')
    elf = root / config['options']['elf_path']
    rebuilt = elf.with_suffix('.z64')
    if not rebuilt.exists() or hashlib.sha1(rebuilt.read_bytes()).hexdigest() != expected:
        raise ValueError(f'{version}: run build-and-verify.sh before matching')
    readelf = next((shutil.which(p + 'readelf') for p in
                    ('mips-linux-gnu-', 'mips64-linux-gnu-', 'mips64-elf-')
                    if shutil.which(p + 'readelf')), None)
    if not readelf:
        raise ValueError('a MIPS readelf is required')
    output = subprocess.check_output([readelf, '-sW', str(elf)], text=True)
    ranges = code_layout(config)
    functions = {}
    for line in output.splitlines():
        fields = line.split()
        if len(fields) != 8 or fields[3] != 'FUNC' or fields[6] == 'UND':
            continue
        address, size, name = int(fields[1], 16), int(fields[2]), fields[7]
        if not size or name.endswith('.NON_MATCHING'):
            continue
        for start, end, file, delta in ranges:
            offset = address - delta
            if start <= offset < end:
                if offset % 4 or size % 4 or offset + size > end:
                    raise ValueError(f'{version}: invalid function extent: {name}')
                f = Function(name, offset, address, size, file, rom[offset:offset + size])
                old = functions.get(offset)
                if old and old.size != size:
                    raise ValueError(f'{version}: conflicting aliases at {offset:#x}')
                if old is None or name < old.name:
                    functions[offset] = f
                break
    result = sorted(functions.values(), key=lambda f: f.rom)
    for left, right in zip(result, result[1:]):
        if left.rom + left.size > right.rom:
            raise ValueError(f'{version}: overlapping functions {left.name}, {right.name}')
    if not result:
        raise ValueError(f'{version}: no functions found')
    ram = []
    for s in config['segments']:
        if isinstance(s, dict) and s.get('type') == 'code':
            end = next((x['vram'] for x in s['subsegments'] if isinstance(x, dict)
                        and x.get('type') == 'bss'), None)
            if end is not None:
                ram.append((s['vram'], end + s.get('bss_size', 0)))
    return result, ranges, ram, expected


def signed16(n):
    return (n & 0x7fff) - (n & 0x8000)


def normalized(body, vram, ram):
    """Conservative MIPS address normalization, bounded by control flow.

    Track LUI origins only within straight-line code (including delay slots).
    Only actual RAM addresses and direct jumps into mapped RAM are normalized.
    Unknown instructions clear state; ordinary immediates are never masked.
    """
    words = list(struct.unpack(f'>{len(body)//4}I', body))
    output = words.copy()
    entries = set()
    for i, w in enumerate(words):
        op = w >> 26
        if op in (1, 4, 5, 6, 7, 20, 21, 22, 23) or (op == 17 and (w >> 21) & 31 == 8):
            entries.add(i + 1 + signed16(w & 0xffff))
        elif op in (2, 3):
            target = ((vram + i*4 + 4) & 0xf0000000) | ((w & 0x3ffffff) << 2)
            if vram <= target < vram + len(body):
                entries.add((target-vram)//4)
    upper = {}
    refs = []
    unsafe_origins = set()
    consumers = defaultdict(list)
    reset_after = False
    loads = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x30, 0x31, 0x35, 0x37}
    stores = {0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x38, 0x39, 0x3d, 0x3f}
    def in_ram(value):
        return any(a <= value < b for a, b in ram)
    for i, w in enumerate(words):
        # A branch into the middle of a LUI/consumer sequence invalidates its
        # apparent straight-line provenance, even if encountered later in code.
        if i in entries:
            upper.clear()
        op, rs, rt, rd = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        imm = w & 0xffff
        prior_branch = reset_after
        reset_after = False
        if op in stores and op not in {0x39, 0x3d} and rt in upper:
            unsafe_origins.add(upper[rt][0])
        if op not in {2, 3, 9, 13, 15} | loads | stores:
            sources = (rt,) if op == 17 and rs in (4, 6) else () if op == 17 else (
                (rs,) if op in (8, 10, 11, 12, 14) else (rs, rt))
            unsafe_origins.update(upper[r][0] for r in sources if r in upper)
        if op in (2, 3):
            target = ((vram + i*4 + 4) & 0xf0000000) | ((w & 0x3ffffff) << 2)
            if vram <= target < vram + len(body):
                # Preserve intra-function control flow independent of placement.
                output[i] = (w & 0xfc000000) | ((target - vram) >> 2)
                reset_after = True
                if prior_branch:
                    upper.clear()
                continue
            if in_ram(target):
                output[i] &= 0xfc000000
                refs.append((i, 'call' if op == 3 else 'jump', target))
            reset_after = True
        elif op == 15:
            upper[rt] = (i, imm << 16)
        elif op in (9, 13) or op in loads | stores:
            if rs in upper:
                origin, high = upper[rs]
                target = (high | imm) if op == 13 else (high + signed16(imm)) & 0xffffffff
                if in_ram(target):
                    output[origin] &= 0xffff0000
                    output[i] &= 0xffff0000
                    consumers[origin].append(i)
                    refs.append((i, 'address', target))
                else:
                    unsafe_origins.add(origin)
            if op in (9, 13, 0x38) or (op in loads and op not in {0x31, 0x35}):
                upper.pop(rt, None)
        elif op == 0:
            funct = w & 63
            if funct in (8, 9):
                reset_after = True
            elif funct not in (12, 13, 15, 17, 19, 24, 25, 26, 27):
                upper.pop(rd, None)
            else:
                upper.clear()
        elif op in (1, 4, 5, 6, 7, 20, 21, 22, 23):
            reset_after = True
        elif op in (8, 10, 11, 12, 14):
            upper.pop(rt, None)
        elif op == 17:
            if rs in (0, 2):
                upper.pop(rt, None)
            elif rs == 8:
                reset_after = True
        else:
            upper.clear()
        upper.pop(0, None)
        if prior_branch:
            upper.clear()
    unsafe_consumers = {i for origin in unsafe_origins for i in consumers[origin]}
    for i in unsafe_origins | unsafe_consumers:
        output[i] = words[i]
    refs = [ref for ref in refs if ref[0] not in unsafe_consumers]
    return tuple(output), refs


def match_functions(us, jp, us_ram, jp_ram):
    us_norm = [normalized(f.body, f.vram, us_ram) for f in us]
    jp_norm = [normalized(f.body, f.vram, jp_ram) for f in jp]
    exact_index, norm_index = defaultdict(list), defaultdict(list)
    for j, f in enumerate(jp):
        exact_index[f.body].append(j)
        norm_index[jp_norm[j][0]].append(j)
    candidates, methods = {}, {}
    for i, f in enumerate(us):
        exact = exact_index.get(f.body, [])
        candidates[i] = list(exact or norm_index.get(us_norm[i][0], []))
        methods[i] = 'exact' if exact else 'address-normalized'
    accepted = {}
    conflicts = set()
    us_addresses = {f.vram: i for i, f in enumerate(us)}
    jp_addresses = {f.vram: j for j, f in enumerate(jp)}

    def compatible(i, j):
        # A mapped call must still lead to the corresponding function. Shared
        # references within a function must preserve equality in both directions.
        left, right = us_norm[i][1], jp_norm[j][1]
        if [(x[0], x[1]) for x in left] != [(x[0], x[1]) for x in right]:
            return False
        forward, reverse = {}, {}
        for (_, kind, a), (_, _, b) in zip(left, right):
            if a in forward and forward[a] != b or b in reverse and reverse[b] != a:
                return False
            forward[a], reverse[b] = b, a
            if a in us_addresses:
                mapped = accepted.get(us_addresses[a])
                if mapped is not None and jp_addresses.get(b) != mapped:
                    return False
        return True

    # Unique anchors first, then uniquely constrained gaps. Do not manufacture
    # an ordering by selecting one side of contradictory anchor evidence.
    while True:
        reverse = Counter(j for i, js in candidates.items() if i not in conflicts for j in js)
        proposals = {i: js[0] for i, js in candidates.items()
                     if i not in accepted and i not in conflicts and len(js) == 1
                     and reverse[js[0]] == 1
                     and (us[i].size >= 16 or
                          (i-1 in accepted and i+1 in accepted
                           and accepted[i-1] < js[0] < accepted[i+1]))
                     and compatible(i, js[0])}
        combined = sorted({**accepted, **proposals}.items())
        bad = set()
        for k, (i, j) in enumerate(combined):
            for ii, jj in combined[k+1:]:
                if jj <= j:
                    bad.update((i, ii))
        conflicts.update(bad)
        previous = dict(accepted)
        accepted = {i: j for i, j in combined if i not in bad}
        for i, j in list(accepted.items()):
            if not compatible(i, j):
                conflicts.add(i)
                del accepted[i]
        anchors = sorted(accepted)
        used = set(accepted.values())
        for i, js in candidates.items():
            if i in accepted or i in conflicts:
                continue
            k = bisect_right(anchors, i)
            lo = accepted[anchors[k-1]] if k else -1
            hi = accepted[anchors[k]] if k < len(anchors) else len(jp)
            candidates[i] = [j for j in js if j not in used and lo < j < hi and compatible(i, j)]
        if accepted == previous:
            break
    return accepted, candidates, methods, conflicts


def infer_boundaries(us, jp, ranges, matches):
    rows = []
    main_ranges = [r for r in ranges if r[0] >= 0x1050]
    for k, (start, end, name, _) in enumerate(main_ranges):
        indices = [i for i, f in enumerate(us) if start <= f.rom < end]
        anchors = [i for i in indices if i in matches]
        entry = next((i for i in indices if us[i].rom == start), None)
        adjacent_start = main_ranges[max(0, k-1)][0]
        adjacent_end = main_ranges[min(len(main_ranges)-1, k+1)][1]
        support = [i for i in matches if adjacent_start <= us[i].rom < adjacent_end and i != entry]
        proposed = jp[matches[entry]].rom if entry in matches else None
        before = [i for i in matches if us[i].rom < start]
        after = [i for i in matches if us[i].rom >= start]
        # Unknown boundary lies after the complete preceding anchor, before
        # the following anchor. This is an interval, never an inferred offset.
        lower = jp[matches[max(before)]].rom + jp[matches[max(before)]].size if before else 0x1050
        upper = jp[matches[min(after)]].rom if after else None
        rows.append(dict(file=name, us_start=start, us_end=end, jp_start=proposed,
                         status='supported' if proposed is not None and support else 'approximate',
                         jp_interval=[lower, upper], matched_functions=len(anchors),
                         total_functions=len(indices),
                         evidence=[dict(us=us[i].rom, jp=jp[matches[i]].rom, name=us[i].name)
                                   for i in sorted(set(anchors + support))]))
    return rows


def assembly_splits(boundaries, functions, jp, text_end):
    """Name supported starts; isolate tails whose next file start is unknown."""
    points = {0x1050: dict(start=0x1050, name='unresolved/001050', reason='unresolved')}
    for k, boundary in enumerate(boundaries):
        if boundary['status'] != 'supported':
            continue
        start = boundary['jp_start']
        points[start] = dict(start=start, name=boundary['file'], reason='supported-start')
        if k+1 == len(boundaries) or boundaries[k+1]['status'] == 'supported':
            continue
        anchors = [f for f in functions if f['file'] == boundary['file'] and f['status'] == 'matched']
        last_end = max(f['jp_rom']+f['size'] for f in anchors)
        # Retain padding with the preceding function. Unknown tails receive no
        # US file identity; stop only at a real JP function start.
        stop = next((f.rom for f in jp if f.rom >= last_end), text_end)
        if stop < text_end:
            points[stop] = dict(start=stop, name=f'unresolved/{stop:06X}', reason='unresolved')
    return [points[start] for start in sorted(points)]


def make_report(root):
    us, ranges, us_ram, us_sha = read_region(root, 'us')
    jp, jp_ranges, jp_ram, jp_sha = read_region(root, 'jp')
    matches, candidates, methods, conflicts = match_functions(us, jp, us_ram, jp_ram)
    boundaries = infer_boundaries(us, jp, ranges, matches)
    rows = []
    for i, f in enumerate(us):
        j = matches.get(i)
        g = jp[j] if j is not None else None
        rows.append(dict(us_name=f.name, us_rom=f.rom, us_vram=f.vram, size=f.size, file=f.file,
                         jp_name=g.name if g else None, jp_rom=g.rom if g else None,
                         jp_vram=g.vram if g else None, delta=g.rom-f.rom if g else None,
                         method=methods[i] if g else None,
                         status='matched' if g else 'conflict' if i in conflicts else
                         'ambiguous' if len(candidates[i]) > 1 else
                         'unconfirmed' if candidates[i] else 'unmatched',
                         candidates=[jp[n].rom for n in candidates[i]]))
    counts = Counter(row['method'] for row in rows if row['status'] == 'matched')
    return dict(schema_version=1, rom_sha1=dict(us=us_sha, jp=jp_sha),
                summary=dict(us_functions=len(us), jp_functions=len(jp), matched=len(matches),
                             methods=dict(counts), matched_bytes=sum(us[i].size for i in matches),
                             us_function_bytes=sum(f.size for f in us),
                             supported_boundaries=sum(b['status']=='supported' for b in boundaries),
                             total_boundaries=len(boundaries), conflicts=len(conflicts)),
                functions=rows, boundaries=boundaries,
                assembly_splits=assembly_splits(boundaries, rows, jp, max(r[1] for r in jp_ranges)),
                unmatched_jp=[dict(name=f.name, rom=f.rom, size=f.size)
                              for j, f in enumerate(jp) if j not in set(matches.values())])


def markdown(report):
    s = report['summary']
    lines = ['# US–JP function alignment', '',
             'Generated by `tools/match_rom_functions.py` from verified ROMs and ELF symbols.', '',
             f"Matched **{s['matched']} / {s['us_functions']} US functions** "
             f"({s['jp_functions']} JP functions), covering **{s['matched_bytes']:,} / "
             f"{s['us_function_bytes']:,} US function bytes**.", '',
             f"Methods: {s['methods'].get('exact', 0)} exact-byte matches and "
             f"{s['methods'].get('address-normalized', 0)} address-normalized matches. Supported file starts: "
             f"**{s['supported_boundaries']} / {s['total_boundaries']}**. "
             f"Conflicting functions: {s['conflicts']}.", '',
             'Normalized matches preserve instructions except recognized RAM address operands. '
             'They establish correspondence, not byte equality. File names describe US layout; '
             'they do not prove original JP compilation units. All addresses below are ROM offsets.', '',
             'The JSON companion records every function, candidate, and supporting anchor.', '',
             '| US file | US start | JP start / interval | JP − US | Evidence | Functions mapped |',
             '| --- | ---: | --- | ---: | --- | ---: |']
    for b in report['boundaries']:
        lo, hi = b['jp_interval']
        jp = f"`0x{b['jp_start']:X}`" if b['jp_start'] is not None else f"`0x{lo:X}..{hex(hi) if hi is not None else '?'}`"
        delta = f"{b['jp_start']-b['us_start']:+#x}" if b['jp_start'] is not None else '—'
        lines.append(f"| {b['file']} | `0x{b['us_start']:X}` | {jp} | {delta} | {b['status']} | "
                     f"{b['matched_functions']}/{b['total_functions']} |")
    return '\n'.join(lines)+'\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--output', type=Path, default=Path('reports/region-functions.json'))
    args = parser.parse_args()
    report = make_report(args.root)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2)+'\n')
    args.output.with_suffix('.md').write_text(markdown(report))
    args.output.with_suffix('.splits.yaml').write_text(
        '# Proposed JP main CPU text subsegments; unresolved tails have no US identity.\n' +
        ''.join(f"- [0x{s['start']:X}, asm, {s['name']}]\n" for s in report['assembly_splits']))
    print(json.dumps(report['summary'], indent=2))


if __name__ == '__main__':
    main()
