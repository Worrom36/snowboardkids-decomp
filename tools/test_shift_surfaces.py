import copy
import struct
import unittest

from tools.surface_relocations import surface_manifest, pack_surfaces


class SurfaceRelocationTests(unittest.TestCase):
    def fixture(self):
        face = dict(coordinate_indices=[0, 1, 2], surface_type=0, skip_first_edge_check=0)
        surface = dict(neighbor_indices=[-1, 1, -1, -1], boundary_coordinate_indices=[0, 1, 2, 0],
                       reference_coordinate_index=0, path_angle=0, face_start_index=0,
                       face_end_index=1, unused_18=0, edge_clamp_flags=0)
        other = dict(surface, neighbor_indices=[0, -1, -1, -1], face_start_index=1, face_end_index=2)
        return surface_manifest(dict(name='collision', coordinates=[dict(x=x, y=0, z=0) for x in (10, 20, 30)],
                                     faces=[dict(face), dict(face)], surfaces=[surface, other]))

    def test_coordinates_and_face_groups_move(self):
        m = self.fixture()
        m['coordinates'].reverse()
        m['faces'].reverse()
        data = pack_surfaces(m)
        self.assertEqual(struct.unpack_from('>3H', data, 22), (2, 1, 0))
        self.assertEqual(struct.unpack_from('>4h', data, 40), (-1, 1, -1, -1))
        self.assertEqual(struct.unpack_from('>4h', data, 48), (2, 1, 0, 2))
        self.assertEqual(struct.unpack_from('>2H', data, 60), (1, 2))
        self.assertEqual(struct.unpack_from('>2H', data, 88), (0, 1))

    def test_missing_targets_and_coverage(self):
        m = self.fixture()
        m['coordinates'].pop()
        with self.assertRaisesRegex(ValueError, 'unresolved coordinates'):
            pack_surfaces(m)
        m = self.fixture()
        m['index_relocations'].pop()
        with self.assertRaisesRegex(ValueError, 'coverage'):
            pack_surfaces(m)

    def test_noncontiguous_faces_and_cpu_order_rejected(self):
        m = self.fixture()
        m['surfaces'][0]['face_ids'].append(m['faces'][1]['id'])
        del m['index_relocations']
        m['faces'].reverse()
        with self.assertRaisesRegex(ValueError, 'contiguous ordered'):
            pack_surfaces(m)
        m = self.fixture()
        m['surfaces'].reverse()
        with self.assertRaisesRegex(ValueError, 'CPU course-progress'):
            pack_surfaces(m)

    def test_coordinate_growth_updates_counts(self):
        m = self.fixture()
        m['coordinates'].insert(0, dict(id='new_coordinate', x=100, y=200, z=300))
        data = pack_surfaces(m)
        self.assertEqual(int.from_bytes(data[:2], 'big'), 4)
        self.assertEqual(struct.unpack_from('>3H', data, 28), (1, 2, 3))

    def test_course_corpus_matches(self):
        from tools.asset_bundles import ROOT, LAYOUT, load_yaml, materialize
        from tools.course_surface_data_common import pack_course_surface_data
        rows = [r for r in load_yaml(ROOT/LAYOUT)['segments'] if r['decoder'] == 'course_surface_data']
        self.assertEqual(len(rows), 10)
        for row in rows:
            with self.subTest(asset=row['name']):
                m, path = materialize(ROOT, row)
                self.assertEqual(pack_surfaces(m), pack_course_surface_data(m))


if __name__ == '__main__':
    unittest.main()
