#!/usr/bin/env python3
"""Audit unrelocated cached-RAM high halves in linked MIPS instructions.

Section-symbol relocations are checked separately against their target identities.
The fixed KSEG0 base/sign bit and libultra exception vectors are intentional
literals. Other address constructions must use linker symbols.
"""
import argparse
from bisect import bisect_right
from collections import Counter
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools.verify_linked_relocations import Elf, signed16


def written_register(word):
    opcode, rs, rt = word >> 26, word >> 21 & 31, word >> 16 & 31
    if opcode == 0:
        return word >> 11 & 31
    if opcode == 3 or opcode == 1 and rt in (16, 17, 18, 19):
        return 31
    if opcode in (*range(8, 16), *range(32, 40), 48, 52, 55):
        return rt
    if opcode in (16, 17, 18) and rs in (0, 1, 2):
        return rt
    return 0


def inspect(path):
    elf = Elf(path)
    entries = [entry for i, section in enumerate(elf.sections) if section[1] == 2 for entry in elf.symbols(i)]
    symbols = {name: value for name, value, kind, index in entries if name}
    for name in ('main_TEXT_START', 'main_TEXT_END'):
        if name not in symbols:
            raise ValueError(f'MIPS literal audit: missing {name}')
    start, end = symbols['main_TEXT_START'], symbols['main_TEXT_END']
    if start >= end:
        raise ValueError('MIPS literal audit: empty main text')
    functions = sorted((value, name) for name, value, kind, index in entries if kind == 2 and start <= value < end)
    if not functions:
        raise ValueError('MIPS literal audit: missing function identities')
    starts = [value for value, name in functions]
    section = elf.sections[elf.names.index('.main')]
    records = elf.relocations().get('.main', [])
    if not records:
        raise ValueError('MIPS literal audit: missing relocations')
    covered = {address for address, kind, symbol, word in records if kind == 5}
    counts, exceptions = Counter(), []
    for address in range(start, end, 4):
        word = elf.word(section[4]+address-section[3])
        if word >> 26 != 15 or not 0x8000 <= word & 0xffff < 0x8040:
            continue
        counts['ram_high_halves'] += 1
        if address in covered:
            counts['relocated'] += 1
            continue
        function_index = bisect_right(starts, address)-1
        if function_index < 0:
            raise ValueError(f'0x{address:X}: unrelocated RAM high half outside a named function')
        function = functions[function_index][1]
        if word & 0xffff != 0x8000:
            raise ValueError(f'{function}:0x{address:X}: unrelocated RAM high half 0x{word & 0xffff:04X}')
        register = word >> 16 & 31
        boundary = starts[function_index+1] if function_index+1 < len(starts) else end
        pairs = []
        for use in range(address+4, boundary, 4):
            instruction = elf.word(section[4]+use-section[3])
            opcode, source = instruction >> 26, instruction >> 21 & 31
            if source == register and opcode in (8, 9, 13, *range(32, 48), 48, 52, 55, 56, 60, 63):
                low = instruction & 0xffff if opcode == 13 else signed16(instruction)
                value = (0x80000000+low) & 0xffffffff
                if 0x80000000 < value < 0x80400000:
                    if function in ('osWritebackDCache', 'osWritebackDCacheAll', 'osInvalDCache', 'osInvalICache') and value <= 0x80004000:
                        reason = 'libultra indexed-cache traversal uses fixed KSEG0 cache-line indices'
                    elif function in ('osInitialize', '__osInitialize_common') and (
                            value in (0x80000080, 0x80000100, 0x80000180) or
                            opcode == 43 and value in (0x80000004, 0x80000008, 0x8000000c)):
                        reason = 'hardware-required TLB/general exception vector installed by libultra'
                    else:
                        raise ValueError(f'{function}:0x{use:X}: unrelocated CPU address 0x{value:X} from LUI at 0x{address:X}')
                else:
                    reason = 'fixed KSEG0 base/sign bit or signed integer bound'
                pairs.append(dict(address=use, value=value, reason=reason))
            if written_register(instruction) == register:
                break
        counts['fixed_base_literals'] += 1
        exceptions.append(dict(address=address, function=function, reason='fixed KSEG0 base/sign-bit literal', uses=pairs))
    if not counts['relocated'] or not counts['fixed_base_literals']:
        raise ValueError(f'MIPS literal audit unexpectedly empty: {dict(counts)}')
    return dict(counts=counts, exceptions=exceptions)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('elf', type=Path)
    args = parser.parse_args()
    print(dict(inspect(args.elf)['counts']))
