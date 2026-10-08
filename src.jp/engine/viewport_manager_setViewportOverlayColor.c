#include "game/engine/viewport_manager.h"
#include "PR/gu.h"

void setViewportOverlayColor(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    ViewportState *viewport = &gViewportStates[arg0];

    viewport->overlayActive = 1;
    viewport->overlayR = arg1;
    viewport->overlayG = arg2;
    viewport->overlayB = arg3;
}
