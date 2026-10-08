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

void updateRacePlayerMode32Character0(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x23);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->animationId == 0x24) {
            player->stateFlags |= 0x8000000;
        }
        if (player->stateTimer == 0) {
            if (player->updateTimer == 1) {
                enqueueRacePlayerVoiceSound(player, 6);
                player->updateTimer++;
            }
            player->stateTimer = 0x14;
            setRaceMotionAnimation(player, 0x24);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode32Character1(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x24);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->animationId == 0x25) {
            player->stateFlags |= 0x8000000;
        }
        if (player->stateTimer == 0) {
            if (player->updateTimer == 1) {
                enqueueRacePlayerVoiceSound(player, 6);
                player->updateTimer++;
            }
            setRaceMotionAnimation(player, 0x25);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode32Character2(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x23);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->animationId == 0x24) {
            player->stateFlags |= 0x8000000;
        }
        if (player->stateTimer == 0) {
            if (player->updateTimer == 1) {
                enqueueRacePlayerVoiceSound(player, 6);
                player->updateTimer++;
            }
            player->stateTimer = 0x14;
            setRaceMotionAnimation(player, 0x24);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode32Character3(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x24);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->animationId == 0x25) {
            player->stateFlags |= 0x8000000;
        }
        if (player->stateTimer == 0) {
            if (player->updateTimer == 1) {
                enqueueRacePlayerVoiceSound(player, 6);
                player->updateTimer++;
            }
            player->stateTimer = 0;
            setRaceMotionAnimation(player, 0x25);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode32Character4(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x24);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->animationId == 0x25) {
            player->stateFlags |= 0x8000000;
        }
        if (player->stateTimer == 0) {
            if (player->updateTimer == 1) {
                enqueueRacePlayerVoiceSound(player, 6);
                player->updateTimer++;
            }
            player->stateTimer = 0x32;
            setRaceMotionAnimation(player, 0x25);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode32Character5(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        enqueueRacePlayerVoiceSound(player, 6);
        player->stateTimer = 0x1E;
        setRaceMotionAnimation(player, 0x23);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        player->stateFlags |= 0x8000000;
        if (player->stateTimer == 0) {
            player->stateTimer = 0x1E;
            setRaceMotionAnimation(player, 0x23);
        } else {
            player->stateTimer--;
        }
    }
}
