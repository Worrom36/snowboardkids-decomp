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

#define ASSET_HANDLE(index) (gAssetHandles[(index)])

extern const char gRaceTypeSelectEntryFeeFormat[];

void drawRaceTypeSelectEntryFee(RaceTypeSelectAnimatedWidgetActor *arg0) {
    char sp40[0x18];

    if (gRaceTypeSelection != 3) {
        drawMenuPanelBackdrop(arg0->x, arg0->y, 0x5000, 0x4000);
        drawMenuSpriteWithAlpha(
            (s16)(arg0->x + 8),
            (s16)(arg0->y + 4),
            getRelocatableHeapBlockBase(ASSET_HANDLE(33)),
            0x11,
            0x20,
            0x20,
            0,
            arg0->alpha,
            0
        );
        sprintf(sp40, gRaceTypeSelectEntryFeeFormat, gRacePlayers[0].money);
        drawMenuAsciiText((s16)(arg0->x + 0x10), (s16)(arg0->y + 0x10), sp40, 0, arg0->alpha);
    }
}
