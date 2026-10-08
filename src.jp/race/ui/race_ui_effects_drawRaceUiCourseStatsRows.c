#include "game/engine/model_asset_symbols.h"
#include "game/race/course/race_course_effects.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/race_state.h"
#include "common.h"
#include <PR/os_libc.h>
#include "game/save_data.h"
#include "game/audio/sound_manager.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/render_callback.h"
#include "game/engine/system_runtime.h"
#include "game/engine/controller_input.h"
#include "assets.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/effects/snowboard_trail_effects.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/race/items/race_item_hit_flags.h"
#include "game/engine/viewport_manager.h"
#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"
#include "game/race/motion/race_motion.h"
#include "game/race/items/race_item_projectiles.h"
#include "game/race/player/race_player_movement.h"
#include "game/race/player/race_player_input.h"
#include "game/race/camera/race_camera.h"
#include "game/race/ui/race_hud.h"
#include <PR/os_cache.h>

typedef struct {
                 u8 pad0[0x4E];
                 RaceTimer courseStatsNames[10][5];
} RaceUiCourseStatsNameData;

typedef struct {
                 u8 pad0[0x77FB];
                 u8 courseStatsIcons[10][5];
} RaceUiCourseStatsIconData;

void drawRaceUiCourseStatsRows(RaceUiCourseStatsActor *arg0) {
    s32 y;
    s32 row;
    s32 offset;
    RaceUiCourseStatsActor *actor;

    actor = arg0;
    y = -0x50;
    for (row = 0, offset = 0; row != 5; row++, offset += 4, y += 0x20) {
        drawAssetTableSprite(-8, (s16)y, getRelocatableHeapBlockBase(gAssetHandles[0x1F]), row + 0x77);
        if ((row == actor->task.userId) && (gUiBlinkTimer & 1)) {
            drawRaceUiPackedTime(
                &((RaceUiCourseStatsNameData *)&gGameSaveDataBuffer[0])
                     ->courseStatsNames[gRaceCourseIndex.signedValue][row],
                0x10,
                y,
                0x10
            );
        } else if (row < 3) {
            drawRaceUiPackedTime(
                &((RaceUiCourseStatsNameData *)&gGameSaveDataBuffer[0])
                     ->courseStatsNames[gRaceCourseIndex.signedValue][row],
                0x10,
                y,
                0xC
            );
        } else {
            drawRaceUiPackedTime(
                &((RaceUiCourseStatsNameData *)&gGameSaveDataBuffer[0])
                     ->courseStatsNames[gRaceCourseIndex.signedValue][row],
                0x10,
                y,
                0xD
            );
        }

        drawAssetTableSprite(
            0x58,
            (s16)y,
            getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
            ((((RaceUiCourseStatsIconData *)&gGameSaveDataBuffer[0])
                  ->courseStatsIcons[gRaceCourseIndex.signedValue][row] &
              7) +
             0x51)
        );
        drawAssetTableSprite(
            0x6C,
            (s16)y,
            getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
            (((*((RaceUiCourseStatsIconData *)&gGameSaveDataBuffer[0]))
                  .courseStatsIcons[gRaceCourseIndex.signedValue][row] >>
              3) +
             0x7C)
        );
    }
}
