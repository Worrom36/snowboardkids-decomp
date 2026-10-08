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

void initShopMenuModeChoiceRows(ShopMenuRowActor *arg0) {
    s32 i;

    for (i = 0; i < 5; i++) { arg0->rowXPositions[i] = -0x104; }

    arg0->baseY = -0x50;
    arg0->revealTimer = 0;
    arg0->visibleRowCount = 1;
    arg0->state = 0;

    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuModeChoiceRows);
}
