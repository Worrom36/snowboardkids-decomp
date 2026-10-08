#include "common.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/main_menu/main_menu_title_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/race_setup/race_setup_ui.h"

void initMainMenuTitleCursor(MainMenuTitleCursorActor *arg0) {
    arg0->common.y = 0x20;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateMainMenuTitleCursor);
}
