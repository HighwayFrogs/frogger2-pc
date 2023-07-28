#ifndef _WORLDEFF_H_INCLUDE
#define _WORLDEFF_H_INCLUDE


//#define SHIFT_DEPTH ( 1 << 0 )
//#define TRANSPARENT ( 1 << 1 )
//#define SUBTRACTIVE ( 1 << 2 )
//#define ADDATIVE	  ( 1 << 3 )


#include "sonylibs.h"

#include "ptexture.h"
#include "map.h"

void DrawWater ( FMA_MESH_HEADER *mesh, int flags );





typedef struct _SCENICOBJ
{
	struct _SCENICOBJ *next,*prev;			// ptr to next / prev platform

	int						flags;
	FMA_WORLD*		fmaObj;
//	SVECTOR				position;

	//bb
	MATRIX matrix;

} SCENICOBJ;

typedef struct _SCENICOBJLIST
{
	int				numEntries;
	SCENICOBJ	head;
} SCENICOBJLIST;

extern SCENICOBJLIST scenicObjList;

extern SVECTOR jiggledVerts[250];


extern FMA_GT4 *lightBeamGouraudValues;

//void CreateAndAddScenicObject ( char *name, short posx, short posy, short posz, int newFlags );
SCENICOBJ *CreateAndAddScenicObject(SCENIC *sc);

void InitScenicObjLinkedList ( void );

void AddScenicObj ( SCENICOBJ *scenicObj );

void FreeScenicObjectList ( void );

void SubScenicObject ( SCENICOBJ *scenicObj );

void DrawScenicObj ( FMA_MESH_HEADER *mesh, int flags );

// JH: Loads in the procedual textures
void PTTextureLoad ( void );

// JH: Creates a procedual texture from a given texture name
void CreateProceduralTexture ( char *name );

// JH: Blits the new texture buffer on to the video memory of the texture.
void PTSurfaceBlit( TextureType *tex, unsigned long *buf, unsigned short *pal );

void PrintStaticBackdrop( FMA_MESH_HEADER *mesh );

void SetObjectGouraudValues ( FMA_MESH_HEADER *mesh, FMA_GT4 *gouraudList );

#endif