#include "game/menu/menu_scratch.h"
#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/character_select/character_select_ui.h"
#include "game/race/player/race_player_input.h"

void initCharacterSelectPlayerStatsPanels(CharacterSelectUiPlayerStatsPanelActor *arg0) {
    arg0->x[0] = -0x88;
    arg0->y[0] = -0x60;
    arg0->x[1] = -0x88;
    arg0->y[1] = 0x24;
    arg0->x[2] = 4;
    arg0->y[2] = -0x60;
    arg0->x[3] = 4;
    arg0->y[3] = 0x24;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectPlayerStatsPanels);
}
