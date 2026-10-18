# Online Battle tools

Single-lobby dedicated server and net client for Snowboard Kids Online Battle.

## Quick start (manual)

```bash
# Terminal 1 — host the lobby
python tools/online_server.py --host 0.0.0.0 --port 7777

# Terminal 2 / 3 — each player
python tools/online_client.py --host <SERVER_IP> --port 7777 --mailbox tmp/online_mailbox.bin
```

Server address defaults come from `tools/online-client/config.ini` (`127.0.0.1:7777`).

## Fast local 2P test

Build a ROM that boots straight into the Online Lobby (default Slash / Big Snow Valley / 3 laps):

```bash
make BUILD_MODE=shiftable ONLINE_BATTLE=1 ONLINE_BATTLE_AUTOTEST=1
```

Then launch server + two Mupens + two pinned clients (auto ready + host start):

```bash
python tools/online_test_2p.py
```

Use `--no-auto` if you want to ready/start with the pad yourself.

## Loopback test

```bash
python tools/online/test_loopback.py
```

## ROM build

```bash
make BUILD_MODE=shiftable ONLINE_BATTLE=1
```

Online Battle shiftable builds need **8 MB RDRAM** (expansion pak / default in most
emulators); the extra resident lobby/net code does not fit the original 4 MB cap.

In-game: GAME MENU → Online Battle → lobby ready-up → host Start.

Without a running bridge (`bridgeHeartbeat == 0`), the lobby self-hosts a 1P+CPU session after a short connect wait so the UI can be exercised offline.

## Bridge (Mupen ↔ client)

The ROM mailbox lives in emulated RDRAM; the Python client uses
`tmp/online_mailbox.bin`. On Windows the **online client** enables a process
memory bridge by default (`--mupen-bridge`):

1. Start the lobby server
2. Start the online client (bridge ON)
3. Launch Mupen with the Online Battle ROM
4. Select Online Battle — client should log `game cmd -> client: 1` (CONNECT)
   and the server should log `JOINED`

Standalone bridge (file sync only):

```bash
python tools/online_bridge.py --mailbox tmp/online_mailbox.bin
```

C plugin skeleton (optional future path): `tools/online/mupen_bridge_plugin.c`
