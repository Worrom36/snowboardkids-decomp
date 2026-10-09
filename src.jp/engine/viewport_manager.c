#include "common.h"
#include "game/engine/viewport_manager.h"
#include "PR/gu.h"

ViewportState gViewportStates[4];

void resetAllViewports(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        gViewportStates[i].active = 0;
        gViewportStates[i].screenBoundsValid = 0;
        gViewportStates[i].clearFramebuffer = 0;
        gViewportStates[i].overlayActive = 0;
        gViewportStates[i].overlayR = 0;
        gViewportStates[i].overlayG = 0;
        gViewportStates[i].overlayB = 0;
        gViewportStates[i].overlayAlpha = 0;
        gViewportStates[i].viewport.vp.vscale[2] = 0x1FF;
        gViewportStates[i].viewport.vp.vscale[3] = 0;
        gViewportStates[i].viewport.vp.vtrans[2] = 0x1FF;
        gViewportStates[i].viewport.vp.vtrans[3] = 0;
    }
}

void resetViewport(s32 arg0) {
    ViewportState *viewport = &gViewportStates[arg0];

    viewport->active = 0;
    viewport->screenBoundsValid = 0;
    viewport->clearFramebuffer = 0;
    viewport->overlayActive = 0;
    viewport->overlayR = 0;
    viewport->overlayG = 0;
    viewport->overlayB = 0;
    viewport->overlayAlpha = 0;
    viewport->viewport.vp.vscale[2] = 0x1FF;
    viewport->viewport.vp.vscale[3] = 0;
    viewport->viewport.vp.vtrans[2] = 0x1FF;
    viewport->viewport.vp.vtrans[3] = 0;
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/engine/viewport_manager/configureViewport.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/engine/viewport_manager/configureViewportWithFovAndFarClip.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/engine/viewport_manager/configureRaceViewport.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/engine/viewport_manager/configureMenuViewport.s")

void setViewportOverlayColor(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    ViewportState *viewport = &gViewportStates[arg0];

    viewport->overlayActive = 1;
    viewport->overlayR = arg1;
    viewport->overlayG = arg2;
    viewport->overlayB = arg3;
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/engine/viewport_manager/enableViewportClear.s")
