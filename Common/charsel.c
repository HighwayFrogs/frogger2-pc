/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: charsel.c
	Programmer	: David Swift
	Date		: 27/03/00 

----------------------------------------------------------------------------------------------- */

#include <islpad.h>
#include "charsel.h"
#include "textover.h"
#include "frontend.h"
#include "define.h"
#include "game.h"
#include "main.h"
#include "frogger.h"
#include "frogmove.h"
#include "cam.h"
#include "layout.h"
#include "maths.h"
#include "fixed.h"
#include "overlays.h"
#include "story.h"
#include <islutil.h>
#include "lang.h"

#ifdef PC_VERSION
#include <banks.h>
#endif

#ifdef PC_VERSION
#define SPOT_LEFT "xxa_sptl.obe"
#define SPOT_RIGHT "xxa_sptr.obe"
#elif PSX_VERSION
#define SPOT_LEFT "XXA_SPTL.PSI"
#define SPOT_RIGHT "XXA_SPTR.PSI"
#endif

typedef struct _CHARSELECT_MENU
{
	CHARSELOPTIONS options[2];

	TEXTOVERLAY *title;
	TEXTOVERLAY *names[2], *level[2];
	ACTOR2 *lights[2];
	SPRITEOVERLAY *subtr[2];
	char selection;
	char levelText[2][30];
} CHARSELECT_MENU;

static CHARSELECT_MENU charSel;

/* ------------ Constants -------------- */

const static FVECTOR charSelCamPos =		{ 0, 2400000, 8000000 };
const static FVECTOR charSelCamTarget =	{ 0, 2400000, 0 };

const static SVECTOR spotLightPos = { 300, 1800, 0 };
const static SVECTOR selectedFrogPos = { 625, 0, 0 };
const static SVECTOR unselectedFrogPos = { 625, 0, 0 };

/*	--------------------------------------------------------------------------------
	Function		: StartCharacterSelectMode
	Purpose			: initialises and starts character select mode.
	Parameters		: character index x 2
	Returns			: void (char select mode calls StorySelectCharacter when complete)
*/

void StartCharacterSelectMode(CHARSELOPTIONS* char1, CHARSELOPTIONS* char2)
{
	utilPrintf("Starting character select mode ..\n");

	gameState.mode = CHARSELECT_MODE;
	FreeAllLists();

	LoadTextureBank(INGAMEGENERIC_TEX_BANK);
	LoadObjectBank(INGAMEGENERIC_OBJ_BANK);
//	LoadObjectBank(MULTIPLAYER_OBJ_BANK);

	charSel.options[0] = *char1;
	charSel.options[1] = *char2;

	// Text overlays
	
	charSel.title = CreateAndAddTextOverlay(2048, 256, "Choose your frog", YES, 255, fontSmall, 0);

	charSel.names[0] = CreateAndAddTextOverlay(512, 3600, frogPool[char1->character].name, NO, 255, font, 0);
	charSel.names[1] = CreateAndAddTextOverlay(2800, 3600, frogPool[char2->character].name, NO, 0x80, font, 0);

	strcpy(charSel.levelText[0], GAMESTRING(worldVisualData[char1->world].levelVisualData[char1->level].description_str));
	strcpy(charSel.levelText[1], GAMESTRING(worldVisualData[char2->world].levelVisualData[char2->level].description_str));

	charSel.level[0] = CreateAndAddTextOverlay(625, 512, charSel.levelText[0], NO, 255, fontSmall, 0);
	charSel.level[1] = CreateAndAddTextOverlay(3000, 512, charSel.levelText[1], NO, 255, fontSmall, 0);

	// Camera

	fixedPos = 1;
	fixedDir = 1;
	
	SetVectorFF(&currCamTarget, &charSelCamTarget);
	SetVectorFF(&currCamSource, &charSelCamPos);

	// use players to hold our actors.. tee hee

	CreateFrogActor(0, frogPool[char1->character].singleModel, 0);
	frog[0]->actor->position.vx = -625;
	frog[0]->actor->shadow->draw = 0;
	actorAnimate(frog[0]->actor, FROG_ANIM_DANCE1, YES, NO, 64, NO);

	CreateFrogActor(0, frogPool[char2->character].singleModel, 1);
	frog[1]->actor->position.vx = 625;
	frog[1]->actor->position.vz = -512;
	frog[0]->actor->shadow->draw = 0;
	actorAnimate(frog[1]->actor, FROG_ANIM_BREATHE, YES, NO, 64, NO);

	// spotlights

	charSel.lights[0] = CreateAndAddActor(SPOT_LEFT,
		-spotLightPos.vx, spotLightPos.vy, spotLightPos.vz, 0);

	charSel.lights[1] = CreateAndAddActor(SPOT_RIGHT,
		spotLightPos.vx, spotLightPos.vy, spotLightPos.vz, 0);

//todo: proper prefix control in CreateAndAdd... (urgh)
#ifdef PC_VERSION
	((MDX_ACTOR *)(charSel.lights[0]->actor->actualActor))->objectController->object->flags =  OBJECT_FLAGS_ADDITIVE;
	((MDX_ACTOR *)(charSel.lights[1]->actor->actualActor))->objectController->object->flags =  OBJECT_FLAGS_ADDITIVE;
#endif

	// subtractive sprite overlays

	charSel.subtr[0] = CreateAndAddSpriteOverlay(0, 0, NULL, 2048, 4096, 0, SPRITE_SUBTRACTIVE);
	charSel.subtr[0]->r = charSel.subtr[0]->g = charSel.subtr[0]->b = 0;
	charSel.subtr[0]->draw = 0;

	charSel.subtr[1] = CreateAndAddSpriteOverlay(2048, 0, NULL, 2048, 4096, 0, SPRITE_SUBTRACTIVE);
	charSel.subtr[1]->r = charSel.subtr[1]->g = charSel.subtr[1]->b = 0xA0;

	// set up some other stuff

	firstTile = NULL;
	charSel.selection = 0;
}

/*	--------------------------------------------------------------------------------
	Function		: CharSelChangeSelection
	Purpose			: runs animations and calculates hops for selected and deselected character
	Parameters		: 
	Returns			: 
*/

void CharSelChangeSelection()
{
	int i;
	SVECTOR tpos;

	for (i=0;i<2;i++)
	{
		if (charSel.selection==i)
		{
			charSel.names[i]->a = 0xFF;
			actorAnimate(frog[i]->actor, FROG_ANIM_SUPERHOP, NO, NO, 96, 0);
			actorAnimate(frog[i]->actor, FROG_ANIM_DANCE2, NO, YES, 64, 0);
			actorAnimate(frog[i]->actor, FROG_ANIM_DANCE1, YES, YES, 64, 0);

			SetVectorSS(&tpos, &frog[i]->actor->position);
			tpos.vz = 0;
			CalculateFrogJump(&frog[i]->actor->position, &tpos, &upVec, 50, 48, i);
		}
		else
		{
			charSel.names[i]->a = 0x80;
			actorAnimate(frog[i]->actor, FROG_ANIM_SOMERSAULT, NO, NO, -96, 0);
			actorAnimate(frog[i]->actor, FROG_ANIM_BREATHE, YES, YES, 64, 0);

			SetVectorSS(&tpos, &frog[i]->actor->position);
			tpos.vz = -700;
			CalculateFrogJump(&frog[i]->actor->position, &tpos, &upVec, 100, 32, i);
		}
	}
}

/*	--------------------------------------------------------------------------------
	Function		: RunCharacterSelectMode
	Purpose			: called by GameLoop() for char select mode
	Parameters		: void
	Returns			: void (char select mode calls StorySelectCharacter when complete)
*/

void RunCharacterSelectMode(void)
{
	int pl;

	for (pl=0;pl<2;pl++)
	{
		SPRITEOVERLAY* s = charSel.subtr[pl];

		FroggerHop(pl);

		if (player[pl].jumpTime > ToFixed(1))
		{
			frog[pl]->actor->position.vz = (charSel.selection==pl)?0:-700;
			frog[pl]->actor->position.vy = 0;
			player[pl].jumpTime = -1;
		}

		if (pl==charSel.selection)
		{
			
			if (s->draw)
			{
				if (s->r < 10)
					s->draw = 0;
				else
					s->r = s->g = s->b = (s->r-10);
			}
		}
		else
		{
			s->draw = 1;
			if (s->r < 0xA0)
			{
				s->r = s->g = s->b = (s->r+10);
			}
			charSel.subtr[pl]->draw = 1;
		}
	}

	if (padData.debounce[0]&PAD_LEFT && (charSel.selection > 0))
	{
		charSel.selection--;
		CharSelChangeSelection();
	}

	if (padData.debounce[0]&PAD_RIGHT && (charSel.selection < 1))
	{
		charSel.selection++;
		CharSelChangeSelection();
	}

	if (padData.debounce[0]&PAD_CROSS||padData.debounce[0]&PAD_START)
	{
		CHARSELOPTIONS* opt;
		utilPrintf("Leaving char select mode (Selection: %d)\n", (int)charSel.selection);
		// todo: We need a pretty effect and an animation here!

		opt = &charSel.options[charSel.selection];

		gameState.mode = INGAME_MODE;
		player[0].character = opt->character;
		player[0].worldNum = opt->world;
		player[0].levelNum = opt->level;
		InitLevel(opt->world, opt->level);
	}
}