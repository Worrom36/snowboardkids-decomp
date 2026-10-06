# Readable asset extraction

`make extract` now decodes every top-level asset segment in the supported US ROM.
It generates editable, semantically grouped N64 asset bundles and self-contained
Blender-importable GLB previews, using Splat extensions and exact packers. The final ROM fill region
remains padding. Game code and assets already defined in C retain their existing
source definitions.

Splat segments declare their actual type directly: `sprite_table`, `tilemap`,
`race_animation`, `scene_animation`, `embedded_model`, `replay`, `pointer_bank`,
`sample_bank`, or `music_sequence`. These use thin format-specific adapters over
shared extraction code; there is no `asset_format` selector in the ROM config.
Existing bundle layouts migrate their decoder names automatically without moving
editable files or changing linker object identities.

Known asset identities are recorded with ROM starts and source evidence in
`tools/asset_names.yaml`. The six characters' graphics, model-resource and race
animation banks, shared race banks, snowboard banks, item/effect/UI sprites, pan
display list and sound pointer bank use semantic segment and symbol names.
Extraction automatically migrates old names, filenames, model references and
linker object paths. It compares every packed section against the existing edited
tree before switching, retaining a backup under `asset-backups/asset-names-*/`.
Conflicting destinations are rejected; repeated migration is a no-op.

Unidentified music tracks, scene-animation purposes and mixed model/UI segments
remain future naming candidates. In particular, `_1499B8` contains more than fan
geometry and is not named after that one contained model. Numeric vertex and
display-list offsets remain offsets, not inferred semantic identities.

```sh
make extract
make verify-assets
make asset-report
make asset-previews
./tools/build-and-verify.sh
```

`make extract` requires the ROM with the SHA1 in `snowboardkids.yaml`. On first
extraction it creates the bundles. Subsequent extraction preserves existing bundle
sources and refreshes their GLBs. `make clean` now preserves **all assets**; it
only removes build/assembly artifacts. Build edits with `make`; intentional edits
will naturally change the original-ROM checksum. GLBs are never ROM build inputs.

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

Matching compression retains Huffman tables, LZ token choices, padding bits,
and unused tails. `./tools/build-and-verify.sh` retains the original SHA1 check.
`./tools/build-and-verify.sh --mode shiftable --shift-tests` uses isolated outputs
under `build/shiftable`, deterministic variable-size compression, flexible ROM
placement, linked BSS and runtime buffers, and the original 4 MB RAM limit.
An unchanged shiftable build is byte-identical to the matching ROM.

Shiftable packing checks the allocation constants shared with the runtime in
`include/game/engine/runtime_memory.h`. Music must fit its 64 KB DMA destination;
decoded data plus compressed staging must fit the heap with 16-byte allocation
rounding. These are necessary per-asset bounds, not proof that every scene's live
allocations fit together. Samples stream from ROM and do not use that bound.
The sound bank retains 251 logical instrument slots because music/SFX indices
and CPU tuning tables use those slots; changing the count fails with a diagnostic.
Individual wave records, books, loops, and shared samples can move independently.

Internal relocation currently covers animation tables, audio-bank objects and
shared samples, music stream tables, snowboard graphics, sprite images and
palettes, tilemap tables, and course/character resource objects. Character
graphics commands export stable symbols used by menu and race model tables.
All 19 course/character resource groups are checked against the original
payloads, including 5,886 graphics reference fields. Changing compressed sizes
automatically moves subsequent ROM assets. ADPCM codebook coefficient arrays
retain the eight-byte alignment required by direct RSP DMA; invalid matching
offsets are rejected. The internal-audio fixture reorders pointer-bank objects
while retaining the original ROM extent to isolate decoded audio relocation.
Animation clips can be reordered independently of table entries, including moving
the clip referenced by entry zero. Table decoding uses the earliest referenced
clip to find the table boundary. Shared audio samples can be reordered with a
`sample_order` list in the first sample-stream manifest (`_27E290`); list every
stable sample ID exactly once. The second partition cannot override that order.
The artificial ROM cut between sample partitions remains independent of sample
identities, and all pointer-bank users resolve against the resulting shared layout.

Original offsets remain extraction metadata. Parts can carry an explicit `id`;
relocation records identify their source object and field, target object, address
kind, and addend. Newly extracted graphics word pairs carry
`/* object:command_000068 kind:command */` comments. Keep these IDs with their word pairs when
moving them, and assign new IDs to inserted commands. Legacy unannotated graphics
cannot be resized. Resource parts retain their original offsets when reordered;
new padding uses `type: padding`, an explicit `id`, and hex `data`. Generated
graphics `.relocations.json` files report resolved targets and field locations.

Embedded bytes use `kind:data`; relocation and texture-load validation skip those
words even when they resemble address-bearing commands. The three original vertex
data ranges in Big Snowman and Grass Valley are declared in `snowboardkids.yaml`
with extraction offsets and reasons. Regeneration preserves their type annotations;
legacy sources inherit the type from their original object IDs. Keep `kind:data`
with newly inserted or moved raw words. Display-list calls into data and unsupported
address-bearing commands are rejected instead of emitting an unbound address.

New commands can use a resource manifest's `graphics_relocations` list instead
of inventing an original numeric address. Each record contains `source`
(`stream_name/command_id`), `field: 4`, `target` (`stream_name/command_id` for
display-list calls or `resource_bank/part_id` for loads), `kind: segmented`,
and `segment` (2 for calls, 3 for resources). Optional `addend` selects a byte
offset. Vertex loads may specify a `vertices` list of stable vertex IDs whose
length matches the command's count. Multiple commands can name one new resource;
they share its storage. Missing targets, duplicate bindings, wrong target types,
and unused bindings fail with an asset/field diagnostic.

Newly extracted vertex entries carry `id: vertex_000010`, relative to the original
vertex part. Keep each ID attached to its vertex when inserting or moving entries.
Vertex loads follow these identities in both embedded models and course/character
resources. A loaded group must stay contiguous and ordered because triangle
commands refer to its RSP cache positions. Missing IDs, duplicate IDs, split loads,
and resized legacy arrays without IDs fail explicitly.
Texture dimensions, CI format, encoded lengths, and palette counts are checked
against the relocated display-list loads. Change the corresponding load commands
with the resource when changing those properties; stale load metadata is rejected.

Music sequence and modulation events also have stable IDs. All three Mus
`goto` operands resolve to event identities relative to their sequence, volume,
and pitch restart streams. Event insertion updates branch offsets and stream
tables; malformed command state, missing targets, and 16-bit branch overflow
fail with a diagnostic. A shared goto with multiple modulation restart contexts
is rejected until those contexts can be represented unambiguously.

Course graphics commands also export stable symbols. Active water, trigger,
scrolling-texture, shared race-object, and rank-model CPU fields bind to those
symbols; unused historical rank-table slots are explicitly marked. The combined
fixture moves commands and resources in all 19 groups, verifies their final linked
fields and decoded resource objects, and also shifts main code and ROM placement.
Some structural changes still fail explicitly. Collision
coordinates and contiguous face groups use stable IDs; counts and indices follow
their new positions. Existing surface order is retained because CPU course-progress
ranges use positional surface indices. Reordering surfaces requires those CPU
bindings first. Replay snapshot matrix/vertex fields are archived render caches:
the first viewport sets `gRenderMatricesDirty`, and the model, shadow, and speed-fan
renderers replace them with current scratch allocations before drawing.
Passing ROM/RAM and packed-field tests does not establish full shiftability.
Combined runtime fixtures are emitted under `build/shiftable/shift-tests`; boot,
menus, every course, multiplayer, audio, replay, save flows, and hardware remain
separate acceptance checks.

The compiled-instruction audit also checks cached-RAM `lui` literals. Its remaining
fixed cases are the KSEG0 base/sign bit, libultra's indexed-cache traversal, and
the CPU exception vectors. Unexpected unrelocated RAM high halves or constructed
CPU addresses fail the shift tests; the inventory records each fixed case and
its uses alongside the compiled data-field audit.
The ROM boundary source check includes newly added source files and the actual
generated assembly inputs named by the linker script. Missing generated inputs
fail explicitly. Historical matching-assembly mirrors remain in the broader
inventory rather than being treated as active build inputs.
The source guard also rejects interior ROM literals in the address arguments of
DMA and asset-loading APIs, and interior legacy `D_...` ROM names in generated
assembly. Symbol-relative offsets, transfer sizes, and scalar constants are not
classified as pointers by their numeric range alone. This guard does not perform
general C dataflow analysis; compiled relocations and packed-format checks remain
necessary for references passed indirectly through variables and tables.

The 21 legacy CPU graphics aliases in `game_syms.ld` bind to individual command
identities, including roots inside shared streams. `combined-interior-graphics.z64`
inserts unreachable data after display-list terminators as well as leading
padding. Gap sizes vary by stream so overlapping bank addresses also diverge.
Interior and downstream roots therefore move by different amounts.
The linked-field checks follow the exported target identities and reject a
deliberately restored stale bundle-relative alias.
Segment numbers alone do not identify a bank: podium roots bind to `_145380`,
menu boards to `_1467B0`, and race items to the shared race bank. Their DMA
contexts are distinct even where original segmented addresses coincide.
Two extracted video-mode words also resemble segment-2 pointers, but encode
`OSViMode.type` and padding. Explicit extraction annotations preserve them as
scalars, and linked checks reject relocations or changed bytes in those fields.
The distinct-bank fixture now passes all 11 menu/early-race captures, including
the previously failing title screen, and all three award podium variants with
four captures each. It also passes loading and early gameplay on all ten courses
and two- and four-player split screen, plus both Big Snowman laps and results.
Generate the podium scenarios with
`python3 tools/runtime_podium_scenarios.py --out /tmp/sbk-podiums`. They inject
completed progression after replay exit and assert the awarded level; they test
bank loading and drawing, not normal unlock requirements.

`combined-formats.z64` additionally permutes all eight animation banks, 30 sprite
tables, and seven tilemaps alongside graphics, audio-bank, ROM, and RAM shifts.
Sprite manifests retain logical `palette_ids` separately from physical palette
order. Reordered palettes append a slot map; eight CPU selection paths translate
logical indices through it. Links select the optional renderer only when a map
is needed. Linked verification checks every translator call, including a negative
test with a corrupted JAL target.

Moving snowboard images also exposed the original renderer's 64×64 load from
64×57 objects: filtering could sample the next object's bytes. Changed snowboard
layouts select a renderer that loads and clamps to each image's declared size.
CI4 images must fit 2 KB texture memory and have 16-texel row alignment. This
intentionally changes edge clamping; it does not claim identical board-edge pixels
against the original renderer. Unchanged matching and shiftable links retain the
original instructions and ROM bytes.

The combined-format fixture now passes all 11 menu/early-gameplay captures in
4 MB RAM against `runtime-indices-baseline.z64`, which uses the same optional
renderers with original asset layouts. This comparison isolates relocation from
the edge-clamping correction. The earlier failed run remains in the runtime
record as evidence of the CPU bindings that packed-field checks alone missed.
It also passes loading and early gameplay on all ten courses, two- and
four-player split screen, and an Exit/Save/restart round trip with the unique
money value restored in both saved and live-player state.

`combined-course-music-samples.z64` extends that fixture with all ten collision
coordinate tables and face groups, all 17 music stream layouts, and all 251
shared audio samples. Collision surface order remains fixed because CPU course
progress uses those indices; overlapping face ranges move together. Semantic
checks follow the final encoded coordinate, face, track, drum, and sample
references and preserve sharing. Independent `internal-samples.z64` and
`internal-music.z64` fixtures separate audio changes from the other
formats. The recorded expanded fixture passes menus, loading/early gameplay on
all ten courses, two- and four-player split screen, and Exit/Save/restart. Those
captures moved music streams while retaining envelope entry order; the separate
envelope capture below checks that additional permutation. ROM hashes distinguish
the tested variants. These checks do not cover complete races or hardware.
The later combined fixture also permutes envelope entries and passes the same
course/menu/multiplayer/save/reload checks and title-replay exit. An extended
CPU-driven Big Snowman run completes both laps and enters results with six
matching captures. Generate it with
`python3 tools/runtime_course_scenarios.py --extended-cpu-race --out /tmp/sbk-full-race`;
the debugger enables CPU control
after setup. This covers the full course path and results, not human steering
or full completion of every course.
The isolated shared-sample fixture passes its decoded target checks and boot
screenshots, but exact PCM comparison currently fails after stereo frame 321860.
It retains the original ROM extents and identical boot/code bytes through
`0x101000`, so this case requires investigation beyond the ROM-padding boot
timing difference. It is not recorded as a passing audio result.
`tools/runtime_audio_dma_probe.py` prepares a diagnostic pair that marks the
exhausted-DMA-pool fallback and asserts that marker every VI after startup.
Both diagnostic runs pass all 1,690 checks while PCM still differs; forcing
exhaustion at frame 600 correctly fails at 601. This narrows the investigation
without accepting the audio difference. The probe alters code/checksum timing,
so its result describes the instrumented ROMs, not every original execution.

Music envelopes have stable `entry_ids`, an optional physical `entry_order`, and
explicit `envelope_relocations` from sequence events. Packing updates all
envelope command indices, including variable-length index growth, before resolving
event and `goto` offsets. The three track tables determine the header track count
and must have equal lengths. They remain together immediately after the header:
`MusStartSong` rebases that contiguous pointer block in place. Nonempty drum maps
and drum commands currently fail with a typed-binding diagnostic; the original
17 music assets have empty drum maps. Their wave/envelope indices must be modeled
before that structural extension is supported.
The isolated music fixture with moved streams and envelopes passes exact PCM
and screenshot comparison through boot/title playback. The longer all-bank test
loads all 17 distinct music banks through the game's pending request queue and
passes playback-state and screenshot assertions, but exact PCM differs during
part of the transition sequence. Isolating bank 11 produces identical PCM over
the common capture duration, with six extra stereo frames at the shifted run's
end; strict equality still fails. These results remain separate from an audio
acceptance pass. Generate this temporary scenario with
`python3 tools/runtime_music_scenario.py --out /tmp/sbk-music.json`; `--bank`
selects individual banks or a shorter transition sequence.
`title-replay-exit.json` separately asserts
recorded-input playback, presses Start, checks the replay fade state, and compares
five captures against the combined fixture. `post-race-replay-exit.json` records
partial time-trial history, injects a finish flag, enters post-race playback,
and exits with Start. Seven captures match the combined fixture; assertions
check results, history-input playback, and camera activation/deactivation.
`ghost-save.json` additionally enables record qualification, compresses the
history into a 166-word saved replay, and uses the normal Exit/Save UI.
`ghost-reload.json` restarts from each run's own pak and asserts the restored
length/tokens and activation of the second racer as a history-input ghost. All six save
and seven reload captures match the distinct-bank fixture. These tests still
inject finish/qualification and menu mode values; they do not establish natural
time-trial completion or qualification.
Scenarios can set `compare_mempak_data: true` to require identical saved data.
The comparison validates pak ID checksums/backups and excludes only serial
numbers and their ID checksums. Device metadata, allocation tables, directories,
reserved bytes, and game data must match. Both ghost runs pass that comparison;
the saved pak images differ only in their independently formatted IDs.

An unmodified-ROM sample-order probe additionally observes 24 audio channels
around the first PCM divergence. Wave indices, cached pitches, and playing flags
match at all 353 observation points. Note/envelope differences occur only when
the music clocks differ by one update; channels 0–13 have identical observed
states at all 173 shared music-clock values. DMA request counts also differ.
This supports a scheduling explanation, but does not prove exact PCM equivalence
or replace validation of every synthesis state. Retriggered SFX cannot be aligned
by a clock value alone; the recorded analysis keeps those cases separate.

`tools/runtime_shift_smoke.py` accepts deterministic controller scenarios, optional
ELF-resolved memory assertions, isolated controller-pak seeds, and optional AI PCM
capture. It records emulator/plugin and ROM hashes, screenshots, observations,
and audio WAV files. All-blank renderer output is rejected. Interrupt randomization
is disabled so unchanged-ROM audio repeats are deterministic. PCM comparison
requires an LLE RSP; the capture plugin rejects HLE audio processing.
Per-observation frame lists schedule their own checks even between screenshots;
empty or out-of-range assertion schedules fail before emulation.

The combined all-graphics fixture passed Mupen64Plus 2.6.0 with Angrylion Plus and
z64 RSP in 4 MB RAM through fresh-pak setup, menus, character/board selection,
and early gameplay in all ten course environments. Each course comparison checks
three captures and the linked course-index symbol. Animal Land uses its normal
Skill Game / Trick Game path; the other generated course scenarios bypass unlocks
with a temporary debugger write before asset loading. These checks cover loading
and early gameplay, not full course completion. Rice's board-preview comparison
differs after internal model relocation; the same fixture passes Angrylion/z64.

Generate temporary course scenarios with
`python3 tools/runtime_course_scenarios.py --out /tmp/sbk-courses`, then run:

```sh
DISPLAY=:92 LIBGL_ALWAYS_SOFTWARE=1 python3 tools/runtime_shift_smoke.py \
  build/snowboardkids.z64 build/shiftable/shift-tests/combined-all-graphics.z64 \
  --baseline-elf build/shiftable/shift-tests/baseline.elf \
  --shifted-elf build/shiftable/shift-tests/combined-all-graphics.elf \
  --emulator-root /path/to/local/emulator-prefix --out /tmp/sbk-course-result \
  --scenario /tmp/sbk-courses/course-00.json \
  --video-plugin video-angrylion-plus --rsp-plugin rsp-z64 --timeout 300
```

An existing X display and locally installed Mupen64Plus core, development headers,
and named plugins are required. The harness compiles its test controller/audio
plugins locally. Use a new output directory for each comparison. Add
`--capture-audio --scenario config/shift-tests/boot-audio.json` for AI PCM checks.
The unchanged-ROM repeat and text/RAM-only shift pass exact PCM comparison.
ROM-padding and combined shifts still differ in audio timing; combined audio
acceptance remains unresolved. The independent internal-audio fixture passes exact
PCM comparison after all pointer-bank body objects move. The combined graphics
fixture also passes two- and four-player split-screen comparisons and an Exit/Save/restart
round trip: both saved and live-player money reload as the unique fixture value.
Use `save-money.json`, then `reload-money.json` with `--baseline-mempak` and
`--shifted-mempak` pointing to the respective isolated save images. These scenarios
change only emulator RAM and temporary pak files. Use the same seed options for
`ghost-reload.json` after `ghost-save.json`. Coverage remains scoped to the recorded
scenarios: full completion of the other courses, natural time-trial qualification,
and hardware validation are still pending.
Recorded ROM/plugin hashes, capture hashes, memory assertions, and audio results
are retained in `docs/shift-runtime-results.json`, including the unresolved audio
comparisons. Temporary screenshots, logs, PCM, and controller-pak files remain in
the individual run directories rather than in the source tree.

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

The codec checks run as part of `./tools/build-and-verify.sh --shift-tests`.
To run them separately:

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
