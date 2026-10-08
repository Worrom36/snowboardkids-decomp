#include "game/race/race_replay.h"

s32 compressRaceRecordReplayData(u8 *src, s32 srcLen, u16 *dst) {
    s32 count;
    s32 srcPos;
    s32 outCount;
    s32 remaining;
    s32 bestDistance;
    s32 bestLength;
    s32 distance;
    s32 maxLength;
    s32 searchPos;
    s32 length;
    volatile u16 *out;
    u8 *cur;
    u8 *prev;

    srcPos = 0;
    *dst = srcLen;
    outCount = 1;
    remaining = srcLen;
    bestDistance = 0;
    bestLength = 0;
    distance = 1;
    goto initialize_out;

loop:
    maxLength = remaining;
    if (remaining >= 0x40) {
        maxLength = 0x3F;
    }

    searchPos = srcPos - 1;
search_loop:
    if (searchPos < 0) {
        goto search_done;
    }

    length = 0;
    count = 0;
    cur = src;
    goto after_initialize_out;

initialize_out:
    out = 1 + dst;
    goto loop;

after_initialize_out:
    if (maxLength > 0) {
        cur += srcPos;
        prev = src;
        prev = searchPos + prev;
    match_loop:
        count += 1;
        if (*cur != *prev) {
            goto compare_best;
        }
        cur += 1;
        prev += 1;
        length += 1;
        if (count != maxLength) {
            goto match_loop;
        }
    }

compare_best:
    if (bestLength < length) {
        bestDistance = distance;
        bestLength = length;
    }

    distance += 1;
    searchPos -= 1;
    if (distance < 0x400) {
        goto search_loop;
    }

search_done:
    distance = 1;
    if (bestLength <= 0) {
        *out = src[srcPos];
        outCount += 1;
        out = out + 1;
        srcPos += 1;
    } else {
        *out = (bestLength << 10) | bestDistance;
        outCount += 1;
        out = out + 1;
        srcPos += bestLength;
    }

    if (srcLen != srcPos) {
        remaining = srcLen - srcPos;
        if (outCount >= 0x1000) {
            return -1;
        }
        bestDistance = 0;
        distance = 1;
        bestLength = 0;
        goto loop;
    }
    return outCount;
}
