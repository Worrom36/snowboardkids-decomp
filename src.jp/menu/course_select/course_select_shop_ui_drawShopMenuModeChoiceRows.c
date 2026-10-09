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

extern u8 gMenuExitSelection;

void drawShopMenuModeChoiceRows(ShopMenuRowActor *actor) {
    ShopMenuRowActor *savedActor;
    ShopMenuRowActor *rows;
    s32 rowIndex;
    s32 blinkAlpha;
    s32 yOffset;

    savedActor = actor;
    rowIndex = 0;
    if (actor->visibleRowCount > 0) {
        /* Keeping the loop setup together preserves IDO's original scheduling. */
        // clang-format off
        yOffset = 0; rows = actor; do {
        // clang-format on
            blinkAlpha = 0;
            if ((gMenuSelectionConfirmTimer > 0) && (gMenuSelectionConfirmTimer < 8) &&
                (gMenuExitSelection == 0) && (rowIndex == gCourseSelectModeSelection) &&
                (gMenuSelectionConfirmTimer & 1)) {
                blinkAlpha = 0xFF;
            }
            drawMenuSprite(rows->rowXPositions[rowIndex], (s16)(actor->baseY + yOffset),
                           getRelocatableHeapBlockBase(gAssetHandles[0x27]),
                           (u16)(rowIndex + 2), 0x20, 0x20, 0, blinkAlpha);
            yOffset += 0x1C;
            rowIndex += 1;
        } while (rowIndex < savedActor->visibleRowCount);
    }
}
