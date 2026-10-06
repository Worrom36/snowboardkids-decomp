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

    # Change a real compressed payload in memory, then substitute only its
    # generated object in a temporary linker script.
    sys.path.insert(0, str(ROOT))
    import copy
    import random
    from tools.asset_bundles import materialize
    from tools.course_surface_data_common import load_yaml
    from tools.readable_assets import pack_payload
    from tools.huffman_asset import compress_flexible, decompress_huffman_asset
    rows = load_yaml(ROOT/'assets/layout/segments.yaml')['segments']
    row = next(r for r in rows if r['name'] == 'SLASH_RACE_ANIMATIONS')
    manifest, path = materialize(ROOT, row)
    original = (ROOT/'snowboardkids.z64').read_bytes()[row['rom_start']:row['rom_end']]
    for name in ('grow', 'shrink'):
        edited = copy.deepcopy(manifest)
        rng = random.Random(0x534B)
        for animation in edited['animations']:
            for frame in animation['frames']:
                for xyz in frame['offsets']:
                    for index in range(3):
                        xyz[index] = rng.randrange(-32768, 32768) if name == 'grow' else 0
                for xyz in frame['rotations']:
                    for index in range(3):
                        xyz[index] = rng.randrange(512) * 8 if name == 'grow' else 0
        payload = pack_payload(edited, path.parent)
        encoded = compress_flexible(payload, original)
        if decompress_huffman_asset(encoded)[0] != payload:
            raise ValueError(f'{name}: compressed payload did not round-trip')
        if not (len(encoded) > len(original) if name == 'grow' else len(encoded) < len(original)):
            raise ValueError(f'{name}: compressed size did not change in the expected direction')
        binary, obj = out/f'asset-{name}.bin', out/f'asset-{name}.o'
        binary.write_bytes(encoded)
        run(linker, '-r', '-b', 'binary', '-o', obj, binary)
        script = (out/'baseline.ld').read_text()
        source_object = a.build_dir + '/' + row['object']
        if script.count(source_object) != 1:
            raise ValueError(f'{name}: expected one linker input for {source_object}')
        temporary = out/f'asset-{name}.ld'
        temporary.write_text(script.replace(source_object, str(obj)))
        elf = out/f'asset-{name}.elf'
        run(linker, '-T', temporary, *extra, '--emit-relocs', '--no-check-sections', '-o', elf)
        print(f'Compressed asset {name}: {len(original)} -> {len(encoded)} bytes; {verify(baseline, elf)}', flush=True)


if __name__ == '__main__':
    main()
