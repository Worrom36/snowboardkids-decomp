#!/usr/bin/env python3
"""Single-lobby dedicated server for Online Battle."""

from __future__ import annotations

import argparse
import asyncio
import random
import sys
import time
from dataclasses import dataclass, field
from typing import Dict, Optional, Set, Tuple

from tools.online.protocol import (
    DEFAULT_PORT,
    MAX_SEATS,
    Cmd,
    CpuPathSnapshot,
    ErrorCode,
    Loadout,
    LobbySeat,
    LobbySnapshot,
    Phase,
    RaceEvent,
    RacerSnapshot,
    SeatKind,
    name_to_str,
    pack_packet,
    sanitize_name,
    unpack_packet,
)

CMD_NAMES = {int(c): c.name for c in Cmd}
PHASE_NAMES = {int(p): p.name for p in Phase}
ERROR_NAMES = {int(e): e.name for e in ErrorCode}


def log(msg: str) -> None:
    stamp = time.strftime("%H:%M:%S")
    print(f"[{stamp}] {msg}", flush=True)


def fmt_addr(addr: Tuple[str, int]) -> str:
    return f"{addr[0]}:{addr[1]}"


@dataclass
class Client:
    addr: Tuple[str, int]
    seat: int
    ready: bool = False
    loadout: Loadout = field(default_factory=Loadout)
    last_seen: float = field(default_factory=time.time)
    last_snapshot: Optional[RacerSnapshot] = None
    snapshot_count: int = 0
    last_snapshot_log: float = 0.0


class LobbyServer:
    def __init__(self, verbose_snapshots: bool = False) -> None:
        self.phase = Phase.LOBBY
        self.host_seat = 0
        self.clients: Dict[Tuple[str, int], Client] = {}
        self.seats: Dict[int, Client] = {}
        self.seq = 1
        self.race_seed = 0
        self.start_frame = 0
        self.race_frame = 0
        self.transport: Optional[asyncio.DatagramTransport] = None
        self.verbose_snapshots = verbose_snapshots
        self._unknown_packet_count = 0
        self.loaded_seats: Set[int] = set()
        self._race_go_sent = False
        self._start_mono: float = 0.0

    def next_seq(self) -> int:
        self.seq = (self.seq + 1) & 0xFFFFFFFF
        return self.seq

    def allocate_seat(self) -> Optional[int]:
        for seat in range(MAX_SEATS):
            if seat not in self.seats:
                return seat
        return None

    def human_count(self) -> int:
        return len(self.seats)

    def all_humans_ready(self) -> bool:
        if not self.seats:
            return False
        return all(client.ready for client in self.seats.values())

    def lobby_status_line(self) -> str:
        parts = []
        for i in range(MAX_SEATS):
            client = self.seats.get(i)
            if client is None:
                parts.append(f"P{i + 1}=empty")
            else:
                host = " host" if i == self.host_seat else ""
                ready = " READY" if client.ready else ""
                parts.append(f"P{i + 1}=human{host}{ready}")
        return (
            f"phase={PHASE_NAMES.get(int(self.phase), self.phase)} "
            f"humans={self.human_count()} [{', '.join(parts)}]"
        )

    def build_lobby_snapshot(self, local_seat: int) -> LobbySnapshot:
        seats = []
        for i in range(MAX_SEATS):
            client = self.seats.get(i)
            if client is None:
                if self.phase == Phase.LOBBY:
                    seats.append(LobbySeat(kind=SeatKind.EMPTY))
                else:
                    # Host-sim CPU filler: stable character per seat so all clients match.
                    seats.append(
                        LobbySeat(
                            kind=SeatKind.CPU,
                            loadout=Loadout(character_id=(i % 6), lap_count=3),
                        )
                    )
            else:
                seats.append(
                    LobbySeat(
                        kind=SeatKind.HUMAN,
                        ready=1 if client.ready else 0,
                        loadout=client.loadout,
                    )
                )
        return LobbySnapshot(
            phase=int(self.phase),
            local_seat=local_seat,
            host_seat=self.host_seat,
            human_count=self.human_count(),
            race_seed=self.race_seed,
            start_frame=self.start_frame,
            seats=seats,
        )

    def all_humans_loaded(self) -> bool:
        humans = set(self.seats.keys())
        return len(humans) > 0 and humans <= self.loaded_seats

    def maybe_send_race_go(self, force: bool = False) -> None:
        if self._race_go_sent or self.phase != Phase.COUNTDOWN:
            return
        timed_out = False
        if self._start_mono > 0 and (time.time() - self._start_mono) >= 8.0:
            timed_out = True
        if not force and not timed_out and not self.all_humans_loaded():
            return
        self._race_go_sent = True
        self.phase = Phase.RACING
        reason = "timeout" if timed_out and not self.all_humans_loaded() else "all loaded"
        log(
            f"RACE_GO ({reason}) loaded={sorted(self.loaded_seats)} "
            f"humans={sorted(self.seats.keys())}"
        )
        for addr_i, client_i in list(self.clients.items()):
            snap = self.build_lobby_snapshot(client_i.seat)
            snap.phase = Phase.RACING
            self.send(addr_i, Cmd.RACE_GO, snap.pack())
            log(f"  -> RACE_GO to seat={client_i.seat} {fmt_addr(addr_i)}")

    def broadcast(self, cmd: int, payload: bytes, exclude: Optional[Tuple[str, int]] = None) -> None:
        if self.transport is None:
            return
        packet = pack_packet(cmd, self.next_seq(), payload)
        for addr in list(self.clients):
            if exclude is not None and addr == exclude:
                continue
            self.transport.sendto(packet, addr)

    def send(self, addr: Tuple[str, int], cmd: int, payload: bytes = b"") -> None:
        if self.transport is None:
            return
        self.transport.sendto(pack_packet(cmd, self.next_seq(), payload), addr)

    def broadcast_lobby(self) -> None:
        log(f"lobby broadcast: {self.lobby_status_line()}")
        for addr, client in list(self.clients.items()):
            snap = self.build_lobby_snapshot(client.seat)
            self.send(addr, Cmd.LOBBY_SNAPSHOT, snap.pack())

    def remove_client(self, addr: Tuple[str, int]) -> None:
        client = self.clients.pop(addr, None)
        if client is None:
            return
        self.seats.pop(client.seat, None)
        log(f"DISCONNECT seat={client.seat} from {fmt_addr(addr)}")
        if self.host_seat == client.seat and self.seats:
            self.host_seat = min(self.seats)
            log(f"host transferred to seat={self.host_seat}")
        if self.phase == Phase.RACING:
            event = RaceEvent(type=4, seat=client.seat, sequence=self.next_seq())
            self.broadcast(Cmd.EVENT, event.pack())
            log(f"race event DISCONNECT seat={client.seat}")
        if not self.seats:
            self.phase = Phase.LOBBY
            self.race_frame = 0
            self.loaded_seats.clear()
            self._race_go_sent = False
            log("lobby empty; phase -> LOBBY")
        self.broadcast_lobby()

    def handle_connect(self, addr: Tuple[str, int]) -> None:
        if addr in self.clients:
            client = self.clients[addr]
            log(f"CONNECT (rejoin) seat={client.seat} from {fmt_addr(addr)}")
            self.send(addr, Cmd.JOINED, self.build_lobby_snapshot(client.seat).pack())
            return
        if self.phase != Phase.LOBBY:
            log(f"CONNECT rejected (phase={PHASE_NAMES.get(int(self.phase), self.phase)}) from {fmt_addr(addr)}")
            self.send(addr, Cmd.ERROR, bytes([ErrorCode.REJECTED]))
            return
        seat = self.allocate_seat()
        if seat is None:
            log(f"CONNECT rejected (FULL) from {fmt_addr(addr)}")
            self.send(addr, Cmd.ERROR, bytes([ErrorCode.FULL]))
            return
        client = Client(addr=addr, seat=seat)
        if not self.seats:
            self.host_seat = seat
        self.clients[addr] = client
        self.seats[seat] = client
        log(
            f"JOINED seat={seat} from {fmt_addr(addr)}"
            f"{' (host)' if seat == self.host_seat else ''} — {self.lobby_status_line()}"
        )
        self.send(addr, Cmd.JOINED, self.build_lobby_snapshot(seat).pack())
        self.broadcast_lobby()

    def handle_datagram(self, data: bytes, addr: Tuple[str, int]) -> None:
        parsed = unpack_packet(data)
        if parsed is None:
            self._unknown_packet_count += 1
            if self._unknown_packet_count <= 5 or self._unknown_packet_count % 50 == 0:
                log(f"bad packet ({len(data)} bytes) from {fmt_addr(addr)} count={self._unknown_packet_count}")
            return
        cmd, seq, payload = parsed
        cmd_name = CMD_NAMES.get(cmd, str(cmd))

        if cmd == Cmd.CONNECT:
            self.handle_connect(addr)
            return

        client = self.clients.get(addr)
        if client is None:
            if cmd != Cmd.DISCONNECT:
                log(f"{cmd_name} from unknown {fmt_addr(addr)} — REJECTED")
                self.send(addr, Cmd.ERROR, bytes([ErrorCode.REJECTED]))
            return

        client.last_seen = time.time()

        if cmd == Cmd.DISCONNECT:
            self.remove_client(addr)
            return

        if cmd == Cmd.SET_LOADOUT and self.phase == Phase.LOBBY:
            loadout, _ = Loadout.unpack(payload)
            loadout.name = sanitize_name(loadout.name)
            client.loadout = loadout
            log(
                f"SET_LOADOUT seat={client.seat} name={name_to_str(loadout.name)!r} "
                f"course={loadout.course_id} "
                f"char={loadout.character_id} board={loadout.snowboard_texture_index} "
                f"laps={loadout.lap_count}"
            )
            self.broadcast_lobby()
            return

        if cmd == Cmd.READY and self.phase == Phase.LOBBY:
            client.ready = True
            log(f"READY seat={client.seat} — {self.lobby_status_line()}")
            self.broadcast_lobby()
            return

        if cmd == Cmd.UNREADY and self.phase == Phase.LOBBY:
            client.ready = False
            log(f"UNREADY seat={client.seat} — {self.lobby_status_line()}")
            self.broadcast_lobby()
            return

        if cmd == Cmd.REQUEST_START and self.phase == Phase.LOBBY:
            if client.seat != self.host_seat:
                log(f"REQUEST_START ignored (not host) seat={client.seat}")
                return
            # Host Start implies the host is ready (solo lobbies often skip A).
            if not client.ready:
                client.ready = True
                log(f"READY (implied by host start) seat={client.seat}")
            if not self.all_humans_ready():
                log(f"REQUEST_START ignored (not all ready) — {self.lobby_status_line()}")
                return
            # Stay in COUNTDOWN until every human has loaded the race scene.
            self.phase = Phase.COUNTDOWN
            self.race_seed = random.randint(1, 0x7FFFFFFF)
            self.start_frame = 90
            self.race_frame = 0
            self.loaded_seats.clear()
            self._race_go_sent = False
            self._start_mono = time.time()
            for seat_client in self.seats.values():
                seat_client.ready = False
                seat_client.snapshot_count = 0
            log(
                f"START race seed={self.race_seed} start_frame={self.start_frame} "
                f"players={self.human_count()} (waiting for RACE_LOADED)"
            )
            for addr_i, client_i in list(self.clients.items()):
                snap = self.build_lobby_snapshot(client_i.seat)
                snap.phase = Phase.COUNTDOWN
                self.send(addr_i, Cmd.START, snap.pack())
                log(f"  -> START to seat={client_i.seat} {fmt_addr(addr_i)}")
            return

        if cmd == Cmd.RACE_LOADED and self.phase == Phase.COUNTDOWN:
            self.loaded_seats.add(client.seat)
            log(
                f"RACE_LOADED seat={client.seat} "
                f"({len(self.loaded_seats)}/{self.human_count()})"
            )
            self.maybe_send_race_go()
            return

        # COUNTDOWN too: remotes must hard-place at the start line before RACE_GO,
        # otherwise puppets stay unposed/invisible until the first racing snap.
        if cmd in (Cmd.RACER_SNAPSHOT, Cmd.CPU_PATH_SNAPSHOT) and self.phase in (
            Phase.COUNTDOWN,
            Phase.RACING,
        ):
            if cmd == Cmd.RACER_SNAPSHOT:
                snap, _ = RacerSnapshot.unpack(payload)
            else:
                snap, _ = CpuPathSnapshot.unpack(payload)
            claimed = int(snap.seat) & 0xFF
            # Humans publish their own seat. Host may also publish CPU filler seats
            # (no human client occupying that index) so remotes share one CPU pose.
            allowed = claimed == client.seat
            if (
                not allowed
                and client.seat == self.host_seat
                and claimed < MAX_SEATS
                and claimed not in self.seats
            ):
                allowed = True
            if not allowed:
                claimed = client.seat
            snap.seat = claimed
            snap.valid = 1
            if cmd == Cmd.RACER_SNAPSHOT:
                client.last_snapshot = snap
            client.snapshot_count += 1
            now = time.time()
            if self.verbose_snapshots or (now - client.last_snapshot_log) >= 1.0:
                client.last_snapshot_log = now
                if cmd == Cmd.RACER_SNAPSHOT:
                    log(
                        f"SNAPSHOT seat={claimed} from={client.seat} seq={snap.sequence} "
                        f"pos=({snap.pos_x},{snap.pos_y},{snap.pos_z}) "
                        f"count={client.snapshot_count}"
                    )
                else:
                    log(
                        f"CPU_PATH seat={claimed} from={client.seat} seq={snap.sequence} "
                        f"path={snap.course_path_index}/{snap.course_path_offset} "
                        f"count={client.snapshot_count}"
                    )
            self.broadcast(cmd, snap.pack(), exclude=addr)
            return

        if cmd == Cmd.EVENT and self.phase == Phase.RACING:
            event, _ = RaceEvent.unpack(payload)
            # seat meaning: ITEM_USE(1)/ACTION_USE(2) = actor, HIT(5) = victim.
            # Forcing seat=sender on HIT reroutes the hit away from the victim.
            if event.type in (1, 2):
                event.seat = client.seat
            log(
                f"EVENT type={event.type} seat={event.seat} from={client.seat} "
                f"args=({event.arg0},{event.arg1})"
            )
            self.broadcast(Cmd.EVENT, event.pack(), exclude=addr)
            return

        if cmd == Cmd.HEARTBEAT:
            self.send(addr, Cmd.HEARTBEAT, b"")
            return

        log(f"unhandled {cmd_name} seq={seq} seat={client.seat} len={len(payload)}")


class LobbyProtocol(asyncio.DatagramProtocol):
    def __init__(self, server: LobbyServer) -> None:
        self.server = server

    def connection_made(self, transport: asyncio.BaseTransport) -> None:
        self.server.transport = transport  # type: ignore[assignment]
        sockname = transport.get_extra_info("sockname")
        log(f"UDP socket bound {sockname}")

    def datagram_received(self, data: bytes, addr: Tuple[str, int]) -> None:
        self.server.handle_datagram(data, addr)

    def error_received(self, exc: Exception) -> None:
        log(f"socket error: {exc}")


async def _race_go_watchdog(server: LobbyServer) -> None:
    while True:
        await asyncio.sleep(0.5)
        server.maybe_send_race_go()


async def run_server(host: str, port: int, verbose_snapshots: bool) -> None:
    loop = asyncio.get_running_loop()
    server = LobbyServer(verbose_snapshots=verbose_snapshots)
    transport, _protocol = await loop.create_datagram_endpoint(
        lambda: LobbyProtocol(server),
        local_addr=(host, port),
    )
    watchdog = asyncio.create_task(_race_go_watchdog(server))
    log(f"Online Battle lobby listening on {host}:{port}")
    log("waiting for CONNECT / READY / REQUEST_START ...")
    try:
        await asyncio.Future()
    finally:
        log("shutting down")
        watchdog.cancel()
        transport.close()


def main() -> None:
    # Avoid Windows cp1252 crashes on redirected consoles.
    if hasattr(sys.stdout, "reconfigure"):
        try:
            sys.stdout.reconfigure(encoding="utf-8", errors="replace")
        except Exception:
            pass

    parser = argparse.ArgumentParser(description="Snowboard Kids Online Battle lobby server")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument(
        "--verbose-snapshots",
        action="store_true",
        help="Log every racer snapshot (default: at most once per seat per second)",
    )
    args = parser.parse_args()
    asyncio.run(run_server(args.host, args.port, args.verbose_snapshots))


if __name__ == "__main__":
    main()
