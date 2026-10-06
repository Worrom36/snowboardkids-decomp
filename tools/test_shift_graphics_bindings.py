import copy
from pathlib import Path
import struct
import unittest

from tools.course_relocations import layout_graphics
from tools import test_shift_textures


class AuthoredGraphicsBindingTests(unittest.TestCase):
    def fixture(self):
        vertex = dict(id='new_vertex', x=123, y=0, z=0, flag=0, s=0, t=0, r=255, g=255, b=255, a=255)
        resources = dict(name='resources', decompressed_size=16, parts=[
            dict(id='new_vertices', original=False, type='vertices', offset=0, size=16, vertices=[vertex])])
        streams = [dict(name='root', offset=0, ids=['new_call', 'end'],
                        data=struct.pack('>4I', 0x06000000, 0, 0xb8000000, 0)),
                   dict(name='body', offset=16, ids=['new_load', 'shared_load', 'end'],
                        data=struct.pack('>6I', 0x04000400, 0, 0x04000400, 0, 0xb8000000, 0))]
        resources['graphics_relocations'] = [
            dict(source='root/new_call', field=4, target='body/new_load', kind='segmented', segment=2),
            *[dict(source='body/'+name, field=4, target='resources/new_vertices', kind='segmented',
                   segment=3, vertices=['new_vertex']) for name in ('new_load', 'shared_load')]]
        return streams, resources

    def test_new_commands_reference_shared_new_vertices_without_original_addresses(self):
        streams, resources = self.fixture()
        packed, exports, records, data = layout_graphics('authored', streams, resources, Path('.'))
        self.assertEqual(int.from_bytes(packed['root'][4:8], 'big'), exports['body_new_load'])
        self.assertEqual(int.from_bytes(packed['body'][4:8], 'big'), 0x03000000)
        self.assertEqual(packed['body'][4:8], packed['body'][12:16])
        self.assertEqual(int.from_bytes(data[:2], 'big'), 123)
        self.assertEqual(len(records), 3)
        resources['parts'].insert(0, dict(id='padding', type='padding', offset=0, size=16, data=bytes(16).hex()))
        packed, _, _, _ = layout_graphics('authored', streams, resources, Path('.'))
        self.assertEqual(int.from_bytes(packed['body'][4:8], 'big'), 0x03000010)

    def test_new_texture_uses_explicit_binding_and_validated_dimensions(self):
        words, resources = test_shift_textures.TextureLoadRelocationTests().fixture()
        resources['parts'][0]['original'] = False
        words[0] = words[0][0], 0
        resources['graphics_relocations'] = [dict(source='stream/command_000000', field=4,
                target='resources/image', kind='segmented', segment=3)]
        test_shift_textures.TextureLoadRelocationTests().pack_course(words, resources)
        resources['parts'][0]['height'] = 1
        with self.assertRaisesRegex(ValueError, 'dimensions|height|size'):
            test_shift_textures.TextureLoadRelocationTests().pack_course(words, resources)

    def test_missing_wrong_duplicate_and_unused_bindings_fail(self):
        streams, original = self.fixture()
        for change, error in [
            (lambda m: m['graphics_relocations'][0].update(target='missing'), 'unresolved'),
            (lambda m: m['graphics_relocations'][0].update(segment=3), 'segment-2'),
            (lambda m: m['graphics_relocations'][1].update(vertices=['missing']), 'unresolved vertex'),
            (lambda m: m['graphics_relocations'][1].update(vertices=['new_vertex']*2), 'vertex.*count|range'),
            (lambda m: m['graphics_relocations'].append(dict(m['graphics_relocations'][0])), 'duplicate'),
            (lambda m: m['graphics_relocations'][0].update(source='root/end'), 'unused|no supported'),
        ]:
            resources = copy.deepcopy(original)
            change(resources)
            with self.subTest(error=error), self.assertRaisesRegex(ValueError, error):
                layout_graphics('authored', streams, resources, Path('.'))


if __name__ == '__main__':
    unittest.main()
