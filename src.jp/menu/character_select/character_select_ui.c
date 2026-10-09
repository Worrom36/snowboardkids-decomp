#include "common.h"
#include "game/menu/menu_scratch.h"
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
#define CHARACTER_SELECT_UI_UNUSED_HANDLE (gAssetHandles[0x1F])

extern CharacterSelectUiConfirmationBannerText gCharacterSelectConfirmationBannerText;
extern const char gCharacterSelectPlayerNumberFormat[];

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

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_ui/updateCharacterSelectConfirmationBanner.s")
// clang-format on

void initCharacterSelectConfirmationBanner(CharacterSelectUiBannerActor *arg0) {
    arg0->x = -0x70;
    arg0->y = -0x1C;
    arg0->alpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectConfirmationBanner);
}

void drawCharacterSelectPlayerPanelFrames(CharacterSelectUiPlayerPanelFrameActor *actor) {
    s32 i;
    u16 alpha;
    register CharacterSelectUiPlayerPanelFrameActor *panel;
    s32 playerNumber;
    volatile s32 unusedStackPadding;
    char text[8];

    for (i = 0; i < 4; i++) {
        if ((gPlayerCount - 1) < i) {
            alpha = 0x50;
        } else {
            alpha = 0x100;
        }

        panel = actor;
        playerNumber = i + 1;
        drawMenuSpriteWithAlpha(
            actor->x[i],
            actor->y[i],
            getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE),
            0x23,
            0x20,
            0x20,
            0,
            alpha,
            playerNumber
        );
        drawMenuSpriteWithAlpha(
            (s16)(actor->x[i] + 0x40),
            actor->y[i],
            getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE),
            0x24,
            0x20,
            0x20,
            0,
            alpha,
            playerNumber
        );
        drawMenuSpriteWithAlpha(
            (s16)(panel->x[i] + 0x80),
            panel->y[i],
            getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE),
            0xC,
            0x20,
            0x20,
            0,
            alpha,
            playerNumber
        );

        sprintf(text, gCharacterSelectPlayerNumberFormat, playerNumber);
        drawMenuAsciiText((s16)(actor->x[i] + 0x32), (s16)(panel->y[i] + 2), (u8 *)text, 0, alpha);

        if (alpha != 0x100) {
            if ((!actor) && (!actor)) {}
            drawMenuSpriteWithAlpha(
                (s16)(actor->x[i] + 2),
                (s16)(actor->y[i] + 0x14),
                getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_UNUSED_HANDLE),
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

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_ui/updateCharacterSelectPlayerPanelFrames.s")
// clang-format on

void initCharacterSelectPlayerPanelFrames(CharacterSelectUiPlayerPanelFrameActor *arg0) {
    arg0->x[0] = -0x88;
    arg0->y[0] = -0x60;
    arg0->x[1] = -0x88;
    arg0->y[1] = 4;
    arg0->x[2] = 4;
    arg0->y[2] = -0x60;
    arg0->x[3] = 4;
    arg0->y[3] = 4;
    arg0->frameLimit = 0x8C;
    arg0->frameStep = 0x44;
    arg0->transitionState = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectPlayerPanelFrames);
}

void drawCharacterSelectRosterIcons(CharacterSelectUiRosterIconActor *arg0) {
    s16 *assetHandles;
    s32 i;
    s32 j;
    s32 xOffset;
    u16 alpha;
    u16 tile;
    u8 textureIndex;
    s32 selectedAlpha;
    s32 tileTemp;

    // clang-format off
    i = 0; \
    if (arg0->state != 0) { \
        assetHandles = gAssetHandles; \
        xOffset = 0; \
        selectedAlpha = 0x60; \
        do { \
            alpha = 0x100; \
            j = 0; \
            if (gPlayerCount > 0) { \
                do { \
                    if ((i == gRacePlayers[j].selectedCharacterId) && (gRacePlayers[j].menuState != 0)) {
                        // clang-format on
                        tileTemp = i + 0x41;
                        alpha = 0x60;
                        tile = tileTemp;
                        textureIndex = 0x1F;
                        break;
                    }
                    j++;
                } while (j < gPlayerCount);
            }
            if (selectedAlpha != alpha) {
                textureIndex = 0x21;
                if (arg0->timer < 0xA) {
                    tileTemp = i + 0x37;
                    tile = tileTemp;
                } else {
                    tileTemp = i + 0x3D;
                    tile = tileTemp;
                }
            }
            drawMenuSpriteTile(
                (s16)(arg0->x + xOffset),
                arg0->y,
                getRelocatableHeapBlockBase(assetHandles[textureIndex]),
                tile,
                0,
                alpha
            );
            i++;
            xOffset += 0x20;
        } while (i < 5);

        if (arg0->leftSecretSlotUnlocked != 0) {
            alpha = 0x100;
            j = 0;
            if (gPlayerCount > 0) {
                do {
                    if ((gRacePlayers[j].selectedCharacterId == 5) && (gRacePlayers[j].menuState != 0)) {
                        alpha = 0x60;
                        tile = 0x46;
                        textureIndex = 0x1F;
                        break;
                    }
                    j++;
                } while (j < gPlayerCount);
            }
            if (selectedAlpha != alpha) {
                textureIndex = 0x21;
                if (arg0->timer < 0xA) {
                    tile = 0x3C;
                } else {
                    tile = 0x42;
                }
            }
            drawMenuSpriteTile(
                (s16)(arg0->x - 0x20),
                arg0->y,
                getRelocatableHeapBlockBase(assetHandles[textureIndex]),
                tile,
                0,
                alpha
            );
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_ui/func_80017404.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_ui/func_800174D0.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_ui/func_800175A4.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_ui/func_80017714.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_ui/func_8001784C.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_ui/drawCharacterSelectPlayerStatsPanels.s")
// clang-format on

void updateCharacterSelectPlayerStatsPanels(CharacterSelectUiPlayerStatsPanelActor *arg0) {
    s32 i;
    CharacterSelectUiPlayerStatsPanelActor *statsPanels;
    CharacterSelectUiPlayerPanelFrameActor *panelFrames;

    panelFrames = gMenuScratch1.playerFrames;
    statsPanels = arg0;
    for (i = 0; i != RACE_PLAYER_COUNT; i++) {
        statsPanels->x[i] = panelFrames->x[i];
        statsPanels->y[i] = panelFrames->y[i];
        if (gRacePlayers[i].menuState == 0) {
            statsPanels->blinkTimer[i] = (statsPanels->blinkTimer[i] + 1) % 20;
        }
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectPlayerStatsPanels, arg0);
}

void initCharacterSelectPlayerStatsPanels(CharacterSelectUiPlayerStatsPanelActor *arg0) {
    arg0->x[0] = -0x88;
    arg0->y[0] = -0x60;
    arg0->x[1] = -0x88;
    arg0->y[1] = 0x24;
    arg0->x[2] = 4;
    arg0->y[2] = -0x60;
    arg0->x[3] = 4;
    arg0->y[3] = 0x24;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectPlayerStatsPanels);
}

void drawCharacterSelectSelectedCharacterTokens(CharacterSelectUiSelectedCharacterTokenActor *arg0) {
    CharacterSelectUiSelectedCharacterTokenActor *tokens;
    s32 i;
    void *texture;
    u16 tileSize;

    tokens = arg0;
    i = 0;
    if (gPlayerCount > 0) {
        do {
            if (gRacePlayers[i].menuState != 0) {
                texture = getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE);
                tileSize = tokens->tileSize[i];
                drawMenuSprite(tokens->x[i], tokens->y[i], texture, 0xD, tileSize, tileSize, 0, 0);
            }
            i++;
        } while (i < gPlayerCount);
    }
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_ui/updateCharacterSelectSelectedCharacterTokens.s")
// clang-format on

void initCharacterSelectSelectedCharacterTokens(CharacterSelectUiSelectedCharacterTokenActor *arg0) {
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectSelectedCharacterTokens);
}
