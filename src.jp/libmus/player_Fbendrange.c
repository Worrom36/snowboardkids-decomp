#include <PR/gu.h>
#include "game/audio/audio_engine_internal.h"

s32 Fbendrange(PlayerCommandState *arg0, u8 *arg1) {
    arg0->pitchBendDepth = *arg1;
    return (s32)(arg1 + 1);
}
