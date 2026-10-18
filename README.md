# Snowboard Kids — Online Battle (WIP)

Fork of
[`cdlewis/snowboardkids-decomp`](https://github.com/cdlewis/snowboardkids-decomp)
adding experimental Online Battle. Matching decomp setup, dependencies, and
SHA1 builds: follow the
[upstream README](https://github.com/cdlewis/snowboardkids-decomp#readme).

```
git clone --recurse-submodules -j8 https://github.com/Worrom36/snowboardkids-decomp.git
```

Needs a US `snowboardkids.z64` and the same toolchain as upstream. Online Battle
builds need **8 MB RDRAM** (Expansion Pak) and are **not** SHA1-matching.

## What's different

- **GAME MENU → ONLINE BATTLE** — lobby ready-up, host Start
- Dedicated lobby server, Mupen process-memory bridge, race sync (pose/anim, items/hits)
- Local 2–4P Windows loopback; `make package-online` Windows play kit
- Windows IDO / MIPS wrappers (`tools/ido-cc.py`, `tools/ido-as.py`)

`ONLINE_BATTLE` defaults to **1** for `BUILD_MODE=shiftable` (0 for matching).

## Build

```bash
make BUILD_MODE=shiftable
```

Optional:

- `ONLINE_BATTLE_AUTOTEST=1` — skip title/menus, boot into the lobby
- `AUTOPILOT_DEFAULT_ON=1` — local LIVE humans start with autopilot on (debug;
  independent of AUTOTEST; requires `ONLINE_BATTLE=1`)
- `UNLOCK_ALL=1` — force unlock cheat (all courses, Shinobin, progression) on saves

`make package-online` ships with `ONLINE_BATTLE_AUTOTEST=0` and
`AUTOPILOT_DEFAULT_ON=0`.

## Local play (Windows)

```bash
make BUILD_MODE=shiftable
python tools/online_test_2p.py
# or: python tools/online_test_4p.py
```

Enter **ONLINE BATTLE** on each Mupen window (clients auto ready/start unless
you pass `--no-auto`).

Windows play kit (ROM + Mupen + embeddable Python):

```bash
make package-online
```

→ `dist/online/` and `dist/SnowboardKids-OnlineBattle-win64.zip`

## Tools

Manual server/client/bridge:
[tools/online/README.md](tools/online/README.md).
