#ifndef BBPSIANI_H
#define BBPSIANI_H

#include <stddef.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <inline_c.h>
#include <gtemac.h>
#include <stdio.h>
#include "types.h"
#include "islpsi.h"
#include <islmem.h>
#include <islfile.h>
#include <isltex.h>
#include <islutil.h>

void bb_psiSetKeyFrames(PSIOBJECT *world, ULONG frame);
void bb_psiSetRotateKeyFrames(PSIOBJECT *world, ULONG frame);
void bb_psiSetScaleKeyFrames(PSIOBJECT *world, ULONG frame);
void bb_psiSetMoveKeyFrames(PSIOBJECT *world, ULONG frame);

#endif
