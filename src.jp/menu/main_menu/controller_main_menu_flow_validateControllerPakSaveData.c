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

#define CONTROLLER_PAK_RECORD_AT(cursor, field) \
    (*(GameSaveRecordTime *)((cursor) + CONTROLLER_PAK_SAVE_FIELD_OFFSET(field)))
#define CONTROLLER_PAK_U8_AT(cursor, field) (*(u8 *)((cursor) + CONTROLLER_PAK_SAVE_FIELD_OFFSET(field)))
#define CONTROLLER_PAK_U16_AT(cursor, field) (*(u16 *)((cursor) + CONTROLLER_PAK_SAVE_FIELD_OFFSET(field)))
#define CONTROLLER_PAK_SAVE_FIELD_OFFSET(field) ((u32) & (((GameSaveData *)0)->field))

extern u8 gMainMenuReturnFromRace;

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
