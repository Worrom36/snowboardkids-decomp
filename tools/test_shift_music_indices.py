import struct
import unittest

from tools.music_assets import parse_music
from tools.structured_relocations import music_manifest, pack_music


class MusicIndexRelocationTests(unittest.TestCase):
    def fixture(self):
        data = struct.pack('>9I', 1, 24, 28, 32, 36, 0, 50, 0, 0)
        data += bytes([1])*7 + bytes([2])*7
        data += bytes([0x90, 0, 0xa1, 0, 2, 0, 0, 0, 0])
        return music_manifest(dict(name='envelope fixture', **parse_music(data))), data

    def test_envelope_move_updates_event_index(self):
        manifest, original = self.fixture()
        self.assertEqual(pack_music(manifest), original)
        table = next(p for p in manifest['parts'] if p['type'] == 'envelopes')
        table['entry_order'] = list(reversed(table['entry_ids']))
        moved = pack_music(manifest)
        envelope = int.from_bytes(moved[16:20], 'big')
        stream = int.from_bytes(moved[24:28], 'big')
        self.assertEqual(moved[envelope:envelope+14], bytes([2])*7+bytes([1])*7)
        self.assertEqual(moved[stream:stream+2], bytes([0x90, 1]))

    def test_index_growth_updates_goto_target(self):
        manifest, _ = self.fixture()
        table = next(p for p in manifest['parts'] if p['type'] == 'envelopes')
        table['entries'] += [[3]*7 for _ in range(128)]
        table['entry_ids'] += [f'new_envelope_{i}' for i in range(128)]
        table['entry_order'] = table['entry_ids'][1:]+table['entry_ids'][:1]
        moved = pack_music(manifest)
        stream = int.from_bytes(moved[24:28], 'big')
        self.assertEqual(moved[stream:stream+6], bytes([0x90, 0x80, 129, 0xa1, 0, 3]))

    def test_track_count_follows_pointer_tables(self):
        manifest, _ = self.fixture()
        for part in manifest['parts']:
            if part['type'] == 'pointers':
                part['values'].append(0)
        moved = pack_music(manifest)
        self.assertEqual(int.from_bytes(moved[:4], 'big'), 2)
        self.assertEqual(parse_music(moved)['parts'][0]['track_count'], 2)
        next(p for p in manifest['parts'] if p.get('role') == 'pitch')['values'].pop()
        with self.assertRaisesRegex(ValueError, 'equal counts'):
            pack_music(manifest)

    def test_invalid_envelope_bindings_and_unsupported_drums_fail(self):
        manifest, _ = self.fixture()
        manifest['envelope_relocations'].clear()
        with self.assertRaisesRegex(ValueError, 'coverage'):
            pack_music(manifest)
        manifest, _ = self.fixture()
        manifest.pop('envelope_relocations')
        event = next(p for p in manifest['parts'] if p['type'] == 'sequence')['events'][0]
        event['envelope_id'] = 'missing'
        with self.assertRaisesRegex(ValueError, 'unresolved envelope'):
            pack_music(manifest)
        manifest, _ = self.fixture()
        table = next(p for p in manifest['parts'] if p['type'] == 'envelopes')
        table['entry_order'] = [table['entry_ids'][0]]*2
        with self.assertRaisesRegex(ValueError, 'every envelope exactly once'):
            pack_music(manifest)
        manifest, _ = self.fixture()
        manifest['parts'].append(dict(id='drums', type='pointers', role='drums', values=[16], offset=60, size=4))
        with self.assertRaisesRegex(ValueError, 'typed wave/envelope bindings'):
            pack_music(manifest)

    def test_runtime_pointer_rebasing_block_cannot_be_split(self):
        manifest, _ = self.fixture()
        manifest['parts'].insert(2, dict(id='gap', type='padding', offset=28, size=4, data='00000000'))
        with self.assertRaisesRegex(ValueError, 'three contiguous track tables first'):
            pack_music(manifest)


if __name__ == '__main__':
    unittest.main()
