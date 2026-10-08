import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
AS = shutil.which('mips-linux-gnu-as')


def sections(path):
    data = path.read_bytes()
    offset = struct.unpack_from('>I', data, 0x20)[0]
    size, count, names_index = struct.unpack_from('>HHH', data, 0x2e)
    headers = [struct.unpack_from('>10I', data, offset+i*size) for i in range(count)]
    names_header = headers[names_index]
    names = data[names_header[4]:names_header[4]+names_header[5]]
    result = {}
    for header in headers:
        name = names[header[0]:names.index(b'\0', header[0])].decode()
        if header[1] != 8:
            result[name] = data[header[4]:header[4]+header[5]]
    return result


@unittest.skipUnless(AS, 'requires MIPS assembler')
class SectionPaddingTests(unittest.TestCase):
    def setUp(self):
        self.workspace = tempfile.TemporaryDirectory()
        self.addCleanup(self.workspace.cleanup)
        self.path = Path(self.workspace.name) / 'sample.o'
        source = ('.set noreorder\n.text\n.globl sample\n.ent sample\nsample:\n'
                  'lui $2,%hi(external)\naddiu $2,$2,%lo(external)\njr $31\nnop\n'
                  '.end sample\n.size sample,.-sample\n.data\n.word external\n')
        subprocess.run([AS, '-o', str(self.path)], input=source, text=True, check=True)

    def resize(self, *args):
        return subprocess.run([sys.executable, str(ROOT/'tools/trim_elf_section_tail.py'),
                               str(self.path), '.text', '4', *args], capture_output=True, text=True)

    def test_padding_preserves_symbols_relocations_and_other_sections(self):
        before = sections(self.path)
        self.assertEqual(self.resize('--pad').returncode, 0)
        padded = sections(self.path)
        self.assertEqual(padded.pop('.text'), before['.text']+bytes(4))
        self.assertEqual(padded, {k: v for k, v in before.items() if k != '.text'})
        self.assertEqual(self.resize().returncode, 0)
        self.assertEqual(sections(self.path), before)

    def test_trimming_into_function_is_rejected(self):
        before = self.path.read_bytes()
        self.assertNotEqual(self.resize().returncode, 0)
        self.assertEqual(self.path.read_bytes(), before)

    def test_padding_executable_is_rejected(self):
        data = bytearray(self.path.read_bytes())
        struct.pack_into('>H', data, 0x10, 2)
        self.path.write_bytes(data)
        self.assertNotEqual(self.resize('--pad').returncode, 0)
        self.assertEqual(self.path.read_bytes(), data)


if __name__ == '__main__':
    unittest.main()
