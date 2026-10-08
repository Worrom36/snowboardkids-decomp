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
#define RACE_MOTION_STATE(player) ((RaceMotionState *)(player))
#define stepRaceMotionAnimationUntilEnd(player) stepRaceMotionAnimationUntilEnd(RACE_MOTION_STATE(player))

void updateRacePlayerTrickSubstateStart(RacePlayer *player) {
    s16 inputMask;

    player->subStateTimer = 0;
    player->subStateStep = 0;
    player->unk2A2 = 0;
    player->unk2A4 = 0;
    if (player->isCpu == 0) {
        if (player->currentInputFlags & 1) {
            player->subStateTimer = 1;
            player->unk2A2 = 1;
        }
        if (player->currentInputFlags & 2) {
            player->subStateTimer = 2;
            player->unk2A2 = 3;
        }
        if (player->currentInputFlags & 8) {
            player->subStateTimer = 8;
            player->unk2A2 = 7;
        }
        if (player->currentInputFlags & 4) {
            player->subStateTimer = 4;
            player->unk2A2 = 5;
        }
        if (player->subStateTimer != 0) {
            if (player->unk2A0 < 6) {
                player->unk2A2++;
            }
            player->subState++;
            if (player->stateFlags & 0x400) {
                inputMask = player->subStateTimer;
                if (inputMask & 1) {
                    setRaceMotionAnimation(player, 0x17);
                    inputMask = player->subStateTimer;
                }
                if (inputMask & 2) {
                    setRaceMotionAnimation(player, 0x15);
                    inputMask = player->subStateTimer;
                }
            } else {
                inputMask = player->subStateTimer;
                if (inputMask & 1) {
                    setRaceMotionAnimation(player, 0x15);
                    inputMask = player->subStateTimer;
                }
                if (inputMask & 2) {
                    setRaceMotionAnimation(player, 0x17);
                    inputMask = player->subStateTimer;
                }
            }
            if (inputMask & 8) {
                setRaceMotionAnimation(player, 0x19);
                inputMask = player->subStateTimer;
            }
            if (inputMask & 4) {
                setRaceMotionAnimation(player, 0x1B);
            }
            player->stateFlags |= 0x800;
        }
        player->unk2A0++;
    }
}

void updateRacePlayerTrickSubstateHold(RacePlayer *player) {
    s16 inputMask;

    if (!(player->inputFlags & player->subStateTimer)) {
        player->subStateStep = 1;
    }
    if (stepRaceMotionAnimationUntilEnd(player) != 0) {
        if (player->subStateStep != 0) {
            player->subState++;
            if (player->stateFlags & 0x400) {
                inputMask = player->subStateTimer;
                if (inputMask & 1) {
                    setRaceMotionAnimation(player, 0x18);
                    inputMask = player->subStateTimer;
                }
                if (inputMask & 2) {
                    setRaceMotionAnimation(player, 0x16);
                    inputMask = player->subStateTimer;
                }
            } else {
                inputMask = player->subStateTimer;
                if (inputMask & 1) {
                    setRaceMotionAnimation(player, 0x16);
                    inputMask = player->subStateTimer;
                }
                if (inputMask & 2) {
                    setRaceMotionAnimation(player, 0x18);
                    inputMask = player->subStateTimer;
                }
            }
            if (inputMask & 8) {
                setRaceMotionAnimation(player, 0x1A);
                inputMask = player->subStateTimer;
            }
            if (inputMask & 4) {
                setRaceMotionAnimation(player, 0x1C);
            }
        } else {
            player->unk2A4++;
        }
    }
    player->stateFlags |= 0x800;
}

void updateRacePlayerTrickSubstateFinish(RacePlayer *player) {
    if (stepRaceMotionAnimationUntilEnd(player) != 0) {
        player->subState++;
    }
    player->stateFlags |= 0x800;
}
