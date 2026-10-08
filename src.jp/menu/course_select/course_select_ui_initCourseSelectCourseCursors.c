#include "game/menu/menu_scratch.h"
#include "game/race/race_state.h"
#include "font_encoding.h"
#include "common.h"
#include <PR/os_libc.h>
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/course_select/course_select_ui.h"
#include "game/menu/course_select/course_select_shop_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/math/fixed_point_math.h"
#include "game/race/player/race_player_model_renderer.h"
#include "game/race/player/race_player_input.h"

extern s16 gCourseSelectIconListXLayout[][4];
extern s16 gCourseSelectIconListYLayout[][6];

void initCourseSelectCourseCursors(CourseSelectCourseCursorsActor *arg0) {
    CourseSelectCourseCursorsActor *actor;
    u8 *courseUnlocked;
    s16 *yLayout;
    s16 *xLayout;
    char mask;
    s32 unlockState;
    s32 hasExtraCourse;
    s32 playerIndex;
    s32 layoutIndex;

    actor = arg0;
    mask = 0xFFFFFFFFFFFFFFFFu; \
    if ((s32)gPlayerCount < 3) { \
        layoutIndex = gPlayerCount - 1; \
    } else { \
        layoutIndex = 2; \
    } \
    playerIndex = 0; \
    if ((s32)gPlayerCount > 0) { \
        courseUnlocked = gCourseSelectHasExtraCourse; \
        yLayout = gCourseSelectIconListYLayout[layoutIndex]; \
        xLayout = gCourseSelectIconListXLayout[layoutIndex]; \
        do { \
            unlockState = *courseUnlocked & mask; \
            courseUnlocked++; \
            hasExtraCourse = (((0, unlockState) == 0) || (gRacePlayers[playerIndex].selectedCharacterId == 5)) ? 0 : 1;
            actor->horizontalSpacing[playerIndex] = yLayout[hasExtraCourse];
            actor->y[playerIndex] =
                yLayout[((playerIndex & 1) * 2) + hasExtraCourse + 2];
            actor->x[playerIndex] = xLayout[((playerIndex >= 2) * 2) + 1];
            actor->alpha[playerIndex] = 0;
            actor->state[playerIndex] = 0;
            actor->pulseTimer[playerIndex] = 0;
            playerIndex++;
        } while (playerIndex < (s32)gPlayerCount);
    }
    setCallbackTaskCallback(actor, (CallbackTaskCallback)updateCourseSelectCourseCursors);
}

void drawCourseSelectCourseListBackdrop(CourseSelectCourseListBackdropActor *arg0) {
    drawMenuSprite(arg0->x, arg0->y, getRelocatableHeapBlockBase(gAssetHandles[0x25]), 3, 0x20, 0x20, 0, 0);
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        4,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        arg0->x,
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        5,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        6,
        0x20,
        0x20,
        0,
        0
    );
}
