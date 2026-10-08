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

void sortRaceUiResultsBannerPlayers(RaceUiResultsBannerActor *arg0) {
    RaceUiResultsBannerActor *actor;
    s32 i;
    s32 j;
    s32 next;
    s32 temp;

    actor = arg0;
    i = 0;
    do {
        actor->playerIndices[i] = i;
        i++;
    } while (i < 4);

    i = 0; do { next = i + 1; j = next;
        if (next < 4) {
            do {
                if (gRacePlayers[actor->playerIndices[j]].rankIndex < gRacePlayers[actor->playerIndices[i]].rankIndex) {
                    temp = actor->playerIndices[i];
                    actor->playerIndices[i] = actor->playerIndices[j];
                    actor->playerIndices[j] = temp;
                }
                j++;
            } while (j < 4);
        }
        next = i + 1;
        i = next;
    } while (next < 3);

    i = 0;
    do {
        if (!(gRacePlayers[actor->playerIndices[i]].stateFlags & 0x40)) {
            actor->playerIndices[i] = -1;
        }
        i++;
    } while (i < 4);
}
