"""Mus event identities, including all three Fgoto restart-relative operands."""
from dataclasses import asdict

from tools.asset_relocations import Object, Reference, relocate
from tools.music_assets import encode_music_part, parse_events, parse_music


def encode_event(part, event):
    record = dict(event, offset=0)
    return encode_music_part(dict(part, offset=0, events=[record]))


def event_manifest(manifest):
    from tools.structured_relocations import music_table_manifest
    m = music_table_manifest(manifest)
    if 'event_relocations' in m:
        return m
    parts = {p['id']: p for p in m['parts']}
    old_events = {}
    for part in m['parts']:
        if part['type'] not in ('sequence', 'modulation'):
            continue
        offset = part['offset']
        for event in part['events']:
            original = event.get('offset', offset)
            event.setdefault('id', f"event_{original-part['offset']:06X}")
            event.setdefault('offset', original)
            key = part['id']+'/'+event['id']
            if original in old_events:
                raise ValueError(f"{m['name']}: overlapping event at 0x{original:X}")
            old_events[original] = key
            offset = original+len(encode_event(part, event))
    tables = {p['role']: p for p in m['parts'] if p['type'] == 'pointers'
              and p['role'] in ('sequence', 'volume', 'pitch')}
    if set(tables) != {'sequence', 'volume', 'pitch'}:
        raise ValueError(f"{m['name']}: missing track pointer tables")
    event_refs = []
    # Preserve the earlier table schema while replacing its sequence byte
    # offsets with source/target event identities.
    for record in m['relocations']:
        if parts[record['source']]['type'] != 'sequence':
            continue
        part = parts[record['source']]
        event = next((e for e in part['events'] if e.get('command') == 'goto'
                      and e['offset']-part['offset']+1 == record['field']), None)
        if event is None:
            raise ValueError(f"{m['name']}/{part['id']}: branch field has no goto event")
        source = part['id']+'/'+event['id']
        event['goto_bases'] = {}
        for field, role in ((1, 'sequence'), (3, 'volume'), (5, 'pitch')):
            addend = int.from_bytes(bytes(event['operands'][field-1:field+1]), 'big')
            bases = {part['offset']} if role == 'sequence' else {
                tables[role]['values'][i] for i, value in enumerate(tables['sequence']['values'])
                if value == part['offset']}
            if not bases:
                if addend:
                    raise ValueError(f"{m['name']}/{source}+{field}: missing {role} track context")
                event['goto_bases'][role] = None
                continue
            if len(bases) != 1:
                raise ValueError(f"{m['name']}/{source}+{field}: shared goto has multiple {role} restart contexts")
            base = bases.pop()
            if base == 0:
                if addend:
                    raise ValueError(f"{m['name']}/{source}+{field}: nonzero offset from null {role} stream")
                event['goto_bases'][role] = None
                continue
            base_part = next((p for p in m['parts'] if p['offset'] == base
                              and p['type'] in ('sequence', 'modulation')), None)
            if base_part is None or base+addend not in old_events:
                raise ValueError(f"{m['name']}/{source}+{field}: {role} branch does not target an event boundary")
            event_refs.append(asdict(Reference(source, field, old_events[base+addend],
                                               width=2, base=base_part['id'])))
            event['goto_bases'][role] = base_part['id']
    m['event_relocations'] = event_refs
    m['relocations'] = [r for r in m['relocations'] if parts[r['source']]['type'] != 'sequence']
    return m


def pack_events(manifest, mode='shiftable'):
    m = event_manifest(manifest)
    objects, expected = [], set()
    event_types, stream_events = {}, {}
    for part in m['parts']:
        kind, identity = part['type'], part['id']
        if kind in ('sequence', 'modulation'):
            if not part['events']:
                raise ValueError(f"{m['name']}/{identity}: empty music stream")
            keys = []
            offset = part['offset']
            for event in part['events']:
                if 'id' not in event:
                    raise ValueError(f"{m['name']}/{identity}: new event requires a stable id")
                key = identity+'/'+event['id']
                encoded = encode_event(part, event)
                objects.append(Object(key, encoded, event.get('offset', offset)))
                event_types[key] = kind
                keys.append(key)
                offset += len(encoded)
            objects.append(Object(identity, b'', part['offset'], alias=keys[0]))
            stream_events[identity] = keys
        else:
            data = bytes.fromhex(part['data']) if kind in ('raw', 'padding') else encode_music_part(part)
            objects.append(Object(identity, data, part['offset'],
                                  4 if kind in ('sequence_header', 'pointers') else 1))
            if kind == 'sequence_header':
                expected.update((identity, 4+4*i) for i, key in enumerate(
                    ('sequence_table', 'volume_table', 'pitch_table', 'envelope_table', 'drum_table')) if part[key])
            elif kind == 'pointers':
                expected.update((identity, 4*i) for i, value in enumerate(part['values']) if value)
    refs = [Reference(**r) for r in m['relocations']]
    if {(r.source, r.field) for r in refs} != expected:
        raise ValueError(f"{m['name']}: music table relocation coverage is incomplete")
    if any(r.kind != 'relative' or r.width != 4 or r.unit != 1 or r.base is not None for r in refs):
        raise ValueError(f"{m['name']}: music tables require 32-bit asset-relative offsets")
    event_refs = [Reference(**r) for r in m['event_relocations']]
    by_source = {}
    for ref in event_refs:
        if ref.width != 2 or ref.kind != 'relative' or ref.field not in (1, 3, 5) or ref.addend or ref.unit != 1:
            raise ValueError(f"{m['name']}/{ref.source}: invalid goto relocation encoding")
        expected_type = 'sequence' if ref.field == 1 else 'modulation'
        if event_types.get(ref.target) != expected_type or ref.base not in stream_events:
            raise ValueError(f"{m['name']}/{ref.source}: unresolved or invalid {expected_type} event target")
        if ref.target not in stream_events[ref.base]:
            raise ValueError(f"{m['name']}/{ref.source}: goto target belongs to another restart stream")
        by_source.setdefault(ref.source, set()).add(ref.field)
    for part in m['parts']:
        if part['type'] != 'sequence':
            continue
        for event in part['events']:
            key = part['id']+'/'+event['id']
            fields = by_source.pop(key, set())
            if event.get('command') == 'goto':
                bases = event.get('goto_bases', {})
                if set(bases) != {'sequence', 'volume', 'pitch'} or bases['sequence'] != part['id']:
                    raise ValueError(f"{m['name']}/{key}: missing goto restart identities")
                expected_fields = {field for field, role in ((1, 'sequence'), (3, 'volume'), (5, 'pitch'))
                                   if bases[role] is not None}
                if fields != expected_fields:
                    raise ValueError(f"{m['name']}/{key}: missing goto relocation coverage")
                for field in (3, 5):
                    if field not in fields and any(event['operands'][field-1:field+1]):
                        raise ValueError(f"{m['name']}/{key}: nonzero offset from null modulation stream")
                for ref in event_refs:
                    if ref.source == key and ref.base != bases[{1: 'sequence', 3: 'volume', 5: 'pitch'}[ref.field]]:
                        raise ValueError(f"{m['name']}/{key}: goto reference uses the wrong restart stream")
            elif fields:
                raise ValueError(f"{m['name']}/{key}: relocation on a non-goto event")
    if by_source:
        raise ValueError(f"{m['name']}: unresolved branch source {next(iter(by_source))}")
    data, offsets = relocate(m['name'], objects, refs+event_refs, mode=mode)
    # Re-parse each stream with its actual event boundaries: malformed operand
    # lengths or duration/velocity state must not silently shift later events.
    lengths = {o.id: len(o.data) for o in objects}
    for part in m['parts']:
        if part['type'] != 'sequence':
            continue
        keys = stream_events[part['id']]
        start = offsets[keys[0]]
        end = offsets[keys[-1]]+lengths[keys[-1]]
        try:
            events, stop = parse_events(data, start, end)
        except ValueError as exc:
            raise ValueError(f"{m['name']}/{part['id']}: {exc}") from exc
        if stop != end or [e['offset'] for e in events] != [offsets[k] for k in keys]:
            raise ValueError(f"{m['name']}/{part['id']}: encoded event boundaries disagree with command state")
    parse_music(data)
    return data
