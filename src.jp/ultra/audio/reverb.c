/*====================================================================
 * reverb.c
 *
 * Copyright 1993, Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
 * Inc.; the contents of this file may not be disclosed to third
 * parties, copied or duplicated in any form, in whole or in part,
 * without the prior written permission of Silicon Graphics, Inc.
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to
 * restrictions as set forth in subdivision (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS
 * 252.227-7013, and/or in similar or successor clauses in the FAR,
 * DOD or NASA FAR Supplement. Unpublished - rights reserved under the
 * Copyright Laws of the United States.
 *====================================================================*/
#include "PR/libaudio.h"
#include "PR/ultraerror.h"
#include "synthInternals.h"
#include "PR/os.h"
#include "PR/os_internal.h"
#include <assert.h>
#include "initfx.h"

#define RANGE 2.0

f32 _doModFunc(ALDelay *d, s32 count) {
    f32 val;

    /*
     * generate bipolar sawtooth
     * from -RANGE to +RANGE
     */
    d->rsval += d->rsinc * count;
    d->rsval = (d->rsval > RANGE) ? d->rsval - (RANGE * 2) : d->rsval;

    /*
     * convert to monopolar triangle
     * from 0 to RANGE
     */
    val = d->rsval;
    val = (val < 0) ? -val : val;

    /*
     * convert to bipolar triangle
     * from -1 to 1
     */
    val -= RANGE / 2;

    return (d->rsgain * val);
}
