#include "game/race/player/race_player_movement.h"

void addRacePlayerScore(RacePlayer *arg0, s32 arg1) {
    if (arg0->isActive != 0) {
        arg0->score += arg1;
        if (arg0->score >= 0x186A0) {
            arg0->score = 0x1869F;
        }
        arg0->cumulativeRaceScore += arg1;
        if (arg0->cumulativeRaceScore >= 0x186A0) {
            arg0->cumulativeRaceScore = 0x1869F;
        }
    }
}
