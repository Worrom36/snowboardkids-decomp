#include "game/menu/race_setup/race_setup_ui.h"

void initMenuTilemapSprite(MenuTilemapSprite *arg0, MenuTilemapSpriteAsset *arg1) {
    MenuTilemapSprite *dst = arg0;
    MenuTilemapSpriteAsset *src = arg1;

    dst->render.scrollX = 0;
    dst->render.scrollY = 0;
    dst->render.tilemapWidth = src->tilemapWidth;
    dst->render.tilemapHeight = src->tilemapHeight;
    dst->render.tileWidth = src->tileWidth;
    dst->render.tileHeight = src->tileHeight;
    dst->render.viewportX = -0x90;
    dst->render.viewportY = -0x68;
    dst->render.viewportWidth = 0x120;
    dst->render.viewportHeight = 0xD0;
    dst->render.tilemap = (s16 *)(src->tilemapOffset + (u8 *)src);
    dst->render.images = (u16 *)(src->imageOffset + (u8 *)src);
    dst->render.tiles = src->tiles;
    dst->render.paletteData = src->paletteOffset + (u8 *)src;
    dst->unk24 = -1;
}

void n_alSynFreeFX(s32 arg0) {
}
