/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: edcamera.c
	Programmer	: David Swift
	Date		: 05/08/99

---------------------------------------------------------------------------- */

#include "edmaths.h"
#include "editdefs.h"
#include "edglobals.h"
#include "edcamera.h"

#define FLOAT2INT(x) ((int)(0x10000 * (x)))

CREATEENTITY *EditorAddCameraCase(EDITGROUP *selection, unsigned long flags, EDVECTOR *v)
{
	CREATEENTITY *entity;
	EDITGROUPNODE *node;
	EDVECTOR avg;
	int count;
	
	// stick icon at the "middle" (mean average) position
	ZeroVector(&avg);

	for (node = selection->nodes, count = 0; node; node = node->link, count++)
	{
		EDVECTOR tv;
		GetTilePos(&tv, (GAMETILE*)node->thing);
		AddToVector(&avg, &tv);
	}

	if( count )
	{
		ScaleVector(&avg, 1.0f/(float)count);
	}
	else 
	{
		return NULL;
	}

	entity = (CREATEENTITY*)calloc(1,sizeof(CREATEENTITY));

	entity->thing = CREATE_CAMERACASE;
	entity->flags = flags;

	//entity->path = MakePathFromTileGroup(selection);
	entity->group = selection; selection->refs++;
	entity->path = NULL;
	
	entity->scale = 45.0;	// FOV
	entity->animSpeed = 1.0; // Camera speed

	entity->camera = *v;


	AddGroupMember(entity, &avg, createList);

	// Old selectionList (selection) is now part of the camera case.
	// Make a new selectionList and add the camera case to it.
	selectionList = MakeEditGroup( );
	selection->refs--;
	AddGroupMember( entity, &avg, selectionList );

	return entity;
}
