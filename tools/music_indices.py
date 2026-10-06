"""Mus track counts and variable-length envelope index bindings."""


def decode_index(operands, source):
    if (not isinstance(operands, list) or len(operands) not in (1, 2) or
            any(not isinstance(v, int) or not 0 <= v <= 255 for v in operands) or
            bool(operands[0] & 128) != (len(operands) == 2)):
        raise ValueError(f'{source}: malformed variable-length envelope index')
    return operands[0] if len(operands) == 1 else ((operands[0] & 127) << 8) | operands[1]


def identify_envelopes(manifest):
    tables = [p for p in manifest['parts'] if p['type'] == 'envelopes']
    if len(tables) > 1:
        raise ValueError(f"{manifest['name']}: multiple envelope tables are unsupported")
    ids = []
    if tables:
        table = tables[0]
        table.setdefault('entry_ids', [f"{table['id']}/envelope_{i:04x}" for i in range(len(table['entries']))])
        ids = table['entry_ids']
        if (not isinstance(ids, list) or len(ids) != len(table['entries']) or any(not isinstance(i, str) or not i for i in ids) or
                len(set(ids)) != len(ids)):
            raise ValueError(f"{manifest['name']}/{table['id']}: missing or duplicate envelope identities")
        if any(not isinstance(entry, list) or len(entry) != 7 or
               any(not isinstance(value, int) or not 0 <= value <= 255 for value in entry)
               for entry in table['entries']):
            raise ValueError(f"{manifest['name']}/{table['id']}: envelopes require seven unsigned bytes each")
    references = []
    for part in manifest['parts']:
        if part['type'] == 'pointers' and part.get('role') == 'drums' and any(part['values']):
            raise ValueError(f"{manifest['name']}/{part['id']}: drum maps require typed wave/envelope bindings")
        if part['type'] != 'sequence':
            continue
        for event in part['events']:
            if event.get('command') == 'drums':
                raise ValueError(f"{manifest['name']}/{part['id']}: drum commands require typed drum-map bindings")
            if event.get('command') != 'envelope':
                continue
            source = part['id']+'/'+event['id']
            index = decode_index(event['operands'], source)
            if 'envelope_id' not in event:
                if index >= len(ids):
                    raise ValueError(f"{manifest['name']}/{source}: envelope index exceeds table")
                event['envelope_id'] = ids[index]
            if not isinstance(event['envelope_id'], str) or not event['envelope_id']:
                raise ValueError(f"{manifest['name']}/{source}: invalid envelope identity")
            references.append(dict(source=source, field=1, target=event['envelope_id'],
                                   kind='index', encoding='mus_varlen', addend=0))
    key = lambda reference: (reference['source'], reference['field'])
    if ('envelope_relocations' in manifest and
            sorted(manifest['envelope_relocations'], key=key) != sorted(references, key=key)):
        raise ValueError(f"{manifest['name']}: envelope relocation coverage disagrees with event identities")
    manifest['envelope_relocations'] = references
    return manifest


def bind_indices(manifest):
    m = identify_envelopes(manifest)
    header = [p for p in m['parts'] if p['type'] == 'sequence_header']
    tracks = [p for p in m['parts'] if p['type'] == 'pointers' and p.get('role') in ('sequence', 'volume', 'pitch')]
    if len(header) != 1 or len(tracks) != 3 or {p['role'] for p in tracks} != {'sequence', 'volume', 'pitch'}:
        raise ValueError(f"{m['name']}: expected one header and three track tables")
    # MusStartSong rebases five header pointers and the following 3*N words
    # in one contiguous walk; other objects cannot interrupt this pointer block.
    if (m['parts'][0] is not header[0] or len(m['parts']) < 4 or
            {p['id'] for p in m['parts'][1:4]} != {p['id'] for p in tracks}):
        raise ValueError(f"{m['name']}: runtime rebasing requires header and three contiguous track tables first")
    lengths = {len(p['values']) for p in tracks}
    if len(lengths) != 1 or not 1 <= next(iter(lengths)) <= 32:
        raise ValueError(f"{m['name']}: track tables require equal counts in 1..32")
    header[0]['track_count'] = next(iter(lengths))
    slots = {}
    for part in m['parts']:
        if part['type'] != 'envelopes':
            continue
        order = part.get('entry_order', part['entry_ids'])
        if (not isinstance(order, list) or not all(isinstance(i, str) for i in order) or
                len(order) != len(part['entry_ids']) or set(order) != set(part['entry_ids'])):
            raise ValueError(f"{m['name']}/{part['id']}: entry_order must contain every envelope exactly once")
        if len(order) > 32768:
            raise ValueError(f"{m['name']}/{part['id']}: envelope indices exceed 15-bit encoding")
        by_id = dict(zip(part['entry_ids'], part['entries']))
        part['entries'] = [by_id[identity] for identity in order]
        part['entry_ids'] = list(order)
        slots = {identity: i for i, identity in enumerate(order)}
    for part in m['parts']:
        if part['type'] != 'sequence':
            continue
        for event in part['events']:
            if event.get('command') != 'envelope':
                continue
            source = part['id']+'/'+event['id']
            if event['envelope_id'] not in slots:
                raise ValueError(f"{m['name']}/{source}: unresolved envelope {event['envelope_id']!r}")
            index = slots[event['envelope_id']]
            event['operands'] = ([index] if index < 128 and len(event['operands']) == 1 else
                                 [128 | (index >> 8), index & 255])
    return m
