#!/usr/bin/env python3
"""Reject hardcoded ROM boundaries and interior operands of ROM-consuming APIs."""

import argparse
import re
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools.rom_reference_context import literal_operands


HEX_RE = re.compile(r"\b0x0*([0-9A-Fa-f]{4,8})\b")
LEGACY_RE = re.compile(r"\bD_([0-7][0-9A-Fa-f]{5,7})\b")
YAML_ADDRESS_RE = re.compile(r"(?:start|rom_start|rom_end):\s*['\"]?0x([0-9A-Fa-f]+)")
YAML_LIST_RE = re.compile(r"-\s*\[\s*0x([0-9A-Fa-f]+)")


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", lambda match: "\n" * match.group(0).count("\n"), text, flags=re.DOTALL)
    return re.sub(r"//.*", "", text)


def load_boundaries(root: Path, include_end=False) -> set[int]:
    paths = [root / "snowboardkids.yaml", *(root / "config" / "courses").glob("*.yaml")]
    boundaries = set()
    for path in paths:
        text = path.read_text()
        boundaries.update(int(value, 16) for value in YAML_ADDRESS_RE.findall(text))
        boundaries.update(int(value, 16) for value in YAML_LIST_RE.findall(text))
    # The highest marker is the exclusive end/declared capacity of the ROM, not
    # a readable runtime address. It is also a common fixed-point/flag value.
    if not boundaries:
        raise ValueError('ROM scan: extraction boundary coverage is unexpectedly empty')
    if not include_end:
        boundaries.discard(max(boundaries))
    return boundaries


def load_allowlist(path: Path) -> set[tuple[str, str]]:
    entries = set()
    for number, line in enumerate(path.read_text().splitlines(), 1):
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        parts = stripped.split(maxsplit=2)
        if len(parts) < 3:
            raise SystemExit(f"{path}:{number}: expected: path token reason")
        entries.add((parts[0], parts[1]))
    return entries


def source_files(root: Path):
    # Include new source files before they are staged. Generated assembly is
    # normally ignored by git, so discover actual linker inputs independently.
    # Historical asm/matchings mirrors are inventoried by shift_inventory, but
    # are not inputs to this build and must not mask the active-file checks.
    paths = {path for directory in ('src', 'include', 'linker_scripts')
             for path in (root/directory).rglob('*') if path.suffix in {'.c', '.h', '.s', '.ld'}}
    linker = root/'snowboardkids.ld'
    if not linker.exists():
        raise ValueError('ROM scan: missing generated linker script; run extraction first')
    inputs = re.findall(r'\b(?:[^\s()]+/)?(asm/[^\s()]+)\.o\(', linker.read_text())
    if not inputs:
        raise ValueError('ROM scan: generated assembly input coverage is unexpectedly empty')
    for name in inputs:
        path = root/(name+'.s')
        if not path.is_file():
            raise ValueError(f'ROM scan: missing generated assembly input {name}.s')
        paths.add(path)
    yield from sorted(paths)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--allowlist", type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    allowlist_path = args.allowlist or root / "config" / "rom_address_allowlist.txt"
    boundaries = load_boundaries(root, include_end=True)
    rom_end = max(boundaries)
    boundaries.remove(rom_end)
    allowlist = load_allowlist(allowlist_path)
    findings = set()
    operand_checks = 0

    for path in source_files(root):
        relative = path.relative_to(root).as_posix()
        original_lines = path.read_text(errors="replace").splitlines()
        clean_lines = strip_comments("\n".join(original_lines)).splitlines()
        for line_number, (original, line) in enumerate(zip(original_lines, clean_lines), 1):
            for match in HEX_RE.finditer(line):
                token = match.group(0)
                value = int(match.group(1), 16)
                if value >= 0x10000 and value in boundaries and (relative, token) not in allowlist:
                    findings.add((relative, line_number, token, original.strip()))
            for match in LEGACY_RE.finditer(line):
                token = match.group(0)
                value = int(match.group(1), 16)
                safe_alias = re.search(rf"\b{re.escape(token)}\s*=.*_ROM_(?:START|END)\b", line)
                if 0x1000 <= value < rom_end and not safe_alias and (relative, token) not in allowlist:
                    findings.add((relative, line_number, token, original.strip()))

        text = '\n'.join(original_lines)
        for position, token, api in literal_operands(text, 0x1000, rom_end):
            if (relative, token) not in allowlist:
                line_number = text.count('\n', 0, position)+1
                findings.add((relative, line_number, token, original_lines[line_number-1].strip()))
        operand_checks += len(re.findall(r'\b(?:dmaReadRom|loadCompressedRomAsset|loadRawRomAsset)\s*\(',
                                        strip_comments(text)))

    if findings:
        for relative, line_number, token, line in sorted(findings):
            print(f"{relative}:{line_number}: hardcoded ROM address {token}: {line}")
        raise SystemExit(f"found {len(findings)} hardcoded ROM address reference(s)")
    if not operand_checks:
        raise ValueError('ROM scan: address-consuming API coverage is unexpectedly empty')
    print(f"ROM address scan OK: {len(boundaries)} boundaries, {operand_checks} ROM API sites, no forbidden references")


if __name__ == "__main__":
    main()
