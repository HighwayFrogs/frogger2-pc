/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: mdxReport.cpp
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <crtdbg.h>
#include <stdio.h>
#include <math.h>

#include "mdxDDraw.h"
#include "mdxD3D.h"
#include "mdxMath.h"
#include "mdxTexture.h"
#include "mdxObject.h"
#include "mdxActor.h"
#include "mdxCRC.h"
#include "mdxTiming.h"
#include "mgeReport.h"
#include "gelf.h"
#include "mdxLandscape.h"
#include "mdxRender.h"
#include "mdxPoly.h"
#include "mdxProfile.h"
#include "mdxException.h"

#ifdef __cplusplus
extern "C"
{
#endif

MDX_VECTOR	parentScaleStack[10];
int		parentScaleStackLevel = 0;
MDX_VECTOR  *actorScale;

MDX_ACTOR *currentDrawActor;

MDX_ACTOR *actorList = NULL;

#define NUM_DLISTS 2

MDX_ACTOR *actorDrawList[NUM_DLISTS] = {NULL};

#define MAX_UNIQUE_ACTORS	100
int uniqueActorCRC[MAX_UNIQUE_ACTORS];
long numActorsUniqe = 0;
void (*StartAnim)(MDX_ACTOR *me) = NULL;

/*  --------------------------------------------------------------------------------

	Function	: AddActorToList
	Purpose		: Add an actor to the MDX actor list
	Parameters	: (MDX_ACTOR *me)
	Returns		: unsigned long 
	Info		:
*/

unsigned long AddActorToList(MDX_ACTOR *me, unsigned long listNum)
{
	
	if (!me)
		return 0;

	if (strcmp((const char *)me->objectController->object->name,"world")==0)
		dp("WorldAddad");

	if (actorList)
	{
		actorList->prev = me;
		me->prev = NULL;
		me->next = actorList;
		actorList = me;
	}
	else
	{
		me->prev = me->next = NULL;
		actorList = me;
	}

	me->listNum = listNum;

	if (actorDrawList[listNum])
	{
		actorDrawList[listNum]->prevDraw = me;
		me->prevDraw = NULL;
		me->nextDraw = actorDrawList[listNum];
		actorDrawList[listNum] = me;
	}
	else
	{
		me->prevDraw = me->nextDraw = NULL;
		actorDrawList[listNum] = me;
	}

	return 1;
}


/*  --------------------------------------------------------------------------------

	Function	: KillObjectSprites
	Purpose		: When an object is culled in draw actor list, this tells the spries not to draw
	Parameters	: (MDX_OBJECT *me)
	Returns		: void 
	Info		: Stops the sticky sprite bug.	
*/

void KillObjectSprites(MDX_OBJECT *me)
{
	int i;
	
	for (i=0; i<me->numSprites; i++)
		me->sprites[i].sprite->draw = 0;
	
	if (me->next)
		KillObjectSprites(me->next);
	if (me->children)
		KillObjectSprites(me->children);
}


/*  --------------------------------------------------------------------------------

	Function	: ActorListDraw
	Purpose		: Draw all actors in the actor list
	Parameters	: (void)
	Returns		: void 
	Info		:
*/

void ActorListDraw(long i)
{
	MDX_ACTOR *cur = actorList;	
	MDX_VECTOR where,tPos,tPos2;
	float radius,scale,depth;
	MDX_MATRIX rotmat;
	char drawOverride = 0;
	int oldNoClip;

	cur = actorDrawList[i];

	while (cur)
	{
		drawOverride = 0;
		drawThisObjectsSprites = cur->draw;
		oldNoClip = noClip;

		if( noClip || (cur->flags & ACTOR_ALWAYSDRAW) )
		{
			drawOverride = 1;
			noClip = 1;
		}
		else
		{
			SetVector(&tPos,&cur->trueCentre);

			tPos.vx *= cur->scale.vx;
			tPos.vy *= cur->scale.vy;
			tPos.vz *= cur->scale.vz;
				
			QuaternionToMatrix(&cur->qRot,&rotmat);
			guMtxXFMF(rotmat.matrix,tPos.vx,tPos.vy,tPos.vz,&tPos2.vx,&tPos2.vy,&tPos2.vz);
			
			AddVector(&tPos,&cur->pos,&tPos2);

			XfmPoint(&where,&tPos,NULL);

			guMtxXFMF(vMatrix.matrix,tPos.vx,tPos.vy,tPos.vz,&tPos2.vx,&tPos2.vy,&tPos2.vz);

			scale = max(cur->scale.vx,max(cur->scale.vy,cur->scale.vz));
			radius = cur->radius * scale;
		}

		depth = tPos2.vz+DIST;

		if( drawOverride || (depth+radius>nearClip && depth-radius<farClip) )
		{
			if( where.vz > nearClip )
				radius = (FOV * radius * (rXRes*(1.0f/640.0f))) / (where.vz+DIST);

			if (drawOverride || ((where.vx > -radius) && (where.vx<rXRes+radius)) &&
				((where.vy > -radius) && (where.vy<rYRes+radius)))
			{	
				XformActor(cur,0);
				if (cur->draw)
				{
					if (cur->flags & ACTOR_WRAPTC)
						wrapCoords = 1;
					else
						wrapCoords = 0;
					DrawActor(cur);
				}
			}
			else
				KillObjectSprites(cur->objectController->object);
			
		}
		else
			KillObjectSprites(cur->objectController->object);
			
		noClip = oldNoClip;

		cur = cur->nextDraw;
	}	
}


/*  --------------------------------------------------------------------------------

	Function	: DrawActor
	Purpose		: Draw a single actor
	Parameters	: (MDX_ACTOR *actor)
	Returns		: void 
	Info		:
*/

void DrawActor(MDX_ACTOR *actor)
{
	// Optimisation.
	MDX_OBJECT_CONTROLLER *objectC = actor->objectController;


	// I can't see you...
	if(actor->visible == FALSE || !objectC)
		return;

	// xluOverride is used in the sprite engine, and globalXLU is used within the main polygon engine.
	
	globalXLU = (float)actor->xluOverride/100.0F;

	// If we are skinned then XForm all the vertices first and then draw the skin, otherwise draw it as we transform it.
	overrideTex = actor->overrideTex;
	softDepthOff = actor->depthOff;

	if (objectC->isSkinned)
	{
		numObjectsDrawn++;
	
		globalXLU2 = (((float)objectC->object->xlu) / ((float)0xff)) * globalXLU;
		if (objectC->object->flags & OBJECT_FLAGS_CLIPPED)
			return;
	
		if (objectC->object->phong)
		{
			//phong = objectC->object->phong;
			DrawObject(objectC->object, 2, objectC->object->mesh);
			PCRenderObjectPhong(objectC->object);
		}
		else
		{
			DrawObject(objectC->object, TRUE, objectC->object->mesh);
			PCRenderObject(objectC->object);
			if ((objectC->object->flags & OBJECT_FLAGS_CARTOON))
			{
				DrawObject(objectC->object, 3, objectC->object->mesh);
				PCRenderObjectOutline(objectC->object);
				AdjustObjectOutline();
				PCRenderObjectOutline(objectC->object);
			}
		}
	}
	else
	{
		DrawObject(objectC->object, FALSE, objectC->object->mesh);		
	}

	softDepthOff = 0;
}


/*  --------------------------------------------------------------------------------

	Function	: QueueFlush
	Purpose		: Flush all anims in the queue for a given actor
	Parameters	: (MDX_ACTOR *myActor)
	Returns		: void 
	Info		:
*/

void QueueFlush(MDX_ACTOR *myActor)
{
	int i;

	myActor->animation->numberQueued = 0;
	for(i = 0;i < MDX_ANIM_QUEUE_LENGTH;i++)
		myActor->animation->queueAnimation[i] = -1;
}


/*  --------------------------------------------------------------------------------

	Function	: Animate
	Purpose		: Animate an actor
	Paramete	: ( MDX_ACTOR *actor, int animNum, char loop, char queue, float speed)rs	
	Returns		: void 
	Info		:
*/

void Animate(MDX_ACTOR *actor, int animNum, char loop, char queue, float speed)
{
	MDX_ACTOR_ANIMATION *actorAnim = actor->animation;
	MDX_ANIMATION *anim;
	float	actualSpeed;

	if(actorAnim == NULL)
		return;
	if(actorAnim->anims == NULL)
		return;


	if(animNum > actorAnim->numAnimations)
	{
		animNum = 0;
	}

	if(speed == 0)
		actualSpeed = actorAnim->anims[animNum].speed;
	else
		actualSpeed = speed;

	if(queue == 0)
	{
		actorAnim->currentAnimation = animNum;
		anim = actorAnim->anims + actorAnim->currentAnimation;
		actorAnim->loopAnimation = loop;
		actorAnim->animationSpeed = actualSpeed;
		actorAnim->reachedEndOfAnimation = FALSE;
		if(actualSpeed < 0)
			actorAnim->animTime = anim->animEnd;
		else
			actorAnim->animTime = anim->animStart;
		QueueFlush(actor);

		if (StartAnim)
			StartAnim(actor);
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
			if(actorAnim->numberQueued == MDX_ANIM_QUEUE_LENGTH)
				return;
			actorAnim->queueAnimation[actorAnim->numberQueued] = animNum;
			actorAnim->queueLoopAnimation[actorAnim->numberQueued] = loop;
			actorAnim->queueAnimationSpeed[actorAnim->numberQueued] = actualSpeed;
			actorAnim->numberQueued++;
		}
	}
}


/*  --------------------------------------------------------------------------------

	Function	: UpdateAnims
	Purpose		: Update an actors anims
	Parameters	: ( MDX_ACTOR *actor)
	Returns		: void 
	Info		: Call once per frame
*/

void UpdateAnims(MDX_ACTOR *actor)
{
	MDX_ACTOR_ANIMATION *actorAnim = actor->animation;
	MDX_ANIMATION *anim;
	int i;

	if(actorAnim == NULL)
		return;

	if(actorAnim->anims == NULL)
		return;
	anim = actorAnim->anims + actorAnim->currentAnimation;

	if(actorAnim->currentAnimation < 0)
		return;

	actorAnim->reachedEndOfAnimation = FALSE;
	actorAnim->animTime += (actorAnim->animationSpeed) * timeInfo.speed;

	if( ((actorAnim->animTime > anim->animEnd) || (actorAnim->animTime < anim->animStart)) && (!actorAnim->loopAnimation || actorAnim->numberQueued) )
	{
		actorAnim->animTime = Bound(actorAnim->animTime,anim->animStart,anim->animEnd);
		actorAnim->reachedEndOfAnimation = 1;//actorAnim->currentAnimation+1;

		if(actorAnim->numberQueued)
		{
			actorAnim->currentAnimation = actorAnim->queueAnimation[0];
			actorAnim->loopAnimation = actorAnim->queueLoopAnimation[0];
			actorAnim->animationSpeed = actorAnim->queueAnimationSpeed[0];
			
			if(actorAnim->animationSpeed < 0)
				actorAnim->animTime = actorAnim->anims[actorAnim->currentAnimation].animEnd;
			else
				actorAnim->animTime = actorAnim->anims[actorAnim->currentAnimation].animStart;


			if (--actorAnim->numberQueued)
			{
				for(i = 0;i < MDX_ANIM_QUEUE_LENGTH - 1;i++)
				{
					actorAnim->queueAnimation[i] = actorAnim->queueAnimation[i + 1];
					actorAnim->queueLoopAnimation[i] = actorAnim->queueLoopAnimation[i + 1];
					actorAnim->queueAnimationSpeed[i] = actorAnim->queueAnimationSpeed[i + 1];
				}

				actorAnim->queueAnimation[MDX_ANIM_QUEUE_LENGTH - 1] = -1;
				actorAnim->queueLoopAnimation[MDX_ANIM_QUEUE_LENGTH - 1] = -1;
				actorAnim->queueAnimationSpeed[MDX_ANIM_QUEUE_LENGTH - 1] = -1;
			}
			else
			{
				*actorAnim->queueAnimation = -1;
				*actorAnim->queueLoopAnimation = -1;
				*actorAnim->queueAnimationSpeed = -1;
			}

			if (StartAnim)
				StartAnim(actor);		
		}
	}
	else
	{
		
		if(anim->animEnd == anim->animStart)
			actorAnim->animTime = anim->animEnd;			
		else if(actorAnim->animTime > anim->animEnd)
		{
			while (actorAnim->animTime > anim->animEnd)
				actorAnim->animTime -= (anim->animEnd - anim->animStart);

			if (StartAnim)
				StartAnim(actor);
		}
		else if(actorAnim->animTime < anim->animStart)
			actorAnim->animTime += (anim->animEnd - anim->animStart);			
		
	}
}



/*  --------------------------------------------------------------------------------
	Function	: CheckBoundingBox
	Purpose		: Compare a bounding box to see if it is visible
	Parameters	: Bounding box, object matrix
	Returns		: clipped?
	Info		: 
*/
enum { TOP=0x1, BOTTOM=0x2, LEFT=0x4, RIGHT=0x8, INWARD=0x10, OUTWARD=0x20 };

unsigned long CheckBoundingBox(MDX_VECTOR *bBox,MDX_MATRIX *m)
{
	MDX_VECTOR t[8];
	MDX_VECTOR *r = t;
	unsigned long i, *oc, ocs[8];

	float mtx[4][4], oozd;

	guMtxCatF((float *)m->matrix,(float *)vMatrix.matrix,(float *)mtx);

	for(i=8, oc=&ocs[7]; i; i--, r++, bBox++, oc-- )
	{
		//XfmPoint(r,bBox,m);
		*oc = 0;

		// do the work of XfmPoint, using the matrix concatenated above
		guMtxXFMF(mtx,
			bBox->vx,bBox->vy,bBox->vz,
			&(r->vx),&(r->vy),&(r->vz));

		// clippage

		if( r->vz < nearClip )
			*oc |= INWARD;
		else if( r->vz > farClip )
			*oc |= OUTWARD;
		else
		if (r->vz>=nearClip&&r->vz<=farClip)
		{
			// Perform XfmPoint's really fucked up perspective calculation
			// that's "so much more efficient".
			oozd = -FOV * *(oneOver+fftol((((long *)r)+2))+DIST);
			r->vx = halfWidth+(r->vx * oozd);
			r->vy = halfHeight+(r->vy * oozd);

			if( r->vx < 0 )
				*oc |= LEFT;
			else if( r->vx > rXRes )
				*oc |= RIGHT;

			if( r->vy < 0 )
				*oc |= TOP;
			else if( r->vy > rYRes )
				*oc |= BOTTOM;
		}

		if( !*oc ) 
			return 0;
	}

	if(!((ocs[0] & ocs[1]) &&
		(ocs[0] & ocs[3]) &&
		(ocs[0] & ocs[4]) &&
		(ocs[1] & ocs[2]) &&
		(ocs[1] & ocs[5]) &&
		(ocs[2] & ocs[3]) &&
		(ocs[2] & ocs[6]) &&
		(ocs[3] & ocs[7]) &&
		(ocs[4] & ocs[5]) &&
		(ocs[4] & ocs[7]) &&
		(ocs[5] & ocs[6]) &&
		(ocs[6] & ocs[7]) &&
		(ocs[4] & ocs[6]) &&
		(ocs[3] & ocs[5]) &&
		(ocs[4] & ocs[2]) &&
		(ocs[6] & ocs[0])))
		return 0;

	return 1;
}

long halve = 0;

/*  --------------------------------------------------------------------------------

	Function	: XformActor
	Purpose		: Transform an actor
	Parameters	: ( MDX_ACTOR *actor,long v)
	Returns		: void 
	Info		: Update anims, Interpolate anim keys, and Create object matrix
*/

void XformActor(MDX_ACTOR *actor, long v)
{
	MDX_OBJECT_CONTROLLER *objectC = actor->objectController;
	float transmat[4][4];
	float rotmat[4][4];
	float animTime;

	if (!objectC)
		return;

	UpdateAnims(actor);

	if((actor->animation) && (actor->animation->anims))
		animTime = actor->animation->animTime;
	else
		animTime = 0;

	currentDrawActor = actor;
	actorScale = &actor->scale;

	parentScaleStackLevel = 0;
	parentScaleStack[parentScaleStackLevel].vx = parentScaleStack[parentScaleStackLevel].vy = parentScaleStack[parentScaleStackLevel].vz = 1;

	guTranslateF(transmat, actor->pos.vx, actor->pos.vy, actor->pos.vz);
	PushMatrix(transmat);

	QuaternionToMatrix(&actor->qRot,(MDX_MATRIX *)rotmat);
	PushMatrix(rotmat);

	if(objectC->object)
		TransformObject(objectC->object, animTime);
	
	halve++;

  	parentScaleStackLevel = 0;
	parentScaleStack[parentScaleStackLevel].vx = parentScaleStack[parentScaleStackLevel].vy = parentScaleStack[parentScaleStackLevel].vz = 1;
	
	matrixStack.stackPosition-=2;
}


/*  --------------------------------------------------------------------------------

	Function	: InitAnims
	Purpose		: Init actors anim structures
	Parameters	: ( MDX_ACTOR *tempActor)
	Returns		: void 
	Info		:
*/

void InitAnims(MDX_ACTOR *tempActor)
{

	if( !tempActor->objectController )
		return;
	
	memcpy(tempActor->animation, tempActor->objectController->animation, sizeof(MDX_ACTOR_ANIMATION));	

	tempActor->animation->currentAnimation = -1;
	tempActor->animation->queueAnimation[0] = -1;
	tempActor->animation->queueAnimation[1] = -1;
	tempActor->animation->queueAnimation[2] = -1;
	tempActor->animation->queueAnimation[3] = -1;
	tempActor->animation->queueAnimation[4] = -1;
	tempActor->animation->loopAnimation = 0;
	tempActor->animation->numberQueued = 0;
	tempActor->animation->animTime = 0;
	tempActor->animation->reachedEndOfAnimation = FALSE;	
}


/*  --------------------------------------------------------------------------------

	Function	: InitActorStructures
	Purpose		: Init actors structures
	Parameters	: (MDX_ACTOR *tempActor, int initFlags)
	Returns		: void 
	Info		:
*/

void InitActorStructures(MDX_ACTOR *tempActor, int initFlags)
{
	tempActor->scale.vx = tempActor->scale.vy = tempActor->scale.vz = 1;

	if(initFlags & INIT_ANIMATION)
	{
		if((tempActor->objectController) && (tempActor->objectController->animation))
		{
			tempActor->animation = (MDX_ACTOR_ANIMATION *)AllocMem(sizeof(MDX_ACTOR_ANIMATION));
			InitAnims(tempActor);
		}
	}

	if(initFlags & INIT_SHADOW)
	{
		tempActor->shadow = (MDX_ACTOR_SHADOW *) AllocMem(sizeof(MDX_ACTOR_SHADOW));
		tempActor->shadow->alpha = 255;
		tempActor->shadow->draw = 1;
	}

	tempActor->status = 1;
	tempActor->xluOverride = 100;
}


/*  --------------------------------------------------------------------------------

	Function	: InitActor
	Purpose		: Init actor
	Parameters	: ( MDX_ACTOR *tempActor, char *name, float x, float y, float z, int initFlags)	
	Returns		: void 
	Info		:
*/

void InitActor(MDX_ACTOR *tempActor, char *name, float x, float y, float z, int initFlags)
{
	FindObject(&tempActor->objectController, UpdateCRC(name), name);
	tempActor->LODObjectController = NULL;
	
	if(!tempActor->objectController)
		return;

	InitActorStructures(tempActor, initFlags);

	tempActor->pos.vx = x;	tempActor->pos.vy = y;	tempActor->pos.vz = z;
	tempActor->scale.vx = tempActor->scale.vy = tempActor->scale.vz = 1;
	tempActor->flags = 0;
	tempActor->overrideTex = 0;

	ZeroQuaternion(&tempActor->qRot);
}


/*  --------------------------------------------------------------------------------

	Function	: FreeUniqueActorList
	Purpose		: Frees the Unique actor list.
	Parameters	: (void)
	Returns		: void 
	Info		: Doesn't free the unique actors themselves!!!
*/

void FreeUniqueActorList(void)
{
	for (int i=0; i<numActorsUniqe; i++)
		uniqueActorCRC[i] = 0;
	numActorsUniqe = 0;

}



/*  --------------------------------------------------------------------------------

	Function	: FreeUniqueObject
	Purpose		: Free A unique copy of an objects structures
	Parameters	: (MDX_OBJECT *object)
	Returns		: void 
	Info		:
*/

void FreeUniqueObject(MDX_OBJECT *object)
{
	if (object)
	{
		
		if (object->numSprites)
			FreeMem (object->sprites);

		if(object->children)
			FreeUniqueObject(object->children);

		if(object->next)
			FreeUniqueObject(object->next);	
		
		FreeMem (object);
	}
}



void FreeActor(MDX_ACTOR **toFree)
{
	MDX_ACTOR *a = *toFree;

	if (a->objectController)
		FreeUniqueObject(a->objectController->object);

	FreeMem (a->objectController);

	if (a->next) a->next->prev = a->prev;
	
	if (a->prev)
		a->prev->next = a->next;
	else
		actorList = a->next;


	if (a->nextDraw) a->nextDraw->prevDraw = a->prevDraw;
	
	if (a->prevDraw)
		a->prevDraw->nextDraw = a->nextDraw;
	else
		actorDrawList[a->listNum] = a->nextDraw;


	FreeMem (a->animation);

	//MC - 0xcdcdcdcd - if( a->shadow ) delete a->shadow;
	FreeMem (a);

	*toFree = 0;
}


/*  --------------------------------------------------------------------------------

	Function	: MakeUniqueObject
	Purpose		: Make an object into a unique copy of itself
	Parameters	: (MDX_OBJECT *object)
	Returns		: MDX_OBJECT *
	Info		: Make a unique copy of all pointers
*/

MDX_OBJECT *MakeUniqueObject(MDX_OBJECT *object)
{
	MDX_OBJECT	*obj;	
	MDX_OBJECTSPRITE **spr, *tempSpr;
		
	obj = object;
	object = (MDX_OBJECT *) AllocMem(sizeof(MDX_OBJECT));
	memcpy(object, obj, sizeof(MDX_OBJECT));

	if(obj->numSprites)
	{
		spr = &object->sprites;
		tempSpr = (MDX_OBJECTSPRITE *) AllocMem(sizeof(MDX_OBJECTSPRITE)*obj->numSprites);		
		memcpy(tempSpr, *spr, sizeof(MDX_OBJECTSPRITE) * obj->numSprites);
		*spr = tempSpr;
	}

	if(object->children)
		object->children = MakeUniqueObject(object->children);

	if(object->next)
		object->next = MakeUniqueObject(object->next);
		
	return object;
}


/*  --------------------------------------------------------------------------------

	Function	: MakeUniqueActor
	Purpose		: Simmilar to above
	Parameters	: (MDX_ACTOR *actor,int type)
	Returns		: void 
	Info		: but on Actor level
*/

void MakeUniqueActor(MDX_ACTOR *actor,int type)
{
	MDX_OBJECT_CONTROLLER *objCont;
	short	unique = TRUE;
	short	i;
	int		CRC;
	
	if(!actor->objectController)
		return;
		
	CRC = UpdateCRC((char *)actor->objectController->object->name);

	//check all crc's to see if actor is among them
	for(i = 0; i < numActorsUniqe; i++)
	{
		if(uniqueActorCRC[i] == CRC)
		{
			//if it is, actor is not unique and must have seperate stuff
			//dprintf"found duplicate actor %s\n", actor->objectController->object->name));
			unique = FALSE;
			break;
		}
	}
	
	//if actor is not in list
	if(unique == TRUE)
	{
		uniqueActorCRC[numActorsUniqe++] = CRC;
	}	

	objCont = actor->objectController;
	actor->objectController = (MDX_OBJECT_CONTROLLER *) AllocMem(sizeof(MDX_OBJECT_CONTROLLER));
	memcpy(actor->objectController, objCont, sizeof(MDX_OBJECT_CONTROLLER));
	actor->objectController->object = MakeUniqueObject(actor->objectController->object);	
}

float maxX,minX;
float maxY,minY;
float maxZ,minZ;

#define BIGVAL 10000.0
#define RADIUS_SCALE 0.95

/*  --------------------------------------------------------------------------------

	Function	: FindHierarchyCentre
	Purpose		: Guess
	Parameters	: (MDX_OBJECT *me)
	Returns		: void 
	Info		:
*/

void FindHierarchyCentre(MDX_OBJECT *me)
{
	int i;
	MDX_VECTOR t;

	if (me)
	{
		if (me->mesh)
			for (i=0; i<me->mesh->numVertices; i++)
			{
				guMtxXFMF(me->objMatrix.matrix,
							me->mesh->vertices[i].vx,
							me->mesh->vertices[i].vy,
							me->mesh->vertices[i].vz,
							&(t.vx),&(t.vy),&(t.vz));

				maxX = (t.vx > maxX)?t.vx:maxX;	maxY = (t.vy > maxY)?t.vy:maxY;	maxZ = (t.vz > maxZ)?t.vz:maxZ;
				minX = (t.vx < minX)?t.vx:minX;	minY = (t.vy < minY)?t.vy:minY;	minZ = (t.vz < minZ)?t.vz:minZ;								
			}

		if (me->children)
			FindHierarchyCentre(me->children);
		if (me->next)
			FindHierarchyCentre(me->next);
	}
}


/*  --------------------------------------------------------------------------------

	Function	: CalculateTrueCentreAndRadius
	Purpose		: Gets the actual centre of an object (On frame 0) 
	Parameters	: (MDX_ACTOR *t)
	Returns		: void 
	Info		: Facility for getting radius from centrepoint is commented out.
*/

void FindActorCentre(MDX_ACTOR *t)
{
	MDX_VECTOR radius;
	maxX = maxY = maxZ = -BIGVAL;
	minX = minY = minZ = BIGVAL;

	FindHierarchyCentre(t->objectController->object);


	t->trueCentre.vx = (minX+maxX)/2;
	t->trueCentre.vy = (minY+maxY)/2;
	t->trueCentre.vz = (minZ+maxZ)/2;

	// Make the radius, but make it slightly smaller so we draw less (Cheat)
	
	// MC - Based on calculated "true" centrepoint

	radius.vx = maxX-t->trueCentre.vx;
	radius.vy = maxY-t->trueCentre.vy;
	radius.vz = maxZ-t->trueCentre.vz;
	
	/*
	// MC - Based on object centrepoint
	radius.vx = ((-minX)>(maxX))?-minX:maxX;
	radius.vy = ((-minY)>(maxY))?-minY:maxY;
	radius.vz = ((-minZ)>(maxZ))?-minZ:maxZ;
	*/

	t->radius = mdxMagnitude(&radius) * RADIUS_SCALE;
}


/*  --------------------------------------------------------------------------------

	Function	: CreateActor
	Purpose		: Create a new actor
	Parameters	: (char *name, unsigned long flags)
	Returns		: MDX_ACTOR *
	Info		:
*/

MDX_ACTOR *CreateActor(char *name, unsigned long flags)
{
	MDX_ACTOR *t;
	t = (MDX_ACTOR *) AllocMem(sizeof(MDX_ACTOR));
	
	InitActor(t,name,0,0,0,flags);

	if (!t->objectController)
	{
		FreeMem (t);
		return NULL;
	}

	MakeUniqueActor(t,0);
	
	//Initial position for radius calculation
	t->pos.vx = t->pos.vy = t->pos.vz = 0;
	t->scale.vx = t->scale.vy = t->scale.vz = 1;
	t->qRot.x = t->qRot.y = t->qRot.z = 0;
	t->qRot.w = 1;

	// Transform object
	XformActor(t,1);
	
	t->flags = ACTOR_NOFLAGS;
		
	FindActorCentre(t);

	return t;
}


/*  --------------------------------------------------------------------------------

	Function	: FindObj
	Purpose		: Find a sub object with a given name in a hierachy
	Parameters	: (MDX_OBJECT *me, char *name)
	Returns		: MDX_OBJECT *
	Info		: Internal, called by find actor subobject, recursive
*/

MDX_OBJECT *FindObj(MDX_OBJECT *me, char *name)
{
	MDX_OBJECT *t = NULL;
	
	if (stricmp((const char *)me->name,name)==0)
		return me;

	if( me->next && (t = FindObj(me->next,name)) )
		return t;
	if( me->children && (t = FindObj(me->children,name)) )
		return t;

	return t;
}


/*  --------------------------------------------------------------------------------

	Function	: FindActorSubObject
	Purpose		: Find a sub object with a given name in a hierachy
	Parameters	: (MDX_ACTOR *me, char *subName)
	Returns		: MDX_OBJECT *
	Info		: 
*/

MDX_OBJECT *FindActorSubObject(MDX_ACTOR *me, char *subName)
{	
	return FindObj(me->objectController->object,subName);
}


/*  --------------------------------------------------------------------------------

	Function	: AttachActorToSubobject
	Purpose		: Attach an actors centrepoint to a specific sub-object in the hierachy - ie, hats on frogs or guns on hands.
	Parameters	: (MDX_OBJECT *dest, MDX_ACTOR *src)
	Returns		: void 
	Info		:
*/

void AttachActorToSubobject(MDX_OBJECT *dest, MDX_ACTOR *src)
{
	dest->attachedActor = src;
}

#ifdef __cplusplus
}
#endif
