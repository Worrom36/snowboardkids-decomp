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

extern SchedulerThread gAudioThread;
extern u8 gCurrentQueuedSoundType;
extern s32 gSoundQueueWriteIndex;
extern s32 gSoundQueueReadIndex;

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
