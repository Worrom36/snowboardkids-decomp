#include "game/race/items/race_items.h"
#include "common.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/audio/sound_manager.h"
#include "game/race/player/race_player_input.h"
#include "game/math/spatial_math.h"
#include "game/race/items/race_item_projectiles.h"
#include "game/race/items/race_item_effects.h"
#include "game/race/player/race_player_pickup_effects.h"
#include "game/race/effects/snowboard_trail_effects.h"
#include "game/race/ui/race_ui_effects.h"
#ifdef ONLINE_BATTLE
#include "game/online/online_session.h"
#endif

typedef void (*EffectCallback)(void *);

#ifdef ONLINE_BATTLE
static s32 spawnRacePlayerItemProjectile(RacePlayer *player, s32 itemType) {
    void *task;

    task = NULL;
    if (itemType == RACE_ITEM_SLAPSTICK) {
        task = createCallbackTaskWithUserIdPreservingArgs(
            initWideHomingItemProjectile,
            (u16)(player->playerIndex + 1),
            0x3C,
            player->playerIndex
        );
    } else if (itemType == RACE_ITEM_PARACHUTE) {
        task = createCallbackTaskWithUserIdPreservingArgs(
            initLongRangeHomingItemProjectile,
            (u16)(player->playerIndex + 1),
            0x3C,
            player->playerIndex
        );
    } else if (itemType == RACE_ITEM_FREEZE_SHOT) {
        task = createCallbackTaskWithUserIdPreservingArgs(
            initCloseRangeHomingItemProjectile,
            (u16)(player->playerIndex + 1),
            0x3C,
            player->playerIndex
        );
    } else if (itemType == RACE_ITEM_SNOWMAN) {
        task = createCallbackTaskWithUserIdPreservingArgs(
            initBouncingItemProjectile,
            (u16)(player->playerIndex + 1),
            0x3C,
            player->playerIndex
        );
    } else if (itemType == RACE_ITEM_BOMB) {
        task = createCallbackTaskWithUserIdPreservingArgs(
            initAreaBlastItemProjectile,
            (u16)(player->playerIndex + 1),
            0x3C,
            player->playerIndex
        );
    }

    if (task == NULL) {
        return 0;
    }
    createCallbackTaskWithUserIdPreservingArgs(initRacePlayerShockEffect, 5, 4, player->playerIndex);
    return 1;
}

static void applyRacePlayerActionEffect(RacePlayer *player, s32 type) {
    if (type == RACE_ACTION_SPEED_FAN) {
        if (player->trailEffectTimer == 0) {
            startSpeedFanEffect(player);
        }
    } else if (type == RACE_ACTION_GHOST) {
        spawnRacePlayerSparkleEffect(player->playerIndex);
    } else if (type == RACE_ACTION_PAN) {
        spawnGhostSlowdownTargets(player->playerIndex);
    } else if (type == RACE_ACTION_ROCK) {
        createCallbackTaskWithUserIdPreservingArgs(
            initFallingActionProjectile,
            0,
            0x3C,
            (u16)player->playerIndex
        );
    } else if (type == RACE_ACTION_RAT_FACE) {
        createCallbackTaskWithUserIdPreservingArgs(
            initForwardActionProjectileEffect,
            0,
            3,
            (u16)player->playerIndex
        );
    } else if (type == RACE_ACTION_INVISIBLE) {
        if (player->actionSoundTimer == 0) {
            player->actionSoundTimer = 0xB4;
            enqueuePositionalSoundEffect(0x10, &player->pos, 0x7F, 0x32);
        }
    }
}

void applyOnlineRemoteItemUse(RacePlayer *player, s32 itemType) {
    RaceItemProjectileActor *actor;
    CallbackTaskCallback initFn;

    if ((player == NULL) || (itemType == RACE_ITEM_NONE)) {
        return;
    }

    initFn = NULL;
    if (itemType == RACE_ITEM_SLAPSTICK) {
        initFn = (CallbackTaskCallback)initWideHomingItemProjectile;
    } else if (itemType == RACE_ITEM_PARACHUTE) {
        initFn = (CallbackTaskCallback)initLongRangeHomingItemProjectile;
    } else if (itemType == RACE_ITEM_FREEZE_SHOT) {
        initFn = (CallbackTaskCallback)initCloseRangeHomingItemProjectile;
    } else if (itemType == RACE_ITEM_SNOWMAN) {
        initFn = (CallbackTaskCallback)initBouncingItemProjectile;
    } else if (itemType == RACE_ITEM_BOMB) {
        initFn = (CallbackTaskCallback)initAreaBlastItemProjectile;
    }
    if (initFn == NULL) {
        return;
    }

    /*
     * Type 0 uses the large shared pool (per-seat type1–4 can be empty on
     * viewers). Run init immediately — a mid-frame create can be skipped by
     * updateRemainingCallbackTasks if it inserts behind the walk cursor.
     */
    actor = createCallbackTaskWithUserIdPreservingArgs(
        (void (*)())initFn,
        0,
        0x3C,
        player->playerIndex
    );
    if (actor == NULL) {
        return;
    }
    initFn(actor);
    if (((CallbackTask *)actor)->header.isActive != 0) {
        actor->matrix = NULL;
        actor->matrixDirty = 1;
    }
    createCallbackTaskWithUserIdPreservingArgs(initRacePlayerShockEffect, 5, 4, player->playerIndex);
}

void applyOnlineRemoteActionUse(RacePlayer *player, s32 actionType) {
    if ((player == NULL) || (actionType == RACE_ACTION_NONE)) {
        return;
    }
    applyRacePlayerActionEffect(player, actionType);
}
#endif

void updateRacePlayerItemEffectUse(RacePlayer *player) {
    s32 trigger;
    s32 deltaX;
    s32 deltaZ;
    s32 angle;
    register s32 i;
#ifdef ONLINE_BATTLE
    s32 firedType;
#endif

#ifdef ONLINE_BATTLE
    if (onlinePlayerIsRemote(player) != 0) {
        return;
    }
#endif

    trigger = 0;
#ifdef ONLINE_BATTLE
    if (!racePlayerAiControls(player)) {
#else
    if (player->isCpu == 0) {
#endif
        if (player->currentInputFlags & 0x2000) {
            trigger = 1;
        }
    } else {
        if (player->itemTriggerCooldown == 0) {
            player->itemTriggerCooldown = 10;
            if (player->itemTriggerChance >= randomNextMain()) {
                trigger = 1;
            }
        } else {
            player->itemTriggerCooldown--;
        }

        if ((trigger != 0) && (player->itemEffectCount != 0)) {
            trigger = 0;
            for (i = 0; i < RACE_PLAYER_COUNT; i++) {
                if (gRacePlayers[i].isCpu == 0) {
                    deltaX = gRacePlayers[i].pos.x - player->pos.x;
                    deltaZ = gRacePlayers[i].pos.z - player->pos.z;
                    if ((deltaX < 0x6000000) && (deltaX >= -0x5FFFFFF) && (deltaZ < 0x6000000) &&
                        (deltaZ >= -0x5FFFFFF)) {
                        angle = (s16)((calculateFixedAngleFromDeltaXZ(deltaX, deltaZ) -
                                       (s16)(player->facingAngle & 0xFFFFu)) &
                                      0xFFF);
                        if ((angle >= 0xE01) ||
                            (angle <
                             (((((((((((0x200 & 0xFFFF) & 0xFFFF) & 0xFFFF) & 0xFFFF) & 0xFFFF) & 0xFFFF) & 0xFFFF) &
                                 0xFFFF) &
                                0xFFFF) &
                               0xFFFF) &
                              0xFFFF))) {
                            trigger = 1;
                            player->itemTriggerCooldown = -0x3E;
                        }
                    }
                }
            }
        }
    }

    if (trigger != 0) {
        if (player->itemEffectCount != 0) {
#ifdef ONLINE_BATTLE
            firedType = player->itemEffectType;
            if (spawnRacePlayerItemProjectile(player, firedType) != 0) {
                player->itemEffectCount--;
                if (onlineSessionActive() != 0) {
                    onlineSessionEnqueueEvent(
                        ONLINE_EVENT_ITEM_USE,
                        (u8)player->playerIndex,
                        (u8)firedType,
                        0
                    );
                }
            }
#else
            if (player->itemEffectType == RACE_ITEM_SLAPSTICK) {
                if (createCallbackTaskWithUserIdPreservingArgs(
                        initWideHomingItemProjectile,
                        (u16)(player->playerIndex + 1),
                        0x3C,
                        player->playerIndex
                    ) != NULL) {
                    createCallbackTaskWithUserIdPreservingArgs(initRacePlayerShockEffect, 5, 4, player->playerIndex);
                    player->itemEffectCount--;
                }
            }
            if (player->itemEffectType == RACE_ITEM_PARACHUTE) {
                if (createCallbackTaskWithUserIdPreservingArgs(
                        initLongRangeHomingItemProjectile,
                        (u16)(player->playerIndex + 1),
                        0x3C,
                        player->playerIndex
                    ) != NULL) {
                    createCallbackTaskWithUserIdPreservingArgs(initRacePlayerShockEffect, 5, 4, player->playerIndex);
                    player->itemEffectCount--;
                }
            }
            if (player->itemEffectType == RACE_ITEM_FREEZE_SHOT) {
                if (createCallbackTaskWithUserIdPreservingArgs(
                        initCloseRangeHomingItemProjectile,
                        (u16)(player->playerIndex + 1),
                        0x3C,
                        player->playerIndex
                    ) != NULL) {
                    createCallbackTaskWithUserIdPreservingArgs(initRacePlayerShockEffect, 5, 4, player->playerIndex);
                    player->itemEffectCount--;
                }
            }
            if (player->itemEffectType == RACE_ITEM_SNOWMAN) {
                if (createCallbackTaskWithUserIdPreservingArgs(
                        initBouncingItemProjectile,
                        (u16)(player->playerIndex + 1),
                        0x3C,
                        player->playerIndex
                    ) != NULL) {
                    createCallbackTaskWithUserIdPreservingArgs(initRacePlayerShockEffect, 5, 4, player->playerIndex);
                    player->itemEffectCount--;
                }
            }
            if (player->itemEffectType == RACE_ITEM_BOMB) {
                if (createCallbackTaskWithUserIdPreservingArgs(
                        initAreaBlastItemProjectile,
                        (u16)(player->playerIndex + 1),
                        0x3C,
                        player->playerIndex
                    ) != NULL) {
                    createCallbackTaskWithUserIdPreservingArgs(initRacePlayerShockEffect, 5, 4, player->playerIndex);
                    player->itemEffectCount--;
                }
            }
#endif
            if (player->itemEffectCount == 0) {
                player->itemEffectType = RACE_ITEM_NONE;
            }
        }

        if (player->shieldEffectTimer != 0) {
            if (createCallbackTaskWithUserIdPreservingArgs(
                    initShieldProjectile,
                    (u16)(player->playerIndex + 1),
                    0x3C,
                    player->playerIndex
                ) != NULL) {
                createCallbackTaskWithUserIdPreservingArgs(initRacePlayerShockEffect, 5, 4, player->playerIndex);
                player->shieldEffectTimer--;
            }
        }
    }
}

void updateRacePlayerActionEffectUse(RacePlayer *player) {
    s32 trigger;
    s32 type;

#ifdef ONLINE_BATTLE
    if (onlinePlayerIsRemote(player) != 0) {
        return;
    }
#endif

    trigger = 0;
#ifdef ONLINE_BATTLE
    if (!racePlayerAiControls(player)) {
#else
    if (player->isCpu == 0) {
#endif
        if (player->currentInputFlags & 0x4000) {
            trigger = 1;
        }
    } else {
        if (player->actionTriggerCooldown == 0) {
            if (player->actionTriggerChance >= randomNextMain()) {
                trigger = 1;
            }
            player->actionTriggerCooldown = 10;
        } else {
            player->actionTriggerCooldown--;
        }

        if (player->actionEffectType == RACE_ACTION_INVISIBLE) {
            trigger = 0;
            if (player->itemTargetFlag != 0) {
                trigger = 1;
            } else {
                trigger = 0;
            }
        }
    }

    if (trigger != 0) {
        type = player->actionEffectType;
        if (type != RACE_ACTION_NONE) {
            if ((type == RACE_ACTION_SPEED_FAN) && (player->trailEffectTimer == 0)) {
                startSpeedFanEffect(player);
                player->actionEffectType = RACE_ACTION_NONE;
#ifdef ONLINE_BATTLE
                if (onlineSessionActive() != 0) {
                    onlineSessionEnqueueEvent(
                        ONLINE_EVENT_ACTION_USE,
                        (u8)player->playerIndex,
                        (u8)RACE_ACTION_SPEED_FAN,
                        0
                    );
                }
#endif
                type = player->actionEffectType;
            }

            if (type == RACE_ACTION_GHOST) {
                spawnRacePlayerSparkleEffect(player->playerIndex);
                player->actionEffectType = RACE_ACTION_NONE;
#ifdef ONLINE_BATTLE
                if (onlineSessionActive() != 0) {
                    onlineSessionEnqueueEvent(
                        ONLINE_EVENT_ACTION_USE,
                        (u8)player->playerIndex,
                        (u8)RACE_ACTION_GHOST,
                        0
                    );
                }
#endif
                type = player->actionEffectType;
            }

            if (type == RACE_ACTION_PAN) {
                spawnGhostSlowdownTargets(player->playerIndex);
                player->actionEffectType = RACE_ACTION_NONE;
#ifdef ONLINE_BATTLE
                if (onlineSessionActive() != 0) {
                    onlineSessionEnqueueEvent(
                        ONLINE_EVENT_ACTION_USE,
                        (u8)player->playerIndex,
                        (u8)RACE_ACTION_PAN,
                        0
                    );
                }
#endif
                type = player->actionEffectType;
            }

            if (type == RACE_ACTION_ROCK) {
                createCallbackTaskWithUserIdPreservingArgs(
                    initFallingActionProjectile,
                    0,
                    0x3C,
                    (u16)player->playerIndex
                );
                player->actionEffectType = RACE_ACTION_NONE;
#ifdef ONLINE_BATTLE
                if (onlineSessionActive() != 0) {
                    onlineSessionEnqueueEvent(
                        ONLINE_EVENT_ACTION_USE,
                        (u8)player->playerIndex,
                        (u8)RACE_ACTION_ROCK,
                        0
                    );
                }
#endif
                type = player->actionEffectType;
            }

            if (type == RACE_ACTION_RAT_FACE) {
                if (player->isCpu == 0) {
                    createCallbackTaskWithUserIdPreservingArgs(
                        initForwardActionProjectileEffect,
                        0,
                        3,
                        (u16)player->playerIndex
                    );
#ifdef ONLINE_BATTLE
                    if (onlineSessionActive() != 0) {
                        onlineSessionEnqueueEvent(
                            ONLINE_EVENT_ACTION_USE,
                            (u8)player->playerIndex,
                            (u8)RACE_ACTION_RAT_FACE,
                            0
                        );
                    }
#endif
                }
                player->actionEffectType = RACE_ACTION_NONE;
                type = player->actionEffectType;
            }

            if ((type == RACE_ACTION_INVISIBLE) && (player->actionSoundTimer == 0)) {
                player->actionSoundTimer = 0xB4;
                player->actionEffectType = RACE_ACTION_NONE;
                enqueuePositionalSoundEffect(0x10, &player->pos, 0x7F, 0x32);
#ifdef ONLINE_BATTLE
                if (onlineSessionActive() != 0) {
                    onlineSessionEnqueueEvent(
                        ONLINE_EVENT_ACTION_USE,
                        (u8)player->playerIndex,
                        (u8)RACE_ACTION_INVISIBLE,
                        0
                    );
                }
#endif
            }
        }
    }
}
