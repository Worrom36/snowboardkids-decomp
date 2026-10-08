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

void drawCourseSelectCourseCursors(CourseSelectCourseCursorsActor *arg0) {
    u8 *actor;
    s16 *drawPosPtr;
    s32 i;
    u8 *statePtr;
    s16 *posPtr;
    int cursorStride;
    s16 *handles;
    u8 *cursorBytes;
    s32 tileIndex;
    s32 handleIndex;
    s32 one;
    s32 playerCount;
    u8 *cursorBase;

    actor = (u8 *)arg0;
    if (gCourseSelectInputLocked == 0) {
        playerCount = gPlayerCount;
        i = 0;
        if ((playerCount - 1) >= 0) {
            statePtr = (u8 *)arg0;
            cursorStride = 2;
            handles = gAssetHandles;
            one = 1;
            do {
                if (statePtr[0x30] < cursorStride) {
                    cursorBase = actor;
                    posPtr = (s16 *)((cursorBytes = cursorBase) + (i * cursorStride));
                    if (one == gPlayerCount) {
                        tileIndex = 7;
                        handleIndex = 0x25;
                    } else {
                        tileIndex = 0x13;
                        handleIndex = 0x21;
                    }
                    drawMenuSpriteWithAlpha(
                        posPtr[0xC],
                        (s16)((drawPosPtr = posPtr)[0x10] + (gCharacterSelectHighlightedRosterIndices[i] * statePtr[0x38])),
                        getRelocatableHeapBlockBase(handles[handleIndex]),
                        tileIndex,
                        0x20,
                        0x20,
                        0,
                        drawPosPtr[0x14],
                        (short)0
                    );
                }
                i++;
                statePtr++;
            } while (i < gPlayerCount);
        }
    }
}
