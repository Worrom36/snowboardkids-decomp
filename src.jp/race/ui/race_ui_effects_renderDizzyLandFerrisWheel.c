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

#define RACE_UI_TRAIL_GFX_ALLOC_PTR (*(RaceUiDisplayCommand **)&gRegionAllocPtr)
#define ASSET_HANDLE(index) (gAssetHandles[(index)])

typedef struct {
    struct {
        u32 w0;
        u32 w1;
    } words;
} RaceUiDisplayCommand;

void renderDizzyLandFerrisWheel(DizzyLandFerrisWheelActor *arg0) {
    struct {
        Transform3D transform;
        s16 unused[2];
    } scratch;
    Vec3i transformedOffset;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }

    if (isPositionNearCurrentRaceViewportCamera(&arg0->pos) != 0) {
        if (arg0->matrixDirty != 0) {
            arg0->matrixDirty = 0;
            makeFixedRotationY(scratch.transform.rotation, arg0->yaw);
            scratch.transform.translation.x = arg0->pos.x;
            scratch.transform.translation.y = arg0->pos.y;
            scratch.transform.translation.z = arg0->pos.z;
            arg0->supportMatrix = allocFixedTransformMatrix(&scratch.transform);

            transformVec3iByFixedMatrix(scratch.transform.rotation, &gDizzyLandFerrisWheelHubOffset, &transformedOffset);
            scratch.transform.translation.x += transformedOffset.x;
            scratch.transform.translation.y += transformedOffset.y;
            scratch.transform.translation.z += transformedOffset.z;
            makeFixedRotationZY(scratch.transform.rotation, arg0->yaw, arg0->wheelAngle);
            arg0->wheelMatrix = allocFixedTransformMatrix(&scratch.transform);
        }

        if (arg0->wheelMatrix != NULL) {
            gDPPipeSync(RACE_UI_TRAIL_GFX_ALLOC_PTR++);
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x02, getRelocatableHeapBlockBase(ASSET_HANDLE(0x8)));
            gSPSegment(RACE_UI_TRAIL_GFX_ALLOC_PTR++, 0x03, getRelocatableHeapBlockBase(ASSET_HANDLE(0x9)));
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->supportMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_FERRIS_WHEEL_SUPPORT_DISPLAY_LIST_VRAM);
            gSPMatrix(RACE_UI_TRAIL_GFX_ALLOC_PTR++, arg0->wheelMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(RACE_UI_TRAIL_GFX_ALLOC_PTR++, &DIZZY_LAND_FERRIS_WHEEL_DISPLAY_LIST_VRAM);
        }
    }
}
