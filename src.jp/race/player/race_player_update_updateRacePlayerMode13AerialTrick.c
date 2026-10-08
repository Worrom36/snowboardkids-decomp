#include "game/race/items/race_items.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/race_state.h"
#include "common.h"
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
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerYellowTrickSparkles, 5, 2, (u16)player->playerIndex);
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
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerYellowTrickSparkles, 5, 2, (u16)player->playerIndex);
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
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerYellowTrickSparkles, 5, 2, (u16)player->playerIndex);
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
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerYellowTrickSparkles, 5, 2, (u16)player->playerIndex);
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
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerYellowTrickSparkles, 5, 2, (u16)player->playerIndex);
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
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerYellowTrickSparkles, 5, 2, (u16)player->playerIndex);
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
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerYellowTrickSparkles, 5, 2, (u16)player->playerIndex);
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
            createCallbackTaskWithUserIdPreservingArgs(initRacePlayerYellowTrickSparkles, 5, 2, (u16)player->playerIndex);
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
            playerAlias->pos.x, player->pos.z, spawn->startSurfaceTargetX, spawn->startSurfaceTargetZ
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
