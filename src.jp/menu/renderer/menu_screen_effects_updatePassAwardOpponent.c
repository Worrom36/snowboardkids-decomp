#include "game/race/ui/race_ui_effects.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/race_state.h"
#include "common.h"
#include "game/audio/sound_manager.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/engine/system_runtime.h"
#include "game/engine/viewport_manager.h"
#include "game/math/fixed_point_math.h"
#include "game/menu/renderer/menu_screen_effects.h"
#include "game/menu/main_menu/main_menu_scene_model.h"
#include "game/menu/main_menu/main_menu_scene_model_renderer.h"
#include "game/race/player/race_player_input.h"

void updatePassAwardOpponent(MenuScreenEffectActor *arg0) {
    s16 temp = arg0->unk1C.half.hi;

    if (temp == 0) {
        loopMainMenuSceneModelAnimation(arg0->unk18.word);
    } else {
        arg0->unk1C.half.hi = temp - 1;
    }
    addMainMenuSceneModelDrawCallbackForViewport(arg0->unk18.word, 1);
}
