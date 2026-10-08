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

#define PACKED_MTX_WORD(matrix, index) ((matrix)->m[(index) / 4][(index) % 4])

typedef struct {
    s16 xx;
    s16 xy;
    s16 xz;
    s16 yx;
    s16 yy;
    s16 yz;
    s16 zx;
    s16 zy;
    s16 zz;
    Vec3i translation;
} Transform3DComponents;

typedef struct {
    s32 w0, w1, w2, w3, w4, w5, w6, w7;
    s32 w8, w9, w10, w11, w12, w13, w14, w15;
} PackedMtxWords;

Mtx *allocFixedTransformMatrix(Transform3D *source) {
    Mtx *dst = allocMenuRenderScratch(sizeof(Mtx));
    Transform3DComponents *components = (Transform3DComponents *)source;

    if (dst == NULL) {
        return NULL;
    }

    PACKED_MTX_WORD(dst, 0) = ((components->xy >> 12) & 0xFFFF) | ((components->xx << 4) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 1) = (components->xz << 4) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 2) = ((components->yy >> 12) & 0xFFFF) | ((components->yx << 4) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 3) = (components->yz << 4) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 4) = ((components->zy >> 12) & 0xFFFF) | ((components->zx << 4) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 5) = (components->zz << 4) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 6) = ((source->translation.y >> 16) & 0xFFFF) | (source->translation.x & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 7) = (source->translation.z & 0xFFFF0000) | 1;
    PACKED_MTX_WORD(dst, 8) = ((components->xy << 4) & 0xFFFF) | ((components->xx << 20) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 9) = (components->xz << 20) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 10) = ((components->yy << 4) & 0xFFFF) | ((components->yx << 20) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 11) = (components->yz << 20) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 12) = ((components->zy << 4) & 0xFFFF) | ((components->zx << 20) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 13) = (components->zz << 20) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 14) = (source->translation.y & 0xFFFF) | ((source->translation.x << 16) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 15) = (source->translation.z << 16) & 0xFFFF0000;
    return dst;
}

Mtx *allocFixedRotationMatrix(Mat3x3 rotation) {
    Mtx *dst = allocMenuRenderScratch(sizeof(Mtx));
    Transform3DComponents *components = (Transform3DComponents *)rotation;

    if (dst == NULL) {
        return NULL;
    }

    PACKED_MTX_WORD(dst, 0) = ((components->xy >> 12) & 0xFFFF) | ((components->xx << 4) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 1) = (components->xz << 4) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 2) = ((components->yy >> 12) & 0xFFFF) | ((components->yx << 4) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 3) = (components->yz << 4) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 4) = ((components->zy >> 12) & 0xFFFF) | ((components->zx << 4) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 5) = (components->zz << 4) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 6) = 0;
    PACKED_MTX_WORD(dst, 7) = 1;
    PACKED_MTX_WORD(dst, 8) = ((components->xy << 4) & 0xFFFF) | ((components->xx << 20) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 9) = (components->xz << 20) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 10) = ((components->yy << 4) & 0xFFFF) | ((components->yx << 20) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 11) = (components->yz << 20) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 12) = ((components->zy << 4) & 0xFFFF) | ((components->zx << 20) & 0xFFFF0000);
    PACKED_MTX_WORD(dst, 13) = (components->zz << 20) & 0xFFFF0000;
    PACKED_MTX_WORD(dst, 14) = 0;
    PACKED_MTX_WORD(dst, 15) = 0;
    return dst;
}

Mtx *allocTranslationOnlyFixedMatrix(Mtx *source) {
    PackedMtxWords *dst = allocMenuRenderScratch(sizeof(Mtx));
    PackedMtxWords *sourceWords = (PackedMtxWords *)source;

    if (dst == NULL) {
        return NULL;
    }

    dst->w0 = 0x10000;
    dst->w1 = 0;
    dst->w2 = 1;
    dst->w3 = 0;
    dst->w4 = 0;
    dst->w5 = 0x10000;
    dst->w6 = ((sourceWords->w6 >> 16) & 0xFFFF) | (sourceWords->w5 & 0xFFFF0000);
    dst->w7 = (sourceWords->w7 & 0xFFFF0000) | 1;
    dst->w8 = 0;
    dst->w9 = 0;
    dst->w10 = 0;
    dst->w11 = 0;
    dst->w12 = 0;
    dst->w13 = 0;
    dst->w14 = (sourceWords->w6 & 0xFFFF) | ((sourceWords->w5 << 16) & 0xFFFF0000);
    dst->w15 = (sourceWords->w7 << 16) & 0xFFFF0000;
    return (Mtx *)dst;
}
