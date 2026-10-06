"""Reject assets which cannot fit their existing runtime allocation contracts."""
from functools import lru_cache
from pathlib import Path
import re


@lru_cache(maxsize=1)
def runtime_limits():
    path = Path(__file__).resolve().parents[1]/'include/game/engine/runtime_memory.h'
    text = path.read_text()
    result = {}
    for name in ('RELOCATABLE_HEAP_SIZE', 'SOUND_HEAP_SIZE', 'MUSIC_SEQUENCE_BUFFER_SIZE', 'SOUND_BANK_WAVE_COUNT'):
        match = re.search(r'^#define\s+'+name+r'\s+(0x[\da-fA-F]+|\d+)\s*$', text, re.M)
        if not match or int(match[1], 0) <= 0:
            raise ValueError(f'{path.name}: missing positive literal allocation limit {name}')
        result[name] = int(match[1], 0)
    if result['SOUND_HEAP_SIZE']+result['MUSIC_SEQUENCE_BUFFER_SIZE'] >= result['RELOCATABLE_HEAP_SIZE']:
        raise ValueError('audio heap and music buffer exhaust the relocatable heap before other assets load')
    return result


def validate_wave_count(name, count, *, limits=None):
    limits = runtime_limits() if limits is None else limits
    expected = limits['SOUND_BANK_WAVE_COUNT']
    if count != expected:
        raise ValueError(f'{name}: wave table has {count} slots, expected SOUND_BANK_WAVE_COUNT {expected}; '
                         'changing logical instrument slots requires matching music/SFX and CPU tuning-table updates')


def validate_runtime_size(name, decoder, decoded_size, encoded_size=None, *, limits=None):
    """Check a necessary allocation bound, not all live scene allocations.

    Samples stream from ROM. Compressed assets allocate decoded output and the
    complete encoded staging buffer together. The allocator rounds each to 16.
    Music DMA targets a separately reserved, fixed-size buffer.
    """
    if decoder == 'sample_bank':
        return
    limits = runtime_limits() if limits is None else limits
    if decoded_size < 0 or encoded_size is not None and encoded_size < 0:
        raise ValueError(f'{name}: negative runtime asset size')
    if decoder == 'music_sequence' and decoded_size > limits['MUSIC_SEQUENCE_BUFFER_SIZE']:
        raise ValueError(f"{name}: music sequence size 0x{decoded_size:X} exceeds "
                         f"MUSIC_SEQUENCE_BUFFER_SIZE 0x{limits['MUSIC_SEQUENCE_BUFFER_SIZE']:X}")
    footprint = (decoded_size+15) & ~15
    if encoded_size is not None:
        footprint += (encoded_size+15) & ~15
    if footprint > limits['RELOCATABLE_HEAP_SIZE']:
        raise ValueError(f'{name}: decoded asset and staging need 0x{footprint:X} heap bytes; '
                         f"RELOCATABLE_HEAP_SIZE is 0x{limits['RELOCATABLE_HEAP_SIZE']:X}")
