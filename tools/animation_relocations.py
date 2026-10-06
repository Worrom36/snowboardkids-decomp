"""Identity-based animation tables; shared table entries keep shared frames."""
import copy
from dataclasses import asdict

from tools.animation_assets import pack_animation
from tools.asset_relocations import Object, Reference, relocate


def identify(manifest):
    """Add identities using original offsets once; keep supplied IDs thereafter."""
    m = copy.deepcopy(manifest)
    word = 4 if m['format'] == 'scene_animation' else 2
    targets = {}
    for animation in m['animations']:
        animation.setdefault('id', f"animation_{animation['offset']:06x}")
        if animation['offset'] in targets:
            raise ValueError(f"{m['name']}: duplicate animation storage; use shared table references")
        targets[animation['offset']] = animation['id']
    if 'animation_ids' not in m:
        try:
            m['animation_ids'] = [targets[offset*2] for offset in m['animation_offsets']]
        except KeyError as exc:
            raise ValueError(f"{m['name']}: animation table refers to missing object at {exc}") from exc
    expected = [asdict(Reference('animation_table', i*word, identity, width=word, unit=2))
                        for i, identity in enumerate(m['animation_ids'])]
    if 'relocations' in m:
        provided = [asdict(Reference(**record)) for record in m['relocations']]
        if provided != expected:
            raise ValueError(f"{m.get('name', m['format'])}: animation relocation coverage disagrees with animation_ids")
    else:
        m['relocations'] = expected
    return m


def pack_relocatable_animation(manifest, mode='shiftable'):
    m = identify(manifest)
    name = m.get('name', m['format'])
    word = 4 if m['format'] == 'scene_animation' else 2
    if not m['animation_ids']:
        raise ValueError(f'{name}: empty animation table')
    objects = [Object('animation_table', bytes(word*len(m['animation_ids'])), 0, word)]
    for animation in m['animations']:
        single = copy.deepcopy(animation)
        single['offset'] = word
        if mode == 'shiftable':
            single['frame_count'] = len(single['frames'])
        local = dict(m, animations=[single], animation_offsets=[word//2])
        encoded = pack_animation(local)[word:]
        objects.append(Object(animation['id'], encoded, animation['offset'], 2))
    refs = [Reference(**record) for record in m['relocations']]
    result, offsets = relocate(name, objects, refs, mode=mode)
    return result
