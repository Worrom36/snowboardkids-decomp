#include <PR/gu.h>
#include "game/audio/audio_engine_internal.h"

u8 *Fstop(PlayerCommandState *arg0, u8 *arg1) {
    arg0->unk60 = 0;
    arg0->unk68 = 0;
    arg0->song_addr = 0;
    arg0->soundId = 0;
    arg0->id = 0;
    return 0;
}

void *Fwave(PlayerCommandState *arg0, u8 *arg1) {
    u16 v;
    u8 *new_var;

    v = *arg1;
    arg1++;
    if (v & 0x80) {
        v = ((v & 0x7F) << 1) << 7;
        v |= *(new_var = arg1);
        arg1++;
    }
    arg0->wave = v;
    return arg1;
}

s32 Fport(PlayerCommandState *arg0, u8 *arg1) {
    arg0->portamentoTime = *arg1;
    return (s32)(arg1 + 1);
}

s32 Fportoff(PlayerCommandState *arg0, s32 arg1) {
    arg0->portamentoTime = 0;
    return arg1;
}

s32 Fdefa(PlayerCommandState *arg0, u8 *arg1) {
    u32 value;

    value = *arg1++;
    if (value == 0) {
        value = 1;
    }
    arg0->unkF4 = value;
    arg0->env_init_vol = arg1[0];
    value = arg1[1];
    arg1 += 2;
    if (value == 0) {
        value = 1;
    }
    arg0->unkFB = value;
    arg0->unk30 = (f32)(1.0 / (f64)(f32)value);

    arg1 += 2;
    arg0->unkF6 = arg1[-2];
    value = arg1[-1];
    if (value == 0) {
        value = 1;
    }
    arg0->unkFC = value;
    arg0->unk34 = (f32)(1.0 / (f64)(f32)value);

    arg1 += 2;
    arg0->unkF7 = arg1[-2];
    value = arg1[-1];
    if (value == 0) {
        value = 1;
    }
    arg0->unkFD = value;
    arg0->unk38 = (f32)(1.0 / (f64)(f32)value);

    return (s32)arg1;
}
