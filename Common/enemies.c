/*

	This file is part of Frogger2, (c) 1997 Interactive Studios Ltd.


	File		: enemies.c
	Programmer	: Jim Hubbard
	Date		: 1/12/99
	Info		: Animation:
					Different standards for groups of enemy types, which at the moment is

					Homers:
					Walk
					Idle
					Attack

					Path based:
					Walk
					Walk

					Snappers:
					Idle
					Full Attack
					Any extra idles, to be played occasionally

----------------------------------------------------------------------------------------------- */

#include "types.h"

#include "islpsi.h"
#include <islmem.h>
#include <islutil.h>


#include "General.h"
#include "ultra64.h"
#include "enemies.h"
#include "frogger.h"
#include "maths.h"
#include "babyfrog.h"
#include "frogmove.h"
#include "platform.h"
#include "multi.h"
#include "anim.h"
#include "game.h"
#include "frontend.h"
#include "map.h"
#include "newpsx.h"
#include "cam.h"

#include "sonylibs.h"

#include "Main.h"
#include "maths.h"

#include "layout.h"

#ifdef PSX_VERSION
#include "audio.h"
#include "textures.h"
#include "timer.h"
#include "psiactor.h"
#else
#include <pcaudio.h>
#endif

// Limited animation updates so the PSX doesn't hurt itself trying to do
// anything too clever 

#define NME_RUMBLE		0
#define NME_BOB			1
#define NME_GALLOP		2
#define NME_ROLL_Z		3
#define NME_BOUNCY		4
#define NME_ROLL		5
#define NME_ROLL2		6

void UpdateBobbingEnemy(ENEMY *cur);
void UpdateJigglingEnemy(ENEMY *cur);
void UpdateGallopingEnemy(ENEMY *cur);
void UpdateRollZEnemy(ENEMY *cur);
void UpdateBouncingEnemy(ENEMY *cur);
void UpdateRollEnemy(ENEMY *cur);
void UpdateRoll2Enemy(ENEMY *cur);

void (*hackFuncs[])(ENEMY*) =
	{ UpdateJigglingEnemy, UpdateBobbingEnemy, UpdateGallopingEnemy, UpdateRollZEnemy, UpdateBouncingEnemy, UpdateRollEnemy, UpdateRoll2Enemy };

typedef struct _NMEHACK {
	char *modelname; short flags;
} NMEHACK;

NMEHACK enemyAnimHack[] = {
#ifdef DREAMCAST_VERSION
#include "x:\teamspirit\pcversion\nmehack.txt"
#else
#include "x:/teamspirit/pcversion/nmehack.txt"
#endif
};

// Reactive Anims

#define ANI_FACE		(1 << 0)
#define ANI_CENTRE		(1 << 1)
#define ANI_FIXED		(1 << 2)
#define ANI_ANTIFACE	(1 << 3)
#define ANI_FACECAMERA	(1 << 4)

#define ANI_REACTIVE	(1 << 31)

REACTIVEANIM reactiveAnims[] = {
#ifdef DREAMCAST_VERSION
#include "x:\teamspirit\pcversion\reactive.txt"
#else
#include "x:/teamspirit/pcversion/reactive.txt"
#endif
};

DEATHANIM_FUNC deathAnims[NUM_DEATHTYPES];

ENEMYLIST enemyList;						// the enemy linked list

#define ENEMY_RANDOMNESS (2048 + Random(4096))	// returns a value from 0.5 to 1.5

void NMEDamageFrog( int pl, ENEMY *nme );
void DoEnemyCollision( ENEMY *cur );
void RotateWaitingNME( ENEMY *cur );
void SlerpWaitingFlappyThing( ENEMY *cur );
void SetSoundEffectsForEnemy( ENEMY *nme );

void UpdatePathNME( ENEMY *cur );
void UpdateSlerpPathNME( ENEMY *cur );
void UpdateSnapper( ENEMY *cur );
void UpdateTileSnapper( ENEMY *cur );
void UpdateVent( ENEMY *cur );
void UpdateMoveVerticalNME( ENEMY *cur );
void UpdateRotatePathNME( ENEMY *cur );
void UpdateHomingNME( ENEMY *cur );
void UpdateMoveOnMoveNME( ENEMY *cur );
void UpdateFlappyThing( ENEMY *cur );
void UpdateFrogWatcher( ENEMY *cur );
void UpdateRandomMoveNME( ENEMY *cur );
void UpdateTileHomingNME( ENEMY *cur );
void UpdateBattleEnemy( ENEMY *cur );

/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
long GetReactiveAnimNumber(char *name)
{
	long i;
	char newname[32];

	strcpy(newname,name);
	gstrlwr( newname );
	stringChangePSIToOBE( newname );

	for (i=0; *reactiveAnims[i].name; i++)
	{
		gstrlwr(reactiveAnims[i].name);
		if (gstrcmp(newname,reactiveAnims[i].name)==0)
			return i;
	}
	return -1;
}


/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
ENEMY *GetEnemyFromUID (long uid)
{
	ENEMY *cur;

	for(cur = enemyList.head.next; cur != &enemyList.head; cur = cur->next)
		if (cur->uid == uid)
			return cur;

//	utilPrintf("GetEnemyFromUID: Couldn't find enemy %d\n", uid);
	return NULL;
}


void DoEnemyCollision( ENEMY *cur )
{
	ACTOR2 *act = cur->nmeActor;

	if( (cur->flags & ENEMY_NEW_VENT) /*|| (cur->flags & ENEMY_NEW_MOVEONMOVE)*/ || (cur->flags & ENEMY_NEW_PUSHESFROG) ) 
		return;

	if (gameState.multi==SINGLEPLAYER)
	{
		if( (cur->flags & ENEMY_NEW_RADIUSBASEDCOLLISION))
		{
			fixed dist = DistanceBetweenPointsSS(&frog[0]->actor->position, &act->actor->position);
			if(!player[0].dead.time && dist < (frog[0]->radius+act->radius) )
			{
				if( cur->flags & ENEMY_NEW_BABYFROG )
				{
					if (PickupBabyFrog( cur->nmeActor, cur->inTile ) )
						cur->active = 0;
				}
				else if (!player[0].safe.time) NMEDamageFrog(0,cur);
			}
		}
		else
		{
			// JIM: checking for lasttile causes totally unfair collision.
			if( ((currTile[0] == cur->inTile) /*|| (lastTile[0] == cur->inTile)*/) && !player[0].dead.time && 
				((!(player[0].isSuperHopping)&&!(player[0].frogState & FROGSTATUS_ISFLOATING)) || (cur->flags & ENEMY_NEW_NOJUMPOVER)) )
			{
				if( cur->flags & ENEMY_NEW_BABYFROG )
				{
					if (PickupBabyFrog( cur->nmeActor, cur->inTile ) )
						cur->active = 0;
				}
				else if (!player[0].safe.time)
					NMEDamageFrog(0,cur);
			}
		}
	}
	else
	{ 
		// We only want to do collision with player 1 in a REMOTE multiplayer game
		int i = (gameState.multi==MULTIREMOTE)?0:(NUM_FROGS-1);

		for (;i>=0;i--)
		{
			if( cur->flags & ENEMY_NEW_RADIUSBASEDCOLLISION )
			{
				fixed dist = DistanceBetweenPointsSS(&frog[i]->actor->position, &act->actor->position);
				if( !player[i].dead.time && dist < (frog[i]->radius+act->radius) )
				{
					if( cur->flags & ENEMY_NEW_BABYFROG )
						PickupBabyFrogMulti( cur, i );
					else if( !player[i].safe.time )
						KillMPFrog(i);
				}
			}
			else if( ((currTile[i] == cur->inTile)/* || (lastTile[i] == cur->inTile)*/) && !player[i].dead.time && 
					(!(player[i].frogState & FROGSTATUS_ISSUPERHOPPING) || (cur->flags & ENEMY_NEW_NOJUMPOVER)) &&
					!(player[i].frogState & FROGSTATUS_ISFLOATING))
			{
				if( cur->flags & ENEMY_NEW_BABYFROG )
					PickupBabyFrogMulti( cur, i );
				else if( !player[i].safe.time )
					KillMPFrog(i);
			}
		}
	}
}

void NMEDamageFrog( int pl, ENEMY *nme )
{
//#ifdef PSX_VERSION
	//oldModel = frog[0]->actor->psiData;
//#endif

	if( !nme || (player[pl].frogState & FROGSTATUS_ISSAFE) || (nme->flags & ENEMY_NEW_NODAMAGE) )
		return;

	if( gameState.mode == INGAME_MODE )
	{
		if( nme->flags & ENEMY_NEW_ONEHITKILL )
			player[pl].healthPoints = 0;
		else
			player[pl].healthPoints--;
	}

	player[pl].frogState |= FROGSTATUS_ISDEAD;

	// No death animations - probably swampy or roobie.
	if( frogPool[player[pl].character].anim == FANIM_NONE )
	{
		DeathMulti( pl ); // Explode
		return;
	}
	
	if( player[pl].lives == 1 )
	{
		deathAnims[DEATHBY_WHACKING] (pl);
		return;
	}

	if (nme->reactiveNumber!=-1)
	{
		if (reactiveAnims[nme->reactiveNumber].type & ANI_FACE)
			SetQuaternion(&(frog[pl]->actor->qRot),&(nme->nmeActor->actor->qRot));
		else if( reactiveAnims[nme->reactiveNumber].type & ANI_ANTIFACE )
		{
			FVECTOR fwd;
			SubVectorFSS( &fwd, &nme->nmeActor->actor->position, &frog[pl]->actor->position );
			MakeUnit( &fwd );
			OrientateFS( &frog[pl]->actor->qRot, &fwd, &currTile[pl]->normal );
		}
		else if( reactiveAnims[nme->reactiveNumber].type & ANI_FACECAMERA )
		{
			FVECTOR fwd;

			SubVectorFFS( &fwd, &currCamSource, &frog[pl]->actor->position );
			MakeUnit( &fwd );

			SetVectorFF( &fwd, FindClosestTileVector(currTile[pl], &fwd) );
			OrientateFS( &frog[pl]->actor->qRot, &fwd, &currTile[pl]->normal );
		}
		
		if (reactiveAnims[nme->reactiveNumber].type & ANI_CENTRE)
			SetVectorSS(&(frog[pl]->actor->position),&(nme->nmeActor->actor->position));

		if (reactiveAnims[nme->reactiveNumber].type & ANI_FIXED)
			nme->flags |= ENEMY_NEW_DONOTMOVE;

		// Matts reactive animations
		if( reactiveAnims[nme->reactiveNumber].type & ANI_REACTIVE )
		{
			actorAnimate(frog[pl]->actor,reactiveAnims[nme->reactiveNumber].animFrog, NO, NO, 64, 0);
			actorAnimate(nme->nmeActor->actor,reactiveAnims[nme->reactiveNumber].animChar, NO, NO, 64, 0);

			GTInit( &player[pl].dead, 3 );
		}
		else // My death animations
		{
//			utilPrintf("DEATH..............................\n");
			deathAnims[reactiveAnims[nme->reactiveNumber].animFrog] (pl);
			if( reactiveAnims[nme->reactiveNumber].animChar != -1 )
			{
				actorAnimate( nme->nmeActor->actor, reactiveAnims[nme->reactiveNumber].animChar, NO, NO, 64, 0 );
				actorAnimate( nme->nmeActor->actor, 0, YES, YES, 64, 0 );
			}
		}
	}
	else deathAnims[0] (pl); // DEATHBY_NORMAL

//#ifdef PSX_VERSION
	//frog[0]->actor->psiData = oldModel;
//#endif

}

/*	--------------------------------------------------------------------------------
	Function		: UpdateEnemies
	Purpose			: updates all enemies and their states / movements, etc.
	Parameters		: 
	Returns			: void
	Info			: 
*/
void UpdateEnemies()
{
	ENEMY *cur,*next;
	ENEMY_UPDATEFUNC *update;

	if(enemyList.count == 0)
		return;

	for(cur = enemyList.head.next; cur != &enemyList.head; cur = next)
	{
		ACTOR2 *act2 = cur->nmeActor;
		ACTOR *act;
		
		next = cur->next;

		// If enemy has an actor2 and it's active and not miles away
		if( cur->active && act2 && act2->distanceFromFrog < (ACTOR_DRAWDISTANCEOUTER<<2) )
			act = cur->nmeActor->actor;
		else
			continue;

		cur->visible = (act2->draw && !act2->clipped);

		// Set point of interest for frog - NOT CURRENTLY USED!
//		if( act2->distanceFromFrog < pOIDistance )
//		{
//			FVECTOR moveVec;
//			fixed angle;
//			SubVectorFSS( &moveVec, &act->position, &frog[0]->actor->position );
//			MakeUnit( &moveVec );

//			angle = rcos ( DotProductFF( &currTile[0]->dirVector[frogFacing[0]], &moveVec ) );

//			if( angle < 3686 )
//			{
//				pOIDistance = act2->distanceFromFrog;
//				pointOfInterest = &act->position;
//			}
//		}

		if( !act2->clipped && (frameCount > 10) )
			DoEnemyCollision( cur );

		// check if this enemy is currently 'waiting' at a node
		if(cur->isWaiting)
		{
			if(cur->isWaiting == -1)
				continue;

			if(actFrameCount > cur->path->startFrame)
				cur->isWaiting = 0;
			else
			{
				// Distance check so it won't bother turning nicely if you can't see it :)
				if( !(cur->flags & ENEMY_NEW_FACEFORWARDS) && cur->visible )
				{
					// if enemy is following a path, do a slerp so it'll rotate nicely
					// (except the way I do it is a bit poo - Dave)
					if( (cur->flags & ENEMY_NEW_FOLLOWPATH) || (cur->flags & ENEMY_NEW_RANDOMMOVE) )
						RotateWaitingNME( cur );
					else if( cur->flags & ENEMY_NEW_FLAPPYTHING )
						SlerpWaitingFlappyThing( cur );
				}
				// if enemy is a pusher and currently waiting - should still 'push' away frog
				if(cur->flags & ENEMY_NEW_PUSHESFROG)
				{
					// perhaps not the best way to do this, though...
					if(cur->inTile == currTile[0])
					{
						SetVectorSS(&frog[0]->actor->position,&prevTile->centre);
						destTile[0] = currTile[0] = prevTile;
					}
				}
				continue;
			}
		}

		// Do update functions for individual enemy types
		for (update = cur->Update; *update; update++)
			(*update)(cur);

		// Do Special Effects attached to enemies
		if( act2->effects && !act2->clipped && !(cur->flags & ENEMY_NEW_VENT) )
			ProcessAttachedEffects( (void *)cur, ENTITY_ENEMY );
	}

	UpdateBabies();
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateAllEnemies
	Purpose			: updates ALL enemies positions; used by network play
	Parameters		: 
	Returns			: void
	
	This function updates EVERY enemy so they start in the right place. This is needed
	for the network synchronisation stuff, where the game's timers can suddenly jump forward
	or backwards a number of seconds as they match the host - ds
*/
void UpdateAllEnemies()
{
	ENEMY *cur,*next;
	ENEMY_UPDATEFUNC *update;

	if(enemyList.count == 0)
		return;

	for(cur = enemyList.head.next; cur != &enemyList.head; cur = next)
	{
		ACTOR2 *act2 = cur->nmeActor;
		ACTOR *act;
		
		next = cur->next;

		if (cur->active && act2)
			act = cur->nmeActor->actor;
		else
			continue;

		// check if this enemy is currently 'waiting' at a node
		if(cur->isWaiting)
		{
			if(cur->isWaiting == -1)
				continue;

			if(actFrameCount > cur->path->startFrame)
				cur->isWaiting = 0;
			else
				continue;
		}

		// Do update functions for individual enemy types
		for (update = cur->Update; *update; update++)
			(*update)(cur);
	}

	UpdateBabies();
}

void RotateWaitingNME( ENEMY *cur )
{
 	IQUATERNION q, res;
 	fixed t;
 	SVECTOR fromPosition, toPosition;
 	FVECTOR fwd;
 	long start_t, end_t, time;
 	
 	end_t = cur->path->startFrame;
 	time = cur->isWaiting*(waitScale>>12);
 	start_t = end_t - time;
 
 	t = 4096 - ToFixed(actFrameCount-start_t)/time;
 
 	GetPositionForPathNode(&toPosition,&cur->path->nodes[cur->path->toNode]);
 	GetPositionForPathNode(&fromPosition,&cur->path->nodes[cur->path->fromNode]);
 	
 	SubVectorFSS(&fwd,&toPosition,&fromPosition);
 	MakeUnit( &fwd );
	Orientate(&q,&fwd,&cur->currNormal);
 
 	IQuatSlerp( &q, &cur->nmeActor->actor->qRot, t, &res );
 
 	SetQuaternion(&cur->nmeActor->actor->qRot, &res);
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateBobbingEnemy
	Purpose			: Bastard Bobbing enemies
	Parameters		: ENEMY
	Returns			: void
	Info			: shite
*/

void UpdateBobbingEnemy(ENEMY *nme)
{
	SVECTOR foo;
	SVECTOR normal;
	fixed bob;

	//nme->animVal = (nme->animVal + gameSpeed) & 0xFFFFFF;

	if( nme->path->endFrame <= nme->path->startFrame )
		return;

	bob = rsin(((actFrameCount-nme->path->startFrame)<<12)/(nme->path->endFrame-nme->path->startFrame))>>8;

	normal = nme->inTile->normal;
	
	foo.vx = (normal.vx * bob)>>10;
	foo.vy = (normal.vy * bob)>>10;
	foo.vz = (normal.vz * bob)>>10;

	AddToVectorSS(&nme->nmeActor->actor->position, &foo);
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateJigglingEnemy
	Purpose			: jiggle, wiggle, shake up and down
	Parameters		: ENEMY
	Returns			: void
	Info			: nya nya nya nya nya
*/

void UpdateJigglingEnemy(ENEMY *nme)
{
	SVECTOR foo;
	SVECTOR normal;
	fixed bob;

#ifdef PSX_VERSION
	bob = ((actFrameCount>>2) & 1)*15;
#else
	bob = ((actFrameCount>>2) & 1)*9;
#endif

	normal = nme->inTile->normal;
	
	foo.vx = (normal.vx * bob)>>12;
	foo.vy = (normal.vy * bob)>>12;
	foo.vz = (normal.vz * bob)>>12;

	AddToVectorSS(&nme->nmeActor->actor->position, &foo);
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateGallopingEnemy
	Purpose			: jiggle, wiggle, shake up and down
	Parameters		: ENEMY
	Returns			: void
	Info			: weeble weeble weeble
*/

void UpdateGallopingEnemy(ENEMY *nme)
{
	FVECTOR foo;
	SVECTOR normal, fwd;
	fixed clam, x, y;

	clam = ((actFrameCount-nme->path->startFrame)<<12)/(nme->path->endFrame-nme->path->startFrame)*2;

	SetVectorFS(&fwd, &nme->path->nodes[nme->path->toNode].worldTile->centre);
	SubFromVectorFS(&fwd, &nme->path->nodes[nme->path->fromNode].worldTile->centre);
	ScaleVectorFF(&fwd, (4096/500));	// 500 is roughly a tile and I don't care any more

	normal = nme->inTile->normal;

	x = rsin(clam)*25+25; y = rcos(clam)*20+40;

	foo.vx = (normal.vx*x - fwd.vx*y)>>12;
	foo.vy = (normal.vy*x - fwd.vy*y)>>12;
	foo.vz = (normal.vz*x - fwd.vz*y)>>12;

	AddToVectorSF(&nme->nmeActor->actor->position, &foo);

	nme->nmeActor->actor->size.vz = 4096+(rsin(clam-1500)>>3);
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateBouncingEnemy
	Purpose			: bounces along its path. Like a bouncy mushroom
	Parameters		: ENEMY
	Returns			: void
	Info			: rolls exactly one rotation per 12 tiles, for ancients3
*/

void UpdateBouncingEnemy(ENEMY *nme)
{
	FVECTOR v;
	fixed t, x, y;
	IQUATERNION q;
	PATH *p = nme->path;

	t = (actFrameCount-p->startFrame)<<12;
	t /= (p->endFrame-p->startFrame);

	// vertical bounce
	// y = 1-((2x-1)^2)
	x = (t<<1)-4096;
	y = (1<<(12+9))-((x*x)>>(12-9));

	SetVectorFF(&v, &nme->inTile->normal);
	ScaleVectorFF(&v, y);
	AddToVectorSF(&nme->nmeActor->actor->position, &v);

	nme->nmeActor->actor->size.vy = 4096 - ((x*x)>>14);

	t >>= 2;
	t += ((p->fromNode & 1)<<10);

	q.w = rcos(t);
	q.x = rsin(t);
	q.y = 0;
	q.z = 0;

	nme->nmeActor->actor->qRot = q;

/*	fixedQuaternionMultiply(
		&nme->nmeActor->actor->qRot,
		&nme->nmeActor->actor->qRot, &q);*/
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateRollZEnemy
	Purpose			: roll around the z axis like a rolling thing
	Parameters		: ENEMY
	Returns			: void
	Info			: rolls exactly one rotation per 12 tiles, for ancients3
*/

void UpdateRollZEnemy(ENEMY *nme)
{
	fixed t;
	PATH *p = nme->path;

	t = (actFrameCount-p->startFrame)<<11;
	t /= (p->endFrame-p->startFrame)*12;

	if (nme->flags & ENEMY_NEW_BACKWARDS)
		t -= ((p->fromNode-p->startNode)%12)*171;
	else
		t += ((p->fromNode-p->startNode)%12)*171;
		

	// rotate around z axis with a quaternion.. oogly

	nme->nmeActor->actor->qRot.w = rcos(t);
	nme->nmeActor->actor->qRot.x = 0;
	nme->nmeActor->actor->qRot.y = 0;

	if (nme->flags & ENEMY_NEW_BACKWARDS)
		nme->nmeActor->actor->qRot.z = rsin(t);
	else
		nme->nmeActor->actor->qRot.z = -rsin(t);
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateRollZEnemy
	Purpose			: rolls along like a rolling along thing
	Parameters		: ENEMY
	Returns			: void
	Info			: rolls exactly one rotation per 12 tiles, for ancients3
*/

void UpdateRollEnemy(ENEMY *nme)
{
	fixed t, c;
	IQUATERNION q;
	SVECTOR fwd;
	PATH *p = nme->path;

	nme->flags |= ENEMY_NEW_NOREORIENTATE|ENEMY_NEW_FACEFORWARDS;

//	Does this work? Does it? Hmm?

	//t = ((actFrameCount-p->startFrame)<<12)/(p->endFrame-p->startFrame);

/*	if (nme->flags & ENEMY_NEW_BACKWARDS)
		t -= ((p->fromNode-p->startNode)%12)*171;
	else
		t += ((p->fromNode-p->startNode)%12)*171;
		
	t >>= 2;
	t += ((p->fromNode & 1)<<10);*/

	SubVectorSSS(&fwd,
		&p->nodes[p->toNode].worldTile->centre,
		&p->nodes[p->fromNode].worldTile->centre);

	//t = (gameSpeed*nme->speed)/nme->nmeActor->radius;
	t = (gameSpeed<<13)/FMul(nme->nmeActor->radius, nme->speed);

	// rotate around z axis with a quaternion.. oogly

	c = rsin(t);

	q.w = rcos(t);
	q.x = (c*fwd.vz)/500;
	q.y = 0;
	q.z = (-c*fwd.vx)/500;

	fixedQuaternionMultiply(
		&nme->nmeActor->actor->qRot,
		&q, &nme->nmeActor->actor->qRot);

	NormaliseQuaternion(&nme->nmeActor->actor->qRot); // testing..

}

void UpdateRoll2Enemy(ENEMY *nme)
{
	fixed t;
	IQUATERNION q;
	SVECTOR fwd;
	PATH *p = nme->path;

	nme->flags |= ENEMY_NEW_NOREORIENTATE|ENEMY_NEW_FACEFORWARDS;

//	Does this work? Does it? Hmm?

	t = ((actFrameCount-p->startFrame)<<12)/(p->endFrame-p->startFrame);

	if (nme->flags & ENEMY_NEW_BACKWARDS)
		t -= ((p->fromNode-p->startNode)%12)*171;
	else
		t += ((p->fromNode-p->startNode)%12)*171;
		
	t >>= 2;
	t += ((p->fromNode & 1)<<10);

	// rotate around z axis with a quaternion, hoorah

	q.w = rcos(t);
	q.x = rsin(t);
	q.y = 0;
	q.z = 0;

	nme->nmeActor->actor->qRot = q;
}
/*	--------------------------------------------------------------------------------
	Function		: UpdatePathNME
	Purpose			: update enemies that move along a path
	Parameters		: ENEMY
	Returns			: void
	Info			: One day, some enemies will be done this way...
*/
void UpdatePathNME( ENEMY *cur )
{
	SVECTOR fromPosition,toPosition;
	FVECTOR fwd;
	PATH *path = cur->path;
	ACTOR2 *act = cur->nmeActor;
	fixed length;
	long progress;

	// make sure we're in the correct section of the path
	if( actFrameCount >= path->endFrame )
	{
		int i;

		// loop through updating the path until the end frame is later than the
		// current frame

		do
		{
			UpdateEnemyPathNodes(cur);
			path->startFrame = path->endFrame + ((cur->isWaiting * waitScale)>>12);

			if (cur->flags & ENEMY_NEW_RANDOMSPEED)
				path->endFrame = path->startFrame + (60*Random(4096));
			else
				path->endFrame = path->startFrame + ((60*cur->speed)>>12);

		} while (actFrameCount >= cur->path->endFrame); // || cur->isWaiting >= 0);

		if( !(cur->flags & ENEMY_NEW_NOREORIENTATE) )
			CalcEnemyNormalInterps(cur);

		if( (cur->flags & ENEMY_NEW_PUSHESFROG) && (cur->flags & ENEMY_NEW_PINGPONG) )
		{
			for( i=0; i<=path->toNode; i++ )
				if( path->nodes[i].worldTile->state == TILESTATE_NORMAL )
					path->nodes[i].worldTile->state = TILESTATE_BARRED;
			for( ; i<path->numNodes; i++ )
				if( path->nodes[i].worldTile->state == TILESTATE_BARRED )
					path->nodes[i].worldTile->state = TILESTATE_NORMAL;
		}
	}

	//if (cur->path->startFrame < cur->path->endFrame)

	if (!cur->isWaiting)
	{
		// update the enemy position
		GetPositionForPathNode(&toPosition,&path->nodes[path->toNode]);
		GetPositionForPathNode(&fromPosition,&path->nodes[path->fromNode]);
		
		SubVectorFSS(&fwd,&toPosition,&fromPosition);
		
		progress = max( (actFrameCount - path->startFrame), 1 );
		length = (progress<<12)/(path->endFrame - path->startFrame);
		
		ScaleVectorFF(&fwd,length);

		AddToVectorFF(&cur->currNormal,&cur->deltaNormal);

//bbopt - not needed !
//		MakeUnit( &cur->currNormal );

		if( !(cur->flags & ENEMY_NEW_DONOTMOVE) )
		{
			AddVectorSFS(&act->actor->position,&fwd,&fromPosition);

			if( cur->visible && !(cur->flags & ENEMY_NEW_NOREORIENTATE) )
			{
				if( !(cur->flags & ENEMY_NEW_FACEFORWARDS) )
				{
					FVECTOR fwdCopy = fwd;
					MakeUnit( &fwdCopy );
					Orientate(&act->actor->qRot, &fwdCopy, &cur->currNormal);
				}
				else // Need to do this so normals still work
				{
					OrientateSF(&act->actor->qRot, &path->nodes->worldTile->dirVector[cur->facing],  &cur->currNormal);
				}
			}
		}
	}

	/*	------------- Push blocks, big fat mofo blocks that push --------------- */
	if (cur->flags & ENEMY_NEW_PUSHESFROG)
	{
		FVECTOR v;
		fixed dot, dist;

		MakeUnit( &fwd );

		SubVectorFSS( &v, &frog[0]->actor->position, &cur->nmeActor->actor->position );
		dist = MagnitudeF( &v );

		if( cur->flags & ENEMY_NEW_PINGPONG )
		{
			MakeUnit( &v );
			dot = DotProductFF( &v, &fwd );
		}
		else
			dot = 4096;

		if( dot > 3900 && dist < act->radius+frog[0]->radius )
		{
			PushFrog(&fwd, 0);
			actorAnimate(frog[0]->actor, FROG_ANIM_SOMERSAULT, NO, NO, 2458, 0);
		}
	}

	if( actFrameCount < cur->path->startFrame+((cur->path->endFrame-cur->path->startFrame)/2) )
		cur->inTile = cur->path->nodes[cur->path->fromNode].worldTile;
	else
		cur->inTile = cur->path->nodes[cur->path->toNode].worldTile;
}


/*	--------------------------------------------------------------------------------
	Function		: UpdatePathSlerpNME
	Purpose			: Enemies that move in a smooth curve along a path
	Parameters		: ENEMY
	Returns			: void
	Info			: Use offset2 for slerp speed.
*/
void UpdateSlerpPathNME( ENEMY *cur )
{
 	IQUATERNION q1, q2, q3;
 	SVECTOR fromPosition,toPosition;
 	FVECTOR fwd;
 	PATH *path = cur->path;
 	ACTOR *act = cur->nmeActor->actor;
 	fixed speed, t;
 	
 	// Find the position of the current 2 nodes
 	GetPositionForPathNode(&toPosition,&path->nodes[path->toNode]);
 	
 	// Set direction to desired point
 	SubVectorFSS(&fwd,&toPosition,&act->position);
 	MakeUnit(&fwd);
 
 	SetQuaternion( &q1, &act->qRot );
 
 	// Skewer a line to rotate around, and make a rotation
	CrossProductFFF((FVECTOR *)&q3,&inVec,&fwd);
 	t = DotProductFF(&inVec,&fwd);
 	if(t<-4092)
 		t=-4092;
 	if(t>4092)
 		t = 4092;

	q3.w=arccos(t);
 	fixedGetQuaternionFromRotation(&q2,&q3);
 
 	// Slerp between current and desired orientation
 	speed = FMul(path->nodes[path->fromNode].offset2/SCALE, gameSpeed);
 
 	if(speed > 4092)
 		speed = 4092;
 	IQuatSlerp( &q1, &q2, speed, &act->qRot );
 
 	// Move forwards a bit in direction of facing
 	if( cur->flags & ENEMY_NEW_RANDOMSPEED )
 	{
 		ScaleVectorFF( &fwd, FMul(FMul(cur->speed,gameSpeed)*SCALE, FDiv(ToFixed(Random(100)),ToFixed(100))) );
 	}
 	else
 	{
 		ScaleVectorFF( &fwd, FMul(cur->speed,gameSpeed)*SCALE );
 	}
 
 	AddToVectorSF(&act->position, &fwd);
 
 	if( DistanceBetweenPointsSS(&act->position, &toPosition) < 409600 )
 	{
 		UpdateEnemyPathNodes(cur);
 
 		cur->path->startFrame = actFrameCount + cur->isWaiting * (waitScale>>12);
 	}
 
 	// Set inTile
 	GetPositionForPathNode(&fromPosition,&path->nodes[path->fromNode]);
 	GetPositionForPathNode(&toPosition,&path->nodes[path->toNode]);
 
 	t = DistanceBetweenPointsSS(&fromPosition, &toPosition) / 2;
 
 	if(DistanceBetweenPointsSS(&fromPosition, &act->position) < t)
 		cur->inTile = path->nodes[path->fromNode].worldTile;
 	else
 		cur->inTile = path->nodes[path->toNode].worldTile;
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateSnapper
	Purpose			: Snap at the frog
	Parameters		: ENEMY
	Returns			: void
	Info			: Path speed -> Slerp speed; waitTime -> Snap delay; 
*/
void UpdateSnapper( ENEMY *cur )
{
 	IQUATERNION q1, q2;
 	ACTOR *act = cur->nmeActor->actor;
 	PATH *path = cur->path;
	FVECTOR fwd;
 	fixed speed;
 
 	// Idle animations
 	switch( cur->isSnapping )
 	{
 	case 0:
 		if( actFrameCount < path->endFrame )
 			break;
 
 		if (Random(1000)>980)
 			actorAnimate(act,2,NO,NO,cur->nmeActor->animSpeed, 0);
 	
 		if (Random(1000)>950)
 		{
 			if (Random(1000)>950)
 				actorAnimate(act,3,NO,YES,cur->nmeActor->animSpeed, 0);
 			else
 				actorAnimate(act,0,NO,YES,cur->nmeActor->animSpeed, 0);
 		}
 
 		// Slerp orientation towards frog
 		SetVectorFF(&cur->currNormal, &path->nodes[0].worldTile->normal);
 		SetQuaternion( &q1, &act->qRot );
		SubVectorFSS( &fwd, &frog[0]->actor->position, &act->position );
		MakeUnit( &fwd );
		Orientate( &act->qRot, &fwd, &cur->currNormal );
 		SetQuaternion( &q2, &act->qRot );
 		speed = FMul(path->nodes[0].speed, gameSpeed);
 
 		if(speed > 4092)
 			speed = 4092;
 		IQuatSlerp( &q1, &q2, speed, &act->qRot );
 
 		// If the snapper has just spotted the frog, set snap time
 		if( cur->nmeActor->distanceFromFrog < ToFixed(750) )
 		{
 			SetVectorSS(&act->rotaim, &currTile[0]->centre);
 
 			SetQuaternion( &q1, &act->qRot );
			Orientate( &act->qRot, &fwd, &cur->currNormal );
 			SetQuaternion( &q2, &act->qRot );
 			IQuatSlerp( &q1, &q2, speed, &act->qRot );
 
 			path->startFrame = actFrameCount;
 			path->endFrame = path->startFrame + (path->nodes[0].waitTime * (waitScale>>12));
 
 			cur->isSnapping = 2;
 		}
 		break;
 
 	case 1: // Time to snap and hope
 		if( actFrameCount < path->endFrame )
 			break;
 
 		path->startFrame = actFrameCount;
 		path->endFrame = path->startFrame + (path->nodes[0].waitTime * (waitScale>>12));
 		cur->isSnapping = 0;
 
 		if( (DistanceBetweenPointsSS(&act->rotaim,&frog[0]->actor->position)<ToFixed(30*SCALE)) && 
 			!player[0].dead.time && !player[0].safe.time ) // If frog is within hitting distance
 		{
 			NMEDamageFrog(0,cur);
 		}
 		break;
 
 	case 2: 				// Snap animation
 		if( (actFrameCount-path->startFrame) < FMul(3277,path->endFrame-path->startFrame) )
 			break;
 
 		actorAnimate(act,1,NO,NO,cur->nmeActor->animSpeed, 0);
 		cur->isSnapping = 1;
 		break;
 	}
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateTileSnapper
	Purpose			: Snap at a sequence of tiles
	Parameters		: ENEMY
	Returns			: void
	Info			: waitTime -> snap delay; path speed -> slerp speed
*/
void UpdateTileSnapper( ENEMY *cur )
{
	IQUATERNION q1, q2;
	SVECTOR toPosition;
	SPECFX *fx;
	ACTOR *act = cur->nmeActor->actor;
	PATH *path = cur->path;
	fixed speed;
	int i;

	switch( cur->isSnapping )
	{
	case -2:
		path->startFrame = actFrameCount;
		path->endFrame = actFrameCount + ((6*cur->nmeActor->radius)>>12); // 60*radius, without downscaling

		cur->isSnapping = -1;
		break;

	case -1:
		if( actFrameCount >= path->endFrame )
			cur->isSnapping = 0;

		break;

	case 0:
		if( actFrameCount < path->endFrame )
			break;

		if( path->toNode >= GET_PATHLASTNODE(path) )	// reached end of path nodes
		{
			path->fromNode	= GET_PATHLASTNODE(path);
			path->toNode	= 1; // Don't snap at the tile we're sitting on
		}
		else
		{
			path->fromNode = path->toNode;
			path->toNode++;
		}
		path->startFrame = actFrameCount;
		path->endFrame = path->startFrame + ((path->nodes[path->fromNode].waitTime * waitScale)>>12);
		cur->isSnapping = 2;
		break;

	case 1:
		if( actFrameCount < path->endFrame )
			break;

		path->startFrame = actFrameCount;
		path->endFrame = path->startFrame + ((path->nodes[path->fromNode].waitTime*waitScale)>>12);
		cur->isSnapping = 0;

		// If the frog is on our current target tile
		for( i=0; i < NUM_FROGS; i++ )
			if((path->nodes[path->fromNode].worldTile == currTile[i]) && (!player[i].dead.time) && (!player[i].safe.time) && !player[i].isSuperHopping )
				NMEDamageFrog(i,cur);
		break;

	case 2:

		if( (actFrameCount-path->startFrame) < ((path->endFrame-path->startFrame)*10)>>4 )
		{
			FVECTOR dir;
			SetVectorFF( &cur->currNormal, &path->nodes->worldTile->normal );
			GetPositionForPathNode(&toPosition,&path->nodes[path->fromNode]);
			SubVectorFSS( &dir, &toPosition, &act->position );
			MakeUnit( &dir );
			SetQuaternion( &q1, &act->qRot );
			Orientate( &act->qRot, &dir, &cur->currNormal ); //ActorLookAt( act, &toPosition, LOOKAT_2D );
			SetQuaternion( &q2, &act->qRot );
			speed = FMul(path->nodes[0].speed, gameSpeed);

			if( speed > 4092 ) speed = 4092;
			IQuatSlerp( &q1, &q2, speed, &act->qRot );
			break;
		}

		if( cur->nmeActor->effects & EF_LIGHTNING )
		{
			FVECTOR dir, v;
			SVECTOR source;

			GetPositionForPathNode(&toPosition,&path->nodes[path->fromNode]);
			SubVectorFSS( &dir, &toPosition, &act->position );
			if( MagnitudeF(&dir)>>12 )
			{
				MakeUnit( &dir );
				SetVectorFF( &v, &cur->currNormal );
				ScaleVectorFF( &v, path->nodes[0].offset2 );
				AddVectorSFS( &source, &v, &act->position );
				
				if( cur->nmeActor->effects & EF_FAST )
					fx = CreateSpecialEffect( FXTYPE_LIGHTNING, &source, &dir, 20480, 163840, 1024, 819 );
				if( cur->nmeActor->effects & EF_SLOW )
					fx = CreateSpecialEffect( FXTYPE_LIGHTNING, &source, &dir, 20480, 163840, 1024, 3277 );
				else
					fx = CreateSpecialEffect( FXTYPE_LIGHTNING, &source, &dir, 20480, 163840, 1024, 2048 );

				if( fx )
				{
					fx->tilt = cur->nmeActor->value1; // Branching factor
					SetAttachedFXColour( fx, cur->nmeActor->effects );
				}
			}
		}

		// Play sound when snapping
		if( path->nodes[path->fromNode].sample )
			PlaySample( path->nodes[path->fromNode].sample, &act->position, 0, SAMPLE_VOLUME, -1 );

		actorAnimate( act, NMEANIM_SNAP_ATTACK, NO, NO, cur->nmeActor->animSpeed, 0);
		actorAnimate( act, NMEANIM_SNAP_IDLE, YES, YES, cur->nmeActor->animSpeed, 0);

		cur->isSnapping = 1;
		break;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateVent
	Purpose			: Blow hurt at the frog
	Parameters		: ENEMY
	Returns			: void
	Info			: radius -> delay before first fire; waitTime -> time between bursts; speed -> burst time;
					animSpeed -> particle speed; value1 -> particle lifetime
*/
void UpdateVent( ENEMY *cur )
{
	PATH *path = cur->path;
	SPECFX *fx;
	ACTOR2 *act = cur->nmeActor;
	FVECTOR pos, up;
	int i, j, progress;
	unsigned long t;

	switch( cur->isSnapping )
	{
	case -2:
		path->startFrame = actFrameCount;
		path->endFrame = actFrameCount + ((act->radius*6)>>12); // Actually radius*60, but allowing for world scaling...

		cur->isSnapping = -1;
		break;

	case -1:
		if( actFrameCount >= path->endFrame )
			cur->isSnapping = 0;

		break;

	case 0: // Start timer
		// Delay until fire
		path->startFrame = actFrameCount;
		path->endFrame = actFrameCount + (path->nodes[0].waitTime*60);

		cur->isSnapping = 1;
		break;

	case 1:
		if( actFrameCount < path->endFrame )
		{
			// If halway through waiting cycle, trigger some warning effect
			if( actFrameCount > path->startFrame + ((path->endFrame-path->startFrame)>>1) )
			{
				SetVectorFF( &up, &path->nodes->worldTile->normal );

				if( cur->nmeActor->effects & EF_FIERYSMOKE ) // Pilot light type thing
				{
					// pre-burn is less important than the smoke itself. - ds
					//PrepForPriorityEffect( );
					if (frameCount&1)
					{
						fx = CreateSpecialEffect( FXTYPE_SMOKE_GROWS, &act->actor->position, &up, 90000, 2048, 1024, 4096 );
						SetFXColour( fx, 255, 255, 255 );
					}
				}
				else if( cur->nmeActor->effects & EF_SMOKEBURST )
				{
					//PrepForPriorityEffect( );
					if (frameCount&1)
					{
						fx = CreateSpecialEffect( FXTYPE_SMOKE_GROWS, &act->actor->position, &up, 90000, 2048, 1024, 4096 );
						SetAttachedFXColour( fx, act->effects );
					}
				}
				else if( (cur->nmeActor->effects & EF_LIGHTNING) && !cur->isIdle ) // TODO: make this better!
				{
					FVECTOR normal;
					fixed time = (((path->endFrame-actFrameCount)<<12)/60) + path->nodes->speed;

					SubVectorFSS( &normal, &path->nodes[path->numNodes-1].worldTile->centre, &path->nodes->worldTile->centre );
					MakeUnit( &normal );
					if( (fx = CreateSpecialEffect( FXTYPE_TWINKLE, &act->actor->position, &normal, 163840, 0, 0, time )) )
					{
						fx->tilt = 8192;
						SetAttachedFXColour( fx, act->effects );
					}

					ScaleVector( &normal, -1 );
					if( (fx = CreateSpecialEffect( FXTYPE_TWINKLE, &path->nodes[path->numNodes-1].worldTile->centre, &normal, 163840, 0, 0, time )) )
					{
						fx->tilt = 8192;
						SetAttachedFXColour( fx, act->effects );
					}

					cur->isIdle = 1;
				}
			}

			break;
		}

		// Delay until stop
		path->startFrame = actFrameCount;
		path->endFrame = actFrameCount + ((path->nodes->speed*60)>>12);
		act->fxCount = 0;

		// Play sound when firing
		if( path->nodes->sample )
			PlaySample( path->nodes->sample, &act->actor->position, 0, SAMPLE_VOLUME, -1 );

		cur->isIdle = 0;
		cur->isSnapping = 2;
		break;

	case 2:
		// Stop firing on timeout, and reset
		if( actFrameCount >= path->endFrame )
		{
			cur->isSnapping = 0;
			break;
		}

		if(act->value1)
			t = (path->endFrame-path->startFrame)/(act->value1>>12);
		else
			t = 1;

		if( (actFrameCount-path->startFrame) > (act->fxCount*t) )
		{
			fixed scale = (path->nodes[0].offset2)?(path->nodes[0].offset2/SCALE):4096;
			act->fxCount++;
			SetVectorFF( &up, &path->nodes->worldTile->normal );

			if( cur->nmeActor->effects & EF_FIERYSMOKE )
			{
				PrepForPriorityEffect( );
#ifdef PSX_VERSION
				fx = CreateSpecialEffect( FXTYPE_FIERYSMOKE, &act->actor->position, &up, 204800, (act->animSpeed<<3)*path->numNodes, scale, 12288 );
#else
				fx = CreateSpecialEffect( FXTYPE_FIERYSMOKE, &act->actor->position, &up, 204800, (act->animSpeed<<3)*path->numNodes, scale, 8192 );
#endif
			}
			else if( cur->nmeActor->effects & EF_LASER )
			{
				fx = CreateSpecialEffect( FXTYPE_LASER, &act->actor->position, &up, 61440, act->animSpeed<<3, 0, 2048*path->numNodes );
				SetAttachedFXColour( fx, act->effects );
			}
			else if( cur->nmeActor->effects & EF_SMOKEBURST )
			{
				PrepForPriorityEffect( );
				fx = CreateSpecialEffect( FXTYPE_SMOKEBURST, &act->actor->position, &up, 204800, (act->animSpeed<<3)*path->numNodes, scale, 6963 );
				SetAttachedFXColour( fx, act->effects );
			}
			else if( cur->nmeActor->effects & EF_LIGHTNING ) // Make a big lighning thing between first and last nodes
			{
				SVECTOR p1, p2;

				GetPositionForPathNode( &p1, &path->nodes[0] );
				GetPositionForPathNode( &p2, &path->nodes[path->numNodes-1] );

				CreateLightningEffect( &p1, &p2, act->effects,
									   ((60*path->nodes->speed)>>12)/((act->value1)?(act->value1>>12):1),
									   act->depthShift);
			}
		}

		// Lightning covers all the path at once - smoke progresses along the path
		if( cur->nmeActor->effects & EF_LIGHTNING )
			progress = path->numNodes;
		else
		{
			progress = 1+((FDiv( (actFrameCount-path->startFrame)<<12, (path->endFrame-path->startFrame)<<12 ) * path->numNodes)>>12);
			if( progress > path->numNodes )
				progress = path->numNodes;
		}

		// Check for collision with frog, and do damage
		for( j=0; j < NUM_FROGS; j++ )
			for( i=0; i < progress; i++ )
			{
				if( (player[j].frogState & FROGSTATUS_ISDEAD) || (player[j].frogState & FROGSTATUS_ISSAFE) )
					continue;

				// Check if frog is on path
				if( !(cur->flags & ENEMY_NEW_RADIUSBASEDCOLLISION) )
				{
					if( path->nodes[i].worldTile != currTile[j] )
						continue;
				}
				else // Else if radius based, do a cylindrical type collision on the vent beam
				{
					// Only do this if the frog is anywhere near
					if( !((path->nodes[i].worldTile == currTile[j]) || (path->nodes[i].worldTile == destTile[j])) )
						continue;

					// Get position in the beam to check for
					SetVectorFF( &pos, &path->nodes[i].worldTile->normal );
					ScaleVectorFF( &pos, path->nodes[i].offset );
					AddToVectorFS( &pos, &path->nodes[i].worldTile->centre );
					if( DistanceBetweenPointsSF(&frog[j]->actor->position, &pos) > 1638400 )
						continue;
				}

				if( gameState.multi == SINGLEPLAYER )
					NMEDamageFrog(j,cur);
				else
					KillMPFrog( j );
			}

		break;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateMoveVerticalNME
	Purpose			: Enemy moves up and down
	Parameters		: ENEMY
	Returns			: void
	Info			: 
*/
void UpdateMoveVerticalNME( ENEMY *cur )
{
	PATH *path = cur->path;
	ACTOR *act = cur->nmeActor->actor;
	FVECTOR moveVec;
	fixed start_offset, end_offset, t;

	// check if this platform has arrived at a path node
	while (actFrameCount > path->endFrame)
	{
		UpdateEnemyPathNodes(cur);
		path->startFrame = path->endFrame + cur->isWaiting * (waitScale>>12);
		path->endFrame = path->startFrame + ((cur->speed*60)>>12);
	}

	if (cur->isWaiting) return;

	if (cur->flags & ENEMY_NEW_MOVEUP)
	{
		start_offset = path->nodes->offset;
		end_offset = path->nodes->offset2;
	}
	else
	{
		start_offset = path->nodes->offset2;
		end_offset = path->nodes->offset;
	}

	// get up vector for this platform
	SetVectorFF(&moveVec, &path->nodes[0].worldTile->normal);
	
	t = FDiv( ToFixed(actFrameCount-path->startFrame) , ToFixed(path->endFrame-path->startFrame) );

	ScaleVectorFF( &moveVec, FMul(t,end_offset) + FMul(4096-t,start_offset) );
	AddVectorSSF(&act->position, &path->nodes->worldTile->centre, &moveVec);
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateRotatePathNME
	Purpose			: Enemy that flies around a point
	Parameters		: ENEMY
	Returns			: void
	Info			: 
*/
void UpdateRotatePathNME( ENEMY *cur )
{
	fixed length, cosx, sinx;
	int radius;
	SVECTOR toPosition;
	FVECTOR fwd;

	// note - our fixed-point angles go from 0-4096, where 4096 => 360degrees => 2pi radians
	// so where we used 2PI we can just use 4096

	length = ToFixed(actFrameCount-cur->path->startFrame) / (cur->path->endFrame-cur->path->startFrame);
	length += cur->path->nodes->offset2; // Fraction of circumference it starts at

	if( cur->speed < 0 )
		length = 4096 - length;

	length &= 4095;	// keep in range 0-1

	radius = cur->nmeActor->radius >> 12;
	cosx = rcos(length);
	sinx = rsin(length);

	ZeroVector(&fwd);

	if( cur->flags & ENEMY_NEW_ROTATEPATH_XZ )
	{
		toPosition.vx = cur->path->nodes->worldTile->centre.vx + (short)((radius*sinx)>>12);
		toPosition.vz = cur->path->nodes->worldTile->centre.vz + (short)((radius*cosx)>>12);
		toPosition.vy = cur->path->nodes->worldTile->centre.vy + (short)(cur->path->nodes->offset>>12);

		fwd.vx = cosx;
		fwd.vz = -sinx;
	}
	else if( cur->flags & ENEMY_NEW_ROTATEPATH_XY )
	{
		toPosition.vx = cur->path->nodes->worldTile->centre.vx + (short)((radius*sinx)>>12);
		toPosition.vz = cur->path->nodes->worldTile->centre.vz;
		toPosition.vy = cur->path->nodes->worldTile->centre.vy + (short)((radius*cosx + cur->path->nodes->offset)>>12);

		fwd.vx = cosx;
		fwd.vy = -sinx;
	}
	else // ZY
	{
		toPosition.vx = cur->path->nodes->worldTile->centre.vx;
		toPosition.vz = cur->path->nodes->worldTile->centre.vz + (short)((radius*sinx)>>12);
		toPosition.vy = cur->path->nodes->worldTile->centre.vy + (short)((radius*cosx+cur->path->nodes->offset)>>12);

		fwd.vz = cosx;
		fwd.vy = -sinx;
	}

	if (cur->speed < 0)	fwd.vx = -fwd.vx, fwd.vy = -fwd.vy, fwd.vz = -fwd.vz;

	if( cur->visible && !(cur->flags & ENEMY_NEW_FACEFORWARDS) ) // Look in direction of travel
	{
//		Orientate( &cur->nmeActor->actor->qRot, &fwd, &cur->path->nodes->worldTile->normal );

		//bbopt - may not need this MakeUnit here,
		//put in 'cos Orientate no longer does MakeUnit(fwd)
		//anyway, fwd is set up with son and cos, so may be a unit.
		FVECTOR fwdCopy = fwd;
		MakeUnit(&fwdCopy);
		OrientateFS( &cur->nmeActor->actor->qRot, &fwdCopy, &cur->path->nodes->worldTile->normal );
	}

	SetVectorSS(&cur->nmeActor->actor->position, &toPosition);

	if( actFrameCount > cur->path->endFrame )
	{
		cur->path->startFrame = cur->path->endFrame;
		cur->path->endFrame = cur->path->startFrame+(Fabs(cur->speed*60)>>12);
	}

	cur->inTile = FindNearestTileS( cur->nmeActor->actor->position );
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateHomingNME
	Purpose			: Enemy moves towards frog
	Parameters		: ENEMY
	Returns			: void
	Info			: 
*/
void UpdateHomingNME( ENEMY *cur )
{
	FVECTOR fwd;
	IQUATERNION q1, q2;

	if( player[0].frogState & FROGSTATUS_ISDEAD )
		return;

	// For now, screw tiles completely and just move. Make radius based if they need to collide
	SubVectorFSS( &fwd, &frog[0]->actor->position, &cur->nmeActor->actor->position );
	MakeUnit( &fwd );

	if( !(cur->flags & ENEMY_NEW_DONOTMOVE) && cur->visible && !(cur->flags & ENEMY_NEW_FACEFORWARDS) )
	{
		fixed speed = FMul(cur->path->nodes->offset2/SCALE, gameSpeed);
 		if(speed > 4092)
			speed = 4092;

		Orientate( &q1, &fwd, &cur->currNormal );
		SetQuaternion( &q2, &cur->nmeActor->actor->qRot );
		IQuatSlerp( &q2, &q1, speed, &cur->nmeActor->actor->qRot );
	}

	ScaleVectorFF( &fwd, FMul(cur->speed, gameSpeed)*SCALE );
	AddToVectorSF( &cur->nmeActor->actor->position, &fwd );
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateTileHomingNME
	Purpose			: Home in on the frog using tiles
	Parameters		: ENEMY
	Returns			: void
	Info			: waitTime -> radius of activation; 
*/
void UpdateTileHomingNME( ENEMY *cur )
{
	FVECTOR up;
	FVECTOR fwd;
	PATH *path = cur->path;
	ACTOR2 *act = cur->nmeActor;
	fixed length;

	if( path->numNodes < 3 )
		return;

	if( DistanceBetweenPointsSS(&act->actor->position, &frog[0]->actor->position) > act->radius )
	{
		cur->isIdle = 0;
		path->nodes[2].worldTile = NULL;
		return;
	}

	if( !cur->isSnapping )
	{
		path->nodes[1].worldTile = NULL;
		path->nodes[2].worldTile = path->nodes[0].worldTile;
		cur->isSnapping = 1; // This is a completely random misuse of isSnapping. And I don't care.
	}

	// If enemy is on the next path node, set startnode worldtile and the next to zero
	if( path->nodes[2].worldTile )
	{
		if( actFrameCount >= path->endFrame )
		{
			path->nodes[1].worldTile = path->nodes[2].worldTile;
			path->nodes[2].worldTile = NULL;
		}
		else if( actFrameCount > path->startFrame+((path->endFrame-path->startFrame)/2) )
		{
			cur->inTile = path->nodes[2].worldTile;
		}
	}

	// If we need a new destination tile, find it by the direction to the frog
	if( !path->nodes[2].worldTile )
	{
		FVECTOR frogVec;
		SubVectorFSS( &frogVec, &currTile[0]->centre, &act->actor->position ); 
		path->nodes[2].worldTile = FindJoinedTileByDirection( path->nodes[1].worldTile, &frogVec );

		path->startFrame = actFrameCount;
		path->endFrame = path->startFrame + ((path->nodes[0].speed*60)>>12);
	}

	// Move towards next node - third condition is so fwd is not scaled to zero
	if( path->nodes[1].worldTile && path->nodes[2].worldTile && (actFrameCount > path->startFrame) )
	{
		// Move towards frog
		SubVectorFSS(&fwd,&path->nodes[2].worldTile->centre,&path->nodes[1].worldTile->centre);
		length = FDiv( ToFixed(actFrameCount-path->startFrame), ToFixed(path->endFrame-path->startFrame) );
		ScaleVectorFF(&fwd,length);
		AddVectorSFS(&act->actor->position,&fwd,&path->nodes[1].worldTile->centre);

		// Orientate to direction of travel
		MakeUnit( &fwd );
		if (!(cur->flags & ENEMY_NEW_FACEFORWARDS) && cur->visible)
			OrientateFS(&act->actor->qRot,&fwd,&path->nodes[1].worldTile->normal);

		// Elevate above gametile
		SetVectorFF(&up, &cur->inTile->normal);
		ScaleVectorFF( &up, path->nodes[0].offset );
		AddToVectorSF( &act->actor->position, &up );
	}
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateMoveOnMoveNME
	Purpose			: Move when frog has moved
	Parameters		: ENEMY
	Returns			: void
	Info			: radius -> radius of activation; 
*/
void UpdateMoveOnMoveNME( ENEMY *cur )
{
	FVECTOR up;
	FVECTOR fwd;
	PATH *path = cur->path;
	ACTOR2 *act = cur->nmeActor;
	fixed length, dist;

	if( path->numNodes < 3 )
		return;

	dist = DistanceBetweenPointsSS(&act->actor->position, &frog[0]->actor->position);
	if(dist > act->radius)
	{
		cur->isIdle = 0;
		path->nodes[2].worldTile = NULL;
		return;
	}
	else if( dist < 1638400 && !player[0].dead.time && !player[0].safe.time ) // Trial and error value of 33
	{
		NMEDamageFrog(0,cur);
	}

	if( !cur->isSnapping )
	{
		path->nodes[1].worldTile = NULL;
		path->nodes[2].worldTile = path->nodes[0].worldTile;
		cur->isSnapping = 1; // This is a completely random misuse of isSnapping. And I don't care.
//		cur->flags |= ENEMY_NEW_NODAMAGE;
	}

	// If frog has moved
	if( player[0].frogState & FROGSTATUS_ISDEAD )
		cur->isIdle = 0;
	else
		cur->isIdle += player[0].hasJumped;

	// If enemy is on the next path node, set startnode worldtile and the next to zero
	if( path->nodes[2].worldTile )
	{
		if( actFrameCount > path->endFrame )
		{
//			cur->inTile->state = TILESTATE_NORMAL;
			cur->inTile = path->nodes[2].worldTile;
			path->nodes[1].worldTile = path->nodes[2].worldTile;
			path->nodes[2].worldTile = NULL;
		}
	}

	// If we need a new destination tile, find it by the direction to the frog
	if( cur->isIdle && !path->nodes[2].worldTile )
	{
		FVECTOR frogVec;
		SubVectorFSS( &frogVec, &frog[0]->actor->position, &act->actor->position ); 
		MakeUnit(&frogVec);
		path->nodes[2].worldTile = FindJoinedTileByDirectionAndType( path->nodes[1].worldTile, &frogVec, TILESTATE_NORMAL );

//		if( path->nodes[2].worldTile )
//		{
//			if( path->nodes[2].worldTile->state == TILESTATE_OCCUPIED )
//				path->nodes[2].worldTile = NULL;
//			else
//				path->nodes[2].worldTile->state = TILESTATE_OCCUPIED;
//		}
		path->startFrame = actFrameCount;
		path->endFrame = path->startFrame + ((60*path->nodes[0].speed)>>12);
		cur->isIdle--;
		actorAnimate( act->actor, NMEANIM_HOMER_WALK, NO, NO, act->animSpeed, 0 );
		actorAnimate( act->actor, NMEANIM_HOMER_IDLE, YES, YES, act->animSpeed, 0 );
	}

	// Move towards next node
	if( path->nodes[1].worldTile && path->nodes[2].worldTile )
	{
		// Move towards frog
		SubVectorFSS(&fwd,&path->nodes[2].worldTile->centre,&path->nodes[1].worldTile->centre);
		length = FDiv( ToFixed(actFrameCount-path->startFrame), ToFixed(path->endFrame-path->startFrame) );
		ScaleVectorFF(&fwd,max(length,1));
		AddVectorSFS(&act->actor->position, &fwd, &path->nodes[1].worldTile->centre);

		// Orientate to direction of travel
		MakeUnit( &fwd );
		if (!(cur->flags & ENEMY_NEW_FACEFORWARDS) && cur->visible)
			OrientateFS(&act->actor->qRot,&fwd,&path->nodes[1].worldTile->normal);

		// Elevate above gametile
		SetVectorFF(&up, &cur->inTile->normal);
		ScaleVectorFF( &up, path->nodes[0].offset );
		AddToVectorSF( &act->actor->position, &up );
	}
}


void UpdateRandomMoveNME( ENEMY *cur )
{
	FVECTOR up;
	FVECTOR fwd;
	PATH *path = cur->path;
	ACTOR2 *act = cur->nmeActor;
	fixed length;

	if( path->numNodes < 3 )
		return;

	if( !cur->isSnapping )
	{
		path->nodes[2].worldTile = path->nodes[1].worldTile;
		path->nodes[1].worldTile = path->nodes[0].worldTile;
		cur->isSnapping = 1; // This is a completely random misuse of isSnapping. And I don't care.
		cur->path->fromNode = 1;
		cur->path->toNode = 2;
	}

	// If enemy is on the next path node, set startnode worldtile and the next to zero
	if( path->nodes[2].worldTile )
	{
		if( actFrameCount > path->endFrame )
		{
			FVECTOR rVec = {0,0,0};
			int r;

			SubVectorFSS( &fwd, &path->nodes[2].worldTile->centre, &path->nodes[1].worldTile->centre );
			path->nodes[1].worldTile = path->nodes[2].worldTile;

			r = Random(4);
			if(r==0) rVec.vx = 4096;
			else if(r==1) rVec.vx = -4096;
			else if(r==2) rVec.vz = 4096;
			else rVec.vz = -4096;
			path->nodes[2].worldTile = FindJoinedTileByDirection( path->nodes[1].worldTile, &rVec );
			if( !path->nodes[2].worldTile )
				path->nodes[2].worldTile = path->nodes[1].worldTile;

			if( !(Random((act->value1>>12)+1)) )
				cur->isWaiting = Random(path->nodes[0].waitTime);

			if( cur->flags & ENEMY_NEW_BABYFROG )
			{
				SubVectorFSS( &rVec, &path->nodes[2].worldTile->centre, &path->nodes[1].worldTile->centre );
	//			if( DotProduct(&fwd, &rVec) < 4092 ) // Only do cartwheel if baby needs to turn
	//				actorAnimate( act->actor, BABY_ANIM_CARTWHEEL, NO, NO, act->animSpeed, 0);

//				actorAnimate( act->actor, BABY_ANIM_WAVE, NO, YES, act->animSpeed, 0);
			}

			path->startFrame = actFrameCount + ((Random(cur->isWaiting/2)+cur->isWaiting/2) * (waitScale>>12));
			path->endFrame = path->startFrame + ((path->nodes[0].speed*60)>>12);
			return;
		}
		else if( actFrameCount > path->startFrame+((path->endFrame-path->startFrame)/2) )
		{
			cur->inTile = path->nodes[2].worldTile;
		}
	}

	// Move towards next node - third condition is so fwd is not scaled to zero
	if( path->nodes[1].worldTile && path->nodes[2].worldTile && (actFrameCount > path->startFrame) )
	{
		if( cur->flags & ENEMY_NEW_BABYFROG )
		{	// Start baby hop the first time we move
//			if( DistanceBetweenPointsSS(&path->nodes[1].worldTile->centre, &act->actor->position) < ToFixed(2*SCALE) )//bb??
//				actorAnimate( act->actor, BABY_ANIM_HOP, NO, NO, act->animSpeed, 0);
		}
		SubVectorFSS(&fwd,&path->nodes[2].worldTile->centre,&path->nodes[1].worldTile->centre);
		length = FDiv( ToFixed(actFrameCount-path->startFrame), ToFixed(path->endFrame-path->startFrame) );
		ScaleVectorFF(&fwd,length);
		AddVectorSFS(&act->actor->position,&fwd,&path->nodes[1].worldTile->centre);

		// Orientate to direction of travel
		MakeUnit( &fwd );
		if (!(cur->flags & ENEMY_NEW_FACEFORWARDS) && cur->visible)
			OrientateFS(&act->actor->qRot,&fwd,&path->nodes[1].worldTile->normal);

		// Elevate above gametile
		SetVectorFF(&up, &cur->inTile->normal);
		ScaleVectorFF( &up, path->nodes[0].offset );
		AddToVectorSF( &act->actor->position, &up );
	}
}


void UpdateFrogWatcher( ENEMY *cur )
{
 	IQUATERNION q1, q2;
	FVECTOR to;
 	ACTOR *act = cur->nmeActor->actor;
 	fixed speed;

	if( !cur->visible ) return;

	SubVectorFSS( &to, &frog[0]->actor->position, &act->position );
	MakeUnit( &to );

 	SetQuaternion( &q1, &act->qRot );
	Orientate( &act->qRot, &to, &cur->currNormal );
// 	ActorLookAt( act, &frog[0]->actor->position, LOOKAT_ANYWHERE );
	SetQuaternion( &q2, &act->qRot );
 	speed = FMul(cur->path->nodes[0].speed, gameSpeed);
 
 	if( speed > 4092 ) speed = 4092;
	IQuatSlerp( &q1, &q2, speed, &act->qRot );
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateFlappyThing
	Purpose			: Make the enemy fly around in a box
	Parameters		: ENEMY
	Returns			: void
	Info			: waitTime -> how long to wait on a point of Interest; node[0].speed -> slerp speed;
					node[1].speed -> move speed
*/
void UpdateFlappyThing( ENEMY *nme )
{
 	PATH *path = nme->path;
 	ACTOR *act = nme->nmeActor->actor;
 	FVECTOR fwd;
 	IQUATERNION q1, q2, q3;
 	fixed t, best = 2048000000, speed;

 	// Use default values if the user can't be arsed
 	if( path->nodes[0].offset == path->nodes[1].offset )
 	{
 		path->nodes[0].offset = 0;
 		path->nodes[1].offset = 409600;
 		path->nodes[0].speed = 287;
 		path->nodes[1].speed = 4096;
 	}

	// If nme is very close to destination (stored in nmeactor->actor->rotaim) then choose a new destination.
//	if( (DistanceBetweenPointsSquared(&act->pos,&act->rotaim) < 300) || !Magnitude(&act->rotaim) ) // 10 units apart
	if( (DistanceBetweenPointsSS(&act->position,&act->rotaim) < (17*SCALE<<12)) || !(MagnitudeS(&act->rotaim)>>12) ) // 10 units apart
  	{
  		FVECTOR up;
  
  		// If just got to the special node we're aiming for, wait at it
  		if( path->startNode )
  		{
  			path->startFrame = actFrameCount + (path->nodes[path->startNode].waitTime*60);
  			nme->isWaiting = 1;
  			path->startNode = 0;
  			return;
  		}
  
  		// On a random chance, aim at the closest special node (DOESN'T INCLUDE OFFSET!)
  		if( path->numNodes > 2 && Random(100)>80 )
			path->startNode = Random(path->numNodes-2)+2;
//  			for( i=2; i<path->numNodes; i++ )
//  			{
//  				t = DistanceBetweenPointsSS(&act->position, &path->nodes[i].worldTile->centre);
//  				if( t < best )
//  				{
//  					best = t;
//  					path->startNode = i;
//  				}
//  			}
  
  		// If we just found a node to aim at, go there
  		if( path->startNode )
  		{
  			SetVectorFF(&up, &path->nodes[path->startNode].worldTile->normal);
  			ScaleVectorFF( &up, path->nodes[path->startNode].offset );
  			AddVectorSFS( &act->rotaim, &up, &path->nodes[path->startNode].worldTile->centre );
  		}
  		else // Else pick a random point in space
  		{
  			SVECTOR p1, p2;
  			int dX, dY, dZ;
  
  			SetVectorFF(&up, &path->nodes[0].worldTile->normal);
  			ScaleVectorFF( &up, path->nodes[0].offset );
  			AddVectorSSF( &p1, &path->nodes[0].worldTile->centre, &up );
  
  			SetVectorFF(&up, &path->nodes[1].worldTile->normal);
  			ScaleVectorFF( &up, path->nodes[1].offset );
  			AddVectorSSF( &p2, &path->nodes[1].worldTile->centre, &up );
  
  			// Get dimensions of the box we can move in.
  			dX = Fabs(p2.vx - p1.vx);
  			dY = Fabs(p2.vy - p1.vy);
  			dZ = Fabs(p2.vz - p1.vz);
  
  			p1.vx = min(p2.vx, p1.vx);
  			p1.vy = min(p2.vy, p1.vy);
  			p1.vz = min(p2.vz, p1.vz);
  
  			// Random destination within the valid box
  			act->rotaim.vx = p1.vx+Random(dX);
  			act->rotaim.vy = p1.vy+Random(dY);
  			act->rotaim.vz = p1.vz+Random(dZ);
  		}
  	}
 
 	// Store current orientation
  	SetQuaternion( &q1, &act->qRot );
 	
 	// Set direction to desired point
  	SubVectorFSS(&fwd,&act->rotaim,&act->position);
 	MakeUnit(&fwd);
 
 	// Skewer a line to rotate around, and make a rotation
	CrossProductFFF( (FVECTOR *)&q3, &inVec, &fwd );
 	MakeUnit( (FVECTOR *)&q3 );
	t = DotProductFF(&inVec,&fwd);
	if (t<-4092)
		t=-4092;
	if (t>4092)
		t = 4092;
	if(t<4 && t>-4) // If its trying to move backwards, make it turn
		t = 410;
	q3.w=arccos(t);
	// Duh.
	fixedGetQuaternionFromRotation(&q2,&q3);

	// Slerp between current and desired orientation
	speed = FMul(path->nodes[0].speed, gameSpeed);
	if( speed > 4092 ) speed = 4092;
	IQuatSlerp( &q1, &q2, speed, &act->qRot );

	// Move forwards a bit in direction of facing
//	RotateVectorByQuaternion( &fwd, &inVec, &act->qRot );
	ScaleVectorFF( &fwd, FMul(path->nodes[1].speed,gameSpeed) *SCALE );
	AddVectorSFS( &act->position, &fwd, &act->position );

	nme->flags |= ENEMY_NEW_NODAMAGE;
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateBattleEnemy
	Purpose			: Enemy bounces around a battle mode arena
	Parameters		: 
	Returns			: 
	Info			: 
*/
void UpdateBattleEnemy( ENEMY *cur )
{
	FVECTOR up;
	FVECTOR fwd;
	PATH *path = cur->path;
	ACTOR2 *act = cur->nmeActor;

	if( path->numNodes < 3 )
		return;

	if( !cur->isSnapping )
	{
		path->nodes[2].worldTile = path->nodes[1].worldTile;
		path->nodes[1].worldTile = path->nodes[0].worldTile;
		cur->isSnapping = 1; // This is a completely random misuse of isSnapping. And I don't care.
		cur->path->fromNode = 1;
		cur->path->toNode = 2;
	}

	if( !cur->inTile )
		cur->inTile = path->nodes->worldTile;

	// If enemy is on the next path node, set startnode worldtile and the next to zero
	if( path->nodes[2].worldTile )
	{
		if( actFrameCount > path->endFrame )
		{
			FVECTOR rVec = {0,0,0};

			SubVectorFSS( &fwd, &path->nodes[2].worldTile->centre, &path->nodes[1].worldTile->centre );
			path->nodes[1].worldTile = path->nodes[2].worldTile;

			path->nodes[2].worldTile->state = TILESTATE_NORMAL;
			path->nodes[2].worldTile = FindJoinedTileByDirectionAndType( path->nodes[1].worldTile, &fwd, TILESTATE_NORMAL );

			if( !path->nodes[2].worldTile )
				path->nodes[2].worldTile = path->nodes[1].worldTile;

			path->nodes[2].worldTile->state = TILESTATE_OCCUPIED;

			path->startFrame = actFrameCount;
			path->endFrame = path->startFrame + ((path->nodes[0].speed*60)>>12);
			return;
		}
		else if( actFrameCount > path->startFrame+((path->endFrame-path->startFrame)/2) )
		{
			cur->inTile = path->nodes[2].worldTile;
		}
	}

	// Move towards next node - third condition is so fwd is not scaled to zero
	if( path->nodes[1].worldTile && path->nodes[2].worldTile )
	{
		fixed length;
		int progress;

		SubVectorFSS(&fwd,&path->nodes[2].worldTile->centre,&path->nodes[1].worldTile->centre);

		progress = max( (actFrameCount - path->startFrame), 1 );
		length = (progress<<12)/(path->endFrame - path->startFrame);

//		length = FDiv( ToFixed(actFrameCount-path->startFrame), ToFixed(path->endFrame-path->startFrame) );
		ScaleVectorFF(&fwd,length);
		AddVectorSFS(&act->actor->position,&fwd,&path->nodes[1].worldTile->centre);

		// Orientate to direction of travel
		MakeUnit( &fwd );
		if (!(cur->flags & ENEMY_NEW_FACEFORWARDS) && cur->visible)
			OrientateFS(&act->actor->qRot,&fwd,&path->nodes[1].worldTile->normal);

		// Elevate above gametile
		SetVectorFF(&up, &cur->inTile->normal);
		ScaleVectorFF( &up, path->nodes[0].offset );
		AddToVectorSF( &act->actor->position, &up );
	}
}


/*	--------------------------------------------------------------------------------
	Function		: InitEnemyLinkedList
	Purpose			: initialises the enemy linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void SlerpWaitingFlappyThing( ENEMY *cur )
{
 	IQUATERNION q1, q2, q3;
 	fixed speed;
 
 	SetQuaternion( &q1, &cur->nmeActor->actor->qRot );
	SetVectorFF( (FVECTOR *)&q3, &cur->path->nodes[cur->path->startNode].worldTile->normal );
 	q3.w = 0;
 	fixedGetQuaternionFromRotation( &q2, &q3 );
 	speed = FMul(cur->path->nodes[0].speed, gameSpeed);
 
 	if( speed > 4092 ) speed = 4092;
	 	IQuatSlerp( &q1, &q2, speed, &cur->nmeActor->actor->qRot );
}


void AddEnemyModelUpdates(char *name, ENEMY *nme)
{
	NMEHACK *hack; char *c, *d;
	
	// if we get to a dot in the name they must be the same, except the extension,
	// and who really gives a shit about that?
	
	for (hack=enemyAnimHack;hack->modelname;hack++)
	{
		c=hack->modelname,d=name;
		while(1)
		{
			if (*d=='.' && (!*c || *c=='.'))
			{
				nme->Update[1] = hackFuncs[hack->flags];
			}
			else if (*c != tolower(*d) || !*c) break;
			c++, d++;
		}
	}
}

ENEMY *CreateAndAddEnemy(char *eActorName, int flags, long ID, PATH *path, fixed animSpeed, unsigned char facing )
{
	ACTOR2 *actor;
	ENEMY *newItem;
	fixed shadowRadius = 0;
	int initFlags,i;

	initFlags = INIT_ANIMATION;

	if( flags & ENEMY_NEW_SHADOW )
	{
		initFlags |= INIT_SHADOW;
		shadowRadius = 819200;
	}

	// create and add the nme actor
	if (!(actor = CreateAndAddActor(eActorName,0,0,0,initFlags)))
		return NULL;

	if( enemyList.count >= enemyList.blocks || !(newItem = &enemyList.array[enemyList.count]) )
		return NULL;
//	newItem = (ENEMY *)MALLOC0(sizeof(ENEMY));

	newItem->nmeActor = actor;
	newItem->flags = flags;
	newItem->facing = facing;
	newItem->reactiveNumber = GetReactiveAnimNumber(eActorName);

	// set shadow radius (if applicable)
 	if( shadowRadius )
 		if (newItem->nmeActor->actor->shadow)//mmshadow
 			newItem->nmeActor->actor->shadow->radius = shadowRadius;//mmshadow

	actorAnimate(newItem->nmeActor->actor,0,YES,NO,animSpeed, 0);

	newItem->nmeActor->animSpeed = animSpeed;

	newItem->active			= 1;
	
	newItem->path			= NULL;
	newItem->inTile			= NULL;
	
	newItem->speed			= 4096;
	newItem->uid = ID;
	newItem->isSnapping = 0;

	AssignPathToEnemy(newItem,path,0);

	if( newItem->flags & ENEMY_NEW_SLERPPATH )
	{
		newItem->Update[0] = UpdateSlerpPathNME;
		actorAnimate(newItem->nmeActor->actor,NMEANIM_PATH_WALK1,YES,NO,animSpeed, 0);
		OrientateSS( &newItem->nmeActor->actor->qRot, &newItem->path->nodes->worldTile->dirVector[newItem->facing], &newItem->path->nodes->worldTile->normal );
	}
	else if( newItem->flags & ENEMY_NEW_FOLLOWPATH )
	{
		newItem->Update[0] = UpdatePathNME;
		actorAnimate(newItem->nmeActor->actor,NMEANIM_PATH_WALK1,YES,NO,animSpeed, 0);
		OrientateSS( &newItem->nmeActor->actor->qRot, &newItem->path->nodes->worldTile->dirVector[newItem->facing], &newItem->path->nodes->worldTile->normal );
	}
	else if( newItem->flags & ENEMY_NEW_WATCHFROG )
		newItem->Update[0] = UpdateFrogWatcher;
	else if( newItem->flags & ENEMY_NEW_SNAPFROG )
	{
		newItem->Update[0] = UpdateSnapper;
		actorAnimate(newItem->nmeActor->actor,NMEANIM_SNAP_IDLE,YES,NO,animSpeed, 0);
	}
	else if( newItem->flags & ENEMY_NEW_SNAPTILES )
	{
		newItem->isSnapping = -2;
		newItem->Update[0] = UpdateTileSnapper;
		actorAnimate(newItem->nmeActor->actor,NMEANIM_SNAP_IDLE,YES,NO,animSpeed, 0);
	}
	else if( newItem->flags & ENEMY_NEW_VENT )
	{
		newItem->isSnapping = -2;
		newItem->Update[0] = UpdateVent;
		OrientateSS( &newItem->nmeActor->actor->qRot, &newItem->path->nodes->worldTile->dirVector[newItem->facing], &newItem->path->nodes->worldTile->normal );
	}
	else if( (newItem->flags & ENEMY_NEW_MOVEUP) || (newItem->flags & ENEMY_NEW_MOVEDOWN) )
	{
		newItem->Update[0] = UpdateMoveVerticalNME;
		OrientateSS( &newItem->nmeActor->actor->qRot, &newItem->path->nodes->worldTile->dirVector[newItem->facing], &newItem->path->nodes->worldTile->normal );
	}
	else if( (newItem->flags & ENEMY_NEW_ROTATEPATH_XZ ) || (newItem->flags & ENEMY_NEW_ROTATEPATH_XY) || (newItem->flags & ENEMY_NEW_ROTATEPATH_ZY) )
	{
		newItem->Update[0] = UpdateRotatePathNME;
		actorAnimate(newItem->nmeActor->actor,NMEANIM_PATH_WALK1,YES,NO,animSpeed, 0);
	}
	else if( newItem->flags & ENEMY_NEW_HOMING )
	{
		newItem->Update[0] = UpdateHomingNME;
		SetVectorFF(&newItem->currNormal, &newItem->path->nodes->worldTile->normal );
		actorAnimate(newItem->nmeActor->actor,NMEANIM_HOMER_IDLE,YES,NO,animSpeed, 0);
	}
	else if( newItem->flags & ENEMY_NEW_MOVEONMOVE )
	{
		newItem->Update[0] = UpdateMoveOnMoveNME;
		actorAnimate(newItem->nmeActor->actor,NMEANIM_HOMER_IDLE,YES,NO,animSpeed, 0);
	}
	else if( newItem->flags & ENEMY_NEW_FLAPPYTHING )
		newItem->Update[0] = UpdateFlappyThing;
	else if( newItem->flags & ENEMY_NEW_RANDOMMOVE )
		newItem->Update[0] = UpdateRandomMoveNME;
	else if( newItem->flags & ENEMY_NEW_TILEHOMING )
	{
		newItem->Update[0] = UpdateTileHomingNME;
		actorAnimate(newItem->nmeActor->actor,NMEANIM_HOMER_IDLE,YES,NO,animSpeed, 0);
	}
	else if( newItem->flags & ENEMY_NEW_BATTLEMODE )
	{
		newItem->Update[0] = UpdateBattleEnemy;
		newItem->isWaiting = -1;
	}
	else // No update function - probably single flag nme
	{
		if( newItem->nmeActor->actor && newItem->path && newItem->path->nodes )
			OrientateSS( &newItem->nmeActor->actor->qRot, &newItem->path->nodes->worldTile->dirVector[newItem->facing], &newItem->path->nodes->worldTile->normal );
	}

	if( newItem->flags & ENEMY_NEW_BABYFROG )
	{
		char name[32];
		int n;

		for( i=0; i<numBabies; i++ )
			if( !babyList[i].baby )
			{
				if( path->nodes->offset2 )
					n = ((path->nodes->offset2>>12)/SCALE)-1;
				else
					n = i;

				if( n > numBabies )
					n = 0;
#ifdef PC_VERSION
				sprintf(name,"bfg0%lu.bmp",n+1);
				((MDX_ACTOR*)newItem->nmeActor->actor->actualActor)->overrideTex = GetTexEntryFromCRC(UpdateCRC(name));
#endif

#ifdef PSX_VERSION
#ifndef DREAMCAST_VERSION
				switch ( n )
				{
					case 0: newItem->nmeActor->actor->clutOverride = LOADPAL_Load16 ( greenpal ); break;
					case 1: newItem->nmeActor->actor->clutOverride = LOADPAL_Load16 ( yellowpal ); break;
					case 2:	newItem->nmeActor->actor->clutOverride = LOADPAL_Load16 ( bluepal ); break;
					case 3:	newItem->nmeActor->actor->clutOverride = LOADPAL_Load16 ( pinkpal ); break;
					case 4: newItem->nmeActor->actor->clutOverride = LOADPAL_Load16 ( redpal ); break;
				}
#endif				
#endif
				babyList[i].baby = newItem->nmeActor;
				babyList[i].enemy = newItem;
				break;
			}

		newItem->nmeActor->effects |= (EF_RIPPLE_RINGS | EF_SLOW);
		if( babyList[i].fxColour[0] ) newItem->nmeActor->effects |= EF_TINTRED;
		if( babyList[i].fxColour[1] ) newItem->nmeActor->effects |= EF_TINTGREEN;
		if( babyList[i].fxColour[2] ) newItem->nmeActor->effects |= EF_TINTBLUE;

		babyIcons[i]->r = babyList[i].fxColour[0];
		babyIcons[i]->g = babyList[i].fxColour[1];
		babyIcons[i]->b = babyList[i].fxColour[2];
#ifdef PC_VERSION
		if( !rHardware )
		{
			babyList[i].baby->depthShift -= 50;

			babyIcons[i]->r /= 2;
			babyIcons[i]->g /= 2;
			babyIcons[i]->b /= 2;
		}
#endif
	}

	if( player[0].worldNum == WORLDID_ANCIENT )
	{
		if( 
#ifdef PSX_VERSION
			strstr(eActorName, "STNWHEEL") || strstr(eActorName, "stnwheel")
#else
			!(strnicmp(eActorName,"STNWHEEL",8)) || !(strnicmp(eActorName,"stnwheel",8))
#endif
			)
		newItem->flags |= ENEMY_NEW_NOJUMPOVER;
	}

	AddEnemyModelUpdates(eActorName, newItem);

	AddEnemy(newItem);

	return newItem;
}

void AssignPathToEnemy(ENEMY *nme,PATH *path,unsigned long pathFlags)
{
	int i;
	SVECTOR enemyStartPos;

	nme->path = path;

	// check if pathnode indices need converting to game tile pointers
	if(pathFlags & PATH_MAKENODETILEPTRS)
		for(i=0; i<path->numNodes; i++)
			nme->path->nodes[i].worldTile = &firstTile[(unsigned long)path->nodes[i].worldTile];

	// set the start position for the enemy
	if( path->startNode < path->numNodes )
		path->fromNode = path->startNode;
	else
		path->fromNode = path->startNode = 0;


	if(nme->flags & ENEMY_NEW_FORWARDS)
	{
		// this enemy moves forward thru path nodes
		nme->flags |= ENEMY_NEW_FOLLOWPATH;
		nme->path->toNode = nme->path->fromNode + 1;

		if(nme->path->toNode > GET_PATHLASTNODE(path))
			nme->path->toNode = 0;

		if( (nme->flags & ENEMY_NEW_FACEFORWARDS) && CheckPathOrientation(path) )
		{
			nme->flags |= ENEMY_NEW_NOREORIENTATE;
#ifdef PSX_VERSION
//			nme->nmeActor->actor->flags |= ACTOR_NEW_NOREORIENTATE;
//			QuatToPSXMatrix(&nme->nmeActor->actor->qRot, &nme->nmeActor->actor->psiData.object->matrix);
#endif
		}
	}
	else if(nme->flags & ENEMY_NEW_BACKWARDS)
	{
		// this enemy moves backward thru path nodes
		nme->flags |= ENEMY_NEW_FOLLOWPATH;
		nme->path->toNode = nme->path->fromNode - 1;

		if(nme->path->toNode < 0)
			nme->path->toNode = GET_PATHLASTNODE(path);

		if( (nme->flags & ENEMY_NEW_FACEFORWARDS) && CheckPathOrientation(path) )
		{
			nme->flags |= ENEMY_NEW_NOREORIENTATE;
#ifdef PSX_VERSION
//			nme->nmeActor->actor->flags |= ACTOR_NEW_NOREORIENTATE;
//			QuatToPSXMatrix(&nme->nmeActor->actor->qRot, &nme->nmeActor->actor->psiData.object->matrix);
#endif
		}
	}
	else if((nme->flags & ENEMY_NEW_MOVEUP) ||
			(nme->flags & ENEMY_NEW_MOVEDOWN))
	{
		// this enemy moves up or down
		nme->path->fromNode = nme->path->toNode = 0;
	}

	// set enemy position to relevant point on path
	GetPositionForPathNode(&enemyStartPos,&path->nodes[nme->path->fromNode]);
	SetVectorSS(&nme->nmeActor->actor->position, &enemyStartPos);
	NormalToQuaternion(&nme->nmeActor->actor->qRot,&path->nodes[nme->path->fromNode].worldTile->normal);

	// set enemy current 'in' tile and speeds and pause times
	nme->inTile		= path->nodes[nme->path->fromNode].worldTile;
	nme->speed		= path->nodes[nme->path->fromNode].speed;
	nme->isWaiting	= path->nodes[nme->path->fromNode].waitTime;

	nme->path->startFrame = actFrameCount;
	nme->path->endFrame = (actFrameCount+((Fabs(nme->speed)*60)>>12));

	CalcEnemyNormalInterps(nme);
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateEnemyPathNodes
	Purpose			: updates enemy path move status
	Parameters		: ENEMY *
	Returns			: void
	Info			: 
*/
void UpdateEnemyPathNodes(ENEMY *nme)
{
	SVECTOR nmePos;
	PATH *path = nme->path;
	unsigned long flags = nme->flags;
	
	if(flags & ENEMY_NEW_FORWARDS)	// enemy moves forward through path nodes
	{
		if(path->toNode >= GET_PATHLASTNODE(path))	// reached end of path nodes
		{
			if(flags & ENEMY_NEW_PINGPONG)		// this enemy has ping-pong movement
			{
				nme->flags		^= (ENEMY_NEW_FORWARDS | ENEMY_NEW_BACKWARDS);	// reverse enemy path movement
				path->fromNode	= GET_PATHLASTNODE(path);
				path->toNode	= GET_PATHLASTNODE(path) - 1;
			}
			else if(flags & ENEMY_NEW_CYCLE) 	// enemy has cyclic movement
			{
				path->fromNode	= GET_PATHLASTNODE(path);
				path->toNode	= 0;
			}
			else
			{
				path->fromNode	= 0;
				path->toNode	= 1;
				actorResetAnimation(nme->nmeActor->actor);
				//GetPositionForPathNode(&nmePos,&path->nodes[0]);
				//SetVectorSS(&nme->nmeActor->actor->position, &nmePos);
			}
		}
		else
		{
			path->fromNode = path->toNode;
			path->toNode++;
		}

 		if( path->nodes[path->fromNode].sample )
 			PlaySample( path->nodes[path->fromNode].sample, &nme->nmeActor->actor->position, 0, SAMPLE_VOLUME, -1 );
	}
	else if(flags & ENEMY_NEW_BACKWARDS) // enemy moves backwards through path nodes
	{
		if(path->toNode <= 0)
		{
			// reached beginning of path nodes
			// check if this enemy has ping-pong movement
			if(flags & ENEMY_NEW_PINGPONG)
			{
				// reverse enenmy path movement
				nme->flags		^= (ENEMY_NEW_FORWARDS | ENEMY_NEW_BACKWARDS);	// reverse enemy path movement
				path->fromNode	= 0;
				path->toNode	= 1;
			}
			else if(flags & ENEMY_NEW_CYCLE)
			{
				// enenmy has cyclic movement
				path->fromNode	= 0;
				path->toNode	= GET_PATHLASTNODE(path);
			}
			else
			{
				path->fromNode	= GET_PATHLASTNODE(path);
				path->toNode	= GET_PATHLASTNODE(path) - 1;
				//GetPositionForPathNode(&nmePos,&path->nodes[GET_PATHLASTNODE(path)]);
				//SetVectorSS(&nme->nmeActor->actor->position, &nmePos);
			}
		}
		else
		{
			path->fromNode = path->toNode;
			path->toNode--;
		}

 		if( path->nodes[path->fromNode].sample )
 			PlaySample( path->nodes[path->fromNode].sample, &nme->nmeActor->actor->position, 0, SAMPLE_VOLUME, -1 );
	}
	else if( flags & (ENEMY_NEW_MOVEDOWN | ENEMY_NEW_MOVEUP) )
	{
		if( flags & ENEMY_NEW_PINGPONG )
		{
			nme->flags	^= (ENEMY_NEW_MOVEUP | ENEMY_NEW_MOVEDOWN);

			if( path->nodes->sample )
 				PlaySample( path->nodes->sample, &nme->nmeActor->actor->position, 0, SAMPLE_VOLUME, -1 );
		}
	}

	nme->speed		= path->nodes[path->fromNode].speed;
	nme->isWaiting	= path->nodes[path->fromNode].waitTime;

	// Stop overshoot when waiting on a path node
	if (nme->isWaiting)
	{
		if( nme->flags & ENEMY_NEW_MOVEDOWN)
			GetPositionForPathNodeOffset2(&nmePos, &path->nodes[path->fromNode]);
		else
			GetPositionForPathNode(&nmePos, &path->nodes[path->fromNode]);

		SetVectorSS(&nme->nmeActor->actor->position, &nmePos);
	}

	if (nme->flags & ENEMY_NEW_RANDOMSPEED)
	{
		nme->speed = FMul( nme->speed, ENEMY_RANDOMNESS );
	}
}


/*	--------------------------------------------------------------------------------
	Function		: CalcEnemyNormalInterps
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
void CalcEnemyNormalInterps(ENEMY *nme)
{
	PATH *path;
	PATHNODE *fromNode,*toNode;
	fixed numSteps;
	FVECTOR destNormal;
	SVECTOR fromPos,toPos;

	path = nme->path;
	if(path->numNodes < 2)
		return;

	fromNode	= &path->nodes[path->fromNode];
	toNode		= &path->nodes[path->toNode];

	// set the current enemy normal to that of the 'from' node and get the dest normal
	SetVectorFF(&nme->currNormal, &fromNode->worldTile->normal);
	SetVectorFF(&destNormal, &toNode->worldTile->normal);

	// calculate deltas for linear interpolation of enemy normal during movement
	SubVectorFFF(&nme->deltaNormal,&destNormal,&nme->currNormal);

	// determine number of 'steps' over which to interpolate
	GetPositionForPathNode(&fromPos,fromNode);
	GetPositionForPathNode(&toPos,toNode);
	
	// Not sure if this will work - faster than using fmul and fdiv though.
	numSteps = DistanceBetweenPointsSS(&fromPos,&toPos)>>12;
	numSteps = max( (numSteps * nme->speed)>>12, 1 );

 	nme->deltaNormal.vx = nme->deltaNormal.vx / numSteps;
 	nme->deltaNormal.vy = nme->deltaNormal.vy / numSteps;
 	nme->deltaNormal.vz = nme->deltaNormal.vz / numSteps;
}


/*	--------------------------------------------------------------------------------
	Function		: InitEnemyLinkedList
	Purpose			: initialises the enemy linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void InitEnemyLinkedList()
{
	enemyList.array = NULL;
	enemyList.count = 0;
	enemyList.head.next = enemyList.head.prev = &enemyList.head;

	// Setup death animation function pointers
	deathAnims[DEATHBY_NORMAL]		= DeathNormal;
	deathAnims[DEATHBY_RUNOVER]		= DeathRunOver;
	deathAnims[DEATHBY_DROWNING]	= DeathDrowning;
	deathAnims[DEATHBY_SQUASHED]	= DeathSquashed;
	deathAnims[DEATHBY_FIRE]		= DeathFire;
	deathAnims[DEATHBY_ELECTRIC]	= DeathElectric;
	deathAnims[DEATHBY_FALLING]		= DeathFalling;
	deathAnims[DEATHBY_WHACKING]	= DeathWhacking;
	deathAnims[DEATHBY_INFLATION]	= DeathInflation;
	deathAnims[DEATHBY_POISON]		= DeathPoison;
	deathAnims[DEATHBY_SLICING]		= DeathSlicing;
	deathAnims[DEATHBY_EXPLOSION]	= DeathExplosion;
	deathAnims[DEATHBY_GIBBING]		= DeathGibbing;
	deathAnims[DEATHBY_INCINERATE]	= DeathIncinerate;
	deathAnims[DEATHBY_BURSTING]	= DeathBursting;
	deathAnims[DEATHBY_SPIKED]		= DeathSpiked;
	deathAnims[DEATHBY_LIGHTNING]	= DeathLightning;
}

/*	--------------------------------------------------------------------------------
	Function		: AddEnemy
	Purpose			: adds an enemy to the linked list
	Parameters		: ENEMY *
	Returns			: void
	Info			: 
*/
void AddEnemy(ENEMY *enemy)
{
	ENEMY *cur;

	if( !enemy || enemy->next )
		return;

	// Try to find another of the same type as the new enemy
	for( cur = enemyList.head.next; cur != &enemyList.head; cur = cur->next )
		if( cur->Update == enemy->Update )
		{
			// If update funcs the same, add before existing one
			enemy->prev = cur->prev;
			enemy->next = cur;
			cur->prev->next = enemy;
			cur->prev = enemy;
			break;
		}

	// If enemy is the first of its kind, add at head of list
	if( !enemy->next )
	{
		enemy->prev = &enemyList.head;
		enemy->next = enemyList.head.next;
		enemyList.head.next->prev = enemy;
		enemyList.head.next = enemy;
	}

	enemyList.count++;
}

/*	--------------------------------------------------------------------------------
	Function		: SubEnemy
	Purpose			: removes an enemy from the linked list
	Parameters		: ENEMY *
	Returns			: void
	Info			: 
*/
void SubEnemy(ENEMY *enemy)
{
	//n.b paths alloced and freed separately

	if(enemy->next == NULL)
		return;

	actor2Free(enemy->nmeActor);

	enemy->prev->next = enemy->next;
	enemy->next->prev = enemy->prev;
	enemy->next = NULL;
	enemyList.count--;

	if(enemy->path)
		FREE(enemy->path);

//	FREE(enemy);
}


/*	--------------------------------------------------------------------------------
	Function		: FreeEnemyLinkedList
	Purpose			: frees the enemy linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void FreeEnemyLinkedList()
{
//	if(enemyList.count)
//		utilPrintf("Freeing linked list : ENEMY : (%d elements)\n",enemyList.count);

	// traverse enemy list and free elements
	while( enemyList.head.next && enemyList.head.next != &enemyList.head )
		SubEnemy(enemyList.head.next);

	FreeNmeBlock( );

	// initialise list for future use
	InitEnemyLinkedList();
}


/*	--------------------------------------------------------------------------------
	Function		: AllocNmeBlock
	Purpose			: Initialise a fixed number of enemies
	Parameters		: 
	Returns			: void
	Info			: 
*/
void AllocNmeBlock( int num )
{
	if( !enemyList.array )
		enemyList.array = (ENEMY *)MALLOC0(sizeof(ENEMY)*num);

	enemyList.count = 0;
	enemyList.blocks = num;
}

/*	--------------------------------------------------------------------------------
	Function		: FreeNmeBlock
	Purpose			: Free statically alloced enemies
	Parameters		: 
	Returns			: void
	Info			: 
*/
void FreeNmeBlock( )
{
	if( enemyList.array )
		FREE( enemyList.array );

	enemyList.array = NULL;
	enemyList.count = 0;
	enemyList.blocks = 0;
}


/*	--------------------------------------------------------------------------------
    Function	: EnumEnemies
	Purpose		: Calls a function for every enemy with a given UID
	Parameters	: 
	Returns		: 

	func takes two params, the platform and the 'param' passed to EnumPlatforms
*/
int EnumEnemies(long id, int (*func)(ENEMY*, int), int param)
{
	ENEMY *cur;
	int count = 0;

	for(cur = enemyList.head.next; cur != &enemyList.head; cur = cur->next, count++)
	{
		if (!id || cur->uid == id)
		{
			if (!func(cur, param)) break;
		}
	}

	if (count==0)
		utilPrintf("EnumEnemies found 0 enemies?\n");

	return count;
}

void SetEnemyVisible(ENEMY *nme, int visible)
{
	if (visible)
	{
		nme->active = 1;
		nme->nmeActor->draw = 1;
		
		if (nme->isWaiting != -1)
		{
			nme->isWaiting = -1;
			SetEnemyMoving(nme, 1);
		}
	}
	else
	{
		nme->nmeActor->draw = 0;
		nme->active = 0;
	}
}

void SetEnemyMoving(ENEMY *nme, int moving)
{
	if (moving)
	{
		if (nme->isWaiting)
		{
			nme->isWaiting = 0;
			//nme->path->toNode = nme->path->fromNode;
			nme->path->startFrame = actFrameCount;
			nme->path->endFrame = actFrameCount + ((60*nme->speed)>>12);
		}
	}
	else
	{
		nme->isWaiting = -1;

		// JIM: This stops brain monkeys flicking when restarted. Nasty I know, but what the hell.
		if( (nme->flags & ENEMY_NEW_MOVEONMOVE) || (nme->flags & ENEMY_NEW_BATTLEMODE) )
		{
			if( nme->path->nodes[2].worldTile )
			{
				//nme->inTile = nme->path->nodes[2].worldTile;
				nme->path->nodes[1].worldTile = nme->path->nodes[2].worldTile;
				nme->path->nodes[2].worldTile = NULL;
			}
			else
			{
				nme->path->nodes[1].worldTile = nme->inTile;
				nme->path->nodes[2].worldTile = NULL;//nme->inTile;
			}
		}
	}
}

/*	--------------------------------------------------------------------------------
    Function	: MoveEnemy
	Purpose		: Moves an enemy to a given node in its path
	Parameters	: ENEMY*, int [suitable for enumEnemies]
	Returns		: 1 for success
*/
int MoveEnemyToNode(ENEMY *nme, int node)
{
	ENEMY_UPDATEFUNC *update;
	// Gratuitous hack to make move-on-move enemies work
	if( (nme->flags & ENEMY_NEW_MOVEONMOVE) || (nme->flags & ENEMY_NEW_BATTLEMODE) )
	{
		ENEMY *ph = GetEnemyFromUID(node);

		if( ph )
		{
			nme->path->nodes[0].worldTile = ph->path->nodes[0].worldTile;

			nme->inTile = nme->path->nodes[0].worldTile;

			if( nme->flags & ENEMY_NEW_BATTLEMODE )
			{
				if( nme->path->nodes[2].worldTile->state == TILESTATE_OCCUPIED )
					nme->path->nodes[2].worldTile->state = TILESTATE_NORMAL;
				nme->path->nodes[1].worldTile = ph->path->nodes[1].worldTile;
				nme->path->nodes[2].worldTile = ph->path->nodes[2].worldTile;
			}

			GetPositionForPathNode( &nme->nmeActor->actor->position, &nme->path->nodes[0] );
			nme->isSnapping = 0;
		}
		else
			utilPrintf("MoveEnemyToNode(): Could not find placeholder %d\n", node);
	}
	else if (node >= 0 && node < nme->path->numNodes)
	{
		nme->path->toNode  = node;
		nme->inTile = nme->path->nodes[node].worldTile;
		GetPositionForPathNode( &nme->nmeActor->actor->position, &nme->path->nodes[node] );
		nme->path->endFrame = actFrameCount;
		
		for (update=nme->Update;*update;update++)
			(*update)(nme);
	}
	else
		utilPrintf("MoveEnemyToNode(): Flag (%d) out of range\n", node);
	return 1;
}

/*	--------------------------------------------------------------------------------
	Function		: DoEnemyCollision
	Purpose			: Check for collision with frog in single and multiplayer
	Parameters		: 
	Returns			: void
	Info			: 
*/
int RadiusCheck(SVECTOR *a, SVECTOR *b, fixed radius)
{
	int rsqr = (radius>>12)*(radius>>12);
	return (
		((a->vx-b->vx)*(a->vx-b->vx)) < rsqr &&
		((a->vy-b->vy)*(a->vy-b->vy)) < rsqr &&
		((a->vz-b->vz)*(a->vz-b->vz)) < rsqr
	);
}

/*	--------------------------------------------------------------------------------
	Function		: EnemyReachedTopOrBottomPoint
	Purpose			: checks if an enemy has arrived at top or bottom point (1 node based)
	Parameters		: ENEMY *
	Returns			: BOOL - TRUE if node reached, else FALSE
	Info			: 
*/
BOOL EnemyReachedTopOrBottomPoint(ENEMY *nme)
{
	SVECTOR toPos;
	PATH *path = nme->path;

	// check if path extreme point is reached
	if(nme->flags & ENEMY_NEW_MOVEUP)
	{
		// moving up
		GetPositionForPathNodeOffset2(&toPos,&path->nodes[0]);

//bb		if(DistanceBetweenPointsSquared(&nme->nmeActor->actor->position,&toPos) < (100 * gameSpeed))
		if(DistanceBetweenPointsSS(&nme->nmeActor->actor->position,&toPos) < (10 * gameSpeed * SCALE <<12))
			return TRUE;
	}
	else if(nme->flags & ENEMY_NEW_MOVEDOWN)
	{
		// moving down
		GetPositionForPathNode(&toPos,&path->nodes[0]);

		if(DistanceBetweenPointsSS(&nme->nmeActor->actor->position,&toPos) < (10 * gameSpeed * SCALE <<12))
			return TRUE;
	}

	return FALSE;
}


/*	--------------------------------------------------------------------------------
	Function		: FindEnemyAtNode
	Purpose			: Search enemy list for an enemy that is on this node
	Parameters		: Node to compare
	Returns			: Found enemy or NULL
	Info			: nme is an optional parameter to allow for exclusions to the search. Also, enemies must share paths
*/
ENEMY *FindEnemyAtNode( ENEMY *nme, int n )
{
	ENEMY *e;
	PATH *p = nme->path;
	int nn, pn, node;

	nn = n+1;
	if( nn >= p->numNodes ) nn = 0;
	pn = n-1;
	if( pn < 0 ) pn = p->numNodes-1;

	for( e = enemyList.head.next; e != &enemyList.head; e = e->next )
	{
		node = e->path->toNode;

		if( (nme && e != nme) && e->active && 
			((&e->path->nodes[node] == &p->nodes[nn]) || (&e->path->nodes[node] == &p->nodes[pn]) || (&e->path->nodes[node] == &p->nodes[n])) )
			return e;
	}

	return NULL;
}
