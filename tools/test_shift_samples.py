from pathlib import Path
import tempfile
import unittest
import yaml
from tools.sample_relocations import layout_samples, pack_sample_slice, bind_pointer_bank


class SampleRelocationTests(unittest.TestCase):
    def fixture(self, root):
        (root/'assets/layout').mkdir(parents=True)
        (root/'a.vadpcm').write_bytes(bytes(18))
        (root/'b.vadpcm').write_bytes(bytes([1])*18)
        first = dict(name='FIRST', bank_asset='BANK', compression='none', parts=[
            dict(type='signature', offset=0, size=16, text='N64 WaveTables ', terminator=0),
            dict(type='sample', offset=16, size=18, path='a.vadpcm', source_offset=0, source_size=18),
            dict(type='padding', offset=34, size=14, data=bytes(14).hex()),
            dict(type='sample', offset=48, size=16, path='b.vadpcm', source_offset=0, source_size=18)])
        second = dict(name='SECOND', bank_asset='BANK', compression='none', parts=[
            dict(type='sample', offset=0, size=2, path='b.vadpcm', source_offset=16, source_size=18),
            dict(type='padding', offset=2, size=14, data=bytes(14).hex())])
        rows = [dict(name='FIRST', decoder='sample_bank', rom_start=0x100, rom_end=0x140,
                     source='first.yaml', compression='none'),
                dict(name='SECOND', decoder='sample_bank', rom_start=0x140, rom_end=0x150,
                     source='second.yaml', compression='none')]
        (root/'first.yaml').write_text(yaml.safe_dump(first))
        (root/'second.yaml').write_text(yaml.safe_dump(second))
        (root/'assets/layout/segments.yaml').write_text(yaml.safe_dump(dict(segments=rows)))
        return rows, first, second

    def test_split_shared_sample_growth_and_shrink(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            rows, first, second = self.fixture(root)
            payload, offsets, sizes, targets, _ = layout_samples(root, 'BANK')
            self.assertEqual(len(payload), 80)
            self.assertEqual(len(sizes), 2)
            self.assertEqual(payload[48:66], bytes([1])*18)
            (root/'a.vadpcm').write_bytes(bytes(36))
            payload, offsets, sizes, targets, _ = layout_samples(root, 'BANK')
            self.assertEqual(offsets[targets[48]], 64)
            self.assertEqual(len(payload), 96)
            self.assertEqual(pack_sample_slice(root, rows[0], first)+pack_sample_slice(root, rows[1], second), payload)
            bank = dict(name='BANK', parts=[dict(type='wavetable', offset=0, sample_offset=48,
                                                 sample_size=18, loop_offset=0)]*2)
            bound = bind_pointer_bank(root, bank)
            self.assertEqual([p['sample_offset'] for p in bound['parts']], [64, 64])
            (root/'a.vadpcm').write_bytes(bytes(9))
            payload, offsets, sizes, targets, _ = layout_samples(root, 'BANK')
            self.assertEqual(offsets[targets[48]], 32)
            self.assertEqual(pack_sample_slice(root, rows[1], second), b'')

    def test_bad_frames_and_missing_sample_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            bank = dict(name='BANK', parts=[dict(type='wavetable', offset=0, sample_offset=17,
                                                 sample_size=18, loop_offset=0)])
            with self.assertRaisesRegex(ValueError, 'unresolved sample'):
                bind_pointer_bank(root, bank)
            (root/'a.vadpcm').write_bytes(bytes(19))
            with self.assertRaisesRegex(ValueError, '9-byte frames'):
                layout_samples(root, 'BANK')


if __name__ == '__main__':
    unittest.main()
