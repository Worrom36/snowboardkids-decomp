#include "common.h"
#include "game/race/race_state.h"
#include "font_encoding.h"
#include <PR/os_libc.h>
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/system_runtime.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/player/race_player_input.h"

#define RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE (gAssetHandles[0x21])
#define ASSET_HANDLE(index) (gAssetHandles[(index)])

extern u8 gMenuExitSelection;
extern const char gRaceSplitscreenSelectEntryFeeFormat[];

void drawRaceSplitscreenSelectPlayerCountIcons(RaceSplitscreenSelectOptionIconActor *icons) {
    RaceSplitscreenSelectOptionIconActor *savedIcons;
    s32 iconIndex;
    s32 blinkAlpha;
    s32 yOffset;
    u16 tileIndex;

    savedIcons = icons;
    iconIndex = 0;
    if (iconIndex < (s32)icons->visibleCount) {
        /*
         * The outer do/while (0), repeated iconIndex initialization, empty if,
         * and paired statements must retain this shape for IDO's register
         * allocation. The icon position itself is nevertheless a normal array
         * access; the compiler converts it to the pointer walk in the ROM.
         */
        // clang-format off
        do { iconIndex = 0; yOffset = 0; do {
                // clang-format on
                blinkAlpha = 0;
                if (((((gMenuSelectionConfirmTimer != 0) && (((s32)gMenuSelectionConfirmTimer) < 8)) &&
                      (gMenuExitSelection == 0)) &&
                     (iconIndex == gRaceSplitscreenMode)) &&
                    (gMenuSelectionConfirmTimer & 1)) {
                    blinkAlpha = 0xFF;
                }
                tileIndex = iconIndex + 8;
                drawMenuSprite(
                    icons->x[iconIndex],
                    (s16)(icons->y + yOffset),
                    getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
                    tileIndex,
                    0x20,
                    0x20,
                    0,
                    blinkAlpha
                );
                yOffset += 0x14;
                iconIndex++;
                if (icons->visibleCount) {}
            } while (iconIndex < ((s32)savedIcons->visibleCount));
        } while (0);
    }
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectPlayerCountIcons.s")
// clang-format on

void initRaceSplitscreenSelectPlayerCountIcons(RaceSplitscreenSelectOptionIconActor *arg0) {
    s32 i;

    // clang-format off
    for (i = 0; i < 5; i++) { arg0->x[i] = -0x104; }
    // clang-format on

    arg0->y = -0x60;
    arg0->spawnTimer = 0;
    arg0->visibleCount = 1;
    arg0->state = 0;

    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectPlayerCountIcons);
}

void drawRaceSplitscreenSelectCornerSprites(RaceSplitscreenSelectCornerSpriteActor *arg0) {
    drawMenuSprite(arg0->x, arg0->y, getRelocatableHeapBlockBase(ASSET_HANDLE(33)), 3, 0x20, 0x20, 0, 0);
    drawMenuSprite((s16)(arg0->x + 0x40), arg0->y, getRelocatableHeapBlockBase(ASSET_HANDLE(33)), 4, 0x20, 0x20, 0, 0);
    drawMenuSprite(arg0->x, (s16)(arg0->y + 0x40), getRelocatableHeapBlockBase(ASSET_HANDLE(33)), 5, 0x20, 0x20, 0, 0);
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(ASSET_HANDLE(33)),
        6,
        0x20,
        0x20,
        0,
        0
    );
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectCornerSprites.s")
// clang-format on

void initRaceSplitscreenSelectCornerSprites(RaceSplitscreenSelectCornerSpriteActor *arg0) {
    arg0->x = -0x108;
    arg0->y = 8;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectCornerSprites);
}

void drawRaceSplitscreenSelectOption0Frame(RaceSplitscreenSelectOption0FrameActor *arg0) {
    s32 shouldDraw;
    s32 i;
    s32 tileOffset;
    s32 offset;
    s32 borderOffset;

    tileOffset = 0;
    shouldDraw = 1;
    for (i = 0; i < 16; i++, tileOffset++) {
        drawMenuSpriteTile(
            (s16)(arg0->x + ((i & 3) << 5)),
            (s16)(arg0->y + ((i / 4) << 5)),
            getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
            gRaceSplitscreenSelectFrameTiles[(u16)arg0->optionIndex].centerTiles[tileOffset],
            0,
            0x100
        );
    }

    if (shouldDraw) {
        tileOffset = 0;
        i = 0x80;
    }
    offset = 0;
    if ((i && i) && i) {}
    do {
        drawMenuSpriteTile(
            (s16)(arg0->x + 0x80),
            (s16)(arg0->y + offset),
            getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
            gRaceSplitscreenSelectFrameTiles[(u16)arg0->optionIndex].rightEdgeTiles[tileOffset],
            0,
            0x100
        );
        i = 0x80;
        drawMenuSpriteTile(
            (s16)(arg0->x + offset),
            (s16)(arg0->y + 0x80),
            getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
            gRaceSplitscreenSelectFrameTiles[(u16)arg0->optionIndex].bottomEdgeTiles[tileOffset],
            0,
            0x100
        );
        offset += 0x40;
        tileOffset++;
    } while (offset < 0x80);
    i++;
    i--;

    drawMenuSpriteTile(
        (s16)(arg0->x + 0x80),
        (s16)(arg0->y + 0x80),
        getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
        gRaceSplitscreenSelectFrameTiles[(u16)arg0->optionIndex].cornerTile,
        0,
        0x100
    );

    drawMenuSprite(
        arg0->x - 4,
        (s16)(arg0->y - 4),
        getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
        0x33,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x - 4),
        (s16)(arg0->y + 0x8C),
        getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
        0x38,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x8C),
        (s16)(arg0->y - 4),
        getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
        0x35,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x8C),
        (s16)(arg0->y + 0x8C),
        getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
        0x3A,
        0x20,
        0x20,
        0,
        0
    );

    for (borderOffset = 0; borderOffset != 0x80; borderOffset += 0x10) {
        drawMenuSprite(
            (s16)((arg0->x + borderOffset) + 0xC),
            (s16)(arg0->y - 4),
            getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
            0x34,
            0x20,
            0x20,
            0,
            0
        );
        drawMenuSprite(
            (s16)((arg0->x + borderOffset) + 0xC),
            (s16)(arg0->y + 0x8C),
            getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
            0x39,
            0x20,
            0x20,
            0,
            0
        );
        drawMenuSprite(
            (s16)(arg0->x - 4),
            (s16)((arg0->y + borderOffset) + 0xC),
            getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
            0x36,
            0x20,
            0x20,
            0,
            0
        );
        drawMenuSprite(
            (s16)(arg0->x + 0x8C),
            (s16)((arg0->y + borderOffset) + 0xC),
            getRelocatableHeapBlockBase(RACE_SPLITSCREEN_SELECT_TEXTURE_HANDLE),
            0x37,
            0x20,
            0x20,
            0,
            0
        );
    }
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectOption0Frame.s")
// clang-format on

void initRaceSplitscreenSelectOption0Frame(RaceSplitscreenSelectOption0FrameActor *arg0) {
    arg0->x = 0x94;
    arg0->y = -0x48;
    arg0->unused1C = -8;
    arg0->unused1E = -0x74;
    arg0->optionIndex = 0;
    arg0->cornerSpawnTimer = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectOption0Frame);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/func_8001946C.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectOption1Frame.s")
// clang-format on

void initRaceSplitscreenSelectOption1Frame(RaceSplitscreenSelectOptionFrameActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x140;
    arg0->optionIndex = 1;
    arg0->unusedTimer = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectOption1Frame);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/func_800199CC.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectOption2Frame.s")
// clang-format on

void initRaceSplitscreenSelectOption2Frame(RaceSplitscreenSelectOptionFrameActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x140;
    arg0->optionIndex = 2;
    arg0->unusedTimer = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectOption2Frame);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/func_80019F2C.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectOption3Frame.s")
// clang-format on

void initRaceSplitscreenSelectOption3Frame(RaceSplitscreenSelectOptionFrameActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x140;
    arg0->optionIndex = 3;
    arg0->unusedTimer = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectOption3Frame);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/func_8001A48C.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectOption4Frame.s")
// clang-format on

void initRaceSplitscreenSelectOption4Frame(RaceSplitscreenSelectOptionFrameActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x140;
    arg0->optionIndex = 4;
    arg0->unusedTimer = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectOption4Frame);
}

void drawRaceSplitscreenSelectCursor(RaceSplitscreenSelectAnimatedWidgetActor *arg0) {
    drawMenuSpriteWithAlpha(
        arg0->x,
        (s16)(arg0->y + (gRaceSplitscreenMode * 0x14)),
        getRelocatableHeapBlockBase(ASSET_HANDLE(33)),
        7,
        0x20,
        0x20,
        0,
        arg0->alpha,
        0
    );
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectCursor.s")
// clang-format on

void initRaceSplitscreenSelectCursor(RaceSplitscreenSelectAnimatedWidgetActor *arg0) {
    arg0->x = -0x7C;
    arg0->y = -0x60;
    arg0->alpha = 0;
    arg0->state = 0;
    arg0->animationTimer = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectCursor);
}

void drawRaceSplitscreenSelectPortrait(RaceSplitscreenSelectAnimatedWidgetActor *arg0) {
    s32 portraitIndex;

    if (gRaceSplitscreenMode == 3) {
        portraitIndex = gRaceSplitscreenMode & 0xFF;
        if (gRacePlayers[0].selectedCharacterId == 5) {
            portraitIndex = 5;
        }
    } else {
        portraitIndex = gRaceSplitscreenMode & 0xFF;
    }
    drawMenuGlyphScript(
        arg0->x,
        arg0->y,
        gRaceSplitscreenSelectDescriptionScripts.descriptions[portraitIndex],
        1,
        arg0->alpha,
        0
    );
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectPortrait.s")
// clang-format on

void initRaceSplitscreenSelectPortrait(RaceSplitscreenSelectAnimatedWidgetActor *arg0) {
    arg0->x = -0x84;
    arg0->y = 0xC;
    arg0->alpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectPortrait);
}

void drawRaceSplitscreenSelectArrowPrompt(RaceSplitscreenSelectAnimatedWidgetActor *arg0) {
    drawMenuSpriteWithAlpha(
        arg0->x,
        arg0->y,
        getRelocatableHeapBlockBase(ASSET_HANDLE(33)),
        0,
        0x20,
        0x20,
        0,
        arg0->alpha,
        0
    );
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectArrowPrompt.s")
// clang-format on

void initRaceSplitscreenSelectArrowPrompt(RaceSplitscreenSelectAnimatedWidgetActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x5C;
    arg0->alpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectArrowPrompt);
}

void drawRaceSplitscreenSelectEntryFee(RaceSplitscreenSelectAnimatedWidgetActor *arg0) {
    char sp40[0x18];

    drawMenuPanelBackdrop(arg0->x, arg0->y, 0x5000, 0x4000);
    drawMenuSpriteWithAlpha(
        (s16)(arg0->x + 8),
        (s16)(arg0->y + 4),
        getRelocatableHeapBlockBase(ASSET_HANDLE(33)),
        0x11,
        0x20,
        0x20,
        0,
        arg0->alpha,
        0
    );
    sprintf(sp40, gRaceSplitscreenSelectEntryFeeFormat, gRacePlayers[0].money);
    drawMenuAsciiText((s16)(arg0->x + 0x10), (s16)(arg0->y + 0x10), sp40, 0, arg0->alpha);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/splitscreen_select/race_splitscreen_select_ui/updateRaceSplitscreenSelectEntryFee.s")
// clang-format on

void initRaceSplitscreenSelectEntryFee(RaceSplitscreenSelectAnimatedWidgetActor *arg0) {
    arg0->x = 0x30;
    arg0->y = 0x40;
    arg0->alpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSplitscreenSelectEntryFee);
}

void drawMenuPanelBackdrop(s32 x, s32 y, s32 width, s32 height) {
    /* These scratch declarations and their scopes reproduce IDO's texture-load spill layout. */
    Gfx *volatile unused0;
    Gfx *volatile unused1;
    Gfx *volatile unused2;
    register s32 widthU;
    register s32 heightU;
    s32 ulx;
    s32 uly;

    gDPPipeSync(gRegionAllocPtr++);
    gDPSetTextureLUT(gRegionAllocPtr++, G_TT_NONE);
    gDPSetCombineMode(gRegionAllocPtr++, G_CC_MODULATEI_PRIM, G_CC_MODULATEI_PRIM);
    gDPSetRenderMode(gRegionAllocPtr++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        gMenuPanelBackdropTexture,
        G_IM_FMT_I,
        16,
        8,
        0,
        0,
        16,
        8,
        0,
        G_TX_NOMIRROR | G_TX_CLAMP,
        G_TX_NOMIRROR | G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 0, 0, 0, 0x64);

    /* The repeated cast assignment is required for the original register allocation. */
    widthU = (u16)width;
    widthU = (u16)width;
    heightU = (u16)height;
    ulx = ((s16)x + gMenuViewportCenterX) << 2;
    uly = ((s16)y + gMenuViewportCenterY) << 2;

    gSPTextureRectangle(
        gRegionAllocPtr++,
        ulx,
        uly,
        ulx + (((widthU << 4) << 2) / 0x1000),
        (((heightU << 3) << 2) / 0x1000) + uly,
        G_TX_RENDERTILE,
        0,
        0,
        (0x400000 / widthU) & 0xFFFFU,
        (0x400000 / (u16)height) & 0xFFFF
    );
    gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
    { Gfx *volatile unused3; }
}
