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

#define MENU_GLYPH_SCRIPT_NEWLINE 0xFFFD
#define MENU_GLYPH_SCRIPT_SPACE 0xFFFE
#define MENU_GLYPH_SCRIPT_END 0xFFFF
#define MENU_GLYPH_WIDE_ADVANCE 0x10
#define MENU_GLYPH_NARROW_ADVANCE 8
#define MENU_GLYPH_LINE_HEIGHT 0x10
#define MENU_GLYPH_PALETTE_INDEX(palette) (((u8 *)&(palette))[3])
#define MENU_GLYPH_SCRIPT_COLOR 0xFFFC
#define MENU_GLYPH_DEFAULT_FONT_BANK 0x22

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
