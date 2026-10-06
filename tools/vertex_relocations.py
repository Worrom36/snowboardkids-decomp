"""Stable identities for vertices addressed inside a graphics resource part."""
import re


def annotate_vertices(part):
    """Called at extraction, while array order is still the original layout."""
    for index, vertex in enumerate(part['vertices']):
        vertex['id'] = f'vertex_{index*16:06X}'


def vertex_range(part, original_offset, count, context):
    """Resolve an original F3DEX load to a contiguous sequence of identities.

    A vertex load preserves order in the RSP cache. Moving a loaded group is
    supported; splitting or reordering its members requires editing triangle
    cache indices and is rejected instead of silently changing geometry.
    """
    vertices = part['vertices']
    if not count or original_offset < 0 or original_offset % 16:
        raise ValueError(f'{context}: invalid vertex range')
    ids = [vertex.get('id') for vertex in vertices]
    if any(identity is not None for identity in ids):
        if any(not isinstance(identity, str) or not re.fullmatch(r'[A-Za-z_][A-Za-z_0-9]*', identity)
               for identity in ids):
            raise ValueError(f'{context}: every vertex requires a valid object identity')
    else:
        if len(vertices)*16 != part['size']:
            raise ValueError(f'{context}: resizing legacy vertices requires object identities')
        ids = [f'vertex_{index*16:06X}' for index in range(len(vertices))]
    if len(ids) != len(set(ids)):
        raise ValueError(f'{context}: duplicate vertex object identity')
    targets = [f'vertex_{original_offset+i*16:06X}' for i in range(count)]
    positions = {identity: index for index, identity in enumerate(ids)}
    missing = [identity for identity in targets if identity not in positions]
    if missing:
        raise ValueError(f'{context}: unresolved vertex {missing[0]}')
    start = positions[targets[0]]
    if ids[start:start+count] != targets:
        raise ValueError(f'{context}: loaded vertex identities must remain contiguous and ordered')
    return start*16, targets
