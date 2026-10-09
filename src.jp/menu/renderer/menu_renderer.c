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
#define MENU_GLYPH_SCRIPT_NEWLINE 0xFFFD
#define MENU_GLYPH_SCRIPT_SPACE 0xFFFE
#define MENU_GLYPH_SCRIPT_END 0xFFFF
#define MENU_GLYPH_WIDE_ADVANCE 0x10
#define MENU_GLYPH_NARROW_ADVANCE 8
#define MENU_GLYPH_LINE_HEIGHT 0x10
#define MENU_GLYPH_PALETTE_INDEX(palette) (((u8 *)&(palette))[3])
#define MENU_GLYPH_DEFAULT_FONT_BANK 0x22
#define MENU_GLYPH_SCRIPT_COLOR 0xFFFC

extern s16 gMenuFadeAlpha;
extern s16 gMenuSpriteFlipScales[8];

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/func_8000E6F0.s")

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/drawMenuSpriteClipped.s")

void drawMenuSpriteWithAlpha(
    s16 arg0,
    s16 arg1,
    AssetTable *arg2,
    u16 arg3,
    u16 arg4,
    u16 arg5,
    u8 arg6,
    u16 arg7,
    u8 arg8
) {
    s32 temp_v0;
    s32 temp_v1;

    drawMenuSpriteWithAlphaClipped(
        arg0,
        arg1,
        arg2,
        arg3,
        arg4,
        arg5,
        arg6,
        arg7,
        arg8,
        temp_v0 = (s16)(gMenuViewportWidth / 2),
        temp_v1 = (s16)(gMenuViewportHeight / 2),
        temp_v0,
        temp_v1
    );
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/drawMenuSpriteWithAlphaClipped.s")

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

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/func_800101B4.s")

void drawMenuSpriteFixedScale(
    s16 x,
    s16 y,
    AssetTable *asset,
    u16 tileIndex,
    u16 scaleX,
    u16 scaleY,
    u8 flipMode,
    u8 unusedPalette
) {
    AssetTableEntry *texture;
    s32 pad;
    u8 *paletteBase;
    s32 drawLeft;
    s32 drawTop;
    s32 drawRight;
    s32 drawBottom;
    s16 clipRight;
    s16 clipBottom;
    s32 texT;
    s32 texS;
    s16 clipLeft;
    s16 clipTop;
    s16 sScale;
    s16 tScale;
    s32 temp_v1;
    u32 height;

    texture = &asset->entries[tileIndex];
    paletteBase = asset->entryCount * sizeof(AssetTableEntry) + (u8 *)asset->entries;

    if (scaleX > 0xF000 || scaleX < 0x10 || scaleY > 0xE800 || scaleY < 0x10) {
        return;
    }

    flipMode &= 3;
    sScale = gMenuSpriteFlipScales[(flipMode) * 2 + 0];
    tScale = gMenuSpriteFlipScales[(flipMode) * 2 + 1];

    texS = texture->width;
    texT = texture->height;
    drawLeft = (x + gMenuViewportCenterX) << 2;
    drawTop = (y + gMenuViewportCenterY) << 2;
    drawRight = (((scaleX * texS) << 2) / 0x1000) + drawLeft;
    drawBottom = (((scaleY * texT) << 2) / 0x1000) + drawTop;

    texS = 0;
    texT = 0;

    if (sScale == -1) {
        texS = ((texture->width - 1) << 5);
    }
    if (tScale == -1) {
        texT = ((texture->height - 1) << 5) - texT;
    }

    clipTop = (gMenuViewportCenterY - (gMenuViewportHeight / 2)) << 2;
    clipBottom = (gMenuViewportCenterY + (gMenuViewportHeight / 2)) << 2;
    clipLeft = (gMenuViewportCenterX - (gMenuViewportWidth / 2)) << 2;
    clipRight = (gMenuViewportCenterX + (gMenuViewportWidth / 2)) << 2;

    if ((drawLeft >= clipRight) || (drawTop >= clipBottom) || (drawRight < clipLeft) || (drawBottom < clipTop)) {
        return;
    }

    if (drawLeft < clipLeft) {
        texS = ((((clipLeft - drawLeft) << 3) << 12) / scaleX);
        if (sScale == -1) {
            texS = ((texture->width - 1) << 5) - texS;
        }
        drawLeft = clipLeft;
    }
    if (drawTop < clipTop) {
        texT = ((((clipTop - drawTop) << 3) << 12) / scaleY);
        // clang-format off
        if (tScale == -1) { texT = ((texture->height - 1) << 5) - texT; }
        // clang-format on
        drawTop = clipTop;
    }
    if (drawRight >= clipRight) {
        drawRight = clipRight - 4;
    }
    if (drawBottom >= clipBottom) {
        drawBottom = clipBottom - 4;
    }

    gDPLoadTextureTile_4b(
        gRegionAllocPtr++,
        OS_PHYSICAL_TO_K0(texture->imageOffset + (u8 *)asset),
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
    gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, OS_PHYSICAL_TO_K0((texture->paletteIndex << 5) + paletteBase));
    gSPTextureRectangle(
        gRegionAllocPtr++,
        drawLeft,
        drawTop,
        drawRight,
        drawBottom,
        G_TX_RENDERTILE,
        texS,
        texT,
        (u16)((u16)(0x400000 / scaleX) * sScale),
        (u16)((u16)(0x400000 / scaleY) * tScale)
    );
}

void drawMenuSpriteTile(s16 arg0, s16 arg1, AssetTable *arg2, u16 arg3, u16 arg4, u16 arg5) {
    drawMenuSpriteTileClipped(arg0, arg1, arg2, arg3, arg4, arg5, gMenuViewportWidth / 2, gMenuViewportHeight / 2);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/drawMenuSpriteTileClipped.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/func_8001130C.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/func_80011314.s")

void initMenuSpriteActorDebugControls(MenuRenderSpriteActor *arg0) {
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateMenuSpriteActorDebugControls);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/updateMenuSpriteActorDebugControls.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/func_800117FC.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/drawMenuTilemapSprite.s")

void drawMenuGlyphScriptWithFontBank(
    volatile s16 x,
    s16 y,
    MenuGlyphScript *script,
    s32 palette,
    u16 scale,
    u16 fontBank
) {
    u16 firstGlyph;
    s32 glyphCode;
    MenuGlyphScript *scriptCursor;
    s32 drawX;
    u16 glyphAdvance;
    s32 drawY;
    u16 scaleValue;
    s32 advance;
    u16 fontBankValue;

    drawX = x;
    drawY = y;
    if (MENU_GLYPH_PALETTE_INDEX(palette) == 0) {
        glyphAdvance = MENU_GLYPH_WIDE_ADVANCE;
    } else {
        glyphAdvance = MENU_GLYPH_NARROW_ADVANCE;
    }

    firstGlyph = *script ^ 0;
    if (firstGlyph != MENU_GLYPH_SCRIPT_END) {
        scriptCursor = script;
        scaleValue = scale;
        fontBankValue = fontBank;
        glyphCode = firstGlyph;
        do {
            if ((glyphCode & 0xFFFF) == MENU_GLYPH_SCRIPT_NEWLINE) {
                drawX = x;
                drawY += MENU_GLYPH_LINE_HEIGHT;
            } else {
                advance = glyphAdvance;
                if ((glyphCode & 0xFFFF) != MENU_GLYPH_SCRIPT_SPACE) {
                    drawMenuGlyph(
                        drawX,
                        drawY,
                        glyphCode & 0xFFFF,
                        MENU_GLYPH_PALETTE_INDEX(palette),
                        scaleValue,
                        fontBankValue
                    );
                }
                drawX += advance;
            }
            glyphCode = scriptCursor[1];
            scriptCursor++;
        } while ((glyphCode & 0xFFFF) != MENU_GLYPH_SCRIPT_END);
    }
}

void drawMenuGlyphScriptDefaultFont(volatile s16 x, s16 y, MenuGlyphScript *script, s32 palette, u16 scale) {
    u16 initialGlyph;
    s32 currentGlyph;
    MenuGlyphScript *cursor;
    s32 drawX;
    u16 glyphAdvance;
    s32 drawY;
    u16 paletteScale;

    drawX = x;
    drawY = y;
    if (MENU_GLYPH_PALETTE_INDEX(palette) == 0) {
        glyphAdvance = MENU_GLYPH_WIDE_ADVANCE;
    } else {
        glyphAdvance = MENU_GLYPH_NARROW_ADVANCE;
    }

    // The no-op preserves IDO's initial-glyph register allocation.
    initialGlyph = *script ^ 0;
    if (initialGlyph != MENU_GLYPH_SCRIPT_END) {
        cursor = script;
        paletteScale = scale;
        currentGlyph = initialGlyph;
        do {
            if ((currentGlyph & 0xFFFF) == MENU_GLYPH_SCRIPT_NEWLINE) {
                drawX = x;
                drawY += MENU_GLYPH_LINE_HEIGHT;
            } else {
                if ((currentGlyph & 0xFFFF) != MENU_GLYPH_SCRIPT_SPACE) {
                    drawMenuGlyph(
                        drawX,
                        drawY,
                        currentGlyph & 0xFFFF,
                        MENU_GLYPH_PALETTE_INDEX(palette),
                        paletteScale,
                        MENU_GLYPH_DEFAULT_FONT_BANK
                    );
                }
                drawX += glyphAdvance;
            }
            cursor++;
            currentGlyph = *cursor;
        } while ((currentGlyph & 0xFFFF) != MENU_GLYPH_SCRIPT_END);
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/drawMenuGlyph.s")

void drawMenuColoredGlyphScript(
    volatile s16 x,
    s16 y,
    MenuGlyphScript *script,
    s32 palette,
    u16 scale,
    u16 colorMode,
    u16 fontBank
) {
    u16 firstGlyph;
    s32 glyphCode;
    MenuGlyphScript *scriptCursor;
    s32 drawX;
    u16 glyphAdvance;
    s32 drawY;
    s32 spaceGlyph;
    s32 advance;
    u16 scaleValue;
    u16 colorModeValue;
    u16 fontBankValue;

    drawX = x;
    drawY = y;
    if (MENU_GLYPH_PALETTE_INDEX(palette) == 0) {
        glyphAdvance = MENU_GLYPH_WIDE_ADVANCE;
    } else {
        glyphAdvance = MENU_GLYPH_NARROW_ADVANCE;
    }

    firstGlyph = *script ^ 0;
    if (MENU_GLYPH_SCRIPT_END != firstGlyph) {
        scriptCursor = script;
        scaleValue = scale;
        glyphCode = firstGlyph;
        colorModeValue = colorMode;
        fontBankValue = fontBank;
        do {
            spaceGlyph = MENU_GLYPH_SCRIPT_SPACE;
            if (MENU_GLYPH_SCRIPT_NEWLINE == (glyphCode & 0xFFFF)) {
                drawX = x;
                drawY += MENU_GLYPH_LINE_HEIGHT;
            } else {
                advance = glyphAdvance;
                if (spaceGlyph != (glyphCode & 0xFFFF)) {
                    drawMenuColoredGlyph(
                        drawX,
                        drawY,
                        glyphCode & 0xFFFF,
                        MENU_GLYPH_PALETTE_INDEX(palette),
                        scaleValue,
                        colorModeValue,
                        fontBankValue
                    );
                }
                drawX += advance;
            }
            glyphCode = scriptCursor[1];
            scriptCursor++;
        } while (MENU_GLYPH_SCRIPT_END != (glyphCode & 0xFFFF));
    }
}

void drawMenuGlyphScript(
    volatile s16 x,
    s16 y,
    MenuGlyphScript *script,
    s32 palette,
    u16 scale,
    volatile u16 colorMode
) {
    u16 firstGlyph;
    s32 glyphCode;
    MenuGlyphScript *scriptCursor;
    s32 drawX;
    s32 drawY;
    s32 spaceGlyph;
    register s32 advance;
    u16 glyphAdvance;
    u16 scaleValue;
    u16 colorModeValue;

    drawX = x;
    drawY = y;
    if (MENU_GLYPH_PALETTE_INDEX(palette) == 0) {
        glyphAdvance = MENU_GLYPH_WIDE_ADVANCE;
    } else {
        glyphAdvance = MENU_GLYPH_NARROW_ADVANCE;
    }

    firstGlyph = *script ^ 0;
    if (MENU_GLYPH_SCRIPT_END != firstGlyph) {
        scriptCursor = script;
        scaleValue = scale;
        glyphCode = firstGlyph;
        colorModeValue = colorMode;
        if (scale) {}
        do {
            spaceGlyph = MENU_GLYPH_SCRIPT_SPACE;
            if (MENU_GLYPH_SCRIPT_NEWLINE == (glyphCode & 0xFFFF)) {
                drawX = x;
                drawY += MENU_GLYPH_LINE_HEIGHT;
            } else if (spaceGlyph == (glyphCode & 0xFFFF)) {
                drawX += glyphAdvance;
                if (0) {}
            } else if ((glyphCode & 0xFFFF) == MENU_GLYPH_SCRIPT_COLOR) {
                colorModeValue = scriptCursor[1];
                scriptCursor++;
            } else {
                drawMenuColoredGlyph(
                    drawX,
                    drawY,
                    glyphCode & 0xFFFF,
                    MENU_GLYPH_PALETTE_INDEX(palette),
                    scaleValue,
                    colorModeValue,
                    MENU_GLYPH_DEFAULT_FONT_BANK
                );
                drawX += (advance = glyphAdvance);
            }
            glyphCode = scriptCursor[1];
            scriptCursor++;
        } while (MENU_GLYPH_SCRIPT_END != (glyphCode & 0xFFFF));
        colorMode = colorModeValue;
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_renderer/drawMenuColoredGlyph.s")

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

                        gDPLoadTextureTile_4b(
                            gRegionAllocPtr++,
                            texture->imageOffset + (u8 *)font,
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
                        gDPLoadTLUT_pal16(gRegionAllocPtr++, 0, scratch);
                        gSPTextureRectangle(
                            gRegionAllocPtr++,
                            x0 << 2,
                            storedY << 2,
                            x1 << 2,
                            y1 << 2,
                            G_TX_RENDERTILE,
                            clipS << 5,
                            clipT << 5,
                            0x400,
                            0x400
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

void drawMenuSolidRect(s16 x0, s16 y0, s16 x1, s16 y1, s16 r, s16 g, s16 b) {
    if ((x0 >= 0) && (x0 < 321) && (x1 >= 0) && (x1 < 321) && (y0 >= 0) && (y0 < 241) && (y1 >= 0) && (y1 < 241)) {
        gDPPipeSync(gRegionAllocPtr++);
        gDPSetCycleType(gRegionAllocPtr++, G_CYC_FILL);
        gDPSetRenderMode(gRegionAllocPtr++, G_RM_NOOP, G_RM_NOOP2);
        gDPSetFillColor(gRegionAllocPtr++, (GPACK_RGBA5551(r, g, b, 1) << 16) | GPACK_RGBA5551(r, g, b, 1));
        gDPFillRectangle(gRegionAllocPtr++, x0, y0, x1, y1);
        gSPDisplayList(gRegionAllocPtr++, gMenuRenderModeResetDl);
    }
}

s32 stepMenuFadeAlpha(s32 value, s16 step, u8 increase) {
    if (increase) {
    } else {
        step = step * -1;
    }

    value += step;
    if (increase) {
        if (value >= 0x100) {
            value = 0xFF;
        }
    } else if (value < 0) {
        value = 0;
    }

    gMenuFadeAlpha = value;
    return value;
}
