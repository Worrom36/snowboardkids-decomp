#!/usr/bin/env python3
"""Loopback test: two clients join one lobby, ready up, and receive START."""

from __future__ import annotations

import asyncio
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.online.client import ClientProtocol, OnlineClient
from tools.online.mailbox_file import MailboxFile
from tools.online.protocol import Cmd, Loadout, Phase, RacerSnapshot
from tools.online.server import LobbyProtocol, LobbyServer


async def run_test() -> None:
    loop = asyncio.get_running_loop()
    server = LobbyServer()
    transport, _ = await loop.create_datagram_endpoint(
        lambda: LobbyProtocol(server),
        local_addr=("127.0.0.1", 0),
    )
    sockname = transport.get_extra_info("sockname")
    host, port = sockname[0], sockname[1]

    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        client_a = OnlineClient(host, port, tmp_path / "a.bin")
        client_b = OnlineClient(host, port, tmp_path / "b.bin")

        transport_a, _ = await loop.create_datagram_endpoint(
            lambda: ClientProtocol(client_a),
            local_addr=("127.0.0.1", 0),
        )
        transport_b, _ = await loop.create_datagram_endpoint(
            lambda: ClientProtocol(client_b),
            local_addr=("127.0.0.1", 0),
        )

        client_a.mailbox.init()
        client_b.mailbox.init()

        # Simulate game CONNECT + loadout.
        for client, course, name in ((client_a, 0, b"ACE"), (client_b, 1, b"BOB")):
            buf = client.mailbox._read()
            buf[0x008] = Cmd.CONNECT
            client.mailbox._write(buf)
            client.poll_mailbox_commands()
            await asyncio.sleep(0.05)
            loadout = Loadout(course_id=course, character_id=course + 1, lap_count=3, name=name)
            buf = client.mailbox._read()
            packed = loadout.pack()
            buf[0x068 : 0x068 + len(packed)] = packed
            buf[0x008] = Cmd.SET_LOADOUT
            client.mailbox._write(buf)
            client.poll_mailbox_commands()
            await asyncio.sleep(0.05)

        assert client_a.connected and client_b.connected
        assert client_a.local_seat != client_b.local_seat
        assert server.human_count() == 2

        for client in (client_a, client_b):
            buf = client.mailbox._read()
            buf[0x008] = Cmd.READY
            client.mailbox._write(buf)
            client.poll_mailbox_commands()
            await asyncio.sleep(0.05)

        assert server.all_humans_ready()

        # Host requests start.
        host_client = client_a if client_a.local_seat == server.host_seat else client_b
        buf = host_client.mailbox._read()
        buf[0x008] = Cmd.REQUEST_START
        host_client.mailbox._write(buf)
        host_client.poll_mailbox_commands()
        await asyncio.sleep(0.1)

        # START only enters COUNTDOWN; both games must report loaded for RACE_GO.
        assert server.phase == Phase.COUNTDOWN
        names = {c.loadout.name_str for c in server.seats.values()}
        assert names == {"ACE", "BOB"}, names
        for client in (client_a, client_b):
            buf = client.mailbox._read()
            buf[0x008] = Cmd.RACE_LOADED
            client.mailbox._write(buf)
            client.poll_mailbox_commands()
            await asyncio.sleep(0.05)

        assert server.phase == Phase.RACING
        assert client_a.phase in (Phase.COUNTDOWN, Phase.RACING)
        assert client_b.phase in (Phase.COUNTDOWN, Phase.RACING)

        # Exchange a racer snapshot.
        snap = RacerSnapshot(seat=client_a.local_seat, valid=1, pos_x=12345, sequence=1)
        client_a.send(Cmd.RACER_SNAPSHOT, snap.pack())
        await asyncio.sleep(0.05)
        remote = MailboxFile(client_b.mailbox.path)
        got = remote._read()
        # remote seat slot for A
        from tools.online.mailbox_file import OFF_REMOTE_IN

        offset = OFF_REMOTE_IN + client_a.local_seat * RacerSnapshot._STRUCT.size
        assert got[offset + 1] == 1  # valid
        assert int.from_bytes(got[offset + 0x0C : offset + 0x10], "big", signed=True) == 12345

        # Post-race: either client can reopen the lobby for a rematch.
        buf = client_a.mailbox._read()
        buf[0x008] = Cmd.RETURN_TO_LOBBY
        client_a.mailbox._write(buf)
        client_a.poll_mailbox_commands()
        await asyncio.sleep(0.1)
        assert server.phase == Phase.LOBBY
        assert not server.all_humans_ready()
        assert client_a.phase == Phase.LOBBY

        print("loopback OK: join, ready, start, snapshot relay, return to lobby")

        transport_a.close()
        transport_b.close()
    transport.close()


def main() -> None:
    asyncio.run(run_test())


if __name__ == "__main__":
    main()
