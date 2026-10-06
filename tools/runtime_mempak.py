"""Compare controller-pak data while retaining all non-serial metadata.

The N64 pak ID occupies four 32-byte blocks on page zero. The first 24 bytes
are the serial; the last four are its checksums. Separate emulator runs format
different serials. Allocation tables, directories, files, reserved bytes, and
the device/bank/version fields must still compare exactly.
"""
import hashlib
from pathlib import Path
import struct


def stable_digest(path):
    data = bytearray(Path(path).read_bytes())
    if len(data) not in (0x8000, 0x10000, 0x18000, 0x20000):
        raise ValueError(f'{path}: expected one to four 32 KB controller paks')
    for start in range(0, len(data), 0x8000):
        identity = bytes(data[start+0x20:start+0x40])
        total = sum(struct.unpack('>14H', identity[:28])) & 0xffff
        if struct.unpack('>2H', identity[28:]) != (total, (0xfff2-total) & 0xffff):
            raise ValueError(f'{path}: controller {start//0x8000+1} has invalid pak ID checksums')
        for offset in (0x20, 0x60, 0x80, 0xc0):
            position = start+offset
            if data[position:position+32] != identity:
                raise ValueError(f'{path}: controller {start//0x8000+1} has inconsistent pak ID backups')
            data[position:position+24] = bytes(24)
            data[position+28:position+32] = bytes(4)
    return hashlib.sha256(data).hexdigest()


def compare_saved_data(baseline, shifted):
    baseline, shifted = Path(baseline), Path(shifted)
    names = {path.name for path in baseline.glob('*.mpk')}
    if not names or names != {path.name for path in shifted.glob('*.mpk')}:
        raise ValueError('save comparison requires matching nonempty controller-pak file sets')
    hashes = {kind: {name: stable_digest(directory/name) for name in sorted(names)}
              for kind, directory in (('baseline', baseline), ('shifted', shifted))}
    return hashes, hashes['baseline'] == hashes['shifted']
