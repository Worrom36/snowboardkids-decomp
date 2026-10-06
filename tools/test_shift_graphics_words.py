import copy
from pathlib import Path
import struct
import unittest

from tools.graphics_words import data_offsets, word_kinds
from tools.course_relocations import layout_graphics


class GraphicsWordKindTests(unittest.TestCase):
    def test_data_follows_original_identity_and_new_explicit_annotations(self):
        spec = dict(name='stream', start=100, end=132,
                    data_ranges=[dict(start=0, end=8, reason='embedded vertex bytes')])
        source = '''Gfx stream[] = {
            {{0xB8000000, 0x00000000}}, /* object:command_000008 */
            {{0xFD000000, 0x03000000}}, /* object:command_000000 */
            {{0x06000000, 0x02000000}}, /* object:new_bytes kind:data */
        };'''
        self.assertEqual(word_kinds(source, ['command_000008', 'command_000000', 'new_bytes'], spec),
                         ['command', 'data', 'data'])
        with self.assertRaisesRegex(ValueError, 'kind:command or kind:data'):
            word_kinds(source.replace('kind:data', 'kind:unknown'), ['a', 'b', 'c'], spec)

    def test_invalid_extraction_ranges_fail(self):
        for records in ([dict(start=1, end=8, reason='unaligned')],
                        [dict(start=0, end=40, reason='past end')],
                        [dict(start=0, end=8, reason='')],
                        [dict(start=0, end=8, reason='one'), dict(start=0, end=16, reason='two')]):
            with self.subTest(records=records), self.assertRaises(ValueError):
                data_offsets(records, 32, 'stream')

    def fixture(self):
        data = struct.pack('>8I', 0xfd500000, 0x03001234, 0x06000000, 0x02005678,
                           0x04000400, 0x03000000, 0xb8000000, 0)
        stream = dict(name='stream', offset=0, data=data,
                      ids=['texture_shaped_data', 'call_shaped_data', 'command_000010', 'end'],
                      kinds=['data', 'data', 'command', 'command'])
        vertex = dict(id='vertex_000000', x=123, y=0, z=0, flag=0, s=0, t=0, r=255, g=255, b=255, a=255)
        manifest = dict(name='resources', decompressed_size=16, parts=[
            dict(id='prefix', type='padding', offset=0, size=16, data=bytes(16).hex()),
            dict(id='vertices', type='vertices', offset=0, size=16, vertices=[vertex])])
        return stream, manifest

    def test_pointer_shaped_data_is_preserved_while_command_relocates(self):
        stream, manifest = self.fixture()
        packed, _, records, _ = layout_graphics('test', [stream], manifest, Path('.'))
        self.assertEqual(packed['stream'][:16], stream['data'][:16])
        self.assertEqual(int.from_bytes(packed['stream'][20:24], 'big'), 0x03000010)
        self.assertEqual(len(records), 1)

    def test_explicit_reference_cannot_bind_data_or_execute_data(self):
        stream, manifest = self.fixture()
        for source, target, segment in [('stream/texture_shaped_data', 'resources/vertices', 3),
                                         ('stream/call', 'stream/texture_shaped_data', 2)]:
            candidate = copy.deepcopy(stream)
            if source == 'stream/call':
                candidate['ids'].append('call')
                candidate['kinds'].append('command')
                candidate['data'] += struct.pack('>II', 0x06000000, 0)
            manifest['graphics_relocations'] = [dict(source=source, field=4, target=target,
                                                     kind='segmented', segment=segment)]
            with self.subTest(source=source), self.assertRaisesRegex(ValueError, 'address operand|data'):
                layout_graphics('test', [candidate], manifest, Path('.'))

    def test_unannotated_pointer_command_and_implicit_data_call_fail(self):
        stream, manifest = self.fixture()
        stream['data'] += struct.pack('>II', 0x01000040, 0x03000000)
        stream['ids'].append('new_matrix')
        stream['kinds'].append('command')
        with self.assertRaisesRegex(ValueError, 'new_matrix: unsupported graphics command'):
            layout_graphics('test', [stream], manifest, Path('.'))
        stream['ids'][0] = 'command_000000'
        stream['data'] = stream['data'][:-8] + struct.pack('>II', 0x06000000, 0x02000000)
        with self.assertRaisesRegex(ValueError, 'target .* is data'):
            layout_graphics('test', [stream], manifest, Path('.'))

    def test_extraction_preserves_types_and_exact_words(self):
        from tools.splat_ext.course_display_list import N64SegCourse_display_list
        stream, _ = self.fixture()
        ranges = [dict(start=0, end=16, reason='embedded data')]
        source = 'Gfx stream[] = {\n' + N64SegCourse_display_list._raw_words(stream['data'], ranges) + '\n};'
        from tools.course_relocations import command_ids
        ids = command_ids(source, stream['data'], len(stream['data']), 'stream')
        self.assertEqual(word_kinds(source, ids, dict(name='stream', start=0, end=32)), stream['kinds'])


if __name__ == '__main__':
    unittest.main()
