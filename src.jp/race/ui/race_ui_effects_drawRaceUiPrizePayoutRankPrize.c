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

extern const char gRaceUiRankPrizeCounterFormat[0x4];
extern const char gRaceUiCoinCounterFormat[0x4];
extern const char gRaceUiTotalMoneyCounterFormat[0x4];
extern const char gRaceUiTotalMoneyLabel[0x10];
extern const char gRaceUiRankPrizeLabel[0x10];
extern const char gRaceUiGetCoinLabel[0x10];

void drawRaceUiPrizePayoutRankPrize(RaceUiAlpha18Actor *arg0) {
    volatile u8 padding[0x18];
    char buffer[8];
    s32 x;
    u8 *ptr;
    volatile s16 *handles;
    s32 space;

    sprintf(buffer + -8, gRaceUiRankPrizeCounterFormat, arg0->score);
    x = 0x20;
    handles = gAssetHandles;
 ptr = (u8 *) (((s32) padding) - 0x10); if (!ptr) { } space = ' '; loop: if ((*ptr) != 0) {
        if (space != *ptr) {
            drawAssetTableSprite(x, -0x3F, getRelocatableHeapBlockBase(handles[0x1F]),
                                 *ptr - 5);
        }
        x += 8;
        ptr++;
        goto loop;
    }

    drawAssetTableSprite(x, -0x3F, getRelocatableHeapBlockBase(handles[0x1F]), 0x37);
}

void drawRaceUiPrizePayoutCollectedCoins(void *arg0)
{
    s32 x;
    s32 i;
    s32 space;
    s32 colorValue;
    u16 color;
    s16 *assets;
    char buffer[0x14];

    i = 0;
    do
    {
        sprintf(&buffer[i], gRaceUiCoinCounterFormat, gRacePlayers[0].score);
        x = 0x20;
        if (gRacePlayers[0].rankIndex == 3)
        {
            colorValue = 0xD; \
        } else { \
            colorValue = 0xC; \
        } \
        do { \
            assets = &gAssetHandles[i]; \
            space = ' '; \
            color = colorValue; \
            if (space != 0U) {} \
        loop: \
            if (buffer[i] != '\0') { \
                if (buffer[i] != space) { \
                    drawAssetTableSpriteWithExplicitPalette( \
                        (s16)x, \
                        -0xF, \
                        getRelocatableHeapBlockBase(assets[0x1F]), \
                        buffer[i] - 5, \
                        color \
                    ); \
                }
                x += 8;
                i++;
                goto loop;
            }
        } while (0);
        drawAssetTableSpriteWithExplicitPalette((s16) x, -0xF, getRelocatableHeapBlockBase(assets[0x1F]), 0x37, color);
    }
    while (0);
    if (gRacePlayers[0].rankIndex == 3)
    {
        drawAssetTableSprite(0x18, -0xB, getRelocatableHeapBlockBase(assets[0x1F]), 0x4E);
    }
}

void drawRaceUiPrizePayoutTotalMoney(void *arg0) {
    volatile u8 padding[0x18];
    char buffer[8];
    s32 x;
    u8 *ptr;
    volatile s16 *handles;
    s32 space;

    sprintf(buffer + -8, gRaceUiTotalMoneyCounterFormat, gRacePlayers[0].money);
    x = 0x18;
    handles = gAssetHandles;
 ptr = (u8 *) (((s32) padding) - 0x10); if (!ptr) { } space = ' '; loop: if ((*ptr) != 0) {
        if (space != *ptr) {
            drawAssetTableSprite(x, 0x21, getRelocatableHeapBlockBase(handles[0x1F]),
                                 *ptr - 5);
        }
        x += 8;
        ptr++;
        goto loop;
    }

    drawAssetTableSprite(x, 0x21, getRelocatableHeapBlockBase(handles[0x1F]), 0x37);
}

void drawRaceUiPrizePayoutLabels(RaceUiAlpha18Actor *arg0) {
    if (arg0->alpha != 0xFF) {
        gDPPipeSync(gRegionAllocPtr++);
        gDPSetCombineMode(gRegionAllocPtr++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetRenderMode(gRegionAllocPtr++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetPrimColor(gRegionAllocPtr++, 0, 0, 0xFF, 0xFF, 0xFF, arg0->alpha);
    }
    drawMenuAsciiTextDefaultScale(8, -0x48, gRaceUiRankPrizeLabel, 5);
    drawMenuAsciiTextDefaultScale(0, -0x18, gRaceUiGetCoinLabel, 5);
    drawMenuAsciiTextDefaultScale(4, 0x18, gRaceUiTotalMoneyLabel, 5);
    if (arg0->alpha != 0xFF) {
        gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
    }
}
