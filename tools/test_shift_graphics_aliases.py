from pathlib import Path
import tempfile
import unittest

from tools.asset_bundles import LAYOUT, commands, load_yaml, materialize
from tools.course_relocations import command_ids, graphics_groups, layout_graphics
from tools.graphics_shift_fixture import cpu_graphics_bindings, insert_internal_commands
from tools.graphics_words import word_kinds


class GraphicsCpuAliasTests(unittest.TestCase):
    def test_real_cpu_roots_follow_different_internal_displacements(self):
        root = Path(__file__).resolve().parents[1]
        rows = {row['name']: row for row in load_yaml(root/LAYOUT)['segments']}
        bindings = cpu_graphics_bindings(root)
        self.assertEqual(len(bindings), 21)
        old, new = {}, {}
        for bank, group in graphics_groups(root):
            if not any(binding['target'].startswith(spec['name']+'_command_')
                       for binding in bindings for spec in group):
                continue
            streams = []
            for spec in group:
                path = root/rows[spec['name']]['source']
                data = commands(path)
                ids = command_ids(path.read_text(), data, spec['end']-spec['start'], spec['name'])
                streams.append(dict(name=spec['name'], offset=spec['start']-bank['graphics_start'], data=data,
                                    ids=ids, kinds=word_kinds(path.read_text(), ids, spec)))
            manifest, path = materialize(root, rows[bank['name']])
            _, exports, _, _ = layout_graphics(bank['name'], streams, manifest, path.parent)
            old.update({binding['alias']: exports[binding['target']] for binding in bindings if binding['target'] in exports})
            insert_internal_commands(streams)
            _, exports, _, _ = layout_graphics(bank['name'], streams, manifest, path.parent)
            new.update({binding['alias']: exports[binding['target']] for binding in bindings if binding['target'] in exports})
        self.assertEqual(len(new), 21)
        for alias, value in old.items():
            if alias.startswith('D_'):
                self.assertEqual(value, int(alias[2:], 16))
        self.assertEqual(new['D_2000000'], old['D_2000000'])
        self.assertGreater(new['D_20006C8'], old['D_20006C8'])
        self.assertGreater(new['gSpeedFanBackDisplayList']-old['gSpeedFanBackDisplayList'],
                           new['D_20006C8']-old['D_20006C8'])

    def test_fixed_interior_aliases_and_empty_binding_sets_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root/'linker_scripts').mkdir()
            for text in ('root = stream_VRAM + 0x10;', ''):
                (root/'linker_scripts/game_syms.ld').write_text(text)
                with self.subTest(text=text), self.assertRaisesRegex(ValueError, 'stable command|empty'):
                    cpu_graphics_bindings(root)


if __name__ == '__main__':
    unittest.main()
