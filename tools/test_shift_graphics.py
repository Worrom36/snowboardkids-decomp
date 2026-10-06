from pathlib import Path
import tempfile
import unittest
from tools.graphics_relocations import model_manifest, pack_model


class GraphicsRelocationTests(unittest.TestCase):
    def fixture(self, base):
        (base/'model.c').write_text('Gfx model[] = {\n{{0x04000400, 0x02000000}},\n{{0xB8000000, 0x00000000}},\n};\n')
        vertex = dict(x=0, y=0, z=0, flag=0, s=0, t=0, r=255, g=255, b=255, a=255)
        return dict(name='MODEL', root_offsets=[16], parts=[
            dict(type='vertices', offset=0, size=16, vertices=[vertex]),
            dict(type='display_list', offset=16, size=16, path='model.c')])

    def test_vertices_and_exported_root_move(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            m = model_manifest(self.fixture(base), base)
            original, exports = pack_model(m, base, 'matching')
            self.assertEqual(exports['MODEL_ROOT_000010'], 0x02000010)
            m['parts'].insert(0, dict(id='padding', type='padding', offset=0, size=16, data=bytes(16).hex()))
            result, exports = pack_model(m, base)
            self.assertEqual(exports['MODEL_ROOT_000010'], 0x02000020)
            self.assertEqual(int.from_bytes(result[36:40], 'big'), 0x02000010)
            self.assertEqual(result[16:32], original[:16])

    def test_missing_reference_and_vertex_count_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            m = model_manifest(self.fixture(base), base)
            m['relocations'] = []
            with self.assertRaisesRegex(ValueError, 'coverage'):
                pack_model(m, base)
            m = model_manifest(self.fixture(base), base)
            m['parts'][0]['vertices'] = []
            with self.assertRaisesRegex(ValueError, 'vertex count'):
                pack_model(m, base)


if __name__ == '__main__':
    unittest.main()
