#include "common.h"
#include "game/race/items/race_items.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/race_state.h"
#include "game/engine/render_callback.h"
#include "game/engine/asset_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/race/player/race_player_rumble.h"
#include "game/audio/sound_manager.h"
#include "game/race/motion/race_motion.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/race_course_effects.h"
#include "game/race/player/race_player_collision.h"
#include "game/math/fixed_point_math.h"
#include "game/math/spatial_math.h"
#include "game/race/player/race_player_input.h"
#include "game/race/items/race_item_effects.h"
#include "game/race/player/race_player_pickup_effects.h"
#include "game/menu/renderer/menu_screen_effects.h"
#include "game/race/player/race_player_movement.h"
#include "game/race/player/race_player_progress.h"
#include "game/race/player/race_player_model_renderer.h"
#include "game/race/player/race_player_update.h"
#include "game/race/course/race_surface_cues.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/effects/snowboard_trail_effects.h"
#include "game/engine/viewport_manager.h"

#define setRaceMotionAnimation(player, animIndex) setRaceMotionAnimation(RACE_MOTION_STATE(player), (animIndex))
#define stepRaceMotionAnimationUntilEnd(player) stepRaceMotionAnimationUntilEnd(RACE_MOTION_STATE(player))
#define RACE_MOTION_STATE(player) ((RaceMotionState *)(player))
#define stepRaceMotionLoopingAnimation(player) stepRaceMotionLoopingAnimation(RACE_MOTION_STATE(player))
#define blendRaceMotionJointAnimation(player, animationIndex, blendTimer, blendDuration) \
    blendRaceMotionJointAnimation(RACE_MOTION_STATE(player), (animationIndex), (blendTimer), (blendDuration))
#define stepRaceMotionLoopingJointAnimation(player) stepRaceMotionLoopingJointAnimation(RACE_MOTION_STATE(player))
#define interpolateRaceMotionJointAnimationFrame(player, animIndex, frameTimer, frameTimerReset) \
    interpolateRaceMotionJointAnimationFrame(RACE_MOTION_STATE(player), (animIndex), (frameTimer), (frameTimerReset))

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/initRacePlayers.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_8008B8BC.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_8008B9F8.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/updateRacePlayers.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_8008C130.s")

void updateRacePlayerMotionFeedback(RacePlayer *player) {
    s16 angleDiff;
    s32 itemType;
    s32 deltaX;
    s32 deltaZ;

    deltaX = player->pos.x - player->previousPosition.x;
    deltaZ = player->pos.z - player->previousPosition.z;
    angleDiff = (calculateFixedAngleFromDeltaXZ(deltaX, deltaZ) - player->facingAngle) & 0xFFF;
    if (angleDiff >= 0x801) {
        angleDiff = 0x1000 - angleDiff;
    }
    if (angleDiff >= 0x401) {
        angleDiff = 0x800 - angleDiff;
    }

    player->unk588 = (f32)((f64)(f32)-angleDiff / 171.0);
    player->unk582 = player->movementSpeed >> 0xC;
    if (player->unk582 >= 0x80) {
        player->unk582 = 0x7F;
    }

    itemType = getRaceItemEffectType(player->unk330);
    player->unk584 = 3;
    if (itemType == 1) {
        player->unk584 = 0x1E;
        player->unk582 = 0x7F;
        player->unk588 = 0.0f;
    }
    if (itemType == 2) {
        player->unk584 = 4;
    }
    if (3 == itemType) {
        player->unk584 = 5;
    }
    if (4 == itemType) {
        player->unk584 = 4;
        if (player->unk330 == 0xD) {
            player->unk584 = 6;
            if (player->unk582 >= 0x51) {
                player->rumbleTimer = 2;
                player->rumblePatternId = 1;
            }
        }
        player->unk588 = 0.0f;
    }
    if (player->stateFlags & 0x2000000) {
        player->unk584 = 4;
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_8008C9E8.s")

void updateRacePlayerMode29Crash(RacePlayer *player) {
    s16 updateState;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        player->stateFlags &= ~0x200;
        player->stateTimer = 0;
        player->unk80 = 0;
        player->subState = 0;
        player->subStateTimer = 0;
        player->unk2FA = 0;
        player->unk528 = 0;
        setRaceMotionAnimation(player, 0x1E);
    }

    player->velocity.y -= player->gravity;
    player->facingAngle = player->unk332;
    player->pitchAngle = player->unk334;
    if (player->stateFlags & 0x400) {
        player->facingAngle += 0x800;
        player->pitchAngle = -(player->pitchAngle & 0xFFF);
    }

    updateRacePlayerLeanAngle(player, 0, 0);
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;

    if (stepRaceMotionAnimationUntilEnd(player) != 0) {
        setRaceMotionAnimation(player, 0x1F);
        stepRaceMotionAnimationUntilEnd(player);
    }

    player->unk578 = 6;
    updateRacePlayerMotionFeedback(player);
    if (player->unk517 != 0) {
        enqueueRacePlayerVoiceSound(player, 5);
    }
}

void updateRacePlayerMode01JumpStart(RacePlayer *player) {
    s32 unused;
    s32 targetX;
    s32 targetZ;
    s16 angleDiff;
    s32 yVel;

    if (player->updateState == 0) {
        if (player->animationId != 4) {
            setRaceMotionAnimation(player, 4);
        }
        player->updateState++;
        player->stateFlags |= 0x200;
        player->targetGroundOffset = 0;
        resetRacePlayerTrickSubstate(player);
        player->unk2A6 = 0;
    }

    if (player->subState == 0) {
        updateRacePlayerItemEffectUse(player);
        if (player->isCpu == 0) {
            if (player->stickX > 0) {
                player->facingAngle -= 0x18;
            }
            if (player->stickX < 0) {
                player->facingAngle += 0x18;
            }
        } else {
            getRaceCourseTargetPositionAhead(
                player->coursePathIndex,
                unused = player->pos.x,
                player->pos.z,
                &targetX,
                &targetZ,
                (s8)player->unk17,
                (u16)player->playerIndex
            );
            angleDiff = (calculateFixedAngleBetweenXZPoints(player->pos.x, player->pos.z, targetX, targetZ) -
                         player->facingAngle) &
                        0xFFF;
            if (angleDiff >= 0x801) {
                angleDiff -= 0x1000;
            }
            if (angleDiff >= 0x19) {
                angleDiff = 0x18;
            }
            if (angleDiff < -0x18) {
                angleDiff = -0x18;
            }
            player->facingAngle += angleDiff;
        }
        stepRaceMotionAnimationUntilEnd(player);
    }

    updateRacePlayerTrickSubstate(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;
    if (player->unk517 != 0) {
        enqueueRacePlayerVoiceSound(player, 5);
    }
}

void updateRacePlayerMode22Airborne(RacePlayer *player) {
    s32 unused;
    s32 targetX;
    s32 targetZ;
    s16 angleDiff;
    s32 yVel;

    if (player->updateState == 0) {
        player->updateState++;
        player->stateFlags |= 0x208;
        player->targetGroundOffset = 0;
        player->unk80 = 0;
        resetRacePlayerTrickSubstate(player);
        player->unk2A6 = 0;
    }

    if (player->subState == 0) {
        updateRacePlayerItemEffectUse(player);
        if (player->isCpu == 0) {
            if (player->stickX > 0) {
                player->facingAngle -= 0x18;
            }
            if (player->stickX < 0) {
                player->facingAngle += 0x18;
            }
        } else {
            getRaceCourseTargetPositionAhead(
                player->coursePathIndex,
                unused = player->pos.x,
                player->pos.z,
                &targetX,
                &targetZ,
                (s8)player->unk17,
                (u16)player->playerIndex
            );
            angleDiff = (calculateFixedAngleBetweenXZPoints(player->pos.x, player->pos.z, targetX, targetZ) -
                         player->facingAngle) &
                        0xFFF;
            if (angleDiff >= 0x801) {
                angleDiff -= 0x1000;
            }
            if (angleDiff >= 0x19) {
                angleDiff = 0x18;
            }
            if (angleDiff < -0x18) {
                angleDiff = -0x18;
            }
            player->facingAngle += angleDiff;
        }
    }

    updateRacePlayerTrickSubstate(player);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;
    if (player->unk517 != 0) {
        enqueueRacePlayerVoiceSound(player, 5);
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_8008D91C.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_8008D954.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_8008DC9C.s")

void resetRacePlayerTrickSubstate(RacePlayer *player) {
    player->subState = 0;
    player->subStateTimer = 0;
    player->subStateStep = 0;
    player->subStateParam = 0;
    player->unk2A0 = 0;
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/updateRacePlayerTrickSubstate.s")

void updateRacePlayerTrickSubstateStart(RacePlayer *player) {
    s16 inputMask;

    player->subStateTimer = 0;
    player->subStateStep = 0;
    player->unk2A2 = 0;
    player->unk2A4 = 0;
    if (player->isCpu == 0) {
        if (player->currentInputFlags & 1) {
            player->subStateTimer = 1;
            player->unk2A2 = 1;
        }
        if (player->currentInputFlags & 2) {
            player->subStateTimer = 2;
            player->unk2A2 = 3;
        }
        if (player->currentInputFlags & 8) {
            player->subStateTimer = 8;
            player->unk2A2 = 7;
        }
        if (player->currentInputFlags & 4) {
            player->subStateTimer = 4;
            player->unk2A2 = 5;
        }
        if (player->subStateTimer != 0) {
            if (player->unk2A0 < 6) {
                player->unk2A2++;
            }
            player->subState++;
            if (player->stateFlags & 0x400) {
                inputMask = player->subStateTimer;
                if (inputMask & 1) {
                    setRaceMotionAnimation(player, 0x17);
                    inputMask = player->subStateTimer;
                }
                if (inputMask & 2) {
                    setRaceMotionAnimation(player, 0x15);
                    inputMask = player->subStateTimer;
                }
            } else {
                inputMask = player->subStateTimer;
                if (inputMask & 1) {
                    setRaceMotionAnimation(player, 0x15);
                    inputMask = player->subStateTimer;
                }
                if (inputMask & 2) {
                    setRaceMotionAnimation(player, 0x17);
                    inputMask = player->subStateTimer;
                }
            }
            if (inputMask & 8) {
                setRaceMotionAnimation(player, 0x19);
                inputMask = player->subStateTimer;
            }
            if (inputMask & 4) {
                setRaceMotionAnimation(player, 0x1B);
            }
            player->stateFlags |= 0x800;
        }
        player->unk2A0++;
    }
}

void updateRacePlayerTrickSubstateHold(RacePlayer *player) {
    s16 inputMask;

    if (!(player->inputFlags & player->subStateTimer)) {
        player->subStateStep = 1;
    }
    if (stepRaceMotionAnimationUntilEnd(player) != 0) {
        if (player->subStateStep != 0) {
            player->subState++;
            if (player->stateFlags & 0x400) {
                inputMask = player->subStateTimer;
                if (inputMask & 1) {
                    setRaceMotionAnimation(player, 0x18);
                    inputMask = player->subStateTimer;
                }
                if (inputMask & 2) {
                    setRaceMotionAnimation(player, 0x16);
                    inputMask = player->subStateTimer;
                }
            } else {
                inputMask = player->subStateTimer;
                if (inputMask & 1) {
                    setRaceMotionAnimation(player, 0x16);
                    inputMask = player->subStateTimer;
                }
                if (inputMask & 2) {
                    setRaceMotionAnimation(player, 0x18);
                    inputMask = player->subStateTimer;
                }
            }
            if (inputMask & 8) {
                setRaceMotionAnimation(player, 0x1A);
                inputMask = player->subStateTimer;
            }
            if (inputMask & 4) {
                setRaceMotionAnimation(player, 0x1C);
            }
        } else {
            player->unk2A4++;
        }
    }
    player->stateFlags |= 0x800;
}

void updateRacePlayerTrickSubstateFinish(RacePlayer *player) {
    if (stepRaceMotionAnimationUntilEnd(player) != 0) {
        player->subState++;
    }
    player->stateFlags |= 0x800;
}

void updateRacePlayerTrickSubstateNoop(RacePlayer *player) {
}

void updateRacePlayerMode13AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 stateTimer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }

    if (player->subState == 0) {
        stepRaceMotionAnimationUntilEnd(player);
    }

    updateRacePlayerTrickSubstate(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    player->modelRotationX = (s16)((s32)(fixedSine(player->stateTimer) * 0x1000) / 0x1000);
    player->stateTimer += 0x20;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }
    stateTimer = player->stateTimer;
    player->stateFlags |= 2;
    if (stateTimer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(
                initRacePlayerYellowTrickSparkles,
                5,
                2,
                (u16)player->playerIndex
            );
        }
    }
}

void updateRacePlayerMode15AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 stateTimer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }

    if (player->subState == 0) {
        stepRaceMotionAnimationUntilEnd(player);
    }

    updateRacePlayerTrickSubstate(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    player->modelRotationX = (s16)((s32)(fixedSine(player->stateTimer) * -0x1000) / 0x1000);
    player->stateTimer += 0x20;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }
    stateTimer = player->stateTimer;
    player->stateFlags |= 2;
    if (stateTimer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(
                initRacePlayerYellowTrickSparkles,
                5,
                2,
                (u16)player->playerIndex
            );
        }
    }
}

void updateRacePlayerMode16AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }
    if (player->subState == 0) {
        stepRaceMotionAnimationUntilEnd(player);
    }
    updateRacePlayerTrickSubstate(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;
    if (player->stateFlags & 0x400) {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) << 0xC) / 4096);
    } else {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) * -0x1000) / 4096);
    }

    player->stateTimer += 0x1E;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }
    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(
                initRacePlayerYellowTrickSparkles,
                5,
                2,
                (u16)player->playerIndex
            );
        }
    }
}

void updateRacePlayerMode17AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }
    if (player->subState == 0) {
        stepRaceMotionAnimationUntilEnd(player);
    }
    updateRacePlayerTrickSubstate(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;
    if (player->stateFlags & 0x400) {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) * -0x1000) / 0x1000);
    } else {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) << 12) / 0x1000);
    }

    player->stateTimer += 0x1D;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }
    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(
                initRacePlayerYellowTrickSparkles,
                5,
                2,
                (u16)player->playerIndex
            );
        }
    }
}

void updateRacePlayerMode18AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }
    if (player->subState == 0) {
        stepRaceMotionAnimationUntilEnd(player);
    }
    updateRacePlayerTrickSubstate(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;
    player->modelRotationX = (s16)((s32)(fixedSine((s16)player->stateTimer) << 12) / 0x1000);
    if (player->stateFlags & 0x400) {
        player->modelRotationY = (s16)((s32)(fixedSine((s16)player->stateTimer) << 12) / 0x1000);
    } else {
        player->modelRotationY = (s16)((s32)(fixedSine((s16)player->stateTimer) * -0x1000) / 0x1000);
    }

    player->stateTimer += 0x1A;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }
    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(
                initRacePlayerYellowTrickSparkles,
                5,
                2,
                (u16)player->playerIndex
            );
        }
    }
}

void updateRacePlayerMode19AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }
    if (player->subState == 0) {
        stepRaceMotionAnimationUntilEnd(player);
    }
    updateRacePlayerTrickSubstate(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;
    player->modelRotationX = (s16)((s32)(fixedSine(player->stateTimer) << 12) / 0x1000);
    if (player->stateFlags & 0x400) {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) * -0x1000) / 0x1000);
    } else {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) << 12) / 0x1000);
    }

    player->stateTimer += 0x1A;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }
    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(
                initRacePlayerYellowTrickSparkles,
                5,
                2,
                (u16)player->playerIndex
            );
        }
    }
}

void updateRacePlayerMode20AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }
    if (player->subState == 0) {
        stepRaceMotionAnimationUntilEnd(player);
    }
    updateRacePlayerTrickSubstate(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;
    player->modelRotationX = (s16)((s32)(fixedSine(player->stateTimer) * -0x1000) / 0x1000);
    if (player->stateFlags & 0x400) {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) << 12) / 0x1000);
    } else {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) * -0x1000) / 0x1000);
    }

    player->stateTimer += 0x1A;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }
    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(
                initRacePlayerYellowTrickSparkles,
                5,
                2,
                (u16)player->playerIndex
            );
        }
    }
}

void updateRacePlayerMode21AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }
    if (player->subState == 0) {
        stepRaceMotionAnimationUntilEnd(player);
    }
    updateRacePlayerTrickSubstate(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;
    player->modelRotationX = (s16)((s32)(fixedSine(player->stateTimer) * -0x1000) / 0x1000);
    if (player->stateFlags & 0x400) {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) * -0x1000) / 0x1000);
    } else {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) << 12) / 0x1000);
    }

    player->stateTimer += 0x1A;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }
    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(
                initRacePlayerYellowTrickSparkles,
                5,
                2,
                (u16)player->playerIndex
            );
        }
    }
}

void updateRacePlayerMode31AerialTrick(RacePlayer *player) {
    s16 updateTimer;
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        setRaceMotionAnimation(player, 0x15);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (fixedSine(player->stateTimer) * -0x2000) / 4096;
    } else {
        player->modelRotationY = (fixedSine(player->stateTimer) * 0x2000) / 4096;
    }

    player->stateTimer += 0x16;
    player->updateTimer++;
    updateTimer = player->updateTimer;
    if (updateTimer == 8) {
        setRaceMotionAnimation(player, 0x16);
        updateTimer = player->updateTimer;
    }
    if (updateTimer == 0xF) {
        setRaceMotionAnimation(player, 0x1B);
        updateTimer = player->updateTimer;
    }
    if (updateTimer == 0x1E) {
        setRaceMotionAnimation(player, 0x1C);
    }

    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode41AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    player->modelRotationX = (fixedSine(player->stateTimer) << 13) / 0x1000;

    player->stateTimer += 0x14;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode42AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    player->modelRotationZ = (fixedSine(player->stateTimer) << 13) / 0x1000;

    player->stateTimer += 0x14;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode43AerialTrick(RacePlayer *player) {
    s16 updateTimer;
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);

    updateTimer = player->updateTimer;
    if (updateTimer < 8) {
        player->pitchAngle = -updateTimer << 7;
    }
    if (updateTimer >= 0x23) {
        player->pitchAngle = (-(-updateTimer) << 6) - 0xC80;
    }
    if (updateTimer < 0x32) {
        player->updateTimer = updateTimer + 1;
    }

    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    player->modelRotationY = (fixedSine(player->stateTimer) << 14) / 0x1000;

    player->stateTimer += 0x14;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode32AerialTrick(RacePlayer *player) {
    s16 updateState;
    s32 yVel;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x28);
        player->stateTimer = 0;
        player->stateFlags |= 0x200;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
    }

    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (stepRaceMotionAnimationUntilEnd(player) == 0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode33AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 0x23);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    player->modelRotationX = (fixedSine(player->stateTimer) * -0x3000) / 4096;
    if (player->stateFlags & 0x400) {
        player->modelRotationY = -player->updateTimer;
    } else {
        player->modelRotationY = player->updateTimer;
    }
    if (player->updateTimer != 0x1000) {
        player->updateTimer += 0x80;
    }

    player->stateTimer += 0x11;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode34AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 0x23);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->modelRotationX = player->stateTimer * 4;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (fixedSine((s16)player->stateTimer) * -0x4000) / 0x1000;
    } else {
        player->modelRotationY = (fixedSine((s16)player->stateTimer) << 0xE) / 0x1000;
    }

    player->stateTimer += 0x15;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode44AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        setRaceMotionAnimation(player, 0x17);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (fixedSine(player->stateTimer) * -0x2000) / 4096;
    } else {
        player->modelRotationY = (fixedSine(player->stateTimer) * 0x2000) / 4096;
    }

    player->stateTimer += 0x16;
    player->updateTimer++;
    if (player->updateTimer == 8) {
        setRaceMotionAnimation(player, 0x18);
    }
    if (player->updateTimer == 0xF) {
        setRaceMotionAnimation(player, 0x19);
    }
    if (player->updateTimer == 0x1E) {
        setRaceMotionAnimation(player, 0x1A);
    }

    timer = player->stateTimer;
    if (timer >= 0x401) {
        timer = 0x400;
        player->stateTimer = timer;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode45AerialTrick(RacePlayer *player) {
    s16 updateTimer;
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    player->updateTimer = (fixedSine(player->stateTimer) << 13) / 0x1000;
    updateTimer = player->updateTimer;
    if (updateTimer >= 0x1001) {
        player->modelRotationY = 0x2000 - updateTimer;
    } else {
        player->modelRotationY = updateTimer;
    }

    if (player->stateFlags & 0x400) {
        player->modelRotationY = -player->modelRotationY;
    }

    player->modelRotationX = (fixedSine(player->stateTimer) << 13) / 0x1000;
    player->stateTimer += 0x14;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode46AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        setRaceMotionAnimation(player, 0x1B);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (fixedSine(player->stateTimer) * -0x5000) / 0x1000;
    } else {
        player->modelRotationY = (fixedSine(player->stateTimer) * 0x5000) / 0x1000;
    }

    player->stateTimer += 0x12;
    player->updateTimer++;
    if (player->updateTimer == 0x28) {
        setRaceMotionAnimation(player, 0x1C);
    }

    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode35AerialTrick(RacePlayer *player) {
    s16 updateState;
    s32 yVel;
    s32 stateTimer;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        setRaceMotionAnimation(player, 0x15);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationZ = (fixedSine(player->stateTimer) * -0x2000) / 0x1000;
    } else {
        player->modelRotationZ = (fixedSine(player->stateTimer) << 13) / 0x1000;
    }

    player->stateTimer += 0x16;
    player->updateTimer++;
    if (player->updateTimer == 0x1E) {
        setRaceMotionAnimation(player, 0x16);
    }

    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    stateTimer = player->stateTimer;
    player->stateFlags |= 2;
    if (stateTimer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode36AerialTrick(RacePlayer *player) {
    s16 updateState;
    s32 yVel;
    s32 timer;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        setRaceMotionAnimation(player, 0x15);
        player->updateTimer = 0;
    }
    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) * -0x3000) / 4096);
    } else {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) * 0x3000) / 4096);
    }

    player->stateTimer += 0x15;
    player->updateTimer++;
    if (player->updateTimer == 0x1E) {
        setRaceMotionAnimation(player, 0x16);
    }

    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode47AerialTrick(RacePlayer *player) {
    s16 updateTimer;
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        setRaceMotionAnimation(player, 0x19);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (fixedSine(player->stateTimer) * -0x2000) / 4096;
    } else {
        player->modelRotationY = (fixedSine(player->stateTimer) << 0xD) / 4096;
    }

    player->stateTimer += 0x16;
    player->updateTimer++;
    updateTimer = player->updateTimer;
    if (updateTimer == 8) {
        setRaceMotionAnimation(player, 0x1A);
        updateTimer = player->updateTimer;
    }
    if (updateTimer == 0xF) {
        setRaceMotionAnimation(player, 0x15);
        updateTimer = player->updateTimer;
    }
    if (updateTimer == 0x1E) {
        setRaceMotionAnimation(player, 0x16);
    }

    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode48AerialTrick(RacePlayer *player) {
    s16 updateState;
    s32 yVel;
    s32 stateTimer;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        setRaceMotionAnimation(player, 0x15);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationZ = (fixedSine(player->stateTimer) * 0x3000) / 0x1000;
    } else {
        player->modelRotationZ = (fixedSine(player->stateTimer) * -0x3000) / 0x1000;
    }

    player->stateTimer += 0x16;
    if (player->updateTimer < 0x2D) {
        player->updateTimer = player->updateTimer + 1;
    }

    if (player->updateTimer == 0x1E) {
        setRaceMotionAnimation(player, 0x16);
    }

    stateTimer = player->stateTimer;
    player->modelRotationY = (player->updateTimer << 0xC) / 45;
    if (stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    stateTimer = player->stateTimer;
    player->stateFlags |= 2;
    if (stateTimer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode49AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    player->modelRotationX = (fixedSine(player->stateTimer) * -0x3000) / 0x1000;

    player->stateTimer += 0x12;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode37AerialTrick(RacePlayer *player) {
    s16 updateState;
    s16 updateTimer;
    s16 tilt;
    s16 *updateTimerPtr;
    s32 yVel;
    s32 timer;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x23);
        player->stateTimer = 0;
        player->stateFlags |= 0x200;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
        player->unk306 = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = (u32)player->velocity.y;
    player->pos.x += player->velocity.x;
    updateTimerPtr = &player->updateTimer;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationZ = (fixedSine(player->stateTimer) * -0x2000) / 4096;
    } else {
        player->modelRotationZ = (fixedSine(player->stateTimer) << 0xD) / 4096;
    }

    player->stateTimer += 0x16;
    player->updateTimer++;
    updateTimer = player->updateTimer;
    if (updateTimer == 0xC) {
        setRaceMotionAnimation(player, 0x15);
        updateTimer = *updateTimerPtr;
    }

    if (updateTimer == 0x1E) {
        setRaceMotionAnimation(player, 0x16);
        updateTimer = player->updateTimer;
    }

    if ((updateTimer >= 9) && (updateTimer < 0x14)) {
        tilt = player->unk306;
        if (tilt != -0x400) {
            player->unk306 = tilt - 0x80;
            updateTimer = player->updateTimer;
        }
    }

    tilt = player->unk306;
    if ((updateTimer >= 0x1F) && (tilt != 0)) {
        player->unk306 = tilt + 0x80;
        tilt = player->unk306;
    }

    if (player->stateFlags & 0x400) {
        player->modelRotationY = -tilt;
    } else {
        player->modelRotationY = tilt;
    }

    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode50AerialTrick(RacePlayer *player) {
    s16 updateTimer;
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        setRaceMotionAnimation(player, 0x19);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (fixedSine(player->stateTimer) * -0x2000) / 4096;
    } else {
        player->modelRotationY = (fixedSine(player->stateTimer) * 0x2000) / 4096;
    }

    player->stateTimer += 0x16;
    player->updateTimer++;
    updateTimer = player->updateTimer;
    if (updateTimer == 8) {
        setRaceMotionAnimation(player, 0x1A);
        updateTimer = player->updateTimer;
    }
    if (updateTimer == 0xF) {
        setRaceMotionAnimation(player, 0x17);
        updateTimer = player->updateTimer;
    }
    if (updateTimer == 0x1E) {
        setRaceMotionAnimation(player, 0x18);
    }

    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode51AerialTrick(RacePlayer *player) {
    s16 updateState;
    s16 updateTimer;
    RacePlayer *playerAlias;
    s16 tilt;
    s32 yVel;
    s32 timer;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x23);
        player->stateTimer = 0;
        player->stateFlags |= 0x200;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
        player->unk306 = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    playerAlias = player;
    playerAlias->velocity.y -= playerAlias->unk264;
    clampRacePlayerVectorXZSpeed(&playerAlias->velocity, player);

    yVel = player->velocity.y;
    playerAlias->pos.x += player->velocity.x;
    player->pos.y += yVel;
    playerAlias->pos.z += player->velocity.z;
    playerAlias->verticalVelocityLimit = yVel;

    if (playerAlias->stateFlags & 0x400) {
        playerAlias->modelRotationZ = (fixedSine((s16)player->stateTimer) * 0x3000) / 4096;
    } else {
        player->modelRotationZ = (fixedSine((s16)player->stateTimer) * -0x3000) / 4096;
    }

    playerAlias->stateTimer += 0x14;
    player->updateTimer++;
    updateTimer = player->updateTimer;
    if (updateTimer == 0xC) {
        setRaceMotionAnimation(playerAlias, 0x15);
        updateTimer = playerAlias->updateTimer;
    }

    if (updateTimer == 0x1E) {
        setRaceMotionAnimation(playerAlias, 0x16);
        updateTimer = playerAlias->updateTimer;
    }

    if ((updateTimer >= 9) && (updateTimer < 0x14)) {
        tilt = player->unk306;
        if (tilt != 0x400) {
            player->unk306 = tilt + 0x80;
            updateTimer = playerAlias->updateTimer;
        }
    }

    tilt = player->unk306;
    if ((updateTimer >= 0x1F) && (tilt != 0)) {
        player->unk306 = tilt - 0x80;
        tilt = player->unk306;
    }

    if (playerAlias->stateFlags & 0x400) {
        playerAlias->modelRotationY = -tilt;
    } else {
        player->modelRotationY = tilt;
    }

    timer = player->stateTimer;
    if (timer >= 0x401) {
        timer = 0x400;
        playerAlias->stateTimer = timer;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        playerAlias->stateFlags |= 0x800;
        if ((playerAlias->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(
                initRacePlayerBlueTrickSparkles,
                5,
                2,
                (u16)playerAlias->playerIndex
            );
        }
    }
}

void updateRacePlayerMode52AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 stateTimer;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        setRaceMotionAnimation(player, 0x1B);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (fixedSine(player->stateTimer) * -0x3000) / 0x1000;
    } else {
        player->modelRotationY = (fixedSine(player->stateTimer) * 0x3000) / 0x1000;
    }

    player->stateTimer += 0x16;
    player->updateTimer++;
    if (player->updateTimer == 0x1E) {
        setRaceMotionAnimation(player, 0x1C);
    }

    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode38AerialTrick(RacePlayer *player) {
    s16 updateState;
    s32 yVel;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x23);
        player->stateTimer = 0;
        player->stateFlags |= 0x200;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
    }

    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (stepRaceMotionAnimationUntilEnd(player) == 0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode53AerialTrick(RacePlayer *player) {
    s16 updateTimer;
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        setRaceMotionAnimation(player, 0x19);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (fixedSine(player->stateTimer) * -0x1800) / 4096;
    } else {
        player->modelRotationY = ((fixedSine(player->stateTimer) * 6) * 1024) / 4096;
    }

    player->stateTimer += 0x16;
    player->updateTimer++;
    updateTimer = player->updateTimer;
    if (updateTimer == 8) {
        setRaceMotionAnimation(player, 0x1A);
        updateTimer = player->updateTimer;
    }
    if (updateTimer == 0xF) {
        setRaceMotionAnimation(player, 0x1B);
        updateTimer = player->updateTimer;
    }
    if (updateTimer == 0x1E) {
        setRaceMotionAnimation(player, 0x1C);
    }

    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode54AerialTrick(RacePlayer *player) {
    s16 updateState;
    s32 yVel;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x28);
        player->stateTimer = 0;
        player->stateFlags |= 0x200;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
    }

    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (stepRaceMotionAnimationUntilEnd(player) == 0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode39AerialTrick(RacePlayer *player) {
    s16 updateTimer;
    s32 yVel;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
        player->updateTimer = 0;
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    updateTimer = player->updateTimer;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    switch (updateTimer) {
        case 0:
            player->modelRotationX = (s16)((s32)(fixedSine(player->stateTimer) << 12) / 4096);
            player->stateTimer += 0x33;
            if (player->stateTimer >= 0x401) {
                player->stateTimer = 0;
                player->updateTimer++;
                player->unk306 = 0;
            }

            player->stateFlags |= 0x800;
            if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
                createCallbackTaskWithUserIdPreservingArgs(
                    initRacePlayerBlueTrickSparkles,
                    5,
                    2,
                    (u16)player->playerIndex
                );
            }
            break;
        case 1:
            if (player->stateFlags & 0x400) {
                player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) * -0x2000) / 4096);
            } else {
                player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) << 13) / 4096);
            }

            player->modelRotationX = -player->unk306;
            player->stateTimer += 0x20;
            if (player->stateTimer >= 0x401) {
                player->stateTimer = 0x400;
            }

            if (player->unk306 != 0x1000) {
                player->unk306 += 0x80;
            }

            if (player->stateTimer < 0x3D0) {
                player->stateFlags |= 0x800;
                if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
                    createCallbackTaskWithUserIdPreservingArgs(
                        initRacePlayerBlueTrickSparkles,
                        5,
                        2,
                        (u16)player->playerIndex
                    );
                }
            }
            break;
    }

    player->stateFlags |= 2;
}

void updateRacePlayerMode55AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) * -0x2000) / 4096);
    } else {
        player->modelRotationY = (s16)((s32)(fixedSine(player->stateTimer) << 0xD) / 4096);
    }

    player->stateTimer += 0x16;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode56AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    if (player->stateFlags & 0x400) {
        player->modelRotationY = (fixedSine(player->stateTimer) * -0x5000) / 0x1000;
    } else {
        player->modelRotationY = (fixedSine(player->stateTimer) * 0x5000) / 0x1000;
    }

    player->stateTimer += 0x14;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode57AerialTrick(RacePlayer *player) {
    s32 yVel;
    s32 timer;

    if (player->updateState == 0) {
        setRaceMotionAnimation(player, 4);
        player->updateState++;
        player->stateFlags |= 0x200;
        player->stateTimer = 0;
        resetRacePlayerTrickSubstate(player);
    }

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y -= player->unk264;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);

    yVel = player->velocity.y;
    player->pos.x += player->velocity.x;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->verticalVelocityLimit = yVel;

    player->modelRotationX = (fixedSine(player->stateTimer) * -0x5000) / 0x1000;

    player->stateTimer += 0x14;
    if (player->stateTimer >= 0x401) {
        player->stateTimer = 0x400;
    }

    timer = player->stateTimer;
    player->stateFlags |= 2;
    if (timer < 0x3D0) {
        player->stateFlags |= 0x800;
        if ((player->soundDisabled == 0) && (gFrameCounter & 1)) {
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerBlueTrickSparkles, 5, 2, (u16)player->playerIndex);
        }
    }
}

void updateRacePlayerMode03Nudge(RacePlayer *player) {
    RacePlayer *playerAlias;
    RaceCourseStartEntry *spawn;
    s16 updateState;
    s16 angleDiff;
    s32 clamped;
    s32 tempX;
    s32 tempZ;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x2000;
        if (((player->unk2DE - player->facingAngle) & 0xFFF) < 0x800) {
            setRaceMotionAnimation(player, 7);
        } else {
            setRaceMotionAnimation(player, 6);
        }
        player->velocity.x = player->unk2E0;
        player->velocity.z = player->unk2E4;
        player->rumbleTimer = 3;
        player->rumblePatternId = 0;
    }

    updateRacePlayerLeanAngle(player, 0x10000, 0);
    playerAlias = player;
    player->speedLimit = 0x60000;
    if (!(playerAlias->stateFlags & 1)) {
        playerAlias->stateFlags &= ~0x200;
        playerAlias->velocity.y -= 0xA000;
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, (unsigned long long)0x2000, 0x2000, 0x2000);
    } else {
        playerAlias->stateFlags |= 0x200;
        player->velocity.y -= 0x7000;
        clampRacePlayerVectorXZSpeed(&player->velocity, player);

        tempX = playerAlias->velocity.x;
        clamped = tempX;
        if (clamped >= 0x2001) {
            clamped = 0x2000;
        }
        updateState = -0x2000;
        if (clamped < -0x2000) {
            clamped = updateState;
        }
        player->velocity.x = tempX - clamped;

        tempZ = playerAlias->velocity.z;
        clamped = tempZ;
        if (clamped >= 0x2001) {
            clamped = 0x2000;
        }
        if (clamped < updateState) {
            clamped = updateState;
        }
        player->velocity.z = tempZ - clamped;
    }

    playerAlias->pos.x += playerAlias->velocity.x;
    playerAlias->pos.y += player->velocity.y;
    playerAlias->pos.z += playerAlias->velocity.z;

    spawn = &gRaceCourseStartEntries[gRaceCourseIndex.signedValue];
    if ((spawn->pathIndex == playerAlias->coursePathIndex) && !(playerAlias->stateFlags & 0x40)) {
        angleDiff = calculateFixedAngleBetweenXZPoints(
            playerAlias->pos.x,
            player->pos.z,
            spawn->startSurfaceTargetX,
            spawn->startSurfaceTargetZ
        );
        if (playerAlias->stateFlags & 0x400) {
            angleDiff += 0x800;
        }

        angleDiff = (angleDiff - player->facingAngle) & 0xFFF;
        if (angleDiff >= 0x801) {
            angleDiff -= 0x1000;
        }
        if (angleDiff >= 0x97) {
            angleDiff = 0x96;
        }
        if (angleDiff < -0x96) {
            angleDiff = -0x96;
        }
        playerAlias->facingAngle += angleDiff;
    }

    if (!(player->stateFlags & 1)) {
        updateRacePlayerMotionFeedback(player);
    }

    if (stepRaceMotionAnimationUntilEnd(player) != 0) {
        playerAlias->stateFlags &= 0xFE0C1FFB;
        if (playerAlias->stateFlags & 0x200) {
            player->mode = 1;
            playerAlias->updateState = 0;
            player->updateTimer = 0;
        } else {
            playerAlias->mode = 0;
            playerAlias->updateState = 0;
            playerAlias->updateTimer = 0;
        }
    }
}

void updateRacePlayerMode04Spinout(RacePlayer *player) {
    s16 updateState;
    s32 tempX;
    s32 tempZ;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0xE);
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x12204;
        player->pitchAngle = 0;
        updateRacePlayerLeanAngle(player, 0, 0);
        player->velocity.x = player->unk2E0;
        player->velocity.z = player->unk2E4;
        player->velocity.y = 0x20000;
        player->targetGroundOffset = 0;
        player->rumbleTimer = 3;
        player->rumblePatternId = 0;
    }

    player->speedLimit = 0x60000;
    if (player->stateFlags & 1) {
        clampRacePlayerVectorXZSpeed(&player->velocity, player);
        tempX = player->velocity.x;
        tempZ = player->velocity.z;
        player->velocity.y -= 0xA000;
        player->velocity.x = tempX - (tempX / 15);
        player->velocity.z = tempZ - (tempZ / 15);
    } else {
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0x3000, 0x3000, 0x3000);
    }

    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;

    if ((stepRaceMotionAnimationUntilEnd(player) != 0) && !(player->stateFlags & 1)) {
        player->mode = 5;
        player->updateState = 0;
        player->updateTimer = 0;
    }
}

void updateRacePlayerMode05SpinoutStun(RacePlayer *player) {
    s16 updateState;
    s32 stateTimer;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        player->stateTimer = 0x1E;
        if (player->stateFlags & 0x800000) {
            player->stateTimer = 0x5A;
            spawnRaceUiStunOrbitingIcons(player->playerIndex);
        }
        setRaceMotionAnimation(player, 0xF);
        stateTimer = player->stateTimer;
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags &= ~0x200;
        player->stateFlags |= 0x1012000;
        player->targetGroundOffset = 0;
        player->stateTimer = stateTimer;
        /* Rank-scaled recovery timer for humans and CPUs. With zero-based rank,
         * integer truncation gives 60/53/45/38 updates for a base of 60. */
        player->stateTimer = stateTimer - ((player->stateTimer * player->rankIndex) / 8);
        player->rumbleTimer = 1;
        player->rumblePatternId = 0;
    }

    updateRacePlayerLeanAngle(player, 0, 0);
    if (!(player->stateFlags & 1)) {
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0x4000, 0x4000, 0x4000);
    }

    player->velocity.y -= 0xA000;
    player->pos.y += player->velocity.y;

    if (!(player->stateFlags & 1)) {
        player->stateTimer = player->stateTimer - getRaceInputTimerDecrementBonus(player) - 1;
        if (player->stateTimer < 0) {
            player->stateTimer = 0;
        }
        if (stepRaceMotionAnimationUntilEnd(player) != 0) {
            player->stateFlags &= ~0x200;
            if (player->stateTimer == 0) {
                player->mode = 8;
                player->updateState = 0;
                player->updateTimer = 0;
            }
        }
    } else {
        player->stateFlags |= 0x200;
        setRaceMotionAnimation(player, 0xF);
        stepRaceMotionAnimationUntilEnd(player);
        if (player->stateTimer < 0x1E) {
            player->stateTimer = 0x1E;
        }
        if (!(player->stateFlags & 1)) {
            player->rumbleTimer = 1;
            player->rumblePatternId = 0;
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80092E38.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80093020.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_8009332C.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80093758.s")

void updateRacePlayerMode10TerrainCrashSlide(RacePlayer *player) {
    s16 updateState;
    s32 yVel;
    s32 velocityX;
    s32 velocityZ;
    u32 stateFlags;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        player->stateFlags &= ~0x200;
        player->stateFlags &= 0xFE0C1FFB;
        (*player).stateFlags |= 0x01006000;
        if (player->animationId != 0x12) {
            setRaceMotionAnimation(player, 0x12);
        }
        player->stateTimer = 0x1E;
        player->targetGroundOffset = 0;
    }
    stepRaceMotionLoopingAnimation(player);
    updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
    player->velocity.y += 0xFFFF6000;
    player->speedLimit = 0x80000;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    stateFlags = player->stateFlags;
    if ((stateFlags & 1) == 0) {
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0, 0, 0);
        player->unk582 = 0x100;
        player->unk584 = 8;
        player->unk588 = 0.0f;
        stateFlags = player->stateFlags;
    }
    if ((stateFlags & 1) != 0) {
        player->stateFlags = stateFlags | 0x200;
    } else {
        player->stateFlags = stateFlags & ~0x200;
    }
    yVel = player->velocity.y;
    velocityX = player->velocity.x;
    velocityZ = player->velocity.z;
    player->pos.y += yVel;
    player->verticalVelocityLimit = 0;
    player->pos.x += velocityX;
    player->pos.y += yVel;
    player->pos.z += player->velocity.z;
    player->facingAngle = calculateFixedAngleFromDeltaXZ(velocityX, velocityZ);
    if (player->stateFlags & 0x400) {
        player->facingAngle += 0x800;
    }
    if (player->unk330 != 3) {
        if (--player->stateTimer == 0) {
            player->stateFlags &= 0xFE0C1FFB;
            if (player->stateFlags & 1) {
                player->mode = 1;
                player->updateState = 0;
                player->updateTimer = 0;
            } else {
                player->mode = 0;
                player->updateState = 0;
                player->updateTimer = 0;
            }
        }
    } else {
        player->stateTimer = 0x1E;
    }
    player->rumbleTimer = 4;
    player->rumblePatternId = 2;
}

void updateRacePlayerMode12LaunchRecover(RacePlayer *player) {
    s16 updateState;
    u32 stateFlags;
    s32 grounded;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x14);
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags &= ~0x200;
        player->stateFlags |= 0x1022000;
        player->targetGroundOffset = 0;
        player->unk80 = 0;
        /* Rank-scaled recovery timer for humans and CPUs. With zero-based rank,
         * integer truncation gives 60/53/45/38 updates for a base of 60. */
        player->stateTimer = 0x1E - ((player->rankIndex * 0x1E) / 8);
        player->rumbleTimer = 1;
        player->rumblePatternId = 0;
    }

    updateRacePlayerLeanAngle(player, 0, 0);
    player->velocity.y += 0xFFFF6000;
    grounded = player->stateFlags & 1;
    if (grounded == 0) {
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0x6000, 0x6000, 0x6000);
        grounded = player->stateFlags & 1;
    }

    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;

    if (grounded == 0) {
        updateRacePlayerMotionFeedback(player);
        if ((player->unk584 != 6) && (player->unk584 != 0x1E)) {
            player->unk588 = -6.0f;
        }
    }

    stateFlags = player->stateFlags;
    if (!(stateFlags & 1)) {
        player->stateTimer = player->stateTimer - getRaceInputTimerDecrementBonus(player) - 1;
        if (player->stateTimer < 0) {
            player->stateTimer = 0;
        }
        if (stepRaceMotionAnimationUntilEnd(player)) {
            player->stateFlags &= ~0x200;
            if (player->stateTimer == 0) {
                player->mode = 8;
                player->updateState = 0;
                player->updateTimer = 0;
            }
        }
    } else {
        player->stateFlags = stateFlags | 0x200;
        setRaceMotionAnimation(player, 0x14);
        stepRaceMotionAnimationUntilEnd(player);
        player->stateTimer = 0x1E;
        player->unk80 = 7;
        if (!(player->stateFlags & 1)) {
            player->rumbleTimer = 1;
            player->rumblePatternId = 0;
        }
    }
}

void updateRacePlayerMode11LaunchHit(RacePlayer *player) {
    s16 updateState;
    s32 tempX;
    s32 tempZ;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x13);
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x22200;
        player->pitchAngle = 0;
        updateRacePlayerLeanAngle(player, 0, 0);
        player->targetGroundOffset = 0;
        player->rumbleTimer = 3;
        player->rumblePatternId = 0;
    }

    player->speedLimit = 0x60000;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    player->velocity.y -= player->unk264;
    if (!(player->stateFlags & 1)) {
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0x4000, 0x4000, 0x4000);
    } else {
        tempX = player->velocity.x;
        tempZ = player->velocity.z;
        player->velocity.x = tempX - (tempX / 16);
        player->velocity.z = tempZ - (tempZ / 16);
    }

    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;

    if ((stepRaceMotionAnimationUntilEnd(player) != 0) && !(player->stateFlags & 1)) {
        player->mode = 0xC;
        player->updateState = 0;
        player->updateTimer = 0;
    }
}

void updateRacePlayerMode26Tumble(RacePlayer *player) {
    s16 updateState;
    s32 timer;

    updateState = player->updateState;
    if (updateState == 0) {
        player->updateState = updateState + 1;
        setRaceMotionAnimation(player, 0x12);
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x6204;
        player->pitchAngle = 0;
        updateRacePlayerLeanAngle(player, 0, 0);
        player->targetGroundOffset = 0;
        player->velocity.y = 0x80000;
        player->stateTimer = 0x14;
    }

    if (gFrameCounter & 1) {
        createCallbackTaskWithUserIdPreservingArgs(initRacePlayerRecoverySparkle, 5, 2, (u16)player->playerIndex);
    }

    player->velocity.y -= 0x7000;
    player->pos.y += player->velocity.y;
    stepRaceMotionLoopingAnimation(player);

    timer = player->stateTimer;
    if (timer == 0) {
        if (!(player->stateFlags & 1)) {
            player->mode = 0xC;
            player->updateState = 0;
            player->updateTimer = 0;
        }
    } else {
        player->stateTimer = timer - 1;
    }

    if (player->velocity.y > 0) {
        player->rumbleTimer = 2;
        player->rumblePatternId = 3;
    }
}

void updateRacePlayerMode14PushHit(RacePlayer *player) {
    s16 updateState;
    s32 velocityX;
    s32 velocityZ;
    u32 stateFlags;
    s16 nextState;

    updateState = player->updateState;
    nextState = updateState + 1;
    if (updateState == 0) {
        player->updateState = nextState;
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x01006000;
        setRaceMotionAnimation(player, 0x12);
        player->stateTimer = 0xF;
        player->targetGroundOffset = 0;
    }
    stepRaceMotionLoopingAnimation(player);
    updateRacePlayerLeanAngle(player, 0x10000, 0);
    player->velocity.y += 0xFFFF6000;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    stateFlags = player->stateFlags;
    if ((stateFlags & 1) == 0) {
        player->stateFlags = stateFlags & ~0x200;
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0, 0, 0);
        player->unk582 = 0x100;
        player->unk584 = 8;
        player->unk588 = 0.0f;
    } else {
        player->stateFlags = stateFlags | 0x200;
    }
    velocityX = player->velocity.x;
    player->pos.x += velocityX;
    player->pos.y += player->velocity.y;
    velocityZ = player->velocity.z;
    player->pos.z += velocityZ;
    player->facingAngle = calculateFixedAngleFromDeltaXZ(velocityX, velocityZ);
    if (player->stateFlags & 0x400) {
        player->facingAngle += 0x800;
    }
    if (--player->stateTimer == 0) {
        player->mode = 0xC;
        player->updateState = 0;
        player->updateTimer = 0;
    }
    player->rumbleTimer = 4;
    player->rumblePatternId = 2;
}

void updateRacePlayerMode23ItemSteal(RacePlayer *player) {
    s16 updateState;
    s16 nextState;
    s32 timer;
    u32 stateFlags;

    updateState = player->updateState;
    nextState = updateState + 1;
    if (updateState == 0) {
        player->updateState = nextState;
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x102000;
        setRaceMotionAnimation(player, 0xE);
        player->stateTimer = 0x5A;
        player->targetGroundOffset = 0;
        stepRaceMotionLoopingAnimation(player);
        player->velocity.y = 0;
        createCallbackTaskWithUserIdPreservingArgs(initRaceUiItemStealTrailEffect, 0, 0x64, (u16)player->playerIndex);
        timer = player->stateTimer;
        /* Rank-scaled recovery timer for humans and CPUs. With zero-based rank,
         * integer truncation gives 60/53/45/38 updates for a base of 60. */
        player->stateTimer = timer - ((player->stateTimer * player->rankIndex) / 8);
        player->rumbleTimer = 4;
        player->rumblePatternId = 0;
    }

    stateFlags = player->stateFlags;
    if (!(stateFlags & 1)) {
        player->stateFlags = stateFlags & ~0x200;
    } else {
        player->stateFlags = stateFlags | 0x200;
    }

    updateRacePlayerLeanAngle(player, 0, 0);
    player->velocity.y -= 0xA000;
    player->pos.y += player->velocity.y;

    player->stateTimer -= getRaceInputTimerDecrementBonus(player) + 1;
    timer = player->stateTimer;
    if (timer < 0) {
        player->stateTimer = 0;
        timer = 0;
    }

    if (timer == 0) {
        player->mode = 4;
        player->updateState = 0;
        player->updateTimer = 0;
        player->unk2E0 = 0;
        player->unk2E4 = 0;
    }
}

void updateRacePlayerMode24HeavyKnockdown(RacePlayer *player) {
    s16 temp_v1_2;
    s16 temp_v0;
    s16 temp_2d6;
    s32 timer;
    s32 ret;
    u32 stateFlags;

    player->leanAngle = 0;
    updateRacePlayerLeanAngle(player, 0, 0);
    player->velocity.y += 0xFFFF6000;
    player->pos.y += player->velocity.y;

    stateFlags = player->stateFlags;
    if (!(stateFlags & 1)) {
        player->stateFlags = stateFlags & ~0x200;
    } else {
        player->stateFlags = stateFlags | 0x200;
    }

    temp_v1_2 = player->updateState;
    temp_v0 = temp_v1_2;
    switch (temp_v0 ^ 0) {
        case 0:
            player->updateState = temp_v1_2 + 1;
            player->stateFlags &= 0xFE0C1FFB;
            player->stateFlags |= 0x1202000;
            player->unk2D6 = 1;
            setRaceMotionAnimation(player, 1);
            player->stateTimer = 0x5A;
            player->targetGroundOffset = 0;
            stepRaceMotionLoopingAnimation(player);
            timer = player->stateTimer;
            /* Rank-scaled recovery timer for humans and CPUs. With zero-based rank,
             * integer truncation gives 60/53/45/38 updates for a base of 60. */
            player->stateTimer = (timer ^ 0) - ((timer * player->rankIndex) / 8);
            player->velocity.y = 0;
            player->rumbleTimer = 6;
            player->rumblePatternId = 0;
            /* fallthrough */
        case 1:
            temp_2d6 = player->unk2D6;
            if (temp_2d6 != 1) {
                player->unk2D6 = temp_2d6 - 1;
            }

            ret = getRaceInputTimerDecrementBonus(player);
            if (ret != 0) {
                player->unk2D6 = 3;
            }

            player->stateTimer -= ret + 1;
            timer = player->stateTimer;
            if (timer < 0) {
                player->stateTimer = 0;
                timer = 0;
            }

            if (timer == 0) {
                player->updateState += 1;
                player->unk2D6 = 1;
            }
            break;
        case 2:
            player->unk2D6 += 1;
            if (player->unk2D6 == 7) {
                player->unk2D6 = 2;
                player->updateState += 1;
            }
            break;
        case 3:
            player->unk2D6 += 1;
            if (player->unk2D6 == 8) {
                player->stateFlags &= 0xFE0C1FFB;
                player->mode = 0;
                player->updateState = 0;
                player->updateTimer = 0;
                player->unk2D4 = 0x3C;
            }
            break;
    }
}

void updateRacePlayerMode25SpinHit(RacePlayer *player) {
    s16 updateState;
    s32 stateTimer;
    u32 stateFlags;
    s16 nextState;

    updateState = player->updateState;
    nextState = updateState + 1;
    if (updateState == 0) {
        player->updateState = nextState;
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x400000;
        player->stateTimer = 0xF0;
        setRaceMotionAnimation(player, 1);
        stepRaceMotionLoopingAnimation(player);
        createCallbackTaskWithUserIdPreservingArgs(
            initRaceUiHeavyKnockdownTrailEffect,
            0,
            0x3C,
            (u16)player->playerIndex
        );
        stateTimer = player->stateTimer;
        player->stateTimer = stateTimer;
        /* Rank-scaled recovery timer for humans and CPUs. With zero-based rank,
         * integer truncation gives 60/53/45/38 updates for a base of 60. */
        player->stateTimer = player->stateTimer - ((stateTimer * player->rankIndex) / 8);
        player->rumbleTimer = 4;
        player->rumblePatternId = 0;
    }

    stateFlags = player->stateFlags;
    if (!(stateFlags & 1)) {
        player->stateFlags = stateFlags & ~0x200;
        updateRacePlayerLeanAngle(player, player->localVelocityZ, 0);
        player->velocity.y -= player->gravity;
        updateRacePlayerLocalVelocity(player, 0, 0, player->unk274, player->unk278, player->unk27C);
    } else {
        player->stateFlags = stateFlags | 0x200;
        player->velocity.y -= player->unk264;
        clampRacePlayerVectorXZSpeed(&player->velocity, player);
    }

    player->stateTimer -= getRaceInputTimerDecrementBonus(player) + 1;
    if (player->stateTimer < 0) {
        player->stateTimer = 0;
    }

    if (player->stateTimer == 0) {
        stateFlags = player->stateFlags;
        if (!(stateFlags & 0x200)) {
            player->stateFlags = stateFlags & 0xFE0C1FFB;
            player->mode = 0;
            player->updateState = 0;
            player->updateTimer = 0;
        } else {
            player->stateFlags = stateFlags & 0xFE0C1FFB;
            player->mode = 1;
            player->updateState = 0;
            player->updateTimer = 0;
        }
    }

    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;

    if (!(player->stateFlags & 1)) {
        updateRacePlayerMotionFeedback(player);
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80094940.s")

void updateRacePlayerMode40Stun(RacePlayer *player) {
    s16 updateState;
    s32 velocityX;
    s32 velocityZ;
    u32 stateFlags;
    s16 nextState;

    updateState = player->updateState;
    nextState = updateState + 1;
    if (updateState == 0) {
        player->updateState = nextState;
        player->stateFlags &= 0xFE0C1FFB;
        player->stateFlags |= 0x01006000;
        setRaceMotionAnimation(player, 0x12);
        player->stateTimer = 0x1E;
        player->stateTimer += randomNextMain() >> 4;
        player->targetGroundOffset = 0;
    }
    stepRaceMotionLoopingAnimation(player);
    updateRacePlayerLeanAngle(player, 0x10000, 0);
    player->velocity.y += 0xFFFF6000;
    clampRacePlayerVectorXZSpeed(&player->velocity, player);
    stateFlags = player->stateFlags;
    if ((stateFlags & 1) == 0) {
        player->stateFlags = stateFlags & ~0x200;
        updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0, 0, 0);
        player->stateFlags &= ~0x200;
        player->unk582 = 0x100;
        player->unk584 = 8;
        player->unk588 = 0.0f;
    } else {
        player->stateFlags = stateFlags | 0x200;
    }
    velocityX = player->velocity.x;
    player->pos.x += velocityX;
    player->pos.y += player->velocity.y;
    velocityZ = player->velocity.z;
    player->pos.z += velocityZ;
    player->facingAngle = calculateFixedAngleFromDeltaXZ(velocityX, velocityZ);
    if (player->stateFlags & 0x400) {
        player->facingAngle += 0x800;
    }
    if (--player->stateTimer == 0) {
        player->mode = 0xC;
        player->updateState = 0;
        player->updateTimer = 0;
    }
    player->rumbleTimer = 4;
    player->rumblePatternId = 2;
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80094C4C.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80094C84.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80094F0C.s")

void updateRacePlayerMode07LaunchRampTakeoff(RacePlayer *player) {
    s16 updateTimer;
    s32 unused;
    s32 scratch[14];

    updateTimer = player->updateTimer;
    if (updateTimer == 0) {
        player->updateTimer = updateTimer + 1;
        player->velocity.y = 0x30000;
        player->stateTimer = 0x1E;
        setRaceMotionAnimation(player, 4);
        if (player->stateFlags & 0x400) {
            player->unk80 = 0x20000;
        } else {
            player->unk80 = -0x20000;
        }
    }

    makeFixedRotationY((s16 *)&scratch[2], player->facingAngle);
    scratch[13] = 0;
    scratch[14] = 0;
    scratch[15] = player->unk80;
    transformVec3iByFixedMatrix((s16 *)&scratch[2], (Vec3i *)&scratch[13], (Vec3i *)&scratch[10]);
    player->pos.x += scratch[10];
    player->pos.z += scratch[12];

    makeFixedRotationY((s16 *)&scratch[2], player->facingAngle);
    scratch[13] = 0;
    scratch[14] = 0;
    scratch[15] = player->unk80;
    transformVec3iByFixedMatrix((s16 *)&scratch[2], (Vec3i *)&scratch[13], (Vec3i *)&scratch[10]);
    player->pos.x += scratch[10];
    player->pos.z += scratch[12];
    player->velocity.y -= 0x7000;
    player->pos.y += player->velocity.y;

    if ((player->velocity.y < 0) && !(player->stateFlags & 1)) {
        if (player->animationId != 1) {
            setRaceMotionAnimation(player, 1);
        }
        stepRaceMotionLoopingAnimation(player);
        updateRacePlayerMotionFeedback(player);
    } else {
        stepRaceMotionAnimationUntilEnd(player);
    }

    if (--player->stateTimer == 0) {
        player->updateState++;
        player->updateTimer = 0;
        player->stateTimer = 0x20;
        createCallbackTask((CallbackTaskCallback)initLaunchRampCourseObject, 0, 0x64);
        player->stateFlags &= ~0x20;
    }
}

void updateRacePlayerMode07LaunchRampSpin(RacePlayer *player) {
    s32 scratch[15];

    player->facingAngle += 0x10;
    if (player->unk80 > 0) {
        player->unk80 -= 0x820;
    } else {
        player->unk80 += 0x820;
    }

    makeFixedRotationY((s16 *)&scratch[1], player->facingAngle);
    scratch[12] = 0;
    scratch[13] = 0;
    scratch[14] = player->unk80;
    transformVec3iByFixedMatrix((s16 *)&scratch[1], (Vec3i *)&scratch[12], (Vec3i *)&scratch[9]);
    player->pos.x += scratch[9];
    player->pos.z += scratch[11];

    player->facingAngle += 0x10;
    if (player->unk80 > 0) {
        player->unk80 -= 0x820;
    } else {
        player->unk80 += 0x820;
    }

    makeFixedRotationY((s16 *)&scratch[1], player->facingAngle);
    scratch[12] = 0;
    scratch[13] = 0;
    scratch[14] = player->unk80;
    transformVec3iByFixedMatrix((s16 *)&scratch[1], (Vec3i *)&scratch[12], (Vec3i *)&scratch[9]);
    player->pos.x += scratch[9];
    player->pos.z += scratch[11];

    stepRaceMotionAnimationUntilEnd(player);
    updateRacePlayerMotionFeedback(player);
    if (--player->stateTimer == 0) {
        player->updateTimer = 0;
        player->updateState++;
    }
}

void updateRacePlayerMode07LaunchRampPose(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        setRaceMotionAnimation(player, 0x1D);
        player->stateTimer = 0x13;
    }
    stepRaceMotionAnimationUntilEnd(player);
    player->stateTimer--;
    if (player->stateTimer == 0) {
        player->updateTimer = 0;
        player->updateState++;
    }
}

void updateRacePlayerMode07LaunchRampDrift(RacePlayer *player) {
    s16 updateTimer;
    s32 scratch[14];

    updateTimer = player->updateTimer;
    if (updateTimer == 0) {
        player->updateTimer = updateTimer + 1;
        player->stateTimer = 0x13;
        if (player->stateFlags & 0x400) {
            player->unk80 = 0x40000;
        } else {
            player->unk80 = -0x40000;
        }
    }

    makeFixedRotationY((s16 *)&scratch[1], player->facingAngle);
    scratch[12] = 0;
    scratch[13] = 0;
    scratch[14] = player->unk80;
    transformVec3iByFixedMatrix((s16 *)&scratch[1], (Vec3i *)&scratch[12], (Vec3i *)&scratch[9]);
    player->pos.x += scratch[9];
    player->pos.z += scratch[11];

    player->stateTimer--;
    if (player->stateTimer == 0) {
        player->updateTimer = 0;
        player->updateState++;
    }
    stepRaceMotionAnimationUntilEnd(player);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_800953D4.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80095530.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/tryStartRacePlayerCourseObjectMode.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80095AC8.s")

void updateRacePlayerMode30AttackApproach(RacePlayer *player) {
    s16 updateTimer;
    s16 facingAngle;
    s16 angleDiff;
    s16 roll;

    updateTimer = player->updateTimer;
    if (updateTimer == 0) {
        player->updateTimer = updateTimer + 1;
        player->subState = 0;
        setRaceCameraMode(player->playerIndex, 5);
        if (player->movementSpeed >= 0x20001) {
            player->unk306 = 1;
            player->unk31E = calculateFixedAngleFromDeltaXZ(player->velocity.x, player->velocity.z);
            if (player->stateFlags & 0x400) {
                player->unk31E += 0x800;
            }

            if ((s16)(angleDiff = 0x800 & (player->unk31E - player->facingAngle)) < 0x800) {
                player->unk31C = 0;
                if (player->stateFlags & 0x400) {
                    player->unk31C = 1;
                }
                player->unk31E -= 0x400;
            } else {
                player->unk31C = 1;
                if (player->stateFlags & 0x400) {
                    player->unk31C = 0;
                }
                player->unk31E += 0x400;
            }
            player->stateTimer = 0;
        } else {
            player->unk306 = 0;
            setRaceMotionAnimation(player, 1);
        }
    }

    if (player->unk306 != 0) {
        facingAngle = player->facingAngle;
        angleDiff = (player->unk31E - facingAngle) & 0xFFF;
        if (angleDiff >= 0x801) {
            angleDiff -= 0x1000;
        }
        if (angleDiff >= 0x41) {
            angleDiff = 0x40;
        }
        if (angleDiff < -0x40) {
            angleDiff = -0x40;
        }
        player->facingAngle = facingAngle + angleDiff;

        if (player->unk31C != 0) {
            player->stateTimer = 0x1F;
        } else {
            player->stateTimer = -0x1F;
        }

        player->leanScale = 0x3F;
        angleDiff = updateRacePlayerLeanAngle(player, player->movementSpeed, player->stateTimer) - player->unk2FA;
        roll = player->unk2FA;
        if (angleDiff >= 0x31) {
            angleDiff = 0x30;
        }
        if (angleDiff < -0x30) {
            angleDiff = -0x30;
        }
        player->unk2FA = player->unk2FA + angleDiff;

        if (player->unk2FA == 0) {
            if (player->animationId != 1) {
                setRaceMotionAnimation(player, 1);
            }
            stepRaceMotionLoopingJointAnimation(player);
        } else if (player->unk2FA >= 0) {
            setRaceMotionAnimation(player, 3);
            blendRaceMotionJointAnimation(player, 3, player->unk2FA, 0x118);
        } else {
            setRaceMotionAnimation(player, 2);
            blendRaceMotionJointAnimation(player, 2, -player->unk2FA, 0x118);
        }
    } else {
        updateRacePlayerLeanAngle(player, 0, 0);
        stepRaceMotionLoopingJointAnimation(player);
    }

    player->velocity.y -= player->gravity;
    updateRacePlayerLocalVelocityNoVerticalOffset(player, 0, 0x6000, 0x6000, 0x6000);
    player->pos.x += player->velocity.x;
    player->pos.y += player->velocity.y;
    player->pos.z += player->velocity.z;
    updateRacePlayerMotionFeedback(player);

    if ((player->velocity.x == 0) && (player->velocity.z == 0) && !(player->stateFlags & 1)) {
        if (player->soundDisabled == 0) {
            player->updateState++;
            player->stateTimer = 0x1E;
            if (player->animationId != 1) {
                setRaceMotionAnimation(player, 1);
            }
        }
    }
}

void updateRacePlayerMode30AttackResolve(RacePlayer *player) {
    if (gRaceResultState != 0) {
        player->stateTimer = 0;
    }
    if (player->stateTimer == 0) {
        if (gRaceSplitscreenMode == 0) {
            if (player->rankIndex == 0) {
                player->updateState = 2;
                player->updateTimer = 0;
            } else {
                player->updateState = 3;
                player->updateTimer = 0;
            }
        } else if (gRaceResultState == 1) {
            player->updateState = 2;
            player->updateTimer = 0;
        } else {
            player->updateState = 3;
            player->updateTimer = 0;
        }
    }
    updateRacePlayerLeanAngle(player, 0, 0);
    stepRaceMotionLoopingJointAnimation(player);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80095F2C.s")

void updateRacePlayerMode32Character0(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x23);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->animationId == 0x24) {
            player->stateFlags |= 0x8000000;
        }
        if (player->stateTimer == 0) {
            if (player->updateTimer == 1) {
                enqueueRacePlayerVoiceSound(player, 6);
                player->updateTimer++;
            }
            player->stateTimer = 0x14;
            setRaceMotionAnimation(player, 0x24);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode32Character1(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x24);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->animationId == 0x25) {
            player->stateFlags |= 0x8000000;
        }
        if (player->stateTimer == 0) {
            if (player->updateTimer == 1) {
                enqueueRacePlayerVoiceSound(player, 6);
                player->updateTimer++;
            }
            setRaceMotionAnimation(player, 0x25);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode32Character2(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x23);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->animationId == 0x24) {
            player->stateFlags |= 0x8000000;
        }
        if (player->stateTimer == 0) {
            if (player->updateTimer == 1) {
                enqueueRacePlayerVoiceSound(player, 6);
                player->updateTimer++;
            }
            player->stateTimer = 0x14;
            setRaceMotionAnimation(player, 0x24);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode32Character3(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x24);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->animationId == 0x25) {
            player->stateFlags |= 0x8000000;
        }
        if (player->stateTimer == 0) {
            if (player->updateTimer == 1) {
                enqueueRacePlayerVoiceSound(player, 6);
                player->updateTimer++;
            }
            player->stateTimer = 0;
            setRaceMotionAnimation(player, 0x25);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode32Character4(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x24);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->animationId == 0x25) {
            player->stateFlags |= 0x8000000;
        }
        if (player->stateTimer == 0) {
            if (player->updateTimer == 1) {
                enqueueRacePlayerVoiceSound(player, 6);
                player->updateTimer++;
            }
            player->stateTimer = 0x32;
            setRaceMotionAnimation(player, 0x25);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode32Character5(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        enqueueRacePlayerVoiceSound(player, 6);
        player->stateTimer = 0x1E;
        setRaceMotionAnimation(player, 0x23);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        player->stateFlags |= 0x8000000;
        if (player->stateTimer == 0) {
            player->stateTimer = 0x1E;
            setRaceMotionAnimation(player, 0x23);
        } else {
            player->stateTimer--;
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_800963D8.s")

void updateRacePlayerMode35Character0(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x25);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->stateTimer == 0) {
            player->stateFlags |= 0x8000000;
            player->stateTimer = 0x1E;
            setRaceMotionAnimation(player, 0x26);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode35Character1(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x26);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->stateTimer == 0) {
            player->stateFlags |= 0x8000000;
            player->stateTimer = 0x1E;
            setRaceMotionAnimation(player, 0x27);
        } else {
            player->stateTimer--;
        }
    }
}

void updateRacePlayerMode35Character2(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x25);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->stateTimer == 0) {
            player->stateTimer = 0;
            player->stateFlags |= 0x8000000;
            setRaceMotionAnimation(player, 0x26);
        } else {
            player->stateTimer--;
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_800965C8.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80096650.s")

void updateRacePlayerMode35Character5(RacePlayer *player) {
    if (player->updateTimer == 0) {
        player->updateTimer++;
        player->stateTimer = 0;
        setRaceMotionAnimation(player, 0x24);
    }
    if (stepRaceMotionAnimationUntilEnd(player)) {
        if (player->stateTimer == 0) {
            if (player->animationId == 0x25) {
                player->stateTimer = 0;
                setRaceMotionAnimation(player, 0x26);
            } else {
                player->stateFlags |= 0x8000000;
                player->stateTimer = 0x1E;
                setRaceMotionAnimation(player, 0x25);
            }
        } else {
            player->stateTimer--;
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/updateRacePlayersPostUpdate.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_80096984.s")

void updateRacePlayerPostUpdateNoop(RacePlayer *player) {
    updateRacePlayerSurfaceContact(player);
}

void updateRacePlayerPostUpdateCourseObject(RacePlayer *player) {
    if (player->updateState < 7) {
        updateRacePlayerGroundAlignment(player);
    } else {
        player->groundCorrectionY = 0;
        updateRacePlayerProjectedPosition(player);
        player->groundOffset = 0x30000;
    }
}

void updateRacePlayerPostUpdateMode00(RacePlayer *player) {
    if (updateRacePlayerSurfaceContact(player)) {
        player->mode = 0x16;
        player->updateState = 0;
        player->updateTimer = 0;
        player->stateFlags |= 0x200;
    } else if (tryStartRacePlayerCourseObjectMode(player->playerIndex)) {
        player->mode = 7;
        player->updateState = 0;
        player->updateTimer = 0;
    }
    interpolateRaceMotionJointAnimationFrame(player, 0, (0x60000 - player->groundOffset) / 0x600, 0x100);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/updateRacePlayerVoiceSounds.s")

void updateRacePlayerLoopingSound(RacePlayer *player) {
    s32 v0 = -player->velocity.y >> 13;
    if (v0 >= 0x80) {
        v0 = 0x7F;
    }
    if (v0 < 0x46) {
        v0 = 0x46;
    }
    if (v0 >= 0x51) {
        player->rumbleTimer = 2;
        player->rumblePatternId = 0;
    } else {
        player->rumbleTimer = 1;
        player->rumblePatternId = 0;
    }
    if (player->soundDisabled == 0) {
        enqueuePositionalSoundEffect(0x21, (Vec3i *)&player->pos.x, (s16)v0, 0x32);
    }
}

void updateRacePlayerPostUpdateMode22(RacePlayer *player) {
    u32 stateFlags;

    if (updateRacePlayerSurfaceContact(player) == 0) {
        stateFlags = player->stateFlags;
        if (stateFlags & 8) {
            player->stateFlags = stateFlags & ~8;
        } else if (stateFlags & 0x800) {
            player->trickAttackMakeCount = 0;
            player->mode = 0xE;
            player->updateState = 0;
            player->updateTimer = 0;
            player->stateFlags = stateFlags & ~0x200;
            enqueueRacePlayerVoiceSound(player, 2);
        } else {
            player->stateFlags = stateFlags & ~0x200;
            if (tryStartRacePlayerCourseObjectMode(player->playerIndex) != 0) {
                player->mode = 7;
                player->updateState = 0;
                player->updateTimer = 0;
                updateRacePlayerVoiceSounds(player);
                updateRacePlayerLoopingSound(player);
            } else if (player->subState == 0) {
                player->mode = 0;
                player->updateState = 1;
                player->updateTimer = 0;
                player->unk57A = 2;
                updateRacePlayerLoopingSound(player);
            } else {
                player->mode = 0;
                player->updateState = 0;
                player->updateTimer = 0;
                player->unk57A = 2;
                updateRacePlayerVoiceSounds(player);
                updateRacePlayerLoopingSound(player);
            }
        }
    }
}

void updateRacePlayerPostUpdateAirborneTrick(RacePlayer *player) {
    u32 stateFlags;

    if (updateRacePlayerSurfaceContact(player) == 0) {
        stateFlags = player->stateFlags;
        if (stateFlags & 8) {
            player->stateFlags = stateFlags & ~8;
        } else if (stateFlags & 0x800) {
            player->trickAttackMakeCount = 0;
            player->mode = 0xE;
            player->updateState = 0;
            player->updateTimer = 0;
            player->stateFlags = stateFlags & ~0x200;
            enqueueRacePlayerVoiceSound(player, 2);
        } else {
            player->stateFlags = stateFlags & ~0x200;
            if (tryStartRacePlayerCourseObjectMode(player->playerIndex) != 0) {
                player->mode = 7;
                player->updateState = 0;
                player->updateTimer = 0;
                updateRacePlayerVoiceSounds(player);
                updateRacePlayerLoopingSound(player);
            } else {
                player->mode = 0;
                player->updateState = 0;
                player->updateTimer = 0;
                player->unk57A = 2;
                updateRacePlayerVoiceSounds(player);
                updateRacePlayerLoopingSound(player);
            }
        }
    }
}

void updateRacePlayerPostUpdateAirborneLaunch(RacePlayer *player) {
    if (updateRacePlayerSurfaceContact(player)) {
        player->mode = 0x16;
        player->updateState = 0;
        player->updateTimer = 0;
        player->stateFlags |= 0x200;
    } else if (tryStartRacePlayerCourseObjectMode(player->playerIndex)) {
        player->mode = 7;
        player->updateState = 0;
        player->updateTimer = 0;
    }
    interpolateRaceMotionJointAnimationFrame(player, 0, (0x60000 - player->groundOffset) / 0x600, 0x100);
}

void updateRacePlayerPostUpdateNudge(RacePlayer *player) {
    if (updateRacePlayerSurfaceContact(player)) {
        player->stateFlags |= 0x200;
    } else {
        player->stateFlags &= ~0x200;
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/race/player/race_player_update/func_8009725C.s")

void updateRacePlayerPostUpdateAttack(RacePlayer *player) {
    updateRacePlayerSurfaceContact(player);
    if (player->updateState < 2) {
        interpolateRaceMotionJointAnimationFrame(player, 0, (0x60000 - player->groundOffset) / 0x600, 0x100);
    }
}
