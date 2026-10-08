#include "game/menu/menu_scratch.h"
#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/audio/sound_manager.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/race_setup/race_setup_menu.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/race/player/race_player_input.h"

void initRaceSetupSaveChoicePrompts(RectListActor *arg0) {
    RectListActor *actor = arg0;

    actor->rects[0].x0 = -0x70;
    actor->rects[1].x0 = -0x34;
    actor->rects[2].x0 = -0x34;
    actor->rects[0].y0 = -0x70;
    actor->rects[1].y0 = 0x30;
    actor->rects[2].y0 = 0x30;
    actor->rects[0].x1 = 0x1C;
    actor->rects[1].x1 = -0x34;
    actor->rects[2].x1 = -0x34;
    actor->rects[0].y1 = 0x1C;
    actor->rects[1].y1 = 0x30;
    actor->rects[2].y1 = 0x30;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSetupSaveChoicePrompts);
}

void drawScrollingMenuBackdropActor(void *arg0) {
    SpriteActor *actor = arg0;

    drawMenuTilemapSprite(&actor->sprite.render, MENU_TILEMAP_TEXEL_4B, actor->x, actor->y);
}
