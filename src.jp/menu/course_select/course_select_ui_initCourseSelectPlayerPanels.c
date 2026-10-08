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

void initCourseSelectPlayerPanels(CourseSelectPlayerPanelListActor *arg0) {
    if (gPlayerCount == 2) {
        arg0->exitTargetX = 0xA0;
        arg0->x[0] = 0x90;
        arg0->y[0] = -0xC8;
        arg0->x[1] = -0x18C;
        arg0->y[1] = 0x6C;
        arg0->targetX[0] = -0x7E;
        arg0->targetX[1] = -0x7E;
        arg0->deltaX[0] = -1;
        arg0->deltaX[1] = 1;
        arg0->deltaY[0] = 1;
        arg0->deltaY[1] = -1;
        arg0->period[0] = 0x10E;
        arg0->period[1] = 0x10E;
        arg0->speed[0] = 0x68;
        arg0->speed[1] = 0x68;
    } else {
        arg0->x[0] = -0x118;
        arg0->x[1] = -0x118;
        arg0->deltaX[0] = 1;
        arg0->deltaY[0] = 1;
        arg0->deltaX[1] = 1;
        arg0->deltaY[2] = 1;
        arg0->exitTargetX = -0x128;
        arg0->y[0] = -0xC8;
        arg0->y[1] = 0x6C;
        arg0->x[2] = 0x94;
        arg0->y[2] = -0xC8;
        arg0->x[3] = 0x94;
        arg0->y[3] = 0x6C;
        arg0->targetX[0] = -0x88;
        arg0->targetX[1] = -0x88;
        arg0->targetX[2] = 4;
        arg0->targetX[3] = 4;
        arg0->deltaY[1] = -1;
        arg0->deltaX[2] = -1;
        arg0->deltaX[3] = -1;
        arg0->deltaY[3] = -1;
        arg0->period[0] = 0x90;
        arg0->speed[0] = 0x68;
        arg0->period[1] = 0x90;
        arg0->speed[1] = 0x68;
        arg0->period[2] = 0x90;
        arg0->speed[2] = 0x68;
        arg0->period[3] = 0x90;
        arg0->speed[3] = 0x68;
    }
    arg0->timer[1] = 0;
    arg0->state[1] = 0;
    arg0->timer[2] = 0;
    arg0->state[2] = 0;
    arg0->timer[3] = 0;
    arg0->state[3] = 0;
    arg0->timer[0] = 0;
    arg0->state[0] = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseSelectPlayerPanels);
}
