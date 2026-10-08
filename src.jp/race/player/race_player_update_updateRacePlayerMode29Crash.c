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
#define stepRaceMotionAnimationUntilEnd(player) stepRaceMotionAnimationUntilEnd(RACE_MOTION_STATE(player))
#define RACE_MOTION_STATE(player) ((RaceMotionState *)(player))

void updateRacePlayerMode29Crash(RacePlayer *player) {
    s16 updateState;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        player->stateFlags &= ~0x200;
        player->stateTimer = 0;
        player->unk80 = 0;
        player->subState = 0;
        player->subStateTimer = 0;
        player->unk2FA = 0;
        player->unk528 = 0;
        setRaceMotionAnimation(player, 0x1E);
    }

    player->velocity.y -= player->gravity;
    player->facingAngle = player->unk332;
    player->pitchAngle = player->unk334;
    if (player->stateFlags & 0x400) {
        player->facingAngle += 0x800;
        player->pitchAngle = -(player->pitchAngle & 0xFFF);
    }

    updateRacePlayerLeanAngle(player, 0, 0);
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;

    if (stepRaceMotionAnimationUntilEnd(player) != 0) {
        setRaceMotionAnimation(player, 0x1F);
        stepRaceMotionAnimationUntilEnd(player);
    }

    player->unk578 = 6;
    updateRacePlayerMotionFeedback(player);
    if (player->unk517 != 0) {
        enqueueRacePlayerVoiceSound(player, 5);
    }
}

void updateRacePlayerMode01JumpStart(RacePlayer *player) {
    s32 unused;
    s32 targetX;
    s32 targetZ;
    s16 angleDiff;
    s32 yVel;

    if (player->updateState == 0) {
        if (player->animationId != 4) {
            setRaceMotionAnimation(player, 4);
        }
        player->updateState++;
        player->stateFlags |= 0x200;
        player->targetGroundOffset = 0;
        resetRacePlayerTrickSubstate(player);
        player->unk2A6 = 0;
    }

    if (player->subState == 0) {
        updateRacePlayerItemEffectUse(player);
        if (player->isCpu == 0) {
            if (player->stickX > 0) {
                player->facingAngle -= 0x18;
            }
            if (player->stickX < 0) {
                player->facingAngle += 0x18;
            }
        } else {
            getRaceCourseTargetPositionAhead(
                player->coursePathIndex,
                unused = player->pos.x,
                player->pos.z,
                &targetX,
                &targetZ,
                (s8)player->unk17,
                (u16)player->playerIndex
            );
            angleDiff = (calculateFixedAngleBetweenXZPoints(player->pos.x, player->pos.z, targetX, targetZ) -
                         player->facingAngle) &
                        0xFFF;
            if (angleDiff >= 0x801) {
                angleDiff -= 0x1000;
            }
            if (angleDiff >= 0x19) {
                angleDiff = 0x18;
            }
            if (angleDiff < -0x18) {
                angleDiff = -0x18;
            }
            player->facingAngle += angleDiff;
        }
        stepRaceMotionAnimationUntilEnd(player);
    }

    updateRacePlayerTrickSubstate(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;
    if (player->unk517 != 0) {
        enqueueRacePlayerVoiceSound(player, 5);
    }
}

void updateRacePlayerMode22Airborne(RacePlayer *player) {
    s32 unused;
    s32 targetX;
    s32 targetZ;
    s16 angleDiff;
    s32 yVel;

    if (player->updateState == 0) {
        player->updateState++;
        player->stateFlags |= 0x208;
        player->targetGroundOffset = 0;
        player->unk80 = 0;
        resetRacePlayerTrickSubstate(player);
        player->unk2A6 = 0;
    }

    if (player->subState == 0) {
        updateRacePlayerItemEffectUse(player);
        if (player->isCpu == 0) {
            if (player->stickX > 0) {
                player->facingAngle -= 0x18;
            }
            if (player->stickX < 0) {
                player->facingAngle += 0x18;
            }
        } else {
            getRaceCourseTargetPositionAhead(
                player->coursePathIndex,
                unused = player->pos.x,
                player->pos.z,
                &targetX,
                &targetZ,
                (s8)player->unk17,
                (u16)player->playerIndex
            );
            angleDiff = (calculateFixedAngleBetweenXZPoints(player->pos.x, player->pos.z, targetX, targetZ) -
                         player->facingAngle) &
                        0xFFF;
            if (angleDiff >= 0x801) {
                angleDiff -= 0x1000;
            }
            if (angleDiff >= 0x19) {
                angleDiff = 0x18;
            }
            if (angleDiff < -0x18) {
                angleDiff = -0x18;
            }
            player->facingAngle += angleDiff;
        }
    }

    updateRacePlayerTrickSubstate(player);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;
    if (player->unk517 != 0) {
        enqueueRacePlayerVoiceSound(player, 5);
    }
}
