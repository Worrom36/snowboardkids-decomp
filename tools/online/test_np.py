#!/usr/bin/env python3
"""Launch a local N-player Online Battle test stack.

Prereq (autotest ROM that boots straight into the lobby):
  make BUILD_MODE=shiftable ONLINE_BATTLE=1 ONLINE_BATTLE_AUTOTEST=1

Then:
  python tools/online_test_2p.py
  python tools/online_test_4p.py
  python tools/online_test_np.py --players 3
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
MAX_PLAYERS = 4


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
    parser = argparse.ArgumentParser(description="Local N-player Online Battle quick test")
    parser.add_argument("--players", type=int, default=2, choices=range(1, MAX_PLAYERS + 1))
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
    n = int(args.players)

    if not MUPEN.is_file():
        raise SystemExit(f"missing Mupen: {MUPEN} (build/wrap the ROM first)")
    if not args.skip_copy_rom and ROM_BUILD.is_file():
        PLAY.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROM_BUILD, ROM_PLAY)
        print(f"copied {ROM_BUILD} -> {ROM_PLAY}")

    print("stopping previous online/mupen processes...")
    _kill_stale()

    for i in range(n):
        path = ROOT / "tmp" / f"online_mailbox_p{i + 1}.bin"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.unlink(missing_ok=True)

    print("starting server...")
    server = _start_python_console(
        ["tools/online_server.py", "--host", args.host, "--port", str(args.port)]
    )
    time.sleep(1.0)
    if server.poll() is not None:
        raise SystemExit("server exited immediately")

    print(f"starting {n} Mupen windows...")
    mupens: list[subprocess.Popen] = []
    for i in range(n):
        mupens.append(_start_mupen())
        time.sleep(1.2 if i == 0 else 1.0)

    mupen_pids: list[int] = []
    for i, proc in enumerate(mupens):
        pid = proc.pid
        if proc.poll() is not None or not _pid_alive_mupen(pid):
            raise SystemExit(f"Mupen P{i + 1} exited early (pid={pid})")
        mupen_pids.append(pid)
    print("  " + "  ".join(f"P{i + 1} pid={pid}" for i, pid in enumerate(mupen_pids)))

    clients: list[subprocess.Popen] = []
    print("starting clients pinned to those PIDs...")
    for i, pid in enumerate(mupen_pids):
        client_args = [
            "tools/online_client.py",
            "--host",
            args.host,
            "--port",
            str(args.port),
            "--mailbox",
            f"tmp/online_mailbox_p{i + 1}.bin",
            "--mupen-bridge",
            "--mupen-pid",
            str(pid),
        ]
        if not args.no_auto:
            client_args += ["--auto-ready", "--min-players", str(n)]
            if i == 0:
                client_args.append("--auto-start")
        clients.append(_start_python_console(client_args))
        time.sleep(0.5)

    # Give slower Mupen/bridge attaches time to JOIN before the host can start.
    time.sleep(2.0 + 0.75 * n)
    for i, client in enumerate(clients):
        if client.poll() is not None:
            raise SystemExit(f"client P{i + 1} exited early ({client.poll()})")

    print()
    print(f"{n}P stack is up.")
    print(f"  server pid={server.pid}  clients={[c.pid for c in clients]}")
    print("  Autotest ROM boots straight into ONLINE LOBBY (Slash / course 0).")
    if args.no_auto:
        print("  Manual: Enter to ready on each window, host Start to launch.")
    else:
        print(f"  Auto: all {n} READY; host starts when all humans are ready.")
    print("  Client consoles should show: attached to Mupen pid=... (pinned)")
    print(
        "  Server console should show: "
        + " and ".join(f"JOINED seat={i}" for i in range(n))
    )


if __name__ == "__main__":
    main()
