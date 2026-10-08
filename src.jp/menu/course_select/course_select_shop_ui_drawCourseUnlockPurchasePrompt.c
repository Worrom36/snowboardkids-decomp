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

extern MenuGlyphScript gCourseUnlockPurchasePromptText[24];

void drawCourseUnlockPurchasePrompt(ShopMenuWidgetActor *arg0) {
    volatile u64 unused;
    u32 drawAlpha;
    u16 alpha;

    drawMenuSprite(arg0->x, arg0->y, getRelocatableHeapBlockBase(gAssetHandles[0x24]), 0, 0x20, 0x20, 0, 0);
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        1,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x78),
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        1,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0xB0),
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        2,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuGlyphScript((s16)(arg0->x + 0x30), (s16)(arg0->y + 4), gCourseUnlockPurchasePromptText, 0, 0x100, 0);

    if (arg0->item.price == 0) {
        alpha = 0x100;
    } else {
        alpha = 0x60;
    }

    drawAlpha = alpha;
    drawMenuSpriteWithAlpha(
        (s16)(arg0->x + 0x50),
        (s16)(arg0->y + 0x14),
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        0x17,
        0x20,
        0x20,
        0,
        drawAlpha,
        0
    );

    if (drawAlpha == 0x100) {
        alpha = 0x60;
    } else {
        alpha = 0x100;
    }

    drawMenuSpriteWithAlpha(
        (s16)(arg0->x + 0x50),
        (s16)(arg0->y + 0x24),
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        0x18,
        0x20,
        0x20,
        0,
        alpha,
        0
    );
    drawMenuSpriteWithAlpha(
        (s16)(arg0->x + 0x50),
        (s16)(arg0->y + (arg0->item.price * 0x10) + 0x14),
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        0x12,
        0x20,
        0x20,
        0,
        arg0->sprite.index,
        0
    );
}
