#include "game/race/race_state.h"
#include "game/engine/game_task_scheduler.h"
#include "font_encoding.h"
#include "common.h"
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

extern MenuGlyphScript mainMenuModeSelectTitleText[];

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
