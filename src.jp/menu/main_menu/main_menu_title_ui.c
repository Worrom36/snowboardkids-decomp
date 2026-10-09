#include "common.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/main_menu/main_menu_title_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/race_setup/race_setup_ui.h"

#define ASSET_HANDLE(index) (gAssetHandles[(index)])

void drawMainMenuTitleLogo(MainMenuTitleLogoActor *arg0) {
    drawMenuTilemapSprite(&arg0->tilemapSprite.render, MENU_TILEMAP_TEXEL_8B, arg0->common.x, arg0->common.y);
}

void updateMainMenuTitleLogo(MainMenuTitleLogoActor *arg0) {
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawMainMenuTitleLogo, arg0);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/main_menu_title_ui/func_80032404.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/main_menu_title_ui/func_80032470.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/main_menu/main_menu_title_ui/updateMainMenuTitleOptions.s")

void initMainMenuTitleOptions(MainMenuTitleOptionsActor *arg0) {
    arg0->common.x = -0x2C;
    arg0->common.y = -0x2C;
    arg0->labelY = 0x20;
    arg0->unused1E = 0x30;
    arg0->selectedOption = 0;
    arg0->blinkTimer = 0;
    arg0->blinkAlpha = 0x100;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateMainMenuTitleOptions);
}

void drawMainMenuTitleCursor(MainMenuTitleCursorActor *arg0) {
    void *temp = getRelocatableHeapBlockBase(ASSET_HANDLE(33));

    drawMenuSprite(arg0->common.x, arg0->common.y, temp, 3, 0x20, 0x20, 0, 0);
}

void updateMainMenuTitleCursor(MainMenuTitleCursorActor *arg0) {
    if (gCurrentGameTask->callbackData1 != 2) {
        arg0->common.x = -0x34;
    } else {
        arg0->common.x = -0x3C;
    }
    arg0->common.y = (gCurrentGameTask->callbackData1 << 4) + 0x20;
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawMainMenuTitleCursor, arg0);
}

void initMainMenuTitleCursor(MainMenuTitleCursorActor *arg0) {
    arg0->common.y = 0x20;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateMainMenuTitleCursor);
}
