/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: pcmisc.h
	Programmer	: Andy Eder
	Date		: 05/05/99 17:43:09

----------------------------------------------------------------------------------------------- */

#ifndef PCMISC_H_INCLUDED
#define PCMISC_H_INCLUDED


#ifdef __cplusplus
extern "C"
{
#endif

#include <ddraw.h>
#include <isltex.h>

extern short mouseX;
extern short mouseY;

extern int drawOverlays;

extern float OVERLAY_X;
extern float OVERLAY_Y;

extern LPDIRECTDRAWSURFACE7 pSurface;

//----- [ FUNCTION PROTOTYPES ] ----------------------------------------------------------------//

TextureType *FindTexture( char *name );

void InitPCSpecifics();
void DeInitPCSpecifics();
short *GetGelfBmpDataAsShortPtr(char *filename);
void FreeGelfData(void *me);
void ClearTimers(void);
void StartTimer(int number,char *name);
void EndTimer(int number);
void PrintTimers(void);
void HoldTimers(void);

void PcNameEntryInit(void);

LPDIRECTDRAWSURFACE7 LoadEditorTexture(const char*);

extern void PTSurfaceBlit( TextureType *tex, unsigned char *buf, unsigned short *pal );
extern void PTTextureLoad( );
extern void CreateProceduralTexture( TextureType *tex, char *name );

//----- [ FLOAT VECTOR MACROS ] ----------------------------------------------------------------//
// Note: 'R' denoted real-valued (i.e. PC floating point) vectors

#define SetVectorRS(v1, v2)						   (v1)->vx = (float)(v2)->vx,\
												   (v1)->vy = (float)(v2)->vy,\
												   (v1)->vz = (float)(v2)->vz

#define SetVectorSR(v1, v2)						   (v1)->vx = (short)(v2)->vx,\
												   (v1)->vy = (short)(v2)->vy,\
												   (v1)->vz = (short)(v2)->vz

#define SetVectorRF(v1, v2)						   (v1)->vx = ((float)(v2)->vx*ONEOVERFIXED),\
												   (v1)->vy = ((float)(v2)->vy*ONEOVERFIXED),\
												   (v1)->vz = ((float)(v2)->vz*ONEOVERFIXED)

#define SetVectorFR(v1, v2)						   (v1)->vx = (int)((v2)->vx*4096),\
												   (v1)->vy = (int)((v2)->vy*4096),\
												   (v1)->vz = (int)((v2)->vz*4096)


#define ROUND2SHORT(x)				((short)((x) + 0.5F))
#define ROUND2INT(x)				((int)((x) + 0.5F))
#define ROUND2LONG(x)				((long)((x) + 0.5F))

#define FOGADJ(x) (1.0-((x-fStart)*fEnd))
#define FOGVAL(y) (((unsigned long)(255*y) << 24))

void mtxSetIdent( float *m );
void CopyTexture ( TextureType *dest, TextureType *src, int copyPalette );

int OutcodeCheck( SVECTOR *p1, SVECTOR *p2 );

#ifdef __cplusplus
}
#endif


#endif
