import unittest

from tools.rom_reference_context import literal_operands


class RomOperandTests(unittest.TestCase):
    def test_interior_addresses_in_all_supported_call_positions(self):
        source = '''dmaReadRom((u32) 0x154123, buffer, 16);
        loadCompressedRomAsset((void *)0x154124, (void *)0x154127, 0);
        loadRawRomAsset((void *)0x154128, (void *)0x154129, 0);
        osPiStartDma(&message, 0, OS_READ, (u32)(0x15412A), buffer, 16, &queue);
        osPiRawStartDma(OS_READ, 1392940U, buffer, 16);
        __osPiRawStartDma(OS_READ, 0x15412D, buffer, 16);'''
        findings = list(literal_operands(source, 0x1000, 0x800000))
        self.assertEqual(len(findings), 8)
        for offset, token, api in findings:
            self.assertEqual(source[offset:offset+len(token)], token)

    def test_symbol_relative_targets_sizes_flags_and_nonaddress_literals_are_safe(self):
        source = '''dmaReadRom((u32)&asset_ROM_START + 0x154123, nested(a, b), 0x154124);
        osPiStartDma(&message, 0x154124, OS_READ, offset, buffer, 0x154124, &queue);
        loadRawRomAsset(start, end, 0x154124);
        u32 replay = 0x154124; dmaReadRom(0, buffer, 0x154124);'''
        source += '\n/* dmaReadRom(0x154124, p, 4); */ "dmaReadRom(0x154124, p, 4)";'
        self.assertEqual(list(literal_operands(source, 0x1000, 0x800000)), [])

    def test_malformed_known_calls_fail(self):
        for text in ('dmaReadRom(', 'osPiStartDma(0)'):
            with self.assertRaises(ValueError):
                list(literal_operands(text, 0x1000, 0x800000))


if __name__ == '__main__':
    unittest.main()
