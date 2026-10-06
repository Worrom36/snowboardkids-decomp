from pathlib import Path
import struct
import tempfile
import unittest

from tools.course_relocations import layout_graphics
from tools.graphics_relocations import pack_model


class TextureLoadRelocationTests(unittest.TestCase):
    def fixture(self):
        words = [(0xfd500000, 0x03000000), (0xf5500000, 0x07000000),
                 (0xf3000000, 0x07007000), (0xf5400200, 0),
                 (0xf2000000, 0x0003c004), (0xb8000000, 0)]
        part = dict(id='image', type='texture', offset=0, size=16, format='ci4', width=16, height=2,
                    data=bytes(16).hex())
        return words, dict(name='resources', decompressed_size=16, parts=[part])

    def pack_course(self, words, manifest):
        data = b''.join(struct.pack('>II', *pair) for pair in words)
        return layout_graphics('graphics', [dict(name='stream', offset=0, data=data,
                              ids=[f'command_{i*8:06X}' for i in range(len(words))])], manifest, Path('.'))

    def test_texture_metadata_and_load_commands_change_together(self):
        words, manifest = self.fixture()
        self.pack_course(words, manifest)
        manifest['parts'][0].update(width=32, height=1)
        with self.assertRaisesRegex(ValueError, 'image: width is 32, graphics load requires 16'):
            self.pack_course(words, manifest)
        words[3] = (0xf5400400, 0)
        self.pack_course(words, manifest)

    def test_palette_count_mismatch_fails(self):
        words = [(0xfd100000, 0x03000000), (0xf5100100, 0x07000000),
                 (0xf0000000, 0x0703c000), (0xb8000000, 0)]
        part = dict(id='palette', type='palette', offset=0, size=32, colors=16, values=[0]*16)
        manifest = dict(name='resources', decompressed_size=32, parts=[part])
        self.pack_course(words, manifest)
        part.update(colors=32, values=[0]*32)
        with self.assertRaisesRegex(ValueError, 'palette: colors is 32, graphics load requires 16'):
            self.pack_course(words, manifest)
        words[2] = (0xf0000000, 0x0707c000)
        self.pack_course(words, manifest)

    def test_embedded_model_checks_texture_dimensions(self):
        words, manifest = self.fixture()
        words[0] = (words[0][0], 0x02000030)
        texture = manifest['parts'][0]
        texture['offset'] = 48
        manifest.update(root_offsets=[0], decompressed_size=64)
        manifest['parts'].insert(0, dict(id='commands', type='display_list', offset=0, size=48, path='model.c'))
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            (base/'model.c').write_text('Gfx model[] = {\n'+''.join(
                f'{{{{0x{a:08X}, 0x{b:08X}}}}},\n' for a,b in words)+'};\n')
            pack_model(manifest, base)
            texture.update(width=32, height=1)
            with self.assertRaisesRegex(ValueError, 'image: width is 32'):
                pack_model(manifest, base)


if __name__ == '__main__':
    unittest.main()
