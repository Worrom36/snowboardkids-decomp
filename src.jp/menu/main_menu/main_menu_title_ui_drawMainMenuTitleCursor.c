#include "common.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/main_menu/main_menu_title_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/race_setup/race_setup_ui.h"

#define ASSET_HANDLE(index) (gAssetHandles[(index)])

void drawMainMenuTitleCursor(MainMenuTitleCursorActor *arg0) {
    void *temp = getRelocatableHeapBlockBase(ASSET_HANDLE(33));

    drawMenuSprite(arg0->common.x, arg0->common.y, temp, 3, 0x20, 0x20, 0, 0);
}
