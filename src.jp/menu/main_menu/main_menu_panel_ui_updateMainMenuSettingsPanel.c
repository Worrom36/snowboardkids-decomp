#include "game/race/race_state.h"
#include "game/engine/game_task_scheduler.h"
#include "font_encoding.h"
#include "common.h"
#include "game/audio/sound_manager.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/system_runtime.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/main_menu/main_menu_panel_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"

void updateMainMenuSettingsPanel(MenuPanelActor *arg0) {
    addRenderCallback(&gMenuRenderCallbackList, drawMainMenuSettingsPanel, arg0);
    addRenderCallback(&gMenuRenderCallbackList, drawMainMenuSettingsOptions, arg0);
}

void initMainMenuSettingsPanel(MenuPanelActor *arg0) {
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateMainMenuSettingsPanel);
}
