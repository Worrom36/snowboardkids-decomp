#include "game/race/items/race_items.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/race_state.h"
#include "common.h"
#include "game/engine/render_callback.h"
#include "game/engine/asset_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/race/player/race_player_rumble.h"
#include "game/audio/sound_manager.h"
#include "game/race/motion/race_motion.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/race_course_effects.h"
#include "game/race/player/race_player_collision.h"
#include "game/math/fixed_point_math.h"
#include "game/math/spatial_math.h"
#include "game/race/player/race_player_input.h"
#include "game/race/items/race_item_effects.h"
#include "game/race/player/race_player_pickup_effects.h"
#include "game/menu/renderer/menu_screen_effects.h"
#include "game/race/player/race_player_movement.h"
#include "game/race/player/race_player_progress.h"
#include "game/race/player/race_player_model_renderer.h"
#include "game/race/player/race_player_update.h"
#include "game/race/course/race_surface_cues.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/effects/snowboard_trail_effects.h"
#include "game/engine/viewport_manager.h"

#define setRaceMotionAnimation(player, animIndex) setRaceMotionAnimation(RACE_MOTION_STATE(player), (animIndex))
#define stepRaceMotionLoopingAnimation(player) stepRaceMotionLoopingAnimation(RACE_MOTION_STATE(player))
#define RACE_MOTION_STATE(player) ((RaceMotionState *)(player))

void updateRacePlayerMode40Stun(RacePlayer *player) {
    s16 updateState;
    s32 velocityX;
    s32 velocityZ;
    u32 stateFlags;
    s16 nextState;

    updateState = player->updateState;
    nextState = updateState + 1;
    if (updateState == 0) {
        player->updateState = nextState;
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x01006000;
        setRaceMotionAnimation(player, 0x12);
        player->stateTimer = 0x1E;
        player->stateTimer += randomNextMain() >> 4;
        player->targetGroundOffset = 0;
    }
    stepRaceMotionLoopingAnimation(player);
    updateRacePlayerLeanAngle(player, 0x10000, 0);
    player->velocity.y += 0xFFFF6000;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    stateFlags = player->stateFlags;
    if ((stateFlags & 1) == 0) {
        player->stateFlags = stateFlags & ~0x200;
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0, 0, 0);
        player->stateFlags &= ~0x200;
        player->unk582 = 0x100;
        player->unk584 = 8;
        player->unk588 = 0.0f;
    } else {
        player->stateFlags = stateFlags | 0x200;
    }
    velocityX = player->velocity.x;
    player->pos.x += velocityX;
    player->pos.y += player->velocity.y;
    velocityZ = player->velocity.z;
    player->pos.z += velocityZ;
    player->facingAngle = calculateFixedAngleFromDeltaXZ(velocityX, velocityZ);
    if (player->stateFlags & 0x400) {
        player->facingAngle += 0x800;
    }
    if (--player->stateTimer == 0) {
        player->mode = 0xC;
        player->updateState = 0;
        player->updateTimer = 0;
    }
    player->rumbleTimer = 4;
    player->rumblePatternId = 2;
}
