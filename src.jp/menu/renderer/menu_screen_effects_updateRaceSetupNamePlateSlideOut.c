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

extern void drawRaceSetupNamePlate(MenuScreenEffectActor *);
extern void updateRaceSetupNamePlateSlideOut(MenuScreenEffectActor *);
extern void holdRaceSetupNamePlate(MenuScreenEffectActor *);

void updateRaceSetupNamePlateSlideOut(MenuScreenEffectActor *arg0) {
    s32 temp_v0 = arg0->unk24.word;
    s32 temp_t8 = (unsigned long long)(temp_v0 + 4);

    arg0->unk18.word += temp_v0;
    arg0->unk24.word = temp_t8;
    if (temp_t8 == 0x38) {
        removeCallbackTask(arg0);
    } else {
        addRenderCallback(&gRaceOverlayRenderCallbackList, (RenderCallback)drawRaceSetupNamePlate, (void *)arg0);
    }
}

void holdRaceSetupNamePlate(MenuScreenEffectActor *arg0) {
    u16 temp = (arg0->timer & 0xFFFFU) - 1;

    arg0->timer = temp;
    temp += 0;
    if (temp == 0) {
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSetupNamePlateSlideOut);
    }
    if (1) {}
    addRenderCallback(&gRaceOverlayRenderCallbackList, (RenderCallback)drawRaceSetupNamePlate, (void *)arg0);
}

void updateRaceSetupNamePlateSlideIn(MenuScreenEffectActor *arg0) {
    arg0->unk18.word += arg0->unk24.word;
    arg0->unk24.word -= 4;
    if (arg0->unk24.word == 0) {
        arg0->timer = 0x1E;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)holdRaceSetupNamePlate);
    }
    addRenderCallback(&gRaceOverlayRenderCallbackList, (RenderCallback)drawRaceSetupNamePlate, (void *)arg0);
}
