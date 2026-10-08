#include "game/race/race_state.h"
#include "game/engine/game_task_scheduler.h"
#include "common.h"
#include "game/engine/asset_manager.h"
#include "game/engine/system_runtime.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/renderer/sprite_palette_relocation.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/math/fixed_point_math.h"
#include "game/engine/viewport_manager.h"
#include "game/race/camera/race_camera.h"
#include <PR/os_cache.h>

extern u8 *gMenuRenderScratchPtr;
extern u8 *gMenuRenderScratchStart;
extern u32 gMenuRenderScratchUsedSize;

void selectMenuRenderScratchBuffer(s32 arg0) {
    gMenuRenderScratchUsedSize = 0;
    if (arg0 == 0) {
        gMenuRenderScratchPtr = gMenuRenderScratchStart = getRelocatableHeapBlockBase(gAssetHandles[1]);
    } else {
        gMenuRenderScratchPtr = gMenuRenderScratchStart = getRelocatableHeapBlockBase(gAssetHandles[2]);
    }
}

void *allocMenuRenderScratch(s32 size) {
    u8 *oldPtr = gMenuRenderScratchPtr;
    u8 *newPtr = ((0, oldPtr)) + ((((u32)(size + 3)) >> 2) * 4);
    s32 new_var2;

    new_var2 = newPtr - gMenuRenderScratchStart;
    newPtr++;
    newPtr--;

    if (gMenuRenderScratchPtr) {}

    if ((u32)(newPtr - gMenuRenderScratchStart) >= 0x8000) {
        return 0;
    }

    gMenuRenderScratchPtr = newPtr;
    gMenuRenderScratchUsedSize = new_var2;

    return oldPtr;
}

void writebackMenuRenderScratchBuffer(s32 arg0) {
    if (arg0 == 0) {
        osWritebackDCache((void *)getRelocatableHeapBlockBase(gAssetHandles[1]), gMenuRenderScratchUsedSize);
    } else {
        osWritebackDCache((void *)getRelocatableHeapBlockBase(gAssetHandles[2]), gMenuRenderScratchUsedSize);
    }
}
