#include "common.h"
#include <PR/os_libc.h>
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/controller_input.h"
#include "game/ending/ending_credits_ui.h"
#include "game/ending/ending_credits_flow.h"
#include "font_encoding.h"
#include <PR/rmon.h>
#include "game/menu/renderer/menu_renderer.h"

void drawEndingCreditsTheEndText(EndingCreditsPageTextActor *arg0) {
    drawMenuSpriteWithAlpha(
        arg0->x,
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x21]),
        0x35,
        0x20,
        0x20,
        0,
        arg0->alpha,
        0
    );
    drawMenuSpriteWithAlpha(
        (s16)(arg0->x + 0x40),
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x21]),
        0x36,
        0x20,
        0x20,
        0,
        arg0->alpha,
        0
    );
}
