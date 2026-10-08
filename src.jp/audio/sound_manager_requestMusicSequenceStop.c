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

extern s32 gPendingMusicCommandArg;
extern s32 gPendingMusicCommand;
extern s32 gCurrentMusicSequenceBank;

void requestMusicSequenceStop(s32 arg0) {
    gPendingMusicCommand = 2;
    gPendingMusicCommandArg = arg0;
    gCurrentMusicSequenceBank = -1;
}
