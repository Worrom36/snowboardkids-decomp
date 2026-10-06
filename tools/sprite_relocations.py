"""Sprite image identities and palette-slot relocation with shared storage."""
import copy
from dataclasses import asdict
import struct

from tools.asset_images import image_bytes
from tools.asset_relocations import Object, Reference, relocate
from tools.course_sprite_table_common import parse_course_sprite_table


def sprite_manifest(manifest):
    m = copy.deepcopy(manifest)
    images = {}
    for image in m['images']:
        image.setdefault('id', image['name'])
        images[image['offset']] = image
    for palette in m['palettes']:
        palette.setdefault('id', palette['name'])
    original_palettes = sorted(m['palettes'], key=lambda p: p['offset'])
    refs = []
    for index, entry in enumerate(m['entries']):
        if 'image_id' not in entry:
            if entry['image_offset'] not in images:
                raise ValueError(f"{m['name']}/entry {index}: unresolved image")
            entry['image_id'] = images[entry['image_offset']]['id']
        if 'palette_ids' not in entry:
            image = next((p for p in m['images'] if p['id'] == entry['image_id']), None)
            if image is None:
                raise ValueError(f"{m['name']}/entry {index}: unresolved image identity")
            count = 16 if image['format'] == 'ci8' else 1
            start = entry['palette_index']
            entry['palette_ids'] = [p['id'] for p in original_palettes[start:start+count]]
            if len(entry['palette_ids']) != count:
                raise ValueError(f"{m['name']}/entry {index}: palette span exceeds storage")
        refs.append(Reference('sprite_table', 8+index*8, entry['image_id']))
        if not entry['palette_ids']:
            raise ValueError(f"{m['name']}/entry {index}: empty palette span")
        refs.append(Reference('sprite_table', 12+index*8, entry['palette_ids'][0],
                              width=2, unit=32, base='palette_base'))
    expected = [asdict(ref) for ref in refs]
    if 'relocations' in m and [asdict(Reference(**r)) for r in m['relocations']] != expected:
        raise ValueError(f"{m['name']}: sprite relocation coverage disagrees with entry identities")
    m['relocations'] = expected
    return m


def pack_sprites(manifest, base, mode='shiftable'):
    m = sprite_manifest(manifest)
    images = {p['id']: p for p in m['images']}
    palettes = [p['id'] for p in m['palettes']]
    header = bytearray(struct.pack('>II', m['unknown_0'], len(m['entries'])))
    ci8 = []
    for index, entry in enumerate(m['entries']):
        if entry['image_id'] not in images:
            raise ValueError(f"{m['name']}/entry {index}: unresolved image {entry['image_id']}")
        image = images[entry['image_id']]
        span = entry['palette_ids']
        if span[0] not in palettes:
            raise ValueError(f"{m['name']}/entry {index}: unresolved palette")
        start = palettes.index(span[0])
        count = 16 if image['format'] == 'ci8' else 1
        if len(span) != count or palettes[start:start+count] != span:
            raise ValueError(f"{m['name']}/entry {index}: CI8 palette slots must remain contiguous and ordered")
        if image['format'] == 'ci8':
            ci8.append(index)
        header.extend(struct.pack('>IHBB', 0, 0, image['width'], image['height']))
    original_end = 8+manifest['entry_count']*8
    objects = [Object('sprite_table', bytes(header), 0), Object('palette_base', b'', original_end)]
    for palette in m['palettes']:
        if len(palette['values']) != 16 or palette['colors'] != 16:
            raise ValueError(f"{m['name']}/{palette['id']}: palette slots require 16 colors")
        objects.append(Object(palette['id'], struct.pack('>16H', *palette['values']), palette['offset']))
    rest = []
    for image in m['images']:
        data = image_bytes(image, base)
        pixels = image['width'] * image['height']
        expected = (pixels+1)//2 if image['format'] == 'ci4' else pixels
        if image['format'] not in ('ci4', 'ci8') or not pixels or len(data) != expected:
            raise ValueError(f"{m['name']}/{image['id']}: invalid image format/dimensions")
        rest.append(Object(image['id'], data, image['offset']))
    for raw in m.get('raw_parts', []):
        rest.append(Object(raw.get('id', f"raw_{raw['offset']:06x}"), bytes.fromhex(raw['data']), raw['offset']))
    if 'image_order' in m:
        by_id = {obj.id: obj for obj in rest}
        if len(m['image_order']) != len(rest) or set(m['image_order']) != set(by_id):
            raise ValueError(f"{m['name']}: image_order must include every image/raw object exactly once")
        rest = [by_id[identity] for identity in m['image_order']]
    else:
        rest.sort(key=lambda obj: obj.offset)
    objects.extend(rest)
    result, offsets = relocate(m['name'], objects, [Reference(**r) for r in m['relocations']], mode=mode)
    parse_course_sprite_table(result, ci8)
    return result
