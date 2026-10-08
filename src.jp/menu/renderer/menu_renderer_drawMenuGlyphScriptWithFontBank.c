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
#define MENU_GLYPH_DEFAULT_FONT_BANK 0x22

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
