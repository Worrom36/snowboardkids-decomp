"""Explicit address operands for newly authored graphics commands."""


def command_bindings(manifest):
    bindings = {}
    allowed = {'source', 'field', 'target', 'kind', 'segment', 'addend', 'width', 'vertices'}
    records = manifest.get('graphics_relocations', [])
    if not isinstance(records, list):
        raise ValueError(f"{manifest['name']}: graphics_relocations must be a list")
    for record in records:
        if not isinstance(record, dict):
            raise ValueError(f"{manifest['name']}: graphics relocation must be a field record")
        context = f"{manifest['name']}/{record.get('source', '<missing>')}"
        if set(record)-allowed:
            raise ValueError(f'{context}: unsupported graphics relocation fields {sorted(set(record)-allowed)}')
        if (not isinstance(record.get('source'), str) or not record['source'] or
                not isinstance(record.get('target'), str) or not record['target']):
            raise ValueError(f'{context}: graphics relocation requires source and target identities')
        if (record.get('field') != 4 or record.get('kind') != 'segmented' or
                record.get('segment') not in (2, 3) or record.get('width', 4) != 4):
            raise ValueError(f'{context}: graphics operand requires a segment-2/3 word at field 4')
        addend = record.get('addend', 0)
        if not isinstance(addend, int) or not 0 <= addend < 0x1000000:
            raise ValueError(f'{context}: invalid graphics relocation addend')
        if record['source'] in bindings:
            raise ValueError(f'{context}: duplicate graphics relocation binding')
        bindings[record['source']] = record
    return bindings
