from pathlib import Path
import struct
import unittest
from tools.readable_assets import parse_tilemap
from tools.tilemap_relocations import tilemap_manifest, pack_tilemap


class TilemapRelocationTests(unittest.TestCase):
    def fixture(self):
        data = struct.pack('>8H', 2, 1, 2, 1, 3, 28, 32, 96)
        data += struct.pack('>hBBhBBhBB', 0, 0, 0, 1, 0, 0, 2, 1, 0)
        data += struct.pack('>2h', 1, 2) + bytes(32) + bytes.fromhex('ffff')*16 + b'\x12\x34'
        return dict(name='tilemap fixture', image_format='ci4', **parse_tilemap(data, 'ci4')), data

    def test_reorder_tiles_images_and_palettes(self):
        m, data = self.fixture()
        self.assertEqual(pack_tilemap(m, Path('.')), data)
        m = tilemap_manifest(m)
        m['images'].reverse()
        m['palette_order'] = list(reversed(m['palette_ids']))
        m['tiles'][1:] = reversed(m['tiles'][1:])
        result = pack_tilemap(m, Path('.'))
        decoded = parse_tilemap(result, 'ci4')
        self.assertEqual(decoded['tilemap'], [2, 1])
        self.assertEqual(decoded['tiles'][2]['image_index'], 2)
        self.assertEqual(decoded['tiles'][2]['palette_index'], 1)
        self.assertEqual(result[-2:], b'\x34\x12')

    def test_table_growth_recomputes_header_offsets(self):
        m, _ = self.fixture()
        m = tilemap_manifest(m)
        extra = dict(m['tiles'][1], id='new tile')
        m['tiles'].append(extra)
        m.pop('relocations')
        result = pack_tilemap(m, Path('.'))
        decoded = parse_tilemap(result, 'ci4')
        self.assertEqual(decoded['header']['tile_count'], 4)
        self.assertEqual(decoded['header']['tilemap_offset'], 32)
        self.assertEqual(decoded['header']['image_offset'], 100)
        self.assertEqual(decoded['tilemap'], [1, 2])

    def test_invalid_target_and_fixed_empty_tile(self):
        m, _ = self.fixture()
        m = tilemap_manifest(m)
        m['tilemap_ids'][0] = 'missing'
        m.pop('relocations')
        with self.assertRaisesRegex(ValueError, 'unresolved'):
            pack_tilemap(m, Path('.'))
        m, _ = self.fixture()
        m = tilemap_manifest(m)
        m['tiles'].reverse()
        with self.assertRaisesRegex(ValueError, 'index zero'):
            pack_tilemap(m, Path('.'))


if __name__ == '__main__':
    unittest.main()
