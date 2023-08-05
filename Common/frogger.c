/*

	This file is part of Frogger2, (c) 1997 Interactive Studios Ltd.


	File		: frogger.c
	Programmer	: James Healey
	Date		: 23/03/99

----------------------------------------------------------------------------------------------- */


//#include "directx.h"
//#include "ultra64.h"
//#include <stdio.h>


#include "sonylibs.h"
#include "shell.h"
#include "islpsi.h"

#include "cam.h"
#include "frogger.h"
#include "tongue.h"
#include "anim.h"
#include "frogmove.h"
#include "game.h"
#include "overlays.h"
#include "platform.h"
#include "layout.h"
#include "particle.h"
//#include "newstuff.h"
#ifndef PSX_VERSION
#include <stdio.h>
#endif

PLATFORMLIST platformList;								// the platform list
PLATFORM *destPlatform[MAX_FROGS] = { NULL,NULL,NULL,NULL };	// platform that frog is about to attempt to jump to
PLATFORM *currPlatform[MAX_FROGS] = { NULL,NULL,NULL,NULL };	// platform that frog is currently on
fixed waitScale = 5<<12;

FOG fog = {120,120,160,FOG_OFF,995,5000};

SPRITEOVERLAYLIST spriteOverlayList;		// the sprite overlay list

SPRITELIST sprList;

TEXTOVERLAYLIST	textOverlayList;			// the text overlay linked list

unsigned int xseed = (7789<<16)+13399;	
int pauseController = 0;

VECTOR zero = {0, 0, 0};
//FLVECTOR flZero = {0, 0, 0};
//QUATERNION zeroQuat = {0,0,0,1};
//QUATERNION vertQ = {0,1,0,0};
IQUATERNION vertQ = {0,4096,0,0};
// MATRIXSTACK	matrixStack;
// MATRIXSTACK	rMatrixStack;

int PFX_NUMPARTICLES = 25;
PARTICLELIST partList;

FVECTOR upVec	= {0,4096,0};
FVECTOR inVec	= {0,0,4096};
FVECTOR outVec	= {0,0,-4096};
FVECTOR rightVec= {4096,0,0};
FVECTOR oneVec = {4096,4096,4096};

SVECTOR *pointOfInterest;
fixed pOIDistance = 20000<<12;


// Display name, model name, icon filename minus extension

//sb this table has the final 4 characters locked at game start
FROGSTORE frogPool[FROG_NUMFROGS] = 
{
	{ "Frogger", 	"frogger.obe",	"mfrogger.obe",	"FROGGER",	1, FANIM_ALL,	80,	255,80 },
	{ "Lillie", 	"lilli.obe",	"mlilli.obe",	"LILLIE",	1, FANIM_ALL,	255,80, 80 },
	{ "Tad",	  	"tad.obe",		"mtad.obe",		"BABYFROG",	1, FANIM_ALL,	80,	80,	255 },
	{ "Treeby", 	"twee.obe",		"mtwee.obe",	"TWEE",		1, FANIM_ALL,	0,	255,255 },
	{ "Wart",	  	"toad.obe",		"mtoad.obe",	"WART",		0, FANIM_ALL,	110,60, 55 },
	{ "Tank",	  	"tank.obe",		"mtank.obe",	"ROBOFROG",	0, FANIM_ALL,	220,220,220 },
	{ "Roobie", 	"roobie.obe",	"mhopper.obe",	"HOPPER",	0, FANIM_ALL,	220,220,30 },
	{ "Swampy", 	"swampy.obe",	"mswampy.obe",	"SWAMPY",	0, FANIM_ALL,	0,	100,0 },
};

//----------------------------------------------------------------------------//
//----- GLOBALS --------------------------------------------------------------//
//----------------------------------------------------------------------------//

PLAYER player[MAX_FROGS];

ACTOR2 *frog[MAX_FROGS]					= {0,0,0,0};
ACTOR2 *hat[MAX_FROGS]					= {0,0,0,0};

long NUM_FROGS = 1;

fixed globalFrogScale			= 2457;
fixed frogRadius				= 200<<12;

TextureType *frogEyeOpen,*frogEyeClosed;

//----------------------------------------------------------------------------//
//----- FUNCTIONS ------------------------------------------------------------//
//----------------------------------------------------------------------------//
void CreateFrogActor (GAMETILE *where, char *name,long p)
{
	ACTOR2 *me = NULL;

#ifdef PSX_VERSION
//bb
//	me = frog[p] = CreateAndAddActor("AFROG.PSI",0,0,819200,INIT_ANIMATION | INIT_SHADOW,ToFixed(0),0);
	me = frog[p] = CreateAndAddActor(name,0,0,0,INIT_ANIMATION | INIT_SHADOW);
#else
//bb
//	me = frog[p] = CreateAndAddActor("frogger.obe",0,0,819200,INIT_ANIMATION | INIT_SHADOW,ToFixed(0),0);
	me = frog[p] = CreateAndAddActor(name,0,0,0,INIT_ANIMATION | INIT_SHADOW);
#endif


/*	
	hat[p] = CreateAndAddActor("hat-top.obe",0,0,200.0,INIT_ANIMATION | INIT_SHADOW,0,0);
	
	if (hat[p])
	{
		actList = actList->next;
		actList->prev = 0;
	}
*/

#ifdef PC_VERSION
	if( !rHardware )
		me->depthShift += -50;
#endif
	
	me->actor->shadow->radius = ToFixed(globalFrogScale/8);//mmshadow
	me->actor->shadow->alpha = 191;//mmshadow
	
	me->flags	= ACTOR_DRAW_ALWAYS;
	
	tongue[p].flags = TONGUE_NONE | TONGUE_IDLE;
	
	if( gameState.multi == SINGLEPLAYER )
		actorAnimate(me->actor,FROG_ANIM_DANCE1,YES,NO,50,0);
	else
		actorAnimate(me->actor,FROG_ANIM_BREATHE,YES,NO,50,0);
	
	if (where)
		SetFroggerStartPos(where,p);
	else
		currTile[p] = NULL;

	player[p].healthPoints	= 1;
	player[p].frogon		= -1;
	player[p].frogunder		= -1;
	player[p].jumpTime		= -1;
	player[p].numSpawn		= 0;

	me->radius				= frogRadius;	//
}

void CreateFrogger(unsigned char createFrogActor,unsigned char createFrogOverlays )
{
	int i, num;

	if(createFrogActor)
	{
		for (i=0; i<NUM_FROGS; i++)
		{
			num = player[i].character;
			if( num > FROG_NUMFROGS || !frogPool[num].active )
			{
				player[i].character = num = 0;
			}

			currPlatform[i] = NULL;

			if( gameState.multi == SINGLEPLAYER )
				CreateFrogActor( (gTStart[i]?gTStart[i]:gTStart[0]), frogPool[num].singleModel, i);
			else
				CreateFrogActor( ((gTStart[i])?(gTStart[i]):(gTStart[0])), frogPool[num].multiModel, i );
		}
	}

// sb removed this 'cus it fucks up the camera on loading
//	InitCamera();
}

/*	--------------------------------------------------------------------------------
	Function 	: FroggerIdleAnim
	Purpose 	: Choose and play an idle animation
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
void FroggerIdleAnim( int i )
{
	if (currTile[i])
	{
		GAMETILE *t = currTile[i]->tilePtrs[(frogFacing[i] + camFacing[i]) & 3];

		if( t && Random(3) )
		{
			// Play a looking-over-the-edge animation when we're next to a drop or deadly tile
			if (t->state == TILESTATE_JOIN)
			{
				FVECTOR v;
				fixed height;
				
				SubVectorFSS(&v, &t->centre, &currTile[i]->centre);
				height = DotProductFF(&v, &currTile[i]->normal);

				if (height < -10<<12)
				{
					actorAnimate(frog[i]->actor, FROG_ANIM_LOOKDOWN, NO, NO, 40,0);
					actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES, 50,0);
					return;
				}
			}
			else if (t->state == TILESTATE_DEADLY)
			{
				actorAnimate(frog[i]->actor, FROG_ANIM_LOOKDOWN, NO, NO, 50,0);
				actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES, 50,0);
				return;
			}
		}
	}

	// otherwise, play a normal idle
	switch( Random(9) )
	{
	case 0:
		actorAnimate(frog[i]->actor,FROG_ANIM_PREEN,NO,NO,50,0);
		if( actFrameCount&1 )
			actorAnimate(frog[i]->actor,FROG_ANIM_PREEN,NO,YES,50,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES,50,0);
		break;
	case 1:
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE1,NO,NO,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES,50,0);
		break;
	case 2:
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE2,NO,NO,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES,50,0);
		break;
	case 3:
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE3,NO,NO,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES,50,0);
		break;
	case 4:
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE4,NO,NO,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES,50,0);
		break;
	case 5:
		actorAnimate(frog[i]->actor,FROG_ANIM_LOOKAROUND,NO,NO,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES,50,0);
		break;
	case 6:
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE1,NO,NO,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE2,NO,YES,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE3,NO,YES,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES,50,0);
		break;
	case 7:
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE2,NO,NO,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE3,NO,YES,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE4,NO,YES,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES,50,0);
		break;
	case 8:
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE1,NO,NO,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE2,NO,YES,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE3,NO,YES,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_DANCE4,NO,YES,80,0);
		actorAnimate(frog[i]->actor,FROG_ANIM_BREATHE,YES,YES,50,0);
		break;
	}
}

