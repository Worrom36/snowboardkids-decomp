#include "common.h"
#include "game/race/items/race_items.h"
#include "game/race/race_state.h"
#include <PR/os_libc.h>
#include "game/save_data.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/render_callback.h"
#include "assets.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/race/items/race_item_effects.h"
#include "game/race/player/race_player_movement.h"
#include "game/race/ui/race_hud.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/player/race_player_input.h"
#include "game/engine/callback_task_scheduler.h"

#define RACE_HUD_POPUP_FONT_HANDLE (gAssetHandles[0x1F])
#define ASSET_HANDLE(index) (gAssetHandles[index])

extern u8 gTrainingCourseLesson;

extern u8 gMainMenuModeSelection;

extern RaceTimer D_800DBC58[10];

extern RaceTimer D_800DBC80;

extern u8 gRaceTimeTrialFinishRecorded;

extern const char gRaceHudTrickAttackPointLabel[];
extern const char gRaceHudTrickAttackTimeLimitLabel[];
extern const char gRaceHudScoreAttackTimeLimitLabel[];
extern const char gRaceHudScoreAttackPointTargetFormat[];
extern const char gRaceHudTargetTimeChallengeSecondaryCounterFormat[];
extern const char gRaceHudTargetTimeChallengeFractionFormat[];
extern const char gRaceHudTargetTimeChallengeSecondsFormat[];
extern u8 gRaceChallengeFailed;
extern const char gRaceHudTargetTimeChallengeMinutesFormat[];
extern const char gRaceHudTargetTimeChallengeTargetTimeFormat[];
extern const char gRaceHudTargetTimeChallengeTargetTimeLabel[];
extern const char gRaceHudTargetTimeChallengeLapTimeLabel[];
extern const char gRaceHudTargetTimeChallengeLapProgressFormat[];
extern const char gRaceHudTimeTrialLapTimeLabel[];
extern const char gRaceHudTimeTrialBestLapFormat[];
extern const char gRaceHudTimeTrialBestLapLabel[];
extern u8 gRaceUpdatePaused;

void initRaceHud(void) {
    LOAD_ASSET(SHARED_UI_SPRITES, 0x1F);
    gRaceHudMode = RACE_HUD_MODE_NONE;
    gRaceHudCoinSpinnerFrame = 0;

    if (gPlayerCount == 1) {
        if (gRaceSplitscreenMode == 0) {
            gRaceHudMode = RACE_HUD_MODE_ONE_PLAYER;
        }
        if (gRaceSplitscreenMode == 2) {
            gRaceHudMode = RACE_HUD_MODE_TIME_TRIAL;
        }
        if (gRaceSplitscreenMode == 1) {
            if (gRaceTypeSelection == 0) {
                gRaceHudMode = RACE_HUD_MODE_TARGET_TIME_CHALLENGE;
            }
            if (gRaceTypeSelection == 1) {
                gRaceHudMode = RACE_HUD_MODE_SCORE_ATTACK;
                gRaceChallengeTimeLimit = D_800DBC58[gRaceCourseIndex.signedValue];
            }
            if (gRaceTypeSelection == 2) {
                gRaceHudMode = RACE_HUD_MODE_TRICK_ATTACK;
                gRaceChallengeTimeLimit = D_800DBC80;
            }
        }
    } else {
        gRaceHudMode = gPlayerCount - 1;
    }

    if ((gTrainingCourseLesson != 0) || (gMainMenuModeSelection != 0)) {
        gRaceHudMode = RACE_HUD_MODE_TRAINING_OR_DEMO;
    }

    gRaceElapsedTimer.minutes = 0;
    gRaceElapsedTimer.seconds = 0;
    gRaceElapsedTimer.fraction = 0;
    gRaceTimeTrialFinishRecorded = 0;
    gRaceChallengeFailed = 0;
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/func_80078068.s")

void drawTrickAttackChallengeLabels(void *arg0) {
    drawMenuAsciiTextDefaultScale(0x60, -0x61, gRaceHudTrickAttackPointLabel, 5);
    drawMenuAsciiTextDefaultScale(0x38, 0x47, gRaceHudTrickAttackTimeLimitLabel, 5);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/func_800784C0.s")

void drawScoreAttackChallengeLabels(void *arg0) {
    char sp18[0x20];

    drawMenuAsciiTextDefaultScale(0x38, 0x47, gRaceHudScoreAttackTimeLimitLabel, 5);
    sprintf(sp18, gRaceHudScoreAttackPointTargetFormat, gRacePlayers[0].scoreAttackPointTarget);
    drawMenuAsciiTextDefaultScale(0x70, -0x48, sp18, 6);
}

void drawTargetTimeChallengeHud(void *arg0) {
    char buffer[12];
    s32 x;
    s32 i;

    sprintf(buffer, gRaceHudTargetTimeChallengeMinutesFormat, gRaceElapsedTimer.minutes);
    x = 0x48;
    for (i = 0; i < 2; i++) {
        drawAssetTableSprite((s16)x, 0x50, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), (u8)buffer[i] - 5);
        x += 8;
    }

    x += 8;
    sprintf(buffer, gRaceHudTargetTimeChallengeSecondsFormat, gRaceElapsedTimer.seconds);
    for (i = 0; i < 2; i++) {
        drawAssetTableSprite((s16)x, 0x50, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), (u8)buffer[i] - 5);
        x += 8;
    }

    x += 8;
    sprintf(buffer, gRaceHudTargetTimeChallengeFractionFormat, gRaceElapsedTimer.fraction >> 8);
    for (i = 0; i != 2; i++) {
        drawAssetTableSprite((s16)x, 0x50, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), (u8)buffer[i] - 5);
        x += 8;
    }

    drawAssetTableSprite(0x58, 0x50, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x36);
    drawAssetTableSprite(0x70, 0x50, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x35);

    if (gRaceChallengeFailed != 0) {
        drawAssetTableSprite(0x48, 0x54, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x4F);
    }

    drawAssetTableSprite(
        -0x88,
        -0x60,
        getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE),
        gRaceHudActionEffectTileIds[1]
    );
    drawAssetTableSprite(-0x88, 0x40, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x24);
    drawAssetTableSprite(-0x88, 0x40, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), 0x25);

    sprintf(buffer, gRaceHudTargetTimeChallengeSecondaryCounterFormat, gRacePlayers[0].collectedCourseCoinMarkerCount);
    drawAssetTableSprite(-0x60, -0x50, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), (u8)buffer[0] - 5);
    if (buffer[1] != 0) {
        drawAssetTableSprite(-0x58, -0x50, getRelocatableHeapBlockBase(RACE_HUD_POPUP_FONT_HANDLE), (u8)buffer[1] - 5);
    }
}

void drawTargetTimeChallengeLabels(void *arg0) {
    char sp28[0x20];
    RaceTimer *targetTime;

    drawMenuAsciiTextDefaultScale(0x48, 0x47, gRaceHudTargetTimeChallengeLapTimeLabel, 5);
    drawMenuAsciiTextDefaultScale(0x32, -0x60, gRaceHudTargetTimeChallengeTargetTimeLabel, 7);
    targetTime = &gRaceCourseTargetTimes[gRaceCourseIndex.signedValue];
    sprintf(
        sp28,
        gRaceHudTargetTimeChallengeTargetTimeFormat,
        targetTime->minutes,
        targetTime->seconds,
        targetTime->fraction >> 8
    );
    drawMenuAsciiTextDefaultScale(0x48, -0x57, sp28, 7);
    sprintf(
        sp28,
        gRaceHudTargetTimeChallengeLapProgressFormat,
        gRacePlayers[0].collectedCourseCoinMarkerCount,
        gRacePlayers[0].totalCourseCoinMarkerCount
    );
    sp28[1] = ' ';
    if (sp28[2] != '/') {
        sp28[2] = ' ';
    }
    drawMenuAsciiTextDefaultScale(-0x68, -0x48, sp28, 6);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/func_80078C54.s")

void drawTimeTrialLabels(void *unused) {
    char bestLapText[0x20];

    drawMenuAsciiTextDefaultScale(0x48, 0x47, gRaceHudTimeTrialLapTimeLabel, 5);
    drawMenuAsciiTextDefaultScale(0x48, -0x61, gRaceHudTimeTrialBestLapLabel, 7);
    sprintf(
        bestLapText,
        gRaceHudTimeTrialBestLapFormat,
        gGameSaveDataBuffer[0].timeTrialRecords[gRaceCourseIndex.signedValue][0].minutes,
        gGameSaveDataBuffer[0].timeTrialRecords[gRaceCourseIndex.signedValue][0].seconds,
        gGameSaveDataBuffer[0].timeTrialRecords[gRaceCourseIndex.signedValue][0].fraction >> 8
    );
    drawMenuAsciiTextDefaultScale(0x48, -0x58, bestLapText, 7);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/func_80078F38.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/func_80079250.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/func_80079258.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/func_800794DC.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/func_80079948.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/func_80079A04.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/func_80079C08.s")

void drawThreePlayerNoEntryPanel(void *arg0) {
    drawAssetTableSprite(0xC, 0x2C, getRelocatableHeapBlockBase(ASSET_HANDLE(0x1F)), 0x90);
}

void incrementRaceElapsedTimer(void) {
    if (gRaceUpdatePaused == 0) {
        gRaceElapsedTimer.fraction = gRaceElapsedTimer.fraction + 0x358;
        if (gRaceElapsedTimer.fraction >= 0x6401) {
            gRaceElapsedTimer.fraction = 0;
            gRaceElapsedTimer.seconds = gRaceElapsedTimer.seconds + 1;
            if (gRaceElapsedTimer.seconds >= 0x3C) {
                gRaceElapsedTimer.seconds = 0;
                gRaceElapsedTimer.minutes = gRaceElapsedTimer.minutes + 1;
                if (gRaceElapsedTimer.minutes >= 0x63) {
                    gRaceElapsedTimer.fraction = 0x6300;
                    gRaceElapsedTimer.seconds = 0x3B;
                    gRaceElapsedTimer.minutes = 0x63;
                }
            }
        }
    }
}

void decrementRaceChallengeTimeLimit(void) {
    if (gRaceUpdatePaused == 0) {
        gRaceChallengeTimeLimit.fraction = gRaceChallengeTimeLimit.fraction - 0x358;
        if (gRaceChallengeTimeLimit.fraction < 0) {
            gRaceChallengeTimeLimit.fraction = 0x60F8;
            gRaceChallengeTimeLimit.seconds = gRaceChallengeTimeLimit.seconds - 1;
            if (gRaceChallengeTimeLimit.seconds < 0) {
                gRaceChallengeTimeLimit.seconds = 0x3B;
                gRaceChallengeTimeLimit.minutes = gRaceChallengeTimeLimit.minutes - 1;
                if (gRaceChallengeTimeLimit.minutes < 0) {
                    gRaceChallengeTimeLimit.fraction = 0;
                    gRaceChallengeTimeLimit.seconds = 0;
                    gRaceChallengeTimeLimit.minutes = 0;
                }
            }
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/drawRaceCourseProgressMeter.s")

void updateRaceCourseProgressMeter(void) {
    s32 meterPosition;
    s32 slots[2];
    s32 i;

    for (i = 0; i < 4; i++) {
        getRacePlayerRankingProgress(i, &slots[1], &slots[0]);
        if (slots[1] < 0) {
            slots[1] = 0;
        }
        gRacePlayers[i].progressMeterPosition =
            (slots[1] << 7) / (gRaceCourseStartEntries[gRaceCourseIndex.signedValue].pathIndex * 8);
        if (gRacePlayers[i].progressMeterPosition >= 0x81) {
            gRacePlayers[i].progressMeterPosition = 0x80;
        }
    }

    if (gRaceHudMode == RACE_HUD_MODE_ONE_PLAYER) {
        addRenderCallback(&gRaceOverlayRenderCallbackList, drawRaceCourseProgressMeter, 0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, drawRaceCourseProgressMeter, 0);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/ui/race_hud/updateRaceHud.s")

s32 calculateRaceTimerDelta(RaceTimer *arg0, RaceTimer *arg1, RaceTimer *arg2) {
    s32 total1;
    s32 diff;

    diff = (arg0->fraction & 0xFF00) + (arg0->seconds * 0x6400) + (arg0->minutes * 0x177000);
    total1 = (arg1->fraction & 0xFF00) + (arg1->seconds * 0x6400) + (arg1->minutes * 0x177000);
    arg0 = (RaceTimer *)1;
    if (diff >= total1) {
        arg0 = (RaceTimer *)0;
        diff -= total1;
    } else {
        diff = total1 - diff;
    }
    arg2->fraction = diff % 0x6400;
    diff /= 0x6400;
    arg1 = (RaceTimer *)60;
    arg2->seconds = diff % (s32)arg1;
    diff /= (s32)arg1;
    arg2->minutes = diff % 99;
    return (s32)arg0;
}
