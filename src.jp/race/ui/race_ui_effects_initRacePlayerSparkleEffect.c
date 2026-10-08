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

void initRacePlayerSparkleEffect(RaceUiSparkleActor *arg0) {
    arg0->zOffset = 0x20000;
    arg0->alpha = 0;
    arg0->alphaStep = 0x10;
    arg0->frame = 0;
    arg0->scale = 0x1000;
    arg0->unk28 = 0;

    getAssetTableImageAndPalette(
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x1C)),
        0x23,
        &arg0->images[0],
        &arg0->palettes[0]
    );
    getAssetTableImageAndPalette(
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x1C)),
        0x24,
        &arg0->images[1],
        &arg0->palettes[1]
    );
    getAssetTableImageAndPalette(
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x1C)),
        0x25,
        &arg0->images[2],
        &arg0->palettes[2]
    );
    getAssetTableImageAndPalette(
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x1C)),
        0x26,
        &arg0->images[3],
        &arg0->palettes[3]
    );
    enqueuePositionalSoundEffect(0x10, &gRacePlayers[arg0->playerIndex].pos, 0x7F, 0x32);
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRacePlayerSparkleRise);
}
