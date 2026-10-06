"""Course collision coordinates and contiguous face ranges by stable identity."""
import copy

from tools.course_surface_data_common import pack_course_surface_data


def surface_manifest(manifest):
    m = copy.deepcopy(manifest)
    for table in ('coordinates', 'faces', 'surfaces'):
        for index, entry in enumerate(m[table]):
            entry.setdefault('id', f'{table}_{index:04x}')
    m.setdefault('surface_order', [s['id'] for s in m['surfaces']])

    def identity(table, index, source, sentinel=False):
        if sentinel and -32768 <= index < 0:
            return index
        if not 0 <= index < len(m[table]):
            raise ValueError(f"{m['name']}/{source}: {table} index {index} is out of range")
        return m[table][index]['id']

    refs = []
    for face in m['faces']:
        if 'coordinate_ids' not in face:
            face['coordinate_ids'] = [identity('coordinates', i, face['id']) for i in face['coordinate_indices']]
        if len(face['coordinate_ids']) != 3:
            raise ValueError(f"{m['name']}/{face['id']}: a face requires three coordinates")
        refs.extend(dict(source=face['id'], field=i*2, target=target, kind='index', table='coordinates', addend=0)
                    for i, target in enumerate(face['coordinate_ids']))
    for surface in m['surfaces']:
        if 'neighbor_ids' not in surface:
            surface['neighbor_ids'] = [identity('surfaces', i, surface['id'], True) for i in surface['neighbor_indices']]
        if 'boundary_coordinate_ids' not in surface:
            surface['boundary_coordinate_ids'] = [identity('coordinates', i, surface['id'])
                                                    for i in surface['boundary_coordinate_indices']]
        if 'reference_coordinate_id' not in surface:
            surface['reference_coordinate_id'] = identity('coordinates', surface['reference_coordinate_index'], surface['id'])
        if 'face_ids' not in surface:
            start, end = surface['face_start_index'], surface['face_end_index']
            if not 0 <= start < end <= len(m['faces']):
                raise ValueError(f"{m['name']}/{surface['id']}: invalid or empty face range")
            surface['face_ids'] = [f['id'] for f in m['faces'][start:end]]
        if len(surface['neighbor_ids']) != 4 or len(surface['boundary_coordinate_ids']) != 4 or not surface['face_ids']:
            raise ValueError(f"{m['name']}/{surface['id']}: invalid surface reference counts")
        fields = [(i*2, target, 'surfaces', 0) for i, target in enumerate(surface['neighbor_ids']) if isinstance(target, str)]
        fields += [(8+i*2, target, 'coordinates', 0) for i, target in enumerate(surface['boundary_coordinate_ids'])]
        fields += [(16, surface['reference_coordinate_id'], 'coordinates', 0),
                   (20, surface['face_ids'][0], 'faces', 0), (22, surface['face_ids'][-1], 'faces', 1)]
        refs.extend(dict(source=surface['id'], field=field, target=target, kind='index', table=table, addend=addend)
                    for field, target, table, addend in fields)
    if 'index_relocations' in m:
        if sorted(m['index_relocations'], key=lambda r: (r['source'], r['field'])) != sorted(refs, key=lambda r: (r['source'], r['field'])):
            raise ValueError(f"{m['name']}: collision relocation coverage disagrees with identities")
    else:
        m['index_relocations'] = refs
    return m


def pack_surfaces(manifest):
    m = surface_manifest(manifest)
    indices = {}
    all_ids = set()
    for table, limit in (('coordinates', 32768), ('faces', 65535), ('surfaces', 32768)):
        if not 0 < len(m[table]) <= limit:
            raise ValueError(f"{m['name']}/{table}: count exceeds encoded index range")
        indices[table] = {}
        for index, entry in enumerate(m[table]):
            key = entry['id']
            if not isinstance(key, str) or not key or key in all_ids:
                raise ValueError(f"{m['name']}/{table}: duplicate or invalid object identity {key!r}")
            all_ids.add(key)
            indices[table][key] = index
    if [s['id'] for s in m['surfaces'][:len(m['surface_order'])]] != m['surface_order']:
        raise ValueError(f"{m['name']}: surface order is used by CPU course-progress ranges; explicit CPU index bindings required")

    def resolve(table, key, source, sentinel=False):
        if sentinel and isinstance(key, int) and -32768 <= key < 0:
            return key
        if key not in indices[table]:
            raise ValueError(f"{m['name']}/{source}: unresolved {table} target {key!r}")
        return indices[table][key]

    for face in m['faces']:
        face['coordinate_indices'] = [resolve('coordinates', key, face['id']) for key in face['coordinate_ids']]
    for surface in m['surfaces']:
        key = surface['id']
        surface['neighbor_indices'] = [resolve('surfaces', target, key, True) for target in surface['neighbor_ids']]
        surface['boundary_coordinate_indices'] = [resolve('coordinates', target, key) for target in surface['boundary_coordinate_ids']]
        surface['reference_coordinate_index'] = resolve('coordinates', surface['reference_coordinate_id'], key)
        faces = [resolve('faces', target, key) for target in surface['face_ids']]
        if faces != list(range(faces[0], faces[0]+len(faces))):
            raise ValueError(f"{m['name']}/{key}: face targets must remain a contiguous ordered range")
        surface['face_start_index'], surface['face_end_index'] = faces[0], faces[-1]+1
    return pack_course_surface_data(m)
