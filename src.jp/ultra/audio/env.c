/*====================================================================
 * envmixer.c
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
#include <assert.h>

s32 alEnvmixerParam(void *filter, s32 paramID, void *param) {
    ALFilter *f = (ALFilter *)filter;
    ALEnvMixer *e = (ALEnvMixer *)filter;

    switch (paramID) {

        case (AL_FILTER_ADD_UPDATE):
            if (e->ctrlTail) {
                e->ctrlTail->next = (ALParam *)param;
            } else {
                e->ctrlList = (ALParam *)param;
            }
            e->ctrlTail = (ALParam *)param;

            break;

        case (AL_FILTER_RESET):
            e->first = 1;
            e->motion = AL_STOPPED;
            e->volume = 1;
            if (f->source)
                (*f->source->setParam)(f->source, AL_FILTER_RESET, param);
            break;

        case (AL_FILTER_START):
            e->motion = AL_PLAYING;
            if (f->source)
                (*f->source->setParam)(f->source, AL_FILTER_START, param);
            break;

        case (AL_FILTER_SET_SOURCE):
            f->source = (ALFilter *)param;
            break;

        default:
            if (f->source)
                (*f->source->setParam)(f->source, paramID, param);
    }
    return 0;
}

f64 _frexpf(f64 value, s32 *eptr) {
    f64 absvalue;

    *eptr = 0;
    if (value == 0.0) /* nothing to do for zero */
        return (value);
    absvalue = (value > 0.0) ? value : -value;
    for (; absvalue >= 1.0; absvalue *= 0.5)
        ++*eptr;
    for (; absvalue < 0.5; absvalue += absvalue)
        --*eptr;
    return (value > 0.0 ? absvalue : -absvalue);
}

f64 _ldexpf(f64 in, s32 ex) {
    s32 exp;

    if (ex) {
        exp = 1 << ex;
        in *= (f64)exp;
    }

    return (in);
}
