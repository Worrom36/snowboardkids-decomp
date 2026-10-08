# Multi-version development

Select the region with `VERSION=us|jp`. Each region has separate ROM metadata
and a corresponding `VERSION_US` or `VERSION_JP` preprocessor definition.
Source trees are `src.us` and `src.jp`, selected through
`SRC_DIR = src.$(VERSION)`. Shared headers stay in `include`. ROM metadata,
assembly, assets, and build outputs retain their established regional paths.
Switching versions does not require discarding the other build.

| Purpose | US | JP |
| --- | --- | --- |
| Splat config | `snowboardkids.yaml` | `snowboardkids.jp.yaml` |
| Expected checksum | `snowboardkids.sha1` | `snowboardkids.jp.sha1` |
| Symbols | `symbol_addrs.txt` | `symbol_addrs.jp.txt` |
| Relocations | `reloc_addrs.txt` | `reloc_addrs.jp.txt` |
| Generated assembly | `asm/` | `asm.jp/` |
| Extracted assets | `assets/` | `assets.jp/` |
| Default C source directory | `src.us/` | `src.jp/` |
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

## Function alignment and file splits

Run both verified builds before refreshing the map:

```bash
./tools/build-and-verify.sh
./tools/build-and-verify.sh --version jp
.venv/bin/python tools/match_rom_functions.py
```

The matcher writes `reports/region-functions.json`, a readable Markdown summary,
and a proposed `.splits.yaml` fragment. It reads complete function extents from
MIPS ELF symbols and bytes from the original ROMs. Exact-byte matches are kept
separate from instruction sequences that differ only in recognized RAM address
operands. Ordinary constants, registers, and branch structure remain significant.
Order, neighboring anchors, pointer identity, and mapped call targets constrain
ambiguous matches. Small stubs need anchors on both sides. These are conservative
correspondences, not a proof of semantic identity or original compilation units.

Supported starts mirror US relative file names as JP `asm` subsegments. Where
an adjacent file start cannot be identified, an `unresolved/<ROM offset>` range
keeps the uncertain tail separate. Do not interpret an approximate interval as
an exact split or copy US addresses into JP symbols. Main data and assets retain
the coarse scaffold described above. JP assembly uses `-no-pad-sections` so a
four-byte split does not introduce assembler padding.

The JSON report records supporting anchors and unresolved functions; rerunning
it after extraction must reproduce the same function map and split proposals.
The tool never edits the configuration itself.

## Shared C implementations

The exact-byte match list has 120 functions compiled from C in `src.jp`.
[The integration checklist](../reports/jp-exact-functions.md) records their
source files and the 17 entries whose US implementations are handwritten assembly.
Each C translation unit covers a contiguous matched range. Later ranges from
the same US file use a function-name suffix; intervening code remains assembly.
These partial C files do not establish additional original file boundaries.

JP C uses the corresponding US compiler settings, including `-O3` function
ordering where applicable. Per-object padding adjustments preserve the exact
original text extents without altering instructions. Symbol names and addresses
are recorded in `symbol_addrs.jp.txt`. The matcher reports C implementation
status and preserves integrated C subsegments in its split export.

## Matching a Japanese function

1. Find the corresponding JP subsegment in `reports/region-functions.md` or
   `snowboardkids.jp.yaml`, then inspect its assembly under `asm.jp/`.
2. Record verified Japanese addresses in `symbol_addrs.jp.txt`. Add relocation
   overrides only to `reloc_addrs.jp.txt`, using Japanese ROM offsets.
3. Split the corresponding range in `snowboardkids.jp.yaml` into asm/C
   subsegments. Its `src_path` is `src.jp`, so new C stubs land there; the existing
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
apply to Japanese code. The text conversion table and inherited compiler overrides still require
per-function verification before introducing JP C code.

Use `make VERSION=jp clean` to remove only the Japanese assembly/build outputs
and generated linker metadata. Extracted binary assets remain available, as
they do for US. Run extraction again before the next build. The full build
helper performs this sequence automatically. US-only readable asset commands,
progress reports, and shiftable builds reject `VERSION=jp` explicitly.

The existing CI still verifies US; adding JP CI requires making its baserom
available through the project's private ROM provisioning. Neither ROM belongs
in this repository.
