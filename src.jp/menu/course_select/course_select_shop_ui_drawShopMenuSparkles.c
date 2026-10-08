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

typedef Vec2s ShopMenuSparklePattern[13];

extern ShopMenuSparklePattern gShopMenuSparklePatterns[5];

void drawShopMenuSparkles(ShopMenuWidgetActor *arg0) {
    s32 patternOffset;
    s32 tileIndex;
    ShopMenuWidgetActor *drawActor;
    ShopMenuWidgetActor *actor;

    actor = arg0;
    tileIndex = 0;
    if (gShopMenuSparklePatterns[actor->sparkle.patternIndex][0].x != -1) {
        patternOffset = 0;
        do {
            drawActor = actor;
            drawMenuSpriteTile(
                (s16)(drawActor->randomValues[0] +
                      gShopMenuSparklePatterns[drawActor->sparkle.patternIndex][patternOffset].x),
                (s16)(drawActor->randomValues[1] +
                      gShopMenuSparklePatterns[drawActor->sparkle.patternIndex][patternOffset].y),
                getRelocatableHeapBlockBase(gAssetHandles[0x27]),
                tileIndex + drawActor->sparkle.tileBase,
                0,
                drawActor->sparkle.alpha
            );
            patternOffset++;
            tileIndex++;
        } while (gShopMenuSparklePatterns[actor->sparkle.patternIndex][patternOffset].x != -1);
    }
}
