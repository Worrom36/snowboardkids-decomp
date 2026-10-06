"""Temporary animation, sprite, and tilemap permutations with semantic checks."""
from collections import Counter
import hashlib
import json

from tools.animation_assets import parse_animation
from tools.animation_relocations import identify, pack_relocatable_animation
from tools.asset_bundles import materialize
from tools.course_sprite_table_common import parse_course_sprite_table
from tools.huffman_asset import compress_flexible, decompress_huffman_asset
from tools.readable_assets import parse_tilemap
from tools.sprite_relocations import sprite_manifest, pack_sprites
from tools.tilemap_relocations import tilemap_manifest, pack_tilemap

FORMATS = {'race_animation', 'scene_animation', 'sprite_table', 'course_sprite_table', 'tilemap'}


def signature(payload, spec):
    """Follow decoded references to compare content, not original addresses."""
    kind = spec['format']
    if kind in ('race_animation', 'scene_animation'):
        parsed = parse_animation(payload, kind, spec['part_count'])
        clips = {clip['offset']: clip for clip in parsed['animations']}
        shared = {}
        contents = [(shared.setdefault(offset, len(shared)), clips[offset*2]['frames'])
                    for offset in parsed['animation_offsets']]
    elif kind in ('sprite_table', 'course_sprite_table'):
        parsed = parse_course_sprite_table(payload, spec['ci8'])
        images = {image['offset']: image for image in parsed['images']}
        shared, contents = {}, []
        for entry in parsed['entries']:
            image = images[entry['image_offset']]
            count = 16 if image['format'] == 'ci8' else 1
            start = entry['palette_index']
            contents.append((shared.setdefault(entry['image_offset'], len(shared)),
                             entry['width'], entry['height'], image['format'], image['data'],
                             [p['values'] for p in parsed['palettes'][start:start+count]]))
    elif kind == 'tilemap':
        parsed = parse_tilemap(payload, spec['image_format'])
        tiles = [None]
        for tile in parsed['tiles'][1:]:
            image = parsed['images'][tile['image_index']-1]['data'] if tile['image_index'] else None
            start, count = tile['palette_index'], 16 if spec['image_format'] == 'ci8' else 1
            tiles.append((image, tile['flip'], parsed['palettes'][start:start+count]))
        # Include unused tile descriptors too, while permitting their order to change.
        contents = ([tiles[index] for index in parsed['tilemap']],
                    sorted(json.dumps(t, sort_keys=True) for t in tiles))
    else:
        raise ValueError(f'unsupported fixture format {kind}')
    return hashlib.sha256(json.dumps(contents, sort_keys=True).encode()).hexdigest()


def permute(manifest, base):
    kind = manifest['format']
    spec = dict(format=kind)
    if kind in ('race_animation', 'scene_animation'):
        moved = identify(manifest)
        moved['animations'].reverse()
        payload = pack_relocatable_animation(moved)
        spec['part_count'] = manifest['part_count']
    elif kind in ('sprite_table', 'course_sprite_table'):
        moved = sprite_manifest(manifest)
        images = {p['id']: p for p in moved['images']}
        spec['ci8'] = [i for i, entry in enumerate(moved['entries'])
                       if images[entry['image_id']]['format'] == 'ci8']
        order = [image['id'] for image in moved['images']]
        order += [part.get('id', f"raw_{part['offset']:06x}") for part in moved.get('raw_parts', [])]
        moved['image_order'] = list(reversed(order))
        if not spec['ci8']:
            moved['palettes'].reverse()
        payload = pack_sprites(moved, base)
    else:
        moved = tilemap_manifest(manifest)
        moved['images'].reverse()
        moved['tiles'][1:] = reversed(moved['tiles'][1:])
        # CI8 spans are indivisible groups of sixteen palette slots.
        unit = 16 if moved['image_format'] == 'ci8' else 1
        groups = [moved['palette_ids'][i:i+unit] for i in range(0, len(moved['palette_ids']), unit)]
        moved['palette_order'] = [identity for group in reversed(groups) for identity in group]
        payload = pack_tilemap(moved, base)
        spec['image_format'] = moved['image_format']
    return payload, spec


def prepare(root, rows, out, script, build_dir, linker, run):
    original_rom = (root/'snowboardkids.z64').read_bytes()
    report, counts, moved_counts = [], Counter(), Counter()
    for row in rows:
        if row['decoder'] not in FORMATS:
            continue
        manifest, path = materialize(root, row)
        payload, spec = permute(manifest, path.parent)
        original = original_rom[row['rom_start']:row['rom_end']]
        compressed = manifest['compression'] != 'none'
        decoded = decompress_huffman_asset(original)[0] if compressed else original
        expected = signature(decoded, spec)
        if signature(payload, spec) != expected:
            raise ValueError(f"{row['name']}: moved objects changed referenced contents or sharing")
        counts[row['decoder']] += 1
        moved_counts[row['decoder']] += payload != decoded
        encoded = compress_flexible(payload, original) if compressed else payload
        binary, obj = out/(row['name']+'.bin'), out/(row['name']+'.o')
        binary.write_bytes(encoded)
        run(linker, '-r', '-b', 'binary', '-o', obj, binary)
        old = build_dir+'/'+row['object']
        if script.count(old) != 1:
            raise ValueError(f"{row['name']}: expected exactly one fixture linker input")
        script = script.replace(old, str(obj))
        report.append(dict(name=row['name'], spec=spec, compressed=compressed,
                           payload_sha256=hashlib.sha256(payload).hexdigest(), semantic_sha256=expected))
    expected_counts = dict(race_animation=6, scene_animation=2, sprite_table=21, course_sprite_table=9, tilemap=7)
    if dict(counts) != expected_counts or set(moved_counts) != FORMATS or not all(moved_counts.values()):
        raise ValueError(f'format fixture coverage changed or empty: {dict(counts)}, moved {dict(moved_counts)}')
    (out/'format-fields.json').write_text(json.dumps(report, indent=2)+'\n')
    return script, counts, moved_counts


def verify(elf_path, report_path):
    from tools.verify_linked_relocations import Elf
    elf = Elf(elf_path)
    records = json.loads(report_path.read_text())
    if len(records) != 45:
        raise ValueError('format fixture final verification is missing expected assets')
    for record in records:
        name = record['name']
        section_name = '.'+('_' if name[0].isdigit() else '')+name
        if section_name not in elf.names:
            raise ValueError(f'{name}: missing linked section {section_name}')
        section = elf.sections[elf.names.index(section_name)]
        encoded = elf.data[section[4]:section[4]+section[5]]
        payload = decompress_huffman_asset(encoded)[0] if record['compressed'] else encoded
        if hashlib.sha256(payload).hexdigest() != record['payload_sha256']:
            raise ValueError(f"{record['name']}: final linked object bytes or fields differ")
        if signature(payload, record['spec']) != record['semantic_sha256']:
            raise ValueError(f"{record['name']}: final linked references changed contents or sharing")
    return len(records)
