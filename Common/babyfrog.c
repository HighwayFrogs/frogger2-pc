/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: babyfrog.c
	Programmer	: James Healey
	Date		: 17/03/99

----------------------------------------------------------------------------------------------- */

#define F3DEX_GBI_2

#include "types.h"

#include "Main.h"
#include "enemies.h"
#include "babyfrog.h"
#include "cam.h"
#include "game.h"
#include "map.h"
#include "frogger.h"
#include "frogmove.h"
#include "maths.h"
#include "specfx.h"
#include "newpsx.h" //mm
#include "layout.h"
#include "hud.h"

#ifdef PSX_VERSION
#include "islpsi.h"
#include "audio.h"
#else
#include <pcaudio.h>
#include <stdio.h>
#endif


BABY babyList[NUM_BABIES];

fixed BABY_SCALE = 1638;
int lastBabySaved	= -1;

SPRITEOVERLAY *babyIcons[NUM_BABIES];
unsigned long babiesSaved = 0;

void RunBabyCollect( int bby );

SPRITEOVERLAY *babyFlash;
long babyFlashFrame;

void InitBabyList( unsigned char createOverlays )
{
	int i;

	lastBabySaved	= -1;
	babiesSaved		= 0;
	
	for (i=0; i<numBabies; i++)
	{
		babyList[i].isSaved = 0;
		babyList[i].baby = NULL;
		babyList[i].idle = 0;
		babyList[i].collect = 0;

		// determine baby colour and set values accordingly
		switch(i)
		{
			case 0:
				// green frog
				babyList[i].fxColour[R] = 60;
				babyList[i].fxColour[G] = 255;
				babyList[i].fxColour[B] = 60;
				babyList[i].fxColour[A] = 255;

				break;
			case 1:
				// yellow frog
				babyList[i].fxColour[R] = 255;
				babyList[i].fxColour[G] = 255;
				babyList[i].fxColour[B] = 60;
				babyList[i].fxColour[A] = 255;
				break;
			case 2:
				// blue frog
				babyList[i].fxColour[R] = 60;
				babyList[i].fxColour[G] = 60;
				babyList[i].fxColour[B] = 255;
				babyList[i].fxColour[A] = 255;
				break;
			case 3:
				// purple frog
				babyList[i].fxColour[R] = 255;
				babyList[i].fxColour[G] = 60;
				babyList[i].fxColour[B] = 255;
				babyList[i].fxColour[A] = 255;
				break;
			case 4:
				// red frog
				babyList[i].fxColour[R] = 255;
				babyList[i].fxColour[G] = 60;
				babyList[i].fxColour[B] = 60;
				babyList[i].fxColour[A] = 255;
				break;
		}
	}

	babyFlash = NULL;
	if ( createOverlays )
	{
#ifdef PSX_VERSION
		babyFlash = CreateAndAddSpriteOverlay( (230+(i*200)),190+BORDER,"00BABYIC",(4096*24)/640,(4096*24)/480,0,SPRITE_ADDITIVE);
#else
//		char name[12];
		babyFlash = CreateAndAddSpriteOverlay( (230+(i*200)),210+BORDER,"00BABYIC",(4096*24)/640,(4096*24)/480,0,SPRITE_ADDITIVE);
#endif
		for(i=0; i<numBabies; i++)
		{
#ifdef PSX_VERSION
			babyIcons[i] = CreateAndAddSpriteOverlay( (230+(i*200)),190+BORDER,"00BABYIC",(4096*24)/640,(4096*24)/480,60,0);
#else
//			sprintf( name, "0%iBABYIC", i );
//			babyIcons[i] = CreateAndAddSpriteOverlay( (230+(i*200)),210+BORDER,name,(4096*24)/640,(4096*24)/480,60,0);
			if( rHardware )
				babyIcons[i] = CreateAndAddSpriteOverlay( (230+(i*200)),210+BORDER,"00BABYIC",(4096*24)/640,(4096*24)/480,60,0);
			else
				babyIcons[i] = CreateAndAddSpriteOverlay( (230+(i*200)),210+BORDER,"00BABYIC",(4096*24)/640,(4096*24)/480,255,0);
#endif
			babyIcons[i]->num = 1;
			babyIcons[i]->r = babyList[i].fxColour[R];
			babyIcons[i]->g = babyList[i].fxColour[G];
			babyIcons[i]->b = babyList[i].fxColour[B];

#ifdef PC_VERSION
			if( !rHardware )
			{
				babyIcons[i]->r /= 2;
				babyIcons[i]->g /= 2;
				babyIcons[i]->b /= 2;
			}
#endif
		}
	}
	babyFlashFrame = -1;
}

void ResetBabies( )
{
	int i;

	for( i=0; i<numBabies; i++ )
	{
		babyList[i].isSaved = 0;
		babyList[i].baby = NULL;
		babyList[i].idle = 0;
		babyList[i].collect = 0;
		babiesSaved = 0;
		lastBabySaved = -1;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: PickupBabyFrog
	Purpose			: called when a baby frog is picked up...
	Parameters		: ACTOR2
	Returns			: truth
	Info			:
*/
int PickupBabyFrog( ACTOR2 *baby, GAMETILE *tile )
{
	unsigned long i,n;

	for( i=0; i<numBabies; i++ ) if( babyList[i].baby == baby ) break;
	if( i==numBabies ) return FALSE;

	// check if this baby has been collected
	if(babyList[i].isSaved)
		return FALSE;

	// Hack for icon ordering. Bleh.
	if( babyList[i].enemy->path->nodes->offset2 )
		n = ((babyList[i].enemy->path->nodes->offset2>>12)/SCALE)-1;
	else
		n = i;

#ifdef PC_VERSION
	if( !rHardware )
	{
		babyIcons[n]->r *= 2;
		babyIcons[n]->g *= 2;
		babyIcons[n]->b *= 2;
	}
#endif

	babyFlashFrame = actFrameCount;
	babyFlash->r = babyIcons[n]->r;
	babyFlash->g = babyIcons[n]->g;
	babyFlash->b = babyIcons[n]->b;

	babyFlash->xPos = babyIcons[n]->xPos;	
	babyFlash->yPos = babyIcons[n]->yPos;
	babyFlash->width = babyIcons[n]->width;	
	babyFlash->height = babyIcons[n]->height;		

	babyList[i].isSaved	= 1;
	babyList[i].collect = 40;

	lastBabySaved = i;

	babyIcons[n]->a = 255;
	arcadeHud.babiesBack[n]->r = arcadeHud.babiesBack[n]->g = arcadeHud.babiesBack[n]->b = 255;

	babiesSaved++;
	
	// If baby references a placeholder then set the respawn position to be the placeholder
	if( baby->value1 )
	{
		ENEMY *respawn = GetEnemyFromUID( baby->value1>>12 );
		if( respawn )
			gTStart[0] = respawn->path->nodes->worldTile;
		else
			gTStart[0] = tile;
	}
	else // Else respawn on the current baby tile
	{
		gTStart[0] = tile;
	}

	startCamFacing = camFacing[0];
	startFrogFacing = frogFacing[0];

	// Make a noise (weeble weeble) and do some snazzy effects (*zing!*)
	PlayVoice(0, "frogokay");
	PlaySample( genSfx[GEN_COLLECT_BABY], NULL, 0, SAMPLE_VOLUME, -1 );

	BabyCollectEffect( baby, tile, n );
 	
	return TRUE;
}


/*	--------------------------------------------------------------------------------
	Function		: GetNearestBabyFrog
	Purpose			: determines the nearest baby frog
	Parameters		: 
	Returns			: ACTOR2 *
	Info			: used for, say, determining
*/
int GetNearestBabyFrog()
{
	int baby=-1;
	fixed t,distance = 9999999999;
	int i;

	i = numBabies;
	while(i--)
	{
		if(!babyList[i].isSaved && babyList[i].baby && babyList[i].baby->draw )
		{
			t = DistanceBetweenPointsSS(&frog[0]->actor->position,&babyList[i].baby->actor->position);
			if(t < distance)
			{
				distance	= t;
				baby = i;
			}
		}
	}

	return baby;
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateBabies
	Purpose			: Determine behaviour of baby frogs
	Parameters		: 
	Returns			: 
	Info			: Mope when far from frog, then do sequence of attention grabbing animations.
						Later, dynamically swap to lower poly model when far from camera.
*/
void UpdateBabies( )
{
	int i, n;
	ACTOR *act;
	ACTOR2 *baby;
	FVECTOR frogV;
	long dist;
	long flashFr = actFrameCount-babyFlashFrame;

	if (babyFlash)
	{
		if ((flashFr < 70) && (babyFlashFrame>0))
		{
			babyFlash->draw = 1;
			babyFlash->a = 0xff - ((flashFr*0xff)/70);
			babyFlash->width = (4096*32)/640 + ((flashFr*(4096*64)/640)/70);
			babyFlash->height = (4096*32)/480 + ((flashFr*(4096*64)/480)/70);
		}
		else
			babyFlash->draw = 0;
	}

	for( i=0; ((i<numBabies) && (babyList[i].baby)); i++ )
	{
		baby = babyList[i].baby;
		act = baby->actor;

		if( !babyList[i].baby->draw ) 
			continue;

		// ------------------------------------------------------
		// Baby frog has been collected, so move up and spin
		if( babyList[i].collect )
		{
			RunBabyCollect(i);
			continue;
		}

		// ------------------------------------------------------
		// Special case hack for floating in space2
		if( player[0].worldNum == WORLDID_SPACE && player[0].levelNum == 1 )
		{
			StartAnimateActor( act, BABY_ANIM_FLOAT, YES, NO, 75, 0 );
			continue;
		}

		// ------------------------------------------------------
		// Otherwise, check for distance from frog and do behaviour
		SubVectorFSS( &frogV, &frog[0]->actor->position, &act->position );
		dist = MagnitudeF(&frogV)>>12;

		if( babyList[i].idle > 0 )
			babyList[i].idle -= gameSpeed>>12;

		if( dist < BABY_ACTIVE_RADIUS ) // Play attract animations when frog is close
		{
			IQUATERNION q;
			fixed speed;

			MakeUnit( &frogV );
			SetQuaternion(&q,&act->qRot);
			OrientateFS( &act->qRot, &frogV, &currTile[0]->normal );
			speed = gameSpeed/5;
			if( speed > 4090 ) speed = 4090;
			IQuatSlerp( &q, &act->qRot, speed, &act->qRot );

			// Decrease twice as fast if very close
			if( dist < BABY_ACTIVE_RADIUS/2 )
				babyList[i].idle -= gameSpeed>>12;

			if( babyList[i].idle < 1 )
			{
				if( dist < BABY_ACTIVE_RADIUS/2 )
				{
					if( !Random(max(dist/180,1)) )
						PlaySample( genSfx[GEN_BABYHAPPY], &act->position, 0, SAMPLE_VOLUME, 48+Random(16) );

					StartAnimateActor( act, BABY_ANIM_HOP, YES, NO, 154, 0 );
					babyList[i].idle = BABY_IDLE_TIME>>2;
				}
				else
				{
					// Randomly play attract animation
					actorAnimate( act, Random(7)+2, NO, NO, 102, 0 );
					actorAnimate( act, BABY_ANIM_SNOOZE, YES, YES, 102, 0 );
					babyList[i].idle = BABY_IDLE_TIME + Random(dist/SCALE);
					PlaySample( genSfx[GEN_BABYSAD], &act->position, 0, SAMPLE_VOLUME, -1 );
				}
			}
		}
		else if( dist > BABY_ACTIVE_RADIUS+(BABY_ACTIVE_RADIUS/2) && babyList[i].idle > 1 ) // If frog is out of range but the idle timer is still going then try to get him back
		{
			SPECFX *fx;
			FVECTOR up;
			SVECTOR pos;

			actorAnimate( act, BABY_ANIM_TWOWAVE, NO, NO, 102, 0 );
			SetVectorFF( &up, &babyList[i].enemy->inTile->normal );
			ScaleVector( &up, 20 );
			AddVectorSFS( &pos, &up, &act->position );
			SetVectorFF( &up, &currTile[0]->normal );

			if( gameState.multi == SINGLEPLAYER )
			{
				PrepForPriorityEffect( );
				if( (fx = CreateSpecialEffectDirect( FXTYPE_CROAK, &pos, &up, 81920, 4096, 410, 6144 )) )
				{
					if( babyList[i].enemy->path->nodes->offset2 )
						n = ((babyList[i].enemy->path->nodes->offset2>>12)/SCALE)-1;
					else
						n = i;

					fx->spin = 25;
					SetFXColour( fx, babyList[n].fxColour[R], babyList[n].fxColour[G], babyList[n].fxColour[B] );
				}
			}

			PlaySample( genSfx[GEN_BABYCRY], NULL, 0, SAMPLE_VOLUME, -1 );
			babyList[i].idle = 0;
		}
		else StartAnimateActor( act, BABY_ANIM_SNOOZE, YES, YES, 102, 0 );
	}
}


/*	--------------------------------------------------------------------------------
	Function		: RunBabyCollect
	Purpose			: Spin and move up
	Parameters		: 
	Returns			: 
	Info			: 
*/
void RunBabyCollect( int bby )
{
	IQUATERNION q;
//	fixed speed;
	FVECTOR fwd, up;
	ACTOR2 *baby = babyList[bby].baby;
	ACTOR *act = baby->actor;

	SetVectorFF( &up, &currTile[0]->normal );
	SetVectorFF( (FVECTOR *)&q, &up );
	q.w = FDiv( (((actFrameCount*2)%360)<<12), 235930 ); // actFrameCount degrees converted to radians

	fixedRotateVectorByRotation( &fwd, &inVec, &q );
	Orientate( &act->qRot, &fwd, &up );

	babyList[bby].collect -= max( gameSpeed>>12, 1 );

	if( babyList[bby].collect < 1 )
	{
		SPECFX *fx;
		if( (fx = CreateSpecialEffectDirect( FXTYPE_SPARKLYTRAIL, &act->position, &up, 204800, 12288, 0, 20480 )) )
		{
			SetFXColour( fx, babyList[bby].fxColour[0], babyList[bby].fxColour[1], babyList[bby].fxColour[2] );
			if(fx->rebound)
				SetVectorSS( &fx->rebound->point, &currTile[0]->centre );
			fx->gravity = 2870;
		}
		if( (fx = CreateSpecialEffectDirect( FXTYPE_SPARKLYTRAIL, &act->position, &up, 163840, 1240, 0, 24576 )) )
		{
			SetFXColour( fx, 220, 220, 220 );
			if(fx->rebound)
				SetVectorSS( &fx->rebound->point, &currTile[0]->centre );
			fx->gravity = 2870;
		}
		if( (fx = CreateSpecialEffectDirect( FXTYPE_SPARKLYTRAIL, &act->position, &up, 122880, 8192, 0, 28672 )) )
		{
			SetFXColour( fx, babyList[bby].fxColour[0], babyList[bby].fxColour[1], babyList[bby].fxColour[2] );
			if(fx->rebound)
				SetVectorSS( &fx->rebound->point, &currTile[0]->centre );
			fx->gravity = 2870;
		}
		if( (fx = CreateSpecialEffectDirect( FXTYPE_SPARKLYTRAIL, &act->position, &up, 81920, 6144, 0, 32768 )) )
		{
			SetFXColour( fx, 220, 220, 220 );
			if(fx->rebound)
				SetVectorSS( &fx->rebound->point, &currTile[0]->centre );
			fx->gravity = 2870;
		}
		baby->draw = 0;
		babyList[bby].collect = 0;
	}

	ScaleVectorFF( &up, gameSpeed*30 );
	AddToVectorSF( &act->position, &up );
}
