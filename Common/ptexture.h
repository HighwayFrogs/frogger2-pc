/*
	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: ptexture.h
	Purpose		: Procedurally generated textures
	Programmer	: Jim Hubbard
	Date		: 2/12/99 10:30

----------------------------------------------------------------------------------------------- */
#ifndef __PTEXTURE_H
#define __PTEXTURE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <isltex.h>

typedef struct TAGPROCTEXTURE
{
	struct TAGPROCTEXTURE *next;

	unsigned long timer;
	unsigned char *buf1;
	unsigned char *buf2;
	unsigned char active;
	unsigned short *palette;

	TextureType *tex;

	void (*Update) (struct TAGPROCTEXTURE *);

} PROCTEXTURE;


extern PROCTEXTURE *dissolveTex;
extern PROCTEXTURE *prcTexList;

extern void ProcessProcTextures( );
extern void FreeProcTextures( );

extern void ProcessPTFire( PROCTEXTURE *pt );
extern void ProcessPTForcefield( PROCTEXTURE *pt );
extern void ProcessPTWaterRipples( PROCTEXTURE *pt );
extern void ProcessPTWaterDrops( PROCTEXTURE *pt );
extern void ProcessPTWaterBubbler( PROCTEXTURE *pt );
extern void ProcessPTWaterTrail( PROCTEXTURE *pt );
extern void ProcessPTWaterWaves( PROCTEXTURE *pt );
extern void ProcessPTSteam( PROCTEXTURE *pt );
extern void ProcessPTCandle( PROCTEXTURE *pt );
extern void ProcessPTDissolve( PROCTEXTURE *pt );

#ifdef __cplusplus
}
#endif

#endif