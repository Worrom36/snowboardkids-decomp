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
