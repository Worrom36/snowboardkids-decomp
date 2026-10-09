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

void initRaceSplitscreenSelectPlayerCountIcons(RaceSplitscreenSelectOptionIconActor *arg0) {
    s32 i;

    // clang-format off
    for (i = 0; i < 5; i++) { arg0->x[i] = -0x104; }
    // clang-format on

    arg0->y = -0x60;
    arg0->spawnTimer = 0;
    arg0->visibleCount = 1;
    arg0->state = 0;

    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectPlayerCountIcons);
}
