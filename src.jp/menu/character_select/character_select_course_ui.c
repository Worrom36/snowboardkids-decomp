#include "common.h"
#include "game/menu/menu_scratch.h"
#include "game/race/race_state.h"
#include "font_encoding.h"
#include <PR/os_libc.h>
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/character_select/character_select_course_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/player/race_player_input.h"

#define CHARACTER_SELECT_FRAME_TEXTURE_HANDLE (gAssetHandles[0x21])
#define ASSET_HANDLE(index) (gAssetHandles[index])

typedef struct {
    u8 pad0[8];
    u8 next;
    u8 pad9[7];
    s16 xNext;
    s16 pad12;
    s16 pad14;
    s16 pad16;
    s16 pad18;
    s16 x0;
    s16 x1;
    s16 x2;
    u8 pad20[6];
    s16 y0;
    s16 y1;
    s16 y2;
    s16 y3;
} CharacterSelectCourseListInitLoop;

extern u8 gHighestUnlockedCourse;
extern MenuGlyphScript gCharacterSelectCourseStartGameText[44];

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001B510.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectUnlockedCourseList.s")
// clang-format on

void initCharacterSelectUnlockedCourseList(CharacterSelectCourseMenuFrameActor *arg0) {
    u8 spacing;
    s16 baseY;
    s32 i;
    int mode;
    unsigned int limit;
    CharacterSelectCourseListInitLoop *loop;

    mode = gHighestUnlockedCourse;
    if (mode == 0) {
        if (1) {}
        spacing = 0x18;
        baseY = -0x50;
    } else if (mode == 1) {
        spacing = 0x16;
        baseY = -0x54;
    } else {
        spacing = 0x13;
        if (mode == 2) {
            spacing = 0x16;
            baseY = -0x60;
        } else {
            baseY = -0x60;
        }
    }

    arg0->baseY = baseY;
    arg0->itemSpacing = spacing;
    // clang-format off
    do { \
        arg0->y[1] = baseY + spacing; \
        arg0->y[2] = baseY + (spacing * 2); \
        arg0->x[0] = -0x104; \
        arg0->x[1] = -0x104; \
        arg0->x[2] = -0x104; \
        arg0->x[2] = -0x104; \
        arg0->y[0] = baseY; \
        i = 3; \
        loop = (CharacterSelectCourseListInitLoop *)&((u8 *)&arg0->taskHeader)[6]; \
        limit = 0xB; \
        do { \
            loop->x0 = -0x104; \
            loop->x1 = -0x104; \
            loop->x2 = -0x104; \
            loop = (CharacterSelectCourseListInitLoop *)&loop->next; \
            loop->xNext = -0x104; \
            loop->y0 = baseY + (i * spacing); \
            loop->y1 = baseY + ((i + 1) * spacing); \
            loop->y2 = baseY + ((i + 2) * spacing); \
            loop->y3 = baseY + ((i + 3) * spacing); \
            i += 4; \
        } while (i != limit); \
    } while (0);
    // clang-format on

    arg0->timer = 0;
    arg0->itemCount = 1;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectUnlockedCourseList);
}

void drawCharacterSelectLimitedCourseListOptions(CharacterSelectCourseMenuFrameActor *arg0) {
    register CharacterSelectCourseMenuFrameActor *actor;
    void *arrowTexture;
    volatile u8 pad;
    u16 tile;
    s32 arrowBase;
    s32 lastArrowIndex;
    s32 i;
    s32 alpha;

    actor = arg0;
    arrowBase = (gRaceTypeSelection < 2) ? 2 : 0;
    i = 0;
    lastArrowIndex = arrowBase + 1;
    if (actor->itemCount > 0) {
        do {
            alpha = 0;
            if (i == 0) {
                tile = (gRaceTypeSelection == 2) ? 0x1D : 0x1C;
                if ((i == gRaceCourseIndex.signedValue) && (gMenuSelectionConfirmTimer > 0) &&
                    (gMenuSelectionConfirmTimer < 8) && (gMenuSelectionConfirmTimer & 1)) {
                    alpha = 0xFF;
                }
                drawMenuSprite(
                    actor->x[i],
                    actor->y[i],
                    getRelocatableHeapBlockBase(CHARACTER_SELECT_FRAME_TEXTURE_HANDLE),
                    tile,
                    0x20,
                    0x20,
                    0,
                    alpha
                );
            } else if (i == lastArrowIndex) {
                if ((i == gRaceCourseIndex.signedValue) && (gMenuSelectionConfirmTimer > 0) &&
                    (gMenuSelectionConfirmTimer < 8) && (gMenuSelectionConfirmTimer & 1)) {
                    alpha = 0xFF;
                }
                drawMenuSprite(
                    actor->x[i],
                    actor->y[i],
                    getRelocatableHeapBlockBase(CHARACTER_SELECT_FRAME_TEXTURE_HANDLE),
                    0x20,
                    0x20,
                    0x20,
                    0,
                    alpha
                );
            } else {
                if ((i == gRaceCourseIndex.signedValue) && (gMenuSelectionConfirmTimer > 0) &&
                    (gMenuSelectionConfirmTimer < 8) && (gMenuSelectionConfirmTimer & 1)) {
                    alpha = 0xFF;
                }
                tile = i + 0x12;
                drawMenuSprite(
                    actor->x[i],
                    actor->y[i],
                    getRelocatableHeapBlockBase(CHARACTER_SELECT_FRAME_TEXTURE_HANDLE),
                    tile,
                    0x20,
                    0x20,
                    0,
                    alpha
                );
            }

            if (i != lastArrowIndex) {
                arrowTexture = getRelocatableHeapBlockBase(CHARACTER_SELECT_FRAME_TEXTURE_HANDLE);
                tile = i + 0x29;
                drawMenuSprite((s16)(actor->x[i] - 0x10), actor->y[i], arrowTexture, tile, 0x20, 0x20, 0, 0);
            }
            i++;
        } while (i < actor->itemCount);
    }
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectLimitedCourseList.s")
// clang-format on

void initCharacterSelectLimitedCourseList(CharacterSelectCourseMenuFrameActor *arg0) {
    s32 i;

    arg0->baseY = -0x58;
    arg0->itemSpacing = 0x18;
    for (i = 0; i < 4; i++) {
        arg0->x[i] = -0x104;
        arg0->y[i] = arg0->baseY + (i * arg0->itemSpacing);
    }
    arg0->timer = 0;
    arg0->itemCount = 1;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectLimitedCourseList);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001CA90.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePreviewFrame.s")
// clang-format on

void initCharacterSelectCoursePreviewFrame(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = 0x96;
    arg0->y = -0x48;
    arg0->sprite.index = 0;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewFrame);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001D2AC.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePreviewPanel1.s")
// clang-format on

void initCharacterSelectCoursePreviewPanel1(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x1;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel1);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001D850.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePreviewPanel2.s")
// clang-format on

void initCharacterSelectCoursePreviewPanel2(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x2;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel2);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001DDE4.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePreviewPanel3.s")
// clang-format on

void initCharacterSelectCoursePreviewPanel3(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x3;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel3);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001E378.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePreviewPanel4.s")
// clang-format on

void initCharacterSelectCoursePreviewPanel4(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x4;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel4);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001E90C.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePreviewPanel5.s")
// clang-format on

void initCharacterSelectCoursePreviewPanel5(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x5;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel5);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001EEA0.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePreviewPanel6.s")
// clang-format on

void initCharacterSelectCoursePreviewPanel6(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x6;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel6);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001F434.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePreviewPanel7.s")
// clang-format on

void initCharacterSelectCoursePreviewPanel7(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x7;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel7);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001F9C8.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePreviewPanel8.s")
// clang-format on

void initCharacterSelectCoursePreviewPanel8(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x8;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel8);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_8001FF5C.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePreviewPanel9.s")
// clang-format on

void initCharacterSelectCoursePreviewPanel9(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x9;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel9);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_800204F0.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCourseExitPreviewPanel.s")
// clang-format on

void initCharacterSelectCourseExitPreviewPanel(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0;
    arg0->row.bytes.subTimer = 0;
    arg0->row.bytes.subState = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseExitPreviewPanel);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_80020A78.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_80020B08.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_80020C84.s")

void drawCharacterSelectCourseTitleCursor(CharacterSelectCourseWidgetActor *arg0) {
    drawMenuSpriteWithAlpha(
        arg0->x,
        arg0->y,
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)),
        1,
        0x20,
        0x20,
        0,
        arg0->sprite.index,
        0
    );
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCourseTitleCursor.s")
// clang-format on

void initCharacterSelectCourseTitleCursor(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x5C;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseTitleCursor);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_80020F18.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCourseStatsBadge.s")
// clang-format on

void initCharacterSelectCourseStatsBadge(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = 0x48;
    arg0->y = 0x34;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseStatsBadge);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_800213A8.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCoursePlayerStatsPanel.s")
// clang-format on

void initCharacterSelectCoursePlayerStatsPanel(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x108;
    arg0->y = -0x50;
    arg0->sprite.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePlayerStatsPanel);
}

void drawCharacterSelectCourseSubmenuFrame(CharacterSelectCourseWidgetActor *arg0) {
    drawMenuSprite(arg0->x, arg0->y, getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)), 3, 0x20, 0x20, 0, 0);
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        arg0->y,
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)),
        4,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        arg0->x,
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)),
        5,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)),
        6,
        0x20,
        0x20,
        0,
        0
    );
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCourseSubmenuFrame.s")
// clang-format on

void initCharacterSelectCourseSubmenuFrame(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x108;
    arg0->y = -0x18;
    arg0->sprite.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseSubmenuFrame);
}

void drawCharacterSelectCourseRecordsFrame(CharacterSelectCourseWidgetActor *arg0) {
    drawMenuSprite(arg0->x, arg0->y, getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)), 3, 0x20, 0x20, 0, 0);
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        arg0->y,
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)),
        4,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        arg0->x,
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)),
        5,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)),
        6,
        0x20,
        0x20,
        0,
        0
    );
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCourseRecordsFrame.s")
// clang-format on

void initCharacterSelectCourseRecordsFrame(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x128;
    arg0->y = 0x8;
    arg0->sprite.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseRecordsFrame);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_80021E40.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCourseNamePopup.s")
// clang-format on

void initCharacterSelectCourseNamePopup(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x84;
    arg0->y = -0x14;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseNamePopup);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_80022130.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCourseDescriptionPopup.s")
// clang-format on

void initCharacterSelectCourseDescriptionPopup(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x84;
    arg0->y = 0xC;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseDescriptionPopup);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_80022430.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCourseConfirmCursor.s")
// clang-format on

void initCharacterSelectCourseConfirmCursor(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x74;
    arg0->y = 0x2F;
    arg0->sprite.index = 0x2F;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseConfirmCursor);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_80022768.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCourseRecordsPopup.s")
// clang-format on

void initCharacterSelectCourseRecordsPopup(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0xF0;
    arg0->y = -0x40;
    arg0->sprite.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseRecordsPopup);
}

void drawCharacterSelectCourseExitPopup(CharacterSelectCourseWidgetActor *arg0) {
    drawMenuSprite(arg0->x, arg0->y, getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)), 0x28, 0x20, 0x20, 0, 0);
    if ((s32)arg0->transition.bytes.state > 0) {
        drawMenuGlyphScript((s16)(arg0->x + 4), (s16)(arg0->y + 4), gCharacterSelectCourseStartGameText, 1, 0x100, 0);
        drawMenuSprite(
            (s16)(arg0->x + 0x64),
            (s16)(arg0->y + 4),
            getRelocatableHeapBlockBase(ASSET_HANDLE(0x24)),
            (u16)((arg0->transition.bytes.timer >= 8) + 5),
            0x20,
            0x20,
            0,
            0
        );
    }
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/updateCharacterSelectCourseExitPopup.s")
// clang-format on

void initCharacterSelectCourseExitPopup(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x108;
    arg0->y = 0x28;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseExitPopup);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_800235A8.s")
