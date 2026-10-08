#include "game/menu/menu_scratch.h"
#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/controller_pak/controller_pak_race_record_save_flow.h"
#include "game/menu/controller_pak/controller_pak_race_record_save_ui.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/player/race_player_input.h"

void initControllerPakRaceRecordSaveScorePanel(ControllerPakRaceRecordSaveScorePanelActor *arg0) {
    arg0->common.x = 0x90;
    arg0->common.y = -0x20;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakRaceRecordSaveScorePanel);
}
