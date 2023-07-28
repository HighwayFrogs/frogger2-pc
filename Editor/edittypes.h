#ifndef _EDITTYPES_H_INCLUDED
#define _EDITTYPES_H_INCLUDED

/* ------------------------------------------------------------------------- */

#include <define.h>

typedef struct _EDVECTOR { float vx, vy, vz; } EDVECTOR;

#define	FSCALE (1.0f/4096.0f)

#define S2EDVECTOR(s, v) (v).v[0]=(float)((s).vx * SCALE),(v).v[1]=(float)((s).vy*SCALE),(v).v[2]=(float)((s).vz*SCALE)
#define F2EDVECTOR(f, v) (v).v[0]=(float)(f).vx*FSCALE, (v).v[1]=(float)(s).vy*FSCALE, (v).v[2]=(float)(f).vz*FSCALE

#define MAXTYPELENGTH 20
#define CE_NUMVALUES 8

typedef struct TAGEDITPATHNODE
{
	struct TAGEDITPATHNODE *link;
	float speed, offset, offset2;
	int start, waitTime, level;
	unsigned long sample;

	GAMETILE *tile;

} EDITPATHNODE;

typedef struct TAGEDITPATH
{
	//struct TAGEDITPATH *prev, *next;
	int refs;				// number of references - dealloc when 0
	EDITPATHNODE *nodes;
} EDITPATH;

typedef struct TAGEDITGROUPNODE
{
	struct TAGEDITGROUPNODE* link;
	//int icon;
	EDVECTOR pos;
	void* thing;
} EDITGROUPNODE;

typedef struct TAGEDITGROUP
{
	EDITGROUPNODE* nodes;
	int refs;
} EDITGROUP;

typedef struct TAGCREATEENTITY
{
	char	type[MAXTYPELENGTH];
	UBYTE	thing;

	EDITGROUP* group;
	EDITPATH* path;

	unsigned char facing;
	int		flags, ID, startNode, effects, objFlags;
	float	scale, radius, animSpeed, value1; // Value1 is quite random
	EDVECTOR	camera;

	char	PSX_shift;	// sorting hack value for PSX
	char	PSX_hack;	// temp HACK for PSX

} CREATEENTITY;

typedef struct _KMAP
{
	char key;
	int command;
} KMAP;

typedef struct tagEDLOADSTATE
{
	HANDLE f;
	int ver, fastLoad;
	EDITPATH **pathList;
	int numPaths;
} EDLOADSTATE;

typedef struct { int icon, command; } TOOLBAR_ENTRY;
typedef TOOLBAR_ENTRY *TOOLBAR_SUBMENU;

typedef enum {
	EDITVAR_STRING,
	EDITVAR_INT,
	EDITVAR_FLOAT,
	EDITVAR_PASSTOFUNCTION,
	EDITVAR_GROUPFUNCINT,
	EDITVAR_GROUPFUNCSTR,
	EDITVAR_GROUPFUNCFLOAT } EDITVAR;

typedef enum {
	CREATE_ENEMY,
	CREATE_PLATFORM,
	CREATE_GARIB,
	CREATE_CAMERACASE,
	CREATE_PLACEHOLDER } CREATETYPE;

/* ------------------------------------------------------------------------- */

#endif
