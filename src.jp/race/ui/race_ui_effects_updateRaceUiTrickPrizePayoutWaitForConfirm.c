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

#define RACE_UI_TRICK_PRIZE_SHOW_TOTAL_MONEY 3
#define RACE_UI_TRICK_PRIZE_REVEAL_DELAY 0x14
#define RACE_UI_TRICK_PRIZE_SHOW_COMPLETE_BONUS 2

extern s32 gMenuFlowState;

void updateRaceUiTrickPrizePayoutWaitForConfirm(RaceUiCourseStatsActor *actor) {
    if (gPlayerInputPressed[0] & A_BUTTON) {
        enqueueSoundEffect(0x18, 0x32);
        if (actor->task.userId < 5) {
            setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiTrickPrizeCourseValuesWaitForConfirm);
        } else {
            gMenuFlowState |= 0x10;
            setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiTrickPrizePayoutConfirmed);
        }
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackTotals, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutCounters, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackResultLabels, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutLabels, actor);
}

void updateRaceUiTrickPrizePayoutCountMoney(RaceUiCourseStatsActor *arg0) {
    s16 value;

    if (arg0->timer == 0) {
        arg0->timer = 2;
        enqueueSoundEffect(0x1A, 0x32);
    } else {
        arg0->timer--;
    }

    value = arg0->pendingTrickPrize;
    if ((value >= 8) && !(gPlayerInputPressed[0] & A_BUTTON)) {
        value = 8;
    }
    /* IDO register allocation depends on this always-zero index. */
    gRacePlayers[gPlayerInputPressed[0] * 0].money += value;
    arg0->pendingTrickPrize -= value;

    value = arg0->pendingMakeBonus;
    if ((value >= 8) && !(gPlayerInputPressed[0] & A_BUTTON)) {
        value = 8;
    }
    gRacePlayers[0].money += value;
    arg0->pendingMakeBonus -= value;

    value = arg0->pendingCompleteBonus;
    if ((value >= 8) && !(gPlayerInputPressed[0] & A_BUTTON)) {
        value = 8;
    }
    gRacePlayers[0].money += value;
    arg0->pendingCompleteBonus -= value;

    if (gRacePlayers[0].money >= 0xF4240) {
        gRacePlayers[0].money = 0xF423F;
    }

    if ((arg0->pendingTrickPrize == 0) && (arg0->pendingMakeBonus == 0) && (arg0->pendingCompleteBonus == 0)) {
        enqueueSoundEffect(2, 0x32);
        arg0->timer = 0x14;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiTrickPrizePayoutWaitForConfirm);
    }

    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackTotals, (void *)arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutCounters, (void *)arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackResultLabels, (void *)arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutLabels, (void *)arg0);
}

void updateRaceUiTrickPrizePayoutHoldMoneyRow(RaceUiCourseStatsActor *arg0) {
    arg0->visibleRows = RACE_UI_TRICK_PRIZE_SHOW_TOTAL_MONEY;
    arg0->timer--;
    if (arg0->timer == 0) {
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiTrickPrizePayoutCountMoney);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackTotals, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutCounters, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackResultLabels, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutLabels, arg0);
}

void updateRaceUiTrickPrizePayoutRevealMoneyRow(RaceUiCourseStatsActor *arg0) {
    arg0->visibleRows = 3;
    arg0->timer--;
    if (arg0->timer == 0) {
        arg0->timer = 0xA;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiTrickPrizePayoutHoldMoneyRow);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackTotals, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutCounters, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackResultLabels, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutLabels, arg0);
}

void updateRaceUiTrickPrizePayoutRevealCompleteBonus(RaceUiCourseStatsActor *arg0) {
    arg0->visibleRows = RACE_UI_TRICK_PRIZE_SHOW_COMPLETE_BONUS;
    arg0->timer--;
    if (arg0->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        arg0->timer = RACE_UI_TRICK_PRIZE_REVEAL_DELAY;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiTrickPrizePayoutRevealMoneyRow);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackTotals, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutCounters, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackResultLabels, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutLabels, arg0);
}

void updateRaceUiTrickPrizePayoutRevealMakeBonus(RaceUiCourseStatsActor *arg0) {
    arg0->visibleRows = 1;
    arg0->timer--;
    if (arg0->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        arg0->timer = RACE_UI_TRICK_PRIZE_REVEAL_DELAY;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiTrickPrizePayoutRevealCompleteBonus);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackTotals, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutCounters, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackResultLabels, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutLabels, arg0);
}

void updateRaceUiTrickPrizePayoutRevealTrickPrize(RaceUiCourseStatsActor *arg0) {
    arg0->timer--;
    arg0->visibleRows = 0;
    if (arg0->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        arg0->timer = RACE_UI_TRICK_PRIZE_REVEAL_DELAY;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiTrickPrizePayoutRevealMakeBonus);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackTotals, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutCounters, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackResultLabels, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutLabels, arg0);
}

void updateRaceUiTrickPrizePayoutWaitBeforeReveal(RaceUiCourseStatsActor *arg0) {
    arg0->timer--;
    if (arg0->timer == 0) {
        arg0->timer = RACE_UI_TRICK_PRIZE_REVEAL_DELAY;
        enqueueSoundEffect(0x1A, 0x32);
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiTrickPrizePayoutRevealTrickPrize);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackTotals, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickAttackResultLabels, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiTrickPrizePayoutLabels, arg0);
}
