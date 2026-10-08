#include "game/race/effects/race_intro_effects.h"

void approachRaceIntroFlyoverSpinStep(RaceIntroEffectActor *arg0, s16 arg1) {
    s16 diff = arg1 - arg0->timer;

    if (diff >= 5) {
        diff = 4;
    }
    if (diff < -4) {
        diff = -4;
    }
    arg0->timer += diff;
}

void approachRaceIntroFlyoverVerticalVelocity(RaceIntroEffectActor *arg0, s32 arg1) {
    s32 diff = arg1 - arg0->velocityY;

    if (diff >= 0x2001) {
        diff = 0x2000;
    }
    if (diff < -0x2000) {
        diff = -0x2000;
    }
    arg0->velocityY += diff;
}

void approachRaceIntroFlyoverOrbitRadius(RaceIntroEffectActor *arg0, s32 arg1) {
    s32 diff = arg1 - arg0->radius;

    if (diff >= 0x2001) {
        diff = 0x2000;
    }
    if (diff < -0x2000) {
        diff = -0x2000;
    }
    if (diff >= 0 && arg0->radius != 0) {
        if (arg0->tilt < 0x800) {
            arg0->tilt += 0x28;
        }
    } else {
        if (arg0->tilt != 0) {
            arg0->tilt -= 0x28;
        }
    }
    arg0->radius += diff;
}

void approachRaceIntroFlyoverPitchVelocity(RaceIntroEffectActor *arg0, s16 arg1) {
    s16 diff = arg1 - arg0->pitchVelocity;

    if (diff >= 5) {
        diff = 4;
    }
    if (diff < -4) {
        diff = -4;
    }

    arg0->pitchVelocity += diff;
    arg0->angle.half.pitch += (((-arg0->pitchVelocity * 2) - arg0->angle.half.pitch) >> 3);
}
