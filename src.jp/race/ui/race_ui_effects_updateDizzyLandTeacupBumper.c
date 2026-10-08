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

void updateDizzyLandTeacupBumper(DizzyLandTeacupBumperActor *arg0) {
    DizzyLandTeacupBumperActor *actor;
    Vec3i *pos;
    s16 scale;

    actor = arg0;
    if (gRaceUpdatePaused == 0) {
        pos = &actor->pos;
        if (isPositionNearAnyRaceViewportFocus(pos) != 0) {
            actor->yaw += actor->yawStep;

            if (isRacePlayerInsideCylinder(pos, 0x200000, 0xF0000, 0) != 0) {
                actor->xzScale = 0x1800;
                enqueuePositionalSoundEffect(0x67, pos, 0x7F, 0x32);
                pushRacePlayerOutOfCylinder(pos, 0x2C0000, 0xF0000, 0);
            }
            if (isRacePlayerInsideCylinder(pos, 0x200000, 0xF0000, 1) != 0) {
                actor->xzScale = 0x1800;
                enqueuePositionalSoundEffect(0x67, pos, 0x7F, 0x32);
                pushRacePlayerOutOfCylinder(pos, 0x2C0000, 0xF0000, 1);
            }
            if (isRacePlayerInsideCylinder(pos, 0x200000, 0xF0000, 2) != 0) {
                actor->xzScale = 0x1800;
                enqueuePositionalSoundEffect(0x67, pos, 0x7F, 0x32);
                pushRacePlayerOutOfCylinder(pos, 0x2C0000, 0xF0000, 2);
            }
            if (isRacePlayerInsideCylinder(pos, 0x200000, 0xF0000, 3) != 0) {
                actor->xzScale = 0x1800;
                enqueuePositionalSoundEffect(0x67, pos, 0x7F, 0x32);
                pushRacePlayerOutOfCylinder(pos, 0x2C0000, 0xF0000, 3);
            }

            scale = actor->xzScale;
            if (scale != 0x1000) {
                actor->xzScale = scale - 0x100;
            }
        }
    }
    addRenderCallback(&gRaceModelEffectRenderCallbackList, (RenderCallback)renderDizzyLandTeacupBumper, actor);
}
