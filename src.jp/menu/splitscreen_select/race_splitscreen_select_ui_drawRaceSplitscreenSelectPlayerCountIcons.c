#include "game/race/race_state.h"
#include "font_encoding.h"
#include "common.h"
#include <PR/os_libc.h>
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/system_runtime.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/player/race_player_input.h"

#define RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE (gAssetHandles[0x21])

extern u8 gMenuExitSelection;

void drawRaceSplitscreenSelectPlayerCountIcons(RaceSplitscreenSelectOptionIconActor *icons) {
    RaceSplitscreenSelectOptionIconActor *savedIcons;
    s32 iconIndex;
    s32 blinkAlpha;
    s32 yOffset;
    u16 tileIndex;

    savedIcons = icons;
    iconIndex = 0;
    if (iconIndex < (s32) icons->visibleCount) {
        /*
         * The outer do/while (0), repeated iconIndex initialization, empty if,
         * and paired statements must retain this shape for IDO's register
         * allocation. The icon position itself is nevertheless a normal array
         * access; the compiler converts it to the pointer walk in the ROM.
         */
        // clang-format off
        do { iconIndex = 0; yOffset = 0; do {
        // clang-format on
            blinkAlpha = 0;
            if (((((gMenuSelectionConfirmTimer != 0) && (((s32) gMenuSelectionConfirmTimer) < 8)) &&
                 (gMenuExitSelection == 0)) &&
                (iconIndex == gRaceSplitscreenMode)) &&
                (gMenuSelectionConfirmTimer & 1)) {
                blinkAlpha = 0xFF;
            }
            tileIndex = iconIndex + 8;
            drawMenuSprite(icons->x[iconIndex], (s16)(icons->y + yOffset),
                           getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
                           tileIndex, 0x20, 0x20, 0, blinkAlpha);
            yOffset += 0x14; iconIndex++;
            if (icons->visibleCount) {}
        } while (iconIndex < ((s32) savedIcons->visibleCount)); } while (0);
    }
}
