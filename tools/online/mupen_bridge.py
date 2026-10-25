"""Sync N64 RDRAM gOnlineMailbox <-> host mailbox file for Mupen64Plus (Windows).

Finds the emulated mailbox by scanning the Mupen process for the ONLB magic
(handling Mupen's host-endian RDRAM word storage), then merges game-owned and
bridge-owned fields each tick so tools/online_client.py can speak UDP.
"""

from __future__ import annotations

import ctypes
import struct
import sys
import time
from ctypes import wintypes
from pathlib import Path
from typing import List, Optional, Tuple

from tools.online.mailbox_file import (
    EVENT_QUEUE,
    EVENT_SIZE,
    MAILBOX_MAGIC,
    MAILBOX_SIZE,
    OFF_BRIDGE_CMD,
    OFF_BRIDGE_HEARTBEAT,
    OFF_CPU_PATH_IN,
    OFF_CPU_PATH_OUT,
    OFF_ERROR,
    OFF_EVENT_IN,
    OFF_EVENT_IN_COUNT,
    OFF_EVENT_OUT,
    OFF_EVENT_OUT_COUNT,
    OFF_GAME_CMD,
    OFF_GAME_HEARTBEAT,
    OFF_LOBBY,
    LOBBY_SEAT_SIZE,
    OFF_LOCAL_OUT,
    OFF_PENDING_LOADOUT,
    OFF_REMOTE_IN,
    OFF_VERSION,
    MailboxFile,
)
from tools.online.protocol import VERSION

# N64 virtual address of gOnlineMailbox in the current shiftable Online Battle build
# (RDRAM offset = addr - 0x80000000). Used as a scan hint after magic is found once.
DEFAULT_MAILBOX_VRAM = 0x801181E0

PROCESS_QUERY_INFORMATION = 0x0400
PROCESS_VM_READ = 0x0010
PROCESS_VM_WRITE = 0x0020
PROCESS_VM_OPERATION = 0x0008
MEM_COMMIT = 0x1000
PAGE_READABLE = (0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80)

# ONLB as bytes in N64/BE view and as stored in LE uint32 RDRAM words.
MAGIC_BE = struct.pack(">I", MAILBOX_MAGIC)  # 4F 4E 4C 42
MAGIC_LE_WORDS = struct.pack("<I", MAILBOX_MAGIC)  # 42 4C 4E 4F


class MEMORY_BASIC_INFORMATION(ctypes.Structure):
    _fields_ = [
        ("BaseAddress", ctypes.c_void_p),
        ("AllocationBase", ctypes.c_void_p),
        ("AllocationProtect", wintypes.DWORD),
        ("RegionSize", ctypes.c_size_t),
        ("State", wintypes.DWORD),
        ("Protect", wintypes.DWORD),
        ("Type", wintypes.DWORD),
    ]


kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
kernel32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
kernel32.OpenProcess.restype = wintypes.HANDLE
kernel32.ReadProcessMemory.argtypes = [
    wintypes.HANDLE,
    wintypes.LPVOID,
    wintypes.LPVOID,
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_size_t),
]
kernel32.ReadProcessMemory.restype = wintypes.BOOL
kernel32.WriteProcessMemory.argtypes = [
    wintypes.HANDLE,
    wintypes.LPVOID,
    wintypes.LPVOID,
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_size_t),
]
kernel32.WriteProcessMemory.restype = wintypes.BOOL
kernel32.VirtualQueryEx.argtypes = [
    wintypes.HANDLE,
    wintypes.LPCVOID,
    ctypes.POINTER(MEMORY_BASIC_INFORMATION),
    ctypes.c_size_t,
]
kernel32.VirtualQueryEx.restype = ctypes.c_size_t
kernel32.CloseHandle.argtypes = [wintypes.HANDLE]
kernel32.CloseHandle.restype = wintypes.BOOL


def log(msg: str) -> None:
    stamp = time.strftime("%H:%M:%S")
    print(f"[bridge {stamp}] {msg}", flush=True)


def swap_words(data: bytes) -> bytearray:
    """Byte-swap every 4-byte word (N64 BE view <-> LE uint32 RDRAM storage)."""
    out = bytearray(len(data))
    for i in range(0, len(data) - 3, 4):
        out[i] = data[i + 3]
        out[i + 1] = data[i + 2]
        out[i + 2] = data[i + 1]
        out[i + 3] = data[i]
    rem = len(data) % 4
    if rem:
        out[-rem:] = data[-rem:]
    return out


MUPEN_IMAGE_NAMES = (
    "mupen64plus-ui-console.exe",
    "mupen64plus.exe",
)


def list_mupen_pids() -> List[int]:
    pids: List[int] = []
    try:
        import subprocess

        for image in MUPEN_IMAGE_NAMES:
            out = subprocess.check_output(
                ["tasklist", "/FI", f"IMAGENAME eq {image}", "/FO", "CSV", "/NH"],
                text=True,
                errors="replace",
            )
            for line in out.splitlines():
                line = line.strip()
                if not line or line.startswith("INFO:"):
                    continue
                # "mupen64plus-ui-console.exe","1234","Session Name","0","12,345 K"
                parts = line.split('","')
                if len(parts) >= 2:
                    try:
                        pids.append(int(parts[1].strip('"')))
                    except ValueError:
                        pass
    except Exception as exc:
        log(f"tasklist failed: {exc}")
    # Preserve order but drop duplicates
    return list(dict.fromkeys(pids))


def validate_mailbox_be(buf: bytes) -> bool:
    if len(buf) < 8:
        return False
    magic, version = struct.unpack_from(">IH", buf, 0)
    return magic == MAILBOX_MAGIC and version == VERSION


class MupenBridge:
    def __init__(
        self,
        mailbox_path: Path,
        mailbox_vram: int = DEFAULT_MAILBOX_VRAM,
        preferred_pid: Optional[int] = None,
    ) -> None:
        self.mailbox = MailboxFile(mailbox_path)
        self.mailbox_vram = mailbox_vram
        self.preferred_pid = preferred_pid
        self.pid: Optional[int] = None
        self.handle: Optional[int] = None
        self.host_addr: Optional[int] = None
        self.word_swapped = True  # Mupen stores RDRAM as host-endian words
        self._rdram_candidates: List[int] = []  # host bases that look like RDRAM
        self._last_scan = 0.0
        self._last_wait_log = 0.0
        self._syncs = 0
        self._last_status = 0.0

    def close(self) -> None:
        if self.handle:
            kernel32.CloseHandle(self.handle)
            self.handle = None

    def _open(self, pid: int) -> bool:
        access = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION
        handle = kernel32.OpenProcess(access, False, pid)
        if not handle:
            return False
        self.close()
        self.pid = pid
        self.handle = handle
        self.host_addr = None
        return True

    def _ensure_process(self) -> bool:
        if self.handle and self.pid:
            # Still alive? OpenProcess succeeds only while the process exists.
            test = kernel32.OpenProcess(PROCESS_QUERY_INFORMATION, False, self.pid)
            if test:
                kernel32.CloseHandle(test)
                return True
            self.close()
            self.pid = None
            self.host_addr = None
        pids = list_mupen_pids()
        if self.preferred_pid is not None:
            if self.preferred_pid not in pids:
                now = time.time()
                if now - self._last_wait_log >= 2.0:
                    self._last_wait_log = now
                    log(
                        f"pinned Mupen pid={self.preferred_pid} not running "
                        f"(alive mupens={pids}) — will offline-fallback in-game"
                    )
                return False
            if self._open(self.preferred_pid):
                log(f"attached to Mupen pid={self.preferred_pid} (pinned)")
                return True
            log(f"OpenProcess failed for pinned pid={self.preferred_pid}")
            return False
        if not pids:
            return False
        # Prefer the most recently started (highest pid is a weak heuristic).
        for pid in sorted(pids, reverse=True):
            if self._open(pid):
                log(f"attached to Mupen pid={pid}")
                return True
        return False

    def _read(self, addr: int, size: int) -> Optional[bytes]:
        if not self.handle:
            return None
        buf = (ctypes.c_ubyte * size)()
        read = ctypes.c_size_t(0)
        ok = kernel32.ReadProcessMemory(self.handle, ctypes.c_void_p(addr), buf, size, ctypes.byref(read))
        if not ok or read.value != size:
            return None
        return bytes(buf)

    def _write(self, addr: int, data: bytes) -> bool:
        if not self.handle:
            return False
        buf = (ctypes.c_ubyte * len(data)).from_buffer_copy(data)
        written = ctypes.c_size_t(0)
        ok = kernel32.WriteProcessMemory(
            self.handle, ctypes.c_void_p(addr), buf, len(data), ctypes.byref(written)
        )
        return bool(ok and written.value == len(data))

    def _regions(self) -> List[Tuple[int, int]]:
        regions: List[Tuple[int, int]] = []
        if not self.handle:
            return regions
        address = 0
        mbi = MEMORY_BASIC_INFORMATION()
        max_addr = 0x7FFFFFFFFFFF if ctypes.sizeof(ctypes.c_void_p) == 8 else 0x7FFFFFFF
        while address < max_addr:
            got = kernel32.VirtualQueryEx(self.handle, ctypes.c_void_p(address), ctypes.byref(mbi), ctypes.sizeof(mbi))
            if not got:
                break
            base = mbi.BaseAddress or 0
            size = mbi.RegionSize or 0
            if mbi.State == MEM_COMMIT and (mbi.Protect & 0xFF) in PAGE_READABLE and size >= MAILBOX_SIZE:
                regions.append((base, size))
            next_addr = base + size
            if next_addr <= address:
                break
            address = next_addr
        return regions

    def _try_addr(self, abs_addr: int, swapped: bool) -> bool:
        raw = self._read(abs_addr, MAILBOX_SIZE)
        if not raw:
            return False
        view = swap_words(raw) if swapped else bytearray(raw)
        if not validate_mailbox_be(view):
            return False
        self.host_addr = abs_addr
        self.word_swapped = swapped
        log(f"found gOnlineMailbox at host 0x{abs_addr:X} (word_swapped={swapped})")
        return True

    def _collect_rdram_candidates(self) -> None:
        """Remember host addresses that may be the start of emulated RDRAM."""
        rdram_off = self.mailbox_vram - 0x80000000
        found: List[int] = []
        for base, size in self._regions():
            if size < 0x400000:
                continue
            # Mupen sometimes places RDRAM at region base, sometimes +0x1000..+0x8000.
            for pad in range(0, 0x9000, 0x1000):
                if pad + 0x800000 <= size or pad + rdram_off + MAILBOX_SIZE <= size:
                    found.append(base + pad)
        self._rdram_candidates = list(dict.fromkeys(found))

    def _probe_candidates(self) -> bool:
        rdram_off = self.mailbox_vram - 0x80000000
        for base in self._rdram_candidates:
            for swapped in (True, False):
                if self._try_addr(base + rdram_off, swapped):
                    return True
        return False

    def _scan_for_mailbox(self) -> bool:
        if not self.handle:
            return False

        # Cheap path every tick: probe known RDRAM bases at gOnlineMailbox offset.
        # Magic appears only after onlineMailboxInit (Online Battle entry).
        if not self._rdram_candidates:
            self._collect_rdram_candidates()
        if self._probe_candidates():
            return True

        now = time.time()
        if now - self._last_wait_log >= 3.0:
            self._last_wait_log = now
            log(
                f"waiting for ONLB magic (enter Online Battle); "
                f"rdram_candidates={len(self._rdram_candidates)}"
            )

        # Occasional full magic scan (covers unusual RDRAM layouts).
        if now - self._last_scan < 2.0:
            return False
        self._last_scan = now
        self._collect_rdram_candidates()
        if self._probe_candidates():
            return True

        regions = self._regions()
        log(f"scanning {len(regions)} regions for ONLB magic...")
        for base, size in regions:
            chunk_size = min(size, 8 * 1024 * 1024)
            offset = 0
            while offset + MAILBOX_SIZE <= size and offset < chunk_size:
                read_size = min(1024 * 1024, size - offset)
                data = self._read(base + offset, read_size)
                if not data:
                    break
                for magic, swapped in ((MAGIC_LE_WORDS, True), (MAGIC_BE, False)):
                    start = 0
                    while True:
                        idx = data.find(magic, start)
                        if idx < 0:
                            break
                        if self._try_addr(base + offset + idx, swapped):
                            return True
                        start = idx + 1
                offset += read_size
        return False

    def _read_mailbox_be(self) -> Optional[bytearray]:
        if self.host_addr is None:
            return None
        raw = self._read(self.host_addr, MAILBOX_SIZE)
        if not raw:
            self.host_addr = None
            return None
        view = swap_words(raw) if self.word_swapped else bytearray(raw)
        if not validate_mailbox_be(view):
            self.host_addr = None
            return None
        return view

    def _write_mailbox_be(self, be_buf: bytearray) -> bool:
        if self.host_addr is None:
            return False
        raw = swap_words(be_buf) if self.word_swapped else bytes(be_buf)
        return self._write(self.host_addr, bytes(raw))

    def sync(self) -> bool:
        """Merge RDRAM mailbox with the host mailbox file. Returns True if synced."""
        if sys.platform != "win32":
            return False
        if not self._ensure_process():
            return False
        if self.host_addr is None and not self._scan_for_mailbox():
            return False

        rdram = self._read_mailbox_be()
        if rdram is None:
            return False

        self.mailbox.path.parent.mkdir(parents=True, exist_ok=True)
        if not self.mailbox.path.is_file():
            self.mailbox.init()
        host = self.mailbox._read()

        # Ensure host file has magic/version.
        struct.pack_into(">I", host, 0, MAILBOX_MAGIC)
        struct.pack_into(">H", host, OFF_VERSION, VERSION)

        # --- One-shot command forwarding ---
        # Game -> client: copy then clear in RDRAM so it is not re-sent forever.
        if rdram[OFF_GAME_CMD] != 0:
            if host[OFF_GAME_CMD] == 0:
                host[OFF_GAME_CMD] = rdram[OFF_GAME_CMD]
                rdram[OFF_GAME_CMD] = 0
                log(f"game cmd -> client: {host[OFF_GAME_CMD]}")
            elif host[OFF_GAME_CMD] == rdram[OFF_GAME_CMD]:
                # Already delivered to file; clear RDRAM copy.
                rdram[OFF_GAME_CMD] = 0
            else:
                # File still holds an unread cmd; keep the newer game cmd in RDRAM.
                pass

        # Client -> game: deliver then clear in file.
        # Track clears so the fresh-file re-merge below cannot resurrect the same
        # one-shot (RACE_GO was sticky forever: deliver → clear host buf → fresh
        # read still had 16 on disk → rewrite 16 → re-deliver every tick).
        bridge_cmd_cleared = False
        if host[OFF_BRIDGE_CMD] != 0 and rdram[OFF_BRIDGE_CMD] == 0:
            rdram[OFF_BRIDGE_CMD] = host[OFF_BRIDGE_CMD]
            log(f"client cmd -> game: {rdram[OFF_BRIDGE_CMD]}")
            host[OFF_BRIDGE_CMD] = 0
            bridge_cmd_cleared = True
        elif host[OFF_BRIDGE_CMD] != 0 and rdram[OFF_BRIDGE_CMD] == host[OFF_BRIDGE_CMD]:
            host[OFF_BRIDGE_CMD] = 0
            bridge_cmd_cleared = True

        # Continuous game-owned fields -> file
        # OnlineLoadout is 0x10 (course/char/… + 8-byte name); copying only 8
        # dropped the name so SET_LOADOUT always shipped blanks → lobby "P#".
        host[OFF_PENDING_LOADOUT:OFF_LOCAL_OUT] = rdram[OFF_PENDING_LOADOUT:OFF_LOCAL_OUT]
        host[OFF_LOCAL_OUT : OFF_REMOTE_IN] = rdram[OFF_LOCAL_OUT : OFF_REMOTE_IN]
        host[OFF_CPU_PATH_OUT:OFF_CPU_PATH_IN] = rdram[OFF_CPU_PATH_OUT:OFF_CPU_PATH_IN]
        # One-shot eventOut: append into the file queue, then clear what we took
        # from RDRAM. Never replace an undrained batch (that dropped ITEM_USE).
        rdram_ev = int(rdram[OFF_EVENT_OUT_COUNT])
        host_ev = int(host[OFF_EVENT_OUT_COUNT])
        if rdram_ev > 0:
            space = EVENT_QUEUE - host_ev
            take = rdram_ev if space >= rdram_ev else max(0, space)
            if take > 0:
                src = OFF_EVENT_OUT
                dst = OFF_EVENT_OUT + host_ev * EVENT_SIZE
                nbytes = take * EVENT_SIZE
                host[dst : dst + nbytes] = rdram[src : src + nbytes]
                host[OFF_EVENT_OUT_COUNT] = host_ev + take
            # Drop taken events from RDRAM (shift remainder down).
            left = rdram_ev - take
            if left > 0 and take > 0:
                rem = left * EVENT_SIZE
                rdram[OFF_EVENT_OUT : OFF_EVENT_OUT + rem] = rdram[
                    OFF_EVENT_OUT + take * EVENT_SIZE : OFF_EVENT_OUT + take * EVENT_SIZE + rem
                ]
            rdram[OFF_EVENT_OUT_COUNT] = left
        host[OFF_GAME_HEARTBEAT : OFF_GAME_HEARTBEAT + 4] = rdram[OFF_GAME_HEARTBEAT : OFF_GAME_HEARTBEAT + 4]

        # Lobby: host wins only after the net client has real server state.
        # Otherwise keep the game's CONNECTING/offline lobby and mirror it to the file.
        # Preserve optimistic local ready bit until the server snapshot catches up.
        host_phase = host[OFF_LOBBY]
        game_phase = rdram[OFF_LOBBY]
        local_seat = rdram[OFF_LOBBY + 1]
        local_ready_opt = 0
        local_name_opt = b""
        if local_seat < 4:
            seat_off = OFF_LOBBY + 12 + int(local_seat) * LOBBY_SEAT_SIZE
            local_ready_opt = rdram[seat_off + 1]
            # loadout.name is 8 bytes at seat+4+8
            local_name_opt = bytes(rdram[seat_off + 12 : seat_off + 20])
        if host_phase >= 2:  # LOBBY / COUNTDOWN / RACING / ERROR
            rdram[OFF_LOBBY:OFF_PENDING_LOADOUT] = host[OFF_LOBBY:OFF_PENDING_LOADOUT]
            if local_seat < 4:
                seat_off = OFF_LOBBY + 12 + int(local_seat) * LOBBY_SEAT_SIZE
                if local_ready_opt and host[seat_off + 1] == 0:
                    rdram[seat_off + 1] = 1
                # Keep optimistic local name until server SET_LOADOUT catches up.
                host_name = bytes(rdram[seat_off + 12 : seat_off + 20])
                if local_name_opt.strip() and not host_name.strip():
                    rdram[seat_off + 12 : seat_off + 20] = local_name_opt
        else:
            host[OFF_LOBBY:OFF_PENDING_LOADOUT] = rdram[OFF_LOBBY:OFF_PENDING_LOADOUT]

        # Continuous bridge-owned fields -> RDRAM
        rdram[OFF_ERROR] = host[OFF_ERROR]
        rdram[OFF_REMOTE_IN:OFF_CPU_PATH_OUT] = host[OFF_REMOTE_IN:OFF_CPU_PATH_OUT]
        rdram[OFF_CPU_PATH_IN:OFF_EVENT_OUT_COUNT] = host[OFF_CPU_PATH_IN:OFF_EVENT_OUT_COUNT]
        # One-shot eventIn: deliver when the game has drained the previous batch.
        if rdram[OFF_EVENT_IN_COUNT] == 0 and host[OFF_EVENT_IN_COUNT] != 0:
            rdram[OFF_EVENT_IN_COUNT] = host[OFF_EVENT_IN_COUNT]
            rdram[OFF_EVENT_IN:OFF_BRIDGE_HEARTBEAT] = host[OFF_EVENT_IN:OFF_BRIDGE_HEARTBEAT]
            host[OFF_EVENT_IN_COUNT] = 0
        # Heartbeat: prove bridge is alive (must be non-zero before offline fallback at ~90 frames)
        hb = struct.unpack_from(">I", host, OFF_BRIDGE_HEARTBEAT)[0] + 1
        struct.pack_into(">I", host, OFF_BRIDGE_HEARTBEAT, hb & 0xFFFFFFFF)
        rdram[OFF_BRIDGE_HEARTBEAT : OFF_BRIDGE_HEARTBEAT + 4] = host[
            OFF_BRIDGE_HEARTBEAT : OFF_BRIDGE_HEARTBEAT + 4
        ]
        _ = game_phase  # retained for status logging clarity

        # Re-merge inbound fields before writing the file. handle_packet may have
        # written remote/path/event/bridge-cmd into the mailbox while we held a
        # stale `host` buffer — a blind _write would wipe those and freeze remotes.
        fresh = self.mailbox._read()
        host[OFF_REMOTE_IN:OFF_CPU_PATH_OUT] = fresh[OFF_REMOTE_IN:OFF_CPU_PATH_OUT]
        host[OFF_CPU_PATH_IN:OFF_EVENT_OUT_COUNT] = fresh[OFF_CPU_PATH_IN:OFF_EVENT_OUT_COUNT]
        # NOTE: do not re-merge eventIn here. Events are one-shot: the file copy
        # is the same batch delivery above already consumed in-memory, so copying
        # it back resurrects it and the game re-applies it every drain (item spam).
        if fresh[OFF_BRIDGE_CMD] != 0 and not bridge_cmd_cleared:
            host[OFF_BRIDGE_CMD] = fresh[OFF_BRIDGE_CMD]
            host[OFF_ERROR] = fresh[OFF_ERROR]
        elif bridge_cmd_cleared:
            # Keep the in-memory clear; drop any stale on-disk copy.
            host[OFF_BRIDGE_CMD] = 0
        # Lobby/phase from net client (JOINED/START/GO) must not be rolled back.
        if fresh[OFF_LOBBY] >= 2:
            host[OFF_LOBBY:OFF_PENDING_LOADOUT] = fresh[OFF_LOBBY:OFF_PENDING_LOADOUT]
            rdram[OFF_LOBBY:OFF_PENDING_LOADOUT] = fresh[OFF_LOBBY:OFF_PENDING_LOADOUT]
        # Push the freshest remotes into RDRAM too (same race window).
        rdram[OFF_REMOTE_IN:OFF_CPU_PATH_OUT] = host[OFF_REMOTE_IN:OFF_CPU_PATH_OUT]
        rdram[OFF_CPU_PATH_IN:OFF_EVENT_OUT_COUNT] = host[OFF_CPU_PATH_IN:OFF_EVENT_OUT_COUNT]
        rdram[OFF_ERROR] = host[OFF_ERROR]
        if (not bridge_cmd_cleared) and host[OFF_BRIDGE_CMD] != 0 and rdram[OFF_BRIDGE_CMD] == 0:
            rdram[OFF_BRIDGE_CMD] = host[OFF_BRIDGE_CMD]
            host[OFF_BRIDGE_CMD] = 0

        self.mailbox._write(host)
        if not self._write_mailbox_be(rdram):
            self.host_addr = None
            return False

        self._syncs += 1
        now = time.time()
        if now - self._last_status >= 2.0:
            self._last_status = now
            log(
                f"sync ok count={self._syncs} hostPhase={host[OFF_LOBBY]} gamePhase={rdram[OFF_LOBBY]} "
                f"gameCmd={host[OFF_GAME_CMD]} bridgeCmd={rdram[OFF_BRIDGE_CMD]} hb={hb & 0xFFFFFFFF}"
            )
        return True


def main() -> None:
    if sys.platform != "win32":
        print("mupen bridge is Windows-only for now", file=sys.stderr)
        raise SystemExit(1)

    import argparse

    parser = argparse.ArgumentParser(description="Mupen64Plus <-> Online mailbox file bridge")
    parser.add_argument("--mailbox", type=Path, default=Path("tmp/online_mailbox.bin"))
    parser.add_argument("--hz", type=float, default=60.0)
    args = parser.parse_args()

    bridge = MupenBridge(args.mailbox)
    log(f"watching Mupen; mailbox file={args.mailbox}")
    try:
        while True:
            bridge.sync()
            time.sleep(1.0 / max(args.hz, 1.0))
    except KeyboardInterrupt:
        log("stopped")
    finally:
        bridge.close()


if __name__ == "__main__":
    # Allow `python tools/online/mupen_bridge.py` from repo root.
    if __package__ is None:
        root = Path(__file__).resolve().parents[2]
        if str(root) not in sys.path:
            sys.path.insert(0, str(root))
    main()
