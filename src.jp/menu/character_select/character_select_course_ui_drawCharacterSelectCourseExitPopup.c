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

#define ASSET_HANDLE(index) (gAssetHandles[index])

extern MenuGlyphScript gCharacterSelectCourseStartGameText[44];

void drawCharacterSelectCourseExitPopup(CharacterSelectCourseWidgetActor *arg0) {
    drawMenuSprite(arg0->x, arg0->y, getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)), 0x28, 0x20, 0x20, 0, 0);
    if ((s32)arg0->transition.bytes.state > 0) {
        drawMenuGlyphScript((s16)(arg0->x + 4), (s16)(arg0->y + 4), gCharacterSelectCourseStartGameText, 1, 0x100, 0);
        drawMenuSprite(
            (s16)(arg0->x + 0x64),
            (s16)(arg0->y + 4),
            getRelocatableHeapBlockBase(ASSET_HANDLE(0x24)),
            (u16)((arg0->transition.bytes.timer >= 8) + 5),
            0x20,
            0x20,
            0,
            0
        );
    }
}
