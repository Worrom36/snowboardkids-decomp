#ifndef SPRITE_PALETTE_RELOCATION_H
#define SPRITE_PALETTE_RELOCATION_H

#include "game/menu/renderer/menu_renderer.h"

#ifdef SBK_SPRITE_PALETTE_REMAP
/* A moved palette bank appends this table. AssetTable.unk0 retains its low
 * format bit and encodes the table byte offset in bits 1..30, with bit 31 set. */
typedef struct SpritePaletteRemap {
    u16 count;
    u16 slots[1];
} SpritePaletteRemap;

u16 resolveSpritePaletteIndex(AssetTable *table, u16 index);
#define SPRITE_PALETTE_INDEX(table, index) resolveSpritePaletteIndex((table), (index))
#else
#define SPRITE_PALETTE_INDEX(table, index) (index)
#endif

#endif
