#!/usr/bin/env python3
"""Vendor Mupen64Plus (~2.6 MB) and wrap the built ROM into dist/play."""
from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

_TOOLS_DIR = Path(__file__).resolve().parent
if str(_TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(_TOOLS_DIR))

from mupen_bundle import MUPEN_VERSION, copy_mupen_tree, ensure_mupen  # noqa: E402


def write_launcher(play_dir: Path, rom_name: str) -> None:
    # write_bytes avoids Windows text-mode turning "\r\n" into "\r\r\n".
    (play_dir / "Play.bat").write_bytes(
        (
            "\r\n".join(
                [
                    "@echo off",
                    "setlocal",
                    'cd /d "%~dp0"',
                    f'start "" "mupen64plus-ui-console.exe" --nogui "{rom_name}"',
                    "",
                ]
            )
            + "\r\n"
        ).encode("ascii")
    )


def wrap(root: Path, rom: Path, out_dir: Path) -> Path:
    if not rom.is_file():
        raise SystemExit(f"wrap-rom: ROM not found: {rom}")

    mupen = ensure_mupen(root)
    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True)

    copy_mupen_tree(mupen, out_dir)

    rom_name = rom.name
    shutil.copy2(rom, out_dir / rom_name)
    write_launcher(out_dir, rom_name)
    (out_dir / "README.txt").write_text(
        "\r\n".join(
            [
                f"Snowboard Kids - matching build + Mupen64Plus {MUPEN_VERSION}",
                "",
                "Double-click Play.bat to start.",
                f"Or: mupen64plus-ui-console.exe --nogui {rom_name}",
                "",
                f"Emulator: https://github.com/mupen64plus/mupen64plus-core/releases/tag/{MUPEN_VERSION}",
                "",
            ]
        ),
        encoding="ascii",
    )
    return out_dir


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=None)
    parser.add_argument("--out", type=Path, default=None)
    parser.add_argument("--run", action="store_true", help="Launch after wrapping")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    rom = (args.rom or (root / "build" / "snowboardkids.z64")).resolve()
    out_dir = (args.out or (root / "dist" / "play")).resolve()

    if sys.platform != "win32":
        out_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(rom, out_dir / rom.name)
        print(f"wrap-rom: non-Windows host — copied ROM to {out_dir / rom.name}")
        return 0

    play = wrap(root, rom, out_dir)
    print(f"Wrapped: {play}")
    print(f"  Run:   {play / 'Play.bat'}")

    if args.run:
        subprocess.Popen(
            [str(play / "mupen64plus-ui-console.exe"), "--nogui", str(play / rom.name)],
            cwd=str(play),
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
