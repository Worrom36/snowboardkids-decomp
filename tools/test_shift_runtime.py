from pathlib import Path
import struct
import tempfile
import unittest
import wave

import png
from tools.runtime_audio import inspect_audio
from tools.runtime_shift_smoke import solid_capture
from tools.runtime_shift_driver import observation_schedule, verify_observation_coverage


class RuntimeCaptureTests(unittest.TestCase):
    def test_memory_assertions_run_without_a_screenshot_at_their_frame(self):
        scenario = dict(stop_frame=100, frames=[50], observe=[dict(symbol='state', frames=[30])])
        self.assertEqual(observation_schedule(scenario), {30})
        for frames in ([], [0], [100], ['30']):
            scenario['observe'][0]['frames'] = frames
            with self.subTest(frames=frames), self.assertRaises(ValueError):
                observation_schedule(scenario)

    def test_missing_late_memory_checks_cannot_pass_with_earlier_screenshots(self):
        scenario = dict(stop_frame=100, frames=[10], observe=[dict(symbol='state', frames=[90])])
        with self.assertRaisesRegex(ValueError, 'missing runtime observation frames'):
            verify_observation_coverage(scenario, [dict(frame=10, values={})])
        records = [dict(frame=10, values={}), dict(frame=90, values={})]
        with self.assertRaisesRegex(ValueError, 'missing memory observation state'):
            verify_observation_coverage(scenario, records)
        records[1]['values']['state'] = 1
        verify_observation_coverage(scenario, records)

    def test_blank_renderer_cannot_supply_visual_coverage(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'capture.png'
            for pixels, expected in [([0]*6, True), ([0, 0, 0, 255, 0, 0], False)]:
                with path.open('wb') as output:
                    png.Writer(2, 1, greyscale=False).write(output, [pixels])
                self.assertEqual(solid_capture(path), expected)

    def test_pcm_capture_exports_stereo_samples_in_wav_byte_order(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'audio.bin'
            packet = struct.pack('>2h', 1, -2)*512
            path.write_bytes((struct.pack('>II', 32000, len(packet))+packet)*2)
            report = inspect_audio(path)
            self.assertEqual(report['stereo_frames'], 1024)
            self.assertEqual(report['nonzero_samples'], 2048)
            with wave.open(report['wav'], 'rb') as output:
                self.assertEqual((output.getnchannels(), output.getsampwidth(), output.getframerate()), (2, 2, 32000))
                self.assertEqual(output.readframes(1), bytes.fromhex('0100feff'))

    def test_empty_silent_and_truncated_audio_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'audio.bin'
            for data, error in [(b'', 'insufficient'), (b'\x01', 'truncated'),
                                ((struct.pack('>II', 32000, 2048)+bytes(2048))*2, 'non-silent'),
                                (struct.pack('>II', 32000, 2048)+bytes(16), 'truncated')]:
                path.write_bytes(data)
                with self.subTest(error=error), self.assertRaisesRegex(ValueError, error):
                    inspect_audio(path)


if __name__ == '__main__':
    unittest.main()
