#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/course_select/course_select_shop_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/player/race_player_input.h"

extern Vec2s gCoursePreviewCloseSparkleOffsetsEnd[];
extern Vec2s gCoursePreviewCloseSparkleMirrorStart[2];
extern Vec2s gCoursePreviewCloseSparkleOffsets[6];
extern s16 gCoursePreviewViewportHeight;

void drawCoursePreviewCloseSparkles(ShopMenuWidgetActor *arg0) {
    Vec2s *offset;
    ShopMenuWidgetActor *actor;
    ShopMenuWidgetActor *counterActor;
    void *texture;

    // clang-format off
    actor = arg0; offset = gCoursePreviewCloseSparkleOffsets; counterActor = arg0; do {
    // clang-format on
        if (offset < gCoursePreviewCloseSparkleMirrorStart) {
            texture = getRelocatableHeapBlockBase(gAssetHandles[0x1C]);
            drawMenuSprite((s16)(offset->x + actor->x), (s16)(offset->y + actor->sprite.index - 8),
                          texture, counterActor->transition.counter + 0x39,
                          0x20, 0x20, 0, 0);
        } else {
            texture = getRelocatableHeapBlockBase(gAssetHandles[0x1C]);
            drawMenuSprite((s16)(offset[-6].x + actor->y), (s16)((offset[-6].y + actor->sprite.index) * -1 - 8),
                          texture, counterActor->transition.counter + 0x39,
                          0x20, 0x20, 0, 0);
        }
        offset++;
        counterActor = (ShopMenuWidgetActor *)&counterActor->pad0[2];
    } while (offset != gCoursePreviewCloseSparkleOffsetsEnd);
}

void updateCoursePreviewCloseSparkles(ShopMenuWidgetActor *arg0) {
    s32 i;

    arg0->sprite.index = gCoursePreviewViewportHeight / 2;
    arg0->x++;
    arg0->y--;

    for (i = 0; i < 12; i += 4) {
        arg0->sparkle.counters[i] = (arg0->sparkle.counters[i] + 1) % 6;
        arg0->sparkle.counters[i + 1] = (arg0->sparkle.counters[i + 1] + 1) % 6;
        arg0->sparkle.counters[i + 2] = (arg0->sparkle.counters[i + 2] + 1) % 6;
        arg0->sparkle.counters[i + 3] = (arg0->sparkle.counters[i + 3] + 1) % 6;
    }

    if (arg0->sprite.index == 0) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCoursePreviewCloseSparkles, arg0);
}

void initCoursePreviewCloseSparkles(ShopMenuWidgetActor *arg0) {
    ShopMenuWidgetActor *new_var;
    s32 i;

    arg0->x = 5;
    arg0->y = 0x48;
    new_var = arg0;
    new_var->sprite.index = gCoursePreviewViewportHeight / 2;

    for (i = 0; i < 12; i++) {
        new_var->randomValues[i + 3] = randomNextMain() % 6;
    }

    setCallbackTaskCallback(new_var, (CallbackTaskCallback)updateCoursePreviewCloseSparkles);
}
