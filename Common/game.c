/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: game.c
	Programmer	: Matthew Cloy
	Date		: 12/11/98

----------------------------------------------------------------------------------------------- */


#include <islutil.h>
#include <islpad.h>

#ifdef PSX_VERSION
#include "map_draw.h"
#include "world.h"
#include "world_eff.h"
#include "water.h"
#include "audio.h"
#include "audio.h"
#include "timer.h"
#else
#include <anim.h>
#include <pcaudio.h>
#include <stdio.h>
#include <banks.h>
#include <controll.h>
#endif

#include "map.h"
#include "game.h"
#include "types2d.h"
#include "frogger.h"
#include "levplay.h"
#include "frogmove.h"
#include "cam.h"
#include "tongue.h"
#include "hud.h"
#include "frontend.h"
#include "menus.h"
#include "textover.h"
#include "multi.h"
#include "layout.h"
#include "platform.h"
#include "enemies.h"
#include "babyfrog.h"
#include "event.h"
#include "main.h"
#include "newpsx.h"
#include "Main.h"
#include "actor2.h"
#include "maths.h"
#include "story.h"
#include "fadeout.h"

#include "specfx.h"
#include "particle.h"

#ifdef PC_VERSION
#define TIMER_START3(x)
#define TIMER_STOP3(x)
#define TIMER_STOP_ADD3(x)
#endif


extern int pauseController;
//----------------------------------------------------------------------

#define SHOW_ME_THE_TILE_NUMBERS 1

#ifdef SHOW_ME_THE_TILE_NUMBERS
TEXTOVERLAY *tileNum = NULL;
unsigned long currTileNum = 0;
#endif

//----------------------------------------------------------------------

unsigned long INPUT_POLLPAUSE	= 10;

struct gameStateStruct gameState = {0,0,0,0,0,0,DIFFICULTY_NORMAL};

//unsigned long fadingLogos = 0;
//long idleCamera = 0;
char tileString[16], dkPressed=0;

short showEndLevelScreen = 1;
long bby = 0;
long displayingTile=0;

TIMER scoreTimer;
TIMER modeTimer;

void UpdateCompletedLevel(unsigned long worldID,unsigned long levelID);

// Switch for irritating swingy camera
//unsigned char swingCam = 0;

//SPRITEOVERLAY *flogo[10];
ACTOR2 *backGnd;

char doubleQueue[4]={0,0,0,0};

#ifdef E3_DEMO
TIMER idleKickTimer;	// kick the game out to the menu if it's left alone
#endif

/* --------------------------------------------------------------------------------
	Function	: GameProcessController
	Purpose		:
	Parameters	: player#
	Returns		: void 
*/
void GameProcessController(long pl)
{
	static u16 button[4];
	static s16 stickX[4], stickY[4],lastStickX[4],lastStickY[4];

	if(camControlMode)
		return;

	if( gameState.multi != SINGLEPLAYER && multiplayerMode == MULTIMODE_BATTLE )
		return;

	button[pl] = padData.digital[pl];

#ifdef DREAMCAST_VERSION
	if( padData.debounce[pl] )
	{
		GTInit( &screenSaveTimer, PAUSEFADETIMESECS );

		keepFade = 1;
		fadingOut = 0;
		fadeProc = NULL;
	}
#endif

	// The only thing we can do when dead is press start
	if((gameState.multi == SINGLEPLAYER) || ((player[pl].frogState & FROGSTATUS_ISDEAD) == 0))
	{
#ifdef PC_VERSION
		if(padData.debounce[pl] & PAD_START)
		{
			pauseController = pl;
			StartPauseMenu();
			return;
		}
#elif PSX_VERSION
#ifdef FINAL_MASTER
		if((padData.debounce[pl] & PAD_START) && (gameState.mode != FRONTEND_MODE))
		{
			pauseController = pl;
			StartPauseMenu();
			return;
		}
#else
		if(padData.debounce[pl] & PAD_START)
		{
			pauseController = pl;
			StartPauseMenu();
			return;
		}
#endif
#endif
	}

	if( player[pl].stun.time )
	{
		GTUpdate( &player[pl].stun, -1 );
		return;
	}
	else if( player[pl].dead.time || (player[pl].frogState & FROGSTATUS_ISDEAD) )
	{
		return;
	}

	player[pl].hasJumped = 0;
	
	// check if frog is using extended hop ability
	if(player[pl].isSuperHopping)
		player[pl].canJump = 0;

	if(player[pl].hasDoubleJumped)
	{
		if ((button[pl]&PAD_CROSS) && (player[pl].jumpTime > player[pl].jumpMultiplier/2))
		{
			StartAnimateActor(frog[pl]->actor, FROG_ANIM_FALL, YES, NO, 256, 512);
			player[pl].frogState |= FROGSTATUS_ISFLOATING;
		}
		else
		{
			if (player[pl].frogState & FROGSTATUS_ISFLOATING)
				actorAnimate(frog[pl]->actor, FROG_ANIM_FALLLAND, NO, NO, 128, NO);
			player[pl].frogState &= ~FROGSTATUS_ISFLOATING;
		}
	}

	if( ((padData.debounce[pl] & PAD_UP) || ((padData.digital[pl] & PAD_UP) && player[pl].autohop.time)) && player[pl].canJump)
	{
		if(!player[pl].inputPause )
		{
			player[pl].inputPause = INPUT_POLLPAUSE;
			player[pl].canJump = 0;
			player[pl].hasJumped = 1;
			if(gameState.mode == FRONTEND_MODE)
			{
				player[pl].frogState |= FROGSTATUS_ISWANTINGSUPERHOPU;
				player[pl].isSuperHopping = 1;
				oldFrogFacing[pl] = frogFacing[pl];
				frogFacing[pl] = (camFacing[pl] + MOVE_UP) & 3;
//				Orientate(&frog[pl]->actor->qRot,&currTile[pl]->dirVector[0],&currTile[pl]->normal);
			}
			else
				player[pl].frogState |= FROGSTATUS_ISWANTINGU;

			// update player idleTime
			player[pl].idleTime = ToFixed(MAX_IDLE_TIME);
			//idleCamera = 0;

			player[pl].extendedHopDir = MOVE_UP;
		}
	}	    

	else if( ((padData.debounce[pl] & PAD_RIGHT) || ((padData.digital[pl] & PAD_RIGHT) && player[pl].autohop.time)) && player[pl].canJump)
	{
		if(!player[pl].inputPause)
		{
			player[pl].inputPause = INPUT_POLLPAUSE;
			player[pl].canJump = 0;
			player[pl].hasJumped = 1;
			if(gameState.mode == FRONTEND_MODE)
			{
				player[pl].frogState |= FROGSTATUS_ISWANTINGSUPERHOPR;
				player[pl].isSuperHopping = 1;
				oldFrogFacing[pl] = frogFacing[pl];
				frogFacing[pl] = (camFacing[pl] + MOVE_RIGHT) & 3;
//				Orientate(&frog[pl]->actor->qRot,&currTile[pl]->dirVector[1],&currTile[pl]->normal);
			}
			else
				player[pl].frogState |= FROGSTATUS_ISWANTINGR;

			// update player idleTime
			player[pl].idleTime = ToFixed(MAX_IDLE_TIME);
			//idleCamera = 0;

			player[pl].extendedHopDir = MOVE_RIGHT;
		}
	}
    
	else if( ((padData.debounce[pl] & PAD_DOWN) || ((padData.digital[pl] & PAD_DOWN) && player[pl].autohop.time)) && player[pl].canJump)
	{
		if(!player[pl].inputPause)
		{
			player[pl].inputPause = INPUT_POLLPAUSE;
			player[pl].canJump = 0;
			player[pl].hasJumped = 1;
			if(gameState.mode == FRONTEND_MODE)
			{
				player[pl].frogState |= FROGSTATUS_ISWANTINGSUPERHOPD;
				player[pl].isSuperHopping = 1;
				oldFrogFacing[pl] = frogFacing[pl];
				frogFacing[pl] = (camFacing[pl] + MOVE_DOWN) & 3;
//				Orientate(&frog[pl]->actor->qRot,&currTile[pl]->dirVector[2],&currTile[pl]->normal);
			}
			else
				player[pl].frogState |= FROGSTATUS_ISWANTINGD;

			// update player idleTime
			player[pl].idleTime = ToFixed(MAX_IDLE_TIME);
			//idleCamera = 0;

			player[pl].extendedHopDir = MOVE_DOWN;
		}
	}
    
	else if( ((padData.debounce[pl] & PAD_LEFT) || ((padData.digital[pl] & PAD_LEFT) && player[pl].autohop.time)) && player[pl].canJump)
	{
		if(!player[pl].inputPause)
		{
			player[pl].inputPause = INPUT_POLLPAUSE;
			player[pl].canJump = 0;
			player[pl].hasJumped = 1;
			if(gameState.mode == FRONTEND_MODE)
			{
				player[pl].frogState |= FROGSTATUS_ISWANTINGSUPERHOPL;
				player[pl].isSuperHopping = 1;
				oldFrogFacing[pl] = frogFacing[pl];
				frogFacing[pl] = (camFacing[pl] + MOVE_LEFT) & 3;
//				Orientate(&frog[pl]->actor->qRot,&currTile[pl]->dirVector[3],&currTile[pl]->normal);
			}
			else
				player[pl].frogState |= FROGSTATUS_ISWANTINGL;

			// update player idleTime
			player[pl].idleTime = ToFixed(MAX_IDLE_TIME);
			//idleCamera = 0;
			
			player[pl].extendedHopDir = MOVE_LEFT;
		}
	}

	// Do not allow superhop if in retro mode
	if( ((doubleQueue[pl] && player[pl].jumpTime > 2048 ) || (padData.debounce[pl] & PAD_CROSS))
		&& (player[0].worldNum != WORLDID_SUPERRETRO) && (gameState.mode != FRONTEND_MODE) && !cameoMode )
    {
		int dir = player[pl].extendedHopDir;

		if( button[pl] & PAD_UP )
			dir = MOVE_UP;
		else if( button[pl] & PAD_DOWN )
			dir = MOVE_DOWN;
		else if( button[pl] & PAD_LEFT )
			dir = MOVE_LEFT;
		else if( button[pl] & PAD_RIGHT )
			dir = MOVE_RIGHT;

/*		ds - er, this doesn't seem to help any
		since superjumping is done specially by frogmove.c, we should only need to play with the directions
		when we're double-jumping..

		frogFacing[pl] = nextFrogFacing[pl];

		nextFrogFacing[pl] = frogFacing[pl] = (dir+camFacing[pl]) &3;
		player[pl].extendedHopDir = dir;
		Orientate( &frog[pl]->actor->qRot, &currTile[pl]->dirVector[frogFacing[pl]], &currTile[pl]->normal );
*/

		// update player idleTime
		player[pl].idleTime = ToFixed(MAX_IDLE_TIME);
		//idleCamera = 0;

		// ------------------- DOUBLE JUMP -------------------------

		if( player[pl].isSuperHopping && destTile[pl] && !player[pl].hasDoubleJumped &&
			destTile[pl]->state != TILESTATE_NOSUPER && destTile[pl]->state != TILESTATE_NOSUPERHOT)  
		{
			GAMETILE *old;
			int oldCamFacing;

			if( !doubleQueue[pl] && player[pl].jumpTime < player[pl].jumpMultiplier )
			{
				doubleQueue[pl] = 1;
				return;
			}

			doubleQueue[pl] = 0;

			PlaySample(genSfx[GEN_DOUBLE_HOP],NULL,0,SAMPLE_VOLUME,-1);

			// we have to keep track here of the previous tile so if it fails we don't
			// just sit in mid-air...
			old = currTile[pl];
			oldCamFacing = camFacing[pl];

			camFacing[pl] = GetTilesMatchingDirection(currTile[pl], camFacing[pl], destTile[pl]);
			currTile[pl] = destTile[pl];
			frogFacing[pl] = (camFacing[pl]+dir) & 3;

			// player is superhopping - make frog double jump in desired direction
			if (MoveToRequestedDestination( dir, pl ))
			{
				player[pl].hasDoubleJumped = 1;
				player[pl].canJump = 0;

				// ds - MoveTo.. should set frogFacing/nextFrogFacing correctly?
				//frogFacing[pl] = (dir+camFacing[pl]) &3;
				OrientateSS( &frog[pl]->actor->qRot, &currTile[pl]->dirVector[frogFacing[pl]], &old->normal );
				/*nextFrogFacing[pl] =*/ 
				player[pl].extendedHopDir = dir;
				actorAnimate(frog[pl]->actor,FROG_ANIM_SOMERSAULT,NO,NO,75,0);
			}
			else
			{
				player[pl].isSuperHopping = 1;
//				if( player[pl].jumpTime > 2400 && (MoveToRequestedDestination((dir+2)&3, pl)) ) // Try to backflip
//				{
//					dir = (dir+2)&3;
//					player[pl].hasDoubleJumped = 1;
//					player[pl].canJump = 0;

//					nextFrogFacing[pl] = frogFacing[pl] = (dir+camFacing[pl]) &3;
//					player[pl].extendedHopDir = dir;
//					Orientate( &frog[pl]->actor->qRot, &currTile[pl]->dirVector[frogFacing[pl]], &old->normal );
//					actorAnimate(frog[pl]->actor,FROG_ANIM_SOMERSAULT,NO,NO,75,0);
//				}
//				else
//				{
					// Restore old state :oP
					
					// ds - hAck! Jump to the platform
					if (currPlatform[pl])
					{
						destPlatform[pl] = currPlatform[pl];
						player[pl].frogState |= FROGSTATUS_ISJUMPINGTOPLATFORM;
					}
					destTile[pl] = currTile[pl];
					currTile[pl] = old;
					camFacing[pl] = oldCamFacing;
					player[pl].canJump = 0;
					
//					CalculateFrogJump(
//						&frog[pl]->actor->position, &destTile[pl]->centre, &destTile[pl]->normal,
//						ToFixed(100), standardHopFrames, pl);
//				}
			}

		}
		else if(!(player[pl].isSuperHopping) && !(player[pl].inputPause) && (gameState.mode != FRONTEND_MODE))
		{
			// Check if the player can jump.. if not, check if we're just hopping..
			// .. but only if we're not sliding! Good heavens.
			if (player[pl].canJump || 
			   (player[pl].frogState & (FROGSTATUS_ISJUMPINGTOTILE | FROGSTATUS_ISJUMPINGTOPLATFORM) &&
			   !(player[pl].frogState & FROGSTATUS_ISSLIDING)))
			{
				// frog is wanting superhop
				player[pl].isSuperHopping = 1;
				player[pl].hasJumped = 1;
				player[pl].hasDoubleJumped = 0;

				player[pl].inputPause = INPUT_POLLPAUSE;

				switch(player[pl].extendedHopDir)
				{
					case MOVE_UP:
						player[pl].frogState |= FROGSTATUS_ISWANTINGSUPERHOPU;
						break;
					case MOVE_LEFT:
						player[pl].frogState |= FROGSTATUS_ISWANTINGSUPERHOPL;
						break;
					case MOVE_DOWN:
						player[pl].frogState |= FROGSTATUS_ISWANTINGSUPERHOPD;
						break;
					case MOVE_RIGHT:
						player[pl].frogState |= FROGSTATUS_ISWANTINGSUPERHOPR;
						break;
				}
			}
		}
	}

	if((gameState.multi == SINGLEPLAYER ) && (player[pl].canJump))
	{
//#ifdef PC_VERSION
		if((padData.debounce[pl] & PAD_CIRCLE) && (tongue[pl].flags & TONGUE_IDLE))
		{
			// want to use tongue
			tongue[pl].flags = TONGUE_NONE | TONGUE_SEARCHING;
			//player[pl].hasJumped = 1;

			// update player idleTime
			player[pl].idleTime = ToFixed(MAX_IDLE_TIME);
			//idleCamera = 0;
		}

		if( (padData.debounce[pl] & PAD_SQUARE) && !player[pl].isCroaking.time )
		{
			player[pl].frogState |= FROGSTATUS_ISCROAKING;
			GTInit( &player[pl].isCroaking, 2 );

			if( player[pl].character != FROG_HOPPER )
			{
				actorAnimate( frog[pl]->actor, FROG_ANIM_BIGHEAD, NO, NO, 64, 0 );
				actorAnimate( frog[pl]->actor, FROG_ANIM_BREATHE, YES, YES, FROG_BREATHE_SPEED, 0 );
			}

			// update player idleTime
			player[pl].idleTime = ToFixed(MAX_IDLE_TIME);
			//idleCamera = 0;
		}
//#endif
	}

//#ifdef PC_VERSION
	if( debugKeys )
	{
		if( !fixedPos && !fixedDir )
		{
			if(padData.debounce[0] & PAD_L2)
			{
				camFacing[pl]--;
				camFacing[pl] &= 3;
			}

			if(padData.debounce[pl] & PAD_R2)
			{
				camFacing[pl]++;
				camFacing[pl] &= 3;		
			}
		}

		if(padData.debounce[pl] & PAD_R1)
		{
			dkPressed++;

			if(numBabies)
			{
				bby++;
				bby %= numBabies;
				if(bTStart[bby])
					SetFroggerStartPos(bTStart[bby],pl);
//				player[pl].frogState |= FROGSTATUS_ISDEBUG;//mmtemp
			}
		}
	}
//#endif

	lastStickX[pl] = stickX[pl];
	lastStickY[pl] = stickY[pl];
}


/* --------------------------------------------------------------------------------
	Programmer	: Matthew Cloy
	Function	: Run
	

	Purpose		:
	Parameters	: (void)
	Returns		: void 
*/

char* oldStackPointer;
long startCam = 0;
TIMER endLevelTimer;
TIMER screenSaveTimer;

void RunGameLoop (void)
{
	unsigned long i;

	if(player[0].numSpawn == 25)
	{
		arcadeHud.coinsOver->r = arcadeHud.coinsText->r = 127+((rsin(actFrameCount*4000)+4096)*64)/4096;
		arcadeHud.coinsOver->g = arcadeHud.coinsText->g = 127+((rcos(actFrameCount*4000)+4096)*64)/4096;
		arcadeHud.coinsOver->b = arcadeHud.coinsText->b = 0;
	}

#ifdef DREAMCAST_VERSION
	GTUpdate( &screenSaveTimer, -1 );
	if( !screenSaveTimer.time )
	{
		// Fade to 75%
		ScreenFade( 255, 63, 30 );
		keepFade = 1;
	}
#endif

//	if( frameCount==2 )
//	{
//		ScreenFade(0,255,30);
//		keepFade = 0;
//		startCam = 1;
//		GTInit( &endLevelTimer, 0 );
//	}

#ifdef SHOW_ME_THE_TILE_NUMBERS
	// Take this out for release
	if( !tileNum )
		tileNum = CreateAndAddTextOverlay(2048,1024,tileString,YES,(char)255,font,0);
#endif

	// setup for frogger point of interest
	pOIDistance = ToFixed(500000);
	pointOfInterest = NULL;

	TIMER_START3(TIMER_TOTAL);

	if( endLevelTimer.time )
	{
		CheckEOLLoopTrack();
		GTUpdate( &endLevelTimer, -1 );
		if( !endLevelTimer.time )
		{
			cheatCombos[CHEAT_SKIP_LEVEL].state = 0;
			camDist.vx	= 0*SCALE;
			camDist.vy	= 680*SCALE;
			camDist.vz	= 192*SCALE;

			gameState.mode = LEVELCOMPLETE_MODE;
			
			//bb put this line back in
			levelTime = actFrameCount;

			GTInit( &modeTimer, 8 );
			StartLevelComplete();
			return;
		}

		UpdateBabies( );
		if((player[0].worldNum != WORLDID_SPACE) || (player[0].levelNum != LEVELID_SPACE2))
		{
			UpdatePlatforms();
			UpdateEnemies();
		}
		UpdateSpecialEffects( );
		return;
	}

	if ((cheatCombos[CHEAT_SKIP_LEVEL].state) || ( babiesSaved==numBabies && numBabies && (worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum ].multiPartLevel == NO_MULTI_LEV) ))
	{
		GTInit( &endLevelTimer, 4 );
		SetTimeForLevel();
		PrepareSong ( AUDIOTRK_LEVELCOMPLETE, NO );
		actorAnimate(frog[0]->actor,FROG_ANIM_VICTORY,NO,NO,100,NO);
		actorAnimate(frog[0]->actor,FROG_ANIM_DANCE4,YES,YES,70,NO);
		return;
	}
	else
	{
		if ((startCam) || (frameCount < 2))
		{
			SetVectorFF(&currCamSource,&camSource);
			SetVectorFF(&currCamTarget,&camTarget);
			startCam = 0;
		}
		TIMER_START3(TIMER_CAM_POS);
		UpdateCameraPosition();
		TIMER_STOP_ADD3(TIMER_CAM_POS);

//this section should no longer be necessary..
		TIMER_START3(TIMER_SET_CAM);
		SetCamFF(currCamSource,currCamTarget);
		TIMER_STOP_ADD3(TIMER_SET_CAM);

		if( frameCount > 15 )
			for (i=0; i<NUM_FROGS; i++)
				GameProcessController(i);                                      

	}



	for( i=0; i<NUM_FROGS; i++ )
	{
		if( frog[i] )
		{
			TIMER_START3(TIMER_FROG_POS);
			UpdateFroggerPos(i);
			TIMER_STOP_ADD3(TIMER_FROG_POS);

			if( !(player[i].frogState & FROGSTATUS_ISDEAD) )
			{
				frog[i]->draw = 1;
				if (player[i].safe.time)
				{
					if( actFrameCount&1 )
						CreateRingEffect( i );

					GTUpdate( &player[i].safe, -1 );
				}
				else if( player[i].frogState & FROGSTATUS_ISTELEPORTING )
				{
					if( actFrameCount&1 )
						CreateRingEffect( i );
				}

				if( player[i].idleEnable && gameState.multi == SINGLEPLAYER )
				{
					player[i].idleTime-=gameSpeed;
					if( player[i].idleTime < 4096 )
					{
						FroggerIdleAnim(i);
						player[i].idleTime = ToFixed(400 + Random(300));
						if( !Random(4) )
							PlayVoice( i, "frogbored" );
					}
				}
			}
		}
	}  


	TIMER_START3(TIMER_UP_PLAT);
#ifdef PSX_VERSION
	oldStackPointer = SetSp(0x1f800400);
#endif
	UpdatePlatforms();
#ifdef PSX_VERSION
	SetSp(oldStackPointer);
#endif
	TIMER_STOP_ADD3(TIMER_UP_PLAT);

	TIMER_START3(TIMER_UP_ENEM);
#ifdef PSX_VERSION
	oldStackPointer = SetSp(0x1f800400);
#endif
	UpdateEnemies();
#ifdef PSX_VERSION
	SetSp(oldStackPointer);
#endif
	TIMER_STOP_ADD3(TIMER_UP_ENEM);

	TIMER_START3(TIMER_UP_SPEC);
#ifdef PSX_VERSION
	oldStackPointer = SetSp(0x1f800400);
#endif
	UpdateSpecialEffects();
#ifdef PSX_VERSION
	SetSp(oldStackPointer);
#endif
	TIMER_STOP_ADD3(TIMER_UP_SPEC);

	TIMER_START3(TIMER_UP_EVEN);
	UpdateEvents();
	TIMER_STOP_ADD3(TIMER_UP_EVEN);

	TIMER_START3(TIMER_UP_AMBI);
	if( gameState.mode != PAUSE_MODE )
		UpdateAmbientSounds();
	TIMER_STOP_ADD3(TIMER_UP_AMBI);

	TIMER_START3(TIMER_COLLECT);
	ProcessCollectables();
	TIMER_STOP_ADD3(TIMER_COLLECT);

	if( gameState.multi == SINGLEPLAYER )
	{
		if(gameState.mode != LEVELCOMPLETE_MODE)
		{
			TIMER_START3(TIMER_ONSCREEN);
			UpDateOnScreenInfo();
			TIMER_STOP_ADD3(TIMER_ONSCREEN);
		}
	}
	else
	{
		RunMultiplayer();
	}

	// Send network update packet
//	if( gameState.multi == MULTIREMOTE )
//		if( controllerdata[0].button == 0 || (controllerdata[0].button != controllerdata[0].lastbutton) )
//			SendUpdateMessage( );

#ifdef SHOW_ME_THE_TILE_NUMBERS
	// displays the tile numbers
	TIMER_START3(TIMER_TILENUM);
	i = tileCount;
	while(i--)
		if(&firstTile[i] == currTile[0])
			break;

	currTileNum = i;

	TIMER_STOP_ADD3(TIMER_TILENUM);
	
 	if (tileNum)
 		if (tileNum->text)
 		{
 			if (displayingTile)
 				sprintf(tileString,"%d",currTileNum);
 			else
				sprintf(tileString,"");
 		}
#endif
	TIMER_STOP_ADD3(TIMER_TOTAL);
}
