#!/usr/bin/env python3
"""Launch a local N-player Online Battle test stack.

Prereq (normal Online Battle ROM — use the in-game Online Battle menu):
  make BUILD_MODE=shiftable ONLINE_BATTLE=1

Optional fast-boot (skips title/menus; not for shipping builds):
  make BUILD_MODE=shiftable ONLINE_BATTLE=1 ONLINE_BATTLE_AUTOTEST=1

ONLINE_BATTLE_AUTOTEST also enables autopilot for each local human so the
N-P stack races hands-off (toggle with R still works in-game).

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


def _find_main_hwnd(pid: int):
    """Return the largest visible top-level window owned by pid, or None."""
    import ctypes
    from ctypes import wintypes

    user32 = ctypes.WinDLL("user32", use_last_error=True)
    WNDENUMPROC = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)

    best = [None, 0]  # hwnd, area

    @WNDENUMPROC
    def _enum(hwnd, _lparam):
        proc_id = wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(proc_id))
        if proc_id.value != pid:
            return True
        if not user32.IsWindowVisible(hwnd):
            return True
        if user32.GetWindow(hwnd, 4):  # GW_OWNER — skip owned popups
            return True
        rect = wintypes.RECT()
        if not user32.GetWindowRect(hwnd, ctypes.byref(rect)):
            return True
        area = max(0, rect.right - rect.left) * max(0, rect.bottom - rect.top)
        if area > best[1]:
            best[0] = hwnd
            best[1] = area
        return True

    user32.EnumWindows(_enum, 0)
    return best[0]


def _tile_mupen_windows(pids: list[int]) -> None:
    """Place Mupen windows in a row at native size, centered in the work area."""
    if sys.platform != "win32" or not pids:
        return
    import ctypes
    from ctypes import wintypes

    user32 = ctypes.WinDLL("user32", use_last_error=True)
    SPI_GETWORKAREA = 0x0030
    SW_RESTORE = 9
    SWP_NOSIZE = 0x0001
    SWP_SHOWWINDOW = 0x0040

    work = wintypes.RECT()
    if not user32.SystemParametersInfoW(SPI_GETWORKAREA, 0, ctypes.byref(work), 0):
        return
    work_w = max(1, work.right - work.left)
    work_h = max(1, work.bottom - work.top)

    hwnds: list = []
    for _attempt in range(20):
        hwnds = [_find_main_hwnd(pid) for pid in pids]
        if all(hwnds):
            break
        time.sleep(0.25)
    if not all(hwnds):
        missing = [i + 1 for i, h in enumerate(hwnds) if not h]
        print(f"  (could not tile Mupen window(s) for P{missing})")
        return

    sizes: list[tuple[int, int]] = []
    for hwnd in hwnds:
        user32.ShowWindow(hwnd, SW_RESTORE)
        rect = wintypes.RECT()
        user32.GetWindowRect(hwnd, ctypes.byref(rect))
        sizes.append((max(1, rect.right - rect.left), max(1, rect.bottom - rect.top)))

    gap = 8
    total_w = sum(w for w, _h in sizes) + gap * (len(sizes) - 1)
    max_h = max(h for _w, h in sizes)
    origin_x = work.left + max(0, (work_w - total_w) // 2)
    origin_y = work.top + max(0, (work_h - max_h) // 2)

    x = origin_x
    for hwnd, (w, _h) in zip(hwnds, sizes):
        user32.SetWindowPos(hwnd, 0, x, origin_y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW)
        x += w + gap
    print(f"  placed {len(pids)} Mupen windows in a centered row (native size)")


def main() -> None:
    parser = argparse.ArgumentParser(description="Local N-player Online Battle quick test")
    parser.add_argument("--players", type=int, default=2, choices=range(1, MAX_PLAYERS + 1))
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=6464)
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
    _tile_mupen_windows(mupen_pids)

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
    print("  Autotest ROM boots straight into ONLINE LOBBY (random character / course 0).")
    print("  Autopilot ON for each local human (ONLINE_BATTLE_AUTOTEST); R toggles.")
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
