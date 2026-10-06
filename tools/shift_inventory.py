#!/usr/bin/env python3
"""Inventory address candidates and ELF relocations, including ignored inputs.

Numeric candidates require semantic review; this report is not a pointer oracle.
"""
import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess

HEX = re.compile(r'\b0x[0-9a-fA-F]{5,8}\b')


def category(value):
    if 0xA4000000 <= value < 0xC0000000:
        return 'hardware-or-uncached-review'
    if 0x80000000 <= value < 0x80800000:
        return 'cpu-ram-candidate'
    if 0x01000000 <= value < 0x10000000:
        return 'segmented-candidate'
    if value < 0x800000:
        return 'rom-physical-or-relative-candidate'
    return 'constant-or-address-review'


def inventory(root, build):
    sources = []
    for directory in ('src', 'include', 'asm', 'linker_scripts'):
        for path in sorted((root / directory).rglob('*')):
            if path.suffix not in ('.c', '.h', '.s', '.ld'):
                continue
            for number, line in enumerate(path.read_text(errors='replace').splitlines(), 1):
                # Strip generated disassembly addresses and instruction bytes.
                line = re.sub(r'/\*.*?\*/', '', line)
                for match in HEX.finditer(line):
                    sources.append(dict(path=str(path.relative_to(root)), line=number,
                                        token=match[0], classification=category(int(match[0], 16))))
    reader = next((shutil.which(p + 'readelf') for p in
                   ('mips-linux-gnu-', 'mips64-linux-gnu-', 'mips64-elf-') if shutil.which(p + 'readelf')), None)
    if not reader:
        raise ValueError('MIPS readelf is required for object relocation inventory')
    objects = []
    for path in sorted(build.rglob('*.o')):
        result = subprocess.run([reader, '-rW', str(path)], check=True, text=True, capture_output=True)
        records = [line.strip() for line in result.stdout.splitlines() if 'R_MIPS_' in line]
        objects.append(dict(path=str(path.relative_to(root)), relocations=records))
    if not sources or not objects or not any(o['relocations'] for o in objects):
        raise ValueError('inventory requires extracted sources and compiled relocation-bearing objects')
    return dict(schema_version=1, numeric_candidates_require_review=True, sources=sources, objects=objects)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    p.add_argument('--build', default='build')
    p.add_argument('--out', type=Path, required=True)
    a = p.parse_args()
    report = inventory(a.root.resolve(), a.root.resolve() / a.build)
    a.out.parent.mkdir(parents=True, exist_ok=True)
    a.out.write_text(json.dumps(report, indent=2) + '\n')
    print(f"Inventoried {len(report['sources'])} numeric candidates and {len(report['objects'])} objects")


if __name__ == '__main__':
    main()
