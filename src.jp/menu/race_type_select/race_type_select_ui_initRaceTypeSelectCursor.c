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

void initRaceTypeSelectCursor(RaceTypeSelectAnimatedWidgetActor *arg0) {
    arg0->x = -0x7C;
    arg0->y = -0x58;
    arg0->alpha = 0;
    arg0->state = 0;
    arg0->animationTimer = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectCursor);
}

void drawRaceTypeSelectPortrait(RaceTypeSelectAnimatedWidgetActor *arg0) {
    RaceTypeSelectDescription *description = &gRaceTypeSelectPortraitScripts.descriptions[gRaceTypeSelection];

    drawMenuGlyphScript(arg0->x, arg0->y, *description, 1, arg0->alpha, 0);
}
