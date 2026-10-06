"""Relocate every pointer-bank body object without changing its ROM extent.

Keeping encoded size constant isolates decoded audio relocation from ROM DMA and
boot timing changes. Padding belongs only to this temporary test fixture.
"""
from tools.asset_bundles import materialize
from tools.audio_assets import parse_pointer_bank
from tools.huffman_asset import compress_flexible, decompress_huffman_asset
from tools.structured_relocations import pointer_bank_manifest, pack_pointer_bank


def wave_contents(payload):
    parts = parse_pointer_bank(payload)['parts']
    by_offset = {p['offset']: p for p in parts}
    header = next(p for p in parts if p['type'] == 'bank_header')
    result, shared_books, shared_loops = [], {}, {}
    for offset in header['wave_offsets']:
        wave = by_offset[offset]
        book = shared_books.setdefault(wave['book_offset'], len(shared_books))
        loop = shared_loops.setdefault(wave['loop_offset'], len(shared_loops)) if wave['loop_offset'] else None
        result.append((book, loop,
                       tuple(wave[k] for k in ('sample_offset', 'sample_size', 'wave_type', 'flags', 'reserved')),
                       {k: v for k, v in by_offset[wave['book_offset']].items() if k != 'offset'},
                       {k: v for k, v in by_offset[wave['loop_offset']].items() if k != 'offset'}
                       if wave['loop_offset'] else None))
    return result


def prepare(root, rows, out, script, build_dir, linker, run):
    row = next(r for r in rows if r['name'] == 'SOUND_POINTER_BANK')
    manifest, _ = materialize(root, row)
    original = pack_pointer_bank(manifest, mode='matching')
    moved = pointer_bank_manifest(manifest)
    moved['parts'][1:] = reversed(moved['parts'][1:])
    payload = pack_pointer_bank(moved)
    if payload == original or wave_contents(payload) != wave_contents(original):
        raise ValueError('SOUND_POINTER_BANK: reordered fixture changed wave contents or did not move objects')
    encoded = compress_flexible(payload)
    extent = row['rom_end']-row['rom_start']
    if len(encoded) > extent:
        raise ValueError('SOUND_POINTER_BANK: isolated runtime fixture exceeds original encoded extent')
    binary, obj = out/'internal-audio.bin', out/'internal-audio.o'
    binary.write_bytes(encoded+bytes(extent-len(encoded)))
    run(linker, '-r', '-b', 'binary', '-o', obj, binary)
    old = build_dir+'/'+row['object']
    if script.count(old) != 1:
        raise ValueError('SOUND_POINTER_BANK: expected exactly one linker input')
    return script.replace(old, str(obj)), payload


def verify(elf_path, expected):
    from tools.verify_linked_relocations import Elf
    elf = Elf(elf_path)
    section = elf.sections[elf.names.index('.SOUND_POINTER_BANK')]
    payload = decompress_huffman_asset(elf.data[section[4]:section[4]+section[5]])[0]
    if payload != expected:
        raise ValueError('SOUND_POINTER_BANK: final linked decoded objects or fields differ')
    waves = wave_contents(payload)
    if not waves:
        raise ValueError('SOUND_POINTER_BANK: empty linked wave verification')
    return len(waves)
