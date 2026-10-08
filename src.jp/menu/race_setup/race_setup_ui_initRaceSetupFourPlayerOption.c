#include "game/menu/menu_scratch.h"
#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/audio/sound_manager.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/race_setup/race_setup_menu.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/race/player/race_player_input.h"

void initRaceSetupFourPlayerOption(MenuItemActor *arg0) {
    MenuItemActor *actor = arg0;

    actor->x = 0x90;
    actor->y = 0x48;
    actor->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSetupFourPlayerOption);
}

void drawRaceSetupPlayerCountCursor(void *arg0) {
    FadeItemActor *actor = arg0;

    drawMenuSpriteWithAlpha(
        actor->x,
        (s16)((actor->y + (gPlayerCount << 5)) - 0x20),
        getRelocatableHeapBlockBase(gAssetHandles[33]),
        4,
        0x20,
        0x20,
        0,
        actor->uAlpha,
        0
    );
}
