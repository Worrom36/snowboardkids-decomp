# Multi-version development

Select the region with `VERSION=us|jp`; US is the default. Sources live in
`src.us` and `src.jp`, with shared headers in `include`. Builds define
`VERSION_US` or `VERSION_JP` respectively.

## Build and verify

```bash
./tools/build-and-verify.sh
./tools/build-and-verify.sh --version jp
```

Each command cleans, extracts, builds, and verifies the selected ROM against
its original SHA1 checksum. The regions have separate build outputs.

## Regional paths

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

## Matching functions and data

Use the selected region's YAML configuration, symbol file, and relocation file.
Keep addresses specific to that region.

Rebuild before comparing a function or data symbol:

```bash
python3 tools/asm-differ/diff.py --version jp --no-pager main
python3 tools/data-differ/data_diff.py --version jp --find-first-mismatch
```

Both differs also accept `VERSION=jp`. Single-symbol data comparisons need
`rom:0x... size:0x...` metadata in the region's symbol file.

Readable asset tools, progress reports, and shiftable builds currently support
US only.
