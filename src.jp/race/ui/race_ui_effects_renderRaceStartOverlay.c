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

extern Vtx D_800D69A8[];

void renderRaceStartOverlay(RaceUiOverlayActor *arg0) {
    volatile u8 pad[0xC];
    Transform3D transform;

    if (gRenderMatricesDirty != 0) {
        arg0->matrixDirty = 1;
    }
    if (arg0->matrixDirty != 0) {
        arg0->matrixDirty = 0;
        transform = gIdentityFixedTransform;
        transform.translation.x = arg0->pos.x;
        transform.translation.y = arg0->pos.y;
        transform.translation.z = arg0->pos.z;
        arg0->matrix = allocFixedTransformMatrix(&transform);
    }
    if (arg0->matrix != NULL) {
        gSPDisplayList(gRegionAllocPtr++, gAlphaSpriteRenderModeDl);
        gDPLoadTextureTile_4b(gRegionAllocPtr++, arg0->panelAImage, G_IM_FMT_CI,
            48, 40, 0, 0, 48, 40, 0, G_TX_CLAMP, G_TX_CLAMP,
            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->panelAPalette);
        gSPMatrix(gRegionAllocPtr++, arg0->matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPMatrix(gRegionAllocPtr++, gViewportMatrix, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
        gSPVertex(gRegionAllocPtr++, D_800D69A8, 8, 0);
        gSP1Quadrangle(gRegionAllocPtr++, 3, 2, 1, 0, 0);
        gDPLoadTextureTile_4b(gRegionAllocPtr++, arg0->panelBImage, G_IM_FMT_CI,
            48, 40, 0, 0, 48, 40, 0, G_TX_CLAMP, G_TX_CLAMP,
            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, arg0->panelBPalette);
        gSP1Quadrangle(gRegionAllocPtr++, 7, 6, 5, 4, 0);
    }
}
