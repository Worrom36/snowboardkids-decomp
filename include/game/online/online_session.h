#ifndef ONLINE_SESSION_H
#define ONLINE_SESSION_H

#include "common.h"
#include "game/online/online_protocol.h"
#include "game/race/player/race_player_input.h"

void onlineSessionReset(void);
s32 onlineSessionActive(void);
s32 onlinePlayerIsRemote(const RacePlayer *player);
void onlineSessionConfigureRacePlayers(void);
void onlineSessionBindLocalView(void);
void onlineSessionPublishLocalRacer(RacePlayer *player);
void onlineSessionApplyRemoteRacers(void);
void onlineSessionApplyRemoteInput(RacePlayer *player);
void onlineSessionEnqueueEvent(u8 type, u8 seat, u8 arg0, u8 arg1);
void onlineSessionDrainInboundEvents(void);

/* Menu / flow helpers */
void onlineSessionBeginConnect(void);
void onlineSessionSetReady(s32 ready);
void onlineSessionRequestStart(void);
void onlineSessionDisconnect(void);
void onlineSessionSetLoadout(u8 courseId, u8 characterId, u8 characterVariant, u8 board, u8 laps);

OnlineLobbySnapshot *onlineSessionLobby(void);
u8 onlineSessionLocalSeat(void);
u8 onlineSessionPhase(void);
/* Player index for single-screen HUD / local-only collectibles (localSeat online). */
s32 onlineSessionFocusPlayerIndex(void);
/* Viewport/camera slot for a player: local seat always maps to 0 online. */
s32 onlineSessionViewIndex(s32 playerIndex);

#endif
