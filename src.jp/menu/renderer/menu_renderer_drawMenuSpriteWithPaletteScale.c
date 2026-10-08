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

#define MENU_PALETTE_SIZE_BYTES 0x20
#define MENU_RGBA5551_ALPHA_BIT 1
#define MENU_RGBA5551_CHANNEL_MASK 0x1F
#define MENU_RGBA5551_SCALE_BASE 0x100
#define MENU_HALF_SCALE_STEP 0x800

void drawMenuSpriteWithPaletteScale(s16 x, s16 y, AssetTable *asset, u16 index, u16 intensity) {
    AssetTableEntry *texture;
    u8 *textureBase;
    u8 *paletteBase;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 srcX;
    s32 srcY;
    u16 headerSize;
    MenuPalette *scratch;
    MenuPalette *source;
    u16 red;
    u16 green;
    u16 blue;
    u16 scaledRed;
    s32 i;

    headerSize = sizeof(AssetTableEntry);
    textureBase = (u8 *)asset + (index * sizeof(AssetTableEntry));
    texture = (AssetTableEntry *)(textureBase + headerSize);
    paletteBase = (asset->entryCount * sizeof(AssetTableEntry)) + (u8 *)asset + headerSize;
    left = x + gMenuViewportCenterX;
    top = y + gMenuViewportCenterY;
    right = left + (texture->width >> 1);
    bottom = top + (texture->height >> 1);
    srcX = 0;
    srcY = 0;

    if (left >= gMenuViewportCenterX + (gMenuViewportWidth / 2))
        return;
    if (top >= gMenuViewportCenterY + (gMenuViewportHeight / 2))
        return;
    if (right < gMenuViewportCenterX - (gMenuViewportWidth / 2))
        return;
    if (bottom < gMenuViewportCenterY - (gMenuViewportHeight / 2))
        return;
    if (left < gMenuViewportCenterX - (gMenuViewportWidth / 2)) {
        srcX = (gMenuViewportCenterX - (gMenuViewportWidth / 2)) - left;
        left = gMenuViewportCenterX - (gMenuViewportWidth / 2);
    }
    if (top < gMenuViewportCenterY - (gMenuViewportHeight / 2)) {
        srcY = (gMenuViewportCenterY - (gMenuViewportHeight / 2)) - top;
        top = gMenuViewportCenterY - (gMenuViewportHeight / 2);
    }
    if (right >= gMenuViewportCenterX + (gMenuViewportWidth / 2))
        right = gMenuViewportCenterX + (gMenuViewportWidth / 2);
    if (bottom >= gMenuViewportCenterY + (gMenuViewportHeight / 2))
        bottom = gMenuViewportCenterY + (gMenuViewportHeight / 2);

    gDPPipeSync(gRegionAllocPtr++);
    gDPSetTextureFilter(gRegionAllocPtr++, G_TF_AVERAGE);

    source = (MenuPalette *)(paletteBase + (texture->paletteIndex * MENU_PALETTE_SIZE_BYTES));
    scratch = allocMenuRenderScratch(sizeof(MenuPalette));
    for (i = 0; i != MENU_PALETTE_COLOR_COUNT; i++) {
        scratch->colors[i] = source->colors[i];
        if (scratch->colors[i] & MENU_RGBA5551_ALPHA_BIT) {
            red = (scratch->colors[i] >> 11) & MENU_RGBA5551_CHANNEL_MASK;
            green = (scratch->colors[i] >> 6) & MENU_RGBA5551_CHANNEL_MASK;
            blue = (scratch->colors[i] >> 1) & MENU_RGBA5551_CHANNEL_MASK;
            red = (red * intensity) / MENU_RGBA5551_SCALE_BASE;
            green = (green * intensity) / MENU_RGBA5551_SCALE_BASE;
            blue = (blue * intensity) / MENU_RGBA5551_SCALE_BASE;

            scratch->colors[i] = (red << 11) | (green << 6) | (blue << 1) | MENU_RGBA5551_ALPHA_BIT;
        }
    }

    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, scratch);
    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        texture->imageOffset + (u8 *)asset,
        G_IM_FMT_CI,
        texture->width,
        texture->height,
        0,
        0,
        texture->width,
        texture->height,
        0,
        G_TX_CLAMP,
        G_TX_CLAMP,
        G_TX_NOMASK,
        G_TX_NOMASK,
        G_TX_NOLOD,
        G_TX_NOLOD
    );
    gSPTextureRectangle(
        gRegionAllocPtr++,
        left << 2,
        top << 2,
        right << 2,
        bottom << 2,
        G_TX_RENDERTILE,
        (srcX << 5) + 0x10,
        (srcY << 5) + 0x10,
        MENU_HALF_SCALE_STEP,
        MENU_HALF_SCALE_STEP
    );
    gDPPipeSync(gRegionAllocPtr++);
    gDPSetTextureFilter(gRegionAllocPtr++, G_TF_POINT);
    gDPPipeSync(gRegionAllocPtr++);
}
