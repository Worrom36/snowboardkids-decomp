#include "game/race/player/race_player_update.h"

void resetRacePlayerTrickSubstate(RacePlayer *player) {
    player->subState = 0;
    player->subStateTimer = 0;
    player->subStateStep = 0;
    player->subStateParam = 0;
    player->unk2A0 = 0;
}
