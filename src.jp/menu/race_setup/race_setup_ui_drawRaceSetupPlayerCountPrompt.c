#include "game/menu/menu_scratch.h"
#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/audio/sound_manager.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/race_setup/race_setup_menu.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/race/player/race_player_input.h"

#define TITLE_MENU_FRAME_TEXTURE_HANDLE (gAssetHandles[33])
#define TITLE_MENU_BANNER_TEXTURE_HANDLE (gAssetHandles[41])

extern MenuGlyphScript gRaceSetupPlayerCountPromptText[5][0x34];

void drawRaceSetupPlayerCountPrompt(MenuIntroActor *arg0) {
    s32 i;
    s32 j;
    s32 selected;
    s32 limit;
    MenuIntroActor *actor;

    actor = arg0;
    drawMenuSpriteWithAlpha(
        (s16)(actor->x - 4),
        (s16)(actor->y - 4),
        getRelocatableHeapBlockBase(TITLE_MENU_BANNER_TEXTURE_HANDLE),
        2,
        0x20,
        0x20,
        0,
        actor->alpha,
        0
    );
    drawMenuSpriteWithAlpha(
        (s16)(actor->x + 0xD4),
        (s16)(actor->y - 4),
        getRelocatableHeapBlockBase(TITLE_MENU_BANNER_TEXTURE_HANDLE),
        4,
        0x20,
        0x20,
        0,
        actor->alpha,
        0
    );
    i = 0;
    do {
        drawMenuSpriteWithAlpha(
            (s16)(actor->x + i),
            (s16)(actor->y - 4),
            getRelocatableHeapBlockBase(TITLE_MENU_BANNER_TEXTURE_HANDLE),
            3,
            0x20,
            0x20,
            0,
            actor->alpha,
            0
        );
        drawMenuSpriteWithAlpha(
            (s16)(actor->x + i),
            (s16)(actor->y + 0x24),
            getRelocatableHeapBlockBase(TITLE_MENU_BANNER_TEXTURE_HANDLE),
            8,
            0x20,
            0x20,
            0,
            actor->alpha,
            0
        );
        i += 0x10;
    } while (i < 0xE0);
    drawMenuSpriteWithAlpha(
        (s16)(actor->x - 4),
        (s16)(actor->y + 0x24),
        getRelocatableHeapBlockBase(TITLE_MENU_BANNER_TEXTURE_HANDLE),
        7,
        0x20,
        0x20,
        0,
        actor->alpha,
        0
    );
    drawMenuSpriteWithAlpha(
        (s16)(actor->x + 0xD4),
        (s16)(actor->y + 0x24),
        getRelocatableHeapBlockBase(TITLE_MENU_BANNER_TEXTURE_HANDLE),
        9,
        0x20,
        0x20,
        0,
        actor->alpha,
        0
    );
    i = (actor->state == 4) * 0;
    limit = 0xE0;
    do {
        drawMenuSpriteWithAlpha(
            (s16)(actor->x - 4),
            (s16)(actor->y + i),
            getRelocatableHeapBlockBase(TITLE_MENU_BANNER_TEXTURE_HANDLE),
            5,
            0x20,
            0x20,
            0,
            actor->alpha,
            0
        );
        drawMenuSpriteWithAlpha(
            (s16)(actor->x + 0xD4),
            (s16)(actor->y + i),
            getRelocatableHeapBlockBase(TITLE_MENU_BANNER_TEXTURE_HANDLE),
            6,
            0x20,
            0x20,
            0,
            actor->alpha,
            0
        );
        j = 0;
        do {
            drawMenuSpriteWithAlpha(
                (s16)(actor->x + j),
                (s16)(actor->y + i),
                getRelocatableHeapBlockBase(TITLE_MENU_BANNER_TEXTURE_HANDLE),
                0xB,
                0x20,
                0x20,
                0,
                actor->alpha,
                0
            );
            j += 0x10;
        } while (j != limit);
        i += 0x10;
    } while (i <= 0x2F);

    if (actor->state < 3) {
        selected = 0;
    } else {
        selected = actor->state - 2;
        if (actor->state < 5) {
            if (gPlayerCount == 1) {
                selected = 1;
            } else {
                selected = 2;
            }
        } else if (selected >= 5) {
            selected = 4;
        }
    }
    drawMenuGlyphScript(actor->x, actor->y, gRaceSetupPlayerCountPromptText[selected], 0, actor->alpha, 0);

    if ((actor->state == 1) || (actor->state == 6)) {
        drawMenuSprite(
            (s16)(actor->x + 0xD0),
            (s16)(actor->y + 0x20),
            getRelocatableHeapBlockBase(TITLE_MENU_FRAME_TEXTURE_HANDLE),
            (actor->timer >= 8) + 5,
            0x20,
            0x20,
            0,
            0
        );
    }
}
