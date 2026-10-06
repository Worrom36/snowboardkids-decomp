from pathlib import Path
import struct
import re
import unittest

from tools.course_relocations import command_ids, layout_graphics, resource_layout
from tools.vertex_relocations import annotate_vertices


class CourseRelocationTests(unittest.TestCase):
    def fixture(self):
        vertex = dict(x=0, y=0, z=0, flag=0, s=0, t=0, r=255, g=255, b=255, a=255)
        manifest = dict(name='resources', decompressed_size=32, parts=[
            dict(id='vertices', type='vertices', offset=0, size=16, vertices=[vertex]),
            dict(id='tail', type='raw', offset=16, size=16, data=bytes(16).hex())])
        streams = [dict(name='FIRST', offset=0, ids=['command_000000', 'command_000008'],
                        data=struct.pack('>4I', 0x06000000, 0x02000010, 0xB8000000, 0)),
                   dict(name='SECOND', offset=16, ids=['command_000000', 'command_000008', 'command_000010'],
                        data=struct.pack('>6I', 0x04000400, 0x03000000, 0x04000400, 0x03000000, 0xB8000000, 0))]
        return streams, manifest

    def test_shared_resources_and_cross_stream_commands_move(self):
        streams, m = self.fixture()
        original = layout_graphics('fixture', streams, m, Path('.'), 'matching')
        self.assertEqual(original[0]['FIRST'], streams[0]['data'])
        streams[0]['ids'].insert(0, 'new_sync')
        streams[0]['data'] = struct.pack('>II', 0xe7000000, 0)+streams[0]['data']
        m['parts'].reverse()
        packed, symbols, records, resources = layout_graphics('fixture', streams, m, Path('.'))
        self.assertEqual(symbols['FIRST_command_000000'], 0x02000008)
        self.assertEqual(symbols['SECOND_command_000000'], 0x02000018)
        self.assertEqual(struct.unpack_from('>I', packed['FIRST'], 12)[0], 0x02000018)
        self.assertEqual([r['value'] for r in records[1:]], [0x03000010]*2)
        self.assertEqual([r['target'] for r in records[1:]], ['resources/vertices']*2)
        self.assertEqual(records[0]['location'], 12)
        self.assertEqual(len(resources), 32)

    def test_growth_and_shrink(self):
        streams, m = self.fixture()
        annotate_vertices(m['parts'][0])
        vertex = m['parts'][0]['vertices'][0]
        m['parts'][0]['vertices'] += [dict(vertex, id='new_a'), dict(vertex, id='new_b')]
        self.assertEqual(len(layout_graphics('fixture', streams, m, Path('.'))[3]), 64)
        m['parts'][0]['vertices'] = []
        with self.assertRaisesRegex(ValueError, 'resizing legacy|unresolved vertex'):
            layout_graphics('fixture', streams, m, Path('.'))

    def test_interior_vertices_follow_identity(self):
        streams, m = self.fixture()
        vertices = m['parts'][0]
        annotate_vertices(vertices)
        vertices['vertices'].insert(0, dict(vertices['vertices'][0], id='new_vertex'))
        packed, _, records, _ = layout_graphics('fixture', streams, m, Path('.'))
        self.assertEqual([r['value'] for r in records[1:]], [0x03000010]*2)
        self.assertEqual(struct.unpack_from('>I', packed['SECOND'], 4)[0], 0x03000010)
        del vertices['vertices'][1]
        with self.assertRaisesRegex(ValueError, 'unresolved vertex vertex_000000'):
            layout_graphics('fixture', streams, m, Path('.'))

    def test_unresolved_and_invalid_alignment(self):
        streams, m = self.fixture()
        streams[1]['ids'][0] = 'replacement'
        with self.assertRaisesRegex(ValueError, 'unresolved segment-2'):
            layout_graphics('fixture', streams, m, Path('.'))
        streams, m = self.fixture()
        m['parts'][0]['offset'] = 1
        with self.assertRaisesRegex(ValueError, 'alignment'):
            resource_layout(m, Path('.'), 'matching')

    def test_command_identity_coverage(self):
        source = 'Gfx x[] = {\n{{0xB8000000, 0x00000000}}, /* object:command_000008 */\n};'
        self.assertEqual(command_ids(source, bytes(8), 16, 'x'), ['command_000008'])
        with self.assertRaisesRegex(ValueError, 'requires object identities'):
            command_ids(source.replace('object:', 'old:'), bytes(8), 16, 'x')
        source = source.replace('};', '{{0xB8000000, 0x00000000}},\n};')
        with self.assertRaisesRegex(ValueError, 'every graphics word pair'):
            command_ids(source, bytes(16), 16, 'x')

    def test_original_course_corpus(self):
        from tools.asset_bundles import LAYOUT, load_yaml, materialize, commands
        from tools.course_relocations import graphics_groups, pack_graphics_group
        from tools.course_graphics_common import pack_course_model_resources
        root = Path(__file__).resolve().parents[1]
        rows = {r['name']: r for r in load_yaml(root/LAYOUT)['segments']}
        groups = graphics_groups(root)
        self.assertEqual(len(groups), 19)
        checked = 0
        for bank, group in groups:
            with self.subTest(bank=bank['name']):
                packed, exports, records, resources = pack_graphics_group(root, rows[group[0]['name']])
                self.assertTrue(exports)
                self.assertTrue(records)
                checked += len(records)
                for name, data in packed.items():
                    self.assertEqual(data, commands(root/rows[name]['source']))
                m, path = materialize(root, rows[bank['name']])
                self.assertEqual(resources, pack_course_model_resources(m, path.parent))
        self.assertEqual(checked, 5886)

    def test_only_uninstantiated_rank_slots_keep_original_fill_values(self):
        root = Path(__file__).resolve().parents[1]
        scene = (root/'src/race/scene/race_scene_setup.c').read_text()
        pieces = re.split(r'\n        case (\d+):', scene)
        active = set()
        for course, body in zip(pieces[1::2], pieces[2::2]):
            for slot in re.findall(r'&initRaceCourseRankModel,\s*0,\s*0x64,\s*(\d+)\)', body):
                active.update((int(course)*4+int(slot)*2, int(course)*4+int(slot)*2+1))
        source = (root/'src/race/ui/race_ui_effects.c').read_text()
        body = re.search(r'gRaceCourseRankModelDisplayLists\[\] = \{(.*?)\};', source, re.S)[1]
        entries = [entry.strip() for entry in body.split(',') if entry.strip()]
        self.assertEqual(len(entries), 40)
        self.assertEqual(len(active), 18)
        for index, entry in enumerate(entries):
            with self.subTest(slot=index):
                if index in active:
                    self.assertRegex(entry, r'_command_[0-9A-F]{6}$')
                else:
                    self.assertIn('Unused course/slot', entry)


if __name__ == '__main__':
    unittest.main()
