/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: training.h
	Programmer	: David Swift
	Date		: 11 May 00

----------------------------------------------------------------------------------------------- */

#include <islmem.h>
#include <islpad.h>

#include "define.h"
#include "overlays.h"
#include "layout.h"
#include "main.h"
#include "training.h"
#include "game.h"
#include "sprite.h"
#include "frogger.h"
#include "hud.h"
#include "babyfrog.h"
#include "frogmove.h"

char *lastStr = NULL;
char *currStr = NULL;

TRAININGMODE *train;

extern short numLives[3];
void InitTrainingMode()
{
	int i;

	train = (TRAININGMODE*)MALLOC0(sizeof(TRAININGMODE));

	gameState.mode = TRAINING_MODE;
	gameState.multi = SINGLEPLAYER;

	DisableHUD();


	for(i = 0;i < numBabies;i++)
	{
		arcadeHud.babiesBack[i]->draw = 1;
		babyIcons[i]->draw = 1;
	}

	train->bg = CreateAndAddSpriteOverlay(0, 4096, "SHEEN", 4096, 900, 180, 0);
	train->bg->speed = 75<<12;
	train->bg->r = train->bg->g = train->bg->b = 128;

	for (i=0; i<2; i++)
	{
		TEXTOVERLAY *t = train->txtover[i] = CreateAndAddTextOverlay(2048, 4096 + 204 + i*250, train->text, YES, 255, fontSmall, TEXTOVERLAY_SHADOW);
		train->txtover[i]->speed = 75<<12;
	}
	currStr = lastStr = NULL;
	player[0].lives = numLives[gameState.difficulty];
}

void RunTrainingMode(void)
{
	RunGameLoop();

	if(train)
		TrainingHint(currStr);
}


void TrainingHint(const char* str)
{
	SPECFX *fx;

	if(currStr == NULL)
		currStr = (char *)str;
	if (str)
	{
		#define MAX_LINES (4)

		char *lines[MAX_LINES];

		if (train->bg)
		{
			if(lastStr != str)
			{
				currStr = (char *)str;
				if( (fx = CreateSpecialEffect( FXTYPE_SPARKLYTRAIL, &currTile[0]->centre, &upVec, 81920, 8192*2, 0, 12288 )) )
				{
					SetFXColour(fx,255,Random(256),Random(256));
					if(fx->rebound)
					{
						SetVectorSS(&fx->rebound->point, &currTile[0]->centre);
						SetVectorFF(&fx->rebound->normal, &upVec);	
					}
					fx->gravity = 4100;
				}

				if(train->bg->yPosTo == train->bg->yPos)
				{
					if(train->bg->yPos >= 4096)
					{
						StringWrap( str, 4096-128, train->text, 1024, lines, MAX_LINES, fontSmall);	// eee

						train->txtover[0]->text = lines[0];
						train->txtover[1]->text = lines[1];

						lastStr = (char *)str;
						train->bg->yPosTo = 3296;
						train->bg->speed = 75<<12;
						train->txtover[0]->yPosTo = 3500;
						train->txtover[0]->speed = 75<<12;
						train->txtover[1]->yPosTo = 3500+250;
						train->txtover[1]->speed = 75<<12;
					}
					else
					{
						StringWrap(lastStr, 4096-128, train->text, 1024, lines, MAX_LINES, fontSmall);	// eee

						train->txtover[0]->text = lines[0];
						train->txtover[1]->text = lines[1];

						train->bg->yPosTo = 4096; 
						train->bg->speed = 75<<12;
						train->txtover[0]->yPosTo = 4096+204; 
						train->txtover[0]->speed = 75<<12;
						train->txtover[1]->yPosTo = 4096+204+250;
						train->txtover[1]->speed = 75<<12;
					}
				}
			}
			else
			{
				StringWrap(str, 4096-128, train->text, 1024, lines, MAX_LINES, fontSmall);	// eee

				train->txtover[0]->text = lines[0];
				train->txtover[1]->text = lines[1];
			}
		}
	}
	else
	{
		train->bg->yPosTo = 4096; train->bg->speed = 75<<12;
		train->txtover[0]->yPosTo = 4096+204; train->txtover[0]->speed = 75<<12;
		train->txtover[1]->yPosTo = 4096+204+250; train->txtover[1]->speed = 75<<12;
	}
}
