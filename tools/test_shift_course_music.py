import struct
import unittest

from tools.asset_bundles import ROOT, LAYOUT, load_yaml, materialize
from tools.course_music_shift_fixture import collision_signature, music_signature, permute, signature
from tools.huffman_asset import decompress_huffman_asset
from tools.surface_relocations import pack_surfaces
from tools import test_shift_surfaces


class CourseMusicFixtureTests(unittest.TestCase):
    def test_collision_signature_follows_coordinates_and_ranges(self):
        manifest = test_shift_surfaces.SurfaceRelocationTests().fixture()
        manifest['format'] = 'course_surface_data'
        original = pack_surfaces(manifest)
        moved = permute(manifest)
        self.assertNotEqual(original, moved)
        self.assertEqual(collision_signature(original), collision_signature(moved))
        broken = bytearray(moved)
        struct.pack_into('>H', broken, 22, 0)  # valid coordinate, wrong target
        self.assertNotEqual(collision_signature(original), collision_signature(broken))
        struct.pack_into('>H', broken, 22, 3)
        with self.assertRaisesRegex(ValueError, 'coordinate exceeds'):
            collision_signature(broken)
        with self.assertRaisesRegex(ValueError, 'truncated'):
            collision_signature(moved[:-1])

    def test_real_corpus_reorders_every_asset_without_changing_targets(self):
        rom = (ROOT/'snowboardkids.z64').read_bytes()
        counts = dict(course_surface_data=0, music_sequence=0)
        for row in load_yaml(ROOT/LAYOUT)['segments']:
            kind = row['decoder']
            if kind not in counts:
                continue
            with self.subTest(asset=row['name']):
                manifest, _ = materialize(ROOT, row)
                data = rom[row['rom_start']:row['rom_end']]
                if manifest['compression'] != 'none':
                    data = decompress_huffman_asset(data)[0]
                moved = permute(manifest)
                self.assertNotEqual(data, moved)
                self.assertEqual(signature(data, kind), signature(moved, kind))
                counts[kind] += 1
        self.assertEqual(counts, dict(course_surface_data=10, music_sequence=17))

    def test_collision_signature_distinguishes_duplicate_storage(self):
        manifest = test_shift_surfaces.SurfaceRelocationTests().fixture()
        manifest['coordinates'].append(dict(id='duplicate_coordinate', x=10, y=0, z=0))
        original = pack_surfaces(manifest)
        manifest.pop('index_relocations')
        manifest['faces'][0]['coordinate_ids'][0] = 'duplicate_coordinate'
        changed = pack_surfaces(manifest)
        self.assertNotEqual(collision_signature(original), collision_signature(changed))

    def test_music_signature_detects_wrong_track_and_shared_target(self):
        row = next(r for r in load_yaml(ROOT/LAYOUT)['segments'] if r['decoder'] == 'music_sequence')
        manifest, _ = materialize(ROOT, row)
        moved = permute(manifest)
        count, table = struct.unpack_from('>II', moved)
        values = struct.unpack_from(f'>{count}I', moved, table)
        first = next(i for i, value in enumerate(values) if value)
        second = next(i for i, value in enumerate(values) if value and value != values[first])
        broken = bytearray(moved)
        struct.pack_into('>I', broken, table+second*4, values[first])
        self.assertNotEqual(music_signature(moved), music_signature(broken))


if __name__ == '__main__':
    unittest.main()
