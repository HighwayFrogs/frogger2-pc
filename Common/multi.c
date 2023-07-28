/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: multi.c
	Programmer	: Jim Hubbard
	Date		: 25/3/00

----------------------------------------------------------------------------------------------- */

#include <islpad.h>
#include <islutil.h>

#include "define.h"
#include "specfx.h"
#include "types.h"
#include "general.h"
#include "enemies.h"
#include "platform.h"
#include "multi.h"
#include "textover.h"
#include <islmem.h>
#include "cam.h"
#include "layout.h"
#include "game.h"
#include "frogger.h"
#include "frontend.h"
#include "frogmove.h"
#include "collect.h"
#include "map.h"
#include "hud.h"
#include "babyfrog.h"
#include "Main.h"
#include "menus.h"

#ifdef PC_VERSION
#include <mdx.h>
#include <pcmisc.h>
#include <pcaudio.h>
#include <pcsprite.h>
#include <net.h>
#endif

#ifdef PSX_VERSION
#include "textures.h"
#include "audio.h"

extern char mpl0[];
extern char mpl1[];
extern char mpl2[];
extern char mpl3[];
extern char mpl4[];
extern char mpl5[];
extern char mpl6[];
extern char mpl7[];

#endif

#define BABY_UID	255

extern int pauseController;
char penalStr[3] = "+1";

int multiplayerMode;
long started = 0;
unsigned long numMultiItems=0;
signed char gameWinner=-1, matchWinner=-1;
char multiLevelIndex[MAX_WORLDS] = {1, 3, 3, 3, 3, 3, 3, 5, 10};

#ifdef PSX_VERSION
unsigned short cubePalettes[4];
#endif

char multiGameTypes[MAX_WORLDS] = 
{
	0,
	MULTIMODE_RACE,
	0,
	MULTIMODE_RACE,
	0,
	MULTIMODE_BATTLE,
	0,
	MULTIMODE_COLLECT,
};

TIMER powerupTimer;
TIMER multiTimer;
TIMER endTimer;

void UpdateRace( );
void UpdateBattle( );
void UpdateCollect( );

void CalcCollectCamera( FVECTOR *target );
void CalcBattleCamera( FVECTOR *target );
void CalcRaceCircuitCamera( FVECTOR *target );
void CalcRaceStraightCamera( FVECTOR *target );

void BattleProcessController( int pl );

int AddBattleTrailNode( int i );
void SubBattleTrailNode( BATTLENODE *p );

void CollectRespawn( int pl );
void FaceFrogToCentre(int i, int toggle);


MPINFO mpl[4];

#define MAX_BATTLENODES 256
BATTLENODELIST nodePool;

fixed initialCamHeight = 0;

/*	--------------------------------------------------------------------------------
	Function		: UpdateRace
	Purpose			: Do game mechanics for mulitplayer race mode
	Parameters		: 
	Returns			: 
	Info			:
*/
void UpdateRace( )
{
	int i, j;//, score;

	// Wait for all players to be on the start/finish line
	if( !started )
	{
		// Hack to make camera move up only
		initialCamHeight = camSource.vy;

		for( i=0,j=0; i<NUM_FROGS; i++ )
			if( currTile[i]->state == TILESTATE_FROGGER1AREA )
			{
				j++;
				player[i].canJump = 0;
			}
			else
			{
				FVECTOR fwd;
				SetVectorFF( &fwd, &currTile[i]->dirVector[(frogFacing[i]+camFacing[i])&3] );
				// Hop forward to starting line 
				if( player[i].canJump )
					HopFrogToTile( FindJoinedTileByDirection(currTile[i],&fwd), i );
			}

		GTInit( &multiTimer, 0);
		if( j==NUM_FROGS )
		{
			GTInit( &endTimer, 0 );
			GTInit( &multiTimer, 6 );
			started = 1;
		}
	}

	// When all players are ready, start a countdown
	if( multiTimer.time)
	{
		int oldTime = multiTimer.time;
		GTUpdate( &multiTimer, -1 );
		if(multiTimer.time==1)
		{
			if( started != 2 )
			{
				PlaySample( FindSample(utilStr2CRC("racehorngo")), NULL, 0, SAMPLE_VOLUME, -1 );
				started = 2;
				for( i=0; i<NUM_FROGS; i++ )
					player[i].canJump = 1;
			}
		}
		else if( !multiTimer.time )
			PlayVoice( Random(NUM_FROGS), "frogletsgo" );
		else if( multiTimer.time != oldTime && multiTimer.time < 5 )
			PlaySample( FindSample(utilStr2CRC("racehorn")), NULL, 0, SAMPLE_VOLUME, -1 );
	}
	else if( endTimer.time ) // If finished the race then wait before replaying
	{
		GTUpdate( &endTimer, -1 );

		if( !endTimer.time )
			StartMultiWinGame( );

		return;
	}

	// Mini-loop for respawning before race start
	if( started != 2 )
	{
		for( i=0; i<NUM_FROGS; i++ )
		{
			// If waiting to respawn, update timer and check for respawn timeout
			if( player[i].dead.time )
			{
				GTUpdate( &player[i].dead, -1 );

				if( !player[i].dead.time )
				{
					RaceRespawn(i);
					frog[i]->draw = 1;
					GTInit( &player[i].safe, 3 );
				}
			}
		}
		return;
	}

	// Proper game loop thing
	for( i=0; i<NUM_FROGS; i++ )
	{
		if(mpl[i].penalText->draw)
		{
			SlideTextOverlayToPos(mpl[i].penalText,backTextX[i],backTextY[i],mpl[i].scrX,mpl[i].scrY,40);	
			if((mpl[i].penalText->xPos == backTextX[i]) && (mpl[i].penalText->yPos == backTextY[i]))
				mpl[i].penalText->draw = 0;
		}


		if( !mpl[i].ready )
		{
			// Increase timer if not finished
			mpl[i].timer += actFrameCount - lastActFrameCount;

			// If waiting to respawn, update timer and check for respawn timeout
			if( player[i].dead.time )
			{
				GTUpdate( &player[i].dead, -1 );

				if( !player[i].dead.time )
				{
					RaceRespawn(i);
					frog[i]->draw = 1;
					GTInit( &player[i].safe, 3 );
				}
				else continue;
			}

			// Initialise quickhop for everyone but the lead player
			if( i != playerFocus )
			{
				GTInit( &player[i].quickhop, 3 );
			}

			// Kill frogs that have fallen off screen
			if( (frameCount > 50) && !(IsPointVisible(&frog[i]->actor->position)) )
			{
				KillMPFrog(i);
			}
			else if((currTile[i]->state >= TILESTATE_FROGGER1AREA && currTile[i]->state <= TILESTATE_FROGGER4AREA) || (currTile[i]->state == TILESTATE_FINISHLINE))
			{
				// If last tile state was type 4 and this is type 1 then we've got around another lap
				// sbond changed but seems to work!!!!!!!!!!!!!!
//				if( currTile[i]->state == TILESTATE_FROGGER1AREA && mpl[i].lasttile != TILESTATE_FROGGER1AREA )
				if( currTile[i]->state == TILESTATE_FINISHLINE && mpl[i].lasttile != TILESTATE_FINISHLINE )
				{
					mpl[i].lap++;
					mpl[i].lasttile = currTile[i]->state;
					mpl[i].check = 0;

					// Start of a new lap - if more then the defined number of maps for the race then this player is the winner
					if( mpl[i].lap >= 1)//MULTI_RACE_NUMLAPS )
					{
						mpl[i].ready = 1;
						player[i].canJump = 0;
						PlayerRaceWin( i );
					}
				}
				// Else if we've just got to another checkpoint (unlimited repetitions of 2,3,4. Tilestate 1 is used to signal lap changes)
				else if( currTile[i]->state == mpl[i].lasttile+1 || (mpl[i].lasttile == TILESTATE_FROGGER4AREA && currTile[i]->state == TILESTATE_FROGGER2AREA) )
				{
					mpl[i].check++;
					mpl[i].lasttile = currTile[i]->state;
				}
			}
		}
	}

	// Check players for finish
	for( i=0; i<NUM_FROGS; i++ )
		if( !mpl[i].ready ) break;

	// If all players finished, check win conditions
	if( i==NUM_FROGS )
	{
		unsigned long best, time, winner, draw;
		// Find best time
		for( j=0,best=999999999; j<NUM_FROGS; j++ )
		{
			time = mpl[j].timer + mpl[j].penalty;
			if( time < best )
			{
				best = time;
				winner = j;
			}
		}
		// If more then one on best time, draw
		for( j=0,draw=0; j<NUM_FROGS; j++ )
		{
			time = mpl[j].timer + mpl[j].penalty;
			if( j!=winner && time == best )
				draw=1;
		}

		if( draw )
		{
			gameWinner = MULTI_ROUND_DRAW;
		}
		else
		{
			PlayVoice( winner, "frogokay" );
			gameWinner = winner;
			mpl[winner].wins++;
		}

		GTInit( &endTimer, 2 );
		controlCamera = 1;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateBattle
	Purpose			: Do game mechanics for lightcycles mode
	Parameters		: 
	Returns			: 
	Info			:
*/
void UpdateBattle( )
{
	int i, j, res, dead=0,topScore,numTop;

	if( !started )
	{
		ENEMY *nme;
		int count;
		for( count=1,nme=enemyList.head.next; nme!=&enemyList.head; nme=nme->next )
			if( nme->flags & ENEMY_NEW_BATTLEMODE )
			{
				MoveEnemyToNode( nme, count );
				nme->isWaiting = -1;
				nme->inTile = 0;
				count++;
			}

		for( i=0; i<NUM_FROGS; i++ )
		{
			player[i].idleEnable = 0;
			// Face all frogs towards the centre of the map
			FaceFrogToCentre(i,1);
		}

		GTInit( &endTimer, 0 );
		GTInit( &powerupTimer, Random(4)+3 );
		GTInit( &multiTimer, 6 );
		started = 1;
	}

	// When all players are ready, start a countdown
	if( multiTimer.time)
	{
		int oldTime = multiTimer.time;

		GTUpdate( &multiTimer, -1 );
		if(multiTimer.time==1)
		{
			if( started != 2 )
			{
				ENEMY *nme;

				PlaySample( FindSample(utilStr2CRC("racehorngo")), NULL, 0, SAMPLE_VOLUME, -1 );
				started = 2;
				for( i=0; i<NUM_FROGS; i++ )
					player[i].canJump = 1;
				for( nme=enemyList.head.next; nme!=&enemyList.head; nme=nme->next )
					if( nme->flags & ENEMY_NEW_BATTLEMODE )
						nme->isWaiting = 0;
			}
		}
		else if( !multiTimer.time )
		{
			PlayVoice( Random(NUM_FROGS), "frogletsgo" );
		}
		else if( multiTimer.time != oldTime && multiTimer.time < 5 )
		{
			PlaySample( FindSample(utilStr2CRC("racehorn")), NULL, 0, SAMPLE_VOLUME, -1 );
		}
	}
	if((multiTimer.time <= 1) && (endTimer.time))
	{
		GTUpdate( &endTimer, -1 );

		if( !endTimer.time )
			StartMultiWinGame( );

		return;
	}

	if( started )
	{
		GTUpdate( &powerupTimer, -1 );
		if( !powerupTimer.time )
		{
			if( numMultiItems < MULTI_BATTLE_MAXITEMS )
			{
				FVECTOR pos;
				SVECTOR tp;
				GARIB *g;
				int r;
				
				do{ r=Random(tileCount); } while( firstTile[r].state == TILESTATE_BARRED || (firstTile[r].state >= TILESTATE_FROGGER1AREA && firstTile[r].state <= TILESTATE_FROGGER4AREA) );

				if( !Random(5) )
				{
					SetVectorFF( &pos, &firstTile[r].normal );
					ScaleVector( &pos, 150 );
					AddToVectorFS( &pos, &firstTile[r].centre );
					SetVectorSF( &tp, &pos );

					switch( Random(3) )
					{
					case 0: g = CreateNewGarib( tp, GOLDCOIN_GARIB ); break;
					case 1: g = CreateNewGarib( tp, QUICKHOP_GARIB ); break;
					case 2: g = CreateNewGarib( tp, SLOWHOP_GARIB ); break;
					}
				}
				else
				{
					SetVectorFF( &pos, &firstTile[r].normal );
					ScaleVector( &pos, 200 );
					AddToVectorFS( &pos, &firstTile[r].centre );
					SetVectorSF( &tp, &pos );

					g = CreateNewGarib( tp, SILVERCOIN_GARIB );
					//DropGaribToTile( g, &firstTile[r], 50 );
				}

				numMultiItems++;
			}

			GTInit( &powerupTimer, (Random(4)+3) );
		}
	}

	for( i=0; i<NUM_FROGS; i++ )
	{
		// Change desired frog facing
		BattleProcessController(i);

		if( started == 2 )
		{
			BATTLENODE *node=NULL, *temp;
			// If the frog can move then continue in current direction
			if( player[i].canJump && !(player[i].frogState & FROGSTATUS_ISDEAD) )
			{
				int tf=1;

				res = MoveToRequestedDestination( ((frogFacing[i] - camFacing[i]) & 3), i );
				player[i].canJump = 0;

				// Add a new node to the trail
				if( res )
				{
					tf = AddBattleTrailNode( i );
					frogAnimSpeed /= 2;
					frogAnimSpeed2 /= 2;
					AnimateFrogHop( frogFacing[i], i );
					frogAnimSpeed *= 2;
					frogAnimSpeed2 *= 2;
				}

				if( !tf || !res )
				{
					for( j=0; j<NUM_FROGS; j++ )
						if( i!=j && !(player[j].frogState & FROGSTATUS_ISDEAD) && !(player[i].frogState & FROGSTATUS_ISDEAD) && 
							(currTile[j] == currTile[i] || (destTile[j] && destTile[i] && destTile[j] == destTile[i]) ) )
						{
							KillMPFrog(j);
						}

					KillMPFrog(i);
				}
			}

			if( mpl[i].path ) 
			{
				int fo = (256*gameSpeed)>>12;

				if( !(player[i].frogState & FROGSTATUS_ISDEAD) )
				{
					mpl[i].score++;
					node = mpl[i].path->next;

					// Update trail. For 0->length, increase alpha to limit. 
					for( j=0; node && j<mpl[i].trail; j++,node = node->next )
						if( node->fx && node->fx->actor->size.vy < 4096-fo )
							node->fx->actor->size.vy += fo;

					fo = (128*gameSpeed)>>12;
					// For length->endoflist, fade out and make not deadly
					while( node )
					{
						temp = node->next;
//						node->tile->state = TILESTATE_NORMAL;

						if( node->fx )
						{
							if( node->fx->actor->size.vy <= fo )
								SubBattleTrailNode( node );
							else
								node->fx->actor->size.vy -= fo;
						}

						node = temp;
					}
				}
				else
				{
					node = mpl[i].path;
					if(node->next)
						if( node->fx && node->fx->actor->size.vy < 4096-fo )
							node->fx->actor->size.vy += fo;



					fo = (128*gameSpeed)>>12;

					while( node->next ) node = node->next;

//					node->tile->state = TILESTATE_NORMAL;

					if( node->fx )
					{
						if( node->fx->actor->size.vy <= fo )
						{
							SubBattleTrailNode( node );
							if( node == mpl[i].path )
								mpl[i].path = NULL;
						}
						else
							node->fx->actor->size.vy -= fo;
					}
				}
			}

			for( j=0; j<NUM_FROGS; j++ )
				if( i!=j && !(player[j].frogState & FROGSTATUS_ISDEAD) && !(player[i].frogState & FROGSTATUS_ISDEAD) && 
					(currTile[j] == currTile[i] || (destTile[j] && destTile[i] && destTile[j] == destTile[i]) ) )
				{
					KillMPFrog(i);
					KillMPFrog(j);
				}

			if( player[i].frogState & FROGSTATUS_ISDEAD )
				dead++;
		}
	}

	// The last player alive wins!
	if((NUM_FROGS > 1) && (dead == NUM_FROGS-1))
	{
		topScore = numTop = 0;
		for( i=0; i<NUM_FROGS; i++ )
		{
			if(!(player[i].frogState & FROGSTATUS_ISDEAD))
				mpl[i].score++;
		}

		for( i=0; i<NUM_FROGS; i++ )
		{
			if(mpl[i].score > topScore)
				topScore = mpl[i].score;
		}

		for( i=0; i<NUM_FROGS; i++ )
		{
			if(abs(mpl[i].score - topScore) < 2)
			{
				mpl[i].score = topScore;
				numTop++;
				gameWinner = i;
			}
		}

		if(numTop > 1)
		{
			controlCamera = 1;
			gameWinner = MULTI_ROUND_DRAW;
		}
		else
		{
			PlayVoice( gameWinner, "frogokay" );
			mpl[gameWinner].wins++;
		}

		GTInit( &endTimer, 2 );
	}
	else if(dead == NUM_FROGS)
	{
		gameWinner = MULTI_ROUND_DRAW;
		GTInit( &endTimer, 2 );
	}
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateCollect
	Purpose			: Do game mechanics for Frogger1 collection mode
	Parameters		: 
	Returns			: 
	Info			:
*/
void UpdateCollect( )
{
	int i, total=0, best, winner;

	if( !started )
	{
		for( i=0; i<NUM_FROGS; i++ )
		{
			player[i].canJump = 0;
			Orientate( &frog[i]->actor->qRot, &outVec, &upVec );
		}

		GTInit( &endTimer, 0 );
		GTInit( &powerupTimer, Random(4)+3 );
		GTInit( &multiTimer, 6 );
		started = 1;
	}

	// When all players are ready, start a countdown
	if( multiTimer.time)
	{
		int oldTime = multiTimer.time;

		GTUpdate( &multiTimer, -1 );
		if(multiTimer.time==1)
		{
			if( started != 2 )
			{
				PlaySample( FindSample(utilStr2CRC("racehorngo")), NULL, 0, SAMPLE_VOLUME, -1 );
				started = 2;
				for( i=0; i<NUM_FROGS; i++ )
					player[i].canJump = 1;
			}
		}
		else if( !multiTimer.time )
			PlayVoice( Random(NUM_FROGS), "frogletsgo" );
		else if( multiTimer.time != oldTime && multiTimer.time < 5 )
			PlaySample( FindSample(utilStr2CRC("racehorn")), NULL, 0, SAMPLE_VOLUME, -1 );
	}
	else if( endTimer.time )
	{
		GTUpdate( &endTimer, -1 );

		if( !endTimer.time )
			StartMultiWinGame( );

		return;
	}

	if( started == 2 )
	{
		for( i=0; i<NUM_FROGS; i++ )
		{
			// Check for collection of objects and update total
			total += mpl[i].score;

			// If waiting to respawn, update timer and check for respawn timeout
			if( player[i].dead.time )
			{
				GTUpdate( &player[i].dead, -1 );

				if( !player[i].dead.time )
				{
					player[i].frogState &= ~FROGSTATUS_ISDEAD;
					CollectRespawn(i);
					frog[i]->draw = 1;
				}
				else continue;
			}

			// Respawn on start tile when dead, after countdown
			if( player[i].frogState & FROGSTATUS_ISDEAD )
				GTInit( &player[i].dead, 4 );
		}

		// Find the highest scoring frog
		for( i=0,best=0,winner=-1; i<NUM_FROGS; i++ )
			if( mpl[i].score >= best )
			{
				best = mpl[i].score;
				winner = i;
			}

		// Check if any other player can possibly win (if score difference is less than number of remaining items)
		for( i=0; i<NUM_FROGS; i++ )
			if( i != winner && (mpl[winner].score - mpl[i].score) <= (numBabies - total) )
			{
				winner=-1;
				break;
			}

		// If the leading frog has an unassailable lead then he wins
		if( winner != -1 )
		{
			PlayVoice( winner, "frogokay" );
			gameWinner = winner;
			mpl[winner].wins++;
			GTInit( &endTimer, 2 );
		}
		else if( total == numBabies ) // If all objects collected then declare draw
		{
			gameWinner = MULTI_ROUND_DRAW;
			GTInit( &endTimer, 2 );
		}
	}
}


/*	--------------------------------------------------------------------------------
	Function		: RunMultiplayer
	Purpose			: Call update function for each game type
	Parameters		: player
	Returns			: void
	Info			:
*/
#ifdef PSX_VERSION
int mpltextx = 250;
int mpltexty = -150;
#elif PC_VERSION
int mpltextx = 0;
int mpltexty = -200;
char mplNameStr[4][64];
#endif



void RunMultiplayer( )
{
	int i;

	UpDateMultiplayerInfo();

	switch( multiplayerMode )
	{
	case MULTIMODE_RACE:
		UpdateRace( );
		break;
	case MULTIMODE_BATTLE:
		UpdateBattle( );
		break;
	case MULTIMODE_COLLECT:
		UpdateCollect( );
		break;
	}

	for( i=0; i<NUM_FROGS; i++ )
	{

#ifdef PSX_VERSION
		//bb - update depth shifts for frog on frog
		int p=player[i].frogon;
		while((p != -1) && (p != 255))
		{
			frog[i]->depthShift -= 10;
			p = player[p].frogon;
		}
#endif

		if(multiTimer.time > 1)
			mpl[i].numText->draw = 1;
		else
			mpl[i].numText->draw = 0;
		TransformPoint(&frog[i]->actor->position,&mpl[i].numText->xPos,&mpl[i].numText->yPos);
		mpl[i].numText->xPos += mpltextx;
		mpl[i].numText->yPos += mpltexty;
		if( mpl[i].wins == 3 )
		{
			if(matchWinner == -1)
			{
				GTInit( &endTimer, 2 );
				matchWinner = i;
			}
			break;
		}
	}
}


/*	--------------------------------------------------------------------------------
	Function		: RaceRespawn
	Purpose			: Teleport frog to a non-dead player
	Parameters		: 
	Returns			: 
	Info			: Oh dear. Respawn near the lead frog if available, or search for another if not
*/
void RaceRespawn( int pl )
{
	int j, respawn=0,noGood;
	GAMETILE *tile;
	FVECTOR diff;

	player[pl].deathBy = 0;
	player[pl].frogState &= ~FROGSTATUS_ISDEAD;
	actorAnimate( frog[pl]->actor, FROG_ANIM_BREATHE, YES, NO, 64, 0 );

	if( playerFocus != pl && !(player[playerFocus].frogState & FROGSTATUS_ISDEAD) && IsPointVisible(&frog[playerFocus]->actor->position) )
		respawn = playerFocus;
	else
		for( j=0; j<NUM_FROGS; j++ )
			if( j != pl && j != playerFocus && !(player[j].frogState & FROGSTATUS_ISDEAD) && IsPointVisible(&frog[j]->actor->position) )
			{
				respawn = j;
				break;
			}

	SubVectorFSS( &diff, &frog[pl]->actor->position, &frog[respawn]->actor->position );
	MakeUnit(&diff);

	// Find a tile to teleport to - first try a tile next to the target, then on the nearest "safe" tile, then on the target frog if that fails
	tile = FindJoinedTileByDirectionAndType( currTile[respawn], &diff, TILESTATE_NORMAL );
	if(tile)
	{
		for(j = 0;j < NUM_FROGS;j++)
		{
			if((j != pl) && (currTile[j] == tile))
			{
				tile = NULL;
				break;
			}
		}
	}
	if( !tile )
	{
		GAMETILE *t, *target = NULL;
		long dist, best = 99999999, i;

		t = firstTile;
		for ( i = 0; i < tileCount; i++, t++ )
		//for( t = firstTile; t != NULL; t = t->next )
		{
			noGood = NO;
			if((t->state == TILESTATE_SAFE) || (t->state == TILESTATE_NORMAL) || (t->state == TILESTATE_FROGGER1AREA) || (t->state == TILESTATE_FROGGER2AREA) || (t->state == TILESTATE_FROGGER3AREA) || (t->state == TILESTATE_FROGGER4AREA))
			{
				for(j = 0;j < NUM_FROGS;j++)
				{
					if((j != pl) && (currTile[j] == t))
					{
						noGood = YES;
						break;
					}
				}
				if(noGood)
					continue;
				dist = DistanceBetweenPointsSS( &frog[respawn]->actor->position, &t->centre ) + DistanceBetweenPointsSS( &frog[pl]->actor->position, &t->centre );
				if( dist < best && IsPointVisible(&t->centre) )
				{
					target = t;
					best = dist;
				}
			}
		}

		if( target )
			tile = target;
		else
			tile = currTile[respawn];
	}

	camFacing[pl] = camFacing[respawn];
	frogFacing[pl] = frogFacing[respawn];
	prevCamFacing[pl] = prevCamFacing[respawn];

	TeleportActorToTile(frog[pl],tile,pl);

	CreateRespawnEffect( tile, pl );

	// Update progress info
	mpl[pl].check = mpl[respawn].check;
	mpl[pl].lap = mpl[respawn].lap;

	// If other frog has finished then we have to be 1 checkpoint behind
	if( mpl[respawn].ready )
		mpl[pl].lasttile = TILESTATE_FROGGER4AREA;		
	else
		mpl[pl].lasttile = mpl[respawn].lasttile;

	lastTile[pl] = tile;
	destTile[pl] = tile;

	CheckForFrogOn(pl,tile);
}


/*	--------------------------------------------------------------------------------
	Function		: CollectRespawn
	Purpose			: Teleport frog to start
	Parameters		: 
	Returns			: 
	Info			: Just respawn on the proper respawn tile
*/
void CollectRespawn( int pl )
{
	TeleportActorToTile(frog[pl],gTStart[pl],pl);
	actorAnimate( frog[pl]->actor, FROG_ANIM_BREATHE, YES, NO, FROG_BREATHE_SPEED, 0 );

	lastTile[pl] = gTStart[pl];
	destTile[pl] = gTStart[pl];

	CreateRespawnEffect( gTStart[pl], pl );
	CheckForFrogOn(pl,currTile[pl]);
}


/*	--------------------------------------------------------------------------------
	Function		: Battle process controller
	Purpose			: Just change direction, and maybe use tongue
	Parameters		: player
	Returns			: void
	Info			:
*/
void BattleProcessController( int pl )
{
	u16 button[4];

	button[pl] = padData.debounce[pl];

	// Change frog facing on direction keys
	player[pl].frogState &= ~FROGSTATUS_ALLHOPFLAGS;

	if( button[pl] & PAD_UP )
		frogFacing[pl] = (camFacing[pl] + MOVE_UP) & 3;
	else if( button[pl] & PAD_DOWN )
		frogFacing[pl] = (camFacing[pl] + MOVE_DOWN) & 3;
	else if( button[pl] & PAD_LEFT )
		frogFacing[pl] = (camFacing[pl] + MOVE_LEFT) & 3;
	else if( button[pl] & PAD_RIGHT )
		frogFacing[pl] = (camFacing[pl] + MOVE_RIGHT) & 3;

	nextFrogFacing[pl] = frogFacing[pl];

	if((button[pl] & PAD_START) && ((player[pl].frogState & FROGSTATUS_ISDEAD) == 0))
	{
		pauseController = pl;
		StartPauseMenu();
	}
}

/*	--------------------------------------------------------------------------------
	Function		: AddBattleTrailNode
	Purpose			: Add an aipathnode to the trail and put an effect on it
	Parameters		: player
	Returns			: void
	Info			:
*/
int AddBattleTrailNode( int i )
{
	BATTLENODE *node;
	char name[16];

	if( destTile[i]->state >= TILESTATE_FROGGER1AREA && destTile[i]->state <= TILESTATE_FROGGER4AREA )
		return 0;

	// Return if allocation is impossible for any reason
	if( nodePool.stackPtr < 1 )
		return 1;

	node = nodePool.stack[nodePool.stackPtr--];

	node->prev = NULL;
	node->next = mpl[i].path;
	if( mpl[i].path ) mpl[i].path->prev = node;
	mpl[i].path = node;

	nodePool.count++;

	// Found a pathnode, continue adding it
	node->tile = destTile[i];
	node->tile->state = TILESTATE_FROGGER1AREA + i;

	node->fx->draw = 1;
	SetVectorSS( &node->fx->actor->position, &node->tile->centre );
	node->fx->actor->size.vy = 10;

#ifdef PC_VERSION
	sprintf(name,"mplt%i.bmp",player[i].character);
	((MDX_ACTOR*)node->fx->actor->actualActor)->overrideTex = GetTexEntryFromCRC(UpdateCRC(name));
#else
	node->fx->actor->clutOverride = cubePalettes[i];
#endif

	return 1;
}


void SubBattleTrailNode( BATTLENODE *p )
{
	if( p->prev ) p->prev->next = p->next;
	if( p->next ) p->next->prev	= p->prev;
	p->next = p->prev = NULL;

	p->fx->draw = 0;
	if(p->tile)
		p->tile->state = TILESTATE_NORMAL;

	p->tile = NULL;

	nodePool.count--;
	nodePool.stack[++nodePool.stackPtr] = p;
}



/*	--------------------------------------------------------------------------------
	Function		: FaceFrogInwards
	Purpose			: make a player face the middle of the battle arena
	Parameters		: player
	Returns			: void
	Info			: toggle switches between face in/out
*/
void FaceFrogToCentre(int pl, int toggle)
{
	FVECTOR dir;
	fixed dp, best=-1;
	short facing=0, i;

	// Find direction to centre
	// NOTE: These can be done in one go, but I can't be bothered right now
	SetVectorFS( &dir, &frog[pl]->actor->position );
	MakeUnit( &dir );
	ScaleVector( &dir, toggle );

	for( i=0; i<4; i++ )
	{
		dp = DotProductFF( &dir, &currTile[pl]->dirVector[i] );
		if( dp > best )
		{
			facing = i;
			best = dp;
		}
	}

	frogFacing[pl] = facing;
	OrientateSS( &frog[pl]->actor->qRot, &currTile[pl]->dirVector[frogFacing[pl]], &currTile[pl]->normal );
}


/*	--------------------------------------------------------------------------------
	Function		: PickupBabyFrogMulti
	Purpose			: Player has collected baby frog in a multiplayer game
	Parameters		: ACTOR2
	Returns			: void
	Info			: Probably not an actual baby frog, more likely a flag or other level collectable
*/
void PickupBabyFrogMulti( ENEMY *baby, int pl )
{
	int i,n;

	for( i=0; i<numBabies; i++ ) if( babyList[i].baby == baby->nmeActor ) break;

	if( i==numBabies || babyList[i].isSaved ) return;

	if( baby->path->nodes->offset2 )
		n = ((baby->path->nodes->offset2>>12)/SCALE)-1;
	else
		n = i;

	if( n > numBabies )
		n = 0;

	lastBabySaved = i;
	babyList[i].isSaved	= 1;
	babyList[i].collect = 40;
	if( babiesSaved == numBabies-1 )
		baby->nmeActor->draw = 0;

	if( multiplayerMode == MULTIMODE_COLLECT )
	{
		mpl[pl].score++;
		GTInit( &player[pl].dead, 2 );

		PlaySample( genSfx[GEN_COLLECT_BABY], NULL, 0, SAMPLE_VOLUME, -1 );
		PlayVoice(pl, "frogokay");

		BabyCollectEffect( baby->nmeActor, baby->inTile, n );
	}
}


/*	--------------------------------------------------------------------------------
	Function		: KillMPFrog
	Purpose			: Subtract healthpoint or kill, or respawn if appropriate
	Parameters		: player
	Returns			: void
	Info			:
*/
void KillMPFrog(int pl)
{
	player[pl].frogState |= FROGSTATUS_ISDEAD;

	// A standard multiplayer death
	CreateRespawnEffect( currTile[pl], pl );

	PlaySample( genSfx[GEN_DEATHEXPLODE], NULL, 0, SAMPLE_VOLUME, -1 );

	switch( multiplayerMode )
	{
	case MULTIMODE_RACE:
		mpl[pl].penalty += MULTI_RACE_TIMEPENALTY;
		TransformPoint(&frog[pl]->actor->position,&mpl[pl].scrX,&mpl[pl].scrY);
		mpl[pl].penalText->draw = 1;
		mpl[pl].penalText->xPos = mpl[pl].scrX;
		mpl[pl].penalText->yPos = mpl[pl].scrY;
		GTInit( &player[pl].dead, 2 );
		frog[pl]->draw = 0;
		break;

	case MULTIMODE_BATTLE:
		GTInit( &player[pl].dead, 2 );
		frog[pl]->draw = 0;
		break;

	case MULTIMODE_COLLECT:
		GTInit( &player[pl].dead, 2 );
		frog[pl]->draw = 0;
		break;

	default:
		GTInit( &player[pl].stun, 2 );
		GTInit( &player[pl].safe, 5 );
		
		player[pl].healthPoints=0;
		player[pl].deathBy = DEATHBY_NORMAL;
		frog[pl]->draw = 0;
		break;
	}
}

/*	--------------------------------------------------------------------------------
	Function		: FreeMultiplayer
	Purpose			: Deallocate any data that needs it
	Parameters		: 
	Returns			: 
	Info			:
*/
void FreeMultiplayer( )
{
	int i;

	if( nodePool.array )
	{
		// Remove all particles in array from particle list so they don't get removed after deallocation
		for( i=0; i<MAX_BATTLENODES; i++ )
		{
			if( nodePool.array[i].fx )
				actor2Free( nodePool.array[i].fx );

			nodePool.array[i].next = NULL;
		}

		FREE( nodePool.array );
		nodePool.array = NULL;
	}

	if( nodePool.stack )
	{
		FREE( nodePool.stack );
		nodePool.stack = NULL;
	}

	nodePool.count = 0;
	nodePool.stackPtr = 0;
}


/*	--------------------------------------------------------------------------------
	Function		: ReinitialiseMultiplayer
	Purpose			: Set mp data back to very initial values
	Parameters		: 
	Returns			: 
	Info			:
*/
extern char *numberText[4];
void ReinitialiseMultiplayer( )
{
	int i;

	matchWinner = -1;

	memset( mpl, 0, sizeof( MPINFO )*4 );

	if( multiplayerMode == MULTIMODE_BATTLE )
	{
		nodePool.array = (BATTLENODE *)MALLOC0( sizeof(BATTLENODE)*MAX_BATTLENODES );
		nodePool.stack = (BATTLENODE **)MALLOC0( sizeof(BATTLENODE*)*MAX_BATTLENODES );

		for( i=0; i<MAX_BATTLENODES; i++ )
		{
			nodePool.stack[i] = &nodePool.array[i];
			nodePool.array[i].fx = CreateAndAddActor( "cube.obe", 0, 0, 0, 0);
			nodePool.array[i].fx->draw = 0;
		}

		nodePool.count = 0;
		nodePool.stackPtr = i-1;

#ifdef PSX_VERSION
#ifndef DREAMCAST_VERSION
		{
			char name[16];
			for( i=0; i<NUM_FROGS; i++ )
			{
				sprintf( name, "mpl%i.pal", player[i].character );
				//cubePalettes[i] = LOADPAL_Load16( name );
				switch ( player[i].character )
				{
				case FROG_FROGGER:
						cubePalettes[i] = LOADPAL_Load16( mpl0 );
					break;
				case FROG_LILLIE:
						cubePalettes[i] = LOADPAL_Load16( mpl1 );
					break;
				case FROG_BABYFROG:
						cubePalettes[i] = LOADPAL_Load16( mpl2 );
					break;
				case FROG_SWAMPY:
						cubePalettes[i] = LOADPAL_Load16( mpl3 );
					break;
				case FROG_TWEE:
						cubePalettes[i] = LOADPAL_Load16( mpl4 );
					break;
				case FROG_WART:
						cubePalettes[i] = LOADPAL_Load16( mpl5 );
					break;
				case FROG_HOPPER:
						cubePalettes[i] = LOADPAL_Load16( mpl6 );
					break;
				case FROG_ROBOFROG:
						cubePalettes[i] = LOADPAL_Load16( mpl7 );
					break;
				}
				// ENDSWITCH
				
			}
		}
#endif
#endif
	}

	for( i=0; i<NUM_FROGS; i++ )
	{
#ifdef PC_VERSION
		if(gameState.multi == MULTIREMOTE)
		{
			sprintf(mplNameStr[i],"%d - %s",i+1,NetGetPlayerName(i));
			mpl[i].nameText = CreateAndAddTextOverlay(backCharsX[i] + 512,backCharsY[i] + 160,mplNameStr[i],NO,255,fontSmall,TEXTOVERLAY_SHADOW);
			
			if((i == 1) || (i == 2))
			{
				mpl[i].nameText->xPos = backCharsX[i] - (float)(CalcStringWidth(mpl[i].nameText->text,(MDX_FONT *)mpl[i].nameText->font,((float)mpl[i].nameText->scale)/4096.0) + 2) * 6.4;
			}
		}
#endif
			mpl[i].numText = CreateAndAddTextOverlay(-1000,0,numberText[i],YES,255,font,TEXTOVERLAY_SHADOW);
		if(gameState.difficulty == DIFFICULTY_HARD)
			mpl[i].trail = MULTI_BATTLE_TRAILLENGTH*2;
		else
			mpl[i].trail = MULTI_BATTLE_TRAILLENGTH;
		mpl[i].lasttile = TILESTATE_FROGGER1AREA;
		if(multiplayerMode == MULTIMODE_RACE)
		{
			mpl[i].penalText = CreateAndAddTextOverlay(0,0,penalStr,NO,255,font,TEXTOVERLAY_SHADOW);
			mpl[i].penalText->g = mpl[i].penalText->b = 0;
			mpl[i].penalText->draw = 0;
		}
	}
}


/*	--------------------------------------------------------------------------------
	Function		: ResetMultiplayer
	Purpose			: Set multiplayer info to initial values EXCEPT the stuff that needs to be preserved between levels
	Parameters		: player
	Returns			: void
	Info			:
*/
void ResetMultiplayer( )
{
	int i;

	started = 0;
	playerFocus = 0;
	gameWinner = -1;
	matchWinner = -1;
	GTInit( &powerupTimer, 0 );

	for( i=0; i<NUM_FROGS; i++ )
	{
		mpl[i].ready = 0;
		player[i].idleEnable = 1;
		player[i].canJump = 1;
	}

	switch( multiplayerMode )
	{
	case MULTIMODE_COLLECT:
		for( i=0; i<NUM_FROGS; i++ )
		{
			mpl[i].ready = 0;
			mpl[i].score = 0;
			SetFroggerStartPos( gTStart[i], i );
		}
		for( i=0; i<numBabies; i++ )
		{
			babyList[i].isSaved = 0;
			babyList[i].idle = 0;
			babyList[i].collect = 0;
			if( babyList[i].baby ) babyList[i].baby->draw = 1;
			babiesSaved = 0;
			lastBabySaved = -1;
			MoveEnemyToNode( babyList[i].enemy, 0 );
			actorAnimate( babyList[i].baby->actor, BABY_ANIM_SNOOZE, YES, NO, 100, 0 );
		}
		break;
	case MULTIMODE_BATTLE:
	{
		BATTLENODE *node, *temp;
		for( i=0; i<NUM_FROGS; i++ )
		{
			if(gameState.difficulty == DIFFICULTY_HARD)
				mpl[i].trail = MULTI_BATTLE_TRAILLENGTH*2;
			else
				mpl[i].trail = MULTI_BATTLE_TRAILLENGTH;
			mpl[i].ready = 0;
			mpl[i].score = 0;
			node = mpl[i].path;
			while(node)
			{
				temp = node->next;
//				node->tile->state = TILESTATE_NORMAL;
				SubBattleTrailNode(node);
				node = temp;
			}
			mpl[i].path = NULL;
			SetFroggerStartPos( gTStart[i], i );
		}

		FreeSpecFXList();
		InitSpecFXList();
		FreeGaribList();
		InitGaribList();
		AllocGaribBlock( MULTI_BATTLE_MAXITEMS );
#ifdef PC_VERSION
		InitSpriteSortArray();
#endif
		numMultiItems = 0;
		scaleV = 4506;
		break;
	}
	case MULTIMODE_RACE:
		for( i=0; i<NUM_FROGS; i++ )
		{
			mpl[i].check = 0;
			mpl[i].lap = 0;
			mpl[i].lasttile = TILESTATE_FROGGER1AREA;
			mpl[i].timer = 0;
			mpl[i].penalty = 0;

			SetFroggerStartPos( gTStart[i], i );
		}
		break;
	}

	InitCamera();
}


/*	--------------------------------------------------------------------------------
	Function		: CalcMPCamera
	Purpose			: Do different cameras for different game types
	Parameters		: target vector
	Returns			: 
	Info			:
*/
void CalcMPCamera( FVECTOR *target )
{
	switch( multiplayerMode )
	{
	case MULTIMODE_RACE:
		if( player[0].worldNum == WORLDID_SUBTERRANEAN )
			CalcRaceCircuitCamera( target );
		else
			CalcRaceStraightCamera( target );
		break;

	case MULTIMODE_BATTLE:
		CalcBattleCamera( target );
		break;

	case MULTIMODE_COLLECT:
		CalcCollectCamera( target );
		break;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: CalcRaceCamera
	Purpose			: Focus on the average point of all the lead players
	Parameters		: target vector
	Returns			: 
	Info			:
*/
void CalcRaceCircuitCamera( FVECTOR *target )
{
	int i, bestLap=0, bestCheck=0, num;

	ZeroVector( target );

	if( gameWinner != -1 && gameWinner != MULTI_ROUND_DRAW )
	{
		playerFocus = gameWinner;
		SetVectorFS( target, &frog[gameWinner]->actor->position );
		return;
	}

	for( i=0; i<NUM_FROGS; i++ )
		if( mpl[i].lap >= bestLap && mpl[i].check >= bestCheck )
		{
			bestLap = mpl[i].lap;
			bestCheck = mpl[i].check;
			playerFocus = i;
		}

	for( i=0,num=0; i<NUM_FROGS; i++ )
	{
		if( mpl[i].lap == bestLap && mpl[i].check == bestCheck )
		{
			num++;
			if( player[i].jumpTime > 0 )	// jumping; calculate linear position
			{
				FVECTOR v;
				SetVectorFF(&v, &player[i].jumpFwdVector);
				ScaleVectorFF(&v, player[i].jumpTime);
				AddToVectorFS(target, &player[i].jumpOrigin);
				AddToVectorFF(target, &v);
			}										
			else
			{
				AddToVectorFS( target, &frog[i]->actor->position );
			}
		}
	}

	ScaleVectorFF( target, 4096/(max(num,1)) );
}


/*	--------------------------------------------------------------------------------
	Function		: CalcStraightRaceCamera
	Purpose			: Focus on the average point of all the lead players
	Parameters		: target vector
	Returns			: 
	Info			:
*/
void CalcRaceStraightCamera( FVECTOR *target )
{
	int i, bestLap=0, bestCheck=0, bestZ=999999999;
	fixed scale;//, vx, vz;
	FVECTOR diff;

	SetVectorFF( target, &zero );
//	target->vx = target->vz = 0;

	if( gameWinner != -1 && gameWinner != MULTI_ROUND_DRAW )
	{
		playerFocus = gameWinner;
		SetVectorFS( target, &frog[gameWinner]->actor->position );
//		target->vx = frog[gameWinner]->actor->position.vx<<12;
//		target->vz = frog[gameWinner]->actor->position.vz<<12;
		return;
	}

	for( i=0; i<NUM_FROGS; i++ )
		if( mpl[i].lap >= bestLap && mpl[i].check >= bestCheck && frog[i]->actor->position.vz < bestZ )
		{
			bestLap = mpl[i].lap;
			bestCheck = mpl[i].check;
			bestZ = frog[i]->actor->position.vz;
			playerFocus = i;
		}

	for( i=0; i<NUM_FROGS; i++ )
	{
		if( player[i].jumpTime > 0 )	// jumping; calculate linear position
		{
			FVECTOR v;
			SetVectorFF(&v, &player[i].jumpFwdVector);
			ScaleVectorFF(&v, player[i].jumpTime);

			SetVectorFS( &diff, &player[i].jumpOrigin );
			AddToVectorFF( &diff, &v );
//			diff.vx = (player[i].jumpOrigin.vx<<12) + v.vx;
//			diff.vy = (player[i].jumpOrigin.vy<<12) + v.vy;
//			diff.vz = (player[i].jumpOrigin.vz<<12) + v.vz;

			AddToVectorFF( target, &diff );
//			target->vx += vx;
//			target->vz += vz;

			// Double weight for lead player
			if( i == playerFocus )
			{
				AddToVectorFF( target, &diff );
//				target->vx += vx;
//				target->vz += vz;
			}
		}										
		else
		{
			SetVectorFS( &diff, &frog[i]->actor->position );
//			vx = frog[i]->actor->position.vx<<12;
//			vz = frog[i]->actor->position.vz<<12;

			AddToVectorFF( target, &diff );
//			target->vx += vx;
//			target->vz += vz;

			// Double weight for lead player
			if( i == playerFocus )
			{
				AddToVectorFF( target, &diff );
//				target->vx += vx;
//				target->vz += vz;
			}
		}
	}

	scale = 4096/(NUM_FROGS+1);
	ScaleVectorFF( target, scale );

	if( target->vy < initialCamHeight )
		target->vy = initialCamHeight;
//	target->vx = FMul( target->vx, scale );
//	target->vz = FMul( target->vz, scale );
}


/*	--------------------------------------------------------------------------------
	Function		: CalcCollectCamera
	Purpose			: Focus on average position of frogs and zoom depending on separation
	Parameters		: target vector
	Returns			: 
	Info			: 
*/
long camDiv[4] = {5000,6500,5000};
long camPlus[4] = {3000,3000,3000};

void CalcCollectCamera( FVECTOR *target )
{
	FVECTOR v;
	int i,l;
	fixed sc;
	FVECTOR minVect = {0,0,0},maxVect = {0,0,0};

	for( i=0,l=0; i<NUM_FROGS; i++ )
	{
		if( !(player[i].frogState & FROGSTATUS_ISDEAD) )
		{
			if( player[i].jumpTime > 0 )	// jumping; calculate linear position
			{
				SetVectorFF(&v, &player[i].jumpFwdVector);
				ScaleVectorFF(&v, player[i].jumpTime);
				AddToVectorFS(&v, &player[i].jumpOrigin);
			}
			else
				SetVectorFS(&v,&frog[i]->actor->position);

			if(l == 0)
			{
				SetVectorFF(&minVect,&v);
				SetVectorFF(&maxVect,&v);
			}
			else
			{
				if(v.vx < minVect.vx)
					minVect.vx = v.vx;
				if(v.vy < minVect.vy)
					minVect.vy = v.vy;
				if(v.vz < minVect.vz)
					minVect.vz = v.vz;
				if(v.vx > maxVect.vx)
					maxVect.vx = v.vx;
				if(v.vy > maxVect.vy)
					maxVect.vy = v.vy;
				if(v.vz > maxVect.vz)
					maxVect.vz = v.vz;
			}

			l++;
		}
	}
	
	// Zoom in/out to keep multiplayer frogs in view
	sc = FindMaxInterFrogDistance( );
	if( sc != -4096 ) 
		scaleV = (sc/camDiv[player[0].levelNum - worldVisualData[player[0].worldNum].numSinglePlayerLevels]) + camPlus[player[0].levelNum - worldVisualData[player[0].worldNum].numSinglePlayerLevels];

	if (l > 0)
	{
		AddVectorFFF(target,&minVect,&maxVect);
		ScaleVectorFF(target,2048);
	}
}


/*	--------------------------------------------------------------------------------
	Function		: CalcCollectCamera
	Purpose			: Focus on average position of frogs and zoom depending on separation
	Parameters		: target vector
	Returns			: 
	Info			: 
*/
int battleCamFac = 3000;
int battleCamPlus = 5000;
void CalcBattleCamera( FVECTOR *target )
{
	FVECTOR v;
	int i,l;
	fixed sc, scv;
	FVECTOR minVect,maxVect;

	for( i=0,l=0; i<NUM_FROGS; i++ )
	{
		if( !(player[i].frogState & FROGSTATUS_ISDEAD) )
		{
			if( player[i].jumpTime > 0 )	// jumping; calculate linear position
			{
				SetVectorFF(&v, &player[i].jumpFwdVector);
				ScaleVectorFF(&v, player[i].jumpTime);
				AddToVectorFS(&v, &player[i].jumpOrigin);
			}
			else
				SetVectorFS(&v,&frog[i]->actor->position);

			if(l == 0)
			{
				SetVectorFF(&minVect,&v);
				SetVectorFF(&maxVect,&v);
			}
			else
			{
				if(v.vx < minVect.vx)
					minVect.vx = v.vx;
				if(v.vy < minVect.vy)
					minVect.vy = v.vy;
				if(v.vz < minVect.vz)
					minVect.vz = v.vz;
				if(v.vx > maxVect.vx)
					maxVect.vx = v.vx;
				if(v.vy > maxVect.vy)
					maxVect.vy = v.vy;
				if(v.vz > maxVect.vz)
					maxVect.vz = v.vz;
			}
			l++;
		}
	}
	
	// Zoom in/out to keep multiplayer frogs in view
	sc = FindMaxInterFrogDistance( );
	if( sc > 0 )
	{
		scv = (sc/battleCamFac) + battleCamPlus;
		scaleV = scv;
	}

	if (l > 0)
	{
		AddVectorFFF(target,&minVect,&maxVect);
		ScaleVectorFF(target,2048);
	}
}

/*	--------------------------------------------------------------------------------
	Function		: IsPointVisible
	Purpose			: Self explanatory really
	Parameters		: target vector
	Returns			: 
	Info			: 
*/
#ifdef PC_VERSION
#include <pcmisc.h>
char IsPointVisible(SVECTOR *pos)
{
	MDX_VECTOR m,p;
	SetVectorRS( &p, pos );
	ScaleVector( &p, 0.1 );

	XfmPoint(&m,&p,NULL);
	if (m.vz==0)
		return 0;
	if (m.vx<0) 
		return 0;
	if (m.vx>rXRes) 
		return 0;
	if (m.vy<0) 
		return 0;
	if (m.vy>rYRes)
		return 0;
	return 1;
}
#else

#ifdef DREAMCAST_VERSION
char IsPointVisible ( SVECTOR *pos )
{
	SVECTOR vt, p;

	long sxy, sz;

	p.vx = -pos->vx;
	p.vy = -pos->vy;
	p.vz = pos->vz;

	gte_SetTransMatrix(&GsWSMATRIX);
	gte_SetRotMatrix(&GsWSMATRIX);

	gte_ldv0 ( &p );
	gte_SetLDDQB(0);
	gte_rtps();
	gte_stsxy ( &sxy );

	vt.vx = ( short ) ( sxy & 0xffff );
	vt.vy = ( short ) ( sxy >> 16 );

	if ( vt.vx < 0 )
		return 0;
	if ( vt.vx > 640 )
		return 0;
	if ( vt.vy < 0 )
		return 0;
	if ( vt.vy > 480 )
		return 0;

	return 1;
}
#else
char IsPointVisible ( SVECTOR *pos )
{
	SVECTOR vt, p;
	long sxy, sz;
	short checky = PALMODE?128:120;


	p.vx = -pos->vx;
	p.vy = -pos->vy;
	p.vz = pos->vz;

	gte_SetTransMatrix(&GsWSMATRIX);
	gte_SetRotMatrix(&GsWSMATRIX);

	gte_SetLDDQB(0);
	gte_ldv0 ( &p );
	gte_rtps();
	gte_stsxy ( &sxy );
	gte_stsz ( &sz );

	vt.vx = (short)(sxy & 0xffff) + 256;
	vt.vy = (short)(sxy >> 16) + checky;
	vt.vz = sz;


	if ( vt.vx < 0 )
		return 0;
	if ( vt.vx > 512 )
		return 0;
	if ( vt.vy < 0 )
		return 0;
	if ( vt.vy > (checky<<1) )
		return 0;
	if( vt.vz < 20 )
		return 0;
	if( vt.vz > fog.max )
		return 0;

	return 1;
}
#endif
#endif

void RestartMulti()
{
	int i,j;

	matchWinner = -1;
	for(i = 0;i < NUM_FROGS;i++)
	{
		mpl[i].check = 0;
		mpl[i].lap = 0;
		mpl[i].penalty = 0;
		mpl[i].score = 0;
		mpl[i].wins = 0;
		for(j = 0;j < 3;j++)
			multiHud.trophies[i][j]->draw = 0;
	}
	ResetMultiplayer();
	multiHud.centreText->draw = 1;
	EnableHUD();
}

void PlayerRaceWin( int pl )
{
	mpl[pl].ready = 1;
	player[pl].canJump = 0;
	actorAnimate(frog[pl]->actor,FROG_ANIM_LOOKAROUND,NO,NO,80,0);
	actorAnimate(frog[pl]->actor,FROG_ANIM_HANDSPRING,NO,YES,80,0);
	actorAnimate(frog[pl]->actor,FROG_ANIM_BREATHE,YES,YES,50,0);

	CreatePickupEffect( pl, frogPool[player[pl].character].r, frogPool[player[pl].character].g, frogPool[player[pl].character].b, 255, 255, 255 );
}
