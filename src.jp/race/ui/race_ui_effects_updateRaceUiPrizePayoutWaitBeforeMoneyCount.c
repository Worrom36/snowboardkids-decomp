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

#define RACE_UI_PRIZE_PAYOUT_REVEAL_TIMER 0x14

void updateRaceUiPrizePayoutWaitBeforeMoneyCount(RaceUiPrizePayoutActor *actor) {
    actor->timer = actor->timer - 1;
    if (actor->timer == 0) {
        actor->timer = 0;
        setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiPrizePayoutCountMoney);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutRankPanel, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutRankPrize, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutCollectedCoins, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutTotalMoney, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutLabels, actor);
}

void updateRaceUiPrizePayoutRevealGetCoin(RaceUiPrizePayoutActor *actor) {
    actor->timer--;
    if (actor->timer == 0) {
        actor->timer = RACE_UI_PRIZE_PAYOUT_REVEAL_TIMER;
        enqueueSoundEffect(0x1A, 0x32);
        setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiPrizePayoutWaitBeforeMoneyCount);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutRankPanel, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutRankPrize, actor);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutCollectedCoins, actor);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutLabels, actor);
}

void updateRaceUiPrizePayoutRevealRankPrize(RaceUiPrizePayoutActor *arg0) {
    arg0->timer--;
    if (arg0->timer == 0) {
        enqueueSoundEffect(0x1A, 0x32);
        arg0->timer = RACE_UI_PRIZE_PAYOUT_REVEAL_TIMER;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiPrizePayoutRevealGetCoin);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutRankPanel, arg0);
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutRankPrize, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutLabels, arg0);
}

void updateRaceUiPrizePayoutShowRankPrize(RaceUiPrizePayoutActor *arg0) {
    arg0->timer--;
    if (arg0->timer == 0) {
        arg0->timer = RACE_UI_PRIZE_PAYOUT_REVEAL_TIMER;
        enqueueSoundEffect(0x1A, 0x32);
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiPrizePayoutRevealRankPrize);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutRankPanel, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutLabels, arg0);
}

void updateRaceUiPrizePayoutFadeIn(RaceUiPrizePayoutActor *arg0) {
    arg0->alpha += 0x10;
    if (arg0->alpha >= 0x100) {
        arg0->timer = RACE_UI_PRIZE_PAYOUT_REVEAL_TIMER;
        arg0->alpha = 0xFF;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiPrizePayoutShowRankPrize);
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutRankPanel, arg0);
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawRaceUiPrizePayoutLabels, arg0);
}
