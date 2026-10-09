#include "game/engine/model_asset_symbols.h"
#include "game/race/course/race_course_effects.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/race_state.h"
#include "common.h"
#include <PR/os_libc.h>
#include "game/save_data.h"
#include "game/audio/sound_manager.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/render_callback.h"
#include "game/engine/system_runtime.h"
#include "game/engine/controller_input.h"
#include "assets.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/effects/snowboard_trail_effects.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/race/items/race_item_hit_flags.h"
#include "game/engine/viewport_manager.h"
#include "game/math/spatial_math.h"
#include "game/math/fixed_point_math.h"
#include "game/race/motion/race_motion.h"
#include "game/race/items/race_item_projectiles.h"
#include "game/race/player/race_player_movement.h"
#include "game/race/player/race_player_input.h"
#include "game/race/camera/race_camera.h"
#include "game/race/ui/race_hud.h"
#include <PR/os_cache.h>

extern u8 gRaceUpdatePaused;

void renderRaceUiProjectile(RaceUiProjectileActor *arg0) {
    volatile s32 pad0;
    Transform3D sp7C;
    volatile u8 padding[4];
    s32 sp74;
    Gfx *temp_v0_2;
    Gfx *temp_v0_3;
    Gfx *temp_v0_4;
    Gfx *temp_v0_5;
    Gfx *temp_v0_6;
    Gfx *temp_v0_7;
    Gfx *temp_v0_8;
    Gfx *temp_v0_9;
    Gfx *temp_v0_10;
    Gfx *temp_v0_11;
    Gfx *temp_v0_12;
    Gfx *temp_v0_13;
    Gfx *temp_v0_14;
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;
    Gfx *temp_t4;
    s32 temp_t0;
    s32 var_ra;
    s32 var_t5;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = (float) 1;
    }

    if (arg0->matrixDirty != 0) {
        arg0->matrixDirty = 0;
        sp7C = gIdentityFixedTransform;
        sp7C.translation.x = arg0->pos.x;
        sp7C.translation.y = arg0->pos.y;
        sp7C.translation.z = arg0->pos.z;
        arg0->matrix = allocFixedTransformMatrix(&sp7C);
    }

    if (arg0->matrix != NULL) {
        var_ra = 0x20;
        if (arg0->flags & 4) {
            var_ra = 0x40;
            sp74 = 4;
        } else {
            sp74 = 0;
            do { } while (0);
        }
        // clang-format off
        do { \
            temp_v0_2 = gRegionAllocPtr++; \
            temp_v0_2->words.w0 = 0x06000000; \
            temp_v0_2->words.w1 = (u32)gAlphaSpriteRenderModeDl; \
            temp_v0_3 = gRegionAllocPtr++; \
            temp_v0_3->words.w0 = 0xFD500000; \
            temp_v0_3->words.w1 = (u32)arg0->image; \
            temp_v0_4 = gRegionAllocPtr++; \
            temp_v0_4->words.w0 = 0xF5500000; \
            temp_v0_4->words.w1 = 0x07080200; \
            temp_v0_5 = gRegionAllocPtr++; \
            temp_v0_5->words.w0 = 0xE6000000; \
            temp_v0_5->words.w1 = 0; \
            temp_t4 = gRegionAllocPtr++; \
            temp_t4->words.w0 = 0xF3000000; \
            temp_t0 = (((var_ra << 5) + 3) >> 2) - 1; \
            if (temp_t0 < 0x7FF) { \
                var_t5 = temp_t0; \
            } else { \
                var_t5 = 0x7FF; \
            } \
            temp_t4->words.w1 = (((var_t5 & 0xFFF) << 0xC) | 0x07000000) | 0x400; \
            temp_v0_6 = gRegionAllocPtr++; \
            temp_v0_6->words.w1 = 0; \
            temp_v0_6->words.w0 = 0xE7000000; \
            temp_v0_7 = gRegionAllocPtr++; \
            temp_v0_7->words.w0 = 0xF5400400; \
            temp_v0_7->words.w1 = 0x00080200; \
            temp_v0_8 = gRegionAllocPtr++; \
            temp_v0_8->words.w0 = 0xF2000000; \
            temp_v0_8->words.w1 = (((var_ra - 1) << 2) & 0xFFF) | 0x0007C000; \
            temp_v0_9 = gRegionAllocPtr++; \
            temp_v0_9->words.w0 = 0xFD100000; \
            temp_v0_9->words.w1 = (u32)arg0->palette; \
            temp_v0_10 = gRegionAllocPtr++; \
            temp_v0_10->words.w1 = 0; \
            temp_v0_10->words.w0 = 0xE8000000; \
            temp_v0_11 = gRegionAllocPtr++; \
            temp_v0_11->words.w0 = 0xF5000100; \
            temp_v0_11->words.w1 = 0x07000000; \
            temp_v0_12 = gRegionAllocPtr++; \
            temp_v0_12->words.w1 = 0; \
            temp_v0_12->words.w0 = 0xE6000000; \
            temp_v0_13 = gRegionAllocPtr++; \
            temp_v0_13->words.w0 = 0xF0000000; \
            temp_v0_13->words.w1 = 0x0703C000; \
            temp_v0_14 = gRegionAllocPtr++; \
            temp_v0_14->words.w1 = 0; \
            temp_v0_14->words.w0 = 0xE7000000; \
            temp_v0_17 = gRegionAllocPtr++; \
            temp_v0_17->words.w0 = 0x01020040; \
            temp_v0_17->words.w1 = (u32)arg0->matrix; \
            temp_v0_18 = gRegionAllocPtr++; \
            temp_v0_18->words.w0 = 0x01000040; \
            temp_v0_18->words.w1 = (u32)gViewportMatrix; \
            temp_v0_2 = gRegionAllocPtr++; \
            temp_v0_2->words.w0 = 0x0400103F; \
            temp_v0_2->words.w1 = (u32)(&D_800D64A0[sp74]); \
            temp_v0_3 = gRegionAllocPtr++; \
            temp_v0_3->words.w0 = 0xB1060402; \
            temp_v0_3->words.w1 = 0x00060200; \
        } while (0);
        // clang-format on
    }
}

void updateRaceUiProjectileFlyAway(RaceUiProjectileActor *arg0) {
    struct {
        s16 mtx[0x10];
        s32 pad;
    } sp24;
    RaceUiProjectileActor *actor;
    s32 temp;

    actor = arg0;
    if (!gRaceUpdatePaused) {
        makeFixedRotationXY(sp24.mtx, gRacePlayers[actor->task.userId].pitchAngle, gRacePlayers[actor->task.userId].facingAngle);
        transformVec3iByFixedMatrix(sp24.mtx, &actor->velocity, &actor->pos);
        actor->pos.x += gRacePlayers[actor->task.userId].unk28.x;
        actor->pos.y += gRacePlayers[actor->task.userId].unk28.y + actor->verticalVelocity;
        actor->pos.z += gRacePlayers[actor->task.userId].unk28.z;
        temp = (actor->verticalVelocity += actor->verticalAcceleration);
        actor->verticalAcceleration += 0x10000;
        if (temp >= 0x300001) {
            removeCallbackTask(actor);
            return;
        }
    }
    addRenderCallback(&D_801248C8, (RenderCallback)renderRaceUiProjectile, actor);
}

void updateRaceUiProjectileStealScores(RaceUiProjectileActor *arg0) {
    RacePlayer *player;
    RacePlayer *otherPlayer;
    s16 sp44[0xE];
    s32 value;
    s32 amount;
    s32 i;
    s16 flags;

    if (gRaceUpdatePaused == 0) {
        player = &gRacePlayers[arg0->task.userId];
        makeFixedRotationXY(sp44, gRacePlayers[arg0->task.userId].pitchAngle, gRacePlayers[arg0->task.userId].facingAngle);
        transformVec3iByFixedMatrix(sp44, &arg0->velocity, &arg0->pos);

        player = &gRacePlayers[arg0->task.userId];
        arg0->pos.x += player->unk28.x;
        arg0->pos.y += player->unk28.y;
        arg0->pos.z += player->unk28.z;
        advanceRaceUiProjectileAnimation(arg0);

        i = 0;
        do {
            otherPlayer = &gRacePlayers[i];
            if ((otherPlayer->isActive != 0) && (otherPlayer->playerIndex != arg0->task.userId)) {
                value = otherPlayer->score;
                amount = (value >= 0xA6) ? 0xA6 : value;
                otherPlayer->score = value - amount;
                addRacePlayerScore(&gRacePlayers[arg0->task.userId], amount);
            }
            i++;
        } while (i < RACE_PLAYER_COUNT);

        flags = arg0->flags;
        amount = flags & 1;
        if (flags & 8) {
            enqueuePositionalSoundEffect(0x69, &gRacePlayers[arg0->task.userId].unk28, 0x7F, 0x32);
            arg0->flags &= ~8;
            amount = 1;
            amount = (flags = arg0->flags) & amount;
        }
        if (amount) {
            arg0->verticalAcceleration = 0;
            arg0->verticalVelocity = 0;
            enqueuePositionalSoundEffect(0x6A, &gRacePlayers[arg0->task.userId].unk28, 0x7F, 0x32);
            setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiProjectileFlyAway);
        }
    }

    addRenderCallback(&D_801248C8, (RenderCallback)renderRaceUiProjectile, arg0);
}

void updateRaceUiProjectileBounceDown(RaceUiProjectileActor *arg0) {
    s16 sp2C[0x12];
    RacePlayer *player;
    RaceUiProjectileActor *actor;

    actor = arg0;
    if (gRaceUpdatePaused == 0) {
        player = &gRacePlayers[actor->task.userId];
        arg0 += 0;
        if (((!actor) && (!actor)) && (!actor)) {}
        player = &gRacePlayers[actor->task.userId];
        makeFixedRotationXY(sp2C, player->pitchAngle, player->facingAngle);
        transformVec3iByFixedMatrix(sp2C, &actor->velocity, &actor->pos);

        player = &gRacePlayers[actor->task.userId];
        actor->pos.x += player->unk28.x;
        actor->pos.y += player->unk28.y + actor->verticalVelocity;
        actor->pos.z += player->unk28.z;
        actor->verticalAcceleration += 0xFFFF0000;
        actor->verticalVelocity += actor->verticalAcceleration;
        if (actor->verticalVelocity <= 0) {
            actor->verticalVelocity = 0;
            enqueuePositionalSoundEffect(0x68, &player->unk28, 0x7F, 0x32);
            advanceRaceUiProjectileAnimation(actor);
            setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiProjectileStealScores);
        }
    }
    addRenderCallback(&D_801248C8, (RenderCallback)renderRaceUiProjectile, actor);
}
