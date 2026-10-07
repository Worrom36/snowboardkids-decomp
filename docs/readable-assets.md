# Readable asset extraction

The extraction step of `./tools/build-and-verify.sh` decodes every top-level
asset segment in the supported US ROM.
It generates editable, semantically grouped N64 asset bundles using Splat
extensions and exact packers. `make asset-previews` separately exports
self-contained Blender-importable GLBs. The final ROM fill region remains
padding. Game code and assets already defined in C retain their source definitions.

Splat segments declare their actual type directly: `sprite_table`, `tilemap`,
`race_animation`, `scene_animation`, `embedded_model`, `replay`, `pointer_bank`,
`sample_bank`, or `music_sequence`. These use thin format-specific adapters over
shared extraction code; there is no `asset_format` selector in the ROM config.
Existing bundle layouts are checked during extraction. Outdated layouts require
the explicit migration command below before building.

Known asset identities are recorded with ROM starts and source evidence in
`tools/asset_names.yaml`. The six characters' graphics, model-resource and race
animation banks, shared race banks, snowboard banks, item/effect/UI sprites, pan
display list and sound pointer bank use semantic segment and symbol names.
Explicit migration updates old names, filenames, model references and linker
object paths. It compares every packed section against the existing edited
tree before switching, retaining a backup under `asset-backups/asset-names-*/`.
Conflicting destinations are rejected; repeated migration is a no-op.

Unidentified music tracks, scene-animation purposes and mixed model/UI segments
remain future naming candidates. In particular, `_1499B8` contains more than fan
geometry and is not named after that one contained model. Numeric vertex and
display-list offsets remain offsets, not inferred semantic identities.

```sh
./tools/build-and-verify.sh
make verify-assets
make asset-report
make asset-previews
```

Extraction requires the ROM with the SHA1 in `snowboardkids.yaml`. On first
extraction it creates the bundles. Subsequent extraction preserves existing bundle
sources. Preview export and inventory reports are explicit `asset-previews` and
`asset-report` targets, not part of a ROM rebuild. Cleaning preserves all assets
and removes only build/assembly artifacts. Use the wrapper with `--mode shiftable`
for intentional edits that change the original-ROM checksum. GLBs are never ROM
build inputs.

All generated Nintendo data remains ignored under `assets/` and `build/`.
No sibling SK2 checkout is required to extract or rebuild.

## Output locations

| Location | Contents |
| --- | --- |
| `assets/index.yaml` | Searchable bundle catalog, models, previews, and animation counts |
| `assets/items/<name>/` | All 11 gameplay items: icons, sprite/effect references, and pan/fan geometry with GLBs |
| `assets/characters/{slash,wendy,jam,linda,tommy,ninja}/` | Character geometry/textures, race animations, normal/ghost GLBs and scene-animation GLBs |
| `assets/courses/<course>/` | One course manifest, assembled `preview.glb`, geometry, textures, sprites, collision, paths and props |
| `assets/models/snowboards/` | Snowboard geometry and GLBs including external texture variants |
| `assets/models/unidentified/` | Other models with stable address-based identities |
| `assets/sprites/`, `assets/tilemaps/`, `assets/animations/`, `assets/replays/` | Family-specific bundles; unconfirmed font/image identities retain sprite IDs |
| `assets/audio/`, `assets/shared/audio_samples/` | Music, pointer banks, sample mappings, shared VADPCM and WAV files |
| `assets/shared/` | Resources used by multiple bundles and references to source-resident assets |
| `assets/text/`, `assets/models/source/` | C-authoritative text manifests and the source-resident fog-panel model |
| `assets/layout/` | ROM segment placement, compression metadata and generated scatter/build rules |
| `assets/source_views/` | Generated inspection views referenced by C-authoritative bundles |
| `build/asset-report.json` | ROM ranges, decoders, consumers, outputs, coverage, and optional reconstruction results |
| `build/model-preview-report.json` | Per-model GLB export success/errors, triangle counts, and animation counts |

Extraction writes intermediate records to `build/asset-staging/`. The old
`assets/course_*` and `assets/readable/` directories are retired during migration;
the original tree is recoverable under `asset-backups/semantic-*/assets/`.

### Bundles and Blender

Open `assets/items/pan/pan.glb` or `assets/courses/rookie_mountain/preview.glb`
directly with Blender's glTF importer.
Textures are embedded; copying the GLB alone is sufficient. For characters, open
`normal.glb` or `ghost.glb`; `scene_215be0.glb` and `scene_5e34a0.glb` expose the two
scene animation banks. Clips use stored frame order at a reference 60 Hz; runtime
playback speed, interpolation, and gameplay effects can change their timing.

Course previews assemble terrain, backdrop, effects and placed scenery. Hide the
backdrop node in Blender to inspect terrain from outside the enclosing sky shell.
Scenery placement and procedural item behavior remain C-authoritative; manifests
identify their source tables/renderers. Public placement values control the
inspection preview, not C-to-ROM gameplay edits.

Each `asset.yaml` identifies its previews, owned files and semantic components.
ROM-shaped records and address bindings live only under `assets/layout/`.
Geometry and textures used only by the pan are stored beside
it, while shared resources have one canonical owner. Palette words and vertices
are editable YAML; display lists retain exact command words. Original segment
offsets are retained where needed to resolve N64 pointers. Layout manifests scatter
these sources back into the original ROM sections without changing linker symbols.

GLBs are **viewing/export artifacts**, not Blender-to-ROM round-trip inputs.
Use `make asset-previews` after editing N64 sources. Export errors are fatal and
recorded in the preview report. Lighting, fog and N64 combiner effects are
approximations; actual draw-time palette bindings, baked vertex colors, and UV
modifications are decoded. Scrolling surfaces export their initial texture phase.
Runtime sprite/effect geometry is indexed with its source renderer instead of
being misrepresented as a complete standalone model.

### Migrating an existing extraction

```sh
make migrate-assets
make asset-previews
make verify-assets
```

Migration validates all 254 packed sections against the pre-migration canonical
bytes before swapping trees. It preserves canonical edits and refuses conflicting
legacy edits or destinations. Repeated extraction retains canonical sources.
`tools/semantic_assets.py --check` validates the public structure. The old
`index-models` command is intentionally disabled for the semantic catalog.

Pack an individual bundled ROM section with:

```sh
python3 tools/asset_bundles.py pack SHARED_RACE_MODEL_RESOURCES --out /tmp/shared-race.bin
```

Stable ROM-address names are retained for assets without established semantic
names. The report connects them to their loaders. For example, `SNOWBOARD_MODELS` is the
snowboard bundle, `SHARED_UI_SPRITES` contains race UI sprites including CI8 portraits,
`_5DCBE0` is the title tilemap, and `SOUND_POINTER_BANK` is the Mus pointer bank.

## Editing and rebuilding

Bundle/layout manifests use `schema_version: 2`. Internal records retain their
decoder-specific schemas. A texture has either an inline `data` field or a PNG
`path`, never both.
Image paths are relative to the manifest. Packers emit per-object `.d` files so
image, display-list, replay, and sample edits trigger the correct rebuild,
including when a manifest changes its source paths.

```sh
python3 tools/asset_bundles.py pack SHARED_UI_SPRITES --out /tmp/ui.bin
python3 tools/asset_bundles.py pack BIG_SNOWMAN_COURSE_MODEL_RESOURCES --out /tmp/course.bin
```

- PNGs must remain indexed and keep their declared dimensions. Pixel **indices**
  are authoritative; exact RGBA5551 palette words live in the manifest. Changing
  only a PNG's display palette does not change the game's palette. This preserves
  duplicate colors, transparent entries, shared palettes, and unused nibbles.
- Shared image offsets have one editable image source. Sprite entries with a
  different palette get separate inspection previews. `preview` files are not
  packer inputs.
- Model texture previews record `palette_candidates` and
  `preview_palette_offset`. A reused TMEM slot can have several candidate
  palettes; the default PNG palette is an inspection aid, not a claim about a
  unique rendered material. Display lists retain the actual palette selection.
- Tilemaps preserve their tile definitions, grid, palette slots, flips, padding,
  and one-based image indices. `preview.png` assembles the full grid. Tile zero
  is empty; unused tile records are retained.
- Geometry remains N64 display lists and vertices, with original offsets and
  rendering state. The snowboard bundle separates embedded vertices, textures,
  and command streams. Command-word C sources retain bits that macro
  disassembly can normalize. GLB previews interpret these exact sources without
  participating in their rebuild.
- Race animation frames expose two translation triples and twelve packed joint
  rotations. Rotations are 0..4088 in steps of 8; unused packed bits are retained.
  Menu/scene animations contain fourteen translation and rotation triples.
  Offset-table aliases and frame counts remain exact.
- Replay CSVs retain four players × 2,400 frames, with button bits and signed
  stick coordinates. Row order, counts, and field ranges are validated against
  the runtime's shared player/frame definitions. Replay payloads contain no
  internal pointers: their contiguous player/frame indexing is a runtime format
  requirement. Input content can change and recompress to a different encoded
  size; changing dimensions without changing the runtime row stride is rejected.
- Music manifests expose named Mus commands, notes, envelopes, and volume/pitch
  streams. Operands and variable-length duration encodings retain their original
  bytes. Loop and jump commands are listings, not an offline music synthesizer.
- The pointer bank exposes wave records, ADPCM codebooks, and loop states.
  Original `.vadpcm` samples are authoritative; WAVs are inspection previews at
  a reference 22,050 Hz. The runtime applies pitch separately. A sample crossing
  the original ROM boundary has one shared encoded file, referenced by slices
  from both segment manifests. WAV re-encoding is not used for matching builds.
- C-resident assets remain editable in their indexed `source` files. Generated
  text and sound-effect views are inspection outputs; edit the original C text
  or data definitions to change the ROM. Text decoding preserves control words,
  embedded terminators, and unknown glyphs through reversible escapes:

```sh
python3 tools/textconv.py tools/charmap.txt input.bin output.txt --decode
```

## Matching and modified builds

```sh
./tools/build-and-verify.sh
./tools/build-and-verify.sh --mode shiftable
```

The default build preserves the original layout and compression and verifies the
original ROM SHA1. Shiftable builds use `build/shiftable`, place sections using
linked sizes, and recompress changed assets deterministically. They apply the
N64 checksum without requiring the original SHA1. An unchanged shiftable build
is byte-identical to the matching build. Use the existing assembly/data differs
and linker map to investigate unexpected changes.

Linker assertions enforce the original 4 MB RAM limit, buffer alignments, and
boot placement. The original IPL3 copies 1 MB from ROM `0x1000` to the header entry
address; enlarging the boot image beyond that window requires a loader change.
Runtime buffers follow main BSS. Packing checks the allocation sizes shared in
`include/game/engine/runtime_memory.h`: music must fit its 64 KB destination, and
decoded assets plus compressed staging must fit the heap. Per-asset bounds do not
prove that every scene's live allocations fit together.

## Editing relocatable assets

Original addresses and offsets remain extraction metadata. Internal references
use stable object IDs, with source field, target, address kind, and addend.
Keep IDs attached to objects when moving them and assign new IDs to new objects.
Missing targets, invalid alignment, overflow, and unsupported structural edits
fail during packing instead of emitting stale addresses.

- Graphics word pairs carry `/* object:command_000068 kind:command */` comments.
  Embedded bytes use `kind:data`, even if their values resemble commands.
  Unannotated legacy streams cannot be resized. Display-list calls into data
  are rejected. CPU graphics roots use exported command symbols; segment
  numbers alone do not identify a loaded bank.
- Resource parts retain their original offsets as metadata. Added padding uses
  `type: padding`, an explicit `id`, and hex `data`. A resource manifest can bind
  new commands through `graphics_relocations`: `source` is `stream/command_id`,
  `field` is 4, `target` is `stream/command_id` or `bank/part_id`, `kind` is
  `segmented`, and `segment` is 2 for calls or 3 for resources. Optional `addend`
  selects an interior byte offset. Shared targets remain shared storage.
- Vertex entries have stable IDs. A load can specify its `vertices` IDs, but the
  loaded group must remain contiguous and ordered because triangle commands use
  RSP cache positions. Texture dimensions, format, lengths, and palette counts
  must agree with their display-list load commands.
- Animation clips can move independently of table entries. Sprite and tilemap
  objects retain logical identities separately from physical order. Reordered
  sprite palettes emit a slot map and select renderer code that translates CPU
  palette indices. Changed snowboard image layouts select size-aware loads and
  clamping; this can change edge pixels from the original 64×64 load of a 64×57
  image. Unchanged layouts retain the matching renderer.
- Collision coordinates and contiguous face groups can move. Surface order
  remains fixed because CPU course-progress ranges use positional indices.
- Audio waves, books, loops, and samples can move; ADPCM books remain eight-byte
  aligned for RSP DMA. The 251 logical instrument slots remain fixed because
  music, SFX, and CPU tuning tables index them. Reorder shared samples with a
  `sample_order` list in the first sample-stream manifest (`_27E290`), listing
  each sample ID once. The second partition cannot override that order.
- Music events, modulation streams, and envelope entries use stable IDs.
  Branch offsets and envelope indices follow those identities. Use
  `entry_order` to move envelopes while preserving `entry_ids` and
  `envelope_relocations`. The three track tables must have equal counts and
  remain contiguous after the header for `MusStartSong` rebasing. Nonempty
  drum maps and ambiguous shared modulation restart contexts are unsupported.
- Replay data remains four players × 2,400 frames with the runtime's fixed row
  stride. Input values can change and recompress; changing dimensions requires
  changing the playback code as well.

Build success is not a gameplay guarantee. Earlier emulator checks covered menus,
course loading, multiplayer, replay, save/reload, and a complete Big Snowman race.
Some relocated audio captures differed; full gameplay and hardware validation
remain incomplete. Check edited content in an emulator or on hardware using the
normal game flows.

## Coverage and validation

The initial converted US inventory has 254 readable top-level assets and one
trailing fill region. This counts ROM segments, not individual images or models.
Source-resident declarations have a separate index; procedural graphics created
at runtime are not separate ROM assets. `readable` means structurally decoded,
not that every field or visual identity has a semantic name.

Nonzero unclassified parts are reported as `partial`; preserving their bytes
does not count as readable extraction. Known zero padding is reported separately
from unknown data. `make verify-assets` reconstructs each registered asset and
compares it with its original ROM slice, including compression tails. It exits
nonzero on missing sources, invalid packing, or mismatches.

The existing asset codec tests can be run separately:

```sh
PYTHONPATH=tools:. python3 -m unittest tools.test_readable_assets \
  tools.test_asset_bundles \
  tools.test_course_graphics tools.test_course_sprite_table \
  tools.test_course_surface_data tools.test_course_display_list \
  tools.test_huffman_asset tools.test_textconv \
  tools.test_course_definitions tools.test_sk1_to_sk2_course
```

The tests cover palette/index preservation, shared image edits, mixed CI4/CI8,
animation packing, tilemap addressing, ADPCM prediction, music loop boundaries,
malformed inputs, and reversible text. Full ROM verification remains the final
matching check. Existing course-to-SK2 binary export continues to read the
original ROM directly.

Optional real Blender validation (Blender is not needed for extraction/builds):

```sh
blender --background --python tools/check_model_previews_blender.py -- \
  --render-dir build/model-preview-checks
```

This imports every GLB in the export report, checks model/animation presence, and
renders the pan, a character, a snowboard, and a course for visual inspection.


## Reported RT64 texture source associations

The external texture-pack investigation dated 2026-09-19 reports complete
word-swapped RDRAM image-byte matches against decompressed asset banks. Its
`texture-pack/identify_sources.py`, dumps, and generated reports remain external
and have not been reproduced here. These are reported source associations, not
new runtime symbols. All offsets below are relative to the decompressed bank;
palette entry numbers and palette byte offsets are distinguished explicitly.

| Bank | Image offset | Reported image | Palette association |
| --- | --- | --- | --- |
| `BIG_SNOWMAN_COURSE_MODEL_RESOURCES` | `0x5B80` | 16×32 scenery figure, previously mistaken for a player face; exact placed object unconfirmed | Matching palette also at `0x6F20` |
| `QUICKSAND_VALLEY_COURSE_MODEL_RESOURCES` | `0x6980` | 32×32 LIFT IN sign | Matching palette at `0x6C60` |
| `SILVER_MOUNTAIN_COURSE_MODEL_RESOURCES` | `0x2600` | 32×16 wood-like artwork | Matching palette at `0x6E20` |
| `RACE_EFFECT_SPRITES` | `0x1FE0` | Star image, sprite entry 18 | Palette entry 7 |
| `RACE_EFFECT_SPRITES` | `0x4EE0` | Fragment-like image, sprite entry 41 | Palette entry 12 |
| `SNOWBOARD_TEXTURES` | `0x4258` | Sprite entry 9, declared 64×57 | Palette entry 9 |

The source confirms that `renderRacePlayerHitEffect` selects animation groups
through `gRacePlayerHitEffectSpriteOffsets = {14, 18, 22, 26}`. Entry 18 starts
one group, not every hit effect. `initRaceUiBurstTextParticle` loads entries
39–42, including entry 41, for its normal texture animation.

The report identifies a 64×64 board-renderer load for the snowboard image whose
sprite table declares 64×57. Preserve both dimensions when identifying the atlas
or exporting replacements: the additional seven sampled rows are not automatically
authored board artwork. Do not resize the source image or change sampling based
on this association alone.

## Unresolved paint-menu border fragment

The external improvement report reviewed on 2026-10-05 describes a panel-border
fragment at native X=0..8 in the paint color list, outside logical menu bounds
16..304 (`gMenuViewportWidth=288`, `gMenuViewportCenterX=160`). Its producer is
still unidentified. `drawShopMenuSelectedModePanel` and the character-select
course frame/list callbacks are candidate border emitters, not proven sources.

Before naming or correcting a source function, capture the draw commands for
the fragment and the callback that emitted them, including coordinate transforms
and viewport/scissor state. No border behavior change is justified by the
current report alone.
