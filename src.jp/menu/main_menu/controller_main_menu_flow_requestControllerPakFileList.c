#include "common.h"
#include "game/save_data.h"
#include "assets.h"
#include "game/audio/sound_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/engine/system_runtime.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/controller_pak/controller_pak_menu.h"
#include "game/menu/renderer/menu_screen_effects.h"
#include "game/menu/main_menu/main_menu_panel_ui.h"
#include "game/menu/main_menu/main_menu_scene_model.h"
#include "game/menu/main_menu/main_menu_scene_model_renderer.h"
#include "game/menu/main_menu/main_menu_race_flow.h"
#include "game/menu/main_menu/training_course_race_flow.h"
#include "game/menu/main_menu/main_menu_title_ui.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/camera/race_camera.h"
#include "game/race/flow/race_flow.h"
#include "game/race/race_state.h"
#include "game/demo/main_menu_demo_race_intro.h"
#include "game/demo/title_demo_race_intro.h"
#include "game/engine/viewport_manager.h"
#include "game/menu/renderer/menu_renderer.h"
#include <PR/os_motor.h>

#define OS_MESG_BLOCK 1

extern OSMesgQueue gControllerSubsystemRequestQueue;
extern OSMesgQueue gControllerSubsystemReplyQueue;
extern OSMesgQueue gControllerEventQueue;
extern OSPfs gControllerPakHandles[];

void requestControllerPakFileList(void) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(&gControllerSubsystemRequestQueue, (OSMesg)CONTROLLER_REQUEST_LIST_FILES, OS_MESG_BLOCK);
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

void readControllerPakFileStates(void) {
    s32 i;

    osPfsInitPak(&gControllerEventQueue, &gControllerPakHandles[0], 0);
    for (i = 0; i != 0x10; i++) {
        osPfsFileState(&gControllerPakHandles[0], i, &gControllerPakFileStates[i]);
    }
}

void requestControllerPakDeleteFile(u16 arg0) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(&gControllerSubsystemRequestQueue, (OSMesg)(arg0 + CONTROLLER_REQUEST_DELETE_FILE), OS_MESG_BLOCK);
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

void deleteControllerPakFile(u16 arg0) {
    OSPfs *pfs;
    OSPfsState *state;
    u16 companyCode;
    u32 gameCode;
    u8 gameName[16];
    u8 extName[4];
    s32 i;

    pfs = gControllerPakHandles;
    osPfsInitPak(&gControllerEventQueue, pfs, 0);

    state = &gControllerPakFileStates[arg0];
    companyCode = state->company_code;
    gameCode = state->game_code;

    for (i = 0; i < 16; i++) {
        gameName[i] = gControllerPakFileStates[arg0].game_name[i];
    }

    for (i = 0; i < 4; i++) {
        extName[i] = gControllerPakFileStates[arg0].ext_name[i];
    }

    for (i = 0; i != 3; i++) {
        if (osPfsDeleteFile(pfs, companyCode, gameCode, gameName, extName) == 0) {
            gControllerPakRetryCounts[0] = 0;
            return;
        }
        gControllerPakRetryCounts[0]++;
    }
}

void requestControllerPakFreeSpaceUpdate(void) {
    OSMesg msg;

    msg = NULL;

    osSendMesg(&gControllerSubsystemRequestQueue, (OSMesg)CONTROLLER_REQUEST_UPDATE_FREE_SPACE, OS_MESG_BLOCK);
    osRecvMesg(&gControllerSubsystemReplyQueue, &msg, OS_MESG_BLOCK);
}

void updateControllerPakFreeSpaceInfo(void) {
    s32 pad;
    s32 maxFiles;
    s32 filesUsed;

    osPfsInitPak(&gControllerEventQueue, &gControllerPakHandles[0], 0);
    osPfsFreeBlocks(&gControllerPakHandles[0], &gControllerPakFreeBytes);
    osPfsNumFiles(&gControllerPakHandles[0], &maxFiles, &filesUsed);
    gControllerPakFreeFileCount = maxFiles - filesUsed;
}
