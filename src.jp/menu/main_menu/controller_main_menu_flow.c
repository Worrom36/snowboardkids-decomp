#include "common.h"
#include "game/save_data.h"
#include "assets.h"
#include "game/audio/sound_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/engine/system_runtime.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/controller_pak/controller_pak_menu.h"
#include "game/menu/renderer/menu_screen_effects.h"
#include "game/menu/main_menu/main_menu_panel_ui.h"
#include "game/menu/main_menu/main_menu_scene_model.h"
#include "game/menu/main_menu/main_menu_scene_model_renderer.h"
#include "game/menu/main_menu/main_menu_race_flow.h"
#include "game/menu/main_menu/training_course_race_flow.h"
#include "game/menu/main_menu/main_menu_title_ui.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/camera/race_camera.h"
#include "game/race/flow/race_flow.h"
#include "game/race/race_state.h"
#include "game/demo/main_menu_demo_race_intro.h"
#include "game/demo/title_demo_race_intro.h"
#include "game/engine/viewport_manager.h"
#include "game/menu/renderer/menu_renderer.h"
#include <PR/os_motor.h>
#include <PR/os_cont.h>

#define OS_MESG_BLOCK 1
#define CONTROLLER_PAK_RECORD_AT(cursor, field) \
    (*(GameSaveRecordTime *)((cursor) + CONTROLLER_PAK_SAVE_FIELD_OFFSET(field)))
#define CONTROLLER_PAK_U8_AT(cursor, field) (*(u8 *)((cursor) + CONTROLLER_PAK_SAVE_FIELD_OFFSET(field)))
#define CONTROLLER_PAK_U16_AT(cursor, field) (*(u16 *)((cursor) + CONTROLLER_PAK_SAVE_FIELD_OFFSET(field)))
#define CONTROLLER_PAK_SAVE_FIELD_OFFSET(field) ((u32) & (((GameSaveData *)0)->field))

extern u8 gRumblePakConnectedMask;
extern u8 gControllerReadPending;
extern OSMesgQueue gControllerSubsystemRequestQueue;
extern u8 gConnectedControllerBitmask;
extern OSContPad gControllerPads[];
extern OSMesgQueue gControllerSubsystemReplyQueue;
extern OSMesgQueue gControllerEventQueue;
extern OSPfs gControllerPakHandles[];
extern u8 gMainMenuReturnFromRace;
extern s16 gMenuFadeAlpha;
extern s32 gMenuFlowState;

void requestControllerRead(void) {
    if ((gControllerReadPending == 0) && (gConnectedControllerBitmask != 0)) {
        osSendMesg(&gControllerSubsystemRequestQueue, (OSMesg)CONTROLLER_REQUEST_READ_INPUT, OS_MESG_BLOCK);
        gControllerReadPending = 1;
    }
}

void updateControllerInputState(void) {
    u16 i;
    // clang-format off
    do { \
        i = 0; \
        do { \
            if ((((s32)gConnectedControllerBitmask) >> i) & 1) { \
                if (gControllerPads[i].errno == 0) { \
                    gControllerInputState[i] = *((ControllerInputState *)(&gControllerPads[i])); \
                } \
            } \
            i++; \
        } while (i < 4); \
        gControllerReadPending = 0; \
    } while (0);
    // clang-format on
}

void requestRumbleMotorInit(u16 controllerIndex) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(
        &gControllerSubsystemRequestQueue,
        (OSMesg)(controllerIndex + CONTROLLER_REQUEST_INIT_RUMBLE),
        OS_MESG_BLOCK
    );
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/func_80000A8C.s")

void requestRumbleMotorStart(u16 controllerIndex) {
    if (gRaceRumbleEnabled != 0) {
        if (gRumblePakConnectedMask & (1 << controllerIndex)) {
            gRumbleMotorRequestStates[controllerIndex] = 1;
        }
    }
}

void requestControllerPakProbe(u16 controllerIndex) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(
        &gControllerSubsystemRequestQueue,
        (OSMesg)(controllerIndex + CONTROLLER_REQUEST_PROBE_PAK),
        OS_MESG_BLOCK
    );
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/probeControllerPak.s")

void requestControllerPakSaveStatus(u16 controllerIndex) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(
        &gControllerSubsystemRequestQueue,
        (OSMesg)(controllerIndex + CONTROLLER_REQUEST_CHECK_SAVE),
        OS_MESG_BLOCK
    );
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/checkControllerPakSaveStatus.s")

void requestControllerPakSaveRead(u16 controllerIndex) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(
        &gControllerSubsystemRequestQueue,
        (OSMesg)(controllerIndex + CONTROLLER_REQUEST_READ_SAVE),
        OS_MESG_BLOCK
    );
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/readControllerPakSave.s")

void requestControllerPakSaveWrite(u16 controllerIndex) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(
        &gControllerSubsystemRequestQueue,
        (OSMesg)(controllerIndex + CONTROLLER_REQUEST_WRITE_SAVE),
        OS_MESG_BLOCK
    );
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/writeControllerPakSave.s")

void requestControllerPakRepair(u16 controllerIndex) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(
        &gControllerSubsystemRequestQueue,
        (OSMesg)(controllerIndex + CONTROLLER_REQUEST_REPAIR_PAK),
        OS_MESG_BLOCK
    );
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

void repairControllerPakId(u16 controllerIndex) {
    OSPfs **sp18;
    OSPfs *pfs;
    s32 ret;

    pfs = &gControllerPakHandles[controllerIndex];
    sp18 = &pfs;
    osPfsInitPak(&gControllerEventQueue, *sp18, controllerIndex);
    ret = osPfsRepairId(pfs);
    if ((ret == 4) || (ret == 0xA)) {
        gControllerPakRetryCounts[controllerIndex] += 1;
    }
}

void requestControllerPakFileList(void) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(&gControllerSubsystemRequestQueue, (OSMesg)CONTROLLER_REQUEST_LIST_FILES, OS_MESG_BLOCK);
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

void readControllerPakFileStates(void) {
    s32 i;

    osPfsInitPak(&gControllerEventQueue, &gControllerPakHandles[0], 0);
    for (i = 0; i != 0x10; i++) {
        osPfsFileState(&gControllerPakHandles[0], i, &gControllerPakFileStates[i]);
    }
}

void requestControllerPakDeleteFile(u16 arg0) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(&gControllerSubsystemRequestQueue, (OSMesg)(arg0 + CONTROLLER_REQUEST_DELETE_FILE), OS_MESG_BLOCK);
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

void deleteControllerPakFile(u16 arg0) {
    OSPfs *pfs;
    OSPfsState *state;
    u16 companyCode;
    u32 gameCode;
    u8 gameName[16];
    u8 extName[4];
    s32 i;

    pfs = gControllerPakHandles;
    osPfsInitPak(&gControllerEventQueue, pfs, 0);

    state = &gControllerPakFileStates[arg0];
    companyCode = state->company_code;
    gameCode = state->game_code;

    for (i = 0; i < 16; i++) {
        gameName[i] = gControllerPakFileStates[arg0].game_name[i];
    }

    for (i = 0; i < 4; i++) {
        extName[i] = gControllerPakFileStates[arg0].ext_name[i];
    }

    for (i = 0; i != 3; i++) {
        if (osPfsDeleteFile(pfs, companyCode, gameCode, gameName, extName) == 0) {
            gControllerPakRetryCounts[0] = 0;
            return;
        }
        gControllerPakRetryCounts[0]++;
    }
}

void requestControllerPakFreeSpaceUpdate(void) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(&gControllerSubsystemRequestQueue, (OSMesg)CONTROLLER_REQUEST_UPDATE_FREE_SPACE, OS_MESG_BLOCK);
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

void updateControllerPakFreeSpaceInfo(void) {
    s32 pad;
    s32 maxFiles;
    s32 filesUsed;

    osPfsInitPak(&gControllerEventQueue, &gControllerPakHandles[0], 0);
    osPfsFreeBlocks(&gControllerPakHandles[0], &gControllerPakFreeBytes);
    osPfsNumFiles(&gControllerPakHandles[0], &maxFiles, &filesUsed);
    gControllerPakFreeFileCount = maxFiles - filesUsed;
}

u16 validateControllerPakSave(volatile s32 channel) {
    u8 *nextByte;
    s32 wordCursorValid;
    u8 *save;
    u32 *wordCursor;
    s32 new_var;
    u8 *byteCursor;
    s32 offset;
    s32 i;
    s32 firstByteStep;
    s32 secondByteStep;
    s32 count;

    save = (u8 *)&gGameSaveDataBuffer[channel << 1 << 1 >> 2];
    wordCursor = (u32 *)save;
    byteCursor = save;
    offset = 0;
    for (;;) {
        offset += 4;
        firstByteStep = (byteCursor + 1) - byteCursor;
        wordCursor++;
        byteCursor++;
        if (1) {}
        if (1) {}
        if (1) {}
        if (1) {}
        secondByteStep = firstByteStep && firstByteStep;
        if (secondByteStep) {}
        if (!(offset < 0x2C)) {
            break;
        }
    }

    i = 0;
    wordCursorValid = wordCursor && wordCursor;
    byteCursor = save;
    offset += firstByteStep - 1;
    count = 0xC;
    for (;;) {
        i++;
        new_var = i;
        wordCursorValid = new_var && new_var;
        nextByte = byteCursor + 1;
        secondByteStep = nextByte - byteCursor;
        firstByteStep = wordCursorValid;
        if (secondByteStep) {}
        if ((firstByteStep ^ 0) && wordCursor) {}
        if (save && save) {}
        byteCursor++;
        if (!(new_var != count)) {
            break;
        }
    }
    i += secondByteStep - 1;

    return validateControllerPakSaveData(channel);
}

s32 validateControllerPakSaveData(s32 channel) {
    s32 pad;
    volatile s32 unusedInvalidCourse;
    u8 *scoreAndCharacterCursor;
    u8 *trickScoreCursor;
    u8 *saveStart;
    u8 *timeTrialCourseCursor;
    u8 *scoreCourseCursor;
    u8 *raceCourseCursor;
    u8 *recordCursor;
    s16 fraction;
    s16 raceFraction;
    s16 targetFraction;
    s32 playerCounter;
    s32 courseOffset;
    s32 invalidSave;
    s8 minutes;
    s8 seconds;
    s8 raceMinutes;
    s8 raceSeconds;
    s8 targetMinutes;
    s8 targetSeconds;
    s32 trickScore;
    s32 trickCharacter;
    s32 timeTrialCharacter;
    s32 raceCharacter;
    s32 scoreCharacter;

    saveStart = (u8 *)&gGameSaveDataBuffer[channel];
    invalidSave = 0;
    timeTrialCourseCursor = saveStart;
    scoreCourseCursor = saveStart;
    raceCourseCursor = saveStart;
    courseOffset = 0;
    saveStart += 0;

    do {
        unusedInvalidCourse = 0;
        recordCursor = timeTrialCourseCursor;
        scoreAndCharacterCursor = scoreCourseCursor;
        playerCounter = 0;
        trickScoreCursor = raceCourseCursor;
    loop_player:
        minutes = CONTROLLER_PAK_RECORD_AT(recordCursor, timeTrialRecords).minutes;
        playerCounter += 2;
        if ((minutes < 0) || (minutes >= 100)) {
            invalidSave = 1;
        }
        seconds = CONTROLLER_PAK_RECORD_AT(recordCursor, timeTrialRecords).seconds;
        if ((seconds < 0) || (seconds >= 60)) {
            invalidSave = 1;
        }
        fraction = CONTROLLER_PAK_RECORD_AT(recordCursor, timeTrialRecords).fraction;
        if ((fraction < 0) || (fraction >= 0x6301)) {
            invalidSave = 1;
        }
        timeTrialCharacter = CONTROLLER_PAK_U8_AT(scoreAndCharacterCursor, timeTrialCharacterIds);
        if ((timeTrialCharacter & 7) >= 6) {
            invalidSave = 1;
        }
        if ((((s32)timeTrialCharacter >> 3) & 0xF) >= 0xF) {
            invalidSave = 1;
        }
        raceMinutes = CONTROLLER_PAK_RECORD_AT(recordCursor, raceRecords).minutes;
        if ((raceMinutes < 0) || (raceMinutes >= 100)) {
            invalidSave = 1;
        }
        raceSeconds = CONTROLLER_PAK_RECORD_AT(recordCursor, raceRecords).seconds;
        if ((raceSeconds < 0) || (raceSeconds >= 60)) {
            invalidSave = 1;
        }
        raceFraction = CONTROLLER_PAK_RECORD_AT(recordCursor, raceRecords).fraction;
        recordCursor += sizeof(GameSaveRecordTime);
        if ((raceFraction < 0) || (raceFraction >= 0x6301)) {
            invalidSave = 1;
        }
        raceCharacter = CONTROLLER_PAK_U8_AT(scoreAndCharacterCursor, raceRecordCharacterIds);
        if ((raceCharacter & 7) >= 6) {
            invalidSave = 1;
        }
        if ((((s32)raceCharacter >> 3) & 0xF) >= 0xF) {
            invalidSave = 1;
        }
        if (courseOffset == 0x24) {
            if (CONTROLLER_PAK_U8_AT(scoreAndCharacterCursor, scoreAttackScores) >= 0x1F) {
                goto block_32;
            }
        } else if (CONTROLLER_PAK_U8_AT(scoreAndCharacterCursor, scoreAttackScores) >= 0x3D) {
        block_32:
            invalidSave = 1;
        }
        scoreCharacter = CONTROLLER_PAK_U8_AT(scoreAndCharacterCursor, scoreAttackCharacterIds);
        if ((scoreCharacter & 7) >= 6) {
            invalidSave = 1;
        }
        if ((((s32)scoreCharacter >> 3) & 0xF) >= 0xF) {
            invalidSave = 1;
        }
        trickScore = (CONTROLLER_PAK_U16_AT(trickScoreCursor, trickAttackScores)) & 0xFFFFu;
        trickScoreCursor += sizeof(u16);
        if ((s32)trickScore >= 0x2710) {
            invalidSave = 1;
        }
        trickCharacter = CONTROLLER_PAK_U8_AT(scoreAndCharacterCursor, trickAttackCharacterIds);
        if ((trickCharacter & 7) >= 6) {
            invalidSave = 1;
        }
        if ((((s32)trickCharacter >> 3) & 0xF) >= 0xF) {
            invalidSave = 1;
        }
        scoreAndCharacterCursor += 1;
        if (playerCounter != 0xA) {
            goto loop_player;
        }
        targetMinutes = CONTROLLER_PAK_RECORD_AT(saveStart, bestLapRecords).minutes;
        timeTrialCourseCursor += sizeof(GameSaveRecordTime) * 5;
        scoreCourseCursor += 5;
        if ((targetMinutes < 0) || (targetMinutes >= 100)) {
            invalidSave = 1;
        }
        targetSeconds = CONTROLLER_PAK_RECORD_AT(saveStart, bestLapRecords).seconds;
        raceCourseCursor += sizeof(u16) * 5;
        courseOffset += 4;
        if ((targetSeconds < 0) || (targetSeconds >= 60)) {
            invalidSave = 1;
        }
        targetFraction = CONTROLLER_PAK_RECORD_AT(saveStart, bestLapRecords).fraction;
        if ((targetFraction < 0) || (targetFraction >= 0x6301)) {
            invalidSave = 1;
        }
        saveStart += sizeof(GameSaveRecordTime);
    } while (courseOffset != 0x2C);

    return invalidSave;
}

void enterMainMenuFromRace(void) {
    gMainMenuReturnFromRace = 1;
    setCurrentGameTaskCallback(initMainMenu, 0);
    createGameTask(4, initTitleDemoRaceIntro, 0x64);
    suspendGameTask(3);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/initMainMenu.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/func_80001FF8.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/func_8000228C.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/func_8000247C.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/func_80002600.s")

void fadeOutMainMenuModeSelect(void) {
    gMenuFadeAlpha += 0x10;
    if (gMenuFadeAlpha >= 0x100) {
        gMenuFadeAlpha = 0xFF;
        gFramebufferSwapHold = 1;
        setCurrentGameTaskCallback(exitMainMenuModeSelect, 0);
    }
    updateRaceCamera(0);
    loopMainMenuSceneModelAnimation(4);
    addMainMenuSceneModelDrawCallback(4);
    updateCallbackTasks();
}

void exitMainMenuModeSelect(void) {
    if (gPendingFramebufferSwapCount == 2) {
        releaseMenuAssetHandles();
        gFramebufferSwapHold = 0;
        gFramebufferRenderInterval.value = 0;
        setCurrentGameTaskCallback(initMainMenu, 0);
        if (gMenuFlowState == 0) {
            createGameTask(4, startTrainingCourseFlow, 0x64);
            suspendGameTask(3);
            return;
        }
        if (gMenuFlowState == 1) {
            createGameTask(4, startMainMenuModePreviewRaceFlow, 0x64);
            suspendGameTask(3);
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/func_80002888.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/func_800029F0.s")

void fadeOutMainMenuSettings(void) {
    gMenuFadeAlpha += 0x10;
    if (gMenuFadeAlpha >= 0x100) {
        gMenuFadeAlpha = 0xFF;
        gFramebufferSwapHold = 1;
        setCurrentGameTaskCallback(exitMainMenuSettings, 0);
    }
    updateRaceCamera(0);
    updateCallbackTasks();
}

void exitMainMenuSettings(void) {
    if (gPendingFramebufferSwapCount == 2) {
        releaseMenuAssetHandles();
        gFramebufferSwapHold = 0;
        gFramebufferRenderInterval.value = 0;
        setCurrentGameTaskCallback(initMainMenu, 0);
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/controller_main_menu_flow/func_80002D74.s")
