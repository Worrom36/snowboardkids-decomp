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

extern s32 gSharedLoopingPositionalSoundHandle;
extern s32 calculatePositionalSoundVolume(Vec3i *pos, s32 volume);

void updateSingleLoopingPositionalSound(s16 soundId, Vec3i *pos, s16 volume) {
    s32 adjustedVolume;

    adjustedVolume = calculatePositionalSoundVolume(pos, volume);
    if (adjustedVolume == 0) {
        if (gSharedLoopingPositionalSoundHandle != 0) {
            MusHandleStop(gSharedLoopingPositionalSoundHandle, 0);
            gSharedLoopingPositionalSoundHandle = 0;
        }
    } else if (gSharedLoopingPositionalSoundHandle == 0) {
        gSharedLoopingPositionalSoundHandle = MusStartEffect2(soundId, adjustedVolume, 0x80, 0, 0x32);
    } else {
        MusHandleSetVolume(gSharedLoopingPositionalSoundHandle, adjustedVolume);
    }
}
