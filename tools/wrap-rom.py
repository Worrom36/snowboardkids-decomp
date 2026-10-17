#!/usr/bin/env python3
"""Vendor Mupen64Plus (~2.6 MB) and wrap the built ROM into dist/play."""
from __future__ import annotations

import argparse
import hashlib
import shutil
import subprocess
import sys
import urllib.request
import zipfile
from pathlib import Path

MUPEN_VERSION = "2.6.0"
MUPEN_ZIP = f"mupen64plus-bundle-win64-{MUPEN_VERSION}.zip"
MUPEN_URL = (
    f"https://github.com/mupen64plus/mupen64plus-core/releases/download/"
    f"{MUPEN_VERSION}/{MUPEN_ZIP}"
)
MUPEN_SHA256 = "8292c68d6ffc3428d4181ac09b1368ef2adb2cc568325545aac78cb6c1e66e21"

# Keep the playable folder small: drop sample ROM and cheat DB.
SKIP_NAMES = {
    "m64p_test_rom.v64",
    "mupencheat.txt",
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def ensure_mupen(root: Path) -> Path:
    """Return tools/mupen64plus containing the console UI + plugins."""
    dest = root / "tools" / "mupen64plus"
    if (dest / "mupen64plus-ui-console.exe").is_file():
        return dest

    cache = root / "tmp" / MUPEN_ZIP
    cache.parent.mkdir(parents=True, exist_ok=True)
    if not cache.is_file():
        print(f"Downloading {MUPEN_URL} ...", flush=True)
        urllib.request.urlretrieve(MUPEN_URL, cache)

    digest = sha256_file(cache)
    if digest.lower() != MUPEN_SHA256:
        cache.unlink(missing_ok=True)
        raise SystemExit(
            f"wrap-rom: Mupen64Plus zip hash mismatch\n"
            f"  expected {MUPEN_SHA256}\n"
            f"  got      {digest}"
        )

    extract_root = root / "tmp" / "mupen-extract"
    if extract_root.exists():
        shutil.rmtree(extract_root)
    extract_root.mkdir(parents=True)
    with zipfile.ZipFile(cache) as archive:
        archive.extractall(extract_root)

    release = extract_root / "Release"
    if not release.is_dir():
        release = extract_root

    if dest.exists():
        shutil.rmtree(dest)
    dest.mkdir(parents=True)

    for path in release.rglob("*"):
        if not path.is_file() or path.name in SKIP_NAMES:
            continue
        out = dest / path.relative_to(release)
        out.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, out)

    shutil.rmtree(extract_root, ignore_errors=True)
    if not (dest / "mupen64plus-ui-console.exe").is_file():
        raise SystemExit(f"wrap-rom: mupen64plus-ui-console.exe missing under {dest}")
    return dest


def write_launcher(play_dir: Path, rom_name: str) -> None:
    (play_dir / "Play.bat").write_text(
        "\r\n".join(
            [
                "@echo off",
                "setlocal",
                'cd /d "%~dp0"',
                f'start "" "mupen64plus-ui-console.exe" --nogui "{rom_name}"',
                "",
            ]
        ),
        encoding="ascii",
    )


def wrap(root: Path, rom: Path, out_dir: Path) -> Path:
    if not rom.is_file():
        raise SystemExit(f"wrap-rom: ROM not found: {rom}")

    mupen = ensure_mupen(root)
    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True)

    for path in mupen.rglob("*"):
        if not path.is_file():
            continue
        target = out_dir / path.relative_to(mupen)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, target)

    rom_name = rom.name
    shutil.copy2(rom, out_dir / rom_name)
    write_launcher(out_dir, rom_name)
    (out_dir / "README.txt").write_text(
        "\r\n".join(
            [
                "Snowboard Kids - matching build + Mupen64Plus 2.6.0",
                "",
                "Double-click Play.bat to start.",
                f"Or: mupen64plus-ui-console.exe --nogui {rom_name}",
                "",
                "Emulator: https://github.com/mupen64plus/mupen64plus-core/releases/tag/2.6.0",
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
