/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: actor.h
	Programmer	: Matthew Cloy
	Date		: 11/11/98

----------------------------------------------------------------------------------------------- */

#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED

#define ANIM_QUEUE_LENGTH	8

#ifdef __cplusplus
extern "C"
{
#endif

#include "ultra64.h"
#include "libgte.h"

#include "fixed.h"
#include "types.h"

typedef struct
{
	short		numAnimations;
	short		currentAnimation;

	unsigned short			*animSegments;			// list of start-end frames (shorts)

	u8		reachedEndOfAnimation;
	u8		loopAnimation;
	short 		numberQueued;

	long		animationSpeed;

	short		queueAnimation[ANIM_QUEUE_LENGTH];
	u8			queueLoopAnimation[ANIM_QUEUE_LENGTH];
	long		queueAnimationSpeed[ANIM_QUEUE_LENGTH];
	long		animTime;

	short		frame;
	u8 		exclusive;
	u8 		spare;

} ACTOR_ANIMATION;

typedef struct
{
	int							visible;
	SVECTOR					oldPosition;	//
	SVECTOR					position;
	SVECTOR					rotaim;
	FVECTOR					size;
	FVECTOR					vel;
	ACTOR_ANIMATION			animation;
	ACTOR_SHADOW			*shadow;

	//bb
	IQUATERNION qRot;

	//bb - temp to compile
	//pc
	void *actualActor;
	char* name;

} ACTOR;


void actorFree(ACTOR *actor);
void actorAnimate(ACTOR *actor, int animNum, char loop, char queue, int speed, char skipendframe);
void actorResetAnimation(ACTOR *actor);

void ChangeModel( ACTOR *actor, char *model );
int UndoChangeModel( ACTOR *actor );

void StartAnimateActor(ACTOR *actor, int animNum, char loop, char queue, int speed, char skip);

void FindFrogSubObjects( int p );
void UpdateFrogCroak( int pl );

#ifdef __cplusplus
}
#endif


#endif