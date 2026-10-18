#include "game/online/online_session.h"
#include "game/online/online_mailbox.h"
#include "game/race/race_state.h"
#include "game/race/camera/race_camera.h"
#include "game/race/motion/race_motion.h"
#include "game/race/player/race_player_collision.h"
#include "game/race/player/race_player_movement.h"
#include "game/race/player/race_player_pickup_effects.h"
#include "game/engine/viewport_manager.h"

extern s16 gRaceLapCount;
extern s8 gRacePlayerCount;

static u8 sOnlineSessionActive;
static u8 sOnlineRemoteSeat[ONLINE_MAX_SEATS];
static s16 sRemoteAppliedAnim[ONLINE_MAX_SEATS];
static u8 sRemoteYawFlipHold[ONLINE_MAX_SEATS];

/* Local renderer bits — must not be wiped by authority snapshots or draws skip frames. */
#define ONLINE_REMOTE_STATEFLAGS_LOCAL_MASK 0x180
/* Board yaw-flip; toggles at the start line and mirrors the mesh every frame if raw-synced. */
#define ONLINE_REMOTE_STATEFLAGS_YAW_FLIP   0x400
#define ONLINE_REMOTE_YAW_FLIP_HOLD_FRAMES  3

/*
 * Remotes skip updateRacePlayer / surface contact (those fight the networked
 * pose and launch puppets). Still need projectedPos + model matrices or the
 * mesh stays at a stale transform and looks like it flies off the course.
 * Animation / model rotations come from the authority snapshot so tricks and
 * falls show up on puppets.
 *
 * Grounded posing mirrors mode00 + postUpdateMode00: turn/idle upper-body
 * first, then board-height blend from anim 0. Sampling turn clips (2/3) for
 * that blend sinks the root into the ground when steering.
 */
static void onlineSessionPoseRemoteRacer(RacePlayer *player) {
    s32 groundLift;
    s32 groundOffset;
    s16 anim;
    s32 seat;
    s32 blendAmount;

    seat = player->playerIndex & 3;
    player->groundCorrectionY = 0;
    player->collisionBottomY = player->pos.y - 0x60000;
    updateRacePlayerProjectedPosition(player);

    anim = player->animationId;
    if (anim < 0) {
        anim = 0;
    }

    if (player->mode == 0) {
        if ((anim == 2) || (anim == 3)) {
            if (sRemoteAppliedAnim[seat] != anim) {
                setRaceMotionAnimation((RaceMotionState *)player, anim);
                sRemoteAppliedAnim[seat] = anim;
            }
            blendAmount = player->stickX;
            if (blendAmount < 0) {
                blendAmount = -blendAmount;
            }
            if (blendAmount < 0x20) {
                blendAmount = 0x40;
            }
            if (blendAmount > 0x118) {
                blendAmount = 0x118;
            }
            blendRaceMotionJointAnimation((RaceMotionState *)player, anim, blendAmount, 0x118);
        } else {
            if (anim == 0) {
                anim = 1;
            }
            if (sRemoteAppliedAnim[seat] != anim) {
                setRaceMotionAnimation((RaceMotionState *)player, anim);
                sRemoteAppliedAnim[seat] = anim;
            }
            stepRaceMotionLoopingJointAnimation((RaceMotionState *)player);
        }

        groundOffset = player->groundOffset;
        if (groundOffset <= 0) {
            groundOffset = 0x40000;
        } else if (groundOffset > 0x60000) {
            groundOffset = 0x60000;
        }
        groundLift = (0x60000 - groundOffset) / 0x600;
        if (groundLift < 0) {
            groundLift = 0;
        } else if (groundLift > 0x100) {
            groundLift = 0x100;
        }
        interpolateRaceMotionJointAnimationFrame((RaceMotionState *)player, 0, groundLift, 0x100);
    } else {
        if (sRemoteAppliedAnim[seat] != anim) {
            setRaceMotionAnimation((RaceMotionState *)player, anim);
            sRemoteAppliedAnim[seat] = anim;
        }
        stepRaceMotionJointAnimationUntilEnd((RaceMotionState *)player);
    }
    resolveRacePlayerCollisionVolumes(player);
}

void onlineSessionReset(void) {
    s32 i;

    sOnlineSessionActive = 0;
    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        sOnlineRemoteSeat[i] = 0;
        sRemoteAppliedAnim[i] = -1;
        sRemoteYawFlipHold[i] = 0;
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

s32 onlineSessionFocusPlayerIndex(void) {
    u8 seat;

    if (sOnlineSessionActive == 0) {
        return 0;
    }
    seat = gOnlineMailbox.lobby.localSeat;
    if (seat >= ONLINE_MAX_SEATS) {
        return 0;
    }
    return (s32)seat;
}

s32 onlineSessionViewIndex(s32 playerIndex) {
    u8 seat;

    if (sOnlineSessionActive == 0) {
        return playerIndex;
    }
    seat = gOnlineMailbox.lobby.localSeat;
    if (seat >= ONLINE_MAX_SEATS) {
        seat = 0;
    }
    if (playerIndex == (s32)seat) {
        return 0;
    }
    return playerIndex;
}

void onlineSessionConfigureRacePlayers(void) {
    OnlineLobbySnapshot *lobby;
    s32 i;
    s32 humanCount;
    u8 localSeat;

    if (sOnlineSessionActive == 0) {
        return;
    }

    lobby = &gOnlineMailbox.lobby;
    localSeat = lobby->localSeat;
    gPlayerCount = 1;
    gRaceSplitscreenMode = 0;
    humanCount = 0;

    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        sOnlineRemoteSeat[i] = 0;
        gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_LIVE;

        if (lobby->seats[i].kind == ONLINE_SEAT_HUMAN) {
            gRacePlayers[i].isActive = 1;
            gRacePlayers[i].characterId = lobby->seats[i].loadout.characterId;
            gRacePlayers[i].selectedCharacterId = lobby->seats[i].loadout.characterId;
            gRacePlayers[i].characterVariant = lobby->seats[i].loadout.characterVariant;
            gRacePlayers[i].menuSelection = lobby->seats[i].loadout.characterVariant;
            gRacePlayers[i].snowboardTextureIndex = lobby->seats[i].loadout.snowboardTextureIndex;
            gRacePlayers[i].selectionUnlockState = lobby->seats[i].loadout.snowboardTextureIndex;
            humanCount++;

            if (i == (s32)localSeat) {
                gRacePlayers[i].isCpu = 0;
                gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_LIVE;
            } else {
                gRacePlayers[i].isCpu = 0;
                gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_REMOTE;
                sOnlineRemoteSeat[i] = 1;
            }
        } else {
            /*
             * Keep empty seats inactive and out of gRacePlayerCount. Leaving
             * count at 4 made cameras 2/3 follow uninitialized players (black
             * screen / emulator fault on both clients).
             */
            gRacePlayers[i].isCpu = 1;
            gRacePlayers[i].isActive = 0;
            gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_LIVE;
        }
    }

    if (lobby->seats[lobby->hostSeat < ONLINE_MAX_SEATS ? lobby->hostSeat : 0].loadout.lapCount != 0) {
        gRaceLapCount = lobby->seats[lobby->hostSeat < ONLINE_MAX_SEATS ? lobby->hostSeat : 0].loadout.lapCount;
    }
    /* Everyone must load the host's course or remotes appear in the wrong space. */
    gRaceCourseIndex.signedValue =
        lobby->seats[lobby->hostSeat < ONLINE_MAX_SEATS ? lobby->hostSeat : 0].loadout.courseId;
    gRacePlayerCount = (humanCount > 0) ? (s8)humanCount : 1;
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
    out->animationId = player->animationId;
    out->inputFlags = player->inputFlags;
    out->updateState = player->updateState;
    out->modelRotationX = player->modelRotationX;
    out->modelRotationY = player->modelRotationY;
    out->modelRotationZ = player->modelRotationZ;
    out->stateFlags = player->stateFlags & ~ONLINE_REMOTE_STATEFLAGS_LOCAL_MASK;
    out->stateTimer = player->stateTimer;
    out->groundOffset = player->groundOffset;
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
    u32 localBits;
    u32 incoming;
    u32 yawBit;
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
        player->animationId = snap->animationId;
        player->updateState = snap->updateState;
        player->modelRotationX = snap->modelRotationX;
        player->modelRotationY = snap->modelRotationY;
        player->modelRotationZ = snap->modelRotationZ;

        localBits = player->stateFlags & ONLINE_REMOTE_STATEFLAGS_LOCAL_MASK;
        incoming = snap->stateFlags & ~ONLINE_REMOTE_STATEFLAGS_LOCAL_MASK;
        yawBit = incoming & ONLINE_REMOTE_STATEFLAGS_YAW_FLIP;
        if (yawBit != (player->stateFlags & ONLINE_REMOTE_STATEFLAGS_YAW_FLIP)) {
            if (sRemoteYawFlipHold[i] < ONLINE_REMOTE_YAW_FLIP_HOLD_FRAMES) {
                sRemoteYawFlipHold[i]++;
                incoming = (incoming & ~ONLINE_REMOTE_STATEFLAGS_YAW_FLIP) |
                           (player->stateFlags & ONLINE_REMOTE_STATEFLAGS_YAW_FLIP);
            } else {
                sRemoteYawFlipHold[i] = 0;
            }
        } else {
            sRemoteYawFlipHold[i] = 0;
        }
        player->stateFlags = incoming | localBits;

        player->stateTimer = snap->stateTimer;
        player->groundOffset = snap->groundOffset;
        if (snap->groundOffset != 0) {
            player->targetGroundOffset = snap->groundOffset;
        }
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
    OnlineRaceEvent *event;
    RacePlayer *player;
    s32 i;
    u8 count;

    if (sOnlineSessionActive == 0) {
        gOnlineMailbox.eventInCount = 0;
        return;
    }

    count = gOnlineMailbox.eventInCount;
    for (i = 0; i < count; i++) {
        event = &gOnlineMailbox.eventIn[i];
        if (event->seat >= ONLINE_MAX_SEATS) {
            continue;
        }
        if (event->type == ONLINE_EVENT_HIT) {
            /* Only the victim resolves hits (authority for own mode/reactions). */
            if (event->seat == gOnlineMailbox.lobby.localSeat) {
                player = &gRacePlayers[event->seat];
                player->pendingItemHitFlags |= (u16)event->arg0 | ((u16)event->arg1 << 8);
            }
            continue;
        }
        if (sOnlineRemoteSeat[event->seat] == 0) {
            continue;
        }
        player = &gRacePlayers[event->seat];
        if (event->type == ONLINE_EVENT_ITEM_USE) {
            applyOnlineRemoteItemUse(player, event->arg0);
        } else if (event->type == ONLINE_EVENT_ACTION_USE) {
            applyOnlineRemoteActionUse(player, event->arg0);
        }
    }
    gOnlineMailbox.eventInCount = 0;
}
