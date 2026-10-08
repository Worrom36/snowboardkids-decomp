#include "common.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/main_menu/main_menu_title_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/race_setup/race_setup_ui.h"

void drawMainMenuTitleLogo(MainMenuTitleLogoActor *arg0) {
    drawMenuTilemapSprite(&arg0->tilemapSprite.render, MENU_TILEMAP_TEXEL_8B, arg0->common.x, arg0->common.y);
}

void updateMainMenuTitleLogo(MainMenuTitleLogoActor *arg0) {
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawMainMenuTitleLogo, arg0);
}
