#include "game/menu/menu_scratch.h"
#include "game/race/race_state.h"
#include "font_encoding.h"
#include "common.h"
#include <PR/os_libc.h>
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/course_select/course_select_ui.h"
#include "game/menu/course_select/course_select_shop_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/math/fixed_point_math.h"
#include "game/race/player/race_player_model_renderer.h"
#include "game/race/player/race_player_input.h"

extern const char D_800E0DB8[];

void drawCourseSelectPlayerPanels(CourseSelectPlayerPanelListActor *arg0) {
    s32 j;
    s32 i;
    s32 middleCount;
    u8 tile;
    s32 alpha;
    s32 count;
    u8 text[4];
    s32 offset;
    CourseSelectPlayerPanelListActor *actor = (CourseSelectPlayerPanelListActor *)arg0;

    if (gPlayerCount == 2) {
        count = 2;
    } else {
        count = 4;
    }

    for (i = 0; i < count; i++) {
        if (i == gPlayerCount) {
            alpha = 0xC0;
        } else {
            alpha = 0x100;
        }

        if (i == 0) {
            tile = 2;
        } else {
            tile = i + 5;
        }

        drawMenuSpriteWithAlphaWideArgs(
            actor->x[i],
            actor->y[i],
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x22,
            0x20,
            0x20,
            0,
            alpha,
            tile
        );

        if (gPlayerCount == 2) {
            middleCount = 0xB;
        } else {
            middleCount = 3;
        }

        for (j = 0, offset = 0x38; j < middleCount; j++, offset += 0x10) {
            drawMenuSpriteWithAlphaWideArgs(
                (s16)(actor->x[i] + offset),
                actor->y[i],
                getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                0x23,
                0x20,
                0x20,
                0,
                alpha,
                tile
            );
        }

        drawMenuSpriteWithAlphaWideArgs(
            (s16)(actor->x[i] + offset),
            actor->y[i],
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x24,
            0x20,
            0x20,
            0,
            alpha,
            tile
        );

        if (gPlayerCount == 2) {
            offset = 0xF8;
        } else {
            offset = 0x78;
        }

        for (j = 0; j < 4; j++) {
            drawMenuSpriteWithAlphaWideArgs(
                actor->x[i],
                (s16)(actor->y[i] + j * 0x10 + 0x10),
                getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                0x25,
                0x20,
                0x20,
                0,
                alpha,
                tile
            );

            drawMenuSpriteWithAlphaWideArgs(
                (s16)(actor->x[i] + offset),
                (s16)(actor->y[i] + j * 0x10 + 0x10),
                getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                0x26,
                0x20,
                0x20,
                0,
                alpha,
                tile
            );
        }

        drawMenuSpriteWithAlphaWideArgs(
            actor->x[i],
            (s16)(actor->y[i] + 0x50),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x27,
            0x20,
            0x20,
            0,
            alpha,
            tile
        );

        if (gPlayerCount == 2) {
            middleCount = 0xE;
        } else {
            middleCount = 6;
        }

        for (j = 0; j < middleCount; j++) {
            drawMenuSpriteWithAlphaWideArgs(
                (s16)(actor->x[i] + j * 0x10 + 0x10),
                (s16)(actor->y[i] + 0x50),
                getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                0x28,
                0x20,
                0x20,
                0,
                alpha,
                tile
            );
        }

        drawMenuSpriteWithAlphaWideArgs(
            (s16)(actor->x[i] + offset - 8),
            (s16)(actor->y[i] + 0x50),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x29,
            0x20,
            0x20,
            0,
            alpha,
            tile
        );

        sprintf(text, D_800E0DB8, i + 1);
        drawMenuAsciiText((s16)(actor->x[i] + 0x34), (s16)(actor->y[i] + 2), text, 0, alpha);

        if (alpha == 0xC0) {
            drawMenuSpriteWithAlphaWideArgs(
                (s16)(actor->x[i] + 2),
                (s16)(actor->y[i] + 0x24),
                getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
                0x90,
                0x20,
                0x20,
                0,
                0xF0,
                0
            );
        }
    }
}
