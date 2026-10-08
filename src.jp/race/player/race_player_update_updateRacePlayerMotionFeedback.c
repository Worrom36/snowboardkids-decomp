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

void updateRacePlayerMotionFeedback(RacePlayer *player) {
    s16 angleDiff;
    s32 itemType;
    s32 deltaX;
    s32 deltaZ;

    deltaX = player->pos.x - player->previousPosition.x;
    deltaZ = player->pos.z - player->previousPosition.z;
    angleDiff = (calculateFixedAngleFromDeltaXZ(deltaX, deltaZ) - player->facingAngle) & 0xFFF;
    if (angleDiff >= 0x801) {
        angleDiff = 0x1000 - angleDiff;
    }
    if (angleDiff >= 0x401) {
        angleDiff = 0x800 - angleDiff;
    }

    player->unk588 = (f32)((f64)(f32)-angleDiff / 171.0);
    player->unk582 = player->movementSpeed >> 0xC;
    if (player->unk582 >= 0x80) {
        player->unk582 = 0x7F;
    }

    itemType = getRaceItemEffectType(player->unk330);
    player->unk584 = 3;
    if (itemType == 1) {
        player->unk584 = 0x1E;
        player->unk582 = 0x7F;
        player->unk588 = 0.0f;
    }
    if (itemType == 2) {
        player->unk584 = 4;
    }
    if (3 == itemType) {
        player->unk584 = 5;
    }
    if (4 == itemType) {
        player->unk584 = 4;
        if (player->unk330 == 0xD) {
            player->unk584 = 6;
            if (player->unk582 >= 0x51) {
                player->rumbleTimer = 2;
                player->rumblePatternId = 1;
            }
        }
        player->unk588 = 0.0f;
    }
    if (player->stateFlags & 0x2000000) {
        player->unk584 = 4;
    }
}
