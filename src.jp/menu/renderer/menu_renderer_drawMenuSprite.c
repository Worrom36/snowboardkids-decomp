#include "common.h"
#include "PR/os_convert.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/controller_input.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/system_runtime.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/renderer/sprite_palette_relocation.h"

void drawMenuSprite(s16 arg0, s16 arg1, AssetTable *arg2, u16 arg3, u16 arg4, u16 arg5, u8 arg6, u8 arg7) {
    s16 temp_v0;
    s16 temp_v1;

    drawMenuSpriteClipped(
        arg0,
        arg1,
        arg2,
        arg3,
        arg4,
        arg5,
        arg6,
        arg7,
        temp_v0 = (s16)(gMenuViewportWidth / 2),
        temp_v1 = (s16)(gMenuViewportHeight / 2),
        temp_v0,
        temp_v1
    );
}
