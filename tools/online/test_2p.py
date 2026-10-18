#!/usr/bin/env python3
"""Launch a local 2P Online Battle test stack in one command.

Prereq (autotest ROM that boots straight into the lobby):
  make BUILD_MODE=shiftable ONLINE_BATTLE=1 ONLINE_BATTLE_AUTOTEST=1

Then:
  python tools/online_test_2p.py
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PLAY = ROOT / "dist" / "play"
ROM_BUILD = ROOT / "build" / "shiftable" / "snowboardkids.z64"
ROM_PLAY = PLAY / "snowboardkids.z64"
MUPEN = PLAY / "mupen64plus-ui-console.exe"
PYTHON = sys.executable
CREATE_NEW_CONSOLE = 0x00000010


def _taskkill(*args: str) -> None:
    if sys.platform != "win32":
        return
    subprocess.run(["taskkill", *args], capture_output=True)


def _kill_stale() -> None:
    """Kill every leftover server/client/mupen from prior runs."""
    if sys.platform != "win32":
        return
    self_pid = os.getpid()
    _taskkill("/IM", "mupen64plus-ui-console.exe", "/F")
    ps = rf"""
$self = {self_pid}
$procs = Get-CimInstance Win32_Process | Where-Object {{
  $_.ProcessId -ne $self -and $_.CommandLine -and (
    $_.CommandLine -match 'online_server\.py' -or
    $_.CommandLine -match 'online_client\.py'
  )
}}
foreach ($p in $procs) {{ Stop-Process -Id $p.ProcessId -Force -ErrorAction SilentlyContinue }}
Get-Process cmd -ErrorAction SilentlyContinue |
  Where-Object {{ $_.MainWindowTitle -match 'Online (Battle Server|Client)' }} |
  ForEach-Object {{ Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue }}
"""
    subprocess.run(["powershell", "-NoProfile", "-Command", ps], capture_output=True, text=True)
    time.sleep(1.0)


def _start_python_console(args: list[str]) -> subprocess.Popen:
    """Visible console without fragile cmd.exe quoting."""
    env = os.environ.copy()
    env["PYTHONUNBUFFERED"] = "1"
    flags = CREATE_NEW_CONSOLE if sys.platform == "win32" else 0
    return subprocess.Popen(
        [PYTHON, "-u", *args],
        cwd=str(ROOT),
        env=env,
        creationflags=flags,
    )


def _start_mupen() -> subprocess.Popen:
    return subprocess.Popen(
        [str(MUPEN), "--nogui", "snowboardkids.z64"],
        cwd=str(PLAY),
    )


def _pid_alive_mupen(pid: int) -> bool:
    if sys.platform != "win32":
        return True
    try:
        out = subprocess.check_output(
            ["tasklist", "/FI", f"PID eq {pid}", "/FO", "CSV", "/NH"],
            text=True,
            errors="replace",
        )
    except Exception:
        return False
    return str(pid) in out and "mupen64plus" in out.lower()


def main() -> None:
    parser = argparse.ArgumentParser(description="Local 2P Online Battle quick test")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=7777)
    parser.add_argument(
        "--no-auto",
        action="store_true",
        help="Do not auto-ready / auto-start (manual lobby buttons)",
    )
    parser.add_argument(
        "--skip-copy-rom",
        action="store_true",
        help="Do not copy build/shiftable ROM into dist/play",
    )
    args = parser.parse_args()

    if not MUPEN.is_file():
        raise SystemExit(f"missing Mupen: {MUPEN} (build/wrap the ROM first)")
    if not args.skip_copy_rom and ROM_BUILD.is_file():
        PLAY.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROM_BUILD, ROM_PLAY)
        print(f"copied {ROM_BUILD} -> {ROM_PLAY}")

    print("stopping previous online/mupen processes...")
    _kill_stale()

    for path in (ROOT / "tmp" / "online_mailbox_p1.bin", ROOT / "tmp" / "online_mailbox_p2.bin"):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.unlink(missing_ok=True)

    print("starting server...")
    server = _start_python_console(
        ["tools/online_server.py", "--host", args.host, "--port", str(args.port)]
    )
    time.sleep(1.0)
    if server.poll() is not None:
        raise SystemExit("server exited immediately")

    print("starting Mupen P1/P2...")
    mupen1 = _start_mupen()
    time.sleep(1.2)
    mupen2 = _start_mupen()
    time.sleep(1.5)
    p1 = mupen1.pid
    p2 = mupen2.pid
    if mupen1.poll() is not None or not _pid_alive_mupen(p1):
        raise SystemExit(f"Mupen P1 exited early (pid={p1})")
    if mupen2.poll() is not None or not _pid_alive_mupen(p2):
        raise SystemExit(f"Mupen P2 exited early (pid={p2})")
    print(f"P1 Mupen pid={p1}  P2 Mupen pid={p2}")

    p1_args = [
        "tools/online_client.py",
        "--host",
        args.host,
        "--port",
        str(args.port),
        "--mailbox",
        "tmp/online_mailbox_p1.bin",
        "--mupen-bridge",
        "--mupen-pid",
        str(p1),
    ]
    p2_args = [
        "tools/online_client.py",
        "--host",
        args.host,
        "--port",
        str(args.port),
        "--mailbox",
        "tmp/online_mailbox_p2.bin",
        "--mupen-bridge",
        "--mupen-pid",
        str(p2),
    ]
    if not args.no_auto:
        p1_args += ["--auto-ready", "--auto-start"]
        p2_args += ["--auto-ready"]

    print("starting clients pinned to those PIDs...")
    c1 = _start_python_console(p1_args)
    time.sleep(0.4)
    c2 = _start_python_console(p2_args)
    time.sleep(4.0)
    if c1.poll() is not None or c2.poll() is not None:
        raise SystemExit(f"client exited early (p1={c1.poll()} p2={c2.poll()})")

    print()
    print("2P stack is up.")
    print(f"  server pid={server.pid}  clientP1={c1.pid}  clientP2={c2.pid}")
    print("  Autotest ROM boots straight into ONLINE LOBBY (Slash / course 0).")
    if args.no_auto:
        print("  Manual: Enter to ready on each window, host Start to launch.")
    else:
        print("  Auto: both READY; host starts when both humans are ready.")
    print("  Client consoles should show: attached to Mupen pid=... (pinned)")
    print("  Server console should show: JOINED seat=0 and JOINED seat=1")


if __name__ == "__main__":
    main()
