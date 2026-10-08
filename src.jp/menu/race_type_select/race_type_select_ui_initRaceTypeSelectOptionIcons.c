#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/race_type_select/race_type_select_ui.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/race_state.h"
#include "game/race/player/race_player_input.h"

void initRaceTypeSelectOptionIcons(RaceTypeSelectOptionIconActor *arg0) {
    s32 temp_v1 = -0x104;
    long long zero;
    s32 temp_t6 = -0x58;
    s32 temp_t7 = 1;

    arg0->x[0] = temp_v1;
    arg0->x[1] = temp_v1;
    arg0->x[2] = temp_v1;
    arg0->x[3] = temp_v1;
    arg0->y = temp_t6;
    zero = 0;
    arg0->spawnTimer = zero;
    arg0->visibleCount = temp_t7;
    arg0->state = zero;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectOptionIcons);
}
