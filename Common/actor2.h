/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: actor.h
	Programmer	: Matthew Cloy
	Date		: 11/11/98

----------------------------------------------------------------------------------------------- */

#ifndef ACTOR2_H_INCLUDED
#define ACTOR2_H_INCLUDED

#include "General.h"
#include "types.h"
#include "sonylibs.h"
#include "islpsi.h"

//#include <islpsi.h>

#ifdef PSX_VERSION
#include "shell.h"
#include "psiactor.h"
#else
#include <actor.h>
#endif


#include "map.h"


//----- [ ACTOR2 DRAWING FLAGS ] -----//

#define	ACTOR_DRAW_NEVER				(1 << 0)	//1
#define ACTOR_DRAW_CULLED				(1 << 1)	//2
#define ACTOR_DRAW_ALWAYS				(1 << 2)	//4
#define ACTOR_DRAW_LAST					(1 << 3)	//8
#define ACTOR_WATER							(1 << 4)	//16
#define ACTOR_SLIDYTEX					(1 << 5)	//32
#define ACTOR_SLIDYTEX2					(1 << 6)	//64
#define ACTOR_MODGETEX					(1 << 7)	//128
#define ACTOR_ADDITIVE					(1 << 8)	//256
#define ACTOR_SLUDGE						(1 << 9)	//512
#define ACTOR_LEAVES						(1 << 10)	//1024
#define ACTOR_FLOWING						(1 << 11)	//2048
#define ACTOR_SLOWSLIDE					(1 << 12)	//4096

//bbxx remove this flag?
#define ACTOR_NEW_NOREORIENTATE ( 1 << 13 )

#define ACTOR_NOANIMATION				( 1 << 14 )
#define LOOKAT_ANYWHERE		1
#define LOOKAT_2D			2


//------------------------------------------------------------------------------------------------


typedef struct TAGACTOR2
{
	ACTOR			*actor;
	
	struct TAGACTOR2	*next,*prev;

	char			draw;
	unsigned short	flags;
	unsigned long	effects;
	unsigned long	fxCount;
	fixed			radius;
//	fixed			angle;
	fixed			animSpeed;
	fixed			value1;

	fixed			distanceFromFrog;		// <--- will need changing !!!

//	fixed			speed;
//	fixed			offset;

	//bb for screen clipping
	char clipped;
	char milesAway;

	// JH: For playstation depth shifting
	signed char depthShift;
	signed char dffClipping; //distancefromfrog, not screen clipping

// JH: Ok, you monkeys!!!!  This is because, it is use's the fast routine drawing, I can't
// JH: put it into the actor struct, because for that I need an actor and to create and actor
// JH: I need a psi, and there crap.......
#ifdef PSX_VERSION
	FMA_WORLD *bffActor;
#endif


} ACTOR2;


#ifdef __cplusplus
extern "C" {
#endif

extern int psiActorCount, fmaActorCount;


extern fixed ACTOR_DRAWDISTANCEINNER;
extern fixed ACTOR_DRAWDISTANCEOUTER;

extern int objectMatrix;

extern ACTOR2 *actList;
extern ACTOR2 *globalLevelActor;

extern int uniqueActorCRC[];
extern int numUniqueActors;

extern int actorCount;

extern int actorsCount;

extern long TestDistanceFromFrog;

#ifdef PSX_VERSION
extern unsigned short globalClut;
extern short actorShiftDepth;
#endif
extern ACTOR2 *backDropObject;

ACTOR2 *CreateAndAddActor(char *name, short cx, short cy, short cz, int initFlags);
void DrawActorList ( void );
void FreeActorList();
void ResetUniqueActorList();
void Orientate(IQUATERNION *me, FVECTOR *fd, FVECTOR *up);
void OrientateSS(IQUATERNION *me, SVECTOR *fd, SVECTOR *up);
void OrientateFS(IQUATERNION *me, FVECTOR *fd, SVECTOR *up);
void OrientateSF(IQUATERNION *me, SVECTOR *fd, FVECTOR *up);

void actor2Free(ACTOR2 *actor);

void TeleportActorToTile(ACTOR2 *act,GAMETILE *tile,long pl);

#ifdef __cplusplus
}
#endif

#endif