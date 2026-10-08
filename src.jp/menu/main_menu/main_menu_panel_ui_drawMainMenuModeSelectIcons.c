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

#define mainMenuModeSelectTopRowIconTiles gMainMenuModeSelectByteData.topRowIconTiles
#define mainMenuModeSelectBottomRowIconTiles gMainMenuModeSelectByteData.bottomRowIconTiles

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

extern MainMenuModeSelectByteData gMainMenuModeSelectByteData;

extern u8 gMainMenuModeSelection;

void drawMainMenuModeSelectIcons(MenuPanelActor *arg0) {
    s32 alpha;
    s32 i;
    s32 next;
    s32 x;
    u8 *tile;

    if (arg0) {
    }

    x = -0x6E;
    tile = mainMenuModeSelectTopRowIconTiles; i = 0; do {
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
