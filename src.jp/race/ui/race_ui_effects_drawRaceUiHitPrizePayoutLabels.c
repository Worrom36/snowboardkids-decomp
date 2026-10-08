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

extern const char gRaceUiHitPrizeX1Label[0x14];
extern const char gRaceUiPerfectHitLabel[0x10];
extern const char gRaceUiCompleteBonusLabel[0x14];
extern const char gRaceUiHitPrizeTotalMoneyLabel[0x10];
extern const char gRaceUiHitPrizeX50Label[0x14];
extern u8 gRaceChallengeFailed;
extern const char gRaceUiCourseValueHitLabelRow3[0x4];
extern const char gRaceUiCourseValueHitLabelRow5[0x4];
extern const char gRaceUiCourseValueHitLabelRow1[0x4];
extern const char gRaceUiCourseValueHitLabelRow4[0x4];
extern const char gRaceUiCourseValueHitLabelRow2[0x4];

void drawRaceUiHitPrizePayoutLabels(RaceUiCounterActor *arg0) {
    s32 y;

    if (arg0->alpha != 0xFF) {
        gDPPipeSync(gRegionAllocPtr++);
        gDPSetCombineMode(gRegionAllocPtr++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetRenderMode(gRegionAllocPtr++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 0xFF, 0xFF, 0xFF, arg0->alpha);
    }

    y = -0x50;
    if (arg0->flag == 0) {
        y = -0x3C;
    }

    if (gRaceChallengeFailed != 0) {
        drawMenuAsciiTextDefaultScale(-8, (s16)y, gRaceUiHitPrizeX1Label, 5);
    } else {
        drawMenuAsciiTextDefaultScale(-0xC, (s16)y, gRaceUiHitPrizeX50Label, 5);
    }

    y += 0x28;
    if (arg0->flag != 0) {
        drawMenuAsciiTextDefaultScale(-0xC, (s16)y, gRaceUiPerfectHitLabel, 5);
        y += 0x28;
    }

    drawMenuAsciiTextDefaultScale(-8, (s16)y, gRaceUiCompleteBonusLabel, 5);
    y += 0x28;
    drawMenuAsciiTextDefaultScale(4, (s16)y, gRaceUiHitPrizeTotalMoneyLabel, 5);

    if (arg0->alpha != 0xFF) {
        gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
    }
}

void drawRaceUiScoreAttackHitLabels(void *arg0) {
    drawMenuAsciiTextDefaultScale(0x20, -0x48, gRaceUiCourseValueHitLabelRow1, 6);
    drawMenuAsciiTextDefaultScale(0x20, -0x28, gRaceUiCourseValueHitLabelRow2, 6);
    drawMenuAsciiTextDefaultScale(0x20, -8, gRaceUiCourseValueHitLabelRow3, 6);
    drawMenuAsciiTextDefaultScale(0x20, 0x18, gRaceUiCourseValueHitLabelRow4, 4);
    drawMenuAsciiTextDefaultScale(0x20, 0x38, gRaceUiCourseValueHitLabelRow5, 4);
}
