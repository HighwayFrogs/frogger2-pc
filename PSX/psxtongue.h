#ifndef PSXTONGUE_INCLUDED
#define PSXTONGUE_INCLUDED

#include "tongue.h"

void DrawTongueSegment ( SVECTOR *vt, TextureType *tEntry, unsigned char r, unsigned char g, unsigned char b );

void CalcTongueNodes(SVECTOR *vT, int pl, int i);

void DrawTongue( int pl );

#endif