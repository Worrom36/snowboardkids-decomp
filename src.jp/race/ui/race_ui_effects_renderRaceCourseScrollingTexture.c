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

extern Vec3i D_800D6030[];

void renderRaceCourseScrollingTexture(RaceCourseScrollingTextureActor *arg0) {
    volatile u8 pad[8];
    Gfx *gfx;

    gDPPipeSync(gRegionAllocPtr++);
    gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
    gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0x9)));
    gSPMatrix(gRegionAllocPtr++, &gIdentityMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gDPLoadTextureBlock_4b(
        gRegionAllocPtr++,
        arg0->texture,
        G_IM_FMT_CI,
        0x20,
        0x40,
        0,
        G_TX_WRAP,
        G_TX_WRAP,
        5,
        6,
        G_TX_NOLOD,
        G_TX_NOLOD
    );

    gfx = gRegionAllocPtr++;
    gfx->words.w0 = ((arg0->textureScrollOffset * 4) & 0xFFF) | 0xF2000000;
    gfx->words.w1 = ((arg0->textureScrollOffset + 0x40) << 2) & 0xFFF;

    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->palette);
    gSPDisplayList(gRegionAllocPtr++, arg0->displayList);
}

void updateRaceCourseScrollingTexture(RaceCourseScrollingTextureActor *arg0) {
    s32 temp_v0;

    temp_v0 = arg0->soundSourceIndex;
    arg0->textureScrollOffset = arg0->textureScrollOffset + 2;
    arg0->textureScrollOffset = arg0->textureScrollOffset & 0x3F;
    if (temp_v0 != 0) {
        enqueuePlayerLoopingPositionalSoundRequest(0xE, &D_800D6030[temp_v0], 0x7F, 0x32, 0.0f, 0xA);
    }
    if (arg0->useOverlayQueue != 0) {
        addRenderCallback(&D_801248EC, (RenderCallback)renderRaceCourseScrollingTexture, arg0);
        return;
    }
    addRenderCallback(&gRaceObjectRenderCallbackList, (RenderCallback)renderRaceCourseScrollingTexture, arg0);
}
