#include "game/menu/menu_scratch.h"
#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/character_select/character_select_ui.h"
#include "game/race/player/race_player_input.h"

void updateCharacterSelectPlayerStatsPanels(CharacterSelectUiPlayerStatsPanelActor *arg0) {
    s32 i;
    CharacterSelectUiPlayerStatsPanelActor *statsPanels;
    CharacterSelectUiPlayerPanelFrameActor *panelFrames;

    panelFrames = gMenuScratch1.playerFrames;
    statsPanels = arg0;
    for (i = 0; i != RACE_PLAYER_COUNT; i++) {
        statsPanels->x[i] = panelFrames->x[i];
        statsPanels->y[i] = panelFrames->y[i];
        if (gRacePlayers[i].menuState == 0) {
            statsPanels->blinkTimer[i] = (statsPanels->blinkTimer[i] + 1) % 20;
        }
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectPlayerStatsPanels, arg0);
}
