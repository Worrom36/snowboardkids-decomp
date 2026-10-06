import unittest
from huffman_asset import CompressionMetadata, compress_huffman_asset, compress_flexible, decompress_huffman_asset


class FlexibleCompressionTests(unittest.TestCase):
    def test_preserves_original_padding_and_tokenization(self):
        metadata = CompressionMetadata(0, bytes.fromhex('000203020100'), '10', b'\xaa')
        original = compress_huffman_asset(bytes((0, 1, 2, 0)), metadata)
        self.assertEqual(compress_flexible(bytes((0, 1, 2, 0)), original), original)

    def test_growth_shrink_and_new_alphabet(self):
        original = compress_flexible(b'a' * 500)
        for payload in (b'', b'x', bytes(range(256))*100, b'\xff'*900):
            encoded = compress_flexible(payload, original)
            self.assertEqual(decompress_huffman_asset(encoded)[0], payload)
            self.assertEqual(compress_flexible(payload, original), encoded)
            self.assertEqual(len(encoded) % 16, 0)
        self.assertGreater(len(compress_flexible(bytes(range(256))*100)), len(original))
        self.assertLess(len(compress_flexible(b'x')), len(original))


if __name__ == '__main__':
    unittest.main()
