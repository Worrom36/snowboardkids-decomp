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

extern void drawTitleMenuSparkle(MenuScreenEffectActor *);
extern s16 titleMenuSparklePositions[2];
extern void renderTitleMenuSparkle(MenuScreenEffectActor *arg0);

void renderTitleMenuSparkle(MenuScreenEffectActor *arg0) {
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawTitleMenuSparkle, arg0);
}

void initTitleMenuSparkle(MenuScreenEffectActor *arg0) {
    s16 *temp = &titleMenuSparklePositions[arg0->index * 2];

    arg0->unk18.half.hi = temp[0];
    arg0->unk18.half.lo = temp[1];
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)renderTitleMenuSparkle);
}
