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

extern s32 gMenuFlowState;

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
