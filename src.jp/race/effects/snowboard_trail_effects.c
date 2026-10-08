#include "game/race/effects/snowboard_trail_effects.h"

#define SNOWBOARD_TRAIL_TIMER 0xF0
#define SNOWBOARD_TRAIL_FLAG_FACING_BACKWARD 0x400

void startSpeedFanEffect(RacePlayer *player) {
    SpeedFanState *trail = &player->speedFan;

    player->trailEffectTimer = SNOWBOARD_TRAIL_TIMER;
    if (player->stateFlags & SNOWBOARD_TRAIL_FLAG_FACING_BACKWARD) {
        trail->localOffset.x = 0x300000;
        trail->localOffset.y = 0;
        trail->localOffset.z = 0;
        trail->modelYaw = 0;
        player->snowboardTrailSide = 1;
    } else {
        trail->localOffset.x = -0x300000;
        trail->localOffset.y = 0;
        trail->localOffset.z = 0;
        trail->modelYaw = 0x800;
        player->snowboardTrailSide = 0;
    }
    trail->spinYaw = 0;
    trail->scale.x = 0xF0000;
    trail->scale.y = 0x230000;
    trail->scale.z = 0;
    trail->scaleStep = 1;
    trail->state = 1;
}
