# Online Battle tools

Single-lobby dedicated server and net client for Snowboard Kids Online Battle.

## Quick start (manual)

```bash
# Terminal 1 — host the lobby
python tools/online_server.py --host 0.0.0.0 --port 6464

# Terminal 2 / 3 — each player
python tools/online_client.py --host <SERVER_IP> --port 6464 --mailbox tmp/online_mailbox.bin
```

Server address defaults come from `tools/online-client/config.ini` (`127.0.0.1:6464`).

`body_collision = bump` (default) or `ghost` — the **first joiner** (lobby host)
publishes this on CONNECT; peers follow the lobby snapshot flag.

## Fast local 2P / 4P test

Build a normal Online Battle ROM, then use the in-game **GAME MENU → Online Battle** entry on each window:

```bash
make BUILD_MODE=shiftable ONLINE_BATTLE=1
python tools/online_test_2p.py
# or: python tools/online_test_4p.py
```

`ONLINE_BATTLE_AUTOTEST=1` is an optional compile flag that skips title/menus and
boots straight into the lobby (handy for iteration; leave it off for normal play).

Use `--no-auto` if you want to ready/start with the pad yourself.

## Loopback test

```bash
python tools/online/test_loopback.py
```

## Windows play package

Ship a self-contained kit (ROM + Mupen + embeddable Python + `.bat` launchers)
with no system Python required on recipient PCs:

```bash
make package-online
```

Output: `dist/online/` and `dist/SnowboardKids-OnlineBattle-win64.zip`.

Ship flags: `ONLINE_BATTLE=1` `UNLOCK_ALL=1`, with `ONLINE_BATTLE_AUTOTEST=0`
and `AUTOPILOT_DEFAULT_ON=0`. Lobby ready/start stay manual (no `--auto-ready` /
`--auto-start` in packaged bats). Default UDP port is **6464**.

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
