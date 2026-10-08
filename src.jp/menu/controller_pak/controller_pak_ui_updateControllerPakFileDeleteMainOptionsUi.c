#include "common.h"
#include "font_encoding.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/controller_pak/controller_pak_continue_prompt_flow.h"
#include "game/menu/controller_pak/controller_pak_menu.h"
#include "game/menu/controller_pak/controller_pak_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/race/player/race_player_input.h"

extern u8 gControllerPakMenuCursorState;

void updateControllerPakFileDeleteMainOptionsUi(ControllerPakOptionsActor *arg0) {
    switch (gControllerPakMenuCursorState) {
        case 0:
            if (arg0->timer < 0x10) {
                arg0->scale -= 8;
            } else {
                arg0->scale += 8;
            }
            arg0->timer = (arg0->timer + 1) & 0x1F;
            break;
        case 1:
        case 2:
            arg0->scale = 0x100;
            arg0->timer = 0;
            break;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawControllerPakFileDeleteMainOptions, (void *)arg0);
}
