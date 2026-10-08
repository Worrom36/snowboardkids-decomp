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
#include <PR/os_cont.h>

#define OS_MESG_BLOCK 1

extern u8 gControllerReadPending;

extern OSMesgQueue gControllerSubsystemRequestQueue;

extern u8 gConnectedControllerBitmask;

extern OSContPad gControllerPads[];

void requestControllerRead(void) {
    if ((gControllerReadPending == 0) && (gConnectedControllerBitmask != 0)) {
        osSendMesg(&gControllerSubsystemRequestQueue, (OSMesg)CONTROLLER_REQUEST_READ_INPUT, OS_MESG_BLOCK);
        gControllerReadPending = 1;
    }
}

void updateControllerInputState(void) {
    u16 i;
    do { \
        i = 0; \
        do { \
            if ((((s32)gConnectedControllerBitmask) >> i) & 1) { \
                if (gControllerPads[i].errno == 0) { \
                    gControllerInputState[i] = *((ControllerInputState *)(&gControllerPads[i])); \
                } \
            } \
            i++; \
        } while (i < 4); \
        gControllerReadPending = 0; \
    } while (0);
}
