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

#define ASSET_HANDLE(index) (gAssetHandles[(index)])

extern u8 gRaceUpdatePaused;

void renderRaceUiThrownTrailObstacle(RaceUiThrownTrailActor *arg0) {
    volatile s32 padlow;
    s32 sine;
    Transform3D sp80;
    volatile s32 pad[0x12];

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            sine = fixedSine(arg0->spin);
            makeFixedRotationY(sp80.rotation, (s16)(arg0->angle + 0x800));
            sp80.translation.x = arg0->pos.x;
            sp80.translation.y = arg0->pos.y + ((sine + 0x1000) << 5) + 0x10000;
            sp80.translation.z = arg0->pos.z;
            scaleFixedMatrix3sByQuarter(sp80.rotation);
            arg0->matrix = allocFixedTransformMatrix(&sp80);
        }

        if (arg0->matrix != NULL) {
            gDPPipeSync(gRegionAllocPtr++);
            gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0xA)));
            gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0xB)));
            gSPMatrix(gRegionAllocPtr++, arg0->matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(gRegionAllocPtr++, &_ADDR_20019C0_VRAM);
        }
    }
}

void updateRaceUiThrownTrailObstacle(RaceUiThrownTrailActor *arg0) {
    volatile s32 pad[4];
    volatile s16 unused0;
    volatile s16 unused1;
    Vec3i *pos;

    if (gRaceUpdatePaused == 0) {
        pos = &arg0->pos;
        if (isPositionNearAnyRaceViewportFocus(pos) != 0) {
            pushRacePlayersOutOfCylinderAndApplyItemHit(pos, 0x1A0000, 0x600000, 2);
            if (arg0->soundTimer == 0) {
                enqueuePositionalSoundEffect(0xD, pos, 0x7F, 0x31);
                createThrownTrailImpactProjectile(
                    arg0->pos.x,
                    arg0->pos.y + 0x700000,
                    arg0->pos.z,
                    arg0->surface,
                    arg0->angle
                );
                arg0->soundTimer = 0xF;
            } else {
                arg0->soundTimer--;
            }
        }

        arg0->angle += arg0->angleStep;
        if (arg0->targetAngle + 0x140 < arg0->angle) {
            arg0->angleStep = -8;
        }
        if (arg0->angle < arg0->targetAngle - 0x140) {
            arg0->angleStep = 8;
        }
        arg0->spin += 0x40;
    }
    addRenderCallback(&gRaceObjectRenderCallbackList, (RenderCallback)renderRaceUiThrownTrailObstacle, (void *)arg0);
}
