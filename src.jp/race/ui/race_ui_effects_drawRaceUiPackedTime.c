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

extern const char gRaceUiPackedTimeSecondsFormat[];
extern const char gRaceUiPackedTimeMinutesFormat[];
extern const char gRaceUiPackedTimeFractionFormat[];

void drawRaceUiPackedTime(RaceTimer *arg0, s32 x, s32 arg2, s32 arg3) {
    char buffer[0x58];
    RaceTimer *record;
    s16 y;
    u16 color;
    s32 i;

    record = arg0;
    y = arg2;
    color = arg3;
    sprintf(&buffer[-0x10], gRaceUiPackedTimeMinutesFormat, record->minutes);
    i = -0x10;
    do {
        /* IDO register allocation for this function depends on this empty loop. */
        do {
        } while (0);
        drawAssetTableSpriteWithExplicitPaletteWideIndex(
            (s16)x,
            y,
            getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
            (((u8)buffer[i]) - 5) & 0xFFFF,
            color
        );
        i++;
        x += 8;
    } while (i < (-0xE));
    drawAssetTableSpriteWithExplicitPaletteWideIndex(
        (s16)x,
        y,
        getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
        0x36,
        color
    );
    x += 8;
    sprintf(&buffer[-0x10], gRaceUiPackedTimeSecondsFormat, record->seconds);
    i = -0x10;
    do {
        drawAssetTableSpriteWithExplicitPaletteWideIndex(
            (s16)x,
            y,
            getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
            (((u8)buffer[i]) - 5) & 0xFFFF,
            color
        );
        i++;
        x += 8;
    } while (i < (-0xE));
    drawAssetTableSpriteWithExplicitPaletteWideIndex(
        (s16)x,
        y,
        getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
        0x35,
        color
    );
    x += 8;
    sprintf(&buffer[-0x10], gRaceUiPackedTimeFractionFormat, record->fraction >> 8);
    i = -0x10;
    do {
        drawAssetTableSpriteWithExplicitPaletteWideIndex(
            (s16)x,
            y,
            getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
            (((u8)buffer[i]) - 5) & 0xFFFF,
            color
        );
        i++;
        x += 8;
    } while (i != (-0xE));
}
