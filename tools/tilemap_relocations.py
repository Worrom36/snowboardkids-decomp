"""Tilemap headers, tile indices, image indices, and palette slots by identity."""
import copy
from dataclasses import asdict
import struct

from tools.asset_images import image_bytes
from tools.asset_relocations import Object, Reference, relocate


def tilemap_manifest(manifest):
    m = copy.deepcopy(manifest)
    for index, tile in enumerate(m['tiles']):
        tile.setdefault('id', f'tile_entry_{index:04x}')
    for image in m['images']:
        image.setdefault('id', image['name'])
    m.setdefault('empty_tile_id', m['tiles'][0]['id'])
    m.setdefault('palette_ids', [f'palette_{i:04x}' for i in range(len(m['palettes']))])
    if len(m['palette_ids']) != len(m['palettes']):
        raise ValueError(f"{m['name']}: palette identity count mismatch")
    if 'tilemap_ids' not in m:
        try:
            m['tilemap_ids'] = [m['tiles'][index]['id'] for index in m['tilemap']]
        except IndexError as exc:
            raise ValueError(f"{m['name']}: tilemap references a missing tile") from exc
    for tile in m['tiles']:
        if tile['id'] == m['empty_tile_id']:
            continue
        if 'image_id' not in tile:
            index = tile['image_index']-1
            if index == -1:
                tile['image_id'] = None
            elif not 0 <= index < len(m['images']):
                raise ValueError(f"{m['name']}/{tile['id']}: unresolved tile image")
            else:
                tile['image_id'] = m['images'][index]['id']
        if 'palette_span' not in tile:
            start = tile['palette_index']
            count = 16 if m['image_format'] == 'ci8' else 1
            tile['palette_span'] = m['palette_ids'][start:start+count]
            if len(tile['palette_span']) != count:
                raise ValueError(f"{m['name']}/{tile['id']}: palette span exceeds storage")
    h = m['header']
    pixels = h['tile_width'] * h['tile_height']
    unit = (pixels+1)//2 if m['image_format'] == 'ci4' else pixels
    refs = [Reference('header', 10, 'tilemap', width=2), Reference('header', 12, 'palette_base', width=2),
            Reference('header', 14, 'image_base', width=2)]
    for tile in m['tiles']:
        if tile['id'] == m['empty_tile_id']:
            continue
        if tile['image_id'] is not None:
            refs.append(Reference(tile['id'], 0, tile['image_id'], width=2, unit=unit, addend=unit, base='image_base'))
        if not tile['palette_span']:
            raise ValueError(f"{m['name']}/{tile['id']}: empty palette span")
        refs.append(Reference(tile['id'], 2, tile['palette_span'][0], width=1, unit=32, base='palette_base'))
    refs.extend(Reference('tilemap', index*2, target, width=2, unit=4, base='tiles_base')
                for index, target in enumerate(m['tilemap_ids']))
    expected = [asdict(ref) for ref in refs]
    if 'relocations' not in m:
        m['relocations'] = expected
    elif {tuple(sorted(asdict(Reference(**r)).items())) for r in m['relocations']} != {
            tuple(sorted(r.items())) for r in expected}:
        raise ValueError(f"{m['name']}: tilemap relocation coverage disagrees with identities")
    return m


def pack_tilemap(manifest, base, mode='shiftable'):
    from tools.readable_assets import parse_tilemap
    m = tilemap_manifest(manifest)
    h = m['header']
    if not m['tiles'] or m['tiles'][0]['id'] != m['empty_tile_id']:
        raise ValueError(f"{m['name']}: the empty tile must remain at index zero")
    if len(m['tilemap_ids']) != h['grid_width']*h['grid_height']:
        raise ValueError(f"{m['name']}: tilemap grid/identity count mismatch")
    width, height = h['tile_width'], h['tile_height']
    if width <= 0 or height <= 0 or m['image_format'] not in ('ci4', 'ci8'):
        raise ValueError(f"{m['name']}: invalid tile dimensions/format")
    unit = (width*height+1)//2 if m['image_format'] == 'ci4' else width*height
    header = struct.pack('>8H', h['grid_width'], h['grid_height'], width, height, len(m['tiles']), 0, 0, 0)
    objects = [Object('header', header, 0, 2), Object('tiles_base', b'', 16, 2)]
    refs = [Reference('header', 10, 'tilemap', width=2), Reference('header', 12, 'palette_base', width=2),
            Reference('header', 14, 'image_base', width=2)]
    palette_order = m.get('palette_order', m['palette_ids'])
    if len(palette_order) != len(m['palette_ids']) or set(palette_order) != set(m['palette_ids']):
        raise ValueError(f"{m['name']}: palette_order must include every palette exactly once")
    for index, tile in enumerate(m['tiles']):
        data = struct.pack('>hBB', tile['image_index'], tile['palette_index'], tile['flip'])
        objects.append(Object(tile['id'], data, 16+4*index, 2))
        if tile['id'] == m['empty_tile_id']:
            continue
        span = tile['palette_span']
        if not span or span[0] not in palette_order:
            raise ValueError(f"{m['name']}/{tile['id']}: unresolved palette span")
        slot = palette_order.index(span[0])
        if palette_order[slot:slot+len(span)] != span:
            raise ValueError(f"{m['name']}/{tile['id']}: palette span must remain contiguous")
        if tile['image_id'] is not None:
            refs.append(Reference(tile['id'], 0, tile['image_id'], width=2, unit=unit, addend=unit, base='image_base'))
        refs.append(Reference(tile['id'], 2, span[0], width=1, unit=32, base='palette_base'))
    objects.append(Object('table_padding', bytes.fromhex(m['table_padding']), 16+h['tile_count']*4))
    objects.append(Object('tilemap', bytes(len(m['tilemap_ids'])*2), h['tilemap_offset'], 2))
    for index, target in enumerate(m['tilemap_ids']):
        refs.append(Reference('tilemap', index*2, target, width=2, unit=4, base='tiles_base'))
    objects.append(Object('map_padding', bytes.fromhex(m['map_padding']), h['tilemap_offset']+len(m['tilemap'])*2))
    objects.append(Object('palette_base', b'', h['palette_offset'], 2))
    for identity in palette_order:
        index = m['palette_ids'].index(identity)
        values = m['palettes'][index]
        if len(values) != 16:
            raise ValueError(f"{m['name']}/{identity}: palette requires 16 colors")
        objects.append(Object(identity, struct.pack('>16H', *values), h['palette_offset']+index*32, 2))
    objects.append(Object('image_base', b'', h['image_offset']))
    for image in m['images']:
        data = image_bytes(image, base)
        if len(data) != unit or (image['width'], image['height'], image['format']) != (width, height, m['image_format']):
            raise ValueError(f"{m['name']}/{image['id']}: image does not match tile dimensions/format")
        objects.append(Object(image['id'], data, image['offset']))
    if {tuple(sorted(asdict(Reference(**r)).items())) for r in m['relocations']} != {
            tuple(sorted(asdict(r).items())) for r in refs}:
        raise ValueError(f"{m['name']}: tilemap relocation coverage disagrees with identities")
    result, offsets = relocate(m['name'], objects, refs, mode=mode)
    parse_tilemap(result, m['image_format'])
    return result
