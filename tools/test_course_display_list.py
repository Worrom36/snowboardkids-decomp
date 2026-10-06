import unittest

from tools.splat_ext.course_display_list import N64SegCourse_display_list
from tools.course_relocations import command_ids
from tools.graphics_words import word_kinds


class CourseDisplayListTests(unittest.TestCase):
    def test_raw_words_preserve_each_big_endian_command(self):
        data = bytes.fromhex("E700000000000000 B800000000000000")

        source = 'Gfx graphics[] = {\n'+N64SegCourse_display_list._raw_words(data)+'\n};\n'

        self.assertIn("{{0xE7000000, 0x00000000}}", source)
        self.assertIn("{{0xB8000000, 0x00000000}}", source)
        identities = command_ids(source, data, len(data), 'graphics')
        self.assertEqual(identities, ['command_000000', 'command_000008'])
        spec = dict(name='graphics', start=0, end=len(data))
        self.assertEqual(word_kinds(source, identities, spec), ['command', 'command'])


if __name__ == "__main__":
    unittest.main()
