#include "common.h"
#include <PR/os_libc.h>
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/controller_input.h"
#include "game/ending/ending_credits_ui.h"
#include "game/ending/ending_credits_flow.h"
#include "font_encoding.h"
#include <PR/rmon.h>
#include "game/menu/renderer/menu_renderer.h"

#define ENDING_CREDITS_TEXT_FADE_MAX 0x100
#define ENDING_CREDITS_PAGE_FADE_STEP 0xA
#define ENDING_CREDITS_PAGE_RESET_DELAY 0x20
#define ENDING_CREDITS_PAGE_VISIBLE_FRAMES 0x96
#define ENDING_CREDITS_PAGE_COUNT 0x19

extern void drawEndingCreditsPageText(EndingCreditsPageTextActor *arg0);
extern void updateEndingCreditsTheEndTextFadeIn(EndingCreditsPageTextActor *arg0);
extern void updateEndingCreditsPageText(EndingCreditsPageTextActor *arg0);

void updateEndingCreditsPageText(EndingCreditsPageTextActor *arg0) {
    switch (arg0->state) {
        case 0:
            arg0->alpha += ENDING_CREDITS_PAGE_FADE_STEP;
            if (!(arg0->alpha < ENDING_CREDITS_TEXT_FADE_MAX)) {
                arg0->alpha = ENDING_CREDITS_TEXT_FADE_MAX;
                arg0->state = 1;
                arg0->timer = 0;
            }
            break;
        case 1:
            arg0->timer = arg0->timer + 1;
            if (arg0->timer == ENDING_CREDITS_PAGE_VISIBLE_FRAMES) {
                arg0->timer = 0;
                arg0->state = 2;
            }
            break;
        case 2:
            arg0->alpha -= ENDING_CREDITS_PAGE_FADE_STEP;
            if (!(arg0->alpha > 0)) {
                arg0->alpha = 0;
                arg0->state = 3;
                arg0->pageIndex = arg0->pageIndex + 1;
                if (arg0->pageIndex == ENDING_CREDITS_PAGE_COUNT) {
                    arg0->pageIndex = 0;
                    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateEndingCreditsTheEndTextFadeIn);
                }
                if (gEndingCreditsSequencePhase == 0) {
                    gEndingCreditsSequencePhase = 1;
                }
            }
            break;
        case 3:
            arg0->timer = arg0->timer + 1;
            if (!(arg0->timer < ENDING_CREDITS_PAGE_RESET_DELAY)) {
                arg0->timer = 0;
                arg0->state = 0;
            }
            break;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawEndingCreditsPageText, arg0);
}

void initEndingCreditsPageTextActor(EndingCreditsPageTextActor *arg0) {
    arg0->state = 3;
    arg0->pageIndex = 0;
    arg0->x = -0x40;
    arg0->y = 0x10;
    arg0->alpha = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateEndingCreditsPageText);
}
