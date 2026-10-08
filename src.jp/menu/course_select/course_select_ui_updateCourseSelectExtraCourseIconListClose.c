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

void updateCourseSelectExtraCourseIconListClose(CourseSelectExtraCourseIconListActor *arg0) {
    s32 i;
    s32 j;
    CourseSelectExtraCourseIconListActor *temp_a2 = arg0;

    i = 0;
    if ((s32)gPlayerCount > 0) {
        do {
            j = 0;
            if ((s32)arg0->itemCounts[i] > 0) {
                do {
                    temp_a2->iconX[i][j] -= 0x20;
                    j++;
                } while (j < (s32)arg0->itemCounts[i]);
            }
            i++;
        } while (i < (s32)gPlayerCount);
    }

    if ((gPlayerCount == 1) && (temp_a2->itemCounts[0] == 0)) {
        removeCallbackTask(temp_a2);
        return;
    }

    if (temp_a2->iconX[0][0] < -0xDF) {
        removeCallbackTask(temp_a2);
        finishCourseSelectUiTask(8);
        gMenuScratch2.task = 0;
        return;
    }

    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCourseSelectExtraCourseIconList, temp_a2);
}
