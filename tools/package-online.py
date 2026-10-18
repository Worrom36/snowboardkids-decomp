#!/usr/bin/env python3
"""Build a self-contained Online Battle Windows play kit under dist/online."""
from __future__ import annotations

import argparse
import shutil
import sys
import urllib.request
import zipfile
from pathlib import Path

_TOOLS_DIR = Path(__file__).resolve().parent
if str(_TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(_TOOLS_DIR))

from mupen_bundle import (  # noqa: E402
    MUPEN_VERSION,
    copy_mupen_tree,
    ensure_mupen,
    sha256_file,
)

PYTHON_VERSION = "3.12.10"
PYTHON_ZIP = f"python-{PYTHON_VERSION}-embed-amd64.zip"
PYTHON_URL = f"https://www.python.org/ftp/python/{PYTHON_VERSION}/{PYTHON_ZIP}"
PYTHON_SHA256 = "4acbed6dd1c744b0376e3b1cf57ce906f9dc9e95e68824584c8099a63025a3c3"

ONLINE_MODULES = (
    "__init__.py",
    "protocol.py",
    "server.py",
    "client.py",
    "mailbox_file.py",
    "mupen_bridge.py",
)

DEFAULT_PORT = 6464
PACKAGED_ROM_NAME = "snowboardkids-online.z64"


def _write_crlf(path: Path, lines: list[str]) -> None:
    # write_bytes avoids Windows text-mode turning "\r\n" into "\r\r\n".
    path.write_bytes(("\r\n".join(lines) + "\r\n").encode("ascii"))


def ensure_embed_python(root: Path, dest: Path) -> Path:
    """Download embeddable CPython into dest (python.exe + stdlib zip)."""
    cache = root / "tmp" / PYTHON_ZIP
    cache.parent.mkdir(parents=True, exist_ok=True)
    if not cache.is_file():
        print(f"Downloading {PYTHON_URL} ...", flush=True)
        urllib.request.urlretrieve(PYTHON_URL, cache)

    digest = sha256_file(cache)
    if digest.lower() != PYTHON_SHA256:
        cache.unlink(missing_ok=True)
        raise SystemExit(
            f"package-online: embeddable Python hash mismatch\n"
            f"  expected {PYTHON_SHA256}\n"
            f"  got      {digest}"
        )

    if dest.exists():
        shutil.rmtree(dest)
    dest.mkdir(parents=True)

    with zipfile.ZipFile(cache) as archive:
        archive.extractall(dest)

    exe = dest / "python.exe"
    if not exe.is_file():
        raise SystemExit(f"package-online: python.exe missing under {dest}")

    # Keep isolated: stdlib zip is already listed in python312._pth.
    # Uncomment import site only if third-party packages are ever needed.
    pth = dest / "python312._pth"
    if pth.is_file():
        text = pth.read_text(encoding="utf-8")
        # Ensure the stdlib zip and cwd are importable; leave site disabled.
        if "python312.zip" not in text:
            pth.write_text(
                "python312.zip\n.\n# import site\n",
                encoding="utf-8",
            )
    return dest


def copy_online_lib(root: Path, lib_dir: Path) -> None:
    """Copy tools.online modules so packaged entrypoints keep the same imports."""
    online_src = root / "tools" / "online"
    tools_pkg = lib_dir / "tools"
    online_dst = tools_pkg / "online"
    if lib_dir.exists():
        shutil.rmtree(lib_dir)
    online_dst.mkdir(parents=True)

    shutil.copy2(root / "tools" / "__init__.py", tools_pkg / "__init__.py")
    for name in ONLINE_MODULES:
        src = online_src / name
        if not src.is_file():
            raise SystemExit(f"package-online: missing {src}")
        shutil.copy2(src, online_dst / name)

    _write_crlf(
        lib_dir / "run_server.py",
        [
            "#!/usr/bin/env python3",
            '"""Packaged Online Battle lobby server entrypoint."""',
            "import sys",
            "from pathlib import Path",
            "",
            "LIB = Path(__file__).resolve().parent",
            "if str(LIB) not in sys.path:",
            "    sys.path.insert(0, str(LIB))",
            "",
            "from tools.online.server import main",
            "",
            'if __name__ == "__main__":',
            "    main()",
            "",
        ],
    )
    _write_crlf(
        lib_dir / "run_client.py",
        [
            "#!/usr/bin/env python3",
            '"""Packaged Online Battle net client entrypoint."""',
            "import sys",
            "from pathlib import Path",
            "",
            "LIB = Path(__file__).resolve().parent",
            "if str(LIB) not in sys.path:",
            "    sys.path.insert(0, str(LIB))",
            "",
            "from tools.online.client import main",
            "",
            'if __name__ == "__main__":',
            "    main()",
            "",
        ],
    )


def write_bats(out_dir: Path, rom_name: str) -> None:
    py = r'"%~dp0python\python.exe"'
    _write_crlf(
        out_dir / "HostServer.bat",
        [
            "@echo off",
            "setlocal",
            'cd /d "%~dp0"',
            f"echo Starting Online Battle lobby on 0.0.0.0:{DEFAULT_PORT} ...",
            f"{py} lib\\run_server.py --host 0.0.0.0 --port {DEFAULT_PORT}",
            "echo.",
            "pause",
            "",
        ],
    )
    _write_crlf(
        out_dir / "Join.bat",
        [
            "@echo off",
            "setlocal",
            'cd /d "%~dp0"',
            "if not exist tmp mkdir tmp",
            "echo Starting Online Battle client (edit config.ini for server IP)...",
            f"{py} lib\\run_client.py --config config.ini --mailbox tmp\\online_mailbox.bin --mupen-bridge",
            "echo.",
            "pause",
            "",
        ],
    )
    _write_crlf(
        out_dir / "Play.bat",
        [
            "@echo off",
            "setlocal",
            'cd /d "%~dp0"',
            "if not exist tmp mkdir tmp",
            "echo Starting Online client + Mupen...",
            'start "Online Client" "%~dp0Join.bat"',
            "timeout /t 1 /nobreak >nul",
            f'start "" "mupen64plus-ui-console.exe" --nogui "{rom_name}"',
            "",
        ],
    )
def write_config(out_dir: Path) -> None:
    _write_crlf(
        out_dir / "config.ini",
        [
            "[online]",
            "host = 127.0.0.1",
            f"port = {DEFAULT_PORT}",
            "",
        ],
    )


def write_readme(out_dir: Path, rom_name: str) -> None:
    _write_crlf(
        out_dir / "README.txt",
        [
            "Snowboard Kids - Online Battle (Windows play kit)",
            "",
            "Contents: Online Battle ROM, Mupen64Plus, lobby server/client,",
            "embeddable Python. No install required.",
            "",
            "How to play (LAN / local)",
            "-------------------------",
            "1. Host: double-click HostServer.bat (keep the window open).",
            "2. Each player: edit config.ini host= to the server PC IP",
            f"   (leave 127.0.0.1 for same-machine). Port default is {DEFAULT_PORT}.",
            "3. Each player: double-click Play.bat (starts Join.bat client + Mupen).",
            "4. In-game: GAME MENU -> Online Battle -> ready up; host Starts.",
            "",
            "Join.bat alone is optional if you only need the client window.",
            "",
            "Requirements",
            "------------",
            f"- UDP port {DEFAULT_PORT} allowed through Windows Firewall for python.exe",
            "  (host PC; first run may prompt).",
            "- Emulator: 8 MB RDRAM / Expansion Pak (Mupen default is usually fine).",
            "- Max 4 players. Ready/start are manual (no auto lobby).",
            "",
            f"ROM: {rom_name}",
            f"Emulator: Mupen64Plus {MUPEN_VERSION}",
            f"Python: embeddable {PYTHON_VERSION}",
            "",
        ],
    )


def refuse_autotest_rom(rom: Path) -> None:
    """Fail if the companion map still contains the AUTOTEST boot symbol."""
    map_path = rom.with_suffix(".map")
    if not map_path.is_file():
        # ELF map lives beside the ROM for shiftable builds; warn but continue
        # only if the user forced a lone ROM copy without map metadata.
        print(
            f"package-online: warning: no map at {map_path}; "
            "cannot verify ONLINE_BATTLE_AUTOTEST is off",
            flush=True,
        )
        return
    text = map_path.read_text(encoding="utf-8", errors="replace")
    if "bootOnlineBattleAutotest" in text:
        raise SystemExit(
            "package-online: refusing to ship an ONLINE_BATTLE_AUTOTEST ROM "
            f"(found bootOnlineBattleAutotest in {map_path}).\n"
            "Rebuild without autotest, then package:\n"
            "  make BUILD_MODE=shiftable ONLINE_BATTLE=1 UNLOCK_ALL=1 "
            "ONLINE_BATTLE_AUTOTEST=0 AUTOPILOT_DEFAULT_ON=0 WRAP_ROM=0\n"
            "  python tools/package-online.py --zip\n"
            "Or: make package-online"
        )


def package(
    root: Path,
    rom: Path,
    out_dir: Path,
    make_zip: bool,
) -> Path:
    if not rom.is_file():
        raise SystemExit(
            "package-online: ROM not found: "
            f"{rom}\n"
            "Build first:\n"
            "  make BUILD_MODE=shiftable ONLINE_BATTLE=1 UNLOCK_ALL=1 "
            "ONLINE_BATTLE_AUTOTEST=0 AUTOPILOT_DEFAULT_ON=0 WRAP_ROM=0\n"
            "Or: make package-online"
        )

    refuse_autotest_rom(rom)

    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True)

    rom_name = PACKAGED_ROM_NAME

    if sys.platform != "win32":
        # Non-Windows: still stage ROM + scripts for inspection; skip Mupen/Python.
        shutil.copy2(rom, out_dir / rom_name)
        copy_online_lib(root, out_dir / "lib")
        write_config(out_dir)
        write_readme(out_dir, rom_name)
        print(f"package-online: non-Windows host — staged ROM + lib under {out_dir}")
        return out_dir

    mupen = ensure_mupen(root)
    copy_mupen_tree(mupen, out_dir)
    ensure_embed_python(root, out_dir / "python")
    copy_online_lib(root, out_dir / "lib")

    shutil.copy2(rom, out_dir / rom_name)
    write_config(out_dir)
    write_bats(out_dir, rom_name)
    write_readme(out_dir, rom_name)
    (out_dir / "tmp").mkdir(exist_ok=True)

    if make_zip:
        zip_path = out_dir.parent / "SnowboardKids-OnlineBattle-win64.zip"
        if zip_path.exists():
            zip_path.unlink()
        with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
            for path in out_dir.rglob("*"):
                if path.is_file():
                    archive.write(path, path.relative_to(out_dir.parent))
        print(f"Zip: {zip_path}")

    return out_dir


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--rom",
        type=Path,
        default=None,
        help="Online Battle ROM (default: build/shiftable/snowboardkids.z64)",
    )
    parser.add_argument(
        "--out",
        type=Path,
        default=None,
        help="Output folder (default: dist/online)",
    )
    parser.add_argument(
        "--zip",
        action="store_true",
        help="Also write dist/SnowboardKids-OnlineBattle-win64.zip",
    )
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    rom = (args.rom or (root / "build" / "shiftable" / "snowboardkids.z64")).resolve()
    out_dir = (args.out or (root / "dist" / "online")).resolve()

    packaged = package(root, rom, out_dir, make_zip=bool(args.zip))
    print(f"Packaged: {packaged}")
    if sys.platform == "win32":
        print(f"  Host:  {packaged / 'HostServer.bat'}")
        print(f"  Join:  {packaged / 'Join.bat'}")
        print(f"  Play:  {packaged / 'Play.bat'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
