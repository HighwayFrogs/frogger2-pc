/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: platform.c
	Programmer	: Andrew Eder
	Date		: 1/21/99

	NOTES FOR PLATFORM USAGE: (or the platform bible according to Sharky) (_|_) <- ass

	Examples of simple platform usage....

	Platform that...						Flags
	--------------------------------------------------------------------------

	moves up and down on the spot			PLATFORM_NEW_MOVEUP | PLATFORM_NEW_PINGPONG
											PLATFORM_NEW_MOVEDOWN | PLATFORM_NEW_PINGPONG

	moves up and down on the spot and		PLATFORM_NEW_MOVEUP | PLATFORM_NEW_PINGPONG | PLATFORM_NEW_NOWALKUNDER
	cannot be walked under (e.g. solid		PLATFORM_NEW_MOVEDOWN | PLATFORM_NEW_PINGPONG | PLATFORM_NEW_NOWALKUNDER
	stone pillar)

	moves up on the spot (rises) when a		PLATFORM_NEW_STEPONACTIVATED | PLATFORM_NEW_MOVEUP
	frog is on it, and returns to start		
	position otherwise

	moves down on the spot (sinks) when a	PLATFORM_NEW_STEPONACTIVATED | PLATFORM_NEW_MOVEDOWN
	frog is on it, and returns to start
	position otherwise

	moves along a path (>1 node) and		PLATFORM_NEW_FORWARDS
	returns immediately to start of path
	when the last node has been reached

	moves along a path (>1 node) and		PLATFORM_NEW_FORWARDS | PLATFORM_NEW_CYCLE
	continues to cycle through all nodes	PLATFORM_NEW_BACKWARDS | PLATFORM_NEW_CYCLE
	in a 'closed' path fashion

	moves back and forth along a path		PLATFORM_NEW_FORWARDS | PLATFORM_NEW_PINGPONG
	(>1 node) in a ping-pong movement.		PLATFORM_NEW_BACKWARDS | PLATFORM_NEW_PINGPONG

	is a non-moving platform (single node)	PLATFORM_NEW_NONMOVING


----------------------------------------------------------------------------------------------- */

#include "ultra64.h"
#include "platform.h"
#include "frogmove.h"
#include "anim.h"
#include "map.h"
#include "collect.h"
#include "game.h"
#include "newpsx.h"
#include <islmem.h>
#include "Main.h"
#include "maths.h"
#include "layout.h"
#include <islutil.h>

#ifdef PSX_VERSION
#include "audio.h"
#include "timer.h"
#else
#include <pcaudio.h>
#endif


void CalcNextPlatformDest(PLATFORM *plat);


//----- [ PLATFORM UPDATE FUNCTIONS ] ------------------------------------------------------------

void UpdatePathPlatform(PLATFORM *);
void UpdatePathUpDownPlatform(PLATFORM *);
void UpdateUpDownPlatform(PLATFORM *);
//void UpdateNonMovingPlatform(PLATFORM *);
void UpdateStepOnActivatedPlatform(PLATFORM *);
//void UpdateSlerpPathPlatform(PLATFORM *);


/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
PLATFORM *GetPlatformFromUID(long uid)
{
	PLATFORM *cur;

	for(cur = platformList.head.next; cur != &platformList.head; cur = cur->next)
		if (cur->uid == uid)
			return cur;

	//dprintf"GetPlatformFromUID: Couldn't find platform %d\n", uid));
	return NULL;
}


/*	--------------------------------------------------------------------------------
	Function		: UpdatePlatforms
	Purpose			: updates all platforms and their states / movements, etc.
	Parameters		: 
	Returns			: void
	Info			: 
*/

extern fixed waitScale;
void UpdatePlatforms()
{
	PLATFORM *cur,*next;
	int j;

	
	if(platformList.count == 0)
		return;

	for(cur = platformList.head.next; cur != &platformList.head; cur = next)
	{
		ACTOR2 *act2 = cur->pltActor;
		ACTOR  *act;
		PATH  *path = cur->path;

		next = cur->next;

		// check if platform is active
		if(!cur->active)
		{
			// update regenerating platforms
			if(cur->flags & PLATFORM_NEW_REGENERATES)
			{
				if((cur->countdown -= gameSpeed) < 0)
				{
					//cur->pltActor->actor->xluOverride = 100;
					act2->draw = 1;
					cur->active	= 1;
					cur->countdown = -4096;
				}
				else continue;
			}
			else continue;
		}

		// If enemy has an actor2 and it's active and not miles away
		if( act2 && act2->distanceFromFrog < (ACTOR_DRAWDISTANCEOUTER<<2) )
			act = act2->actor;
		else
			continue;

		cur->visible = (act2->draw && !act2->clipped);

		// call update function for individual platform type
		if(cur->Update && !cur->isWaiting)
			cur->Update(cur);

		// update frog if on the current platform
		if(cur->carrying)
		{
			SetVectorSS(&cur->carrying->actor->position,&act->position);

			if( gameState.multi != SINGLEPLAYER )
			{
				int i;
				for( i=0; i<NUM_FROGS; i++ )
					if( frog[i] == cur->carrying ) break;

				if( i<NUM_FROGS )
				{
					j = player[i].frogunder;
					while(j != -1)
					{
						if( !player[j].isSuperHopping )
						{
							SetVectorSS( &frog[j]->actor->position, &cur->carrying->actor->position );
						}

						j = player[j].frogunder;
					}
					j = player[i].frogon;
					while(j != -1)
					{
						SetVectorSS( &frog[j]->actor->position, &cur->carrying->actor->position );
						j = player[j].frogon;
					}
//					if( player[i].frogunder != -1 )
//						SetVectorSS( &frog[player[i].frogunder]->actor->position, &cur->carrying->actor->position );
//					if( player[i].frogon != -1 )
//						SetVectorSS( &frog[player[i].frogon]->actor->position, &cur->carrying->actor->position );
				}
			}
		}

		// check if this platform is currently 'waiting' at a node
		if(cur->isWaiting)
		{
			if (((cur->isWaiting != -1) && (actFrameCount > path->startFrame)) ||
				(cur->flags & PLATFORM_NEW_STEPONACTIVATED && cur->carrying))
				cur->isWaiting = 0;
			else
				continue;
		}

		// check if this is a disappearing or crumbling platform
		if(cur->flags & (PLATFORM_NEW_DISAPPEARWITHFROG| PLATFORM_NEW_CRUMBLES))
		{
			if (cur->countdown < 0)
			{
				if (cur->flags & PLATFORM_NEW_CARRYINGFROG)
					cur->countdown = path->nodes->speed;
			}
			else	// we're counting down...
			{
				// Wibble platform
				GetPositionForPathNode(&act->position, &path->nodes[0]);
				act->position.vx += (-24 + Random(50));
				act->position.vy += (-24 + Random(50));
				act->position.vz += (-24 + Random(50));

				cur->countdown -= gameSpeed;

				if(cur->countdown <= 0)	// .. timer has reached zero; collapse
				{
					int i;

					if (cur->flags & PLATFORM_NEW_CARRYINGFROG)
					{
						// What frog are we carrying?


						for(i = 0;i < NUM_FROGS;i++)
						{
							if(currPlatform[i] == cur)
							{
								long t = FsqrtF(path->nodes[0].offset>>2)>>12;

								currTile[i] = destTile[i] = cur->inTile[0];
								currPlatform[i] = NULL;
								player[i].frogState |= FROGSTATUS_ISJUMPINGTOTILE;
								cur->flags &= ~PLATFORM_NEW_CARRYINGFROG;
								cur->carrying = NULL;

								//utilPrintf("And Maybe Here : %d\n", pl); - wtf? - ds

								if (!(player[i].frogState & FROGSTATUS_ISDEAD))
									CalculateFrogJumpS(&frog[i]->actor->position, &destTile[i]->centre, &destTile[i]->normal, 0,
										t+1, i);
							}
						}
/*						
						for (i = 0, pl = -1; i<NUM_FROGS; i++)
							if (frog[i] == cur->carrying)
							{
								pl = i; break;
							}

						if (pl != -1)
						{
							long t = FsqrtF(path->nodes[0].offset>>2)>>12;

							currTile[pl] = destTile[pl] = cur->inTile[0];
							currPlatform[pl] = NULL;
							player[pl].frogState |= FROGSTATUS_ISJUMPINGTOTILE;
							cur->flags &= ~PLATFORM_NEW_CARRYINGFROG;
							cur->carrying = NULL;

							//utilPrintf("And Maybe Here : %d\n", pl); - wtf? - ds

							if (!(player[pl].frogState & FROGSTATUS_ISDEAD))
								CalculateFrogJumpS(&frog[pl]->actor->position, &destTile[pl]->centre, &destTile[pl]->normal, 0,
									t+1, pl);
						}
*/
					}

					//cur->pltActor->actor->xluOverride = 0;
					cur->pltActor->draw = 0;
					cur->active = 0;
					cur->carrying = NULL;
					cur->countdown = ToFixed(cur->path->nodes->waitTime);
					//Matt E3bodge
					if (cur->countdown<-5000)
						cur->countdown = 245760;

					PlaySample( path->nodes->sample, &act->position, 0, SAMPLE_VOLUME, -1 );

					if(cur->flags & PLATFORM_NEW_CRUMBLES)
					{
						FVECTOR n;
						SetVectorFF(&n, &cur->inTile[0]->normal);
						CreateSpecialEffect(FXTYPE_SMOKEBURST, &act->position, &n, 204800, 8192, 4096, 4096 );
						CreateSpecialEffect(FXTYPE_SMOKEBURST, &act->position, &n, 163840, 4096, 4096, 6144 );
					}
				}
			}
		}
		else
		{
			// platform is not carrying a frog
			if(cur->countdown < path->nodes->speed)
				cur->countdown += gameSpeed;
		}

		// special fx associated with platform
		if(cur->pltActor->effects && !act2->clipped )
			ProcessAttachedEffects((void *)cur,ENTITY_PLATFORM);
	}
}

/*	--------------------------------------------------------------------------------
	Function		: ResetPlatformFlags
	Purpose			: resets platform flags
	Parameters		: 
	Returns			: void
	Info			: 
*/
void ResetPlatformFlags()
{
	PLATFORM *cur,*next;

	if(platformList.count == 0)
		return;

	for(cur = platformList.head.next; cur != &platformList.head; cur = next)
	{
		next = cur->next;

		cur->flags		&= ~PLATFORM_NEW_CARRYINGFROG;
		cur->carrying	= NULL;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: InitPlatformLinkedList
	Purpose			: initialises the platform linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void InitPlatformLinkedList()
{
	platformList.array = NULL;
	platformList.count = 0;
	platformList.head.next = platformList.head.prev = &platformList.head;
}

/*	--------------------------------------------------------------------------------
	Function		: AddPlatform
	Purpose			: adds a platform to the linked list
	Parameters		: PLATFORM *
	Returns			: void
	Info			: 
*/
void AddPlatform(PLATFORM *plat)
{
	PLATFORM *cur;

	if( !plat || plat->next )
		return;

	// Try to find another of the same type as the new plat
	for( cur = platformList.head.next; cur != &platformList.head; cur = cur->next )
		if( cur->Update == plat->Update )
		{
			// If update funcs the same, add before existing one
			plat->prev = cur->prev;
			plat->next = cur;
			cur->prev->next = plat;
			cur->prev = plat;
			break;
		}

	// If plat is the first of its kind, add at head of list
	if( !plat->next )
	{
		plat->prev = &platformList.head;
		plat->next = platformList.head.next;
		platformList.head.next->prev = plat;
		platformList.head.next = plat;
	}

	platformList.count++;
}

/*	--------------------------------------------------------------------------------
	Function		: SubPlatform
	Purpose			: removes a platform from the linked list
	Parameters		: PLATFORM *
	Returns			: void
	Info			: 
*/
void SubPlatform(PLATFORM *plat)
{
	if(plat->next == NULL)
		return;

	if(plat->pltActor) 
		actor2Free(plat->pltActor);

	plat->prev->next = plat->next;
	plat->next->prev = plat->prev;
	plat->next = NULL;
	platformList.count--;

	if(plat->path)
		FREE(plat->path);

//	FREE(plat);
}


/*	--------------------------------------------------------------------------------
	Function		: FreePlatformLinkedList
	Purpose			: frees the platform linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void FreePlatformLinkedList()
{
	if (platformList.count)
		utilPrintf("Freeing linked list : PLATFORM : (%d elements)\n",platformList.count);

	// free elements
	while(platformList.head.next && platformList.head.next != &platformList.head )
		SubPlatform(platformList.head.next);

	FreePlatformBlock( );

	// initialise list for future use
	InitPlatformLinkedList();
}

/*	--------------------------------------------------------------------------------
	Function		: AllocNmeBlock
	Purpose			: Initialise a fixed number of enemies
	Parameters		: 
	Returns			: void
	Info			: 
*/
void AllocPlatformBlock( int num )
{
	if( !platformList.array )
		platformList.array = (PLATFORM *)MALLOC0(sizeof(PLATFORM)*num);

	platformList.count = 0;
	platformList.blocks = num;
}

/*	--------------------------------------------------------------------------------
	Function		: FreeNmeBlock
	Purpose			: Free statically alloced enemies
	Parameters		: 
	Returns			: void
	Info			: 
*/
void FreePlatformBlock( )
{
	if( platformList.array )
		FREE( platformList.array );

	platformList.array = NULL;
	platformList.count = 0;
	platformList.blocks = 0;
}


PLATFORM *CreateAndAddPlatform(char *pActorName,int flags,long ID,PATH *path,fixed animSpeed,unsigned char facing)
{
	ACTOR2 *actor;
	PLATFORM *newItem;
	int initFlags,i;
	fixed shadowRadius = 0;

	initFlags = INIT_ANIMATION;

	if( flags & PLATFORM_NEW_SHADOW )
	{
		initFlags |= INIT_SHADOW;
		shadowRadius = 819200;
	}

	// create and add platform actor
	if( !(actor = CreateAndAddActor(pActorName,0,0,0,initFlags)) )
	{
		return NULL;
	}

	
	if( platformList.count >= platformList.blocks || !(newItem = &platformList.array[platformList.count]) )
		return NULL;
//	newItem = (PLATFORM *)MALLOC0(sizeof(PLATFORM));

	newItem->pltActor = actor;
	newItem->flags = flags;
	newItem->facing = facing;

	newItem->pltActor->animSpeed = animSpeed;

	actorAnimate(newItem->pltActor->actor,0,YES,NO,newItem->pltActor->animSpeed,0);

	// set shadow radius (if applicable)
 	if(shadowRadius)//mmshadow
 		if(newItem->pltActor->actor->shadow)//mmshadow
 			newItem->pltActor->actor->shadow->radius = shadowRadius;//mmshadow

	// currently set all surrounding platform ptrs to null
	for(i=0; i<4; i++)
		newItem->pltPtrs[i] = NULL;

	// set the platform to be active (i.e. is updated)
	newItem->active			= 1;
	newItem->path			= NULL;
	newItem->inTile[0]		= NULL;
	newItem->inTile[1]		= NULL;
	newItem->uid			= ID;

	newItem->currSpeed		= 4096;

	// set this platform to be carrying nowt
	newItem->carrying		= NULL;

	AssignPathToPlatform(newItem,path,0);

	if (newItem->flags & PLATFORM_NEW_STEPONACTIVATED)
	{
		if (newItem->flags & PLATFORM_NEW_MOVEUP)
		{
			newItem->flags &= ~PLATFORM_NEW_MOVEUP;
			newItem->flags |= PLATFORM_NEW_RISEWITHFROG;
		}
		else if (newItem->flags & PLATFORM_NEW_MOVEDOWN)
		{
			newItem->flags &= ~PLATFORM_NEW_MOVEDOWN;
			newItem->flags |= PLATFORM_NEW_SINKWITHFROG;
		}
	}

	// determine relevant platform update function
/*	if (newItem->flags & PLATFORM_NEW_SLERPPATH)
		newItem->Update = UpdateSlerpPathPlatform;
	else */
	if(newItem->flags & PLATFORM_NEW_FOLLOWPATH)
	{
		newItem->Update = UpdatePathPlatform;
		OrientateSS( &newItem->pltActor->actor->qRot, &path->nodes->worldTile->dirVector[newItem->facing], &path->nodes->worldTile->normal );
		SetQuaternion( &newItem->destOrientation, &newItem->pltActor->actor->qRot );
	}
	else if(newItem->flags & (PLATFORM_NEW_MOVEUP | PLATFORM_NEW_MOVEDOWN))
	{
		newItem->Update = UpdateUpDownPlatform;
		OrientateSS( &newItem->pltActor->actor->qRot, &newItem->path->nodes->worldTile->dirVector[newItem->facing], &newItem->path->nodes->worldTile->normal );
	}
	else if(newItem->flags & PLATFORM_NEW_STEPONACTIVATED)
	{
		newItem->Update = UpdateStepOnActivatedPlatform;
		OrientateSS( &newItem->pltActor->actor->qRot, &newItem->path->nodes->worldTile->dirVector[newItem->facing], &newItem->path->nodes->worldTile->normal );
	}
	else if( (newItem->flags & PLATFORM_NEW_NONMOVING) || (newItem->pltActor->actor && newItem->path && newItem->path->nodes) )
		OrientateSS( &newItem->pltActor->actor->qRot, &newItem->path->nodes->worldTile->dirVector[newItem->facing], &newItem->path->nodes->worldTile->normal );

	AddPlatform(newItem);
	return newItem;
}

void AssignPathToPlatform(PLATFORM *pform,PATH *path,unsigned long pathFlags)
{
	int i;
	SVECTOR platformStartPos;
	FVECTOR fwd;

	pform->path	= path;

	// check if pathnode indices need converting to game tile pointers
	if(pathFlags & PATH_MAKENODETILEPTRS)
		for(i=0; i<path->numNodes; i++)
			pform->path->nodes[i].worldTile = &firstTile[(unsigned long)path->nodes[i].worldTile];

	// set the start position for the platform
	pform->path->fromNode = pform->path->startNode;
	
	if(pform->flags & PLATFORM_NEW_FORWARDS)
	{
		// this platform moves forward thru path nodes
		pform->flags |= PLATFORM_NEW_FOLLOWPATH;
		pform->path->toNode = pform->path->fromNode + 1;
		
		if(pform->path->toNode > GET_PATHLASTNODE(path))
			pform->path->toNode = 0;

		if( (pform->flags & PLATFORM_NEW_FACEFORWARDS) && CheckPathOrientation(path) )
		{
			pform->flags |= PLATFORM_NEW_NOREORIENTATE;
#ifdef PSX_VERSION
			pform->pltActor->actor->flags |= PLATFORM_NEW_NOREORIENTATE;
#endif
		}
	}
	else if(pform->flags & PLATFORM_NEW_BACKWARDS)
	{
		// this platform moves backward thru path nodes
		pform->flags |= PLATFORM_NEW_FOLLOWPATH;
		pform->path->toNode = pform->path->fromNode - 1;
		
		if(pform->path->toNode < 0)
			pform->path->fromNode = GET_PATHLASTNODE(path);

		if( (pform->flags & PLATFORM_NEW_FACEFORWARDS) && CheckPathOrientation(path) )
		{
			pform->flags |= PLATFORM_NEW_NOREORIENTATE;
#ifdef PSX_VERSION
			pform->pltActor->actor->flags |= PLATFORM_NEW_NOREORIENTATE;
#endif
		}
	}
	else if((pform->flags & PLATFORM_NEW_MOVEUP) ||
			(pform->flags & PLATFORM_NEW_MOVEDOWN))
	{
		// this platform moves up or down
		pform->path->fromNode = pform->path->toNode = 0;
	}
	else
	{
		// this platform does not move - why use this at all ever?
//		pform->flags |= PLATFORM_NEW_NONMOVING;
		pform->path->fromNode = pform->path->toNode = 0;
	}

	// set platform position to relevant point on path
	if (pform->flags & PLATFORM_NEW_MOVEDOWN)
		GetPositionForPathNodeOffset2(&platformStartPos,&path->nodes[pform->path->fromNode]);
	else
		GetPositionForPathNode(&platformStartPos,&path->nodes[pform->path->fromNode]);

	SetVectorSS(&pform->pltActor->actor->position,&platformStartPos);

	SubVectorFSS(&fwd,
		&pform->path->nodes[pform->path->toNode].worldTile->centre,
		&pform->path->nodes[pform->path->fromNode].worldTile->centre);
	MakeUnit(&fwd);

	//if( !(pform->flags & PLATFORM_NEW_NOREORIENTATE) )
	//{
		if (!(pform->flags & PLATFORM_NEW_FACEFORWARDS))
			OrientateFS(&pform->pltActor->actor->qRot, &fwd, &pform->path->nodes[pform->path->fromNode].worldTile->normal);
		else
			OrientateSS( &pform->pltActor->actor->qRot, &pform->path->nodes->worldTile->dirVector[pform->facing], &pform->path->nodes->worldTile->normal );
	//}

	// set platform current 'in' tile and speeds and pause times
	pform->inTile[0]	= path->nodes[pform->path->fromNode].worldTile;
	pform->currSpeed	= path->nodes[pform->path->fromNode].speed;
	pform->isWaiting	= path->nodes[pform->path->fromNode].waitTime;

	pform->path->startFrame = actFrameCount;
	pform->path->endFrame = ( actFrameCount+ ((pform->currSpeed*60)>>12) );
	pform->countdown = ToFixed(-1);

	CalcPlatformNormalInterps(pform);
}


/*	--------------------------------------------------------------------------------
	Function		: UpdatePlatformPathNodes
	Purpose			: updates platform path move status
	Parameters		: PLATFORM *
	Returns			: void
	Info			: 
*/
void UpdatePlatformPathNodes(PLATFORM *pform)
{
	SVECTOR pformPos;

	ACTOR			*act = pform->pltActor->actor;

	PATH			*path = pform->path;
	PATHNODE *nodes = pform->path->nodes;

//	unsigned long flags = pform->flags;

	path->fromNode = path->toNode;

	CalcNextPlatformDest(pform);

	pform->currSpeed = nodes[path->fromNode].speed;
	pform->isWaiting = nodes[path->fromNode].waitTime;

	// Stop overshoot when waiting on a path node
	if (pform->isWaiting)
	{
		if (pform->flags & PLATFORM_NEW_MOVEDOWN)
			GetPositionForPathNodeOffset2(&pformPos, &nodes[path->fromNode]);
		else
			GetPositionForPathNode(&pformPos, &nodes[path->fromNode]);

		SetVectorSS(&act->position, &pformPos);
	}

	CalcPlatformNormalInterps(pform);
}


/*	--------------------------------------------------------------------------------
	Function		: CalcPlatformNormalInterps
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
void CalcPlatformNormalInterps(PLATFORM *pform)
{
	PATH *path;
	PATHNODE *fromNode,*toNode;
//	fixed numSteps;
	FVECTOR fwd;
//	IQUATERNION tempQuat;

	path = pform->path;
	if(path->numNodes < 2)
		return;

	fromNode	= &path->nodes[path->fromNode];
	toNode		= &path->nodes[path->toNode];

	if( pform->visible && !(pform->flags & PLATFORM_NEW_NOREORIENTATE) )
	{
		pform->srcOrientation = pform->pltActor->actor->qRot;
		if(!(pform->flags & PLATFORM_NEW_FACEFORWARDS))
		{
			SubVectorFSS(&fwd, &toNode->worldTile->centre, &fromNode->worldTile->centre);
			MakeUnit(&fwd);

 			OrientateFS(&pform->destOrientation, &fwd, &toNode->worldTile->normal);
		}
		else
		{
			OrientateSS( &pform->destOrientation, &pform->path->nodes->worldTile->dirVector[pform->facing], &toNode->worldTile->normal );
		}
	}
}

/*	--------------------------------------------------------------------------------
	Function		: CalcNextPlatformDest
	Purpose			: Figures out what tile to go to next, on a path
	Parameters		: PLATFORM *
	Returns			: void
	Info			: 
*/
void CalcNextPlatformDest(PLATFORM *pform)
{
	PATH *path = pform->path;
	unsigned long flags = pform->flags;

	if(pform->flags & PLATFORM_NEW_FORWARDS)	// platform moves forward through path nodes
	{
		path->toNode = path->fromNode + 1;

		if(path->toNode > GET_PATHLASTNODE(path))
		{
			if(flags & PLATFORM_NEW_PINGPONG)
			{
				pform->flags	^= (PLATFORM_NEW_FORWARDS | PLATFORM_NEW_BACKWARDS);
				path->fromNode	= GET_PATHLASTNODE(path);
				path->toNode	= GET_PATHLASTNODE(path) - 1;
			}
			else if(flags & PLATFORM_NEW_CYCLE)
			{
				// platform has cyclic movement
				path->fromNode	= GET_PATHLASTNODE(path);
				path->toNode	= 0;
			}
			else
			{
				path->fromNode	= 0;
				path->toNode	= 1;
			}
		}
		if( path->nodes[path->fromNode].sample )
			PlaySample( path->nodes[path->fromNode].sample, &pform->pltActor->actor->position, 0, SAMPLE_VOLUME, -1 );
	}
	else if(flags & PLATFORM_NEW_BACKWARDS)
	{
		// platform moves backwards through path nodes
		path->toNode = path->fromNode - 1;

		if(path->toNode < 0)
		{
			if(flags & PLATFORM_NEW_PINGPONG)
			{
				pform->flags	^= (PLATFORM_NEW_FORWARDS | PLATFORM_NEW_BACKWARDS);
				path->fromNode	= 0;
				path->toNode	= 1;
			}
			else if(flags & PLATFORM_NEW_CYCLE)
			{
				path->fromNode	= 0;
				path->toNode	= GET_PATHLASTNODE(path);
			}
			else
			{
				path->fromNode	= GET_PATHLASTNODE(path);
				path->toNode	= GET_PATHLASTNODE(path) - 1;
			}
		}
		if( path->nodes[path->fromNode].sample )
			PlaySample( path->nodes[path->fromNode].sample, &pform->pltActor->actor->position, 0, SAMPLE_VOLUME, -1 );
	}
	else if( flags & (PLATFORM_NEW_MOVEUP | PLATFORM_NEW_MOVEDOWN) )
	{
		if( flags & PLATFORM_NEW_PINGPONG )
		{
			pform->flags	^= (PLATFORM_NEW_MOVEUP | PLATFORM_NEW_MOVEDOWN);	// invert flags
			if( path->nodes->sample )
				PlaySample( path->nodes->sample, &pform->pltActor->actor->position, 0, SAMPLE_VOLUME, -1 );
		}
	}
}




void FrogLeavePlatform(long pl)
{
	PLATFORM *plt;

	if ((plt = currPlatform[pl]))
	{
		plt->flags &= ~PLATFORM_NEW_CARRYINGFROG;	// plat is NOT carrying a frog
		plt->carrying = NULL;
		currPlatform[pl] = NULL;
	}
}


//----- [ PLATFORM UPDATE FUNCTIONS ] ------------------------------------------------------------


/*	--------------------------------------------------------------------------------
	Function		: UpdatePathPlatform
	Purpose			: updates platforms that follow a path
	Parameters		: PLATFORM *
	Returns			: void
	Info			: 
*/
void UpdatePathPlatform(PLATFORM *plat)
{
	SVECTOR fromPosition,toPosition;
	FVECTOR fwd;
	fixed length;

	ACTOR *act = plat->pltActor->actor;

	PATH *path = plat->path;
	PATHNODE *nodes = plat->path->nodes;
	// check if this platform has arrived at a path node
	if(actFrameCount >= plat->path->endFrame)
	{
		do {
			UpdatePlatformPathNodes(plat);
			plat->path->startFrame = plat->path->endFrame + ((plat->isWaiting * waitScale) >> 12);
			plat->path->endFrame = plat->path->startFrame + ((60*plat->currSpeed) >> 12);
		}
		while (actFrameCount >= plat->path->endFrame && plat->isWaiting >= 0);

		if (plat->isWaiting) return;
	}
	else if (actFrameCount > ((plat->path->startFrame + plat->path->endFrame) / 2))
	{
		plat->inTile[0] = plat->path->nodes[plat->path->toNode].worldTile;
	}

	// update the platform position
	GetPositionForPathNode(&toPosition,&nodes[path->toNode]);
	GetPositionForPathNode(&fromPosition,&nodes[path->fromNode]);
	
	length = ((actFrameCount-path->startFrame)<<12) / (path->endFrame-path->startFrame);

	if( plat->visible && !(plat->flags & PLATFORM_NEW_NOREORIENTATE) )
	{
		if( !(plat->flags & PLATFORM_NEW_SLERPPATH) )
			SetQuaternion(&act->qRot,&plat->destOrientation);
		else // mainly for space2 platforms
			IQuatSlerp( &plat->srcOrientation, &plat->destOrientation, length, &plat->pltActor->actor->qRot );
	}

	SubVectorFSS(&fwd,&toPosition,&fromPosition);
	
 	ScaleVectorFF(&fwd,length);
	RoundVectorRandF(&fwd);
 	AddVectorSFS(&act->position,&fwd,&fromPosition);
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateUpDownPlatform
	Purpose			: updates platforms that rise and sink
	Parameters		: PLATFORM *
	Returns			: void
	Info			: 
*/
void UpdateUpDownPlatform(PLATFORM *plat)
{
	FVECTOR moveVec;
	fixed start_offset, end_offset, t;
	int i;

	// check if this platform has arrived at a path node
	if( actFrameCount > plat->path->endFrame )
	{
		do {
			UpdatePlatformPathNodes(plat);

			plat->path->startFrame = plat->path->endFrame + ((plat->isWaiting*waitScale)>>12);
			plat->path->endFrame = plat->path->startFrame + ((plat->currSpeed*60)>>12);
		} while (actFrameCount > plat->path->endFrame);

		for( i=0; i<NUM_FROGS; i++ )
			if( plat == currPlatform[i] )
				CheckTileForCollectable( currTile[i], i );

		if (plat->isWaiting) return;
	}

	if (plat->flags & PLATFORM_NEW_MOVEUP)
	{
		start_offset = plat->path->nodes->offset;
		end_offset = plat->path->nodes->offset2;
	}
	else
	{
		start_offset = plat->path->nodes->offset2;
		end_offset = plat->path->nodes->offset;
	}

	// get up vector for this platform
	SetVectorFF(&moveVec,&plat->path->nodes[0].worldTile->normal);
	
	t = ToFixed(actFrameCount-plat->path->startFrame) / (plat->path->endFrame-plat->path->startFrame);

	ScaleVectorFF( &moveVec, FMul(t,end_offset) + FMul(ToFixed(1)-t,start_offset) );
	RoundVectorRandF(&moveVec);
	AddVectorSSF(&plat->pltActor->actor->position, &plat->path->nodes->worldTile->centre, &moveVec);
}




/*	--------------------------------------------------------------------------------
	Function		: UpdateStepOnActivatedPlatform
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
void UpdateStepOnActivatedPlatform(PLATFORM *plat)
{
	FVECTOR moveVec;
	FVECTOR v;
	fixed offs, startOffs, destOffs, speed;

	startOffs = plat->path->nodes->offset;
	destOffs = plat->path->nodes->offset2;

	// get up vector for this platform
	SetVectorFF(&moveVec,&plat->path->nodes[0].worldTile->normal);

	SubVectorFSS(&v, &plat->pltActor->actor->position, &plat->path->nodes->worldTile->centre);
	offs = DotProductFF(&v, &moveVec);

	speed = FMul(plat->currSpeed*SCALE, gameSpeed);

	ScaleVectorFF(&moveVec, speed);
	RoundVectorRandF(&moveVec);

	// platform only moves when frog is it - otherwsie returns to start position
	if(plat->flags & PLATFORM_NEW_CARRYINGFROG)
	{
		if (offs > (destOffs + speed))
		{
			SubFromVectorSF(&plat->pltActor->actor->position, &moveVec);
		}
		else if (offs < (destOffs - speed))
		{
			AddToVectorSF(&plat->pltActor->actor->position, &moveVec);
		}
	}
	else
	{
		if (offs > (startOffs + speed))
		{
			SubFromVectorSF(&plat->pltActor->actor->position, &moveVec);
		}
		else if (offs < (startOffs - speed))
		{
			AddToVectorSF(&plat->pltActor->actor->position, &moveVec);
		}
	}
}


/*	--------------------------------------------------------------------------------
    Function	: EnumPlatforms
	Purpose		: Calls a function for every platform with a given UID
	Parameters	: 
	Returns		: 

	func takes two params, the platform and the 'param' passed to EnumPlatforms
*/
int EnumPlatforms(long id, int (*func)(PLATFORM*, int), int param)
{
	PLATFORM *cur, *next;
	int count = 0, down;

	cur = platformList.head.next;

	for(down=platformList.count; down; cur=next,count++,down--)
	{
		next = cur->next;
		if (!id || cur->uid == id)
		{
			if (!func(cur, param)) break;
		}
	}

	if (count==0)
		utilPrintf("EnumPlatforms(%d) found 0 enemies?\n", id);

	return count;
}

void SetPlatformVisible(PLATFORM *plt, int visible)
{
	if (visible)
	{
		plt->active = 1;
		plt->pltActor->draw = 1;
		plt->countdown = -1;
		
		if (plt->isWaiting != -1)
		{
			plt->isWaiting = -1;
			SetPlatformMoving(plt, 1);
		}

		MovePlatformToStartOfList(plt);
	}
	else
	{
		plt->pltActor->draw = 0;
		plt->active = 0;
		MovePlatformToEndOfList(plt);
	}
}

void SetPlatformMoving(PLATFORM *plt, int moving)
{
	if (moving)
	{
		if (plt->isWaiting)
		{
			plt->isWaiting = 0;
			//plt->path->toNode = plt->path->fromNode; -- ? clearly this ain't gonna woik
			plt->path->startFrame = actFrameCount;
			plt->path->endFrame = actFrameCount + ((60*plt->currSpeed)>>12);
		}
		
		if (plt->Update) plt->Update(plt);
	}
	else
	{
		plt->isWaiting = -1;
	}
}

void MovePlatformToEndOfList(PLATFORM *plt)
{
	plt->prev->next = plt->next;
	plt->next->prev = plt->prev;
	plt->prev = platformList.head.prev;
	plt->next = &platformList.head;
	platformList.head.prev = plt;
	plt->prev->next = plt;
}

void MovePlatformToStartOfList(PLATFORM *plt)
{
	plt->prev->next = plt->next;
	plt->next->prev = plt->prev;
	plt->next = platformList.head.next;
	plt->prev = &platformList.head;
	platformList.head.next = plt;
	plt->next->prev = plt;
}
/*	--------------------------------------------------------------------------------
	Function		: MovePlatform
	Purpose			: moves a platform to a given node in its path
	Parameters		: PLATFORM*, int [can be used with EnumPlatforms]
	Returns			: 1 for success
*/
int MovePlatformToNode(PLATFORM *plt, int flag)
{
	if (flag >= 0 && flag < plt->path->numNodes)
	{
		plt->path->toNode = flag;
		plt->inTile[0] = plt->path->nodes[flag].worldTile;
		plt->path->endFrame = actFrameCount;
		plt->Update(plt);

		if( !(plt->flags & PLATFORM_NEW_NOREORIENTATE) )
		{
			if( !(plt->flags & PLATFORM_NEW_FACEFORWARDS) )
			{
				FVECTOR fwd;

				SubVectorFSS( &fwd, &plt->path->nodes[(flag+1)%plt->path->numNodes].worldTile->centre, &plt->inTile[0]->centre );
				MakeUnit(&fwd);

 				OrientateFS(&plt->destOrientation, &fwd, &plt->inTile[0]->normal);
			}
			else
			{
				OrientateSS(&plt->destOrientation, &plt->path->nodes->worldTile->dirVector[plt->facing], &plt->inTile[0]->normal);
			}

			plt->pltActor->actor->qRot = plt->srcOrientation = plt->destOrientation;
		}
	}
	else
		utilPrintf("MoveEnemyToNode(): Flag (%d) out of range\n", flag);

	return 1;
}


/*	--------------------------------------------------------------------------------
	Function		: RecalculatePlatform
	Parameters		: PLATFORM *
	Returns			: void
*/
/*
void RecalculatePlatform(PLATFORM *plat)
{
	// only recalculate when we're actually waiting, otherwise when it gets to the next
	// path node it'll sort itself out anyway.

	if (plat->isWaiting && (plat->Update))
	{
		plat->Update(plat);
	}
}
*/
/*	--------------------------------------------------------------------------------
	Function		: UpdateNonMovingPlatform
	Purpose			: updates platforms that don't move
	Parameters		: PLATFORM *
	Returns			: void
	Info			: 
*/
//void UpdateNonMovingPlatform(PLATFORM *plat)
//{
//}

/*	--------------------------------------------------------------------------------
	Function		: UpdatePathSlerpPlatform
	Purpose			: Platforms that move in a smooth curve along a path (using Jim's slerpy code)
	Parameters		: PLATFORM*
	Returns			: void
	Info			: Use offset2 for slerp speed.
*/
//void UpdateSlerpPathPlatform( PLATFORM *cur )
//{
// 	QUATERNION q1, q2, q3;
// 	SVECTOR fromPosition,toPosition;
// 	FVECTOR fwd, moveVec;
// 	PATH *path = cur->path;
// 	ACTOR *act = cur->pltActor->actor;
// 	fixed speed, t;
// 	long i;
// 	
// 	// Find the position of the current 2 nodes
// 	GetPositionForPathNode(&toPosition,&path->nodes[path->toNode]);
// 	
// 	// Set direction to desired point
//  	SubVectorFSS(&fwd,&toPosition,&act->position);
// 	MakeUnit(&fwd);
// 
// // 	SetQuaternion( &q1, &act->qRot );
// 
// 	// Skewer a line to rotate around, and make a rotation
// //	CrossProductFlFF((FLVECTOR *)&q3,&inVec,&fwd);
// 	t = DotProduct(&inVec,&fwd);
// //	if (t<ToFixed(-0.999))
// //		t=ToFixed(-0.999);
// //	if (t>ToFixed(0.999))
// //		t = ToFixed(0.999);
// 	if (t<4092)
// 		t=4092;
// 	if (t>4092)
// 		t = 4092;
// 	q3.w=acos(t/4096.0f);
// 	// Duh.
// 	GetQuaternionFromRotation(&q2,&q3);
// 
// 	// Slerp between current and desired orientation
// 	speed = FMul(path->nodes[path->fromNode].offset2, gameSpeed);
// 
// 	if(speed > 4092)
// 		speed = 4092;
// // 	QuatSlerp( &q1, &q2, speed, &act->qRot );
// 
// 	// Move forwards a bit in direction of facing
// // 	RotateVectorByQuaternionFF( &fwd, &inVec, &act->qRot );
// 	ScaleVectorFF( &fwd, FMul(cur->currSpeed,gameSpeed) );
// 
//  	AddToVectorSF(&act->position, &fwd);
// 
// 
// 	// Update path nodes
// 	if( DistanceBetweenPointsSS(&act->position,&toPosition) < ToFixed(10*SCALE) )
// 	{
// 		UpdatePlatformPathNodes(cur);
// 
// 		cur->path->startFrame = actFrameCount + cur->isWaiting * (waitScale>>12);
// 	}
// 
// 	// Set inTile
// 	GetPositionForPathNode(&fromPosition,&path->nodes[path->fromNode]);
// 	GetPositionForPathNode(&toPosition,&path->nodes[path->toNode]);
// 
// 	t = DistanceBetweenPointsSS(&fromPosition,&toPosition) / 2;
// 
// 	if(DistanceBetweenPointsSS(&fromPosition,&act->position) < t)
// 		cur->inTile[0] = path->nodes[path->fromNode].worldTile;
// 	else
// 		cur->inTile[0] = path->nodes[path->toNode].worldTile;
// 
// 	// We need to find which frog we're carrying (yeeeeeesh)
// 	for (i=0; i<4; i++)
// 		if (frog[i] == cur->carrying)
// 			break;
// 
// 	if( i!=4 ) CheckTileForCollectable( cur->inTile[0], i );
//}
