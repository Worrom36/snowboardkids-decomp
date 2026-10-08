#include "common.h"
#include "font_encoding.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/controller_pak/controller_pak_continue_prompt_flow.h"
#include "game/menu/controller_pak/controller_pak_menu.h"
#include "game/menu/controller_pak/controller_pak_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/race/player/race_player_input.h"

void initControllerPakContinuePrompt(ControllerPakOptionsActor *arg0) {
    arg0->common.x = -0x70;
    arg0->common.y = -0x28;
    arg0->selectedOption = 0;
    arg0->blinkState = 0;
    arg0->scale = 0x100;
    arg0->timer = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakContinuePrompt);
}
