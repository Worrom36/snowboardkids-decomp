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
#define stepRaceMotionAnimationUntilEnd(player) stepRaceMotionAnimationUntilEnd(RACE_MOTION_STATE(player))

void updateRacePlayerMode10TerrainCrashSlide(RacePlayer *player) {
    s16 updateState;
    s32 yVel;
    s32 velocityX;
    s32 velocityZ;
    u32 stateFlags;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        player->stateFlags &= ~0x200;
        player->stateFlags &= 0xFE0C1FFB;
        (*player).stateFlags |= 0x01006000;
        if (player->animationId != 0x12) {
            setRaceMotionAnimation(player, 0x12);
        }
        player->stateTimer = 0x1E;
        player->targetGroundOffset = 0;
    }
    stepRaceMotionLoopingAnimation(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y += 0xFFFF6000;
    player->speedLimit = 0x80000;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    stateFlags = player->stateFlags;
    if ((stateFlags & 1) == 0) {
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0, 0, 0);
        player->unk582 = 0x100;
        player->unk584 = 8;
        player->unk588 = 0.0f;
        stateFlags = player->stateFlags;
    }
    if ((stateFlags & 1) != 0) {
        player->stateFlags = stateFlags | 0x200;
    } else {
        player->stateFlags = stateFlags & ~0x200;
    }
    yVel = player->velocity.y;
    velocityX = player->velocity.x;
    velocityZ = player->velocity.z;
    player->pos.y += yVel;
    player->verticalVelocityLimit = 0;
    player->pos.x += velocityX;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->facingAngle = calculateFixedAngleFromDeltaXZ(velocityX, velocityZ);
    if (player->stateFlags & 0x400) {
        player->facingAngle += 0x800;
    }
    if (player->unk330 != 3) {
        if (--player->stateTimer == 0) {
            player->stateFlags &= 0xFE0C1FFB;
            if (player->stateFlags & 1) {
                player->mode = 1;
                player->updateState = 0;
                player->updateTimer = 0;
            } else {
                player->mode = 0;
                player->updateState = 0;
                player->updateTimer = 0;
            }
        }
    } else {
        player->stateTimer = 0x1E;
    }
    player->rumbleTimer = 4;
    player->rumblePatternId = 2;
}

void updateRacePlayerMode12LaunchRecover(RacePlayer *player) {
    s16 updateState;
    u32 stateFlags;
    s32 grounded;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x14);
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags &= ~0x200;
        player->stateFlags |= 0x1022000;
        player->targetGroundOffset = 0;
        player->unk80 = 0;
        /* Rank-scaled recovery timer for humans and CPUs. With zero-based rank,
         * integer truncation gives 60/53/45/38 updates for a base of 60. */
        player->stateTimer = 0x1E - ((player->rankIndex * 0x1E) / 8);
        player->rumbleTimer = 1;
        player->rumblePatternId = 0;
    }

    updateRacePlayerLeanAngle(player, 0, 0);
    player->velocity.y += 0xFFFF6000;
    grounded = player->stateFlags & 1;
    if (grounded == 0) {
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0x6000, 0x6000, 0x6000);
        grounded = player->stateFlags & 1;
    }

    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;

    if (grounded == 0) {
        updateRacePlayerMotionFeedback(player);
        if ((player->unk584 != 6) && (player->unk584 != 0x1E)) {
            player->unk588 = -6.0f;
        }
    }

    stateFlags = player->stateFlags;
    if (!(stateFlags & 1)) {
        player->stateTimer = player->stateTimer - getRaceInputTimerDecrementBonus(player) - 1;
        if (player->stateTimer < 0) {
            player->stateTimer = 0;
        }
        if (stepRaceMotionAnimationUntilEnd(player)) {
            player->stateFlags &= ~0x200;
            if (player->stateTimer == 0) {
                player->mode = 8;
                player->updateState = 0;
                player->updateTimer = 0;
            }
        }
    } else {
        player->stateFlags = stateFlags | 0x200;
        setRaceMotionAnimation(player, 0x14);
        stepRaceMotionAnimationUntilEnd(player);
        player->stateTimer = 0x1E;
        player->unk80 = 7;
        if (!(player->stateFlags & 1)) {
            player->rumbleTimer = 1;
            player->rumblePatternId = 0;
        }
    }
}

void updateRacePlayerMode11LaunchHit(RacePlayer *player) {
    s16 updateState;
    s32 tempX;
    s32 tempZ;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x13);
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x22200;
        player->pitchAngle = 0;
        updateRacePlayerLeanAngle(player, 0, 0);
        player->targetGroundOffset = 0;
        player->rumbleTimer = 3;
        player->rumblePatternId = 0;
    }

    player->speedLimit = 0x60000;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    player->velocity.y -= player->unk264;
    if (!(player->stateFlags & 1)) {
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0x4000, 0x4000, 0x4000);
    } else {
        tempX = player->velocity.x;
        tempZ = player->velocity.z;
        player->velocity.x = tempX - (tempX / 16);
        player->velocity.z = tempZ - (tempZ / 16);
    }

    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;

    if ((stepRaceMotionAnimationUntilEnd(player) != 0) && !(player->stateFlags & 1)) {
        player->mode = 0xC;
        player->updateState = 0;
        player->updateTimer = 0;
    }
}

void updateRacePlayerMode26Tumble(RacePlayer *player) {
    s16 updateState;
    s32 timer;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x12);
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x6204;
        player->pitchAngle = 0;
        updateRacePlayerLeanAngle(player, 0, 0);
        player->targetGroundOffset = 0;
        player->velocity.y = 0x80000;
        player->stateTimer = 0x14;
    }

    if (gFrameCounter & 1) {
        createCallbackTaskWithUserIdPreservingArgs(initRacePlayerRecoverySparkle, 5, 2, (u16)player->playerIndex);
    }

    player->velocity.y -= 0x7000;
    player->pos.y += player->velocity.y;
    stepRaceMotionLoopingAnimation(player);

    timer = player->stateTimer;
    if (timer == 0) {
        if (!(player->stateFlags & 1)) {
            player->mode = 0xC;
            player->updateState = 0;
            player->updateTimer = 0;
        }
    } else {
        player->stateTimer = timer - 1;
    }

    if (player->velocity.y > 0) {
        player->rumbleTimer = 2;
        player->rumblePatternId = 3;
    }
}

void updateRacePlayerMode14PushHit(RacePlayer *player) {
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
        player->stateTimer = 0xF;
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

void updateRacePlayerMode23ItemSteal(RacePlayer *player) {
    s16 updateState;
    s16 nextState;
    s32 timer;
    u32 stateFlags;

    updateState = player->updateState;
    nextState = updateState + 1;
    if (updateState == 0) {
        player->updateState = nextState;
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x102000;
        setRaceMotionAnimation(player, 0xE);
        player->stateTimer = 0x5A;
        player->targetGroundOffset = 0;
        stepRaceMotionLoopingAnimation(player);
        player->velocity.y = 0;
        createCallbackTaskWithUserIdPreservingArgs(initRaceUiItemStealTrailEffect, 0, 0x64, (u16)player->playerIndex);
        timer = player->stateTimer;
        /* Rank-scaled recovery timer for humans and CPUs. With zero-based rank,
         * integer truncation gives 60/53/45/38 updates for a base of 60. */
        player->stateTimer = timer - ((player->stateTimer * player->rankIndex) / 8);
        player->rumbleTimer = 4;
        player->rumblePatternId = 0;
    }

    stateFlags = player->stateFlags;
    if (!(stateFlags & 1)) {
        player->stateFlags = stateFlags & ~0x200;
    } else {
        player->stateFlags = stateFlags | 0x200;
    }

    updateRacePlayerLeanAngle(player, 0, 0);
    player->velocity.y -= 0xA000;
    player->pos.y += player->velocity.y;

    player->stateTimer -= getRaceInputTimerDecrementBonus(player) + 1;
    timer = player->stateTimer;
    if (timer < 0) {
        player->stateTimer = 0;
        timer = 0;
    }

    if (timer == 0) {
        player->mode = 4;
        player->updateState = 0;
        player->updateTimer = 0;
        player->unk2E0 = 0;
        player->unk2E4 = 0;
    }
}

void updateRacePlayerMode24HeavyKnockdown(RacePlayer *player) {
    s16 temp_v1_2;
    s16 temp_v0;
    s16 temp_2d6;
    s32 timer;
    s32 ret;
    u32 stateFlags;

    player->leanAngle = 0;
    updateRacePlayerLeanAngle(player, 0, 0);
    player->velocity.y += 0xFFFF6000;
    player->pos.y += player->velocity.y;

    stateFlags = player->stateFlags;
    if (!(stateFlags & 1)) {
        player->stateFlags = stateFlags & ~0x200;
    } else {
        player->stateFlags = stateFlags | 0x200;
    }

    temp_v1_2 = player->updateState;
    temp_v0 = temp_v1_2;
    switch (temp_v0 ^ 0) {
        case 0:
            player->updateState = temp_v1_2 + 1;
            player->stateFlags &= 0xFE0C1FFB;
            player->stateFlags |= 0x1202000;
            player->unk2D6 = 1;
            setRaceMotionAnimation(player, 1);
            player->stateTimer = 0x5A;
            player->targetGroundOffset = 0;
            stepRaceMotionLoopingAnimation(player);
            timer = player->stateTimer;
            /* Rank-scaled recovery timer for humans and CPUs. With zero-based rank,
             * integer truncation gives 60/53/45/38 updates for a base of 60. */
            player->stateTimer = (timer ^ 0) - ((timer * player->rankIndex) / 8);
            player->velocity.y = 0;
            player->rumbleTimer = 6;
            player->rumblePatternId = 0;
            /* fallthrough */
        case 1:
            temp_2d6 = player->unk2D6;
            if (temp_2d6 != 1) {
                player->unk2D6 = temp_2d6 - 1;
            }

            ret = getRaceInputTimerDecrementBonus(player);
            if (ret != 0) {
                player->unk2D6 = 3;
            }

            player->stateTimer -= ret + 1;
            timer = player->stateTimer;
            if (timer < 0) {
                player->stateTimer = 0;
                timer = 0;
            }

            if (timer == 0) {
                player->updateState += 1;
                player->unk2D6 = 1;
            }
            break;
        case 2:
            player->unk2D6 += 1;
            if (player->unk2D6 == 7) {
                player->unk2D6 = 2;
                player->updateState += 1;
            }
            break;
        case 3:
            player->unk2D6 += 1;
            if (player->unk2D6 == 8) {
                player->stateFlags &= 0xFE0C1FFB;
                player->mode = 0;
                player->updateState = 0;
                player->updateTimer = 0;
                player->unk2D4 = 0x3C;
            }
            break;
    }
}

void updateRacePlayerMode25SpinHit(RacePlayer *player) {
    s16 updateState;
    s32 stateTimer;
    u32 stateFlags;
    s16 nextState;

    updateState = player->updateState;
    nextState = updateState + 1;
    if (updateState == 0) {
        player->updateState = nextState;
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x400000;
        player->stateTimer = 0xF0;
        setRaceMotionAnimation(player, 1);
        stepRaceMotionLoopingAnimation(player);
        createCallbackTaskWithUserIdPreservingArgs(
            initRaceUiHeavyKnockdownTrailEffect,
            0,
            0x3C,
            (u16)player->playerIndex
        );
        stateTimer = player->stateTimer;
        player->stateTimer = stateTimer;
        /* Rank-scaled recovery timer for humans and CPUs. With zero-based rank,
         * integer truncation gives 60/53/45/38 updates for a base of 60. */
        player->stateTimer = player->stateTimer - ((stateTimer * player->rankIndex) / 8);
        player->rumbleTimer = 4;
        player->rumblePatternId = 0;
    }

    stateFlags = player->stateFlags;
    if (!(stateFlags & 1)) {
        player->stateFlags = stateFlags & ~0x200;
        updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
        player->velocity.y -= player->gravity;
        updateRacePlayerLocalVelocity(player, 0, 0, player->unk274, player->unk278, player->unk27C);
    } else {
        player->stateFlags = stateFlags | 0x200;
        player->velocity.y -= player->unk264;
        clampRacePlayerVectorXZSpeed(&player->velocity, player);
    }

    player->stateTimer -= getRaceInputTimerDecrementBonus(player) + 1;
    if (player->stateTimer < 0) {
        player->stateTimer = 0;
    }

    if (player->stateTimer == 0) {
        stateFlags = player->stateFlags;
        if (!(stateFlags & 0x200)) {
            player->stateFlags = stateFlags & 0xFE0C1FFB;
            player->mode = 0;
            player->updateState = 0;
            player->updateTimer = 0;
        } else {
            player->stateFlags = stateFlags & 0xFE0C1FFB;
            player->mode = 1;
            player->updateState = 0;
            player->updateTimer = 0;
        }
    }

    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;

    if (!(player->stateFlags & 1)) {
        updateRacePlayerMotionFeedback(player);
    }
}
