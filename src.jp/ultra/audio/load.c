/*====================================================================
 * load.c
 *
 * Copyright 1993, Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
 * Inc.; the contents of this file may not be disclosed to third
 * parties, copied or duplicated in any form, in whole or in part,
 * without the prior written permission of Silicon Graphics, Inc.
 *
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to
 * restrictions as set forth in subdivision (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS
 * 252.227-7013, and/or in similar or successor clauses in the FAR,
 * DOD or NASA FAR Supplement. Unpublished - rights reserved under the
 * Copyright Laws of the United States.
 *====================================================================*/
#include <libaudio.h>
#include "synthInternals.h"
#include <os.h>
#include <R4300.h>
#include "compiler_diagnostics.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

Acmd *alRaw16Pull(void *filter, s16 *outp, s32 outCount, s32 sampleOffset, Acmd *p) {
    Acmd *ptr = p;
    s32 nbytes;
    s32 dramLoc;
    s32 dramAlign;
    s32 dmemAlign;
    s32 overFlow;
    s32 startZero;
    s32 nSam;
    s32 op;

    ALLoadFilter *f = (ALLoadFilter *)filter;
    ALFilter *a = (ALFilter *)filter;

    if (outCount == 0)
        return ptr;

    if ((outCount + f->sample > f->loop.end) && (f->loop.count != 0)) {

        nSam = f->loop.end - f->sample;
        nbytes = nSam << 1;
        if (nSam > 0) {
            dramLoc = (f->dma)(f->memin, nbytes, f->dmaState);

            /*
             * Make sure enough is loaded into DMEM to take care
             * of 8 byte alignment
             */
            dramAlign = dramLoc & 0x7;
            nbytes += dramAlign;
            aSetBuffer(ptr++, 0, *outp, 0, nbytes + 8 - (nbytes & 0x7));
            aLoadBuffer(ptr++, dramLoc - dramAlign);
        } else
            dramAlign = 0;

        /*
         * Fix up output pointer to allow for dram alignment
         */
        *outp += dramAlign;

        f->memin = (s32)f->table->base + (f->loop.start << 1);
        f->sample = f->loop.start;
        op = *outp;

        while (outCount > nSam) {

            op += (nSam << 1);
            outCount -= nSam;
            /*
             * -1 is loop forever
             */
            if ((f->loop.count != -1) && (f->loop.count != 0))
                f->loop.count--;

            /*
             * What to compute.
             */
            nSam = MIN(outCount, f->loop.end - f->loop.start);
            nbytes = nSam << 1;

            /*
             * Do the next section, same as last.
             */
            dramLoc = (f->dma)(f->memin, nbytes, f->dmaState);

            /*
             * Make sure enough is loaded into DMEM to take care
             * of 8 byte alignment
             */
            dramAlign = dramLoc & 0x7;
            nbytes += dramAlign;
            if (op & 0x7)
                dmemAlign = 8 - (op & 0x7);
            else
                dmemAlign = 0;

            aSetBuffer(ptr++, 0, op + dmemAlign, 0, nbytes + 8 - (nbytes & 0x7));
            aLoadBuffer(ptr++, dramLoc - dramAlign);

            /*
             * Merge the two sections in DMEM.
             */
            if (dramAlign || dmemAlign)
                aDMEMMove(ptr++, op + dramAlign + dmemAlign, op, nSam << 1);
        }

        f->sample += outCount;
        f->memin += (outCount << 1);

        return ptr;
    }

    /*
     * The unlooped case, which is executed most of the time
     *
     * overFlow is the number of bytes past the end
     * of the bitstream I try to generate
     */

    nbytes = outCount << 1;
    overFlow = f->memin + nbytes - ((s32)f->table->base + f->table->len);
    if (overFlow < 0)
        overFlow = 0;
    if (overFlow > nbytes)
        overFlow = nbytes;

    if (overFlow < nbytes) {
        if (outCount > 0) {
            nbytes -= overFlow;
            dramLoc = (f->dma)(f->memin, nbytes, f->dmaState);

            /*
             * Make sure enough is loaded into DMEM to take care
             * of 8 byte alignment
             */
            dramAlign = dramLoc & 0x7;
            nbytes += dramAlign;
            aSetBuffer(ptr++, 0, *outp, 0, nbytes + 8 - (nbytes & 0x7));
            aLoadBuffer(ptr++, dramLoc - dramAlign);
        } else
            dramAlign = 0;
        *outp += dramAlign;

        f->sample += outCount;
        f->memin += outCount << 1;
    } else {
        f->memin += outCount << 1;
    }

    /*
     * Put zeros in if necessary
     */
    if (overFlow) {
        startZero = (outCount << 1) - overFlow;
        if (startZero < 0)
            startZero = 0;
        aClearBuffer(ptr++, startZero + *outp, overFlow);
    }
    return ptr;
}
