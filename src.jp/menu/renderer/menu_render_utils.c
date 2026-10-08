#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/renderer/sprite_palette_relocation.h"

void *resolveAssetTableRelativePointer(void *arg0, u32 arg1) {
    return (void *)((u8 *)arg0 + (arg1 & 0xFFFFFF));
}

void getAssetTableImageAndPalette(void *asset, u16 arg1, void **arg2, void **arg3) {
    AssetTableEntry *temp_v1;
    u8 *temp_v0;
    short idx;
    u8 *arg0 = asset;

    temp_v1 = (AssetTableEntry *)((s32)arg0 + (arg1 * sizeof(AssetTableEntry)));
    temp_v0 = arg0 + (((AssetTable *)arg0)->entryCount * sizeof(AssetTableEntry));
    idx = 1;
    *arg2 = (void *)(arg0 + temp_v1[idx].imageOffset);
    temp_v0 += 8;
    *arg3 = (void *)((temp_v1[idx].paletteIndex << 5) + temp_v0);
}

void getAssetTableImageAndExplicitPalette(u8 *arg0, u16 arg1, u16 arg2, void **arg3, void **arg4) {
    u8 *temp_v0;
    AssetTableEntry *temp_v1;

    temp_v1 = (AssetTableEntry *)((s32)arg0 + (arg1 * sizeof(AssetTableEntry)));
    temp_v0 = arg0 + (((AssetTable *)arg0)->entryCount * sizeof(AssetTableEntry));
    *arg3 = (void *)(arg0 + temp_v1[1].imageOffset);
    temp_v0 += 8;
    *arg4 = (void *)((SPRITE_PALETTE_INDEX((AssetTable *)arg0, arg2) << 5) + temp_v0);
}

void getAssetTableImagePaletteAndSize(u8 *arg0, u16 arg1, void **arg2, void **arg3, s16 *arg4, s16 *arg5) {
    AssetTableEntry *temp_v1;
    u8 *temp_v0;
    short idx;

    temp_v1 = (AssetTableEntry *)((s32)arg0 + (arg1 * sizeof(AssetTableEntry)));
    temp_v0 = arg0 + (((AssetTable *)arg0)->entryCount * sizeof(AssetTableEntry));
    idx = 1;
    *arg2 = (void *)(arg0 + temp_v1[idx].imageOffset);
    temp_v0 += 8;
    *arg3 = (void *)((temp_v1[idx].paletteIndex << 5) + temp_v0);
    *arg4 = temp_v1[idx].width;
    *arg5 = temp_v1[idx].height;
}
