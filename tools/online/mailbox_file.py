"""File-backed OnlineMailbox used by the Python bridge / loopback tests.

The C Mupen plugin is expected to mirror the same binary layout into RDRAM
`gOnlineMailbox`. Until that plugin is loaded, the net client uses this file.
"""

from __future__ import annotations

import struct
from pathlib import Path
from typing import List, Optional

from tools.online.protocol import (
    VERSION,
    Cmd,
    CpuPathSnapshot,
    Loadout,
    LobbySnapshot,
    RaceEvent,
    RacerSnapshot,
)

MAILBOX_MAGIC = 0x4F4E4C42  # 'ONLB'
EVENT_QUEUE = 8

# Offsets matching include/game/online/online_mailbox.h
OFF_MAGIC = 0x000
OFF_VERSION = 0x004
OFF_FLAGS = 0x006
OFF_GAME_CMD = 0x008
OFF_BRIDGE_CMD = 0x009
OFF_ERROR = 0x00A
OFF_LOBBY = 0x00C
OFF_PENDING_LOADOUT = 0x048
OFF_LOCAL_OUT = 0x050  # authorityOut[0]; array of ONLINE_MAX_SEATS
OFF_AUTHORITY_OUT = 0x050
OFF_REMOTE_IN = 0x190
OFF_CPU_PATH_OUT = 0x2D0
OFF_CPU_PATH_IN = 0x380
OFF_EVENT_OUT_COUNT = 0x430
OFF_EVENT_IN_COUNT = 0x431
OFF_EVENT_OUT = 0x434
OFF_EVENT_IN = 0x474
OFF_BRIDGE_HEARTBEAT = 0x4B4
OFF_GAME_HEARTBEAT = 0x4B8
MAILBOX_SIZE = 0x4BC
MAX_SEATS = 4

RACER_SIZE = RacerSnapshot._STRUCT.size
CPU_PATH_SIZE = CpuPathSnapshot._STRUCT.size
EVENT_SIZE = RaceEvent._STRUCT.size
LOBBY_SIZE = 12 + 4 * (4 + Loadout._STRUCT.size)


class MailboxFile:
    def __init__(self, path: Path) -> None:
        self.path = path
        # Last accepted remote snapshot sequence per seat (UDP reorder guard).
        self._remote_seq: List[int] = [-1, -1, -1, -1]
        self._cpu_path_seq: List[int] = [-1, -1, -1, -1]

    def init(self) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True)
        buf = bytearray(MAILBOX_SIZE)
        struct.pack_into(">I", buf, OFF_MAGIC, MAILBOX_MAGIC)
        struct.pack_into(">H", buf, OFF_VERSION, VERSION)
        self.path.write_bytes(buf)
        self._remote_seq = [-1, -1, -1, -1]
        self._cpu_path_seq = [-1, -1, -1, -1]

    def _read(self) -> bytearray:
        if not self.path.is_file():
            self.init()
        data = bytearray(self.path.read_bytes())
        if len(data) < MAILBOX_SIZE:
            data.extend(b"\x00" * (MAILBOX_SIZE - len(data)))
        return data

    def _write(self, buf: bytearray) -> None:
        self.path.write_bytes(buf)

    def bump_bridge_heartbeat(self) -> None:
        buf = self._read()
        value = struct.unpack_from(">I", buf, OFF_BRIDGE_HEARTBEAT)[0] + 1
        struct.pack_into(">I", buf, OFF_BRIDGE_HEARTBEAT, value & 0xFFFFFFFF)
        self._write(buf)

    def read_game_cmd(self) -> int:
        buf = self._read()
        cmd = buf[OFF_GAME_CMD]
        if cmd != 0:
            buf[OFF_GAME_CMD] = 0
            self._write(buf)
        return cmd

    def read_phase(self) -> int:
        buf = self._read()
        return buf[OFF_LOBBY]

    def read_pending_loadout(self) -> Loadout:
        buf = self._read()
        loadout, _ = Loadout.unpack(bytes(buf), OFF_PENDING_LOADOUT)
        return loadout

    def write_lobby(self, lobby: LobbySnapshot, bridge_cmd: int) -> None:
        buf = self._read()
        packed = lobby.pack()
        if len(packed) != LOBBY_SIZE:
            # Still write; C side uses fixed OnlineLobbySnapshot.
            pass
        buf[OFF_LOBBY : OFF_LOBBY + len(packed)] = packed
        buf[OFF_BRIDGE_CMD] = int(bridge_cmd)
        buf[OFF_ERROR] = 0
        self._write(buf)

    def write_error(self, code: int) -> None:
        buf = self._read()
        buf[OFF_BRIDGE_CMD] = int(Cmd.ERROR)
        buf[OFF_ERROR] = code & 0xFF
        buf[OFF_LOBBY] = 5  # ONLINE_PHASE_ERROR
        self._write(buf)

    def _accept_seq(self, last_list: List[int], seat: int, seq: int) -> bool:
        last = last_list[seat]
        if last >= 0:
            delta = (seq - last) & 0xFFFFFFFF
            if delta == 0 or delta > 0x7FFFFFFF:
                return False
        last_list[seat] = seq
        return True

    def write_remote_snapshot(self, snap: RacerSnapshot) -> bool:
        """Write a remote racer snapshot. Returns False if the sequence is stale."""
        if snap.seat >= 4:
            return False
        seat = int(snap.seat)
        seq = int(snap.sequence) & 0xFFFFFFFF
        if not self._accept_seq(self._remote_seq, seat, seq):
            return False
        # Full pose supersedes path coast for this seat.
        self._cpu_path_seq[seat] = seq
        buf = self._read()
        offset = OFF_REMOTE_IN + seat * RACER_SIZE
        packed = snap.pack()
        buf[offset : offset + len(packed)] = packed
        # Invalidate path slot so apply prefers full pose.
        path_off = OFF_CPU_PATH_IN + seat * CPU_PATH_SIZE
        buf[path_off + 1] = 0
        self._write(buf)
        return True

    def write_cpu_path_snapshot(self, snap: CpuPathSnapshot) -> bool:
        if snap.seat >= 4:
            return False
        seat = int(snap.seat)
        seq = int(snap.sequence) & 0xFFFFFFFF
        if not self._accept_seq(self._cpu_path_seq, seat, seq):
            return False
        self._remote_seq[seat] = seq
        buf = self._read()
        offset = OFF_CPU_PATH_IN + seat * CPU_PATH_SIZE
        packed = snap.pack()
        buf[offset : offset + len(packed)] = packed
        # Invalidate full remote so path mode wins while grounded.
        rem_off = OFF_REMOTE_IN + seat * RACER_SIZE
        buf[rem_off + 1] = 0
        self._write(buf)
        return True

    def read_local_snapshot(self) -> Optional[RacerSnapshot]:
        """Backward-compatible: first valid authorityOut seat (usually local human). """
        snaps = self.read_authority_snapshots()
        return snaps[0] if snaps else None

    def read_authority_snapshots(self) -> List[RacerSnapshot]:
        """All valid host/local authority racer snapshots for this client."""
        buf = self._read()
        snaps: List[RacerSnapshot] = []
        for seat in range(MAX_SEATS):
            snap, _ = RacerSnapshot.unpack(bytes(buf), OFF_AUTHORITY_OUT + seat * RACER_SIZE)
            if snap.valid:
                snaps.append(snap)
        return snaps

    def read_cpu_path_out_snapshots(self) -> List[CpuPathSnapshot]:
        buf = self._read()
        snaps: List[CpuPathSnapshot] = []
        for seat in range(MAX_SEATS):
            snap, _ = CpuPathSnapshot.unpack(bytes(buf), OFF_CPU_PATH_OUT + seat * CPU_PATH_SIZE)
            if snap.valid:
                snaps.append(snap)
        return snaps

    def push_event_in(self, event: RaceEvent) -> None:
        buf = self._read()
        count = buf[OFF_EVENT_IN_COUNT]
        if count >= EVENT_QUEUE:
            return
        offset = OFF_EVENT_IN + count * EVENT_SIZE
        packed = event.pack()
        buf[offset : offset + len(packed)] = packed
        buf[OFF_EVENT_IN_COUNT] = count + 1
        self._write(buf)

    def drain_events_out(self) -> List[RaceEvent]:
        buf = self._read()
        count = buf[OFF_EVENT_OUT_COUNT]
        events: List[RaceEvent] = []
        for i in range(count):
            event, _ = RaceEvent.unpack(bytes(buf), OFF_EVENT_OUT + i * EVENT_SIZE)
            events.append(event)
        buf[OFF_EVENT_OUT_COUNT] = 0
        self._write(buf)
        return events
