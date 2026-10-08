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

#define RACE_UI_COURSE_RECORD_HOLD_FINAL_MONEY_STATE 4
#define RACE_UI_RESULTS_REVEAL_TIMER 0x14
#define RACE_UI_COURSE_RECORD_FINAL_MONEY_STATE 3
#define RACE_UI_COURSE_RECORD_REVEAL_PERFECT_GET_STATE 2
#define RACE_UI_COURSE_RECORD_REVEAL_SPEED_FAN_GET_STATE 1
#define RACE_UI_COURSE_RECORD_REVEAL_TIME_PRIZE_STATE 0
#define RACE_UI_RESULTS_FADE_STEP 0x10
#define RACE_UI_RESULTS_FULL_ALPHA 0xFF

extern s32 gMenuFlowState;

void updateRaceUiCourseRecordPayoutWaitForConfirm(RaceUiDualCounterActor *actor) {
    if (gPlayerInputPressed[0] & A_BUTTON) {
        enqueueSoundEffect(0x18, 0x32);
        if (actor->task.userId < 5) {
            setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiCourseRecordRowsWaitForConfirm);
        } else {
            gMenuFlowState |= 0x10;
            setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiCourseRecordPayoutConfirmed);
        }
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordElapsedTime, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutCounters, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordLapTimeLabel, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutLabels, actor);
}

void updateRaceUiCourseRecordPayoutCountMoney(RaceUiDualCounterActor *arg0) {
    s16 value;

    if (arg0->timer == 0) {
        arg0->timer = 2;
        enqueueSoundEffect(0x1A, 0x32);
    } else {
        arg0->timer--;
    }

    value = arg0->leftValue;
    if ((value >= 8) && !(gPlayerInputPressed[0] & A_BUTTON)) {
        value = 8;
    }
    /* IDO register allocation depends on this always-zero index. */
    gRacePlayers[gPlayerInputPressed[0] * 0].money += value;
    arg0->leftValue -= value;

    value = arg0->rightValue;
    if ((value >= 8) && !(gPlayerInputPressed[0] & A_BUTTON)) {
        value = 8;
    }
    gRacePlayers[0].money += value;
    arg0->rightValue -= value;

    value = arg0->bonus;
    if ((value >= 8) && !(gPlayerInputPressed[0] & A_BUTTON)) {
        value = 8;
    }
    gRacePlayers[0].money += value;
    arg0->bonus -= value;

    value = arg0->leftTarget;
    if ((value >= 8) && !(gPlayerInputPressed[0] & A_BUTTON)) {
        value = 8;
    }
    gRacePlayers[0].money += value;
    arg0->leftTarget -= value;

    if (gRacePlayers[0].money >= 0xF4240) {
        gRacePlayers[0].money = 0xF423F;
    }

    if ((arg0->rightValue == 0) && (arg0->leftValue == 0) && (arg0->bonus == 0) && (arg0->leftTarget == 0)) {
        enqueueSoundEffect(2, 0x32);
        arg0->timer = 0x14;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiCourseRecordPayoutWaitForConfirm);
    }

    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordElapsedTime, (void *)arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutCounters, (void *)arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordLapTimeLabel, (void *)arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutLabels, (void *)arg0);
}

void updateRaceUiCourseRecordWaitBeforePayout(RaceUiDualCounterActor *actor) {
    actor->state = RACE_UI_COURSE_RECORD_HOLD_FINAL_MONEY_STATE;
    actor->timer--;
    if (actor->timer == 0) {
        setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiCourseRecordPayoutCountMoney);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordElapsedTime, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutCounters, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordLapTimeLabel, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutLabels, actor);
}

void updateRaceUiCourseRecordHoldFinalMoney(RaceUiDualCounterActor *actor) {
    actor->state = RACE_UI_COURSE_RECORD_HOLD_FINAL_MONEY_STATE;
    actor->timer--;
    if (actor->timer == 0) {
        actor->timer = 0xA;
        setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiCourseRecordWaitBeforePayout);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordElapsedTime, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutCounters, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordLapTimeLabel, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutLabels, actor);
}

void updateRaceUiCourseRecordRevealFinalMoney(RaceUiDualCounterActor *actor) {
    actor->state = RACE_UI_COURSE_RECORD_FINAL_MONEY_STATE;
    actor->timer--;
    if (actor->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        actor->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiCourseRecordHoldFinalMoney);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordElapsedTime, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutCounters, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordLapTimeLabel, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutLabels, actor);
}

void updateRaceUiCourseRecordRevealPerfectGet(RaceUiDualCounterActor *actor) {
    actor->state = RACE_UI_COURSE_RECORD_REVEAL_PERFECT_GET_STATE;
    actor->timer--;
    if (actor->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        actor->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiCourseRecordRevealFinalMoney);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordElapsedTime, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutCounters, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordLapTimeLabel, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutLabels, actor);
}

void updateRaceUiCourseRecordRevealSpeedFanGet(RaceUiDualCounterActor *actor) {
    actor->state = RACE_UI_COURSE_RECORD_REVEAL_SPEED_FAN_GET_STATE;
    actor->timer--;
    if (actor->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        actor->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        if (actor->flag != 0) {
            setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiCourseRecordRevealPerfectGet);
        } else {
            setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiCourseRecordRevealFinalMoney);
        }
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordElapsedTime, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutCounters, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordLapTimeLabel, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutLabels, actor);
}

void updateRaceUiCourseRecordRevealTimePrize(RaceUiDualCounterActor *actor) {
    actor->state = RACE_UI_COURSE_RECORD_REVEAL_TIME_PRIZE_STATE;
    actor->timer--;
    if (actor->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        actor->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiCourseRecordRevealSpeedFanGet);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordElapsedTime, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutCounters, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordLapTimeLabel, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutLabels, actor);
}

void updateRaceUiCourseRecordWaitBeforeReveal(RaceUiDualCounterActor *actor) {
    actor->timer--;
    if (actor->timer == 0) {
        actor->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        enqueueSoundEffect(0x1A, 0x32);
        setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiCourseRecordRevealTimePrize);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordElapsedTime, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordLapTimeLabel, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutLabels, actor);
}

void updateRaceUiCourseRecordFadeIn(RaceUiDualCounterActor *arg0) {
    arg0->alpha += RACE_UI_RESULTS_FADE_STEP;
    if (arg0->alpha >= 0x100) {
        arg0->timer = RACE_UI_RESULTS_REVEAL_TIMER;
        arg0->alpha = RACE_UI_RESULTS_FULL_ALPHA;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiCourseRecordWaitBeforeReveal);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordElapsedTime, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordLapTimeLabel, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiCourseRecordPayoutLabels, arg0);
}
