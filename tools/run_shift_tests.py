#!/usr/bin/env python3
"""Link temporary perturbations from existing matching objects; never edit assets."""
import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys

from verify_linked_relocations import Elf, verify

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
    sys.path.insert(0, str(ROOT))
    from tools.check_ram_references import inspect as check_ram_references
    from tools.check_mips_address_literals import inspect as check_mips_literals
    print(f'CPU data coverage: {check_ram_references(baseline)["counts"]}', flush=True)
    print(f'MIPS instruction literal audit: {check_mips_literals(baseline)["counts"]}', flush=True)
    try:
        verify(baseline, baseline)
    except ValueError as exc:
        if 'did not change' not in str(exc):
            raise
    else:
        raise ValueError('unchanged ELF incorrectly passed the perturbation check')
    for section in ('text', 'data', 'bss'):
        for size in (16, 0x800):
            elf, result = link(f'{section}-{size:x}', section, size)
            if result.returncode:
                raise ValueError(result.stderr)
            counts = verify(baseline, elf)
            check_ram_references(elf)
            check_mips_literals(elf)
            if section == 'text' and size == 0x800 and not counts.get('changed_HI16'):
                raise ValueError('text fixture did not cross a HI16 carry boundary')
            print(f'{section} +0x{size:X}: {counts}', flush=True)
    broken = out/'corrupt-reference.elf'
    parsed = Elf(out/'text-10.elf')
    contents = bytearray(parsed.data)
    entry = parsed.sections[parsed.names.index('.entry')]
    contents[entry[4]+15] ^= 1
    broken.write_bytes(contents)
    try:
        verify(baseline, broken)
    except ValueError as exc:
        if 'expected' not in str(exc):
            raise
    else:
        raise ValueError('corrupt linked reference was not detected')
    print('Unchanged and corrupt relocation fixtures correctly rejected', flush=True)
    parsed = Elf(baseline)
    symbols = {name: value for i, section in enumerate(parsed.sections) if section[1] == 2
               for name, value, kind, index in parsed.symbols(i) if name}
    main_section = parsed.sections[parsed.names.index('.main')]
    location = symbols['drawSnowboardModel']
    covered = {address for address, kind, symbol, word in parsed.relocations()['.main']}
    if location in covered or location+4 in covered:
        raise ValueError('literal coverage fixture requires two instruction words without relocations')
    position = main_section[4]+location-main_section[3]
    for name, instructions, diagnostic in [
            ('uncovered-high-half', bytes.fromhex('3c04800100000000'), 'unrelocated RAM high half'),
            ('uncovered-address-pair', bytes.fromhex('3c04800034840400'), 'unrelocated CPU address')]:
        contents = bytearray(parsed.data)
        contents[position:position+8] = instructions
        broken = out/(name+'.elf')
        broken.write_bytes(contents)
        try:
            check_mips_literals(broken)
        except ValueError as exc:
            if diagnostic not in str(exc):
                raise
        else:
            raise ValueError(f'{name}: unrelocated instruction address was not detected')
    print('Uncovered instruction high-half and address-pair fixtures correctly rejected', flush=True)
    position = main_section[4]+symbols['gMenuRenderModeResetDl']-main_section[3]
    contents = bytearray(parsed.data)
    contents[position:position+4] = (0x80001000).to_bytes(4, 'big')
    uncovered = out/'uncovered-cpu-pointer.elf'
    uncovered.write_bytes(contents)
    try:
        check_ram_references(uncovered)
    except ValueError as exc:
        if 'uncovered CPU word' not in str(exc):
            raise
    else:
        raise ValueError('uncovered CPU pointer was not detected')
    print('Uncovered CPU pointer correctly rejected', flush=True)
    _, result = link('ram-exhaustion', 'bss', 0x2000)
    if result.returncode == 0 or 'exceeds original 4 MB RAM' not in result.stderr:
        raise ValueError('RAM exhaustion did not fail with the required diagnostic: '+result.stderr)
    print('RAM exhaustion correctly rejected', flush=True)
    for boundary in ('main', 'SLASH_RACE_ANIMATIONS', '_609AA0'):
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
    from tools.audio_shift_fixture import prepare as prepare_audio, verify as verify_audio
    audio_script, audio_payload = prepare_audio(ROOT, rows, out, (out/'baseline.ld').read_text(),
                                                a.build_dir, linker, run)
    audio_ld, audio_elf, audio_rom = (out/('internal-audio'+suffix) for suffix in ('.ld', '.elf', '.z64'))
    audio_ld.write_text(audio_script)
    run(linker, '-T', audio_ld, *extra, '--emit-relocs', '--no-check-sections', '-o', audio_elf)
    print(f'Internal audio: {verify_audio(audio_elf, audio_payload)} linked waves verified', flush=True)
    run(linker[:-2]+'objcopy', '-O', 'binary', audio_elf, audio_rom)
    run(sys.executable, ROOT/'tools/n64crc.py', audio_rom)
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

    from tools.graphics_relocations import model_manifest, pack_model
    row = next(r for r in rows if r['decoder'] == 'embedded_model')
    manifest, path = materialize(ROOT, row)
    edited = model_manifest(manifest, path.parent)
    edited['parts'].insert(0, dict(id='fixture_padding', type='padding', offset=0,
                                  size=16, data=bytes(16).hex()))
    payload, exports = pack_model(edited, path.parent)
    binary, assembly, obj = out/'model.bin', out/'model.s', out/'model.o'
    binary.write_bytes(payload)
    source = f'.section .data\n.balign 8\n.incbin "{binary}"\n'
    for symbol, value in exports.items():
        source += f'.globl {symbol}\n.set {symbol}, 0x{value:08X}\n'
    assembly.write_text(source)
    assembler = linker[:-2]+'as'
    run(assembler, '-mips3', '-mabi=32', '-no-pad-sections', '-o', obj, assembly)
    for name, original_script in [('internal-model', out/'baseline.ld'), ('combined', out/'text-800.ld')]:
        script = out/f'{name}.ld'
        source = original_script.read_text()
        source_object = a.build_dir+'/'+row['object']
        if source.count(source_object) != 1:
            raise ValueError('expected exactly one embedded-model input')
        script.write_text(source.replace(source_object, str(obj)))
        if name == 'combined':
            run(sys.executable, ROOT/'tools/make_shiftable_linker.py', script, script,
                '--after', 'main', '--pad-size', '0x10000')
        elf = out/f'{name}.elf'
        run(linker, '-T', script, *extra, '--emit-relocs', '--no-check-sections', '-o', elf)
        print(f'{name}: {verify(baseline, elf)}', flush=True)
        rom = out/f'{name}.z64'
        run(linker[:-2]+'objcopy', '-O', 'binary', elf, rom)
        run(sys.executable, ROOT/'tools/n64crc.py', rom)
    print(f'Combined runtime fixture (not runtime-validated): {out / "combined.z64"}', flush=True)

    from tools.asset_bundles import commands
    from tools.course_relocations import layout_graphics, command_ids
    graphics = next(r for r in rows if r['name'] == 'SLASH_GRAPHICS')
    bank = next(r for r in rows if r['name'] == 'SLASH_MODEL_RESOURCES')
    manifest, path = materialize(ROOT, bank)
    manifest['parts'].insert(0, dict(id='fixture_padding', type='padding', offset=0,
                                    size=16, data=bytes(16).hex()))
    source_path = ROOT/graphics['source']
    data = commands(source_path)
    ids = command_ids(source_path.read_text(), data, graphics['rom_end']-graphics['rom_start'], graphics['name'])
    streams = [dict(name=graphics['name'], offset=0, ids=['fixture_sync_a', 'fixture_sync_b']+ids,
                    data=bytes.fromhex('e700000000000000')*2+data)]
    packed, exports, records, resources = layout_graphics('slash-fixture', streams, manifest, path.parent)
    if not records or any(r['value'] != 0x03000000+16+int.from_bytes(data[i*8+4:i*8+8], 'big') % 0x1000000
                          for i, identity in enumerate(ids) for r in records
                          if r['source'] == 'SLASH_GRAPHICS/'+identity and r['target'].startswith(bank['name']+'/')):
        raise ValueError('character resource fields did not follow their moved targets')
    script = (out/'combined.ld').read_text()
    for row, payload in ((graphics, packed[graphics['name']]), (bank, compress_flexible(resources))):
        binary, obj = out/(row['name']+'.bin'), out/(row['name']+'.o')
        binary.write_bytes(payload)
        if row == graphics:
            assembly = out/(row['name']+'.s')
            source = f'.section .data\n.balign 8\n.globl {row["name"]}\n{row["name"]}:\n.incbin "{binary}"\n'
            source += ''.join(f'.globl {s}\n.set {s}, 0x{v:08X}\n' for s, v in exports.items())
            assembly.write_text(source)
            run(assembler, '-mips3', '-mabi=32', '-no-pad-sections', '-o', obj, assembly)
        else:
            run(linker, '-r', '-b', 'binary', '-o', obj, binary)
        original_object = a.build_dir+'/'+row['object']
        if script.count(original_object) != 1:
            raise ValueError(f'{row["name"]}: missing character fixture input')
        script = script.replace(original_object, str(obj))
    combined = out/'combined-character.ld'
    combined.write_text(script)
    elf, rom = out/'combined-character.elf', out/'combined-character.z64'
    run(linker, '-T', combined, *extra, '--emit-relocs', '--no-check-sections', '-o', elf)
    print(f'Combined character commands and shared resources: {verify(baseline, elf)}', flush=True)
    run(linker[:-2]+'objcopy', '-O', 'binary', elf, rom)
    run(sys.executable, ROOT/'tools/n64crc.py', rom)
    print(f'Combined character runtime fixture (not runtime-validated): {rom}', flush=True)
    from tools.graphics_shift_fixture import prepare, verify_fields
    graphics_out = out/'all-graphics'
    graphics_out.mkdir(exist_ok=True)
    source, fields = prepare(ROOT, rows, graphics_out, (out/'combined.ld').read_text(), a.build_dir, linker, run)
    script = out/'combined-all-graphics.ld'
    script.write_text(source)
    elf, rom = out/'combined-all-graphics.elf', out/'combined-all-graphics.z64'
    run(linker, '-T', script, *extra, '--emit-relocs', '--no-check-sections', '-o', elf)
    counts = verify(baseline, elf)
    checked = verify_fields(elf, graphics_out/'graphics-fields.json')
    if checked != fields:
        raise ValueError('combined graphics relocation coverage changed during linking')
    parsed = Elf(elf)
    report = json.loads((graphics_out/'graphics-fields.json').read_text())
    group, = report[:1]
    ref = group['records'][0]
    stream, identity = ref['source'].split('/')
    section = parsed.sections[parsed.names.index('.'+stream)]
    start = min(value for name, value in group['exports'].items() if name.startswith(stream+'_'))
    position = section[4]+group['exports'][stream+'_'+identity]-start+ref['field']
    contents = bytearray(parsed.data)
    contents[position+3] ^= 8
    broken = out/'corrupt-packed-graphics.elf'
    broken.write_bytes(contents)
    try:
        verify_fields(broken, graphics_out/'graphics-fields.json')
    except ValueError as exc:
        if 'packed field points to' not in str(exc):
            raise
    else:
        raise ValueError('corrupt final packed graphics field was not detected')
    print('Corrupt final packed graphics field correctly rejected', flush=True)
    print(f'All graphics groups: {checked} linked packed fields; {counts}', flush=True)
    run(linker[:-2]+'objcopy', '-O', 'binary', elf, rom)
    run(sys.executable, ROOT/'tools/n64crc.py', rom)
    print(f'All-graphics runtime fixture (not runtime-validated): {rom}', flush=True)
    from tools.format_shift_fixture import prepare as prepare_formats, verify as verify_formats
    format_out = out/'formats'
    format_out.mkdir(exist_ok=True)
    source, coverage, moved = prepare_formats(ROOT, rows, format_out, script.read_text(), a.build_dir, linker, run)
    source, audio_payload = prepare_audio(ROOT, rows, out, source, a.build_dir, linker, run)
    combined_ld = out/'combined-formats.ld'
    combined_ld.write_text(source)
    elf, rom = out/'combined-formats.elf', out/'combined-formats.z64'
    run(linker, '-T', combined_ld, *extra, '--emit-relocs', '--no-check-sections', '-o', elf)
    print(f'Combined format corpus: {dict(coverage)}; moved {dict(moved)}', flush=True)
    print(f'Combined format fields: {verify_formats(elf, format_out/"format-fields.json")} assets, '
          f'{verify_audio(elf, audio_payload)} audio waves, '
          f'{verify_fields(elf, graphics_out/"graphics-fields.json")} graphics fields; '
          f'{verify(baseline, elf)}', flush=True)
    run(linker[:-2]+'objcopy', '-O', 'binary', elf, rom)
    run(sys.executable, ROOT/'tools/n64crc.py', rom)
    print(f'Combined format runtime fixture (not runtime-validated): {rom}', flush=True)


if __name__ == '__main__':
    main()
