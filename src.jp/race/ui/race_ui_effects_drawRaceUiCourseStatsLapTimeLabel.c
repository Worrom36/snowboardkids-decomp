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

extern const char gRaceUiLapTimeLabel[0xC];

void drawRaceUiCourseStatsLapTimeLabel(RaceUiAlpha18Actor *arg0) {
    Gfx *gfx;
    s32 w0;
    s32 w1;

    /* IDO scheduling for this function depends on this block staying on one line. */
    do { \
        if (arg0->alpha != 0xFF) { \
            gfx = gRegionAllocPtr; \
            gRegionAllocPtr = gfx + 1; \
            gfx->words.w1 = 0; \
            gfx->words.w0 = 0xE7000000; \
            gfx = gRegionAllocPtr; \
            gRegionAllocPtr = gfx + 1; \
            w1 = 0xFF2FFFFF; \
            w0 = 0xFC119623; \
            gfx->words.w0 = w0; \
            gfx->words.w1 = w1; \
            gfx = gRegionAllocPtr; \
            gRegionAllocPtr = gfx + 1; \
            w1 = 0x00504240; \
            w0 = 0xB900031D; \
            gfx->words.w0 = w0; \
            gfx->words.w1 = w1; \
            gfx = gRegionAllocPtr; \
            gRegionAllocPtr = gfx + 1; \
            gfx->words.w0 = 0xFA000000; \
            gfx->words.w1 = (arg0->alpha & 0xFF) | (~0xFF); \
        } \
        drawMenuAsciiTextDefaultScale(-0x74, -0x3C, gRaceUiLapTimeLabel, 5); \
        if (arg0->alpha != 0xFF) { \
            gfx = gRegionAllocPtr; \
            gRegionAllocPtr = gfx + 1; \
            w1 = (s32)gMenuRenderModeResetDl; \
            w0 = 0x06000000; \
            gfx->words.w0 = w0; \
            gfx->words.w1 = w1; \
        } \
    } while (0);
}
