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

void runRenderCallbacks(RenderCallbackNode **arg0) {
    RenderCallbackNode *s0 = *arg0;
    if (s0 != NULL) {
    loop:
        if ((u32)(((u8 *)gRegionAllocPtr - (u8 *)gCurrentFrameRenderData) - sizeof(FrameRenderData)) < 0x14181U) {
            s0->callback(s0->arg);
            s0 = s0->next;
            if (s0 != NULL) {
                goto loop;
            }
        }
    }
}

void allocMenuRenderScratchBuffers(void) {
    gAssetHandles[1] = allocRelocatableHeapBlock(0x8000);
    gAssetHandles[2] = allocRelocatableHeapBlock(0x8000);
}
