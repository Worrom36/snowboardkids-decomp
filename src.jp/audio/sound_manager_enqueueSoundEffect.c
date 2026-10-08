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

typedef struct SoundQueueEntry {
    u8 type;
    u8 soundId;
    u8 volume;
    u8 pan;
    u8 priority;
    u8 unk5;
} SoundQueueEntry;

extern SoundQueueEntry gSoundQueue[];

extern s32 reserveSoundEffectQueueWriteIndex(void);

s32 enqueueSoundEffect(s16 arg0, s16 arg1) {
    s32 temp_v0 = reserveSoundEffectQueueWriteIndex();
    SoundQueueEntry *temp_v1;

    if (temp_v0 == -1) {
        return 1;
    }
    temp_v1 = &gSoundQueue[temp_v0];
    temp_v1->type = 1;
    temp_v1->soundId = arg0;
    temp_v1->volume = 0xFF;
    temp_v1->pan = 0x80;
    temp_v1->unk5 = 0;
    temp_v1->priority = arg1;
    return 0;
}

s32 enqueueSoundEffectWithVolume(s16 arg0, s16 arg1, s16 arg2) {
    s32 temp_v0;
    SoundQueueEntry *temp_v1;

    if (arg1 <= 0) {
        return 0;
    }
    temp_v0 = reserveSoundEffectQueueWriteIndex();
    if (temp_v0 == -1) {
        return 1;
    }
    if (arg1 >= 0x100) {
        arg1 = 0xFF;
    }
    temp_v1 = &gSoundQueue[temp_v0];
    temp_v1->type = 1;
    temp_v1->soundId = (u8)arg0;
    temp_v1->volume = (u8)arg1;
    temp_v1->pan = 0x80;
    temp_v1->unk5 = 0;
    temp_v1->priority = (u8)arg2;
    return 0;
}
