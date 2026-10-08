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

extern s16 gShopMenuSparkleInitTable[16];

void initShopMenuSparkles(ShopMenuWidgetActor *arg0) {
    s16 *entry;

    arg0->sparkle.patternIndex = gRacePlayers[0].selectedCharacterId;
    entry = &gShopMenuSparkleInitTable[arg0->sparkle.patternIndex * 3];
    arg0->sprite.index = entry[0];
    arg0->x = 0x94;
    arg0->y = entry[1];
    arg0->sparkle.tileBase = entry[2];
    arg0->sparkle.alpha = 0x100;
    arg0->slide.slideState = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuSparkles);
}
