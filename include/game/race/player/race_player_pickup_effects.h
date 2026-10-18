#ifndef RACE_PLAYER_PICKUP_EFFECTS_H
#define RACE_PLAYER_PICKUP_EFFECTS_H

#include "game/race/player/race_player_input.h"

void updateRacePlayerItemEffectUse(RacePlayer *player);
void updateRacePlayerActionEffectUse(RacePlayer *player);

#ifdef ONLINE_BATTLE
void applyOnlineRemoteItemUse(RacePlayer *player, s32 itemType);
void applyOnlineRemoteActionUse(RacePlayer *player, s32 actionType);
#endif

#endif
