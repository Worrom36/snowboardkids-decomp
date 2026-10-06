"""Guard OSViMode type/padding words which resemble segment-2 pointers."""
from tools.verify_linked_relocations import Elf


FIELDS = (('gSchedulerViModes', 0xA0), ('osViModeNtscLan1', 0))


def inspect(path):
    elf = Elf(path)
    symbols = {name: (value, owner) for i, section in enumerate(elf.sections) if section[1] == 2
               for name, value, kind, owner in elf.symbols(i) if name}
    relocations = {address for records in elf.relocations().values()
                   for address, kind, target, word in records}
    for name, offset in FIELDS:
        if name not in symbols:
            raise ValueError(f'{name}: missing video-mode scalar owner')
        value, index = symbols[name]
        if not 0 < index < len(elf.sections):
            raise ValueError(f'{name}: video-mode scalar owner is not section-defined')
        section = elf.sections[index]
        address = value+offset
        position = address-section[3]
        if section[1] == 8 or not 0 <= position <= section[5]-4:
            raise ValueError(f'{name}+0x{offset:X}: video-mode scalar exceeds its section')
        if address in relocations or elf.word(section[4]+position) != 0x02000000:
            raise ValueError(f'{name}+0x{offset:X}: OSViMode type/padding is a scalar, not a graphics pointer')
    return len(FIELDS)
