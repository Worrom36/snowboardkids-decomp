"""Recognize literal ROM operands without classifying unrelated integers as pointers.

This is a source guard for known address-consuming APIs, not general C dataflow
analysis. Linker relocation checks independently cover symbol-based operands.
"""
import re

ROM_ARGUMENTS = {
    'dmaReadRom': (0,),
    'loadCompressedRomAsset': (0, 1),
    'loadRawRomAsset': (0, 1),
    'osPiStartDma': (3,),
    'osPiRawStartDma': (1,),
    '__osPiRawStartDma': (1,),
}
INTEGER = re.compile(r'\b(?:0[xX][\da-fA-F]+|\d+)[uUlL]*\b')
CAST = re.compile(r'\(\s*(?:const\s+)?(?:void|char|u8|s8|u16|s16|u32|s32|uintptr_t|unsigned(?:\s+int)?|int)\s*\**\s*\)')


def call_arguments(text, start):
    """Return argument source spans, respecting nested calls and comma groups."""
    depth, begin, args = 1, start, []
    for position in range(start, len(text)):
        char = text[position]
        if char in '([{':
            depth += 1
        elif char in ')]}':
            depth -= 1
            if not depth:
                args.append((begin, position))
                return args
        elif char == ',' and depth == 1:
            args.append((begin, position))
            begin = position+1
    raise ValueError('unterminated ROM API argument list')


def literal_operands(text, low, high):
    """Yield (source offset, literal, API) for constant ROM address arguments.

    A symbol plus a numeric interior addend is valid. Scalar sizes, flags, and
    controller data never become references merely by falling in the ROM range.
    """
    text = re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|/\*.*?\*/|//[^\n]*',
                  lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text, flags=re.S)
    pattern = r'\b('+'|'.join(ROM_ARGUMENTS)+r')\s*\('
    for call in re.finditer(pattern, text):
        args = call_arguments(text, call.end())
        for index in ROM_ARGUMENTS[call[1]]:
            if index >= len(args):
                raise ValueError(f'{call[1]}: missing ROM argument {index}')
            begin, end = args[index]
            expression = text[begin:end]
            without_casts = CAST.sub(lambda m: ' '*len(m[0]), expression)
            remainder = INTEGER.sub('', without_casts)
            if re.search(r'[A-Za-z_]', remainder):
                continue
            for match in INTEGER.finditer(without_casts):
                token = match[0]
                digits = token.rstrip('uUlL')
                value = int(digits, 16 if digits.lower().startswith('0x') else 10)
                if low <= value < high:
                    yield begin+match.start(), token, call[1]
