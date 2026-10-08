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

extern const char gShopMenuMoneyFormat[];

void drawShopMenuMoneyPanel(ShopMenuWidgetActor *arg0) {
    char sp40[0x18];

    drawMenuPanelBackdrop(arg0->x, arg0->y, 0x5000, 0x4000);
    drawMenuSpriteWithAlpha(
        (s16)(arg0->x + 8),
        (s16)(arg0->y + 4),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        0x11,
        0x20,
        0x20,
        0,
        arg0->sprite.index,
        0
    );
    sprintf(sp40, gShopMenuMoneyFormat, gRacePlayers[0].money);
    drawMenuAsciiText((s16)(arg0->x + 0x10), (s16)(arg0->y + 0x10), sp40, 0, arg0->sprite.index);
}
