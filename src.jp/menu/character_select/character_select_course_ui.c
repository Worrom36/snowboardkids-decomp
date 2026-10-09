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

typedef MenuGlyphScript CharacterSelectCourseText[0x4A];

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

extern s32 gMenuFlowState;

extern CharacterSelectCourseText gCharacterSelectCourseNameText[10];

extern u8 gCourseSelectFromRaceTypeMenu;

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

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePreviewFrame.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePreviewFrame(CharacterSelectCourseWidgetActor *arg0) {
    u8 state;
    u8 *new_var;
    int switchState;
    u32 selectedSpriteIndex;
    int menuState;
    register s32 selectedIndex;
    register u16 spriteIndex;

    selectedIndex = gRaceCourseIndex.signedValue;
    spriteIndex = (u16)arg0->sprite.index;
    selectedSpriteIndex = spriteIndex;
    if ((selectedIndex >= (s32)selectedSpriteIndex) && (arg0->transition.bytes.state != 0) && (arg0->y != -0x48)) {
        switchState = state = arg0->transition.bytes.state = 2;
    } else {
        switchState = state = arg0->transition.bytes.state;
        if ((selectedIndex < (s32)selectedSpriteIndex) && (switchState != 0) && (arg0->y != -0x140)) {
            switchState = state = arg0->transition.bytes.state = 1;
        } else if ((switchState != 0) && (switchState < 4)) {
            switchState++;
            switchState--;
            switchState = state = arg0->transition.bytes.state = 3;
        }
    }

    if (switchState != gCharacterSelectCourseCursorState.previewFrameState) {
        switchState = state = arg0->transition.bytes.state =
            gCharacterSelectCourseCursorState.previewFrameState;
    }

    switch (switchState) {
        case 0:
            arg0->x -= 0x26;
            if (arg0->x < -7) {
                arg0->x = -8;
                arg0->transition.bytes.state = 3;
                gActiveMenuTask =
                    createCallbackTask((CallbackTaskCallback)initCharacterSelectCourseListCursor, 0, 0x64);
                createCallbackTask((CallbackTaskCallback)initCharacterSelectCourseTitleCursor, 0, 0x62);
                createCallbackTask((CallbackTaskCallback)initCharacterSelectCourseStatsBadge, 0, 0x62);
            }
            state = arg0->transition.bytes.state;
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            gMenuFlowState += 1;
            menuState = gRacePlayers[0].menuState;
            new_var = &gRacePlayers[0].menuState;
            if (menuState == 1) {
                arg0->transition.bytes.state = 4;
                menuState = *new_var;
            }
            if (menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 3;
                gRacePlayers[0].menuState = 6;
            }
            if (gRacePlayers[0].menuState == 7) {
                state = 6 & 0xFF;
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x += 0x20;
            if (arg0->x >= 0x94) {
                arg0->transition.bytes.state = 6;
            }
            state = arg0->transition.bytes.state;
            break;
        case 6:
            arg0->transition.bytes.state = 7;
            gRacePlayers[0].menuState = 8;
            state = arg0->transition.bytes.state;
            break;
    }

    gCharacterSelectCourseCursorState.previewFrameState = state;
    if (arg0->transition.bytes.state == 7) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePreviewFrame, arg0);
}
// clang-format on

void initCharacterSelectCoursePreviewFrame(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = 0x96;
    arg0->y = -0x48;
    arg0->sprite.index = 0;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewFrame);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePreviewPanel1.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePreviewPanel1(CharacterSelectCourseWidgetActor *arg0) {
    int state;
    u8 *stateField = &arg0->transition.bytes.state;

    if ((gRaceCourseIndex.signedValue >= (s32)(u16)arg0->sprite.index) && (arg0->y != -0x48)) {
        state = arg0->transition.bytes.state = 2;
    } else if ((gRaceCourseIndex.signedValue < (s32)(u16)arg0->sprite.index) && (arg0->y != -0x140)) {
        state = arg0->transition.bytes.state = 1;
    } else {
        state = *stateField;
        if (state < 4) {
            state = arg0->transition.bytes.state = 3;
        }
    }

    switch (state) {
        case 0:
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            gMenuFlowState += 1;
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 4;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 3;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x += 0x20;
            state = *stateField;
            break;
        case 6:
            state = arg0->transition.bytes.state = 7;
            break;
    }

    if ((state == 5) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePreviewPanel1, arg0);
}
// clang-format on

void initCharacterSelectCoursePreviewPanel1(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x1;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel1);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePreviewPanel2.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePreviewPanel2(CharacterSelectCourseWidgetActor *arg0) {
    int state;
    u8 *stateField = &arg0->transition.bytes.state;

    if ((gRaceCourseIndex.signedValue >= (s32)(u16)arg0->sprite.index) && (arg0->y != -0x48)) {
        state = arg0->transition.bytes.state = 2;
    } else if ((gRaceCourseIndex.signedValue < (s32)(u16)arg0->sprite.index) && (arg0->y != -0x140)) {
        state = arg0->transition.bytes.state = 1;
    } else {
        state = *stateField;
        if (state < 4) {
            state = arg0->transition.bytes.state = 3;
        }
    }

    switch (state) {
        case 0:
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            gMenuFlowState += 1;
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 4;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 3;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x += 0x20;
            state = *stateField;
            break;
    }

    if ((state == 5) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePreviewPanel2, arg0);
}
// clang-format on

void initCharacterSelectCoursePreviewPanel2(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x2;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel2);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePreviewPanel3.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePreviewPanel3(CharacterSelectCourseWidgetActor *arg0) {
    int state;
    u8 *stateField = &arg0->transition.bytes.state;

    if ((gRaceCourseIndex.signedValue >= (s32)(u16)arg0->sprite.index) && (arg0->y != -0x48)) {
        state = arg0->transition.bytes.state = 2;
    } else if ((gRaceCourseIndex.signedValue < (s32)(u16)arg0->sprite.index) && (arg0->y != -0x140)) {
        state = arg0->transition.bytes.state = 1;
    } else {
        state = *stateField;
        if (state < 4) {
            state = arg0->transition.bytes.state = 3;
        }
    }

    switch (state) {
        case 0:
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            gMenuFlowState += 1;
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 4;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 3;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x += 0x20;
            state = *stateField;
            break;
    }

    if ((state == 5) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePreviewPanel3, arg0);
}
// clang-format on

void initCharacterSelectCoursePreviewPanel3(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x3;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel3);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePreviewPanel4.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePreviewPanel4(CharacterSelectCourseWidgetActor *arg0) {
    int state;
    u8 *stateField = &arg0->transition.bytes.state;

    if ((gRaceCourseIndex.signedValue >= (s32)(u16)arg0->sprite.index) && (arg0->y != -0x48)) {
        state = arg0->transition.bytes.state = 2;
    } else if ((gRaceCourseIndex.signedValue < (s32)(u16)arg0->sprite.index) && (arg0->y != -0x140)) {
        state = arg0->transition.bytes.state = 1;
    } else {
        state = *stateField;
        if (state < 4) {
            state = arg0->transition.bytes.state = 3;
        }
    }

    switch (state) {
        case 0:
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            gMenuFlowState += 1;
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 4;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 3;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x += 0x20;
            state = *stateField;
            break;
    }

    if ((state == 5) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePreviewPanel4, arg0);
}
// clang-format on

void initCharacterSelectCoursePreviewPanel4(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x4;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel4);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePreviewPanel5.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePreviewPanel5(CharacterSelectCourseWidgetActor *arg0) {
    int state;
    u8 *stateField = &arg0->transition.bytes.state;

    if ((gRaceCourseIndex.signedValue >= (s32)(u16)arg0->sprite.index) && (arg0->y != -0x48)) {
        state = arg0->transition.bytes.state = 2;
    } else if ((gRaceCourseIndex.signedValue < (s32)(u16)arg0->sprite.index) && (arg0->y != -0x140)) {
        state = arg0->transition.bytes.state = 1;
    } else {
        state = *stateField;
        if (state < 4) {
            state = arg0->transition.bytes.state = 3;
        }
    }

    switch (state) {
        case 0:
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            gMenuFlowState += 1;
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 4;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 3;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x += 0x20;
            state = *stateField;
            break;
    }

    if ((state == 5) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePreviewPanel5, arg0);
}
// clang-format on

void initCharacterSelectCoursePreviewPanel5(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x5;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel5);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePreviewPanel6.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePreviewPanel6(CharacterSelectCourseWidgetActor *arg0) {
    int state;
    u8 *stateField = &arg0->transition.bytes.state;

    if ((gRaceCourseIndex.signedValue >= (s32)(u16)arg0->sprite.index) && (arg0->y != -0x48)) {
        state = arg0->transition.bytes.state = 2;
    } else if ((gRaceCourseIndex.signedValue < (s32)(u16)arg0->sprite.index) && (arg0->y != -0x140)) {
        state = arg0->transition.bytes.state = 1;
    } else {
        state = *stateField;
        if (state < 4) {
            state = arg0->transition.bytes.state = 3;
        }
    }

    switch (state) {
        case 0:
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            gMenuFlowState += 1;
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 4;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 3;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x += 0x20;
            state = *stateField;
            break;
    }

    if ((state == 5) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePreviewPanel6, arg0);
}
// clang-format on

void initCharacterSelectCoursePreviewPanel6(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x6;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel6);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePreviewPanel7.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePreviewPanel7(CharacterSelectCourseWidgetActor *arg0) {
    int state;
    u8 *stateField = &arg0->transition.bytes.state;

    if ((gRaceCourseIndex.signedValue >= (s32)(u16)arg0->sprite.index) && (arg0->y != -0x48)) {
        state = arg0->transition.bytes.state = 2;
    } else if ((gRaceCourseIndex.signedValue < (s32)(u16)arg0->sprite.index) && (arg0->y != -0x140)) {
        state = arg0->transition.bytes.state = 1;
    } else {
        state = *stateField;
        if (state < 4) {
            state = arg0->transition.bytes.state = 3;
        }
    }

    switch (state) {
        case 0:
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            gMenuFlowState += 1;
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 4;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 3;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x += 0x20;
            state = *stateField;
            break;
    }

    if ((state == 5) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePreviewPanel7, arg0);
}
// clang-format on

void initCharacterSelectCoursePreviewPanel7(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x7;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel7);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePreviewPanel8.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePreviewPanel8(CharacterSelectCourseWidgetActor *arg0) {
    int state;
    u8 *stateField = &arg0->transition.bytes.state;

    if ((gRaceCourseIndex.signedValue >= (s32)(u16)arg0->sprite.index) && (arg0->y != -0x48)) {
        state = arg0->transition.bytes.state = 2;
    } else if ((gRaceCourseIndex.signedValue < (s32)(u16)arg0->sprite.index) && (arg0->y != -0x140)) {
        state = arg0->transition.bytes.state = 1;
    } else {
        state = *stateField;
        if (state < 4) {
            state = arg0->transition.bytes.state = 3;
        }
    }

    switch (state) {
        case 0:
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            gMenuFlowState += 1;
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 4;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 3;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x += 0x20;
            state = *stateField;
            break;
    }

    if ((state == 5) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePreviewPanel8, arg0);
}
// clang-format on

void initCharacterSelectCoursePreviewPanel8(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x8;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel8);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePreviewPanel9.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePreviewPanel9(CharacterSelectCourseWidgetActor *arg0) {
    int state;
    u8 *stateField = &arg0->transition.bytes.state;

    if ((gRaceCourseIndex.signedValue >= (s32)(u16)arg0->sprite.index) && (arg0->y != -0x48)) {
        state = arg0->transition.bytes.state = 2;
    } else if ((gRaceCourseIndex.signedValue < (s32)(u16)arg0->sprite.index) && (arg0->y != -0x140)) {
        state = arg0->transition.bytes.state = 1;
    } else {
        state = *stateField;
        if (state < 4) {
            state = arg0->transition.bytes.state = 3;
        }
    }

    switch (state) {
        case 0:
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            gMenuFlowState += 1;
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 4;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 3;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x += 0x20;
            state = *stateField;
            break;
    }

    if ((state == 5) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePreviewPanel9, arg0);
}
// clang-format on

void initCharacterSelectCoursePreviewPanel9(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0x9;
    arg0->transition.bytes.timer = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCoursePreviewPanel9);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCourseExitPreviewPanel.s")
// clang-format on

// clang-format off
void updateCharacterSelectCourseExitPreviewPanel(CharacterSelectCourseWidgetActor *arg0) {
    int state;
    u8 *stateField = &arg0->row.bytes.subState;

    if ((gRaceCourseIndex.signedValue == gCharacterSelectCourseExitOptionIndex) && (arg0->y != -0x48) &&
        ((s32)arg0->row.bytes.subState < 6)) {
        state = arg0->row.bytes.subState = 2;
    } else {
        state = (s32)arg0->row.bytes.subState;
        if ((gRaceCourseIndex.signedValue != gCharacterSelectCourseExitOptionIndex) && (arg0->y != -0x140) &&
            (state < 6)) {
            state = arg0->row.bytes.subState = 1;
        } else {
            state = arg0->row.bytes.subState;
            if (state < 4) {
                state = arg0->row.bytes.subState = 3;
            }
        }
    }

    switch (state) {
        case 0:
            break;
        case 1:
            arg0->y -= 0x24;
            if (arg0->y < -0x13F) {
                arg0->y = -0x140;
                arg0->row.bytes.subState = 3;
            }
            state = arg0->row.bytes.subState;
            break;
        case 2:
            arg0->y += 0x24;
            if (arg0->y >= -0x48) {
                arg0->y = -0x48;
                arg0->row.bytes.subState = 6;
                arg0->transition.alpha = 8;
            }
            state = arg0->row.bytes.subState;
            break;
        case 3:
            gMenuFlowState += 1;
            if (gRacePlayers[0].menuState == 1) {
                arg0->row.bytes.subState = 4;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->row.bytes.subState = 5;
            }
            state = arg0->row.bytes.subState;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                arg0->row.bytes.subState = 3;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->row.bytes.subState = 5;
            }
            state = arg0->row.bytes.subState;
            break;
        case 5:
            arg0->x += 0x20;
            state = *stateField;
            break;
        case 6:
            arg0->y -= arg0->transition.alpha;
            state = arg0->row.bytes.subState = 7;
            break;
        case 7:
            arg0->y += arg0->transition.alpha;
            arg0->transition.alpha /= 2;
            if (arg0->transition.alpha == 0) {
                arg0->row.bytes.subState = 3;
            } else {
                arg0->row.bytes.subState = 6;
            }
            state = arg0->row.bytes.subState;
            break;
    }

    if ((state == 5) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseExitPreviewPanel, arg0);
}
// clang-format on

void initCharacterSelectCourseExitPreviewPanel(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x140;
    arg0->sprite.index = 0;
    arg0->row.bytes.subTimer = 0;
    arg0->row.bytes.subState = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseExitPreviewPanel);
}

void drawCharacterSelectCourseListCursor(CharacterSelectCourseWidgetActor *arg0) {
    if (arg0->transition.bytes.state < 3) {
        drawMenuSpriteWithAlpha(
            arg0->x,
            (s16)(arg0->y + gRaceCourseIndex.signedValue * arg0->selection.bytes.rowSpacing),
            getRelocatableHeapBlockBase(ASSET_HANDLE(0x21)),
            0x12,
            0x20,
            0x20,
            0,
            arg0->sprite.index,
            0
        );
    }
}

// clang-format off
void updateCharacterSelectCourseListCursor(CharacterSelectCourseWidgetActor *arg0) {
    u8 state;
    s32 primaryPlayerIndex;
    u8 globalState;

    state = arg0->transition.bytes.state;
    if (state != (globalState = gCharacterSelectCourseCursorState.listCursorState)) {
        arg0->transition.bytes.state = globalState;
        state = globalState;
        arg0->sprite.index = gCharacterSelectCourseCursorState.listCursorSpriteIndex;
        arg0->transition.bytes.timer = gCharacterSelectCourseCursorState.listCursorTimer;
    }

    /* Preserve IDO's state selector register allocation. */
    primaryPlayerIndex = 0;
    switch (state ^ primaryPlayerIndex) {
        case 0:
            arg0->sprite.index += 0x26;
            if (arg0->sprite.index >= 0x100) {
                arg0->sprite.index = 0x100;
                arg0->transition.bytes.state = 1;
            }
            state = arg0->transition.bytes.state;
            break;
        case 1:
            if ((s32)arg0->transition.bytes.timer < 0x10) {
                arg0->sprite.index -= 9;
            } else {
                arg0->sprite.index += 9;
            }
            state = arg0->transition.bytes.state;
            arg0->transition.bytes.timer = (arg0->transition.bytes.timer + 1) & 0x1F;
            break;
        case 2:
            if (gRacePlayers[primaryPlayerIndex].menuState == 1) {
                state = arg0->transition.bytes.state = 3;
            }
            break;
        case 3:
            break;
    }

    (&gCharacterSelectCourseCursorState.listCursorState)[primaryPlayerIndex] = state;
    if (gRacePlayers[primaryPlayerIndex].menuState == 7) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseListCursor, arg0);
}
// clang-format on

void initCharacterSelectCourseListCursor(CharacterSelectCourseWidgetActor *arg0) {
    u8 rowSpacing;
    s16 y;
    int mode;

    mode = gHighestUnlockedCourse;
    if (mode == 0) {
        if (1) {}
        rowSpacing = 0x18;
        y = -0x50;
    } else if (mode == 1) {
        y = -0x54;
        rowSpacing = 0x16;
    } else if (mode == 2) {
        rowSpacing = 0x16;
        y = -0x60;
    } else {
        y = -0x60;
        rowSpacing = 0x13;
    }

    if (gCourseSelectFromRaceTypeMenu != 0) {
        y = -0x58;
        rowSpacing = 0x18;
    }

    arg0->x = -0x7C;
    arg0->y = y;
    arg0->selection.bytes.rowSpacing = rowSpacing;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    arg0->transition.bytes.timer = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseListCursor);
}

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
void updateCharacterSelectCourseTitleCursor(CharacterSelectCourseWidgetActor *arg0) {
    u8 state = arg0->transition.bytes.state;

    switch (state) {
        case 0:
            arg0->sprite.index += 0x26;
            if (arg0->sprite.index >= 0x100) {
                arg0->sprite.index = 0x100;
                arg0->transition.bytes.state = 1;
            }
            state = arg0->transition.bytes.state;
            break;
        case 1:
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 2;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 1;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            arg0->x += 0x20;
            if (arg0->x >= 0x94) {
                arg0->transition.bytes.state = 4;
            }
            state = arg0->transition.bytes.state;
            break;
    }
    if ((unsigned int)state == 4) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseTitleCursor, arg0);
}
// clang-format on

void initCharacterSelectCourseTitleCursor(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x5C;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseTitleCursor);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCourseStatsBadge.s")
// clang-format on

// clang-format off
void updateCharacterSelectCourseStatsBadge(CharacterSelectCourseWidgetActor *arg0) {
    u8 state = arg0->transition.bytes.state;

    switch (state) {
        case 0:
            arg0->sprite.index += 0x26;
            if (arg0->sprite.index >= 0x100) {
                arg0->sprite.index = 0x100;
                arg0->transition.bytes.state = 1;
            }
            state = arg0->transition.bytes.state;
            break;
        case 1:
            if (gRacePlayers[0].menuState == 1) {
                arg0->transition.bytes.state = 2;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            if (gRacePlayers[0].menuState == 5) {
                arg0->transition.bytes.state = 1;
            }
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 3;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            arg0->x += 0x20;
            if (arg0->x >= 0x94) {
                arg0->transition.bytes.state = 4;
            }
            state = arg0->transition.bytes.state;
            break;
    }
    if ((unsigned int)state == 4) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseStatsBadge, arg0);
}
// clang-format on

void initCharacterSelectCourseStatsBadge(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = 0x48;
    arg0->y = 0x34;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseStatsBadge);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCoursePlayerStatsPanel.s")
// clang-format on

// clang-format off
void updateCharacterSelectCoursePlayerStatsPanel(CharacterSelectCourseWidgetActor *arg0) {
    u8 state = arg0->sprite.bytes.state;
    s16 target;

    switch (state) {
        case 0:
            if (gPlayerCount == 1) {
                target = -0x78;
            } else {
                target = -0x80;
            }
            arg0->x += 0x20;
            if (arg0->x >= target) {
                arg0->x = target;
                arg0->sprite.bytes.state = 1;
            }
            state = arg0->sprite.bytes.state;
            break;
        case 1:
            if ((gRacePlayers[0].menuState == 3) || (gRacePlayers[0].menuState == 7)) {
                state = arg0->sprite.bytes.state = 2;
            }
            break;
        case 2:
            arg0->x -= 0x20;
            if (arg0->x < -0x107) {
                arg0->sprite.bytes.state = 3;
            }
            state = arg0->sprite.bytes.state;
            break;
        case 3:
            break;
    }
    if ((unsigned int)state == 3) {
        removeCallbackTask(arg0);
        if (gRacePlayers[0].menuState == 3) {
            gRacePlayers[0].menuState = 4;
        }
    } else {
        addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCoursePlayerStatsPanel, arg0);
    }
}
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
void updateCharacterSelectCourseSubmenuFrame(CharacterSelectCourseWidgetActor *arg0) {
    u8 state = arg0->sprite.bytes.state;

    switch (state) {
        case 0:
            arg0->x += 0x20;
            if (arg0->x >= -0x88) {
                arg0->x = -0x88;
                arg0->sprite.bytes.state = 1;
                gMenuScratch0.task = createCallbackTask((CallbackTaskCallback)initCharacterSelectCourseNamePopup, 0, 0x63);
            }
            state = arg0->sprite.bytes.state;
            break;
        case 1:
            if ((gRacePlayers[0].menuState == 3) || (gRacePlayers[0].menuState == 7)) {
                state = arg0->sprite.bytes.state = 2;
            }
            break;
        case 2:
            arg0->x -= 0x20;
            if (arg0->x < -0x107) {
                arg0->sprite.bytes.state = 3;
            }
            state = arg0->sprite.bytes.state;
            break;
        case 3:
            state = arg0->sprite.bytes.state = 4;
            break;
    }
    state = arg0->sprite.bytes.state;
    if (state == 4) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseSubmenuFrame, arg0);
}
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
void updateCharacterSelectCourseRecordsFrame(CharacterSelectCourseWidgetActor *arg0) {
    u32 state = arg0->sprite.bytes.state;

    switch (state) {
        case 0:
            arg0->x += 0x20;
            if (arg0->x >= -0x88) {
                arg0->x = -0x88;
                arg0->sprite.bytes.state = 1;
                createCallbackTask((CallbackTaskCallback)initCharacterSelectCourseDescriptionPopup, 0, 0x63);
            }
            state = arg0->sprite.bytes.state;
            break;
        case 1:
            if (gRacePlayers[0].menuState == 7) {
                arg0->sprite.bytes.state = 5;
            }
            if (gRacePlayers[0].menuState == 1) {
                arg0->sprite.bytes.state = 2;
            }
            state = arg0->sprite.bytes.state;
            break;
        case 2:
            arg0->x -= 0x20;
            if (arg0->x < -0x127) {
                arg0->sprite.bytes.state = 3;
                arg0->x = -0x128;
            }
            state = arg0->sprite.bytes.state;
            break;
        case 3:
            if (gRacePlayers[0].menuState == 5) {
                state = arg0->sprite.bytes.state = 4;
            }
            break;
        case 4:
            arg0->x += 0x20;
            if (arg0->x >= -0x88) {
                arg0->x = -0x88;
                arg0->sprite.bytes.state = 1;
            }
            state = arg0->sprite.bytes.state;
            break;
        case 5:
            arg0->x -= 0x20;
            if (arg0->x < -0x107) {
                arg0->sprite.bytes.state = 6;
            }
            state = arg0->sprite.bytes.state;
            break;
        case 6:
            state = arg0->sprite.bytes.state = 7;
            break;
    }
    if (state == 7) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseRecordsFrame, arg0);
}
// clang-format on

void initCharacterSelectCourseRecordsFrame(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x128;
    arg0->y = 0x8;
    arg0->sprite.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseRecordsFrame);
}

void drawCharacterSelectCourseNamePopup(CharacterSelectCourseWidgetActor *arg0) {
    CharacterSelectCourseText *texts;
    MenuGlyphScript *text;

    texts = gCharacterSelectCourseNameText;
    text = texts[gRaceCourseIndex.signedValue];
    drawMenuGlyphScript(arg0->x, arg0->y, text, 1, arg0->sprite.index, 0);
    if (arg0->transition.bytes.state == 2) {
        drawMenuSprite(
            (s16)(arg0->x + 0x60),
            (s16)(arg0->y + 0x40),
            getRelocatableHeapBlockBase(ASSET_HANDLE(0x24)),
            ((s32)arg0->transition.bytes.timer >= 8) + 5,
            0x20,
            0x20,
            0,
            0
        );
    }
}

// clang-format off
void updateCharacterSelectCourseNamePopup(CharacterSelectCourseWidgetActor *arg0) {
    u8 state;
    u8 globalState;

    state = arg0->transition.bytes.state;
    if (state != (globalState = gCharacterSelectCourseCursorState.submenuState)) {
        arg0->transition.bytes.state = globalState;
        state = globalState;
        arg0->transition.bytes.timer = gCharacterSelectCourseCursorState.submenuTimer;
    }

    if (state) {
        while (0) {}
    }
    switch (state) {
        case 0:
            if ((++arg0->transition.bytes.timer) == 2) {
                arg0->transition.bytes.state = 1;
                arg0->transition.bytes.timer = state * 0;
            }
            state = arg0->transition.bytes.state;
            break;
        case 1:
            arg0->sprite.index += 0x30;
            if (arg0->sprite.index >= 0x100) {
                arg0->sprite.index = 0x100;
                arg0->transition.bytes.state = 2;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->transition.bytes.timer = (arg0->transition.bytes.timer + 1) & 0xF;
            if (gRacePlayers[0].menuState == 3) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            state = arg0->transition.bytes.state;
            arg0->transition.bytes.timer = 0;
            break;
        case 4:
            arg0->transition.bytes.timer++;
            if ((gRacePlayers[0].menuState == 3) || (gRacePlayers[0].menuState == 7)) {
                arg0->transition.bytes.state = 5;
            }
            if (globalState) {}
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x -= 0x20;
            if (arg0->x < -0xFF) {
                arg0->transition.bytes.state = 6;
            }
            state = arg0->transition.bytes.state;
            break;
        case 6:
            break;
    }

    (arg0 ? &gCharacterSelectCourseCursorState : &gCharacterSelectCourseCursorState)->submenuState = state;
    (arg0 ? &gCharacterSelectCourseCursorState : &gCharacterSelectCourseCursorState)->submenuTimer =
        arg0->transition.bytes.timer;

    if (arg0->transition.bytes.state == 6) {
        removeCallbackTask(arg0);
        gMenuScratch0.task = NULL;
        gCharacterSelectCourseCursorState.submenuState = 0;
        gCharacterSelectCourseCursorState.submenuTimer = 0;
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseNamePopup, arg0);
}
// clang-format on

void initCharacterSelectCourseNamePopup(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x84;
    arg0->y = -0x14;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseNamePopup);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCourseDescriptionPopup.s")
// clang-format on

// clang-format off
void updateCharacterSelectCourseDescriptionPopup(CharacterSelectCourseWidgetActor *arg0) {
    u32 state = arg0->transition.bytes.state;

    switch (state) {
        case 0:
            if (++arg0->transition.bytes.timer == 2) {
                arg0->transition.bytes.state = 1;
                arg0->transition.bytes.timer = 0;
            }
            state = arg0->transition.bytes.state;
            break;
        case 1:
            arg0->sprite.index += 0x30;
            if (arg0->sprite.index >= 0x100) {
                arg0->sprite.index = 0x100;
                arg0->transition.bytes.state = 2;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            if ((gRacePlayers[0].menuState == 7) || (gRacePlayers[0].menuState == 1)) {
                state = arg0->transition.bytes.state = 3;
            }
            break;
        case 3:
            arg0->x -= 0x20;
            if (arg0->x < -0x11F) {
                if (gRacePlayers[0].menuState == 7) {
                    arg0->transition.bytes.state = 7;
                } else {
                    arg0->transition.bytes.state = 4;
                }
            }
            state = arg0->transition.bytes.state;
            break;
        case 4:
            if (gRacePlayers[0].menuState == 5) {
                state = arg0->transition.bytes.state = 5;
            }
            break;
        case 5:
            arg0->x += 0x20;
            if (arg0->x >= -0x84) {
                arg0->transition.bytes.state = 2;
            }
            state = arg0->transition.bytes.state;
            break;
        case 6:
            if (gRacePlayers[0].menuState == 7) {
                arg0->transition.bytes.state = 3;
            }
            if (gRacePlayers[0].menuState == 0) {
                arg0->transition.bytes.state = 2;
            }
            state = arg0->transition.bytes.state;
            break;
        case 7:
            state = arg0->transition.bytes.state = 8;
            break;
    }
    if (state == 8) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseDescriptionPopup, arg0);
}
// clang-format on

void initCharacterSelectCourseDescriptionPopup(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x84;
    arg0->y = 0xC;
    arg0->sprite.index = 0;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseDescriptionPopup);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCourseConfirmCursor.s")
// clang-format on

// clang-format off
void updateCharacterSelectCourseConfirmCursor(CharacterSelectCourseWidgetActor *arg0) {
    s32 step;

    if ((gMenuChoicePromptState[0] != 0) && (gMenuChoicePromptState[0] != 3) && (gMenuChoicePromptState[0] != 4)) {
        step = ((gMenuChoicePromptState[0] < 5) ? 1 : -1) * 8;
        arg0->transition.signedAlpha += step;

        if (arg0->transition.signedAlpha == 0x20) {
            gMenuChoicePromptState[0] += 2;
            arg0->row.value = 0;
            arg0->selection.counter = 0x100;
        }

        if (step == 8) {
            if (arg0->transition.signedAlpha < 0x18) {
                arg0->y += 8;
            }
            arg0->sprite.index += 8;
        } else {
            if (arg0->transition.signedAlpha < 0x10) {
                arg0->y -= 8;
            }
            arg0->sprite.index -= 8;
        }
    }

    if ((gMenuChoicePromptState[0] == 3) || (gMenuChoicePromptState[0] == 4)) {
        if ((u16)arg0->row.value < 0x10) {
            arg0->selection.counter = (u16)arg0->selection.counter - 9;
        } else {
            arg0->selection.counter = (u16)arg0->selection.counter + 9;
        }
        arg0->row.value = ((u16)arg0->row.value + 1) & 0x1F;
    }

    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseConfirmCursor, arg0);
}
// clang-format on

void initCharacterSelectCourseConfirmCursor(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x74;
    arg0->y = 0x2F;
    arg0->sprite.index = 0x2F;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseConfirmCursor);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/drawCharacterSelectCourseRecordsPopup.s")
// clang-format on

// clang-format off
void updateCharacterSelectCourseRecordsPopup(CharacterSelectCourseWidgetActor *arg0) {
    u8 state = arg0->sprite.bytes.state;
    u8 unk;
    s16 target;

    switch (state) {
        case 0:
            unk = gRaceTypeSelection;
            if ((gRaceSplitscreenMode == 2) || ((gRaceSplitscreenMode == 1) && (unk == 0))) {
                target = -0x8A;
            } else {
                target = -0x88;
            }
            arg0->x += 0x20;
            if (arg0->x >= target) {
                arg0->x = target;
                arg0->sprite.bytes.state = 1;
            }
            state = arg0->sprite.bytes.state;
            break;
        case 1:
            if ((gRacePlayers[0].menuState == 3) || (gRacePlayers[0].menuState == 7)) {
                state = arg0->sprite.bytes.state = 2;
            }
            break;
        case 2:
            arg0->x -= 0x20;
            if (arg0->x < -0xEF) {
                arg0->x = -0xF0;
                arg0->sprite.bytes.state = 3;
            }
            state = arg0->sprite.bytes.state;
            break;
        case 3:
            state = arg0->sprite.bytes.state = 4;
            break;
        case 4:
            break;
    }
    state = arg0->sprite.bytes.state;
    if (state == 4) {
        removeCallbackTask(arg0);
        gRacePlayers[0].menuState = 4;
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseRecordsPopup, arg0);
}
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
void updateCharacterSelectCourseExitPopup(CharacterSelectCourseWidgetActor *arg0) {
    u8 state;
    u8 globalState;

    state = arg0->transition.bytes.state;
    if (state != (globalState = gCharacterSelectCourseCursorState.submenuState)) {
        arg0->transition.bytes.state = globalState;
        /* Preserve IDO's state/globalState register allocation. */
        if (1) {}
        if (1) {}
        if (1) {}
        if (1) {}
        if (1) {}
        state = globalState;
        arg0->transition.bytes.timer = gCharacterSelectCourseCursorState.submenuTimer;
    }

    switch (state) {
        case 0:
            arg0->x += 0x20;
            if (arg0->x >= -0x88) {
                arg0->x = -0x88;
                arg0->transition.bytes.state = 2;
            }
            state = arg0->transition.bytes.state;
            break;
        case 2:
            arg0->transition.bytes.timer = (arg0->transition.bytes.timer + 1) & 0xF;
            if (gRacePlayers[0].menuState == 3) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 3:
            state = arg0->transition.bytes.state;
            arg0->transition.bytes.timer = 0;
            break;
        case 4:
            arg0->transition.bytes.timer++;
            if ((gRacePlayers[0].menuState == 3) || (gRacePlayers[0].menuState == 7)) {
                arg0->transition.bytes.state = 5;
            }
            state = arg0->transition.bytes.state;
            break;
        case 5:
            arg0->x -= 0x20;
            if (arg0->x < -0x107) {
                arg0->transition.bytes.state = 6;
            }
            state = arg0->transition.bytes.state;
            break;
    }

    (arg0 ? &gCharacterSelectCourseCursorState : &gCharacterSelectCourseCursorState)->submenuState = state;
    (arg0 ? &gCharacterSelectCourseCursorState : &gCharacterSelectCourseCursorState)
        ->submenuTimer = arg0->transition.bytes.timer;

    if (arg0->transition.bytes.state == 6) {
        removeCallbackTask(arg0);
        gMenuScratch0.task = NULL;
        gCharacterSelectCourseCursorState.submenuState = 0;
        gCharacterSelectCourseCursorState.submenuTimer = 0;
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCharacterSelectCourseExitPopup, arg0);
}
// clang-format on

void initCharacterSelectCourseExitPopup(CharacterSelectCourseWidgetActor *arg0) {
    arg0->x = -0x108;
    arg0->y = 0x28;
    arg0->transition.bytes.state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectCourseExitPopup);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/character_select/character_select_course_ui/func_800235A8.s")
