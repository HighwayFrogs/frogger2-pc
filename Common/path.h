/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: path.h
	Programmer	: Andrew Eder
	Date		: 16/3/99

----------------------------------------------------------------------------------------------- */

#ifndef PATH_H_INCLUDED
#define PATH_H_INCLUDED

#include "actor2.h"


//#define NODE_RANGETHRESHOLD				10.0F


#define GET_PATHLASTNODE(x)		(x->numNodes - 1)


//----- [ PATH FLAGS ] -------------------------------------------------------------------------//

#define PATH_MAKENODETILEPTRS			(1 << 0)



//----- [ PATHNODE DATA STRUCTURE ] -----//

typedef struct TAGPATHNODE
{
	GAMETILE				*worldTile;				// ptr to underlying world tile
	fixed					offset;	 				// offset(1) from underlying world tile
	fixed					offset2;	 			// offset(2) from underlying world tile

	fixed					speed;					// speed at this node
	short					waitTime;				// pause time at this node
	struct _SAMPLE			*sample;				// Played when an enemy/platform reaches this node //mmsfx
//	SAMPLE			*sample;				// Played when an enemy/platform reaches this node //mmsfx


} PATHNODE;

//----- [ PATH DATA STRUCTURE ] -----//

typedef struct TAGPATH
{
	short					numNodes;				// number of nodes in path
	short					fromNode;				// from node
	short					toNode;					// node we're heading towards
	short					startNode;				// start node for this path
	unsigned long			startFrame;				// The start frame
	unsigned long			endFrame;				// The start frame
	PATHNODE				*nodes;					// actual path nodes

} PATH;

#ifdef __cplusplus
extern "C" {
#endif

//----- [ FUNCTION PROTOTYPES ] -----//

void GetPositionForPathNode(SVECTOR *vecPos, PATHNODE *pNode);
void GetPositionForPathNodeOffset2(SVECTOR *vecPos,PATHNODE *pNode);
BOOL ActorHasArrivedAtNode(ACTOR2 *pAct,PATH *pPath,int nodeID);

void SetPathNodeSpeed(PATHNODE *pNode,fixed speed);
void SetPathNodeWaitTime(PATHNODE *pNode,short pause);
void SetAllPathNodesToSetSpeed(PATH *pPath,fixed speed);
void SetAllPathNodesToSetWaitTime(PATH *pPath,short pause);
fixed GetSpeedFromIndexedNode(PATH *path,short index);
short GetWaitTimeFromIndexedNode(PATH *path,short index);


void AssignRandomSpeedsToPathNodes(PATH *path,fixed minSpeed,fixed maxSpeed);
void AssignRandomWaitTimesToPathNodes(PATH *path,short minPause,fixed maxPause);

void ScalePathSpeed(PATH *path, fixed scale);

void AddPathNodesToList(PATHNODE*);
void AllocatePathList(int totalPathNodes);
PATHNODE *GetPathNodes(int num);
void FreePathList(void);

int CheckPathOrientation( PATH *path );

#ifdef __cplusplus
}
#endif
#endif
