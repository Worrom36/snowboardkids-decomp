#include "game/race/race_state.h"
#include "font_encoding.h"
#include "common.h"
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
