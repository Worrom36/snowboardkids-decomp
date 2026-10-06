/*
 * Project-local aliases for symbols still referenced by extracted assembly.
 *
 * Prefer aliases to canonical splat-generated asset symbols so that renamed
 * C references and legacy assembly references resolve to the same address.
 */

/* Shared race-model overlay addresses. */
D_2000000 = SHARED_RACE_GRAPHICS_command_000000;
D_20006C8 = SHARED_RACE_GRAPHICS_command_0006C8;
D_2000910 = _148220_command_000000;
D_2000D78 = _148440_command_000248;
D_2000E48 = _148440_command_000318;
D_2000E70 = _148440_command_000340;
D_2001678 = _148F88_command_000000;
D_2001730 = _149040_command_000000;
D_2001810 = _149120_command_000000;
D_20018E8 = _1491F8_command_000000;
D_20019C0 = _ADDR_20019C0_command_000000;
D_2001D00 = _149610_command_000000;
gSpeedFanFrontDisplayList = _1499B8_command_000160;
gSpeedFanBackDisplayList = _1499B8_command_000300;
D_2002490 = _ADDR_2002490_command_000000;
D_2002660 = PAN_DISPLAY_LIST_command_000000;
D_20028F0 = _14A200_command_000000;
D_2002DB8 = _14A200_command_0004C8;
gRaceItemPickupDisplayList = SHARED_RACE_GRAPHICS_SUFFIX_command_000000;
gRaceActionPickupDisplayList = SHARED_RACE_GRAPHICS_SUFFIX_command_000080;

/* Rookie Mountain overlay address. */
D_20058A8 = ROOKIE_MOUNTAIN_COURSE_BACKDROP_DISPLAY_LIST_command_000000;

/* Fixed physical/virtual addresses encoded in original data. */
/* Physical GPU alias of gDepthBuffer (0x80369000), not an image asset. */
D_369000 = gDepthBuffer - 0x80000000;
D_80000000 = 0x80000000;

/* Typed slot-zero subobject views used where IDO must retain distinct relocations. */
gPrimaryCourseUnlockStates = gGameSaveDataBuffer + 0x3F;
gCourseUnlockScanStart = gGameSaveDataBuffer + 0x1;
gCourseUnlockScanEnd = gGameSaveDataBuffer + 0x9;
gPrimaryGameSaveRawData = gGameSaveDataBuffer;

/* Character-select course-menu data subobjects. */
gCharacterSelectCourseOptionsByUnlock = gCharacterSelectCourseMenuData;
gPlayerBadgeDisplayOrder = gCharacterSelectCourseMenuData + 0x42;
gCharacterSelectShortCourseOptions = gCharacterSelectCourseMenuData + 0x58;
gCharacterSelectSingleCourseOption = gCharacterSelectCourseMenuData + 0x60;

/* Race-course effect data subobject used by extracted collision assembly. */
D_800DA856 = gCourseTriggerEntries + 0x16;

/* Race-player ground probe subobjects used by extracted movement assembly. */
D_800DE7F8 = gRacePlayerGroundProbeOffsets + 0x48;
D_800DE7FC = gRacePlayerGroundProbeOffsets + 0x4C;
D_800DE814 = gRacePlayerGroundProbeOffsets + 0x64;
D_800DE818 = gRacePlayerGroundProbeOffsets + 0x68;

/* Keep the end sentinel opaque to IDO loop-trip-count folding. */
gRaceCamerasEnd = gRaceCameras + 0x2C0;

/* Eight 0x0C slots; existing queue names alias their head words. */
gModelRenderCallbackQueues = gModelRenderCallbackList;

/* ORI uses an unsigned low half: do not use a carry-adjusted HI16 here. */
main_BSS_CLEAR_HI = main_BSS_SIZE >> 16;
main_BSS_CLEAR_LO = main_BSS_SIZE & 0xFFFF;
ASSERT(main_BSS_SIZE > 0 && (main_BSS_SIZE & 7) == 0, "boot BSS clear requires nonempty 8-byte units")
ASSERT(buffers_BSS_START >= main_BSS_END, "runtime buffers overlap main BSS")
ASSERT(buffers_BSS_END <= 0x80400000, "runtime layout exceeds original 4 MB RAM")
ASSERT((buffers_BSS_START & 15) == 0, "runtime buffers require 16-byte alignment")
ASSERT((gDepthBuffer & 63) == 0, "depth buffer requires 64-byte alignment")
ASSERT((gFramebuffers & 63) == 0, "framebuffers require 64-byte alignment")
ASSERT(entry_ROM_START == 0x1000 && entry_VRAM == 0x80000400, "hardware boot placement changed")

/* Typed views of the packed menu-effect data consumed by race intros. */
gRaceIntroCourseVertices = passAwardLabelFlashTileOffsets + 0x24;
gRaceIntroAlternateCourseVertices = passAwardLabelFlashTileOffsets + 0x1A4;
gRaceIntroCourseCommands = passAwardLabelFlashTileOffsets + 0x264;
gRaceIntroAlternateCourseCommands = passAwardLabelFlashTileOffsets + 0x3E4;
