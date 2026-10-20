#ifndef ONLINE_MAILBOX_H
#define ONLINE_MAILBOX_H

#include "game/online/online_protocol.h"

/*
 * Shared RDRAM mailbox between the ROM and the emulator bridge / net client.
 * Layout must stay packed and stable; Python tools mirror these offsets.
 */
#define ONLINE_MAILBOX_MAGIC    0x4F4E4C42 /* 'ONLB' */
#define ONLINE_MAILBOX_VERSION  ONLINE_PROTOCOL_VERSION
#define ONLINE_EVENT_QUEUE_SIZE 8

typedef struct OnlineMailbox {
    /* 0x000 */ u32 magic;
    /* 0x004 */ u16 version;
    /* 0x006 */ u16 flags;
    /* 0x008 */ volatile u8 gameToBridgeCmd; /* OnlineCmd written by game */
    /* 0x009 */ volatile u8 bridgeToGameCmd; /* OnlineCmd written by bridge */
    /* 0x00A */ u8 errorCode;                /* OnlineErrorCode */
    /* 0x00B */ u8 pad0B;
    /* 0x00C */ OnlineLobbySnapshot lobby;
    /* 0x068 */ OnlineLoadout pendingLoadout;
    /* 0x078 */ OnlineRacerSnapshot authorityOut[ONLINE_MAX_SEATS]; /* local + host-sim CPUs */
    /* 0x1B8 */ OnlineRacerSnapshot remoteIn[ONLINE_MAX_SEATS];
    /* 0x2F8 */ OnlineCpuPathSnapshot cpuPathOut[ONLINE_MAX_SEATS];
    /* 0x3A8 */ OnlineCpuPathSnapshot cpuPathIn[ONLINE_MAX_SEATS];
    /* 0x458 */ u8 eventOutCount;
    /* 0x459 */ u8 eventInCount;
    /* 0x45A */ u8 pad45A[2];
    /* 0x45C */ OnlineRaceEvent eventOut[ONLINE_EVENT_QUEUE_SIZE];
    /* 0x49C */ OnlineRaceEvent eventIn[ONLINE_EVENT_QUEUE_SIZE];
    /* 0x4DC */ u32 bridgeHeartbeat;
    /* 0x4E0 */ u32 gameHeartbeat;
} OnlineMailbox;

typedef char OnlineMailboxLoadoutOffsetCheck[(sizeof(OnlineLoadout) == 0x10) ? 1 : -1];
typedef char OnlineMailboxLobbyOffsetCheck[(sizeof(OnlineLobbySnapshot) == 0x5C) ? 1 : -1];
typedef char OnlineMailboxRacerOffsetCheck[(sizeof(OnlineRacerSnapshot) == 0x50) ? 1 : -1];
typedef char OnlineMailboxCpuPathOffsetCheck[(sizeof(OnlineCpuPathSnapshot) == 0x2C) ? 1 : -1];
typedef char OnlineMailboxSizeCheck[(sizeof(OnlineMailbox) == 0x4E4) ? 1 : -1];

extern OnlineMailbox gOnlineMailbox;

void onlineMailboxInit(void);
void onlineMailboxRequest(u8 cmd);
u8 onlineMailboxPollBridgeCmd(void);
s32 onlineMailboxIsOnlineSession(void);

#endif
