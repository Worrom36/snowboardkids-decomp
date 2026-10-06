#!/usr/bin/env python3
"""Prepare temporary ROMs that expose the audio DMA pool exhaustion branch.

This is diagnostic instrumentation, not a shipping code change or audio fix.
It replaces the exhausted-pool fallback call with a persistent failure marker.
The successful paths remain untouched; either a failed assertion or a stall is
a failed run. Modified checksum timing means results describe these probe ROMs.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools.n64crc import calculate_crcs
from tools.verify_linked_relocations import Elf, signed16


def prepare(rom_path, elf_path, output):
    elf = Elf(elf_path)
    symbols = {name: value for i, section in enumerate(elf.sections) if section[1] == 2
               for name, value, kind, owner in elf.symbols(i) if name}
    for name in ('audioDmaCallback', 'osVirtualToPhysical', 'gAudioDmaState', 'main_ROM_START', 'main_VRAM'):
        if name not in symbols:
            raise ValueError(f'{elf_path}: missing diagnostic symbol {name}')
    data = bytearray(rom_path.read_bytes())
    start = symbols['audioDmaCallback']-symbols['main_VRAM']+symbols['main_ROM_START']
    if data[:4] != bytes.fromhex('80371240') or len(data) < 0x101000 or not 0 <= start < len(data)-0xb0:
        raise ValueError(f'{rom_path}: invalid diagnostic ROM or function extent')
    word = lambda offset: struct.unpack_from('>I', data, start+offset)[0]
    if (word(4) & 0xffff0000, word(8) & 0xffff0000) != (0x3c080000, 0x25080000):
        raise ValueError('audioDmaCallback: expected t0 state-address construction')
    if ((word(4) & 0xffff) << 16)+signed16(word(8)) != symbols['gAudioDmaState']:
        raise ValueError('audioDmaCallback: t0 does not address gAudioDmaState')
    call = (3 << 26) | ((symbols['osVirtualToPhysical'] >> 2) & 0x3ffffff)
    expected = {0x8c: 0x8d100008, 0x94: 0x16000005, 0x98: 0x02002025,
                0x9c: call, 0xa0: 0x02202025}
    for offset, instruction in expected.items():
        if word(offset) != instruction:
            raise ValueError(f'audioDmaCallback+0x{offset:X}: exhausted-pool branch encoding changed')
    original = hashlib.sha256(data).hexdigest()
    # sb zero, 0(t0): initialized is a byte which normally remains one.
    struct.pack_into('>I', data, start+0x9c, 0xa1000000)
    struct.pack_into('>II', data, 0x10, *calculate_crcs(data))
    output.write_bytes(data)
    return dict(input_sha256=original, output_sha256=hashlib.sha256(data).hexdigest(),
                patch_rom_offset=start+0x9c, old_word=call, new_word=0xa1000000)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for kind in ('baseline', 'shifted'):
        parser.add_argument('--'+kind+'-rom', type=Path, required=True)
        parser.add_argument('--'+kind+'-elf', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True, help='new temporary directory')
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=False)
    records = {kind: prepare(getattr(args, kind+'_rom'), getattr(args, kind+'_elf'), args.out/(kind+'.z64'))
               for kind in ('baseline', 'shifted')}
    (args.out/'patches.json').write_text(json.dumps(records, indent=2)+'\n')
    root = Path(__file__).resolve().parents[1]
    scenario = json.loads((root/'config/shift-tests/boot-audio.json').read_text())
    scenario['description'] = 'Diagnostic audio DMA pool guard; assert its failure marker every VI after startup'
    scenario['observe'] = [dict(symbol='gAudioDmaState', width=8, expected=1,
                                frames=list(range(120, scenario['stop_frame'])))]
    (args.out/'scenario.json').write_text(json.dumps(scenario, indent=2)+'\n')
    # A missing free list and active list force the guarded path on the next miss.
    scenario['writes'] = [dict(frame=600, symbol='gAudioDmaState', offset=offset, width=32, value=0)
                          for offset in (4, 8)]
    scenario['description'] = 'Negative control: force DMA pool exhaustion; runtime assertions must fail'
    (args.out/'forced-exhaustion.json').write_text(json.dumps(scenario, indent=2)+'\n')


if __name__ == '__main__':
    main()
