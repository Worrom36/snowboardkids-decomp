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

void updateRaceSetupSavePanelIcons(void *arg0) {
    RectListActor *temp_a2;
    RectListActor *actor = arg0;
    s32 i;

    for (i = 0; i < 4; i++) {
        actor->rectCoords[i] = gRaceSetupSavePanelRects[0][i];
        actor->rectCoords[i + 4] = gRaceSetupSavePanelRects[1][i];
    }
    temp_a2 = actor;
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceSetupSavePanelIcons, (void *)temp_a2);
}

void initRaceSetupSavePanelIcons(RectListActor *arg0) {
    RectListActor *actor = arg0;

    actor->rects[0].x0 = -0x114;
    actor->rects[1].x0 = -0xA4;
    actor->rects[0].y0 = -0x114;
    actor->rects[1].y0 = 0x48;
    actor->rects[0].x1 = 0x90;
    actor->rects[1].x1 = -0xA4;
    actor->rects[0].y1 = 0x90;
    actor->rects[1].y1 = 0x48;
    actor->stepLimit = 4;
    actor->stepIncrement = 0x11;
    actor->stepAccumulator = 2;
    actor->unk2E = 0x27;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSetupSavePanelIcons);
}
