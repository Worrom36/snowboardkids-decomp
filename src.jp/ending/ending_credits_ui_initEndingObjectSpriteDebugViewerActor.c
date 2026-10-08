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

extern void updateEndingObjectSpriteDebugViewer(EndingObjectSpriteDebugViewerActor *arg0);

void initEndingObjectSpriteDebugViewerActor(EndingObjectSpriteDebugViewerActor *arg0) {
    arg0->x = 0;
    arg0->y = 0;
    arg0->spriteId = 0;
    arg0->enabled = 0;
    arg0->palette = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateEndingObjectSpriteDebugViewer);
}
