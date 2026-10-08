#include "game/math/fixed_matrix_multiply.h"
#include "game/math/fixed_point_math.h"

#define FIXED_MATRIX_ROWS(matrix) ((s16(*)[3])(matrix))

void multiplyFixedMatrix3s(Mat3x3 arg0, Mat3x3 arg1, Mat3x3 arg2) {
    s32 i;
    s32 j;

    i = 0;
    do {
        j = 0;
        do {
            FIXED_MATRIX_ROWS(arg2)
            [i][j] = ((FIXED_MATRIX_ROWS(arg0)[i][0] * FIXED_MATRIX_ROWS(arg1)[0][j]) / FIXED_MATRIX_ONE) +
                     ((FIXED_MATRIX_ROWS(arg0)[i][1] * FIXED_MATRIX_ROWS(arg1)[1][j]) / FIXED_MATRIX_ONE) +
                     ((FIXED_MATRIX_ROWS(arg0)[i][2] * FIXED_MATRIX_ROWS(arg1)[2][j]) / FIXED_MATRIX_ONE);
            j++;
        } while (j != 3);
        i++;
    } while (i != 3);
}
