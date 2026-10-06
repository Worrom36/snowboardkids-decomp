"""Shared segment-2 graphics and segment-3 resource object placement.

Original offsets are extraction identities. Command comments retain those IDs
when commands move; numeric operands are resolved through that original map.
"""
import re
import struct

from tools.asset_relocations import Object, relocate
from tools.graphics_relocations import encoded_parts
from tools.structured_relocations import identify_parts, target_at
from tools.vertex_relocations import vertex_range
from tools.graphics_bindings import command_bindings


def resource_layout(manifest, base, mode='shiftable'):
    m = identify_parts(manifest)
    original = sorted((p for p in m['parts'] if p['type'] != 'padding' and p.get('original', True)),
                      key=lambda p: p['offset'])
    for i, part in enumerate(original):
        end = original[i+1]['offset'] if i+1 < len(original) else m['decompressed_size']
        part.setdefault('size', end-part['offset'])
    data = encoded_parts(m, base)
    objects = [Object(p['id'], data[p['id']], p['offset'],
                      8 if p['type'] not in ('raw', 'padding') else 1)
               for p in m['parts']]
    payload, offsets = relocate(m['name'], objects, [], mode=mode)
    if len(payload) > 0x1000000:
        raise ValueError(f"{m['name']}: resource objects exceed the 24-bit segment offset")
    return m, payload, offsets, data


def command_ids(source, data, original_size, name):
    # An all-or-nothing annotation prevents an inserted command from silently
    # changing the identities of every command following it.
    body = source[source.index('= {')+3:source.rindex('}')]
    rows = re.findall(r'\{\{\s*0x[\da-fA-F]{8},\s*0x[\da-fA-F]{8}\s*\}\}\s*,?([^\n]*)', body)
    ids = [re.search(r'object:([A-Za-z_][A-Za-z_0-9]*)', row) for row in rows]
    if any(ids):
        if not all(ids) or len(ids)*8 != len(data):
            raise ValueError(f'{name}: every graphics word pair requires an object identity')
        result = [match[1] for match in ids]
    else:
        if len(data) != original_size:
            raise ValueError(f'{name}: resizing legacy graphics requires object identities on every word pair')
        result = [f'command_{offset:06X}' for offset in range(0, len(data), 8)]
    if len(set(result)) != len(result):
        raise ValueError(f'{name}: duplicate graphics object identity')
    return result


def layout_graphics(name, streams, manifest, base, mode='shiftable'):
    """Return per-stream bytes, exported command symbols, and resolved records.

    Streams carry name, original offset, data, and ordered stable command IDs.
    References use the format-defined VTX, DL and SETTIMG operands only.
    """
    m, resources, resource_offsets, resource_data = resource_layout(manifest, base, mode)
    resource_parts = {p['id']: p for p in m['parts']}
    originals, locations, exports = {}, {}, {}
    cursor = 0
    for stream in streams:
        start = stream['offset'] if mode == 'matching' else cursor
        if start % 8 or start < cursor or len(stream['data']) % 8:
            raise ValueError(f"{name}/{stream['name']}: invalid graphics alignment or overlap")
        if len(stream['ids'])*8 != len(stream['data']):
            raise ValueError(f"{name}/{stream['name']}: command identity coverage is incomplete")
        for i, identity in enumerate(stream['ids']):
            key = stream['name']+'/'+identity
            if key in locations or not re.fullmatch(r'[A-Za-z_][A-Za-z_0-9]*', identity):
                raise ValueError(f'{name}: duplicate or invalid command identity {key}')
            locations[key] = start+i*8
            exports[stream['name']+'_'+identity] = 0x02000000+start+i*8
            if re.fullmatch(r'command_[0-9A-Fa-f]{6}', identity):
                old = stream['offset']+int(identity[8:], 16)
                if old in originals:
                    raise ValueError(f'{name}: duplicate original graphics offset 0x{old:X}')
                originals[old] = key
        cursor = start+len(stream['data'])
    if cursor > 0x1000000:
        raise ValueError(f'{name}: graphics exceed the 24-bit segment offset')
    bindings = command_bindings(m)
    address_sources = {stream['name']+'/'+identity
                       for stream in streams for i, identity in enumerate(stream['ids'])
                       if stream['data'][i*8] in (4, 6, 0xfd)}
    for source in bindings:
        if source not in address_sources:
            raise ValueError(f'{name}/{source}+4: binding has no supported address operand')
    packed, records = {}, []
    for stream in streams:
        data = bytearray(stream['data'])
        for i, identity in enumerate(stream['ids']):
            w0, w1 = struct.unpack_from('>II', data, i*8)
            opcode = w0 >> 24
            if opcode not in (4, 6, 0xfd):
                continue
            source = stream['name']+'/'+identity
            binding = bindings.pop(source, None)
            old = w1 & 0xffffff
            if opcode == 6:
                if binding:
                    if binding['segment'] != 2 or binding.get('addend', 0) or 'vertices' in binding:
                        raise ValueError(f'{name}/{source}+4: display-list binding requires a segment-2 command identity')
                    target = binding['target']
                    if target not in locations:
                        raise ValueError(f'{name}/{source}+4: unresolved display-list command {target}')
                else:
                    if w1 >> 24 != 2 or old not in originals:
                        raise ValueError(f'{name}/{source}+4: unresolved segment-2 display list 0x{w1:X}')
                    target = originals[old]
                addend = 0
                value = 0x02000000+locations[target]
            else:
                kinds = ('vertices',) if opcode == 4 else ('texture', 'palette')
                if binding:
                    prefix = m['name']+'/'
                    if binding['segment'] != 3 or not binding['target'].startswith(prefix):
                        raise ValueError(f'{name}/{source}+4: resource binding requires this segment-3 bank')
                    target, addend = binding['target'][len(prefix):], binding.get('addend', 0)
                    if target not in resource_parts or resource_parts[target]['type'] not in kinds:
                        raise ValueError(f'{name}/{source}+4: unresolved or invalid {kinds} target {target}')
                    if opcode != 4 and 'vertices' in binding:
                        raise ValueError(f'{name}/{source}+4: vertex identities require a vertex-load command')
                else:
                    if w1 >> 24 != 3:
                        raise ValueError(f'{name}/{source}+4: unsupported graphics segment {w1 >> 24}')
                    target, addend = target_at(dict(m, parts=[p for p in m['parts']
                                                             if p['type'] != 'padding' and p.get('original', True)]),
                                              old, kinds=kinds)
                if opcode == 4:
                    count = (w0 >> 10) & 63
                    addend, _ = vertex_range(resource_parts[target], addend, count, f'{name}/{source}+4 -> {target}',
                                             targets=binding.get('vertices') if binding else None)
                if addend >= len(resource_data[target]):
                    raise ValueError(f'{name}/{source}+4: offset exceeds {target}')
                value = 0x03000000+resource_offsets[target]+addend
                target = m['name']+'/'+target
            struct.pack_into('>I', data, i*8+4, value)
            records.append(dict(source=source, field=4, target=target, kind='segmented',
                                addend=addend, value=value, location=locations[source]+4))
        packed[stream['name']] = bytes(data)
    if bindings:
        raise ValueError(f'{name}: unused graphics bindings {sorted(bindings)}')
    from tools.course_graphics_common import CourseGraphicsGraph, DisplayListNode
    from tools.texture_relocations import validate_texture_loads
    whole = bytearray(cursor)
    for stream in streams:
        start = locations[stream['name']+'/'+stream['ids'][0]] if stream['ids'] else 0
        whole[start:start+len(packed[stream['name']])] = packed[stream['name']]
    graph = CourseGraphicsGraph([DisplayListNode(0, len(whole))], [])
    validate_texture_loads(name, whole, graph, m['parts'], resource_offsets, resource_data, len(resources), 3)
    return packed, exports, records, resources


def graphics_groups(root):
    from tools.asset_bundles import source_segments
    _, specs = source_segments(root)
    return [(bank, [s for s in specs if s['type'] == 'course_display_list'
                    and bank['graphics_start'] <= s['start'] < bank['graphics_end']])
            for bank in specs if bank['type'] in ('model_resources', 'course_model_resources')]


def pack_graphics_group(root, row, mode='shiftable'):
    from tools.asset_bundles import commands, load_yaml, LAYOUT, materialize
    rows = {r['name']: r for r in load_yaml(root/LAYOUT)['segments']}
    matches = [(b, g) for b, g in graphics_groups(root) if any(s['name'] == row['name'] for s in g)]
    if len(matches) != 1:
        raise ValueError(f"{row['name']}: expected exactly one graphics resource group")
    bank, group = matches[0]
    streams = []
    for spec in group:
        path = root/rows[spec['name']]['source']
        data = commands(path)
        ids = command_ids(path.read_text(), data, spec['end']-spec['start'], spec['name'])
        streams.append(dict(name=spec['name'], offset=spec['start']-bank['graphics_start'], data=data, ids=ids))
    manifest, path = materialize(root, rows[bank['name']])
    return layout_graphics(bank['name'], streams, manifest, path.parent, mode)
