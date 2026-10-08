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

#define RACE_UI_HIT_PRIZE_SHOW_TOTAL_MONEY 3
#define RACE_UI_RESULTS_REVEAL_TIMER 0x14
#define RACE_UI_HIT_PRIZE_SHOW_COMPLETE_BONUS 2
#define RACE_UI_HIT_PRIZE_SHOW_PERFECT_HIT 1
#define RACE_UI_HIT_PRIZE_SHOW_HIT_PRIZE 0
#define RACE_UI_RESULTS_FADE_STEP 0x10
#define RACE_UI_RESULTS_FULL_ALPHA 0xFF

extern s32 gMenuFlowState;

void updateRaceUiHitPrizePayoutWaitForConfirm(RaceUiCounterActor *actor) {
    if (gPlayerInputPressed[0] & A_BUTTON) {
        enqueueSoundEffect(0x18, 0x32);
        if (actor->task.userId < 5) {
            setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiHitPrizeCourseValuesWaitForConfirm);
        } else {
            gMenuFlowState |= 0x10;
            setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiHitPrizePayoutConfirmed);
        }
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackPointTotal, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutCounters, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackTargetStatus, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutLabels, actor);
}

void updateRaceUiHitPrizePayoutCountMoney(RaceUiCounterActor *arg0) {
    s16 value;

    if (arg0->timer == 0) {
        arg0->timer = 2;
        enqueueSoundEffect(0x1A, 0x32);
    } else {
        arg0->timer--;
    }

    value = arg0->value;
    if ((value >= 8) && !(gPlayerInputPressed[0] & A_BUTTON)) {
        value = 8;
    }
    /* IDO register allocation depends on this always-zero index. */
    gRacePlayers[gPlayerInputPressed[0] * 0].money += value;
    arg0->value -= value;

    value = arg0->bonus;
    if ((value >= 8) && !(gPlayerInputPressed[0] & A_BUTTON)) {
        value = 8;
    }
    gRacePlayers[0].money += value;
    arg0->bonus -= value;

    value = arg0->target;
    if ((value >= 8) && !(gPlayerInputPressed[0] & A_BUTTON)) {
        value = 8;
    }
    gRacePlayers[0].money += value;
    arg0->target -= value;

    if (gRacePlayers[0].money >= 0xF4240) {
        gRacePlayers[0].money = 0xF423F;
    }

    if ((arg0->value == 0) && (arg0->bonus == 0) && (arg0->target == 0)) {
        enqueueSoundEffect(2, 0x32);
        arg0->timer = 0x14;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiHitPrizePayoutWaitForConfirm);
    }

    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackPointTotal, (void *)arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutCounters, (void *)arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackTargetStatus, (void *)arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutLabels, (void *)arg0);
}

void updateRaceUiHitPrizeWaitBeforePayout(RaceUiCounterActor *arg0) {
    arg0->state = RACE_UI_HIT_PRIZE_SHOW_TOTAL_MONEY;
    arg0->timer--;
    if (arg0->timer == 0) {
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiHitPrizePayoutCountMoney);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackPointTotal, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutCounters, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackTargetStatus, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutLabels, arg0);
}

void updateRaceUiHitPrizeRevealTotalMoney(RaceUiCounterActor *arg0) {
    arg0->state = RACE_UI_HIT_PRIZE_SHOW_TOTAL_MONEY;
    arg0->timer--;
    if (arg0->timer == 0) {
        arg0->timer = 0xA;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiHitPrizeWaitBeforePayout);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackPointTotal, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutCounters, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackTargetStatus, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutLabels, arg0);
}

void updateRaceUiHitPrizeRevealCompleteBonus(RaceUiCounterActor *arg0) {
    arg0->state = RACE_UI_HIT_PRIZE_SHOW_COMPLETE_BONUS;
    arg0->timer--;
    if (arg0->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        arg0->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiHitPrizeRevealTotalMoney);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackPointTotal, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutCounters, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackTargetStatus, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutLabels, arg0);
}

void updateRaceUiHitPrizeRevealPerfectHit(RaceUiCounterActor *arg0) {
    arg0->state = RACE_UI_HIT_PRIZE_SHOW_PERFECT_HIT;
    arg0->timer--;
    if (arg0->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        arg0->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiHitPrizeRevealCompleteBonus);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackPointTotal, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutCounters, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackTargetStatus, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutLabels, arg0);
}

void updateRaceUiHitPrizeHoldHitPrize(RaceUiCounterActor *arg0) {
    arg0->state = RACE_UI_HIT_PRIZE_SHOW_HIT_PRIZE;
    arg0->timer--;
    if (arg0->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        arg0->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        if (arg0->hasPerfectHitBonus != 0) {
            setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiHitPrizeRevealPerfectHit);
        } else {
            setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiHitPrizeRevealCompleteBonus);
        }
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackPointTotal, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutCounters, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackTargetStatus, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutLabels, arg0);
}

void updateRaceUiHitPrizeWaitBeforeHitPrize(RaceUiCounterActor *arg0) {
    arg0->timer--;
    if (arg0->timer == 0) {
        arg0->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        enqueueSoundEffect(0x1A, 0x32);
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiHitPrizeHoldHitPrize);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackPointTotal, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackTargetStatus, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutLabels, arg0);
}

void updateRaceUiHitPrizeFadeIn(RaceUiCounterActor *arg0) {
    arg0->alpha += RACE_UI_RESULTS_FADE_STEP;
    if (arg0->alpha >= 0x100) {
        arg0->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        arg0->alpha = RACE_UI_RESULTS_FULL_ALPHA;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiHitPrizeWaitBeforeHitPrize);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackPointTotal, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiScoreAttackTargetStatus, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiHitPrizePayoutLabels, arg0);
}
