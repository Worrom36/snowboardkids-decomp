#ifndef ONLINE_PROTOCOL_H
#define ONLINE_PROTOCOL_H

#include "common.h"

#define ONLINE_PROTOCOL_VERSION 2
#define ONLINE_MAX_SEATS        4
#define ONLINE_DEFAULT_PORT     6464

/* Wire / mailbox command ids (shared with Python tools). */
typedef enum OnlineCmd {
    ONLINE_CMD_NONE = 0,
    ONLINE_CMD_CONNECT = 1,
    ONLINE_CMD_DISCONNECT = 2,
    ONLINE_CMD_READY = 3,
    ONLINE_CMD_UNREADY = 4,
    ONLINE_CMD_REQUEST_START = 5,
    ONLINE_CMD_SET_LOADOUT = 6,
    ONLINE_CMD_JOINED = 7,
    ONLINE_CMD_LOBBY_SNAPSHOT = 8,
    ONLINE_CMD_START = 9,
    ONLINE_CMD_ERROR = 10,
    ONLINE_CMD_RACER_SNAPSHOT = 11,
    ONLINE_CMD_EVENT = 12,
    ONLINE_CMD_HEARTBEAT = 13,
    ONLINE_CMD_CPU_PATH_SNAPSHOT = 14,
    /* Client finished race-scene load; server replies RACE_GO when all humans are ready. */
    ONLINE_CMD_RACE_LOADED = 15,
    ONLINE_CMD_RACE_GO = 16
} OnlineCmd;

typedef enum OnlinePhase {
    ONLINE_PHASE_IDLE = 0,
    ONLINE_PHASE_CONNECTING = 1,
    ONLINE_PHASE_LOBBY = 2,
    ONLINE_PHASE_COUNTDOWN = 3,
    ONLINE_PHASE_RACING = 4,
    ONLINE_PHASE_ERROR = 5
} OnlinePhase;

typedef enum OnlineSeatKind {
    ONLINE_SEAT_EMPTY = 0,
    ONLINE_SEAT_HUMAN = 1,
    ONLINE_SEAT_CPU = 2
} OnlineSeatKind;

typedef enum OnlineErrorCode {
    ONLINE_ERR_NONE = 0,
    ONLINE_ERR_FULL = 1,
    ONLINE_ERR_REJECTED = 2,
    ONLINE_ERR_TIMEOUT = 3,
    ONLINE_ERR_PROTOCOL = 4,
    ONLINE_ERR_DISCONNECTED = 5
} OnlineErrorCode;

typedef enum OnlineRaceEventType {
    ONLINE_EVENT_NONE = 0,
    ONLINE_EVENT_ITEM_USE = 1,
    ONLINE_EVENT_ACTION_USE = 2,
    ONLINE_EVENT_FINISH = 3,
    ONLINE_EVENT_DISCONNECT = 4,
    /* seat=victim, arg0=flags lo, arg1=flags hi — victim applies locally. */
    ONLINE_EVENT_HIT = 5
} OnlineRaceEventType;

typedef struct OnlineLoadout {
    /* 0x00 */ u8 courseId;
    /* 0x01 */ u8 characterId;
    /* 0x02 */ u8 characterVariant;
    /* 0x03 */ u8 snowboardTextureIndex;
    /* 0x04 */ u8 lapCount;
    /* 0x05 */ u8 pad05[3];
} OnlineLoadout;

typedef struct OnlineLobbySeat {
    /* 0x00 */ u8 kind;   /* OnlineSeatKind */
    /* 0x01 */ u8 ready;
    /* 0x02 */ u8 pad02[2];
    /* 0x04 */ OnlineLoadout loadout;
} OnlineLobbySeat;

typedef struct OnlineLobbySnapshot {
    /* 0x00 */ u8 phase; /* OnlinePhase */
    /* 0x01 */ u8 localSeat;
    /* 0x02 */ u8 hostSeat;
    /* 0x03 */ u8 humanCount;
    /* 0x04 */ u32 raceSeed;
    /* 0x08 */ u32 startFrame;
    /* 0x0C */ OnlineLobbySeat seats[ONLINE_MAX_SEATS];
} OnlineLobbySnapshot;

/* Compact racer pose published each frame while racing. */
typedef struct OnlineRacerSnapshot {
    /* 0x00 */ u8 seat;
    /* 0x01 */ u8 valid;
    /* 0x02 */ u8 itemEffectType;
    /* 0x03 */ u8 itemEffectCount;
    /* 0x04 */ u8 actionEffectType;
    /* 0x05 */ u8 lapDigit; /* Remotes skip updateRacePlayer; must sync for HUD rank. */
    /* 0x06 */ u8 pad06[2];
    /* 0x08 */ u32 sequence;
    /* 0x0C */ s32 posX;
    /* 0x10 */ s32 posY;
    /* 0x14 */ s32 posZ;
    /* 0x18 */ s32 velX;
    /* 0x1C */ s32 velY;
    /* 0x20 */ s32 velZ;
    /* 0x24 */ s16 facingAngle;
    /* 0x26 */ s16 pitchAngle;
    /* 0x28 */ s16 leanAngle;
    /* 0x2A */ s16 coursePathIndex;
    /* 0x2C */ s32 coursePathOffset;
    /* 0x30 */ s16 mode;
    /* 0x32 */ s16 stickX;
    /* 0x34 */ s16 stickY;
    /* 0x36 */ s16 animationId;
    /* 0x38 */ u32 inputFlags;
    /* 0x3C */ s16 updateState;
    /* 0x3E */ s16 modelRotationX;
    /* 0x40 */ s16 modelRotationY;
    /* 0x42 */ s16 modelRotationZ;
    /* 0x44 */ u32 stateFlags;
    /* 0x48 */ s32 stateTimer;
    /* 0x4C */ s32 groundOffset;
} OnlineRacerSnapshot;

/*
 * Compact grounded AI-path sync (CPU / autopilot). Remotes place from path
 * params; lateralOffset is the real perpendicular component (not lane tables).
 * Size 0x2C; full OnlineRacerSnapshot used for air/hit.
 */
typedef struct OnlineCpuPathSnapshot {
    /* 0x00 */ u8 seat;
    /* 0x01 */ u8 valid;
    /* 0x02 */ u8 itemEffectType;
    /* 0x03 */ u8 itemEffectCount;
    /* 0x04 */ u8 actionEffectType;
    /* 0x05 */ u8 lapDigit;
    /* 0x06 */ u8 pad06[2];
    /* 0x08 */ u32 sequence;
    /* 0x0C */ s16 coursePathIndex;
    /* 0x0E */ s16 facingAngle;
    /* 0x10 */ s32 coursePathOffset;
    /* 0x14 */ s32 alongSpeed;
    /* 0x18 */ s32 lateralOffset;
    /* 0x1C */ s16 mode;
    /* 0x1E */ s16 animationId;
    /* 0x20 */ s16 updateState;
    /* 0x22 */ s16 pad22;
    /* 0x24 */ u32 stateFlags;
    /* 0x28 */ s32 groundOffset;
} OnlineCpuPathSnapshot;

typedef struct OnlineRaceEvent {
    /* 0x00 */ u8 type; /* OnlineRaceEventType */
    /* 0x01 */ u8 seat;
    /* 0x02 */ u8 arg0;
    /* 0x03 */ u8 arg1;
    /* 0x04 */ u32 sequence;
} OnlineRaceEvent;

#endif
