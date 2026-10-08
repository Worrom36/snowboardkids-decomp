#include "game/menu/renderer/menu_render_utils.h"

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

void packFixedTransformMatrix(Transform3D *source, Mtx *dest) {
    Transform3DComponents *components = (Transform3DComponents *)source;

    PACKED_MTX_WORD(dest, 0) = ((components->xy >> 12) & 0xFFFF) | ((components->xx << 4) & 0xFFFF0000);
    PACKED_MTX_WORD(dest, 1) = (components->xz << 4) & 0xFFFF0000;
    PACKED_MTX_WORD(dest, 2) = ((components->yy >> 12) & 0xFFFF) | ((components->yx << 4) & 0xFFFF0000);
    PACKED_MTX_WORD(dest, 3) = (components->yz << 4) & 0xFFFF0000;
    PACKED_MTX_WORD(dest, 4) = ((components->zy >> 12) & 0xFFFF) | ((components->zx << 4) & 0xFFFF0000);
    PACKED_MTX_WORD(dest, 5) = (components->zz << 4) & 0xFFFF0000;
    PACKED_MTX_WORD(dest, 6) = ((components->translation.y >> 16) & 0xFFFF) | (components->translation.x & 0xFFFF0000);
    PACKED_MTX_WORD(dest, 7) = (components->translation.z & 0xFFFF0000) | 1;
    PACKED_MTX_WORD(dest, 8) = ((components->xy << 4) & 0xFFFF) | ((components->xx << 20) & 0xFFFF0000);
    PACKED_MTX_WORD(dest, 9) = (components->xz << 20) & 0xFFFF0000;
    PACKED_MTX_WORD(dest, 10) = ((components->yy << 4) & 0xFFFF) | ((components->yx << 20) & 0xFFFF0000);
    PACKED_MTX_WORD(dest, 11) = (components->yz << 20) & 0xFFFF0000;
    PACKED_MTX_WORD(dest, 12) = ((components->zy << 4) & 0xFFFF) | ((components->zx << 20) & 0xFFFF0000);
    PACKED_MTX_WORD(dest, 13) = (components->zz << 20) & 0xFFFF0000;
    PACKED_MTX_WORD(dest, 14) = (components->translation.y & 0xFFFF) | ((components->translation.x << 16) & 0xFFFF0000);
    PACKED_MTX_WORD(dest, 15) = (components->translation.z << 16) & 0xFFFF0000;
}
