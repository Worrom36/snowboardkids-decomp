#include "game/menu/menu_scratch.h"
#include "game/race/race_state.h"
#include "font_encoding.h"
#include "common.h"
#include <PR/os_libc.h>
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/course_select/course_select_ui.h"
#include "game/menu/course_select/course_select_shop_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/math/fixed_point_math.h"
#include "game/race/player/race_player_model_renderer.h"
#include "game/race/player/race_player_input.h"

void initCourseSelectCompletePanels(CourseSelectCompletePanelsActor *arg0) {
    s32 var_v0;
    CourseSelectCompletePanelsActor *var_v1;

    if (gPlayerCount == 2) {
        arg0->x = -0x6E;
        arg0->y = -0x4C;
    } else {
        arg0->x = -0x8C;
        arg0->y = -0x46;
    }
    var_v0 = 0;
    var_v1 = arg0;
    if ((s32)gPlayerCount > 0) {
        do {
            var_v1->playerPanelFadeAlpha[0] = 0;
            var_v0++;
            var_v1 = (CourseSelectCompletePanelsActor *)((u8 *)var_v1 + sizeof(s16));
        } while (var_v0 < (s32)gPlayerCount);
    }
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseSelectCompletePanels);
}
