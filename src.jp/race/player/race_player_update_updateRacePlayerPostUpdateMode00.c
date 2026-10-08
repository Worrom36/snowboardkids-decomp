#include "game/race/items/race_items.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/race_state.h"
#include "common.h"
#include "game/engine/render_callback.h"
#include "game/engine/asset_manager.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/race/player/race_player_rumble.h"
#include "game/audio/sound_manager.h"
#include "game/race/motion/race_motion.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/race_course_effects.h"
#include "game/race/player/race_player_collision.h"
#include "game/math/fixed_point_math.h"
#include "game/math/spatial_math.h"
#include "game/race/player/race_player_input.h"
#include "game/race/items/race_item_effects.h"
#include "game/race/player/race_player_pickup_effects.h"
#include "game/menu/renderer/menu_screen_effects.h"
#include "game/race/player/race_player_movement.h"
#include "game/race/player/race_player_progress.h"
#include "game/race/player/race_player_model_renderer.h"
#include "game/race/player/race_player_update.h"
#include "game/race/course/race_surface_cues.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/race/effects/snowboard_trail_effects.h"
#include "game/engine/viewport_manager.h"

#define interpolateRaceMotionJointAnimationFrame(player, animIndex, frameTimer, frameTimerReset) \
    interpolateRaceMotionJointAnimationFrame(RACE_MOTION_STATE(player), (animIndex), (frameTimer), (frameTimerReset))
#define RACE_MOTION_STATE(player) ((RaceMotionState *)(player))

void updateRacePlayerPostUpdateMode00(RacePlayer *player) {
    if (updateRacePlayerSurfaceContact(player)) {
        player->mode = 0x16;
        player->updateState = 0;
        player->updateTimer = 0;
        player->stateFlags |= 0x200;
    } else if (tryStartRacePlayerCourseObjectMode(player->playerIndex)) {
        player->mode = 7;
        player->updateState = 0;
        player->updateTimer = 0;
    }
    interpolateRaceMotionJointAnimationFrame(player, 0, (0x60000 - player->groundOffset) / 0x600, 0x100);
}
