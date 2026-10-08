#include <PR/gu.h>
#include "game/audio/audio_engine_internal.h"

s32 Fenvoff(PlayerCommandState *arg0, s32 arg1) {
    arg0->env_trigger_off = 1;
    return arg1;
}

s32 Fenvon(PlayerCommandState *arg0, s32 arg1) {
    arg0->env_trigger_off = 0;
    return arg1;
}

s32 Ftroff(PlayerCommandState *arg0, s32 arg1) {
    arg0->trigger_off = 1;
    return arg1;
}

s32 Ftron(PlayerCommandState *arg0, s32 arg1) {
    arg0->trigger_off = 0;
    return arg1;
}

s32 Ffor(PlayerCommandState *arg0, u8 *arg1) {
    u8 depth;

    depth = arg0->returnDepth;
    arg0->returnUnk109[depth] = *arg1++;
    arg0->returnScripts[depth] = arg1;
    arg0->returnUnk60[depth] = arg0->unk60;
    arg0->returnUnk68[depth] = arg0->unk68;
    arg0->returnUnk10E[depth] = arg0->unkEF;
    arg0->returnUnk113[depth] = arg0->pitchBendValue;
    arg0->returnUnkC8[depth] = arg0->unkC8;
    arg0->returnUnkCA[depth] = arg0->unkCA;
    arg0->returnDepth++;
    return (s32)arg1;
}

s32 Fnext(PlayerCommandState *arg0, s32 arg1) {
    s32 depth;
    u8 *countPtr;
    u8 count;
    u8 nextCount;

    depth = arg0->returnDepth - 1;
    countPtr = &arg0->returnUnk109[depth];
    count = *countPtr;
    nextCount = count - 1;
    if (count != 0xFF) {
        *countPtr = nextCount;
        if ((nextCount & 0xFF) == 0) {
            arg0->returnDepth = depth;
            depth = -1;
        }
    }
    if (depth >= 0) {
        arg1 = (s32)arg0->returnScripts[depth];
        arg0->unk60 = arg0->returnUnk60[depth];
        arg0->unk68 = arg0->returnUnk68[depth];
        arg0->unkEF = arg0->returnUnk10E[depth];
        arg0->pitchBendValue = arg0->returnUnk113[depth];
        arg0->unkC8 = arg0->returnUnkC8[depth];
        arg0->unkCA = arg0->returnUnkCA[depth];
    }
    return arg1;
}

s32 Fwobble(PlayerCommandState *arg0, u8 *arg1) {
    arg0->unk119 = arg1[0];
    arg0->unkEB = arg1[1];
    arg0->wobble_off_speed = arg1[2];
    return (s32)(arg1 + 3);
}

s32 Fwobbleoff(PlayerCommandState *arg0, s32 arg1) {
    arg0->unkEB = 0;
    return arg1;
}

s32 Fvelon(PlayerCommandState *arg0, s32 arg1) {
    arg0->velocity_on = 1;
    return arg1;
}

s32 Fveloff(PlayerCommandState *arg0, s32 arg1) {
    arg0->velocity_on = 0;
    return arg1;
}

s32 Fvelocity(PlayerCommandState *arg0, u8 *arg1) {
    s32 ret;

    ret = (s32)(arg1 + 1);
    arg0->default_velocity = arg1[0];
    if (ret) {}
    arg0->velocity_on = 0;
    return ret;
}

s32 Fpan(PlayerCommandState *arg0, u8 *arg1) {
    arg0->pan = (s32)(u8)*arg1 / 2;
    return (s32)(arg1 + 1);
}

s32 Fstereo(s32 arg0, s32 arg1) {
    return arg1 + 2;
}

void *Fdrums(void *arg0, u8 *arg1) {
    u8 *temp_a2;
    s32 phi;

    temp_a2 = *(u8 **)((s32)arg0 + 0x54);
    phi = (s32)temp_a2 + (*(s32 **)((s32)(*(u8 **)((s32)arg0 + 0x54)) + 0x14))[*arg1];
    *(s32 *)((s32)arg0 + 0x70) = phi;
    return arg1 + 1;
}
