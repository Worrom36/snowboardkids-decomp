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

void updateRaceSetupSaveChoicePrompts(TitleMenuTransitionActor *arg0) {
    TitleMenuTransitionActor *actor;
    s32 i;
    s32 step;
    s32 stepAmount;
    u32 new_var;
    s16 state;

    actor = arg0;
    i = 0;
    if (gPlayerCount > 0) {
        do {
            if (gRaceSetupMenuSubState.pendingStatusCodes[i] != actor->selection[i]) {
                actor->selection[i] = gRaceSetupMenuSubState.pendingStatusCodes[i];
            }

            state = gMenuChoicePromptState[i];
            new_var = 4;
            if ((state != 0) && ((state ^ 0) != 3) && (state != new_var)) {
                if (state < 5) {
                    step = 1;
                } else {
                    step = -1;
                }

                stepAmount = step * 8;
                actor->slideOffset[i] += stepAmount;
                state = actor->slideOffset[i];
                new_var = 0x20;
                if (state == new_var) {
                    gMenuChoicePromptState[i] += 2;
                    actor->alphaTimer[i] = 0;
                    actor->alpha[i] = 0x100;
                    state = actor->slideOffset[i];
                }

                if (stepAmount == 8) {
                    if (state < 0x18) {
                        actor->y[i] += 8;
                    }
                    actor->topY[i] += 8;
                    state = gMenuChoicePromptState[i];
                } else {
                    if (state < 0x10) {
                        actor->y[i] -= 8;
                    }
                    actor->topY[i] -= 8;
                    state = gMenuChoicePromptState[i];
                }
            }

            new_var = 4;
            if (((state ^ 0) == 3) || (state == new_var)) {
                if ((s32)(u16)actor->alphaTimer[i] < 0x10) {
                    actor->alpha[i] -= 9;
                } else {
                    actor->alpha[i] += 9;
                }
                actor->alphaTimer[i] = ((u16)(*actor).alphaTimer[i] + 1) & 0x1F;
            }
            i++;
        } while (i < gPlayerCount);

        if (((!state) && (!state)) && (!state)) {}
    }

    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceSetupSaveChoicePrompts, (void *)arg0);
}
