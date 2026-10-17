#ifndef _GBI_H_
#define _GBI_H_
#include <PR/ultratypes.h>
typedef union {
    long long force_structure_alignment;
    struct { unsigned int hi; unsigned int lo; } words;
} Gfx;
#endif
