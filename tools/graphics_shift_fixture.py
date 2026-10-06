"""Temporary combined graphics fixtures; authoritative asset sources stay intact."""
import json
import hashlib

from tools.asset_bundles import commands, materialize
from tools.course_relocations import command_ids, graphics_groups, layout_graphics, resource_layout
from tools.course_graphics_common import trace_course_graphics
from tools.huffman_asset import compress_flexible


def prepare(root, rows, out, script, build_dir, linker, run):
    rows = {r['name']: r for r in rows}
    report = []
    for bank, group in graphics_groups(root):
        streams = []
        for spec in group:
            path = root/rows[spec['name']]['source']
            data = commands(path)
            identities = command_ids(path.read_text(), data, spec['end']-spec['start'], spec['name'])
            streams.append(dict(name=spec['name'], offset=spec['start']-bank['graphics_start'], data=data, ids=identities))
        streams[0]['data'] = bytes.fromhex('e700000000000000')*2+streams[0]['data']
        streams[0]['ids'][:0] = ['fixture_sync_a', 'fixture_sync_b']
        manifest, path = materialize(root, rows[bank['name']])
        manifest['parts'].insert(0, dict(id='fixture_prefix', type='padding', offset=0,
                                         size=16, data=bytes(16).hex()))
        packed, exports, references, resources = layout_graphics(bank['name'], streams, manifest, path.parent)
        whole = b''.join(packed[s['name']] for s in streams)
        roots = []
        for offset in bank['root_offsets']:
            spec = next(s for s in group if s['start']-bank['graphics_start'] <= offset < s['end']-bank['graphics_start'])
            identity = f"{spec['name']}_command_{offset-(spec['start']-bank['graphics_start']):06X}"
            roots.append(exports[identity] & 0xffffff)
        trace_course_graphics(whole, roots)
        if not references:
            raise ValueError(f"{bank['name']}: graphics fixture has no reference checks")
        _, _, resource_offsets, resource_data = resource_layout(manifest, path.parent)
        targets = {identity: dict(offset=resource_offsets[identity], size=len(data), sha256=hashlib.sha256(data).hexdigest())
                   for identity, data in resource_data.items()}
        report.append(dict(asset=bank['name'], records=references, exports=exports, resource_objects=targets))
        for stream in streams:
            name = stream['name']
            binary, assembly, obj = (out/(name+suffix) for suffix in ('.bin', '.s', '.o'))
            binary.write_bytes(packed[name])
            source = f'.section .data\n.balign 8\n.globl {name}\n{name}:\n.incbin "{binary}"\n'
            source += ''.join(f'.globl {name}_{identity}\n.set {name}_{identity}, 0x{exports[name+"_"+identity]:08X}\n'
                              for identity in stream['ids'])
            assembly.write_text(source)
            run(linker[:-2]+'as', '-mips3', '-mabi=32', '-no-pad-sections', '-o', obj, assembly)
            original = build_dir+'/'+rows[name]['object']
            if script.count(original) != 1:
                raise ValueError(f'{name}: graphics fixture input is missing or ambiguous')
            script = script.replace(original, str(obj))
        binary, obj = out/(bank['name']+'.bin'), out/(bank['name']+'.o')
        binary.write_bytes(compress_flexible(resources))
        run(linker, '-r', '-b', 'binary', '-o', obj, binary)
        original = build_dir+'/'+rows[bank['name']]['object']
        if script.count(original) != 1:
            raise ValueError(f"{bank['name']}: resource fixture input is missing or ambiguous")
        script = script.replace(original, str(obj))
    (out/'graphics-fields.json').write_text(json.dumps(report, indent=2)+'\n')
    return script, sum(len(g['records']) for g in report)


def verify_fields(elf_path, report_path):
    """Check reference locations and packed fields in the final linked ELF."""
    from tools.verify_linked_relocations import Elf
    elf = Elf(elf_path)
    sections = {name: section for name, section in zip(elf.names, elf.sections)}
    symbols = {name: value for i, section in enumerate(elf.sections) if section[1] == 2
               for name, value, kind, index in elf.symbols(i) if name}
    count = 0
    for group in json.loads(report_path.read_text()):
        from tools.huffman_asset import decompress_huffman_asset
        resource_section = sections['.'+group['asset']]
        try:
            resource_data = decompress_huffman_asset(elf.data[resource_section[4]:resource_section[4]+resource_section[5]])[0]
        except ValueError as exc:
            raise ValueError(f"{group['asset']}: invalid linked compressed resources: {exc}") from exc
        for identity, target in group['resource_objects'].items():
            data = resource_data[target['offset']:target['offset']+target['size']]
            if len(data) != target['size'] or hashlib.sha256(data).hexdigest() != target['sha256']:
                raise ValueError(f"{group['asset']}/{identity}: linked target object is missing or corrupt")
        for name, value in group['exports'].items():
            if symbols.get(name) != value:
                raise ValueError(f'{name}: missing or incorrect exported graphics identity')
        for ref in group['records']:
            stream, identity = ref['source'].split('/')
            section = sections['.'+stream]
            stream_names = [name for name in group['exports'] if name.startswith(stream+'_')]
            start = min(group['exports'][name] for name in stream_names)
            field = symbols[stream+'_'+identity]-start+ref['field']
            if field < 0 or field+4 > section[5]:
                raise ValueError(f"{ref['source']}: relocated field exceeds its linked stream")
            expected = ref['value']
            target_stream, target = ref['target'].split('/')
            if target_stream != group['asset']:
                expected = symbols[target_stream+'_'+target]+ref['addend']
            else:
                expected = 0x03000000+group['resource_objects'][target]['offset']+ref['addend']
            actual = elf.word(section[4]+field)
            if actual != expected:
                raise ValueError(f"{ref['source']}+{ref['field']}: packed field points to 0x{actual:X}, expected 0x{expected:X}")
            count += 1
    if not count:
        raise ValueError('combined graphics field verification unexpectedly empty')
    return count
