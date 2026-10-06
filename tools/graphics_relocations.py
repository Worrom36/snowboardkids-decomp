"""F3DEX embedded objects and link-visible segmented display-list roots."""
from dataclasses import asdict, replace
import struct

from tools.asset_relocations import Object, Reference, relocate
from tools.model_assets import encode_embedded_part
from tools.structured_relocations import identify_parts, target_at
from tools.course_graphics_common import trace_course_graphics
from tools.vertex_relocations import vertex_range


def encoded_parts(m, base):
    return {p['id']: (bytes.fromhex(p['data']) if p['type'] in ('raw', 'padding')
                      else encode_embedded_part(p, base)) for p in m['parts']}


def pointer_fields(m, data):
    for part in m['parts']:
        if part['type'] != 'display_list':
            continue
        encoded = data[part['id']]
        if len(encoded) % 8:
            raise ValueError(f"{m['name']}/{part['id']}: partial display-list command")
        for field in range(0, len(encoded), 8):
            word0, word1 = struct.unpack_from('>II', encoded, field)
            opcode = word0 >> 24
            if opcode in (0x01, 0x03, 0xbc):
                raise ValueError(f"{m['name']}/{part['id']}+0x{field:X}: unsupported matrix/memory/segment command")
            if opcode in (0x04, 0x06, 0xfd):
                if word1 >> 24 != 2:
                    raise ValueError(f"{m['name']}/{part['id']}+0x{field+4:X}: external graphics segment requires a cross-asset binding")
                kinds = {4: ('vertices',), 6: ('display_list',), 0xfd: ('texture', 'palette')}[opcode]
                yield part, field+4, word0, word1 & 0xffffff, kinds


def model_manifest(manifest, base):
    m = identify_parts(manifest)
    data = encoded_parts(m, base)
    if 'relocations' not in m:
        refs = []
        for part, field, word0, value, kinds in pointer_fields(m, data):
            target, addend = target_at(m, value, kinds=kinds)
            refs.append(Reference(part['id'], field, target, 'segmented', addend=addend, segment=2))
        m['relocations'] = [asdict(r) for r in refs]
    if 'exports' not in m:
        m['exports'] = []
        for root in m['root_offsets']:
            target, addend = target_at(m, root, kinds=('display_list',))
            m['exports'].append(dict(symbol=f"{m['name']}_ROOT_{root:06X}", target=target, addend=addend))
    return m


def pack_model(manifest, base, mode='shiftable'):
    m = model_manifest(manifest, base)
    data = encoded_parts(m, base)
    fields = list(pointer_fields(m, data))
    refs = [Reference(**record) for record in m['relocations']]
    if {(r.source, r.field) for r in refs} != {(p['id'], f) for p, f, _, _, _ in fields}:
        raise ValueError(f"{m['name']}: display-list relocation coverage is incomplete")
    parts = {p['id']: p for p in m['parts']}
    resolved = []
    for ref in refs:
        if ref.kind != 'segmented' or ref.segment != 2 or ref.width != 4 or ref.unit != 1:
            raise ValueError(f"{m['name']}/{ref.source}+0x{ref.field:X}: expected a segment-2 word")
        target = parts.get(ref.target)
        if target is None:
            raise ValueError(f"{m['name']}/{ref.source}: unresolved target {ref.target}")
        word0 = int.from_bytes(data[ref.source][ref.field-4:ref.field], 'big')
        opcode = word0 >> 24
        expected_kind = {4: ('vertices',), 6: ('display_list',), 0xfd: ('texture', 'palette')}[opcode]
        if target['type'] not in expected_kind:
            raise ValueError(f"{m['name']}/{ref.source}: invalid {target['type']} target for opcode 0x{opcode:X}")
        if opcode == 4:
            count = (word0 >> 10) & 63
            addend, _ = vertex_range(target, ref.addend, count, f"{m['name']}/{ref.source}+{ref.field}")
            ref = replace(ref, addend=addend)
        resolved.append(ref)
    objects = [Object(p['id'], data[p['id']], p['offset'], 8 if p['type'] not in ('raw', 'padding') else 1)
               for p in m['parts']]
    result, offsets = relocate(m['name'], objects, resolved, mode=mode)
    exports = {}
    for record in m['exports']:
        target, addend = record['target'], record.get('addend', 0)
        if target not in parts or parts[target]['type'] != 'display_list':
            raise ValueError(f"{m['name']}/{record['symbol']}: unresolved display-list export")
        if addend < 0 or addend % 8 or addend >= len(data[target]):
            raise ValueError(f"{m['name']}/{record['symbol']}: invalid display-list export offset")
        if record['symbol'] in exports:
            raise ValueError(f"{m['name']}: duplicate exported root {record['symbol']}")
        exports[record['symbol']] = 0x02000000 + offsets[target] + addend
    if len(exports) != len(m['root_offsets']) or not exports:
        raise ValueError(f"{m['name']}: missing exported display-list roots")
    graph = trace_course_graphics(result, [value & 0xffffff for value in exports.values()])
    from tools.texture_relocations import validate_texture_loads
    validate_texture_loads(m['name'], result, graph, m['parts'], offsets, data, len(result), 2)
    return result, exports
