"""Validate canonical AI DMA captures and export their PCM for listening."""
import hashlib
from pathlib import Path
import struct
import wave


def inspect_audio(path):
    data = Path(path).read_bytes()
    position, packets, rates = 0, 0, set()
    pcm = bytearray()
    while position < len(data):
        if position+8 > len(data):
            raise ValueError('truncated AI capture header')
        rate, size = struct.unpack_from('>II', data, position)
        position += 8
        if not 1000 <= rate <= 192000 or not size or size % 8 or position+size > len(data):
            raise ValueError('invalid AI sample rate, length, or truncated PCM packet')
        rates.add(rate)
        pcm.extend(data[position:position+size])
        position += size
        packets += 1
    nonzero = sum(sample != 0 for sample, in struct.iter_unpack('>h', pcm))
    if packets < 2 or nonzero < 1000:
        raise ValueError('AI capture has insufficient non-silent audio; runtime audio coverage is empty')
    if len(rates) != 1:
        raise ValueError(f'AI capture changes sample rate; separate WAV segments are required: {sorted(rates)}')
    rate, = rates
    wav_path = Path(path).with_suffix('.wav')
    little_endian = bytearray(len(pcm))
    little_endian[0::2], little_endian[1::2] = pcm[1::2], pcm[0::2]
    with wave.open(str(wav_path), 'wb') as output:
        output.setnchannels(2)
        output.setsampwidth(2)
        output.setframerate(rate)
        output.writeframes(little_endian)
    return dict(sample_rate=rate, stereo_frames=len(pcm)//4, packets=packets,
                nonzero_samples=nonzero, pcm_sha256=hashlib.sha256(pcm).hexdigest(), wav=str(wav_path))
