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
                 u8 pad0[0x156];
                 RaceTimer resultNames[10][5];
} RaceUiResultNameData;

typedef struct {
                 u8 pad0[0x78A0];
                 u8 resultIcons[10][5];
} RaceUiResultIconData;

void drawRaceUiCourseRecordRows(RaceUiPopupActor *arg0) {
    s32 y;
    s32 color;
    s32 i;
    s32 offset;

    y = -0x50;
    /* IDO scheduling for this function depends on this initialization staying on one line. */
    // clang-format off
    i = 0; offset = 0; while (1) {
    // clang-format on
        drawAssetTableSprite(-8, (s16)y, getRelocatableHeapBlockBase(gAssetHandles[0x1F]), i + 0x77);
        if ((i == arg0->task.userId) && (gUiBlinkTimer & 1)) {
            color = 0x10;
        } else if (i < 3) {
            color = 0xC;
        } else {
            color = 0xD;
        }
        drawRaceUiPackedTime(&((RaceUiResultNameData *)&gGameSaveDataBuffer[0])->resultNames[gRaceCourseIndex.signedValue][i], 0x10, y, color);
        drawAssetTableSprite(0x58, (s16)y, getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
                      (((RaceUiResultIconData *)&gGameSaveDataBuffer[0])->resultIcons[gRaceCourseIndex.signedValue][i] & 7) + 0x51);
        drawAssetTableSprite(0x6C, (s16)y, getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
                      ((*((RaceUiResultIconData *)&gGameSaveDataBuffer[0])).resultIcons[gRaceCourseIndex.signedValue][i] >> 3) + 0x7C);
        i++;
        offset += 4;
        y += 0x20;
        if (i == 5) {
            break;
        }
    }
}
