#include "game/engine/runtime_memory.h"
#include "common.h"
#include "game/race/camera/race_camera.h"
#include "game/audio/sound_manager.h"
#include "game/engine/system_runtime.h"
#include "game/engine/system_boot.h"
#include "game/engine/frame_render_task.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/render_callback.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/relocatable_heap.h"
#include "game/race/flow/race_flow.h"
#include "game/race/race_state.h"
#include "game/engine/viewport_manager.h"
#include "game/math/fixed_point_math.h"
#include "PR/os_cache.h"
#include "PR/os_thread.h"
#include "PR/sptask.h"
#include "PR/ucode.h"
#include "game/audio/audio_engine.h"

#define BOOT_FADE_TIMER 0x3E4
#define TITLE_FADE_TIMER 0x3B6

extern u8 gFadeColorBlue;
extern s16 gFadeTimer;
extern u8 gFadeColorRed;
extern u8 gFadeColorGreen;

void initFramebufferRenderTaskState(void) {
    gFrameRenderTasks[0].completionMessage = 5;
    gFrameRenderTasks[0].framebuffer = gFramebuffers[0];
    gFrameRenderTasks[1].completionMessage = 6;
    if (1) {
        gFrameRenderTasks[1].framebuffer = gFramebuffers[1];
    }
    osViSetSpecialFeatures(0x6A);
    gFrameRenderTaskStatuses[0][0] = 0;
    gFrameRenderTaskStatuses[1][0] = 0;
    gFramebufferColorBufferIndex = 0;
    gFrameRenderTasks[0].renderData.viewport.rotations[0] = gIdentityMatrix;
    gFrameRenderTasks[0].renderData.viewport.rotations[1] = gIdentityMatrix;
    gFrameRenderTasks[0].renderData.viewport.rotations[2] = gIdentityMatrix;
    gFrameRenderTasks[0].renderData.viewport.rotations[3] = gIdentityMatrix;
    gFrameRenderTasks[0].renderData.viewport.translations[0] = gIdentityMatrix;
    gFrameRenderTasks[0].renderData.viewport.translations[1] = gIdentityMatrix;
    gFrameRenderTasks[0].renderData.viewport.translations[2] = gIdentityMatrix;
    gFrameRenderTasks[0].renderData.viewport.translations[3] = gIdentityMatrix;
    gFrameRenderTasks[1].renderData.viewport.rotations[0] = gIdentityMatrix;
    gFrameRenderTasks[1].renderData.viewport.rotations[1] = gIdentityMatrix;
    gFrameRenderTasks[1].renderData.viewport.rotations[2] = gIdentityMatrix;
    gFrameRenderTasks[1].renderData.viewport.rotations[3] = gIdentityMatrix;
    gFrameRenderTasks[1].renderData.viewport.translations[0] = gIdentityMatrix;
    gFrameRenderTasks[1].renderData.viewport.translations[1] = gIdentityMatrix;
    gFrameRenderTasks[1].renderData.viewport.translations[2] = gIdentityMatrix;
    gFrameRenderTasks[1].renderData.viewport.translations[3] = gIdentityMatrix;
    gMenuFadeOverlayActive = 0;
}

void setBootFadeColor(u8 a0, u8 a1, u8 a2) {
    gFadeColorRed = a0;
    gFadeColorGreen = a1;
    gFadeColorBlue = a2;
    gFadeTimer = BOOT_FADE_TIMER;
}

void setTitleFadeColor(u8 a0, u8 a1, u8 a2) {
    gFadeColorRed = a0;
    gFadeColorGreen = a1;
    gFadeColorBlue = a2;
    gFadeTimer = TITLE_FADE_TIMER;
}

void appendFadeOverlayDisplayList(void) {
    gSPFogPosition(gRegionAllocPtr++, gFadeTimer, 1000);
    gDPSetFogColor(gRegionAllocPtr++, gFadeColorRed, gFadeColorGreen, gFadeColorBlue, 255);
}
