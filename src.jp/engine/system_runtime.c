#include "common.h"
#include "game/engine/runtime_memory.h"
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

#define BOOT_THREAD_ID 1
#define THREAD_PRIORITY 10
#define DMA_CHUNK_SIZE 0x2000
#define OS_MESG_BLOCK 1
#define OS_READ 0
#define BOOT_FADE_TIMER 0x3E4
#define TITLE_FADE_TIMER 0x3B6

typedef struct {
    RenderCallbackQueueSlot queue0;
    RenderCallbackQueueSlot queue1;
    RenderCallbackQueueSlot queue2;
    RenderCallbackQueueSlot queue3;
} RenderCallbackQueueGroup;

extern u8 gBootThreadStack[BOOT_THREAD_STACK_SIZE];

extern OSThread gBootThread;

extern OSMesgQueue gRomDmaQueue;
extern u8 gFadeColorBlue;
extern s16 gFadeTimer;
extern u8 gFadeColorRed;
extern u8 gFadeColorGreen;

void main(void *arg) {
    osInitialize();
    osCreateThread(
        &gBootThread,
        BOOT_THREAD_ID,
        bootThreadMain,
        arg,
        gBootThreadStack + sizeof(gBootThreadStack),
        THREAD_PRIORITY
    );
    osStartThread(&gBootThread);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/engine/system_runtime/bootThreadMain.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/engine/system_runtime/func_800990D0.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/engine/system_runtime/func_800991F8.s")

void dmaReadRom(u32 romOffset, void *ramAddress, s32 size) {
    OSIoMesg dmaRequest;
    OSMesg dmaDoneMsg;
    s32 transferSize;

    while (size != 0) {
        if (size > DMA_CHUNK_SIZE) {
            transferSize = DMA_CHUNK_SIZE;
        } else {
            transferSize = size;
        }
        osInvalDCache(ramAddress, transferSize);
        osPiStartDma(&dmaRequest, 0, OS_READ, romOffset, ramAddress, transferSize, &gRomDmaQueue);
        osRecvMesg(&gRomDmaQueue, &dmaDoneMsg, OS_MESG_BLOCK);
        size -= transferSize;
        romOffset += transferSize;
        ramAddress = (void *)((u8 *)ramAddress + transferSize);
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/engine/system_runtime/func_80099610.s")

void resetRenderCallbackQueues(void) {
    u32 end;
    RenderCallbackQueueGroup *group;

    gMenuForegroundRenderCallbackList = NULL;
    gRaceForegroundRenderCallbackList = NULL;
    // clang-format off
    do { \
        end = (u32)&gBackdropRenderCallbackList; \
        group = (RenderCallbackQueueGroup *)gModelRenderCallbackQueues; \
    loop: \
        group++; \
        group[-1].queue0.head = NULL; \
        group[-1].queue1.head = NULL; \
    } while (0);
    // clang-format on
    group[-1].queue2.head = NULL;
    group[-1].queue3.head = NULL;
    if ((u32)group != end) {
        goto loop;
    }
    gBackdropRenderCallbackList = NULL;
    gMenuOverlayRenderCallbackList = NULL;
    D_80124848 = NULL;
    gRaceOverlayRenderCallbackList = NULL;
    gMenuRenderCallbackList = NULL;
}

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/engine/system_runtime/submitFramebufferRenderTask.s")
