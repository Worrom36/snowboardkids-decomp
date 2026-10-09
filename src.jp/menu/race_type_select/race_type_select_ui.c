#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/race_type_select/race_type_select_ui.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/race_state.h"
#include "game/race/player/race_player_input.h"

#define ASSET_HANDLE(index) (gAssetHandles[(index)])

extern u8 gMenuExitSelection;
extern const char gRaceTypeSelectEntryFeeFormat[];

void drawRaceTypeSelectOptionIcons(RaceTypeSelectOptionIconActor *arg0) {
    RaceTypeSelectOptionIconActor *savedArg;
    s32 i;
    s32 alpha;
    s32 yOffset;
    u16 tileIndex;

    savedArg = arg0;
    // clang-format off
    do { \
        i = 0; \
        if (arg0->visibleCount > 0) { \
            yOffset = 0; \
            do { \
                alpha = 0; \
                if (((((gMenuSelectionConfirmTimer > 0) && (gMenuSelectionConfirmTimer < 8)) && \
                      (gMenuExitSelection == 0)) && \
                     (i == gRaceTypeSelection)) && \
                    (gMenuSelectionConfirmTimer & 1)) { \
                    if ((!savedArg->visibleCount) && (!savedArg->visibleCount)) {} \
                    alpha = 0xFF; \
                } \
                tileIndex = i + 0xD; \
                drawMenuSprite( \
                    arg0->x[i], \
                    (s16)(arg0->y + yOffset), \
                    getRelocatableHeapBlockBase(gAssetHandles[0x21]), \
                    tileIndex, \
                    0x20, \
                    0x20, \
                    0, \
                    alpha \
                ); \
                yOffset += 0x18; \
                i++; \
            } while (i < savedArg->visibleCount); \
        } \
    } while (0);
    // clang-format on
}

void updateRaceTypeSelectOptionIcons(RaceTypeSelectOptionIconActor *arg0) {
    s32 i;
    s32 moved;
    s32 state;
    int stateByte;
    RaceTypeSelectOptionIconActor *row;
    RaceTypeSelectOptionIconActor *actor;

    stateByte = arg0->state;
    actor = arg0;
    state = stateByte;
    row = arg0;
    switch (state) {
        case 0:
            moved = 0;
            for (i = 0; i < row->visibleCount; i++) {
                if (row->x[i] < -0x7C) {
                    row->x[i] += 0x10;
                    moved++;
                    if (row->x[i] >= -0x7C) {
                        row->x[i] = -0x7C;
                    }
                }
            }

            row->spawnTimer++;
            if (!(row->spawnTimer & 1)) {
                if (row->visibleCount < 4) {
                    row->visibleCount++;
                }
            }

            if (moved == 0) {
                row->state = 1;
                createCallbackTask((CallbackTaskCallback)initRaceTypeSelectOption0Frame, 0, 0x5F);
                createCallbackTask((CallbackTaskCallback)initRaceTypeSelectOption1Frame, 0, 0x60);
                createCallbackTask((CallbackTaskCallback)initRaceTypeSelectOption2Frame, 0, 0x61);
                createCallbackTask((CallbackTaskCallback)initRaceTypeSelectOption3Frame, 0, 0x63);
            }
            state = arg0->state;
            break;
        case 1:
            if (gRacePlayers[0].menuState == 1) {
                state = (u8)(arg0->state = 2);
            }
            break;
        case 2:
            for (i = 0; i < 5; i++) {
                arg0->x[i] -= 0x20;
            }
            if (arg0->x[0] < -0x103) {
                arg0->state = 3;
            }
            break;
        case 3:
            break;
    }

    if (arg0->state == 3) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceTypeSelectOptionIcons, actor);
}

void initRaceTypeSelectOptionIcons(RaceTypeSelectOptionIconActor *arg0) {
    s32 temp_v1 = -0x104;
    long long zero;
    s32 temp_t6 = -0x58;
    s32 temp_t7 = 1;

    arg0->x[0] = temp_v1;
    arg0->x[1] = temp_v1;
    arg0->x[2] = temp_v1;
    arg0->x[3] = temp_v1;
    arg0->y = temp_t6;
    zero = 0;
    arg0->spawnTimer = zero;
    arg0->visibleCount = temp_t7;
    arg0->state = zero;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectOptionIcons);
}

void drawRaceTypeSelectCornerSprites(RaceTypeSelectCornerSpriteActor *arg0) {
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

void updateRaceTypeSelectCornerSprites(RaceTypeSelectCornerSpriteActor *arg0) {
    u8 state = arg0->state;

    switch (state) {
        case 0:
            arg0->x += 0x20;
            if (arg0->x >= -0x88) {
                arg0->x = -0x88;
                arg0->state = 1;
            }
            state = arg0->state;
            break;
        case 1:
            if (gRacePlayers[0].menuState == 1) {
                state = arg0->state = 2;
            }
            break;
        case 2:
            arg0->x -= 0x20;
            if (arg0->x < -0x10D) {
                arg0->state = 3;
            }
            state = arg0->state;
            break;
        case 3:
            break;
    }
    if ((unsigned int)state == 3) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceTypeSelectCornerSprites, arg0);
}

void initRaceTypeSelectCornerSprites(RaceTypeSelectCornerSpriteActor *arg0) {
    arg0->x = -0x108;
    arg0->y = 8;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectCornerSprites);
}

void drawRaceTypeSelectOption0Frame(RaceTypeSelectOption0FrameActor *arg0) {
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
            getRelocatableHeapBlockBase(gAssetHandles[0x25]),
            gRaceTypeSelectFrameTileMaps.optionFrames[(u16)arg0->optionIndex].centerTiles[tileOffset],
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
            getRelocatableHeapBlockBase(gAssetHandles[0x25]),
            gRaceTypeSelectFrameTileMaps.optionFrames[(u16)arg0->optionIndex].rightEdgeTiles[tileOffset],
            0,
            0x100
        );
        i = 0x80;
        drawMenuSpriteTile(
            (s16)(arg0->x + offset),
            (s16)(arg0->y + 0x80),
            getRelocatableHeapBlockBase(gAssetHandles[0x25]),
            gRaceTypeSelectFrameTileMaps.optionFrames[(u16)arg0->optionIndex].bottomEdgeTiles[tileOffset],
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
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        gRaceTypeSelectFrameTileMaps.optionFrames[(u16)arg0->optionIndex].cornerTile,
        0,
        0x100
    );

    drawMenuSprite(
        (s16)(arg0->x - 4),
        (s16)(arg0->y - 4),
        getRelocatableHeapBlockBase(gAssetHandles[0x21]),
        0x33,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x - 4),
        (s16)(arg0->y + 0x8C),
        getRelocatableHeapBlockBase(gAssetHandles[0x21]),
        0x38,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x8C),
        (s16)(arg0->y - 4),
        getRelocatableHeapBlockBase(gAssetHandles[0x21]),
        0x35,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x8C),
        (s16)(arg0->y + 0x8C),
        getRelocatableHeapBlockBase(gAssetHandles[0x21]),
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
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x34,
            0x20,
            0x20,
            0,
            0
        );
        drawMenuSprite(
            (s16)((arg0->x + borderOffset) + 0xC),
            (s16)(arg0->y + 0x8C),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x39,
            0x20,
            0x20,
            0,
            0
        );
        drawMenuSprite(
            (s16)(arg0->x - 4),
            (s16)((arg0->y + borderOffset) + 0xC),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x36,
            0x20,
            0x20,
            0,
            0
        );
        drawMenuSprite(
            (s16)(arg0->x + 0x8C),
            (s16)((arg0->y + borderOffset) + 0xC),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x37,
            0x20,
            0x20,
            0,
            0
        );
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_type_select/race_type_select_ui/updateRaceTypeSelectOption0Frame.s")

void initRaceTypeSelectOption0Frame(RaceTypeSelectOption0FrameActor *arg0) {
    arg0->x = 0x94;
    arg0->y = -0x48;
    arg0->unused1C = -8;
    arg0->unused1E = -0x74;
    arg0->optionIndex = 0;
    arg0->cornerSpawnTimer = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectOption0Frame);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_type_select/race_type_select_ui/func_8002A0B8.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_type_select/race_type_select_ui/updateRaceTypeSelectOption1Frame.s")

void initRaceTypeSelectOption1Frame(RaceTypeSelectOptionFrameActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x140;
    arg0->optionIndex = 1;
    arg0->unusedTimer = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectOption1Frame);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_type_select/race_type_select_ui/func_8002A618.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_type_select/race_type_select_ui/updateRaceTypeSelectOption2Frame.s")

void initRaceTypeSelectOption2Frame(RaceTypeSelectOptionFrameActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x140;
    arg0->optionIndex = 2;
    arg0->unusedTimer = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectOption2Frame);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_type_select/race_type_select_ui/func_8002AB78.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_type_select/race_type_select_ui/updateRaceTypeSelectOption3Frame.s")

void initRaceTypeSelectOption3Frame(RaceTypeSelectExitFrameActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x140;
    arg0->unused1C = 2;
    arg0->unusedTimer = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectOption3Frame);
}

void drawRaceTypeSelectCursor(RaceTypeSelectAnimatedWidgetActor *arg0) {
    drawMenuSpriteWithAlpha(
        arg0->x,
        (s16)(arg0->y + (gRaceTypeSelection * 0x18)),
        getRelocatableHeapBlockBase(ASSET_HANDLE(33)),
        7,
        0x20,
        0x20,
        0,
        arg0->alpha,
        0
    );
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/race_type_select/race_type_select_ui/updateRaceTypeSelectCursor.s")

void initRaceTypeSelectCursor(RaceTypeSelectAnimatedWidgetActor *arg0) {
    arg0->x = -0x7C;
    arg0->y = -0x58;
    arg0->alpha = 0;
    arg0->state = 0;
    arg0->animationTimer = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectCursor);
}

void drawRaceTypeSelectPortrait(RaceTypeSelectAnimatedWidgetActor *arg0) {
    RaceTypeSelectDescription *description = &gRaceTypeSelectPortraitScripts.descriptions[gRaceTypeSelection];

    drawMenuGlyphScript(arg0->x, arg0->y, *description, 1, arg0->alpha, 0);
}

void updateRaceTypeSelectPortrait(RaceTypeSelectAnimatedWidgetActor *arg0) {
    u8 state = arg0->state;

    switch (state) {
        case 0:
            arg0->alpha += 0x26;
            if (arg0->alpha >= 0x100) {
                arg0->alpha = 0x100;
                arg0->state = 1;
            }
            state = arg0->state;
            break;
        case 1:
            if (gRacePlayers[0].menuState == 1) {
                state = arg0->state = 2;
            }
            break;
        case 2:
            arg0->x -= 0x20;
            if (arg0->x < -0xFF) {
                arg0->state = 3;
            }
            state = arg0->state;
            break;
        case 3:
            break;
    }
    if ((unsigned int)state == 3) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceTypeSelectPortrait, arg0);
}

void initRaceTypeSelectPortrait(RaceTypeSelectAnimatedWidgetActor *arg0) {
    arg0->x = -0x84;
    arg0->y = 0xC;
    arg0->alpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectPortrait);
}

void drawRaceTypeSelectArrowPrompt(RaceTypeSelectAnimatedWidgetActor *arg0) {
    drawMenuSpriteWithAlpha(
        arg0->x,
        arg0->y,
        getRelocatableHeapBlockBase(ASSET_HANDLE(33)),
        2,
        0x20,
        0x20,
        0,
        arg0->alpha,
        0
    );
}

void updateRaceTypeSelectArrowPrompt(RaceTypeSelectAnimatedWidgetActor *arg0) {
    u8 state = arg0->state;

    switch (state) {
        case 0:
            arg0->alpha += 0x26;
            if (arg0->alpha >= 0x100) {
                arg0->alpha = 0x100;
                arg0->state = 1;
            }
            state = arg0->state;
            break;
        case 1:
            if (gRacePlayers[0].menuState == 1) {
                state = arg0->state = 2;
            }
            break;
        case 2:
            arg0->x += 0x20;
            if (arg0->x >= 0xA0) {
                arg0->state = 3;
            }
            state = arg0->state;
            break;
        case 3:
            break;
    }
    if ((unsigned int)state == 3) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceTypeSelectArrowPrompt, arg0);
}

void initRaceTypeSelectArrowPrompt(RaceTypeSelectAnimatedWidgetActor *arg0) {
    arg0->x = -8;
    arg0->y = -0x5C;
    arg0->alpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectArrowPrompt);
}

void drawRaceTypeSelectEntryFee(RaceTypeSelectAnimatedWidgetActor *arg0) {
    char sp40[0x18];

    if (gRaceTypeSelection != 3) {
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
        sprintf(sp40, gRaceTypeSelectEntryFeeFormat, gRacePlayers[0].money);
        drawMenuAsciiText((s16)(arg0->x + 0x10), (s16)(arg0->y + 0x10), sp40, 0, arg0->alpha);
    }
}

void updateRaceTypeSelectEntryFee(RaceTypeSelectAnimatedWidgetActor *arg0) {
    u8 state = arg0->state;

    switch (state) {
        case 0:
            arg0->alpha += 0x26;
            if (arg0->alpha >= 0x100) {
                arg0->alpha = 0x100;
                arg0->state = 1;
            }
            state = arg0->state;
            break;
        case 1:
            if (gRacePlayers[0].menuState == 1) {
                state = arg0->state = 2;
            }
            break;
        case 2:
            arg0->x += 0x20;
            if (arg0->x >= 0xA0) {
                arg0->state = 3;
            }
            state = arg0->state;
            break;
        case 3:
            break;
    }
    if ((unsigned int)state == 3) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceTypeSelectEntryFee, arg0);
}

void initRaceTypeSelectEntryFee(RaceTypeSelectAnimatedWidgetActor *arg0) {
    arg0->x = 0x30;
    arg0->y = 0x40;
    arg0->alpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceTypeSelectEntryFee);
}
