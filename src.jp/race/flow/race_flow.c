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

typedef struct {
    s32 unk0;
    u8 pad4[4];
    s8 unk8;
} Unk80043040;

extern s8 D_80120E81;
extern u8 gPendingEndingCreditsFlow;
extern s32 gMenuFlowState;
extern u8 gRumblePakConnectedMask;
extern s16 gMenuFadeAlpha;
extern u8 D_80120E80;
extern u8 D_80120E77;

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_80072780.s")

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_80072960.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_800729E8.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_80072B14.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_80072C3C.s")

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_8007435C.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/updateRaceGameplayFlow.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_80074754.s")

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_80074CBC.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_80075B4C.s")

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_80077474.s")

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_8007780C.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/initRaceRecordSettingsFlow.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/flow/race_flow/func_80077B70.s")

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
