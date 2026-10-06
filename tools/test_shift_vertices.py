import copy
import unittest

from tools.vertex_relocations import annotate_vertices, vertex_range


class VertexRelocationTests(unittest.TestCase):
    def fixture(self):
        part = dict(size=64, vertices=[{} for _ in range(4)])
        annotate_vertices(part)
        return part

    def test_shared_interior_range_moves_as_a_group(self):
        part = self.fixture()
        part['vertices'] = part['vertices'][2:] + part['vertices'][:2]
        for _ in range(2):
            offset, targets = vertex_range(part, 0, 2, 'model/load')
            self.assertEqual(offset, 32)
            self.assertEqual(targets, ['vertex_000000', 'vertex_000010'])
        part['vertices'].insert(0, dict(id='new_vertex'))
        self.assertEqual(vertex_range(part, 0, 2, 'model/load')[0], 48)

    def test_split_reordered_missing_or_duplicate_targets_fail(self):
        original = self.fixture()
        for order, error in [([0, 2, 1, 3], 'contiguous'), ([1, 0, 2, 3], 'ordered'),
                             ([1, 2, 3], 'unresolved'), ([0, 0, 1, 2, 3], 'duplicate')]:
            part = copy.deepcopy(original)
            part['vertices'] = [part['vertices'][i] for i in order]
            with self.subTest(order=order), self.assertRaisesRegex(ValueError, error):
                vertex_range(part, 0, 2, 'model/load')

    def test_legacy_resize_and_partial_annotation_fail(self):
        part = dict(size=32, vertices=[{}, {}])
        self.assertEqual(vertex_range(part, 16, 1, 'model/load')[0], 16)
        part['vertices'].append({})
        with self.assertRaisesRegex(ValueError, 'resizing legacy'):
            vertex_range(part, 0, 1, 'model/load')
        part['vertices'][0]['id'] = 'vertex_000000'
        with self.assertRaisesRegex(ValueError, 'every vertex'):
            vertex_range(part, 0, 1, 'model/load')


if __name__ == '__main__':
    unittest.main()
