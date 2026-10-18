#include "game/online/online_mailbox.h"

OnlineMailbox gOnlineMailbox;

void onlineMailboxInit(void) {
    s32 i;

    gOnlineMailbox.magic = ONLINE_MAILBOX_MAGIC;
    gOnlineMailbox.version = ONLINE_MAILBOX_VERSION;
    gOnlineMailbox.flags = 0;
    gOnlineMailbox.gameToBridgeCmd = ONLINE_CMD_NONE;
    gOnlineMailbox.bridgeToGameCmd = ONLINE_CMD_NONE;
    gOnlineMailbox.errorCode = ONLINE_ERR_NONE;
    gOnlineMailbox.lobby.phase = ONLINE_PHASE_IDLE;
    gOnlineMailbox.lobby.localSeat = 0xFF;
    gOnlineMailbox.lobby.hostSeat = 0;
    gOnlineMailbox.lobby.humanCount = 0;
    gOnlineMailbox.lobby.raceSeed = 0;
    gOnlineMailbox.lobby.startFrame = 0;
    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        gOnlineMailbox.lobby.seats[i].kind = ONLINE_SEAT_EMPTY;
        gOnlineMailbox.lobby.seats[i].ready = 0;
        gOnlineMailbox.authorityOut[i].valid = 0;
        gOnlineMailbox.authorityOut[i].seat = (u8)i;
        gOnlineMailbox.remoteIn[i].valid = 0;
        gOnlineMailbox.remoteIn[i].seat = (u8)i;
        gOnlineMailbox.cpuPathOut[i].valid = 0;
        gOnlineMailbox.cpuPathOut[i].seat = (u8)i;
        gOnlineMailbox.cpuPathIn[i].valid = 0;
        gOnlineMailbox.cpuPathIn[i].seat = (u8)i;
    }
    gOnlineMailbox.eventOutCount = 0;
    gOnlineMailbox.eventInCount = 0;
    gOnlineMailbox.bridgeHeartbeat = 0;
    gOnlineMailbox.gameHeartbeat = 0;
}

void onlineMailboxRequest(u8 cmd) {
    gOnlineMailbox.gameToBridgeCmd = cmd;
}

u8 onlineMailboxPollBridgeCmd(void) {
    u8 cmd;

    cmd = gOnlineMailbox.bridgeToGameCmd;
    if (cmd != ONLINE_CMD_NONE) {
        gOnlineMailbox.bridgeToGameCmd = ONLINE_CMD_NONE;
    }
    return cmd;
}

s32 onlineMailboxIsOnlineSession(void) {
    u8 phase;

    if (gOnlineMailbox.magic != ONLINE_MAILBOX_MAGIC) {
        return 0;
    }
    phase = gOnlineMailbox.lobby.phase;
    return (phase == ONLINE_PHASE_LOBBY) || (phase == ONLINE_PHASE_COUNTDOWN) || (phase == ONLINE_PHASE_RACING) ||
           (phase == ONLINE_PHASE_CONNECTING);
}
