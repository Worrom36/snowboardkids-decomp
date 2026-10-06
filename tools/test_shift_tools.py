import subprocess
import sys
import tempfile
from pathlib import Path
import unittest

TOOLS = Path(__file__).resolve().parent


class ShiftToolsTests(unittest.TestCase):
    def test_layout_preserves_extraction_addresses(self):
        with tempfile.TemporaryDirectory() as d:
            source, target = Path(d) / 'in.ld', Path(d) / 'out.ld'
            source.write_text('build/src/a.o(.text);\n.entry 0x80000400 : {}\n.buffers_bss 0x80160480 (NOLOAD) : {}\n')
            subprocess.run([sys.executable, str(TOOLS / 'make_shiftable_linker.py'),
                            str(source), str(target), '--layout'], check=True)
            self.assertEqual(target.read_text(), 'build/shiftable/src/a.o(.text);\n.entry 0x80000400 : {}\n.buffers_bss ALIGN(main_BSS_END, 64) (NOLOAD) : {}\n')

    def test_boot_padding_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            source = Path(d) / 'in.ld'
            source.write_text('    ipl3_ROM_END = __romPos;')
            result = subprocess.run([sys.executable, str(TOOLS / 'make_shiftable_linker.py'),
                                     str(source), str(Path(d) / 'out.ld'), '--after', 'ipl3'], capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn(b'hardware boot placement', result.stderr)

    def test_empty_maps_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            source = Path(d) / 'empty.map'
            source.write_text('0x100 main_ROM_END = __romPos;\n')
            result = subprocess.run([sys.executable, str(TOOLS / 'verify_shifted_map.py'),
                                     str(source), str(source), '--after', 'main', '--pad-size', '16'], capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn(b'empty', result.stderr)

    def test_graphics_and_sample_stream_padding_rejected(self):
        from tools.asset_bundles import ROOT, LAYOUT, load_yaml
        from tools.course_relocations import graphics_groups
        graphic = graphics_groups(ROOT)[0][1][0]['name']
        sample = next(r['name'] for r in load_yaml(ROOT/LAYOUT)['segments'] if r['decoder'] == 'sample_bank')
        for boundary, diagnostic in ((graphic, b'graphics DMA stream'), (sample, b'contiguous sample stream')):
            with self.subTest(boundary=boundary), tempfile.TemporaryDirectory() as d:
                source = Path(d)/'in.ld'
                source.write_text(f'    {boundary}_ROM_END = __romPos;')
                result = subprocess.run([sys.executable, str(TOOLS/'make_shiftable_linker.py'),
                                         str(source), str(Path(d)/'out.ld'), '--after', boundary], capture_output=True)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(diagnostic, result.stderr)


if __name__ == '__main__':
    unittest.main()
