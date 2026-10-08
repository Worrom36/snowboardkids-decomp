#include "game/menu/menu_scratch.h"
#include "common.h"
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

extern ControllerPakRaceRecordSaveUiData gControllerPakRaceRecordSaveUiData;

void drawControllerPakRaceRecordSaveExitMessage(ControllerPakRaceRecordSaveExitMessageActor *arg0) {
    drawMenuGlyphScriptDefaultFont(arg0->common.x, arg0->common.y, gControllerPakRaceRecordSaveExitMessage, 1, arg0->alpha);
}
