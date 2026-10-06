"""Relocation records derived from decoded format fields, never integer scans."""
import copy
from dataclasses import asdict

from tools.asset_relocations import Object, Reference, relocate


def identify_parts(manifest):
    m = copy.deepcopy(manifest)
    for part in m['parts']:
        part.setdefault('id', f"{part['type']}_{part['offset']:06x}")
    return m


def target_at(m, offset, *, kinds=None):
    for part in m['parts']:
        if part['offset'] <= offset < part['offset'] + part['size']:
            if kinds is not None and part['type'] not in kinds:
                raise ValueError(f"{m['name']}: pointer 0x{offset:X} targets {part['type']}, expected {kinds}")
            return part['id'], offset-part['offset']
    raise ValueError(f"{m['name']}: unresolved interior target at 0x{offset:X}")


def pointer_bank_manifest(manifest):
    m = identify_parts(manifest)
    if 'relocations' in m:
        return m
    refs = []
    for part in m['parts']:
        if part['type'] == 'bank_header':
            fields = [(20+i*4, value, ('wavetable',)) for i, value in enumerate(part['wave_offsets'])]
        elif part['type'] == 'wavetable':
            fields = [(12, part['loop_offset'], ('loop',)), (16, part['book_offset'], ('codebook',))]
        else:
            fields = []
        for field, value, kinds in fields:
            if value:
                target, addend = target_at(m, value, kinds=kinds)
                refs.append(Reference(part['id'], field, target, addend=addend))
    m['relocations'] = [asdict(ref) for ref in refs]
    return m


def pack_pointer_bank(manifest, mode='shiftable'):
    from tools.audio_assets import encode_pointer_part, parse_pointer_bank
    m = pointer_bank_manifest(manifest)
    objects, expected_fields = [], set()
    for part in m['parts']:
        kind = part['type']
        data = bytes.fromhex(part['data']) if kind in ('raw', 'padding') else encode_pointer_part(part)
        objects.append(Object(part['id'], data, part['offset'], 4 if kind not in ('raw', 'padding') else 1))
        if kind == 'bank_header':
            expected_fields.update((part['id'], 20+i*4) for i in range(len(part['wave_offsets'])))
        elif kind == 'wavetable':
            expected_fields.add((part['id'], 16))
            if part['loop_offset']:
                expected_fields.add((part['id'], 12))
    refs = [Reference(**r) for r in m['relocations']]
    if {(r.source, r.field) for r in refs} != expected_fields:
        raise ValueError(f"{m['name']}: pointer-bank relocation coverage is incomplete")
    if any(r.kind != 'relative' or r.width != 4 or r.unit != 1 for r in refs):
        raise ValueError(f"{m['name']}: pointer-bank fields require 32-bit byte offsets")
    result, offsets = relocate(m['name'], objects, refs, mode=mode)
    parse_pointer_bank(result)
    return result


def music_manifest(manifest):
    m = identify_parts(manifest)
    if 'relocations' in m:
        return m
    refs = []
    for part in m['parts']:
        kind = part['type']
        if kind == 'sequence_header':
            fields = [(4+4*i, part[key]) for i, key in enumerate(
                ('sequence_table', 'volume_table', 'pitch_table', 'envelope_table', 'drum_table'))]
        elif kind == 'pointers':
            fields = [(4*i, value) for i, value in enumerate(part['values'])]
        else:
            fields = []
        for field, value in fields:
            if value:
                target, addend = target_at(m, value)
                refs.append(Reference(part['id'], field, target, addend=addend))
        if kind == 'sequence':
            for event in part['events']:
                if event.get('command') == 'goto':
                    # Mus Fgoto adds its first u16 operand to restartPos.
                    offset = int.from_bytes(bytes(event['operands'][:2]), 'big')
                    target, addend = target_at(m, part['offset']+offset, kinds=('sequence',))
                    refs.append(Reference(part['id'], event['offset']-part['offset']+1, target,
                                          addend=addend, width=2, base=part['id']))
    m['relocations'] = [asdict(ref) for ref in refs]
    return m


def pack_music(manifest, mode='shiftable'):
    from tools.music_assets import encode_music_part, parse_music
    m = music_manifest(manifest)
    expected = set()
    for part in m['parts']:
        if part['type'] == 'sequence_header':
            expected.update((part['id'], 4+4*i) for i, key in enumerate(
                ('sequence_table', 'volume_table', 'pitch_table', 'envelope_table', 'drum_table')) if part[key])
        elif part['type'] == 'pointers':
            expected.update((part['id'], 4*i) for i, value in enumerate(part['values']) if value)
        elif part['type'] == 'sequence':
            expected.update((part['id'], event['offset']-part['offset']+1)
                            for event in part['events'] if event.get('command') == 'goto')
    refs = [Reference(**r) for r in m['relocations']]
    if {(r.source, r.field) for r in refs} != expected:
        raise ValueError(f"{m['name']}: music relocation coverage is incomplete")
    objects = []
    for part in m['parts']:
        kind = part['type']
        data = bytes.fromhex(part['data']) if kind in ('raw', 'padding') else encode_music_part(part)
        if kind in ('sequence', 'modulation') and len(data) != part['size']:
            raise ValueError(f"{m['name']}/{part['id']}: resizing event streams requires event-level branch identities")
        objects.append(Object(part['id'], data, part['offset'],
                              4 if kind in ('sequence_header', 'pointers') else 1))
    result, offsets = relocate(m['name'], objects, refs, mode=mode)
    parse_music(result)
    return result
