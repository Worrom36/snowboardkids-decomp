#include "game/menu/menu_scratch.h"
#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/controller_pak/controller_pak_race_record_save_flow.h"
#include "game/menu/controller_pak/controller_pak_race_record_save_ui.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/player/race_player_input.h"

void drawControllerPakRaceRecordSavePromptFrame(ControllerPakRaceRecordSavePromptFrameActor *arg0) {
    s32 zero = 0;
    u16 alpha;

    if (gControllerPakStatusCodes[0] != 8) {
        alpha = arg0->scale;
        drawMenuSpriteWithAlpha(
            (s16)(arg0->common.x - 2),
            (s16)(arg0->common.y + 0xC),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            8,
            0x20,
            0x20,
            zero,
            alpha,
            zero
        );
        drawMenuSpriteWithAlpha(
            (s16)(arg0->common.x + 0x3E),
            (s16)(arg0->common.y + 0xC),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            9,
            0x20,
            0x20,
            zero,
            alpha,
            zero
        );
    }
}
