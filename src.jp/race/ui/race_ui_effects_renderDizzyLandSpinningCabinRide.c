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
extern Vec3i gDizzyLandSpinningCabinRidePositions[];

void renderDizzyLandSpinningCabinRide(DizzyLandSpinningCabinRideActor *arg0) {
    struct {
        Transform3D transform;
        s16 unused[2];
    } scratch;
    s32 temp2;
    s32 pad;
    s32 pad2;
    SplitWord temp;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) == 0) {
        return;
    }

    if (arg0->matrixDirty != 0) {
        arg0->matrixDirty = 0;
        makeFixedRotationY(scratch.transform.rotation, arg0->baseYaw);
        scratch.transform.translation.x = arg0->pos.x;
        scratch.transform.translation.y = arg0->pos.y;
        scratch.transform.translation.z = arg0->pos.z;
        arg0->poleMatrix = allocFixedTransformMatrix(&scratch.transform);

        scratch.transform.translation.y += 0x01000000;
        temp.half.lo = fixedSine(arg0->pitchPhase) >> 5;
        temp2 = fixedSine(arg0->rollPhase) >> 5;
        makeFixedRotationYZX(scratch.transform.rotation, temp.half.lo, arg0->spinAngle, temp2);
        arg0->canopyMatrix = allocFixedTransformMatrix(&scratch.transform);
    }

    if (arg0->canopyMatrix != NULL) {
        gDPPipeSync(gRegionAllocPtr++);
        gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
        gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0x9)));
        gSPMatrix(gRegionAllocPtr++, arg0->poleMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(gRegionAllocPtr++, &DIZZY_LAND_SPINNING_CABIN_RIDE_POLE_DISPLAY_LIST_VRAM);
        gSPMatrix(gRegionAllocPtr++, arg0->canopyMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(gRegionAllocPtr++, &DIZZY_LAND_SPINNING_CABIN_RIDE_CANOPY_DISPLAY_LIST_VRAM);
    }
}

void updateDizzyLandSpinningCabinRide(DizzyLandSpinningCabinRideActor *arg0) {
    if (gRaceUpdatePaused == 0) {
        arg0->spinAngle += 0x60;
        arg0->pitchPhase += 0x10;
        arg0->rollPhase += 4;
    }
    addRenderCallback(&gRaceModelEffectRenderCallbackList, (RenderCallback)renderDizzyLandSpinningCabinRide, arg0);
}

void initDizzyLandSpinningCabinRide(DizzyLandSpinningCabinRideActor *arg0) {
    arg0->pos = gDizzyLandSpinningCabinRidePositions[arg0->task.userId];
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateDizzyLandSpinningCabinRide);
}
