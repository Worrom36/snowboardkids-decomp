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

#define ASSET_HANDLE(index) (gAssetHandles[(index)])

void initRaceStartOverlay(RaceUiOverlayActor *arg0) {
    s32 i;

    arg0->pos.x = 0;
    arg0->pos.z = gRacePlayers[0].pos.z + 0xFF900000;
    arg0->velocity = 0;
    arg0->pos.y = getRaceCourseSurfaceHeight(
                      findRaceCourseSurfaceFromHint(0, 0, arg0->pos.z), arg0->pos.x, arg0->pos.z
                  ) +
                  0x200000;

    for (i = 0; i < 0x40; i++) {
        arg0->velocity += 0x2000;
        arg0->pos.y += arg0->velocity;
        arg0->timer++;
    }

    getAssetTableImageAndPalette(
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x1F)),
        0x3A,
        &arg0->panelAImage,
        &arg0->panelAPalette
    );
    getAssetTableImageAndPalette(
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x1F)),
        0x3B,
        &arg0->panelBImage,
        &arg0->panelBPalette
    );
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceStartOverlayRiseIn);
}
