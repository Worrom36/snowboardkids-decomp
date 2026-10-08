#include <PR/gu.h>
#include "game/audio/audio_engine_internal.h"

void __MusIntProcessContinuousVolume(PlayerCommandState *arg0) {
    u16 temp_c8;
    s32 one = 1;
    int new_var;
    register u8 temp_v0;
    register u8 temp_t2;
    register u8 temp_t4;
    short temp_s16;
    register u16 temp_t6;
    register int temp_t9;
    register u16 temp_t0;

    if ((u32)arg0->unk4 < (u32)arg0->channel_frame) {
        do {
            temp_t9 = (short)(temp_c8 = arg0->unkC8) - one;
            arg0->unk4 = arg0->unk4 + 0x100;
            arg0->unkC8 = temp_t9;
            temp_t0 = (temp_t9 ^ (temp_v0 = 0)) & 0xFFFF;
            if (!temp_t0) {
                temp_v0 = *arg0->unk60++;
                if ((s32)temp_v0 >= 0x80) {
                    temp_t2 = temp_v0 & 0x7F;
                    arg0->unkEF = temp_t2;
                    temp_v0 = *arg0->unk60++;
                    temp_t0 = one;
                    if ((s32)temp_v0 >= 0x80) {
                        temp_t4 = temp_v0 & 0x7F;
                        temp_s16 = temp_t4 << 8;
                        temp_t6 = temp_s16;
                        arg0->unkC8 = (short)temp_t6;
                        arg0->unkC8 = temp_t6 + *arg0->unk60++ + (new_var = 2);
                    } else {
                        arg0->unkC8 = temp_v0 + 2;
                    }
                } else {
                    arg0->unkEF = temp_v0;
                    arg0->unkC8 = one;
                }
            }
        } while ((u32)arg0->unk4 < (u32)arg0->channel_frame);
    }
}

void __MusIntProcessContinuousPitchBend(PlayerCommandState *arg0) {
    u16 temp_ca;
    s32 one = 1;
    int new_var;
    register u8 temp_v0;
    register u8 temp_t2;
    register u8 temp_t4;
    short temp_s16;
    register u16 temp_t6;
    register int temp_t9;
    register u16 temp_t0;

    if ((u32)arg0->unk8 < (u32)arg0->channel_frame) {
        do {
            temp_t9 = (short)(temp_ca = arg0->unkCA) - one;
            arg0->unk8 = arg0->unk8 + 0x100;
            arg0->unkCA = temp_t9;
            temp_t0 = (temp_t9 ^ (temp_v0 = 0)) & 0xFFFF;
            if (!temp_t0) {
                temp_v0 = *arg0->unk68++;
                if ((s32)temp_v0 >= 0x80) {
                    temp_t2 = temp_v0 & 0x7F;
                    arg0->pitchBendValue = temp_t2;
                    temp_v0 = *arg0->unk68++;
                    temp_t0 = one;
                    if ((s32)temp_v0 >= 0x80) {
                        temp_t4 = temp_v0 & 0x7F;
                        temp_s16 = temp_t4 << 8;
                        temp_t6 = temp_s16;
                        arg0->unkCA = (short)temp_t6;
                        arg0->unkCA = temp_t6 + *arg0->unk68++ + (new_var = 2);
                    } else {
                        arg0->unkCA = temp_v0 + 2;
                    }
                } else {
                    arg0->pitchBendValue = temp_v0;
                    arg0->unkCA = one;
                }
            }
        } while ((u32)arg0->unk8 < (u32)arg0->channel_frame);
    }
}
