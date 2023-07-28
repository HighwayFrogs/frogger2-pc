/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: editdefs.c
	Programmer	: David Swift
	Date		: 08/07/99

---------------------------------------------------------------------------- */

#include <ultra64.h>
#include <stdio.h>

#include "islutil.h"

#include "edittypes.h"
#include "edglobals.h"
#include "editdefs.h"
#include "edcommand.h"
#include "edmaths.h"

#include "mdx.h"

#include "enemies.h"
#include "platform.h"
#include "specfx.h"
#include "cam.h"
#include "pcaudio.h"

/*	------------------------------------------------------------------------
	Function		: EditorAddFlag

	Parameters		: GAMETILE*, EDITPATH*
	Returns			: EDITPATHNODE*
*/

EDITPATHNODE *EditorAddFlag(GAMETILE* tile, EDITPATH* path)
{
	EDITPATHNODE *node, *newPath;
	int i = 0;

	newPath = (EDITPATHNODE*)calloc(1,sizeof(EDITPATHNODE)); //JallocAlloc(sizeof(EDITPATHNODE), YES, "epathnode");

	if (path->nodes)
	{
		for (node = path->nodes; node; node = node->link)
		{
			if (tile == node->tile) i++;
			if (!node->link) { node->link = newPath; break; }
		}
		newPath->speed = node->speed;
		newPath->offset = node->offset;
		newPath->offset2 = node->offset2;
	}
	else
	{
		newPath->speed = 1.0;
		newPath->offset = 0;
		newPath->offset2 = 0;
		path->nodes = newPath;
	}

	newPath->waitTime = 0;
	newPath->sample = 0;
	newPath->start = editPath ? FALSE : TRUE;
	newPath->tile = tile;
	newPath->link = NULL;
	newPath->level = i;

	return newPath;
}

/*	-------------------------------------------------------------------------
	Function		: EditorSubFlag

	Parameters		: GAMETIME*, EDITPATH*
	Returns			: 
*/

void EditorSubFlag(EDITPATHNODE *node, EDITPATH *path)
{
	EDITPATHNODE *next, *p;

	if (node == path->nodes)
	{
		path->nodes = node->link;
		free(node); //JallocFree((BYTE**)&node);
	}
	else
	for (p = path->nodes; p; p = next)
	{
		next = p->link;
		if (next == node)
		{
			p->link = next->link;
			free(next); //JallocFree((BYTE**)&next);
			break;
		}
	}
}


/*	-------------------------------------------------------------------------
	Function		: EditorAddFlag

	Parameters		: GAMETIME*, EDITPATH*
	Returns			:

	Follows link pointers from flags until it gets as close as possible to the target.
*/

void EditorAddFlags(GAMETILE *end, EDITPATH *path)
{
	EDITPATHNODE *node;
	GAMETILE *tile, *next;
	EDVECTOR v, a, b;
	int i, count;
	float dot;

	if (!path->nodes)
	{
		EditorAddFlag(end, path);
		return;
	}
	
	for (node = path->nodes; node->link; node = node->link);

	tile = node->tile; count = 50;

	while (tile != end)
	{
		next = NULL;

		GetTilePos(&a, tile);
		GetTilePos(&b, end);

		SubVector(&v, &a, &b);
		EdMakeUnit(&v);

		for (i = 0; i < 4; i++)
		{
			dot = DotProduct(&v, &tile->dirVector[i]);
			if (dot > 0.6)
			{
				next = tile->tilePtrs[i];
				break;
			}
		}
		if (!next)
			break;
		else
			EditorAddFlag(next, path);

		tile = next;

		if (!--count) return;
	}

	if (tile != end)
		EditorAddFlag(end, path);
}

/*	--------------------------------------------------------------------------------
	Function		: EditorAddCreateNode

	Parameters		: const char*, int, PATH*, CREATETYPE
	Returns			: CREATEENTITYNODE*
*/

CREATEENTITY *EditorAddCreateNode(const char *id, int flags, int ID, CREATETYPE type,
									  EDITPATH *path, int startNode)
{
	CREATEENTITY *newNode;
	EDITPATHNODE *node;
	int count;
	EDVECTOR iconPos;
	EDITGROUPNODE *p;

	for (count = startNode, node = path->nodes; count; count--)
	{
		if (node->link)	node = node->link;
	}

	newNode = (CREATEENTITY*)calloc(sizeof(CREATEENTITY), 1); //JallocAlloc(sizeof(CREATEENTITY), YES, "eentity");

	// If the paste button is pressed
	if( copyFlag && copyEntity && copyEntity->thing == type )
	{
		strncpy(newNode->type, copyEntity->type, MAXTYPELENGTH); newNode->type[MAXTYPELENGTH-1] = 0;
		newNode->flags = copyEntity->flags;
		newNode->thing = copyEntity->thing;
		newNode->scale = copyEntity->scale;
		newNode->radius = copyEntity->radius;
		newNode->animSpeed = copyEntity->animSpeed;
		newNode->value1 = copyEntity->value1;
		newNode->objFlags = copyEntity->objFlags;
		newNode->facing = copyEntity->facing;
	}
	else
	{
		strncpy(newNode->type, id, MAXTYPELENGTH); newNode->type[MAXTYPELENGTH-1] = 0;
		newNode->flags = flags;
		newNode->thing = type;
		newNode->scale = 1.0;
		newNode->radius = 50.0;
		newNode->animSpeed = 0.25;
		newNode->value1 = 0;
		newNode->objFlags = 0;
		newNode->facing = 0;
	}

	if( type == CREATE_CAMERACASE )
	{
		newNode->scale = 45.0;
		newNode->animSpeed = 1.0;
	}
	
	newNode->effects = EF_TINTRED | EF_TINTGREEN | EF_TINTBLUE | EF_MEDIUM;
	newNode->startNode = startNode;
	newNode->ID = ID;
	newNode->path = path; path->refs++;
	
	GetTilePos(&iconPos, node->tile);

	iconPos.vz += 10;

	for (p = createList->nodes; p; p = p->link)
		if( DistanceBetweenPointsSquared(&iconPos, &p->pos) < 20 )
			iconPos.vz += 10;

	AddGroupMember(newNode, &iconPos, createList);
	
	return newNode;
}

void EditorSubCreateNode(CREATEENTITY *node)
{
	SubGroupMember(node, createList);
	FreeEditPath(node->path);
	free(node); //JallocFree((BYTE**)&node);
}


/*	--------------------------------------------------------------------------------
	Function		: EditorPathMake


	Parameters		: 
	Returns			: 
*/

PATH *EditorPathMake(const EDITPATH *editPath, int startnode)
{
	int count, i;
	EDITPATHNODE *path;
	PATHNODE *node;

	PATH *newPath = (PATH *)calloc(sizeof(PATH), 1);

	/* count the number of nodes first */
	for (path = editPath->nodes, count = 0; path; path = path->link, count++);

	if (!count) return NULL;

	newPath->numNodes = count;

	// allocate memory for path nodes
	newPath->nodes = (PATHNODE *)calloc(count,sizeof(PATHNODE));

	i = 0;
	node = newPath->nodes;

	for (path = editPath->nodes; path; path = path->link, node++, i++)
	{
		if (!(node->worldTile = path->tile))
		{
			utilPrintf("worldtile is NULL!\n");
		}

		node->speed		= GAMEFLOAT(path->speed);
		node->offset	= GAMEFLOAT(path->offset)*10;
		node->offset2	= GAMEFLOAT(path->offset2)*10;
		node->waitTime  = path->waitTime;
		node->sample	= FindSample(path->sample);

		//if (path->start) newPath->startNode = i;
	}

	if (startnode < count)
		newPath->startNode = startnode;
	else
	{
		utilPrintf("start node (%d) > number of nodes (%d)?\n",
			startnode, count);
		newPath->startNode = 0;
	}

	AddPathNodesToList(newPath->nodes);

	return newPath;
}

/*	--------------------------------------------------------------------------------
	Function		: EditorGetPath
	Parameters		: 
	Returns			: 
*/

EDITPATH *EditorGetPath(const PATH *path)
{
	int i;
	EDITPATHNODE *newPath, *prev, *start;
	EDITPATH *ep;

	prev = NULL; start = NULL;

	for (i = 0; i<path->numNodes; i++)
	{
		newPath = (EDITPATHNODE*)calloc(1,sizeof(EDITPATHNODE)); //JallocAlloc(sizeof(EDITPATHNODE), YES, "epath");

		if (prev)
			prev->link = newPath;
		else
			start = newPath;

		newPath->tile = path->nodes[i].worldTile;
		newPath->level = 0;
		newPath->speed = EDFLOAT(path->nodes[i].speed);
		newPath->offset = EDFLOAT(path->nodes[i].offset);
		newPath->offset2 = EDFLOAT(path->nodes[i].offset2);
		newPath->start = (i == path->startNode) ? TRUE : FALSE;
		newPath->waitTime = path->nodes[i].waitTime;

		newPath->sample = (path->nodes[i].sample)?(path->nodes[i].sample->uid):0;

		newPath->link = NULL;
		prev = newPath;
	}

	ep = CreateEditPath();
	ep->nodes = start;

	return ep;
}


EDITPATH *CreateEditPath()
{
	EDITPATH *path;
	path = (EDITPATH *)calloc(1, sizeof(EDITPATH));
	path->refs = 1;
/*	
	path->next = pathList;
	if (pathList)
		pathList->prev = path;
	pathList = path;
*/
	return path;
}


void FreeEditPath(EDITPATH *editPath)
{
	EDITPATHNODE *del, *path;
	
	if (editPath)
	{
		editPath->refs--;

		if (editPath->refs < 0)
			utilPrintf("ERROR! FreeEditPath: refs < 0\n");
		
		if (editPath->refs <= 0)
		{
			path = editPath->nodes;
			while (path)
			{
				del = path; path = path->link;
				free(del);
			}
/*
			if (editPath->next)
				editPath->next->prev = editPath->prev;
			if (editPath->prev)
				editPath->prev->next = editPath->next;
			else
				pathList = editPath->next;
*/
			free(editPath);
		}
	}
}

void FreeCreateList()
{
	while (createList->nodes) EditorSubCreateNode(createList->nodes->thing);
}

/*	------------------------------------------------------------------------------ */

EDITGROUP *MakeEditGroup(void)
{
	EDITGROUP *group = (EDITGROUP*)calloc(1,sizeof(EDITGROUP)); //JallocAlloc(sizeof(EDITGROUP), YES, "egroup");
	group->refs++;
	return group;
}

void FreeEditGroup(EDITGROUP **group)
{
	EDITGROUPNODE *node, *next;

	if (!group || !(*group)) return;

	--(*group)->refs;

	if ((*group)->refs == 0)
	{
		for (node = (*group)->nodes; node; node = next)
		{
			next = node->link;
			free(node);
		}

		free(*group);
	}
	*group = NULL;
}

/*	-------------------------------------------------------------------------
	Function		: MakePathEditGroup

	Parameters		: EDITPATH*
	Returns			: EDITGROUP*

	Makes an EDITGROUP containing all the nodes from an editpath
*/
EDITGROUP *MakePathEditGroup(EDITPATH *path)
{
	EDITPATHNODE *pn;
	EDITGROUP *group;

	path->refs++;

	group = MakeEditGroup();

	for (pn = path->nodes; pn; pn = pn->link)
	{
		EDVECTOR v;
		SetVectorF(&v, &pn->tile->centre);
		AddGroupMember(pn, &v, group);
	}

	return group;
}

/*	-------------------------------------------------------------------------
	Function		: MakePathEditGroup

	Parameters		: EDITGROUP*
	Returns			: EDITPATH*

	Makes an EDITPATH from a bunch of tiles stored in an EDITGROUP
*/
EDITPATH *MakePathFromTileGroup(EDITGROUP *group)
{
	EDITGROUPNODE *node;
	EDITPATH *path;

	path = CreateEditPath();

	for (node = group->nodes; node; node = node->link)
		EditorAddFlag((GAMETILE*)node->thing, path);

	return path;
}

/*	-------------------------------------------------------------------------
	Function		: MakePathEditGroup

	Parameters		: EDITGROUP*
	Returns			: EDITPATH*

	Makes an EDITGROUP from the tiles in an EDITPATH
*/
EDITGROUP *MakeTileGroupFromPath(EDITPATH *path)
{
	EDITPATHNODE *node;
	EDITGROUP *group;

	group = MakeEditGroup();

	for (node = path->nodes; node; node = node->link)
	{
		EDVECTOR v;
		SetVectorS(&v, &node->tile->centre);
		AddGroupMember((void*)node->tile, &v, group);
	}

	return group;
}

void AddGroupMember(void *thing, EDVECTOR *pos, EDITGROUP *group)
{
	EDITGROUPNODE *s;

	s = (EDITGROUPNODE*)calloc(sizeof(EDITGROUPNODE), 1); //JallocAlloc(sizeof(EDITGROUPNODE), NO, "egrpm");
	s->thing = thing;
	s->pos = *pos;
	s->link = group->nodes;
	group->nodes = s;
}

BOOL AddUniqueGroupMember(void *thing, EDVECTOR *pos, EDITGROUP *group)
{
	EDITGROUPNODE *n;

	for (n = group->nodes; n; n = n->link)
		if (n->thing == thing) return FALSE;

	AddGroupMember(thing, pos, group);
	return TRUE;
}

int SubGroupMember(void *thing, EDITGROUP *group)
{
	EDITGROUPNODE *s, *prev;

	for (s = group->nodes, prev = NULL; s; prev = s, s = s->link)
	{
		if (s->thing == thing)
		{
			if (prev)
				prev->link = s->link;
			else
				group->nodes = s->link;
			free(s);
			return -1;
		}
	}
	return 0;
}

int ToggleGroupMember(void* thing, EDVECTOR* pos, EDITGROUP *group)
{
	EDITGROUPNODE *s, *prev;

	prev = NULL;

	for (s = group->nodes;; prev = s, s = s->link)
	{
		if (!s) {
			s = (EDITGROUPNODE*)calloc(sizeof(EDITGROUPNODE), 1); //JallocAlloc(sizeof(EDITGROUPNODE), NO, "egrpm");
			s->thing = thing;
			s->pos = *pos;
			s->link = group->nodes;
			group->nodes = s;
			return 1;
		}
		else if (s->thing == thing)
		{
			if (prev)
				prev->link = s->link;
			else
				group->nodes = s->link;
			free(s);
			return -1;
		}
	}
}

int ToggleRectSelection(int ax, int ay, int bx, int by, const EDITGROUP *list, EDITGROUP *selection)
{
	EDITGROUPNODE *node;
	EDVECTOR v;
	int fuzz = 0;

	// flip rect the right way round
	if (ax > bx) { int foo = ax; ax = bx; bx = foo; }
	if (ay > by) { int foo = ay; ay = by; by = foo; }

	for (node = list->nodes; node; node = node->link)
	{
		EdXfmPoint(&v, &node->pos);
		if (!v.vz) continue;
		
		if (v.vx > ax && v.vx < bx && v.vy > ay && v.vy < by)
			fuzz += ToggleGroupMember(node->thing, &node->pos, selection);
	}

	return fuzz;
}

int ToggleRectTileSelection(int ax, int ay, int bx, int by, EDITGROUP *selection)
{
	GAMETILE *tile;
	EDVECTOR c, v;
	int n, fuzz = 0;

	// flip rect the right way round
	if (ax > bx) { int foo = ax; ax = bx; bx = foo; }
	if (ay > by) { int foo = ay; ay = by; by = foo; }

	for (tile = firstTile, n = tileCount; n; tile++, n--)
	{
		GetTilePos(&c, tile);
		EdXfmPoint(&v, &c);
		if (!v.vz) continue;
		
		if (v.vx > ax && v.vx < bx && v.vy > ay && v.vy < by)
			fuzz += ToggleGroupMember(tile, &c, selection);
	}

	return fuzz;
}

int ToggleRectFlagSelection(int ax, int ay, int bx, int by, EDITGROUP *selection, EDITPATH *path)
{
	EDITPATHNODE *pnode;
	EDVECTOR c, v;
	int fuzz = 0;

	if( !selection || !path )
		return 0;

	// flip rect the right way round
	if (ax > bx) { int foo = ax; ax = bx; bx = foo; }
	if (ay > by) { int foo = ay; ay = by; by = foo; }

	for (pnode = path->nodes; pnode; pnode = pnode->link)
	{
		GetTilePos(&c, pnode->tile);
		EdXfmPoint(&v, &c);
		if (!v.vz) continue;

		v.vy -= (pnode->level*16);
		
		if (v.vx > ax && v.vx < bx && v.vy > ay && v.vy < by)
			fuzz += ToggleGroupMember(pnode, &c, selection);
	}

	return fuzz;
}

/*	------------------------------------------------------------------------------ */


int GetSelectionTileState(EDITGROUP *group)
{
	EDITGROUPNODE *node;
	unsigned int state;

	if (!group->nodes) return -1;

	node = group->nodes;

	state = ((GAMETILE*)node->thing)->state;

	for (node = node->link; node; node = node->link)
	{
		if (((GAMETILE*)node->thing)->state != state) return -1;
	}

	return state;
}

int NextTileSelectionState(EDITGROUP *group)
{
	EDITGROUPNODE *node;
	int state;

	state = GetSelectionTileState(group);
	if (state == -1)
		state = TILESTATE_NORMAL;
	else
		if ((++state) > (NUMTILESTATES-1)) state = 0;
	
	for (node = group->nodes; node; node = node->link)
		((GAMETILE*)node->thing)->state = state;

	return state;
}


int PrevTileSelectionState(EDITGROUP *group)
{
	EDITGROUPNODE *node;
	int state;

	state = GetSelectionTileState(group);
	if (state == -1)
		state = TILESTATE_NORMAL;
	else
		if ((--state) < 0) state = NUMTILESTATES-1;
	
	for (node = group->nodes; node; node = node->link)
		((GAMETILE*)node->thing)->state = state;

	return state;
}

int CountGroupMembers(EDITGROUP *group)
{
	EDITGROUPNODE *node;
	int count;

	for (node = group->nodes, count = 0; node; node = node->link, count++);
	return count;
}

/*	--------------------------------------------------------------------------------
	Function		: ForAllGroupNodesStr
	Parameters		: group, str, function(node, str)
	Returns			: 
*/
int ForAllGroupNodesStr(EDITGROUP *group, const char* str, void(*func)(CREATEENTITY*,const char*))
{
	EDITGROUPNODE *enode;
	int count;

	if (!group) return 0;

	for (enode = group->nodes, count=0; enode; enode = enode->link, count++)
		func((CREATEENTITY*)enode->thing, str);
	return count;
}

/*	--------------------------------------------------------------------------------
	Function		: ForAllGroupNodesInt
	Parameters		: group, int, proc(node, int)
	Returns			: 
*/
int ForAllGroupNodesInt(EDITGROUP *group, const char *str, void(*func)(CREATEENTITY*,const int))
{
	EDITGROUPNODE *enode;
	int count, i;
	
	if (!group) return 0;

	i = atoi(str);

	for (enode = group->nodes, count=0; enode; enode = enode->link, count++)
		func((CREATEENTITY*)enode->thing, i);
	return count;
}

/*	--------------------------------------------------------------------------------
	Function		: ForAllGroupNodesFloat
	Parameters		: group, float, proc(node, float)
	Returns			: 
*/
int ForAllGroupNodesFloat(EDITGROUP *group, const char *str, void(*func)(CREATEENTITY*,const float))
{
	EDITGROUPNODE *enode;
	int count;
	float f = (float)atof(str);

	if (!group) return 0;

	for (enode = group->nodes, count=0; enode; enode = enode->link, count++)
		func((CREATEENTITY*)enode->thing, f);
	return count;
}

/*	--------------------------------------------------------------------------------
	Function		: EdXfmPoint
	Parameters		: 
	Returns			: 
*/

void EdXfmPoint (EDVECTOR *res, EDVECTOR *in)
{
	XfmPoint((MDX_VECTOR*)res, (MDX_VECTOR*)in, NULL);

	//if (res->vx < 0 || res->vx > rXRes ||
	//	res->vy < 0 || res->vy > rYRes)	// clip result
	//	res->vz = 0;
}

/*	--------------------------------------------------------------------------------
    Function		: FindNearestTile
	Parameters		: VECTOR
	Returns			: GAMETILE*
*/


GAMETILE* FindNearestTile(EDVECTOR v)
{
	GAMETILE *t, *closest;
	int n, dist, closestDist;
	EDVECTOR a;

	closest = NULL; closestDist = 0x10000000;

	for (t = firstTile, n = tileCount; n; t++, n--)
	{
		GetTilePos(&a, t);
		dist = (int)DistanceBetweenPointsSquared(&v, &a);

		if (dist < closestDist)
		{
			closest = t;
			closestDist = dist;
		}
	}

	return closest;
}
