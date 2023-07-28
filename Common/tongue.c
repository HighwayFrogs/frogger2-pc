/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: tongue.c
	Programmer	: Andrew Eder
	Date		: 12/15/98

----------------------------------------------------------------------------------------------- */

#include <islmem.h>
#include <islutil.h>
#include <islpad.h>

#include "ultra64.h"
#include "tongue.h"

#include "maths.h"
#include "frogmove.h"

#include "event.h"
#include "evenfunc.h"
#include "sprite.h"
#include "anim.h"
#include "game.h"
#include "enemies.h"
#include "platform.h"
#include "babyfrog.h"
#include "multi.h"
#include "Cam.h"
#include "map.h"
#include "newpsx.h"
#include "Main.h"
#include "layout.h"

#ifdef PSX_VERSION
#include "textures.h"
#include "audio.h"
#else
#include <pcaudio.h>
#include <pcmisc.h>
#include <mdx.h>
#endif

#define TONGUE_OFFSET_UP			150
#define TONGUE_OFFSET_FWD			100

unsigned char eatEverythingMode = 0;

TONGUE tongue[MAX_FROGS];

void StartTongue(unsigned char type, SVECTOR *dest, int pl);
void CalculateTongue( int pl );
void CalcTongueSource( int pl );
void CheckForAttachedNMEs( ENEMY *e, int flag );

// ARGB
unsigned long tongueColours[] = 
{
	0xffdd0000, // Frogger - red
	0xffdd0000, // Lilli - red
	0xffff9000, // Tad - Orange
	0xff8f00ff, // Twee - purple
	0xffaa0000, // Wart - Deep Red
	0xffbbccee, // Tank - Metallic
	0xffdd0000, // Roobie - Red
	0xff555500, // Swampy - Dark green
};

#define DEACTIVATEENEMY( e ) \
{\
	e->active = 0;\
	e->visible = 0;\
	e->nmeActor->draw = 0;\
}

#define ACTIVATEENEMY( e ) \
{\
	e->visible = 1;\
	e->active = 1;\
	SetVectorFF( &e->nmeActor->actor->size, &oneVec );\
}
/*	--------------------------------------------------------------------------------
	Function		: InitTongues
	Purpose			: Set all tongue data to nothing
	Parameters		: 
	Returns			: 
	Info			: 
*/
void InitTongues( )
{
	long i;

	for( i=0; i<4; i++ )
	{
		tongue[i].flags = TONGUE_NONE | TONGUE_IDLE;
		tongue[i].thing = NULL;
		tongue[i].radius = ToFixed(TONGUE_RADIUSNORMAL);
		tongue[i].segment = NULL;
		tongue[i].tex = NULL;
		tongue[i].type = 0;
		tongue[i].canTongue = 1;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: FreeTongues
	Purpose			: Delete allocated data
	Parameters		: 
	Returns			: 
	Info			: 
*/
void FreeTongues( )
{
	long i;

	for( i=0; i<4; i++ )
	{
		tongue[i].flags = TONGUE_NONE | TONGUE_IDLE;
		tongue[i].thing = NULL;
		tongue[i].radius = ToFixed(TONGUE_RADIUSNORMAL);

		if( tongue[i].segment ) FREE( tongue[i].segment );
		tongue[i].segment = NULL;
		tongue[i].tex = NULL;
		tongue[i].type = 0;
		tongue[i].canTongue = 1;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: StartTongue
	Purpose			: initialises Frogger's tongue for picking up, throwing, etc.
	Parameters		: char, dest, player
	Returns			: 
	Info			: contains initialisation data for shooting out the tongue
*/
void StartTongue(unsigned char type, SVECTOR *dest, int pl)
{
	fixed dp;
	FVECTOR to;
	int i, no=0;

	if( (!tongue[pl].canTongue) || (!player[pl].canJump) )
	{
		tongue[pl].flags = TONGUE_NONE | TONGUE_IDLE;
		tongue[pl].thing = NULL;
		return;
	}

	// Determine frog's forward vector
 	RotateVectorByQuaternionFF(&tongue[pl].fwd, &inVec, &frog[pl]->actor->qRot);

//	if( !(FindJoinedTileByDirectionConstrained( currTile[pl], &tongue[pl].fwd, 1024 )) )
//	{
//		tongue[pl].flags = TONGUE_NONE | TONGUE_IDLE;
//		tongue[pl].thing = NULL;
//		return;
//	}

	// Set source, including forward and up offsets
	tongue[pl].source.vx = frog[pl]->actor->position.vx + ((currTile[pl]->normal.vx * TONGUE_OFFSET_UP)>>12);
	tongue[pl].source.vy = frog[pl]->actor->position.vy + ((currTile[pl]->normal.vy * TONGUE_OFFSET_UP)>>12);
	tongue[pl].source.vz = frog[pl]->actor->position.vz + ((currTile[pl]->normal.vz * TONGUE_OFFSET_UP)>>12);

	SetVectorSS(&tongue[pl].pos, &tongue[pl].source);

	// Calculate the vector to the collectable item and its magnitude
	SetVectorSS(&tongue[pl].target, dest);

	SubVectorFSS(&to, &tongue[pl].target, &tongue[pl].source);
	MakeUnit( &to );

	tongue[pl].progress = 0;

	// Calculate angle through which the tongue needs to rotate
	dp = DotProductFF( &tongue[pl].fwd, &to );
	if( dp<TONGUE_WRAPAROUNDTHRESHOLD || no )
	{
		tongue[pl].flags = TONGUE_NONE | TONGUE_IDLE;
		tongue[pl].thing = NULL;
		return;
	}

	tongue[pl].type = type;
	tongue[pl].tex = FindTexture("TONGUE");

	if( !tongue[pl].segment )
		tongue[pl].segment = (FVECTOR *)MALLOC0( sizeof(FVECTOR)*MAX_TONGUENODES );

	for( i=0; i<MAX_TONGUENODES; i++ )
	{
		SetVectorFS( &tongue[pl].segment[i], &tongue[pl].pos );
	}

	// Set frog mouth open animation, and the speed (Swampy has a silly tongue anim - ignore it)
	if( player[pl].character != FROG_SWAMPY )
		actorAnimate(frog[pl]->actor, FROG_ANIM_TONGUE, NO, NO, 128, 0);

	tongue[pl].flags = TONGUE_NONE | TONGUE_BEINGUSED | TONGUE_OUTGOING;
	tongue[pl].canTongue = 0;

	PlayVoice(pl, "tongue");
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateFrogTongue
	Purpose			: updates Frogger's tongue for collecting collectables
	Parameters		: none
	Returns			: 
	Info			:
*/
void UpdateFrogTongue( int pl )
{
	if( tongue[pl].flags & TONGUE_IDLE )
		return;

	// Frogs tongue is ready for action
	if( tongue[pl].flags & TONGUE_BEINGUSED )
	{
		// Check if tongue can actually be used
		if( tongue[pl].flags & TONGUE_HASITEMINMOUTH )
			return;

		// Update source from matrix of head bone
		CalcTongueSource( pl );

		// Update the targets position
		switch( tongue[pl].type )
		{
 		case TONGUE_GET_SCENIC:	SetVectorSS(&tongue[pl].target, &((ENEMY *)tongue[pl].thing)->nmeActor->actor->position ); break;
		case TONGUE_GET_PICKUP: SetVectorSS(&tongue[pl].target, &((GARIB *)tongue[pl].thing)->fx->act[0]->actor->position ); break;
		case TONGUE_GET_NONE: SetVectorSS( &tongue[pl].target, &frog[pl]->actor->position ); break;
		}

		if( tongue[pl].flags & TONGUE_OUTGOING )
		{
			fixed radcheck = TONGUE_STICKYRADIUS;
			if( player[0].worldNum == WORLDID_SPACE && player[0].levelNum == LEVELID_SPACE2 )
				if( currPlatform[pl] && !currPlatform[pl]->isWaiting && currPlatform[pl]->active ) 
					radcheck += FMul(currPlatform[pl]->currSpeed, gameSpeed)*256;

			// If tongue has not got to the target
			if( DistanceBetweenPointsSF(&tongue[pl].target,&tongue[pl].interp) > radcheck )
			{
				// Extend the tongue a bit more
				tongue[pl].progress += FMul( TONGUE_FRACTION, FMul(gameSpeed,6144-tongue[pl].progress) );

				if( tongue[pl].progress > 4096 ) tongue[pl].progress = 4096;
				LinearInterpFSSF( &tongue[pl].interp, &tongue[pl].source, &tongue[pl].target, tongue[pl].progress );
				CalculateTongue( pl );

				// If we have gone too far
				if( DistanceBetweenPointsSS(&tongue[pl].source,&tongue[pl].pos) > tongue[pl].radius )
				{
					tongue[pl].flags = TONGUE_NONE | TONGUE_BEINGUSED | TONGUE_INCOMING;
					return;
				}
			}
			else // Got to target - return with item
			{
				tongue[pl].flags = TONGUE_NONE | TONGUE_BEINGUSED | TONGUE_INCOMING;
				if( tongue[pl].thing )
					tongue[pl].flags |= TONGUE_HASITEMONIT;
				return;
			}
		}
		
		if(tongue[pl].flags & TONGUE_INCOMING)
		{
			fixed radcheck = TONGUE_STICKYRADIUS;
			if( currPlatform[pl] ) radcheck += currPlatform[pl]->currSpeed;

			if( tongue[pl].flags & TONGUE_HASITEMONIT )
			{
/*				if(tongue[pl].type == TONGUE_GET_GARIB)
				{
					GARIB *g = (GARIB *)tongue[pl].thing;
					SetVectorSS( &g->sprite->pos, &tongue[pl].pos );
					g->scale = FMul(g->scale, 3686);
				}
				else*/ if(tongue[pl].type == TONGUE_GET_PICKUP)
				{
					GARIB *g = (GARIB *)tongue[pl].thing;
					SetVectorSS( &g->fx->act[0]->actor->position, &tongue[pl].pos );
				}
				else if( tongue[pl].type == TONGUE_GET_SCENIC )
				{
					ENEMY *e = (ENEMY *)tongue[pl].thing;
					ScaleVectorFF( &e->nmeActor->actor->size, 4096-((128*gameSpeed)>>12) );
					SetVectorSS( &e->nmeActor->actor->position, &tongue[pl].pos );
				}
			}

			// Retract tongue a bit
//			if( !(padData.digital[pl] & PAD_CIRCLE) || !tongue[pl].thing )
				tongue[pl].progress -= FMul( TONGUE_FRACTION, FMul(gameSpeed,6144-tongue[pl].progress) );

			if( tongue[pl].progress < 0 ) tongue[pl].progress = 0;
			LinearInterpFSSF( &tongue[pl].interp, &tongue[pl].source, &tongue[pl].target, tongue[pl].progress );
			CalculateTongue( pl );

			// If tongue has got back to mouth
			if( DistanceBetweenPointsSF(&tongue[pl].source,&tongue[pl].interp) < radcheck )
			{
				if( tongue[pl].flags & TONGUE_HASITEMONIT )
				{
					TRIGGER *t;
					void *arg1;

					t = MakeTrigger( OnTimeout, (void *)(actFrameCount + 10), NULL, NULL, NULL );

//					if( actFrameCount&1) 
						arg1 = (void *)FindVoice(utilStr2CRC("frogbelch1"),pl);
//					else arg1 = (void *)FindVoice(utilStr2CRC("frogbelch2"),pl);

					AttachEvent( t, TRIGGER_ONCE, 0, PlaySFX, arg1, (void *)(&frog[pl]->actor->position), NULL, NULL );

/*					if( tongue[pl].type == TONGUE_GET_GARIB )
					{
						PickupCollectable( (GARIB *)tongue[pl].thing, pl );
					}
					else */if( tongue[pl].type == TONGUE_GET_PICKUP )
					{
						GARIB *g = (GARIB *)tongue[pl].thing;
						PickupCollectable( (GARIB *)tongue[pl].thing, pl );
					}
					else if( tongue[pl].type == TONGUE_GET_SCENIC )
					{
						ENEMY *e = (ENEMY *)tongue[pl].thing;

						DEACTIVATEENEMY( e );

						if( eatEverythingMode )
						{
							FVECTOR up;
							SetVectorFF( &up, &currTile[pl]->normal );

							CreateExplodeEffect( &frog[pl]->actor->position, &up );

							CheckForAttachedNMEs( e, 0 );
						}
					}
				}

				tongue[pl].thing = NULL;
				RemoveFrogTongue(pl);

				// Set frog idle animation
				if( !(player[pl].frogState & FROGSTATUS_ISDEAD) )
					actorAnimate(frog[pl]->actor,FROG_ANIM_BREATHE,YES,YES,FROG_BREATHE_SPEED,0);
			}
		}
	}

	if( tongue[pl].flags & TONGUE_SEARCHING )
	{
		if( (tongue[pl].thing = (void *)GaribIsInRange(tongue[pl].radius,pl)) )
		{
			switch( ((GARIB *)tongue[pl].thing)->type )
			{
//			case SILVERCOIN_GARIB:
//			case GOLDCOIN_GARIB:
//				StartTongue( TONGUE_GET_GARIB, &((GARIB *)tongue[pl].thing)->sprite->pos, pl ); 
//				break;
			case QUICKHOP_GARIB:
			case SLOWHOP_GARIB:
			case AUTOHOP_GARIB:
			case INVULNERABILITY_GARIB:
			case EXTRALIFE_GARIB:
				if( ((GARIB *)tongue[pl].thing)->fx )
					StartTongue( TONGUE_GET_PICKUP, &((GARIB *)tongue[pl].thing)->fx->act[0]->actor->position, pl ); 
				break;
			default:
				tongue[pl].thing = NULL;
				tongue[pl].flags = TONGUE_IDLE;
				break;
			}
		}
		
		if( !tongue[pl].thing )
		{
			if( (tongue[pl].thing = (void *)ScenicIsInRange(tongue[pl].radius,pl)) )
			{
				ENEMY *e = (ENEMY *)tongue[pl].thing;
				// Stop fish making bubbles when held - butterflies can carry on doing something
				StartTongue( TONGUE_GET_SCENIC, &e->nmeActor->actor->position, pl );
				if( tongue[pl].thing )
				{
					e->nmeActor->effects &= ~EF_BUBBLES;
					e->active = 0;
				}
			}
		}

		if( !tongue[pl].thing ) // Go to a point out in front and wave about a bit
			StartTongue( TONGUE_GET_NONE, &frog[pl]->actor->position, pl );
	}
}


/*	--------------------------------------------------------------------------------
	Function		: CalculateTongue
	Purpose			: Plot a Hermite curve based on the start and end points
	Parameters		: 
	Returns			: 
	Info			: 
*/
void CalculateTongue( int pl )
{
	SVECTOR to;
	fixed gx[4], gy[4], gz[4], t=0, t2, t3;
	int n, i, index;

	n = MAX_TONGUENODES; // Should probably be worked out by fraction of max radius covered

	SubVectorSSS(&to, &tongue[pl].target, &tongue[pl].source);

	// Set geometry components
	gx[0] = ToFixed(tongue[pl].source.vx);
	gx[1] = tongue[pl].interp.vx;
	gx[2] = tongue[pl].fwd.vx *= 2000;
	gx[3] = ToFixed(to.vx);
	gy[0] = ToFixed(tongue[pl].source.vy);
	gy[1] = tongue[pl].interp.vy;
	gy[2] = tongue[pl].fwd.vy *= 2000;
	gy[3] = ToFixed(to.vy);
	gz[0] = ToFixed(tongue[pl].source.vz);
	gz[1] = tongue[pl].interp.vz;
	gz[2] = tongue[pl].fwd.vz *= 2000;
	gz[3] = ToFixed(to.vz);

	index = (tongue[pl].progress * (MAX_TONGUENODES-1))>>12;

	SetVectorFS( &tongue[pl].segment[0], &tongue[pl].source );

	for( i=1; i<=index; i++ )
	{
		t += TONGUE_FRACTION;
		t2 = FMul(t,t);
		t3 = FMul(t2,t);

		tongue[pl].segment[i].vx = FMul((t3*2-t2*3+4096),gx[0]) + FMul((t3*-2+t2*3),gx[1]) + FMul((t3-t2*2+t),gx[2]) + FMul((t2-t2),gx[3]);
		tongue[pl].segment[i].vy = FMul((t3*2-t2*3+4096),gy[0]) + FMul((t3*-2+t2*3),gy[1]) + FMul((t3-t2*2+t),gy[2]) + FMul((t2-t2),gy[3]);
		tongue[pl].segment[i].vz = FMul((t3*2-t2*3+4096),gz[0]) + FMul((t3*-2+t2*3),gz[1]) + FMul((t3-t2*2+t),gz[2]) + FMul((t2-t2),gz[3]);
	}
	
	SetVectorSF(&tongue[pl].pos, &tongue[pl].segment[index]);
}


/*	--------------------------------------------------------------------------------
	Function		: RemoveFrogTongue
	Purpose			: removes the frog tongue
	Parameters		: 
	Returns			: 
	Info			: 
*/
void RemoveFrogTongue( int pl )
{
	tongue[pl].flags = TONGUE_NONE | TONGUE_IDLE;
	if( tongue[pl].thing )
	{
		switch( tongue[pl].type )
		{
		case TONGUE_GET_SCENIC: 
			{
				ENEMY *nme = (ENEMY *)tongue[pl].thing;

				ACTIVATEENEMY( nme );

				if( eatEverythingMode )
					CheckForAttachedNMEs( nme, 1 );

				break;
			}
		}
	}

	tongue[pl].thing = NULL;
	tongue[pl].radius = ToFixed(TONGUE_RADIUSNORMAL);
	tongue[pl].type = 0;
	tongue[pl].progress = 0;
	tongue[pl].canTongue = 1;

	player[pl].frogState &= ~FROGSTATUS_ISTONGUEING;
}




/*	--------------------------------------------------------------------------------
	Function		: GaribIsInRange
	Purpose			: indicates if a garib is in range when tongueing
	Parameters		: 
	Returns			: GARIB *
	Info			: returns ptr to the nearest garib (if in range)
*/
GARIB *GaribIsInRange( fixed radius, int pl )
{
	GARIB *g, *nearest=NULL;
	fixed dist, best=radius;

	for( g=garibList.head.next; g != &garibList.head; g=g->next )
	{
		if( !g->fx || !g->active )
			continue;

		dist = DistanceBetweenPointsSS( &frog[pl]->actor->position, &g->fx->act[0]->actor->position );
		if( dist < best )
		{
			best = dist;
			nearest = g;
		}
	}

	return nearest;
}


/*	--------------------------------------------------------------------------------
	Function		: BabyFrogIsInTongueRange
	Purpose			: indicates if a baby frog is in range when tongueing
	Parameters		: 
	Returns			: 
	Info			: returns ptr to the nearest baby frog (if in range)
*/
ENEMY *BabyFrogIsInRange( fixed radius, int pl )
{
	ENEMY *nearest=NULL;
	fixed dist, best=radius;
	int i;

	for( i=0; i<numBabies; i++ )
	{
		if( !babyList[i].baby || !babyList[i].enemy || babyList[i].isSaved )
			continue;

		dist = DistanceBetweenPointsSS( &frog[pl]->actor->position, &babyList[i].baby->actor->position );
		if( dist < best )
		{
			best = dist;
			nearest = babyList[i].enemy;
		}
	}

	return nearest;
}


/*	--------------------------------------------------------------------------------
	Function		: ScenicIsInTongueRange
	Purpose			: indicates if a scenic is in range when tongueing
	Parameters		: 
	Returns			: ACTOR2 *
	Info			: returns ptr to the nearest scenic (if in range)
*/
ENEMY *ScenicIsInRange( fixed radius, int pl )
{
	ENEMY *cur, *nearest=NULL;
	fixed dist, best=radius;
		
	for(cur = enemyList.head.next; cur != &enemyList.head; cur = cur->next)
	{
		if( !eatEverythingMode )
		{
			if( !(cur->flags & ENEMY_NEW_FLAPPYTHING) || !cur->active || !cur->visible )
				continue;
		}
		else
		{
			if( (cur->flags & ENEMY_NEW_BABYFROG) || (cur->flags & ENEMY_NEW_PUSHESFROG) || !cur->active || !cur->visible || cur->uid ) 
				continue;
		}

		dist = DistanceBetweenPointsSS( &frog[pl]->actor->position, &cur->nmeActor->actor->position );
		if( dist < best )
		{
			best = dist;
			nearest = cur;
		}
	}

	return nearest;
}


/*	--------------------------------------------------------------------------------
	Function		: FrogIsInRange
	Purpose			: Find nearest frog 
	Parameters		: 
	Returns			: ACTOR2 *
	Info			: Multiplayer use only
*/
ACTOR2 *FrogIsInRange(fixed radius, int pl)
{
//	ACTOR2 *f;
	int i, closest=-1;
	fixed dist, best=2048000000;

	for( i=(pl+1)%NUM_FROGS; i!=pl; i=(i+1)%NUM_FROGS )
	{
		dist = DistanceBetweenPointsSS(&frog[i]->actor->position, &frog[pl]->actor->position);
		if( (dist < radius) && (dist < best) )
		{
			closest = i;
			best = dist;
		}
	}

	if( closest == -1 ) return NULL;

	return frog[closest];
}


/*	--------------------------------------------------------------------------------
	Function		: ThrowFrogDirection
	Purpose			: Hurl a frog along a tile pointer direction
	Parameters		: 
	Returns			: 
	Info			: Multiplayer use only
*/
void ThrowFrogDirection( int thrower, int throwee, int dir )
{
	GAMETILE *tile = NULL, *tile2 = currTile[thrower], *tile3 = NULL;
	fixed count=0;
	FVECTOR fwd;

	SetVectorFF( &fwd, &currTile[thrower]->dirVector[dir] );
	// Find all tiles in the direction as far as the eye can see
	while( (tile = FindJoinedTileByDirectionConstrained(tile2,&fwd,1024)) )
	{
		if( tile3 == tile ) { tile = tile2; break; } // Oscillating between 2 tiles
		tile3 = tile2;
		tile2 = tile;
		count += 4096;
	}

	if( !tile ) tile = tile2;

	actorAnimate( frog[throwee]->actor, FROG_ANIM_BOWLEDOVER, NO, NO, 128, 0 );
	actorAnimate( frog[throwee]->actor, FROG_ANIM_FALLFLAP, NO, YES, 128, 0 );

	SpringFrogToTile(tile, (204800+FMul(count,40960))*SCALE, 410+FMul(count,328), throwee);
}


/*	--------------------------------------------------------------------------------
	Function		: CalcTongueSource
	Purpose			: Work out the orientation and position of the tongue from froggers head bone matrix
	Parameters		: 
	Returns			: 
	Info			: 
*/
void CalcTongueSource( int pl )
{
#ifdef PC_VERSION
	if( headMatrix )
	{
		MDX_VECTOR fwd, up, pos;

		// Store head bone pos and set matrix translate elements to zero
		SetVector( &pos, (MDX_VECTOR *)&headMatrix[12] );
		headMatrix[12] = headMatrix[13] = headMatrix[14] = 0;

		// Matrix multiply to get rotation only
		guMtxXFMF( headMatrix, 0, 0, 1, &fwd.vx, &fwd.vy, &fwd.vz );
		guMtxXFMF( headMatrix, 0, -1, 0, &up.vx, &up.vy, &up.vz );

		// JIM: Nasty special case until I can be bothered working out another way
		if( player[pl].character == FROG_HOPPER )
		{
			ScaleVector( &fwd, -3 );
		}
		else if( player[pl].character == FROG_SWAMPY )
		{
			ScaleVector(&fwd,3);
			ScaleVector(&up,-0.5);
		}


		// Store in tongue forward as unit vector
		SetVectorFR( &tongue[pl].fwd, &fwd );
		MakeUnit( &tongue[pl].fwd );
		// Scale to get offset to front of head
		ScaleVector( &fwd, 12 );
		ScaleVector( &up, 5 );

		// Actual front of head position
		mdxAddToVector( &fwd, &pos );
		mdxAddToVector( &fwd, &up );
		ScaleVector( &fwd, SCALE );
		// Which is our source
		SetVectorSR( &tongue[pl].source, &fwd );
	}
#else
	if(pHeadMatrix)
	{
		FVECTOR fwd, up;
		SVECTOR pos;
		FVECTOR foreVec = {0,0,-4096};

		// Store head bone pos and set matrix translate elements to zero
		pos.vx = -pHeadMatrix->t[0];
		pos.vy = -pHeadMatrix->t[1];
		pos.vz = pHeadMatrix->t[2];
		pHeadMatrix->t[0] = pHeadMatrix->t[1] = pHeadMatrix->t[2] = 0;

		// Matrix multiply to get rotation only
		ApplyMatrixLV(pHeadMatrix, &foreVec, &fwd);
		ApplyMatrixLV(pHeadMatrix, &upVec, &up);
		//this invert is because we're using a psx matrix
		//on a pc coordinate. (i think that's why!)
		fwd.vx = -fwd.vx;
		fwd.vy = -fwd.vy;
		up.vx = -up.vx;
		up.vy = -up.vy;
		
		// JIM: Nasty special case until I can be bothered working out another way
		if( player[pl].character == FROG_HOPPER )
		{
			ScaleVectorFF( &fwd, -12000 );
		}
		else if( player[pl].character == FROG_SWAMPY )
		{
			ScaleVectorFF(&fwd,12000);
			ScaleVectorFF(&up,-2000);
		}

		// Store in tongue forward as unit vector
		SetVectorFF(&tongue[pl].fwd, &fwd);
		MakeUnit(&tongue[pl].fwd);
		// Scale to get offset to front of head
		ScaleVector(&fwd, 12*SCALE);
		ScaleVector(&up, 5*SCALE);

		// Actual front of head position
		AddToVectorFS(&fwd, &pos);
		AddToVectorFF(&fwd, &up);
		// Which is our source
		SetVectorSF(&tongue[pl].source, &fwd);
	}

#endif

	else
	{
		// Update the forward vector
 		RotateVectorByQuaternionFF(&tongue[pl].fwd, &inVec, &frog[pl]->actor->qRot);
		tongue[pl].source.vx = frog[pl]->actor->position.vx + ((currTile[pl]->normal.vx * TONGUE_OFFSET_UP)>>12);
		tongue[pl].source.vy = frog[pl]->actor->position.vy + ((currTile[pl]->normal.vy * TONGUE_OFFSET_UP)>>12);
		tongue[pl].source.vz = frog[pl]->actor->position.vz + ((currTile[pl]->normal.vz * TONGUE_OFFSET_UP)>>12);
	}
}


/*	--------------------------------------------------------------------------------
	Function		: CheckForAttachedNMEs
	Purpose			: Mainly for Sub2 - check back and forward along path for nothing.obes until blank flag reached
	Parameters		: Starting nme, activeate/deactivate
	Returns			: 
	Info			: Not sure how well this will work
*/
void CheckForAttachedNMEs( ENEMY *e, int flag )
{
	ENEMY *nme;
	PATH *p = e->path;
	int i;

	// Search backwards in the path
	i = p->toNode;
	while( 1 )
	{
		if( (--i) < 0 ) 
			i = p->numNodes-1;

		if( !(nme = FindEnemyAtNode(e,i)) )
			break;

		if( nme->uid )
			continue;

		if( flag )
		{
			ACTIVATEENEMY( nme );
		}
		else
		{
			DEACTIVATEENEMY( nme );
		}
	}

	// Then forwards.
	i = p->toNode;
	while( 1 )
	{
		if( (++i) >= p->numNodes ) 
			i = 0;

		if( !(nme = FindEnemyAtNode(e,i)) )
			break;

		if( nme->uid )
			continue;

		if( flag )
		{
			ACTIVATEENEMY( nme );
		}
		else
		{
			DEACTIVATEENEMY( nme );
		}
	}
}


