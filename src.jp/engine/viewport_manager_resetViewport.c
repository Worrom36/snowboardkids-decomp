#include "game/engine/viewport_manager.h"
#include "PR/gu.h"

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
