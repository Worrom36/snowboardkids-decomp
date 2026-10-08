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

#define SAVE_STATUS_FADE_STEP 0x26
#define SAVE_STATUS_TRANSITION_NONE 0
#define SAVE_STATUS_TRANSITION_FADE_IN 1
#define SAVE_STATUS_TRANSITION_FADE_OUT 2
#define SAVE_STATUS_TRANSITION_DONE 3

void updateRaceSetupSaveStatusWidgets(TitleMenuWidgetActor *arg0) {
    s32 transitionStateSum;
    TitleMenuWidgetActor *actor;
    s32 playerIndex;
    TitleMenuWidgetActor *panelPositions;

    panelPositions = gMenuScratch0.saveWidgets;
    actor = arg0;
    for (playerIndex = 0; playerIndex < (s32)gPlayerCount; playerIndex++) {
        actor->x[playerIndex] = panelPositions->x[playerIndex];
        actor->y[playerIndex] = panelPositions->y[playerIndex];

        if (gRaceSetupMenuSubState.statusTransitionStates[playerIndex] != actor->statusTransitionStates[playerIndex]) {
            actor->statusTransitionStates[playerIndex] = gRaceSetupMenuSubState.statusTransitionStates[playerIndex];
            actor->nextStatusCodes[playerIndex] = gRaceSetupMenuSubState.nextStatusCodes[playerIndex];
        }

        switch (actor->statusTransitionStates[playerIndex]) {
            case SAVE_STATUS_TRANSITION_FADE_IN:
                actor->alpha[playerIndex] += SAVE_STATUS_FADE_STEP;
                if (actor->alpha[playerIndex] >= 0x100) {
                    actor->alpha[playerIndex] = 0x100;
                    actor->statusTransitionStates[playerIndex] = SAVE_STATUS_TRANSITION_NONE;
                }
                break;
            case SAVE_STATUS_TRANSITION_FADE_OUT:
                actor->alpha[playerIndex] -= SAVE_STATUS_FADE_STEP;
                if (actor->alpha[playerIndex] <= 0) {
                    actor->alpha[playerIndex] = 0;
                    actor->statusTransitionStates[playerIndex] = SAVE_STATUS_TRANSITION_FADE_IN;
                    gControllerPakStatusCodes[playerIndex] = arg0->nextStatusCodes[playerIndex];
                }
                break;
            case SAVE_STATUS_TRANSITION_NONE:
            case SAVE_STATUS_TRANSITION_DONE:
                break;
        }

        gRaceSetupMenuSubState.statusTransitionStates[playerIndex] = actor->statusTransitionStates[playerIndex];
    }

    arg0->frame = (arg0->frame + 1) & 0xF;
    transitionStateSum = 0;
    for (playerIndex = 0; playerIndex < (s32)gPlayerCount; playerIndex++) {
        transitionStateSum += actor->statusTransitionStates[playerIndex];
    }

    if (transitionStateSum == (gPlayerCount * SAVE_STATUS_TRANSITION_DONE)) {
        removeCallbackTask(arg0);
    } else {
        addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawRaceSetupSaveStatusWidgets, (void *)arg0);
    }
}

void initRaceSetupSaveStatusWidgets(TitleMenuWidgetActor *arg0) {
    s32 i;
    TitleMenuWidgetActor *new_var;

    arg0->x[0] = -0x114;
    arg0->y[0] = -0xA4;
    arg0->x[1] = -0x114;
    arg0->y[1] = 0x48;
    arg0->x[2] = 0x90;
    new_var = arg0;
    new_var->y[2] = -0xA4;
    new_var->x[3] = 0x90;
    new_var->y[3] = 0x48;

    for (i = 0; i < gPlayerCount; i++) {
        new_var->alpha[i] = 0x100;
        new_var->statusTransitionStates[i] = SAVE_STATUS_TRANSITION_NONE;
    }

    setCallbackTaskCallback(new_var, (CallbackTaskCallback)updateRaceSetupSaveStatusWidgets);
}
