#include "common.h"
#include "game/race/camera/race_camera.h"
#include "game/race/race_state.h"
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

typedef struct SoundRomRange {
    u32 romStart;
    u32 romEnd;
} SoundRomRange;

typedef struct SoundQueueEntry {
    u8 type;
    u8 soundId;
    u8 volume;
    u8 pan;
    u8 priority;
    u8 unk5;
} SoundQueueEntry;

extern s32 gMusicSequenceStopped;
extern s32 gCurrentMusicSequenceHandle;
extern SoundRomRange gRaceMusicSequenceRomRanges[9];
extern s32 gCurrentMusicSequenceBank;
extern SoundRomRange gMusicSequenceRomRanges[10];
extern s32 gSoundQueueWriteIndex;
extern s32 gSoundQueueReadIndex;
extern s32 gPendingMusicCommandArg;
extern s32 gPendingMusicCommand;
extern SoundQueueEntry gSoundQueue[];
extern SchedulerThread gAudioThread;
extern u8 gCurrentQueuedSoundType;
extern s32 gSharedLoopingPositionalSoundHandle;

extern void stopCurrentMusicSequence(s32 arg0);
extern s32 reserveSoundEffectQueueWriteIndex(void);
extern s32 calculatePositionalSoundVolume(Vec3i *pos, s32 volume);

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/func_80071380.s")

void stopCurrentMusicSequence(s32 arg0) {
    if (gMusicSequenceStopped == 0) {
        if (gCurrentMusicSequenceHandle != 0) {
            MusHandleStop(gCurrentMusicSequenceHandle, arg0);
        }
        gMusicSequenceStopped = 1;
    }
}

s32 loadMusicSequenceBank(s32 arg0) {
    s32 size;
    SoundRomRange *range;

    if (gCurrentMusicSequenceHandle == 0) {
        range = (SoundRomRange *)((arg0 * 2) + (s32 *)gMusicSequenceRomRanges);
        size = range->romEnd - range->romStart;
        dmaReadRom(range->romStart, getRelocatableHeapBlockBase(gAssetHandles[5]), size);
        if ((gCurrentMusicSequenceHandle =
                 MusStartSong((PlayerCommandData *)getRelocatableHeapBlockBase(gAssetHandles[5]))) != 0) {
            gCurrentMusicSequenceBank = arg0;
            if (range == gRaceMusicSequenceRomRanges) {
                MusSetMasterVolume(2, 0x7FFF);
            } else {
                MusSetMasterVolume(2, 0x60FF);
            }
            gMusicSequenceStopped = 0;
            return 0;
        }
    } else {
        stopCurrentMusicSequence(4);
    }
    return 1;
}

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/func_80071738.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/func_800717D4.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/func_80071810.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/func_800719D0.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/requestMusicSequenceBank.s")

void requestMusicSequenceStop(s32 arg0) {
    gPendingMusicCommand = 2;
    gPendingMusicCommandArg = arg0;
    gCurrentMusicSequenceBank = -1;
}

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

void stopSoundEffects(void) {
    osStopThread(&gAudioThread);
    gCurrentQueuedSoundType = 0;
    gSoundQueueWriteIndex = 0;
    gSoundQueueReadIndex = 0;
    MusStop(1, 0);
    osStartThread(&gAudioThread);
}

void fadeOutAllMusicSequences(void) {
    osStopThread(&gAudioThread);
    MusStop(3, 0x14);
    osStartThread(&gAudioThread);
}

s32 calculatePositionalSoundVolume(Vec3i *pos, s32 volume) {
    RaceCamera *camera;
    s32 distance;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 attenuation;
    s32 adjustedVolume;

    distance = 0x4000000;
    if (volume <= 0) {
        return 0;
    }

    if (volume >= 0x80) {
        volume = 0x7F;
    }

    camera = gRaceCameras;
    do {
        if (camera->initialized.signedValue == 1) {
            dx = camera->prevPos.x - pos->x;
            if ((dx >= -0x4000000) && (dx < 0x4000001)) {
                dy = camera->prevPos.y - pos->y;
                if (((camera->prevPos.y - pos->y) >= -0x4000000) && (dy < 0x4000001)) {
                    dz = camera->prevPos.z - pos->z;
                    if ((dz >= -0x4000000) && (dz < 0x4000001)) {
                        adjustedVolume = integerSquareRoot64((s64)dx * dx + (s64)dy * dy + (s64)dz * dz);
                        if (adjustedVolume < distance) {
                            distance = adjustedVolume;
                        }
                    }
                }
            }
        }
        camera++;
    } while (camera != gRaceCamerasEnd);

    attenuation = 0x1000 - fixedSine(distance / 0x10000);
    adjustedVolume = (volume * attenuation) / 0x1000;
    if (adjustedVolume < 0) {
        adjustedVolume = 0;
    }
    return adjustedVolume;
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/func_80072068.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/func_800722BC.s")

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/requestCourseMusicSequence.s")

void countActiveSoundPlayers(void) {
    MusAsk(3);
}

s32 countActiveMusicSequences(void) {
    return MusAsk(2);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/clearPendingPositionalSoundRequests.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/enqueuePositionalSoundRequest.s")

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/audio/sound_manager/playPendingPositionalSoundRequests.s")
