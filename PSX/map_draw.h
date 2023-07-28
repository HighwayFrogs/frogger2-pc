
#ifndef _MAPDRAW_H_
#define _MAPDRAW_H_


// the "depend" batch doesn't like non-existant headers
//	#include "global.h"
	#include "types.h"
//	#include <islpsi.h>
	#include <psitypes.h>
	#include "shell.h"

#include "bff_load.h"


#define DRAW_SEGMENT  ( 1 << 0 )


// NB - The structures defined in here must match those defined in the PC map compilation tool!

// BFF header types for frog-map segments


#define USE_NEW_LIBS
#define BFF_FMA_MESH_ID (('F'<<0) | ('M'<<8) | ('M'<<16) | (0<<24))
#define BFF_FMA_WORLD_ID (('F'<<0) | ('M'<<8) | ('W'<<16) | (0<<24))


extern long *transformedVertices;
extern long *transformedDepths;

extern int worldPolyCount;


// frog-map versions of the playstation quad/tri structures

typedef struct FMA_GT3
{
	u_char	r0, g0, b0, code;
	u_char	u0, v0;	u_short	clut;
	u_char	r1, g1, b1, p1;
	u_char	u1, v1;	u_short	tpage;
	u_char	r2, g2, b2, p2;
	u_char	u2, v2;	u_short	pad2;
	short vert0,vert1,vert2, pad3;
}FMA_GT3;

typedef struct FMA_GT4
{
	u_char	r0, g0, b0, code;
	u_char	u0, v0;	u_short	clut;
	u_char	r1, g1, b1, p1;
	u_char	u1, v1;	u_short	tpage;
	u_char	r2, g2, b2, p2;
	u_char	u2, v2;	u_short	pad2;
	u_char	r3, g3, b3, p3;
	u_char	u3, v3;	u_short	pad3;
	short vert0,vert1,vert2,vert3;
}FMA_GT4;

typedef struct FMA_SPR
{
	u_char	r0, g0, b0, code;
  short	x, y, z;
  u_short	tpage;
	u_char u0, v0;
	u_char u1, v1;
	u_char u2, v2;
	u_char u3, v3;
  u_short clut; u_char w, h;


	/*u_char	r0, g0, b0, code;
	u_char	u0, v0;	u_short	clut;
	u_char	r1, g1, b1, p1;
	u_char	u1, v1;	u_short	tpage;
	u_char	r2, g2, b2, p2;
	u_char	u2, v2;	u_short	pad2;
	u_char	r3, g3, b3, p3;
	u_char	u3, v3;	u_short	pad3;
	int vert0,vert1,vert2,vert3;*/
}FMA_SPR;


// Structure containing all the data for a single mesh object
// (A collection of which make up the map)

#define MESH_ISWATER	1

typedef struct FMA_MESH_HEADER
{
// start with a BFF header
	unsigned long id;
	int length;
	unsigned long name_crc;
	unsigned long flags;
	unsigned long shift;

// Follow that up with a bounding box
	int minx,miny,minz, maxx,maxy,maxz;

// pos/rotate, or matrix... Dunno.
// For the MAP (as opposed to platforms), the position will always be zero
	int posx,posy,posz;
	short rotx,roty,rotz,rotw;

// All map meshes are sorted to a position further away than their actual position,
// so that objects can sit on top of them nicely.
// "Underwater" meshes should have this set higher than that of the water itself.
	short extra_depth;

// CPW -  I've added this in on 8/12/99
//	short flags;

// Finally, vertices, triangles, quads, and texture names.
// These are stored as integers in the file, and are converted to pointers after loading.
	int n_verts;
//	VERT *verts;
	SVECTOR *verts;

	int n_gt3s;
	FMA_GT3 *gt3s;
	int n_gt4s;
	FMA_GT4 *gt4s;

	int n_sprs;
	FMA_SPR *sprs;

	int n_tmaps;
	unsigned long *tmap_crcs;

}FMA_MESH_HEADER;


// Structure containing a list of FMA_MESH_HEADER pointers, and not much else

typedef struct FMA_WORLD
{
// start with a BFF header
	unsigned long id;
	int length;
	unsigned long name_crc;

// and there really ain't much else...
	int n_meshes;
// Followed immediately by an array of meshname CRC's to be replaced with mesh pointers
}FMA_WORLD;


//bb- these were in the c file,
//I moved them here so I could use them outside,
//when calling FmaActor_GetSBox()
typedef struct{
	SHORT x,y,z,w;
}SHORTQUAT;

typedef struct
{
	short x,y;
}SHORTXY;


#include "world_eff.h"
#include "water.h"

// =========== Hurrah... Some functions! =========
// Draw the whole world map
void MapDraw_DrawFMA_World(FMA_WORLD *world);

// Draw a platform mesh
void MapDraw_DrawFMA_Mesh(FMA_MESH_HEADER *mesh);
//void MapDraw_DrawFMA_Water ( WATER *cur );

int MapDraw_ClipCheck(FMA_MESH_HEADER *mesh);
int FmaActor_ClipCheck(FMA_MESH_HEADER *mesh);
void FmaActor_GetSBox(FMA_MESH_HEADER *mesh, SHORTXY *sBox);

void MapDraw_SetPlatformMatrix(FMA_MESH_HEADER *mesh);

#endif
