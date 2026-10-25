"""Binary wire protocol shared by the Online Battle server and clients."""

from __future__ import annotations

import struct
from dataclasses import dataclass, field
from enum import IntEnum
from typing import List, Optional, Tuple

MAGIC = 0x4F4E504B  # 'ONPK'
VERSION = 4
DEFAULT_PORT = 6464
MAX_SEATS = 4
NAME_LEN = 8
# Lobby human_count bit 7: body collision bump (default on).
RACE_RULE_BODY_BUMP = 0x80
_NAME_CHARS = frozenset("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -.'!")


def sanitize_name(raw) -> bytes:
    """Normalize a player name to NAME_LEN space-padded printable ASCII bytes."""
    if raw is None:
        text = ""
    elif isinstance(raw, bytes):
        text = raw.decode("ascii", errors="ignore")
    else:
        text = str(raw)
    cleaned = "".join(ch if ch in _NAME_CHARS else " " for ch in text.upper())
    cleaned = cleaned.strip()[:NAME_LEN]
    return cleaned.encode("ascii").ljust(NAME_LEN, b" ")


def name_to_str(raw: bytes) -> str:
    return raw.decode("ascii", errors="ignore").strip()


class Cmd(IntEnum):
    NONE = 0
    CONNECT = 1
    DISCONNECT = 2
    READY = 3
    UNREADY = 4
    REQUEST_START = 5
    SET_LOADOUT = 6
    JOINED = 7
    LOBBY_SNAPSHOT = 8
    START = 9
    ERROR = 10
    RACER_SNAPSHOT = 11
    EVENT = 12
    HEARTBEAT = 13
    CPU_PATH_SNAPSHOT = 14
    RACE_LOADED = 15
    RACE_GO = 16
    RETURN_TO_LOBBY = 17


class Phase(IntEnum):
    IDLE = 0
    CONNECTING = 1
    LOBBY = 2
    COUNTDOWN = 3
    RACING = 4
    ERROR = 5


class SeatKind(IntEnum):
    EMPTY = 0
    HUMAN = 1
    CPU = 2


class ErrorCode(IntEnum):
    NONE = 0
    FULL = 1
    REJECTED = 2
    TIMEOUT = 3
    PROTOCOL = 4
    DISCONNECTED = 5


HEADER_STRUCT = struct.Struct(">IHBBI")  # magic, size, version, cmd, seq
# size is total packet length including header


@dataclass
class Loadout:
    course_id: int = 0
    character_id: int = 0
    character_variant: int = 0
    snowboard_texture_index: int = 0
    lap_count: int = 3
    name: bytes = b""

    _STRUCT = struct.Struct(">BBBBBxxx8s")

    def pack(self) -> bytes:
        return self._STRUCT.pack(
            self.course_id,
            self.character_id,
            self.character_variant,
            self.snowboard_texture_index,
            self.lap_count,
            sanitize_name(self.name),
        )

    @classmethod
    def unpack(cls, data: bytes, offset: int = 0) -> Tuple["Loadout", int]:
        values = cls._STRUCT.unpack_from(data, offset)
        return cls(*values), offset + cls._STRUCT.size

    @property
    def name_str(self) -> str:
        return name_to_str(sanitize_name(self.name))


@dataclass
class LobbySeat:
    kind: int = SeatKind.EMPTY
    ready: int = 0
    loadout: Loadout = field(default_factory=Loadout)

    def pack(self) -> bytes:
        return struct.pack(">BBxx", self.kind, self.ready) + self.loadout.pack()

    @classmethod
    def unpack(cls, data: bytes, offset: int = 0) -> Tuple["LobbySeat", int]:
        kind, ready = struct.unpack_from(">BBxx", data, offset)
        loadout, next_off = Loadout.unpack(data, offset + 4)
        return cls(kind=kind, ready=ready, loadout=loadout), next_off


@dataclass
class LobbySnapshot:
    phase: int = Phase.IDLE
    local_seat: int = 0xFF
    host_seat: int = 0
    human_count: int = 0
    race_seed: int = 0
    start_frame: int = 0
    # Bit flags packed into human_count high bits on the wire (bit7 = body bump).
    race_rules: int = RACE_RULE_BODY_BUMP
    seats: List[LobbySeat] = field(default_factory=lambda: [LobbySeat() for _ in range(MAX_SEATS)])

    @property
    def body_bump(self) -> bool:
        return (self.race_rules & RACE_RULE_BODY_BUMP) != 0

    def pack(self) -> bytes:
        packed_humans = (self.human_count & 0x7F) | (self.race_rules & RACE_RULE_BODY_BUMP)
        body = struct.pack(
            ">BBBBII",
            self.phase,
            self.local_seat,
            self.host_seat,
            packed_humans,
            self.race_seed,
            self.start_frame,
        )
        for seat in self.seats:
            body += seat.pack()
        return body

    @classmethod
    def unpack(cls, data: bytes, offset: int = 0) -> Tuple["LobbySnapshot", int]:
        phase, local_seat, host_seat, packed_humans, race_seed, start_frame = struct.unpack_from(
            ">BBBBII", data, offset
        )
        offset += 12
        seats: List[LobbySeat] = []
        for _ in range(MAX_SEATS):
            seat, offset = LobbySeat.unpack(data, offset)
            seats.append(seat)
        return (
            cls(
                phase=phase,
                local_seat=local_seat,
                host_seat=host_seat,
                human_count=packed_humans & 0x7F,
                race_seed=race_seed,
                start_frame=start_frame,
                race_rules=packed_humans & RACE_RULE_BODY_BUMP,
                seats=seats,
            ),
            offset,
        )


@dataclass
class RacerSnapshot:
    seat: int = 0
    valid: int = 0
    item_effect_type: int = 0
    item_effect_count: int = 0
    action_effect_type: int = 0
    lap_digit: int = 0
    squash_scale: int = 0
    sequence: int = 0
    pos_x: int = 0
    pos_y: int = 0
    pos_z: int = 0
    vel_x: int = 0
    vel_y: int = 0
    vel_z: int = 0
    facing_angle: int = 0
    pitch_angle: int = 0
    lean_angle: int = 0
    course_path_index: int = 0
    course_path_offset: int = 0
    mode: int = 0
    stick_x: int = 0
    stick_y: int = 0
    animation_id: int = 0
    input_flags: int = 0
    update_state: int = 0
    model_rotation_x: int = 0
    model_rotation_y: int = 0
    model_rotation_z: int = 0
    state_flags: int = 0
    state_timer: int = 0
    ground_offset: int = 0

    # pad06 is squash_scale (s16) — pan flatten scale for remotes.
    _STRUCT = struct.Struct(">BBBBBBhIiiiiiihhhhihhhhIhhhhIii")

    def pack(self) -> bytes:
        return self._STRUCT.pack(
            self.seat,
            self.valid,
            self.item_effect_type,
            self.item_effect_count,
            self.action_effect_type,
            self.lap_digit,
            self.squash_scale,
            self.sequence,
            self.pos_x,
            self.pos_y,
            self.pos_z,
            self.vel_x,
            self.vel_y,
            self.vel_z,
            self.facing_angle,
            self.pitch_angle,
            self.lean_angle,
            self.course_path_index,
            self.course_path_offset,
            self.mode,
            self.stick_x,
            self.stick_y,
            self.animation_id,
            self.input_flags,
            self.update_state,
            self.model_rotation_x,
            self.model_rotation_y,
            self.model_rotation_z,
            self.state_flags,
            self.state_timer,
            self.ground_offset,
        )

    @classmethod
    def unpack(cls, data: bytes, offset: int = 0) -> Tuple["RacerSnapshot", int]:
        values = cls._STRUCT.unpack_from(data, offset)
        return cls(*values), offset + cls._STRUCT.size


@dataclass
class CpuPathSnapshot:
    seat: int = 0
    valid: int = 0
    item_effect_type: int = 0
    item_effect_count: int = 0
    action_effect_type: int = 0
    lap_digit: int = 0
    sequence: int = 0
    course_path_index: int = 0
    facing_angle: int = 0
    course_path_offset: int = 0
    along_speed: int = 0
    lateral_offset: int = 0
    mode: int = 0
    animation_id: int = 0
    update_state: int = 0
    squash_scale: int = 0
    state_flags: int = 0
    ground_offset: int = 0

    # Matches OnlineCpuPathSnapshot (0x2C); pad22 is squash_scale (s16).
    _STRUCT = struct.Struct(">BBBBBBxxIhhiiihhhhIi")

    def pack(self) -> bytes:
        return self._STRUCT.pack(
            self.seat,
            self.valid,
            self.item_effect_type,
            self.item_effect_count,
            self.action_effect_type,
            self.lap_digit,
            self.sequence,
            self.course_path_index,
            self.facing_angle,
            self.course_path_offset,
            self.along_speed,
            self.lateral_offset,
            self.mode,
            self.animation_id,
            self.update_state,
            self.squash_scale,
            self.state_flags,
            self.ground_offset,
        )

    @classmethod
    def unpack(cls, data: bytes, offset: int = 0) -> Tuple["CpuPathSnapshot", int]:
        values = cls._STRUCT.unpack_from(data, offset)
        return cls(*values), offset + cls._STRUCT.size


@dataclass
class RaceEvent:
    type: int = 0
    seat: int = 0
    arg0: int = 0
    arg1: int = 0
    sequence: int = 0

    _STRUCT = struct.Struct(">BBBBI")

    def pack(self) -> bytes:
        return self._STRUCT.pack(self.type, self.seat, self.arg0, self.arg1, self.sequence)

    @classmethod
    def unpack(cls, data: bytes, offset: int = 0) -> Tuple["RaceEvent", int]:
        values = cls._STRUCT.unpack_from(data, offset)
        return cls(*values), offset + cls._STRUCT.size


def pack_packet(cmd: int, seq: int, payload: bytes = b"") -> bytes:
    size = HEADER_STRUCT.size + len(payload)
    return HEADER_STRUCT.pack(MAGIC, size, VERSION, int(cmd), seq) + payload


def unpack_packet(data: bytes) -> Optional[Tuple[int, int, bytes]]:
    if len(data) < HEADER_STRUCT.size:
        return None
    magic, size, version, cmd, seq = HEADER_STRUCT.unpack_from(data)
    if magic != MAGIC or version != VERSION or size > len(data) or size < HEADER_STRUCT.size:
        return None
    return cmd, seq, data[HEADER_STRUCT.size : size]
