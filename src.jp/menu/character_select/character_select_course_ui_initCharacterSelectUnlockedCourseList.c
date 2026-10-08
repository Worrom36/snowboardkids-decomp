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

typedef struct {
               u8 pad0[8];
               u8 next;
               u8 pad9[7];
               s16 xNext;
               s16 pad12;
               s16 pad14;
               s16 pad16;
               s16 pad18;
               s16 x0;
               s16 x1;
               s16 x2;
               u8 pad20[6];
               s16 y0;
               s16 y1;
               s16 y2;
               s16 y3;
} CharacterSelectCourseListInitLoop;

extern u8 gHighestUnlockedCourse;

void initCharacterSelectUnlockedCourseList(CharacterSelectCourseMenuFrameActor *arg0) {
    u8 spacing;
    s16 baseY;
    s32 i;
    int mode;
    unsigned int limit;
    CharacterSelectCourseListInitLoop *loop;

    mode = gHighestUnlockedCourse;
    if (mode == 0) {
        if (1) {}
        spacing = 0x18;
        baseY = -0x50;
    } else if (mode == 1) {
        spacing = 0x16;
        baseY = -0x54;
    } else {
        spacing = 0x13;
        if (mode == 2) {
            spacing = 0x16;
            baseY = -0x60;
        } else {
            baseY = -0x60;
        }
    }

    arg0->baseY = baseY;
    arg0->itemSpacing = spacing;
    do { \
        arg0->y[1] = baseY + spacing; \
        arg0->y[2] = baseY + (spacing * 2); \
        arg0->x[0] = -0x104; \
        arg0->x[1] = -0x104; \
        arg0->x[2] = -0x104; \
        arg0->x[2] = -0x104; \
        arg0->y[0] = baseY; \
        i = 3; \
        loop = (CharacterSelectCourseListInitLoop *)&((u8 *)&arg0->taskHeader)[6]; \
        limit = 0xB; \
        do { \
            loop->x0 = -0x104; \
            loop->x1 = -0x104; \
            loop->x2 = -0x104; \
            loop = (CharacterSelectCourseListInitLoop *)&loop->next; \
            loop->xNext = -0x104; \
            loop->y0 = baseY + (i * spacing); \
            loop->y1 = baseY + ((i + 1) * spacing); \
            loop->y2 = baseY + ((i + 2) * spacing); \
            loop->y3 = baseY + ((i + 3) * spacing); \
            i += 4; \
        } while (i != limit); \
    } while (0);

    arg0->timer = 0;
    arg0->itemCount = 1;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCharacterSelectUnlockedCourseList);
}
