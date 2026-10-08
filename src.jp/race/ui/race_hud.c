#include "game/race/ui/race_hud.h"

s32 calculateRaceTimerDelta(RaceTimer *arg0, RaceTimer *arg1, RaceTimer *arg2) {
    s32 total1;
    s32 diff;

    diff = (arg0->fraction & 0xFF00) + (arg0->seconds * 0x6400) + (arg0->minutes * 0x177000);
    total1 = (arg1->fraction & 0xFF00) + (arg1->seconds * 0x6400) + (arg1->minutes * 0x177000);
    arg0 = (RaceTimer *)1;
    if (diff >= total1) {
        arg0 = (RaceTimer *)0;
        diff -= total1;
    } else {
        diff = total1 - diff;
    }
    arg2->fraction = diff % 0x6400;
    diff /= 0x6400;
    arg1 = (RaceTimer *)60;
    arg2->seconds = diff % (s32)arg1;
    diff /= (s32)arg1;
    arg2->minutes = diff % 99;
    return (s32)arg0;
}
