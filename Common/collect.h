/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: collect.h
	Programmer	: Andrew Eder
	Date		: 11/27/98

----------------------------------------------------------------------------------------------- */

#ifndef COLLECT_H_INCLUDED
#define COLLECT_H_INCLUDED

#include "specfx.h"
#include "sprite.h"
#include "define.h"
#include "types2d.h"

// ----- [ DEFINES ] --------------- //

#ifdef __cplusplus
extern "C" {
#endif

#define PICKUP_RADIUS			(400<<12)

enum
{
	SILVERCOIN_GARIB,
	EXTRALIFE_GARIB,
	AUTOHOP_GARIB,
	LONGTONGUE_GARIB,
	QUICKHOP_GARIB,
	INVULNERABILITY_GARIB,
	GOLDCOIN_GARIB,
	SLOWHOP_GARIB,

	NUM_GARIB_TYPES
};

typedef struct TAGGARIB
{
	struct TAGGARIB		*prev,*next;
	char				active;
	unsigned char				type;
//	fixed				scale;
//	fixed				scaleAim;
	SPRITE				*sprite;
	SPECFX				*fx;
	SVECTOR				pos;
//	unsigned long		flags;
//	GAMETILE			*gameTile;
	long				dropSpeed;
} GARIB;

typedef struct TAGGARIBLIST
{
	GARIB	*array;
	GARIB	head;
	short	blocks, count, total, maxCoins;

} GARIBLIST;


extern GARIBLIST garibList;
extern TIMER pauseTimer;
extern int pauseFlag;

extern char pickupObjName[];

// ----- [ FUNCTION PROTOTYPES ] --------------- //

void CheckTileForCollectable(GAMETILE *tile,long pl);

void ProcessCollectables();
void PickupCollectable(GARIB *garib, int pl);

void AllocGaribBlock( int num );
void InitGaribList();
void FreeGaribList();
void AddGarib(GARIB *garib);
void SubGarib(GARIB *garib);

void InitGaribSprite(GARIB *garib);
GARIB *CreateNewGarib(SVECTOR pos, int type);
void UpdateGaribs();

void ReactivatePowerups( );
//void DropGaribToTile(GARIB *garib, GAMETILE *tile, short dropSpeed);

#ifdef __cplusplus
}
#endif
#endif
