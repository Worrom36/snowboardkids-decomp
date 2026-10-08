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

extern CLANG_DIAGNOSTIC_POP extern u8 *gRenderCallbackScratchPtr;
extern void *allocRenderCallbackScratch(s32 arg0);

void resetRenderScratchAllocator(void) {
    gRenderCallbackScratchPtr = getRelocatableHeapBlockBase(gAssetHandles[0]);
}

void *allocRenderCallbackScratch(s32 arg0) {
    s32 *new_var2;
    u32 new_var;
    s32 sp1C;
    s32 base;
    s32 temp_a0;

    sp1C = (s32)gRenderCallbackScratchPtr;
    base = (s32)getRelocatableHeapBlockBase(gAssetHandles[0]);
    new_var2 = &temp_a0;
    temp_a0 = (s32)gRenderCallbackScratchPtr + ((((u32)(arg0 + 3)) >> 2) * 4);
    new_var = (u32)(*new_var2 - base);
    if (new_var >= 0x4001U) {
        return NULL;
    }
    gRenderCallbackScratchPtr = (u8 *)*new_var2;
    return (void *)sp1C;
}

void addRenderCallback(RenderCallbackNode **arg0, RenderCallback arg1, void *arg2) {
    RenderCallbackNode *temp_v0 = allocRenderCallbackScratch(sizeof(RenderCallbackNode));

    if (temp_v0 != NULL) {
        temp_v0->next = *arg0;
        temp_v0->callback = arg1;
        temp_v0->arg = arg2;
        *arg0 = temp_v0;
    }
}
