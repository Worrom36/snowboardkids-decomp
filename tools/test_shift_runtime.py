from pathlib import Path
import struct
import tempfile
import unittest
import wave

import png
from tools.runtime_audio import inspect_audio
from tools.runtime_shift_smoke import solid_capture


class RuntimeCaptureTests(unittest.TestCase):
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
