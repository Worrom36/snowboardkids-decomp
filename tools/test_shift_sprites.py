from pathlib import Path
import struct
import unittest
import tempfile
import yaml
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
        image = int.from_bytes(result[8:12], 'big')
        self.assertEqual(result[image:image+1], b'\x12')
        tagged = int.from_bytes(result[:4], 'big')
        self.assertTrue(tagged & 0x80000000)
        remap = (tagged & 0x7ffffffe) >> 1
        self.assertEqual(struct.unpack_from('>3H', result, remap), (2, 1, 0))

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

    def test_cpu_palette_slots_follow_moved_storage(self):
        from tools.format_shift_fixture import signature
        m, original = self.fixture()
        m = sprite_manifest(m)
        m['palettes'].reverse()
        moved = pack_sprites(m, Path('.'))
        spec = dict(format='sprite_table', ci8=[], palette_count=2)
        self.assertEqual(signature(original, spec), signature(moved, spec))
        offset = (int.from_bytes(moved[:4], 'big') & 0x7ffffffe) >> 1
        broken = bytearray(moved)
        struct.pack_into('>2H', broken, offset+2, 0, 1)
        self.assertNotEqual(signature(original, spec), signature(broken, spec))
        struct.pack_into('>H', broken, offset, 3)
        with self.assertRaisesRegex(ValueError, 'public slot count'):
            signature(broken, spec)
        m['palette_ids'].pop()
        with self.assertRaisesRegex(ValueError, 'CPU palette identities'):
            pack_sprites(m, Path('.'))

    def test_build_selects_optional_renderer_only_for_reordered_cpu_slots(self):
        from tools.asset_bundles import write_makefile
        m, _ = self.fixture()
        row = dict(name=m['name'], decoder='sprite_table', source='sprite.yaml',
                   compression='none', object='assets/readable/sprite.o')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root/'assets/layout').mkdir(parents=True)
            for moved in (False, True):
                if moved:
                    m['palettes'].reverse()
                (root/'sprite.yaml').write_text(yaml.safe_dump(m))
                write_makefile(root, dict(segments=[row]))
                self.assertIn(f'ASSET_RUNTIME_INDEX_REMAP := {int(moved)}',
                              (root/'assets/layout/bundles.mk').read_text())

    def test_snowboard_loads_fit_rdp_rows_and_ci_texture_memory(self):
        data = struct.pack('>IIIHBB', 0, 1, 48, 0, 16, 1)+bytes(32)+bytes(8)
        m = dict(name='SNOWBOARD_TEXTURES', decompressed_size=len(data), **parse_course_sprite_table(data))
        self.assertEqual(pack_sprites(m, Path('.')), data)
        image = m['images'][0]
        for width, height in ((17, 1), (64, 65)):
            image.update(width=width, height=height, data=bytes((width*height+1)//2).hex())
            with self.assertRaisesRegex(ValueError, 'RDP block loads require'):
                pack_sprites(m, Path('.'))

    def test_build_selects_bounded_snowboard_loads_for_changed_layout(self):
        from tools.asset_bundles import write_makefile
        data = struct.pack('>IIIHBB', 0, 1, 48, 0, 64, 57)+bytes(32)+bytes(1824)
        m = dict(name='SNOWBOARD_TEXTURES', decompressed_size=len(data), **parse_course_sprite_table(data))
        row = dict(name=m['name'], decoder='sprite_table', source='sprite.yaml',
                   compression='none', object='assets/readable/sprite.o')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root/'assets/layout').mkdir(parents=True)
            for resized in (False, True):
                if resized:
                    m['images'][0].update(height=56, data=bytes(1792).hex())
                (root/'sprite.yaml').write_text(yaml.safe_dump(m))
                write_makefile(root, dict(segments=[row]))
                self.assertIn(f'ASSET_RUNTIME_SNOWBOARD_SIZE := {int(resized)}',
                              (root/'assets/layout/bundles.mk').read_text())


if __name__ == '__main__':
    unittest.main()
