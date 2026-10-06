"""Temporary collision and music permutations with decoded reference checks."""
import hashlib
import json
import struct

from tools.asset_bundles import materialize
from tools.huffman_asset import compress_flexible, decompress_huffman_asset
from tools.music_assets import parse_music
from tools.structured_relocations import music_manifest, pack_music
from tools.surface_relocations import surface_manifest, pack_surfaces


def collision_signature(data):
    """Resolve every surface and face coordinate from encoded index tables."""
    cursor = 0

    def read(fmt):
        nonlocal cursor
        size = struct.calcsize(fmt)
        if cursor+size > len(data):
            raise ValueError('collision fixture: truncated field')
        values = struct.unpack_from(fmt, data, cursor)
        cursor += size
        return values

    coordinates = [read('>hhh') for _ in range(read('>H')[0])]
    faces = []
    for _ in range(read('>H')[0]):
        a, b, c, kind, skip = read('>HHHbB')
        if max(a, b, c) >= len(coordinates):
            raise ValueError('collision fixture: face coordinate exceeds table')
        faces.append(((a, b, c), kind, skip))
    shared_coordinates, shared_faces = {}, {}

    def coordinate(index):
        return shared_coordinates.setdefault(index, len(shared_coordinates)), coordinates[index]

    def face(index):
        indices, kind, skip = faces[index]
        return shared_faces.setdefault(index, len(shared_faces)), [coordinate(i) for i in indices], kind, skip

    count = read('>H')[0]
    surfaces = []
    for _ in range(count):
        neighbors = read('>4h')
        boundary = read('>4h')
        reference, angle, start, end, unused, flags = read('>hhHHHH')
        if any(i >= count for i in neighbors):
            raise ValueError('collision fixture: neighbor exceeds surface table')
        if any(not 0 <= i < len(coordinates) for i in (*boundary, reference)):
            raise ValueError('collision fixture: surface coordinate exceeds table')
        if not 0 <= start < end <= len(faces):
            raise ValueError('collision fixture: face range exceeds table')
        surfaces.append((neighbors, [coordinate(i) for i in boundary], coordinate(reference),
                         angle, [face(i) for i in range(start, end)], unused, flags))
    if cursor != len(data) or not surfaces:
        raise ValueError('collision fixture: trailing bytes or empty surface coverage')
    unused_contents = [([coordinates[i] for i in indices], kind, skip) for indices, kind, skip in faces]
    return surfaces, sorted(coordinates), sorted(json.dumps(f) for f in unused_contents)


def music_signature(data):
    """Follow header, track and drum pointers, retaining shared target classes."""
    parts = parse_music(data)['parts']
    by_offset = {p['offset']: p for p in parts}
    header = by_offset[0]
    shared, active = {}, set()
    envelope_entries = by_offset[header['envelope_table']]['entries'] if header['envelope_table'] else []
    shared_envelopes = {}

    def target(offset):
        if not offset:
            return None
        if offset not in by_offset or offset in active:
            raise ValueError('music fixture: unresolved or cyclic table target')
        active.add(offset)
        part = by_offset[offset]
        kind = part['type']
        identity = shared.setdefault(offset, len(shared))
        if kind in ('sequence', 'modulation'):
            # Real assets contain no goto events; their three relative operands
            # have separate positive and negative structural unit tests.
            if any(e.get('command') == 'goto' for e in part['events']):
                raise ValueError('music fixture: goto streams need branch-aware semantic comparison')
            value = [{k: v for k, v in e.items() if k != 'offset'} for e in part['events']]
            from tools.music_indices import decode_index
            for event in value:
                if event.get('command') == 'envelope':
                    index = decode_index(event.pop('operands'), 'music fixture envelope')
                    if index >= len(envelope_entries):
                        raise ValueError('music fixture: envelope target exceeds table')
                    event['envelope'] = (shared_envelopes.setdefault(index, len(shared_envelopes)), envelope_entries[index])
        elif kind == 'envelopes':
            value = sorted(part['entries'])
        elif kind == 'pointers':
            value = [target(p) for p in part['values']]
        else:
            raise ValueError(f'music fixture: unsupported target type {kind}')
        active.remove(offset)
        return identity, kind, value

    return header['track_count'], [target(header[key]) for key in
            ('sequence_table', 'volume_table', 'pitch_table', 'envelope_table', 'drum_table')]


def signature(data, kind):
    if kind == 'course_surface_data':
        value = collision_signature(data)
    elif kind == 'music_sequence':
        value = music_signature(data)
    else:
        raise ValueError(f'unsupported collision/music fixture format {kind}')
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def permute(manifest):
    if manifest['format'] == 'course_surface_data':
        moved = surface_manifest(manifest)
        moved['coordinates'].reverse()
        # Move overlapping face ranges together, preserving each range order.
        intervals = sorted((s['face_start_index'], s['face_end_index']) for s in moved['surfaces'])
        groups = []
        for start, end in intervals:
            if groups and start < groups[-1][1]:
                groups[-1] = (groups[-1][0], max(groups[-1][1], end))
            else:
                groups.append((start, end))
        covered = {i for start, end in groups for i in range(start, end)}
        groups += [(i, i+1) for i in range(len(moved['faces'])) if i not in covered]
        moved['faces'] = [face for start, end in reversed(sorted(groups)) for face in moved['faces'][start:end]]
        return pack_surfaces(moved)
    moved = music_manifest(manifest)
    for part in moved['parts']:
        if part['type'] == 'envelopes':
            part['entry_order'] = list(reversed(part['entry_ids']))
    fixed = [p for p in moved['parts'] if p['type'] == 'sequence_header' or
             p['type'] == 'pointers' and p.get('role') in ('sequence', 'volume', 'pitch')]
    moved['parts'] = fixed + list(reversed([p for p in moved['parts'] if p not in fixed]))
    return pack_music(moved)


def prepare(root, rows, out, source, build_dir, linker, run):
    original = (root/'snowboardkids.z64').read_bytes()
    records = []
    counts = dict(course_surface_data=0, music_sequence=0)
    for row in rows:
        kind = row['decoder']
        if kind not in counts:
            continue
        manifest, _ = materialize(root, row)
        payload = permute(manifest)
        encoded_original = original[row['rom_start']:row['rom_end']]
        compressed = manifest['compression'] != 'none'
        decoded = decompress_huffman_asset(encoded_original)[0] if compressed else encoded_original
        expected = signature(decoded, kind)
        if payload == decoded or signature(payload, kind) != expected:
            raise ValueError(f"{row['name']}: fixture did not move objects or changed resolved contents")
        encoded = compress_flexible(payload, encoded_original) if compressed else payload
        binary, obj = out/(row['name']+'.bin'), out/(row['name']+'.o')
        binary.write_bytes(encoded)
        run(linker, '-r', '-b', 'binary', '-o', obj, binary)
        old = build_dir+'/'+row['object']
        if source.count(old) != 1:
            raise ValueError(f"{row['name']}: expected one fixture linker input")
        source = source.replace(old, str(obj))
        counts[kind] += 1
        records.append(dict(name=row['name'], kind=kind, compressed=compressed,
                            payload_sha256=hashlib.sha256(payload).hexdigest(), semantic_sha256=expected))
    if counts != dict(course_surface_data=10, music_sequence=17):
        raise ValueError(f'collision/music fixture coverage changed: {counts}')
    (out/'course-music-fields.json').write_text(json.dumps(records, indent=2)+'\n')
    return source


def verify(elf_path, report_path, *, music_only=False):
    from tools.verify_linked_relocations import Elf
    elf = Elf(elf_path)
    records = json.loads(report_path.read_text())
    if len(records) != 27 or len({r['name'] for r in records}) != 27:
        raise ValueError('collision/music fixture: incomplete asset coverage')
    if music_only:
        records = [r for r in records if r['kind'] == 'music_sequence']
        if len(records) != 17:
            raise ValueError('music fixture: incomplete stream asset coverage')
    for record in records:
        name = '.'+record['name']
        if name not in elf.names:
            raise ValueError(f'{name}: missing linked collision/music section')
        section = elf.sections[elf.names.index(name)]
        encoded = elf.data[section[4]:section[4]+section[5]]
        data = decompress_huffman_asset(encoded)[0] if record['compressed'] else encoded
        if (hashlib.sha256(data).hexdigest() != record['payload_sha256'] or
                signature(data, record['kind']) != record['semantic_sha256']):
            raise ValueError(f'{name}: linked collision/music references changed contents')
    return len(records)
