from pathlib import Path
import struct
import unittest
from tools.course_sprite_table_common import parse_course_sprite_table
from tools.sprite_relocations import sprite_manifest, pack_sprites


class SpriteRelocationTests(unittest.TestCase):
    def fixture(self):
        data = struct.pack('>II', 0, 2)
        data += struct.pack('>IHBB', 88, 0, 2, 1) + struct.pack('>IHBB', 88, 1, 2, 1)
        data += bytes(32) + bytes.fromhex('ffff')*16 + b'\x12'
        return dict(name='shared sprites', decompressed_size=len(data), **parse_course_sprite_table(data)), data

    def test_palette_move_and_table_growth_preserve_shared_image(self):
        m, data = self.fixture()
        self.assertEqual(pack_sprites(m, Path('.')), data)
        m = sprite_manifest(m)
        m['palettes'].reverse()
        result = pack_sprites(m, Path('.'))
        self.assertEqual(int.from_bytes(result[12:14], 'big'), 1)
        self.assertEqual(int.from_bytes(result[20:22], 'big'), 0)
        self.assertEqual(result[8:12], result[16:20])
        m.pop('relocations')
        m['entries'].append(dict(m['entries'][0]))
        result = pack_sprites(m, Path('.'))
        self.assertEqual(int.from_bytes(result[4:8], 'big'), 3)
        self.assertEqual(int.from_bytes(result[8:12], 'big'), 96)
        self.assertEqual(result[-1:], b'\x12')

    def test_missing_target_and_coverage_rejected(self):
        m, _ = self.fixture()
        m = sprite_manifest(m)
        m['relocations'].pop()
        with self.assertRaisesRegex(ValueError, 'coverage'):
            pack_sprites(m, Path('.'))
        m, _ = self.fixture()
        m['entries'][0]['image_id'] = 'missing'
        with self.assertRaisesRegex(ValueError, 'unresolved'):
            pack_sprites(m, Path('.'))


if __name__ == '__main__':
    unittest.main()
