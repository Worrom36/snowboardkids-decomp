#include "common.h"
#include "game/menu/menu_scratch.h"
#include "game/race/race_state.h"
#include "font_encoding.h"
#include "game/save_data.h"
#include "assets.h"
#include "game/engine/asset_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/audio/sound_manager.h"
#include "game/menu/race_setup/race_setup_menu.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/engine/viewport_manager.h"
#include "game/race/player/race_player_input.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/controller_pak/controller_pak_ui.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/course_select/multiplayer_course_select_menu.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"

typedef struct RaceSetupSaveDefaultTime {
    s8 minutes;
    s8 seconds;
    s8 fraction;
} RaceSetupSaveDefaultTime;

typedef union RaceSetupSaveDefaultTimeRow {
    u8 bytes[sizeof(RaceSetupSaveDefaultTime) * 5];
    RaceSetupSaveDefaultTime records[5];
} RaceSetupSaveDefaultTimeRow;

typedef union RaceSetupSaveDefaultTimeTable {
    s8 bytes[sizeof(RaceSetupSaveDefaultTimeRow) * 2];
    RaceSetupSaveDefaultTimeRow rows[2];
} RaceSetupSaveDefaultTimeTable;

typedef struct {
    u16 trickAttackScores[5];
    u16 padding;
    MenuGlyphScript courseNames[8][18];
    u16 tailPadding[2];
} RaceSetupDefaultSaveData;

extern s32 gMenuFlowState;
extern u8 gConnectedControllerCount;
extern s16 gRaceSetupSavePanelInitialRects[4][2];
extern RaceSetupSaveDefaultTime D_800B31C8[11][5];
extern RaceSetupDefaultSaveData D_800B32D0;
extern RaceSetupSaveDefaultTime D_800B3270[11];
extern RaceSetupSaveDefaultTimeTable D_800B32A4;
extern u8 D_800B32C4[10];
extern RaceSetupSaveDefaultTimeRow D_800B3294[1];

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_setup/race_setup_menu/initRaceSetupMenu.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_setup/race_setup_menu/func_8000334C.s")

void initRaceSetupSaveMenu(void) {
    s32 i;
    s32 connectedControllerCount;
    RacePlayer *player;

    for (i = 0; i < 4; i++) {
        gControllerPakStatusCodes[i] = 0;
        gMenuChoicePromptState[i] = 0;
        gControllerPakRetryCounts[i] = 0;
        gControllerPakOperationCounts[i] = 0;
    }

    gRaceSetupSavePanelCreateTimer = 0;
    // clang-format off
    do { \
        i = 0; \
        connectedControllerCount = gConnectedControllerCount; \
        if (connectedControllerCount > 0) { \
            player = gRacePlayers; \
            do { \
                player++; \
                player[-1].menuState = 0; \
                i++; \
            } while (player < &gRacePlayers[connectedControllerCount]); \
            i = 0; \
        } \
        do { \
            initRaceSetupPlayerSaveData(i); \
            i++; \
        } while (i < 4); \
        gMenuScratch0.task = 0; \
        gMenuScratch1.task = 0; \
    } while (0);
    // clang-format on
    gMenuScratch2.task = 0;
    gMenuSelectionConfirmTimer = 0;
    gMenuFlowState = 0;
    gRaceRumbleEnabled = 0;

    for (i = 0; i < 4; i++) {
        gRaceSetupSavePanelRects[0][i] = gRaceSetupSavePanelInitialRects[i][0];
        gRaceSetupSavePanelRects[1][i] = gRaceSetupSavePanelInitialRects[i][1];
    }

    setCurrentGameTaskCallback(updateRaceSetupSaveMenu, 0);
    updateCallbackTasks();
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_setup/race_setup_menu/updateRaceSetupSaveMenu.s")

void raceSetupMenuNoop(void) {
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_setup/race_setup_menu/func_80004134.s")

void initRaceSetupPlayerSaveData(s32 playerIndex) {
    s32 course;
    s32 record;

    gGameSaveDataBuffer[playerIndex].checksum = 0;
    gGameSaveDataBuffer[playerIndex].highScores[0] = 0;
    for (record = 0; record < 11; record++) {
        gGameSaveDataBuffer[playerIndex].highScores[record + 1] = 0;
        gGameSaveDataBuffer[playerIndex].cupPlacements[record] = 0;
    }
    for (record = 0; record < 3; record++) {
        gGameSaveDataBuffer[playerIndex].courseUnlockStates[record] = record;
    }
    for (record = 3; record < 12; record++) {
        gGameSaveDataBuffer[playerIndex].courseUnlockStates[record] = -1;
    }
    gGameSaveDataBuffer[playerIndex].characterFlags = 0;
    gGameSaveDataBuffer[playerIndex].progressionLevel = 0;
    gGameSaveDataBuffer[playerIndex].extraCourseUnlockFlags = 0;

    for (course = 0; course < 11; course++) {
        for (record = 0; record < 5; record++) {
            gGameSaveDataBuffer[playerIndex].timeTrialRecords[course][record].minutes =
                D_800B31C8[course][record].minutes;
            gGameSaveDataBuffer[playerIndex].timeTrialRecords[course][record].seconds =
                D_800B31C8[course][record].seconds;
            gGameSaveDataBuffer[playerIndex].timeTrialRecords[course][record].fraction =
                D_800B31C8[course][record].fraction << 8;
            gGameSaveDataBuffer[playerIndex].timeTrialCharacterIds[course][record] = 0x10;
            gGameSaveDataBuffer[playerIndex].timeTrialCharacterIds[course][record] += record;

            if (course == 9) {
                gGameSaveDataBuffer[playerIndex].raceRecords[course][record].minutes =
                    D_800B3294[0].records[record].minutes;
                gGameSaveDataBuffer[playerIndex].raceRecords[course][record].seconds =
                    D_800B3294[0].records[record].seconds;
                gGameSaveDataBuffer[playerIndex].raceRecords[course][record].fraction =
                    D_800B3294[0].records[record].fraction << 8;
            } else {
                gGameSaveDataBuffer[playerIndex].raceRecords[course][record].minutes =
                    (&D_800B32A4.bytes[record * 3])[(course & 1) * sizeof(RaceSetupSaveDefaultTimeRow)];
                gGameSaveDataBuffer[playerIndex].raceRecords[course][record].seconds =
                    (&D_800B32A4.bytes[record * 3])[(course & 1) * sizeof(RaceSetupSaveDefaultTimeRow) + 1];
                gGameSaveDataBuffer[playerIndex].raceRecords[course][record].fraction =
                    (&D_800B32A4.bytes[record * 3])[(course & 1) * sizeof(RaceSetupSaveDefaultTimeRow) + 2] << 8;
            }
            gGameSaveDataBuffer[playerIndex].raceRecordCharacterIds[course][record] = 0x10;
            gGameSaveDataBuffer[playerIndex].raceRecordCharacterIds[course][record] += record;
            if (course == 9) {
                gGameSaveDataBuffer[playerIndex].scoreAttackScores[course][record] = D_800B32C4[record];
            } else {
                gGameSaveDataBuffer[playerIndex].scoreAttackScores[course][record] = D_800B32C4[record + 5];
            }
            gGameSaveDataBuffer[playerIndex].scoreAttackCharacterIds[course][record] = record;
            gGameSaveDataBuffer[playerIndex].trickAttackScores[course][record] = D_800B32D0.trickAttackScores[record];
            gGameSaveDataBuffer[playerIndex].trickAttackCharacterIds[course][record] = record;
        }
        gGameSaveDataBuffer[playerIndex].bestLapRecords[course].minutes = D_800B3270[course].minutes;
        gGameSaveDataBuffer[playerIndex].bestLapRecords[course].seconds = D_800B3270[course].seconds;
        gGameSaveDataBuffer[playerIndex].bestLapRecords[course].fraction = D_800B3270[course].fraction;
    }

    gGameSaveDataBuffer[playerIndex].replaySlots[0].length = 0;
    gGameSaveDataBuffer[playerIndex].replaySlots[1].length = 0;
    gGameSaveDataBuffer[playerIndex].replaySlots[2].length = 0;
    gGameSaveDataBuffer[playerIndex].replaySlots[3].length = 0;
    gGameSaveDataBuffer[playerIndex].replaySlots[4].length = 0;
    gGameSaveDataBuffer[playerIndex].replaySlots[5].length = 0;
    gGameSaveDataBuffer[playerIndex].replaySlots[6].length = 0;
    gGameSaveDataBuffer[playerIndex].replaySlots[7].length = 0;
    gGameSaveDataBuffer[playerIndex].replaySlots[8].length = 0;

    for (record = 0; record < (s32)sizeof(gGameSaveDataBuffer[playerIndex].reserved); record++) {
        gGameSaveDataBuffer[playerIndex].reserved[record] = 0;
    }

    if (gMainMenuSecretCodeUnlocked == 1) {
        record = 0;
        while (record < 11) {
            gGameSaveDataBuffer[playerIndex].cupPlacements[record] = 1;
            record++;
        }
        record = 0;
        for (;;) {
            gGameSaveDataBuffer[playerIndex].courseUnlockStates[record] =
                gMultiplayerCourseSelectDefaultCourseIds[record];
            gGameSaveDataBuffer[playerIndex].courseUnlockStates[record + 1] =
                gMultiplayerCourseSelectDefaultCourseIds[record + 1];
            gGameSaveDataBuffer[playerIndex].courseUnlockStates[record + 2] =
                gMultiplayerCourseSelectDefaultCourseIds[record + 2];
            gGameSaveDataBuffer[playerIndex].courseUnlockStates[record + 3] =
                gMultiplayerCourseSelectDefaultCourseIds[record + 3];
            record += 4;
            if ((s8 *)gCourseSelectColumnSoundEffects == &gMultiplayerCourseSelectDefaultCourseIds[record]) {
                break;
            }
        }
        gGameSaveDataBuffer[playerIndex].characterFlags = 1;
        gGameSaveDataBuffer[playerIndex].progressionLevel = 3;
        gGameSaveDataBuffer[playerIndex].extraCourseUnlockFlags = 0x3F;
    }
}
