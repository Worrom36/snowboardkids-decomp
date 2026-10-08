#include <PR/gu.h>
#include "game/audio/audio_engine_internal.h"

u8 *Fgoto(PlayerCommandState *arg0, u8 *arg1) {
    s32 offset;
    s32 sequenceOffset;

    sequenceOffset = *arg1++ << 8;
    sequenceOffset += *arg1++;

    offset = *arg1++ << 8;
    offset += *arg1++;
    arg0->unk60 = arg0->unk64 + offset;
    arg0->unkC8 = 1;

    offset = *arg1++ << 8;
    offset += *arg1++;
    arg0->unk68 = arg0->unk6C + offset;
    arg0->unkCA = 1;

    return arg0->restartPos + sequenceOffset;
}

s32 Freverb(PlayerCommandState *arg0, u8 *arg1) {
    arg0->reverb = *arg1;
    return (s32)(arg1 + 1);
}
