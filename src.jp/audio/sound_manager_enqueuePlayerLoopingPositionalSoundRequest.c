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

void enqueuePlayerLoopingPositionalSoundRequest(s32 arg0, Vec3i *arg1, s32 arg2, s32 arg3, f32 arg4, s16 mode) {
    s32 temp_a0 = arg0 << 16;
    s32 temp_a2 = arg2 << 16;
    s32 temp_a3 = arg3 << 16;

    enqueuePositionalSoundRequest(temp_a0 >> 16, arg1, temp_a2 >> 16, temp_a3 >> 16, arg4, mode, 0);
}

void enqueuePositionalSoundEffect(s16 arg0, Vec3i *arg1, s16 arg2, s16 arg3) {
    enqueuePositionalSoundRequest(arg0, arg1, arg2, arg3, 0.0f, -1, 0);
}

void enqueuePlayerPositionalSoundEffect(s16 arg0, Vec3i *arg1, s16 arg2, s16 arg3, s16 arg4, s16 minVolume) {
    enqueuePositionalSoundRequest(arg0, arg1, arg2, arg3, 0.0f, arg4 + 4, minVolume);
}
