from pathlib import Path
import struct
import tempfile
import unittest

from tools.asset_bundles import ROOT, LAYOUT, load_yaml, materialize
from tools.audio_assets import parse_pointer_bank
from tools.huffman_asset import decompress_huffman_asset
from tools.sample_relocations import layout_samples, bind_pointer_bank
from tools.sample_shift_fixture import make_sources, sample_contents
from tools.structured_relocations import pack_pointer_bank


class SharedSampleFixtureTests(unittest.TestCase):
    def test_all_sample_targets_move_and_bad_wave_binding_is_detected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            rows = make_sources(ROOT, root)
            stream, _, _, _, _ = layout_samples(root, 'SOUND_POINTER_BANK')
            row = next(r for r in load_yaml(ROOT/LAYOUT)['segments'] if r['name'] == 'SOUND_POINTER_BANK')
            manifest, _ = materialize(ROOT, row)
            bank = pack_pointer_bank(bind_pointer_bank(root, manifest))
            rom = (ROOT/'snowboardkids.z64').read_bytes()
            original_bank = decompress_huffman_asset(rom[row['rom_start']:row['rom_end']])[0]
            original_stream = rom[rows[0][0]['rom_start']:rows[-1][0]['rom_end']]
            self.assertNotEqual(stream, original_stream)
            self.assertEqual(len(stream), len(original_stream))
            expected = sample_contents(original_bank, original_stream)
            self.assertEqual(sample_contents(bank, stream), expected)
            wave = next(p for p in parse_pointer_bank(bank)['parts'] if p['type'] == 'wavetable')
            broken = bytearray(bank)
            struct.pack_into('>I', broken, wave['offset'], 16)
            self.assertNotEqual(sample_contents(broken, stream), expected)
            struct.pack_into('>I', broken, wave['offset'], 17)
            with self.assertRaisesRegex(ValueError, 'bounds/alignment'):
                sample_contents(broken, stream)


if __name__ == '__main__':
    unittest.main()
