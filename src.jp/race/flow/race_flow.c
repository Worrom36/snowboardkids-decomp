#include "common.h"
#include "game/save_data.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/render_callback.h"
#include "assets.h"
#include "game/audio/sound_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/asset_manager.h"
#include "game/engine/system_runtime.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/course_select/multiplayer_course_select_menu.h"
#include "game/menu/controller_pak/controller_pak_race_record_save_flow.h"
#include "game/menu/controller_pak/controller_pak_replay_save_message_flow.h"
#include "game/menu/controller_pak/controller_pak_file_delete_flow.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/race_setup/race_setup_menu.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_screen_effects.h"
#include "game/menu/main_menu/main_menu_panel_ui.h"
#include "game/menu/race_type_select/race_type_select_menu.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_menu.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/race_course_effects.h"
#include "game/race/flow/race_flow.h"
#include "game/race/race_state.h"
#include "game/race/race_replay.h"
#include "game/race/player/race_player_update.h"
#include "game/race/scene/race_scene_setup.h"
#include "game/race/effects/race_start_transition.h"
#include "game/race/ui/race_hud.h"
#include "game/ending/ending_credits_flow.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/menu/controller_pak/controller_pak_continue_prompt_flow.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/engine/viewport_manager.h"
#include "game/race/player/race_player_input.h"

#define RACE_PLAYER_READY_FLAG 0x40
#define COURSE_GRID_ENTRY_FREE -1
#define COURSE_GRID_ENTRY_END -2
typedef struct {
    s16 status;
    s16 courseId;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} CourseGridEntry;

typedef struct {
    s32 unk0;
    u8 pad4[4];
    s8 unk8;
} Unk80043040;

extern u16 D_800DB8F0[16];

extern u8 gRaceUpdatePaused;

extern s32 gRaceCameraReplayStartY;

extern s32 gRaceCameraReplayStartZ;

extern s32 gRaceCameraReplayStartX;

extern s16 gRaceLapCount;

extern CourseGridEntry *D_800DB7C0[12];

extern s8 D_80120E81;
extern u8 gPendingEndingCreditsFlow;
extern s32 gMenuFlowState;
extern u8 gRumblePakConnectedMask;
extern s16 gMenuFadeAlpha;
extern u8 D_80120E80;
extern u8 D_80120E77;

void initStartupControllerPakFlow(void) {
    LOAD_ASSET(_2427D0, 6);
    initCallbackTaskScheduler(0);
    gCurrentGameTask->callbackData0 = 0xA;
    setCurrentGameTaskCallback(&waitStartupRumbleInit, 0);
}

void waitStartupRumbleInit(void) {
    gCurrentGameTask->callbackData0 -= 1;
    if (gCurrentGameTask->callbackData0 == 0) {
        requestRumbleMotorInit(0);
        requestRumbleMotorInit(1);
        requestRumbleMotorInit(2);
        requestRumbleMotorInit(3);
        setCurrentGameTaskCallback(&routeStartupControllerPakFlow, 0);
    }
}

void routeStartupControllerPakFlow(void) {
    if (gPlayerInputHeld[0] & START_BUTTON) {
        setCurrentGameTaskCallback(&openStartupControllerPakFileDeleteFlow, 0);
    } else {
        setCurrentGameTaskCallback(&openStartupReplaySaveMessageFlow, 0);
    }
}

void openStartupControllerPakFileDeleteFlow(void) {
    setCurrentGameTaskCallback(&openStartupReplaySaveMessageFlow, 0);
    createGameTask(4, &initControllerPakFileDeleteFlow, 0x64);
    suspendGameTask(0);
}

void openStartupReplaySaveMessageFlow(void) {
    setCurrentGameTaskCallback(&enterMainMenuAfterStartupControllerPakFlow, 0);
    createGameTask(4, &initControllerPakReplaySaveMessageFlow, 0x64);
    suspendGameTask(0);
}

void enterMainMenuAfterStartupControllerPakFlow(void) {
    createGameTask(3, &enterMainMenuFromRace, 0x64);
    removeGameTask(0);
}

void clearRaceReplayCourseGrid(void) {
    CourseGridEntry *entry = D_800DB7C0[gRaceCourseIndex.signedValue];
    s32 count = 0;
loop:
    if (entry->status != COURSE_GRID_ENTRY_END) {
        entry->status = COURSE_GRID_ENTRY_FREE;
        entry++;
        count += 1;
        goto loop;
    }
    if (count != 0) {
        gAssetHandles[0x2C] = allocRelocatableHeapBlock(count * sizeof(RacePlayer));
    }
}

void saveRaceReplayCourseGridEntry(void) {
    CourseGridEntry *entry = D_800DB7C0[gRaceCourseIndex.signedValue];
    s32 i = 0;

loop:
    if (entry->status != COURSE_GRID_ENTRY_END) {
        if ((entry->status == COURSE_GRID_ENTRY_FREE) && (entry->courseId == gRacePlayers[0].coursePathIndex)) {
            s32 *status = (s32 *)getRelocatableHeapBlockBase(gAssetHandles[0x2B]);
            if (*status < 0x1194) {
                entry->status = *status;
                ((RacePlayer *)getRelocatableHeapBlockBase(gAssetHandles[0x2C]))[i] = gRacePlayers[0];
            }
        }
        entry++;
        i++;
        goto loop;
    }
}

s32 loadNextRaceReplayCourseGridEntry(void) {
    s32 count;
    CourseGridEntry *entry;

    entry = D_800DB7C0[gRaceCourseIndex.signedValue];
    count = 0;
loop:
    if (entry->status != COURSE_GRID_ENTRY_END) {
        if (entry->status != COURSE_GRID_ENTRY_FREE) {
            gRacePlayers[0] = ((RacePlayer *)getRelocatableHeapBlockBase(gAssetHandles[0x2C]))[count];
            *(s32 *)getRelocatableHeapBlockBase(gAssetHandles[0x2B]) = entry->status;
            entry->status = COURSE_GRID_ENTRY_FREE;
            gRaceCameraReplayStartX = entry->unk4;
            gRaceCameraReplayStartY = entry->unk8;
            gRaceCameraReplayStartZ = entry->unkC;
            setRaceCameraModeForced(0, 7);
            return 1;
        }
        entry++;
        count += 1;
        goto loop;
    }
    return 0;
}

s32 hasPendingRaceReplayCourseGridEntry(void) {
    CourseGridEntry *entry = D_800DB7C0[gRaceCourseIndex.signedValue];

loop:
    if (entry->status != COURSE_GRID_ENTRY_END) {
        if (entry->status != COURSE_GRID_ENTRY_FREE) {
            return 1;
        }
        entry++;
        goto loop;
    }
    return 0;
}

void initNewGameSaveData(void) {
    s32 course;
    s32 one;
    s32 player;

    for (course = 0; course < 11; course++) {
        one = 1;
        for (player = 0; player < 5; player++) {
            gGameSaveDataBuffer[0].timeTrialRecords[course][player].minutes = 3;
            gGameSaveDataBuffer[0].timeTrialRecords[course][player].seconds = 0x3B;
            gGameSaveDataBuffer[0].timeTrialRecords[course][player].fraction = 0;
            gGameSaveDataBuffer[0].raceRecords[course][player].minutes = 3;
            gGameSaveDataBuffer[0].raceRecords[course][player].seconds = 0;
            gGameSaveDataBuffer[0].raceRecords[course][player].fraction = 0;
            gGameSaveDataBuffer[0].timeTrialCharacterIds[course][player] = player;
            gGameSaveDataBuffer[0].scoreAttackScores[course][player] = 0;
            gGameSaveDataBuffer[0].scoreAttackCharacterIds[course][player] = player;
            gGameSaveDataBuffer[0].raceRecordCharacterIds[course][player] = player;
            gGameSaveDataBuffer[0].trickAttackScores[course][player] = 0;
            gGameSaveDataBuffer[0].trickAttackCharacterIds[course][player] = player;
        }
        gGameSaveDataBuffer[0].bestLapRecords[course].minutes = one;
        gGameSaveDataBuffer[0].bestLapRecords[course].seconds = 0x18;
        gGameSaveDataBuffer[0].bestLapRecords[course].fraction = 0;
        gGameSaveDataBuffer[0].highScores[course + 1] = 0;
    }

    gGameSaveDataBuffer[0].replaySlots[0].length = 0;
    gGameSaveDataBuffer[0].replaySlots[1].length = 0;
    gGameSaveDataBuffer[0].replaySlots[2].length = 0;
    gGameSaveDataBuffer[0].replaySlots[3].length = 0;
    gGameSaveDataBuffer[0].replaySlots[4].length = 0;
    gGameSaveDataBuffer[0].replaySlots[5].length = (u8)0;
    gGameSaveDataBuffer[0].replaySlots[6].length = 0;
    gGameSaveDataBuffer[0].replaySlots[7].length = 0;
    gGameSaveDataBuffer[0].replaySlots[8].length = 0;
    gPlayerCount = 1;
    gRaceTypeSelection = 0;
    gRacePlayers[0].selectedCharacterId = 0;
    gRacePlayers[1].selectedCharacterId = 0;
    gRacePlayers[2].selectedCharacterId = 0;
    gRacePlayers[3].selectedCharacterId = 0;
    gRacePlayers[0].menuSelection = 0;
    gRacePlayers[1].menuSelection = 0;
    gRacePlayers[2].menuSelection = 0;
    gRacePlayers[3].menuSelection = 0;
    gRacePlayers[0].selectionUnlockState = 0;
    gRacePlayers[1].selectionUnlockState = 0;
    gRacePlayers[2].selectionUnlockState = 0;
    gRacePlayers[3].selectionUnlockState = 0;
    gRacePlayers[0].money = 0;
    gRacePlayers[1].money = 0;
    gRacePlayers[2].money = 0;
    gRacePlayers[3].money = 0;
    gRacePlayers[0].unk18 = 0;
    gRacePlayers[1].unk18 = 0;
    gRacePlayers[2].unk18 = 0;
    gRacePlayers[3].unk18 = 0;
    gRaceCourseIndex.signedValue = 0;
    gRaceSplitscreenMode = 0;
    gRaceCameraModeChangeDisabled = 0;
    setCurrentGameTaskCallback(openRaceSetupMenuFlow, 0);
}

void openRaceSetupMenuFlow(void) {
    setCurrentGameTaskCallback(&routeRaceCharacterSetupFlow, 0);
    createGameTask(4, &initRaceSetupMenu, 0x64);
    suspendGameTask(2);
}

void openRaceCharacterSelectMenuFlow(void) {
    setCurrentGameTaskCallback(&routeRaceCharacterSetupFlow, 0);
    createGameTask(4, &initCharacterSelectMenu, 0x64);
    suspendGameTask(2);
}

void routeRaceCharacterSetupFlow(void) {
    if (gPlayerCount >= 2) {
        setCurrentGameTaskCallback(&openPlayerSelectCourseMenuFlow, 0);
    } else {
        setCurrentGameTaskCallback(&openRaceSplitscreenSelectFlow, 0);
    }
}

void openPlayerSelectCourseMenuFlow(void) {
    setCurrentGameTaskCallback(&handlePlayerSelectCourseMenuFlow, 0);
    createGameTask(4, &initCharacterSelectCourseMenuFromPlayerSelect, 0x64);
    suspendGameTask(2);
}

void handlePlayerSelectCourseMenuFlow(void) {
    if (gMenuFlowState == 0) {
        setCurrentGameTaskCallback(&openMultiplayerCourseSelectFlow, 0);
    } else {
        gMenuFlowState = 0;
        setCurrentGameTaskCallback(&openRaceCharacterSelectMenuFlow, 0);
    }
}

void openRaceCourseMenuFlow(void) {
    if (gRaceSplitscreenMode == 1) {
        setCurrentGameTaskCallback(&handleRaceTypeSelectFlow, 0);
    } else {
        setCurrentGameTaskCallback(&handleRaceSplitscreenSelectFlow, 0);
    }
    createGameTask(4, &initCharacterSelectCourseMenuFromRace, 0x64);
    suspendGameTask(2);
}

void openRaceSplitscreenSelectFlow(void) {
    setCurrentGameTaskCallback(&handleRaceSplitscreenSelectFlow, 0);
    createGameTask(4, &initRaceSplitscreenSelectMenu, 0x64);
    suspendGameTask(2);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/handleRaceSplitscreenSelectFlow.s")

void openRaceTypeSelectFlow(void) {
    setCurrentGameTaskCallback(&handleRaceTypeSelectFlow, 0);
    createGameTask(4, &initRaceTypeSelectMenu, 0x64);
    suspendGameTask(2);
}

void handleRaceTypeSelectFlow(void) {
    if (gMenuFlowState == 1) {
        gMenuFlowState = 0;
        setCurrentGameTaskCallback(&openRaceSplitscreenSelectFlow, 0);
    } else {
        setCurrentGameTaskCallback(&openMultiplayerCourseSelectFlow, 0);
    }
}

void openSinglePlayerCourseSelectFlow(void) {
    setCurrentGameTaskCallback(&openRaceSplitscreenSelectFlow, 0);
    createGameTask(4, &initCourseSelectMenu, 0x64);
    suspendGameTask(2);
}

void openRaceRecordSaveOptionsFlow(void) {
    gRumblePakConnectedMask = 0;
    setCurrentGameTaskCallback(&exitRaceFlowToMainMenu, 0);
    createGameTask(4, &initControllerPakRaceRecordSaveFlow, 0x64);
    suspendGameTask(2);
}

void openMultiplayerCourseSelectFlow(void) {
    setCurrentGameTaskCallback(&handleMultiplayerCourseSelectFlow, 0);
    createGameTask(4, &initMultiplayerCourseSelectMenu, 0x64);
    suspendGameTask(2);
}

void handleMultiplayerCourseSelectFlow(void) {
    if (gMenuFlowState == 1) {
        gMenuFlowState = 0;
        if (gPlayerCount >= 2) {
            setCurrentGameTaskCallback(&openPlayerSelectCourseMenuFlow, 0);
        } else {
            setCurrentGameTaskCallback(&openRaceCourseMenuFlow, 0);
        }
    } else {
        setCurrentGameTaskCallback(&initRaceRecordSettingsFlow, 0);
    }
}

void exitRaceFlowToMainMenu(void) {
    requestRumbleMotorInit(0);
    requestRumbleMotorInit(1);
    requestRumbleMotorInit(2);
    requestRumbleMotorInit(3);
    requestMusicSequenceStop(0);
    createGameTask(3, &enterMainMenuFromRace, 0x64);
    removeGameTask(2);
}

void openPostRaceControllerPakContinuePromptFlow(void) {
    setCurrentGameTaskCallback(&handleControllerPakContinuePromptFlow, 0);
    createGameTask(4, &initControllerPakContinuePromptFlow, 0x64);
    suspendGameTask(2);
}

void handleControllerPakContinuePromptFlow(void) {
    if (gMenuFlowState == 1) {
        gMenuFlowState = 0;
        setCurrentGameTaskCallback(&exitRaceFlowToMainMenu, 0);
    } else {
        setCurrentGameTaskCallback(&openRaceCharacterSelectMenuFlow, 0);
    }
}

void openEndingCreditsIfUnlockedFlow(void) {
    setCurrentGameTaskCallback(&openRaceSplitscreenSelectFlow, 0);
    if (gPendingEndingCreditsFlow != 0) {
        createGameTask(4, &initEndingCreditsFlow, 0x64);
        suspendGameTask(2);
    }
}

void openPassAwardCelebrationFlow(void) {
    setCurrentGameTaskCallback(&openEndingCreditsIfUnlockedFlow, 0);
    createGameTask(4, &initPassAwardCelebration, 0x64);
    suspendGameTask(2);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/initRaceSceneFlow.s")

void fadeOutPassAwardCelebrationFlow(void) {
    gMenuFadeAlpha -= 8;
    if (gMenuFadeAlpha < 0) {
        gMenuFadeAlpha = 0;
        gCurrentGameTask->callbackData2--;
    }
    if (gCurrentGameTask->callbackData2 == 0) {
        setRaceCameraMode(0, 0x1E);
        requestMusicSequenceBank(5);
        setCurrentGameTaskCallback(&fadeInRaceGameplayViewports, 0);
    }
    updateRaceFlowFrame();
    gViewportStates[0].overlayAlpha = 0xFF;
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/fadeInRaceGameplayViewports.s")

void startRaceGameplayFlow(void) {
    if (gMenuFadeAlpha != 0) {
        gMenuFadeAlpha -= 0x10;
    }
    if (gMenuFadeAlpha < 0) {
        gMenuFadeAlpha = 0;
    }
    updateRaceFlowFrameWithCourseEffects();
    if (!(gMenuFlowState & 1)) {
        gMenuFadeOverlayActive = 0;
        requestCourseMusicSequence();
        setCurrentGameTaskCallback(updateRaceGameplayFlow, 0);
    }
}

void drawRacePauseMenu(void *arg0) {
    s32 color;

    drawAssetTableSprite(-0x14, -0x10, getRelocatableHeapBlockBase(gAssetHandles[0x1F]), 0x57);
    color = 0x1A;
    if (D_80120E77 == 0) {
        color = 0x1B;
    }
    drawAssetTableSpriteWithExplicitPalette(-0x1C, 0, getRelocatableHeapBlockBase(gAssetHandles[0x1F]), 0x58, color);
    color = 0x1A;
    if (D_80120E77 == 1) {
        color = 0x1B;
    }
    drawAssetTableSpriteWithExplicitPalette(-0x1C, 0xA, getRelocatableHeapBlockBase(gAssetHandles[0x1F]), 0x59, color);
    color = 0x1A;
    if (D_80120E77 == 2) {
        color = 0x1B;
    }
    drawAssetTableSpriteWithExplicitPalette(-0x1C, 0x14, getRelocatableHeapBlockBase(gAssetHandles[0x1F]), 0x5A, color);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/updateRaceGameplayFlow.s")

void waitRaceFinishResultsFlow(void) {
    D_80120E77 = 0;
    updateRaceFlowFrameWithCourseEffects();
    gCurrentGameTask->callbackData0--;
    if (gCurrentGameTask->callbackData0 == 0) {
        gMenuFlowState |= 8;
        switch (gPlayerCount) {
            case 3:
                gCurrentGameTask->callbackData1 = 0;
                if (gRacePlayers[1].rankIndex < gRacePlayers[gCurrentGameTask->callbackData1].rankIndex) {
                    gCurrentGameTask->callbackData1 = 1;
                }
                if (gRacePlayers[2].rankIndex < gRacePlayers[gCurrentGameTask->callbackData1].rankIndex) {
                    gCurrentGameTask->callbackData1 = 2;
                }
                gCurrentGameTask->callbackData1 += 2;
                setCurrentGameTaskCallback(zoomRaceWinnerViewport, 0);
                break;
            case 4:
                gCurrentGameTask->callbackData1 = 0;
                if (gRacePlayers[1].rankIndex < gRacePlayers[gCurrentGameTask->callbackData1].rankIndex) {
                    gCurrentGameTask->callbackData1 = 1;
                }
                if (gRacePlayers[2].rankIndex < gRacePlayers[gCurrentGameTask->callbackData1].rankIndex) {
                    gCurrentGameTask->callbackData1 = 2;
                }
                if (gRacePlayers[3].rankIndex < gRacePlayers[gCurrentGameTask->callbackData1].rankIndex) {
                    gCurrentGameTask->callbackData1 = 3;
                }
                gCurrentGameTask->callbackData1 += 5;
                setCurrentGameTaskCallback(zoomRaceWinnerViewport, 0);
                break;
            case 2:
                if (gRacePlayers[0].rankIndex < gRacePlayers[1].rankIndex) {
                    gCurrentGameTask->callbackData1 = 0;
                    gRacePlayerHudStatuses[1].active = 0;
                } else {
                    gCurrentGameTask->callbackData1 = 1;
                    gRacePlayerHudStatuses[0].active = 0;
                }
                gCurrentGameTask->callbackData0 = 0;
                setCurrentGameTaskCallback(zoomRaceWinnerViewport, 0);
                break;
            case 1:
                gCurrentGameTask->callbackData0 = 0;
                setCurrentGameTaskCallback(prepareRaceResultsFlow, 0);
                break;
        }
    }
}

void interpolateRaceViewport(
    s32 arg0,
    s32 arg1,
    s32 arg2,
    s32 arg3,
    s32 arg4,
    s32 arg5,
    s32 arg6,
    f32 arg7,
    s32 arg8,
    s32 arg9,
    s32 arg10,
    s32 arg11,
    s32 arg12,
    s32 arg13,
    f32 arg14
) {
    s32 sp2C;
    s32 sp28;
    f32 temp_fv0;
    s32 temp_t0;
    s32 temp_t1;
    s32 temp_t2;
    s32 temp_t3;

    sp2C = (((arg8 - arg1) * gCurrentGameTask->callbackData0) / 15) + arg1;
    sp28 = (((arg9 - arg2) * gCurrentGameTask->callbackData0) / 15) + arg2;
    temp_t0 = (((arg10 - arg3) * gCurrentGameTask->callbackData0) / 15) + arg3;
    temp_t1 = (((arg11 - arg4) * gCurrentGameTask->callbackData0) / 15) + arg4;
    temp_t2 = (((arg12 - arg5) * gCurrentGameTask->callbackData0) / 15) + arg5;
    temp_t3 = (((arg13 - arg6) * gCurrentGameTask->callbackData0) / 15) + arg6;
    temp_fv0 = (((arg14 - arg7) * (f32)gCurrentGameTask->callbackData0) / 15.0f) + arg7;
    if (gRaceCourseIndex.signedValue != 6) {
        configureViewport(arg0, sp2C, sp28, temp_t0, temp_t1, temp_t2, temp_t3, temp_fv0);
    } else {
        configureRaceViewport(arg0, sp2C, sp28, temp_t0, temp_t1, temp_t2, temp_t3, temp_fv0);
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/zoomRaceWinnerViewport.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/prepareRaceResultsFlow.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_80075F88.s")

void updateRaceResultsMusicFlow(void) {
    if (countActiveMusicSequences() == 0) {
        requestMusicSequenceBank(7);
        gCurrentGameTask->callbackData1 = 0x3C;
        gMenuFlowState |= 0x20;
    }
    if (D_80120E80 != 0) {
        createCallbackTaskWithUserId((CallbackTaskCallback)&initFallingConfetti, 5, 0x64, D_80120E80 - 1);
    }
    updateRaceFlowFrame();
    if (gMenuFlowState & 0x10) {
        gCurrentGameTask->callbackData0 -= 1;
        if (gCurrentGameTask->callbackData0 == 0) {
            gCurrentGameTask->callbackData0 = 4;
            setCurrentGameTaskCallback(&fadeOutRaceResultsFlow, 0);
            requestMusicSequenceStop(0x14);
        }
    }
}

void fadeOutRaceResultsFlow(void) {
    gFramebufferRenderInterval.value = 0;
    if (D_80120E80 != 0) {
        createCallbackTaskWithUserId((CallbackTaskCallback)initFallingConfetti, 5, 0x64, D_80120E80 - 1);
    }
    updateRaceFlowFrame();
    gRaceRumbleEnabled = 0;
    gMenuFadeAlpha += 0x10;
    if (gMenuFadeAlpha >= 0xFF) {
        gMenuFadeAlpha = 0xFF;
        gCurrentGameTask->callbackData0 -= 1;
        if (gCurrentGameTask->callbackData0 == 0) {
            stopSoundEffects();
            if ((gRaceSplitscreenMode == 2) &&
                (((Unk80043040 *)getRelocatableHeapBlockBase(gAssetHandles[0x2B]))->unk8 != 0) && (D_80120E81 != 0) &&
                (saveRaceRecordReplayData() != 0)) {
                D_80120E81 = -1;
            }
            if (gRaceSplitscreenMode == 2) {
                setCurrentGameTaskCallback(&initRaceGhostReplayFlow, 0);
            } else {
                gFramebufferSwapHold = 1;
                setCurrentGameTaskCallback(finalizeRaceExitFlow, 0);
            }
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/initRaceGhostReplayFlow.s")

void updateRaceGhostReplayFlow(void) {
    void *sp18;

    if (gCurrentGameTask->callbackData0 == D_800DB8F0[gCurrentGameTask->callbackData1]) {
        if (loadNextRaceReplayCourseGridEntry() != 0) {
            if (gCurrentGameTask->callbackData1 != 0xB) {
                gCurrentGameTask->callbackData1++;
                gRacePlayers[0].replayInputSource = RACE_PLAYER_INPUT_HISTORY_2B;
                gRacePlayers[0].unk15 = 0;
                gViewportStates[0].overlayAlpha = 0;
            } else {
                requestMusicSequenceStop(0x48);
                setCurrentGameTaskCallback(fadeOutRaceGhostReplayFlow, 0);
            }
        } else {
            requestMusicSequenceStop(0x48);
            setCurrentGameTaskCallback(fadeOutRaceGhostReplayFlow, 0);
        }
    }
    if (gCurrentGameTask->callbackData0 == 1) {
        gMenuFadeAlpha = 0;
    }
    if (gRaceUpdatePaused == 0) {
        gCurrentGameTask->callbackData0++;
    }
    updateRaceReplayFrame();
    if (gPlayerInputPressed[0] & START_BUTTON) {
        sp18 = fadeOutRaceGhostReplayFlow;
        requestMusicSequenceStop(0x48);
        setCurrentGameTaskCallback(sp18, 0);
    }
}

void fadeOutRaceGhostReplayFlow(void) {
    updateRaceReplayFrame();
    gRaceRumbleEnabled = 0;
    gMenuFadeAlpha += 7;
    if (gMenuFadeAlpha >= 0xFF) {
        gMenuFadeAlpha = 0xFF;
        setCurrentGameTaskCallback(&finalizeRaceExitFlow, 0);
    }
}

void finalizeRaceExitFlow(void) {
    gFramebufferSwapHold = 1;
    if (gPendingFramebufferSwapCount == 2) {
        gRaceRumbleEnabled = 0;
        gRaceCameraModeChangeDisabled = 0;
        updateRelocatableHeap();
        releaseMenuAssetHandles();
        gFramebufferSwapHold = 0;
        gFramebufferRenderInterval.value = 0;
        stopSoundEffects();
        requestRumbleMotorInit(0);
        requestRumbleMotorInit(1);
        requestRumbleMotorInit(2);
        requestRumbleMotorInit(3);
        gMenuFlowState = 0;
        if (D_80120E77 == 2) {
            setCurrentGameTaskCallback(initRaceSceneFlow, 0);
        } else if (gPlayerCount == 1) {
            if (gRaceSplitscreenMode == 1) {
                setCurrentGameTaskCallback(openRaceTypeSelectFlow, 0);
            } else {
                setCurrentGameTaskCallback(openPassAwardCelebrationFlow, 0);
            }
        } else {
            setCurrentGameTaskCallback(openPostRaceControllerPakContinuePromptFlow, 0);
        }
        requestMusicSequenceStop(0);
    }
}

void updateRaceFlowFrameWithCourseEffects(void) {
    updateRacePlayers();
    updateCallbackTasksWithMinPriority(0x63);
    updateRacePlayersPostUpdate();
    updateRemainingCallbackTasks();
    updateRaceCameras();
    updateRaceHud();
}

void updateRaceFlowFrame(void) {
    updateRacePlayers();
    updateCallbackTasksWithMinPriority(0x63);
    updateRacePlayersPostUpdate();
    updateRemainingCallbackTasks();
    updateRaceCameras();
}

void updateRaceReplayFrame(void) {
    updateRacePlayers();
    updateCallbackTasksWithMinPriority(0x63);
    updateRacePlayersPostUpdate();
    updateRemainingCallbackTasks();
    updateRaceCameras();
}

s32 areRacePlayersFinished(void) {
    s32 var_v0;
    RacePlayer *player;

    if (gRaceSplitscreenMode == 1) {
        if (!(gRacePlayers[0].stateFlags & 0x40)) {
            return 0;
        }
    } else {
        var_v0 = 0;
        if ((s32)gPlayerCount > 0) {
            player = gRacePlayers;
        loop:
            if (!(player->stateFlags & RACE_PLAYER_READY_FLAG)) {
                return 0;
            }
            var_v0++;
            player++;
            if (var_v0 < (s32)gPlayerCount) {
                goto loop;
            }
        }
    }
    gMenuFlowState |= 2;
    return 1;
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/initRaceRecordSettingsFlow.s")

void updateRaceRecordSettingsFlow(void) {
    s32 temp_v0;

    gMenuFadeAlpha -= 0x10;
    if (gMenuFadeAlpha < 0) {
        gMenuFadeAlpha = 0;
        temp_v0 = gPlayerInputPressed[0];
        if ((temp_v0 & (STICK_DOWN | D_JPAD)) && (gRaceLapCount != 9)) {
            enqueueSoundEffect(0x19, 0x32);
            gRaceLapCount += 1;
            temp_v0 = gPlayerInputPressed[0];
        }
        if ((temp_v0 & (STICK_UP | U_JPAD)) && (gRaceLapCount != 1)) {
            enqueueSoundEffect(0x19, 0x32);
            gRaceLapCount -= 1;
            temp_v0 = gPlayerInputPressed[0];
        }
        if (temp_v0 & (A_BUTTON | START_BUTTON)) {
            gMenuFlowState = 1;
            enqueueSoundEffect(0x18, 0x32);
            requestMusicSequenceStop(0x3C);
            setCurrentGameTaskCallback(fadeOutRaceRecordSettingsFlow, 0);
        }
    }
    updateCallbackTasks();
}

void fadeOutRaceRecordSettingsFlow(void) {
    gMenuFadeAlpha += 0x10;
    if (gMenuFadeAlpha >= 0x100) {
        gMenuFadeAlpha = 0xFF;
        gFramebufferSwapHold = 1;
        setCurrentGameTaskCallback(&closeRaceRecordSettingsFlow, 0);
    }
    updateCallbackTasks();
}

void closeRaceRecordSettingsFlow(void) {
    if (gPendingFramebufferSwapCount == 2) {
        releaseMenuAssetHandles();
        gFramebufferSwapHold = 0;
        gFramebufferRenderInterval.value = 0;
        setCurrentGameTaskCallback(&initRaceSceneFlow, 0);
    }
}
