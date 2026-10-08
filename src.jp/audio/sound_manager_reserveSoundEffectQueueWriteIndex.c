#include "game/race/camera/race_camera.h"
#include "game/race/race_state.h"
#include "common.h"
#include "assets.h"
#include "game/engine/asset_manager.h"
#include "game/math/fixed_point_math.h"
#include "game/engine/relocatable_heap.h"
#include "game/audio/sound_manager.h"
#include "game/engine/system_runtime.h"
#include "game/engine/runtime_memory.h"
#include "libmus/libmus.h"
#include "game/race/player/race_player_input.h"

#define SOUND_MANAGER_QUEUE_MASK (SOUND_MANAGER_QUEUE_CAPACITY - 1)
#define SOUND_MANAGER_QUEUE_CAPACITY 64

extern s32 gSoundQueueWriteIndex;
extern s32 gSoundQueueReadIndex;

s32 reserveSoundEffectQueueWriteIndex(void) {
    u32 ret;
    s32 temp_v1;

    temp_v1 = gSoundQueueWriteIndex;
    ret = -1;
    if (gSoundQueueReadIndex == ((temp_v1 + 1) & SOUND_MANAGER_QUEUE_MASK)) {
        return ret;
    }
    (&gSoundQueueWriteIndex)[(gSoundQueueReadIndex == ((temp_v1 + 1) & SOUND_MANAGER_QUEUE_MASK)) * 0] =
        (temp_v1 + 1) & SOUND_MANAGER_QUEUE_MASK;
    return temp_v1;
}

s32 reserveSoundEffectQueueReadIndex(void) {
    u32 ret;
    s32 temp_v1;

    temp_v1 = gSoundQueueReadIndex;
    ret = -1;
    if (temp_v1 == gSoundQueueWriteIndex) {
        return ret;
    }
    (&gSoundQueueReadIndex)[(temp_v1 == gSoundQueueWriteIndex) * 0] = (temp_v1 + 1) & SOUND_MANAGER_QUEUE_MASK;
    return temp_v1;
}
