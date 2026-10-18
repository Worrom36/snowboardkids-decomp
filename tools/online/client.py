#!/usr/bin/env python3
"""Net client: talks to the dedicated lobby and mirrors state into a mailbox file."""

from __future__ import annotations

import argparse
import asyncio
import configparser
from pathlib import Path
from typing import Optional, Tuple

from tools.online.mailbox_file import MailboxFile
from tools.online.protocol import (
    DEFAULT_PORT,
    Cmd,
    Loadout,
    LobbySnapshot,
    Phase,
    RaceEvent,
    RacerSnapshot,
    SeatKind,
    pack_packet,
    unpack_packet,
)


class OnlineClient:
    def __init__(
        self,
        host: str,
        port: int,
        mailbox_path: Path,
        auto_ready: bool = False,
        auto_start: bool = False,
        min_players: int = 2,
    ) -> None:
        self.host = host
        self.port = port
        self.mailbox = MailboxFile(mailbox_path)
        self.transport: Optional[asyncio.DatagramTransport] = None
        self.remote_addr: Tuple[str, int] = (host, port)
        self.seq = 1
        self.local_seat = 0xFF
        self.phase = Phase.CONNECTING
        self.connected = False
        self.auto_ready = auto_ready
        self.auto_start = auto_start
        self.min_players = max(1, int(min_players))
        self._auto_ready_sent = False
        self._auto_start_sent = False

    def next_seq(self) -> int:
        self.seq = (self.seq + 1) & 0xFFFFFFFF
        return self.seq

    def send(self, cmd: int, payload: bytes = b"") -> None:
        if self.transport is None:
            return
        self.transport.sendto(pack_packet(cmd, self.next_seq(), payload), self.remote_addr)

    def handle_packet(self, data: bytes) -> None:
        parsed = unpack_packet(data)
        if parsed is None:
            return
        cmd, _seq, payload = parsed

        if cmd == Cmd.JOINED:
            lobby, _ = LobbySnapshot.unpack(payload)
            self.local_seat = lobby.local_seat
            self.phase = Phase.LOBBY
            self.connected = True
            self.mailbox.write_lobby(lobby, bridge_cmd=Cmd.JOINED)
            self.mailbox.bump_bridge_heartbeat()
            print(f"joined seat {self.local_seat}")
            self._send_pending_loadout()
            self._maybe_auto_ready(lobby)
            return

        if cmd == Cmd.LOBBY_SNAPSHOT:
            lobby, _ = LobbySnapshot.unpack(payload)
            self.local_seat = lobby.local_seat
            self.phase = Phase(lobby.phase)
            self.mailbox.write_lobby(lobby, bridge_cmd=Cmd.LOBBY_SNAPSHOT)
            self.mailbox.bump_bridge_heartbeat()
            self._maybe_auto_ready(lobby)
            self._maybe_auto_start(lobby)
            return

        if cmd == Cmd.START:
            lobby, _ = LobbySnapshot.unpack(payload)
            self.phase = Phase.COUNTDOWN
            lobby.phase = Phase.COUNTDOWN
            self.mailbox.write_lobby(lobby, bridge_cmd=Cmd.START)
            self.mailbox.bump_bridge_heartbeat()
            print("race START received")
            return

        if cmd == Cmd.ERROR:
            code = payload[0] if payload else 0
            self.mailbox.write_error(code)
            print(f"server error {code}")
            return

        if cmd == Cmd.RACER_SNAPSHOT:
            snap, _ = RacerSnapshot.unpack(payload)
            self.mailbox.write_remote_snapshot(snap)
            self.mailbox.bump_bridge_heartbeat()
            return

        if cmd == Cmd.EVENT:
            event, _ = RaceEvent.unpack(payload)
            self.mailbox.push_event_in(event)
            self.mailbox.bump_bridge_heartbeat()
            return

        if cmd == Cmd.HEARTBEAT:
            self.mailbox.bump_bridge_heartbeat()

    def _send_pending_loadout(self) -> None:
        loadout = self.mailbox.read_pending_loadout()
        self.send(Cmd.SET_LOADOUT, loadout.pack())

    def _maybe_auto_ready(self, lobby: LobbySnapshot) -> None:
        if not self.auto_ready or self._auto_ready_sent or not self.connected:
            return
        if lobby.phase != Phase.LOBBY:
            return
        self._auto_ready_sent = True
        print("-> READY (auto)", flush=True)
        self.send(Cmd.READY)

    def _maybe_auto_start(self, lobby: LobbySnapshot) -> None:
        if not self.auto_start or self._auto_start_sent or not self.connected:
            return
        if lobby.phase != Phase.LOBBY:
            return
        if self.local_seat != lobby.host_seat:
            return
        humans = [s for s in lobby.seats if s.kind == SeatKind.HUMAN]
        # Wait for the full party (default 2). Otherwise a 4P launch starts as soon
        # as the first two clients ready and rejects late CONNECTs mid-race.
        if len(humans) < self.min_players:
            return
        if not all(s.ready for s in humans):
            return
        self._auto_start_sent = True
        print(
            f"-> READY + REQUEST_START (auto host, {len(humans)}/{self.min_players})",
            flush=True,
        )
        self.send(Cmd.READY)
        self.send(Cmd.REQUEST_START)

    def poll_mailbox_commands(self) -> None:
        cmd = self.mailbox.read_game_cmd()
        if cmd == 0:
            return

        if cmd == Cmd.CONNECT:
            self.phase = Phase.CONNECTING
            self.connected = False
            self.send(Cmd.CONNECT)
            # Loadout is stored in the mailbox before/with connect; send after JOIN.
            return
        if cmd == Cmd.DISCONNECT:
            self.send(Cmd.DISCONNECT)
            self.connected = False
            return
        if cmd == Cmd.READY:
            print("-> READY", flush=True)
            self.send(Cmd.READY)
            return
        if cmd == Cmd.UNREADY:
            print("-> UNREADY", flush=True)
            self.send(Cmd.UNREADY)
            return
        if cmd == Cmd.REQUEST_START:
            # Mailbox may have dropped READY (1-byte slot); host start needs it.
            print("-> READY (before START) + REQUEST_START", flush=True)
            self.send(Cmd.READY)
            self.send(Cmd.REQUEST_START)
            return
        if cmd == Cmd.SET_LOADOUT:
            # If CONNECT was overwritten in the 1-byte cmd slot, join first.
            if not self.connected:
                self.phase = Phase.CONNECTING
                self.send(Cmd.CONNECT)
            self._send_pending_loadout()
            return

    def publish_local_snapshot(self) -> None:
        if self.phase not in (Phase.RACING, Phase.COUNTDOWN):
            # Still forward if mailbox marks racing via lobby phase.
            lobby_phase = self.mailbox.read_phase()
            if lobby_phase not in (Phase.RACING, Phase.COUNTDOWN):
                return
        snap = self.mailbox.read_local_snapshot()
        if snap is None or not snap.valid:
            return
        self.send(Cmd.RACER_SNAPSHOT, snap.pack())
        for event in self.mailbox.drain_events_out():
            self.send(Cmd.EVENT, event.pack())


class ClientProtocol(asyncio.DatagramProtocol):
    def __init__(self, client: OnlineClient) -> None:
        self.client = client

    def connection_made(self, transport: asyncio.BaseTransport) -> None:
        self.client.transport = transport  # type: ignore[assignment]

    def datagram_received(self, data: bytes, _addr) -> None:
        self.client.handle_packet(data)


async def client_loop(
    client: OnlineClient,
    mupen_bridge: bool = False,
    mupen_pid: Optional[int] = None,
) -> None:
    loop = asyncio.get_running_loop()
    transport, _ = await loop.create_datagram_endpoint(lambda: ClientProtocol(client), local_addr=("0.0.0.0", 0))
    client.mailbox.init()
    bridge = None
    if mupen_bridge:
        from tools.online.mupen_bridge import MupenBridge

        bridge = MupenBridge(client.mailbox.path, preferred_pid=mupen_pid)
        pin = f" mupen-pid={mupen_pid}" if mupen_pid is not None else ""
        print(f"online-client -> {client.host}:{client.port} mailbox={client.mailbox.path} (mupen bridge ON{pin})")
    else:
        print(f"online-client -> {client.host}:{client.port} mailbox={client.mailbox.path}")
    try:
        while True:
            if bridge is not None:
                bridge.sync()
            client.poll_mailbox_commands()
            client.publish_local_snapshot()
            if bridge is None:
                # File-only mode still needs a heartbeat for offline detection tests.
                client.mailbox.bump_bridge_heartbeat()
            await asyncio.sleep(1 / 60)
    finally:
        if bridge is not None:
            bridge.close()
        transport.close()


def load_config(path: Path) -> Tuple[str, int]:
    host = "127.0.0.1"
    port = DEFAULT_PORT
    if path.is_file():
        parser = configparser.ConfigParser()
        parser.read(path)
        if parser.has_section("online"):
            host = parser.get("online", "host", fallback=host)
            port = parser.getint("online", "port", fallback=port)
    return host, port


def main() -> None:
    import sys

    parser = argparse.ArgumentParser(description="Snowboard Kids Online Battle net client")
    parser.add_argument("--host", default=None)
    parser.add_argument("--port", type=int, default=None)
    parser.add_argument(
        "--config",
        type=Path,
        default=Path("tools/online-client/config.ini"),
    )
    parser.add_argument(
        "--mailbox",
        type=Path,
        default=Path("tmp/online_mailbox.bin"),
    )
    parser.add_argument(
        "--mupen-bridge",
        dest="mupen_bridge",
        action="store_true",
        default=None,
        help="Sync mailbox with running Mupen64Plus (default: on for Windows)",
    )
    parser.add_argument(
        "--no-mupen-bridge",
        dest="mupen_bridge",
        action="store_false",
        help="Disable Mupen process-memory bridge",
    )
    parser.add_argument(
        "--mupen-pid",
        type=int,
        default=None,
        help="Pin process-memory bridge to this Mupen PID (required for 2 local clients)",
    )
    parser.add_argument(
        "--auto-ready",
        action="store_true",
        help="Send READY automatically after joining the lobby",
    )
    parser.add_argument(
        "--auto-start",
        action="store_true",
        help="Host only: REQUEST_START when min-players humans are all ready",
    )
    parser.add_argument(
        "--min-players",
        type=int,
        default=2,
        help="Host auto-start waits until this many humans have joined (default 2)",
    )
    args = parser.parse_args()
    cfg_host, cfg_port = load_config(args.config)
    host = args.host or cfg_host
    port = args.port or cfg_port
    client = OnlineClient(
        host,
        port,
        args.mailbox,
        auto_ready=bool(args.auto_ready),
        auto_start=bool(args.auto_start),
        min_players=int(args.min_players),
    )
    use_bridge = (sys.platform == "win32") if args.mupen_bridge is None else bool(args.mupen_bridge)
    asyncio.run(client_loop(client, mupen_bridge=use_bridge, mupen_pid=args.mupen_pid))


if __name__ == "__main__":
    main()
