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
extern u16 D_800D6520[];

void updateRaceUiProjectileInitialFall(RaceUiProjectileActor *arg0) {
    s16 sp2C[0x12];
    RacePlayer *player;
    RaceUiProjectileActor *actor;

    actor = arg0;
    if (gRaceUpdatePaused == 0) {
        makeFixedRotationXY(sp2C, gRacePlayers[actor->task.userId].pitchAngle, gRacePlayers[actor->task.userId].facingAngle);
        transformVec3iByFixedMatrix(sp2C, &actor->velocity, &actor->pos);

        player = &gRacePlayers[actor->task.userId];
        actor->pos.x += player->unk28.x;
        actor->pos.y += player->unk28.y + actor->verticalVelocity;
        actor->pos.z += player->unk28.z;
        actor->verticalVelocity += actor->verticalAcceleration;
        actor->verticalAcceleration += 0xFFFF0000;
        if (actor->verticalVelocity <= 0) {
            actor->verticalVelocity = 0;
            setCallbackTaskCallback(actor, (CallbackTaskCallback)updateRaceUiProjectileGrounded);
        }
    }
    addRenderCallback(&D_801248C8, (RenderCallback)renderRaceUiProjectile, actor);
}

void initForwardActionProjectileEffect(RaceUiProjectileActor *arg0) {
    arg0->velocity.x = 0;
    arg0->velocity.y = 0x280000;
    arg0->velocity.z = 0;
    arg0->unk3C = 0;
    arg0->unk4C = 0;
    arg0->verticalVelocity = 0x300000;
    arg0->verticalAcceleration = 0;
    setRaceUiProjectileAnimation(arg0, D_800D6520);
    enqueuePositionalSoundEffect(0x66, &gRacePlayers[arg0->task.userId].unk28, 0x7F, 0x32);
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceUiProjectileInitialFall);
}
