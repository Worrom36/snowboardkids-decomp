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

void drawControllerPakFileDeleteFreeSpaceInfo(ControllerPakTwoPointActor *arg0) {
    char padBefore[4];
    u16 text[4];
    char padAfter[8];
    s32 value;
    s32 i;

    // IDO register allocation nudge.
    if (1) {
        i = 0;
        do {
            text[i] = 0xFFFE;
            i++;
        } while (i < 4);

        value = gControllerPakFreeFileCount;
        i = 0;
        do {
            text[1 - i] = value % 10;
            value = value / 10;
            i++;
        } while (value != 0);
        text[2] = 0xFFFF;
        drawMenuGlyphScript(arg0->common.x, arg0->common.y, text, 1, 0x100, 8);
        i = 0;
    }
    do {
        text[i] = 0xFFFE;
        i++;
    } while (i < 4);

    value = gControllerPakFreeBytes / 0x100;
    i = 0;
    do {
        text[2 - i] = value % 10;
        value = value / 10;
        i++;
    } while (value != 0);
    text[3] = 0xFFFF;
    drawMenuGlyphScript(arg0->x2, arg0->y2, text, 1, 0x100, 8);
}
