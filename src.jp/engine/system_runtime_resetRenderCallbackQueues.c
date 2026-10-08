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

typedef struct {
    RenderCallbackQueueSlot queue0;
    RenderCallbackQueueSlot queue1;
    RenderCallbackQueueSlot queue2;
    RenderCallbackQueueSlot queue3;
} RenderCallbackQueueGroup;

void resetRenderCallbackQueues(void) {
    u32 end;
    RenderCallbackQueueGroup *group;

    gMenuForegroundRenderCallbackList = NULL;
    gRaceForegroundRenderCallbackList = NULL;
    do { \
        end = (u32)&gBackdropRenderCallbackList; \
        group = (RenderCallbackQueueGroup *)gModelRenderCallbackQueues; \
    loop: \
        group++; \
        group[-1].queue0.head = NULL; \
        group[-1].queue1.head = NULL; \
    } while (0);
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
