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

void updateRaceUiHeavyKnockdownTrailEffect(RaceUiRankTrailActor *arg0) {
    RacePlayer *player;
    s32 i;

    arg0->copyBlock = (&gRacePlayers[arg0->task.userId])->modelPartTransforms[0];
    arg0->copyBlock.rotation[0] = arg0->copyBlock.rotation[0] / 4;
    arg0->copyBlock.rotation[1] = arg0->copyBlock.rotation[1] / 4;
    arg0->copyBlock.rotation[2] = arg0->copyBlock.rotation[2] / 4;
    arg0->copyBlock.rotation[3] = arg0->copyBlock.rotation[3] / 4;
    arg0->copyBlock.rotation[4] = arg0->copyBlock.rotation[4] / 4;
    arg0->copyBlock.rotation[5] = arg0->copyBlock.rotation[5] / 4;
    arg0->copyBlock.rotation[6] = arg0->copyBlock.rotation[6] / 4;
    arg0->copyBlock.rotation[7] = arg0->copyBlock.rotation[7] / 4;
    arg0->copyBlock.rotation[8] = arg0->copyBlock.rotation[8] / 4;

    player = &gRacePlayers[arg0->task.userId];
    if (player->stateFlags & 0x400000) {
        addRenderCallback(&gRaceModelEffectRenderCallbackList, (RenderCallback)renderRaceUiHeavyKnockdownTrailEffect, (void *)arg0);
        return;
    }

    enqueuePositionalSoundEffect(0x11, &player->pos, 0x7F, 0x32);

    for (i = 0; i < 8; i++) {
        spawnRaceUiAltBurstTextParticle(
            (void *)arg0->copyBlock.translation.x,
            (void *)arg0->copyBlock.translation.y,
            (void *)arg0->copyBlock.translation.z,
            i
        );
    }

    removeCallbackTask(arg0);
}
