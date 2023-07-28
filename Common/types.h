/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: types.h
	Programmer	: Andy Eder
	Date		: 15/04/99 14:17:22

----------------------------------------------------------------------------------------------- */

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED


#ifndef PSX_VERSION
#include <windows.h>
#ifndef LEAVEOUT_MATH
#include <math.h>
#endif
#endif


#ifdef PSX_VERSION
#define E3_DEM
//#define FINAL_MASTER
#endif

//#include "sonylibs.h"

#include <isltex.h>
#include "fixed.h"
//#include "newpsx.h"

#include "ultra64.h"
//#include "block.h"

#include "sonylibs.h"
//#include "islpsi.h"
//#include "psiactor.h"

#ifndef PSX_VERSION
#include <windows.h>
#endif



#define	SCREEN_HT	240

#define X 0
#define Y 1
#define Z 2

#define R	0
#define G	1
#define B	2
#define A	3

#define YES 1
#define NO  0
#define ON  1
#define OFF 0

#define INACTIVE	0
#define ACTIVE		1
#define DEAD		2

// some extra type definitions
#define BYTE	char
#define SHORT	short
#define LONG	long
#define UBYTE	unsigned char
#define uchar	unsigned char
#define VOID	void
#ifdef PSX_VERSION
#define USHORT	unsigned short
#define ULONG	unsigned long
#endif

#define u_char unsigned char
#define u_short unsigned short


#define MOD		%

#define	INIT_ANIMATION	(1 << 0)
#define	INIT_SHADOW		(1 << 1)

//object flags
#define OBJECT_FLAGS_GOURAUD_SHADED			   	(1<<0)		// 1
#define OBJECT_FLAGS_XLU					   	(1<<1)		// 2
#define OBJECT_FLAGS_TRANSPARENT			   	(1<<2)		// 4
#define OBJECT_FLAGS_PRELIT					   	(1<<3)		// 8
#define OBJECT_FLAGS_MIP_MAP				   	(1<<4)		// 16
#define OBJECT_FLAGS_MODGE					   	(1<<5)		// 32
#define OBJECT_FLAGS_TEXTURE_BLEND			   	(1<<6)		// 64
#define OBJECT_FLAGS_IA						   	(1<<7)		// 128
#define OBJECT_FLAGS_PHONG					   	(1<<8)		// 256
#define OBJECT_FLAGS_FADEOUT				   	(1<<9)		// 512
#define OBJECT_FLAGS_FACE_COLOUR			   	(1<<10)		// 1024
#define OBJECT_FLAGS_OBJECT_COLOUR			   	(1<<11)		// 2048
#define OBJECT_FLAGS_TEXTURE_BLEND_ENV		   	(1<<12)		// 4096
#define OBJECT_FLAGS_COLOUR_BLEND			   	(1<<13)		// 8192
#define OBJECT_FLAGS_COLOUR_BLEND_AFTERLIGHT   	(1<<14)		// 16384


//--------------
#define Max(p1,p2)	(((p1) > (p2)) ? (p1) : (p2))
#define Min(p1,p2)	(((p1) > (p2)) ? (p2) : (p1))

#ifndef min
#define min Min
#define max Max
#endif

#define SKINNED_SEG (1<<24)

typedef struct
{
	LONG	x, y, z, w;
}IQUATERNION;

typedef struct
{
	LONG	matrix[4][4];
}MATRIXI;

// typedef struct
// {
// 	float	x, y, z, w;
// }QUATERNION;

// NEW DATA TYPE FOR REDUCING MEMORY REQUIREMENT OF N64 VERSION ----------------------------------

typedef struct
{
	short x,y,z,w;

} QUATERNIONSHORT;

typedef struct
{
	QUATERNIONSHORT	quat;
	int	time;

} KEYFRAMESHORT;

// -----------------------------------------------------------------------------------------------

// typedef struct
// {
// 	float	matrix[4][4];
// }MATRIX;
/*
typedef struct
{
	int v[3];
}VECTOR;

typedef struct
{
	int v[3];
}SVECTOR;
*/

//typedef struct
//{
//	float v[3];
//	float vx,vy,vz;
//}FLVECTOR;

typedef struct
{
	fixed	x,z;
}POINT2D;

typedef struct
{
	SVECTOR	pos;
	VECTOR	size;
}BOX;

typedef struct
{
	signed char	v[4];
}BYTEVECTOR;

typedef struct
{
	UBYTE	v[4];
}UBYTEVECTOR;


typedef struct
{
	unsigned short	vz;
}u162DVECTOR;

typedef struct
{
	short	vz;
}SHORT2DVECTOR;

typedef struct
{
	fixed	vz;
}VECTOR2D;

// typedef struct
// {
// 	short	v[3];
// }SHORTVECTOR;

typedef struct
{
	unsigned short	v[3];
}u16VECTOR;

typedef struct
{
	UBYTE r,g,b,a;
}COLOUR;


typedef struct
{
	FVECTOR	normal;
	fixed	J;
}PLANE;

typedef struct
{
	int	handle, volume, actualVolume,origVolume;
	short pitch,sampleNum;
	SVECTOR *pos;
	fixed radius;
}SFX;

typedef struct
{
	int volume;
	short pitch,sampleNum;
}SPOT_SFX;

//======================texture type
/*typedef struct
{
	int		texID;
	UBYTE	cycleStart;
	UBYTE	cycleEnd;
	short	flags;
	short	cycleSpeed;
	short	cycleCount;
	unsigned short	sx;
	unsigned short	sy;
	unsigned short	TCScaleX;						PC>>>>>>>>>>>>>>>>>>>>>>>>>>>
	unsigned short	TCScaleY;
	int		size;
	short	format;
	short	pixSize;
	char	*data;
	short	*palette;
}TEXTURE;*/


typedef struct
{
	SVECTOR	offset;
	fixed	radius;
	fixed	radiusAim;
	SVECTOR	oldOffset;
	UBYTE	posTag;
	UBYTE	quatTag;
	short	flags;

}COLLSPHERE;


/*typedef struct
{
	short			numFaces;
	short			numVertices;
	SVECTOR			*vertices;
	SHORTVECTOR		*faceIndex;
	BYTEVECTOR		*faceNormals;
	u162DVECTOR	*faceTC;
	u162DVECTOR	*originalFaceTC;
	BYTEVECTOR		*vertexNormals;
	char		 	*faceFlags;
	TEXTURE			**textureIDs;

}MESH;*/


typedef struct
{
	VECTOR	vect;
	int	time;
}VKEYFRAME;

typedef struct
{
	SVECTOR	vect;
	int	time;
}SKEYFRAME;

// typedef struct
// {
// 
// 	QUATERNION	quat;
// 	int	time;
// }QKEYFRAME;

typedef struct
{
	fixed			radius;
	UBYTE			alpha;
	UBYTE			alphaAim;
	UBYTE			alphaAimSpeed;
	char			draw;//mm
} ACTOR_SHADOW;


//----------------animation
typedef struct 
{
	short	   animStart, animEnd;
	fixed	   speed;
	int		   loop;
} animation;

//#define ANIM_QUEUE_LENGTH 5

// typedef struct
// {
// 	short		numAnimations;
// 	short		currentAnimation;
// 	BOOL		reachedEndOfAnimation;
// 	BOOL		loopAnimation;
// 	fixed		animationSpeed;
// 	short		queueAnimation[ANIM_QUEUE_LENGTH];
// 	BOOL		queueLoopAnimation[ANIM_QUEUE_LENGTH];
// 	fixed		queueAnimationSpeed[ANIM_QUEUE_LENGTH];
// 	short 		numberQueued;
// 	animation	*anims;
// 	fixed		animTime;
// 	char		queueNumMorphFrames[ANIM_QUEUE_LENGTH];
// 	BYTE		numMorphFrames;
// 	char		currentMorphFrame;
// 	fixed		morphTo;
// 	fixed		morphFrom;
// 
// }ACTOR_ANIMATION;

typedef struct
{
	TextureType **texture;
	short	numFrames;
	short	overrideNumFrames;
	char	filename[16];

}FRAMELIST;

typedef struct
{
	FRAMELIST	*frameList;
	UBYTE		delay;
	UBYTE		type;
	UBYTE		startAlpha;
	UBYTE		endAlpha;
	u16			startScale;
	u16			endScale;
	u16			spriteFlags;
	short		pad;

}SPRITE_ANIMATION_TEMPLATE;

typedef struct
{
	FRAMELIST	*frameList;
	UBYTE		delay;
	UBYTE		type;
	UBYTE		startAlpha;
	UBYTE		endAlpha;
	u16		startScale;
	u16		endScale;
	u16		spriteFlags;
	u16		lifespan;
	u16		currentFrame;
	fixed		counter;
	fixed		lifetime;

}SPRITE_ANIMATION;

//bbupdate now in sprite.h
// typedef struct TAGSPRITE
// {
// 	struct TAGSPRITE *next,*prev;
// 
// 	TextureType *texture;
// 	//VERT pos;
// 	SVECTOR pos;
// 	short scaleX;
// 	short scaleY;
// 	short flags;
// 	UBYTE r,g,b,a;
// 	UBYTE red2,green2,blue2,alpha2;
// 	BYTE  offsetX;
// 	BYTE  offsetY;
// 	SPRITE_ANIMATION anim;
// 
// 	char kill;
// 
// }SPRITE;

//bbupdate - now in sprite.h
// typedef struct OBJECTSPRITE
// {
// 	TextureType		*textureID;
// 	SPRITE		*sprite;		//actual sprite in sprite list
// 	short		x, y, z;
// 	short		sx;	//xsize - 32 is normal
// 	short		sy;	//ysize - 32 is normal
// 	short		ox;	//offset from centre
// 	short		oy;	//offset from centre
// 	short		flags;
// 
// }OBJECTSPRITE;


typedef struct
{
	FRAMELIST	*frameList;
	UBYTE		delay;
	UBYTE		flags;
	UBYTE		currentFrame;
	BYTE		counter;

}TEXTURE_ANIMATION;

typedef struct
{
	s16		*verts;
	char	numVerts;

}SKINVTX;

typedef struct
{
	fixed	v[3];
	s8		n[3];

}DUELVECTOR; 


// typedef struct PSIOBJECT
// {
// 	u32			objID;
// 	u8			name[16];
// 	short		pixelOutAlpha;
// 	u8			xlu;
// 
// 	u8			numSprites;
// 	OBJECTSPRITE	*sprites;
// 
// 	//for non skinned objects
// 	MESH		*mesh;
// 	Gfx			*drawList;
// 
// 	TEXTURE_ANIMATION *textureAnim;
// 	//for skinning
// 	SKINVTX		*effectedVerts;
// 	DUELVECTOR	*originalVerts;
// 
// 	short		numVerts;
// 
// 
// 	VKEYFRAME	*scaleKeys;
// 	VKEYFRAME	*moveKeys;
// 
// 	QKEYFRAME	*rotateKeys;
// 
// 	short		numScaleKeys;
// 	short		numMoveKeys;
// 	short		numRotateKeys;
// 
// 	COLOUR		colour;
// 	void		*renderData;
// 
// 	short		flags;
// 
// 	short		numChildren;
// 	struct PSIOBJECT	*children;
// 	struct PSIOBJECT	*next;
// 
// 	COLLSPHERE	*collSphere;
// 	MATRIX		objMatrix;
// 
// }PSIOBJECT;
// 
// typedef struct
// {
// 	int		objectID;
// 
// 	Mtx		*matrix;
// 	short	objectSize;
// 	PSIOBJECT	*object;
// 	fixed	radius;
// 	PLANE	*planes;
// 	ACTOR_ANIMATION	*animation;
// 	Gfx		*drawList;
// 	Vtx		*vtx[2];
// 	MESH	*LOCMesh;
// 	short	vtxBuf;
// 	short	numVtx;
// 
// }OBJECT_CONTROLLER;
// 
// 
// typedef struct
// {
// 	int		objectID;
//  	OBJECT_CONTROLLER	*objCont;
// }OBJECT_DESCRIPTOR;
// 
// 
// typedef struct
// {
// 	char	*data;
// 	char	*freePtr;
// 	int		numObjects;
// 	OBJECT_DESCRIPTOR	*objList;
// }OBJECT_BANK;

// typedef struct TAGACTOR
// {
// 	struct TAGACTOR				*next,*prev;
// 	fixed						distanceFromCamera;
// 
// 	Mtx							*matrix;
// 	BYTE						status;
// 	short						xluOverride;
// 	int							flags;
// 
// 	fixed						maxRadius;
// 
// 	SVECTOR						pos;
// 	SVECTOR						oldpos;
// 	FVECTOR						vel;
// 	VECTOR						rot;
// 	SVECTOR						rotaim;
// 								
// 	QUATERNION					qRot;
// 												
// 	FVECTOR	  					scale;
// 
// 	OBJECT_CONTROLLER			*objectController;
// 	OBJECT_CONTROLLER			*LODObjectController;
// 	ACTOR_SHADOW				*shadow;
// 	ACTOR_ANIMATION				*animation;
// 	int							visible;
// 
// } ACTOR;

typedef struct TAGPLANE2
{
	fixed	distance;
	fixed	J;
	SVECTOR	point;
	FVECTOR	normal;

} PLANE2;


typedef struct TAGPOLY
{
	struct TAGPOLY *next;

	PLANE2 plane;
	unsigned char r, g, b, a;
	TextureType *tex;
	VECTOR *vT;

} POLYGON;


#ifdef __cplusplus
extern "C" {
#endif

//#define dprintf debugPrintf(sprintf(outputMessageBuffer, 

#define bprintf sprintf(statusMessage,

extern char outputMessageBuffer[];

extern Gfx *glistp;

VECTOR  MakeVECTOR(long x, long y, long z);
SVECTOR MakeSVECTOR(short x, short y, short z);
FVECTOR MakeFVECTOR(fixed x, fixed y, fixed z);
//FLVECTOR MakeFLVECTOR(fixed x, fixed y, fixed z);


#ifdef __cplusplus
}
#endif
#endif