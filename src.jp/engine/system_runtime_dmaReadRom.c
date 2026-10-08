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

#define DMA_CHUNK_SIZE 0x2000
#define OS_MESG_BLOCK 1
#define OS_READ 0

extern OSMesgQueue gRomDmaQueue;

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
