"""Distinguish executable F3DEX words from embedded data by declared identity."""
import re


def data_offsets(ranges, size, name):
    """Validate extraction-only byte ranges; never infer pointers from data."""
    result = set()
    for record in ranges:
        if not isinstance(record, dict) or set(record) != {'start', 'end', 'reason'}:
            raise ValueError(f'{name}: graphics data range requires start, end, and reason')
        start, end = record['start'], record['end']
        if (type(start) is not int or type(end) is not int or
                start < 0 or end <= start or end > size or start % 8 or end % 8):
            raise ValueError(f'{name}: invalid graphics data range {start!r}..{end!r}')
        if not isinstance(record['reason'], str) or not record['reason'].strip():
            raise ValueError(f'{name}: graphics data range requires a reason')
        offsets = set(range(start, end, 8))
        if result & offsets:
            raise ValueError(f'{name}: overlapping graphics data ranges')
        result.update(offsets)
    return result


def word_kinds(source, identities, spec):
    """Read movable kind annotations, falling back to original extraction IDs."""
    name = spec['name']
    original_data = data_offsets(spec.get('data_ranges', []), spec['end']-spec['start'], name)
    body = source[source.index('= {')+3:source.rindex('}')]
    rows = re.findall(r'\{\{\s*0x[\da-fA-F]{8},\s*0x[\da-fA-F]{8}\s*\}\}\s*,?([^\n]*)', body)
    if len(rows) != len(identities):
        raise ValueError(f'{name}: graphics kind coverage is incomplete')
    result = []
    for identity, row in zip(identities, rows):
        annotations = re.findall(r'kind:([^\s*]+)', row)
        if len(annotations) > 1 or annotations and annotations[0] not in ('command', 'data'):
            raise ValueError(f'{name}/{identity}: expected one graphics kind:command or kind:data')
        original = re.fullmatch(r'command_([0-9A-Fa-f]{6})', identity)
        default = 'data' if original and int(original[1], 16) in original_data else 'command'
        result.append(annotations[0] if annotations else default)
    return result


def stream_kinds(stream):
    kinds = stream.get('kinds', ['command'] * len(stream['ids']))
    if len(kinds) != len(stream['ids']) or any(k not in ('command', 'data') for k in kinds):
        raise ValueError(f"{stream['name']}: invalid graphics kind coverage")
    return kinds


def validate_command(word0, name):
    # F3DEX commands with implemented pointer operands or no address operand.
    # Other microcode/DMA commands need a typed binding before being supported.
    opcode = word0 >> 24
    supported = {0, 4, 6, *range(0xb1, 0xbc), 0xbd, 0xbe, 0xbf, 0xc0,
                 *range(0xe4, 0xf1), *range(0xf2, 0xfe)}
    if opcode not in supported:
        raise ValueError(f'{name}: unsupported graphics command 0x{opcode:02X}; '
                         'address-bearing commands require a typed relocation, embedded bytes require kind:data')
