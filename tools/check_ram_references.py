#!/usr/bin/env python3
"""Require relocation coverage for address-shaped words in linked CPU data.

Exceptions are semantic fields, not a blanket numeric-address allowlist.
Instruction literals and segmented asset fields have separate checks.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.verify_linked_relocations import Elf


def inspect(path):
    elf = Elf(path)
    symbols = {name: value for i, section in enumerate(elf.sections) if section[1] == 2
               for name, value, kind, index in elf.symbols(i) if name}
    required = ('main_DATA_START', 'main_DATA_END', 'main_RODATA_START', 'main_RODATA_END',
                'gSineTable', 'gTitleDemoReplayPlayerSnapshots', 'gMenuRenderModeResetDl',
                'gTranslucentOverlaySetupDisplayList')
    for name in required:
        if name not in symbols:
            raise ValueError(f'RAM coverage: missing expected symbol {name}')
    section = elf.sections[elf.names.index('.main')]
    relocations = elf.relocations().get('.main', [])
    if not relocations:
        raise ValueError('RAM coverage: missing main relocation records')
    covered = {address for address, kind, symbol, word in relocations if kind == 2}
    reviewed, counts, failures = [], Counter(), []
    for prefix in ('main_DATA', 'main_RODATA'):
        start, end = symbols[prefix+'_START'], symbols[prefix+'_END']
        if end <= start:
            raise ValueError(f'RAM coverage: empty {prefix}')
        for address in range(start, end, 4):
            file_offset = section[4]+address-section[3]
            value = elf.word(file_offset)
            if not 0x80000400 <= value < 0x80400000:
                continue
            counts['candidates'] += 1
            if address in covered:
                counts['relocated'] += 1
                continue
            reason = None
            if 0 <= address-symbols['gSineTable'] < 0x2000:
                reason = 'two signed 16-bit sine samples, not a pointer'
            for name in ('gMenuRenderModeResetDl', 'gTranslucentOverlaySetupDisplayList'):
                if address == symbols[name]+0x1c and value == 0x80008000 and elf.word(file_offset-4) >> 24 == 0xbb:
                    reason = 'G_TEXTURE S/T scale immediate, not a pointer'
            snapshot = address-symbols['gTitleDemoReplayPlayerSnapshots']
            fields = {0x498, 0x49c, 0x5f8, 0x5fc, *range(0x530, 0x568, 4)}
            if 0 <= snapshot < 64*0x60c and snapshot % 0x60c in fields:
                reason = ('archived replay render cache; appendViewportDisplayLists sets gRenderMatricesDirty, '
                          'then model/shadow/speed-fan renderers allocate fresh matrices and vertices before use')
            if reason is None:
                failures.append(f'0x{address:08X}: uncovered CPU word 0x{value:08X}')
            else:
                counts['reviewed'] += 1
                reviewed.append(dict(address=address, value=value, reason=reason))
    if not counts['relocated'] or not counts['reviewed']:
        raise ValueError(f'RAM coverage unexpectedly empty: {dict(counts)}')
    if failures:
        raise ValueError('RAM relocation coverage failed:\n'+'\n'.join(failures))
    return dict(counts=counts, exceptions=reviewed)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('elf', type=Path)
    p.add_argument('--out', type=Path)
    a = p.parse_args()
    report = inspect(a.elf)
    if a.out:
        a.out.write_text(json.dumps(report, indent=2)+'\n')
    print('CPU data reference coverage:', dict(report['counts']))


if __name__ == '__main__':
    main()
