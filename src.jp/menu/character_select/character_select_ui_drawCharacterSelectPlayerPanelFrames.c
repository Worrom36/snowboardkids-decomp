#include "game/menu/menu_scratch.h"
#include "common.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/character_select/character_select_ui.h"
#include "game/race/player/race_player_input.h"

#define CHARACTER_SELECT_UI_UNUSED_HANDLE (gAssetHandles[0x1F])
#define CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE (gAssetHandles[0x21])

extern const char gCharacterSelectPlayerNumberFormat[];

void drawCharacterSelectPlayerPanelFrames(CharacterSelectUiPlayerPanelFrameActor *actor) {
    s32 i;
    u16 alpha;
    register CharacterSelectUiPlayerPanelFrameActor *panel;
    s32 playerNumber;
    volatile s32 unusedStackPadding;
    char text[8];

    for (i = 0; i < 4; i++) {
        if ((gPlayerCount - 1) < i) {
            alpha = 0x50;
        } else {
            alpha = 0x100;
        }

        panel = actor;
        playerNumber = i + 1;
        drawMenuSpriteWithAlpha(
            actor->x[i],
            actor->y[i],
            getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE),
            0x23,
            0x20,
            0x20,
            0,
            alpha,
            playerNumber
        );
        drawMenuSpriteWithAlpha(
            (s16)(actor->x[i] + 0x40),
            actor->y[i],
            getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE),
            0x24,
            0x20,
            0x20,
            0,
            alpha,
            playerNumber
        );
        drawMenuSpriteWithAlpha(
            (s16)(panel->x[i] + 0x80),
            panel->y[i],
            getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_PLAYER_FRAME_HANDLE),
            0xC,
            0x20,
            0x20,
            0,
            alpha,
            playerNumber
        );

        sprintf(text, gCharacterSelectPlayerNumberFormat, playerNumber);
        drawMenuAsciiText((s16)(actor->x[i] + 0x32), (s16)(panel->y[i] + 2), (u8 *)text, 0, alpha);

        if (alpha != 0x100) {
            if ((!actor) && (!actor)) {}
            drawMenuSpriteWithAlpha(
                (s16)(actor->x[i] + 2),
                (s16)(actor->y[i] + 0x14),
                getRelocatableHeapBlockBase(CHARACTER_SELECT_UI_UNUSED_HANDLE),
                0x90,
                0x20,
                0x20,
                0,
                0xF0,
                0
            );
        }
    }
}
