#include "game/engine/model_asset_symbols.h"
#include "game/race/course/race_course_effects.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/race_state.h"
#include "common.h"
#include <PR/os_libc.h>
#include "game/save_data.h"
#include "game/audio/sound_manager.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/render_callback.h"
#include "game/engine/system_runtime.h"
#include "game/engine/controller_input.h"
#include "assets.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/effects/snowboard_trail_effects.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/race/items/race_item_hit_flags.h"
#include "game/engine/viewport_manager.h"
#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"
#include "game/race/motion/race_motion.h"
#include "game/race/items/race_item_projectiles.h"
#include "game/race/player/race_player_movement.h"
#include "game/race/player/race_player_input.h"
#include "game/race/camera/race_camera.h"
#include "game/race/ui/race_hud.h"
#include <PR/os_cache.h>

extern const char gRaceUiTargetTimeChallengeLapTimeLabel[0xC];
extern const char gRaceUiTargetTimeChallengePrizeLabel[0x10];
extern const char gRaceUiTargetTimeChallengeCompleteBonusLabel[0x14];
extern const char gRaceUiTargetTimeChallengeSpeedFanGetLabel[0x14];
extern const char gRaceUiTargetTimeChallengeDeltaTimeFormat[0x14];
extern const char gRaceUiTargetTimeChallengePerfectGetLabel[0x10];
extern const char gRaceUiTargetTimeChallengeTargetLabel[0xC];
extern const char gRaceUiTargetTimeChallengeMoneyLabel[0x10];
extern const char gRaceUiTargetTimeChallengeTimeOutLabel[0xC];
extern u8 gRaceChallengeFailed;
extern const char gRaceUiTargetTimeChallengeTargetTimeFormat[0x14];
extern const char gRaceUiTargetTimeChallengeFastLabel[0x8];

void drawRaceUiCourseRecordLapTimeLabel(RaceUiAlphaActor *arg0) {
    Gfx *gfx;
    s32 w0;
    s32 w1;

    /* IDO scheduling for this function depends on this block staying on one line. */
    do { \
        if (arg0->alpha != 0xFF) { \
            gfx = gRegionAllocPtr; \
            gRegionAllocPtr = gfx + 1; \
            gfx->words.w1 = 0; \
            gfx->words.w0 = 0xE7000000; \
            gfx = gRegionAllocPtr; \
            gRegionAllocPtr = gfx + 1; \
            w1 = 0xFF2FFFFF; \
            w0 = 0xFC119623; \
            gfx->words.w0 = w0; \
            gfx->words.w1 = w1; \
            gfx = gRegionAllocPtr; \
            gRegionAllocPtr = gfx + 1; \
            w1 = 0x00504240; \
            w0 = 0xB900031D; \
            gfx->words.w0 = w0; \
            gfx->words.w1 = w1; \
            gfx = gRegionAllocPtr; \
            gRegionAllocPtr = gfx + 1; \
            gfx->words.w0 = 0xFA000000; \
            gfx->words.w1 = (arg0->alpha & 0xFF) | (~0xFF); \
        } \
        drawMenuAsciiTextDefaultScale(-0x70, -0x40, gRaceUiTargetTimeChallengeLapTimeLabel, 5); \
        if (arg0->alpha != 0xFF) { \
            gfx = gRegionAllocPtr; \
            gRegionAllocPtr = gfx + 1; \
            w1 = (s32)gMenuRenderModeResetDl; \
            w0 = 0x06000000; \
            gfx->words.w0 = w0; \
            gfx->words.w1 = w1; \
        } \
    } while (0);
}

void drawRaceUiCourseRecordPayoutLabels(RaceUiDualCounterActor *arg0) {
    char buf[0x18];
    char *bufp;
    s32 y;
    s16 textY;
    RaceTimer *targetTime;

    if (arg0->alpha != 0xFF) {
        gDPPipeSync(gRegionAllocPtr++);
        gDPSetCombineMode(gRegionAllocPtr++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetRenderMode(gRegionAllocPtr++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 0xFF, 0xFF, 0xFF, arg0->alpha);
    }

    if (gRaceChallengeFailed == 0) {
        drawMenuAsciiTextDefaultScale(-0x68, -0x1F, gRaceUiTargetTimeChallengeTargetLabel, 7);
        targetTime = &gRaceCourseTargetTimes[gRaceCourseIndex.signedValue];
        bufp = buf - 0xC;
        sprintf(
            bufp,
            gRaceUiTargetTimeChallengeTargetTimeFormat,
            targetTime->minutes,
            targetTime->seconds,
            targetTime->fraction >> 8
        );
        drawMenuAsciiTextDefaultScale(-0x68, -0x16, bufp, 7);
        drawMenuAsciiTextDefaultScale(-0x68, -8, gRaceUiTargetTimeChallengeFastLabel, 7);
        sprintf(bufp, gRaceUiTargetTimeChallengeDeltaTimeFormat, arg0->row, arg0->column, arg0->timeFraction >> 8);
        drawMenuAsciiTextDefaultScale(-0x6C, 1, bufp, 7);
    } else {
        drawMenuAsciiTextDefaultScale(-0x68, -0x24, gRaceUiTargetTimeChallengeTimeOutLabel, 4);
    }

    y = -0x50;
    drawMenuAsciiTextDefaultScale(-8, (s16)y, gRaceUiTargetTimeChallengePrizeLabel, 5);
    y += 0x20;
    drawMenuAsciiTextDefaultScale(-0xC, (s16)y, gRaceUiTargetTimeChallengeSpeedFanGetLabel, 5);
    y += 0x20;
    if (arg0->flag != 0) {
        drawMenuAsciiTextDefaultScale(-0xC, (s16)y, gRaceUiTargetTimeChallengePerfectGetLabel, 5);
        y += 0x20;
    }
    drawMenuAsciiTextDefaultScale(-8, (s16)y, gRaceUiTargetTimeChallengeCompleteBonusLabel, 5);
    y += 0x20;
    textY = y;
    drawMenuAsciiTextDefaultScale(-0xC, textY, gRaceUiTargetTimeChallengeMoneyLabel, 5);

    if (arg0->alpha != 0xFF) {
        gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
    }
}
