#!/usr/bin/env python3
"""Compare fixed-frame captures in a local Mupen64Plus installation.

Requires an existing X display (Xvfb works). The default checks boot and the
automatic demo; --scenario supplies deterministic controller input. Optional
AI PCM capture requires an LLE RSP. Visual equivalence alone does not establish
complete gameplay.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import png
import shutil


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def solid_capture(path):
    _, _, rows, _ = png.Reader(bytes=path.read_bytes()).asRGB8()
    first = None
    for row in rows:
        for index in range(0, len(row), 3):
            pixel = tuple(row[index:index+3])
            if first is None:
                first = pixel
            elif pixel != first:
                return False
    return True


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('baseline', type=Path)
    p.add_argument('shifted', type=Path)
    p.add_argument('--emulator-root', type=Path, required=True,
                   help='installation prefix containing usr/games and usr/lib')
    p.add_argument('--out', type=Path, required=True, help='new directory for captures and report')
    p.add_argument('--frames', default='120,600,1800')
    p.add_argument('--scenario', type=Path, help='optional deterministic controller scenario JSON')
    p.add_argument('--baseline-elf', type=Path)
    p.add_argument('--shifted-elf', type=Path)
    p.add_argument('--baseline-mempak', type=Path)
    p.add_argument('--shifted-mempak', type=Path)
    p.add_argument('--video-plugin', default='video-rice')
    p.add_argument('--rsp-plugin', default='rsp-hle')
    p.add_argument('--capture-audio', action='store_true', help='capture AI PCM with an LLE RSP; requires --scenario')
    p.add_argument('--timeout', type=int, default=180)
    a = p.parse_args()
    scenario = json.loads(a.scenario.read_text()) if a.scenario else None
    if bool(a.baseline_mempak) != bool(a.shifted_mempak) or (a.baseline_mempak and not scenario):
        p.error('save reload checks require both mempak images and --scenario')
    if a.capture_audio and not scenario:
        p.error('--capture-audio requires a controller scenario (events may be empty)')
    frames = scenario['frames'] if scenario else [int(v) for v in a.frames.split(',')]
    if not frames or frames != sorted(set(frames)) or frames[0] <= 0:
        p.error('--frames must be distinct positive increasing frame numbers')
    if not os.environ.get('DISPLAY'):
        p.error('set DISPLAY to an existing local X display')
    a.out = a.out.resolve()
    a.out.mkdir(parents=True, exist_ok=False)
    prefix = a.emulator_root.resolve()
    lib = prefix/'usr/lib/x86_64-linux-gnu'
    env = dict(os.environ, LD_LIBRARY_PATH=f'{lib}:{prefix / "usr/lib"}:'+os.environ.get('LD_LIBRARY_PATH', ''))
    plugin = a.out/'test-input.so'
    if scenario:
        subprocess.run(['cc', '-shared', '-fPIC', '-I'+str(prefix/'usr/include/mupen64plus'),
                        str(Path(__file__).with_name('runtime_test_input.c')), '-o', str(plugin)], check=True)
    audio_plugin = a.out/'test-audio.so'
    if a.capture_audio:
        subprocess.run(['cc', '-shared', '-fPIC', '-I'+str(prefix/'usr/include/mupen64plus'),
                        str(Path(__file__).with_name('runtime_test_audio.c')), '-o', str(audio_plugin)], check=True)
    if scenario:
        # Preserve one harness version for both runs even while work continues.
        for name in ('runtime_shift_driver.py', 'verify_linked_relocations.py'):
            shutil.copyfile(Path(__file__).with_name(name), a.out/name)
        scenario_path = a.out/'scenario.json'
        scenario_path.write_text(json.dumps(scenario, indent=2)+'\n')
    report = dict(frames=frames, ram_bytes=4*1024*1024, runs={},
                  scope='boot and automatic title demo; dummy audio; no gameplay input',
                  pending=['menus', 'all courses', 'multiplayer', 'audio', 'replay controls', 'save flows', 'hardware'])
    report['emulator'] = dict(core_sha256=digest(lib/'libmupen64plus.so.2'),
                             randomize_interrupt=False,
                             video_plugin=a.video_plugin, rsp_plugin=a.rsp_plugin,
                             video_sha256=digest(lib/'mupen64plus'/f'mupen64plus-{a.video_plugin}.so'),
                             rsp_sha256=digest(lib/'mupen64plus'/f'mupen64plus-{a.rsp_plugin}.so'))
    if scenario:
        report['scope'] = scenario['description']+('; AI PCM capture' if a.capture_audio else '; dummy audio')
        report['scenario'] = scenario
        report['harness_sha256'] = {name: digest(a.out/name) for name in
                                    ('runtime_shift_driver.py', 'verify_linked_relocations.py', 'test-input.so')}
        if a.capture_audio:
            report['harness_sha256']['test-audio.so'] = digest(audio_plugin)
    captures = []
    for label, rom in [('baseline', a.baseline), ('shifted', a.shifted)]:
        directory = a.out/label
        directory.mkdir()
        config = directory/'config'
        config.mkdir()
        saves = directory/'saves'
        saves.mkdir()
        command = [str(prefix/'usr/games/mupen64plus'), '--corelib', str(lib/'libmupen64plus.so.2'),
                   '--configdir', str(config), '--datadir', str(prefix/'usr/share/games/mupen64plus'),
                   '--plugindir', str(lib/'mupen64plus'), '--gfx', 'mupen64plus-'+a.video_plugin,
                   '--rsp', 'mupen64plus-'+a.rsp_plugin, '--audio', 'dummy', '--input', 'mupen64plus-input-sdl',
                   '--nospeedlimit', '--noosd', '--sshotdir', str(directory), '--testshots', a.frames,
                   '--set', 'Core[DisableExtraMem]=True', '--set', 'Core[GbCameraVideoCaptureBackend1]=null',
                   '--set', 'Core[RandomizeInterrupt]=False',
                   '--set', f'Core[SaveSRAMPath]={saves}', '--set', f'Core[SaveStatePath]={saves}',
                   str(rom.resolve())]
        if scenario:
            command = [sys.executable, str(a.out/'runtime_shift_driver.py'), str(rom.resolve()),
                       '--emulator-root', str(prefix), '--input-plugin', str(plugin), '--scenario', str(scenario_path),
                       '--out', str(directory), '--video-plugin', a.video_plugin, '--rsp-plugin', a.rsp_plugin]
            elf = a.baseline_elf if label == 'baseline' else a.shifted_elf
            if elf:
                command += ['--elf', str(elf.resolve())]
            if a.capture_audio:
                command += ['--audio-plugin', str(audio_plugin)]
            mempak = a.baseline_mempak if label == 'baseline' else a.shifted_mempak
            if mempak:
                command += ['--seed-mempak', str(mempak.resolve())]
        with (directory/'emulator.log').open('w') as log:
            try:
                subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, env=env,
                               timeout=a.timeout, check=True)
            except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as exc:
                report['failure'] = dict(run=label, error=str(exc), command=command,
                                        rom_sha256=digest(rom), log=str(directory/'emulator.log'))
                (a.out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
                raise
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
        solid_frames = [frame for frame, shot in zip(frames, screenshots) if solid_capture(shot)]
        report['runs'][label] = dict(rom=str(rom.resolve()), rom_sha256=digest(rom), screenshots=hashes,
                                    command=command, solid_frames=solid_frames)
        if scenario and mempak:
            report['runs'][label]['seed_mempak_sha256'] = digest(mempak)
        report['runs'][label]['saves'] = {path.name: digest(path) for path in sorted(saves.glob('*.mpk'))}
        observation_file = directory/'observations.json'
        if observation_file.exists():
            report['runs'][label]['observations'] = json.loads(observation_file.read_text())
        if a.capture_audio:
            from runtime_audio import inspect_audio
            report['runs'][label]['audio'] = inspect_audio(directory/'audio.bin')
        if len(solid_frames) == len(frames):
            (a.out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
            raise ValueError(f'{label}: every screenshot is blank; runtime comparison has no visual coverage')
        captures.append(hashes)
        print(f'{label}: captured {len(hashes)} frames with 4 MB RAM', flush=True)
    report['captures_identical'] = captures[0] == captures[1]
    if scenario and scenario.get('compare_mempak_data'):
        from runtime_mempak import compare_saved_data
        report['harness_sha256']['runtime_mempak.py'] = digest(Path(__file__).with_name('runtime_mempak.py'))
        hashes, report['save_data_identical'] = compare_saved_data(
            a.out/'baseline/saves', a.out/'shifted/saves')
        report['save_comparison'] = 'Only validated pak serial numbers and their ID checksums are excluded'
        for kind in ('baseline', 'shifted'):
            report['runs'][kind]['save_data_sha256'] = hashes[kind]
    if a.capture_audio:
        audio = [report['runs'][label]['audio'] for label in ('baseline', 'shifted')]
        report['audio_identical'] = all(audio[0][key] == audio[1][key]
                                        for key in ('sample_rate', 'stereo_frames', 'pcm_sha256'))
    (a.out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    if not report['captures_identical']:
        raise ValueError('fixed-frame captures differ; inspect captures and report.json')
    if report.get('save_data_identical') is False:
        raise ValueError('controller-pak game data or non-serial metadata differ; inspect report.json')
    if a.capture_audio and not report['audio_identical']:
        raise ValueError('AI PCM captures differ; inspect audio.wav files and report.json')
    print(f'Runtime captures identical: {a.out / "report.json"}')


if __name__ == '__main__':
    main()
