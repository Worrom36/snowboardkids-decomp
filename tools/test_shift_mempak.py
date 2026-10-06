from pathlib import Path
import struct
import tempfile
import unittest

from tools.runtime_mempak import compare_saved_data, stable_digest


def pak(serial, device_id=1):
    data = bytearray(0x8000)
    identity = struct.pack('>6IHBB', *([serial]*6), device_id, 1, 0)
    total = sum(struct.unpack('>14H', identity)) & 0xffff
    identity += struct.pack('>HH', total, (0xfff2-total) & 0xffff)
    for offset in (0x20, 0x60, 0x80, 0xc0):
        data[offset:offset+32] = identity
    return data


class MempakComparisonTests(unittest.TestCase):
    def test_distinct_serials_preserve_file_and_allocation_comparisons(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            left, right = root/'baseline', root/'shifted'
            left.mkdir(); right.mkdir()
            (left/'game.mpk').write_bytes(pak(1))
            original = pak(2)
            (right/'game.mpk').write_bytes(original)
            self.assertTrue(compare_saved_data(left, right)[1])
            for offset in (0, 0x100, 0x300, 0x500, 0x7fff):
                edited = bytearray(original)
                edited[offset] ^= 1
                (right/'game.mpk').write_bytes(edited)
                with self.subTest(offset=offset):
                    self.assertFalse(compare_saved_data(left, right)[1])
            (right/'game.mpk').write_bytes(pak(2, device_id=2))
            self.assertFalse(compare_saved_data(left, right)[1])

    def test_invalid_ids_and_empty_coverage_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaisesRegex(ValueError, 'nonempty'):
                compare_saved_data(root, root)
            path = root/'game.mpk'
            for offset, message in ((0x20, 'checksums'), (0x60, 'backups')):
                data = pak(1); data[offset] ^= 1
                path.write_bytes(data)
                with self.subTest(offset=offset), self.assertRaisesRegex(ValueError, message):
                    stable_digest(path)
            path.write_bytes(bytes(100))
            with self.assertRaisesRegex(ValueError, '32 KB'):
                stable_digest(path)


if __name__ == '__main__':
    unittest.main()
