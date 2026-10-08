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

typedef struct SoundRomRange {
    u32 romStart;
    u32 romEnd;
} SoundRomRange;

extern s32 gCurrentMusicSequenceHandle;

extern SoundRomRange gRaceMusicSequenceRomRanges[9];

extern s32 gCurrentMusicSequenceBank;

extern SoundRomRange gMusicSequenceRomRanges[10];

extern void stopCurrentMusicSequence(s32 arg0);

extern s32 gMusicSequenceStopped;

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
