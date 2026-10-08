/*====================================================================
 * drvrNew.c
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
#include "initfx.h"

#define SCALE 16384

void _init_lpfilter(ALLowPass *lp) {
    s32 i, temp;
    s16 fc;
    f64 ffc, fcoef;

    temp = lp->fc * SCALE;
    fc = temp >> 15;
    lp->fgain = SCALE - fc;

    lp->first = 1;
    for (i = 0; i < 8; i++)
        lp->fcvec.fccoef[i] = 0;

    lp->fcvec.fccoef[i++] = fc;
    fcoef = ffc = (f64)fc / SCALE;

    for (; i < 16; i++) {
        fcoef *= ffc;
        lp->fcvec.fccoef[i] = (s16)(fcoef * SCALE);
    }
}
