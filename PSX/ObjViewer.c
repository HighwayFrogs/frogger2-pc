#include <islpad.h>
#include "sonylibs.h"
#include "shell.h"
#include "types.h"
#include "islpsi.h"
#include "psi.h"
#include "psiactor.h"
#include "main.h"
#include "layout.h"
#include "types2d.h"
#include "actor2.h"
#include "cam.h"

#include "frogger.h"
#include "objviewer.h"

#include "world.h"

int worldSelect = 0;
int levelSelect = 0;

int worldSelecting = 1;

int viewMode;

int currentAnimation = 0;
int animSpeed = 0;

TEXTOVERLAY *worldNames [ MAX_WORLDS ];
TEXTOVERLAY *levelNames [ MAX_LEVELS ];

ACTOR2 *objViewerActorList [ MAX_OBJVIEWER_ACTORS ];

void customDrawPrimitives2(int depth);
//void LSCAPE_DrawSortedPrimitives(int depth);

void InitObjectViewer ( void )
{
	FreeAllLists();
	viewMode = OBJECT_BANK_SELECT;
}

void RunBankSelect ( void )
{
	int counter;
	static char init = 0;

	if ( !init )
	{
		init ^= 1;

		//CreateAndAddTextOverlay(128, 256, "Object Viewer", 0, 128, font, 0, 0 );

		/*for ( counter = 0; counter < MAX_WORLDS; counter++ )
		{
			worldNames[counter] = CreateAndAddTextOverlay(0, (200 * counter)+768, worldVisualData [ counter ].description_str, 0, 128, fontSmall, 0, 0 );
		}
		// ENDFOR
		for ( counter = 0; counter < worldVisualData [ 0 ].numLevels; counter++ )
		{
			levelNames [ counter ] = CreateAndAddTextOverlay(2048, (200 * counter)+768, worldVisualData [ 0 ].levelVisualData [ counter ].description_str, 0, 128, fontSmall, 0, 0 ); 
		}
		// ENDFOR*/
	}
	// ENDIF

	if ( padData.debounce[0] & PAD_CROSS )
	{
		psiInitialise(100);

		LoadTextureBank ( INGAMEGENERIC_TEX_BANK );

		LoadTextureBank ( worldVisualData [ worldSelect ].masterTextureBank );
		LoadObjectBank ( worldVisualData [ worldSelect ].levelVisualData [ levelSelect ].objectBankToUse );

		CreateFrogger ( 1, 1 );
		objViewerActorList[0] = CreateAndAddActor ( "dth-ball.PSI", 0, 0, 0, INIT_ANIMATION); 

		actorAnimate ( objViewerActorList[0]->actor, 0, 1, 0, 128, 0 );

//		psiRegisterDrawFunction (customDrawPrimitives2);
//		psiRegisterDrawFunction(LSCAPE_DrawSortedPrimitives);
	//	psiRegisterDrawFunction2(customDrawPrimitives2);

		psiAllocWorkspace();

		viewMode = OBJECT_BANK_VIEW;

		return;
	}
	// ENDIF

	if ( padData.debounce[0] & PAD_RIGHT )
		worldSelecting = 0;

	if ( padData.debounce[0] & PAD_LEFT )
		worldSelecting = 1;

	if ( padData.debounce[0] & PAD_UP )
	{
		if ( worldSelecting )
		{
			for ( counter = 0; counter < worldVisualData [ worldSelect ].numLevels; counter++ )
			{
				//SubTextOverlay ( levelNames [ counter ] );
				//levelNames [ counter ] = NULL;
			}
			// ENDFOR

			worldSelect--;
			if ( worldSelect < 0 ) worldSelect = 0;

			for ( counter = 0; counter < worldVisualData [ worldSelect ].numLevels; counter++ )
			{
				//levelNames [ counter ] = CreateAndAddTextOverlay(2048, (200 * counter)+768, worldVisualData [ worldSelect ].levelVisualData [ counter ].description_str, 0, 128, fontSmall, 0, 0 ); 
			}
			// ENDFOR
		}
		else
		{
		}
		// ENDELSEIF
	}
	// ENDIF

	if ( padData.debounce[0] & PAD_DOWN )
	{
		if ( worldSelecting )
		{
			for ( counter = 0; counter < worldVisualData [ worldSelect ].numLevels; counter++ )
			{
				//SubTextOverlay ( levelNames [ counter ] );
				//levelNames [ counter ] = NULL;
			}
			// ENDFOR

			worldSelect++;
			if ( worldSelect > MAX_WORLDS-1 ) worldSelect = MAX_WORLDS-1;

			for ( counter = 0; counter < worldVisualData [ worldSelect ].numLevels; counter++ )
			{
				//levelNames [ counter ] = CreateAndAddTextOverlay(2048, (200 * counter)+768, worldVisualData [ worldSelect ].levelVisualData [ counter ].description_str, 0, 128, fontSmall, 0, 0 ); 
			}
			// ENDFOR
		}
		else
		{
		}
		// ENDELSEIF
	}
	// ENDIF

	for ( counter = 0; counter < MAX_WORLDS; counter++ )
	{
		/*worldNames [ counter ]->r = 30;
		worldNames [ counter ]->g = 30;
		worldNames [ counter ]->b = 30;*/
	}
	// ENDFOR

	for ( counter = 0; counter < MAX_WORLDS; counter++ )
	{
		if ( levelNames [ counter ] )
		{
			/*levelNames [ counter ]->r = 30;
			levelNames [ counter ]->g = 30;
			levelNames [ counter ]->b = 30;*/
		}
		// ENDIF
	}
	// ENDFOR

	/*worldNames [ worldSelect ]->r = 255;
	worldNames [ worldSelect ]->g = 255;
	worldNames [ worldSelect ]->b = 255;

	levelNames [ levelSelect ]->r = 255;
	levelNames [ levelSelect ]->g = 255;
	levelNames [ levelSelect ]->b = 255;*/

//	PrintTextOverlays();
}

void RunBankView ( void )
{



	camera.vpx = 0;
	camera.vpy = 0;
	camera.vpz = 5000;

	camera.vrx = 0;
	camera.vry = 0;
	camera.vrz = 0;

	camera.rz  = 0;

	GsSetRefView2(&camera);

	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);

	objViewerActorList[0]->actor->position.vx = 0;
	objViewerActorList[0]->actor->position.vy = 0;
	objViewerActorList[0]->actor->position.vz = 0;

	if ( padData.digital[0] & PAD_LEFT )
	{
		objViewerActorList[0]->actor->psiData.object->rotate.vy -= 10;
	}
	// ENDIF

	if ( padData.digital[0] & PAD_RIGHT )
	{
		objViewerActorList[0]->actor->psiData.object->rotate.vy += 10;
	}
	// ENDIF

	if ( padData.digital[0] & PAD_UP )
	{
		objViewerActorList[0]->actor->psiData.object->rotate.vx += 10;
	}
	// ENDIF
	if ( padData.digital[0] & PAD_DOWN )
	{
		objViewerActorList[0]->actor->psiData.object->rotate.vx -= 10;
	}
	// ENDIF

	if ( padData.digital[0] & PAD_L1 )
	{
		objViewerActorList[0]->actor->psiData.object->rotate.vz += 10;
	}
	// ENDIF
	if ( padData.digital[0] & PAD_L2 )
	{
		objViewerActorList[0]->actor->psiData.object->rotate.vz -= 10;
	}
	// ENDIF

	if ( padData.debounce[0] & PAD_CIRCLE )
	{
		currentAnimation++;
		actorAnimate ( objViewerActorList[0]->actor, currentAnimation, 1, 0, animSpeed, 0 );
	}
	// ENDIF
	if ( padData.debounce[0] & PAD_SQUARE )
	{
		currentAnimation--;
		actorAnimate ( objViewerActorList[0]->actor, currentAnimation, 1, 0, animSpeed, 0 );
	}
	// ENDIF

	if ( padData.debounce[0] & PAD_TRIANGLE )
	{
		animSpeed++;
		actorAnimate ( objViewerActorList[0]->actor, currentAnimation, 1, 0, animSpeed, 0 );
	}
	// ENDIF
	if ( padData.debounce[0] & PAD_CROSS )
	{
		animSpeed--;
		actorAnimate ( objViewerActorList[0]->actor, currentAnimation, 1, 0, animSpeed, 0 );
	}
	// ENDIF

	actorUpdateAnimations(objViewerActorList[0]->actor);
	actorSetAnimation(objViewerActorList[0]->actor, objViewerActorList[0]->actor->animation.frame, 1);


	//convert qRot to matrix

// 	if(actor->qRot.x==0 && actor->qRot.y==0 && actor->qRot.z==0 && actor->qRot.w==0)
// 		utilPrintf("ZERO QUAT !\n");

	if(objViewerActorList[0]->actor->psiData.flags & ACTOR_MOTIONBONE)
	{
		objViewerActorList[0]->actor->psiData.object->matrix.t[0] = -objViewerActorList[0]->actor->position.vx;
		objViewerActorList[0]->actor->psiData.object->matrix.t[1] = -objViewerActorList[0]->actor->position.vy;
		objViewerActorList[0]->actor->psiData.object->matrix.t[2] = objViewerActorList[0]->actor->position.vz;
	}
	else
	{
		objViewerActorList[0]->actor->psiData.object->matrix.t[0] += -objViewerActorList[0]->actor->position.vx;
		objViewerActorList[0]->actor->psiData.object->matrix.t[1] += -objViewerActorList[0]->actor->position.vy;
		objViewerActorList[0]->actor->psiData.object->matrix.t[2] += objViewerActorList[0]->actor->position.vz;
	}

	PSIrootScale = &objViewerActorList[0]->actor->psiData.object->scale;
	
	psiCalcWorldMatrix(objViewerActorList[0]->actor->psiData.object);

	psiDrawSegments(&objViewerActorList[0]->actor->psiData);


//	actorDraw ( objViewerActorList[0]->actor );


	PrintTextOverlays();
}

void RunObjectViewer ( void )
{
	switch ( viewMode )
	{
		case OBJECT_BANK_SELECT:
				RunBankSelect();				
			break;
		// ENDCASE
		case OBJECT_BANK_VIEW:
				RunBankView();
			break;
		// ENDCASE
	}
	// ENDSWITCH
}
