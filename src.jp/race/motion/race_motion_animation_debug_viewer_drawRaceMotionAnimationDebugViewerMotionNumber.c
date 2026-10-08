#include "common.h"
#include <PR/os_libc.h>
#include "game/engine/controller_input.h"
#include "game/engine/render_callback.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/race/camera/race_camera.h"
#include "game/race/motion/race_motion_animation_debug_viewer.h"
#include "game/race/motion/race_motion.h"
#include "game/race/player/race_player_model_renderer.h"
#include "game/race/player/race_player_input.h"
#include "game/race/player/race_player_update.h"

extern const char gRaceMotionAnimationDebugViewerMotionNumberFormat[];

void drawRaceMotionAnimationDebugViewerMotionNumber(void *unused) {
    char buf[0x64];

    sprintf(buf, gRaceMotionAnimationDebugViewerMotionNumberFormat, gRacePlayers[0].animationId);
    drawMenuAsciiTextDefaultScale(0x28, 0x28, buf, 1);
}
