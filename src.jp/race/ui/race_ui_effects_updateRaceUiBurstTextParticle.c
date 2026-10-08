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

extern u8 gRaceUpdatePaused;
extern Vec3i D_800D61C0[];

void updateRaceUiBurstTextParticle(RaceUiTextParticleActor *arg0) {
    s32 velocityY;

    if (gRaceUpdatePaused == 0) {
        velocityY = arg0->velocity.y;
        arg0->pos.x += arg0->velocity.x;
        arg0->pos.y += velocityY;
        arg0->pos.z += arg0->velocity.z;
        arg0->velocity.y = velocityY - 0x8000;
        arg0->timer++;
        if (arg0->timer >= 0x10) {
            removeCallbackTask(arg0);
            return;
        }
    }
    if (arg0->useAltTextures == 0) {
        addRenderCallback(&D_801248EC, (RenderCallback)renderRaceUiBurstTextParticle, arg0);
        return;
    }
    addRenderCallback(&gSceneModelRenderCallbackList, (RenderCallback)renderRaceUiAltBurstTextParticle, arg0);
}

void initRaceUiBurstTextParticle(RaceUiTextParticleActor *arg0) {
    arg0->timer = 0;
    arg0->velocity = D_800D61C0[arg0->task.userId];
    if (arg0->useAltTextures == 0) {
        getAssetTableImageAndPalette(
            getRelocatableHeapBlockBase(gAssetHandles[0x1C]),
            0x27,
            &arg0->images[0],
            &arg0->palettes[0]
        );
        getAssetTableImageAndPalette(
            getRelocatableHeapBlockBase(gAssetHandles[0x1C]),
            0x28,
            &arg0->images[1],
            &arg0->palettes[1]
        );
        getAssetTableImageAndPalette(
            getRelocatableHeapBlockBase(gAssetHandles[0x1C]),
            0x29,
            &arg0->images[2],
            &arg0->palettes[2]
        );
        getAssetTableImageAndPalette(
            getRelocatableHeapBlockBase(gAssetHandles[0x1C]),
            0x2A,
            &arg0->images[3],
            &arg0->palettes[3]
        );
    } else {
        getAssetTableImageAndPalette(
            getRelocatableHeapBlockBase(gAssetHandles[0x1C]),
            0x2B,
            &arg0->images[0],
            &arg0->palettes[0]
        );
        getAssetTableImageAndPalette(
            getRelocatableHeapBlockBase(gAssetHandles[0x1C]),
            0x2C,
            &arg0->images[1],
            &arg0->palettes[1]
        );
        getAssetTableImageAndPalette(
            getRelocatableHeapBlockBase(gAssetHandles[0x1C]),
            0x2D,
            &arg0->images[2],
            &arg0->palettes[2]
        );
        getAssetTableImageAndPalette(
            getRelocatableHeapBlockBase(gAssetHandles[0x1C]),
            0x2E,
            &arg0->images[3],
            &arg0->palettes[3]
        );
    }
    updateRaceUiBurstTextParticle(arg0);
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiBurstTextParticle);
}
