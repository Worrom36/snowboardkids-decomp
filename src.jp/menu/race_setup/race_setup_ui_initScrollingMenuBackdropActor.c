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

void initScrollingMenuBackdropActor(SpriteActor *arg0) {
    SpriteActor *temp_a2 = arg0;

    initMenuTilemapSprite(&temp_a2->sprite, getRelocatableHeapBlockBase(gAssetHandles[38]));
    temp_a2->x = temp_a2->sprite.render.tilemapWidth;
    temp_a2->y = temp_a2->sprite.render.tilemapHeight;
    setCallbackTaskCallback(temp_a2, (CallbackTaskCallback)updateScrollingMenuBackdropActor);
}
