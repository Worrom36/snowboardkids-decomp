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

void drawCourseSelectExtraCourseIconList(CourseSelectExtraCourseIconListActor *arg0) {
    s32 overlayTile;
    s32 iconAlpha;
    s32 playerIndex;
    s32 iconIndex;

    for (playerIndex = 0; playerIndex < gPlayerCount; playerIndex++) {
        if (gMenuChoicePromptState[playerIndex] != 0) {
            if (gMenuChoicePromptState[playerIndex] != 9) {
                overlayTile = (playerIndex >= 2) * 0x8C;
                for (iconIndex = arg0->itemCounts[playerIndex] - 1; iconIndex >= 0; iconIndex--) {
                    if (gMenuChoicePromptState[playerIndex] == 1) {
                        iconAlpha = 0x100;
                    } else if ((iconIndex + 2) == gMenuChoicePromptState[playerIndex]) {
                        iconAlpha = 0x100;
                    } else {
                        iconAlpha = 0x60;
                    }
                    drawMenuSpriteWithAlphaClipped(
                        arg0->iconX[playerIndex][iconIndex],
                        arg0->iconY[playerIndex][iconIndex],
                        getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                        arg0->tileIndices[playerIndex][iconIndex],
                        0x20,
                        0x20,
                        0,
                        iconAlpha,
                        0,
                        arg0->clipLeft - overlayTile,
                        arg0->clipTop,
                        arg0->clipRight,
                        arg0->clipBottom
                    );
                }
                if (gCourseSelectInputLocked == 0) {
                    if ((gMenuChoicePromptState[playerIndex] >= 2) && (gMenuChoicePromptState[playerIndex] < 5)) {
                        if (gPlayerCount == 1) {
                            overlayTile = 0xF;
                        } else {
                            overlayTile = 0x1B;
                        }
                        drawMenuSpriteWithAlpha(
                            arg0->iconX[playerIndex][0],
                            (s16)(arg0->iconY[playerIndex][0] +
                                  ((gMenuChoicePromptState[playerIndex] - 2) * arg0->rowSpacing[playerIndex])),
                            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                            overlayTile,
                            0x20,
                            0x20,
                            0,
                            ((CourseSelectExtraCourseIconListActor *)&((u8 *)&arg0->task)[playerIndex * 2])->alpha[0],
                            0
                        );
                    }
                }
            }
        }
    }
}
