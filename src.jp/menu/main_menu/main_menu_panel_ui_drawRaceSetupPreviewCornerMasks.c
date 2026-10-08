#include "game/race/race_state.h"
#include "game/engine/game_task_scheduler.h"
#include "font_encoding.h"
#include "common.h"
#include "game/audio/sound_manager.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/system_runtime.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/menu/main_menu/main_menu_panel_ui.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"

void drawRaceSetupPreviewCornerMasks(void *arg0) {
    drawAssetTableSpriteWithDefaultPalette(-0x84, -0x64, getRelocatableHeapBlockBase(gAssetHandles[6]), 6);
    drawAssetTableSpriteWithDefaultPalette(0x74, -0x64, getRelocatableHeapBlockBase(gAssetHandles[6]), 7);
    drawAssetTableSpriteWithDefaultPalette(-0x84, 4, getRelocatableHeapBlockBase(gAssetHandles[6]), 8);
    drawAssetTableSpriteWithDefaultPalette(0x74, 4, getRelocatableHeapBlockBase(gAssetHandles[6]), 9);
}
