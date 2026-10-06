#!/usr/bin/env python3
"""Run one deterministic controller scenario through the Mupen64Plus frontend API."""
import argparse
import array
import ctypes as C
import json
import shutil
from pathlib import Path
import sys


def observation_schedule(scenario):
    """Every declared memory assertion must actually run, including off-capture frames."""
    stop = scenario['stop_frame']
    scheduled = set(scenario.get('observation_frames', []))
    for observation in scenario.get('observe', []):
        if 'frames' not in observation:
            continue
        frames = observation['frames']
        if not isinstance(frames, list) or not frames:
            raise ValueError(f"{observation['symbol']}: memory observation has an empty frame schedule")
        if any(type(frame) is not int for frame in frames):
            raise ValueError(f"{observation['symbol']}: memory observation frames must be integers")
        scheduled.update(frames)
    if any(type(frame) is not int or not 0 < frame < stop for frame in scheduled):
        raise ValueError('memory observation frame lies outside the scenario')
    return scheduled


def verify_observation_coverage(scenario, observations):
    scheduled = set(scenario['frames']) | observation_schedule(scenario)
    recorded = {record['frame']: record['values'] for record in observations}
    if len(recorded) != len(observations):
        raise ValueError('duplicate runtime observation frame')
    missing = scheduled-recorded.keys()
    if missing:
        raise ValueError(f'missing runtime observation frames: {sorted(missing)}')
    for spec in scenario.get('observe', []):
        label = spec.get('label', spec['symbol'])
        for frame in spec.get('frames', scheduled):
            if label not in recorded[frame]:
                raise ValueError(f'frame {frame}: missing memory observation {label}')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('rom', type=Path)
    p.add_argument('--emulator-root', type=Path, required=True)
    p.add_argument('--input-plugin', type=Path, required=True)
    p.add_argument('--scenario', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--elf', type=Path, help='linked symbols for scenario memory observations')
    p.add_argument('--video-plugin', default='video-rice')
    p.add_argument('--rsp-plugin', default='rsp-hle')
    p.add_argument('--audio-plugin', type=Path)
    p.add_argument('--seed-mempak', type=Path, help='copy a four-controller pak image into isolated saves')
    a = p.parse_args()
    scenario = json.loads(a.scenario.read_text())
    frames, stop = set(scenario['frames']), scenario['stop_frame']
    observation_frames = observation_schedule(scenario)
    if not frames or min(frames) < 1 or max(frames) >= stop or not 1 <= scenario['controllers'] <= 4:
        p.error('invalid scenario frame or controller counts')
    events, occupied = {}, set()
    for event in scenario['events']:
        frame, player, duration = event['frame'], event['player'], event['duration']
        if not 0 <= player < scenario['controllers'] or frame <= 0 or duration <= 0 or frame+duration >= stop:
            p.error('invalid controller event bounds')
        span = {(i, player) for i in range(frame, frame+duration+1)}
        if occupied & span:
            p.error('overlapping controller events')
        occupied |= span
        events.setdefault(frame, []).append((player, event['buttons']))
        events.setdefault(frame+duration, []).append((player, 0))
    prefix, out = a.emulator_root.resolve(), a.out.resolve()
    lib = prefix/'usr/lib/x86_64-linux-gnu'
    for directory in (out, out/'config', out/'saves'):
        directory.mkdir(parents=True, exist_ok=True)
    core = C.CDLL(str(lib/'libmupen64plus.so.2'), mode=C.RTLD_GLOBAL)
    Debug = C.CFUNCTYPE(None, C.c_void_p, C.c_int, C.c_char_p)
    State = C.CFUNCTYPE(None, C.c_void_p, C.c_int, C.c_int)
    debug = Debug(lambda context, level, message: print(message.decode(), flush=True) if level <= 3 else None)
    state = State(lambda *args: None)
    core.CoreStartup.argtypes = [C.c_int, C.c_char_p, C.c_char_p, C.c_void_p, Debug, C.c_void_p, State]
    core.CoreAttachPlugin.argtypes = [C.c_int, C.c_void_p]
    core.CoreDoCommand.argtypes = [C.c_int, C.c_int, C.c_void_p]
    core.ConfigOpenSection.argtypes = [C.c_char_p, C.POINTER(C.c_void_p)]
    core.ConfigSetParameter.argtypes = [C.c_void_p, C.c_char_p, C.c_int, C.c_void_p]
    observations, observed_symbols = [], {}
    writes = {}
    if scenario.get('observe') or scenario.get('dump_ram') or scenario.get('writes'):
        if not a.elf:
            p.error('scenario memory observations require --elf')
        capabilities = C.c_int()
        core.PluginGetVersion.argtypes = [C.c_void_p]*5
        if core.PluginGetVersion(None, None, None, None, C.byref(capabilities)) or not capabilities.value & 2:
            p.error('memory observations require a debugger-enabled Mupen64Plus core')
        from verify_linked_relocations import Elf
        elf = Elf(a.elf)
        symbols = {name: value for i, section in enumerate(elf.sections) if section[1] == 2
                   for name, value, kind, index in elf.symbols(i) if name}
        for observation in scenario.get('observe', []):
            name, width = observation['symbol'], observation['width']
            if name not in symbols or width not in (8, 16, 32):
                p.error(f'invalid memory observation: {observation}')
            label = observation.get('label', name)
            if label in observed_symbols:
                p.error(f'duplicate observation label: {label}')
            observed_symbols[label] = (symbols[name], width, observation)
        for width, result in [(8, C.c_uint8), (16, C.c_uint16), (32, C.c_uint32)]:
            reader = getattr(core, f'DebugMemRead{width}')
            reader.argtypes, reader.restype = [C.c_uint], result
            writer = getattr(core, f'DebugMemWrite{width}')
            writer.argtypes, writer.restype = [C.c_uint, result], None
        for write in scenario.get('writes', []):
            name, width, value, index = (write[key] for key in ('symbol', 'width', 'value', 'frame'))
            if name not in symbols or width not in (8, 16, 32) or not 0 <= value < 1 << width or not 0 < index < stop:
                p.error(f'invalid fixture memory write: {write}')
            address = symbols[name]+write.get('offset', 0)
            if not 0x80000000 <= address <= 0x80400000-width//8 or address % (width//8):
                p.error(f'{name}: fixture write exceeds aligned 4 MB cached RAM')
            writes.setdefault(index, []).append((address, width, value, name))
        core.DebugMemGetPointer.argtypes, core.DebugMemGetPointer.restype = [C.c_int], C.c_void_p

    def check(code):
        if code:
            raise ValueError(f'Mupen64Plus API error {code}')

    check(core.CoreStartup(0x020001, str(out/'config').encode(), str(prefix/'usr/share/games/mupen64plus').encode(),
                           None, debug, None, state))
    section = C.c_void_p()
    check(core.ConfigOpenSection(b'Core', C.byref(section)))
    check(core.ConfigSetParameter(section, b'DisableExtraMem', 3, C.byref(C.c_int(1))))
    core.ConfigGetParameterType.argtypes = [C.c_void_p, C.c_char_p, C.POINTER(C.c_int)]
    parameter_type = C.c_int()
    check(core.ConfigGetParameterType(section, b'RandomizeInterrupt', C.byref(parameter_type)))
    if parameter_type.value != 3:
        raise ValueError('core does not expose deterministic PI/SI interrupt timing')
    check(core.ConfigSetParameter(section, b'RandomizeInterrupt', 3, C.byref(C.c_int(0))))
    check(core.ConfigSetParameter(section, b'SaveFilenameFormat', 1, C.byref(C.c_int(0))))
    for key, value in [('GbCameraVideoCaptureBackend1', 'null'), ('ScreenshotPath', out),
                       ('SaveSRAMPath', out/'saves'), ('SaveStatePath', out/'saves')]:
        check(core.ConfigSetParameter(section, key.encode(), 4, C.c_char_p(str(value).encode())))
    rom_bytes = a.rom.read_bytes()
    if a.seed_mempak:
        if a.seed_mempak.stat().st_size != 4*32768:
            raise ValueError('seed mempak must contain four 32 KB controller images')
        name = rom_bytes[0x20:0x34].decode('ascii').strip()
        if not name or any(c in name for c in ':<>"/\\|?*'):
            raise ValueError('unsupported ROM header name for isolated save fixture')
        shutil.copyfile(a.seed_mempak, out/'saves'/(name+'.mpk'))
    rom = C.create_string_buffer(rom_bytes)
    check(core.CoreDoCommand(1, len(rom)-1, rom))
    plugins = []
    # The core requires graphics, audio, input, then RSP: RSP initialization
    # captures the other plugins' callbacks.
    for kind, name in [(2, a.video_plugin), (3, 'test-audio' if a.audio_plugin else None), (4, 'test-input'), (1, a.rsp_plugin)]:
        if name is None:
            check(core.CoreAttachPlugin(kind, None))
            continue
        path = a.input_plugin.resolve() if kind == 4 else lib/'mupen64plus'/f'mupen64plus-{name}.so'
        if kind == 3:
            path = a.audio_plugin.resolve()
        plugin = C.CDLL(str(path))
        plugin.PluginStartup.argtypes = [C.c_void_p, C.c_void_p, Debug]
        check(plugin.PluginStartup(core._handle, None, debug))
        if kind == 3:
            plugin.SetCapturePath.argtypes = [C.c_char_p]
            if not plugin.SetCapturePath(str(out/'audio.bin').encode()):
                raise ValueError('cannot create PCM capture')
        if kind == 4:
            controller = plugin
            controller.SetButtons.argtypes = [C.c_int, C.c_uint]
            controller.SetPlayerCount(scenario['controllers'])
        check(core.CoreAttachPlugin(kind, plugin._handle))
        plugins.append((kind, plugin))
    errors = []
    Frame = C.CFUNCTYPE(None, C.c_uint)

    def observe(index):
        values = {}
        for name, (address, width, spec) in observed_symbols.items():
            if 'frames' in spec and index not in spec['frames']:
                continue
            if spec.get('indirect'):
                address = core.DebugMemRead32(address)
                if not address:
                    if 'expected' in spec and spec['expected'] is not None:
                        raise ValueError(f'frame {index}: {name} has a null observation pointer')
                    values[name] = None
                    continue
            address += spec.get('offset', 0)
            if not 0x80000000 <= address <= 0x80400000-width//8 or address % (width//8):
                raise ValueError(f'{name}: observation exceeds aligned 4 MB cached RAM: 0x{address:X}')
            values[name] = getattr(core, f'DebugMemRead{width}')(address)
            if 'expected' in spec and values[name] != spec['expected']:
                raise ValueError(f"frame {index}: {name} is {values[name]}, expected {spec['expected']}")
        observations.append(dict(frame=index, values=values))

    @Frame
    def frame(index):
        try:
            for address, width, value, name in writes.get(index, []):
                getattr(core, f'DebugMemWrite{width}')(address, value)
                print(f'Fixture frame {index}: {name} = {value}', flush=True)
            for player, buttons in events.get(index, []):
                controller.SetButtons(player, buttons)
            if index in frames | observation_frames:
                observe(index)
            if index in frames:
                if index in scenario.get('dump_ram', []):
                    pointer = core.DebugMemGetPointer(1)
                    if not pointer:
                        raise ValueError('debugger did not provide RDRAM')
                    words = array.array('I', C.string_at(pointer, 4*1024*1024))
                    if sys.byteorder == 'little':
                        words.byteswap()
                    (out/f'ram-{index}.bin').write_bytes(words.tobytes())
                check(core.CoreDoCommand(16, 0, None))
            if index == stop:
                check(core.CoreDoCommand(6, 0, None))
        except Exception as exc:
            errors.append(str(exc))
            core.CoreDoCommand(6, 0, None)

    check(core.CoreDoCommand(15, 0, C.cast(frame, C.c_void_p)))
    check(core.CoreDoCommand(17, 5, C.byref(C.c_int(0))))
    check(core.CoreDoCommand(5, 0, None))
    for kind, plugin in reversed(plugins):
        core.CoreDetachPlugin(kind)
        code = plugin.PluginShutdown()
        if kind == 3 and plugin.GetCaptureError() == 2:
            raise ValueError('AI PCM capture requires an LLE RSP; HLE ProcessAList was called')
        check(code)
    core.CoreDoCommand(2, 0, None)
    core.CoreShutdown()
    (out/'observations.json').write_text(json.dumps(observations, indent=2)+'\n')
    if errors:
        raise ValueError('; '.join(errors))
    verify_observation_coverage(scenario, observations)


if __name__ == '__main__':
    main()
