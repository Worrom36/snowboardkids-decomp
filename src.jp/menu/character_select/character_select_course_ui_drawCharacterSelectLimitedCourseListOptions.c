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
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/character_select/character_select_course_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/player/race_player_input.h"

#define CHARACTER_SELECT_FRAME_TEXTURE_HANDLE (gAssetHandles[0x21])

void drawCharacterSelectLimitedCourseListOptions(CharacterSelectCourseMenuFrameActor *arg0) {
    register CharacterSelectCourseMenuFrameActor *actor;
    void *arrowTexture;
    volatile u8 pad;
    u16 tile;
    s32 arrowBase;
    s32 lastArrowIndex;
    s32 i;
    s32 alpha;

    actor = arg0;
    arrowBase = (gRaceTypeSelection < 2) ? 2 : 0;
    i = 0;
    lastArrowIndex = arrowBase + 1;
    if (actor->itemCount > 0) {
        do {
            alpha = 0;
            if (i == 0) {
                tile = (gRaceTypeSelection == 2) ? 0x1D : 0x1C;
                if ((i == gRaceCourseIndex.signedValue) && (gMenuSelectionConfirmTimer > 0) &&
                    (gMenuSelectionConfirmTimer < 8) && (gMenuSelectionConfirmTimer & 1)) {
                    alpha = 0xFF;
                }
                drawMenuSprite(
                    actor->x[i],
                    actor->y[i],
                    getRelocatableHeapBlockBase(CHARACTER_SELECT_FRAME_TEXTURE_HANDLE),
                    tile,
                    0x20,
                    0x20,
                    0,
                    alpha
                );
            } else if (i == lastArrowIndex) {
                if ((i == gRaceCourseIndex.signedValue) && (gMenuSelectionConfirmTimer > 0) &&
                    (gMenuSelectionConfirmTimer < 8) && (gMenuSelectionConfirmTimer & 1)) {
                    alpha = 0xFF;
                }
                drawMenuSprite(
                    actor->x[i],
                    actor->y[i],
                    getRelocatableHeapBlockBase(CHARACTER_SELECT_FRAME_TEXTURE_HANDLE),
                    0x20,
                    0x20,
                    0x20,
                    0,
                    alpha
                );
            } else {
                if ((i == gRaceCourseIndex.signedValue) && (gMenuSelectionConfirmTimer > 0) &&
                    (gMenuSelectionConfirmTimer < 8) && (gMenuSelectionConfirmTimer & 1)) {
                    alpha = 0xFF;
                }
                tile = i + 0x12;
                drawMenuSprite(
                    actor->x[i],
                    actor->y[i],
                    getRelocatableHeapBlockBase(CHARACTER_SELECT_FRAME_TEXTURE_HANDLE),
                    tile,
                    0x20,
                    0x20,
                    0,
                    alpha
                );
            }

            if (i != lastArrowIndex) {
                arrowTexture = getRelocatableHeapBlockBase(CHARACTER_SELECT_FRAME_TEXTURE_HANDLE);
                tile = i + 0x29;
                drawMenuSprite((s16)(actor->x[i] - 0x10), actor->y[i], arrowTexture, tile, 0x20, 0x20, 0, 0);
            }
            i++;
        } while (i < actor->itemCount);
    }
}
