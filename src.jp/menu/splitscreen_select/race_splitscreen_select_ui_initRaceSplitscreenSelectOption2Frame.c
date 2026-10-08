#include "game/race/race_state.h"
#include "font_encoding.h"
#include "common.h"
#include <PR/os_libc.h>
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/system_runtime.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/player/race_player_input.h"

void initRaceSplitscreenSelectOption2Frame(RaceSplitscreenSelectOptionFrameActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x140;
    arg0->optionIndex = 2;
    arg0->unusedTimer = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectOption2Frame);
}
