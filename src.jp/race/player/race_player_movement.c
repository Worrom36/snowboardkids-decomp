#include "game/race/player/race_player_movement.h"

s32 updateRacePlayerLeanAngle(RacePlayer *player, s32 arg1, s16 arg2) {
    s16 temp_v0;
    s16 scale;

    if (arg1 < 0) {
        arg1 = -arg1;
    }
    temp_v0 = player->leanInput;
    arg2 = arg2 * 0x10;
    arg2 = arg2 - temp_v0;
    if (arg2 >= 0x81) {
        arg2 = 0x80;
    }
    if (arg2 < -0x80) {
        arg2 = -0x80;
    }
    player->leanInput = temp_v0 + arg2;
    arg2 = player->leanInput * player->leanScale / 0x3F;
    if (arg1 >= 0x40001) {
        scale = 0x117;
    } else {
        scale = (arg1 * 0x117) / 0x40000;
    }
    arg1 = player->leanAngle;
    arg2 = arg2 * scale / 0x1F0;
    temp_v0 = arg2;
    player->leanAngle = player->leanAngle + ((temp_v0 - arg1) >> 2);
    return temp_v0;
}
