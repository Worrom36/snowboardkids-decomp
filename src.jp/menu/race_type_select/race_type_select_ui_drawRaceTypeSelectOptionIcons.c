#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/race_type_select/race_type_select_ui.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/race_state.h"
#include "game/race/player/race_player_input.h"

extern u8 gMenuExitSelection;

void drawRaceTypeSelectOptionIcons(RaceTypeSelectOptionIconActor *arg0) {
    RaceTypeSelectOptionIconActor *savedArg;
    s32 i;
    s32 alpha;
    s32 yOffset;
    u16 tileIndex;

    savedArg = arg0;
    do { \
        i = 0; \
        if (arg0->visibleCount > 0) { \
            yOffset = 0; \
            do { \
                alpha = 0; \
                if (((((gMenuSelectionConfirmTimer > 0) && (gMenuSelectionConfirmTimer < 8)) && \
                      (gMenuExitSelection == 0)) && \
                     (i == gRaceTypeSelection)) && \
                    (gMenuSelectionConfirmTimer & 1)) { \
                    if ((!savedArg->visibleCount) && (!savedArg->visibleCount)) {} \
                    alpha = 0xFF; \
                } \
                tileIndex = i + 0xD; \
                drawMenuSprite( \
                    arg0->x[i], \
                    (s16)(arg0->y + yOffset), \
                    getRelocatableHeapBlockBase(gAssetHandles[0x21]), \
                    tileIndex, \
                    0x20, \
                    0x20, \
                    0, \
                    alpha \
                ); \
                yOffset += 0x18; \
                i++; \
            } while (i < savedArg->visibleCount); \
        } \
    } while (0);
}
