"""Move the real shared VADPCM sample stream using temporary source manifests."""
import copy
import hashlib
import json

import yaml

from tools.asset_bundles import materialize
from tools.audio_shift_fixture import wave_contents
from tools.audio_assets import parse_pointer_bank
from tools.huffman_asset import compress_flexible, decompress_huffman_asset
from tools.sample_relocations import sample_group, layout_samples, bind_pointer_bank
from tools.structured_relocations import pack_pointer_bank


def sample_contents(bank, samples):
    parts = parse_pointer_bank(bank)['parts']
    by_offset = {p['offset']: p for p in parts}
    header = next(p for p in parts if p['type'] == 'bank_header')
    shared, result = {}, []
    for address, contents in zip(header['wave_offsets'], wave_contents(bank)):
        wave = by_offset[address]
        start, size = wave['sample_offset'], wave['sample_size']
        if start < 16 or start % 16 or size <= 0 or size % 9 or start+size > len(samples):
            raise ValueError(f'sample fixture: wave at 0x{address:X} has invalid sample bounds/alignment')
        contents = list(contents)
        contents[2] = contents[2][1:]  # storage offset is replaced by target content
        result.append((shared.setdefault(start, len(shared)),
                       hashlib.sha256(samples[start:start+size]).hexdigest(), contents))
    if len(result) != 251:
        raise ValueError('sample fixture: expected every one of the 251 wave references')
    return result


def make_sources(root, out):
    """Copy only needed manifests and sample bytes; never edit game sources."""
    rows, samples, signature = sample_group(root, 'SOUND_POINTER_BANK')
    out.mkdir(parents=True, exist_ok=True)
    (out/'assets/layout').mkdir(parents=True, exist_ok=True)
    sample_paths = {}
    for original, (identity, path, size) in samples.items():
        name = f'sample_{original:06x}.vadpcm'
        (out/name).write_bytes(path.read_bytes())
        sample_paths[path] = name
    temporary = []
    for index, (row, manifest, path) in enumerate(rows):
        moved = copy.deepcopy(manifest)
        moved.pop('sections', None)
        for part in moved['parts']:
            part.pop('source', None)
            if part['type'] == 'sample':
                part['path'] = sample_paths[(path.parent/part['path']).resolve()]
        if index == 0:
            moved['sample_order'] = [samples[offset][0] for offset in reversed(sorted(samples))]
        name = f'partition-{index}.yaml'
        (out/name).write_text(yaml.safe_dump(moved))
        temporary.append(dict(row, source=name))
    (out/'assets/layout/segments.yaml').write_text(yaml.safe_dump(dict(segments=temporary)))
    return rows


def prepare(root, rows, out, source, build_dir, linker, run):
    source_root = out/'sample-sources'
    sample_rows = make_sources(root, source_root)
    stream, offsets, sizes, targets, _ = layout_samples(source_root, 'SOUND_POINTER_BANK')
    row = next(r for r in rows if r['name'] == 'SOUND_POINTER_BANK')
    manifest, _ = materialize(root, row)
    bank = pack_pointer_bank(bind_pointer_bank(source_root, manifest))
    original_rom = (root/'snowboardkids.z64').read_bytes()
    original_bank = decompress_huffman_asset(original_rom[row['rom_start']:row['rom_end']])[0]
    original_stream = original_rom[sample_rows[0][0]['rom_start']:sample_rows[-1][0]['rom_end']]
    expected = sample_contents(original_bank, original_stream)
    if stream == original_stream or sample_contents(bank, stream) != expected:
        raise ValueError('sample fixture: reordered wave targets changed contents or sharing')
    if len(stream) != len(original_stream):
        raise ValueError('sample fixture: pure reordering unexpectedly changed stream extent')
    encoded_bank = compress_flexible(bank)
    replacements = [(row, encoded_bank)]
    cursor = 0
    for part, _, _ in sample_rows:
        length = part['rom_end']-part['rom_start']
        replacements.append((part, stream[cursor:cursor+length]))
        cursor += length
    for part, data in replacements:
        binary, obj = out/(part['name']+'.bin'), out/(part['name']+'.o')
        binary.write_bytes(data)
        run(linker, '-r', '-b', 'binary', '-o', obj, binary)
        old = build_dir+'/'+part['object']
        if source.count(old) != 1:
            raise ValueError(f"{part['name']}: expected one sample fixture input")
        source = source.replace(old, str(obj))
    record = dict(bank=row['name'], partitions=[r['name'] for r, _, _ in sample_rows],
                  bank_sha256=hashlib.sha256(bank).hexdigest(),
                  stream_sha256=hashlib.sha256(stream).hexdigest(),
                  semantic_sha256=hashlib.sha256(json.dumps(expected, sort_keys=True).encode()).hexdigest())
    (out/'sample-fields.json').write_text(json.dumps(record, indent=2)+'\n')
    return source


def verify(elf_path, report_path):
    from tools.verify_linked_relocations import Elf
    elf = Elf(elf_path)
    record = json.loads(report_path.read_text())

    def section(name):
        name = '.'+('_' if name[0].isdigit() else '')+name
        if name not in elf.names:
            raise ValueError(f'{name}: missing sample fixture section')
        s = elf.sections[elf.names.index(name)]
        return elf.data[s[4]:s[4]+s[5]]

    bank = decompress_huffman_asset(section(record['bank']))[0]
    stream = b''.join(section(name) for name in record['partitions'])
    contents = sample_contents(bank, stream)
    if (hashlib.sha256(bank).hexdigest() != record['bank_sha256'] or
            hashlib.sha256(stream).hexdigest() != record['stream_sha256'] or
            hashlib.sha256(json.dumps(contents, sort_keys=True).encode()).hexdigest() != record['semantic_sha256']):
        raise ValueError('sample fixture: linked wave targets changed contents or sharing')
    return len(contents)
