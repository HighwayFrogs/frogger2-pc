/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: tongue.h
	Programmer	: Andrew Eder
	Date		: 12/15/98

----------------------------------------------------------------------------------------------- */

#ifndef TONGUE_H_INCLUDED
#define TONGUE_H_INCLUDED

#include "types.h"
#include "frogger.h"
#include "enemies.h"
#include "collect.h"



#define	MAX_TONGUENODES				12
#define TONGUE_FRACTION				(4096/MAX_TONGUENODES)

#define TONGUE_RADIUSNORMAL			1800
#define TONGUE_RADIUSLONG			2250

#define TONGUE_STICKYRADIUS			409600
#define TONGUE_WRAPAROUNDTHRESHOLD	-3072


enum
{
	TONGUE_GET_BABY,
	TONGUE_GET_GARIB,
	TONGUE_GET_FROG,
	TONGUE_GET_SCENIC,
	TONGUE_GET_PICKUP,
	TONGUE_GET_NONE
};


typedef struct
{
	SVECTOR source, target, pos;
	FVECTOR fwd, interp;

	unsigned long flags;
	fixed radius, progress;
	unsigned char type, canTongue;

	TextureType *tex;
	FVECTOR *segment;

	void *thing;	// What we're trying to collect

} TONGUE;


//----- [ FROG TONGUE FLAGS ] ----------------------------------------------

#define TONGUE_NONE				0
#define TONGUE_IDLE				(1 << 0)
#define TONGUE_BEINGUSED		(1 << 1)
#define TONGUE_OUTGOING			(1 << 2)
#define TONGUE_INCOMING			(1 << 3)
#define TONGUE_HASITEMONIT		(1 << 4)
#define TONGUE_HASITEMINMOUTH	(1 << 5)
#define TONGUE_THROWING			(1 << 6)
#define TONGUE_SEARCHING		(1 << 7)
#define TONGUE_GRAPPLE          (1 << 8)

//----- [ GLOBALS ] --------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char eatEverythingMode;
extern TONGUE tongue[MAX_FROGS];
extern unsigned long tongueColours[];

//----- [ FUNCTION PROTOTYPES ] --------------------------------------------

extern void InitTongues( );
extern void UpdateFrogTongue( int pl );
extern void RemoveFrogTongue( int pl );
extern void FreeTongues( );

extern ENEMY *BabyFrogIsInRange(fixed radius, int pl);
extern GARIB *GaribIsInRange(fixed radius, int pl);
extern ENEMY *ScenicIsInRange(fixed radius, int pl);
extern ACTOR2 *FrogIsInRange(fixed radius, int pl);

extern void ThrowFrogDirection( int thrower, int throwee, int dir );

#ifdef __cplusplus
}
#endif
#endif
