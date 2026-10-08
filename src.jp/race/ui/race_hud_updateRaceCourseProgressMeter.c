#include "game/race/items/race_items.h"
#include "game/race/race_state.h"
#include "common.h"
#include <PR/os_libc.h>
#include "game/save_data.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/render_callback.h"
#include "assets.h"
#include "game/engine/asset_manager.h"
#include "game/engine/relocatable_heap.h"
#include "game/race/items/race_item_effects.h"
#include "game/race/player/race_player_movement.h"
#include "game/race/ui/race_hud.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/player/race_player_input.h"
#include "game/engine/callback_task_scheduler.h"

void updateRaceCourseProgressMeter(void) {
    s32 meterPosition;
    s32 slots[2];
    s32 i;

    for (i = 0; i < 4; i++) {
        getRacePlayerRankingProgress(i, &slots[1], &slots[0]);
        if (slots[1] < 0) {
            slots[1] = 0;
        }
        gRacePlayers[i].progressMeterPosition =
            (slots[1] << 7) / (gRaceCourseStartEntries[gRaceCourseIndex.signedValue].pathIndex * 8);
        if (gRacePlayers[i].progressMeterPosition >= 0x81) {
            gRacePlayers[i].progressMeterPosition = 0x80;
        }
    }

    if (gRaceHudMode == RACE_HUD_MODE_ONE_PLAYER) {
        addRenderCallback(&gRaceOverlayRenderCallbackList, drawRaceCourseProgressMeter, 0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, drawRaceCourseProgressMeter, 0);
}
