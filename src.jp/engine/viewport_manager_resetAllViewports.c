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
