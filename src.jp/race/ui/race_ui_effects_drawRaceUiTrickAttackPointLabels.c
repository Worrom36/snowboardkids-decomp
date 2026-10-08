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

extern const char gRaceUiTrickValuePointLabelRow5[0x8];
extern const char gRaceUiTrickValuePointLabelRow3[0x4];
extern const char gRaceUiTrickValuePointLabelRow2[0x4];
extern const char gRaceUiTrickValuePointLabelRow1[0x4];
extern const char gRaceUiTrickValuePointLabelRow4[0x4];

void drawRaceUiTrickAttackPointLabels(void *arg0) {
    drawMenuAsciiTextDefaultScale(0x30, -0x48, gRaceUiTrickValuePointLabelRow1, 6);
    drawMenuAsciiTextDefaultScale(0x30, -0x28, gRaceUiTrickValuePointLabelRow2, 6);
    drawMenuAsciiTextDefaultScale(0x30, -8, gRaceUiTrickValuePointLabelRow3, 6);
    drawMenuAsciiTextDefaultScale(0x30, 0x18, gRaceUiTrickValuePointLabelRow4, 4);
    drawMenuAsciiTextDefaultScale(0x30, 0x38, gRaceUiTrickValuePointLabelRow5, 4);
}
