#include <PR/gu.h>
#include "game/audio/audio_engine_internal.h"

void __MusIntInitEnvelope(PlayerCommandState *arg0) {
    u16 temp_v0;

    if (arg0->length != 0x7FFF) {
        temp_v0 = arg0->unkC2;
        if (temp_v0 != 0) {
            arg0->release_frame = arg0->note_start_frame + (temp_v0 << 8);
        } else {
            arg0->release_frame = arg0->note_end_frame - (arg0->unkC4 << 8);
        }
    } else {
        arg0->release_frame = 0x7FFFFFFF;
    }

    arg0->env_current = arg0->env_init_vol;
    arg0->env_count = arg0->unkF4;
    arg0->env_phase = 1;
}

void __MusIntProcessEnvelope(PlayerCommandState *arg0) {
    u32 env_phase_count;
    u8 phase;
    f32 initial_level;
    u8 rate;
    f32 envelope_value;
    u8 start_level;
    f32 release_amount;

    if (!arg0->env_phase) {
        return;
    }

    if ((arg0->channel_frame >= arg0->release_frame) && (arg0->env_phase < 4)) {
        arg0->env_phase = 4;
        arg0->env_count = 1;
        arg0->release_start_vol = arg0->env_current;
    }

    arg0->env_count--;
    if (!arg0->env_count) {
        rate = arg0->unkF4;
        phase = arg0->env_phase;
        arg0->env_count = rate;
        switch (arg0->env_phase) {
            case 1:
                env_phase_count = (u32)(arg0->channel_frame - arg0->note_start_frame) >> 8;
                env_phase_count = env_phase_count / rate;
                if (env_phase_count < arg0->unkFB) {
                    start_level = arg0->env_init_vol;
                    envelope_value = arg0->unkF6 - start_level;
                    envelope_value *= arg0->unk30;
                    envelope_value *= (f32)env_phase_count;
                    initial_level = (f32)start_level;
                    arg0->env_current = (s32)(initial_level + envelope_value);
                    return;
                } else {
                    arg0->env_phase = arg0->env_phase + 1;
                    arg0->env_current = arg0->unkF6;
                    return;
                }
            case 2:
                env_phase_count = ((u32)(arg0->channel_frame - arg0->note_start_frame) >> 8) - arg0->unkFB;
                env_phase_count = env_phase_count / rate;
                if (env_phase_count < arg0->unkFC) {
                    envelope_value = arg0->unkF7 - arg0->unkF6;
                    envelope_value *= arg0->unk34;
                    envelope_value *= (f32)env_phase_count;
                    arg0->env_current = (s32)((f32)arg0->unkF6 + envelope_value);
                    return;
                } else {
                    arg0->env_phase = arg0->env_phase + 1;
                    arg0->env_current = arg0->unkF7;
                    return;
                }
            case 3:
                return;
            case 4:
                env_phase_count = (u32)(arg0->channel_frame - arg0->release_frame) >> 8;
                env_phase_count = env_phase_count / rate;
                if (env_phase_count < arg0->unkFD) {
                    release_amount = arg0->release_start_vol;
                    release_amount *= arg0->unk38;
                    release_amount *= (f32)env_phase_count;
                    arg0->env_current = (s32)((f32)arg0->release_start_vol - release_amount);
                    return;
                } else {
                    arg0->env_phase = arg0->env_phase + 1;
                    arg0->env_current = 0;
                    return;
                }
        }
    }
}

void __MusIntProcessWobble(PlayerCommandState *arg0) {
    u8 temp_t7;
    u8 temp_v0;

    if (1) {
        temp_t7 = (arg0->wobble_count & 0xFFFFU) - 1;
    }
    arg0->wobble_count = temp_t7;
    if ((temp_t7 & 0xFF) == 0) {
        temp_v0 = arg0->unkEB;
        if (temp_v0 != 0) {
            if (arg0->wobble_current == 0) {
                arg0->wobble_count = temp_v0;
                arg0->wobble_current = arg0->unk119;
                return;
            }
            arg0->wobble_current = 0;
            arg0->wobble_count = arg0->wobble_off_speed;
        }
    }
}
