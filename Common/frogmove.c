 /*
	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: frogmove.c
	Programmer	: Andrew Eder
	Date		: 11/13/98

-----------------------------------------------------------------------------------------------*/


//#include "directx.h"

//#include "ultra64.h"
//#include <stdio.h>

#include "sonylibs.h"
#include "shell.h"
#include "islpsi.h"
#include <islutil.h>

#include "frogmove.h"
#include "maths.h"
#include "collect.h"
#include "cam.h"
#include "anim.h"
#include "platform.h"
#include "tongue.h"
#include "enemies.h"
#include "babyfrog.h"
#include "frontend.h"
#include "game.h"
#include "main.h"
#include "newpsx.h"
#include "layout.h"
#include "multi.h"
#include "menus.h"

#ifdef PC_VERSION
#include <pcaudio.h>
#else
#include "audio.h"
#endif

long hopCount = 0;
long hopTimer = 0;
long numHops_TOTAL = 0;
long speedHops_TOTAL = 0;
long numHealth_TOTAL = 0;

GAMETILE *destTile[4]			= {0,0,0,0};
GAMETILE *currTile[4]			= {0,0,0,0};
GAMETILE *prevTile				= NULL;

fixed frogAnimSpeed		= 200;
fixed frogAnimSpeed2		= 400;

int	frogFacing[4]				= {0,0,0,0};
int	oldFrogFacing[4]			= {0,0,0,0};
int nextFrogFacing[4]			= {0,0,0,0};

unsigned long standardHopFrames = 7;
unsigned long battleHopFrames	= 15;
unsigned long quickSuperFrames	= 24;
unsigned long superHopFrames	= 32;
unsigned long quickDoubleFrames	= 32;
unsigned long doubleHopFrames	= 44;
unsigned long quickHopFrames	= 4;
unsigned long slowHopFrames		= 14;
unsigned long floatFrames       = 30;
unsigned long iceFrames			= 15;
unsigned long conveyorFrames[3] = { 60, 30, 15 };

unsigned long standardHopJumpDownDivisor	= 10;
unsigned long superHopJumpDownDivisor		= 12;

unsigned long lastPlayerAnimCooldown = 0;

struct {
	int lastHopOn;
	int freq;
	int Time;
	int Addr;
	int Subr;
	int Max;
	int Min;
} frogPitch = { 0, 128, 15, 2, 8, 64,50 };


#define MAX_HOP_HEIGHT			(( 26*SCALE)<<12)	// +1 for rounding :o)
#define MAX_SUPERHOP_HEIGHT		(( 51*SCALE)<<12)
#define DROP_HURT_HEIGHT		((100*SCALE)<<12)
#define DROP_KILL_HEIGHT		((150*SCALE)<<12)

fixed	hopHeight		= (30*SCALE)<<12;
fixed	superhopHeight	= (50*SCALE)<<12;
fixed	doublehopHeight = (75*SCALE)<<12;

fixed	floatMultiply	= 1024;

BOOL cameoMode			= FALSE;


// PC doesn't care about case now - can use same array (YAY!)
char *headNames[] = 
{
	"FGHED",	//FROG_FROGGER
	"FMHED",	//FROG_LILLIE
	"TADHED",	//FROG_BABYFROG
	"TWHED",	//FROG_TWEE
	"TDHED",	//FROG_WART
	"THED",		//FROG_ROBOFROG
	"ROOBHED",	//FROG_HOPPER
	"SSPYHED",	//FROG_SWAMPY
};

#ifdef PC_VERSION

float *headMatrix=NULL, *breastMatrix=NULL, *headPostMatrix=NULL;

#else

MATRIX headMatrix;
MATRIX *pHeadMatrix=NULL;

MATRIX breastMatrix[4];
MATRIX *pBreastMatrix[4]={NULL,NULL,NULL,NULL};

#endif



// heh heh.. breast.. heh heh..

/* ---------------- Local functions -------------- */

void CalculateFrogJump(SVECTOR *startPos, SVECTOR *endPos, FVECTOR *normal, fixed height, long time, long player);
void CheckTileState(GAMETILE *tile, int pl);
void CheckForFroggerLanding(long pl);
BOOL GameTileTooHigh(GAMETILE *tile,long pl);

/*	--------------------------------------------------------------------------------
	Function		: SetFroggerStartPos
	Purpose			: sets initial position of Frogger on the world
	Parameters		: GAMETILE *,long
	Returns			: none
	Info			:
*/
void SetFroggerStartPos(GAMETILE *startTile,long p)
{
	ENEMY *cur;

	for(cur = enemyList.head.next; cur != &enemyList.head; cur = cur->next)
	{
		cur->flags &= ~ENEMY_NEW_DONOTMOVE;
		if( cur->flags & ENEMY_NEW_FLAPPYTHING )
		{
			cur->active = 1;
			cur->visible = 1;
			cur->nmeActor->draw = 1;
			SetVectorSS( &cur->nmeActor->actor->position, &cur->path->nodes->worldTile->centre );
			SetVectorFF( &cur->nmeActor->actor->size, &oneVec );
		}
	}

	// Change frog's position to reflect that of the start tile
	SetVectorSS(&frog[p]->actor->position,&startTile->centre);

	actorAnimate(frog[p]->actor,FROG_ANIM_BREATHE,YES,NO,64,0);

	if( frog[p]->actor->shadow ) 
	{
#ifdef PC_VERSION
		if( rHardware ) frog[p]->actor->shadow->draw = 1;
		else frog[p]->actor->shadow->draw = 0;
#else
		frog[p]->actor->shadow->draw = 0;
#endif
	}

	if (currPlatform[p])
	{
		currPlatform[p]->carrying = 0;
		currPlatform[p]->flags &= ~PLATFORM_NEW_CARRYINGFROG;
	}

	currTile[p]		= startTile;

	destTile[p]		= NULL;
	currPlatform[p]	= NULL;
	destPlatform[p]	= NULL;

	player[p].healthPoints	= 1;
	GTInit( &player[p].isCroaking, 0 );
//	GTInit( &player[p].isOnFire, 0 );
	GTInit( &player[p].stun, 0 );
	GTInit( &player[p].safe, 0 );
	GTInit( &player[p].dead, 0 );
	GTInit( &player[p].autohop, 0 );
//	GTInit( &player[p].longtongue, 0 );
	GTInit( &player[p].quickhop, 0 );
	GTInit( &player[p].slowhop, 0 );

	// set frog action movement variables
	ZeroVector(&frog[p]->actor->vel);

	player[p].deathBy			= 0;
	player[p].canJump			= 1;
	player[p].frogState			= 0;
	player[p].isSuperHopping	= 0;
	player[p].isSinking			= 0;
	player[p].idleTime			= MAX_IDLE_TIME;
	player[p].idleEnable		= 1;
	player[p].heightJumped		= 0;
	player[p].jumpTime			= -1;
	//player[p].extendedHopDir	= (startFrogFacing-startCamFacing)&3;

	fixedPos = 0;
	fixedDir = 0;

	// JIM: I hope this doesn'y break owt...
	cameoMode = 0;

	frog[p]->draw = 1;
	frog[p]->clipped = 0;
	frog[p]->milesAway = 0;

 	frog[p]->actor->size.vx = globalFrogScale;
 	frog[p]->actor->size.vy = globalFrogScale;
 	frog[p]->actor->size.vz = globalFrogScale;

	if( player[p].character == FROG_BABYFROG )
		ScaleVectorFF( &frog[p]->actor->size, 3072 );

	camFacing[p] = startCamFacing;
	frogFacing[p] = startFrogFacing;
	player[p].extendedHopDir = (startFrogFacing - startCamFacing)&3;
	OrientateSS( &frog[p]->actor->qRot, &currTile[p]->dirVector[frogFacing[p]], &currTile[p]->normal );

#ifdef PC_VERSION
	FindFrogSubObjects( p );
#else
	//bb - we should make a version of FindFrogSubObjects
	//'cos we'll need breastmtx and another headmtx
	if(!pHeadMatrix)
	{
		PSIOBJECT *obj;
		char objName[16];

		sprintf(objName, headNames[player[p].character]);
		
		if( (obj = psiObjectScan(frog[p]->actor->psiData.object, objName)) )
		{
			pHeadMatrix= &headMatrix;
		}
		else
		{
			pHeadMatrix = NULL;
		}
	}

#endif

	if( !(gameState.multi != SINGLEPLAYER && multiplayerMode == MULTIMODE_BATTLE) )
		ReactivatePowerups( );
}

/*	--------------------------------------------------------------------------------
	Function		: SetFroggerOnTile
	Parameters		: tile, player
	Returns			: void
	Info			:
*/
void SetFroggerOnTile(GAMETILE *tile, long pl)
{
	if (currPlatform[pl])
	{
		currPlatform[pl]->carrying = 0;
		currPlatform[pl]->flags &= ~PLATFORM_NEW_CARRYINGFROG;
		currPlatform[pl] = NULL;
	}

	currTile[pl] = tile;

	CheckTileState(tile, pl);
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateFroggerControls
	Parameters		: player number
	Returns			: void
	Info			:
*/

BOOL UpdateFroggerControls(long pl)
{
	/* ----------------------- Frog wants to HOP u/d/l/r ----------------------------- */

	//if (!player[pl].canJump) return;

	if(player[pl].frogState & (FROGSTATUS_ISWANTINGU|FROGSTATUS_ISWANTINGL|FROGSTATUS_ISWANTINGR|FROGSTATUS_ISWANTINGD))
	{
		int dir, jump, actF, oldFacing;

		if(player[pl].frogState & FROGSTATUS_ISWANTINGU)		dir = MOVE_UP;
		else if(player[pl].frogState & FROGSTATUS_ISWANTINGD)	dir = MOVE_DOWN;
		else if(player[pl].frogState & FROGSTATUS_ISWANTINGL)	dir = MOVE_LEFT;
		else dir = MOVE_RIGHT;

		player[pl].frogState &= ~FROGSTATUS_ALLHOPFLAGS;

		if ((actFrameCount-frogPitch.lastHopOn)<frogPitch.Time)
		{
			if (frogPitch.freq<frogPitch.Max)
				frogPitch.freq+=frogPitch.Addr;
		}
		else
			frogPitch.freq = frogPitch.Min;
		
		actF = frogPitch.freq+Random(10);

		frogPitch.lastHopOn = actFrameCount;

		prevTile = currTile[pl];

 		AnimateFrogHop((dir - camFacing[pl]) & 3, pl);
		oldFacing = frogFacing[pl];
		frogFacing[pl] = (camFacing[pl] + dir) & 3;
		jump = MoveToRequestedDestination(dir,pl);

		if( jump )
		{
			PlaySample(genSfx[GEN_FROG_HOP],NULL,0,100-Random(15),actF);
			hopCount++;
			hopTimer = 30 * 4096;
		}
		else
		{
			switch( (((dir - camFacing[pl]) & 3) - oldFacing) & 3 )
			{
			case 1: actorAnimate(frog[pl]->actor,FROG_ANIM_HOPLEFT,NO,NO,frogAnimSpeed2,0); break;
			case 3: actorAnimate(frog[pl]->actor,FROG_ANIM_HOPRIGHT,NO,NO,frogAnimSpeed2,0); break;
			default:
				if( hopCount >= 3 )
				{
					actorAnimate(frog[pl]->actor, FROG_ANIM_WALLSPLAT, NO, NO, 154, NO);
					PlayVoice( pl, "frogannoyed" );
				}
				else
				{
					actorAnimate(frog[pl]->actor, FROG_ANIM_LOOKUP, NO, NO, 154, NO);
				}
				break;
			}

			actorAnimate(frog[pl]->actor, FROG_ANIM_BREATHE, YES, YES, FROG_BREATHE_SPEED, NO);

			hopCount = 0;
			hopTimer = 0;
		}
	}

  	/* ----------------------- Frog wants to SUPERHOP u/d/l/r ----------------------------- */

	else if(player[pl].frogState & (FROGSTATUS_ISWANTINGSUPERHOPU|FROGSTATUS_ISWANTINGSUPERHOPL|FROGSTATUS_ISWANTINGSUPERHOPR|FROGSTATUS_ISWANTINGSUPERHOPD))
	{
		int dir, jump = 0, t;
		
		if(player[pl].frogState & FROGSTATUS_ISWANTINGSUPERHOPU)		dir = MOVE_UP;
		else if(player[pl].frogState & FROGSTATUS_ISWANTINGSUPERHOPD)	dir = MOVE_DOWN;
		else if(player[pl].frogState & FROGSTATUS_ISWANTINGSUPERHOPL)	dir = MOVE_LEFT;
		else dir = MOVE_RIGHT;

		hopCount = 0;
		hopTimer = 0;

		player[pl].frogState &= ~FROGSTATUS_ALLHOPFLAGS;

		if ((!currPlatform[pl] && currTile[pl]->state == TILESTATE_NOSUPER) || currTile[pl]->state == TILESTATE_NOSUPERHOT)
		{
			//OrientateSS( &frog[pl]->actor->qRot, &currTile[pl]->dirVector[frogFacing[pl]], &currTile[pl]->normal );
			player[pl].isSuperHopping = 0;
			player[pl].canJump = 1;
			actorAnimate(frog[pl]->actor, FROG_ANIM_LOOKUP, NO, NO, 154, NO);
			actorAnimate(frog[pl]->actor, FROG_ANIM_BREATHE, YES, YES, FROG_BREATHE_SPEED, NO);
			return TRUE;
		}
			
		prevTile = currTile[pl];
		frogFacing[pl] = (camFacing[pl] + dir) & 3;

		player[pl].frogState |= FROGSTATUS_ISSUPERHOPPING;
		jump = MoveToRequestedDestination(dir,pl);

		if( !jump )
		{
			player[pl].frogState &= ~(FROGSTATUS_ALLHOPFLAGS | FROGSTATUS_ISJUMPINGTOTILE | FROGSTATUS_ISJUMPINGTOPLATFORM);
			if(gameState.mode != FRONTEND_MODE)
			{
				player[pl].hasJumped = 1;
				player[pl].isSuperHopping = 1;
				player[pl].canJump = 0;
				nextFrogFacing[pl] = frogFacing[pl]; // ds
				
				if( player[pl].quickhop.time )
					t = quickSuperFrames;
				else
					t = superHopFrames;

				// ds - hAck! Jump to the platform
				if (currPlatform[pl])
				{
					destPlatform[pl] = currPlatform[pl];
					player[pl].frogState |= FROGSTATUS_ISJUMPINGTOPLATFORM;

					CalculateFrogJumpS(
						&frog[pl]->actor->position,
						&currPlatform[pl]->pltActor->actor->position,
						&currTile[pl]->normal, superhopHeight, t, pl);
				}
				else
				{
					destTile[pl] = currTile[pl];
					CalculateFrogJumpS( &frog[pl]->actor->position, &currTile[pl]->centre, &currTile[pl]->normal, superhopHeight, t, pl);
				}
			}
			else
			{
				frogFacing[pl] = oldFrogFacing[pl];
				OrientateSS( &frog[pl]->actor->qRot, &currTile[pl]->dirVector[frogFacing[pl]], &currTile[pl]->normal );
			}
		}

		if((jump) || (gameState.mode != FRONTEND_MODE))
		{
	 		AnimateFrogHop((dir - camFacing[pl]) & 3,pl);
			PlaySample(genSfx[GEN_SUPER_HOP],NULL,0,255,-1/*64*/);
		}
	}
	else
	{
		if( hopTimer )
			hopTimer -= gameSpeed;

		if( hopTimer <= 0 )
		{
			hopTimer = 0;
			hopCount = 0;
		}

		return FALSE;	// nope, we didn't do nuffink
	}

	return TRUE;	// yep, we did somefink
}


/*	---------------------------------------------------------------------------------
	Notes on Frog Movement
	by Dave S.

	The frog's vertical motion during a jump is described by the simple parabolic curve equation

	y = h(1-(2mt-1)²)

	where h is the height of the curve and t is the 'time' variable (see below). m is a multiplier
	calculated using the formula

	m = (1+sqrtf(1-d))/2

	where d is the difference in height between the start and end position. height is displacement
	parallel to the starting normal vector. Horizontal motion is simply linear,	parallel to the
	plane described by the starting position and its normal. (phew)

	x = A(1-t) + Bt

	The variable t ranges from 0 (zero) at the start of the jump and 1 (one) at the end of the
	jump. This makes calculating the end of the jump nice and easy!
*/

/*	--------------------------------------------------------------------------------
	Function		: FroggerHop
	Parameters		: player
	Returns			: void
	Info			: See notes above for the maths

	Note that we also check for falling through platforms here! I know it's stupid, shut up.
*/
void FroggerHop(int pl)
{
	FVECTOR up, fwd;
	SVECTOR pos;
	fixed p, t, delta;
	
	PLATFORM *cur, *nearestPlat;
	long nearestPlatDist;

	static landingRadius = 220;	// just less than the radius of one tile
	static landingHeight = 200;	// how 'thick' platforms are (duhhh.. <- not that kind of thick)

	if (player[pl].jumpTime < 0) return;

	delta = FMul(player[pl].jumpSpeed, gameSpeed);

	if (player[pl].frogState & FROGSTATUS_ISFLOATING)
		delta = FMul(delta, floatMultiply);

	//if (delta >= 0) //This should never be false
	player[pl].jumpTime += delta;

	t = player[pl].jumpTime;

	// Calculate frog's position (see above for the maths)
	// horizontal (parallel to plane)
	
	SetVectorFF(&fwd, &player[pl].jumpFwdVector);
	fwd.vx = FMul(fwd.vx, t);
	fwd.vy = FMul(fwd.vy, t);
	fwd.vz = FMul(fwd.vz, t);
	
	AddVectorSFS(&pos, &fwd, &player[pl].jumpOrigin);

	// vertical

	if (player[pl].jumpMultiplier)
	{
		if( gameState.multi == SINGLEPLAYER )
		{
			// Stop looking at the frog if it's going to die from falling
			if( (player[pl].heightJumped < -DROP_KILL_HEIGHT && player[pl].jumpTime > 2048) || 
				((destTile[pl] && destTile[pl]->state == TILESTATE_DEADLY) && player[pl].jumpTime > 2600 && !destPlatform[pl]) )
				controlCamera = 1;
			else if( player[pl].deathBy != DEATHBY_WHACKING )//!(player[pl].frogState & FROGSTATUS_ISDEAD) )
				controlCamera = 0;
		}

		p = FMul(t,player[pl].jumpMultiplier)*2 - ToFixed(1);
		p = 4096 - FMul(p,p);
		SetVectorFF(&up, &player[pl].jumpUpVector);
		up.vx = FMul(up.vx, p);
		up.vy = FMul(up.vy, p);
		up.vz = FMul(up.vz, p);

		AddToVectorSF(&pos, &up);

		nearestPlat = NULL;
		nearestPlatDist = 1000;

		// And now... fun with platforms (again)
		// While we're jumping, we need to check if we've fallen through a platform
		// We also need to check if we're trying to jump under a 'don't jump under' platform
		// (since checking beforehand is unreliable)

		if (!destPlatform[pl] && !(player[pl].frogState & FROGSTATUS_ISDEAD))
		{
			int checked = 0;
			// find nearest platform
			for (cur = platformList.head.next; cur != &platformList.head; cur = cur->next)
			{
				long check;
				SVECTOR sv, plt;
				FVECTOR v;
				fixed before, after;	// height above platform before and after moving

				if (!cur->active || /*cur == currPlatform[pl] ||*/ cur->inTile[0]->state == TILESTATE_BARRED) continue;

				// don't include the *current* plat in the check
				// and only consider platforms in the dest tile?? (TODO: debug!)
				//if (cur->inTile[0] != destTile[pl] /*|| cur == currPlatform[pl]*/) continue;

				SubVectorSSS(&sv, &cur->pltActor->actor->position, &pos);
				
				// Make sv positive..
				if (sv.vx<0) sv.vx = -sv.vx;
				if (sv.vy<0) sv.vy = -sv.vy;
				if (sv.vz<0) sv.vz = -sv.vz;

				// Get maximum..
				check = sv.vx;
				if (sv.vy > check) check = sv.vy;
				if (sv.vz > check) check = sv.vz;

				// bounding-box discard
				if (check > nearestPlatDist) continue;

				// 'kay, now check to see if we've fallen through it...
				SetVectorSS(&plt, &cur->pltActor->actor->position);

				SubVectorFSS(&v, &pos, &plt);
				after = DotProductFF(&v, &currTile[pl]->normal);

				SubVectorFSS(&v, &frog[pl]->actor->position, &plt);
				before = DotProductFF(&v, &currTile[pl]->normal);

				if ((before > -landingHeight && 0 >= after) /*|| (before < 0 && after > 0)*/ )
				{
					fixed radius;
					// we've passed through the platform's plane (with a bit of a hacked threshold)

					// if the platform's tile and our destination are the same
					// then it's a pretty safe bet we've landed
					if (cur->inTile[0] == destTile[pl])
					{
						destPlatform[pl] = cur;
						player[pl].frogState &= ~FROGSTATUS_ALLHOPFLAGS;
						player[pl].frogState |= FROGSTATUS_ISJUMPINGTOPLATFORM;
						player[pl].jumpTime = 4096;	// aaand land.
						player[pl].heightJumped = DotProductFF(&up, &destPlatform[pl]->inTile[0]->normal);
						break;
					}

					// Now comes the nasty bit...
					// Project our 'after' position onto the tile's normal and subtract that from
					// the vector from the frog to the platform

					// The magnitude of this vector is the radius of an infinite cylinder centred on the
					// platform's centre, in the direction of its normal, with the frog's new
					// position a point on its perimeter...
					//      ___
					//     /   \
					//  N  |___|
					//  ^  |   |
					//  |  |   X	frog
					//  |  |../| 
					//  |  | / |
					//     \___/	(a bit like that)
					//		 -->r

					SetVectorFF(&v, &currTile[pl]->normal);
					ScaleVectorFF(&v, -after);
					
					SubFromVectorFS(&v, &plt);
					AddToVectorFS(&v, &pos);


					radius = MagnitudeF(&v)>>12;

					if (radius < landingRadius)
					{
						destPlatform[pl] = cur;
						player[pl].frogState &= ~FROGSTATUS_ALLHOPFLAGS;
						player[pl].frogState |= FROGSTATUS_ISJUMPINGTOPLATFORM;
						player[pl].jumpTime = 4096;	// aaand land.
						player[pl].heightJumped = DotProductFF(&up, &destPlatform[pl]->inTile[0]->normal);
						controlCamera = 0;
						break;
					}
				}
				else if (cur->flags & PLATFORM_NEW_NOWALKUNDER &&
					player[pl].jumpTime > 2048 && after < 0 && cur->inTile[pl] == destTile[pl])
				{

					// Fix for accidentally jumping underneath moving platforms - ds

					FVECTOR dir;

					dir.vx = -player[pl].jumpFwdVector.vx;
					dir.vy = -player[pl].jumpFwdVector.vy;
					dir.vz = -player[pl].jumpFwdVector.vz;

					currTile[pl] = destTile[pl];
					PushFrog(&dir, pl);
				}
			}
		}
	}

	if( player[pl].deathBy == DEATHBY_FALLINGFOREVER || t < (1<<12) )
	{
		// MY GOD this is a stupid hack
		if( !(frog[pl]->actor->position.vy < -30000 && pos.vy > 30000) ) // Stop wrapping
			SetVectorSS(&frog[pl]->actor->position, &pos);
		else
			frog[pl]->draw = 0;
	}

	if( !(player[pl].frogState & FROGSTATUS_ISDEAD) )
		frog[pl]->actor->shadow->draw = 1;

	if( currTile[pl]->state >= TILESTATE_CONVEYOR || currTile[pl]->state == TILESTATE_ICE )
	{
#ifdef PSX_VERSION
		frog[pl]->actor->shadow->draw = 0;
#else
		if( !rHardware ) frog[pl]->actor->shadow->draw = 0;
#endif
	}
	
/*
	// if we've just passed the midpoint, play a falling animation
	// this check is wrong and we're probably not gonna do this anyway - ds

	if (player[pl].jumpTime > player[pl].jumpMultiplier)
	{
		animSpeed = (player[pl].jumpSpeed * 30) >> 5;
		actorAnimate(frog[pl]->actor, FROG_ANIM_FALL, NO, NO, animSpeed, 0);
		if( !Random(10) )
			PlayVoice( pl, "froguhoh" );
	}
*/
}


#define DEC_POWERUP(x, pl) { \
	int oldTime = x.time;	\
	GTUpdate( &x, -1 ); \
	if( !x.time ) \
		PlayVoice( pl, "frogannoyed" ); \
	if((gameState.multi == SINGLEPLAYER) && (x.time != oldTime)) \
		PlaySample( genSfx[GEN_POWERTICK], NULL, 0, SAMPLE_VOLUME, -1 ); \
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateFroggerPos
	Parameters		: player number
	Returns			: void
	Info			:
*/

void UpdateFroggerPos(long pl)
{
	FVECTOR moveVec;

	if (cameoMode)
	{
		player[pl].frogState &= ~FROGSTATUS_ALLHOPFLAGS;
//			~(FROGSTATUS_ISWANTINGU | FROGSTATUS_ISWANTINGD |
//			FROGSTATUS_ISWANTINGL | FROGSTATUS_ISWANTINGR);
		//player[pl].canJump = 0;
	}

	if( player[pl].autohop.time )
		DEC_POWERUP( player[pl].autohop, pl );
	if( player[pl].quickhop.time )
		DEC_POWERUP( player[pl].quickhop, pl );
	if( player[pl].slowhop.time )
		DEC_POWERUP( player[pl].slowhop, pl );

	// But first... platforms
	if (currPlatform[pl])
	{
		//currTile[pl] = currPlatform[pl]->inTile[0];
	
		PLATFORM *plat = currPlatform[pl];
		GAMETILE *tile = plat->inTile[0];

		controlCamera = 0;

		if (tile != currTile[pl])
		{
			// mm, try this..?
			frogFacing[pl] = GetTilesMatchingDirection(currTile[pl], frogFacing[pl], tile);
			currTile[pl] = tile;
			OrientateSS( &frog[pl]->actor->qRot, &tile->dirVector[frogFacing[pl]], &tile->normal );

			CheckTileForCollectable(tile, pl);
		}

		if (tile->state == TILESTATE_BARRED)
		{
			// if we're moving onto a barred tile, push the frog in the other direction
			FVECTOR v;
			
			SubVectorFSS(&v,
				&plat->path->nodes[plat->path->fromNode].worldTile->centre,
				&plat->path->nodes[plat->path->toNode].worldTile->centre);
			MakeUnit( &v );

			PushFrog(&v, pl);
			//actorAnimate(frog[pl]->actor, FROG_ANIM_BOWLEDOVER, NO, NO, 256, NO);
		}
		else if (tile->state == TILESTATE_DEADLY)	// test for platforms going down through deadly tiles
		{
			SVECTOR v;
			SubVectorSSS(&v, &plat->pltActor->actor->position, &tile->centre);
			if (DotProductSS(&v, &tile->normal) < 0)
				SetFroggerOnTile(tile, pl);
		}	// hur hur, I typed 'going down'

	}
	else if( currTile[pl]->state == TILESTATE_SINK )
	{
		FVECTOR dir;
		fixed J, dist;

		SubVectorFSS( &dir, &frog[pl]->actor->position, &currTile[pl]->centre );
		MakeUnit( &dir );
		J = DotProductFF( &currTile[pl]->normal, &dir );
		dist = DistanceBetweenPointsSqrSS( &frog[pl]->actor->position, &currTile[pl]->centre );
		if( J < 0 ) dist *= -1;

		if( player[pl].frogState & FROGSTATUS_ALLHOPFLAGS )
		{
			SetVectorFF( &moveVec, &currTile[pl]->normal );
			ScaleVectorFF( &moveVec, gameSpeed*2 );
			RoundVectorRandF(&moveVec);
			AddToVectorSF( &frog[pl]->actor->position, &moveVec );

			// Frog has broken free!
			if( dist >= 0 )
				player[pl].isSinking = 0;
		}
		else if( player[pl].isSinking )
		{
			SetVectorFF( &moveVec, &currTile[pl]->normal );
			ScaleVectorFF( &moveVec, gameSpeed/-5 );
			RoundVectorRandF(&moveVec);
			AddToVectorSF( &frog[pl]->actor->position, &moveVec );

			// Frog is dead
			if( dist < -900 && !player[pl].dead.time )
			{
				if( frogPool[player[pl].character].anim )
					DeathDrowning( pl );
				else
					DeathMulti( pl );
			}
		}
	}
	else if ( currTile[pl]->state == TILESTATE_SAFE )
	{
		player[pl].frogState |= FROGSTATUS_ISSAFE;
	}
	else if ( currTile[pl]->state & TILESTATE_CONVEYOR )
	{
		/*	If we're on a conveyor and can jump - i.e. we're not already jumping -
			consider the player to be on its destination tile once it goes past half-way
			along its 'jump'.
		*/
		if (destTile[pl] && player[pl].canJump &&
			(player[pl].jumpTime > 2048))
		{
			currTile[pl] = destTile[pl];
			//destTile[pl] = 0;
		}
	}

	/*	--------------------------------------------------------------------------------------------
		Calculate frog hop
	*/

	FroggerHop(pl);

	//--------------------------------------------------------------------------------------------
	
	if( player[pl].frogState & FROGSTATUS_ISDEAD )
	{
		CheckForFroggerLanding(pl);

		if( gameState.multi == SINGLEPLAYER )
			KillFrog(pl);

		return;
	}
	
	if (cameoMode || !UpdateFroggerControls(pl))
	{
		CheckForFroggerLanding(pl);
	}
}

/*	--------------------------------------------------------------------------------
	Function		: GetNextTile(unsigned long direction)
	Purpose			: determines the next tile
	Parameters		:
	Returns			:
	Info			:
*/

long GetTilesMatchingDirection(GAMETILE *me, long direction, GAMETILE *next)
{
	long dirNumber,i;
	fixed t,distance = ToFixed(0);
	
	for(i=0; i<4; i++)
	{
		t = DotProductFF(&me->dirVector[direction],&next->dirVector[i]);
		if(t >= distance)
		{
			distance = t;
			dirNumber = i;			
		}
	}

	return dirNumber;
}

long GetTilesMatchingVector( SVECTOR *dir, GAMETILE *next)
{
	long dirNumber,i;
	fixed t,distance=0;
	
	for(i=0; i<4; i++)
	{
		t = DotProductFF(dir,&next->dirVector[i]);
		if(t >= distance)
		{
			distance = t;
			dirNumber = i;			
		}
	}

	return dirNumber;
}

/*	--------------------------------------------------------------------------------
	Function		: GetNextTile(unsigned long direction)
	Purpose			: Finds the next tile in a given direction
	Parameters		: direction, player
	Returns			: GAMETILE*
*/
GAMETILE *GetNextTile(unsigned long *pdir,long pl)
{
	GAMETILE *dest = NULL;
	long i = *pdir, direction = *pdir;
	fixed distance, t, t2, at2;
	VECTOR vecUp;
		
	if((gameState.mode == FRONTEND_MODE) && (currTileNum == TILENUM_OPTIONS))
	{
		dest = currTile[pl]->tilePtrs[1];
	}
	else
	{
		direction = *pdir;

		i = (direction + camFacing[pl] + 2) & 3;
		dest = currTile[pl]->tilePtrs[i];
	}

	*pdir = i;

	if (!dest || dest->state == TILESTATE_BARRED)
		return NULL;

	if(dest->state == TILESTATE_JOIN)	// || dest->state == TILESTATE_SUPERHOP
	{
		GAMETILE *joiningTile = dest;

		// Find the direction closest to "up" or "down" in the opposite direction to where we are now

		SetVectorFF(&vecUp, &currTile[pl]->normal);

		distance = ToFixed(-1000);
		
		for(i=0; i<4; i++)
			if(joiningTile->tilePtrs[i])
			{					
				t2 = DotProductFF(&vecUp,&joiningTile->dirVector[i]);
				t=Fabs(t2);
				if(t > distance)
					if(currTile[pl] != joiningTile->tilePtrs[i])
					{
						distance = t;
						direction = i;
						at2 = t2;
					}
			}

		dest = joiningTile->tilePtrs[direction];

		if (!dest)
			 return NULL;

		if (dest->state == TILESTATE_BARRED)
			return NULL;
	}

	return dest;
}

/*	-------------------------------------------------------------------------
	Function:	AnimateFrogHop
	Purpose:	Runs the appropriate hop animation
	Parameters:	direction, player
	Returns:
*/
void AnimateFrogHop( unsigned long direction, long pl )
{
	fixed animSpeed; 

	if(player[pl].isSuperHopping)
	{
		// play animation for superhopping
		animSpeed = (player[pl].jumpSpeed * 15) >> 4;	// play 15 frames of anim before landing
		actorAnimate(frog[pl]->actor, FROG_ANIM_SUPERHOP, NO, NO, animSpeed, 0);
		return;
	}
	else // Otherwise, play appropriate jump animation
	{
		switch ((direction - frogFacing[pl]) & 3)
		{
		case 1:
			actorAnimate(frog[pl]->actor,FROG_ANIM_HOPLEFT,NO,NO,frogAnimSpeed2,0);
			break;

		case 3:
			actorAnimate(frog[pl]->actor,FROG_ANIM_HOPRIGHT,NO,NO,frogAnimSpeed2,0);
			break;

		default:
			actorAnimate(frog[pl]->actor,FROG_ANIM_HOP,NO,NO,frogAnimSpeed,0);
			break;
		}
	}
}

/*	--------------------------------------------------------------------------------
	Function		: MoveToRequestedDestination
	Purpose			: Moves a player in the requested destination
	Parameters		: direction, player
	Returns			: TRUE if we did a hop, FALSE otherwise
*/
long speedFrameCount = 0;

BOOL MoveToRequestedDestination(int dir,long pl)
{
	GAMETILE *dest, *from;
	fixed t,h,height;
	unsigned long tiledir;
	PLATFORM *plat; int n;
	FVECTOR v, w;

/*	TODO: check if this is actually necessary

//	if( tongue[pl].flags & TONGUE_BEINGUSED)
//		RemoveFrogTongue(pl);

	// see if frog is currently on a moving platform (i.e. platform that follows a path)
	if(currPlatform[pl])
	{
		// frog is on a platform - get current tile
		currTile[pl] = currPlatform[pl]->inTile[0];
		currPlatform[pl]->carrying = frog[pl];

		// check if the platform follows a path
		if(currPlatform[pl]->flags & PLATFORM_NEW_FOLLOWPATH)
			player[pl].frogState |= FROGSTATUS_ISONMOVINGPLATFORM;
	}
*/
	// clear all movement flags
	player[pl].frogState &= ~(FROGSTATUS_ALLHOPFLAGS | FROGSTATUS_ISJUMPINGTOTILE | FROGSTATUS_ISJUMPINGTOPLATFORM);
	
	OrientateSS( &frog[pl]->actor->qRot, &currTile[pl]->dirVector[frogFacing[pl]], &currTile[pl]->normal );

	tiledir = dir;
	dest = GetNextTile(&tiledir, pl);
	

	if (!dest || player[pl].isSinking )
	{
		player[pl].canJump = 1;
		player[pl].isSuperHopping = 0;
		
		return FALSE;
	}

	// check destination tile
	if(GameTileTooHigh(dest, pl) && gameState.mode != FRONTEND_MODE)
	{			
		// gametile is too high
		player[pl].canJump = 1;
		player[pl].isSuperHopping = 0;
		return FALSE;
	}
	else if(dest->state == TILESTATE_SMASH)
	{
		// smash tiles

		if(!(player[pl].frogState & FROGSTATUS_ISSUPERHOPPING))
		{
			player[pl].canJump = 1;
			player[pl].isSuperHopping = 0;
			return FALSE;
		}
		else
		{
			dest->state = TILESTATE_NORMAL;
			player[pl].canJump = 1;
			player[pl].isSuperHopping = 0;
			return FALSE;
		}
	}

	// ------------------------------------------------------------------------------------------
	// If we get this far, it's a valid jump to a tile

	// clear all movement flags
	player[pl].frogState &= ~(FROGSTATUS_ALLHOPFLAGS | FROGSTATUS_ISJUMPINGTOTILE | FROGSTATUS_ISJUMPINGTOPLATFORM);

	from = currTile[pl];
	destPlatform[pl] = NULL;
	player[pl].hasJumped = 1;

	// Platform tests

	for (n = platformList.count, plat = platformList.head.next; n; n--, plat = plat->next)
	{
		if (!plat->active) continue;

		if (plat->inTile[0] == dest)
		{
			SubVectorFSS(&w, &plat->pltActor->actor->position, &frog[pl]->actor->position);
			height = DotProductFF(&w, &dest->normal);

			if (plat->flags & PLATFORM_NEW_NOWALKUNDER)
			{
				if (height > (player[pl].isSuperHopping ? MAX_SUPERHOP_HEIGHT : MAX_HOP_HEIGHT))
				{
					player[pl].canJump = 1;
					player[pl].isSuperHopping = 0;
					return FALSE;
				}
			}

			if (!player[pl].isSuperHopping && height <= MAX_HOP_HEIGHT &&
				plat->path->nodes[plat->path->toNode].offset >= 0)
			{
				destPlatform[pl] = plat;
				break;
			}
		}
		else
		if (!player[pl].isSuperHopping && currPlatform[pl])	// platform->platform hopping
		{
			fixed dist;
			// find roughly where we're trying to hop to by scaling the direction vector
			// by the size of a normal tile

			SetVectorFF(&v, &currTile[pl]->dirVector[tiledir]);
			ScaleVector(&v, -500);	// ~ one tile
			AddToVectorFS(&v, &frog[pl]->actor->position);

			// 400 is some completely arbitrary value that seems to work

			dist = DistanceBetweenPointsFS(&v, &plat->pltActor->actor->position);

			if ((dist < (400*4096)) && (plat->path->nodes[plat->path->toNode].offset >= 0))
			{
				SubVectorFSS(&w, &plat->pltActor->actor->position, &frog[pl]->actor->position);
				height = DotProductFF(&w, &dest->normal);

				// Check we're not trying to jump too high!
				if (height <= MAX_HOP_HEIGHT)
				{
					if (!player[pl].isSuperHopping)
						destPlatform[pl] = plat;
				}
				break;
			}
		}
	}

	destTile[pl] = dest;

	if (currPlatform[pl])
	{
	//Matt - PSX fix, as in single player frogon is not -1
		if((player[pl].frogon == -1) || (NUM_FROGS==1) )
		{
			currPlatform[pl]->carrying = NULL;
			currPlatform[pl]->flags &= ~PLATFORM_NEW_CARRYINGFROG;
		}
		else
		{
			currPlatform[pl]->carrying = frog[player[pl].frogon];
			currPlatform[player[pl].frogon] = currPlatform[pl];
		}

		currPlatform[pl] = NULL;
	}

	player[pl].frogState |= (destPlatform[pl]) ? FROGSTATUS_ISJUMPINGTOPLATFORM : FROGSTATUS_ISJUMPINGTOTILE;

	nextFrogFacing[pl] = GetTilesMatchingDirection(from, frogFacing[pl], dest);

	if( gameState.multi != SINGLEPLAYER && multiplayerMode == MULTIMODE_BATTLE )
 	{
		if(player[pl].quickhop.time)
			t = standardHopFrames;
		else if(player[pl].slowhop.time)
			t = superHopFrames;
		else
	 		t = battleHopFrames;

 		h = hopHeight;
 	}
	else if(player[pl].isSuperHopping)
	{
		if( player[pl].quickhop.time )
			t = quickSuperFrames;
		else
			t = superHopFrames;

		h = superhopHeight;
	}
	else if(player[pl].quickhop.time)
	{
		t = quickHopFrames;
		h = hopHeight;
	}
	else if(player[pl].slowhop.time)
	{
		t = slowHopFrames;
		h = hopHeight;
	}
	else
	{
		t = standardHopFrames;
		h = hopHeight;
	}

	// determine where frog is going to land...

    if(player[pl].frogState & FROGSTATUS_ISJUMPINGTOPLATFORM)
	{
		CalculateFrogJumpS(
			&frog[pl]->actor->position, &destPlatform[pl]->pltActor->actor->position, &currTile[pl]->normal,
			h, t, pl);
	}
	else
	{	
		CalculateFrogJumpS(
			&frog[pl]->actor->position, &destTile[pl]->centre, &currTile[pl]->normal,
			h, t, pl);
	}

	// ------------------------------------------------------------------------------------------

	numHops_TOTAL++;
	if(speedFrameCount)
		speedHops_TOTAL += frameCount - speedFrameCount;	
	speedFrameCount = frameCount;

	return TRUE;
}

/*	--------------------------------------------------------------------------------
	Function		: CheckForFroggerLanding
	Purpose			: determine whether the player has finished its jump
	Parameters		: int
	Returns			: void
	Info			:
*/
void CheckForFroggerLanding(long pl)
{
	fixed overrun;
	GAMETILE *tile=NULL;

	if (player[pl].jumpTime < 4096 || (player[pl].deathBy == DEATHBY_FALLINGFOREVER)) return;	// we haven't landed yet.

	// in case we have to keep jumping, keep track of how far past the tile we've jumped this frame
	overrun = player[pl].jumpTime;

	player[pl].canJump = 1;
	player[pl].isSuperHopping = 0;
	player[pl].hasDoubleJumped = 0;
	player[pl].jumpTime = ToFixed(-1);
	//utilPrintf("jumpTime set to %d by ToFixed(-1) in CheckForFroggerLanding\n", player[pl].jumpTime);
	doubleQueue[pl] = 0;

#ifdef PC_VERSION
		if( !rHardware ) frog[pl]->actor->shadow->draw = 0;
#else
		frog[pl]->actor->shadow->draw = 0;
#endif

	// Assume for now that if we've landed after being on another frogs head then we're no longer on it.
	if( gameState.multi == MULTILOCAL && multiplayerMode != MULTIMODE_BATTLE )
	{
		if( player[pl].frogon != -1 )
		{
			int f = player[pl].frogon;

	//bb??		GTInit( &player[f].stun, 1.5 );
			if(player[f].frogunder == pl)
			{
				player[f].frogunder = -1;
				GTInit( &player[f].stun, 1 );
				player[f].canJump = 1;
			}

			
				
				
			player[pl].frogon = -1;
//			player[pl].frogunder = -1;

			player[pl].idleEnable = 1;

			if(player[f].frogon == -1)
			{
				if(player[f].frogunder == -1)
				{
					player[f].idleEnable = 1;
					actorAnimate( frog[f]->actor, FROG_ANIM_BREATHE, YES, NO, FROG_BREATHE_SPEED, 0);
				}
			}
			else
			{
//				frog[f]->actor->

				actorAnimate( frog[f]->actor, FROG_ANIM_PIN, YES, NO, 128, 0);
			}
		}
	}

	if (currPlatform[pl])
	{
		currPlatform[pl]->carrying = NULL;
		currPlatform[pl] = NULL;
	}

	// Finish anims when floating
	if((player[pl].frogState & FROGSTATUS_ISFLOATING) || ((gameState.multi != SINGLEPLAYER) && (frog[pl]->actor->animation.currentAnimation == FROG_ANIM_FALL)))
	{
		if((frogPool[player[pl].character].anim) && (gameState.multi == SINGLEPLAYER))
		{
			actorAnimate(frog[0]->actor, FROG_ANIM_FALLLAND, NO, NO, 128, 0);
			actorAnimate(frog[pl]->actor,FROG_ANIM_BREATHE,YES,YES,FROG_BREATHE_SPEED,0);
		}
		else
			actorAnimate(frog[pl]->actor,FROG_ANIM_BREATHE,YES,NO,FROG_BREATHE_SPEED,0);
	}
/*
	else
	{
		actorAnimate(frog[pl]->actor,FROG_ANIM_BREATHE,YES,YES,154,0);
	}
/* 	else if( player[pl].deathBy == DEATHBY_WHACKING )
 	{
 		FVECTOR up, fwd, side;
 
 		// Find the forward and up vectors for camera
		SubVectorFFF( &fwd, &currCamSource, &currCamTarget );
 		MakeUnit( &fwd );
 
 		CrossProductFFF( &side, &fwd, &upVec );
 		CrossProductFFF( &up, &side, &fwd );
 		MakeUnit( &up );
 
 		// Orientate to camera
 		Orientate( &frog[pl]->actor->qRot, &fwd, &up );
 	}
*/
	if (player[pl].deathBy == DEATHBY_WHACKING)
	{
		FVECTOR pos = { 0, 0, 0 };
		fixed mult;

		// Do some simplified frog jump maths to stick Frogger to the screen

		if (player[pl].jumpMultiplier)
		{
			mult = (2*player[pl].jumpMultiplier - 4096);
			mult = 4096 - ((mult*mult)>>12);

			SetVectorFF(&pos, &player[pl].jumpUpVector);
			ScaleVectorFF(&pos, mult);
		}

		AddToVectorFS(&pos, &player[pl].jumpOrigin);
		AddToVectorFF(&pos, &player[pl].jumpFwdVector);

		SetVectorSF(&frog[pl]->actor->position, &pos);
	}
	else
	if(player[pl].frogState & FROGSTATUS_ISJUMPINGTOPLATFORM)
	{
		GAMETILE *tile = destPlatform[pl]->inTile[0];

		// ok - frog has landed
		SetVectorSS(&frog[pl]->actor->position, &destPlatform[pl]->pltActor->actor->position);

		frogFacing[pl] = GetTilesMatchingDirection(currTile[pl], frogFacing[pl], tile);
		
		destPlatform[pl]->flags		|= PLATFORM_NEW_CARRYINGFROG;
		player[pl].frogState		|= FROGSTATUS_ISONMOVINGPLATFORM;

		destPlatform[pl]->carrying	= frog[pl];
		player[pl].frogState &= ~(	FROGSTATUS_ISJUMPINGTOTILE | FROGSTATUS_ISFLOATING | FROGSTATUS_ISSLIDING |
									FROGSTATUS_ISJUMPINGTOPLATFORM | FROGSTATUS_ISSUPERHOPPING | FROGSTATUS_ISSAFE);

		currPlatform[pl] = destPlatform[pl];
		currTile[pl] = tile;
		destPlatform[pl] = NULL;

		if(player[pl].heightJumped < -DROP_KILL_HEIGHT)
		{
			if(!player[pl].dead.time)
			{
				if( gameState.multi != SINGLEPLAYER )
					KillMPFrog(pl);
				else
				{
					FVECTOR n;
					SetVectorFF(&n, &destTile[pl]->normal);
					CreateSpecialEffect( FXTYPE_DECAL, &destTile[pl]->centre, &n, 102400, 4096, 410, 3277 );
					player[pl].deathBy = DEATHBY_NORMAL;
					actorAnimate(frog[pl]->actor,FROG_ANIM_SPLATFRAME,NO,NO,64,0);

					player[pl].frogState |= FROGSTATUS_ISDEAD;
					GTInit( &player[pl].dead, 3 );
					frog[pl]->actor->shadow->draw = 0;
				}

				PlaySample(genSfx[GEN_DEATHFALL],NULL,0,SAMPLE_VOLUME,-1);
			}
		}
		else
			actorAnimate(frog[pl]->actor,FROG_ANIM_BREATHE,YES,YES,FROG_BREATHE_SPEED,0);

		CheckTileForCollectable(tile,pl);
	}
	else /*if (player[pl].frogState & FROGSTATUS_ISJUMPINGTOTILE)*/
	{
		int state;

		if (!destTile[pl])
			tile = currTile[pl];
		else
		{
			if( currTile[pl]->state >= TILESTATE_CONVEYOR_ONEWAY && destTile[pl]->state < TILESTATE_CONVEYOR_ONEWAY )
			{
				FVECTOR fwd;
				SVECTOR dir;
				long cam = GetTilesMatchingDirection( currTile[pl], camFacing[pl], destTile[pl] );

				actorAnimate( frog[pl]->actor, FROG_ANIM_BREATHE, YES, NO, FROG_BREATHE_SPEED, 0 );

				SubVectorFSS( &fwd, &destTile[pl]->centre, &currTile[pl]->centre );
				MakeUnit( &fwd );
				SetVectorFF( &dir, &fwd );
				frogFacing[pl] = nextFrogFacing[pl] = GetTilesMatchingVector( &dir, destTile[pl] );

				player[pl].extendedHopDir = (frogFacing[pl] - cam)&3;
			}

			currTile[pl] = tile = destTile[pl];
			destTile[pl] = NULL;
		}

		state = tile->state;

		//frogFacing[pl] = GetTilesMatchingDirection(currTile[pl], frogFacing[pl], tile);

		if( player[pl].deathBy != DEATHBY_WHACKING )
		{
			frogFacing[pl] = nextFrogFacing[pl];
			OrientateSS( &frog[pl]->actor->qRot, &tile->dirVector[frogFacing[pl]], &tile->normal );

			// DON'T set frog to centre of tile - ds
			if( (state != TILESTATE_FALL) /*&& (state < TILESTATE_CONVEYOR)*/ )
				SetVectorSS(&frog[pl]->actor->position, &tile->centre);
		}

		player[pl].frogState &= ~(FROGSTATUS_ISJUMPINGTOTILE | FROGSTATUS_ISJUMPINGTOPLATFORM | FROGSTATUS_ISSLIDING |
			FROGSTATUS_ISONMOVINGPLATFORM | FROGSTATUS_ISSUPERHOPPING | FROGSTATUS_ISFLOATING | FROGSTATUS_ISSAFE);

		if( player[pl].frogState & FROGSTATUS_ISDEAD )
		{
			// We're bouncing along the ground
 			if( player[pl].deathBy == DEATHBY_FIRE )
 			{
 				BounceFrog( pl, 2048000, 25 );
 				actorAnimate( frog[pl]->actor, FROG_ANIM_ASSONFIRE, NO, NO, 128, 0);
 			}
		}
		else
		{
			// check tile to see if frog has jumped onto a certain tile type
			CheckTileState(tile, pl);

			if(player[pl].heightJumped < -DROP_KILL_HEIGHT || state == TILESTATE_DEADLYFALL)
			{
				if(!player[pl].dead.time)
				{
					if( gameState.multi != SINGLEPLAYER )
						KillMPFrog(pl);
					else
					{
						FVECTOR n;
						SetVectorFF(&n, &tile->normal);
						CreateSpecialEffect( FXTYPE_DECAL, &tile->centre, &n, 102400, 4096, 410, 3277 );
						player[pl].deathBy = DEATHBY_NORMAL;
						actorAnimate(frog[pl]->actor,FROG_ANIM_SPLATFRAME,NO,NO,256,0);

						player[pl].frogState |= FROGSTATUS_ISDEAD;
						GTInit( &player[pl].dead, 3 );
						frog[pl]->actor->shadow->draw = 0;
					}

					PlaySample(genSfx[GEN_DEATHFALL],NULL,0,SAMPLE_VOLUME,-1);
				}
			}
			if (state == TILESTATE_FALL)
			{
				player[pl].jumpTime = overrun;
				utilPrintf("jumpTime set to %d by TILESTATE_FAIL condition in CheckForFroggerLanding\n", player[pl].jumpTime);
			}
//			else if (destTile[pl] && (state & TILESTATE_CONVEYOR || state == TILESTATE_ICE))
//				player[pl].jumpTime = overrun-4096;
		}

		// Next, check if frog has landed on a collectable
		CheckTileForCollectable(tile, pl);
	}

	CheckForFrogOn(pl,tile);
}

void CheckForFrogOn(int pl,GAMETILE *tile)
{
	int i;

	if(((player[pl].frogunder == -1)/* || (player[player[pl].frogunder].canJump == 0)*/) && (gameState.multi == MULTILOCAL) && (multiplayerMode != MULTIMODE_BATTLE))
	{
		// If we've landed on another frog, sit on his head
		for( i=(pl+1)%NUM_FROGS; i!=pl; i=(i+1)%NUM_FROGS )
			if( (tile && currTile[i] == tile) || (currPlatform[i] && currPlatform[i] == currPlatform[pl]) )
			{
				if(((destTile[i] != tile) && (destTile[i]) && (tile)) || (player[pl].frogState & FROGSTATUS_ISDEAD) || (DistanceBetweenPointsSS( &frog[pl]->actor->position, &frog[i]->actor->position ) > ToFixed(20*SCALE)))
					continue;

				// Face all lower frogs to our direction
				nextFrogFacing[i] = frogFacing[i] = frogFacing[pl];
				SetQuaternion( &frog[i]->actor->qRot, &frog[pl]->actor->qRot );
				OrientateSS( &frog[i]->actor->qRot, &currTile[i]->dirVector[frogFacing[i]], &currTile[i]->normal );

				if((player[i].frogunder != -1) && ((player[player[i].frogunder].canJump) || (player[player[i].frogunder].frogunder != -1)))
//				if((player[i].frogunder != -1) && (player[player[i].frogunder].jumpTime < 0))
					continue;

				PlaySample(genSfx[GEN_DEATHCRUSH], NULL, 0, SAMPLE_VOLUME, -1 );

				utilPrintf("PLAYER %d PINNING PLAYER %d : CAN JUMP: %d\n",pl,i,player[i].canJump);
				player[i].canJump = 0;

				player[pl].frogon = i;
				player[i].frogunder = pl;

				player[i].frogState &= ~FROGSTATUS_ALLHOPFLAGS;
				player[pl].idleEnable = 0;
				player[i].idleEnable = 0;

				actorAnimate( frog[pl]->actor, FROG_ANIM_PIN, YES, NO, 128, 0);

				if( player[i].frogon != -1 )
				{
					FVECTOR up;
					SetVectorFF(&up, &currTile[i]->normal);
					ScaleVector( &up, 10 );
					AddVectorSSF( &frog[i]->actor->position, &frog[player[i].frogon]->actor->position, &up );
				}

				SetVectorSS(&frog[pl]->actor->position, &frog[i]->actor->position);
				actorAnimate( frog[i]->actor, FROG_ANIM_PINNED, YES, NO, 128, 0);

				if( currPlatform[i] && currPlatform[i] == currPlatform[pl] )
					currPlatform[i]->carrying = frog[i];
			}
	}
}

/*	--------------------------------------------------------------------------------
	Function		: CheckTileState
	Purpose			: when landing on a tile, check what state it is and do stuff
	Parameters		: int
	Returns			: void
	Info			: 
*/
void CheckTileState(GAMETILE *tile, int pl)
{
	int state = tile->state;

	if (state & TILESTATE_CONVEYOR)
	{	
		// -------------------------------- Conveyors ----------------------------

 		int res = MoveToRequestedDestination( ((state & (TILESTATE_CONVEYOR-1)) - camFacing[pl]) & 3, pl );

		if (res)
		{
			FVECTOR fwd;
			int speed;

			if (state >= TILESTATE_CONVEYOR_ONEWAY)
			{
				speed = 2;
				StartAnimateActor(frog[pl]->actor, FROG_ANIM_ICE2, YES, NO, 64, NO);

				if( lastTile[pl]->state < TILESTATE_CONVEYOR_ONEWAY )
					PlayVoice( pl, "frogslide" );

				player[pl].canJump = FALSE;
			}
			else
			{
				if (state >= TILESTATE_CONVEYOR_FAST)
					speed = 2;
				else if (state >= TILESTATE_CONVEYOR_MED)
					speed = 1;
				else
					speed = 0;

				StartAnimateActor(frog[pl]->actor,FROG_ANIM_BREATHE,YES,YES,FROG_BREATHE_SPEED,0);
				//StartAnimateActor(frog[pl]->actor,FROG_ANIM_BREATHE,YES,YES,154,0);
			}

			if (player[pl].frogState & FROGSTATUS_ISJUMPINGTOPLATFORM)
			{
				CalculateFrogJumpS( &frog[pl]->actor->position, &destPlatform[pl]->pltActor->actor->position, &tile->normal, -1, conveyorFrames[speed], pl);
				SubVectorFSS( &fwd, &destPlatform[pl]->pltActor->actor->position, &frog[pl]->actor->position );
			}
			else
			{
				CalculateFrogJumpS( &frog[pl]->actor->position, &destTile[pl]->centre, &tile->normal, -1, conveyorFrames[speed], pl);
				SubVectorFSS( &fwd, &destTile[pl]->centre, &frog[pl]->actor->position );
			}

			player[pl].frogState |= FROGSTATUS_ISSLIDING;

			if (state >= TILESTATE_CONVEYOR_ONEWAY)
			{
				MakeUnit( &fwd );
				OrientateFS( &frog[pl]->actor->qRot, &fwd, &currTile[pl]->normal );
			}
		}
	}
	else switch (state)
	{
	case TILESTATE_DEADLY:
	{
		if(!player[pl].dead.time)
		{
			SPECFX *fx;
			FVECTOR n;
			SetVectorFF(&n, &currTile[pl]->normal);

			CreateSpecialEffect( FXTYPE_WATERRIPPLE, &tile->centre, &n, 81920, 3277, 410, 2458 );
			if( (fx = CreateSpecialEffect( FXTYPE_SPLASH, &frog[pl]->actor->position, &n, 40960, 16384, 0, 8192 )) )
			{
				fx->gravity = 8190;
				if( (fx->rebound = AllocateP2()) )
				{
					SetVectorFF( &fx->rebound->normal, &currTile[pl]->normal );
					SetVectorFF( &fx->rebound->point, &frog[pl]->actor->position );
				}
			}

			// If single player
			if( gameState.multi == SINGLEPLAYER )
			{
				if( frogPool[player[pl].character].anim )
					DeathDrowning( pl );
				else
					DeathMulti( pl );
			}
			else // multiplayer mode
			{
				KillMPFrog(pl);
			}
		}
		break;
	}

	case TILESTATE_PANTS:
	{
		if(!player[pl].dead.time)
		{
			if( gameState.multi == SINGLEPLAYER )
			{
				player[pl].frogState |= FROGSTATUS_ISDEAD;
				DeathPoison(pl);
			}
			else
				KillMPFrog( pl );
		}
		break;
	}
	
	case TILESTATE_FALL:
	{
		if (!player[pl].dead.time)
		{
			if( gameState.multi == SINGLEPLAYER )
			{
				player[pl].deathBy = DEATHBY_FALLINGFOREVER;
				player[pl].frogState |= FROGSTATUS_ISDEAD;
				GTInit( &player[pl].dead, 2 );

				if( frog[pl]->actor->shadow ) frog[pl]->actor->shadow->draw = 0;
				
				//player[pl].jumpTime = jump_overrun;
				player[pl].jumpSpeed = 75;

				// DOn't follow frog cos it exposes nasty things
				fixedPos = 1;
				fixedDir = 1;
				SetVectorFF(&camSource, &currCamSource);
			}
			else
				KillMPFrog(pl);

			PlaySample(genSfx[GEN_DEATHFALL],NULL,0,SAMPLE_VOLUME,-1);
		}
		break;
	}
	
	case TILESTATE_NOSUPERHOT:
	case TILESTATE_HOT:
	{
		if( gameState.multi != SINGLEPLAYER )
			KillMPFrog(pl);
		else
		{
			player[pl].frogState |= FROGSTATUS_ISDEAD;
			if (Random(2)) DeathFire(pl); else DeathIncinerate(pl);
		}
		break;
	}

	case TILESTATE_ELECTRIC:
	{
		if( gameState.multi != SINGLEPLAYER )
			KillMPFrog(pl);
		else
		{
			player[pl].frogState |= FROGSTATUS_ISDEAD;
			DeathElectric(pl);
		}
		break;
	}

	case TILESTATE_SINK:
	{
		player[pl].isSinking = 1;
		break;
	}

	case TILESTATE_ICE:
	{
		FVECTOR fwd;
		int res = MoveToRequestedDestination((frogFacing[pl] - camFacing[pl]) & 3, pl);

		if (res)
		{
			if (player[pl].frogState & FROGSTATUS_ISJUMPINGTOPLATFORM)
			{
				CalculateFrogJumpS( &frog[pl]->actor->position, &destPlatform[pl]->pltActor->actor->position, &tile->normal, -1, iceFrames, pl);
				SubVectorFSS( &fwd, &destPlatform[pl]->pltActor->actor->position, &frog[pl]->actor->position );
			}
			else
			{
				CalculateFrogJumpS( &frog[pl]->actor->position, &destTile[pl]->centre, &tile->normal, -1, iceFrames, pl);
				SubVectorFSS( &fwd, &destTile[pl]->centre, &frog[pl]->actor->position );
			}

			StartAnimateActor(frog[pl]->actor, FROG_ANIM_ICETUBE, YES, NO, 64, NO);

			if( lastTile[pl]->state != TILESTATE_ICE )
				PlayVoice( pl, "frogslide" );

			player[pl].canJump = FALSE;
			player[pl].frogState |= FROGSTATUS_ISSLIDING;

			MakeUnit( &fwd );
			OrientateFS( &frog[pl]->actor->qRot, &fwd, &currTile[pl]->normal );
		}
		break;
	}

	default:
		// thank heavens, we've landed on a normal tile!
		actorAnimate(frog[pl]->actor,FROG_ANIM_BREATHE,YES,YES,FROG_BREATHE_SPEED,0);
		break;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: GameTileTooHigh
	Purpose			: checks is destination tile is too high to jump to
	Parameters		: GAMETILE *
	Returns			: BOOL
	Info			: 
*/
BOOL GameTileTooHigh(GAMETILE *tile,long pl)
{
//	VECTOR vec;
	fixed height;//,h;
	FVECTOR diff;
		
	SubVectorFSS(&diff,&tile->centre,&frog[pl]->actor->position);
	height = DotProductFF(&diff,&currTile[pl]->normal);

	return
		height > (player[pl].isSuperHopping ? MAX_SUPERHOP_HEIGHT : MAX_HOP_HEIGHT);
}

/*	--------------------------------------------------------------------------------
	Function		: KillFrog
	Purpose			: kills the specified frog - runs the relevant death sequence
	Parameters		: long
	Returns			: BOOL - true if running death 'sequence'
	Info			: 
*/
BOOL KillFrog(long pl)
{
	RemoveFrogTongue(pl);
	
	GTUpdate( &player[pl].dead, -1 );
	if( !player[pl].dead.time )
	{
		numHealth_TOTAL++;

		// If substitute frog model was used then swap them back here
		if( UndoChangeModel( frog[pl]->actor ) )
			player[pl].idleEnable = 1;

#ifndef E3_DEMO
		if((gameState.mode == INGAME_MODE ) && (cheatCombos[CHEAT_INFINITE_LIVES].state == 0))
		{
			if( (--player[pl].lives) <= 0 )
			{
				StartGameOver();
				return FALSE;
			}
		}
#endif

		player[pl].frogState &= ~FROGSTATUS_ISDEAD;

		SetFroggerStartPos(gTStart[0],pl);
		GTInit(&player[0].safe,2);
		InitCamera();

		PlayVoice( pl, "frogletsgo" );

		ResetPlatformFlags();

		return FALSE;
	}

	switch(player[pl].deathBy)
	{
		case DEATHBY_NORMAL:
			break;

		case DEATHBY_RUNOVER:
			break;

		case DEATHBY_DROWNING:
			if( !(actFrameCount % 5) && actFrameCount != lastPlayerAnimCooldown)
			{
				VECTOR up;
				SPECFX *fx;
				FVECTOR n;

				SetVectorFF(&n, &currTile[pl]->normal);
				fx = CreateSpecialEffect( FXTYPE_BUBBLES, &frog[pl]->actor->position, &n, 32768, 3277, 0, 2458 );

				if( fx )
				{
					SetVectorFF( &up, &currTile[pl]->normal );
					if( (fx->rebound = AllocateP2()) )
					{
						SetVectorFF( &fx->rebound->normal, &up );
						ScaleVector( &up, 30 );
						AddVectorSSF( &fx->rebound->point, &frog[pl]->actor->position, &up );
					}
				}

				lastPlayerAnimCooldown = actFrameCount;
			}
			break;

		case DEATHBY_SQUASHED:
			break;

		case DEATHBY_FIRE:
			if( !(actFrameCount % 2)  && lastPlayerAnimCooldown != actFrameCount)
			{
				FVECTOR n;
				SetVectorFF(&n, &currTile[pl]->normal);
				CreateSpecialEffect( FXTYPE_FIERYSMOKE, &frog[pl]->actor->position, &n, 204800, 2048, 0, 6144 );

				lastPlayerAnimCooldown = actFrameCount;
			}
			break;

		case DEATHBY_ELECTRIC:
			frog[pl]->actor->position.vx += (-2048 + (Random(2)<<12))>>12;
			frog[pl]->actor->position.vy += (-2048 + (Random(2)<<12))>>12;
			frog[pl]->actor->position.vz += (-2048 + (Random(2)<<12))>>12;

			break;
	}

	return TRUE;
}


/*	--------------------------------------------------------------------------------
	Function		: HopFrogToTile
	Purpose			: Hop the frog to a tile
	Parameters		: 
	Returns			: void
*/

void HopFrogToTile(GAMETILE *tile, long pl)
{
	FVECTOR v;
	fixed maxdot = ToFixed(0), dot;
	int j, dir = 0;

	player[pl].frogState |= FROGSTATUS_ISJUMPINGTOTILE;
	player[pl].canJump = 0;
	destTile[pl] = tile;

	SubVectorFSS(&v, &currTile[pl]->centre, &tile->centre);

	for(j=0; j<4; j++)
	{	
		dot = DotProductFF(&currTile[pl]->dirVector[j], &v);
		if(dot > maxdot)
		{
			maxdot = dot;
			dir = j; //(j - camFacing + 2) & 3;
		}
	}

	AnimateFrogHop(dir, pl);	
	frogFacing[pl] = nextFrogFacing[pl] = (dir + 2) & 3;

	OrientateSS( &frog[pl]->actor->qRot, &currTile[pl]->dirVector[frogFacing[pl]], &currTile[pl]->normal );

	CalculateFrogJumpS(
		&frog[pl]->actor->position, &destTile[pl]->centre, &currTile[pl]->normal,
		hopHeight, standardHopFrames, pl);
}

/*	--------------------------------------------------------------------------------
	Function		: HopFrogToTile
	Purpose			: Hop the frog to a tile
	Parameters		: 
	Returns			: void
*/

void SpringFrogToTile(GAMETILE *tile, fixed height, fixed time, long pl)
{
	long t = (time*60)>>12;

	player[pl].frogState |= FROGSTATUS_ISJUMPINGTOTILE;
	player[pl].canJump = 0;
	//destTile[pl] = tile;
	if (currPlatform[pl])
	{
		currPlatform[pl]->carrying = NULL;
		currPlatform[pl]->flags &= ~PLATFORM_NEW_CARRYINGFROG;
		currPlatform[pl] = NULL;
	}
	destTile[pl] = tile; //Why does this get assigned twice?
	

	CalculateFrogJumpS(
		&frog[pl]->actor->position, &destTile[pl]->centre, &currTile[pl]->normal,
		height, t, pl);
}

/*	--------------------------------------------------------------------------------
	Function		: PushFrog
	Purpose			: Push the frog in a given direction
	Parameters		: VECTOR, VECTOR, long
	Returns			: void
*/
void PushFrog(FVECTOR *direction, long pl)
{
//	GAMETILE *tile;
	
	if( currPlatform[pl] )
	{
		fixed distance = -100000, t;
		int i, res;

		for (i=0; i<4; i++)
		{
			t = DotProductFF( direction, &currTile[pl]->dirVector[i] );
			if( t > distance )
			{
				distance = t;
				res = i;
			}
		}

		MoveToRequestedDestination((res - camFacing[pl]) & 3, pl);
	}

	if (player[pl].frogState & FROGSTATUS_ISJUMPINGTOPLATFORM)
	{
		CalculateFrogJumpS( &frog[pl]->actor->position, &destPlatform[pl]->pltActor->actor->position, &currTile[pl]->normal,	819200, standardHopFrames, pl );
		currPlatform[pl] = destPlatform[pl];
	}
	else
	{
		fixed distance = -100000, t;
		int i, res;

		for (i=0; i<4; i++)
		{
			t = DotProductFF( direction, &currTile[pl]->dirVector[i] );
			if( t > distance )
			{
				distance = t;
				res = i;
			}
		}

		MoveToRequestedDestination((res - camFacing[pl]) & 3, pl);
	}

	if( destTile[pl] )
		currTile[pl] = destTile[pl];
	else
	{
		player[pl].frogState |= FROGSTATUS_ISDEAD;
		DeathNormal( pl );
	}
}

/*	--------------------------------------------------------------------------------
	Function		: ThrowFrogAtScreen
	Purpose			: Throw the frog at the screen
	Parameters		: 
	Returns			: void
*/

#define THROWFROG_FRAMES	12
#ifdef PSX_VERSION
fixed screenDist = 1750000;
#else
fixed screenDist = 2200000;
#endif
void ThrowFrogAtScreen(long pl)
{
/*#ifdef PSX_VERSION
	actorAnimate( frog[0]->actor, 31, NO, NO, 64, 0 );
#else*/

	SVECTOR target;
	FVECTOR v, w, up, cross;

	if( destTile[pl] )
	{
		currTile[pl] = destTile[pl];
		destTile[pl] = NULL;
	}

	player[pl].frogState &= ~(FROGSTATUS_ALLHOPFLAGS|FROGSTATUS_ISFLOATING);
	SetVectorSS( &frog[pl]->actor->position, &currTile[pl]->centre );

	// Calculate the point to throw the frog to
	SubVectorFFF(&v, &currCamSource, &currCamTarget);	// v points from screen target to screen
	MakeUnit( &v );

	SetVectorFF(&w, &v);
	ScaleVectorFF(&w,-screenDist);
	AddVectorSFF(&target, &w, &currCamSource);

	SetVectorFF( &cross, &currTile[pl]->dirVector[(camFacing[pl]+1)&3] );
	CrossProductFFF( &up, &cross, &v );
	MakeUnit( &up );
	Orientate( &frog[pl]->actor->qRot, &v, &up );

	SetVectorFF(&v, &up);
	ScaleVector(&v, -150);
	AddToVectorSF(&target, &v);

	CalculateFrogJumpS(&frog[pl]->actor->position, &target, &currTile[pl]->normal, 0, 80, pl);

	frogFacing[pl] = (camFacing[pl]+2)&3;

	camSource = currCamSource;
	camTarget = currCamTarget;
	fixedPos = 1;
	fixedDir = 1;
	controlCamera = 1;

	player[pl].idleEnable = 0;
//#endif
}


/*	--------------------------------------------------------------------------------
	Function		: CalculateFrogJump
	Purpose			: Calculates jump curve variables
	Parameters		: VECTOR*, VECTOR*, VECTOR*, fixed, fixed, long
	Returns			: void
*/
void CalculateFrogJump(SVECTOR *startPos, SVECTOR *endPos, FVECTOR *normal, fixed height, long time, long p)
{
	FVECTOR F, V, Q;
	fixed diff, m;
	PLAYER *pl = &player[p];

	SetVectorSS(&pl->jumpOrigin, startPos);
	SubVectorFSS(&V, endPos, startPos);
	
	if (height<0)
	{
		// just *slide*, force linear movement with no curve
		SetVectorFF(&pl->jumpFwdVector, &V);
		pl->jumpMultiplier = 0;
		pl->heightJumped = 0;
		pl->jumpSpeed = 4096/time;
	}
	else
	{
		// Forward vector
		diff = DotProductFF(normal, &V);		// projection of V onto normal
		SetVectorFF(&Q, normal);
		ScaleVectorFF(&Q, diff);
		
		SubVectorFFF(&F, &V, &Q);			// F = V - N(N.V)
		SetVectorFF(&pl->jumpFwdVector, &F);

		if(diff > 0)
			height += diff;		// When we're jumping UP, add height

		if(height)
		{
			// Up vector
			SetVectorFF(&V, normal);
			ScaleVectorFF(&V, height);
			SetVectorFF(&pl->jumpUpVector, &V);


	// 		m = 0.5f * (1 + sqrtf(1 - diff/height));
	// 		pl->jumpSpeed = 1.0f/(m*(float)time); //1.0f/(float)time; 
			m = (4096 + FsqrtF(4096-FDiv(diff,height))) >> 1;
			pl->jumpSpeed = FDiv(4096, m*time);
			pl->jumpTime = 0;
		}
		else if (diff < 0)
		{
			// Up vector
			SetVectorFF(&V, normal);
			ScaleVectorFF(&V, -diff);
			SetVectorFF(&pl->jumpUpVector, &V);

			SubFromVectorSF(&pl->jumpOrigin, &V);

			m = 4096;
			pl->jumpSpeed = 2048/time;
			pl->jumpTime = 2048;
		}
		else
		{
			m = 0;	// Probably only happens on conveyors/ice, but still.
			pl->jumpSpeed = 4096/time;
			pl->jumpTime = 0;
		}

		pl->jumpMultiplier = m;
		pl->heightJumped = diff;
	}
}


void CalculateFrogJumpS(SVECTOR *startPos, SVECTOR *endPos, SVECTOR *normal, fixed height, long time, long p)
{
	FVECTOR F;

	SetVectorFF(&F, normal);
	CalculateFrogJump(startPos, endPos, &F, height, time, p);
}

/*	--------------------------------------------------------------------------------
	Function		: BounceFrog
	Purpose			: Jump in current direction
	Parameters		: player number, height, time
	Returns			: void
*/
void BounceFrog( int pl, fixed height, long time )
{
	GAMETILE *bounceTo;
	FVECTOR dir;
	SVECTOR pos;

	// Frog forward vector
	RotateVectorByQuaternionFF( &dir, &inVec, &frog[pl]->actor->qRot );

	// And a tile to bounce to
	if( !(bounceTo = FindJoinedTileByDirectionConstrained( currTile[pl], &dir, 1024 )) )
		bounceTo = currTile[pl];

	ScaleVector( &dir, 300 );
	AddToVectorFS( &dir, &frog[pl]->actor->position );

	SetVectorSF(&pos, &dir);

	CalculateFrogJumpS( &frog[pl]->actor->position, &bounceTo->centre, &currTile[pl]->normal, height, time, pl );

	destTile[pl] = bounceTo;
}
