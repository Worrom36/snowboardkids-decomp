#include "game/race/course/race_surface_cues.h"

s32 advanceRaceSurfaceCueSequence(RaceSurfaceCue *cues, RacePlayer *player, s16 cueIndex) {
    RaceSurfaceCue *cue;
    s16 step;
    s16 cueValue;
    u32 surfaceType;

    surfaceType = (player->inputFlags & 0xF0000) >> 0x10;
    switch (player->surfaceCueState[cueIndex]) {
        case 0:
            cue = cues;
            cue += player->surfaceCueStep[cueIndex];
            if (surfaceType != cue->surfaceType) {
                player->surfaceCueStep[cueIndex] = 0;
                return 0;
            }
            cueValue = cue->cueValue;
            if (cueValue != 0) {
                return cueValue;
            }
            player->surfaceCueState[cueIndex] = player->surfaceCueState[cueIndex] + 1;
        case 1:
            step = player->surfaceCueStep[cueIndex];
            if (surfaceType == cues[step].surfaceType) {
                return 0;
            }
            if (surfaceType == 0) {
                return 0;
            }
            player->surfaceCueState[cueIndex] = 0;
            player->surfaceCueStep[cueIndex] = step + 1;
        default:
            return 0;
    }
}

void resetRacePlayerSurfaceCueState(RacePlayer *player) {
    player->surfaceCueState[0] = 0;
    player->surfaceCueState[1] = 0;
    player->surfaceCueState[2] = 0;
    player->surfaceCueState[3] = 0;
    player->surfaceCueState[4] = 0;
    player->surfaceCueState[5] = 0;
    player->surfaceCueStep[0] = 0;
    player->surfaceCueStep[1] = 0;
    player->surfaceCueStep[2] = 0;
    player->surfaceCueStep[3] = 0;
    player->surfaceCueStep[4] = 0;
    player->surfaceCueStep[5] = 0;
}
