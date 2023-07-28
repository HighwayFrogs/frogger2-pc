#define COINS_IN_ARCADE_MODE

/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: collect.c
	Programmer	: Andrew Eder
	Date		: 11/27/98

----------------------------------------------------------------------------------------------- */

//mm #include "ultra64.h"

#include "NewPsx.h"
#include "Types.h"

#include "islpsi.h"
#include <islutil.h>
#include <islmem.h>

#include "collect.h"
#include "maths.h"
#include "frogger.h"
#include "hud.h"
#include "types.h"
#include "layout.h"
#include "frontend.h"
#include "Main.h"
#include "tongue.h"
#include "game.h"
#include "multi.h"
#include "frogmove.h"
#include "lang.h"
#include "menus.h"

#ifdef PC_VERSION
#include <pcaudio.h>
#include <pcmisc.h>
#include <stdio.h>
#include <mdxFont.h>
#else
#include "audio.h"
#include "Textures.h"
#endif

extern long coinZoomX,coinZoomY;

GARIBLIST garibList;
TIMER pauseTimer;
int pauseFlag;

char pickupObjName[16] = "bfly.obe";

GARIB *AllocGarib( );

// If in multiplayer then kill the effect, else set it to not draw
void TogglePowerup( GARIB *x, char v )
{
	if( !x->fx )
		return;

	x->fx->sprites->draw = v;
	x->fx->act[0]->draw = v;
	x->active = v; 
}


/*	--------------------------------------------------------------------------------
	Function		: CheckTileForCollectable
	Purpose			: checks frogs position for a collectable item
	Parameters		: 
	Returns			: void
	Info			: used to check if Frogger has jumped on a collectable item
*/
void CheckTileForCollectable(GAMETILE *tile, long pl)
{
	GARIB *garib, *next;
	SVECTOR *check;

	if( player[pl].frogState & FROGSTATUS_ISDEAD )
		return;
	
	//Allows the babies to be picked up again after the "Go to next baby" debug button is
	//pressed. 
// 	if (!( tile[pl].state & TILESTATE_OCCUPIED) &&(player[0].frogState & FROGSTATUS_ISDEBUG)) //mmtemp
// 		player[0].frogState &= ~FROGSTATUS_ISDEBUG; //mmtemp

	// check current tile for a garib
	for(garib = garibList.head.next; garib != &garibList.head; garib = next)
	{
		next = garib->next;
		// Also don't pickup garibs that the frog is trying to tongue
		if( !garib->active || garib == (GARIB *)tongue[pl].thing )
			continue;

		if( garib->sprite )
			check = &garib->sprite->pos;
		else
			check = &garib->pos;

		if( DistanceBetweenPointsSS( check, &frog[pl]->actor->position ) < PICKUP_RADIUS)
			PickupCollectable(garib,pl);
	}
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessCollectables
	Purpose			: processes collectables scattered across the many varied worldscapes...
	Parameters		: none
	Returns			: void
	Info			:
*/
void ProcessCollectables()
{
	UpdateGaribs();
	
	// update players (for spawn counters / score bonus)
	if(player[0].spawnTimer && gameState.multi == SINGLEPLAYER)
	{
		player[0].spawnTimer--;
		if(!player[0].spawnTimer)
			player[0].spawnScoreLevel = 1;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
void PickupCollectable(GARIB *garib, int pl)
{
	SPECFX *fx;
	FVECTOR seUp;
	int i;

	switch(garib->type)
	{
		case GOLDCOIN_GARIB:
			player[pl].numSpawn+=5;

		case SILVERCOIN_GARIB:
			if(gameState.multi == SINGLEPLAYER && gameState.mode != DEMO_MODE )
			{
				arcadeHud.coinZoom->draw = 1;
				arcadeHud.coinZoom->xPosTo = arcadeHud.coinsOver->xPos;
				arcadeHud.coinZoom->yPosTo = arcadeHud.coinsOver->yPos;
				arcadeHud.coinZoom->speed = 4096*50;

				TransformPoint(&frog[pl]->actor->position,&arcadeHud.coinZoom->xPos,&arcadeHud.coinZoom->yPos);
				coinZoomX = arcadeHud.coinZoom->xPos;
				coinZoomY = arcadeHud.coinZoom->yPos;
			}

			player[pl].numSpawn++;
			if((gameState.multi == SINGLEPLAYER) && (player[pl].numSpawn == 25))
			{
				arcadeHud.coinsOver->tex = FindTexture("COINMEDAL");
				PlayVoice(pl,"frogokay");
			}


			// we need to get the up vector for this collectable...
			SetVectorFF(&seUp, &upVec);
			RotateVectorByQuaternionFF(&seUp, &seUp, &frog[pl]->actor->qRot);

			if( gameState.multi != SINGLEPLAYER && multiplayerMode == MULTIMODE_BATTLE )
			{
				if( garib->type == GOLDCOIN_GARIB )
				{
 					mpl[pl].trail+=20;
				}
				else
				{
 					mpl[pl].trail+=4;
				}
			}
			else
			{
				if(player[pl].spawnTimer && player[pl].spawnScoreLevel < 5)
					player[pl].spawnScoreLevel++;

				player[pl].spawnTimer = SPAWN_SCOREUPTIMER;

				if(player[pl].spawnScoreLevel == 5)
				{
					player[pl].spawnScoreLevel = 0;

					PlayVoice(pl, "frogokay");

					if( (fx = CreateSpecialEffect( FXTYPE_SPARKLYTRAIL, &garib->sprite->pos, &seUp, 204800, 16384, 0, 24576 )) )
					{
						SetFXColour(fx,0,255,255);
						if(fx->rebound)
						{
							SetVectorSS(&fx->rebound->point,&frog[pl]->actor->position);
							SetVectorFF(&fx->rebound->normal,&seUp);
						}
						fx->gravity = 6140;
					}
				}

				/* 
				*	Check here for opening retro levels
				*/
			}

			if( (fx = CreateSpecialEffect( FXTYPE_SPARKLYTRAIL, &garib->sprite->pos, &seUp, 81920, 8192, 0, 8192 )) )
			{
				SetFXColour(fx,255,255,255);
				if(fx->rebound)
				{
					SetVectorSS(&fx->rebound->point, &garib->sprite->pos);
					SetVectorFF(&fx->rebound->normal, &seUp);
				}
				fx->gravity = 8190;
			}
			if( (fx = CreateSpecialEffect( FXTYPE_SPARKLYTRAIL, &garib->sprite->pos, &seUp, (player[pl].spawnScoreLevel*10)<<12, player[pl].spawnScoreLevel<<12, 0, 12288 )) )
			{
				SetFXColour(fx,255,255,0);
				if(fx->rebound)
				{
					SetVectorSS(&fx->rebound->point, &garib->sprite->pos);
					SetVectorFF(&fx->rebound->normal, &seUp);
				}
				fx->gravity = 4100;
			}

		  	PlaySample( genSfx[GEN_COLLECT_COIN], NULL, 0, SAMPLE_VOLUME, -1 );
			break;

		case EXTRALIFE_GARIB:
			if((player[pl].lives < 999) && (gameState.mode != DEMO_MODE))
				player[pl].lives++;

			CreatePickupEffect( pl, 0, 200, 0, 220, 220, 220 );
		  	PlaySample( genSfx[GEN_POWERUP], NULL, 0, SAMPLE_VOLUME, -1 );

			if( garib->fx )
 				DeallocateFX( garib->fx, 1 );

			if( gameState.multi == SINGLEPLAYER )
			{
				strcpy( arcadeHud.collectText->text, GAMESTRING(STR_EXTRA_LIFE) );
				arcadeHud.collectText->xPosTo = (arcadeHud.collectText->xPos < 0)?2049:2047;
				arcadeHud.collectText->speed = 327680;
			}
			break;

		case QUICKHOP_GARIB:
			GTInit( &player[pl].quickhop, 11 );

			CreatePickupEffect( pl, 255, 255, 255, 130, 130, 255 );
		  	PlaySample( genSfx[GEN_POWERUP], NULL, 0, SAMPLE_VOLUME, -1 );

			TogglePowerup(garib,0);

			if( gameState.multi == SINGLEPLAYER )
			{
				strcpy( arcadeHud.collectText->text, GAMESTRING(STR_QUICK_HOP) );
				arcadeHud.collectText->xPosTo = (arcadeHud.collectText->xPos < 0)?2049:2047;
				arcadeHud.collectText->speed = 327680;
			}
			break;

		case SLOWHOP_GARIB:
			// Slow down all OTHER players
			for( i=0; i<NUM_FROGS; i++ )
				if( i != pl )
				{
					GTInit( &player[i].slowhop, 6 );
					CreatePickupEffect( i, 0, 220, 0, 255, 0, 0 );
				}
			
			CreatePickupEffect( pl, 255, 255, 255, 0, 0, 255 );

			PlaySample( genSfx[GEN_POWERUP], NULL, 0, SAMPLE_VOLUME, -1 );

			TogglePowerup(garib,0);

			if( gameState.multi == SINGLEPLAYER )
			{
				strcpy( arcadeHud.collectText->text, GAMESTRING(STR_SLOW_HOP) );
				arcadeHud.collectText->xPosTo = (arcadeHud.collectText->xPos < 0)?2049:2047;
				arcadeHud.collectText->speed = 327680;
			}
			break;

		case AUTOHOP_GARIB:
			GTInit( &player[pl].autohop, 11 );
			
			CreatePickupEffect( pl, 255, 255, 255, 255, 0, 0 );
		  	PlaySample( genSfx[GEN_POWERUP], NULL, 0, SAMPLE_VOLUME, -1 );

			TogglePowerup(garib,0);

			if( gameState.multi == SINGLEPLAYER )
			{
				strcpy( arcadeHud.collectText->text, GAMESTRING(STR_AUTO_HOP) );
				arcadeHud.collectText->xPosTo = (arcadeHud.collectText->xPos < 0)?2049:2047;
				arcadeHud.collectText->speed = 327680;
			}
			break;

		case INVULNERABILITY_GARIB:
			GTInit( &player[pl].safe, 6 );

			CreatePickupEffect( pl, 255, 255, 255, 255, 0, 0 );
		  	PlaySample( genSfx[GEN_POWERUP], NULL, 0, SAMPLE_VOLUME, -1 );

			TogglePowerup(garib,0);

			if( gameState.multi == SINGLEPLAYER )
			{
				strcpy( arcadeHud.collectText->text, GAMESTRING(STR_INVULNERABILITY) );
				arcadeHud.collectText->xPosTo = (arcadeHud.collectText->xPos < 0)?2049:2047;
				arcadeHud.collectText->speed = 327680;
			}
			break;
	}

	if( gameState.multi != SINGLEPLAYER && multiplayerMode == MULTIMODE_BATTLE )
	{
		if( garib->fx )
 			DeallocateFX( garib->fx, 1 );

		SubGarib(garib);

 		numMultiItems--;
	}
	else if( !(garib->type == AUTOHOP_GARIB || garib->type == QUICKHOP_GARIB || garib->type == SLOWHOP_GARIB || garib->type == INVULNERABILITY_GARIB) )
	{
		SubGarib(garib);
	}
}


/*	--------------------------------------------------------------------------------
	Function		: InitGaribList
	Purpose			: initialises the garib linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void InitGaribList()
{
	garibList.count = 0;
	garibList.total = 0;
	garibList.maxCoins = 0;
	garibList.blocks = 0;
	garibList.array = NULL;
	garibList.head.next = garibList.head.prev = &garibList.head;
}


/*	--------------------------------------------------------------------------------
	Function		: AddGarib
	Purpose			: adds a garib to the garib list
	Parameters		: GARIB *
	Returns			: void
	Info			: 
*/
void AddGarib(GARIB *garib)
{
	if(garib->next == NULL)
	{
		garibList.count++;
		garibList.total++;
		if( garib->type == SILVERCOIN_GARIB )
			garibList.maxCoins++;
		else if( garib->type == GOLDCOIN_GARIB )
			garibList.maxCoins+=5;

		garib->prev = &garibList.head;
		garib->next = garibList.head.next;
		garibList.head.next->prev = garib;
		garibList.head.next = garib;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: SubGarib
	Purpose			: removes a garib from the garib list
	Parameters		: GARIB *
	Returns			: void
	Info			: 
*/
void SubGarib(GARIB *garib)
{
	if(garib->next == NULL || garibList.count<=0 )
	{
		utilPrintf("GARIB ERROR: invalid SubGarib()\n");
		return;
	}

	if( garib->sprite )
		DeallocateSprites( garib->sprite, 1 );

	garib->prev->next = garib->next;
	garib->next->prev = garib->prev;
	garib->next = NULL;
	garibList.count--;

	memset( garib, 0, sizeof(GARIB) );
}


/*	--------------------------------------------------------------------------------
	Function		: FreeGaribLinkedList
	Purpose			: frees the garib linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void FreeGaribList()
{
	if(garibList.count <= 0)
		return;

	while( garibList.head.next != &garibList.head )
		SubGarib(garibList.head.next);

	if( garibList.array )
		FREE( garibList.array );

	garibList.array = NULL;
	garibList.count = 0;
	garibList.blocks = 0;
}


/*	--------------------------------------------------------------------------------
	Function		: AllocGarib
	Purpose			: Nasty thing to dynamically allocate a garib in multiplayer
	Parameters		: 
	Returns			: void
	Info			: Does a search through the array for a free garib
*/
GARIB *AllocGarib( )
{
	int i=garibList.blocks-1;

	for( ; i-1; i-- )
		if( !garibList.array[i].next )
			return &garibList.array[i];

	return NULL;
}

/*	--------------------------------------------------------------------------------
	Function		: AllocNmeBlock
	Purpose			: Initialise a fixed number of enemies
	Parameters		: 
	Returns			: void
	Info			: 
*/
void AllocGaribBlock( int num )
{
	if( !garibList.array && num > 0 )
		garibList.array = (GARIB *)MALLOC0(sizeof(GARIB)*num);

	garibList.count = 0;
	garibList.blocks = num;
}


/*	--------------------------------------------------------------------------------
	Function		: CreateNewGarib
	Purpose			: creates a new garib
	Parameters		: VECTOR,int
	Returns			: GARIB *
	Info			: 
*/
GARIB *CreateNewGarib(SVECTOR pos,int type)
{
	GARIB *garib;

#ifndef COINS_IN_ARCADE_MODE
	if( (type == SILVERCOIN_GARIB || type == GOLDCOIN_GARIB) && gameState.single == ARCADE_MODE )
		return NULL;
#endif

	// If singleplayer then no dynamic garib stuff so get the next one in the array
	if( gameState.multi == SINGLEPLAYER )
	{
		if( garibList.count >= garibList.blocks || !(garib = &garibList.array[garibList.count]) )
			return NULL;
	}
	else // Else have to search for a free one. Meh.
	{
		if( garibList.count >= garibList.blocks || !(garib = AllocGarib()) )
			return NULL;
	}

	SetVectorSS(&garib->pos,&pos);

	garib->type = type;
	garib->active = 1;
//	garib->scale = 0;
//	garib->scaleAim = 4096;

	if( type == GOLDCOIN_GARIB || type == SILVERCOIN_GARIB )
	{
		// initialise garib sprite
		if (!(garib->sprite = AllocateSprites(1)))
		{
			utilPrintf("couldn't allocate sprite for garib!\n");
			FREE(garib);
			return NULL;
		}

		garib->sprite->pos.vx = pos.vx;
		garib->sprite->pos.vy = pos.vy;
		garib->sprite->pos.vz = pos.vz;

		if(cheatCombos[CHEAT_MAD_GARIBS].state)
			garib->sprite->texture = FindTexture("RGARIB01");
		else
			garib->sprite->texture = FindTexture("SCOIN0001");

		if( type == GOLDCOIN_GARIB )
		{
			garib->sprite->r = 255;
			garib->sprite->g = 255;
			garib->sprite->b = 0;
		}
		else
		{
			garib->sprite->r = 255;
			garib->sprite->g = 255;
			garib->sprite->b = 255;
		}

		garib->sprite->a = 255;
		garib->sprite->scaleX = garib->sprite->scaleY = 0;

		garib->sprite->angle = Random(4096);
		garib->sprite->angleInc = 50;
		
		garib->sprite->offsetX = -16;
		garib->sprite->offsetY = -16;

		garib->sprite->flags = 0;
		garib->sprite->a = 200;
	}
	else
	{
		unsigned char r, g, b;

		switch( type )
		{
		case EXTRALIFE_GARIB: sprintf( pickupObjName, "bfly04.obe" ); r=0; g=200; b=0; break;
		case QUICKHOP_GARIB: sprintf( pickupObjName, "bfly02.obe" ); r=130; g=130; b=255; break;
		case INVULNERABILITY_GARIB: sprintf( pickupObjName, "bfly03.obe" ); r=255; g=0; b=0; break;
		case SLOWHOP_GARIB: sprintf( pickupObjName, "bfly05.obe" ); r=255; g=255; b=0; break;
		case AUTOHOP_GARIB: sprintf( pickupObjName, "bfly.obe" ); r=255; g=80; b=255; break;
		default: 
			utilPrintf( "Error creating garib - %i is not a known garib type!\n", type );
			return NULL;
		}

		garib->fx = CreateSpecialEffectDirect( FXTYPE_HEALTHFLY, &garib->pos, &upVec, 4096, 4096, 246, 0 );
		SetFXColour( garib->fx, r, g, b );
	}

	AddGarib(garib);
	return garib;
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateGaribs
	Purpose			: updates the garibs
	Parameters		: 
	Returns			: void
	Info			: 
*/
void UpdateGaribs()
{
	GARIB *garib = NULL;
	SPECFX *fx;

	if((gameState.multi == SINGLEPLAYER ) && (gameState.mode != LEVELCOMPLETE_MODE))
	{
		if((pauseFlag == 2) && (arcadeHud.collectText->xPos == arcadeHud.collectText->xPosTo))
		{
			arcadeHud.collectText->draw = 0;
			pauseFlag = 0;
		}
		else if(pauseFlag != 2)
		{
			arcadeHud.collectText->draw = 1;

			if( pauseTimer.time )
			{
				arcadeHud.collectText->speed = 0;
				GTUpdate( &pauseTimer, -1 );
				if( !pauseTimer.time )
				{
					arcadeHud.collectText->speed = 327680;
					if(arcadeHud.collectText->xPosTo == 2049)
						arcadeHud.collectText->xPosTo = 4096+2048;
					else
						arcadeHud.collectText->xPosTo = -2048;
					pauseFlag = 2;
				}
			}
			else if( !pauseFlag && (abs(arcadeHud.collectText->xPos - 2048) == 1))
			{
				GTInit( &pauseTimer, 2 );
				arcadeHud.collectText->speed = 0;
				pauseFlag = 1;
			}
		}
	}

	// update garib scales and calculate distance from Frog
	for(garib = garibList.head.next; garib != &garibList.head; garib = garib->next)
	{
		if(garib->active == NO)
			continue;
		else if(garib->active > 1)
			garib->active--;

		if( garib->sprite )
		{
//#ifdef PC_VERSION
			garib->sprite->scaleX = 64;
			garib->sprite->scaleY = 64;
//#else
//			garib->sprite->scaleX = 96;
//			garib->sprite->scaleY = 96;
//#endif
		}
		else if( garib->fx )
		{
			garib->dropSpeed -= max(gameSpeed>>12, 1);

			if( garib->dropSpeed <= 0 )
			{
				if( (fx=CreateSpecialEffect(FXTYPE_TWINKLE, &garib->fx->act[0]->actor->position, &garib->fx->normal, 123840, 0, 0, 16384)) )
				{
					fx->tilt = 4096;
					SetFXColour( fx, garib->fx->r, garib->fx->g, garib->fx->b );
				}
				garib->dropSpeed = 30;
			}
		}

		// Drop Garibs.............
//		if ( garib->gameTile && garib->sprite )
//		{			
//			SetVectorSS(&actualPos, &garib->gameTile->centre);
//			actualPos.vy += 100;
//			SubVectorFSS( &fwd, &actualPos, &garib->pos );
//			MakeUnit( &fwd );
//			ScaleVector( &fwd, garib->dropSpeed );
//			AddToVectorSF( &garib->pos, &fwd );
//			SetVectorSS(&garib->sprite->pos, &garib->pos );
//		}
	}
}

void ReactivatePowerups( )
{
	GARIB *garib;

	for(garib = garibList.head.next; garib != &garibList.head; garib = garib->next)
	{
		if( garib->type == AUTOHOP_GARIB || garib->type == QUICKHOP_GARIB || garib->type == SLOWHOP_GARIB || garib->type == INVULNERABILITY_GARIB )
		{
			TogglePowerup( garib, 1 );
		}
	}
}

//void DropGaribToTile(GARIB *garib, GAMETILE *tile, short dropSpeed)
//{
//	garib->gameTile = tile;
//	garib->dropSpeed = dropSpeed;
//}
