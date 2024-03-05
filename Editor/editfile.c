/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: editfile.c
	Programmer	: David Swift
	Date		: 02/07/99

----------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <ultra64.h>

#include "edmaths.h"
#include "editor.h"
#include "editdefs.h"
#include "edglobals.h"
#include "edcamera.h"

#include "enemies.h"
#include "platform.h"
#include "islutil.h"
#include "collect.h"

const UBYTE fileVersion = 16;
const UBYTE releaseVersion = 0x40;

int releaseQuality = 0;

#define EDITOR_MAXPATH 1024

/* ------------------------------------------------------------------------------ */

void WriteEditPath(EDITPATH*, HANDLE);
void WriteTileGroup(EDITGROUP*, HANDLE);
EDITPATH *ReadEditPath(EDLOADSTATE *state);
EDITGROUP *ReadTileGroup(EDLOADSTATE *state);

/* ------------------------------------------------------------------------------ */

int bytesWritten, bytesRead;

void WriteByte(int v, HANDLE f)		{ signed char b = (signed char)v; WriteFile(f, &b, 1, &bytesWritten, NULL); }
void WriteWord(int v, HANDLE f)		{ WORD w = (WORD)v; WriteFile(f, &w, 2, &bytesWritten, NULL); }
void WriteInt(int v, HANDLE f)		{ WriteFile(f, &v, 4, &bytesWritten, NULL); }
void WriteFloat(float v, HANDLE f)	{ WriteInt((int)(v * 0x10000), f); }

UBYTE ReadByte(HANDLE f)		{ UBYTE v; ReadFile(f, &v, 1, &bytesRead, NULL); return v; }
int ReadInt(HANDLE f)		{ int v; ReadFile(f, &v, 4, &bytesRead, NULL); return v; }
short ReadWord(HANDLE f)	{ short v; ReadFile(f, &v, 2, &bytesRead, NULL); return v; }
float ReadFloat(HANDLE f)	{ return (float)ReadInt(f) / (float)0x10000; }

void WriteVector(EDVECTOR *v, HANDLE f)
{
	WriteFloat(v->vx, f);
	WriteFloat(v->vy, f);
	WriteFloat(v->vz, f);
}

void ReadVector(EDVECTOR *v, HANDLE f)
{
	v->vx = ReadFloat(f);
	v->vy = ReadFloat(f);
	v->vz = ReadFloat(f);
}

void WriteTile(GAMETILE *t, HANDLE h, int extraFast)
{
	if (extraFast)
	{
		WriteInt(((int)t-(int)firstTile)/sizeof(GAMETILE), h);	// ugh
	}
	else
	{
		EDVECTOR v;
		GetTilePos(&v, t);
		WriteVector(&v, h);
	}
}

GAMETILE *ReadTile(EDLOADSTATE *state)
{
	if (state->fastLoad)
	{
		int i = ReadInt(state->f);
		return &firstTile[i];
	}
	else
	{
		EDVECTOR v;
		ReadVector(&v, state->f);
		return FindNearestTile(v);
	}
}

int FindPtrIndex(void *i, void** list, int num)
{
	int j;
	for (j=0;j<num;j++)
		if (list[j] == i) return j;
	return 0;
}

/* ------------------------------ */

void MakeBackup(const char* filename)
{
	char s[MAX_PATH];
	SYSTEMTIME currTime;

	GetLocalTime(&currTime);
	sprintf(s, "%s.%02d-%02d-%d.%02d-%02d-%02d",
		filename, currTime.wDay, currTime.wMonth, currTime.wYear,
		currTime.wHour, currTime.wMinute, currTime.wSecond);

	rename(filename, s);
}

/*	--------------------------------------------------------------------------------
	Programmer		: David Swift
    Function		: LoadCreateList
	Parameters		: const char*
	Returns			: 
*/
BOOL LoadEntity(EDLOADSTATE *state, int type)
{
	int flags, i, ID, start, effects, objFlags;
	float scale, radius, animSpeed, value1;
	char PSXshift, PSXhack;
	char name[MAXTYPELENGTH];
	unsigned char facing;
	HANDLE f = state->f;

	EDITPATH *ep;

	i = ReadByte(f);
	ReadFile(f, name, i, &bytesRead, NULL);
	name[i] = 0;
	//stringChange(name);	// auto convert ".ndo" to ".obe"

	flags = ReadInt(f);
	ID = ReadInt(f);
	scale = ReadFloat(f);
	radius = ReadFloat(f);
	
	if(state->ver > 10 )
	{
		animSpeed = ReadFloat(f);
		value1 = ReadFloat(f);
	}
	else
	{
		animSpeed = 0.25; value1 = 0.0;
	}

	if( state->ver > 13 )
	{
		facing = ReadByte(f);
		objFlags = ReadInt(f);
	}
	else
	{
		facing = 0;
		objFlags = 0;
	}

	if (state->ver > 14)
	{
		PSXshift = ReadByte(f);
		PSXhack = ReadByte(f);
		for (i=2; i<4; i++) ReadByte(f);
	}
	else
		PSXshift = 0;

	if(state->ver > 11 ) effects = ReadInt(f); else effects = 0;

	start = ReadInt(f);

	if (state->ver < 13)
		ep = ReadEditPath(state);
	else
	{
		i = ReadWord(f);

		if (i < 0 || i > state->numPaths) return FALSE;

		ep = state->pathList[i];
		ep->refs++;
	}

	if (!ep) return FALSE;

	if (ep->nodes)
	{
		CREATEENTITY *create;

		create = EditorAddCreateNode(name, flags, ID, type, ep, start);
		create->scale = scale;
		create->radius = radius;
		create->animSpeed = animSpeed;
		create->value1 = value1;
		create->effects = effects;
		create->facing = facing;
		create->objFlags = objFlags;
		create->PSX_shift = PSXshift;
		create->PSX_hack = PSXhack;
	}

	FreeEditPath(ep);

	return TRUE;
}


BOOL LoadCreateList(const char* filename)
{
	HANDLE f;
	UBYTE type, ver;
	EDITGROUP *group;
	CREATEENTITY *create;
	GAMETILE *tile;
	int i, count, pathCount;
	
	EDLOADSTATE state;

	EDITPATH **pathList = NULL;

	f = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (f == INVALID_HANDLE_VALUE) {
		utilPrintf("Couldn't find file %s\n", filename);
		return FALSE;
	}

	ver = ReadByte(f);
	if (ver & releaseVersion)
	{
		state.fastLoad = 1;
		ver &= ~releaseVersion;
	}
	else
		state.fastLoad = 0;

	if (ver > fileVersion || ver < 2)
	{
		int i = ver;
		utilPrintf("Couldn't load edit file version (%s): %d\n", filename, i);
		return FALSE;
	}

	FreeCreateList();
	FreeEditPath(editPath); editPath = NULL;
	
	if (ver < 10)
	{
		utilPrintf("Error: Support for old files temporarily disabled\n");
		return FALSE;
	}

	state.f = f;
	state.ver = ver;

	//  ----------- Files v13 and greater start with a list of paths ---------
	if (ver >= 13)
	{
		int i;

		if (ver >= 16)
			ReadWord(f);	// total pathnodes

		pathCount = ReadWord(f);
		pathList = malloc(sizeof(EDITPATH*) * pathCount);

		state.pathList = pathList;
		state.numPaths = pathCount;

		for (i = 0; i < pathCount; i++)
			pathList[i] = ReadEditPath(&state);

		if (ver >= 16)
			for (i=0; i<5; i++)
				ReadWord(f);	// total enemies, platforms, garibs, cameracases and placeholders
		
		count = ReadWord(f);
	}
	else
	{
		count = ReadInt(f);
		state.pathList = NULL;
		state.numPaths = 0;
	}

	while (count--)
	{
		type = ReadByte(f);

		switch (type)
		{
		case CREATE_ENEMY:
		case CREATE_PLACEHOLDER:
		case CREATE_PLATFORM:
			if (!LoadEntity(&state, type))
			{
				CloseHandle(f); return FALSE;
			}
			break;

		case CREATE_GARIB:
			{
				EDITPATH *ep;
				EDVECTOR pos;

				int i = ReadByte(f);
				//ReadVector(&pos, f);
				//if (!(tile = FindNearestTile(pos)))

				if (!(tile = ReadTile(&state)))
				{
					utilPrintf("FindNearestTile() returned NULL\n");
					continue;
				}
				GetTilePos(&pos, tile);
				ep = CreateEditPath();
				EditorAddFlag(tile, ep);
				
				if (ver < 13)
				{
					// Calculate offset, you dozy cretin
					SubFromVector(&pos, &tile->centre);
					ep->nodes->offset = DotProduct(&pos, &tile->normal);
				}
				else
					ep->nodes->offset = ReadFloat(f);

				EditorAddCreateNode("", i, 0, CREATE_GARIB, ep, 0);
				FreeEditPath(ep);
				break;
			}

		case CREATE_CAMERACASE:
			{
				unsigned long flags, target;
				float scale, animSpeed;
				EDVECTOR pos;

				flags = ReadInt(f);
				ReadVector(&pos, f);
				if( ver > 10 )
				{
					scale = ReadFloat(f);	// field of view
					animSpeed = ReadFloat(f); // Camera slurp speed
					target = ReadInt(f);    // Cam target compressed into a byte EDVECTOR
				}
				group = ReadTileGroup(&state);
				create = EditorAddCameraCase(group, flags, &pos);
				create->scale = scale;
				create->animSpeed = animSpeed;
				create->startNode = target;
				break;
			}
		}
	}
	
	CloseHandle(f);

	if (pathList)
	{
		for (i = 0; i < pathCount; i++)
			FreeEditPath(pathList[i]);
		free(pathList);
	}

	return TRUE;
}

/*	--------------------------------------------------------------------------------
	Programmer		: David Swift
    Function		: SaveCreateList
	Parameters		: const char*, CREATEENTITYNODE*
	Returns			: 
*/

BOOL SaveCreateList(const char* filename, EDITGROUP *list)
{
	HANDLE f;
	CREATEENTITY *create;
	EDITGROUPNODE *node;
	int i, count;
	int counters[5];
//	EDVECTOR v;

	int numPaths = 0;
	EDITPATH *pathBuf[EDITOR_MAXPATH];

	MakeBackup(filename);
	
	f = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (f == INVALID_HANDLE_VALUE) return FALSE;

	if (releaseQuality)
		WriteByte(fileVersion | releaseVersion, f);	
	else
		WriteByte(fileVersion, f);

	// ----------------------- Step 1: Build up a list of paths -------------------------

	for (node = list->nodes; node; node = node->link)
	{
		int found;
		EDITPATH *path;
		CREATEENTITY *create;
		
		create = (CREATEENTITY*)node->thing;

		if (!(create->thing == CREATE_ENEMY || create->thing == CREATE_PLATFORM || create->thing == CREATE_PLACEHOLDER))
			continue;

		found = 0;
		path = create->path;

		for (i=0; i<numPaths; i++)
		{
			if (pathBuf[i] == path) { found = 1; break; }
		}

		if (!found) pathBuf[numPaths++] = path;
	}

	// Write the total number of path nodes in the woooooorrrrrrld

	for (i=0, count=0; i < numPaths; i++)
	{
		EDITPATHNODE *node;
		for(node=pathBuf[i]->nodes; node; node = node->link, count++);
	}
	
	WriteWord(count, f);

	// Write this list to the start of the file

	WriteWord(numPaths, f);
	for (i = 0; i < numPaths; i++)
		WriteEditPath(pathBuf[i], f);
	
	// ----------------------- Step 2: Write the entity list ---------------------------
	
	memset(counters, 0, sizeof(int) * 5);
	for (count = 0, node = list->nodes; node; node = node->link, count++)
	{
		if( ((CREATEENTITY*)(node->thing))->thing == CREATE_CAMERACASE )
			counters[CREATE_CAMERACASE] += CountGroupMembers( ((CREATEENTITY*)(node->thing))->group );
		else
			counters[((CREATEENTITY*)(node->thing))->thing]++;
	}

	for (i=0; i<5; i++)
		WriteWord(counters[i], f);

	WriteWord(count, f);
	
	for (node = list->nodes; node; node = node->link)
	{
		create = (CREATEENTITY*)node->thing;

		WriteByte(create->thing, f);

		switch (create->thing)
		{
		case CREATE_ENEMY:
		case CREATE_PLACEHOLDER:
		case CREATE_PLATFORM:
			count = strlen(create->type);
			WriteByte((BYTE)count, f);
			WriteFile(f, create->type, count, &bytesWritten, NULL);
			WriteInt(create->flags, f);
			WriteInt(create->ID, f);
			WriteFloat(create->scale, f);
			WriteFloat(create->radius, f);
			WriteFloat(create->animSpeed, f);
			WriteFloat(create->value1, f);
			WriteByte(create->facing, f);
			WriteInt(create->objFlags, f);
			
			// version 15+ has 4 bytes of platform-specific info after 'objflags'
			WriteByte(create->PSX_shift, f);
			WriteByte(create->PSX_hack, f);
			for (i=2; i<4; i++) WriteByte(0, f);

			WriteInt(create->effects, f);
			WriteInt(create->startNode, f);

			//WriteEditPath(create->path, f);
			i = FindPtrIndex(create->path, pathBuf, numPaths);
			
			if (i == -1) {
				utilPrintf("Error in path saving!\n"); i = 0;
			}

			WriteWord(i, f);
			break;

		case CREATE_GARIB:
			WriteByte((BYTE)create->flags, f);
			//GetTilePos(&v, create->path->nodes->tile);
			//WriteVector(&v, f);
			WriteTile(create->path->nodes->tile, f, releaseQuality);
			WriteFloat(create->path->nodes->offset, f);
			break;

		case CREATE_CAMERACASE:
			WriteInt(create->flags, f);
			WriteVector(&create->camera, f);
			WriteFloat(create->scale, f);
			WriteFloat(create->animSpeed, f);
			WriteInt(create->startNode, f);

			WriteTileGroup(create->group, f);
			break;
		}
	}

	CloseHandle(f);

	return TRUE;
}
/*	--------------------------------------------------------------------------------
	Programmer		: David Swift
    Function		: WriteEditPath
	Parameters		: EDITPATH*, HANDLE
	Returns			: 
*/

void WriteEditPath(EDITPATH *p, HANDLE f)
{
	int count;
	EDITPATHNODE *node;
//	EDVECTOR v;

	for (count=0, node = p->nodes; node; node = node->link, count++);
	
	WriteInt(count, f);

	for (node = p->nodes; count; node = node->link, count--)
	{
		//GetTilePos(&v, node->tile);
		//WriteVector(&v, f);
		WriteTile(node->tile, f, releaseQuality);
		WriteFloat(node->offset, f);
		WriteFloat(node->offset2, f);
		WriteFloat(node->speed, f);
		WriteInt(node->waitTime, f);
		WriteInt(node->sample, f);
	}
}

/*	--------------------------------------------------------------------------------
	Programmer		: David Swift
    Function		: ReadEditPath
	Parameters		: HANDLE
	Returns			: EDITPATH*
*/

EDITPATH *ReadEditPath(EDLOADSTATE *state)
{
	EDITPATH *path;
	EDITPATHNODE *node;
	int count;
	GAMETILE *tile, *prevtile = NULL;
	HANDLE f = state->f;

	path = CreateEditPath();

	count = ReadInt(f);

	while (count--)
	{
		float offset, offset2;
		int wait;
		unsigned long sample;
		float speed;

		//ReadVector(&v, f);
		tile = ReadTile(state);
		offset = ReadFloat(f);
		offset2 = ReadFloat(f);
		speed = ReadFloat(f);
		wait = ReadInt(f);

		if( state->ver > 13 ) sample = (unsigned long)ReadInt(f);
		else sample = 0;

		//tile = FindNearestTile(v);  
		// if .. || tile == prevtile)
		if (!tile) continue;	// discard invalid path nodes

		node = EditorAddFlag(tile, path);

		node->offset = offset;
		node->offset2 = offset2;
		node->speed = speed;
		node->waitTime = wait;
		node->sample = sample;

		prevtile = tile;
	}

	return path;
}

/*	--------------------------------------------------------------------------------
	Programmer		: David Swift
    Function		: ReadTileGroup
	Parameters		: HANDLE
	Returns			: EDITGROUP* containing GAMETILE*s
*/

EDITGROUP *ReadTileGroup(EDLOADSTATE *state)
{
	EDITGROUP *group;
	GAMETILE *tile;
	int count;
	EDVECTOR v;

	group = MakeEditGroup();

	count = ReadInt(state->f);

	while (count--)
	{
		//ReadVector(&v, f);
		//tile = FindNearestTile(v);
		tile = ReadTile(state);
		GetTilePos(&v, tile);

		if (tile)
			AddGroupMember((void*)tile, &v, group);
	}

	return group;
}

/*	--------------------------------------------------------------------------------
	Programmer		: David Swift
    Function		: ReadTileGroup
	Parameters		: HANDLE
	Returns			: EDITGROUP* containing GAMETILE*s
*/

void WriteTileGroup(EDITGROUP *group, HANDLE f)
{
	EDITGROUPNODE *node;
	int count;

	count = CountGroupMembers(group);

	WriteInt(count, f);

	for (node = group->nodes; count--; node = node->link)
	{
//		GetTilePos(&v, (GAMETILE*)node->thing);
//		WriteVector(&v, f);
		WriteTile((GAMETILE*)node->thing, f, releaseQuality);
	}
}

