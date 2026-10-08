#include <PR/gu.h>
#include "game/audio/audio_engine_internal.h"

void __MusIntMemSet(void *p, unsigned char c, s32 n) {
    u8 *q = p;

    while (n--) {
        *(q++) = c;
    }
}
