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

#define RACE_UI_SNOWBOARD_TRAIL_SCALE_X 0xF0000
#define RACE_UI_SNOWBOARD_TRAIL_SCALE_Y 0x230000
#define RACE_UI_SNOWBOARD_TRAIL_SCALE_Z 0
#define RACE_UI_SNOWBOARD_TRAIL_TIMER 0xF
#define RACE_UI_SNOWBOARD_TRAIL_INITIAL_VELOCITY_Y 0x20000

void initRaceUiExpiredSpeedFanEffect(RaceUiExpiredSpeedFanActor *actor) {
    actor->scaleStep = 1;
    actor->scale.x = RACE_UI_SNOWBOARD_TRAIL_SCALE_X;
    actor->scale.y = RACE_UI_SNOWBOARD_TRAIL_SCALE_Y;
    actor->scale.z = RACE_UI_SNOWBOARD_TRAIL_SCALE_Z;
    actor->timer = RACE_UI_SNOWBOARD_TRAIL_TIMER;
    actor->velocityY = RACE_UI_SNOWBOARD_TRAIL_INITIAL_VELOCITY_Y;
    updateRaceUiExpiredSpeedFanEffect(actor);
    setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiExpiredSpeedFanEffect);
}

void spawnRaceUiExpiredSpeedFanEffect(RacePlayer *player) {
    RaceUiExpiredSpeedFanActor *actor =
        createCallbackTaskPreservingArgs((CallbackTaskCallback)initRaceUiExpiredSpeedFanEffect, 0, 0x62);

    if (actor != NULL) {
        actor->playerIndex = player->playerIndex;
        actor->worldPos = player->speedFan.worldPos;
        actor->frontTransform = player->speedFan.frontTransform;
        actor->spinYaw = player->speedFan.spinYaw;
    }
}
