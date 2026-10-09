#include "common.h"
#include "font_encoding.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/controller_pak/controller_pak_continue_prompt_flow.h"
#include "game/menu/controller_pak/controller_pak_menu.h"
#include "game/menu/controller_pak/controller_pak_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/race/player/race_player_input.h"

extern u8 gControllerPakMenuCursorState;

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/func_80030490.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/updateControllerPakContinuePrompt.s")

void initControllerPakContinuePrompt(ControllerPakOptionsActor *arg0) {
    arg0->common.x = -0x70;
    arg0->common.y = -0x28;
    arg0->selectedOption = 0;
    arg0->blinkState = 0;
    arg0->scale = 0x100;
    arg0->timer = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakContinuePrompt);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/func_80030A78.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/updateControllerPakRumbleCheckPrompt.s")

void initControllerPakRumbleCheckPrompt(ControllerPakRumbleCheckPromptActor *arg0) {
    arg0->common.x = -0x70;
    arg0->common.y = -0x1C;
    arg0->state = CONTROLLER_PAK_RUMBLE_INITIAL_PROBE;
    arg0->scale = 0;
    arg0->timer = 0;
    arg0->messageIndex = CONTROLLER_PAK_RUMBLE_MESSAGE_INSERT;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakRumbleCheckPrompt);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/drawControllerPakFileDeleteMainOptions.s")

void updateControllerPakFileDeleteMainOptionsUi(ControllerPakOptionsActor *arg0) {
    switch (gControllerPakMenuCursorState) {
        case 0:
            if (arg0->timer < 0x10) {
                arg0->scale -= 8;
            } else {
                arg0->scale += 8;
            }
            arg0->timer = (arg0->timer + 1) & 0x1F;
            break;
        case 1:
        case 2:
            arg0->scale = 0x100;
            arg0->timer = 0;
            break;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawControllerPakFileDeleteMainOptions, (void *)arg0);
}

void initControllerPakFileDeleteMainOptions(ControllerPakOptionsActor *arg0) {
    arg0->common.x = -0x68;
    arg0->common.y = -0x44;
    arg0->scale = 0x100;
    arg0->timer = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakFileDeleteMainOptionsUi);
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/drawControllerPakFileDeleteConfirmOptions.s")
// clang-format on

void updateControllerPakFileDeleteConfirmOptionsUi(ControllerPakOptionsActor *arg0) {
    switch (gControllerPakMenuCursorState) {
        case 0:
        case 1:
            break;
        case 2:
            if (arg0->timer < 0x10) {
                arg0->scale -= 8;
            } else {
                arg0->scale += 8;
            }
            arg0->timer = (arg0->timer + 1) & 0x1F;
            break;
    }
    addRenderCallback(
        &gMenuRenderCallbackList,
        (RenderCallback)drawControllerPakFileDeleteConfirmOptions,
        (void *)arg0
    );
}

void initControllerPakFileDeleteConfirmOptions(ControllerPakOptionsActor *arg0) {
    arg0->common.x = -0x68;
    arg0->common.y = 0x48;
    arg0->scale = 0x100;
    arg0->timer = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakFileDeleteConfirmOptionsUi);
}

void drawControllerPakFileDeleteFreeSpaceInfo(ControllerPakTwoPointActor *arg0) {
    char padBefore[4];
    u16 text[4];
    char padAfter[8];
    s32 value;
    s32 i;

    // IDO register allocation nudge.
    if (1) {
        i = 0;
        do {
            text[i] = 0xFFFE;
            i++;
        } while (i < 4);

        value = gControllerPakFreeFileCount;
        i = 0;
        do {
            text[1 - i] = value % 10;
            value = value / 10;
            i++;
        } while (value != 0);
        text[2] = 0xFFFF;
        drawMenuGlyphScript(arg0->common.x, arg0->common.y, text, 1, 0x100, 8);
        i = 0;
    }
    do {
        text[i] = 0xFFFE;
        i++;
    } while (i < 4);

    value = gControllerPakFreeBytes / 0x100;
    i = 0;
    do {
        text[2 - i] = value % 10;
        value = value / 10;
        i++;
    } while (value != 0);
    text[3] = 0xFFFF;
    drawMenuGlyphScript(arg0->x2, arg0->y2, text, 1, 0x100, 8);
}

void updateControllerPakFileDeleteFreeSpaceInfo(ControllerPakTwoPointActor *arg0) {
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawControllerPakFileDeleteFreeSpaceInfo, arg0);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/func_800319D0.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/func_80031A10.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/func_80031D9C.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/func_80031E38.s")

void drawControllerPakFileDeleteIcon(ControllerPakSpriteActor *arg0) {
    drawMenuTilemapSprite(&arg0->sprite.render, MENU_TILEMAP_TEXEL_4B, arg0->common.x, arg0->common.y);
}

void updateControllerPakFileDeleteIcon(ControllerPakSpriteActor *arg0) {
    addRenderCallback(&gMenuOverlayRenderCallbackList, (RenderCallback)drawControllerPakFileDeleteIcon, arg0);
}

void initControllerPakFileDeleteIcon(ControllerPakSpriteActor *arg0) {
    initMenuTilemapSprite(&arg0->sprite, getRelocatableHeapBlockBase(gAssetHandles[0x26]));
    arg0->sprite.render.scrollX = 0x10;
    arg0->sprite.render.scrollY = 0x10;
    arg0->common.x = arg0->sprite.render.tilemapWidth;
    arg0->common.y = arg0->sprite.render.tilemapHeight;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakFileDeleteIcon);
}

void drawControllerPakMessageIcon(ControllerPakSpriteActor *arg0) {
    drawMenuTilemapSprite(&arg0->sprite.render, MENU_TILEMAP_TEXEL_8B, arg0->common.x, arg0->common.y);
}

void updateControllerPakMessageIcon(ControllerPakSpriteActor *arg0) {
    addRenderCallback(&gMenuOverlayRenderCallbackList, (RenderCallback)drawControllerPakMessageIcon, arg0);
}

void initControllerPakMessageIcon(ControllerPakSpriteActor *arg0) {
    initMenuTilemapSprite(&arg0->sprite, getRelocatableHeapBlockBase(gAssetHandles[0x26]));
    arg0->sprite.render.scrollX = 0x10;
    arg0->sprite.render.scrollY = 0x20;
    arg0->common.x = arg0->sprite.render.tilemapWidth;
    arg0->common.y = arg0->sprite.render.tilemapHeight;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakMessageIcon);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/func_80032038.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_ui/updateControllerPakFileDeleteErrorPromptUi.s")
// clang-format on

void initControllerPakFileDeleteErrorPrompt(ControllerPakDeletePromptActor *arg0) {
    arg0->common.x = -0x70;
    arg0->common.y = -0x1C;
    arg0->timer = 0;
    arg0->scale = 0;
    arg0->selectedOption = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakFileDeleteErrorPromptUi);
}
