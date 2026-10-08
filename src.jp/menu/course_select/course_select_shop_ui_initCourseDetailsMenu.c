#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/course_select/course_select_shop_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/player/race_player_input.h"

enum {
    COURSE_DETAILS_PAGE_COUNT = 2,
    COURSE_DETAILS_COURSES_PER_PAGE = 7,
    COURSE_DETAILS_OPTION_COUNT = 10,
    COURSE_DETAILS_ROW_SPACING = 0x13
};

void initCourseDetailsMenu(ShopMenuWidgetActor *arg0) {
    s32 i;

    for (i = 0; i < COURSE_DETAILS_OPTION_COUNT; i++) {
        arg0->cursorPositions[i] = -0xFC;
    }

    arg0->targetX = -0x7C;
    arg0->targetY = -0x5C;
    arg0->prompt.bytes.pulseTimer = 0;
    arg0->spawnTimer = 0;
    arg0->visibleCount = 1;
    arg0->prompt.bytes.pulseAlpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseDetailsMenu);
}
