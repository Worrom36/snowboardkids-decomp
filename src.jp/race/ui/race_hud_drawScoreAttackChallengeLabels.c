#include "game/race/items/race_items.h"
#include "game/race/race_state.h"
#include "common.h"
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
