#include "game/online/online_session.h"
#include "game/online/online_mailbox.h"
#include "game/race/race_state.h"
#include "game/race/camera/race_camera.h"
#include "game/race/motion/race_motion.h"
#include "game/race/items/race_item_effects.h"
#include "game/race/player/race_player_collision.h"
#include "game/race/player/race_player_movement.h"
#include "game/race/player/race_player_pickup_effects.h"
#include "game/race/items/race_items.h"
#include "game/math/fixed_point_math.h"
#include "game/math/spatial_math.h"
#include "game/engine/asset_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/viewport_manager.h"
#include "game/engine/game_task_scheduler.h"
#include "game/audio/sound_manager.h"

extern s16 gRaceLapCount;
extern s8 gRacePlayerCount;
extern s32 gMenuFlowState;
extern Vec3s *gRaceCourseSurfaceCoords;
extern RaceCourseSurface *gRaceCourseSurfaces;
extern Vec3i gRacePlayerGroundProbeOffsets[];

static u8 sOnlineSessionActive;
static u8 sOnlineRemoteSeat[ONLINE_MAX_SEATS];
static s16 sRemoteAppliedAnim[ONLINE_MAX_SEATS];
static u8 sRemoteYawFlipHold[ONLINE_MAX_SEATS];
/*
 * Path B bump: cooldown gates net events only. Position separation runs
 * every overlapping frame (same cadence as resolveRacePlayerBodyCollisions).
 */
static u8 sBumpCooldown[ONLINE_MAX_SEATS][ONLINE_MAX_SEATS];
#define ONLINE_BUMP_EVENT_COOLDOWN_FRAMES 8
/* Hold caster-side cosmetic hit mode/anim against snap overwrite briefly. */
static u8 sRemoteCosmeticHitHold[ONLINE_MAX_SEATS];
#define ONLINE_REMOTE_COSMETIC_HIT_HOLD_FRAMES 12
/*
 * Deferred remote item/action visuals. UDP ITEM_USE/ACTION_USE is one-shot and
 * often lost while hit reactions still show via continuous pose snaps — spawn
 * from authority itemEffectCount / actionEffectType drops instead.
 */
static u8 sRemotePendingItemType[ONLINE_MAX_SEATS];
static u8 sRemotePendingItemDrops[ONLINE_MAX_SEATS];
static u8 sRemotePendingActionType[ONLINE_MAX_SEATS];
/* Ignore duplicate EVENT+snap spawns for the same fire (~10 frames). */
static u8 sRemoteItemSpawnCooldown[ONLINE_MAX_SEATS];
static u8 sRemoteActionSpawnCooldown[ONLINE_MAX_SEATS];
#define ONLINE_REMOTE_ITEM_SPAWN_COOLDOWN 10

/* Local renderer bits — must not be wiped by authority snapshots or draws skip frames. */
#define ONLINE_REMOTE_STATEFLAGS_LOCAL_MASK 0x180
/* Board yaw-flip; toggles at the start line and mirrors the mesh every frame if raw-synced. */
#define ONLINE_REMOTE_STATEFLAGS_YAW_FLIP   0x400
/* Hold long enough that a flicker/reorder cannot invert the mesh into the snow. */
#define ONLINE_REMOTE_YAW_FLIP_HOLD_FRAMES  12
/* Finished race — once seen, keep it so stale snaps cannot drop remotes from results. */
#define ONLINE_REMOTE_STATEFLAGS_FINISHED   0x40

/*
 * Target-follow: snaps set target+vel; each tick eases pos toward target.
 * Gap frames advance the target with vel (prediction). Never add vel on top
 * of a full error dump — that oscillated forward/back up close.
 */
#define ONLINE_REMOTE_COAST_MAX_FRAMES 12 /* ~200 ms at 60 fps */
#define ONLINE_REMOTE_SMOOTH_DIV       5  /* XZ only — Y is client/terrain when grounded */
#define ONLINE_REMOTE_HARD_SNAP_AXIS   0x1000000 /* teleport / respawn only */
#define ONLINE_REMOTE_Y_FLOOR_PAD      0xA000    /* air: never draw below local mesh */
/* Stay surface-locked through brief mode0↔air bumps so Y does not gravity-coast. */
#define ONLINE_REMOTE_AIR_UNLOCK_FRAMES 4

typedef struct OnlineRemoteMotion {
    u8 initialized;
    u8 framesSinceSnap;
    u8 grounded; /* surface-locked: no gravity Y coast */
    u8 airFrames; /* consecutive non-locked snaps; hysteresis before unlock */
    u8 pathMode; /* CPU: hard-place from path+lateral */
    u32 lastSequence;
    s32 posX;
    s32 posY;
    s32 posZ;
    s32 velX;
    s32 velY;
    s32 velZ;
    s32 targetX;
    s32 targetY;
    s32 targetZ;
    s32 groundOffset;
    s32 alongSpeed;
    s32 coursePathOffset;
    s32 lateralOffset;
    s16 coursePathIndex;
    s16 facingAngle;
    s16 pitchAngle;
    s16 leanAngle;
    s16 mode;
    s16 animationId;
    s16 updateState;
    u8 itemEffectType;
    u8 itemEffectCount;
    u8 actionEffectType;
    u8 lapDigit;
    u32 stateFlags;
} OnlineRemoteMotion;

static char sOnlinePlayerNames[ONLINE_MAX_SEATS][ONLINE_PLAYER_NAME_LEN + 1];
static const char sOnlineSeatFallbackNames[ONLINE_MAX_SEATS][3] = { "P1", "P2", "P3", "P4" };

/* Copy up to NAME_LEN printable chars, trim trailing spaces, NUL-terminate. */
static void onlineCopyPlayerName(char *dst, const char *src, s32 srcLen) {
    s32 i;
    s32 out;

    out = 0;
    for (i = 0; i < srcLen && out < ONLINE_PLAYER_NAME_LEN; i++) {
        char c = src[i];
        if (c == '\0') {
            break;
        }
        if (c < ' ' || c > '~') {
            c = ' ';
        }
        dst[out++] = c;
    }
    while (out > 0 && dst[out - 1] == ' ') {
        out--;
    }
    dst[out] = '\0';
}

const char *onlineSessionPlayerName(s32 seat) {
    if (seat < 0 || seat >= ONLINE_MAX_SEATS) {
        return "";
    }
    if (sOnlinePlayerNames[seat][0] == '\0') {
        return sOnlineSeatFallbackNames[seat];
    }
    return sOnlinePlayerNames[seat];
}

void onlineSessionSetPlayerName(const char *name) {
    char cleaned[ONLINE_PLAYER_NAME_LEN + 1];
    s32 i;
    s32 len;

    onlineCopyPlayerName(cleaned, name, ONLINE_PLAYER_NAME_LEN);
    len = 0;
    while (cleaned[len] != '\0') {
        len++;
    }
    for (i = 0; i < ONLINE_PLAYER_NAME_LEN; i++) {
        gOnlineMailbox.pendingLoadout.name[i] = (i < len) ? cleaned[i] : ' ';
    }
    if (gOnlineMailbox.lobby.localSeat < ONLINE_MAX_SEATS) {
        onlineCopyPlayerName(sOnlinePlayerNames[gOnlineMailbox.lobby.localSeat], cleaned, ONLINE_PLAYER_NAME_LEN);
        for (i = 0; i < ONLINE_PLAYER_NAME_LEN; i++) {
            gOnlineMailbox.lobby.seats[gOnlineMailbox.lobby.localSeat].loadout.name[i] =
                (i < len) ? cleaned[i] : ' ';
        }
    }
    if (gOnlineMailbox.lobby.phase >= ONLINE_PHASE_LOBBY) {
        onlineMailboxRequest(ONLINE_CMD_SET_LOADOUT);
    }
}

static OnlineRemoteMotion sRemoteMotion[ONLINE_MAX_SEATS];
static s16 sRemotePredictFrame;
static u32 sPublishSeq[ONLINE_MAX_SEATS];
static s32 sCpuPrevPathOffset[ONLINE_MAX_SEATS];
static s16 sCpuPrevPathIndex[ONLINE_MAX_SEATS];
static u8 sCpuPrevPathValid[ONLINE_MAX_SEATS];

/*
 * Modes that still run surface contact on the authority (board stays on snow).
 * Remotes have no contact probes — treating these as airborne lets gravity
 * velY coast the puppet through the mesh (visible when holding A/SHIFT crouch).
 * stateFlags 0x10 = jump charge; mode 2 updateState 0 = launch squat countdown.
 */
static s32 onlineRemoteSurfaceLocked(s16 mode, s16 updateState, u32 stateFlags) {
    if (mode == 0) {
        return 1;
    }
    if (stateFlags & 0x10) {
        return 1;
    }
    if ((mode == 2) && (updateState == 0)) {
        return 1;
    }
    /*
     * Ground hit reactions (pan 0x18, spin 0x19, steal 0x17):
     * authority Mode* sinks pos.y then PostUpdateNoop surface-contact fixes it,
     * but publish can still carry a sunk Y — remotes that follow snap Y bury the
     * puppet in the mesh (seen as the player "disappearing" after a pan).
     * Do NOT include slide 0x1B: Mode27 launches with vel.y and spins in air
     * (parachute-item hit). Terrain-glue left victims spinning on the snow.
     */
    if (mode == 0x17 || mode == 0x18 || mode == 0x19) {
        return 1;
    }
    return 0;
}

/*
 * True airborne / aerial-trick / launched-hit modes — never terrain-glue.
 * Do NOT include ground knockdowns (0x17–0x19); those publish a sunk Y
 * before surface contact and must stick to local terrain on remotes.
 */
static s32 onlineRemoteIsReactionAirMode(s16 mode) {
    if (mode == 1 || mode == 2 || mode == 3 || mode == 4 || mode == 6 || mode == 9 || mode == 0xA) {
        return 1;
    }
    if (mode == 0xB || mode == 0xC || mode == 0xD || mode == 0xE || mode == 0x28) {
        return 1;
    }
    if (mode >= 0xF && mode <= 0x15) {
        return 1;
    }
    /*
     * 0x16 airborne cruise, 0x1A bomb tumble, 0x1B parachute-item slide (launches),
     * 0x1C terrain-fall-with-item.
     */
    if (mode == 0x16 || mode == 0x1A || mode == 0x1B || mode == 0x1C) {
        return 1;
    }
    if (mode >= 0x1F && mode <= 0x27) {
        return 1;
    }
    if (mode >= 0x29 && mode <= 0x39) {
        return 1;
    }
    return 0;
}

/* Update grounded with hysteresis; returns 1 if surface-locked this frame. */
static s32 onlineRemoteUpdateGrounded(OnlineRemoteMotion *motion, OnlineRacerSnapshot *snap) {
    if (onlineRemoteSurfaceLocked(snap->mode, snap->updateState, snap->stateFlags)) {
        motion->airFrames = 0;
        motion->grounded = 1;
    } else if (onlineRemoteIsReactionAirMode(snap->mode)) {
        /* Instant unlock so parachute/knockdown Y is not stuck to the snow. */
        motion->airFrames = ONLINE_REMOTE_AIR_UNLOCK_FRAMES;
        motion->grounded = 0;
    } else if (motion->airFrames < ONLINE_REMOTE_AIR_UNLOCK_FRAMES) {
        /* Brief mode0↔0x16 bumps only. */
        motion->airFrames++;
        motion->grounded = 1;
    } else {
        motion->grounded = 0;
    }
    return motion->grounded != 0;
}

static s32 onlineClampPathIndex(s32 index) {
    s32 maxIndex;

    maxIndex = gRaceCourseStartEntries[gRaceCourseIndex.signedValue].maxSurfaceIndex;
    if (maxIndex < 0) {
        return 0;
    }
    if (index < 0) {
        return 0;
    }
    if (index > maxIndex) {
        return maxIndex;
    }
    return index;
}

/*
 * findRaceCourseSurfaceFromHint walks neighbor links with no cycle guard —
 * safe for the local player (physics keeps it on the strip), but a predicted
 * remote can drift off-course and bounce between two surfaces forever,
 * hanging the whole client (publish/apply stop, emulator spins). Use the
 * bounded full scan for remote placement; fall back to the clamped hint
 * surface when the point is outside every quad.
 */
static s32 onlineSafeSurfaceAt(s32 hint, s32 x, s32 z) {
    s32 terrainId;

    terrainId = findRaceCourseSurfaceAtPoint(x, z);
    if (terrainId != 0) {
        return terrainId;
    }
    return onlineClampPathIndex(hint);
}

/* Air only: never draw below the local mesh (sunk snap Y / mode glitches). */
static void onlineRemoteClampAboveSurface(OnlineRemoteMotion *motion, s16 pathHint) {
    s32 terrainId;
    s32 surfaceY;
    s32 floorY;

    terrainId = onlineSafeSurfaceAt(pathHint, motion->posX, motion->posZ);
    surfaceY = getRaceCourseSurfaceHeight(terrainId, motion->posX, motion->posZ);
    floorY = surfaceY + ONLINE_REMOTE_Y_FLOOR_PAD;
    if (motion->posY < floorY) {
        motion->posY = floorY;
        if (motion->targetY < floorY) {
            motion->targetY = floorY;
        }
    }
}

/*
 * Client-authoritative grounded Y: hard-place on this machine's mesh under the
 * visual XZ. Ignore net posY entirely while surface-locked — easing/filtering
 * net or local height is what bobbed remotes up/down and left them hovering.
 * groundOffset still comes from authority (crouch amount).
 */
static void onlineRemoteStickToSurface(OnlineRemoteMotion *motion, s16 pathHint) {
    s32 terrainId;
    s32 surfaceY;
    s32 offset;

    terrainId = onlineSafeSurfaceAt(pathHint, motion->posX, motion->posZ);
    surfaceY = getRaceCourseSurfaceHeight(terrainId, motion->posX, motion->posZ);
    offset = motion->groundOffset;
    if (offset <= 0) {
        offset = 0x40000;
    } else if (offset > 0x60000) {
        offset = 0x60000;
    }
    motion->posY = surfaceY + offset;
    motion->targetY = motion->posY;
    motion->velY = 0;
}

/*
 * Air / launched hits: follow authority Y (jumps/tricks), then floor-clamp.
 * Grounded paths never call this.
 */
static void onlineRemoteEaseAirY(OnlineRemoteMotion *motion, s32 authorityY, s16 pathHint) {
    motion->targetY = authorityY;
    motion->posY = authorityY;
    motion->velY = 0;
    onlineRemoteClampAboveSurface(motion, pathHint);
}

static void onlineRemoteMotionResetSeat(s32 seat) {
    OnlineRemoteMotion *motion;

    motion = &sRemoteMotion[seat & 3];
    motion->initialized = 0;
    motion->framesSinceSnap = 0;
    motion->grounded = 1;
    motion->airFrames = 0;
    motion->pathMode = 0;
    motion->lastSequence = 0;
    motion->posX = 0;
    motion->posY = 0;
    motion->posZ = 0;
    motion->velX = 0;
    motion->velY = 0;
    motion->velZ = 0;
    motion->targetX = 0;
    motion->targetY = 0;
    motion->targetZ = 0;
    motion->groundOffset = 0x40000;
    motion->alongSpeed = 0;
    motion->coursePathOffset = 0;
    motion->lateralOffset = 0;
    motion->coursePathIndex = 0;
    motion->facingAngle = 0;
    motion->pitchAngle = 0;
    motion->leanAngle = 0;
    motion->mode = 0;
    motion->animationId = 0;
    motion->updateState = 0;
    motion->itemEffectType = 0;
    motion->itemEffectCount = 0;
    motion->actionEffectType = 0;
    motion->lapDigit = 0;
    motion->stateFlags = 0;
    sRemotePendingItemType[seat & 3] = RACE_ITEM_NONE;
    sRemotePendingItemDrops[seat & 3] = 0;
    sRemotePendingActionType[seat & 3] = RACE_ACTION_NONE;
    sRemoteItemSpawnCooldown[seat & 3] = 0;
    sRemoteActionSpawnCooldown[seat & 3] = 0;
    sPublishSeq[seat & 3] = 0;
    sCpuPrevPathValid[seat & 3] = 0;
}

/*
 * Lateral complementary to projectRaceCourseSurfaceProgress:
 * along = (-sin*dx + cos*dz)/0x1000, lateral = (cos*dx + sin*dz)/0x1000.
 */
static s32 onlineCpuLateralFromPos(s16 pathIndex, s32 posX, s32 posZ) {
    RaceCourseSurface *surface;
    s32 refX;
    s32 refZ;
    s32 deltaX;
    s32 deltaZ;
    s16 sinA;
    s16 cosA;

    pathIndex = (s16)onlineClampPathIndex(pathIndex);
    surface = &gRaceCourseSurfaces[pathIndex];
    refX = gRaceCourseSurfaceCoords[surface->referenceCoordIndex].x << 0x11;
    refZ = gRaceCourseSurfaceCoords[surface->referenceCoordIndex].z << 0x11;
    deltaX = posX - refX;
    deltaZ = posZ - refZ;
    sinA = fixedSine(surface->pathAngle);
    cosA = fixedCosine(surface->pathAngle);
    return (s32)((((s64)cosA * deltaX) + ((s64)sinA * deltaZ)) / 0x1000);
}

/*
 * Invert path project: place XZ from along-track offset + true lateral.
 * coursePathOffset is stored as -project(...); along = -offset.
 */
static void onlineCpuPlaceFromPath(
    s16 pathIndex,
    s32 pathOffset,
    s32 lateral,
    s32 groundOffset,
    s32 *outX,
    s32 *outY,
    s32 *outZ
) {
    RaceCourseSurface *surface;
    s32 refX;
    s32 refZ;
    s32 along;
    s16 sinA;
    s16 cosA;
    s32 terrainId;
    s32 offset;

    pathIndex = (s16)onlineClampPathIndex(pathIndex);
    surface = &gRaceCourseSurfaces[pathIndex];
    refX = gRaceCourseSurfaceCoords[surface->referenceCoordIndex].x << 0x11;
    refZ = gRaceCourseSurfaceCoords[surface->referenceCoordIndex].z << 0x11;
    along = -pathOffset;
    sinA = fixedSine(surface->pathAngle);
    cosA = fixedCosine(surface->pathAngle);
    *outX = refX + (s32)((((s64)cosA * lateral) + ((s64)(-sinA) * along)) / 0x1000);
    *outZ = refZ + (s32)((((s64)sinA * lateral) + ((s64)cosA * along)) / 0x1000);
    terrainId = onlineSafeSurfaceAt(pathIndex, *outX, *outZ);
    offset = groundOffset;
    if (offset <= 0) {
        offset = 0x40000;
    } else if (offset > 0x60000) {
        offset = 0x60000;
    }
    *outY = getRaceCourseSurfaceHeight(terrainId, *outX, *outZ) + offset;
}

static void onlineRemoteAcceptCpuPath(OnlineRemoteMotion *motion, OnlineCpuPathSnapshot *snap, s32 seat) {
    motion->pathMode = 1;
    motion->grounded = 1;
    motion->airFrames = 0;
    motion->coursePathIndex = (s16)onlineClampPathIndex(snap->coursePathIndex);
    motion->coursePathOffset = snap->coursePathOffset;
    motion->alongSpeed = snap->alongSpeed;
    motion->lateralOffset = snap->lateralOffset;
    motion->facingAngle = snap->facingAngle;
    motion->pitchAngle = 0;
    motion->leanAngle = 0;
    motion->mode = snap->mode;
    motion->animationId = snap->animationId;
    motion->updateState = snap->updateState;
    motion->itemEffectType = snap->itemEffectType;
    motion->itemEffectCount = snap->itemEffectCount;
    motion->actionEffectType = snap->actionEffectType;
    motion->lapDigit = snap->lapDigit;
    motion->stateFlags = snap->stateFlags;
    if (snap->groundOffset <= 0) {
        motion->groundOffset = 0x40000;
    } else {
        motion->groundOffset = snap->groundOffset;
    }
    motion->lastSequence = snap->sequence;
    motion->framesSinceSnap = 0;
    motion->velX = 0;
    motion->velY = 0;
    motion->velZ = 0;
    /* Hard place from path+true lateral. No soft-correct/coast (those drifted remotes). */
    onlineCpuPlaceFromPath(
        motion->coursePathIndex,
        motion->coursePathOffset,
        motion->lateralOffset,
        motion->groundOffset,
        &motion->posX,
        &motion->posY,
        &motion->posZ
    );
    motion->targetX = motion->posX;
    motion->targetY = motion->posY;
    motion->targetZ = motion->posZ;
    motion->initialized = 1;
    (void)seat;
}

static void onlineRemoteTickCpuPath(OnlineRemoteMotion *motion) {
    if (motion->initialized == 0 || motion->pathMode == 0) {
        return;
    }
    if (motion->framesSinceSnap < 0xFF) {
        motion->framesSinceSnap++;
    }
    /* Hold last placed pose until the next path snap — no along-track coast. */
}

static s16 onlineRemoteBlendAngle(s16 current, s16 target) {
    s32 delta;

    delta = (s16)(target - current);
    return (s16)(current + (delta / ONLINE_REMOTE_SMOOTH_DIV));
}

static s32 onlineRemoteAbs(s32 value) {
    return (value < 0) ? -value : value;
}

static s32 onlineRemoteHardSnapNeeded(s32 errX, s32 errY, s32 errZ) {
    return (onlineRemoteAbs(errX) > ONLINE_REMOTE_HARD_SNAP_AXIS) ||
           (onlineRemoteAbs(errY) > ONLINE_REMOTE_HARD_SNAP_AXIS) ||
           (onlineRemoteAbs(errZ) > ONLINE_REMOTE_HARD_SNAP_AXIS);
}

/*
 * Full XYZ snap → update follow target. Angles are blended in apply (not
 * hard-assigned here) so close-up facing does not teleport every packet.
 * Large error / first snap → hard place; otherwise tick eases toward target.
 */
static void onlineRemoteAcceptSnap(OnlineRemoteMotion *motion, OnlineRacerSnapshot *snap) {
    s32 errX;
    s32 errY;
    s32 errZ;

    motion->mode = snap->mode;
    onlineRemoteUpdateGrounded(motion, snap);
    motion->velX = snap->velX;
    motion->velY = snap->velY;
    motion->velZ = snap->velZ;
    if (snap->groundOffset <= 0) {
        motion->groundOffset = 0x40000;
    } else {
        motion->groundOffset = snap->groundOffset;
    }
    motion->lastSequence = snap->sequence;
    motion->framesSinceSnap = 0;

    if (motion->grounded != 0) {
        motion->velY = 0;
    }

    motion->targetX = snap->posX;
    motion->targetZ = snap->posZ;
    if (motion->grounded == 0) {
        motion->targetY = snap->posY;
    }

    if (motion->initialized == 0) {
        motion->posX = motion->targetX;
        motion->posZ = motion->targetZ;
        if (motion->grounded == 0) {
            motion->posY = motion->targetY;
        }
        /* Seed angles once; later packets ease in apply. */
        motion->facingAngle = snap->facingAngle;
        motion->pitchAngle = snap->pitchAngle;
        motion->leanAngle = snap->leanAngle;
        motion->initialized = 1;
        return;
    }

    errX = motion->targetX - motion->posX;
    errZ = motion->targetZ - motion->posZ;
    if (motion->grounded != 0) {
        errY = 0;
    } else {
        errY = motion->targetY - motion->posY;
    }

    /* Teleport / respawn only — routine jitter eases in tick. */
    if (onlineRemoteHardSnapNeeded(errX, errY, errZ) != 0) {
        motion->posX = motion->targetX;
        motion->posZ = motion->targetZ;
        if (motion->grounded == 0) {
            motion->posY = motion->targetY;
        }
    }
}

static void onlineRemoteTickMotion(OnlineRemoteMotion *motion) {
    s32 errX;
    s32 errY;
    s32 errZ;

    if (motion->initialized == 0) {
        return;
    }

    if (motion->framesSinceSnap < 0xFF) {
        motion->framesSinceSnap++;
    }

    /*
     * Gap frames: advance the follow target with authority vel so we predict
     * instead of freezing. Snap frames leave target at the packet pose.
     */
    if ((motion->framesSinceSnap > 1) && (motion->framesSinceSnap <= ONLINE_REMOTE_COAST_MAX_FRAMES)) {
        motion->targetX += motion->velX;
        motion->targetZ += motion->velZ;
        if (motion->grounded == 0) {
            motion->targetY += motion->velY;
        }
    }

    errX = motion->targetX - motion->posX;
    errZ = motion->targetZ - motion->posZ;
    motion->posX += errX / ONLINE_REMOTE_SMOOTH_DIV;
    motion->posZ += errZ / ONLINE_REMOTE_SMOOTH_DIV;
    /* Grounded Y is set later from local terrain; air follows predicted target. */
    if (motion->grounded == 0) {
        motion->posY = motion->targetY;
    }
}

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
/*
 * Mode24 (pan heavy knockdown) sets SQUASHED + unk2D6 on the authority.
 * Remotes skip that update — apply the same enter visuals so flatten shows
 * on casters/observers before (and while) snaps catch up.
 */
static void onlineRemoteApplyPanFlattenVisual(RacePlayer *player) {
    s32 seat;

    seat = player->playerIndex & 3;
    player->stateFlags &= 0xFE0C1FFB;
    player->stateFlags |= 0x1202000;
    player->unk2D6 = 1;
    player->animationId = 1;
    sRemoteAppliedAnim[seat] = -1;
}

/* Keep pan flatten readable when snap scale is missing or wiped mid-hold. */
static void onlineRemoteEnsurePanFlatten(RacePlayer *player) {
    if (player->mode != 0x18) {
        return;
    }
    player->stateFlags |= RACE_PLAYER_COLLISION_SQUASHED;
    if ((player->unk2D6 <= 0) || (player->unk2D6 > 8)) {
        if (player->updateState <= 1) {
            player->unk2D6 = 1;
        } else if (player->updateState == 2) {
            player->unk2D6 = 4;
        } else {
            player->unk2D6 = 7;
        }
    }
}

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
    } else if (player->mode == 0x1A) {
        /* Parachute tumble: looping body anim + recovery sparkles (authority does this in mode26). */
        if (sRemoteAppliedAnim[seat] != anim) {
            setRaceMotionAnimation((RaceMotionState *)player, anim);
            sRemoteAppliedAnim[seat] = anim;
        }
        stepRaceMotionLoopingAnimation((RaceMotionState *)player);
        if (gFrameCounter & 1) {
            createCallbackTaskWithUserIdPreservingArgs(
                initRacePlayerRecoverySparkle, 5, 2, (u16)player->playerIndex
            );
        }
    } else {
        if (sRemoteAppliedAnim[seat] != anim) {
            setRaceMotionAnimation((RaceMotionState *)player, anim);
            sRemoteAppliedAnim[seat] = anim;
        }
        stepRaceMotionJointAnimationUntilEnd((RaceMotionState *)player);
    }

    /*
     * Board-height crouch blend — mode 0 post and mode 2 airborne-launch post
     * both sample anim 0 from groundOffset. Skip only when truly airborne.
     */
    if (onlineRemoteSurfaceLocked(player->mode, player->updateState, player->stateFlags)) {
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
    }
    resolveRacePlayerCollisionVolumes(player);
}

/*
 * Cosmetic snow spray for remotes: place board-edge markers and spawn the same
 * spark bursts locals get in updateRacePlayerPostUpdate. Does not run surface
 * contact / push (those fight networked pose).
 * Skip until RACING with the countdown gate clear — spawning during GO soft-locked.
 */
static void onlineRemoteSpawnBoardSparks(RacePlayer *player) {
    Mat3x3 rotation;
    s32 i;
    s32 originX;
    s32 originY;
    s32 originZ;
    unsigned long long random;

    if (player == NULL) {
        return;
    }
    if (gOnlineMailbox.lobby.phase != ONLINE_PHASE_RACING) {
        return;
    }
    if (gMenuFlowState & 1) {
        return;
    }
    if (player->stateFlags & 0x41000) {
        return;
    }
    if (player->soundDisabled != 0) {
        return;
    }
    if (onlineRemoteSurfaceLocked(player->mode, player->updateState, player->stateFlags) == 0) {
        return;
    }

    /* Preserve last frame's edges for streak length (local updateRacePlayer does this). */
    player->groundMarkerSources[4] = player->groundMarkerSources[0];
    player->groundMarkerSources[5] = player->groundMarkerSources[1];
    player->groundMarkerSources[6] = player->groundMarkerSources[2];
    player->groundMarkerSources[7] = player->groundMarkerSources[3];

    makeFixedRotationZXY(rotation, player->pitchAngle, player->facingAngle, player->leanAngle);
    originX = player->pos.x;
    originY = player->pos.y - player->groundOffset + 0xA000;
    originZ = player->pos.z;

    for (i = 0; i < 4; i++) {
        transformVec3iByFixedMatrix(
            rotation,
            &gRacePlayerGroundProbeOffsets[i + 9],
            &player->groundMarkerSources[i]
        );
        player->groundMarkerSources[i].x += originX;
        player->groundMarkerSources[i].y += originY;
        player->groundMarkerSources[i].z += originZ;
    }

    player->unk330 = (s8)getRaceCourseSurfaceType(player->coursePathIndex, player->pos.x, player->pos.z);
    player->groundContactMask = 0xF;

    random = randomNextMain();
    spawnRaceItemTrackSparkBurst(
        &player->groundMarkerSources[0],
        &player->groundMarkerSources[1],
        &player->groundMarkerSources[4],
        &player->groundMarkerSources[5],
        random,
        player->unk330
    );
    random = randomNextMain();
    spawnRaceItemTrackSparkBurst(
        &player->groundMarkerSources[2],
        &player->groundMarkerSources[3],
        &player->groundMarkerSources[6],
        &player->groundMarkerSources[7],
        random,
        player->unk330
    );
}

void onlineSessionReset(void) {
    s32 i;
    s32 j;

    sOnlineSessionActive = 0;
    sRemotePredictFrame = -1;
    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        sOnlineRemoteSeat[i] = 0;
        sRemoteAppliedAnim[i] = -1;
        sRemoteYawFlipHold[i] = 0;
        sRemoteCosmeticHitHold[i] = 0;
        onlineRemoteMotionResetSeat(i);
        for (j = 0; j < ONLINE_MAX_SEATS; j++) {
            sBumpCooldown[i][j] = 0;
        }
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
    s32 activeCount;
    u8 localSeat;
    u8 hostSeat;
    s32 isHost;

    if (sOnlineSessionActive == 0) {
        return;
    }

    lobby = &gOnlineMailbox.lobby;
    localSeat = lobby->localSeat;
    hostSeat = lobby->hostSeat;
    if (hostSeat >= ONLINE_MAX_SEATS) {
        hostSeat = 0;
    }
    isHost = (localSeat == hostSeat);
    gPlayerCount = 1;
    gRaceSplitscreenMode = 0;
    activeCount = 0;

    sRemotePredictFrame = -1;
    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        onlineCopyPlayerName(sOnlinePlayerNames[i], lobby->seats[i].loadout.name, ONLINE_PLAYER_NAME_LEN);
        sOnlineRemoteSeat[i] = 0;
        sRemoteCosmeticHitHold[i] = 0;
        onlineRemoteMotionResetSeat(i);
        gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_LIVE;
        gOnlineMailbox.authorityOut[i].valid = 0;
        gOnlineMailbox.cpuPathOut[i].valid = 0;
        gOnlineMailbox.cpuPathIn[i].valid = 0;
        sPublishSeq[i] = 0;
        sCpuPrevPathValid[i] = 0;

        if (lobby->seats[i].kind == ONLINE_SEAT_HUMAN) {
            gRacePlayers[i].isActive = 1;
            gRacePlayers[i].characterId = lobby->seats[i].loadout.characterId;
            gRacePlayers[i].selectedCharacterId = lobby->seats[i].loadout.characterId;
            gRacePlayers[i].characterVariant = lobby->seats[i].loadout.characterVariant;
            gRacePlayers[i].menuSelection = lobby->seats[i].loadout.characterVariant;
            gRacePlayers[i].snowboardTextureIndex = lobby->seats[i].loadout.snowboardTextureIndex;
            gRacePlayers[i].selectionUnlockState = lobby->seats[i].loadout.snowboardTextureIndex;
            activeCount++;

            if (i == (s32)localSeat) {
                gRacePlayers[i].isCpu = 0;
                gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_LIVE;
#if defined(AUTOPILOT_DEFAULT_ON) || defined(ONLINE_BATTLE_AUTOTEST)
                gRacePlayers[i].autopilotEnabled = 1;
#else
                gRacePlayers[i].autopilotEnabled = 0;
#endif
            } else {
                gRacePlayers[i].isCpu = 0;
                gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_REMOTE;
                gRacePlayers[i].autopilotEnabled = 0;
                sOnlineRemoteSeat[i] = 1;
            }
        } else if (lobby->seats[i].kind == ONLINE_SEAT_CPU) {
            /*
             * Host simulates CPUs; other clients treat them as remotes so the
             * host's authorityOut snapshots drive a single shared CPU pose.
             */
            gRacePlayers[i].isActive = 1;
            gRacePlayers[i].isCpu = 1;
            gRacePlayers[i].autopilotEnabled = 0;
            gRacePlayers[i].characterId = lobby->seats[i].loadout.characterId;
            gRacePlayers[i].selectedCharacterId = lobby->seats[i].loadout.characterId;
            gRacePlayers[i].characterVariant = lobby->seats[i].loadout.characterVariant;
            gRacePlayers[i].menuSelection = lobby->seats[i].loadout.characterVariant;
            gRacePlayers[i].snowboardTextureIndex = lobby->seats[i].loadout.snowboardTextureIndex;
            gRacePlayers[i].selectionUnlockState = lobby->seats[i].loadout.snowboardTextureIndex;
            if (isHost != 0) {
                gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_LIVE;
            } else {
                gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_REMOTE;
                sOnlineRemoteSeat[i] = 1;
            }
            activeCount++;
        } else {
            /* EMPTY: keep inactive (lobby / unused). */
            gRacePlayers[i].isCpu = 1;
            gRacePlayers[i].isActive = 0;
            gRacePlayers[i].replayInputSource = RACE_PLAYER_INPUT_LIVE;
            gRacePlayers[i].autopilotEnabled = 0;
        }
    }

    if (lobby->seats[hostSeat].loadout.lapCount != 0) {
        gRaceLapCount = lobby->seats[hostSeat].loadout.lapCount;
    }
    /* Everyone must load the host's course or remotes appear in the wrong space. */
    gRaceCourseIndex.signedValue = lobby->seats[hostSeat].loadout.courseId;
    /*
     * Ranking / update loops index 0..gRacePlayerCount-1. Server fills unused
     * seats as CPU for the race, so actives are contiguous from seat 0.
     */
    gRacePlayerCount = (activeCount > 0) ? (s8)activeCount : 1;
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
    OnlineCpuPathSnapshot *pathOut;
    s32 seat;
    s32 alongSpeed;
    s32 usePath;

    if ((sOnlineSessionActive == 0) || (player == NULL)) {
        return;
    }
    if (onlinePlayerIsRemote(player) != 0) {
        return;
    }
    seat = player->playerIndex;
    if ((seat < 0) || (seat >= ONLINE_MAX_SEATS)) {
        return;
    }
    /* Local human or host-simulated CPU — each seat has its own authorityOut slot. */
    if ((seat != (s32)gOnlineMailbox.lobby.localSeat) && (player->isCpu == 0)) {
        return;
    }

    sPublishSeq[seat]++;
    usePath = 0;
    /*
     * Path hard-place is host-CPU only. Humans publish full XYZ and remotes
     * hard-follow (no soft-correct) — path reconstruction jittered humans.
     */
    if ((player->isCpu != 0) &&
        (onlineRemoteSurfaceLocked(player->mode, player->updateState, player->stateFlags) != 0) &&
        (onlineRemoteIsReactionAirMode(player->mode) == 0)) {
        usePath = 1;
    }

    if (usePath != 0) {
        gOnlineMailbox.authorityOut[seat].valid = 0;
        pathOut = &gOnlineMailbox.cpuPathOut[seat];
        alongSpeed = 0;
        if ((sCpuPrevPathValid[seat] != 0) && (sCpuPrevPathIndex[seat] == player->coursePathIndex)) {
            alongSpeed = player->coursePathOffset - sCpuPrevPathOffset[seat];
        }
        sCpuPrevPathOffset[seat] = player->coursePathOffset;
        sCpuPrevPathIndex[seat] = player->coursePathIndex;
        sCpuPrevPathValid[seat] = 1;

        pathOut->seat = (u8)seat;
        pathOut->valid = 1;
        pathOut->itemEffectType = (u8)player->itemEffectType;
        pathOut->itemEffectCount = (u8)player->itemEffectCount;
        pathOut->actionEffectType = (u8)player->actionEffectType;
        pathOut->lapDigit = (u8)player->lapDigit;
        pathOut->sequence = sPublishSeq[seat];
        pathOut->coursePathIndex = player->coursePathIndex;
        pathOut->facingAngle = player->facingAngle;
        pathOut->coursePathOffset = player->coursePathOffset;
        pathOut->alongSpeed = alongSpeed;
        pathOut->lateralOffset =
            onlineCpuLateralFromPos(player->coursePathIndex, player->pos.x, player->pos.z);
        pathOut->mode = player->mode;
        pathOut->animationId = player->animationId;
        pathOut->updateState = player->updateState;
        pathOut->squashScale = player->unk2D6;
        pathOut->stateFlags = player->stateFlags & ~ONLINE_REMOTE_STATEFLAGS_LOCAL_MASK;
        pathOut->groundOffset = player->groundOffset;
        gOnlineMailbox.gameHeartbeat++;
        return;
    }

    sCpuPrevPathValid[seat] = 0;
    gOnlineMailbox.cpuPathOut[seat].valid = 0;
    out = &gOnlineMailbox.authorityOut[seat];
    out->seat = (u8)seat;
    out->valid = 1;
    out->itemEffectType = (u8)player->itemEffectType;
    out->itemEffectCount = (u8)player->itemEffectCount;
    out->actionEffectType = (u8)player->actionEffectType;
    out->lapDigit = (u8)player->lapDigit;
    out->sequence = sPublishSeq[seat];
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
    out->squashScale = player->unk2D6;
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

static void onlineRemoteQueueItemVisual(s32 seat, u8 itemType, u8 drops) {
    if ((seat < 0) || (seat >= ONLINE_MAX_SEATS) || (itemType == RACE_ITEM_NONE) || (drops == 0)) {
        return;
    }
    sRemotePendingItemType[seat] = itemType;
    if ((u8)(sRemotePendingItemDrops[seat] + drops) < sRemotePendingItemDrops[seat]) {
        sRemotePendingItemDrops[seat] = 0xFF;
    } else {
        sRemotePendingItemDrops[seat] += drops;
    }
}

static void onlineRemoteQueueActionVisual(s32 seat, u8 actionType) {
    if ((seat < 0) || (seat >= ONLINE_MAX_SEATS) || (actionType == RACE_ACTION_NONE)) {
        return;
    }
    sRemotePendingActionType[seat] = actionType;
}

static void onlineRemoteFlushPendingVisuals(s32 seat, RacePlayer *player) {
    u8 drops;
    u8 itemType;
    u8 actionType;

    drops = sRemotePendingItemDrops[seat];
    itemType = sRemotePendingItemType[seat];
    sRemotePendingItemDrops[seat] = 0;
    sRemotePendingItemType[seat] = RACE_ITEM_NONE;
    while (drops != 0) {
        if (sRemoteItemSpawnCooldown[seat] == 0) {
            applyOnlineRemoteItemUse(player, itemType);
            sRemoteItemSpawnCooldown[seat] = ONLINE_REMOTE_ITEM_SPAWN_COOLDOWN;
        }
        drops--;
    }

    actionType = sRemotePendingActionType[seat];
    sRemotePendingActionType[seat] = RACE_ACTION_NONE;
    if ((actionType != RACE_ACTION_NONE) && (sRemoteActionSpawnCooldown[seat] == 0)) {
        applyOnlineRemoteActionUse(player, actionType);
        sRemoteActionSpawnCooldown[seat] = ONLINE_REMOTE_ITEM_SPAWN_COOLDOWN;
    }
}

void onlineSessionApplyRemoteRacers(void) {
    RacePlayer *player;
    OnlineRacerSnapshot *snap;
    OnlineCpuPathSnapshot *pathSnap;
    OnlineRemoteMotion *motion;
    u32 localBits;
    u32 incoming;
    u32 yawBit;
    s32 i;
    s32 shouldTick;
    s32 usePath;
    s32 gotNewerPath;
    s32 gotNewerFull;
    u8 prevItemType;
    u8 prevItemCount;
    u8 prevActionType;

    if (sOnlineSessionActive == 0) {
        return;
    }

    /*
     * Called after collisions and again after post-update. Integrate prediction
     * once per game frame; later calls only re-assert the rendered pose.
     */
    shouldTick = (gFrameCounter != sRemotePredictFrame);
    if (shouldTick != 0) {
        sRemotePredictFrame = gFrameCounter;
        for (i = 0; i < ONLINE_MAX_SEATS; i++) {
            if (sRemoteItemSpawnCooldown[i] != 0) {
                sRemoteItemSpawnCooldown[i]--;
            }
            if (sRemoteActionSpawnCooldown[i] != 0) {
                sRemoteActionSpawnCooldown[i]--;
            }
        }
    }

    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        if (sOnlineRemoteSeat[i] == 0) {
            continue;
        }
        snap = &gOnlineMailbox.remoteIn[i];
        pathSnap = &gOnlineMailbox.cpuPathIn[i];
        motion = &sRemoteMotion[i];
        player = &gRacePlayers[i];
        usePath = 0;
        gotNewerPath = 0;
        gotNewerFull = 0;
        prevItemType = (u8)player->itemEffectType;
        prevItemCount = (u8)player->itemEffectCount;
        prevActionType = (u8)player->actionEffectType;

        /* Path snaps are host-CPU only; human remotes always use XYZ authority. */
        if ((player->isCpu != 0) && (pathSnap->valid != 0)) {
            if ((motion->initialized == 0) || ((s32)(pathSnap->sequence - motion->lastSequence) > 0)) {
                gotNewerPath = 1;
            }
        }
        if (snap->valid != 0) {
            if ((motion->initialized == 0) || ((s32)(snap->sequence - motion->lastSequence) > 0)) {
                gotNewerFull = 1;
            }
        }

        /* Prefer whichever packet is newer; path wins on ties for grounded CPU. */
        if ((gotNewerPath != 0) && (gotNewerFull != 0)) {
            if ((s32)(pathSnap->sequence - snap->sequence) >= 0) {
                gotNewerFull = 0;
            } else {
                gotNewerPath = 0;
            }
        }

        if (gotNewerPath != 0) {
            /*
             * Path snaps carry item/action slots for host CPUs. Detect fires
             * before accept overwrites motion's previous inventory.
             */
            if (motion->initialized != 0) {
                if ((pathSnap->itemEffectCount < motion->itemEffectCount)
                    && (motion->itemEffectType != RACE_ITEM_NONE)) {
                    onlineRemoteQueueItemVisual(
                        i,
                        motion->itemEffectType,
                        (u8)(motion->itemEffectCount - pathSnap->itemEffectCount)
                    );
                }
                if ((motion->actionEffectType != RACE_ACTION_NONE)
                    && (pathSnap->actionEffectType == RACE_ACTION_NONE)) {
                    onlineRemoteQueueActionVisual(i, motion->actionEffectType);
                }
            }
            onlineRemoteAcceptCpuPath(motion, pathSnap, i);
            usePath = 1;
        } else if (gotNewerFull != 0) {
            /* Leaving pathMode hard-places via acceptSnap (clears stale coast). */
            motion->pathMode = 0;
            onlineRemoteAcceptSnap(motion, snap);
        } else if (motion->pathMode != 0) {
            usePath = 1;
        }

        if (motion->initialized == 0) {
            /* Keep start-grid / last local pose drawable until the first snap. */
            onlineSessionPoseRemoteRacer(player);
            continue;
        }

        if (usePath != 0) {
            /* Hold last path place; framesSinceSnap only tracks packet gaps. */
            if (shouldTick != 0) {
                onlineRemoteTickCpuPath(motion);
            }
            if (pathSnap->valid != 0) {
                motion->facingAngle = onlineRemoteBlendAngle(motion->facingAngle, pathSnap->facingAngle);
            }
            player->facingAngle = motion->facingAngle;
            player->pitchAngle = motion->pitchAngle;
            player->leanAngle = motion->leanAngle;
            player->coursePathIndex = motion->coursePathIndex;
            player->coursePathOffset = motion->coursePathOffset;
            player->lapDigit = (s8)motion->lapDigit;
            player->itemEffectType = (s8)motion->itemEffectType;
            player->itemEffectCount = (s8)motion->itemEffectCount;
            player->actionEffectType = (s8)motion->actionEffectType;
            if (sRemoteCosmeticHitHold[i] == 0) {
                player->mode = motion->mode;
                player->animationId = motion->animationId;
                player->updateState = motion->updateState;
            }
            if ((pathSnap->valid != 0) && (sRemoteCosmeticHitHold[i] == 0)) {
                player->unk2D6 = pathSnap->squashScale;
            }
            player->stickX = 0;
            player->stickY = 0;

            localBits = player->stateFlags & ONLINE_REMOTE_STATEFLAGS_LOCAL_MASK;
            incoming = motion->stateFlags & ~ONLINE_REMOTE_STATEFLAGS_LOCAL_MASK;
            if (player->stateFlags & ONLINE_REMOTE_STATEFLAGS_FINISHED) {
                incoming |= ONLINE_REMOTE_STATEFLAGS_FINISHED;
            }
            /* Cosmetic pan hold: do not let a pre-hit snap clear SQUASHED. */
            if ((sRemoteCosmeticHitHold[i] != 0)
                && (player->stateFlags & RACE_PLAYER_COLLISION_SQUASHED)) {
                incoming |= RACE_PLAYER_COLLISION_SQUASHED;
            }
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
            onlineRemoteEnsurePanFlatten(player);
        } else {
            /* Extrapolate on gap frames; accept already hard-placed on snap frames. */
            if (shouldTick != 0) {
                onlineRemoteTickMotion(motion);
            }

            if (snap->valid != 0) {
                motion->facingAngle = onlineRemoteBlendAngle(motion->facingAngle, snap->facingAngle);
                motion->pitchAngle = onlineRemoteBlendAngle(motion->pitchAngle, snap->pitchAngle);
                motion->leanAngle = onlineRemoteBlendAngle(motion->leanAngle, snap->leanAngle);
                player->facingAngle = motion->facingAngle;
                player->pitchAngle = motion->pitchAngle;
                player->leanAngle = motion->leanAngle;
                player->coursePathIndex = (s16)onlineClampPathIndex(snap->coursePathIndex);
                player->coursePathOffset = snap->coursePathOffset;
                player->lapDigit = (s8)snap->lapDigit;
                /*
                 * Inventory drops are the reliable fire signal (UDP ITEM_USE /
                 * ACTION_USE can vanish while pose snaps keep arriving).
                 */
                if (motion->initialized != 0) {
                    if ((snap->itemEffectCount < prevItemCount)
                        && (prevItemType != RACE_ITEM_NONE)) {
                        onlineRemoteQueueItemVisual(
                            i,
                            prevItemType,
                            (u8)(prevItemCount - snap->itemEffectCount)
                        );
                    }
                    if ((prevActionType != RACE_ACTION_NONE)
                        && (snap->actionEffectType == RACE_ACTION_NONE)) {
                        onlineRemoteQueueActionVisual(i, prevActionType);
                    }
                }
                player->itemEffectType = (s8)snap->itemEffectType;
                player->itemEffectCount = (s8)snap->itemEffectCount;
                player->actionEffectType = (s8)snap->actionEffectType;
                if (sRemoteCosmeticHitHold[i] == 0) {
                    player->mode = snap->mode;
                    player->animationId = snap->animationId;
                    player->updateState = snap->updateState;
                    player->unk2D6 = snap->squashScale;
                }
                player->modelRotationX = snap->modelRotationX;
                player->modelRotationY = snap->modelRotationY;
                player->modelRotationZ = snap->modelRotationZ;
                player->stickX = (s8)snap->stickX;
                player->stickY = (s8)snap->stickY;

                localBits = player->stateFlags & ONLINE_REMOTE_STATEFLAGS_LOCAL_MASK;
                incoming = snap->stateFlags & ~ONLINE_REMOTE_STATEFLAGS_LOCAL_MASK;
                if (player->stateFlags & ONLINE_REMOTE_STATEFLAGS_FINISHED) {
                    incoming |= ONLINE_REMOTE_STATEFLAGS_FINISHED;
                }
                if ((sRemoteCosmeticHitHold[i] != 0)
                    && (player->stateFlags & RACE_PLAYER_COLLISION_SQUASHED)) {
                    incoming |= RACE_PLAYER_COLLISION_SQUASHED;
                }
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
                onlineRemoteEnsurePanFlatten(player);

                player->stateTimer = snap->stateTimer;
                if (shouldTick != 0) {
                    onlineRemoteUpdateGrounded(motion, snap);
                } else if (onlineRemoteSurfaceLocked(snap->mode, snap->updateState, snap->stateFlags)) {
                    motion->airFrames = 0;
                    motion->grounded = 1;
                }
            } else {
                player->facingAngle = motion->facingAngle;
                player->pitchAngle = motion->pitchAngle;
                player->leanAngle = motion->leanAngle;
            }

            /*
             * Y every apply when grounded (client mesh under visual XZ). Air
             * uses authority Y once per tick. Net posY is never used grounded.
             */
            if ((sRemoteCosmeticHitHold[i] != 0) && (onlineRemoteIsReactionAirMode(player->mode) != 0)
                && (snap->valid != 0)) {
                motion->grounded = 0;
                if (shouldTick != 0) {
                    onlineRemoteEaseAirY(motion, snap->posY, player->coursePathIndex);
                }
            } else if (motion->grounded != 0) {
                onlineRemoteStickToSurface(motion, player->coursePathIndex);
            } else if ((snap->valid != 0) && (onlineRemoteIsReactionAirMode(player->mode) != 0)) {
                if (shouldTick != 0) {
                    onlineRemoteEaseAirY(motion, snap->posY, player->coursePathIndex);
                }
            } else {
                onlineRemoteClampAboveSurface(motion, player->coursePathIndex);
            }
        }

        player->previousPosition = player->pos;
        player->pos.x = motion->posX;
        player->pos.y = motion->posY;
        player->pos.z = motion->posZ;
        player->velocity.x = motion->velX;
        player->velocity.y = motion->velY;
        player->velocity.z = motion->velZ;
        player->groundOffset = motion->groundOffset;
        player->targetGroundOffset = motion->groundOffset;
        onlineRemoteEnsurePanFlatten(player);
        onlineSessionPoseRemoteRacer(player);
        /* Spawn after pose so projectile init sees valid modelPartTransforms. */
        onlineRemoteFlushPendingVisuals(i, player);
        /* Once per frame: cosmetic board snow (skipped with remote post-update). */
        if (shouldTick != 0) {
            onlineRemoteSpawnBoardSparks(player);
        }
    }
}

s32 onlineSessionQueueRemoteHit(u8 seat, u16 flag) {
    RacePlayer *player;

    if ((sOnlineSessionActive == 0) || (seat >= ONLINE_MAX_SEATS) || (flag == 0)) {
        return 0;
    }
    if (sOnlineRemoteSeat[seat] == 0) {
        return 0;
    }
    /* Already playing caster-side reaction — wait out the hold. */
    if (sRemoteCosmeticHitHold[seat] != 0) {
        return 0;
    }
    player = &gRacePlayers[seat];
    if ((player->pendingItemHitFlags & flag) != 0) {
        return 0;
    }
    onlineSessionEnqueueEvent(
        ONLINE_EVENT_HIT,
        seat,
        (u8)(flag & 0xFF),
        (u8)((flag >> 8) & 0xFF)
    );
    player->pendingItemHitFlags |= flag;
    return 1;
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

s32 onlineSessionBodyBumpEnabled(void) {
    if (sOnlineSessionActive == 0) {
        return 0;
    }
    return ONLINE_LOBBY_BODY_BUMP(&gOnlineMailbox.lobby) ? 1 : 0;
}

static void onlineApplyBodySeparation(RacePlayer *player, s16 angleAtoB, s32 halfPush, s32 aSide) {
    s32 sine;
    s32 cosine;
    s32 pushX;
    s32 pushZ;

    /*
     * Mirror resolveRacePlayerBodyCollisions half-push onto one body.
     * aSide!=0: move like player A ( += ); else like player B ( -= ).
     * angleAtoB matches calculateFixedAngleBetweenXZPoints(A, B).
     */
    sine = fixedSine(angleAtoB);
    cosine = fixedCosine(angleAtoB);
    pushX = (s32)((s64)-sine * halfPush / 0x1000);
    pushZ = (s32)((s64)cosine * halfPush / 0x1000);
    if (aSide != 0) {
        player->pos.x += pushX;
        player->pos.z -= pushZ;
    } else {
        player->pos.x -= pushX;
        player->pos.z += pushZ;
    }
}

static void onlineTickBumpCooldowns(void) {
    s32 i;
    s32 j;

    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        for (j = 0; j < ONLINE_MAX_SEATS; j++) {
            if (sBumpCooldown[i][j] != 0) {
                sBumpCooldown[i][j]--;
            }
        }
    }
}

void onlineSessionResolveViewpointBumps(void) {
    RacePlayer *local;
    RacePlayer *remote;
    s32 i;
    s32 j;
    s32 yDiff;
    s32 yLimit;
    s32 radius;
    s32 xDiff;
    s32 zDiff;
    s32 dist;
    s32 halfPush;
    s16 angle;
    s64 closingLocal;
    s64 closingRemote;
    u8 mag;

    if ((sOnlineSessionActive == 0) || (onlineSessionBodyBumpEnabled() == 0)) {
        return;
    }
    /*
     * Countdown stacks everyone on the start grid — do not shove until GO
     * clears bit0 (same gate as offline body collision becoming "live").
     */
    if (gMenuFlowState & 1) {
        return;
    }

    onlineTickBumpCooldowns();

    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        local = &gRacePlayers[i];
        if ((local->isActive == 0) || (onlinePlayerIsRemote(local) != 0)) {
            continue;
        }
        if ((local->soundDisabled != 0) || (local->stateFlags & RACE_PLAYER_COLLISION_SQUASHED)) {
            continue;
        }

        for (j = 0; j < ONLINE_MAX_SEATS; j++) {
            if (i == j) {
                continue;
            }
            remote = &gRacePlayers[j];
            if ((remote->isActive == 0) || (onlinePlayerIsRemote(remote) == 0)) {
                continue;
            }
            if ((remote->soundDisabled != 0) || (remote->stateFlags & RACE_PLAYER_COLLISION_SQUASHED)) {
                continue;
            }

            yDiff = local->collisionBottomY - remote->collisionBottomY;
            if (yDiff < 0) {
                yDiff = -yDiff;
                yLimit = local->collisionHeight;
            } else {
                yLimit = remote->collisionHeight;
            }
            if (yDiff > yLimit) {
                continue;
            }

            radius = local->collisionRadius + remote->collisionRadius;
            xDiff = local->pos.x - remote->pos.x;
            if (xDiff < 0) {
                xDiff = -xDiff;
            }
            if (xDiff >= radius) {
                continue;
            }
            zDiff = local->pos.z - remote->pos.z;
            if (zDiff < 0) {
                zDiff = -zDiff;
            }
            if (zDiff >= radius) {
                continue;
            }
            dist = integerSquareRoot64((s64)xDiff * xDiff + (s64)zDiff * zDiff);
            if (dist >= radius) {
                continue;
            }

            /* Same half-penetration as offline resolveRacePlayerBodyCollisions. */
            halfPush = ((radius - dist) * -1) / 2;
            if (halfPush == 0) {
                continue;
            }

            /*
             * Offline always splits along A->B. Remotes are pose-locked, so
             * each machine only moves its local (A-side). Peers do the same
             * on their screen — together that mirrors the mutual half-push.
             */
            angle = calculateFixedAngleBetweenXZPoints(
                local->pos.x,
                local->pos.z,
                remote->pos.x,
                remote->pos.z
            );
            onlineApplyBodySeparation(local, angle, halfPush, 1);

            /*
             * Net event / peer B-side only on impact frames so the queue does
             * not fill every tick. Closing decides who was the bumper.
             */
            if (sBumpCooldown[i][j] != 0) {
                continue;
            }
            closingLocal = (s64)local->velocity.x * (remote->pos.x - local->pos.x)
                + (s64)local->velocity.z * (remote->pos.z - local->pos.z);
            closingRemote = (s64)remote->velocity.x * (local->pos.x - remote->pos.x)
                + (s64)remote->velocity.z * (local->pos.z - remote->pos.z);
            if (closingLocal > closingRemote) {
                mag = (u8)((-halfPush > 255) ? 255 : -halfPush);
                onlineSessionEnqueueEvent(
                    ONLINE_EVENT_BUMP,
                    (u8)j,
                    (u8)((u16)angle >> 8),
                    mag
                );
            }
            sBumpCooldown[i][j] = ONLINE_BUMP_EVENT_COOLDOWN_FRAMES;
            sBumpCooldown[j][i] = ONLINE_BUMP_EVENT_COOLDOWN_FRAMES;
        }
    }
}

void onlineSessionResolveRemoteCosmeticHits(void) {
    RacePlayer *player;
    s32 i;
    s8 savedSurface;
    s8 savedPrevSurface;

    if (sOnlineSessionActive == 0) {
        return;
    }

    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        if (sRemoteCosmeticHitHold[i] != 0) {
            sRemoteCosmeticHitHold[i]--;
        }
        if (sOnlineRemoteSeat[i] == 0) {
            continue;
        }
        player = &gRacePlayers[i];
        if (player->pendingItemHitFlags == 0) {
            continue;
        }
        /*
         * Caster-side impact FX / reaction mode only. Remotes often have a stale
         * unk330 (they skip surface update); resolveRacePlayerHitReactions would
         * then also queue terrain fall/crash and bury the puppet in the mesh
         * (seen with pan / ghost-slowdown on the shooter's screen).
         */
        savedSurface = player->unk330;
        savedPrevSurface = player->unk331;
        player->unk330 = 0;
        player->unk331 = 0;
        resolveRacePlayerHitReactions(player);
        player->unk330 = savedSurface;
        player->unk331 = savedPrevSurface;
        /*
         * Hit resolve only sets mode 0x18 — Mode24 enter (SQUASHED + unk2D6)
         * never runs on remotes. Apply that visual here so the pan flatten is
         * visible on the caster's screen immediately.
         */
        if (player->mode == 0x18) {
            onlineRemoteApplyPanFlattenVisual(player);
        }
        sRemoteCosmeticHitHold[i] = ONLINE_REMOTE_COSMETIC_HIT_HOLD_FRAMES;
    }
}

void onlineSessionDrainInboundEvents(void) {
    OnlineRaceEvent *event;
    RacePlayer *player;
    s32 i;
    u8 count;
    s16 angle;
    s32 halfPush;

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
        if (event->type == ONLINE_EVENT_BUMP) {
            /* Target seat applies shove to its local sim (human or host AI). */
            if ((onlineSessionBodyBumpEnabled() != 0) && (sOnlineRemoteSeat[event->seat] == 0)
                && !(gMenuFlowState & 1)) {
                player = &gRacePlayers[event->seat];
                if ((player->isActive != 0) && (player->soundDisabled == 0)
                    && !(player->stateFlags & RACE_PLAYER_COLLISION_SQUASHED)) {
                    /* arg angle is bumper->victim; apply B-side half-push. */
                    angle = (s16)((u16)event->arg0 << 8);
                    halfPush = -(s32)event->arg1;
                    if (halfPush != 0) {
                        onlineApplyBodySeparation(player, angle, halfPush, 0);
                    }
                }
            }
            continue;
        }
        if (sOnlineRemoteSeat[event->seat] == 0) {
            continue;
        }
        player = &gRacePlayers[event->seat];
        /*
         * Early visual from UDP when it arrives. Snap inventory drops also
         * queue the same spawn (onlineRemoteFlushPendingVisuals); credit via
         * pending clear is wrong here, so only use events as a latency hint
         * when snaps have not already reflected the fire (count still high).
         */
        if (event->type == ONLINE_EVENT_ITEM_USE) {
            /*
             * Spawn whenever UDP arrives — do not require a prior inventory
             * snap (count>0). Cooldown dedups the matching snap drop.
             */
            if (event->arg0 != RACE_ITEM_NONE) {
                onlineRemoteQueueItemVisual(event->seat, event->arg0, 1);
                onlineSessionPoseRemoteRacer(player);
                onlineRemoteFlushPendingVisuals(event->seat, player);
                if ((u8)player->itemEffectCount > 0) {
                    player->itemEffectCount--;
                    if (player->itemEffectCount == 0) {
                        player->itemEffectType = RACE_ITEM_NONE;
                    }
                }
            }
        } else if (event->type == ONLINE_EVENT_ACTION_USE) {
            if (event->arg0 != RACE_ACTION_NONE) {
                onlineRemoteQueueActionVisual(event->seat, event->arg0);
                onlineSessionPoseRemoteRacer(player);
                onlineRemoteFlushPendingVisuals(event->seat, player);
                player->actionEffectType = RACE_ACTION_NONE;
            }
        }
    }
    gOnlineMailbox.eventInCount = 0;
}
