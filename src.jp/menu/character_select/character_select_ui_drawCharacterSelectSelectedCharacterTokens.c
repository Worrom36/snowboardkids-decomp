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

#define CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE (gAssetHandles[0x21])

void drawCharacterSelectSelectedCharacterTokens(CharacterSelectUiSelectedCharacterTokenActor *arg0) {
    CharacterSelectUiSelectedCharacterTokenActor *tokens;
    s32 i;
    void *texture;
    u16 tileSize;

    tokens = arg0;
    i = 0;
    if (gPlayerCount > 0) {
        do {
            if (gRacePlayers[i].menuState != 0) {
                texture = getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE);
                tileSize = tokens->tileSize[i];
                drawMenuSprite(tokens->x[i], tokens->y[i], texture, 0xD, tileSize, tileSize, 0, 0);
            }
            i++;
        } while (i < gPlayerCount);
    }
}
