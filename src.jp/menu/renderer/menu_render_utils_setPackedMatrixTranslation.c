#include "game/math/fixed_point_math.h"
#include "game/menu/renderer/menu_render_utils.h"

#define PACKED_MTX_WORD(matrix, index) ((matrix)->m[(index) / 4][(index) % 4])

typedef struct {
    s32 w0, w1, w2, w3, w4, w5, w6, w7;
    s32 w8, w9, w10, w11, w12, w13, w14, w15;
} PackedMtxWords;

void setPackedMatrixTranslation(Mtx *matrix, Vec3i *translation) {
    PACKED_MTX_WORD(matrix, 6) = (translation->x & 0xFFFF0000) | ((translation->y >> 16) & 0xFFFF);
    PACKED_MTX_WORD(matrix, 7) = (translation->z & 0xFFFF0000) | 1;
    PACKED_MTX_WORD(matrix, 14) = ((translation->x << 16) & 0xFFFF0000) | (translation->y & 0xFFFF);
    PACKED_MTX_WORD(matrix, 15) = (translation->z << 16) & 0xFFFF0000;
}

void copyPackedMatrixTranslation(Mtx *source, Mtx *dest) {
    PackedMtxWords *destWords = (PackedMtxWords *)dest;
    s32 *word6;
    s32 *word5;
    PackedMtxWords *sourceWords = (PackedMtxWords *)source;

    word5 = &sourceWords->w5;
    word6 = &sourceWords->w6;
    destWords->w6 = (*word5 & 0xFFFF0000) | ((*word6 >> 16) & 0xFFFF);
    destWords->w7 = (sourceWords->w7 & 0xFFFF0000) | 1;
    destWords->w14 = ((*word5 << 16) & 0xFFFF0000) | (*word6 & 0xFFFF);
    destWords->w15 = (sourceWords->w7 << 16) & 0xFFFF0000;
}

void scaleFixedMatrix3sByQuarter(Mat3x3 arg0) {
    arg0[0] = arg0[0] / 4;
    arg0[1] = arg0[1] / 4;
    arg0[2] = arg0[2] / 4;
    arg0[3] = arg0[3] / 4;
    arg0[4] = arg0[4] / 4;
    arg0[5] = arg0[5] / 4;
    arg0[6] = arg0[6] / 4;
    arg0[7] = arg0[7] / 4;
    arg0[8] = arg0[8] / 4;
}

void noopThreeArgs(void *arg0, void *arg1, void *arg2) {
}

void noopFourArgs(void *arg0, void *arg1, void *arg2, void *arg3) {
}
