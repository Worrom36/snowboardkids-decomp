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
#define stepRaceMotionAnimationUntilEnd(player) stepRaceMotionAnimationUntilEnd(RACE_MOTION_STATE(player))
#define RACE_MOTION_STATE(player) ((RaceMotionState *)(player))

void updateRacePlayerMode07LaunchRampTakeoff(RacePlayer *player) {
    s16 updateTimer;
    s32 unused;
    s32 scratch[14];

    updateTimer = player->updateTimer;
    if (updateTimer == 0) {
        player->updateTimer = updateTimer + 1;
        player->velocity.y = 0x30000;
        player->stateTimer = 0x1E;
        setRaceMotionAnimation(player, 4);
        if (player->stateFlags & 0x400) {
            player->unk80 = 0x20000;
        } else {
            player->unk80 = -0x20000;
        }
    }

    makeFixedRotationY((s16 *)&scratch[2], player->facingAngle);
    scratch[13] = 0;
    scratch[14] = 0;
    scratch[15] = player->unk80;
    transformVec3iByFixedMatrix((s16 *)&scratch[2], (Vec3i *)&scratch[13], (Vec3i *)&scratch[10]);
    player->pos.x += scratch[10];
    player->pos.z += scratch[12];

    makeFixedRotationY((s16 *)&scratch[2], player->facingAngle);
    scratch[13] = 0;
    scratch[14] = 0;
    scratch[15] = player->unk80;
    transformVec3iByFixedMatrix((s16 *)&scratch[2], (Vec3i *)&scratch[13], (Vec3i *)&scratch[10]);
    player->pos.x += scratch[10];
    player->pos.z += scratch[12];
    player->velocity.y -= 0x7000;
    player->pos.y += player->velocity.y;

    if ((player->velocity.y < 0) && !(player->stateFlags & 1)) {
        if (player->animationId != 1) {
            setRaceMotionAnimation(player, 1);
        }
        stepRaceMotionLoopingAnimation(player);
        updateRacePlayerMotionFeedback(player);
    } else {
        stepRaceMotionAnimationUntilEnd(player);
    }

    if (--player->stateTimer == 0) {
        player->updateState++;
        player->updateTimer = 0;
        player->stateTimer = 0x20;
        createCallbackTask((CallbackTaskCallback)initLaunchRampCourseObject, 0, 0x64);
        player->stateFlags &= ~0x20;
    }
}

void updateRacePlayerMode07LaunchRampSpin(RacePlayer *player) {
    s32 scratch[15];

    player->facingAngle += 0x10;
    if (player->unk80 > 0) {
        player->unk80 -= 0x820;
    } else {
        player->unk80 += 0x820;
    }

    makeFixedRotationY((s16 *)&scratch[1], player->facingAngle);
    scratch[12] = 0;
    scratch[13] = 0;
    scratch[14] = player->unk80;
    transformVec3iByFixedMatrix((s16 *)&scratch[1], (Vec3i *)&scratch[12], (Vec3i *)&scratch[9]);
    player->pos.x += scratch[9];
    player->pos.z += scratch[11];

    player->facingAngle += 0x10;
    if (player->unk80 > 0) {
        player->unk80 -= 0x820;
    } else {
        player->unk80 += 0x820;
    }

    makeFixedRotationY((s16 *)&scratch[1], player->facingAngle);
    scratch[12] = 0;
    scratch[13] = 0;
    scratch[14] = player->unk80;
    transformVec3iByFixedMatrix((s16 *)&scratch[1], (Vec3i *)&scratch[12], (Vec3i *)&scratch[9]);
    player->pos.x += scratch[9];
    player->pos.z += scratch[11];

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerMotionFeedback(player);
    if (--player->stateTimer == 0) {
        player->updateTimer = 0;
        player->updateState++;
    }
}

void updateRacePlayerMode07LaunchRampPose(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        setRaceMotionAnimation(player, 0x1D);
        player->stateTimer = 0x13;
    }
    stepRaceMotionAnimationUntilEnd(player);
    player->stateTimer--;
    if (player->stateTimer == 0) {
        player->updateTimer = 0;
        player->updateState++;
    }
}

void updateRacePlayerMode07LaunchRampDrift(RacePlayer *player) {
    s16 updateTimer;
    s32 scratch[14];

    updateTimer = player->updateTimer;
    if (updateTimer == 0) {
        player->updateTimer = updateTimer + 1;
        player->stateTimer = 0x13;
        if (player->stateFlags & 0x400) {
            player->unk80 = 0x40000;
        } else {
            player->unk80 = -0x40000;
        }
    }

    makeFixedRotationY((s16 *)&scratch[1], player->facingAngle);
    scratch[12] = 0;
    scratch[13] = 0;
    scratch[14] = player->unk80;
    transformVec3iByFixedMatrix((s16 *)&scratch[1], (Vec3i *)&scratch[12], (Vec3i *)&scratch[9]);
    player->pos.x += scratch[9];
    player->pos.z += scratch[11];

    player->stateTimer--;
    if (player->stateTimer == 0) {
        player->updateTimer = 0;
        player->updateState++;
    }
    stepRaceMotionAnimationUntilEnd(player);
}
