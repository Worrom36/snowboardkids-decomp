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

void drawCharacterSelectRosterIcons(CharacterSelectUiRosterIconActor *arg0) {
    s16 *assetHandles;
    s32 i;
    s32 j;
    s32 xOffset;
    u16 alpha;
    u16 tile;
    u8 textureIndex;
    s32 selectedAlpha;
    s32 tileTemp;

    i = 0; \
    if (arg0->state != 0) { \
        assetHandles = gAssetHandles; \
        xOffset = 0; \
        selectedAlpha = 0x60; \
        do { \
            alpha = 0x100; \
            j = 0; \
            if (gPlayerCount > 0) { \
                do { \
                    if ((i == gRacePlayers[j].selectedCharacterId) && (gRacePlayers[j].menuState != 0)) {
                        tileTemp = i + 0x41;
                        alpha = 0x60;
                        tile = tileTemp;
                        textureIndex = 0x1F;
                        break;
                    }
                    j++;
                } while (j < gPlayerCount);
            }
            if (selectedAlpha != alpha) {
                textureIndex = 0x21;
                if (arg0->timer < 0xA) {
                    tileTemp = i + 0x37;
                    tile = tileTemp;
                } else {
                    tileTemp = i + 0x3D;
                    tile = tileTemp;
                }
            }
            drawMenuSpriteTile((s16)(arg0->x + xOffset), arg0->y,
                          getRelocatableHeapBlockBase(assetHandles[textureIndex]), tile, 0, alpha);
            i++;
            xOffset += 0x20;
        } while (i < 5);

        if (arg0->leftSecretSlotUnlocked != 0) {
            alpha = 0x100;
            j = 0;
            if (gPlayerCount > 0) {
                do {
                    if ((gRacePlayers[j].selectedCharacterId == 5) && (gRacePlayers[j].menuState != 0)) {
                        alpha = 0x60;
                        tile = 0x46;
                        textureIndex = 0x1F;
                        break;
                    }
                    j++;
                } while (j < gPlayerCount);
            }
            if (selectedAlpha != alpha) {
                textureIndex = 0x21;
                if (arg0->timer < 0xA) {
                    tile = 0x3C;
                } else {
                    tile = 0x42;
                }
            }
            drawMenuSpriteTile((s16)(arg0->x - 0x20), arg0->y,
                          getRelocatableHeapBlockBase(assetHandles[textureIndex]), tile, 0, alpha);
        }
    }
}
