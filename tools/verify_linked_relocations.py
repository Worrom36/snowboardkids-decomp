#!/usr/bin/env python3
"""Check emitted ELF32 MIPS relocations against their relocated target symbols."""
import argparse
from collections import Counter
from pathlib import Path
import struct


class Elf:
    def __init__(self, path):
        self.data = Path(path).read_bytes()
        if self.data[:6] != b'\x7fELF\x01\x02':
            raise ValueError(f'{path}: expected big-endian ELF32')
        offset = self.word(32)
        stride, count, strings = struct.unpack_from('>HHH', self.data, 46)
        self.sections = [struct.unpack_from('>10I', self.data, offset+i*stride) for i in range(count)]
        self.names = [self.string(strings, s[0]) for s in self.sections]

    def word(self, offset):
        return struct.unpack_from('>I', self.data, offset)[0]

    def string(self, section, offset):
        start = self.sections[section][4] + offset
        return self.data[start:self.data.index(b'\0', start)].decode()

    def symbols(self, index):
        s = self.sections[index]
        result = []
        for offset in range(s[4], s[4]+s[5], s[9]):
            name, value, size, info, other, section = struct.unpack_from('>IIIBBH', self.data, offset)
            result.append((self.string(s[6], name), value, info & 15, section))
        return result

    def relocations(self):
        result = {}
        for section in self.sections:
            if section[1] != 9:
                continue
            target = self.sections[section[7]]
            symbols = self.symbols(section[6])
            records = []
            for pos in range(section[4], section[4]+section[5], section[9]):
                address, info = struct.unpack_from('>II', self.data, pos)
                kind = info & 255
                if kind == 0:
                    continue
                if target[1] == 8:
                    raise ValueError('relocation into uninitialized BSS')
                word = self.word(target[4] + address - target[3])
                records.append((address, kind, symbols[info >> 8], word))
            if records:
                result[self.names[section[7]]] = records
        return result


def signed16(value):
    return (value & 0x7fff) - (value & 0x8000)


def verify(base_path, shifted_path):
    base_elf, shifted_elf = Elf(base_path), Elf(shifted_path)
    def named(elf):
        return {name: value for i, section in enumerate(elf.sections) if section[1] == 2
                for name, value, kind, index in elf.symbols(i) if name}
    old_names, new_names = named(base_elf), named(shifted_elf)
    regions = []
    for prefix in ('main_TEXT', 'main_DATA', 'main_RODATA', 'main_BSS', 'buffers_BSS'):
        start, end = prefix+'_START', prefix+'_END'
        if start not in old_names or end not in old_names or start not in new_names:
            raise ValueError(f'missing expected layout symbol: {prefix}')
        content = prefix+'_CONTENT_START'
        regions.append((old_names[start], old_names[end],
                        new_names.get(content, new_names[start])-old_names.get(content, old_names[start])))
    def translate(value):
        for start, end, delta in regions:
            if start <= value < end:
                return value + delta
        raise ValueError(f'no target identity for section-relative address 0x{value:X}')
    base, shifted = base_elf.relocations(), shifted_elf.relocations()
    if not base or base.keys() != shifted.keys():
        raise ValueError('missing relocation sections; link both ELFs with --emit-relocs')
    counts = Counter()
    for section, records in base.items():
        changed = shifted[section]
        if len(records) != len(changed):
            raise ValueError(f'{section}: relocation coverage changed')
        pending = []
        last_hi = {}
        for old, new in zip(records, changed):
            address, kind, symbol, word = old
            new_address, new_kind, target, new_word = new
            label = f'{section}:0x{address:X} -> 0x{new_address:X}, {symbol[0]}'
            if (kind, symbol[0], symbol[2]) != (new_kind, target[0], target[2]):
                raise ValueError(f'{label}: relocation identity changed')
            if section == '.main' and new_address != translate(address):
                raise ValueError(f'{label}: reference location did not follow its section')
            if section == '.entry' and new_address != address:
                raise ValueError(f'{label}: boot reference location moved')
            if word != new_word:
                counts['changed_fields'] += 1
            if address != new_address:
                counts['moved_fields'] += 1
            if kind in (4, 5, 6):
                mask = 0xfc000000 if kind == 4 else 0xffff0000
                if (word & mask) != (new_word & mask):
                    raise ValueError(f'{label}: relocation changed instruction opcode/registers')
            delta = target[1] - symbol[1]
            if kind == 5:
                pending.append((old, new))
                last_hi[(symbol, (word >> 16) & 31)] = (old, new)
                continue
            if kind == 2:  # R_MIPS_32
                expected, actual = (translate(word) if symbol[2] == 3 else word + delta) & 0xffffffff, new_word
            elif kind == 4:  # R_MIPS_26: region bits come from PC.
                old_value = ((address+4) & 0xf0000000) | ((word & 0x3ffffff) << 2)
                new_value = ((new_address+4) & 0xf0000000) | ((new_word & 0x3ffffff) << 2)
                expected, actual = (translate(old_value) if symbol[2] == 3 else old_value + delta) & 0xffffffff, new_value
            elif kind == 6:  # R_MIPS_LO16, plus all pending HI16 uses of this symbol.
                expected, actual = (word + delta) & 0xffff, new_word & 0xffff
                if symbol[2] == 3:
                    previous = last_hi.get((symbol, (word >> 21) & 31))
                    if previous is None:
                        raise ValueError(f'{label}: section-relative LO16 without known HI16')
                    old_hi, moved_hi = previous
                    value = translate(((old_hi[3] & 0xffff) << 16) + signed16(word))
                    expected = value & 0xffff
                    if (moved_hi[3] & 0xffff) != ((value+0x8000) >> 16) & 0xffff:
                        raise ValueError(f'{label}: reused HI16 target mismatch')
                remaining = []
                for hi, new_hi in pending:
                    if hi[2] != symbol:
                        remaining.append((hi, new_hi))
                        continue
                    old_value = ((hi[3] & 0xffff) << 16) + signed16(word)
                    value = translate(old_value) if symbol[2] == 3 else old_value + delta
                    expected = value & 0xffff
                    if (new_hi[3] & 0xffff) != ((value+0x8000) >> 16) & 0xffff:
                        raise ValueError(f'{label}: HI16 carry/target mismatch')
                    counts['HI16'] += 1
                    if (hi[3] & 0xffff) != (new_hi[3] & 0xffff):
                        counts['changed_HI16'] += 1
                pending = remaining
            else:
                raise ValueError(f'{label}: unsupported MIPS relocation {kind}')
            if expected != actual:
                raise ValueError(f'{label}: expected 0x{expected:X}, got 0x{actual:X}')
            counts[{2: '32', 4: '26', 6: 'LO16'}[kind]] += 1
        if pending:
            raise ValueError(f'{section}: HI16 relocation without matching LO16')
    if not counts['32'] or not counts['26'] or not counts['HI16']:
        raise ValueError(f'incomplete relocation coverage: {dict(counts)}')
    if not counts['changed_fields']:
        raise ValueError('perturbation did not change any checked reference')
    return dict(counts)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('base', type=Path)
    p.add_argument('shifted', type=Path)
    a = p.parse_args()
    print('Linked relocations OK:', verify(a.base, a.shifted))


if __name__ == '__main__':
    main()
