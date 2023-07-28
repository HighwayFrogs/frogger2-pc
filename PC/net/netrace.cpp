/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: netrace.cpp
	Programmer	: David Swift
	Date		:

	Based on the race-mode part of Jim's multiplayer code with 'optimisations'
	(i.e. cludges that may or may not work) so only player one is updated - the
	other players are dutifully faked by the netgame code - ds

----------------------------------------------------------------------------------------------- */

// PC headers
#include "main.h"
#include "netrace.h"
#include "netgame.h"
#include "network.h"
#include "controll.h"
#include "pcaudio.h"
#include "islutil.h"

// COMMON headers
#include "layout.h"
#include "lang.h"
#include "game.h"
#include "frogger.h"
#include "frogmove.h"
#include "cam.h"
#include "multi.h"
#include "hud.h"
#include "frontend.h"	// why is 'SlideTextOverlayToPos' defined here?


enum
{
	GAMEMSG_RACEWON = GAMEMSG_START
};


struct MSG_RACEWON
{
	UBYTE id;
	DWORD frame, penalty;
};

// should be in header, but then the net code would keep making everything recompile and that's pants
extern NET_MESSAGEHANDLER netgameHandler;	

long started = 0;

int countdown = 4;

int NetRaceCountdown();
int NetRaceCheckWin();
int NetRaceRun();

int NetRaceMessageDispatch(void *data, unsigned long size, NETPLAYER *player);
void NetRaceRespawn();

// ------------------------------------------------------------------
// NetRaceInit()
// Initialises network race mode

int NetRaceInit()
{
	netgameHandler	= NetRaceMessageDispatch;
	netgameLoopFunc	= NetRaceRun;
	multiplayerMode = MULTIMODE_RACE;
	cameoMode = true;

	return 0;
}



// ------------------------------------------------------------------
// NetRaceCountDown()
// Do the start-of-race countdown based on gameStartTime

int NetRaceCountdown()
{
	static char txt[3];

	if (actFrameCount < gameStartTime)
	{
		if (gameStartTime)
		{
			FVECTOR fwd;
			SetVectorFF( &fwd, &currTile[0]->dirVector[(frogFacing[0]+camFacing[0])&3] );
			
			int count = (gameStartTime-actFrameCount+59)/60;

			if (count<4)
			{
				if( currTile[0]->state == TILESTATE_FROGGER1AREA )
					player[0].canJump = 0;

				if( player[0].canJump )
					HopFrogToTile( FindJoinedTileByDirection(currTile[0],&fwd), 0);
			}

			if (count<countdown)
			{
				itoa(count, txt, 10);
				countdown = count;
				
				multiHud.centreText->a = (char)255;
				multiHud.centreText->draw = 1;
				multiHud.centreText->text = txt;
				multiHud.centreText->scale = 8192;
				PlaySample( FindSample(utilStr2CRC("racehorn")), NULL, 0, SAMPLE_VOLUME, -1 );
			}
		}

		cameoMode = 1;
		return 1;
	}
	else
	{
		if (lastActFrameCount < gameStartTime)
		{
			multiHud.centreText->text = GAMESTRING(STR_GO);
			multiHud.centreText->a = (char)255;

			PlaySample( FindSample(utilStr2CRC("racehorngo")), NULL, 0, SAMPLE_VOLUME, -1 );

			player[0].canJump = 1;
			cameoMode = 0;
			
			// Reset countdown for next time
			countdown = 4;
		}

		return 0;
	}
}


// ------------------------------------------------------------------
// NetRaceRun()
// gameloop for networked race mode

int NetRaceRun()
{
	int i;
	//RunGameLoop();

	NetRaceCountdown();

	UpdateCameraPosition();
	GameProcessController(0);                                      

		
	UpdateFroggerPos(0);

	if (actFrameCount > gameStartTime)
	{
		if (multiHud.centreText->a > (gameSpeed>>10))
			multiHud.centreText->a -= (gameSpeed>>10);
		else
		{
			multiHud.centreText->a = 0;
			multiHud.centreText->draw = 0;
		}

		cameoMode = false;
		UpDateMultiplayerInfo( );

		if (!mpl[0].ready)
			mpl[0].timer = actFrameCount - gameStartTime;

		if( player[0].dead.time )
		{
			GTUpdate( &player[0].dead, -1 );

			// from multi.c
			if( !player[0].dead.time )
			{
				NetRaceRespawn();
				frog[0]->draw = 1;
				GTInit( &player[0].safe, 3 );
			}
		}
/*		else if( frog[0]->draw && !(IsPointVisible(&frog[0]->actor->position)) )
		{
			KillMPFrog(0);
		}
*/
		for( i=0; i<NUM_FROGS; i++ )
		{
			// Do overlay stuff
			mpl[i].nameText->draw = 0;
			mpl[i].numText->draw = 0;

			if(mpl[i].penalText->draw)
			{
				SlideTextOverlayToPos(mpl[i].penalText,backTextX[i],backTextY[i],mpl[i].scrX,mpl[i].scrY,40);	
				if((mpl[i].penalText->xPos == backTextX[i]) && (mpl[i].penalText->yPos == backTextY[i]))
					mpl[i].penalText->draw = 0;
			}

			//if (frog[i]->actor->position.vz > frog[leadingPlayer]->actor->position.vz)
			//	leadingPlayer = i;

			if(!frog[i]) continue;

			// Do pretty effects
			if (player[i].safe.time)
			{
				if( actFrameCount&1 )
					CreateRingEffect(i);

				GTUpdate( &player[i].safe, -1 );
			}

			frog[i]->draw = (player[i].dead.time==0);

			// Increment the current checkpoint for each player
			if( currTile[i]->state == mpl[i].lasttile+1 ||
				(mpl[i].lasttile == TILESTATE_FROGGER4AREA && currTile[i]->state == TILESTATE_FROGGER2AREA) )
			{
				mpl[i].check++;
				mpl[i].lasttile = currTile[i]->state;
			}
		}

		//if (leadingPlayer != 0)	GTInit( &player[i].quickhop, 3 );

		if( currTile[0]->state == TILESTATE_FINISHLINE && mpl[0].lasttile != TILESTATE_FINISHLINE )
		{
			mpl[0].lap++;
			mpl[0].lasttile = currTile[0]->state;
			mpl[0].check = 0;

			// Start of a new lap - if more then the defined number of maps for the race then this player is the winner
			if( mpl[0].lap >= 1)//MULTI_RACE_NUMLAPS )
			{
				MSG_RACEWON msg;
				
				msg.id		= GAMEMSG_RACEWON;
				msg.penalty	= mpl[0].penalty;
				msg.frame	= actFrameCount;

				NetBroadcastUrgentMessage(&msg, sizeof(msg));

				PlayerRaceWin( 0 );

				multiHud.centreText->text = GAMESTRING(STR_NET_FINISHED);
				multiHud.centreText->a = 255;
				multiHud.centreText->draw = 1;
			}
		}

		NetRaceCheckWin();
	}
	else
	{
		for(i = 0;i < NUM_FROGS;i++)
		{
			mpl[i].nameText->draw = 1;
			TransformPoint(&frog[i]->actor->position,&mpl[i].numText->xPos,&mpl[i].numText->yPos);
			mpl[i].numText->yPos -= 200;
		}
	}

	return 0;
}

int NetRaceCheckWin()
{
	int i;

	// Check players for finish
	for( i=0; i<NUM_FROGS; i++ )
		if( !mpl[i].ready ) break;

	// If all players finished, check win conditions

#ifdef _DEBUG	
	if (i==NUM_FROGS)
#else
	if( i==NUM_FROGS || NUM_FROGS==1)
#endif
	{
		unsigned long best, time, winner, draw;
		// Find best time
		for( i=0,best=0xffffffffUL; i<NUM_FROGS; i++ )
		{
			time = mpl[i].timer + mpl[i].penalty;
			if( time < best )
			{
				best = time;
				winner = i;
			}
		}
		// If more then one on best time, draw
		for( i=0,draw=0; i<NUM_FROGS; i++ )
		{
			time = mpl[i].timer + mpl[i].penalty;
			if( i!=winner && time == best )
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

			if( mpl[winner].wins == 3 && matchWinner == -1 || NUM_FROGS==1)
				matchWinner = winner;
		}

		GTInit( &endTimer, 2 );
		controlCamera = 1;

		return 1;
	}
	else
		return 0;
}


int NetRaceMessageDispatch(void *data, unsigned long size, NETPLAYER *player)
{
	int pl;

	switch (*(UBYTE*)data)
	{
	case GAMEMSG_RACEWON:
		MSG_RACEWON *won = (MSG_RACEWON*)data;

		utilPrintf("Player %08x finished on frame %d with penalty %d\n", player->dpid, won->frame, (int)won->penalty);

		pl = GetPlayerNumFromID(player->dpid);
		mpl[pl].timer = (won->frame-gameStartTime);
		mpl[pl].penalty = won->penalty;
		mpl[pl].ready = 1;
		return 0;
	};

	return -1;
}



void NetRaceRespawn()
{
	int j, respawn=0,noGood;
	GAMETILE *tile;
	FVECTOR diff;

	player[0].deathBy = 0;
	player[0].frogState &= ~FROGSTATUS_ISDEAD;
	actorAnimate( frog[0]->actor, FROG_ANIM_BREATHE, YES, NO, 64, 0 );

/*
	if( playerFocus != 0 && !(player[playerFocus].frogState & FROGSTATUS_ISDEAD) && IsPointVisible(&frog[playerFocus]->actor->position) )
		respawn = playerFocus;
	else
		for( j=0; j<NUM_FROGS; j++ )
			if( j != 0 && j != playerFocus && !(player[j].frogState & FROGSTATUS_ISDEAD) && IsPointVisible(&frog[j]->actor->position) )
			{
				respawn = j;
				break;
			}
*/
	float la_la_i_dont_care = frog[0]->actor->position.vz;
	for( j=1; j<NUM_FROGS; j++ )
		if (!(player[j].frogState & FROGSTATUS_ISDEAD) && frog[j]->actor->position.vz < la_la_i_dont_care)
		{
			la_la_i_dont_care = frog[j]->actor->position.vz;
			respawn = j;
		}

	SubVectorFSS( &diff, &frog[0]->actor->position, &frog[respawn]->actor->position );
	MakeUnit(&diff);

	// Find a tile to teleport to - first try a tile next to the target, then on the nearest "safe" tile, then on the target frog if that fails
	tile = FindJoinedTileByDirectionAndType( currTile[respawn], &diff, TILESTATE_NORMAL );
	if(tile)
	{
		for(j = 0;j < NUM_FROGS;j++)
		{
			if((j != 0) && (currTile[j] == tile))
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
					if((j != 0) && (currTile[j] == t))
					{
						noGood = YES;
						break;
					}
				}
				if(noGood)
					continue;
				dist = DistanceBetweenPointsSS( &frog[respawn]->actor->position, &t->centre ) + DistanceBetweenPointsSS( &frog[0]->actor->position, &t->centre );
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

	camFacing[0] = camFacing[respawn];
	frogFacing[0] = frogFacing[respawn];
	prevCamFacing[0] = prevCamFacing[respawn];

	TeleportActorToTile(frog[0],tile,0);

	CreateRespawnEffect( tile, 0 );

	// Update progress info
	mpl[0].check = mpl[respawn].check;
	mpl[0].lap = mpl[respawn].lap;

	// If other frog has finished then we have to be 1 checkpoint behind
	if( mpl[respawn].ready )
		mpl[0].lasttile = TILESTATE_FROGGER4AREA;		
	else
		mpl[0].lasttile = mpl[respawn].lasttile;

	lastTile[0] = tile;
	destTile[0] = tile;

	CheckForFrogOn(0,tile);
}