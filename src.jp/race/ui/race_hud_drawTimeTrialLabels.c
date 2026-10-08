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

extern const char gRaceHudTimeTrialLapTimeLabel[];
extern const char gRaceHudTimeTrialBestLapFormat[];
extern const char gRaceHudTimeTrialBestLapLabel[];

void drawTimeTrialLabels(void *unused) {
    char bestLapText[0x20];

    drawMenuAsciiTextDefaultScale(0x48, 0x47, gRaceHudTimeTrialLapTimeLabel, 5);
    drawMenuAsciiTextDefaultScale(0x48, -0x61, gRaceHudTimeTrialBestLapLabel, 7);
    sprintf(
        bestLapText,
        gRaceHudTimeTrialBestLapFormat,
        gGameSaveDataBuffer[0].timeTrialRecords[gRaceCourseIndex.signedValue][0].minutes,
        gGameSaveDataBuffer[0].timeTrialRecords[gRaceCourseIndex.signedValue][0].seconds,
        gGameSaveDataBuffer[0].timeTrialRecords[gRaceCourseIndex.signedValue][0].fraction >> 8
    );
    drawMenuAsciiTextDefaultScale(0x48, -0x58, bestLapText, 7);
}
