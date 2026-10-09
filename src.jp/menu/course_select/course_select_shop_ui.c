#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/course_select/course_select_shop_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/player/race_player_input.h"

typedef struct {
    u16 center[16];
    u16 right[2];
    u16 bottom[2];
    u16 corner;
} ShopMenuFrameTileMap;

typedef Vec2s ShopMenuSparklePattern[13];

enum {
    COURSE_DETAILS_PAGE_COUNT = 2,
    COURSE_DETAILS_COURSES_PER_PAGE = 7,
    COURSE_DETAILS_OPTION_COUNT = 10,
    COURSE_DETAILS_ROW_SPACING = 0x13
};
extern u8 gMenuExitSelection;
extern ShopMenuFrameTileMap gShopMenuPanelFrameTilemaps[2];
extern const char gShopMenuMoneyFormat[];
extern ShopMenuSparklePattern gShopMenuSparklePatterns[5];
extern s16 gShopMenuSparkleInitTable[16];
extern MenuGlyphScript gCourseUnlockPurchasePromptText[24];
extern Vec2s gCoursePreviewCloseSparkleOffsetsEnd[];
extern Vec2s gCoursePreviewCloseSparkleMirrorStart[2];
extern Vec2s gCoursePreviewCloseSparkleOffsets[6];
extern s16 gCoursePreviewViewportHeight;

void drawShopMenuModeChoiceRows(ShopMenuRowActor *actor) {
    ShopMenuRowActor *savedActor;
    ShopMenuRowActor *rows;
    s32 rowIndex;
    s32 blinkAlpha;
    s32 yOffset;

    savedActor = actor;
    rowIndex = 0;
    if (actor->visibleRowCount > 0) {
        /* Keeping the loop setup together preserves IDO's original scheduling. */
        // clang-format off
        yOffset = 0; rows = actor; do {
            // clang-format on
            blinkAlpha = 0;
            if ((gMenuSelectionConfirmTimer > 0) && (gMenuSelectionConfirmTimer < 8) && (gMenuExitSelection == 0) &&
                (rowIndex == gCourseSelectModeSelection) && (gMenuSelectionConfirmTimer & 1)) {
                blinkAlpha = 0xFF;
            }
            drawMenuSprite(
                rows->rowXPositions[rowIndex],
                (s16)(actor->baseY + yOffset),
                getRelocatableHeapBlockBase(gAssetHandles[0x27]),
                (u16)(rowIndex + 2),
                0x20,
                0x20,
                0,
                blinkAlpha
            );
            yOffset += 0x1C;
            rowIndex += 1;
        } while (rowIndex < savedActor->visibleRowCount);
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateShopMenuModeChoiceRows.s")

void initShopMenuModeChoiceRows(ShopMenuRowActor *arg0) {
    s32 i;

    // clang-format off
    for (i = 0; i < 5; i++) { arg0->rowXPositions[i] = -0x104; }
    // clang-format on

    arg0->baseY = -0x50;
    arg0->revealTimer = 0;
    arg0->visibleRowCount = 1;
    arg0->state = 0;

    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuModeChoiceRows);
}

void drawShopMenuSidePanel(ShopMenuWidgetActor *arg0) {
    drawMenuSprite(arg0->x, arg0->y, getRelocatableHeapBlockBase(gAssetHandles[0x25]), 3, 0x20, 0x20, 0, 0);
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        4,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        arg0->x,
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        5,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        6,
        0x20,
        0x20,
        0,
        0
    );
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateShopMenuSidePanel.s")

void initShopMenuSidePanel(ShopMenuWidgetActor *arg0) {
    arg0->x = -0x108;
    arg0->y = 8;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuSidePanel);
}

void drawShopMenuSelectedModePanel(ShopMenuWidgetActor *arg0) {
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
            getRelocatableHeapBlockBase(gAssetHandles[0x27]),
            gShopMenuPanelFrameTilemaps[(u16)arg0->item.counter].center[tileOffset],
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
            getRelocatableHeapBlockBase(gAssetHandles[0x27]),
            gShopMenuPanelFrameTilemaps[(u16)arg0->item.counter].right[tileOffset],
            0,
            0x100
        );
        i = 0x80;
        drawMenuSpriteTile(
            (s16)(arg0->x + offset),
            (s16)(arg0->y + 0x80),
            getRelocatableHeapBlockBase(gAssetHandles[0x27]),
            gShopMenuPanelFrameTilemaps[(u16)arg0->item.counter].bottom[tileOffset],
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
        getRelocatableHeapBlockBase(gAssetHandles[0x27]),
        gShopMenuPanelFrameTilemaps[(u16)arg0->item.counter].corner,
        0,
        0x100
    );

    drawMenuSprite(
        (s16)(arg0->x - 4),
        (s16)(arg0->y - 4),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        0x33,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x - 4),
        (s16)(arg0->y + 0x8C),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        0x38,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x8C),
        (s16)(arg0->y - 4),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        0x35,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x8C),
        (s16)(arg0->y + 0x8C),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
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
            getRelocatableHeapBlockBase(gAssetHandles[0x25]),
            0x34,
            0x20,
            0x20,
            0,
            0
        );
        drawMenuSprite(
            (s16)((arg0->x + borderOffset) + 0xC),
            (s16)(arg0->y + 0x8C),
            getRelocatableHeapBlockBase(gAssetHandles[0x25]),
            0x39,
            0x20,
            0x20,
            0,
            0
        );
        drawMenuSprite(
            (s16)(arg0->x - 4),
            (s16)((arg0->y + borderOffset) + 0xC),
            getRelocatableHeapBlockBase(gAssetHandles[0x25]),
            0x36,
            0x20,
            0x20,
            0,
            0
        );
        drawMenuSprite(
            (s16)(arg0->x + 0x8C),
            (s16)((arg0->y + borderOffset) + 0xC),
            getRelocatableHeapBlockBase(gAssetHandles[0x25]),
            0x37,
            0x20,
            0x20,
            0,
            0
        );
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateShopMenuSelectedModePanel.s")

void initShopMenuSelectedModePanel(ShopMenuWidgetActor *arg0) {
    arg0->x = 0x94;
    arg0->y = -0x48;
    arg0->sprite.index = -8;
    arg0->transition.alpha = -0x74;
    arg0->item.counter = 0;
    arg0->item.bytes.subTimer = 0;
    arg0->item.bytes.subState = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuSelectedModePanel);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002D604.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateShopMenuUnselectedModePanel.s")

void initShopMenuUnselectedModePanel(ShopMenuWidgetActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x140;
    arg0->sprite.index = 1;
    arg0->item.bytes.timer = 0;
    arg0->item.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuUnselectedModePanel);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002DB64.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateShopMenuCourseListPanel.s")

void initShopMenuCourseListPanel(ShopMenuWidgetActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x140;
    arg0->sprite.index = 2;
    arg0->item.bytes.timer = 0;
    arg0->item.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuCourseListPanel);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002E110.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateShopMenuModeCursor.s")

void initShopMenuModeCursor(ShopMenuWidgetActor *arg0) {
    arg0->x = -0x7C;
    arg0->y = -0x50;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    arg0->transition.bytes.timer = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuModeCursor);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002E364.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateShopMenuDescriptionText.s")

void initShopMenuDescriptionText(ShopMenuWidgetActor *arg0) {
    arg0->x = -0x84;
    arg0->y = 0xC;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuDescriptionText);
}

void drawShopMenuPromptPanel(ShopMenuWidgetActor *arg0) {
    drawMenuSpriteWithAlpha(
        arg0->x,
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x27]),
        0,
        0x20,
        0x20,
        0,
        arg0->sprite.index,
        0
    );
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateShopMenuPromptPanel.s")

void initShopMenuPromptPanel(ShopMenuWidgetActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x5C;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuPromptPanel);
}

void drawShopMenuMoneyPanel(ShopMenuWidgetActor *arg0) {
    char sp40[0x18];

    drawMenuPanelBackdrop(arg0->x, arg0->y, 0x5000, 0x4000);
    drawMenuSpriteWithAlpha(
        (s16)(arg0->x + 8),
        (s16)(arg0->y + 4),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        0x11,
        0x20,
        0x20,
        0,
        arg0->sprite.index,
        0
    );
    sprintf(sp40, gShopMenuMoneyFormat, gRacePlayers[0].money);
    drawMenuAsciiText((s16)(arg0->x + 0x10), (s16)(arg0->y + 0x10), sp40, 0, arg0->sprite.index);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002E7E4.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002E8E4.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002E920.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002EA20.s")

void drawShopMenuSparkles(ShopMenuWidgetActor *arg0) {
    s32 patternOffset;
    s32 tileIndex;
    ShopMenuWidgetActor *drawActor;
    ShopMenuWidgetActor *actor;

    actor = arg0;
    tileIndex = 0;
    if (gShopMenuSparklePatterns[actor->sparkle.patternIndex][0].x != -1) {
        patternOffset = 0;
        do {
            drawActor = actor;
            drawMenuSpriteTile(
                (s16)(drawActor->randomValues[0] +
                      gShopMenuSparklePatterns[drawActor->sparkle.patternIndex][patternOffset].x),
                (s16)(drawActor->randomValues[1] +
                      gShopMenuSparklePatterns[drawActor->sparkle.patternIndex][patternOffset].y),
                getRelocatableHeapBlockBase(gAssetHandles[0x27]),
                tileIndex + drawActor->sparkle.tileBase,
                0,
                drawActor->sparkle.alpha
            );
            patternOffset++;
            tileIndex++;
        } while (gShopMenuSparklePatterns[actor->sparkle.patternIndex][patternOffset].x != -1);
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateShopMenuSparkles.s")

void initShopMenuSparkles(ShopMenuWidgetActor *arg0) {
    s16 *entry;

    arg0->sparkle.patternIndex = gRacePlayers[0].selectedCharacterId;
    entry = &gShopMenuSparkleInitTable[arg0->sparkle.patternIndex * 3];
    arg0->sprite.index = entry[0];
    arg0->x = 0x94;
    arg0->y = entry[1];
    arg0->sparkle.tileBase = entry[2];
    arg0->sparkle.alpha = 0x100;
    arg0->slide.slideState = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateShopMenuSparkles);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002ECC8.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002EE9C.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/initCourseUnlockPricePanel.s")

void drawCourseUnlockPurchasePrompt(ShopMenuWidgetActor *arg0) {
    volatile u64 unused;
    u32 drawAlpha;
    u16 alpha;

    drawMenuSprite(arg0->x, arg0->y, getRelocatableHeapBlockBase(gAssetHandles[0x24]), 0, 0x20, 0x20, 0, 0);
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        1,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x78),
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        1,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0xB0),
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        2,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuGlyphScript((s16)(arg0->x + 0x30), (s16)(arg0->y + 4), gCourseUnlockPurchasePromptText, 0, 0x100, 0);

    if (arg0->item.price == 0) {
        alpha = 0x100;
    } else {
        alpha = 0x60;
    }

    drawAlpha = alpha;
    drawMenuSpriteWithAlpha(
        (s16)(arg0->x + 0x50),
        (s16)(arg0->y + 0x14),
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        0x17,
        0x20,
        0x20,
        0,
        drawAlpha,
        0
    );

    if (drawAlpha == 0x100) {
        alpha = 0x60;
    } else {
        alpha = 0x100;
    }

    drawMenuSpriteWithAlpha(
        (s16)(arg0->x + 0x50),
        (s16)(arg0->y + 0x24),
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        0x18,
        0x20,
        0x20,
        0,
        alpha,
        0
    );
    drawMenuSpriteWithAlpha(
        (s16)(arg0->x + 0x50),
        (s16)(arg0->y + (arg0->item.price * 0x10) + 0x14),
        getRelocatableHeapBlockBase(gAssetHandles[0x24]),
        0x12,
        0x20,
        0x20,
        0,
        arg0->sprite.index,
        0
    );
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateCourseUnlockPurchasePrompt.s")

void initCourseUnlockPurchasePrompt(ShopMenuWidgetActor *arg0) {
    arg0->x = -0x76;
    arg0->y = -0x40;
    arg0->item.price = 0;
    arg0->sprite.index = 0x100;
    arg0->transition.counter = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseUnlockPurchasePrompt);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002F4B4.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateCourseDetailsMenu.s")

void initCourseDetailsMenu(ShopMenuWidgetActor *arg0) {
    s32 i;

    for (i = 0; i < COURSE_DETAILS_OPTION_COUNT; i++) {
        arg0->cursorPositions[i] = -0xFC;
    }

    arg0->targetX = -0x7C;
    arg0->targetY = -0x5C;
    arg0->prompt.bytes.pulseTimer = 0;
    arg0->spawnTimer = 0;
    arg0->visibleCount = 1;
    arg0->prompt.bytes.pulseAlpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseDetailsMenu);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/func_8002FD94.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_shop_ui/updateCourseDetailsPreviewTile.s")

void initCourseDetailsPreviewTile(ShopMenuWidgetActor *arg0) {
    arg0->x = 0x90;
    arg0->y = 0x44;
    arg0->sprite.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseDetailsPreviewTile);
}

void drawCoursePreviewCloseSparkles(ShopMenuWidgetActor *arg0) {
    Vec2s *offset;
    ShopMenuWidgetActor *actor;
    ShopMenuWidgetActor *counterActor;
    void *texture;

    // clang-format off
    actor = arg0; offset = gCoursePreviewCloseSparkleOffsets; counterActor = arg0; do {
        // clang-format on
        if (offset < gCoursePreviewCloseSparkleMirrorStart) {
            texture = getRelocatableHeapBlockBase(gAssetHandles[0x1C]);
            drawMenuSprite(
                (s16)(offset->x + actor->x),
                (s16)(offset->y + actor->sprite.index - 8),
                texture,
                counterActor->transition.counter + 0x39,
                0x20,
                0x20,
                0,
                0
            );
        } else {
            texture = getRelocatableHeapBlockBase(gAssetHandles[0x1C]);
            drawMenuSprite(
                (s16)(offset[-6].x + actor->y),
                (s16)((offset[-6].y + actor->sprite.index) * -1 - 8),
                texture,
                counterActor->transition.counter + 0x39,
                0x20,
                0x20,
                0,
                0
            );
        }
        offset++;
        counterActor = (ShopMenuWidgetActor *)&counterActor->pad0[2];
    } while (offset != gCoursePreviewCloseSparkleOffsetsEnd);
}

void updateCoursePreviewCloseSparkles(ShopMenuWidgetActor *arg0) {
    s32 i;

    arg0->sprite.index = gCoursePreviewViewportHeight / 2;
    arg0->x++;
    arg0->y--;

    for (i = 0; i < 12; i += 4) {
        arg0->sparkle.counters[i] = (arg0->sparkle.counters[i] + 1) % 6;
        arg0->sparkle.counters[i + 1] = (arg0->sparkle.counters[i + 1] + 1) % 6;
        arg0->sparkle.counters[i + 2] = (arg0->sparkle.counters[i + 2] + 1) % 6;
        arg0->sparkle.counters[i + 3] = (arg0->sparkle.counters[i + 3] + 1) % 6;
    }

    if (arg0->sprite.index == 0) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCoursePreviewCloseSparkles, arg0);
}

void initCoursePreviewCloseSparkles(ShopMenuWidgetActor *arg0) {
    ShopMenuWidgetActor *new_var;
    s32 i;

    arg0->x = 5;
    arg0->y = 0x48;
    new_var = arg0;
    new_var->sprite.index = gCoursePreviewViewportHeight / 2;

    for (i = 0; i < 12; i++) {
        new_var->randomValues[i + 3] = randomNextMain() % 6;
    }

    setCallbackTaskCallback(new_var, (CallbackTaskCallback)updateCoursePreviewCloseSparkles);
}
