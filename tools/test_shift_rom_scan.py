from pathlib import Path
import tempfile
import unittest
import subprocess
import sys

from tools.check_rom_addresses import source_files


class GeneratedRomReferenceTests(unittest.TestCase):
    def test_untracked_sources_and_ignored_active_assembly_are_scanned(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ('src/new.c', 'asm/data/active.data.s', 'asm/matchings/old.s'):
                path = root/name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('.word 0x00123456\n')
            (root/'snowboardkids.ld').write_text('build/asm/data/active.data.o(.data);\n')
            self.assertEqual({p.relative_to(root).as_posix() for p in source_files(root)},
                             {'src/new.c', 'asm/data/active.data.s'})
            (root/'asm/data/active.data.s').unlink()
            with self.assertRaisesRegex(ValueError, 'missing generated assembly input'):
                list(source_files(root))

    def test_empty_generated_input_coverage_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root/'snowboardkids.ld').write_text('SECTIONS {}')
            with self.assertRaisesRegex(ValueError, 'unexpectedly empty'):
                list(source_files(root))

    def test_interior_api_operand_and_generated_legacy_symbol_are_rejected(self):
        scanner = Path(__file__).with_name('check_rom_addresses.py')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ('src', 'asm/data', 'config/courses'):
                (root/name).mkdir(parents=True)
            (root/'snowboardkids.yaml').write_text('start: 0x1000\nstart: 0x200000\nstart: 0x800000\n')
            (root/'config/rom_address_allowlist.txt').write_text('# path token reason\n')
            (root/'snowboardkids.ld').write_text('build/asm/data/active.data.o(.data);\n')
            source, assembly = root/'src/new.c', root/'asm/data/active.data.s'
            source.write_text('dmaReadRom(asset_ROM_START + 16, buffer, 0x154123);\n')
            assembly.write_text('.word 0\n')

            def scan():
                return subprocess.run([sys.executable, str(scanner), '--root', str(root)],
                                      text=True, capture_output=True)

            self.assertEqual(scan().returncode, 0)
            source.write_text('dmaReadRom(0x154123, buffer, 16);\n')
            result = scan()
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('src/new.c:1: hardcoded ROM address 0x154123', result.stdout)
            source.write_text('dmaReadRom(asset_ROM_START + 16, buffer, 16);\n')
            assembly.write_text('.word D_154123\n')
            result = scan()
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('asm/data/active.data.s:1: hardcoded ROM address D_154123', result.stdout)


if __name__ == '__main__':
    unittest.main()
