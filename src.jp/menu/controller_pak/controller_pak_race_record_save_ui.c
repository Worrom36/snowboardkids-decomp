#include "common.h"
#include "game/menu/menu_scratch.h"
#include <PR/os_libc.h>
#include "font_encoding.h"
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/menu/controller_pak/controller_pak_race_record_save_flow.h"
#include "game/menu/controller_pak/controller_pak_race_record_save_ui.h"
#include "game/menu/character_select/character_select_course_menu.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/race/player/race_player_input.h"

#define CONTROLLER_PAK_RACE_RECORD_SAVE_SCORE_TEXTURE_HANDLE (gAssetHandles[0x21])
#define gControllerPakRaceRecordSaveExitMessage (gControllerPakRaceRecordSaveUiData.exitMessage)

typedef struct {
    MenuGlyphScript statusMessagePrefix[0x2CB];
    u8 choicePromptSprites[0x26];
} ControllerPakRaceRecordSaveUiDataStorage;

typedef struct {
    MenuGlyphScript statusMessages[0x13][0x26];
    u8 choicePromptSpriteSuffix[0x18];
} ControllerPakRaceRecordSaveUiDataTyped;

typedef union {
    ControllerPakRaceRecordSaveUiDataTyped typed;
    ControllerPakRaceRecordSaveUiDataStorage storage;
} ControllerPakRaceRecordSaveUiDataContent;

typedef struct {
    ControllerPakRaceRecordSaveUiDataContent content;
    MenuGlyphScript exitMessage[10];
} ControllerPakRaceRecordSaveUiData;

extern const char D_800E0F30[];
extern s16 gControllerPakRaceRecordSaveStatusTransitionAlpha;
extern ControllerPakRaceRecordSaveUiData gControllerPakRaceRecordSaveUiData;

void drawControllerPakRaceRecordSaveScorePanel(ControllerPakRaceRecordSaveScorePanelActor *arg0) {
    s16 *new_var;
    s16 *badgeIndex;
    s32 xOffset;
    s32 tile;
    s32 alpha;
    s32 i;
    char text[8];
    s32 next;
    s32 count;

    drawMenuSprite(
        arg0->common.x,
        arg0->common.y,
        getRelocatableHeapBlockBase(CONTROLLER_PAK_RACE_RECORD_SAVE_SCORE_TEXTURE_HANDLE),
        0xA,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->common.x + 0x40),
        arg0->common.y,
        getRelocatableHeapBlockBase(CONTROLLER_PAK_RACE_RECORD_SAVE_SCORE_TEXTURE_HANDLE),
        0xB,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->common.x + 0x80),
        arg0->common.y,
        getRelocatableHeapBlockBase(CONTROLLER_PAK_RACE_RECORD_SAVE_SCORE_TEXTURE_HANDLE),
        0xC,
        0x20,
        0x20,
        0,
        0
    );

    text[0] = '1';
    text[1] = 0;
    drawMenuAsciiText((s16)(arg0->common.x + 0x32), (s16)((unsigned long)(arg0->common.y + 2)), text, 0, 0x100);

    sprintf(text, D_800E0F30, gRacePlayers[0].money);
    drawMenuAsciiText((s16)(arg0->common.x + 0x44), (s16)(arg0->common.y + 0x1B), text, 0, 0x100);

    if (gGameSaveDataBuffer[0].progressionLevel == 3) {
        count = 3;
    } else {
        count = gGameSaveDataBuffer[0].progressionLevel + 1;
    }

    i = 0;
    if (count > 0) {
        next = 0;
        do {
            xOffset = 1;
            xOffset = i + xOffset;
            if ((s32)gGameSaveDataBuffer[0].progressionLevel < xOffset) {
                alpha = 0x70;
            } else {
                alpha = 0x100;
            }

            drawMenuSpriteWithAlpha(
                (s16)(arg0->common.x + next + 4),
                (s16)(arg0->common.y + 0x11),
                getRelocatableHeapBlockBase(CONTROLLER_PAK_RACE_RECORD_SAVE_SCORE_TEXTURE_HANDLE),
                0x19,
                0x20,
                0x20,
                0,
                alpha,
                9 - i
            );
            i = xOffset;
            next += 0x10;
        } while (xOffset != count);
        i = 0;
    }

    count = 6;
    if (gGameSaveDataBuffer[0].progressionLevel == 1) {
        count = 7;
    } else if (gGameSaveDataBuffer[0].progressionLevel == 2) {
        count = 8;
    } else {
        i = 0;
        if (gGameSaveDataBuffer[0].progressionLevel == 3) {
            count = 9;
        }
    }

    if (count > 0) {
        xOffset = 0;
        new_var = gCharacterSelectCourseMenuData.playerBadgeDisplayOrder;
        badgeIndex = new_var;
        do {
            tile = gGameSaveDataBuffer[0].cupPlacements[*badgeIndex];
            alpha = 0x70;
            if (tile != 0) {
                alpha = 0x100;
                tile += 6;
            } else {
                tile = 9;
                if ((!i) && (!i)) {}
            }
            drawMenuSpriteWithAlpha(
                (s16)(arg0->common.x + xOffset + 4),
                (s16)(arg0->common.y + 0x27),
                getRelocatableHeapBlockBase(CONTROLLER_PAK_RACE_RECORD_SAVE_SCORE_TEXTURE_HANDLE),
                i + 0x1A,
                0x20,
                0x20,
                0,
                alpha,
                tile
            );
            i++;
            xOffset += 0xE;
            badgeIndex++;
        } while (i != count);
    }
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_race_record_save_ui/updateControllerPakRaceRecordSaveScorePanel.s")
// clang-format on

void initControllerPakRaceRecordSaveScorePanel(ControllerPakRaceRecordSaveScorePanelActor *arg0) {
    arg0->common.x = 0x90;
    arg0->common.y = -0x20;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakRaceRecordSaveScorePanel);
}

void drawControllerPakRaceRecordSavePromptFrame(ControllerPakRaceRecordSavePromptFrameActor *arg0) {
    s32 zero = 0;
    u16 alpha;

    if (gControllerPakStatusCodes[0] != 8) {
        alpha = arg0->scale;
        drawMenuSpriteWithAlpha(
            (s16)(arg0->common.x - 2),
            (s16)(arg0->common.y + 0xC),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            8,
            0x20,
            0x20,
            zero,
            alpha,
            zero
        );
        drawMenuSpriteWithAlpha(
            (s16)(arg0->common.x + 0x3E),
            (s16)(arg0->common.y + 0xC),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            9,
            0x20,
            0x20,
            zero,
            alpha,
            zero
        );
    }
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_race_record_save_ui/updateControllerPakRaceRecordSavePromptFrame.s")
// clang-format on

void initControllerPakRaceRecordSavePromptFrame(ControllerPakRaceRecordSavePromptFrameActor *arg0) {
    arg0->common.x = -0x44;
    arg0->common.y = -0x1E;
    arg0->scale = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakRaceRecordSavePromptFrame);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_race_record_save_ui/func_8002BFBC.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_race_record_save_ui/updateControllerPakRaceRecordSaveStatusMessage.s")
// clang-format on

void initControllerPakRaceRecordSaveStatusMessage(ControllerPakRaceRecordSaveStatusMessageActor *arg0) {
    arg0->common.x = -0x42;
    arg0->common.y = -0xE;
    arg0->alpha = 0x100;
    gControllerPakRaceRecordSaveStatusTransitionAlpha = 0x100;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakRaceRecordSaveStatusMessage);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_race_record_save_ui/func_8002C2BC.s")

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_race_record_save_ui/updateControllerPakRaceRecordSaveStatusChoicePrompt.s")
// clang-format on

void initControllerPakRaceRecordSaveStatusChoicePrompt(ControllerPakRaceRecordSaveStatusChoicePromptActor *arg0) {
    arg0->common.x = -0x28;
    arg0->common.y = 0xC;
    arg0->bottomChoiceY = 0xC;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakRaceRecordSaveStatusChoicePrompt);
}

void drawControllerPakRaceRecordSaveExitMessage(ControllerPakRaceRecordSaveExitMessageActor *arg0) {
    drawMenuGlyphScriptDefaultFont(
        arg0->common.x,
        arg0->common.y,
        gControllerPakRaceRecordSaveExitMessage,
        1,
        arg0->alpha
    );
}

// clang-format off
#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/controller_pak/controller_pak_race_record_save_ui/updateControllerPakRaceRecordSaveExitMessage.s")
// clang-format on

void initControllerPakRaceRecordSaveExitMessage(ControllerPakRaceRecordSaveExitMessageActor *arg0) {
    arg0->common.x = -0x24;
    arg0->common.y = -0x38;
    arg0->alpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateControllerPakRaceRecordSaveExitMessage);
}
