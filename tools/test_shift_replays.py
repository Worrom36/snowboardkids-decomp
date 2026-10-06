import copy
import csv
from pathlib import Path
import tempfile
import unittest

from tools.asset_bundles import LAYOUT, load_yaml, materialize
from tools.huffman_asset import compress_flexible, decompress_huffman_asset
from tools.readable_assets import pack_payload


class ReplayLayoutTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root = Path(__file__).resolve().parents[1]
        cls.rows = [row for row in load_yaml(cls.root/LAYOUT)['segments'] if row['decoder'] == 'replay']

    def test_all_replay_payloads_keep_the_runtime_player_and_frame_order(self):
        self.assertEqual(len(self.rows), 17)
        for row in self.rows:
            manifest, path = materialize(self.root, row)
            with self.subTest(asset=row['name']):
                self.assertEqual(pack_payload(manifest, path.parent, mode='shiftable'),
                                 pack_payload(manifest, path.parent, mode='matching'))

    def test_changed_replay_content_recompresses_without_fixed_encoded_size(self):
        manifest, path = materialize(self.root, self.rows[0])
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            original = pack_payload(manifest, path.parent, mode='shiftable')
            with (path.parent/manifest['path']).open(newline='') as stream:
                reader = csv.DictReader(stream)
                fields, rows = reader.fieldnames, list(reader)
            # New input bytes preserve the runtime's player/frame indexing.
            for index, row in enumerate(rows):
                row['buttons'] = str((index*73+19) & 255)
                row['stick_x'] = str((index*31 & 255)-128)
            with (base/'inputs.csv').open('w', newline='') as stream:
                writer = csv.DictWriter(stream, fieldnames=fields)
                writer.writeheader()
                writer.writerows(rows)
            manifest['path'] = 'inputs.csv'
            edited = pack_payload(manifest, base, mode='shiftable')
            self.assertEqual(len(edited), len(original))
            self.assertNotEqual(edited, original)
            encoded = compress_flexible(edited)
            self.assertEqual(decompress_huffman_asset(encoded)[0], edited)
            self.assertEqual(encoded, compress_flexible(edited))
            self.assertNotEqual(len(encoded), len(compress_flexible(original)))

    def test_changed_runtime_dimensions_are_rejected_before_reading_rows(self):
        original, path = materialize(self.root, self.rows[0])
        for players, frames in ((3, 2400), (5, 2400), (4, 2399), (4, 2401)):
            manifest = copy.deepcopy(original)
            manifest.update(player_count=players, frames_per_player=frames)
            with self.subTest(players=players, frames=frames):
                with self.assertRaisesRegex(ValueError, manifest['name']+':.*playback row stride'):
                    pack_payload(manifest, path.parent, mode='shiftable')

    def test_invalid_row_addresses_and_input_fields_name_the_asset(self):
        manifest, _ = materialize(self.root, self.rows[0])
        manifest['path'] = 'inputs.csv'
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            for row, reason in (('0,1,0,0,0', 'player/frame order'),
                                ('no,0,0,0,0', 'player/frame must be integers'),
                                ('0,0,256,0,0', 'field buttons'),
                                ('0,0,0,128,0', 'field stick_x'),
                                ('0,0,0,0,-129', 'field stick_y'),
                                ('0,0,0,no,0', 'field stick_x'),
                                ('0,0,0,0,0', 'frame count mismatch')):
                (base/'inputs.csv').write_text('player,frame,buttons,stick_x,stick_y\n'+row+'\n')
                with self.subTest(row=row), self.assertRaisesRegex(ValueError, manifest['name']+':.*'+reason):
                    pack_payload(manifest, base, mode='shiftable')


if __name__ == '__main__':
    unittest.main()
