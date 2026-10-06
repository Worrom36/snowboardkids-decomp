import copy
import unittest
from tools.animation_assets import pack_animation
from tools.animation_relocations import identify, pack_relocatable_animation


class AnimationRelocationTests(unittest.TestCase):
    def manifest(self, wide=False):
        word = 4 if wide else 2
        count = 1
        frame = dict(offsets=[[1, 2, 3]]*(count if wide else 2), rotations=[[8, 16, 24]], reserved_rotation_bits=[0])
        stride = 12 if wide else 16
        first = word*3
        second = first+2+stride
        return dict(name='fixture', format='scene_animation' if wide else 'race_animation', part_count=count,
                    animation_offsets=[first//2, second//2, second//2],
                    animations=[dict(offset=first, frame_count=1, frames=[frame]),
                                dict(offset=second, frame_count=1, frames=[copy.deepcopy(frame)])])

    def test_matching_growth_and_shared_targets(self):
        for wide in (False, True):
            m = self.manifest(wide)
            word = 4 if wide else 2
            baseline = pack_animation(m)
            self.assertEqual(pack_relocatable_animation(m), baseline)
            self.assertEqual(pack_relocatable_animation(m, mode='matching'), baseline)
            identified = identify(m)
            identified['animations'][0]['frames'].append(copy.deepcopy(m['animations'][0]['frames'][0]))
            result = pack_relocatable_animation(identified)
            target = int.from_bytes(result[word:2*word], 'big')*2
            self.assertGreater(target, m['animations'][1]['offset'])
            self.assertEqual(result[word:2*word], result[2*word:3*word])
            self.assertEqual(int.from_bytes(result[word*3:word*3+2], 'big'), 2)
            self.assertEqual(result[target:], baseline[m['animations'][1]['offset']:])

    def test_missing_object_and_offset_overflow(self):
        m = identify(self.manifest())
        m['animation_ids'][1] = 'missing'
        m['relocations'][1]['target'] = 'missing'
        with self.assertRaisesRegex(ValueError, 'unresolved'):
            pack_relocatable_animation(m)
        m = self.manifest()
        m['animations'][0]['frames'] *= 9000
        with self.assertRaisesRegex(ValueError, 'overflows'):
            pack_relocatable_animation(m)


if __name__ == '__main__':
    unittest.main()
