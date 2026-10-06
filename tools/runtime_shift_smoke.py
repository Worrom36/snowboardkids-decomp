#!/usr/bin/env python3
"""Compare fixed-frame boot/demo captures in a local Mupen64Plus installation.

Requires an existing X display (Xvfb works). This checks 4 MB boot and visual
demo equivalence; it does not validate audio, input, saves, or hardware.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('baseline', type=Path)
    p.add_argument('shifted', type=Path)
    p.add_argument('--emulator-root', type=Path, required=True,
                   help='installation prefix containing usr/games and usr/lib')
    p.add_argument('--out', type=Path, required=True, help='new directory for captures and report')
    p.add_argument('--frames', default='120,600,1800')
    p.add_argument('--timeout', type=int, default=180)
    a = p.parse_args()
    frames = [int(v) for v in a.frames.split(',')]
    if not frames or frames != sorted(set(frames)) or frames[0] <= 0:
        p.error('--frames must be distinct positive increasing frame numbers')
    if not os.environ.get('DISPLAY'):
        p.error('set DISPLAY to an existing local X display')
    a.out = a.out.resolve()
    a.out.mkdir(parents=True, exist_ok=False)
    prefix = a.emulator_root.resolve()
    lib = prefix/'usr/lib/x86_64-linux-gnu'
    env = dict(os.environ, LD_LIBRARY_PATH=f'{lib}:{prefix / "usr/lib"}:'+os.environ.get('LD_LIBRARY_PATH', ''))
    report = dict(frames=frames, ram_bytes=4*1024*1024, runs={},
                  scope='boot and automatic title demo; dummy audio; no gameplay input',
                  pending=['menus', 'all courses', 'multiplayer', 'audio', 'replay controls', 'save flows', 'hardware'])
    captures = []
    for label, rom in [('baseline', a.baseline), ('shifted', a.shifted)]:
        directory = a.out/label
        directory.mkdir()
        config = directory/'config'
        config.mkdir()
        command = [str(prefix/'usr/games/mupen64plus'), '--corelib', str(lib/'libmupen64plus.so.2'),
                   '--configdir', str(config), '--datadir', str(prefix/'usr/share/games/mupen64plus'),
                   '--plugindir', str(lib/'mupen64plus'), '--gfx', 'mupen64plus-video-rice',
                   '--rsp', 'mupen64plus-rsp-hle', '--audio', 'dummy', '--input', 'mupen64plus-input-sdl',
                   '--nospeedlimit', '--noosd', '--sshotdir', str(directory), '--testshots', a.frames,
                   '--set', 'Core[DisableExtraMem]=True', '--set', 'Core[GbCameraVideoCaptureBackend1]=null',
                   str(rom.resolve())]
        with (directory/'emulator.log').open('w') as log:
            subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, env=env,
                           timeout=a.timeout, check=True)
        log = (directory/'emulator.log').read_text()
        if 'Initializing 2 RDRAM modules for a total of 4 MB' not in log:
            raise ValueError(f'{label}: emulator did not confirm 4 MB RAM')
        for frame in frames:
            if f'Captured screenshot for frame {frame}.' not in log:
                raise ValueError(f'{label}: missing frame {frame}')
        screenshots = sorted(directory.glob('*.png'))
        if len(screenshots) != len(frames):
            raise ValueError(f'{label}: incorrect screenshot count')
        hashes = [digest(s) for s in screenshots]
        report['runs'][label] = dict(rom=str(rom.resolve()), rom_sha256=digest(rom), screenshots=hashes,
                                    command=command)
        captures.append(hashes)
        print(f'{label}: captured {len(hashes)} frames with 4 MB RAM', flush=True)
    report['captures_identical'] = captures[0] == captures[1]
    (a.out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    if not report['captures_identical']:
        raise ValueError('fixed-frame captures differ; inspect captures and report.json')
    print(f'Boot/demo captures identical: {a.out / "report.json"}')


if __name__ == '__main__':
    main()
