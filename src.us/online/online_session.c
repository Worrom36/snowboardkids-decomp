#include "game/online/online_session.h"
#include "game/online/online_mailbox.h"
#include "game/race/race_state.h"
#include "game/race/camera/race_camera.h"
#include "game/race/motion/race_motion.h"
#include "game/race/player/race_player_collision.h"
#include "game/race/player/race_player_movement.h"
#include "game/engine/viewport_manager.h"

extern s16 gRaceLapCount;
extern s8 gRacePlayerCount;

static u8 sOnlineSessionActive;
static u8 sOnlineRemoteSeat[ONLINE_MAX_SEATS];

/*
 * Remotes skip updateRacePlayer / surface contact (those fight the networked
 * pose and launch puppets). Still need projectedPos + model matrices or the
 * mesh stays at a stale transform and looks like it flies off the course.
 */
static void onlineSessionPoseRemoteRacer(RacePlayer *player) {
    s32 groundLift;

    if (player->targetGroundOffset == 0) {
        player->targetGroundOffset = 0x40000;
    }
    player->groundOffset = player->targetGroundOffset;
    player->groundCorrectionY = 0;
    player->collisionBottomY = player->pos.y - 0x60000;
    updateRacePlayerProjectedPosition(player);
    groundLift = (0x60000 - player->groundOffset) / 0x600;
    interpolateRaceMotionJointAnimationFrame((RaceMotionState *)player, 0, groundLift, 0x100);
    resolveRacePlayerCollisionVolumes(player);
}

void onlineSessionReset(void) {
    s32 i;

    sOnlineSessionActive = 0;
    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        sOnlineRemoteSeat[i] = 0;
    }
    onlineMailboxInit();
}

s32 onlineSessionActive(void) {
    return sOnlineSessionActive != 0;
}

s32 onlinePlayerIsRemote(const RacePlayer *player) {
    if (player == NULL) {
        return 0;
    }
    if (player->replayInputSource == RACE_PLAYER_INPUT_REMOTE) {
        return 1;
    }
    if (player->playerIndex < ONLINE_MAX_SEATS) {
        return sOnlineRemoteSeat[player->playerIndex] != 0;
    }
    return 0;
}

void onlineSessionBeginConnect(void) {
    onlineSessionReset();
    sOnlineSessionActive = 1;
    gOnlineMailbox.lobby.phase = ONLINE_PHASE_CONNECTING;
    onlineMailboxRequest(ONLINE_CMD_CONNECT);
}

void onlineSessionDisconnect(void) {
    onlineMailboxRequest(ONLINE_CMD_DISCONNECT);
    sOnlineSessionActive = 0;
    gOnlineMailbox.lobby.phase = ONLINE_PHASE_IDLE;
}

void onlineSessionSetReady(s32 ready) {
    if (ready != 0) {
        onlineMailboxRequest(ONLINE_CMD_READY);
    } else {
        onlineMailboxRequest(ONLINE_CMD_UNREADY);
    }
}

void onlineSessionRequestStart(void) {
    onlineMailboxRequest(ONLINE_CMD_REQUEST_START);
}

void onlineSessionSetLoadout(u8 courseId, u8 characterId, u8 characterVariant, u8 board, u8 laps) {
    gOnlineMailbox.pendingLoadout.courseId = courseId;
    gOnlineMailbox.pendingLoadout.characterId = characterId;
    gOnlineMailbox.pendingLoadout.characterVariant = characterVariant;
    gOnlineMailbox.pendingLoadout.snowboardTextureIndex = board;
    gOnlineMailbox.pendingLoadout.lapCount = laps;
    /*
     * Single-byte gameToBridgeCmd cannot queue. During CONNECT the PC client
     * reads pendingLoadout after CONNECT; only push SET_LOADOUT once joined.
     */
    if (gOnlineMailbox.lobby.phase >= ONLINE_PHASE_LOBBY) {
        onlineMailboxRequest(ONLINE_CMD_SET_LOADOUT);
    }
}

OnlineLobbySnapshot *onlineSessionLobby(void) {
    return &gOnlineMailbox.lobby;
}

u8 onlineSessionLocalSeat(void) {
    return gOnlineMailbox.lobby.localSeat;
}

u8 onlineSessionPhase(void) {
    return gOnlineMailbox.lobby.phase;
}

void onlineSessionConfigureRacePlayers(void) {
    OnlineLobbySnapshot *lobby;
    s32 i;
    u8 localSeat;

    if (sOnlineSessionActive == 0) {
        return;
    }

    lobby = &gOnlineMailbox.lobby;
    localSeat = lobby->localSeat;
    gPlayerCount = 1;
    gRaceSplitscreenMode = 0;

    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        sOnlineRemoteSeat[i] = 0;
        gRacePlayers[i].isActive = 1;
        gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_LIVE;

        if (lobby->seats[i].kind == ONLINE_SEAT_HUMAN) {
            gRacePlayers[i].characterId = lobby->seats[i].loadout.characterId;
            gRacePlayers[i].selectedCharacterId = lobby->seats[i].loadout.characterId;
            gRacePlayers[i].characterVariant = lobby->seats[i].loadout.characterVariant;
            gRacePlayers[i].menuSelection = lobby->seats[i].loadout.characterVariant;
            gRacePlayers[i].snowboardTextureIndex = lobby->seats[i].loadout.snowboardTextureIndex;
            gRacePlayers[i].selectionUnlockState = lobby->seats[i].loadout.snowboardTextureIndex;

            if (i == (s32)localSeat) {
                gRacePlayers[i].isCpu = 0;
                gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_LIVE;
            } else {
                gRacePlayers[i].isCpu = 0;
                gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_REMOTE;
                sOnlineRemoteSeat[i] = 1;
            }
        } else {
            /* Empty / CPU filler — leave isCpu set by initRaceSceneFlow defaults. */
            gRacePlayers[i].isCpu = 1;
            gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_LIVE;
        }
    }

    if (lobby->seats[lobby->hostSeat < ONLINE_MAX_SEATS ? lobby->hostSeat : 0].loadout.lapCount != 0) {
        gRaceLapCount = lobby->seats[lobby->hostSeat < ONLINE_MAX_SEATS ? lobby->hostSeat : 0].loadout.lapCount;
    }
    /* Everyone must load the host's course or remotes appear in the wrong space. */
    gRaceCourseIndex.signedValue =
        lobby->seats[lobby->hostSeat < ONLINE_MAX_SEATS ? lobby->hostSeat : 0].loadout.courseId;
    gRacePlayerCount = 4;
}

void onlineSessionBindLocalView(void) {
    u8 localSeat;

    if (sOnlineSessionActive == 0) {
        return;
    }
    localSeat = gOnlineMailbox.lobby.localSeat;
    if (localSeat >= ONLINE_MAX_SEATS) {
        localSeat = 0;
    }
    /* Single-screen online: viewport/camera 0 must follow the local seat (not P1). */
    gPlayerCount = 1;
    gRaceCameras[0].playerIndex = localSeat;
    gViewportStates[0].overlayAlpha = 0;
}

void onlineSessionPublishLocalRacer(RacePlayer *player) {
    OnlineRacerSnapshot *out;

    if ((sOnlineSessionActive == 0) || (player == NULL)) {
        return;
    }
    if (player->playerIndex != gOnlineMailbox.lobby.localSeat) {
        return;
    }
    if (onlinePlayerIsRemote(player) != 0) {
        return;
    }

    out = &gOnlineMailbox.localOut;
    out->seat = (u8)player->playerIndex;
    out->valid = 1;
    out->itemEffectType = (u8)player->itemEffectType;
    out->itemEffectCount = (u8)player->itemEffectCount;
    out->actionEffectType = (u8)player->actionEffectType;
    out->sequence++;
    out->posX = player->pos.x;
    out->posY = player->pos.y;
    out->posZ = player->pos.z;
    out->velX = player->velocity.x;
    out->velY = player->velocity.y;
    out->velZ = player->velocity.z;
    out->facingAngle = player->facingAngle;
    out->pitchAngle = player->pitchAngle;
    out->leanAngle = player->leanAngle;
    out->coursePathIndex = player->coursePathIndex;
    out->coursePathOffset = player->coursePathOffset;
    out->mode = player->mode;
    out->stickX = player->stickX;
    out->stickY = player->stickY;
    out->inputFlags = player->inputFlags;
    gOnlineMailbox.gameHeartbeat++;
}

void onlineSessionApplyRemoteInput(RacePlayer *player) {
    OnlineRacerSnapshot *snap;

    if ((player == NULL) || (onlinePlayerIsRemote(player) == 0)) {
        return;
    }
    snap = &gOnlineMailbox.remoteIn[player->playerIndex & 3];
    if (snap->valid == 0) {
        player->inputFlags = 0;
        player->stickX = 0;
        player->stickY = 0;
        return;
    }
    player->inputFlags = snap->inputFlags;
    player->stickX = (s8)snap->stickX;
    player->stickY = (s8)snap->stickY;
}

void onlineSessionApplyRemoteRacers(void) {
    RacePlayer *player;
    OnlineRacerSnapshot *snap;
    s32 i;

    if (sOnlineSessionActive == 0) {
        return;
    }

    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        if (sOnlineRemoteSeat[i] == 0) {
            continue;
        }
        snap = &gOnlineMailbox.remoteIn[i];
        if (snap->valid == 0) {
            continue;
        }
        player = &gRacePlayers[i];
        player->previousPosition = player->pos;
        player->pos.x = snap->posX;
        player->pos.y = snap->posY;
        player->pos.z = snap->posZ;
        player->velocity.x = snap->velX;
        player->velocity.y = snap->velY;
        player->velocity.z = snap->velZ;
        player->facingAngle = snap->facingAngle;
        player->pitchAngle = snap->pitchAngle;
        player->leanAngle = snap->leanAngle;
        player->coursePathIndex = snap->coursePathIndex;
        player->coursePathOffset = snap->coursePathOffset;
        player->mode = snap->mode;
        player->itemEffectType = (s8)snap->itemEffectType;
        player->itemEffectCount = (s8)snap->itemEffectCount;
        player->actionEffectType = (s8)snap->actionEffectType;
        onlineSessionPoseRemoteRacer(player);
    }
}

void onlineSessionEnqueueEvent(u8 type, u8 seat, u8 arg0, u8 arg1) {
    OnlineRaceEvent *event;
    u8 index;

    if (sOnlineSessionActive == 0) {
        return;
    }
    if (gOnlineMailbox.eventOutCount >= ONLINE_EVENT_QUEUE_SIZE) {
        return;
    }
    index = gOnlineMailbox.eventOutCount;
    event = &gOnlineMailbox.eventOut[index];
    event->type = type;
    event->seat = seat;
    event->arg0 = arg0;
    event->arg1 = arg1;
    event->sequence = gOnlineMailbox.gameHeartbeat;
    gOnlineMailbox.eventOutCount = index + 1;
}

void onlineSessionDrainInboundEvents(void) {
    /* Consumed by bridge→game path; game clears after reading. */
    gOnlineMailbox.eventInCount = 0;
}
