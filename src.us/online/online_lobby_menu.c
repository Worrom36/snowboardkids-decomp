#include "game/online/online_lobby_menu.h"
#include "assets.h"
#include "common.h"
#include <PR/os_reg.h>
#include "game/audio/sound_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/controller_input.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/render_callback.h"
#include "game/engine/viewport_manager.h"
#include "game/menu/race_setup/race_setup_ui.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/splitscreen_select/race_splitscreen_select_menu.h"
#include "game/online/online_mailbox.h"
#include "game/online/online_session.h"
#include "game/race/flow/race_flow.h"
#include "game/race/player/race_player_input.h"
#include "game/race/race_state.h"

extern s32 gMenuFlowState;
extern u8 gMenuExitSelection;
extern s16 gMenuFadeAlpha;
extern u8 gPendingFramebufferSwapCount;
extern u8 gFramebufferSwapHold;

static const char sOnlineConnectingText[] = "CONNECTING...";
static const char sOnlineFailedText[] = "CONNECT FAILED";
static const char sOnlineLobbyTitle[] = "ONLINE LOBBY";
static const char sOnlineReadyText[] = "READY";
static const char sOnlineWaitText[] = "WAITING";
static const char sOnlineEmptyText[] = "EMPTY";
static const char sOnlineCpuText[] = "CPU";
static const char sOnlineHostStart[] = "PRESS START";
static const char sOnlinePressAReady[] = "PRESS START READY";
static const char sOnlineWaitHost[] = "WAIT FOR HOST";
static const char sOnlineStartingText[] = "STARTING...";
static const char sOnlineSeatLabels[ONLINE_MAX_SEATS][3] = { "P1", "P2", "P3", "P4" };

static u8 sOnlineLobbyLocalReady;
static u8 sOnlineConnectFrames;
static u8 sOnlineFadeSwapWait;
static u8 sOnlineRaceStartArmed;
/* Task that runs handleOnlineBattleConnectFlow while lobby task 4 is up. */
static u8 sOnlineParentTaskId;

static void fadeOutOnlineLobbyMenu(void);
static void beginOnlineLobbyRaceStart(void);

static void beginOnlineLobbyRaceStart(void) {
    OnlineLobbySnapshot *lobby;

    if (sOnlineRaceStartArmed != 0) {
        return;
    }
    sOnlineRaceStartArmed = 1;
    sOnlineFadeSwapWait = 0;
    lobby = onlineSessionLobby();
    lobby->phase = ONLINE_PHASE_COUNTDOWN;
    gMenuFlowState = 3;
    gMenuExitSelection = 0;
    gCurrentGameTask->callbackData0 = 0;
    setCurrentGameTaskCallback(fadeOutOnlineLobbyMenu, 0);
}

static void drawOnlineLobbyHud(void *unused) {
    OnlineLobbySnapshot *lobby;
    s32 i;
    s16 y;
    const char *status;
    u8 localSeat;
    u8 allReady;
    u8 humans;

    (void)unused;
    lobby = onlineSessionLobby();

    /* Screen space is centered at (0,0); ASCII must use the foreground list. */
    drawMenuFillRectangle(-0x78, -0x58, 0xF0, 0xB0, 0, 0, 0x28);
    drawMenuAsciiTextDefaultScale(-0x38, -0x48, sOnlineLobbyTitle, 0);

    if (lobby->phase == ONLINE_PHASE_CONNECTING) {
        drawMenuAsciiTextDefaultScale(-0x38, -0x28, sOnlineConnectingText, 0);
        return;
    }
    if (lobby->phase == ONLINE_PHASE_ERROR) {
        drawMenuAsciiTextDefaultScale(-0x38, -0x28, sOnlineFailedText, 0);
        return;
    }

    localSeat = lobby->localSeat;
    allReady = 1;
    humans = 0;
    y = -0x18;
    for (i = 0; i < ONLINE_MAX_SEATS; i++) {
        drawMenuAsciiTextDefaultScale(-0x50, y, sOnlineSeatLabels[i], 0);
        if (lobby->seats[i].kind == ONLINE_SEAT_HUMAN) {
            humans++;
            if (lobby->seats[i].ready != 0) {
                status = sOnlineReadyText;
            } else {
                status = sOnlineWaitText;
                allReady = 0;
            }
        } else if (lobby->seats[i].kind == ONLINE_SEAT_CPU) {
            status = sOnlineCpuText;
        } else {
            status = sOnlineEmptyText;
        }
        drawMenuAsciiTextDefaultScale(-0x20, y, status, (i == (s32)localSeat) ? 1 : 0);
        y += 0x12;
    }

    if (lobby->phase == ONLINE_PHASE_COUNTDOWN) {
        drawMenuAsciiTextDefaultScale(-0x38, 0x40, sOnlineStartingText, 0);
    } else if (localSeat == lobby->hostSeat) {
        if ((allReady != 0) && (humans >= 1)) {
            drawMenuAsciiTextDefaultScale(-0x38, 0x40, sOnlineHostStart, 0);
        } else {
            drawMenuAsciiTextDefaultScale(-0x38, 0x40, sOnlinePressAReady, 0);
        }
    } else if (lobby->seats[localSeat].ready == 0) {
        drawMenuAsciiTextDefaultScale(-0x38, 0x40, sOnlinePressAReady, 0);
    } else {
        drawMenuAsciiTextDefaultScale(-0x38, 0x40, sOnlineWaitHost, 0);
    }
}

void openOnlineBattleConnectFlow(void) {
    sOnlineParentTaskId = 2;
    setCurrentGameTaskCallback(&handleOnlineBattleConnectFlow, 0);
    createGameTask(4, &initOnlineLobbyMenu, 0x64);
    suspendGameTask(sOnlineParentTaskId);
}

#ifdef ONLINE_BATTLE_AUTOTEST
/*
 * Fast boot for local N-P testing: skip title/menus and open the online lobby
 * with course 0 / random character / board 0 / 3 laps.
 * Build with: make BUILD_MODE=shiftable ONLINE_BATTLE=1 ONLINE_BATTLE_AUTOTEST=1
 */
void bootOnlineBattleAutotest(void) {
    u8 characterId;

    /* 6 roster ids (0..5). osGetCount varies across staggered Mupen boots. */
    characterId = (u8)(osGetCount() % 6);
    gRaceCourseIndex.signedValue = 0;
    gRaceSplitscreenMode = 0;
    gPlayerCount = 1;
    gRacePlayers[0].selectedCharacterId = characterId;
    gRacePlayers[0].characterId = characterId;
    gRacePlayers[0].menuSelection = 0;
    gRacePlayers[0].characterVariant = 0;
    gRacePlayers[0].selectionUnlockState = 0;
    gRacePlayers[0].snowboardTextureIndex = 0;
    gMenuFlowState = 0;
    gMenuExitSelection = 0;
    gFramebufferSwapHold = 0;
    gMenuFadeAlpha = 0xFF;
    /* Startup runs as task 0 (not task 2 like GAME MENU). */
    sOnlineParentTaskId = 0;
    setCurrentGameTaskCallback(&handleOnlineBattleConnectFlow, 0);
    createGameTask(4, &initOnlineLobbyMenu, 0x64);
    suspendGameTask(sOnlineParentTaskId);
}
#endif

void initOnlineLobbyMenu(void) {
    requestMusicSequenceBank(1);
    resetAllViewports();
    configureViewport(0, 0xA0, 0x78, 0x120, 0xD0, 0x140, 0xF0, 1.333333373f);
    gFramebufferRenderInterval.value = 0;
    gFramebufferSwapHold = 0;

    /* Backdrop + UI chrome (ASCII font on handle 6 survives menu release). */
    LOAD_ASSET(_593D10, 0x22);
    LOAD_ASSET(_59DFE0, 0x26);
    LOAD_ASSET(SHARED_UI_SPRITES, 0x1F);

    initCallbackTaskScheduler(0);
    createCallbackTask((CallbackTaskCallback)initScrollingMenuBackdropActor, 0, 0x5E);

    sOnlineLobbyLocalReady = 0;
    sOnlineConnectFrames = 0;
    sOnlineFadeSwapWait = 0;
    sOnlineRaceStartArmed = 0;
    gMenuExitSelection = 0;
    gMenuFlowState = 0;
    /* Fade in from the black frame left by GAME MENU teardown. */
    gCurrentGameTask->callbackData0 = 0xFF;
    gMenuFadeAlpha = 0xFF;

    onlineSessionBeginConnect();
    onlineSessionSetLoadout(
        (u8)gRaceCourseIndex.signedValue,
        gRacePlayers[0].selectedCharacterId,
        gRacePlayers[0].menuSelection,
        gRacePlayers[0].selectionUnlockState,
        3
    );
    setCurrentGameTaskCallback(updateOnlineLobbyMenu, 0);
    updateCallbackTasks();
}

void updateOnlineLobbyMenu(void) {
    OnlineLobbySnapshot *lobby;
    u8 cmd;
    s32 pressed;

    /* Render lists are cleared every frame; re-queue HUD like other menus. */
    addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawOnlineLobbyHud, NULL);

    /*
     * Always poll bridge cmds — START must not be lost while the lobby fade-in
     * still has callbackData0 != 0 (P2 often receives START in that window).
     */
    cmd = onlineMailboxPollBridgeCmd();
    lobby = onlineSessionLobby();
    gOnlineMailbox.gameHeartbeat++;

    if (cmd == ONLINE_CMD_ERROR) {
        lobby->phase = ONLINE_PHASE_ERROR;
    } else if ((cmd == ONLINE_CMD_JOINED) || (cmd == ONLINE_CMD_LOBBY_SNAPSHOT)) {
        if (lobby->phase == ONLINE_PHASE_CONNECTING) {
            lobby->phase = ONLINE_PHASE_LOBBY;
        }
    } else if (cmd == ONLINE_CMD_START) {
        beginOnlineLobbyRaceStart();
        updateCallbackTasks();
        return;
    }

    /* Lobby snapshot may advance phase to COUNTDOWN before the START cmd is seen. */
    if ((sOnlineRaceStartArmed == 0) && (lobby->phase == ONLINE_PHASE_COUNTDOWN)) {
        beginOnlineLobbyRaceStart();
        updateCallbackTasks();
        return;
    }

    /* Fade the black overlay out before accepting lobby input. */
    if (gCurrentGameTask->callbackData0 != 0) {
        gCurrentGameTask->callbackData0 = stepMenuFadeAlpha((s16)gCurrentGameTask->callbackData0, 0x24, 0);
        gMenuFadeAlpha = (s16)gCurrentGameTask->callbackData0;
        updateCallbackTasks();
        return;
    }

    if (lobby->phase == ONLINE_PHASE_CONNECTING) {
        sOnlineConnectFrames++;
        /* Bridge-less solo test: after a short wait, self-host a 1P lobby. */
        if ((sOnlineConnectFrames >= 90) && (gOnlineMailbox.bridgeHeartbeat == 0)) {
            lobby->phase = ONLINE_PHASE_LOBBY;
            lobby->localSeat = 0;
            lobby->hostSeat = 0;
            lobby->humanCount = 1;
            lobby->seats[0].kind = ONLINE_SEAT_HUMAN;
            lobby->seats[0].ready = 0;
            lobby->seats[0].loadout = gOnlineMailbox.pendingLoadout;
            lobby->seats[1].kind = ONLINE_SEAT_CPU;
            lobby->seats[2].kind = ONLINE_SEAT_CPU;
            lobby->seats[3].kind = ONLINE_SEAT_CPU;
        }
    }

    pressed = gPlayerInputPressed[0];
    if (lobby->phase == ONLINE_PHASE_LOBBY) {
        /*
         * Ready: A (0x8000) or Start/Enter (0x1000). Mupen keyboard maps Enter to
         * Start, not A — non-hosts must be able to ready with Enter.
         * Do not toggle: a second press must not send UNREADY over a laggy bridge.
         */
        if ((sOnlineLobbyLocalReady == 0) && (pressed & (0x8000 | 0x1000))) {
            sOnlineLobbyLocalReady = 1;
            onlineSessionSetReady(1);
            if (lobby->localSeat < ONLINE_MAX_SEATS) {
                lobby->seats[lobby->localSeat].ready = 1;
            }
            enqueueSoundEffect(0x18, 0x32);
        }
        if ((pressed & 0x1000) && (lobby->localSeat == lobby->hostSeat)) {
            if (gOnlineMailbox.bridgeHeartbeat == 0) {
                beginOnlineLobbyRaceStart();
                updateCallbackTasks();
                return;
            }
            onlineSessionRequestStart();
            enqueueSoundEffect(0x18, 0x32);
        }
    }

    if (pressed & 0x4000) {
        if (sOnlineLobbyLocalReady != 0) {
            sOnlineLobbyLocalReady = 0;
            onlineSessionSetReady(0);
            if (lobby->localSeat < ONLINE_MAX_SEATS) {
                lobby->seats[lobby->localSeat].ready = 0;
            }
        } else {
            onlineSessionDisconnect();
            gMenuExitSelection = 1;
            gMenuFlowState = 1;
            setCurrentGameTaskCallback(fadeOutOnlineLobbyMenu, 0);
            updateCallbackTasks();
            return;
        }
    }

    updateCallbackTasks();
}

static void fadeOutOnlineLobbyMenu(void) {
    if (gCurrentGameTask->callbackData0 != 0xFF) {
        gCurrentGameTask->callbackData0 = stepMenuFadeAlpha((s16)gCurrentGameTask->callbackData0, 0x24, 1);
        gMenuFadeAlpha = (s16)gCurrentGameTask->callbackData0;
        if (gCurrentGameTask->callbackData0 == 0xFF) {
            gFramebufferSwapHold = 1;
            sOnlineFadeSwapWait = 0;
        } else {
            updateCallbackTasks();
        }
    } else if ((gPendingFramebufferSwapCount == 2) || (sOnlineFadeSwapWait >= 3)) {
        /* Failsafe: do not soft-lock on black if swap-count never hits 2 (seen on P2). */
        releaseMenuAssetHandles();
        gFramebufferSwapHold = 0;
        gFramebufferRenderInterval.value = 0;
        sOnlineFadeSwapWait = 0;
        if (gMenuExitSelection == 1) {
            gMenuFlowState = 1;
        }
        /* Autotest parent is task 0; GAME MENU path uses task 2. */
        resumeGameTask(sOnlineParentTaskId);
        removeGameTask(4);
    } else {
        sOnlineFadeSwapWait++;
        updateCallbackTasks();
    }
}

void handleOnlineLobbyMenuSelection(void) {
    updateCallbackTasks();
}

void handleOnlineBattleConnectFlow(void) {
    OnlineLobbySnapshot *lobby;
    u8 seat;

    if (gMenuFlowState == 1) {
        gMenuFlowState = 0;
        gRaceSplitscreenMode = GAME_MENU_CURSOR_ONLINE_REOPEN;
        setCurrentGameTaskCallback(&openRaceSplitscreenSelectFlow, 0);
        return;
    }
    if (gMenuFlowState == 3) {
        gMenuFlowState = 0;
        gRaceSplitscreenMode = 0;
        gPlayerCount = 1;
        lobby = onlineSessionLobby();
        seat = lobby->localSeat;
        if (seat >= ONLINE_MAX_SEATS) {
            seat = 0;
        }
        /* Course/laps come from the host so both clients load the same track. */
        gRaceCourseIndex.signedValue = lobby->seats[lobby->hostSeat < ONLINE_MAX_SEATS ? lobby->hostSeat : 0]
                                           .loadout.courseId;
        gRacePlayers[seat].selectedCharacterId = lobby->seats[seat].loadout.characterId;
        gRacePlayers[seat].menuSelection = lobby->seats[seat].loadout.characterVariant;
        gRacePlayers[seat].selectionUnlockState = lobby->seats[seat].loadout.snowboardTextureIndex;
        /* Keep slot 0 filled for init paths that still read player 0 before configure. */
        gRacePlayers[0].selectedCharacterId = lobby->seats[seat].loadout.characterId;
        gRacePlayers[0].menuSelection = lobby->seats[seat].loadout.characterVariant;
        gRacePlayers[0].selectionUnlockState = lobby->seats[seat].loadout.snowboardTextureIndex;
        setCurrentGameTaskCallback(&initRaceSceneFlow, 0);
        return;
    }
}
