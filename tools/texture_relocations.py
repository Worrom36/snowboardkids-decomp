"""Validate texture load metadata against the objects it actually addresses."""
from tools.course_graphics_common import collect_course_texture_references


def validate_texture_loads(name, graphics, graph, parts, offsets, data, resource_size, segment):
    textures, palettes = collect_course_texture_references(graphics, graph, resource_size, segment)
    targets = {offsets[part['id']]: part for part in parts if part['type'] in ('texture', 'palette')}
    for load in [*textures, *palettes]:
        part = targets.get(load.offset)
        kind = 'texture' if hasattr(load, 'format') else 'palette'
        if part is None or part['type'] != kind:
            raise ValueError(f'{name}: {kind} load at 0x{load.offset:X} lacks a matching object boundary')
        fields = ('format', 'width', 'height') if kind == 'texture' else ('colors',)
        for field in fields:
            expected, actual = getattr(load, field), part[field]
            if expected != actual:
                raise ValueError(f"{name}/{part['id']}: {field} is {actual}, graphics load requires {expected}; update load commands together with the object")
        expected_size = load.size if kind == 'texture' else load.colors*2
        if len(data[part['id']]) != expected_size:
            raise ValueError(f"{name}/{part['id']}: graphics load size disagrees with encoded object length")
    return len(textures)+len(palettes)
