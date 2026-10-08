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

extern const char gRaceUiTrickScorePopupBonusFormat[0xC];
extern const char gRaceUiTrickScorePopupPointsFormat[0x4];

void drawRaceUiTrickScorePopup(RaceUiPopupActor *arg0) {
    volatile u8 padding[0x20];
    char buffer[8];
    s32 i;

    if (gCurrentViewportIndex == 0) {
        drawAssetTableSprite(
            (s16)(arg0->x - 0x3C),
            arg0->y.half.lo,
            getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
            0x8B
        );
        drawAssetTableSprite(
            (s16)(arg0->x + 0x14),
            arg0->y.half.lo,
            getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
            0x8E
        );
        sprintf(buffer, gRaceUiTrickScorePopupPointsFormat, arg0->parent);
        for (i = 0; i < 2; i++) {
            if (buffer[i] != ' ') {
                drawAssetTableSpriteWithExplicitPalette(
                    (s16)(arg0->x + (i * 8) + 4),
                    arg0->y.half.lo,
                    getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
                    buffer[i] - 5,
                    0xE
                );
            }
        }

        if (arg0->playerIndex != 0) {
            drawAssetTableSprite(
                (s16)(arg0->x - 0x48),
                (s16)(arg0->y.word + 0x12),
                getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
                0x8D
            );
            drawAssetTableSprite(
                (s16)(arg0->x + 0x20),
                (s16)(arg0->y.word + 0x12),
                getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
                0x8E
            );
            sprintf(buffer, gRaceUiTrickScorePopupBonusFormat, 0x12C);
            for (i = 0; i != 3; i++) {
                if (buffer[i] != ' ') {
                    drawAssetTableSpriteWithExplicitPalette(
                        (s16)(arg0->x + (i * 8) + 8),
                        (s16)(arg0->y.word + 0x12),
                        getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
                        buffer[i] - 5,
                        0xE
                    );
                }
            }
        }
    }
}
