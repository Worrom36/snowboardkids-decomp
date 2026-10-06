import copy
import unittest
from tools.audio_assets import pack_pointer_bank as pack_matching
from tools.structured_relocations import pointer_bank_manifest, pack_pointer_bank


class PointerBankRelocationTests(unittest.TestCase):
    def manifest(self):
        return dict(name='audio fixture', decompressed_size=92, parts=[
            dict(type='bank_header', offset=0, size=28, signature=b'N64 PtrTables \0\0'.hex(), wave_offsets=[28, 48]),
            dict(type='wavetable', offset=28, size=20, sample_offset=16, sample_size=9,
                 wave_type=0, flags=0, reserved=0, loop_offset=0, book_offset=68),
            dict(type='wavetable', offset=48, size=20, sample_offset=16, sample_size=9,
                 wave_type=0, flags=0, reserved=0, loop_offset=0, book_offset=68),
            dict(type='codebook', offset=68, size=24, order=1, predictors=1, coefficients=[0]*8)])

    def test_shared_book_moves_once(self):
        m = self.manifest()
        baseline = pack_matching(m)
        self.assertEqual(pack_pointer_bank(m), baseline)
        identified = pointer_bank_manifest(m)
        identified['parts'].insert(3, dict(id='new_padding', type='padding', offset=68, size=16, data=bytes(16).hex()))
        result = pack_pointer_bank(identified)
        self.assertEqual(int.from_bytes(result[44:48], 'big'), 84)
        self.assertEqual(int.from_bytes(result[64:68], 'big'), 84)
        self.assertEqual(result[84:], baseline[68:])

    def test_bad_target_and_missing_coverage(self):
        m = pointer_bank_manifest(self.manifest())
        m['relocations'].pop()
        with self.assertRaisesRegex(ValueError, 'coverage'):
            pack_pointer_bank(m)
        m = pointer_bank_manifest(self.manifest())
        m['relocations'][-1]['target'] = 'missing_book'
        with self.assertRaisesRegex(ValueError, 'unresolved'):
            pack_pointer_bank(m)


class MusicRelocationTests(unittest.TestCase):
    def manifest(self):
        import struct
        from tools.music_assets import parse_music
        data = struct.pack('>9I', 1, 24, 28, 32, 0, 0, 36, 0, 0) + bytes([0xa1, 0, 0, 0, 0, 0, 0])
        return dict(name='music fixture', decompressed_size=len(data), **parse_music(data)), data

    def test_sequence_move_preserves_goto_and_tables(self):
        from tools.structured_relocations import music_manifest, pack_music
        m, data = self.manifest()
        self.assertEqual(pack_music(m), data)
        m = music_manifest(m)
        m['parts'].insert(-1, dict(id='padding', type='padding', offset=36, size=16, data=bytes(16).hex()))
        relocated = pack_music(m)
        self.assertEqual(int.from_bytes(relocated[24:28], 'big'), 52)
        self.assertEqual(relocated[52:], data[36:])

    def test_missing_branch_coverage_rejected(self):
        from tools.structured_relocations import music_manifest, pack_music
        m, _ = self.manifest()
        m = music_manifest(m)
        m['relocations'].pop()
        with self.assertRaisesRegex(ValueError, 'coverage'):
            pack_music(m)
        m, _ = self.manifest()
        m = music_manifest(m)
        m['event_relocations'].clear()
        with self.assertRaisesRegex(ValueError, 'coverage'):
            pack_music(m)

    def test_insert_event_before_goto_target(self):
        from tools.structured_relocations import music_manifest, pack_music
        m, _ = self.manifest()
        m = music_manifest(m)
        m['parts'][-1]['events'].insert(0, dict(id='new_tempo', command='tempo', operands=[120]))
        data = pack_music(m)
        self.assertEqual(data[36:39], bytes([0x85, 120, 0xa1]))
        self.assertEqual(int.from_bytes(data[39:41], 'big'), 2)

    def test_all_goto_stream_operands_follow_events(self):
        import struct
        from tools.music_assets import parse_music
        from tools.structured_relocations import music_manifest, pack_music
        data = struct.pack('>9I', 1, 24, 28, 32, 0, 0, 36, 45, 47)
        data += bytes([0x85, 120, 0xa1, 0, 0, 0, 1, 0, 1, 60, 70, 60, 70])
        m = music_manifest(dict(name='three branches', **parse_music(data)))
        self.assertEqual(pack_music(m), data)
        for part in m['parts']:
            if part['type'] == 'modulation':
                part['events'].insert(0, dict(id='new_value', value=20))
        shifted = pack_music(m)
        self.assertEqual(shifted[41:45], bytes([0, 2, 0, 2]))
        broken = copy.deepcopy(m)
        broken['event_relocations'].pop()
        with self.assertRaisesRegex(ValueError, 'coverage'):
            pack_music(broken)
        broken = copy.deepcopy(m)
        broken['event_relocations'][0]['target'] = 'missing_event'
        with self.assertRaisesRegex(ValueError, 'unresolved'):
            pack_music(broken)

    def test_malformed_command_state_rejected(self):
        from tools.structured_relocations import music_manifest, pack_music
        m, _ = self.manifest()
        m = music_manifest(m)
        m['parts'][-1]['events'].insert(0, dict(id='bad_note', note=30))
        with self.assertRaisesRegex(ValueError, 'boundaries|truncated'):
            pack_music(m)

    def test_branch_offset_overflow(self):
        from tools.structured_relocations import music_manifest, pack_music
        m, _ = self.manifest()
        m = music_manifest(m)
        m['parts'][-1]['events'][:0] = [dict(id=f'tempo_{i}', command='tempo', operands=[120])
                                       for i in range(32768)]
        with self.assertRaisesRegex(ValueError, 'overflows 2-byte'):
            pack_music(m)

    def test_music_corpus_matches(self):
        from tools.asset_bundles import ROOT, LAYOUT, load_yaml, materialize
        from tools.music_assets import pack_music as pack_matching_music
        from tools.structured_relocations import pack_music
        rows = [r for r in load_yaml(ROOT/LAYOUT)['segments'] if r['decoder'] == 'music_sequence']
        self.assertEqual(len(rows), 17)
        for row in rows:
            with self.subTest(asset=row['name']):
                manifest, path = materialize(ROOT, row)
                self.assertEqual(pack_music(manifest), pack_matching_music(manifest))


if __name__ == '__main__':
    unittest.main()
