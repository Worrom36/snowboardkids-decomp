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
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/character_select/character_select_course_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/player/race_player_input.h"

void initCharacterSelectLimitedCourseList(CharacterSelectCourseMenuFrameActor *arg0) {
    s32 i;

    arg0->baseY = -0x58;
    arg0->itemSpacing = 0x18;
    for (i = 0; i < 4; i++) {
        arg0->x[i] = -0x104;
        arg0->y[i] = arg0->baseY + (i * arg0->itemSpacing);
    }
    arg0->timer = 0;
    arg0->itemCount = 1;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectLimitedCourseList);
}
