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

typedef struct EndingObjectSpriteDebugViewerActor {
               u8 pad0[0x18];
               s16 x;
               s16 y;
               s16 spriteId;
               u8 enabled;
               u8 palette;
} EndingObjectSpriteDebugViewerActor;

void drawEndingObjectSpriteDebugViewer(EndingObjectSpriteDebugViewerActor *arg0) {
    char sp38[0x10];

    if (arg0->enabled == 1) {
        drawMenuSprite(
            arg0->x,
            arg0->y,
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            (u16)arg0->spriteId,
            0x20,
            0x20,
            arg0->palette,
            0
        );
        sprintf(sp38, "ENDOBJ %2d \n", arg0->spriteId);
        drawMenuAsciiText(0x40, -0x66, sp38, 0, 0x100);
    }
}
