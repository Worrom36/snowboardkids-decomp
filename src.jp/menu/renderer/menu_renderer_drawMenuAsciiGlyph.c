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

void drawMenuAsciiGlyph(s16 x, s16 y, u16 tileS, s32 tileT, u16 palette, u16 paletteScale) {
    s32 x0;
    s32 storedY;
    s32 x1;
    s32 y1;
    s32 clipS;
    s32 clipT;
    s32 maxX;
    volatile u16 *dst;
    AssetTableEntry *texture;
    MenuPalette *palettes;
    AssetTable *font;
    s32 maxY;
    s32 i;
    s16 paletteIndex;

    font = getRelocatableHeapBlockBase(gAssetHandles[6]);
    palettes = (MenuPalette *)(font->entryCount + font->entries);
    paletteIndex = *(s16 *)&font->entries[0].paletteIndex;
    x0 = x + gMenuViewportCenterX;
    i = y + gMenuViewportCenterY;
    texture = &font->entries[0];
    x1 = x0 + 8;
    y1 = i + 8;
    clipS = 0;
    clipT = 0;

    {
        s32 minX;
        s32 minY;
        s32 halfHeight;

        maxX = gMenuViewportCenterX + (gMenuViewportWidth / 2);
        if (x0 < maxX) {
            halfHeight = gMenuViewportHeight / 2;
            maxY = gMenuViewportCenterY + halfHeight;
            minX = gMenuViewportCenterX - (gMenuViewportWidth / 2);
            if (i < maxY) {
                minY = gMenuViewportCenterY - halfHeight;
                if ((x1 >= minX) && (y1 >= minY)) {
                    s32 colorValue;
                    MenuPalette *scratch;
                    MenuPalette *source;
                    s32 color;
                    s32 red;
                    u16 green;
                    u16 blue;
                    s32 scaleValue;
                    u16 scaledRed;

                    if (x0 < minX) {
                        clipS = minX - x0;
                        x0 = minX;
                    }
                    if (i < minY) {
                        clipT = minY - i;
                        i = minY;
                    }
                    if (x1 >= maxX) {
                        x1 = maxX - 1;
                    }
                    if (y1 >= maxY) {
                        y1 = maxY - 1;
                    }
                    clipS += tileS;
                    clipT += (u16)tileT;

                    if (paletteIndex != palette) {
                        paletteIndex = palette;
                    }
                    storedY = i;
                    if (1) {
                        // clang-format off
                        scratch = allocMenuRenderScratch(sizeof(MenuPalette)); \
                        i = 0; \
                        source = &palettes[SPRITE_PALETTE_INDEX(font, paletteIndex)]; \
                        dst = scratch->colors; \
                    palette_loop: \
                        *dst = (color = (*(u16 *)&source->bytes[i]) & 0xFFFFu); \
                        i += sizeof(u16); \
                        if ((colorValue = color & 0xFFFF) & 1) { \
                            red = ((colorValue >> 11) & 0x1F) & 0xFFFF;
                        // clang-format on
                            green = (colorValue >> 6) & 0x1F;
                            colorValue = (blue = (colorValue >> 1) & 0x1F);
                            scaleValue = paletteScale;
                            red = red * scaleValue;
                            scaledRed = red / 0x100;
                            red = scaledRed;
                            green = (green * paletteScale) / 0x100;
                            colorValue = green;
                            blue = (blue * paletteScale) / 0x100;
                            *dst = (red << 11) | (colorValue << 6) | (blue << 1) | 1;
                        }
                        dst += 2;
                        dst--;
                        if (i != sizeof(MenuPalette)) {
                            goto palette_loop;
                        }

                        gDPLoadTextureTile_4b( \
                            gRegionAllocPtr++, \
                            texture->imageOffset + (u8 *)font,
                            G_IM_FMT_CI, \
                            texture->width, \
                            texture->height,
                            0, \
                            0, \
                            texture->width, \
                            texture->height, \
                            0,
                            G_TX_CLAMP, \
                            G_TX_CLAMP, \
                            G_TX_NOMASK, \
                            G_TX_NOMASK,
                            G_TX_NOLOD, \
                            G_TX_NOLOD \
                        );
                        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, scratch);
                        gSPTextureRectangle( \
                            gRegionAllocPtr++, \
                            x0 << 2, \
                            storedY << 2, \
                            x1 << 2, \
                            y1 << 2,
                            G_TX_RENDERTILE, \
                            clipS << 5, \
                            clipT << 5, \
                            0x400, \
                            0x400 \
                        );
                    }
                }
            }
        }
    }
}

void drawMenuAsciiText(s16 arg0, s16 arg1, u8 *arg2, u16 arg3, u16 arg4) {
    s32 var_s0;
    u8 *var_s1;
    char mask;
    s32 var_s2;
    s32 temp_s3;
    s32 temp_t9;
    long var_v0;
    s32 var_a2;
    s32 newline;
    s32 var_v1;

    mask = 0xFFFFFFFFFFFFFFFFu;
    temp_t9 = *arg2 & mask;
    var_s0 = arg0;
    var_s1 = arg2;
    var_s2 = arg1;
    temp_s3 = var_s0;
    if ((0, temp_t9) != 0) {
        var_v0 = temp_t9 & 0xFF;
        newline = 0xA;
        do {
            var_v1 = var_v0;
            if (newline == var_v0) {
                var_s0 = temp_s3;
                var_s2 += 8;
            } else {
                var_v0 = var_v1 - 0x20;
                if ((u32)var_v0 < 0x40) {
                    var_a2 = var_v0 & 7;
                    drawMenuAsciiGlyph(var_s0, var_s2, ((var_a2 << 3) & 0xFFFF) & 0xFFFF, var_v0 & 0x38, arg3, arg4);
                }
                var_s0 += 8;
            }
            var_v0 = var_s1[1];
            var_s1++;
        } while (var_v0 != 0);
    }
}
