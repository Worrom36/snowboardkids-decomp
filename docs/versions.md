# Multi-version development

The version selector follows the conventions used by
[SSSV](https://github.com/mkst/sssv/blob/master/Makefile) and
[SM64](https://github.com/n64decomp/sm64/blob/master/Makefile): `VERSION=us|jp`,
separate ROM metadata, and `VERSION_US`/`VERSION_JP` preprocessor definitions.
US keeps its established paths because the readable asset and progress tools
use them. JP uses isolated outputs, so switching versions does not require
discarding the other build.

| Purpose | US | JP |
| --- | --- | --- |
| Splat config | `snowboardkids.yaml` | `snowboardkids.jp.yaml` |
| Expected checksum | `snowboardkids.sha1` | `snowboardkids.jp.sha1` |
| Symbols | `symbol_addrs.txt` | `symbol_addrs.jp.txt` |
| Relocations | `reloc_addrs.txt` | `reloc_addrs.jp.txt` |
| Generated assembly | `asm/` | `asm.jp/` |
| Extracted assets | `assets/` | `assets.jp/` |
| Default C source directory | `src/` | `src/jp/` |
| Build directory | `build/` | `build/jp/` |
| Generated linker script | `snowboardkids.ld` | `snowboardkids.jp.ld` |
| Generated unresolved symbols | `undefined_*_auto.txt` | `undefined_*_auto.jp.txt` |

JP links only its own generated symbol assignments and the shared fixed N64
hardware/OS addresses. US game aliases and relocation overrides must not be
applied to JP. The Japanese header uses Splat's `word` encoding to preserve its
half-width katakana bytes exactly.

## Initial Japanese map

The ROM header identifies `NSKJ`, revision 0, with entry VRAM `0x80000400`.
The entrypoint occupies ROM `0x1000..0x1050` and jumps to `main` at
VRAM `0x80098FE0` (ROM `0x99BE0`). Its BSS clear loop gives start
`0x800E1CE0` and size `0x7DAC0`, placing the loaded image's end at ROM
`0xE28E0`.

Main CPU code is disassembled from ROM `0x1050` through `0xB12C0`. Three
consecutive RSP blobs at `0xB12C0`, `0xB1390`, and `0xB21B0` are byte-identical
to the US `rspboot`, `aspMain`, and `f3dlx` blobs. Remaining loaded data begins
at `0xB35E0`; ROM `0xE28E0..0x800000` is retained as one asset blob. These
coarse splits provide a matching starting point, not a complete data map.

## Matching a Japanese function

1. Inspect `asm.jp/main.s` and identify the function and surrounding boundaries.
2. Record verified Japanese addresses in `symbol_addrs.jp.txt`. Add relocation
   overrides only to `reloc_addrs.jp.txt`, using Japanese ROM offsets.
3. Split the corresponding range in `snowboardkids.jp.yaml` into asm/C
   subsegments. Its `src_path` is `src/jp`, so new C stubs land there; the existing
   compilation rules also support these files. Keep generated ROM-derived
   assembly in the ignored `asm.jp` tree.
4. Rebuild with `./tools/build-and-verify.sh --version jp`, then compare:

   ```bash
   python3 tools/asm-differ/diff.py --version jp --no-pager main
   python3 tools/data-differ/data_diff.py --version jp --find-first-mismatch
   ```

Both differs also honor `VERSION=jp`. Explicit data-differ path arguments
override version defaults. For a single data-symbol comparison, add its
Japanese `rom:0x... size:0x...` metadata to `symbol_addrs.jp.txt`.

Shared C can be introduced after verifying the layouts and compiler settings
for both versions; use `VERSION_US`/`VERSION_JP` only for confirmed differences.
Do not assume US function/data addresses, struct layouts, or text encoding
apply to Japanese code. The existing text conversion table and specialized
US compiler overrides have not been validated for JP C code.

Use `make VERSION=jp clean` to remove only the Japanese assembly/build outputs
and generated linker metadata. Extracted binary assets remain available, as
they do for US. Run extraction again before the next build. The full build
helper performs this sequence automatically. US-only readable asset commands,
progress reports, and shiftable builds reject `VERSION=jp` explicitly.

The existing CI still verifies US; adding JP CI requires making its baserom
available through the project's private ROM provisioning. Neither ROM belongs
in this repository.
