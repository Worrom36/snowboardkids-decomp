#!/usr/bin/env python3
"""Inventory address candidates and ELF relocations, including ignored inputs.

Numeric candidates require semantic review; this report is not a pointer oracle.
"""
import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

HEX = re.compile(r'\b0x[0-9a-fA-F]{5,8}\b')


def category(value):
    if 0xA4000000 <= value < 0xC0000000:
        return 'hardware-or-uncached-review'
    if 0x80000000 <= value < 0x80800000:
        return 'cpu-ram-candidate'
    if 0x01000000 <= value < 0x10000000:
        return 'segmented-candidate'
    if value < 0x800000:
        return 'rom-physical-or-relative-candidate'
    return 'constant-or-address-review'


def inventory(root, build):
    sources = []
    for directory in ('src', 'include', 'asm', 'assets', 'linker_scripts'):
        for path in sorted((root / directory).rglob('*')):
            if path.suffix not in ('.c', '.h', '.s', '.ld'):
                continue
            for number, line in enumerate(path.read_text(errors='replace').splitlines(), 1):
                # Strip generated disassembly addresses and instruction bytes.
                line = re.sub(r'/\*.*?\*/', '', line)
                for match in HEX.finditer(line):
                    sources.append(dict(path=str(path.relative_to(root)), line=number,
                                        token=match[0], classification=category(int(match[0], 16))))
                for match in re.finditer(r'\blui\s+[^,]+,\s*(0x[0-9a-fA-F]{4})\b', line):
                    sources.append(dict(path=str(path.relative_to(root)), line=number,
                                        token=match[1], classification='instruction-high-half-review'))
    reader = next((shutil.which(p + 'readelf') for p in
                   ('mips-linux-gnu-', 'mips64-linux-gnu-', 'mips64-elf-') if shutil.which(p + 'readelf')), None)
    if not reader:
        raise ValueError('MIPS readelf is required for object relocation inventory')
    objects = []
    for path in sorted(build.rglob('*.o')):
        result = subprocess.run([reader, '-rW', str(path)], check=True, text=True, capture_output=True)
        records = [line.strip() for line in result.stdout.splitlines() if 'R_MIPS_' in line]
        objects.append(dict(path=str(path.relative_to(root)), relocations=records))
    if not sources or not objects or not any(o['relocations'] for o in objects):
        raise ValueError('inventory requires extracted sources and compiled relocation-bearing objects')
    packed = packed_inventory(root, build)
    from tools.check_ram_references import inspect
    cpu = inspect(build/'snowboardkids.elf')
    linker = []
    for path in [root/'snowboardkids.ld', *sorted(build.glob('*.ld'))]:
        for number, line in enumerate(path.read_text().splitlines(), 1):
            if HEX.search(line):
                linker.append(dict(path=str(path.relative_to(root)), line=number, text=line.strip()))
    return dict(schema_version=2, numeric_candidates_require_review=True, sources=sources, objects=objects,
                packed_assets=packed, cpu_data_coverage=cpu, generated_linker_addresses=linker)


def packed_inventory(root, build):
    from tools.asset_bundles import LAYOUT, load_yaml, materialize
    from tools.animation_relocations import identify
    from tools.structured_relocations import music_manifest, pointer_bank_manifest
    from tools.graphics_relocations import model_manifest
    from tools.sprite_relocations import sprite_manifest
    from tools.tilemap_relocations import tilemap_manifest
    from tools.sample_relocations import bind_pointer_bank
    from tools.surface_relocations import surface_manifest
    result = []
    seen_graphics = set()
    for row in load_yaml(root/LAYOUT)['segments']:
        kind = row['decoder']
        if kind == 'course_display_list':
            path = build/Path(row['object']).with_suffix('.relocations.json')
            if not path.exists():
                raise ValueError(f"{row['name']}: missing generated graphics relocation report {path}")
            records = []
            for record in json.loads(path.read_text()):
                key = (record['source'], record['field'])
                if key not in seen_graphics:
                    seen_graphics.add(key)
                    records.append(record)
            result.append(dict(asset=row['name'], decoder=kind, records=records,
                               status='shared-group references deduplicated by source identity'))
            continue
        m, path = materialize(root, row)
        if kind in ('race_animation', 'scene_animation'):
            m = identify(m)
        elif kind == 'pointer_bank':
            m = pointer_bank_manifest(bind_pointer_bank(root, m))
        elif kind == 'music_sequence':
            m = music_manifest(m)
        elif kind == 'embedded_model':
            m = model_manifest(m, path.parent)
        elif kind in ('sprite_table', 'course_sprite_table'):
            m = sprite_manifest(m)
        elif kind == 'tilemap':
            m = tilemap_manifest(m)
        elif kind == 'course_surface_data':
            m = surface_manifest(m)
        records = list(m.get('relocations', []))+list(m.get('event_relocations', []))+list(m.get('index_relocations', []))
        if kind == 'pointer_bank':
            for part in m['parts']:
                if part['type'] == 'wavetable':
                    for field, encoding in ((0, 'relative'), (4, 'size')):
                        records.append(dict(source=part['id'], field=field, target=part['sample_id'],
                                            kind=encoding, base='shared_sample_stream', width=4))
        status = 'explicit format references'
        if kind in ('model_resources', 'course_model_resources', 'sample_bank'):
            status = 'reference targets; users are recorded in graphics or pointer banks'
        elif kind == 'replay':
            status = 'fixed-count button/stick records; no pointer fields'
        elif kind == 'course_surface_data':
            status = 'explicit coordinate/face indices; CPU course-progress surface order is constrained'
        result.append(dict(asset=row['name'], decoder=kind, manifest=row['source'], records=records, status=status))
    if not seen_graphics or not any(a['records'] for a in result if a['decoder'] == 'music_sequence'):
        raise ValueError('packed relocation inventory unexpectedly empty')
    return result


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    p.add_argument('--build', default='build')
    p.add_argument('--out', type=Path, required=True)
    a = p.parse_args()
    report = inventory(a.root.resolve(), a.root.resolve() / a.build)
    a.out.parent.mkdir(parents=True, exist_ok=True)
    a.out.write_text(json.dumps(report, indent=2) + '\n')
    print(f"Inventoried {len(report['sources'])} numeric candidates and {len(report['objects'])} objects")


if __name__ == '__main__':
    main()
