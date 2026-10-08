#include "game/menu/menu_scratch.h"
#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/character_select/character_select_ui.h"
#include "game/race/player/race_player_input.h"

#define CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE (gAssetHandles[0x21])
#define CHARACTER_SELECT_UI_BANNER_TEXTURE_HANDLE (gAssetHandles[0x29])

extern CharacterSelectUiConfirmationBannerText gCharacterSelectConfirmationBannerText;

void drawCharacterSelectConfirmationBanner(CharacterSelectUiBannerActor *arg0) {
    s32 i;
    s32 j;
    unsigned int alpha;
    s32 selected;
    s32 limit;
    CharacterSelectUiBannerActor *actor;

    actor = arg0;
    if (actor->state != 3) {
        drawMenuSpriteWithAlpha(
            (s16)(actor->x - 4),
            (s16)(actor->y - 4),
            getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_BANNER_TEXTURE_HANDLE),
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
            getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_BANNER_TEXTURE_HANDLE),
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
                getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_BANNER_TEXTURE_HANDLE),
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
                getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_BANNER_TEXTURE_HANDLE),
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
            getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_BANNER_TEXTURE_HANDLE),
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
            getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_BANNER_TEXTURE_HANDLE),
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
                getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_BANNER_TEXTURE_HANDLE),
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
                getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_BANNER_TEXTURE_HANDLE),
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
                    getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_BANNER_TEXTURE_HANDLE),
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
            selected = 1;
        }
        drawMenuGlyphScript(
            actor->x,
            actor->y,
            gCharacterSelectConfirmationBannerText.confirmationText[selected],
            0,
            actor->alpha,
            0
        );
        if (actor->state == 4) {
            if (actor->alpha != 0x100) {
                alpha = actor->alpha & 0xFFFF;
            } else {
                if (actor->mode != 0) {
                    alpha = 0x60;
                } else {
                    alpha = 0x100;
                }
            }
            drawMenuSpriteWithAlpha(
                (s16)(actor->x + 0x4C),
                (s16)(actor->y + 0x10),
                getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE),
                0x17,
                0x20,
                0x20,
                0,
                alpha,
                0
            );
            if (actor->alpha < 0x60) {
                alpha = actor->alpha & 0xFFFF;
            } else {
                if (actor->mode != 0) {
                    alpha = 0x100;
                } else {
                    alpha = 0x60;
                }
            }
            drawMenuSpriteWithAlpha(
                (s16)(actor->x + 0x4C),
                (s16)(actor->y + 0x20),
                getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE),
                0x18,
                0x20,
                0x20,
                0,
                alpha,
                0
            );
            if (actor->alpha != 0x100) {
                alpha = actor->alpha & 0xFFFF;
                ;
            } else {
                alpha = (u16)actor->confirmationCursorAlpha;
            }
            drawMenuSpriteWithAlpha(
                (s16)(actor->x + 0x4C),
                (s16)((actor->y + (actor->mode * 0x10)) + 0x10),
                getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE),
                0x12,
                0x20,
                0x20,
                0,
                alpha,
                0
            );
        }
        if (actor->state == 1) {
            drawMenuSprite(
                (s16)(actor->x + 0xD0),
                (s16)(actor->y + 0x20),
                getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE),
                (actor->frame >= 8) + 5,
                0x20,
                0x20,
                0,
                0
            );
        }
    }
}
