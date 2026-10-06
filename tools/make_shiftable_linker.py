#!/usr/bin/env python3
"""Inject ROM-only padding after a linker segment for shiftability testing."""

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))


def graphics_layout(source):
    from tools.course_relocations import graphics_groups
    for bank, group in graphics_groups(Path(__file__).resolve().parents[1]):
        if not any('.'+s['name']+' ' in source for s in group):
            continue
        for previous, following in zip(group, group[1:]):
            name = following['name']
            source, count = re.subn(rf'(\.{re.escape(name)}) 0x[\da-fA-F]+ :',
                                    rf"\1 ALIGN({previous['name']}_VRAM_END, 8) :", source)
            if count != 1:
                raise ValueError(f'{name}: expected one graphics section placement')
    return source


def parse_int(value: str) -> int:
    return int(value, 0)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--grow", choices=("text", "data", "bss"))
    parser.add_argument("--growth", type=parse_int, default=0)
    parser.add_argument("--layout", action="store_true")
    parser.add_argument("--build-dir", default="build/shiftable")
    parser.add_argument("--after", help="segment name without _ROM_END")
    parser.add_argument("--pad-size", type=parse_int, default=0x10)
    args = parser.parse_args()

    if args.layout:
        source = args.input.read_text().replace("build/", args.build_dir.rstrip("/") + "/")
        source = graphics_layout(source)
        source, count = re.subn(r"\.buffers_bss 0x[0-9A-Fa-f]+ \(NOLOAD\)",
                                ".buffers_bss ALIGN(main_BSS_END, 64) (NOLOAD)", source)
        if count != 1:
            parser.error("expected one runtime buffer section")
        if args.grow:
            if args.growth <= 0 or args.growth % 16:
                parser.error("--growth must be a positive multiple of 16")
            marker = f"        main_{args.grow.upper()}_START = .;"
            if source.count(marker) != 1:
                parser.error(f"expected one {marker.strip()}")
            if args.grow == 'bss':
                source = source.replace(marker, marker + f"\n        . += 0x{args.growth:X};\n"
                                        "        main_BSS_CONTENT_START = .;")
            else:
                source = source.replace(marker, f"        . += 0x{args.growth:X};\n" + marker)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(source)
        return
    if not args.after:
        parser.error("--after is required for ROM padding")
    if args.after in {"header", "ipl3", "entry"}:
        parser.error("padding before the end of main would alter hardware boot placement")
    from tools.course_relocations import graphics_groups
    for bank, group in graphics_groups(Path(__file__).resolve().parents[1]):
        if any(s['name'] == args.after for s in group[:-1]):
            parser.error(f"cannot insert padding inside {bank['name']} graphics DMA stream")
    layout = Path(__file__).resolve().parents[1] / 'assets/layout/segments.yaml'
    if layout.exists():
        import yaml
        segments = yaml.safe_load(layout.read_text())['segments']
        for previous, following in zip(segments, segments[1:]):
            if previous['name'] == args.after and previous['decoder'] == following['decoder'] == 'sample_bank':
                parser.error('cannot insert padding inside the contiguous sample stream; move the whole stream instead')
    if args.pad_size <= 0 or args.pad_size % 16 != 0:
        parser.error("--pad-size must be a positive multiple of 16")

    source = args.input.read_text()
    marker = f"    {args.after}_ROM_END = __romPos;"
    if source.count(marker) != 1:
        parser.error(f"expected exactly one linker marker: {marker.strip()}")

    injection = (
        f"{marker}\n"
        f"    /* ROM shiftability test padding; VRAM location counter is unchanged. */\n"
        f"    __romPos += 0x{args.pad_size:X};"
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(source.replace(marker, injection, 1))


if __name__ == "__main__":
    main()
