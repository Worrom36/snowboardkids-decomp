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
#define blendRaceMotionJointAnimation(player, animationIndex, blendTimer, blendDuration) \
    blendRaceMotionJointAnimation(                                                        \
        RACE_MOTION_STATE(player), (animationIndex), (blendTimer), (blendDuration)       \
    )
#define stepRaceMotionLoopingJointAnimation(player) stepRaceMotionLoopingJointAnimation(RACE_MOTION_STATE(player))
#define RACE_MOTION_STATE(player) ((RaceMotionState *)(player))

void updateRacePlayerMode30AttackApproach(RacePlayer *player) {
    s16 updateTimer;
    s16 facingAngle;
    s16 angleDiff;
    s16 roll;

    updateTimer = player->updateTimer;
    if (updateTimer == 0) {
        player->updateTimer = updateTimer + 1;
        player->subState = 0;
        setRaceCameraMode(player->playerIndex, 5);
        if (player->movementSpeed >= 0x20001) {
            player->unk306 = 1;
            player->unk31E = calculateFixedAngleFromDeltaXZ(player->velocity.x, player->velocity.z);
            if (player->stateFlags & 0x400) {
                player->unk31E += 0x800;
            }

            if ((s16)(angleDiff = 0x800 & (player->unk31E - player->facingAngle)) < 0x800) {
                player->unk31C = 0;
                if (player->stateFlags & 0x400) {
                    player->unk31C = 1;
                }
                player->unk31E -= 0x400;
            } else {
                player->unk31C = 1;
                if (player->stateFlags & 0x400) {
                    player->unk31C = 0;
                }
                player->unk31E += 0x400;
            }
            player->stateTimer = 0;
        } else {
            player->unk306 = 0;
            setRaceMotionAnimation(player, 1);
        }
    }

    if (player->unk306 != 0) {
        facingAngle = player->facingAngle;
        angleDiff = (player->unk31E - facingAngle) & 0xFFF;
        if (angleDiff >= 0x801) {
            angleDiff -= 0x1000;
        }
        if (angleDiff >= 0x41) {
            angleDiff = 0x40;
        }
        if (angleDiff < -0x40) {
            angleDiff = -0x40;
        }
        player->facingAngle = facingAngle + angleDiff;

        if (player->unk31C != 0) {
            player->stateTimer = 0x1F;
        } else {
            player->stateTimer = -0x1F;
        }

        player->leanScale = 0x3F;
        angleDiff = updateRacePlayerLeanAngle(player, player->movementSpeed, player->stateTimer) - player->unk2FA;
        roll = player->unk2FA;
        if (angleDiff >= 0x31) {
            angleDiff = 0x30;
        }
        if (angleDiff < -0x30) {
            angleDiff = -0x30;
        }
        player->unk2FA = player->unk2FA + angleDiff;

        if (player->unk2FA == 0) {
            if (player->animationId != 1) {
                setRaceMotionAnimation(player, 1);
            }
            stepRaceMotionLoopingJointAnimation(player);
        } else if (player->unk2FA >= 0) {
            setRaceMotionAnimation(player, 3);
            blendRaceMotionJointAnimation(player, 3, player->unk2FA, 0x118);
        } else {
            setRaceMotionAnimation(player, 2);
            blendRaceMotionJointAnimation(player, 2, -player->unk2FA, 0x118);
        }
    } else {
        updateRacePlayerLeanAngle(player, 0, 0);
        stepRaceMotionLoopingJointAnimation(player);
    }

    player->velocity.y -= player->gravity;
    updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0x6000, 0x6000, 0x6000);
    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;
    updateRacePlayerMotionFeedback(player);

    if ((player->velocity.x == 0) && (player->velocity.z == 0) && !(player->stateFlags & 1)) {
        if (player->soundDisabled == 0) {
            player->updateState++;
            player->stateTimer = 0x1E;
            if (player->animationId != 1) {
                setRaceMotionAnimation(player, 1);
            }
        }
    }
}

void updateRacePlayerMode30AttackResolve(RacePlayer *player) {
    if (gRaceResultState != 0) {
        player->stateTimer = 0;
    }
    if (player->stateTimer == 0) {
        if (gRaceSplitscreenMode == 0) {
            if (player->rankIndex == 0) {
                player->updateState = 2;
                player->updateTimer = 0;
            } else {
                player->updateState = 3;
                player->updateTimer = 0;
            }
        } else if (gRaceResultState == 1) {
            player->updateState = 2;
            player->updateTimer = 0;
        } else {
            player->updateState = 3;
            player->updateTimer = 0;
        }
    }
    updateRacePlayerLeanAngle(player, 0, 0);
    stepRaceMotionLoopingJointAnimation(player);
}
