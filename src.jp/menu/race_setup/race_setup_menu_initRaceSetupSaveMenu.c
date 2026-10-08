#include "game/menu/menu_scratch.h"
#include "game/race/race_state.h"
#include "common.h"
#include "font_encoding.h"
#include "game/save_data.h"
#include "assets.h"
#include "game/engine/asset_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/audio/sound_manager.h"
#include "game/menu/race_setup/race_setup_menu.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/engine/viewport_manager.h"
#include "game/race/player/race_player_input.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/controller_pak/controller_pak_ui.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/course_select/multiplayer_course_select_menu.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"

extern s32 gMenuFlowState;
extern u8 gConnectedControllerCount;
extern s16 gRaceSetupSavePanelInitialRects[4][2];

void initRaceSetupSaveMenu(void) {
    s32 i;
    s32 connectedControllerCount;
    RacePlayer *player;

    for (i = 0; i < 4; i++) {
        gControllerPakStatusCodes[i] = 0;
        gMenuChoicePromptState[i] = 0;
        gControllerPakRetryCounts[i] = 0;
        gControllerPakOperationCounts[i] = 0;
    }

    gRaceSetupSavePanelCreateTimer = 0;
    do { \
        i = 0; \
        connectedControllerCount = gConnectedControllerCount; \
        if (connectedControllerCount > 0) { \
            player = gRacePlayers; \
            do { \
                player++; \
                player[-1].menuState = 0; \
                i++; \
            } while (player < &gRacePlayers[connectedControllerCount]); \
            i = 0; \
        } \
        do { \
            initRaceSetupPlayerSaveData(i); \
            i++; \
        } while (i < 4); \
        gMenuScratch0.task = 0; \
        gMenuScratch1.task = 0; \
    } while (0);
    gMenuScratch2.task = 0;
    gMenuSelectionConfirmTimer = 0;
    gMenuFlowState = 0;
    gRaceRumbleEnabled = 0;

    for (i = 0; i < 4; i++) {
        gRaceSetupSavePanelRects[0][i] = gRaceSetupSavePanelInitialRects[i][0];
        gRaceSetupSavePanelRects[1][i] = gRaceSetupSavePanelInitialRects[i][1];
    }

    setCurrentGameTaskCallback(updateRaceSetupSaveMenu, 0);
    updateCallbackTasks();
}
