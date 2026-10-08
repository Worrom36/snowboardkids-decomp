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

extern Vtx D_800D63D0[];

void renderSilverMountainSnowParticles(RaceUiEffectParticleActor *arg0) {
    RaceUiEffectParticleActor *actor;
    s32 cameraX;
    s32 cameraY;
    s32 cameraZ;
    union {
        Transform3D transform;
        s64 forceAlignment;
    } transform;
    s32 i;
    Mtx *matrix;

    actor = arg0;
    gDPLoadTextureBlock_4b(
        gRegionAllocPtr++,
        actor->unk20,
        G_IM_FMT_CI,
        32,
        32,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        0,
        0,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, actor->unk1C);
    gSPDisplayList(gRegionAllocPtr++, gAlphaSpriteRenderModeDl);

    transform.transform = gIdentityFixedTransform;
    cameraX = -gRaceCameras[gCurrentViewportIndex].cameraTransform.translation.x;
    cameraY = -gRaceCameras[gCurrentViewportIndex].cameraTransform.translation.y;
    cameraZ = -gRaceCameras[gCurrentViewportIndex].cameraTransform.translation.z;

    i = 0;
    if (actor->count > 0) {
        do {
            transform.transform.translation.x =
                ((actor->particles[i].unk0 - (cameraX & 0xFFFFFF)) & 0xFFFFFF) + cameraX + 0xFF800000;
            transform.transform.translation.y =
                ((actor->particles[i].unk4 - (cameraY & 0xFFFFFF)) & 0xFFFFFF) + cameraY + 0xFF800000;
            transform.transform.translation.z =
                ((actor->particles[i].unk8 - (cameraZ & 0xFFFFFF)) & 0xFFFFFF) + cameraZ + 0xFF800000;
            matrix = allocFixedTransformMatrix(&transform.transform);
            if (matrix != NULL) {
                gSPMatrix(gRegionAllocPtr++, matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPMatrix(gRegionAllocPtr++, gViewportMatrix, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
                gSPVertex(gRegionAllocPtr++, D_800D63D0, 3, 0);
                gSP1Triangle(gRegionAllocPtr++, 0, 2, 1, 0);
                actor = arg0;
            }
            i++;
        } while (i < actor->count);
    }
}

void updateSilverMountainSnowParticles(RaceUiEffectParticleActor *arg0) {
    register RaceUiEffectParticleActor *actor;
    register s32 i;

    actor = arg0;
    for (i = 0; i < actor->count; i++) {
        actor->particles[i].unk0 += 0xA0000;
        if (actor->particles[i].unk0 >= 0x1000000) {
            actor->particles[i].unk8 = randomNextMain() << 0x10;
            actor->particles[i].unk4 = randomNextMain() << 0x10;
            actor->particles[i].unk0 = 0;
        }
        actor->particles[i].unk4 += 0xFFFD0000;
        actor->particles[i].unk4 &= 0xFFFFFF;
    }
    addRenderCallback(&gEffectRenderCallbackList, (RenderCallback)renderSilverMountainSnowParticles, (void *)actor);
}

void initSilverMountainSnowParticles(RaceUiEffectParticleActor *actor) {
    s32 i;

    if (gPlayerCount != 0) {
        actor->count = 0xF;
    } else {
        actor->count = 0x3C;
    }

    getAssetTableImageAndPalette(getRelocatableHeapBlockBase(gAssetHandles[0x1C]), 0x42, &actor->unk20, &actor->unk1C);
    gAssetHandles[0x25] = allocRelocatableHeapBlock(actor->count * sizeof(RaceUiEffectParticle));
    actor->particles = (RaceUiEffectParticle *)getRelocatableHeapBlockBase(gAssetHandles[0x25]);

    for (i = 0; i < actor->count; i++) {
        actor->particles[i].unk0 = randomNextMain() << 0x10;
        actor->particles[i].unk4 = randomNextMain() << 0x10;
        actor->particles[i].unk8 = randomNextMain() << 0x10;
    }

    setCallbackTaskCallback(actor, (CallbackTaskCallback)updateSilverMountainSnowParticles);
}
