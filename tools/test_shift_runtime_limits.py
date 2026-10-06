import unittest
from pathlib import Path
from unittest.mock import patch

from tools.asset_runtime_limits import validate_runtime_size, validate_wave_count, runtime_limits


class RuntimeAssetLimitTests(unittest.TestCase):
    def test_repository_music_packer_rejects_real_asset_growth_past_the_buffer(self):
        from tools.asset_bundles import LAYOUT, load_yaml, materialize, pack_segment
        root = Path(__file__).resolve().parents[1]
        row = next(row for row in load_yaml(root/LAYOUT)['segments'] if row['decoder'] == 'music_sequence')
        manifest, path = materialize(root, row)
        manifest['parts'].append(dict(id='oversized_fixture', type='padding', offset=0,
                                      size=0x10000, data=bytes(0x10000).hex()))
        with patch('tools.asset_bundles.materialize', return_value=(manifest, path)):
            with self.assertRaisesRegex(ValueError, row['name']+':.*MUSIC_SEQUENCE_BUFFER_SIZE'):
                pack_segment(root, row, mode='shiftable')

    def test_wave_slot_count_must_match_the_cpu_instrument_tables(self):
        limits = runtime_limits()
        validate_wave_count('bank', limits['SOUND_BANK_WAVE_COUNT'])
        for count in (limits['SOUND_BANK_WAVE_COUNT']-1, limits['SOUND_BANK_WAVE_COUNT']+1):
            with self.subTest(count=count), self.assertRaisesRegex(ValueError, 'bank:.*CPU tuning-table'):
                validate_wave_count('bank', count)

    def test_music_growth_is_bounded_by_its_dma_destination(self):
        limits = dict(MUSIC_SEQUENCE_BUFFER_SIZE=0x10000, RELOCATABLE_HEAP_SIZE=0x1c0000)
        validate_runtime_size('song', 'music_sequence', 0x10000, limits=limits)
        with self.assertRaisesRegex(ValueError, 'song:.*MUSIC_SEQUENCE_BUFFER_SIZE'):
            validate_runtime_size('song', 'music_sequence', 0x10001, limits=limits)

    def test_encoded_and_decoded_allocations_must_fit_together_with_alignment(self):
        limits = dict(MUSIC_SEQUENCE_BUFFER_SIZE=0x10000, RELOCATABLE_HEAP_SIZE=0x100)
        validate_runtime_size('model', 'embedded_model', 0x80, 0x80, limits=limits)
        with self.assertRaisesRegex(ValueError, 'model:.*staging need 0x110'):
            validate_runtime_size('model', 'embedded_model', 0x81, 0x80, limits=limits)
        with self.assertRaisesRegex(ValueError, 'graphics:.*RELOCATABLE_HEAP_SIZE'):
            validate_runtime_size('graphics', 'course_display_list', 0x101, limits=limits)
        validate_runtime_size('samples', 'sample_bank', 0x1000000, limits=limits)


if __name__ == '__main__':
    unittest.main()
