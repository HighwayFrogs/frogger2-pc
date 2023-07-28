/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: path.c
	Programmer	: Andrew Eder
	Date		: 16/3/99

----------------------------------------------------------------------------------------------- */

#define F3DEX_GBI_2

#include "ultra64.h"

#include "path.h"
//#include "jalloc.h"
#include "maths.h"
#include <islmem.h>
#include <islutil.h>
#include "Main.h"

PATHNODE *pathNodes = NULL;
PATHNODE *nextPathNode = NULL;
unsigned short pathCount, pathAlloc;


#ifdef PC_VERSION

/*	Only the PC version uses the path list, cause other versions just use a single
	pre-allocated block of nodes */

typedef struct TAG_PATHNODESTORE
{
	PATHNODE *nodes;
	struct TAG_PATHNODESTORE *next;
} PATHNODESTORE;

PATHNODESTORE *pathNodeList = NULL;

/*	--------------------------------------------------------------------------------
	Function		: AddPathNodesToList
	Purpose			: Adds an array of pathnodes to a global list
	Parameters		: PATHNODE *first, int count
	Returns			: 
*/

void AddPathNodesToList(PATHNODE *first)
{
	PATHNODESTORE *newNode;

	newNode = (PATHNODESTORE*)MALLOC0(sizeof(PATHNODESTORE));
	newNode->nodes = first;
	newNode->next = pathNodeList;
	pathNodeList = newNode;
}

/*	--------------------------------------------------------------------------------
	Function		: FreePathList
	Purpose			: Free the global list of PATHs
	Parameters		: void
*/
void FreePathList(void)
{
	PATHNODESTORE *node, *next;

	for (node = pathNodeList; node; node = next)
	{
		next = node->next;
		FREE(node->nodes);
		FREE(node);
	}
	pathNodeList = NULL;
	pathNodes = NULL;
}

#else	// for other versions..

void FreePathList(void)
{
	FREE(pathNodes);
	pathNodes = NULL;
}

#endif

/*	--------------------------------------------------------------------------------
	Function		: AllocatePathList
	Purpose			: Allocates a big block of pathnodes
	Parameters		: void
*/
void AllocatePathList(int numNodes)
{
	nextPathNode = pathNodes = MALLOC0(numNodes * sizeof(PATHNODE));
	pathCount = 0;
	pathAlloc = numNodes;

#ifdef PC_VERSION
	AddPathNodesToList(pathNodes);
#endif
}

/*	--------------------------------------------------------------------------------
	Function		: GetPathNodes
	Purpose			: Get a block of pathnodes from a globally allocated list
	Parameters		: void
*/
PATHNODE *GetPathNodes(int numNodes)
{
	PATHNODE *nodes;

	if ((pathCount+numNodes) > pathAlloc)
		utilPrintf("Error! Tried to allocate %d nodes with %d left\n", numNodes, (pathAlloc-pathCount));

	nodes = nextPathNode;

	nextPathNode += numNodes;	
	pathCount += numNodes;

	//if you WANT lots of debug info...
	//utilPrintf("GetPathNodes(%d), total %d/%d\n", numNodes, pathCount, pathAlloc);

	return nodes;
}


/*	--------------------------------------------------------------------------------
	Function		: GetPositionForPathNode
	Purpose			: gets position for specified path node
	Parameters		: VECTOR *,PATHNODE *
	Returns			: void
	Info			: 
*/
void GetPositionForPathNode(SVECTOR *vecPos, PATHNODE *pNode)
{
	vecPos->vx = pNode->worldTile->centre.vx + (short)(FMul(pNode->worldTile->normal.vx, pNode->offset)>>12);
	vecPos->vy = pNode->worldTile->centre.vy + (short)(FMul(pNode->worldTile->normal.vy, pNode->offset)>>12);
	vecPos->vz = pNode->worldTile->centre.vz + (short)(FMul(pNode->worldTile->normal.vz, pNode->offset)>>12);
}


/*	--------------------------------------------------------------------------------
	Function		: GetPositionForPathNodeOffset2
	Purpose			: gets position for specified path node using offset 2
	Parameters		: VECTOR *,PATHNODE *
	Returns			: void
	Info			: 
*/
void GetPositionForPathNodeOffset2(SVECTOR *vecPos,PATHNODE *pNode)
{
	vecPos->vx = pNode->worldTile->centre.vx + (short)(FMul(pNode->worldTile->normal.vx, pNode->offset2)>>12);
	vecPos->vy = pNode->worldTile->centre.vy + (short)(FMul(pNode->worldTile->normal.vy, pNode->offset2)>>12);
	vecPos->vz = pNode->worldTile->centre.vz + (short)(FMul(pNode->worldTile->normal.vz, pNode->offset2)>>12);
}









/*	--------------------------------------------------------------------------------
	Function		: ScalePathSpeed
	Purpose			: Scales the speed of each path node by a given amount
	Parameters		: PATH *, fixed
	Returns			: void
*/

void ScalePathSpeed(PATH *path, fixed scale)
{
	int i;
	PATHNODE *n;
	for (i = path->numNodes, n = path->nodes; i; i--, n++)
		n->speed = FMul(n->speed, scale);
}


/*	--------------------------------------------------------------------------------
	Function		: SetAllPathNodesToSetSpeed
	Purpose			: sets all nodes in specified path to specified speed
	Parameters		: PATH *,fixed
	Returns			: void
	Info			: 
*/
void SetAllPathNodesToSetSpeed(PATH *pPath,fixed speed)
{
	int i = pPath->numNodes;
	while(i--)
		pPath->nodes[i].speed = speed;
}


/*	--------------------------------------------------------------------------------
	Function		: SetAllPathNodesToSetWaitTime
	Purpose			: sets all nodes in specified path to specified wait time
	Parameters		: PATH *,short
	Returns			: void
	Info			: 
*/
void SetAllPathNodesToSetWaitTime(PATH *pPath,short pause)
{
	int i = pPath->numNodes;
	while(i--)
		pPath->nodes[i].waitTime = pause;
}


/*	--------------------------------------------------------------------------------
	Function		: CalcDirectionVectorFromNodeToNode
	Purpose			: calculates the direction vector from one node to another
	Parameters		: VECTOR *,PATHNODE *,PATHNODE *
	Returns			: void
	Info			: 
*/
void CalcDirectionVectorFromNodeToNode(FVECTOR *result,PATHNODE *fromNode,PATHNODE *toNode)
{
	SVECTOR vecFrom,vecTo;

	GetPositionForPathNode(&vecFrom,fromNode);
	GetPositionForPathNode(&vecTo,toNode);

	SubVectorFSS(result,&vecTo,&vecFrom);
	MakeUnit(result);
}


/*	--------------------------------------------------------------------------------
	Function		: AssignRandomSpeedsToPathNodes
	Purpose			: assigns random speed values to path nodes
	Parameters		: PATH *,fixed,fixed
	Returns			: void
	Info			: 
*/
void AssignRandomSpeedsToPathNodes(PATH *path,fixed minSpeed,fixed maxSpeed)
{
	int i = path->numNodes;
	while(i--)
		path->nodes[i].speed = Random(maxSpeed) + minSpeed;
}


/*	--------------------------------------------------------------------------------
	Function		: AssignRandomWaitTimesToPathNodes
	Purpose			: assigns random wait times to path nodes
	Parameters		: PATH *,short,short
	Returns			: void
	Info			: 
*/
void AssignRandomWaitTimesToPathNodes(PATH *path,short minPause,fixed maxPause)
{
	int i = path->numNodes;
	while(i--)
		path->nodes[i].waitTime = (short)(Random(maxPause)+minPause);
}


// Run through path - if all tiles have same orientation and this enemy is faceforwards, never reorientate
int CheckPathOrientation( PATH *path )
{
	int i;

	for( i=0; i<path->numNodes-1; i++ )
	{
		if( path->nodes[i].worldTile->normal.vx != path->nodes->worldTile->normal.vx ||
			path->nodes[i].worldTile->normal.vy != path->nodes->worldTile->normal.vy ||
			path->nodes[i].worldTile->normal.vz != path->nodes->worldTile->normal.vz )
			return 0;
	}

	return 1;
}

