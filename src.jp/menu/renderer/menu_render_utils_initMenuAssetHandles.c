#include "game/race/race_state.h"
#include "game/engine/game_task_scheduler.h"
#include "common.h"
#include "game/engine/asset_manager.h"
#include "game/engine/system_runtime.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/renderer/sprite_palette_relocation.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/math/fixed_point_math.h"
#include "game/engine/viewport_manager.h"
#include "game/race/camera/race_camera.h"
#include <PR/os_cache.h>

void initMenuAssetHandles(void)
{
    s16 *end;
    s16 *handle;
    end = &gMenuAsciiFontPaletteIndex;
    do { handle = gAssetHandles; do { end = &gMenuAsciiFontPaletteIndex;
        *handle++ = -1;
        *handle++ = -1;
        *handle++ = -1;
        *handle++ = -1;
    }
    while (handle != end);
    }
    while (0);
    end++;
    end--;
}

void releaseMenuAssetHandles(void)
{
    do { \
        s16 *handle = &gAssetHandles[7]; \
        do { \
            if ((*handle) != (-1)) { \
                *handle = freeRelocatableHeapBlock(*handle); \
            } \
            handle++; \
        } while (handle != (&gMenuAsciiFontPaletteIndex)); \
    } while (0);
}
