import unittest
from asset_relocations import Object, Reference, relocate


class AssetRelocationTests(unittest.TestCase):
    def test_internal_growth_and_shared_alias(self):
        objects = [Object('table', bytes(12), 0, 4), Object('mesh', b'12345678', 12, 4),
                   Object('shared', b'', 12, 4, alias='mesh')]
        refs = [Reference('table', 0, 'mesh', 'segmented', segment=2),
                Reference('table', 4, 'shared', 'relative'), Reference('table', 8, 'mesh', 'size')]
        original, _ = relocate('model', objects, refs)
        objects.insert(1, Object('new vertices', bytes(16), 12, 8))
        moved, offsets = relocate('model', objects, refs, mode='shiftable')
        self.assertEqual(int.from_bytes(original[:4], 'big'), 0x0200000c)
        self.assertEqual(int.from_bytes(moved[:4], 'big'), 0x02000020)
        self.assertEqual(int.from_bytes(moved[4:8], 'big'), offsets['mesh'])
        self.assertEqual(offsets['mesh'], offsets['shared'])
        self.assertEqual(int.from_bytes(moved[8:12], 'big'), 8)

    def test_physical_cpu_rom_and_halfword_offsets(self):
        objects = [Object('table', bytes(16), 0), Object('data', bytes(8), 16)]
        for kind, stage, address, expected in [('physical', 'decoded', 0x80001000, 0x1010),
                                              ('cpu', 'decoded', 0x80001000, 0x80001010),
                                              ('rom', 'rom', 0x1000, 0x1010)]:
            result, _ = relocate('fixture', objects, [Reference('table', 0, 'data', kind)], stage=stage, address=address)
            self.assertEqual(int.from_bytes(result[:4], 'big'), expected)
        result, _ = relocate('anim', objects, [Reference('table', 0, 'data', width=2, unit=2)])
        self.assertEqual(int.from_bytes(result[:2], 'big'), 8)

    def test_rejects_missing_target_bad_field_overflow_and_cross_stage(self):
        objects = [Object('table', bytes(4), 0), Object('data', bytes(300), 4)]
        cases = [(Reference('table', 0, 'missing'), 'unresolved'),
                 (Reference('table', 3, 'data'), 'source field'),
                 (Reference('table', 0, 'data', 'size', width=1), 'overflows'),
                 (Reference('table', 0, 'data', 'rom'), 'cross-stage'),
                 (Reference('table', 0, 'data', addend=301), 'addend')]
        for ref, message in cases:
            with self.subTest(message=message), self.assertRaisesRegex(ValueError, message):
                relocate('test asset', objects, [ref])

    def test_rejects_overlap_alignment_duplicate_fields_and_alias_cycles(self):
        cases = [([Object('a', bytes(8), 0), Object('b', bytes(1), 4)], [], 'overlaps'),
                 ([Object('a', bytes(8), 1, 4)], [], 'alignment'),
                 ([Object('a', bytes(8), 0)], [Reference('a', 0, 'a')]*2, 'overlapping relocation'),
                 ([Object('a', b'', 0, alias='b'), Object('b', b'', 0, alias='a')], [], 'cyclic')]
        for objects, refs, message in cases:
            with self.subTest(message=message), self.assertRaisesRegex(ValueError, message):
                relocate('fixture', objects, refs)


if __name__ == '__main__':
    unittest.main()
