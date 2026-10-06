"""One contiguous Mus sample stream, including extraction cuts inside samples.

The two extracted ROM slices are storage partitions, not independent banks.
Sample layout is resolved before compressing the pointer bank because its fields
are relative to the sample-stream base, independent of final absolute ROM LMA.
"""
from pathlib import Path

from tools.asset_relocations import Object, relocate
from tools.course_surface_data_common import load_yaml


def sample_group(root, bank_name):
    from tools.asset_bundles import materialize, safe_path
    rows = []
    for row in load_yaml(root/'assets/layout/segments.yaml')['segments']:
        if row['decoder'] != 'sample_bank':
            continue
        m, path = materialize(root, row)
        if m['bank_asset'] == bank_name:
            rows.append((row, m, path))
    rows.sort(key=lambda item: item[0]['rom_start'])
    if not rows:
        raise ValueError(f'{bank_name}: no sample stream manifests')
    base = rows[0][0]['rom_start']
    samples = {}
    signature = None
    previous_end = base
    for row, m, path in rows:
        if row['rom_start'] != previous_end:
            raise ValueError(f'{bank_name}/{row["name"]}: sample stream partitions are not contiguous')
        previous_end = row['rom_end']
        for part in m['parts']:
            kind = part['type']
            if kind == 'signature':
                if row['rom_start'] != base or part['offset'] != 0 or signature is not None:
                    raise ValueError(f'{bank_name}: sample signature must occur once at stream start')
                signature = part['text'].encode('ascii') + bytes([part['terminator']])
                if signature != b'N64 WaveTables \0':
                    raise ValueError(f'{bank_name}: unsupported sample signature')
            elif kind == 'padding':
                if any(bytes.fromhex(part['data'])):
                    raise ValueError(f'{bank_name}: nonzero sample padding needs an explicit object identity')
            elif kind == 'sample':
                original = row['rom_start']-base+part['offset']-part['source_offset']
                source = safe_path(root, path.parent/part['path'])
                identity = part.get('id', f'sample_{original:06x}')
                spec = (identity, source, part['source_size'])
                if original in samples and samples[original] != spec:
                    raise ValueError(f'{bank_name}: split sample at 0x{original:X} has inconsistent shared storage')
                if original < 16 or original % 16:
                    raise ValueError(f'{bank_name}: sample at 0x{original:X} violates 16-byte alignment')
                samples[original] = spec
            else:
                raise ValueError(f'{bank_name}: unsupported sample object {kind!r}')
    if signature is None:
        raise ValueError(f'{bank_name}: missing sample signature')
    return rows, samples, signature


def layout_samples(root, bank_name):
    rows, samples, signature = sample_group(root, bank_name)
    objects = [Object('sample_signature', signature, 0, 16)]
    sizes, targets = {}, {}
    for original, (identity, path, original_size) in sorted(samples.items()):
        data = path.read_bytes()
        if not data or len(data) % 9:
            raise ValueError(f'{bank_name}/{identity}: VADPCM data must contain complete 9-byte frames')
        objects.append(Object(identity, data, original, 16))
        sizes[identity] = len(data)
        targets[original] = identity
    declarations = [(row['name'], m['sample_order']) for row, m, path in rows if 'sample_order' in m]
    if declarations:
        if len(declarations) != 1 or declarations[0][0] != rows[0][0]['name']:
            raise ValueError(f'{bank_name}: sample_order must be declared only in the first stream partition')
        order = declarations[0][1]
        by_id = {obj.id: obj for obj in objects[1:]}
        if len(by_id) != len(objects)-1:
            raise ValueError(f'{bank_name}: duplicate shared sample identity')
        if (not isinstance(order, list) or not all(isinstance(identity, str) for identity in order)
                or len(order) != len(by_id) or set(order) != set(by_id)):
            raise ValueError(f'{bank_name}: sample_order must include every shared sample exactly once')
        objects[1:] = [by_id[identity] for identity in order]
    payload, offsets = relocate(bank_name+'/samples', objects, [], mode='shiftable')
    payload += bytes((-len(payload)) % 16)
    return payload, offsets, sizes, targets, rows


def pack_sample_slice(root, row, manifest):
    payload, offsets, sizes, targets, rows = layout_samples(root, manifest['bank_asset'])
    cursor = 0
    for index, (part, _, _) in enumerate(rows):
        # Preserve extraction cuts for unchanged output. These cuts may pass
        # through samples; no target identity is attached to an artificial cut.
        end = len(payload) if index == len(rows)-1 else min(len(payload), cursor+part['rom_end']-part['rom_start'])
        if part['name'] == row['name']:
            return payload[cursor:end]
        cursor = end
    raise ValueError(f"{row['name']}: absent from its declared sample stream")


def bind_pointer_bank(root, manifest):
    import copy
    payload, offsets, sizes, targets, rows = layout_samples(root, manifest['name'])
    m = copy.deepcopy(manifest)
    loops = {p['offset']: p for p in m['parts'] if p['type'] == 'loop'}
    for part in m['parts']:
        if part['type'] != 'wavetable':
            continue
        identity = part.get('sample_id', targets.get(part['sample_offset']))
        if identity not in sizes:
            raise ValueError(f"{m['name']}/wave at 0x{part['offset']:X}: unresolved sample {identity!r}")
        length = sizes[identity]
        if part['loop_offset']:
            loop = loops.get(part['loop_offset'])
            if loop is None or not 0 <= loop['start'] <= loop['end'] <= length//9*16:
                raise ValueError(f"{m['name']}/{identity}: loop range exceeds resized sample")
        part['sample_id'] = identity
        part['sample_offset'] = offsets[identity]
        part['sample_size'] = length
    return m
