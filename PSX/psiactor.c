/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	actor.c			Skinned model control routines

************************************************************************************/

#include <islutil.h>
#include <islmem.h>
//#include "global.h"
#include "actor2.h"
#include "Main.h"
#include "Shell.h"
#include "layout.h"
#include "bbpsiani.h"
#include "frogmove.h"
#include "enemies.h"
#include "babyfrog.h"
#include "audio.h"
#include "game.h"

ACTORLIST	actorList;


#include "timer.h"
#include "temp_psx.h"
#include "frogger.h"

//#define min(a,b) (((a) < (b)) ? (a) : (b))
//#define max(a,b) (((a) > (b)) ? (a) : (b))
//#define Bound(a,x,b) min(b,max(a,x))


int globalCount = 0;
ACTORSETANIM globalActors [ 50 ];

//extern PSIMODEL *psiCheck(char *psiName);

//PSIDATA oldModel;
ACTOR oldActor;

void actorInitialise()
{
	actorList.numEntries = 0;
	actorList.head.next = actorList.head.prev = &actorList.head;
}



/**************************************************************************
	Function 	: actorAdd()
	Purpose 	: adds an actor to the linked list
	Parameters 	: pointer to actor
	Returns 	: none
	Info 		:
**************************************************************************/

void actorAdd(ACTOR *actor)
{
	ACTOR *ptr;

	ptr = actorList.head.next;

	if(actor->next == NULL)
	{
		actorList.numEntries++;
		actor->next = ptr;
		actor->prev = ptr->prev;
		ptr->prev->next = actor;
		ptr->prev = actor;		
	}
}


/**************************************************************************
	Function 	: actorSub()
	Purpose 	: removes an actor from the linked list
	Parameters 	: pointer to actor
	Returns 	: none
	Info 		:
**************************************************************************/

void actorSub(ACTOR *actor)
{
	if(actor->next != NULL)
	{
		actor->prev->next = actor->next;
		actor->next->prev = actor->prev;
		actorList.numEntries--;
		actor->next = NULL;
	}

}


/**************************************************************************
	FUNCTION:	actorFree
	PURPOSE:	free all memory used by actor structure
	PARAMETERS:	ACTOR *
	RETURNS:	
**************************************************************************/

void actorFree(ACTOR *actor)
{
	FREE(actor->psiData.objectTable);
	actor->psiData.objectTable = NULL;

	if (actor->shadow)
		FREE(actor->shadow);//IF SHADOWS ARE ADDED...mm

	actor->shadow = NULL;

	actorSub(actor);
	FREE(actor);

	actor = NULL;
}

void actorFree2(ACTOR *actor)
{
	FREE(actor->psiData.objectTable);
	actor->psiData.objectTable = NULL;

	if (actor->shadow)
		FREE(actor->shadow);//IF SHADOWS ARE ADDED...mm

	actor->shadow = NULL;

	actorSub(actor);
//	FREE(actor);

//	actor = NULL;
}




static ULONG 	*segTable;
static PSIOBJECT *psiobNext;

static void actorSetTree(PSIOBJECT *obj,PSIMESH *mesh)
{
	*segTable = (ULONG)obj;
	segTable++;

	obj->meshdata = mesh;

	if (mesh->child)
	{
		obj->child	  = psiobNext;
		actorSetTree(psiobNext++,mesh->child);
	}
	if (mesh->next)
	{
		obj->next  = psiobNext;
		actorSetTree(psiobNext++,mesh->next);
	}
}


/**************************************************************************
	FUNCTION:	CreateActor
	PURPOSE:	create an actor structure from PSIModel data
	PARAMETERS:	PSIModel*
	RETURNS:	ACTOR*
**************************************************************************/

ACTOR *actorCreate(PSIMODEL *psiModel, int checkForModel, int scaleSkinned )
{

	ULONG len,parts,i;
	ULONG *amem;
	ACTOR *actor;
	PSIOBJECT *psiobRoot;
	PSIMESH *meshRoot;
	char *compare;
	char	actorName[32];

	if (psiModel==NULL)
		return 0;

	parts = psiModel->noofmeshes;			// number of objects in this model

//	utilPrintf("Number of Objects in Model : %d\n",parts);

	len = parts * sizeof(PSIOBJECT);
	len += sizeof (ACTOR);

//	utilPrintf("New actor: %d bytes\n",len);
	
#if CDGOLD==0
	sprintf(actorName,"%s",psiModel->name);
#endif

	actor = (ACTOR *)MALLOC0(len);
	
	// flush memory
	//bbxx don't need this zeroing

// JH: Becuase we use zero the memory when allocated we shouldn't need to do this.
/*	amem = (ULONG *)actor;
	for (i=0; i<(len/4); i++)
		*amem++ = 0;*/

	actor->psiData.flags = 0;

	if (psiModel->flags & 1)
	{
		utilPrintf("Skinned Model..............");
		actor->psiData.flags = ACTOR_BONED;
		actor->psiData.flags |= ACTOR_DYNAMICSORT;
	}

	//utilPrintf("%s : %s\n", actor->psiData.modelName, actorName);

	actor->psiData.modelName = psiModel->name;

	//utilPrintf("%s : %s\n", actor->psiData.modelName, actorName);

	actor->psiData.numObjects = parts;

	// list of pointers to model objects/meshes
	actor->psiData.objectTable = (ULONG *)MALLOC0(parts * sizeof(int));

	// set primitive pointer
	actor->psiData.primitiveList = psiModel->primOffset;

	// set prim counter
	actor->psiData.noofPrims = psiModel->noofPrims;

//	utilPrintf ( "Name : %s, Number of polys: %d", actor->psiData.modelName, actor->psiData.noofPrims );

	actor->psiData.noofVerts = psiModel->noofVerts;


	meshRoot = (PSIMESH *)psiModel->firstMesh;

	//	set anim frame pointers
	actor->animation.numAnimations = psiModel->animSegments;
	actor->animSegments = (USHORT*)psiModel->animSegmentList;	// offest to frame list

	// set up actor structure

	psiobRoot = (PSIOBJECT *)((char*)actor + sizeof(ACTOR));		// object structs follow actor struct
	psiobNext = psiobRoot + 1;

	actor->psiData.object = psiobRoot;									// pointer to 1st object

	// fill in next & child pointers
	segTable = actor->psiData.objectTable;
	actorSetTree(psiobRoot,meshRoot);

	actorAdd(actor);											// add actor to actorList

	actorInitAnim(actor);										// initialise animation structure

	actor->animFrames = psiModel->animEnd;						// total noof frames of animation

	//utilPrintf("actor->animFrames, %d\n", actor->animFrames);

	actor->radius = psiModel->radius;
	
	if ( actor->psiData.flags & ACTOR_DYNAMICSORT )
		psiInitSortList( (actor->radius * 2) + 8 );	 					// make enough room to z sort polys

	actor->size.vx = actor->size.vy = actor->size.vz = 4096;

	//if (psiObjectScan(actor->psiData.object,"MOTION"))
	//{
	//	utilPrintf("motion bone found\n");
	//	actor->psiData.flags |= ACTOR_MOTIONBONE;
	//}
//	else
//		utilPrintf("motion bone NOT found\n");

//	actor->psiData.flags |= ACTOR_DYNAMICSORT;							// by default

	//actorSetBoundingRotated(actor,0,0,0,0);						// fill in bounding box info

	actor->clutOverride = NULL;

	//scale skinned objects (if not already scaled)
	if(psiModel->flags & 1)
	{
		if( !(actor->psiData.flags&PSIACTOR_SCALED) && ( scaleSkinned ) )
		{
			ScalePsi(actor->psiData.object->meshdata);
			actor->radius *= 10;
			psiInitSortList((actor->radius*2)+8);
			actor->psiData.flags |= PSIACTOR_SCALED;
		}
	}

	return actor;
}

/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void actorInitAnim(ACTOR *actor)
{
	actor->animation.currentAnimation = -1;

	actor->animation.queueAnimation[0] = -1;
	actor->animation.queueAnimation[1] = -1;
	actor->animation.queueAnimation[2] = -1;
	actor->animation.queueAnimation[3] = -1;
	actor->animation.queueAnimation[4] = -1;
	actor->animation.loopAnimation = 1;
	actor->animation.numberQueued = 0;
	actor->animation.animTime = 0;
	actor->animation.reachedEndOfAnimation = 0;
}


/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void actorFlushQueue(ACTOR *actor)
{
	int i;

	actor->animation.numberQueued = 0;

	for(i = 0;i < ANIM_QUEUE_LENGTH;i++)
	{
		actor->animation.queueAnimation[i] = -1;
		actor->animation.queueAnimationSpeed[i] = 1;
	}
}




/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void actorAdjustPosition(ACTOR *actor)
{

	actor->oldPosition.vx = 0;//actor->position.vx;// actor->accumulator.vx;
	actor->oldPosition.vy = 0;//actor->position.vy;//actor->accumulator.vy;
	actor->oldPosition.vz = 0;//actor->position.vz;//actor->accumulator.vz;
	

	actor->accumulator.vx = 0;
	actor->accumulator.vy = 0;
	actor->accumulator.vz = 0;

//	actor->position = actor->oldPosition;

//	actor->position.vx = actor->position.vy = actor->position.vz = 0;

}






/**************************************************************************
	FUNCTION:	
	PURPOSE:	draw actor at actor->position
	PARAMETERS:	ACTOR *
	RETURNS:	
**************************************************************************/

void actorDraw(ACTOR *actor)
{
	char *compare;
	PSIOBJECT *world;

	world = actor->psiData.object;

	//convert qRot to matrix

// 	if(actor->qRot.x==0 && actor->qRot.y==0 && actor->qRot.z==0 && actor->qRot.w==0)
// 		utilPrintf("ZERO QUAT !\n");

	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);

/*	if ( ( compare = strstr ( actor->psiData.modelName, "BOULDER" ) ) )
	{
//		utilPrintf("%d : %d : %d : %d\n", actor->qRot.x, actor->qRot.y, actor->qRot.z, actor->qRot.w);
//		actor->psiData.object->rotate.vx += 1;
	}
	else*/

// bb - now done at the same time as actor clipping.
// i.e Actor2ClipCheck(), or for dffClipping (distancefromfrog) in DrawActorList().
// 	{
// 		TIMER_START2(TIMER_ACTDR_QUAT);
// 
// 		if ( !( actor->flags & ACTOR_NEW_NOREORIENTATE ) )
// 		{
// 			QuatToPSXMatrix(&actor->qRot, &actor->psiData.object->matrix);
// 		}
// 		// ENDIF
// 
// 		TIMER_STOP_ADD2(TIMER_ACTDR_QUAT);
// 	}
	// ENDELSEIF


	if(actor->psiData.flags & ACTOR_MOTIONBONE)
	{
		world->matrix.t[0] = -actor->position.vx;
		world->matrix.t[1] = -actor->position.vy;
		world->matrix.t[2] = actor->position.vz;
	}
	else
	{
		/*if((world->rotate.vx) || (world->rotate.vy) || (world->rotate.vz))
		{
			MATRIX		rotmat1;
			RotMatrixYXZ_gte(&world->rotate,&rotmat1);

			gte_SetRotMatrix(&world->matrix);
//			gte_SetTransMatrix	// not doing an rtirtr, so we don't need so set the tx to zero
			gte_ldlvl(&world->matrix.t);
			gte_rtir();
			gte_stlvl(&world->matrix.t);

			//bb move into here from outside this if,
			//because the QuatToPsxMatrix call has been moved,
			//(and the .t setting with it.)
	 		world->matrix.t[0] += -actor->position.vx;
	 		world->matrix.t[1] += -actor->position.vy;
	 		world->matrix.t[2] += actor->position.vz;
		}*/

//bb see above comment
//  		world->matrix.t[0] += -actor->position.vx;
 // 		world->matrix.t[1] += -actor->position.vy;
 // 		world->matrix.t[2] += actor->position.vz;
	}




	PSIrootScale = &actor->psiData.object->scale;
	
	TIMER_START2(TIMER_ACTDR_MTX);
	psiCalcWorldMatrix(actor->psiData.object);
	TIMER_STOP_ADD2(TIMER_ACTDR_MTX);

	TIMER_START2(TIMER_ACTDR_SEG);
	psiDrawSegments(&actor->psiData);
	TIMER_STOP_ADD2(TIMER_ACTDR_SEG);
}

/*
void actorDraw2(ACTOR *actor)
{
	MATRIX		scaleMatrix;
	PSIOBJECT	*world;

	world = actor->psiData.object;

	// create out rotation matrix
	RotMatrixYXZ_gte(&world->rotate,&world->matrix);

	// create our scale matrix
	scaleMatrix = GsIDMATRIX;
	ScaleMatrix(&scaleMatrix, &actor->size);

	// multiply camera matrix by our scale matrix
	gte_MulMatrix0(&GsWSMATRIX, &scaleMatrix, &world->matrixscale);

	// then by our rotation matrix
	gte_MulMatrix0(&world->matrixscale, &world->matrix, &world->matrixscale);

	// get our position in camera space
	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);
	gte_ldlvl(&actor->position);
	gte_rtirtr();
	gte_stlvl(&world->matrixscale.t);

	psiDrawSegments(&actor->psiData);

}
*/




/*void actorDrawBones(ACTOR *actor)
{
	PSIOBJECT *world;

   	world = actor->psiData.object;

	if(actor->psiData.flags & ACTOR_MOTIONBONE)
	{
		world->matrix.t[0] = actor->position.vx;
		world->matrix.t[1] = actor->position.vy;
		world->matrix.t[2] = actor->position.vz;
	}
	else
	{
		world->matrix.t[0] += actor->position.vx;
		world->matrix.t[1] += actor->position.vy;
		world->matrix.t[2] += actor->position.vz;
	}

	PSIrootScale = &world->scale;
	
	psiCalcLocalMatrix(world);

	if(world->child)
	{
		gte_SetRotMatrix(&GsWSMATRIX);
		gte_SetTransMatrix(&GsWSMATRIX);
		drawBones(world->child, world);
	}
}*/




/**************************************************************************
	FUNCTION:	actorUpdateAnimations
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void actorUpdateAnimations(ACTOR *actor)
{
	ACTOR_ANIMATION *actorAnim = &actor->animation;
	ANIMATION *anim;
	int i;

	if((gameState.mode == PAUSE_MODE ) || (gameState.mode == GAMEOVER_MODE))
		return;

	//reject all objects which have no animation
	if(actorAnim->numAnimations == 0)
		return;

	if(actorAnim->currentAnimation < 0)
		return;

	anim = (ANIMATION*) (actor->animSegments + (actorAnim->currentAnimation*2));
	
	actorAnim->reachedEndOfAnimation = 0;

//bb
//	actorAnim->animTime += actorAnim->animationSpeed;
	actorAnim->animTime += (actorAnim->animationSpeed*gameSpeed)>>12;

	actorAnim->frame = actorAnim->animTime >> ANIMSHIFT;
	
	if( ((actorAnim->frame > anim->animEnd) || (actorAnim->frame < anim->animStart)) &&
	 (actorAnim->loopAnimation == 0))
	{
		actorAnim->frame = Bound(actorAnim->frame,anim->animStart,anim->animEnd);
		actorAnim->animTime = actorAnim->frame << ANIMSHIFT;
		actorAnim->reachedEndOfAnimation = actorAnim->currentAnimation+1;

		//queue stuff - do this later
		if(actorAnim->queueAnimation[0] != -1)
		{
			actorAnim->currentAnimation = actorAnim->queueAnimation[0];//actorAnim;
			actorAnim->loopAnimation = actorAnim->queueLoopAnimation[0];//loop;
			actorAnim->animationSpeed = actorAnim->queueAnimationSpeed[0];//speed;
			if(actorAnim->animationSpeed < 0)
			{
				actorAnim->animTime = actor->animSegments[actorAnim->currentAnimation*2 +1] <<ANIMSHIFT;
			}
			else
			{
				actorAnim->animTime = actor->animSegments[actorAnim->currentAnimation*2] <<ANIMSHIFT;
			}

			for(i = 0;i < ANIM_QUEUE_LENGTH - 1;i++)
			{
				actorAnim->queueAnimation[i] = actorAnim->queueAnimation[i + 1];
				actorAnim->queueLoopAnimation[i] = actorAnim->queueLoopAnimation[i + 1];
				actorAnim->queueAnimationSpeed[i] = actorAnim->queueAnimationSpeed[i + 1];
			}
			actorAnim->queueAnimation[ANIM_QUEUE_LENGTH - 1] = -1;
			actorAnim->queueLoopAnimation[ANIM_QUEUE_LENGTH - 1] = -1;
			actorAnim->queueAnimationSpeed[ANIM_QUEUE_LENGTH - 1] = -1;
			if(actorAnim->numberQueued >0) 
			{
				actorAnim->numberQueued--;
			}
		}
	}
	else
	{
		if(actorAnim->frame > anim->animEnd)
		{
			actorAnim->frame = anim->animStart;
			actorAnim->animTime = anim->animStart<<ANIMSHIFT;
			actorAdjustPosition(actor);
		}
		else 
		{
			if(actorAnim->frame < anim->animStart)
			{
				actorAnim->frame = anim->animEnd;
				actorAnim->animTime = anim->animEnd <<ANIMSHIFT;
				actorAdjustPosition(actor);
			}
		}
	}
}

/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void actorSetAnimation(ACTOR *actor, ULONG frame, int setKeys)
{
	PSIOBJECT *world;
	ACTOR_ANIMATION *actorAnim = &actor->animation;
	ANIMATION *anim;
//	long temp0,temp1,temp2;
//	VECTOR result;

	world = actor->psiData.object;
	
	PSIactorScale = &actor->size;

//bbopt
  psiSetKeyFrames(world, frame);
	//if ( setKeys )
		//bb_psiSetKeyFrames(world, frame);

	/*
		
	actor->accumulator.vx = world->matrix.t[0];
	actor->accumulator.vy = world->matrix.t[1];
	actor->accumulator.vz = world->matrix.t[2];

	world->matrix.t[0] -= actor->oldPosition.vx;
	world->matrix.t[1] -= actor->oldPosition.vy;
	world->matrix.t[2] -= actor->oldPosition.vz;

	actor->oldPosition = actor->accumulator;

	temp0 = world->matrix.t[0];
	temp1 = world->matrix.t[1];
	temp2 = world->matrix.t[2];
	
	actorRotate(world->rotate.vx,world->rotate.vy,world->rotate.vz,temp0,temp1,temp2,&result);

	world->matrix.t[0] = result.vx;
	world->matrix.t[1] = result.vy;
	world->matrix.t[2] = result.vz;
	*/
	
	if (actorAnim->exclusive)  
	{
		anim = (ANIMATION*) (actor->animSegments + (actorAnim->currentAnimation*2));
		if(actorAnim->frame >= anim->animEnd)
		{
			actorAnim->frame = anim->animStart;
			actorAnim->animTime = anim->animStart<<ANIMSHIFT;
			actorAdjustPosition(actor);
		}
		else 
		{
			if(actorAnim->frame < anim->animStart)
			{
				actorAnim->frame = anim->animEnd;
				actorAnim->animTime = anim->animEnd <<ANIMSHIFT;
				actorAdjustPosition(actor);
			}
		}
	}		
		// Store Root Bone movement
		// Use Basic Position - IE Set root bone to zero
}





/**************************************************************************
	FUNCTION:	actorSetAnimation()
	PURPOSE:	Set keyframe information for current object frame
	PARAMETERS:	Model structure, frame number
	RETURNS:	
**************************************************************************/


/*void actorSetAnimation2(ACTOR *actor, ULONG frame0, ULONG frame1, ULONG blend)
{
	PSIOBJECT *world;
	ACTOR_ANIMATION *actorAnim = &actor->animation;
	ANIMATION *anim;
//	long temp0,temp1,temp2;
//	VECTOR result;

	world = actor->psiData.object;
	
	PSIactorScale = &actor->size;

	psiSetKeyFrames2(world, frame0, frame1, blend);
	
	/*
	actor->accumulator.vx = world->matrix.t[0];
	actor->accumulator.vy = world->matrix.t[1];
	actor->accumulator.vz = world->matrix.t[2];

	world->matrix.t[0] -= actor->oldPosition.vx;
	world->matrix.t[1] -= actor->oldPosition.vy;
	world->matrix.t[2] -= actor->oldPosition.vz;

//	utilPrintf("add %d (%d)\n",world->matrix.t[1],actor->oldPosition.vy);

	actor->oldPosition = actor->accumulator;

	temp0 = world->matrix.t[0];
	temp1 = world->matrix.t[1];
	temp2 = world->matrix.t[2];
	
	actorRotate(world->rotate.vx,world->rotate.vy,world->rotate.vz,temp0,temp1,temp2,&result);

	world->matrix.t[0] = result.vx;
	world->matrix.t[1] = result.vy;
	world->matrix.t[2] = result.vz;
	*/

	/*if (actorAnim->exclusive)  
	{
		anim = (ANIMATION*) (actor->animSegments + (actorAnim->currentAnimation*2));
		if(actorAnim->frame >= anim->animEnd)
		{
			actorAnim->frame = anim->animStart;
			actorAnim->animTime = anim->animStart<<ANIMSHIFT;
			actorAdjustPosition(actor);
		}
		else 
		{
			if(actorAnim->frame < anim->animStart)
			{
				actorAnim->frame = anim->animEnd;
				actorAnim->animTime = anim->animEnd <<ANIMSHIFT;
				actorAdjustPosition(actor);
			}
		}
	}		
		// Store Root Bone movement
		// Use Basic Position - IE Set root bone to zero
}
*/





/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
/*
void actorDrawRadiusBox(ACTOR *actor)
{

	BOUNDINGBOX *corners;
	SVECTOR v;
	//long lx,rx,ty,by,ison;
	DVECTOR scrxy1,scrxy2;

//	PSISetBoundingRotated(actor,actor->animation.frame,actor->object->rotate.vx,actor->object->rotate.vy,actor->object->rotate.vz);
//	PSISetBounding(actor,0);

	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);

	corners = &actor->bounding;
//////////////
	corners->maxX = actor->radius;
	corners->maxY = actor->radius;
	corners->maxZ = actor->radius;

	corners->minX = -actor->radius;
	corners->minY = -actor->radius;
	corners->minZ = -actor->radius;

////////////

	polyLine.r0 = 240;
	polyLine.g0 = 0;
	polyLine.b0 = 240;

	v.vx = corners->minX+actor->position.vx;
	v.vy = corners->minY+actor->position.vy;
	v.vz = corners->minZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

	v.vx = corners->maxX+actor->position.vx;
	v.vy = corners->maxY+actor->position.vy;
	v.vz = corners->minZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);


	v.vx = corners->maxX+actor->position.vx;
	v.vy = corners->minY+actor->position.vy;
	v.vz = corners->maxZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

	v.vx = corners->minX+actor->position.vx;
	v.vy = corners->maxY+actor->position.vy;
	v.vz = corners->maxZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);


}
*/

/**************************************************************************
	FUNCTION:	PSISetBoundingRotated
	PURPOSE:	set bounding box values for animation frame & rotations
	PARAMETERS:	
	RETURNS:	sets values in actor structure's bounding box
**************************************************************************/

void actorSetBoundingRotated(ACTOR *actor,int frame,int rotX,int rotY, int rotZ)
{
	int x,y,z;

	// save rotations
	
	x = actor->psiData.object->rotate.vx;	
	y = actor->psiData.object->rotate.vy;	
	z = actor->psiData.object->rotate.vz;	

	actor->psiData.object->rotate.vx = rotX;		
	actor->psiData.object->rotate.vy = rotY;
	actor->psiData.object->rotate.vz = rotZ;

	// set the model 'pose'
	actorSetAnimation(actor, frame, 1);

	PSIrootScale = &actor->psiData.object->scale;
	psiCalcLocalMatrix(actor->psiData.object);

	actorCalcSegments(actor);

	// restore rotations
		
	actor->psiData.object->rotate.vx = x;		
	actor->psiData.object->rotate.vy = y;
	actor->psiData.object->rotate.vz = z;
}

/**************************************************************************
	FUNCTION:
	PURPOSE:	find min and max xyz from model 'pose'
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

int actorCalcSegments(ACTOR *actor)
{
	register PSIOBJECT	*world;
	int loop,obs,i,j,q,x,y,z,s;
	long flg;
	VECTOR	tfv;
	short *verts;
	SVECTOR v;
	BOUNDINGBOX *corners;

	q = 0;
	obs = actor->psiData.numObjects;
	corners = &actor->bounding;

	corners->minX = corners->maxX = 0;
	corners->minY = corners->maxY = 0;
	corners->minZ = corners->maxZ = 0;

	for (loop = 0; loop < obs; loop++)
	{
		world = (PSIOBJECT*)actor->psiData.objectTable[loop];

	 	gte_SetRotMatrix(&world->matrixscale);
	   	gte_SetTransMatrix(&world->matrixscale);

		verts=(short*)world->meshdata->vertop;
		j = world->meshdata->vern;

		for (i=0; i < j; i++)
		{

			v.vx = *(verts++);
			v.vy = *(verts++);
			v.vz = *(verts++);

			RotTrans(&v,&tfv,&flg);
			
			verts++;

			x = tfv.vx;
			y = tfv.vy;
			z = tfv.vz;


			if ( x < corners->minX )
				corners->minX = x;
			if ( x > corners->maxX )
				corners->maxX = x;
			if ( y < corners->minY )
				corners->minY = y;
			if ( y > corners->maxY )
				corners->maxY = y;
			if ( z < corners->minZ )
				corners->minZ = z;
			if ( z > corners->maxZ )
				corners->maxZ = z;
							 

			s = abs(x*x) + abs(y*y) + abs(z*z);

			if (s>q)
			{
				q = s;
			}
		}
	}

  	j = utilSqrt(q) >> 16;

	return j;	
}




void ScalePsi(PSIMESH* pMesh)
{
//	int m,v;
	int v;
	SVKEYFRAME* pK;
	int i;

	//loop verts
	for(v=pMesh->vern-1; v>=0; v--)
	{
		pMesh->vertop[v].vx *= SCALE;
		pMesh->vertop[v].vy *= SCALE;
		pMesh->vertop[v].vz *= SCALE;
	}


	pK = pMesh->movekeys;
	for(i=0; i<pMesh->numMoveKeys; i++, pK++)
	{
		pK->vect.x *= SCALE;
		pK->vect.y *= SCALE;
		pK->vect.z *= SCALE;
	}

		//loop scalekeys
/* 		for(v=pMesh->numScaleKeys-1; v>=0; v--)
 		{
 			pMesh->scalekeys[v].vect.x *= SCALE;
 			pMesh->scalekeys[v].vect.y *= SCALE;
 			pMesh->scalekeys[v].vect.z *= SCALE;
 		}*/

	if(pMesh->child)
		ScalePsi(pMesh->child);

	if(pMesh->next)
		ScalePsi(pMesh->next);
}


/*	--------------------------------------------------------------------------------
	Function 	: ChangeModel
	Purpose 	: Swap models within an actor
	Parameters 	: actor, name of new model
	Returns 	: void
	Info 		:
*/
void *ChangeModelNoUndo( ACTOR *actor, char *model )
{
	ACTOR *newActor, oldActor;
	PSIMODEL *newModel;
	char newName[16];
	int i=0;

	actorSub(actor);

	//find model
	while( model[i] != '.' && model[i] != '\0' )
	{
		newName[i] = model[i];
		i++;
	}
	newName[i] = '\0';
	strcat( newName, ".psi" );

	newModel = psiCheck ( newName );

	utilPrintf("Trying To Find New Model %s : %s................\n", newModel);

	if( !newModel )
	{
		utilPrintf("Could Not Find Replacment Model................\n");
		return;
	}

	//create new actor
	newActor = actorCreate( newModel, 1, 0 );

	//backup current actor
	oldActor = *actor;

	//copy over current actor
	*actor = *newActor;

	//keep some of the original data
	actor->position = oldActor.position;
	actor->qRot		= oldActor.qRot;

	actorFree(newActor);
}


/*	--------------------------------------------------------------------------------
	Function 	: ChangeModel
	Purpose 	: Swap models within an actor
	Parameters 	: actor, name of new model
	Returns 	: void
	Info 		:
*/
ACTOR *newActor = NULL;
void *ChangeModel( ACTOR *actor, char *model )
{
	PSIMODEL *newModel;
	char newName[16];
	int i=0;

	UndoChangeModel(actor);
	actorSub(actor);
//	actorFree2( &oldActor );
	//find model
	while( model[i] != '.' && model[i] != '\0' )
	{
		newName[i] = model[i];
		i++;
	}
	newName[i] = '\0';
	strcat( newName, ".psi" );

//	oldModel = actor->psiData;
	newModel = psiCheck ( newName );

	utilPrintf("Trying To Find New Model %s : %s................\n", newModel);

	if( !newModel )
	{
		utilPrintf("Could Not Find Replacment Model................\n");
		return;
	}

	//create new actor
	newActor = actorCreate ( newModel, 1, 0 );

	newActor->size.vx = globalFrogScale;
	newActor->size.vy = globalFrogScale;
	newActor->size.vz = globalFrogScale;

	if( player[0].character == FROG_BABYFROG )
		ScaleVectorFF( &newActor->size, 3072 );
//bb
//	actor->psiData = newActor->psiData;
//	actorInitAnim ( actor );				// initialise animation structure
//	actorSetAnimation ( actor, 0, 1 );		// initialise animation structure

	//backup current actor
	oldActor = *actor;

	//copy over current actor
	*actor = *newActor;


	//keep some of the original data
	actor->position = oldActor.position;
	actor->qRot			= oldActor.qRot;

//	actorFree( newActor );
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
/*
	if ( !oldModel.object )
		return 0;

	actor->psiData = oldModel;

	actorInitAnim ( actor );										// initialise animation structure
	actorAnimate( actor, FROG_ANIM_BREATHE, YES, NO, 102, 0);

	return 1;
*/
//	ACTOR *oa;

	//make sure we are resetting to a valid actor
	if(!oldActor.psiData.object)
		return 0;

//	oa = &oldActor;
	actorFree2( actor );
	FREE(newActor);
	*actor = oldActor;
	actor->next = NULL;
	actorAdd(actor);
	oldActor.psiData.object = NULL;

//	actorFree( &oldActor );

	return 1;
}


/*	--------------------------------------------------------------------------------
	Function	: AnimateActor
	Purpose		: Runs an animation on an actor EXCEPT when it's already running
	Parameters	: actor, anim, loop?, queue?, speed, morph?, keep proportion?
	Returns		: void
*/
void StartAnimateActor(ACTOR *actor, int animNum, char loop, char queue, int speed, short blendSpeed)
{
 	if (actor->animation.currentAnimation != animNum)
 		actorAnimate(actor, animNum, loop, queue, speed, blendSpeed);
}

/*void actorSetAnimationSpeed(ACTOR *actor, int speed)
{
	actor->animation.animationSpeed = speed;
}
*/
/**************************************************************************
	FUNCTION:	
	PURPOSE:	draw all actors in actorList
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

/*void actorDrawAll()
{

	int loop;
	ACTOR *tempActor;

	loop = actorList.numEntries;
	tempActor = actorList.head.next;
	
	while (loop)
	{
		actorMove(tempActor);

		actorDraw(tempActor);

		tempActor = tempActor->next;
		loop--;
	}
}*/

/**************************************************************************
	FUNCTION:	
	PURPOSE:	move actor at actor->position
	PARAMETERS:	ACTOR *
	RETURNS:	
**************************************************************************/

/*void actorMove(ACTOR *actor)
{
	int i,v,rotcalc,rotate;
	PSIOBJECT *world;
	register PSIMODELCTRL	*modctrl = &PSImodelctrl;

   	world = actor->psiData.object;

	if ( !(actor->psiData.flags & ACTOR_DYNAMICSORT) )
	{
		v = 0;
		if (-PSImodelctrl.whichcamera->vpy < actor->position.vy)
		{
			v=4;
		}

		rotcalc=utilCalcAngle(	PSImodelctrl.whichcamera->vpx - actor->position.vx,
							PSImodelctrl.whichcamera->vpz - actor->position.vz)&0xffff;
		rotate=world->rotate.vy+2048; 

		i = utilCalcAngle(	PSImodelctrl.whichcamera->vpx - actor->position.vx,
						PSImodelctrl.whichcamera->vpz - actor->position.vz)&0xffff;
		i=((((rotcalc+rotate)&4095)/1024))&3;
		i += v;
		modctrl->sorttable=i;
	}

	actorSetAnimation(actor, actor->animation.frame, 1);


	if (actor->psiData.flags&ACTOR_MOTIONBONE)
	{
		if (actor->psiData.flags&ACTOR_HOLDMOTION)
		{
			world->matrix.t[0] = actor->position.vx;
			world->matrix.t[1] = actor->position.vy;
			world->matrix.t[2] = actor->position.vz;
		}
		else
		{
			world->matrix.t[0] += actor->position.vx;
			world->matrix.t[1] += actor->position.vy;
			world->matrix.t[2] += actor->position.vz;
			actor->position.vx = world->matrix.t[0];
			actor->position.vy = world->matrix.t[1];
			actor->position.vz = world->matrix.t[2];
		}
	}
}
*/
/*void actorDraw2(ACTOR *actor)
{
	MATRIX		scaleMatrix;
	PSIOBJECT	*world;

	world = actor->psiData.object;

	// create out rotation matrix
	RotMatrixYXZ_gte(&world->rotate,&world->matrix);

	// create our scale matrix
	scaleMatrix = GsIDMATRIX;
	ScaleMatrix(&scaleMatrix, &actor->size);

	gte_MulMatrix0(&GsWSMATRIX, &world->matrix, &world->matrixscale);
	gte_MulMatrix0(&world->matrixscale, &scaleMatrix, &world->matrixscale);

	// get our position in camera space
	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);
	gte_ldlvl(&actor->position);
	gte_rtirtr();
	gte_stlvl(&world->matrixscale.t);

	psiDrawSegments(&actor->psiData);
}*/

/*void drawBones(PSIOBJECT *world, PSIOBJECT *parent)
{
	LINE_F2	*line;
	VERT	worldPos, parentPos;
	long	worldXY, parentXY;

	while(world)
	{
		worldPos.vx = world->matrixscale.t[0];
		worldPos.vy = world->matrixscale.t[1];
		worldPos.vz = world->matrixscale.t[2];
		
		parentPos.vx = parent->matrixscale.t[0];
		parentPos.vy = parent->matrixscale.t[1];
		parentPos.vz = parent->matrixscale.t[2];

		gte_ldv0(&worldPos);
		gte_rtps();
		gte_stsxy(&worldXY);

		gte_ldv0(&parentPos);
		gte_rtps();
		gte_stsxy(&parentXY);

		BEGINPRIM(line, LINE_F2);
		*(u_long *)&line->x0 = *(u_long *)&worldXY;
		*(u_long *)&line->x1 = *(u_long *)&parentXY;
		setRGB0(line, 255, 255, 255);
		setLineF2(line);
		ENDPRIM(line, 0, LINE_F2);

		if(world->child)
			drawBones(world->child, world);

		world = world->next;
	}
}
*/
/**************************************************************************
	FUNCTION:	actorUpdateAnimation()
	PURPOSE:	Set keyframe information for current object frame
	PARAMETERS:	Model structure, frame number
	RETURNS:	
**************************************************************************/

void actorUpdateAnimation()
{
	int loop;
	ACTOR *actor;
	
	loop = actorList.numEntries;
	actor = actorList.head.next;
	while (loop)
	{

		actorUpdateAnimations(actor);
		actor = actor->next;
		loop--;

	}	
}

/**************************************************************************
	FUNCTION:	actorSetAnimation2()
	PURPOSE:	Set keyframe information for current object ((4096 - blend) * frame0) + (blend * frame1)
	PARAMETERS:	actor, frame0, frame1, blend
	RETURNS:	
**************************************************************************/

void actorAnimate(ACTOR *actor, int animNum, char loop, char queue, int speed, short blendSpeed)
{

	ACTOR_ANIMATION *actorAnim = &actor->animation;
	ANIMATION *anim;
	int	actualSpeed;

	
	//bb
	//this func keeps crashing when it couldn't create the actor
//	if(!actor)
//		return;



	if(actorAnim->numAnimations == 0)
	{
		return;
	}

	if( animNum >= actorAnim->numAnimations )
	{
		utilPrintf("anim index out of range: %s\n",actor->psiData.modelName);
		return;
	}


//	utilPrintf("Actor Animate.......%s : %d : %d\n", actor->psiData.modelName, animNum, actorAnim->numberQueued);

/*	if( (actorAnim->currentAnimation == animNum) &&
		(queue == 0) && 
		(actorAnim->reachedEndOfAnimation == 0) &&
		( speed == actorAnim->animationSpeed) )
	{
		actorAnim->loopAnimation = loop;
		actorFlushQueue(actor);
		return;
	}*/

	if(speed == 0)
	{
		actualSpeed = 256;
	}
	else
	{
		actualSpeed = speed;
	}

	actorAnim->exclusive = 0;
	
	if(queue == 0)
	{
		actorAdjustPosition(actor);

		actor->animation.blendFrame = 0;
		actorAnim->oldAnimation = actorAnim->currentAnimation;

		actorAnim->currentAnimation = animNum;

		actorAnim->loopAnimation = loop;
		actorAnim->animationSpeed = actualSpeed;
		actorAnim->reachedEndOfAnimation = 0;
		anim = (ANIMATION*)(actor->animSegments + (actorAnim->currentAnimation*2));
		actorAnim->animTime = anim->animStart << ANIMSHIFT;
		actorAnim->frame = anim->animStart;
		actorFlushQueue(actor);
		if(actualSpeed < 0)
		{
			actorAnim->animTime = anim->animEnd << ANIMSHIFT;
		}

		//if ( blendSpeed )
	//	{
	//		actorAnim->blendSpeed				= blendSpeed;
	//		actorAnim->blendStartFrame	= ((ANIMATION*)(actor->animSegments + (actorAnim->oldAnimation*2)))->animEnd;
	//		actorAnim->blendEndFrame		= anim->animStart;
	//	}
		// ENDIF
	}
	else
	{
		if(actorAnim->currentAnimation == -1)
		{
			actorAnim->currentAnimation = animNum;
			actorAnim->loopAnimation = loop;
			actorAnim->animationSpeed = actualSpeed;
		}
		else
		{
			if(actorAnim->numberQueued == 5)
			{
				return;
			}

			actorAnim->queueAnimation[actorAnim->numberQueued] = animNum;
			actorAnim->queueLoopAnimation[actorAnim->numberQueued] = loop;
			actorAnim->queueAnimationSpeed[actorAnim->numberQueued] = actualSpeed;
			actorAnim->numberQueued++;
		}
	}


}

/*void actorRotate(short angx, short angy, short angz, long movex, long movey, long movez, VECTOR *result)
{																
	SVECTOR  MATHS_ang;											
	VECTOR   MATHS_move;										
	MATRIX   MATHS_rotMat;										
																
	MATHS_ang.vx = (angx);										
	MATHS_ang.vy = (angy);										
	MATHS_ang.vz = (angz);										
																
	MATHS_move.vx = (movex);								
	MATHS_move.vy = (movey);									
	MATHS_move.vz = (movez);									
																
	MATHS_rotMat = GsIDMATRIX;									
	RotMatrixYXZ_gte(&MATHS_ang, &MATHS_rotMat);				
	ApplyMatrixLV(&MATHS_rotMat, &MATHS_move, (result));		
}*/

/**************************************************************************
	FUNCTION:	actorIsVisible
	PURPOSE:	using actor's preset radius, see if it's screen vivible
	PARAMETERS:	
	RETURNS:	true / false
**************************************************************************/

UBYTE actorIsVisible(ACTOR *actor)
{
	VERT	v[3];
	DVECTOR	scrXY[3];
	long	scrZ[3];
//	LINE_F2	*line;

	gte_SetRotMatrix(&GsWSMATRIX);		// load viewpoint matrices
	gte_SetTransMatrix(&GsWSMATRIX);

	v[0].vx = actor->position.vx;		// use actor's world position
	v[0].vy = actor->position.vy - actor->radius;
	v[0].vz = actor->position.vz;

	v[1].vx = v[0].vx + actor->radius;
	v[1].vy = v[0].vy + actor->radius;
	v[1].vz = v[0].vz;

	v[2].vx = v[0].vx - actor->radius;
	v[2].vy = v[0].vy - actor->radius;
	v[2].vz = v[0].vz;

	gte_ldv3c(&v[0]);
	gte_rtpt();
	gte_stsxy3c(&scrXY[0]);
	gte_stsz3c(&scrZ[0]);

	if (scrZ[0] <= 0)
	{
		return 0;					// return if 'behind' camera
	}

	if ((scrZ[0]-actor->radius)>PSImodelctrl.farclip)
	{
		return 0;					// Too far away
	}

	if( (scrXY[1].vx < -256) || (scrXY[1].vy < -128) || (scrXY[2].vx > 256) || (scrXY[2].vy > 128))
	{
		return 0;					// Off screen
	}

	return 1;
}

/**************************************************************************
	FUNCTION:	actorSetBounding
	PURPOSE:	as actorSetBoundingRotated but forces 0 rotation
	PARAMETERS:	
	RETURNS:	sets values in actor structure's bounding box
**************************************************************************/

/*void actorSetBounding(ACTOR *actor,int frame)
{
	actorSetBoundingRotated(actor,frame,0,0,0);
}*/


/*void CopyKeyFrames ( PSIOBJECT *dest, PSIOBJECT *src )
{
	MATRIX		rotmat1;
	int j, i;

	
	while ( dest )
	{
/*		for ( i = 0; i < 3; i++ )
			for ( j = 0; j < 3; j++ )
				dest->matrix.m[i][j] = src->matrix.m[i][j];*/

	/*		dest->matrix = src->matrix;
			
			dest->rotate = src->rotate;
			
/*		for ( i = 0; i < 3; i++ )
			for ( j = 0; j < 3; j++ )
				dest->matrixscale.m[i][j] = src->matrixscale.m[i][j];

		for ( j = 0; j < 3; j++ )
			dest->matrix.t[j] = src->matrix.t[j];
		for ( j = 0; j < 3; j++ )
			dest->matrixscale.t[j] = src->matrixscale.t[j];

		dest->scale.vx = src->scale.vx;
		dest->scale.vy = src->scale.vy;
		dest->scale.vz = src->scale.vz;*/

		/*dest->rotate.vx = src->rotate.vx;
		dest->rotate.vy = src->rotate.vy;
		dest->rotate.vz = src->rotate.vz;*/

/*		dest->meshdata->center.vx = src->meshdata->center.vx;
		dest->meshdata->center.vy = src->meshdata->center.vy;
		dest->meshdata->center.vz = src->meshdata->center.vz;
*/
	/*	if((src->rotate.vx) || (src->rotate.vy) || (src->rotate.vz))
		{
			RotMatrixYXZ_gte(&src->rotate,&rotmat1);
			gte_MulMatrix0(&rotmat1,&src->matrix,&dest->matrix);
		}*/

//		RotMatrixYXZ_gte ( &dest->rotate, &rotmat1 );
//		gte_MulMatrix0 ( &rotmat1, &dest->matrix, &dest->matrix );

/*		if ( src->child )
			CopyKeyFrames ( dest->child, src->child );

		dest	= dest->next;
		src		= src->next;
		
	}
	
*/

/*	for ( i = 0; i < 3; i++ )
		for ( j = 0; j < 3; j++ )
			dest->psiData.object->matrix.m[i][j] = src->psiData.object->matrix.m[i][j];

/*	for ( i = 0; i < 3; i++ )
		for ( j = 0; j < 3; j++ )
			dest->psiData.object->matrixscale.m[i][j] = src->psiData.object->matrixscale.m[i][j];*/

/*	for ( j = 0; j < 3; j++ )
		dest->psiData.object->matrix.t[j] = src->psiData.object->matrix.t[j];
	/*for ( j = 0; j < 3; j++ )
		dest->psiData.object->matrixscale.t[j] = src->psiData.object->matrixscale.t[j];
*/
/*	dest->psiData.object->rotate.vx = src->psiData.object->rotate.vx;
	dest->psiData.object->rotate.vy = src->psiData.object->rotate.vy;
	dest->psiData.object->rotate.vz = src->psiData.object->rotate.vz;

	dest->psiData.object->scale.vx = src->psiData.object->scale.vx;
	dest->psiData.object->scale.vy = src->psiData.object->scale.vy;
	dest->psiData.object->scale.vz = src->psiData.object->scale.vz;


	if ( src->psiData.object->child )
		KeyFrameScan ( src->psiData.object->child, dest->psiData.object->child );

	if ( src->psiData.object->next )*
		KeyFrameScan ( src->psiData.object->next, dest->psiData.object->next );*/
/*}
*/



void actorResetAnimation ( ACTOR *actor )
{
}

int initcroakscale = 0;
int croaklife = 6000;
char croakDir			= 0;
void UpdateFrogCroak( int pl )
{
	SVECTOR effectPos;

	// frog is croaking
	if(player[pl].frogState & FROGSTATUS_ISCROAKING)
	{
		SPECFX *fx;
		FVECTOR normal;

		SetVectorFF( &normal, &currTile[pl]->normal );
		if( !(player[pl].isCroaking.time%2) )
		{
			SetVectorSS(&effectPos, &frog[pl]->actor->position);
			specFXAlpha = 128;

			PrepForPriorityEffect( );
			if( (fx = CreateSpecialEffectDirect( FXTYPE_CROAK, &effectPos, &normal, initcroakscale, 4096, 410, croaklife )) )
			{
				fx->spin = 20;
				SetFXColour( fx, 191,255,0);
			}
			specFXAlpha = 255;
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
				SVECTOR pos;
				FVECTOR up;
				SetVectorFF( &up, &upVec );
				ScaleVector( &up, 20 );
				AddVectorSFS( &pos, &up, &babyList[baby].baby->actor->position );

				specFXAlpha = 128;
	
				PrepForPriorityEffect( );
				if( (fx = CreateSpecialEffectDirect( FXTYPE_CROAK, &pos, &normal, initcroakscale, 4096, 410, croaklife )) )
				{
					int n;

					if( babyList[baby].enemy->path->nodes->offset2 )
						n = ((babyList[baby].enemy->path->nodes->offset2>>12)/SCALE)-1;
					else
						n = baby;

					fx->spin = 25;
					SetFXColour( fx, babyList[n].fxColour[R], babyList[n].fxColour[G], babyList[n].fxColour[B] );
				}
				else
					utilPrintf("Failed to create croak!!\n");
				specFXAlpha = 255;

				PlaySample( genSfx[GEN_BABYREPLY], &pos, 0, SAMPLE_VOLUME, -1 );
			}
		}
	}
}


void actorSetAnimation2(ACTOR *actor, ULONG frame0, ULONG frame1, ULONG blend)
{
	/*PSIOBJECT *world;
	ACTOR_ANIMATION *actorAnim = &actor->animation;
	ANIMATION *anim;
	long temp0,temp1,temp2;
	VECTOR result;

	world = actor->psiData.object;
	
	PSIactorScale = &actor->size;

	psiSetKeyFrames2(world, frame0, frame1, blend);

	utilPrintf("BlendFrame : %d : %d : %d\n", frame0, frame1, blend );

	actor->animation.blendFrame += (actor->animation.blendSpeed) * (gameSpeed>>12);
	
	if ( actor->animation.blendFrame > 4096 )
	{
		actor->animation.blendFrame = 4096;
		actor->animation.blendSpeed = 0;
	}
	// ENDIF

	/*
	actor->accumulator.vx = world->matrix.t[0];
	actor->accumulator.vy = world->matrix.t[1];
	actor->accumulator.vz = world->matrix.t[2];

	world->matrix.t[0] -= actor->oldPosition.vx;
	world->matrix.t[1] -= actor->oldPosition.vy;
	world->matrix.t[2] -= actor->oldPosition.vz;

//	utilPrintf("add %d (%d)\n",world->matrix.t[1],actor->oldPosition.vy);

	actor->oldPosition = actor->accumulator;

	temp0 = world->matrix.t[0];
	temp1 = world->matrix.t[1];
	temp2 = world->matrix.t[2];
	
	actorRotate(world->rotate.vx,world->rotate.vy,world->rotate.vz,temp0,temp1,temp2,&result);

	world->matrix.t[0] = result.vx;
	world->matrix.t[1] = result.vy;
	world->matrix.t[2] = result.vz;
	*/

	/*if (actorAnim->exclusive)  
	{
		anim = (ANIMATION*) (actor->animSegments + (actorAnim->currentAnimation*2));
		if(actorAnim->frame >= anim->animEnd)
		{
			actorAnim->frame = anim->animStart;
			actorAnim->animTime = anim->animStart<<ANIMSHIFT;
			actorAdjustPosition(actor);
		}
		else 
		{
			if(actorAnim->frame < anim->animStart)
			{
				actorAnim->frame = anim->animEnd;
				actorAnim->animTime = anim->animEnd <<ANIMSHIFT;
				actorAdjustPosition(actor);
			}
		}
	}		
		// Store Root Bone movement
		// Use Basic Position - IE Set root bone to zero*/
}


