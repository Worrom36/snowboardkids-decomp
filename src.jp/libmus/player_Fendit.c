#include <PR/gu.h>
#include "game/audio/audio_engine_internal.h"

s32 Fendit(PlayerCommandState *arg0, u8 *arg1) {
    u8 *ret;
    u8 *temp;

    arg0->unkC4 = *arg1;
    ;
    ret = (temp = arg1) + 1;
    arg0->unkC2 = 0;
    arg1++;
    arg1--;
    return (s32)ret;
}

s32 Fcutoff(PlayerCommandState *arg0, u8 *arg1) {
    u8 *ret;
    u8 *temp;
    u32 v;
    short new_var;

    new_var = arg1[0] << 8;
    new_var = arg1[1] | ((0, new_var));
    v = new_var;
    ret = (temp = arg1) + 2;
    arg0->unkC2 = v;
    arg0->unkC4 = 0;
    arg1 += 2;
    arg1 -= 2;
    return (s32)ret;
}

s32 Fvibup(PlayerCommandState *arg0, u8 *arg1) {
    u32 temp_t8;
    f32 var_ft1;

    arg0->unk105 = arg1[0];
    arg0->unkE9 = arg1[1];
    arg1 += 2;
    temp_t8 = arg1[0];
    var_ft1 = (f32)(temp_t8 & 0xFFFF);
    arg0->vibratoDepth = (f32)(((double)var_ft1) / 50.0);
    return (s32)(arg1 + 1);
}

s32 Fvibdown(PlayerCommandState *arg0, u8 *arg1) {
    u32 temp_t8;
    f32 var_ft1;

    arg0->unk105 = arg1[0];
    arg0->unkE9 = arg1[1];
    arg1 += 2;
    temp_t8 = arg1[0];
    var_ft1 = (f32)(temp_t8 & 0xFFFF);
    arg0->vibratoDepth = (f32)(((double)-var_ft1) / 50.0);
    return (s32)(arg1 + 1);
}

s32 Fviboff(PlayerCommandState *arg0, s32 arg1) {
    arg0->unkE9 = 0;
    arg0->vibratoPitchOffset = 0.0f;
    return arg1;
}

s32 Flength(PlayerCommandState *arg0, u8 *arg1) {
    u8 b;
    unsigned int new_var;
    s16 value;

    b = arg1[0];
    arg1++;
    if (b < 0x80) {
        arg0->fixed_length = b;
    } else {
        value = ((b & 0x7F) << 6) << 2;
        arg0->fixed_length = value;
        new_var = arg1[0];
        b = new_var;
        arg0->fixed_length = value + b;
        arg1++;
    }
    return (s32)arg1;
}

s32 Fignore(PlayerCommandState *arg0, s32 arg1) {
    arg0->ignore = 1;
    return arg1;
}

s32 Ftrans(PlayerCommandState *arg0, u8 *arg1) {
    arg0->pitchOffset = *arg1;
    return (s32)(arg1 + 1);
}

s32 Fignore_trans(PlayerCommandState *arg0, s32 arg1) {
    arg0->skipPitchOffsetOnce = 1;
    return arg1;
}

s32 Fdistort(PlayerCommandState *arg0, u8 *arg1) {
    s32 b;
    f32 f;
    b = *arg1;
    arg1++;
    if (b & 0x80) {
        b |= 0xFFFFFF00;
    }
    f = (f32)b;
    arg0->finePitchOffset = f / 100.0;
    return (s32)arg1;
}
