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
extern Vtx D_800D6230[];

void updateRaceUiSpinHitTransitionEffect(RaceUiTransitionActor *arg0) {
    RacePlayer *player;
    s16 temp_v0;
    s16 temp_v1;
    s16 temp_a0;

    if (gRaceUpdatePaused == 0) {
        if (!(gRacePlayers[arg0->task.userId].stateFlags & 0x800000)) {
            arg0->unk6C = 0;
        }
        if (arg0->unk6C != 0) {
            player = &gRacePlayers[arg0->task.userId];
            arg0->transformSource.translation.x = player->modelPartTransforms[1].translation.x;
            arg0->transformSource.translation.y = player->modelPartTransforms[1].translation.y;
            arg0->transformSource.translation.z = player->modelPartTransforms[1].translation.z;
        } else {
            if (arg0->unk6E == 0) {
                removeCallbackTask(arg0);
                return;
            }
            temp_v0 = arg0->transformSource.rotation[1];
            temp_v1 = arg0->transformSource.rotation[4];
            temp_a0 = arg0->transformSource.rotation[7];
            arg0->unk68 -= 0x1000;
            arg0->transformSource.translation.y += arg0->unk68;
            arg0->transformSource.rotation[1] = temp_v0 - (temp_v0 / 16);
            arg0->transformSource.rotation[4] = temp_v1 - (temp_v1 / 16);
            arg0->transformSource.rotation[7] = temp_a0 - (temp_a0 / 16);
        }
    }
    addRenderCallback(&gRaceModelEffectRenderCallbackList, (RenderCallback)renderRaceUiSpinHitTransitionEffect, arg0);
}

void initRaceUiSpinHitTransitionEffect(RaceUiTransitionActor *arg0) {
    s16 *transform = arg0->transformSource.rotation;
    volatile u8 padding[0x20];

    arg0->unk68 = 0;
    makeFixedRotationX(transform, 0x400);
    scaleFixedMatrix3sByQuarter(transform);
    arg0->unk6C = 1;
    arg0->unk6E = 0xF;
    enqueuePositionalSoundEffect(0xB, &gRacePlayers[arg0->task.userId].pos, 0x7F, 0x32);
    updateRaceUiSpinHitTransitionEffect(arg0);
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiSpinHitTransitionEffect);
}

void renderRaceUiStunOrbitingIcon(RaceUiOrbitingSpriteActor *arg0) {
    volatile u8 padding[4];
    Transform3D sp64;
    Gfx *temp_v0;
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
    Gfx *temp_v0_15;
    Gfx *temp_v0_16;
    Gfx *temp_v0_17;
    Gfx *temp_v0_18;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (arg0->matrixDirty != 0) {
        arg0->matrixDirty = 0;
        sp64 = gIdentityFixedTransform;
        sp64.translation.x = arg0->pos.x;
        sp64.translation.y = arg0->pos.y;
        sp64.translation.z = arg0->pos.z;
        arg0->matrix = allocFixedTransformMatrix(&sp64);
    }

    // clang-format off
    do { \
        if (arg0->matrix != NULL) { \
            temp_v0 = gRegionAllocPtr++; \
            temp_v0->words.w0 = 0x06000000; \
            temp_v0->words.w1 = (u32)gAlphaSpriteRenderModeDl; \
            temp_v0_2 = gRegionAllocPtr++; \
            temp_v0_2->words.w0 = 0xFD500000; \
            temp_v0_2->words.w1 = (u32)arg0->image; \
            temp_v0_3 = gRegionAllocPtr++; \
            temp_v0_3->words.w0 = 0xF5500000; \
            temp_v0_3->words.w1 = 0x07080200; \
            temp_v0_4 = gRegionAllocPtr++; \
            temp_v0_4->words.w1 = 0; \
            temp_v0_4->words.w0 = 0xE6000000; \
            temp_v0_5 = gRegionAllocPtr++; \
            temp_v0_5->words.w0 = 0xF3000000; \
            temp_v0_5->words.w1 = 0x0703F800; \
            temp_v0_6 = gRegionAllocPtr++; \
            temp_v0_6->words.w1 = 0; \
            temp_v0_6->words.w0 = 0xE7000000; \
            temp_v0_7 = gRegionAllocPtr++; \
            temp_v0_7->words.w0 = 0xF5400200; \
            temp_v0_7->words.w1 = 0x00080200; \
            temp_v0_8 = gRegionAllocPtr++; \
            temp_v0_8->words.w0 = 0xF2000000; \
            temp_v0_8->words.w1 = 0x0003C03C; \
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
            temp_v0_15 = gRegionAllocPtr++; \
            temp_v0_15->words.w0 = 0x01020040; \
            temp_v0_15->words.w1 = (u32)arg0->matrix; \
            temp_v0_16 = gRegionAllocPtr++; \
            temp_v0_16->words.w0 = 0x01000040; \
            temp_v0_16->words.w1 = (u32)gViewportMatrix; \
            temp_v0_17 = gRegionAllocPtr++; \
            temp_v0_17->words.w0 = 0x0400103F; \
            temp_v0_17->words.w1 = (u32)D_800D6230; \
            temp_v0_18 = gRegionAllocPtr++; \
            temp_v0_18->words.w0 = 0xB1060402; \
            temp_v0_18->words.w1 = 0x00060200; \
        } \
    } while (0);
    // clang-format on
}
