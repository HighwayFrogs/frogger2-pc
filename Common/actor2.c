/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: actor.c
	Programmer	: Matthew Cloy
	Date		: 11/11/98

----------------------------------------------------------------------------------------------- */

//#define ALLOW_FADE

#ifndef PSX_VERSION
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#endif

#include "types.h"

#include <isltex.h>
#include "islpsi.h"
#include <islmem.h>
#include <islutil.h>

#include "define.h"
#include "general.h"
#include "actor2.h"
#include "frontend.h"
#include "maths.h"
#include "frogger.h"
#include "frogmove.h"
#include "game.h"
#include "sprite.h"
#include "crc32.h"
#include "Main.h"
#include "cam.h"
#include "platform.h"
#include "tongue.h"

#ifdef PSX_VERSION
#include "psiactor.h"
#include "bff_load.h"	//had to include this, cos ASSERT in global.h needs it (!)
#include "timer.h"
#include "bbpsiani.h"
#include "temp_psx.h"
#else
#include <actor.h>
#include <mdx.h>
#include <pcaudio.h>
#include <pcsprite.h>
#endif


#ifdef PSX_VERSION
void bb_psiSetRotateKeyFrames(PSIOBJECT *world, ULONG frame);
void bb_psiSetScaleKeyFrames(PSIOBJECT *world, ULONG frame);
void bb_psiSetMoveKeyFrames(PSIOBJECT *world, ULONG frame);
#endif

#define MAX_UNIQUE_ACTORS	50


long globalFadeVal;
int actorsCount = 0;

fixed ACTOR_DRAWDISTANCEINNER = 12288000;
fixed ACTOR_DRAWDISTANCEOUTER = 16384000;


int objectMatrix = 0;

ACTOR2 *actList = NULL;				// entire actor list
ACTOR2 *globalLevelActor = NULL;	// ptr to actor representing level


int uniqueActorCRC[MAX_UNIQUE_ACTORS];
int numUniqueActors = 0;


#ifdef PSX_VERSION
//void LSCAPE_DrawSortedPrimitives(int);
void DrawSortedPrimitivesFaded(int);
void customDrawPrimitives2(int, long*);
int actorCount=0;
ACTOR2 *backDropObject=NULL;

//PSIMODEL* pPSI;*/


/*void drawSpriteActor(ACTOR *actor)
{
	POLY_FT4	*ft4;
	VERT		point[4];
	int			x,y,z, loop, depth;
	TextureType	*tex;
	TMD_P_GT4I	*tmdGT4;
	short		*verts;


	tmdGT4 = (TMD_P_GT4I*)(actor->psiData.primitiveList);

	x = -actor->position.vx;
	y = -actor->position.vy;
	z = actor->position.vz;

	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);

	BEGINPRIM(ft4, POLY_FT4);
	setPolyFT4(ft4);

	verts = actor->psiData.object->meshdata->vertop;
	for(loop=0; loop<4; loop++)
	{
		point[loop].vx = x+verts[loop*4+0];
		point[loop].vy = y+verts[loop*4+1];
		point[loop].vz = z+verts[loop*4+2];
	}

	gte_ldv0(point+3);
	gte_rtps();
	gte_stsxy((long *)&ft4->x3);

	gte_ldv3(point,point+1,point+2);
	gte_rtpt();

	setRGB0(ft4, 128,128,128);
	setUV4(ft4, tmdGT4->tu0,tmdGT4->tv0, tmdGT4->tu1,tmdGT4->tv1, tmdGT4->tu2,tmdGT4->tv2, tmdGT4->tu3,tmdGT4->tv3);
	ft4->tpage =tmdGT4->tpage;
	ft4->clut = tmdGT4->clut;

	gte_stsxy3((long *)&ft4->x0,(long *)&ft4->x1,(long *)&ft4->x2);
	gte_stsz(&depth);

	ENDPRIM(ft4, 0, POLY_FT4);
}*/

#endif


//void Orientate(ACTOR *act, SVECTOR *at/*, FVECTOR *fd, SVECTOR *mfd, FVECTOR *up*/)
//void Orientate(ACTOR2 *me, FVECTOR *fd, FVECTOR *mfd, FVECTOR *up)
//void Orientate(SVECTOR* dest, FVECTOR *fd, FVECTOR *mfd, FVECTOR *up)
// void Orientate(IQUATERNION* dest, FVECTOR *fd, FVECTOR *mfd, FVECTOR *up)
// {
// 	FVECTOR dirn;
// 	IQUATERNION rotn,q;
// 	fixed dp,m;
// 
// 	CalculateQuatForPlane2( 0, dest, up);
// 	RotateVectorByQuaternionFF(&dirn, mfd, dest);
// 	dp = DotProductFF( fd, &dirn );
// 	CrossProductFFF((FVECTOR *)&rotn, &dirn, fd);
// 	if(dp > -4055)
// 	{
// 		m = MagnitudeF((FVECTOR *)&rotn);
// 		if(m > 1)
// 		{
// 			ScaleVectorFF((FVECTOR *)&rotn, FDiv(4096,m));
// 
// 			rotn.x = -rotn.x;
// 			rotn.y = -rotn.y;
// 			rotn.w = arccos(dp);
// 			rotn.w -= 2048;
// 
// 			fixedGetQuaternionFromRotation( &q, &rotn );
// 			fixedQuaternionMultiply( dest, &q, dest );
// 		}
// 		else
// 		{
// 			vertQ.w = 2048;
// 			fixedGetQuaternionFromRotation(&q,&vertQ);
// 			fixedQuaternionMultiply(&dest,&dest,&q);
// 		}
// 	}
// 	else
// 	{
// 		vertQ.w = 0;
// 		fixedGetQuaternionFromRotation(&q,&vertQ);
// 		fixedQuaternionMultiply(dest,dest,&q);
// 	}
// }


#ifdef PSX_VERSION

//psx head matrix stuff
//for tongue drawing
//assumes cur==frog[0]
void GetHeadMatrix(ACTOR2 *cur)
{
	if(pHeadMatrix)
	if(cur->actor)
	if(cur->actor->psiData.object)
	{
//		char objName[32];
		PSIOBJECT *obj;
		extern MATRIX headMatrix, *pHeadMatrix;

		//all objects absolute matrices
		gte_SetRotMatrix(&GsWSMATRIX);
		gte_SetTransMatrix(&GsWSMATRIX);

		//bb test - update source position (?)
		cur->actor->psiData.object->matrix.t[0] = -cur->actor->position.vx;
		cur->actor->psiData.object->matrix.t[1] = -cur->actor->position.vy;
		cur->actor->psiData.object->matrix.t[2] =  cur->actor->position.vz;
		PSIrootScale = &cur->actor->psiData.object->scale;

		psiCalcLocalMatrix(cur->actor->psiData.object);

		//find the head
//		sprintf(objName, headNames[player[0].character]);
//		if( (obj = psiObjectScan(cur->actor->psiData.object, objName)) )
		if((obj = psiObjectScan(cur->actor->psiData.object, headNames[player[0].character])))
		{
			//copy head matrix
			int i,j;
			for(i=0; i<3; i++)
				for(j=0; j<3; j++)
					headMatrix.m[i][j] = obj->matrixscale.m[i][j];

			for(i=0; i<3; i++)
				headMatrix.t[i] = obj->matrixscale.t[i];
		}
		else
		{
			//zero head matrix,
			//this should never hapen in the end.
			int i,j;
			for(i=0; i<3; i++)
				for(j=0; j<3; j++)
					headMatrix.m[i][j] = GsIDMATRIX.m[i][j];

			for(i=0; i<3; i++)
				headMatrix.t[i] = GsIDMATRIX.t[i];

			utilPrintf("\n\nERROR GETTING FROG HEAD MATRIX : %s\n\n",
						frog[0]->actor->psiData.object->meshdata->name);
		}

		//restore after local calculation disruption
		//after we've got the frog head absolute matrix
		actorSetAnimation(cur->actor, cur->actor->animation.frame, 1);
		QuatToPSXMatrix(&cur->actor->qRot, &cur->actor->psiData.object->matrix);

		cur->actor->psiData.object->matrix.t[0] = -cur->actor->position.vx;
		cur->actor->psiData.object->matrix.t[1] = -cur->actor->position.vy;
		cur->actor->psiData.object->matrix.t[2] =  cur->actor->position.vz;
	}
}


int psiActorCount, fmaActorCount;
void DrawActorList ( void )
{
 	ACTOR2 *cur;
	char *compare;
	int p, oF;

 	objectMatrix = 0;

	// Separate backdrop printing hack - BUT ITS BUGGERED!!!
	if( backDropObject && gameState.mode != LEVELCOMPLETE_MODE )
	{
		FMA_MESH_HEADER **mesh;

		cur->distanceFromFrog = 0;
		SetVectorSF( &backDropObject->actor->position, &currCamSource );

		FmaActor2ClipCheck(backDropObject);

		fmaActorCount++;
		//globalClut = backDropObject->actor->clutOverride;
		actorShiftDepth = 4095;

		mesh = ADD2POINTER(backDropObject->bffActor, sizeof(FMA_WORLD));
		PrintStaticBackdrop( *mesh );
	}
	
 	cur = actList;
 	while(cur)
 	{
		cur->distanceFromFrog = DistanceBetweenPointsSS ( &cur->actor->position, &frog[0]->actor->position );
		oF = fog.max;

		//ok to draw?
		if(cur->draw)
		{
			if( cur->flags & ACTOR_DRAW_ALWAYS )
				fog.max = 32000;

			//is this a psi?
			if(cur->actor->psiData.object)
			{
				char* oldStackPointer;
				
				//screen clip
				TIMER_START1(TIMER_PSICLIP);
				PsiActor2ClipCheck(cur);
				TIMER_STOP_ADD1(TIMER_PSICLIP);

				//on screen?
				if(!cur->clipped)
				{
					psiActorCount++;
					globalClut = cur->actor->clutOverride;
					actorShiftDepth = cur->depthShift;

					if( cur==frog[0] && (tongue[0].flags & TONGUE_BEINGUSED) )
					{
						TIMER_START1(TIMER_HEADCHK);
						GetHeadMatrix(cur);
						TIMER_STOP_ADD1(TIMER_HEADCHK);
					}

					if(cur->actor->psiData.flags & PSIACTOR_TRANSPARENT)
					{
						psiRegisterDrawFunction(DrawSortedPrimitivesFaded);
						globalFadeVal = 50;
					}
					else
					{
						//psiRegisterDrawFunction(LSCAPE_DrawSortedPrimitives);
						globalFadeVal = 255;
					}

					/*TIMER_START1(TIMER_PSIBACK);
					if( (compare = strstr(cur->actor->psiData.modelName,"BACK")) )
					{
						TIMER_STOP_ADD1(TIMER_PSIBACK);
						SetVectorSF(&cur->actor->position, &currCamSource);
					}
					else
					{
						TIMER_STOP_ADD1(TIMER_PSIBACK);
					}*/

					//N.B custom draw functions for dynamic and pre-sorted
					//are defined at the start
					TIMER_START1(TIMER_PSIDRAW);
	 				actorDraw ( cur->actor );
					TIMER_STOP_ADD1(TIMER_PSIDRAW);

					actorCount++;
		
				}
			}
			else if(cur->bffActor)
			{
				TIMER_START1(TIMER_FMACLIP);
				FmaActor2ClipCheck(cur);
				TIMER_STOP_ADD1(TIMER_FMACLIP);

				if(!cur->clipped)
				{
					int i;
					char* oldStackPointer;
					FMA_MESH_HEADER **mesh;

					fmaActorCount++;
					globalClut = cur->actor->clutOverride;
					actorShiftDepth = cur->depthShift;
				
					//TIMER_START1(TIMER_SPHERE);
					//if( (compare = strstr(cur->actor->psiData.modelName, "SPHERE")) )
					{
			 			//TIMER_STOP_ADD1(TIMER_SPHERE);
						//cur = cur->next;
						//continue;
					}
					//TIMER_STOP_ADD1(TIMER_SPHERE);

					mesh = ADD2POINTER(cur->bffActor, sizeof(FMA_WORLD));
					
					TIMER_START1(TIMER_FMADRAW);
					DrawScenicObj ( *mesh, cur->flags );
					TIMER_STOP_ADD1(TIMER_FMADRAW);
				}
			}
		}

		fog.max = oF;

		cur = cur->next;
 	}

	globalActors [ 0 ].done = 0;

// 	//print actor count
// 	#ifdef COUNT_ACTORS
// 	{
// 		char actorCountText[64];
// 		sprintf(actorCountText, "%d Actors", actorCount);
// 		fontPrint(fontSmall, -240,80, actorCountText, 200,128,128);
// 	}
// 	#endif
}


#endif

void FreeActorList()
{
	ACTOR2 *next,*cur;

	//utilPrintf("Freeing linked list : ACTOR2\n");
	cur = actList;
	while (cur)
	{
		next = cur->next;

// 		if((cur->actor->objectController) && (cur->actor->objectController->object))
// 		{
// //		 	FreeObjectSprites(cur->actor->objectController->object);
// 
// 			// NEW
// 			if(cur->actor->objectController->drawList)
// 			{
// 				FREE((UBYTE *)cur->actor->objectController->vtx[0]);
// 				FREE((UBYTE *)cur->actor->objectController->drawList);
// 			}
// 
// 			// NEW
// //			RemoveUniqueObject(cur->actor->objectController->object);
// 			FREE((UBYTE *)cur->actor->objectController);
// 
// 			if(cur->actor->objectController)
// 				FREE((UBYTE *)cur->actor->objectController);
// 		}

// 		if(cur->actor->LODObjectController)
// 			FREE((UBYTE *)cur->actor->LODObjectController);
// 
// 		if(cur->actor->matrix)
// 			FREE((UBYTE *)cur->actor->matrix);
 
// 		if(cur->actor->animation)
// 			FREE((UBYTE *)cur->actor->animation);
 
//  		if(cur->actor->shadow)
//  			FREE((UBYTE *)cur->actor->shadow);
 
// 		FREE((UBYTE*)cur->actor);
//		FREE((UBYTE*)cur);
		actor2Free(cur);
		
		cur = next;
	}
	
	actList = NULL;

// 	FreeActor2(backGnd);		
// 	backGnd = NULL;

// 	FreeActor2(hat[0]);		
// 	hat[0] = NULL;
}

#ifdef PC_VERSION
#define MDX_ADDACTORFLAGS(a, f)		{ ((MDX_ACTOR *)((a)->actor->actualActor))->flags |= (f); }
#define MDX_ADDOBJECTFLAGS(a, f)	{ ((MDX_ACTOR *)((a)->actor->actualActor))->objectController->object->flags |= (f); }
#else
#define MDX_ADDACTORFLAGS(actor2, flags)
#define MDX_ADDOBJECTFLAGS(actor2, flags)
#endif

ACTOR2 *CreateAndAddActor(char *name, short cx, short cy, short cz, int initFlags)
{
#ifndef PSX_VERSION
	MDX_ACTOR *tAct;
#endif
	ACTOR2* newItem;
	int i;
	char newname[32];

	newItem	= (ACTOR2 *)MALLOC0(sizeof(ACTOR2));

	// NOTE TO WHOEVER ELSE EDITS THIS:
	// We have to copy the name or the original string gets fucked over.
	// SO DON'T TAKE THIS OUT AGAIN!
	strcpy (newname,name);

	newItem->flags = 0;

#ifdef PSX_VERSION
	{
		PSIMODEL* pPSI;
		char *c = (char*)strstr(newname, ".obe");
		char *compare;

		if (c)
		{
			strcpy(c, ".psi");
		}

		// Hack so that we can have reactives triggered off null objects
		if( strstr(newname,"NULL_") || strstr(newname,"null_"))
			sprintf(newname,"NOTHING.PSI");

		utilUpperStr ( newname );
		newItem->bffActor = ( void * ) BFF_FindObject ( BFF_FMA_WORLD_ID, utilStr2CRC ( newname ) );

		if ( newItem->bffActor )
		{
			//SetUpWaterMesh ( newItem->bffActor );
			newItem->flags |= ACTOR_NOANIMATION;
			newItem->actor	= (ACTOR *)MALLOC0(sizeof(ACTOR));

			//bb store name of fma actor in empty psiData struct.
			newItem->actor->psiData.modelName = MALLOC0(24);
			sprintf(newItem->actor->psiData.modelName, "(FMA)");
			strcat(newItem->actor->psiData.modelName, newname);
		}
		// ENDIF

		if ( !newItem->bffActor )
		{
			if ( !(pPSI = psiLoad(newname)) )
			{
				utilPrintf ( "Could not Create Psi Actor %s\n", newname );
				//return NULL;
			}
			else
			{
				sprintf ( pPSI->name, "%s", newname );
				actorsCount++;

	 			newItem->actor = actorCreate(pPSI, 1, 1);
				if ( (strstr(newname,"GHOST")) || (strstr(newname,"GHOSTIE")) || (strstr(newname,"LIGHTNG")) )
				{
					newItem->actor->psiData.flags |= PSIACTOR_TRANSPARENT;
					newItem->actor->psiData.flags |= ACTOR_DYNAMICSORT;
				}
			}
		}

		// null object - do not display
		if( strstr(newname,"TRANSOBJ") || strstr(newname,"NOTHING") || strstr(newname,"NULL_") ||
			strstr(newname,"transobj") || strstr(newname,"nothing") || strstr(newname,"null_") )
			newItem->draw = 0;
		else
			newItem->draw = 1;
	}
#elif PC_VERSION

	newItem->actor = (ACTOR*)MALLOC0(sizeof(ACTOR));

	for (i=0; newname[i]!='.'; i++);
	newname[i+1] = 'o';
	newname[i+2] = 'b';
	newname[i+3] = 'e';

	// Hack so that we can have reactives triggered off null objects
	if( !(strnicmp(newname,"NULL_",5)) )
		sprintf(newname,"nothing.obe");

	if (!(newItem->actor->actualActor = (void *)CreateActor(newname,INIT_ANIMATION)))
	{
		FREE(newItem->actor); FREE(newItem);
		return NULL;
	}

	tAct = (MDX_ACTOR *)newItem->actor->actualActor;

	if((tAct->objectController) && (tAct->objectController->object))
	{
		AddObjectsSpritesToSpriteList(tAct->objectController->object,0);

		// Cue sounds from animations
		FindSfxMapping( tAct->objectController->objectID, newItem->actor );
	}

	// null object - do not display
	if( !(strnicmp(newname,"TRANSOBJ",8)) || !(strnicmp(newname,"NOTHING",7)) || !(strnicmp(newname,"NULL_",5)) )
		newItem->draw = 0;
	else
		newItem->draw = 1;
#endif

	gstrlwr(newname);

	if (newname[0] == 'a' && newname[1] == 'd')
	{
		MDX_ADDACTORFLAGS(newItem, ACTOR_ALWAYSDRAW);
		newItem->flags |= ACTOR_DRAW_ALWAYS;

		if (newname[2]=='a')
		{
			newItem->flags |= ACTOR_ADDITIVE;
			MDX_ADDOBJECTFLAGS(newItem, OBJECT_FLAGS_ADDITIVE);
		}
	}
	else
		newItem->flags |= ACTOR_DRAW_CULLED;

	// Yes, seriously, this is the check that has been done forever and breaks if you change it
	if (newname[2] == '_' && newname[3]=='g')
	{
		MDX_ADDACTORFLAGS(newItem, ACTOR_SLIDY);
		MDX_ADDOBJECTFLAGS(newItem, OBJECT_FLAGS_MODGE);
		newItem->flags |= ACTOR_MODGETEX;
		if (newname[4]=='_')
			newItem->flags |= ACTOR_SLIDYTEX;
	}

	if (newname[0] == 's' && newname[1] == 'p')
	{
		switch (newname[2])
		{
			case 'l':
				newItem->flags |= ACTOR_SLIDYTEX;
				MDX_ADDACTORFLAGS(newItem, ACTOR_WRAPTC);
				MDX_ADDACTORFLAGS(newItem, ACTOR_SLIDY);
				break;

			case 'm':
				newItem->flags |= ACTOR_SLIDYTEX2;
				MDX_ADDACTORFLAGS(newItem, ACTOR_WRAPTC);
				MDX_ADDACTORFLAGS(newItem, ACTOR_SLIDY);
				break;

			case 'f':
				newItem->flags |= (ACTOR_SLIDYTEX | ACTOR_SLIDYTEX2);
				MDX_ADDACTORFLAGS(newItem, ACTOR_WRAPTC);
				MDX_ADDACTORFLAGS(newItem, ACTOR_SLIDY);
				break;
		}
		/*
		switch (newname[2])
		{
			case 'l':
				newItem->flags |= ACTOR_SLIDYTEX;
				break;
			case 'm':
				newItem->flags |= ACTOR_SLIDYTEX2;
				break;
			case 'f':
				newItem->flags |= (ACTOR_SLIDYTEX | ACTOR_SLIDYTEX2);
				break;
		}
		*/
	}

#ifdef PC_VERSION

	if( (initFlags & INIT_SHADOW) || (newItem->flags & ACTOR_ADDITIVE) || !strnicmp(newname,"shadow",6))
		AddActorToList(tAct,1);
	else
		AddActorToList(tAct,0);

#elif PSX_VERSION
	gstrupr(newname);

	/*if ((strstr(newname,"CARA"))||(strstr(newname,"CARB"))||(strstr(newname,"CARC"))||(strstr(newname,"CARD"))||(strstr(newname,"CARE"))||
		(strstr(newname,"LOGAA"))||(strstr(newname,"LOGBB"))||(strstr(newname,"LOGCC"))||(strstr(newname,"LORRYA"))||(strstr(newname,"LORRYB"))||
		(strstr(newname,"TURT"))||(strstr(newname,"LOGBB"))||(strstr(newname,"LOGCC"))||(strstr(newname,"LORRYA"))||(strstr(newname,"LORRYB"))||
		(strstr(newname,"SNAKEHED"))||(strstr(newname,"SNAKETAL"))||(strstr(newname,"LOGD"))||(strstr(newname,"LOGE"))||(strstr(newname,"LOGF")))
	{
		newItem->flags |= 1<<15;	// This flag should be allocated properly in common code
		utilPrintf("*** FOUND SPRITE ACTOR'%s'\n", newname);
	}*/
	// HACK: Should have ad_ so it works proper.
	/*else */if( strstr(newname,"BACKDROP") || strstr(newname,"BOULDER") || strstr(newname, "RISER") )
	{
		newItem->flags |= ACTOR_DRAW_ALWAYS;
		newItem->depthShift = 0;
	}
#endif

	// Setup a shadow
 	if(initFlags & INIT_SHADOW)
 	{
 		newItem->actor->shadow = (ACTOR_SHADOW *)MALLOC0(sizeof(ACTOR_SHADOW));
 		newItem->actor->shadow->alpha/* = tempActor->shadow->alphaAim */= 255;
 		newItem->actor->shadow->draw = 1;
 	}

 	newItem->actor->oldPosition.vx	= cx;
 	newItem->actor->oldPosition.vy	= cy;
 	newItem->actor->oldPosition.vz	= cz;

	newItem->actor->size.vx = 4096;
	newItem->actor->size.vy = 4096;
	newItem->actor->size.vz = 4096;

//	newItem->draw	= 1;
	newItem->flags	|= ACTOR_DRAW_CULLED;
	newItem->radius	= 0;
	newItem->animSpeed = 4096;
	newItem->value1 = 0;
	newItem->effects = 0;

	newItem->distanceFromFrog	= 0;

	newItem->actor->position.vx = cx;				
	newItem->actor->position.vy = cy;
	newItem->actor->position.vz = cz;
#ifdef PSX_VERSION
	actorSetAnimation(newItem->actor, 0, 1);	
#endif

#ifdef PSX_VERSION
	if( strstr(newname,"BACKDROP") )
	{
		backDropObject = newItem;
		return NULL;
	}
#endif
	
	newItem->clipped = 0;
	newItem->milesAway = 0;

	newItem->next = actList;
	newItem->prev = NULL;

	if (actList) actList->prev = newItem;
	actList = newItem;

	return newItem;
}


/*	--------------------------------------------------------------------------------
	Function		: AddObjectsSpritesToSpriteList
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
// void AddObjectsSpritesToSpriteList(OBJECT *obj,short flags)
// {
// 	SPRITE *sprite;
// 	int i;
// 
// 	if(obj->numSprites > 0)
// 	{
// 		for(i=0; i<obj->numSprites; i++)
// 		{
// 			sprite = (SPRITE *)MALLOC0(sizeof(SPRITE));
// 
// 			sprite->texture = obj->sprites[i].textureID;
// 			SetVectorI(&sprite->pos,&zero);
// 
// 			if((obj->sprites[i].flags & SPRITE_DONE) == 0)
// 			{
// 				obj->sprites[i].flags |= SPRITE_DONE;
// 				switch(sprite->texture->sx)
// 				{
// 					case 8:
// 						obj->sprites[i].sx /= 2;
// 						break;
// 					case 16:
// 						obj->sprites[i].sx *= 2;
// 						break;
// 					case 32:
// 						break;
// 				}
// 				switch(sprite->texture->sy)
// 				{
// 					case 8:
// 						break;
// 					case 16:
// 						obj->sprites[i].sy *= 2;
// 						break;
// 					case 32:
// 						break;
// 				}
// 			}
// 
// 			sprite->r = sprite->g = sprite->b = 255;
// 			sprite->a = 128;
// 			if(obj->xlu < 255)
// 				sprite->flags = SPRITE_TRANSLUCENT;
// 			else
// 				sprite->flags = 0;
// 
// 			sprite->flags |= flags;
// 
// 			sprite->offsetX = -sprite->texture->sx / 2;
// 			sprite->offsetY = -sprite->texture->sy / 2;
// 
// 			AddSprite(sprite,NULL);
// 			obj->sprites[i].sprite = sprite;
// 		}
// 	}
// 
// 	if(obj->children)
// 		AddObjectsSpritesToSpriteList(obj->children,flags);
// 
// 	if(obj->next)
// 		AddObjectsSpritesToSpriteList(obj->next,flags);
// 
// }



/*	--------------------------------------------------------------------------------
	Function		: FreeObjectSprites
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/


/* --------------------------------------------------------------------------------
	Function	: ActorsHaveCollided
	Purpose		: determines is two actors have collided
	Parameters	: ACTOR2 *,ACTOR2 *
	Returns		: BOOL 
	Info		:
*/




/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/

// void MakeUniqueVtx(OBJECT_CONTROLLER *objC)
// {
// 
// 	Vtx *vtxa;
// 	Vtx *vtxb;
// 	Vtx *oldVtxa, *oldVtxb;
// 
// 
// 	oldVtxa = objC->vtx[0];
// 	oldVtxb = objC->vtx[1];
// 
// 	vtxa = (Vtx *)MALLOC0(sizeof(Vtx) * objC->numVtx * 2);
// 	vtxb = vtxa + objC->numVtx;
// 
// 	memcpy(vtxa, oldVtxa, sizeof(Vtx) * objC->numVtx);
// 	memcpy(vtxb, oldVtxb, sizeof(Vtx) * objC->numVtx);
// 
// 	//controller now references new vtx's, must also make sure that drawlist is updated
// 	objC->vtx[0] = vtxa;
// 	objC->vtx[1] = vtxb;
// 
// }

/*	--------------------------------------------------------------------------------
	Function		: MakeUniqueObject
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

// OBJECT *MakeUniqueObject(OBJECT *object)
// {
// 
// 	OBJECT	*obj;	
// 	OBJECTSPRITE **spr, *tempSpr;
// 		
// 	obj = object;
// 	object = (OBJECT *)MALLOC0(sizeof(OBJECT));
// 	memcpy(object, obj, sizeof(OBJECT));
// 
// 	if(obj->numSprites)
// 	{
// 		spr = &object->sprites;
// 		tempSpr = (OBJECTSPRITE *)MALLOC0(sizeof(OBJECTSPRITE) * obj->numSprites);
// 		memcpy(tempSpr, *spr, sizeof(OBJECTSPRITE) * obj->numSprites);
// 		*spr = tempSpr;
// 
// 	}
// 
// 	if(object->children)
// 		object->children = MakeUniqueObject(object->children);
// 
// 	if(object->next)
// 		object->next = MakeUniqueObject(object->next);
// 		
// 	return object;
// }


/*	--------------------------------------------------------------------------------
	Function 	: MakeUniqueActor
	Purpose 	: Makes an actor unique by giving it its own object controller, and calls
        		  make unique object, to sort out the object (including children)
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
// void MakeUniqueActor(ACTOR *actor,int type)
// {
// 
// 	OBJECT_CONTROLLER	*objCont;
// 	short	unique = TRUE;
// 	short	i;
// 	int		CRC;
// 	
// 	if (!actor->objectController)
// 	{
// 		return;
// 	}
// 		
// 	CRC = utilStr2CRC((char*)actor->objectController->object->name);
// 
// 	//check all crc's to see if actor is among them
// 	for(i = 0; i < numUniqueActors; i++)
// 	{
// 		if(uniqueActorCRC[i] == CRC)
// 		{
// 			//if it is, actor is not unique and must have seperate stuff
// 			dprintf"found duplicate actor %s\n", actor->objectController->object->name));
// 			unique = FALSE;
// 			break;
// 		}
// 	}
// 	//if actor is not in list
// 	if(unique == TRUE)
// 	{
// 		uniqueActorCRC[numUniqueActors++] = CRC;
// 	}	
// 
// 	objCont = actor->objectController;
// 	actor->objectController = (OBJECT_CONTROLLER *)MALLOC0(sizeof(OBJECT_CONTROLLER));
// 	memcpy(actor->objectController, objCont, sizeof(OBJECT_CONTROLLER));
// 	actor->objectController->object = MakeUniqueObject(actor->objectController->object);
// 
// 	if(unique == FALSE)
// 	{
// 		//if actor is skinned, duplicate Vtx's
// 		if(actor->objectController->drawList)
// 		{
// 			MakeUniqueVtx(actor->objectController);
// 			XformActor(actor);
// 		}
// 	}
// 
// 	objCont = actor->LODObjectController;
// 	if(objCont)
// 	{
// 		actor->LODObjectController = (OBJECT_CONTROLLER *)MALLOC0(sizeof(OBJECT_CONTROLLER));
// 		memcpy(actor->LODObjectController, objCont, sizeof(OBJECT_CONTROLLER));
// 		actor->LODObjectController->object = MakeUniqueObject(actor->LODObjectController->object);
// 	}
// 
// }




/*	--------------------------------------------------------------------------------
	Function 	: RemoveUniqueObject
	Purpose 	: Frees all the unique stuff from an object
	Parameters 	: OBJECT *object
	Returns 	: OBJECT *
	Info 		:
*/



// void XformActorList()
// {
// 	ACTOR2 *cur;
// 		
// 	cur = actList;
// 	while(cur)
// 	{
// 		if (frontEndState.mode != OBJVIEW_MODE)
// 		{
// 			// calculate the distance between the camera and this actor
// 			cur->distanceFromFrog = DistanceBetweenPointsSS(&cur->actor->position,&frog[0]->actor->position);
// 
// 			// transform actor
// 			if(cur->distanceFromFrog < ToFixed(bbsqrt_ACTOR_DRAWDISTANCEINNER) || cur->flags & ACTOR_DRAW_ALWAYS)
// 				XformActor(cur->actor);
// 
// 
// 			// determine this actor's visibility
// 			cur->draw = 1;
// 
// 			if(cur->flags & ACTOR_DRAW_ALWAYS)
// 			{
// 				// always draw this actor
// 				cur->draw = 1;
// 			}
// 			else if(cur->flags & ACTOR_DRAW_CULLED)
// 			{
// 				// determine if actor is visible
// 				if(cur->distanceFromFrog < ToFixed(bbsqrt_ACTOR_DRAWDISTANCEINNER))
// 					cur->draw = 1;
// 			}
// 		}
// 		//bb frontend object view mode(?)
// 		else
// 		{
// 			// transform actor
// 			XformActor(cur->actor);
// 			cur->draw = 0;
// 			if(cur->flags & ACTOR_DRAW_ALWAYS)
// 			{
// 				// always draw this actor
// 				cur->draw = 1;
// 			}
// 		}
// 
// 
// 		cur = cur->next;
// 	}
// }

void OrientateSS(IQUATERNION *me, SVECTOR *fd, SVECTOR *up)
{
	FVECTOR f, g;
	SetVectorFF(&f, fd);
	SetVectorFF(&g, up);
	Orientate(me, &f, &g);
}

void OrientateFS(IQUATERNION *me, FVECTOR *fd, SVECTOR *up)
{
	FVECTOR f;
	SetVectorFF(&f, up);
	Orientate(me, fd, &f);
}

void OrientateSF(IQUATERNION *me, SVECTOR *fd, FVECTOR *up)
{
	FVECTOR f;
	SetVectorFF(&f, fd);
	Orientate(me, &f, up);
}

void Orientate(IQUATERNION *me, FVECTOR *fd, FVECTOR *up)
{
	IQUATERNION r1,q1,q2;
	FVECTOR v1;//,v2,v3,v4;
	fixed dp,dp2,scl;
	fixed thetaOver2;
	fixed sinThetaOver2;
	long q1Unimportant = 0,q1Flip = 0;
	
	
	//bbopt - do this outside, only when neccessary
//	MakeUnit(fd);


	dp = up->vy;
	
	if (dp<4055)
	{
		if (dp>-4055)
		{
			r1.x = -up->vz;
			r1.y = 0;
			r1.z = up->vx;
			r1.w = arccos(dp);

			// Normalise it (only need to do 2 coords!)
//			scl = FDiv(4096, FsqrtF(FMul(r1.x,r1.x)+FMul(r1.z,r1.z)) );
//bbopt
			scl = FDiv2(4096, FsqrtF(FMul(r1.x,r1.x)+FMul(r1.z,r1.z)) );
//			scl = FDiv2( 4096, FsqrtF(((r1.x*r1.x)>>12)+((r1.z*r1.z)>>12)) );
//bbopt
 			r1.x = FMul(r1.x, scl);
 			r1.z = FMul(r1.z, scl);
//			r1.x *= scl;
//			r1.z *= scl;

			fixedRotateVectorByRotation(&v1,fd,&r1);
	
			r1.w = -r1.w;
	//		v1.vy = 0;

			GetQuaternionFromXZRotation(&q1,&r1);
		}
		else
		{
			// We're just fliping the X&Y coords!
			v1.vx = fd->vx;
			v1.vy = 0;
			v1.vz = -fd->vz;
			q1.w = q1.y = q1.z = 0;
			q1.x = 4096;

			q1Flip = 1;			// We don't need the qMult later on!
		}
	}
	else
	{
		v1.vx = fd->vx;
		v1.vy = 0;
		v1.vz = fd->vz;
		
		q1.x = q1.y = q1.z = 0;
		q1.w = 4096;

		q1Unimportant = 1;	
	}

	// Again, makeunit from the two used values
//bbopt
	scl = FDiv2(4096, FsqrtF(FMul(v1.vx,v1.vx)+FMul(v1.vz,v1.vz)) );
//	scl = FDiv2(4096, FsqrtF(((v1.vx*v1.vx)>>12)+((v1.vz*v1.vz)>>12)) );

//bbopt
	dp2 = FMul(v1.vz, scl);
//	dp2 = (v1.vz * scl)>>12;
	
//	if (dp2<1-OEPSILON)
	if (dp2<4055)
	{
//		if (dp2>OEPSILON-1)
		if (dp2>-4055)
		{
			if (q1Unimportant)
			{
				if (v1.vx>0)
					thetaOver2 = arccos(dp2)/2;
				else
					thetaOver2 = arccos(dp2)/-2;

				sinThetaOver2 = rsin(thetaOver2);

				me->x = me->z = 0;
				me->w = rcos(thetaOver2);
				me->y = sinThetaOver2;
			}
			else
			{

				SetVectorFF((FVECTOR*)(&r1), up);
							
				if (v1.vx>0)
					r1.w = arccos(dp2);
				else
					r1.w = -arccos(dp2);
	
				fixedGetQuaternionFromRotation(&q2,&r1);			

				if (q1Flip)
				{
					me->w = -q2.x;
					me->x = q2.w;
					me->y = q2.z;
					me->z = -q2.y;
				}
				else				
				{
					fixedQuaternionMultiply(me,&q2,&q1);
				}
			
			}
		}
		else
		{	
			// This could be more optimal!
			q2.w = 0;
			q2.x = -up->vx;
			q2.y = -up->vy;
			q2.z = -up->vz;
			
			if (q1Unimportant)
				SetQuaternion(me,&q2);
			else
			{
				if (q1Flip)
				{
					me->w = -q2.x;
					me->x = q2.w;
					me->y = q2.z;
					me->z = -q2.y;
				}
				else
				{
					// QuatMult of a w = 0 quaternion.
					me->w = -FMul(q2.x,q1.x) - FMul(q2.y,q1.y) - FMul(q2.z,q1.z);
					me->x =  FMul(q2.x,q1.w) + FMul(q2.y,q1.z) - FMul(q2.z,q1.y);
					me->y =  FMul(q2.y,q1.w) + FMul(q2.z,q1.x) - FMul(q2.x,q1.z);
					me->z =  FMul(q2.z,q1.w) + FMul(q2.x,q1.y) - FMul(q2.y,q1.x);
				}
			}
		}
	}
	else // No change, hence no need for qmult.		
		SetQuaternion(me,&q1);			
}


/* --------------------------------------------------------------------------------
	Function	: actor2Free
	Purpose		: Delete an actor2 and its associated actor
	Parameters	: (void)
	Returns		: void 
*/
void actor2Free(ACTOR2 *actor)
{
	if (!actor) return;

	//utilPrintf("actor2free: %08x\n", actor);
	actorFree(actor->actor);
	
	if (actor->next)
		actor->next->prev = actor->prev;

	if (actor->prev)
		actor->prev->next = actor->next;
	else
		actList = actor->next;

#ifdef PC_VERSION
	FREE(actor);
#endif
	actor = NULL;
}


/*	--------------------------------------------------------------------------------
	Function		: TeleportActorToTile
	Purpose			: teleports the actor to the specified tile
	Parameters		: ACTOR2 *,GAMETILE *
	Returns			: void
	Info			: 
*/

void TeleportActorToTile(ACTOR2 *act,GAMETILE *tile,long pl)
{
	// make the teleport 'to' tile the current tile
	currTile[pl] = tile;
	SetVectorSS(&act->actor->position,&tile->centre);
	player[pl].frogState &= ~FROGSTATUS_ISTELEPORTING;
	player[pl].canJump = 1;
	player[pl].frogon = -1;
	player[pl].frogunder = -1;

	if (currPlatform[pl])
	{
		currPlatform[pl]->carrying = 0;
		currPlatform[pl]->flags &= ~PLATFORM_NEW_CARRYINGFROG;
	}

	destTile[pl]		= NULL;
	currPlatform[pl]	= NULL;
	destPlatform[pl]	= NULL;

	player[pl].extendedHopDir = (frogFacing[pl] - camFacing[pl])&3;
	OrientateSS( &act->actor->qRot, &tile->dirVector[frogFacing[pl]], &tile->normal );
}

