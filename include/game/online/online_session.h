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
/*
 * Caster-side hit on a remote seat: enqueue ONLINE_EVENT_HIT for the victim and
 * set pending locally for ResolveRemoteCosmeticHits. Returns 0 if skipped
 * (inactive hold / already pending) so continuous overlaps do not spam.
 */
s32 onlineSessionQueueRemoteHit(u8 seat, u16 flag);
void onlineSessionDrainInboundEvents(void);
/* Path B: local-viewpoint bumps vs remote-owned seats (Bump race rule). */
void onlineSessionResolveViewpointBumps(void);
s32 onlineSessionBodyBumpEnabled(void);
/*
 * Caster-side: remotes skip updateRacePlayer so pending hits never resolve.
 * Call after projectile callbacks (and after the last applyRemoteRacers) so
 * impact FX / reaction modes appear on the attacker's screen.
 */
void onlineSessionResolveRemoteCosmeticHits(void);

/* Menu / flow helpers */
void onlineSessionBeginConnect(void);
/* Keep the session; ask the server to reopen the lobby after a race. */
void onlineSessionReturnToLobby(void);
/* Lobby START/COUNTDOWN for this race — enables sticky RACE_GO. */
void onlineSessionNoteRaceCountdown(void);
/* True after NoteRaceCountdown until ReturnToLobby (rematch GO gate). */
s32 onlineSessionRaceGoAllowed(void);
void onlineSessionSetReady(s32 ready);
void onlineSessionRequestStart(void);
void onlineSessionDisconnect(void);
void onlineSessionSetLoadout(u8 courseId, u8 characterId, u8 characterVariant, u8 board, u8 laps);
/* Copy a printable name into pendingLoadout and push SET_LOADOUT once joined. */
void onlineSessionSetPlayerName(const char *name);
/* Race-time display name for a seat (falls back to P1..P4 when blank). */
const char *onlineSessionPlayerName(s32 seat);

OnlineLobbySnapshot *onlineSessionLobby(void);
u8 onlineSessionLocalSeat(void);
u8 onlineSessionPhase(void);
/* Player index for single-screen HUD / local-only collectibles (localSeat online). */
s32 onlineSessionFocusPlayerIndex(void);
/* Viewport/camera slot for a player: local seat always maps to 0 online. */
s32 onlineSessionViewIndex(s32 playerIndex);

#endif
