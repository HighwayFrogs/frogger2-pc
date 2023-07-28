/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: general.h
	Programmer	: Matthew Cloy
	Date		: 12/01/99

----------------------------------------------------------------------------------------------- */


#ifndef GENERAL_H_INCLUDED
#define GENERAL_H_INCLUDED

#define inline __inline

//bb
#include "define.h"
#include "newpsx.h"
#include "maths.h"


typedef struct
{
	unsigned short time;
	unsigned long lastCount;

} TIMER;

#define NAME_LENGTH 3

#define GTInit( t, s ) { (t)->time = s; (t)->lastCount = 0; }
#define GTUpdate( t, s ) { if(actFrameCount > ((t)->lastCount+(60*Fabs(s)))){(t)->lastCount=actFrameCount; (t)->time+=s;} }
#define GTUpdate2( t, s ) { if(actFrameCount > ((t)->lastCount+(30*Fabs(s)))){(t)->lastCount=actFrameCount; (t)->time+=s;} }

#ifdef __cplusplus
extern "C" {
#endif

unsigned long gstrcmp(char *a, char *b);
unsigned long gstrlen(const char *a);
void stringChange ( char *name );
void stringChangePSIToOBE ( char *name );

char *gstrlwr( char *str );
char *gstrupr( char *str );

inline void smemcpy( unsigned short *a, unsigned short *b, unsigned long size );
inline void lmemcpy( long *a, long *b, long size );
inline void cmemcpy( char *a, char *b, unsigned long size );

GAMETILE* FindNearestTileV(VECTOR v);
GAMETILE* FindNearestTileF(FVECTOR v);
GAMETILE* FindNearestTileS(SVECTOR v);
GAMETILE* FindNearestJoinedTile(GAMETILE *tile, SVECTOR *pos);
GAMETILE *FindJoinedTileByDirection( GAMETILE *st, FVECTOR *d );
GAMETILE *FindJoinedTileByDirectionConstrained(GAMETILE *st, FVECTOR *d, fixed angle);
GAMETILE *FindJoinedTileByDirectionAndType( GAMETILE *st, FVECTOR *d, int type );
SVECTOR *FindClosestTileVector( GAMETILE *tile, FVECTOR *d );
void LoadSfxMapping( int world, int level );


extern char message[256];





#ifdef __cplusplus
}
#endif
#endif
