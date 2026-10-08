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

void updateRaceUiFadingImpact(RaceUiFadingImpactActor *arg0) {
    volatile u8 pad[0x28];
    s16 scale;
    Vec3i *sp24;
    s32 sp2C;
    RaceUiFadingImpactActor *new_var;

    if (gRaceUpdatePaused == 0) {
        arg0->angle += 0x100;
        makeFixedRotationY(arg0->copyBlock.rotation, arg0->angle);

        scale = arg0->scale;
        arg0->copyBlock.rotation[0] = (arg0->copyBlock.rotation[0] * scale) / 64;
        arg0->copyBlock.rotation[1] = (arg0->copyBlock.rotation[1] * scale) / 64;
        arg0->copyBlock.rotation[2] = (arg0->copyBlock.rotation[2] * scale) / 64;
        new_var = arg0;
        arg0->copyBlock.rotation[3] = (arg0->copyBlock.rotation[3] * scale) / 64;
        new_var->copyBlock.rotation[4] = (new_var->copyBlock.rotation[4] * scale) / 64;
        arg0->copyBlock.rotation[5] = (new_var->copyBlock.rotation[5] * scale) / 64;
        new_var->copyBlock.rotation[6] = (new_var->copyBlock.rotation[6] * scale) / 64;
        arg0->copyBlock.rotation[7] = (arg0->copyBlock.rotation[7] * scale) / 64;
        new_var->copyBlock.rotation[8] = (arg0->copyBlock.rotation[8] * scale) / 64;

        if (scale != 0x34) {
            new_var->scale = scale + 4;
            scale = new_var->scale;
        }

        if (scale >= 0x21) {
            arg0->alpha -= 0x10;
        }

        if (arg0->alpha >= 0x71) {
            sp24 = &arg0->pos;
            sp2C = (new_var->scale * 0x480000) / 64;
            if (tryApplyRacePlayerItemHit(sp24, sp2C, 0x80, 0) != 0) {
                gRacePlayers[0].hitSourcePlayerIndex = new_var->task.userId;
            }
            if (tryApplyRacePlayerItemHit(sp24, sp2C, 0x80, 1) != 0) {
                gRacePlayers[1].hitSourcePlayerIndex = new_var->task.userId;
            }
            if (tryApplyRacePlayerItemHit(sp24, sp2C, 0x80, 2) != 0) {
                gRacePlayers[2].hitSourcePlayerIndex = arg0->task.userId;
            }
            if (tryApplyRacePlayerItemHit(sp24, sp2C, 0x80, 3) != 0) {
                gRacePlayers[3].hitSourcePlayerIndex = arg0->task.userId;
            }
        }
    }

    if (arg0->alpha <= 0) {
        removeCallbackTask(arg0);
    } else {
        addRenderCallback(&D_801248EC, (RenderCallback)renderRaceUiFadingImpact, (void *)arg0);
    }
}
