/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: actor.c
	Programmer	: Matthew Cloy
	Date		: 11/11/98

----------------------------------------------------------------------------------------------- */
#include <math.h>
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

#define F3DEX_GBI_2
#define LEAVEOUT_MATH

#include "actor.h"
#include "frogger.h"
#include "enemies.h"
#include "babyfrog.h"
#include "game.h"
#include "Types.h"
#include "maths.h"
#include "cam.h"
#include "sprite.h"
#include "frogmove.h"
#include <islmem.h>
#include <islutil.h>
#include "Main.h"
#include "mdx.h"
#include "pcaudio.h"
#include "tongue.h"
#include "pcmisc.h"


void actorAnimate(ACTOR *actor, int animNum, char loop, char queue, int speed, char skipendframe)
{
	if (actor->actualActor)
		Animate((MDX_ACTOR *)actor->actualActor,animNum,loop,queue,speed/256.0f);
}

void actorResetAnimation(ACTOR *actor)
{
	MDX_ACTOR* a = (MDX_ACTOR*)actor->actualActor;
	
	if (a && a->animation->anims)
		a->animation->animTime = a->animation->anims[a->animation->currentAnimation].animStart;
}


void actorFree(ACTOR *actor)
{
	if( actor->shadow )
		FREE( actor->shadow );

	// Free the mdx_actor
	if (actor->actualActor)
		FreeActor((MDX_ACTOR **)&actor->actualActor);

	FREE(actor);
}

/*	--------------------------------------------------------------------------------
	Function 	: ChangeModel
	Purpose 	: Swap models within an actor
	Parameters 	: actor, name of new model
	Returns 	: void
	Info 		:
*/
void ChangeModel( ACTOR *actor, char *model )
{
	MDX_ACTOR *a = (MDX_ACTOR *)actor->actualActor;
	char newName[16];
	int i=0;

	while( model[i] != '.' && model[i] != '\0' )
	{
		newName[i] = model[i];
		i++;
	}
	newName[i] = '\0';
	strcat( newName, ".obe" );

	a->LODObjectController = a->objectController;
	FindObject( &a->objectController, UpdateCRC(newName), newName );
	InitAnims( a );
}


/*	--------------------------------------------------------------------------------
	Function 	: UndoChangeModel
	Purpose 	: Swap models back after a ChangeModel
	Parameters 	: actor
	Returns 	: void
	Info 		:
*/
int UndoChangeModel( ACTOR *actor )
{
	MDX_ACTOR *a = (MDX_ACTOR *)actor->actualActor;

	if( a->LODObjectController )
	{
		a->objectController = a->LODObjectController;
		a->LODObjectController = NULL;
		InitAnims( a );
		actorAnimate( actor, FROG_ANIM_BREATHE, YES, NO, FROG_BREATHE_SPEED, 0);
		return 1;
	}
	return 0;
}


/*	--------------------------------------------------------------------------------
	Function	: AnimateActor
	Purpose		: Runs an animation on an actor EXCEPT when it's already running
	Parameters	: actor, anim, loop?, queue?, speed, morph?, keep proportion?
	Returns		: void
*/
void StartAnimateActor(ACTOR *actor, int animNum, char loop, char queue, int speed, char skip)
{
	MDX_ACTOR *a = (MDX_ACTOR *)actor->actualActor;

 	if (a->animation->currentAnimation != animNum)
 		actorAnimate(actor, animNum, loop, queue, speed, skip);
}

void FindFrogSubObjects( int p )
{
	char objName[8];

	if( !headMatrix )
	{
		MDX_OBJECT *obj;

		strcpy(objName, headNames[player[p].character]);

		if( (obj = FindActorSubObject( (MDX_ACTOR *)frog[0]->actor->actualActor, objName )) )
		{
			headMatrix = (float *)obj->objMatrix.matrix;

			if( !obj->postMatrix )
			{
				headPostMatrix = obj->postMatrix = (float *)MALLOC0(sizeof(float)*16);
				mtxSetIdent(headPostMatrix);
			}
		}
	}
	if( !breastMatrix )
	{
		MDX_OBJECT *obj;

		strcpy( objName, "fgshld" );
		if( player[0].character == FROG_LILLIE )
			objName[1] = 'm';
		if( (obj = FindActorSubObject( (MDX_ACTOR *)frog[0]->actor->actualActor, objName )) )
			if( !obj->postMatrix )
			{
				breastMatrix = obj->postMatrix = (float *)MALLOC0(sizeof(float)*16);
				mtxSetIdent(breastMatrix);
			}
	}
}

char croakDir			= 0;

void UpdateFrogCroak( int pl )
{
	SVECTOR effectPos;

	// frog is croaking
	if(player[pl].frogState & FROGSTATUS_ISCROAKING)
	{
		SPECFX *fx;

		if( breastMatrix && headPostMatrix )
		{
			if( croakDir > 1 )
			{
				croakDir -= max((int)timeInfo.speed, 1);
				if( croakDir < 1 ) croakDir = 1;
			}
			else if( breastMatrix[0] < 1.5 && !croakDir )
			{
				breastMatrix[0] *= 1+(0.1*timeInfo.speed);
				breastMatrix[5] *= 1+(0.1*timeInfo.speed);
				breastMatrix[10] *= 1+(0.1*timeInfo.speed);

				if( breastMatrix[0] >= 1.5 )
				{
					croakDir = 10;
					breastMatrix[0] = breastMatrix[5] = breastMatrix[10] = 1.5;
				}

				headPostMatrix[0] *= 1+(0.08*timeInfo.speed);
				headPostMatrix[5] *= 1+(0.08*timeInfo.speed);
				headPostMatrix[10] *= 1+(0.08*timeInfo.speed);

				if( headPostMatrix[0] >= 1.5 )
				{
					headPostMatrix[0] = headPostMatrix[5] = headPostMatrix[10] = 1.5;
				}
			}
			else if( breastMatrix[0] > 1 && croakDir == 1 )
			{
				breastMatrix[0] *= 1-(0.1*timeInfo.speed);
				breastMatrix[5] *= 1-(0.1*timeInfo.speed);
				breastMatrix[10] *= 1-(0.1*timeInfo.speed);

				if( breastMatrix[0] <= 1 )
				{
					croakDir = 2;
					breastMatrix[0] = breastMatrix[5] = breastMatrix[10] = 1;
				}

				headPostMatrix[0] *= 1-(0.08*timeInfo.speed);
				headPostMatrix[5] *= 1-(0.08*timeInfo.speed);
				headPostMatrix[10] *= 1-(0.08*timeInfo.speed);

				if( headPostMatrix[0] <= 1 )
				{
					headPostMatrix[0] = headPostMatrix[5] = headPostMatrix[10] = 1;
				}
			}
		}

		if( !(player[pl].isCroaking.time%2) )
		{
			FVECTOR up;
			SetVectorSS(&effectPos, &frog[pl]->actor->position);
			SetVectorFF( &up, &currTile[pl]->normal );

			PrepForPriorityEffect( );
			if( (fx = CreateSpecialEffectDirect( FXTYPE_CROAK, &effectPos, &up, 81920, 4096, 410, 6144 )) )
			{
				fx->spin = 25;
				SetFXColour( fx, 191, 255, 0 );
			}
			if( moolardCheat )
				PlaySample( FindSample(UpdateCRC("mullard")), NULL, 0, SAMPLE_VOLUME, -1 );
			else
				PlayVoice( pl, "frogcroak" );
		}

		GTUpdate( &player[pl].isCroaking, -1 );
		if( !player[pl].isCroaking.time )
		{
			int baby;

			player[pl].frogState &= ~FROGSTATUS_ISCROAKING;

			// check for nearest baby frog - do radius check ????
			baby = GetNearestBabyFrog();

			if( baby != -1 )
			{
				FVECTOR up;
				SVECTOR pos;
				SetVectorSF( &pos, &upVec );
				ScaleVector( &pos, 20 );
				AddToVectorSS( &pos, &babyList[baby].baby->actor->position );
				SetVectorFF( &up, &currTile[pl]->normal );

				PrepForPriorityEffect( );
				if( (fx = CreateSpecialEffectDirect( FXTYPE_CROAK, &pos, &up, 81920, 4096, 410, 6144 )) )
				{
					int n;

					if( babyList[baby].enemy->path->nodes->offset2 )
						n = ((babyList[baby].enemy->path->nodes->offset2>>12)/SCALE)-1;
					else
						n = baby;

					fx->spin = 25;
					SetFXColour( fx, babyList[n].fxColour[R], babyList[n].fxColour[G], babyList[n].fxColour[B] );
				}

				PlaySample( genSfx[GEN_BABYREPLY], &pos, 0, SAMPLE_VOLUME, -1 );
			}
		}
	}
	else
	{
		if( breastMatrix )
		{
			breastMatrix[0] = breastMatrix[5] = breastMatrix[10] = 1;
			croakDir = 0;
		}
		if( headPostMatrix )
			headPostMatrix[0] = headPostMatrix[5] = headPostMatrix[10] = 1;
	}
}
