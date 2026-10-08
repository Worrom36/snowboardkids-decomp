# JP exact-function integration

The initial exact-byte list contains 137 functions. **120 functions (12,024 bytes) now compile from C** in 58 translation units.

Each C function has the expected JP address and size, identical target bytes, and no assembly nonmatching marker. The complete JP ROM passes its original SHA1 check. This checklist covers the exact-byte list; address-normalized candidates are separate work.

## Integrated C

| JP source | JP ROM start | Functions |
| --- | ---: | --- |
| [src.jp/menu/race_setup/race_setup_ui.c](../src.jp/menu/race_setup/race_setup_ui.c) | `0x171D0` | `initMenuTilemapSprite`, `n_alSynFreeFX` |
| [src.jp/menu/course_select/course_select_ui.c](../src.jp/menu/course_select/course_select_ui.c) | `0x29EA0` | `finishCourseSelectUiTask` |
| [src.jp/ending/ending_credits_slash.c](../src.jp/ending/ending_credits_slash.c) | `0x333C0` | `noopEndingCreditsSlash` |
| [src.jp/ending/ending_credits_slash_noopEndingCreditsSlashAlt.c](../src.jp/ending/ending_credits_slash_noopEndingCreditsSlashAlt.c) | `0x341F8` | `noopEndingCreditsSlashAlt` |
| [src.jp/ending/ending_credits_tommy.c](../src.jp/ending/ending_credits_tommy.c) | `0x36CC0` | `noopEndingCreditsTommy` |
| [src.jp/menu/main_menu/main_menu_scene_model.c](../src.jp/menu/main_menu/main_menu_scene_model.c) | `0x415A0` | `compressRaceRecordReplayData` |
| [src.jp/menu/main_menu/main_menu_scene_model_applyMainMenuSceneModelAnimationFrame.c](../src.jp/menu/main_menu/main_menu_scene_model_applyMainMenuSceneModelAnimationFrame.c) | `0x427B0` | `applyMainMenuSceneModelAnimationFrame` |
| [src.jp/menu/renderer/menu_render_utils.c](../src.jp/menu/renderer/menu_render_utils.c) | `0x4629C` | `resolveAssetTableRelativePointer`, `getAssetTableImageAndPalette`, `getAssetTableImageAndExplicitPalette`, `getAssetTableImagePaletteAndSize` |
| [src.jp/menu/renderer/menu_render_utils_packFixedTransformMatrix.c](../src.jp/menu/renderer/menu_render_utils_packFixedTransformMatrix.c) | `0x48E94` | `packFixedTransformMatrix` |
| [src.jp/menu/renderer/menu_render_utils_setPackedMatrixTranslation.c](../src.jp/menu/renderer/menu_render_utils_setPackedMatrixTranslation.c) | `0x49468` | `setPackedMatrixTranslation`, `copyPackedMatrixTranslation`, `scaleFixedMatrix3sByQuarter`, `noopThreeArgs`, `noopFourArgs` |
| [src.jp/race/effects/race_intro_effects.c](../src.jp/race/effects/race_intro_effects.c) | `0x56AE8` | `approachRaceIntroFlyoverSpinStep`, `approachRaceIntroFlyoverVerticalVelocity`, `approachRaceIntroFlyoverOrbitRadius`, `approachRaceIntroFlyoverPitchVelocity` |
| [src.jp/race/ui/race_hud.c](../src.jp/race/ui/race_hud.c) | `0x7B830` | `calculateRaceTimerDelta` |
| [src.jp/race/course/race_surface_cues.c](../src.jp/race/course/race_surface_cues.c) | `0x83720` | `advanceRaceSurfaceCueSequence`, `resetRacePlayerSurfaceCueState` |
| [src.jp/race/effects/snowboard_trail_effects.c](../src.jp/race/effects/snowboard_trail_effects.c) | `0x8425C` | `startSpeedFanEffect` |
| [src.jp/race/player/race_player_movement.c](../src.jp/race/player/race_player_movement.c) | `0x8B968` | `updateRacePlayerLeanAngle` |
| [src.jp/race/player/race_player_movement_addRacePlayerScore.c](../src.jp/race/player/race_player_movement_addRacePlayerScore.c) | `0x8C0BC` | `addRacePlayerScore` |
| [src.jp/race/player/race_player_update.c](../src.jp/race/player/race_player_update.c) | `0x8F700` | `resetRacePlayerTrickSubstate` |
| [src.jp/race/player/race_player_update_updateRacePlayerTrickSubstateNoop.c](../src.jp/race/player/race_player_update_updateRacePlayerTrickSubstateNoop.c) | `0x8FAAC` | `updateRacePlayerTrickSubstateNoop` |
| [src.jp/math/fixed_point_math.c](../src.jp/math/fixed_point_math.c) | `0x98230` | `multiplyFixedMatrix3s` |
| [src.jp/libmus/player.c](../src.jp/libmus/player.c) | `0x9D190` | `Fstop`, `Fwave`, `Fport`, `Fportoff`, `Fdefa` |
| [src.jp/libmus/player_Fendit.c](../src.jp/libmus/player_Fendit.c) | `0x9D40C` | `Fendit`, `Fcutoff`, `Fvibup`, `Fvibdown`, `Fviboff`, `Flength`, `Fignore`, `Ftrans`, `Fignore_trans`, `Fdistort` |
| [src.jp/libmus/player_Fenvoff.c](../src.jp/libmus/player_Fenvoff.c) | `0x9D628` | `Fenvoff`, `Fenvon`, `Ftroff`, `Ftron`, `Ffor`, `Fnext`, `Fwobble`, `Fwobbleoff`, `Fvelon`, `Fveloff`, `Fvelocity`, `Fpan`, `Fstereo`, `Fdrums` |
| [src.jp/libmus/player_Fgoto.c](../src.jp/libmus/player_Fgoto.c) | `0x9D838` | `Fgoto`, `Freverb` |
| [src.jp/libmus/player_Fvolume.c](../src.jp/libmus/player_Fvolume.c) | `0x9D99C` | `Fvolume` |
| [src.jp/libmus/player_Fbendrange.c](../src.jp/libmus/player_Fbendrange.c) | `0x9DA88` | `Fbendrange` |
| [src.jp/libmus/player_MusIntInitEnvelope.c](../src.jp/libmus/player_MusIntInitEnvelope.c) | `0x9F05C` | `__MusIntInitEnvelope`, `__MusIntProcessEnvelope`, `__MusIntProcessWobble` |
| [src.jp/libmus/player_MusIntProcessContinuousVolume.c](../src.jp/libmus/player_MusIntProcessContinuousVolume.c) | `0x9F4E4` | `__MusIntProcessContinuousVolume`, `__MusIntProcessContinuousPitchBend` |
| [src.jp/libmus/player_MusIntMemSet.c](../src.jp/libmus/player_MusIntMemSet.c) | `0x9FC38` | `__MusIntMemSet` |
| [src.jp/ultra/libc/ll.c](../src.jp/ultra/libc/ll.c) | `0xA3AB0` | `__ull_rshift`, `__ull_rem`, `__ull_div`, `__ll_lshift`, `__ll_rem`, `__ll_div`, `__ll_mul`, `__ull_divremi`, `__ll_mod`, `__ll_rshift` |
| [src.jp/ultra/audio/heapinit.c](../src.jp/ultra/audio/heapinit.c) | `0xA53D0` | `alHeapInit` |
| [src.jp/ultra/audio/heapalloc.c](../src.jp/ultra/audio/heapalloc.c) | `0xA5410` | `alHeapDBAlloc` |
| [src.jp/ultra/audio/sl.c](../src.jp/ultra/audio/sl.c) | `0xA5D80` | `alUnlink`, `alLink` |
| [src.jp/ultra/io/aigetlen.c](../src.jp/ultra/io/aigetlen.c) | `0xA5F70` | `osAiGetLength` |
| [src.jp/ultra/os/thread.c](../src.jp/ultra/os/thread.c) | `0xA6660` | `__osDequeueThread` |
| [src.jp/ultra/io/crc.c](../src.jp/ultra/io/crc.c) | `0xA7640` | `__osContAddressCrc`, `__osContDataCrc` |
| [src.jp/ultra/io/contpfs.c](../src.jp/ultra/io/contpfs.c) | `0xA7FE0` | `__osSumcalc`, `__osIdCheckSum` |
| [src.jp/ultra/libc/string.c](../src.jp/ultra/libc/string.c) | `0xAA460` | `memcpy`, `strlen`, `strchr` |
| [src.jp/ultra/gu/mtxutil.c](../src.jp/ultra/gu/mtxutil.c) | `0xAA500` | `guMtxF2L`, `guMtxIdentF` |
| [src.jp/ultra/gu/mtxutil_guMtxL2F.c](../src.jp/ultra/gu/mtxutil_guMtxL2F.c) | `0xAA6B8` | `guMtxL2F` |
| [src.jp/ultra/io/pirawread.c](../src.jp/ultra/io/pirawread.c) | `0xAAA90` | `osPiRawReadIo` |
| [src.jp/ultra/io/spsetstat.c](../src.jp/ultra/io/spsetstat.c) | `0xAC350` | `__osSpSetStatus` |
| [src.jp/ultra/io/spsetpc.c](../src.jp/ultra/io/spsetpc.c) | `0xAC360` | `__osSpSetPc` |
| [src.jp/ultra/io/sp.c](../src.jp/ultra/io/sp.c) | `0xAC430` | `__osSpDeviceBusy` |
| [src.jp/ultra/io/ai.c](../src.jp/ultra/io/ai.c) | `0xAC540` | `__osAiDeviceBusy` |
| [src.jp/ultra/audio/drvrnew.c](../src.jp/ultra/audio/drvrnew.c) | `0xAC830` | `init_lpfilter` |
| [src.jp/ultra/audio/mainbus.c](../src.jp/ultra/audio/mainbus.c) | `0xACDB0` | `alMainBusParam`, `alMainBusPull` |
| [src.jp/ultra/audio/load.c](../src.jp/ultra/audio/load.c) | `0xAD0D4` | `alRaw16Pull` |
| [src.jp/ultra/audio/env.c](../src.jp/ultra/audio/env.c) | `0xADD50` | `_ldexpf`, `_frexpf`, `alEnvmixerParam` |
| [src.jp/ultra/audio/auxbus.c](../src.jp/ultra/audio/auxbus.c) | `0xAE9B0` | `alAuxBusParam`, `alAuxBusPull` |
| [src.jp/ultra/audio/save.c](../src.jp/ultra/audio/save.c) | `0xAEAC0` | `alSaveParam`, `alSavePull` |
| [src.jp/ultra/io/si.c](../src.jp/ultra/io/si.c) | `0xAEB80` | `__osSiDeviceBusy` |
| [src.jp/ultra/io/epirawwrite.c](../src.jp/ultra/io/epirawwrite.c) | `0xB02F0` | `osEPiRawWriteIo` |
| [src.jp/ultra/io/epirawread.c](../src.jp/ultra/io/epirawread.c) | `0xB0340` | `osEPiRawReadIo` |
| [src.jp/ultra/audio/filter.c](../src.jp/ultra/audio/filter.c) | `0xB0440` | `alFilterNew` |
| [src.jp/ultra/audio/reverb.c](../src.jp/ultra/audio/reverb.c) | `0xB0460` | `_doModFunc` |
| [src.jp/ultra/audio/reverb_alFxParam.c](../src.jp/ultra/audio/reverb_alFxParam.c) | `0xB0D54` | `alFxParam` |
| [src.jp/ultra/audio/copy.c](../src.jp/ultra/audio/copy.c) | `0xB10B0` | `alCopy` |
| [src.jp/ultra/libc/ldiv.c](../src.jp/ultra/libc/ldiv.c) | `0xB1230` | `ldiv` |

## Remaining assembly

These 17 entries use handwritten assembly in the US source tree. They remain assembly in JP; they are not counted as C matches. Cache, TLB, and processor-register operations require machine instructions unavailable in ordinary C. The thread-queue helpers also preserve specific temporary-register conventions used by exception handling. The two libc memory routines retain their hand-scheduled implementations.

| Function | JP ROM | US source |
| --- | ---: | --- |
| `osWritebackDCache` | `0xA3D70` | [src.us/ultra/os/writebackdcache.s](../src.us/ultra/os/writebackdcache.s) |
| `osInvalDCache` | `0xA46B0` | [src.us/ultra/os/invaldcache.s](../src.us/ultra/os/invaldcache.s) |
| `osWritebackDCacheAll` | `0xA4E70` | [src.us/ultra/os/writebackdcacheall.s](../src.us/ultra/os/writebackdcacheall.s) |
| `__osDisableInt` | `0xA66A0` | [src.us/ultra/os/interrupt.s](../src.us/ultra/os/interrupt.s) |
| `__osRestoreInt` | `0xA66C0` | [src.us/ultra/os/interrupt.s](../src.us/ultra/os/interrupt.s) |
| `__osEnqueueThread` | `0xA70EC` | [src.us/ultra/os/exceptasm.s](../src.us/ultra/os/exceptasm.s) |
| `__osPopThread` | `0xA7134` | [src.us/ultra/os/exceptasm.s](../src.us/ultra/os/exceptasm.s) |
| `__osSetSR` | `0xAA8E0` | [src.us/ultra/os/setsr.s](../src.us/ultra/os/setsr.s) |
| `__osGetSR` | `0xAA8F0` | [src.us/ultra/os/getsr.s](../src.us/ultra/os/getsr.s) |
| `__osSetFpcCsr` | `0xAA900` | [src.us/ultra/os/setfpccsr.s](../src.us/ultra/os/setfpccsr.s) |
| `osInvalICache` | `0xAA9B0` | [src.us/ultra/os/invalicache.s](../src.us/ultra/os/invalicache.s) |
| `osMapTLBRdb` | `0xAAA30` | [src.us/ultra/os/maptlbrdb.s](../src.us/ultra/os/maptlbrdb.s) |
| `_bzero` | `0xAAAF0` | [src.us/ultra/libc/bzero.s](../src.us/ultra/libc/bzero.s) |
| `osGetCount` | `0xAC030` | [src.us/ultra/os/getcount.s](../src.us/ultra/os/getcount.s) |
| `_bcopy` | `0xAC040` | [src.us/ultra/libc/bcopy.s](../src.us/ultra/libc/bcopy.s) |
| `__osProbeTLB` | `0xAC480` | [src.us/ultra/os/probetlb.s](../src.us/ultra/os/probetlb.s) |
| `__osSetCompare` | `0xB0430` | [src.us/ultra/os/setcompare.s](../src.us/ultra/os/setcompare.s) |
