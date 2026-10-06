#!/usr/bin/env python3
"""Link temporary perturbations from existing matching objects; never edit assets."""
import argparse
from pathlib import Path
import re
import shutil
import subprocess
import sys

from verify_linked_relocations import verify

ROOT = Path(__file__).resolve().parents[1]


def run(*args, success=True):
    result = subprocess.run([str(a) for a in args], cwd=ROOT, text=True, capture_output=True)
    if success and result.returncode:
        raise ValueError(result.stdout + result.stderr)
    return result


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build-dir', default='build')
    a = p.parse_args()
    build = ROOT / a.build_dir
    out = build / 'shift-tests'
    out.mkdir(parents=True, exist_ok=True)
    linker = next((shutil.which(x+'ld') for x in ('mips-linux-gnu-', 'mips64-linux-gnu-', 'mips64-elf-') if shutil.which(x+'ld')), None)
    if not linker:
        p.error('MIPS linker not found')
    makefile = (ROOT / 'Makefile').read_text().replace('\\\n', ' ')
    scripts = re.search(r'^LINKER_SCRIPTS\s*=\s*(.*)$', makefile, re.M)[1].split()
    extra = [x for path in scripts for x in ('-T', path)]

    def link(name, grow=None, growth=0, after=None):
        script, elf = out / f'{name}.ld', out / f'{name}.elf'
        args = [sys.executable, ROOT/'tools/make_shiftable_linker.py', ROOT/'snowboardkids.ld', script,
                '--layout', '--build-dir', a.build_dir]
        if grow:
            args += ['--grow', grow, '--growth', str(growth)]
        run(*args)
        if after:
            run(sys.executable, ROOT/'tools/make_shiftable_linker.py', script, script,
                '--after', after, '--pad-size', str(growth))
        result = run(linker, '-T', script, *extra, '--emit-relocs', '--no-check-sections',
                     '-Map', out/f'{name}.map', '-o', elf, success=False)
        return elf, result

    baseline, result = link('baseline')
    if result.returncode:
        raise ValueError(result.stderr)
    for section in ('text', 'data', 'bss'):
        for size in (16, 0x800):
            elf, result = link(f'{section}-{size:x}', section, size)
            if result.returncode:
                raise ValueError(result.stderr)
            counts = verify(baseline, elf)
            print(f'{section} +0x{size:X}: {counts}', flush=True)
    _, result = link('ram-exhaustion', 'bss', 0x2000)
    if result.returncode == 0 or 'exceeds original 4 MB RAM' not in result.stderr:
        raise ValueError('RAM exhaustion did not fail with the required diagnostic: '+result.stderr)
    print('RAM exhaustion correctly rejected', flush=True)
    for boundary in ('main', 'SLASH_RACE_ANIMATIONS', '_60F1A0'):
        for size in (16, 0x10000):
            elf, result = link(f'rom-{boundary}-{size:x}', growth=size, after=boundary)
            if result.returncode:
                raise ValueError(result.stderr)
            print(f'ROM after {boundary} +0x{size:X}: {verify(baseline, elf)}', flush=True)


if __name__ == '__main__':
    main()
