from pathlib import Path
import tempfile
import unittest

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


if __name__ == '__main__':
    unittest.main()
