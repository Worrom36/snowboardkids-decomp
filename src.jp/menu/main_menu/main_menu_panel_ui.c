#include "common.h"
#include "game/race/race_state.h"
#include "game/engine/game_task_scheduler.h"
#include "font_encoding.h"
#include "game/audio/sound_manager.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/system_runtime.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/main_menu/main_menu_panel_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"

#define MENU_PANEL_TEXTURE_HANDLE (gAssetHandles[0x2A])
#define mainMenuModeSelectTopRowIconTiles gMainMenuModeSelectByteData.topRowIconTiles
#define mainMenuModeSelectBottomRowIconTiles gMainMenuModeSelectByteData.bottomRowIconTiles
#define MENU_PANEL_SCROLL_STEP 0x10
#define MENU_PANEL_SCROLL_LIMIT 0x79

typedef struct {
    union {
        MenuGlyphScript finalTitleText[12];
        struct {
            u8 finalTitleTextPrefix[0x17];
            u8 primaryIconTiles[1];
        } bytes;
    } titleAndPrimaryIcon;
    u8 topRowIconTiles[2];
    u8 secondaryIconTiles[6];
    u8 bottomRowIconTiles[8];
    u8 upTargets[0x10];
    u8 downTargets[0x10];
    u8 leftTargets[0x10];
    u8 rightTargets[0x10];
} MainMenuModeSelectByteData;

extern MenuGlyphScript *mainMenuModeDescriptionTexts[];

extern MenuGlyphScript mainMenuModeSelectTitleText[];
extern MainMenuModeSelectByteData gMainMenuModeSelectByteData;
extern u8 gMainMenuModeSelection;
extern MenuGlyphScript raceGhostUnavailableText[];
extern s32 gMenuFlowState;

void drawMainMenuModeSelectFrame(void *arg0) {
    s32 edgeX;
    s32 edgeY;
    s32 y;
    s32 yOrigin;
    s32 xOrigin;

    xOrigin = -0x74;
    yOrigin = 0x54;

    drawMenuSprite(-0x84, -0x64, getRelocatableHeapBlockBase(MENU_PANEL_TEXTURE_HANDLE), 2, 0x20, 0x20, 0, 0);
    drawMenuSprite(0x78, -0x64, getRelocatableHeapBlockBase(MENU_PANEL_TEXTURE_HANDLE), 4, 0x20, 0x20, 0, 0);

    edgeX = xOrigin;
    do {
        drawMenuSprite(edgeX, -0x64, getRelocatableHeapBlockBase(MENU_PANEL_TEXTURE_HANDLE), 3, 0x20, 0x20, 0, 0);
        drawMenuSprite(edgeX, -0x3C, getRelocatableHeapBlockBase(MENU_PANEL_TEXTURE_HANDLE), 8, 0x20, 0x20, 0, 0);
        edgeX += 0x10;
    } while (edgeX < 0x7C);

    drawMenuSprite(-0x84, -0x3C, getRelocatableHeapBlockBase(MENU_PANEL_TEXTURE_HANDLE), 7, 0x20, 0x20, 0, 0);
    drawMenuSprite(0x78, -0x3C, getRelocatableHeapBlockBase(MENU_PANEL_TEXTURE_HANDLE), 9, 0x20, 0x20, 0, 0);

    edgeY = -yOrigin;
    do {
        drawMenuSprite(-0x84, edgeY, getRelocatableHeapBlockBase(MENU_PANEL_TEXTURE_HANDLE), 5, 0x20, 0x20, 0, 0);
        drawMenuSprite(0x78, edgeY, getRelocatableHeapBlockBase(MENU_PANEL_TEXTURE_HANDLE), 6, 0x20, 0x20, 0, 0);
        edgeY += 0x10;
    } while (edgeY < -0x34);

    if (yOrigin) {}

    for (edgeX = xOrigin; edgeX != 0x7C; edgeX += 0x10) {
        for (y = -yOrigin; y != -0x34; y += 0x10) {
            drawMenuSprite(edgeX, y, getRelocatableHeapBlockBase(MENU_PANEL_TEXTURE_HANDLE), 0xB, 0x20, 0x20, 0, 0);
        }
    }

    drawMenuColoredGlyphScript(-0x68, -0x58, mainMenuModeSelectTitleText, 0, 0x100, 5, 0x29);
}

void drawMainMenuModeSelectIcons(MenuPanelActor *arg0) {
    s32 alpha;
    s32 i;
    s32 next;
    s32 x;
    u8 *tile;

    if (arg0) {}

    x = -0x6E;
    // clang-format off
    tile = mainMenuModeSelectTopRowIconTiles; i = 0; do {
        // clang-format on
        next = i + 1;
        alpha = 0;
        if (next == gMainMenuModeSelection) {
            alpha = arg0->x;
        }

        drawMenuFillRectangle((s16)(x + 0xA), -0x16, 0x22, 0x22, 0x40, alpha / 4, 0);
        drawMenuFillRectangle((s16)(x + 4), -0x1C, 0x22, 0x22, 0xFF, alpha, 0);
        drawAssetTableSprite((s16)(x + 5), -0x1B, getRelocatableHeapBlockBase(gAssetHandles[0x1F]), *tile);
        i = next;
        tile++;
        x += 0x2C;
    } while (next < 5);

    x = -0x84;
    tile = mainMenuModeSelectBottomRowIconTiles;
    i = 0;
    do {
        alpha = 0;
        if ((i + 6) == gMainMenuModeSelection) {
            alpha = arg0->x;
        }

        drawMenuFillRectangle((s16)(x + 0xA), 0x1E, 0x22, 0x22, 0, alpha / 4, 0x40);
        drawMenuFillRectangle((s16)(x + 4), 0x18, 0x22, 0x22, arg0->x * 0, alpha, 0xFF);
        drawAssetTableSprite((s16)(x + 5), 0x19, getRelocatableHeapBlockBase(gAssetHandles[0x1F]), *tile);
        i++;
        tile++;
        x += 0x2C;
    } while (i != 6);

    if (gMainMenuModeSelection == 0xC) {
        if (arg0->x != 0) {
            drawAssetTableSprite(-0x30, 0x4D, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 2);
            drawPulsingAssetTableSprite(-0x30, 0x4D, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 3);
            return;
        }
        drawAssetTableSpriteWithExplicitPalette(-0x30, 0x4D, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 2, 1);
        drawPulsingAssetTableSprite(-0x30, 0x4D, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 3);
        return;
    }

    drawAssetTableSpriteWithExplicitPalette(-0x30, 0x4D, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 2, 1);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/main_menu_panel_ui/updateMainMenuModeSelectGrid.s")

void initMainMenuModeSelectGrid(MenuPanelActor *arg0) {
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateMainMenuModeSelectGrid);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/main_menu_panel_ui/drawMainMenuModeDescriptionPanel.s")

void scrollMainMenuModeDescriptionPanelOut(MenuPanelActor *arg0) {
    arg0->y += MENU_PANEL_SCROLL_STEP;
    if (arg0->y >= MENU_PANEL_SCROLL_LIMIT) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawMainMenuModeDescriptionPanel, arg0);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/main_menu_panel_ui/updateMainMenuModeDescriptionPanel.s")

void initMainMenuModeDescriptionPanel(MenuPanelActor *arg0) {
    arg0->x = -0x80;
    arg0->y = 4;
    arg0->selectionState = 0;
    arg0->inputRepeatTimer = 0;
    arg0->tileList = mainMenuModeDescriptionTexts[gMainMenuModeSelection];
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateMainMenuModeDescriptionPanel);
}

void drawMainMenuModeSelectMenuOptions(void *arg0) {
    if (gMenuFlowState == 0) {
        if ((gMainMenuSelectionResult == 0) || (gFrameCounter & 1)) {
            drawAssetTableSprite(-0x30, -0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0);
            drawPulsingAssetTableSprite(-0x30, -0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 3);
        } else {
            drawAssetTableSpriteWithExplicitPalette(
                -0x30,
                -0x20,
                getRelocatableHeapBlockBase(gAssetHandles[0x28]),
                0,
                1
            );
            drawPulsingAssetTableSprite(-0x30, -0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 3);
        }
    } else {
        drawAssetTableSpriteWithExplicitPalette(-0x30, -0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0, 1);
    }

    if (gMenuFlowState == 1) {
        if ((gMainMenuSelectionResult == 0) || (gFrameCounter & 1)) {
            drawAssetTableSprite(-0x30, -8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 1);
            drawPulsingAssetTableSprite(-0x30, -8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 3);
        } else {
            drawAssetTableSpriteWithExplicitPalette(-0x30, -8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 1, 1);
            drawPulsingAssetTableSprite(-0x30, -8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 3);
        }
    } else {
        drawAssetTableSpriteWithExplicitPalette(-0x30, -8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 1, 1);
    }

    if (gMenuFlowState == 2) {
        if ((gMainMenuSelectionResult == 0) || (gFrameCounter & 1)) {
            drawAssetTableSprite(-0x30, 0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 2);
            drawPulsingAssetTableSprite(-0x30, 0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 3);
            return;
        }
        drawAssetTableSpriteWithExplicitPalette(-0x30, 0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 2, 1);
        drawPulsingAssetTableSprite(-0x30, 0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 3);
        return;
    }

    drawAssetTableSpriteWithExplicitPalette(-0x30, 0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 2, 1);
}

void updateMainMenuModeSelectMenuOptions(MenuPanelActor *arg0) {
    addRenderCallback(&gMenuRenderCallbackList, drawMainMenuModeSelectMenuOptions, arg0);
}

void initMainMenuModeSelectMenuOptions(MenuPanelActor *arg0) {
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateMainMenuModeSelectMenuOptions);
}

void drawRaceSetupPreviewCornerMasks(void *arg0) {
    drawAssetTableSpriteWithDefaultPalette(-0x84, -0x64, getRelocatableHeapBlockBase(gAssetHandles[6]), 6);
    drawAssetTableSpriteWithDefaultPalette(0x74, -0x64, getRelocatableHeapBlockBase(gAssetHandles[6]), 7);
    drawAssetTableSpriteWithDefaultPalette(-0x84, 4, getRelocatableHeapBlockBase(gAssetHandles[6]), 8);
    drawAssetTableSpriteWithDefaultPalette(0x74, 4, getRelocatableHeapBlockBase(gAssetHandles[6]), 9);
}

void updateRaceSetupPreviewCornerMasks(MenuPanelActor *arg0) {
    if (gMainMenuSelectionResult == 0) {
        addRenderCallback(&gMenuRenderCallbackList, drawRaceSetupPreviewCornerMasks, arg0);
        return;
    }
    removeCallbackTask(arg0);
}

void initRaceSetupPreviewCornerMasks(MenuPanelActor *arg0) {
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSetupPreviewCornerMasks);
}

void drawRaceGhostUnavailableMessage(void *arg0) {
    drawMenuColoredGlyphScript(-0x78, -0x58, raceGhostUnavailableText, 1, 0x100, 5, 0x28);
}

void updateRaceGhostUnavailableMessage(void *arg0) {
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceGhostUnavailableMessage, arg0);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/main_menu_panel_ui/drawMainMenuSettingsPanel.s")

void drawMainMenuSettingsOptions(void *arg0) {
    s32 var_v0;
    s32 sp28;

    var_v0 = 1;
    sp28 = var_v0;
    if (gMenuFlowState == 0) {
        var_v0 = 2;
        sp28 = var_v0;
    }
    drawAssetTableSpriteWithExplicitPalette(-0x78, -0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 4, sp28);
    sp28 = 1;
    if (gRaceRecordSettingsEnabled == 1) {
        drawAssetTableSpriteWithExplicitPalette(0x10, -0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 7, 2);
        drawAssetTableSpriteWithExplicitPalette(0x48, -0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 8, 1);
        if (gMenuFlowState == 0) {
            drawPulsingAssetTableSprite(0x10, -0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        } else {
            drawAssetTableSprite(0x10, -0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        }
    } else {
        drawAssetTableSpriteWithExplicitPalette(0x10, -0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 7, 1);
        drawAssetTableSpriteWithExplicitPalette(0x48, -0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 8, 2);
        if (gMenuFlowState == 0) {
            drawPulsingAssetTableSprite(0x48, -0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        } else {
            drawAssetTableSprite(0x48, -0x10, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        }
    }
    if (gMenuFlowState == 1) {
        sp28 = 2;
    }
    drawAssetTableSpriteWithExplicitPalette(-0x78, 8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 5, sp28);
    sp28 = 1;
    if (gRaceCourseModelEffectsDisabled == 0) {
        drawAssetTableSpriteWithExplicitPalette(0x10, 8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 9, 2);
        drawAssetTableSpriteWithExplicitPalette(0x48, 8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xA, 1);
        if (gMenuFlowState == 1) {
            drawPulsingAssetTableSprite(0x10, 8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        } else {
            drawAssetTableSprite(0x10, 8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        }
    } else {
        drawAssetTableSpriteWithExplicitPalette(0x10, 8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 9, 1);
        drawAssetTableSpriteWithExplicitPalette(0x48, 8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xA, 2);
        if (gMenuFlowState == 1) {
            drawPulsingAssetTableSprite(0x48, 8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        } else {
            drawAssetTableSprite(0x48, 8, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        }
    }
    if (gMenuFlowState == 2) {
        sp28 = 2;
    }
    drawAssetTableSpriteWithExplicitPalette(-0x78, 0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 6, sp28);
    sp28 = 1;
    if (gRaceCourseOverlayEffectsDisabled == 0) {
        drawAssetTableSpriteWithExplicitPalette(0x10, 0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 9, 2);
        drawAssetTableSpriteWithExplicitPalette(0x48, 0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xA, 1);
        if (gMenuFlowState == 2) {
            drawPulsingAssetTableSprite(0x10, 0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        } else {
            drawAssetTableSprite(0x10, 0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        }
    } else {
        drawAssetTableSpriteWithExplicitPalette(0x10, 0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 9, 1);
        drawAssetTableSpriteWithExplicitPalette(0x48, 0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xA, 2);
        if (gMenuFlowState == 2) {
            drawPulsingAssetTableSprite(0x48, 0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        } else {
            drawAssetTableSprite(0x48, 0x20, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 0xB);
        }
    }
    if (gMenuFlowState == 3) {
        sp28 = 2;
    }
    drawAssetTableSpriteWithExplicitPalette(-0x68, 0x48, getRelocatableHeapBlockBase(gAssetHandles[0x28]), 2, sp28);
}

void updateMainMenuSettingsPanel(MenuPanelActor *arg0) {
    addRenderCallback(&gMenuRenderCallbackList, drawMainMenuSettingsPanel, arg0);
    addRenderCallback(&gMenuRenderCallbackList, drawMainMenuSettingsOptions, arg0);
}

void initMainMenuSettingsPanel(MenuPanelActor *arg0) {
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateMainMenuSettingsPanel);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/main_menu_panel_ui/drawRaceRecordSettingsPanel.s")

void updateRaceRecordSettingsPanel(MenuPanelActor *arg0) {
    addRenderCallback(&gMenuRenderCallbackList, drawRaceRecordSettingsPanel, arg0);
}

void initRaceRecordSettingsPanel(MenuPanelActor *arg0) {
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceRecordSettingsPanel);
}
