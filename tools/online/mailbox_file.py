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
OFF_LOCAL_OUT = 0x050
OFF_REMOTE_IN = 0x0A0
OFF_EVENT_OUT_COUNT = 0x1E0
OFF_EVENT_IN_COUNT = 0x1E1
OFF_EVENT_OUT = 0x1E4
OFF_EVENT_IN = 0x224
OFF_BRIDGE_HEARTBEAT = 0x264
OFF_GAME_HEARTBEAT = 0x268
MAILBOX_SIZE = 0x26C

RACER_SIZE = RacerSnapshot._STRUCT.size
EVENT_SIZE = RaceEvent._STRUCT.size
LOBBY_SIZE = 12 + 4 * (4 + Loadout._STRUCT.size)


class MailboxFile:
    def __init__(self, path: Path) -> None:
        self.path = path

    def init(self) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True)
        buf = bytearray(MAILBOX_SIZE)
        struct.pack_into(">I", buf, OFF_MAGIC, MAILBOX_MAGIC)
        struct.pack_into(">H", buf, OFF_VERSION, VERSION)
        self.path.write_bytes(buf)

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

    def write_remote_snapshot(self, snap: RacerSnapshot) -> None:
        if snap.seat >= 4:
            return
        buf = self._read()
        offset = OFF_REMOTE_IN + snap.seat * RACER_SIZE
        packed = snap.pack()
        buf[offset : offset + len(packed)] = packed
        self._write(buf)

    def read_local_snapshot(self) -> Optional[RacerSnapshot]:
        buf = self._read()
        snap, _ = RacerSnapshot.unpack(bytes(buf), OFF_LOCAL_OUT)
        return snap

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
