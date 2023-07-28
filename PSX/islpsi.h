/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islpsi.h			Skinned model routines

************************************************************************************/


#ifndef __ISLPSI_H__
#define __ISLPSI_H__


// lighting types
enum
{
	NOLIGHTING,
	DIRECTIONAL,
	DIRECTIONONLY,
	COLOURIZE,
	AMBIENT,
};

// flags for PSIDATA
#define ACTOR_DYNAMICSORT	1
#define ACTOR_HOLDMOTION	2
#define ACTOR_BONED			4
#define ACTOR_MOTIONBONE	8
#define PSIACTOR_TRANSPARENT 16
//bb
#define PSIACTOR_SCALED		32


// keyframe data structures
typedef struct
{
	SHORT x,y,z,w;
} SHORTQVECTOR;

typedef struct
{
	SHORT x,y,z;
} SHORTVECTOR;

typedef struct
{
	SHORTQVECTOR vect;
	SHORT time;
} SQKEYFRAME;

typedef struct
{
	SHORTVECTOR vect;
	SHORT time;
} SVKEYFRAME;


typedef struct
{
	SVKEYFRAME *lastscalekey;
	SVKEYFRAME *lastmovekey;
	SQKEYFRAME *lastrotatekey;
} LASTKEYS;

// PSI mesh data structure
typedef struct _PSIMESH
{

	VERT *vertop;  	       	// vertex top address
	u_long  vern;          	// the number of vertices
	VERT *nortop;			// normal top address
	u_long  norn;           // the number of normals
	u_long  scale;          // the scale factor of TMD format

	UBYTE			name[16];
	struct _PSIMESH	*child;
	struct _PSIMESH	*next;

	USHORT			numScaleKeys;
	USHORT			numMoveKeys;
	USHORT			numRotateKeys;
//	USHORT			pad1;
	USHORT			lastKeysOff;	// offset to  LASTKEYS structure.

	SVKEYFRAME		*scalekeys;
	SVKEYFRAME		*movekeys;
	SQKEYFRAME		*rotatekeys;
	USHORT			sortlistsize[8];
	USHORT			*sortlistptr[8];
	SVECTOR			center;

	//bbopt - for optimising animation
	//can't store these here, because it's block loaded data.
	//damn it.
//	SVKEYFRAME *lastscalekey;
//	SVKEYFRAME *lastmovekey;
//	SQKEYFRAME *lastrotatekey;

} PSIMESH;


// PSI object data structure
typedef struct _PSIOBJECT
{

	MATRIX  	matrixscale;		//not needed
	MATRIX  	matrix;

	PSIMESH		*meshdata;		//sort list size and pointers included in TMD

	struct _PSIOBJECT	*child;
	struct _PSIOBJECT	*next;

	
 	SVECTOR		rotate;			// calculated angle of object vx,vy,vz
  VECTOR		scale;			// calculated scale


	int	depth;

} PSIOBJECT;

typedef struct _PSIDATA
{
	unsigned long	flags;
	unsigned long	numObjects;
	unsigned long	noofVerts;
	unsigned long	noofPrims;
	unsigned long	primitiveList;
	unsigned long	*objectTable;
	PSIOBJECT		*object;
	char			*modelName;
} PSIDATA;


// PSI model structure from Jobe
typedef struct
{

	char	id[4];			// "PSI",0

	long	version;		// version number

	long   	flags;			// 

	char	name[32];		// model name

	long	noofmeshes;		// number of objects in this model

	long	noofVerts;		// number of vertices in this model

	long	noofPrims;		// primitive list size
	long	primOffset;		// primitive list offset from start of file

	USHORT 	animStart;		// first frame number
	USHORT 	animEnd;		// last frame number

	long	animSegments;	// noof anim segments
	long	animSegmentList;// offest (from file start) to frame list


	long	noofTextures;	// number of textures
	long	textureOffset;	// offset to texture names

	long	firstMesh;		// offset to mesh data

	long	radius;			// max radius this model ever reaches

	char	pad[192-88];

} PSIMODEL;


// RGB colour struct
typedef struct {
	u_char r, g, b;
} RGB;


// model control struct
typedef struct
{
	USHORT 		depthoverride;	//0=sort normally, otherwise model will be at this depth;
	UBYTE  		specialmode;	//OFF addhue
	UBYTE		onmap;			//YES or NO;
	RGB			col;			//override RGB of model
	UBYTE		sprites;		//ON or OFF attached sprites on model
	UBYTE		alpha;			//0 or 1 (NO or YES)	//	was UBYTE	lighting;	//OFF ON (not used)
	UBYTE		semitrans;		//0 or 2 for ON!
	USHORT		PrimLeft;
	int			*SortPtr;
	ULONG		*PrimTop;
	VERT		*VertTop;
	VERT		*NormTop;
	LONG		polysdrawn;
	LONG		polysclipped;
	LONG		preclipped;
	USHORT		lastdepth;
	USHORT		nearclip;		//default 100
	long		farclip;
	GsRVIEW2	*whichcamera;			
	USHORT		sorttable;
	USHORT		*SortOffs;
	UBYTE		lighting;		// lighting mode
	int			depthShift;		// 0== use default shift !0=shift value
	UBYTE		inheritScale;	// 0 = off, !0 = on

} PSIMODELCTRL;


typedef struct
{
	char *modelName;
	int done;
	void *actor;
} ACTORSETANIM;

extern int globalCount;
extern ACTORSETANIM globalActors [ 50 ];

// control variables

extern PSIMODELCTRL	PSImodelctrl;

extern VECTOR *PSIactorScale;

extern VECTOR *PSIrootScale;

extern long *pilLibraryList[8];

// workspace variables
extern int	biggestVertexModel;		// number of verts in largest model
extern long *transformedVertices;	// pointer to transformed vert workspace
extern long *transformedDepths;		// pointer to transformed depth workspace
extern VERT *transformedNormals;	// pointer to transformed normal workspace

extern int totalObjs;

// function prototypes

/**************************************************************************
	FUNCTION:	psiInitialise()
	PURPOSE:	Initialise PSI library
	PARAMETERS:	Maximum number of models
	RETURNS:	
**************************************************************************/

void psiInitialise(int maxModels);


/**************************************************************************
	FUNCTION:	psiDestroy()
	PURPOSE:	Shut down PSI library
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void psiDestroy();


/**************************************************************************
	FUNCTION:	psiLoad()
	PURPOSE:	Load a PSI file
	PARAMETERS:	filename
	RETURNS:	pointer to a PSIMODEL
**************************************************************************/

PSIMODEL *psiLoad(char *psiName);


/**************************************************************************
	FUNCTION:	psiFixup()
	PURPOSE:	Fix up pointers in a PSIMODEL file
	PARAMETERS:	pointer to model in memory
	RETURNS:	Pointer to a PSIMODEL
**************************************************************************/

PSIMODEL *psiFixup(char *addr);


/**************************************************************************
	FUNCTION:	psiInitLights()
	PURPOSE:	Initialise hardware lights
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void psiInitLights();


/**************************************************************************
	FUNCTION:	psiSetLight()
	PURPOSE:	Set a hardware light
	PARAMETERS:	light number (0-2), colour, direction
	RETURNS:	
**************************************************************************/

void psiSetLight(int lightNum, int r, int g, int b, int x, int y, int z);


/**************************************************************************
	FUNCTION:	psiSetAmbient()
	PURPOSE:	Set hardware ambient colour
	PARAMETERS:	colour (0-4095)
	RETURNS:	
**************************************************************************/

void psiSetAmbient(int r, int g, int b);


/**************************************************************************
	FUNCTION:	psiObjectScan()
	PURPOSE:	Find a specific part of a model
	PARAMETERS:	Pointer to root object, name of object in hierarchy
	RETURNS:	Pointer to object
**************************************************************************/

PSIOBJECT *psiObjectScan(PSIOBJECT *obj, char *name);


/**************************************************************************
	FUNCTION:	psiSetKeyFrames()
	PURPOSE:	Set the keyframes for an object and its siblings/children
	PARAMETERS:	Pointer to object, frame number
	RETURNS:	
**************************************************************************/

void psiSetKeyFrames(PSIOBJECT *world, ULONG frame);


/**************************************************************************
	FUNCTION:	psiSetKeyFrames2()
	PURPOSE:	Set the keyframes for an object and its siblings/children
	PARAMETERS:	Pointer to object, source frame number, destination frame number, blend value
	RETURNS:	
**************************************************************************/

void psiSetKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG blend);


/**************************************************************************
	FUNCTION:	psiSetKeyFrames3()
	PURPOSE:	Set the keyframes for an object and its descendants
	PARAMETERS:	Pointer to object, source frame number, destination frame number, blend value
	RETURNS:	
**************************************************************************/

void psiSetKeyFrames3(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG blend);


/**************************************************************************
	FUNCTION:	psiInitSortList()
	PURPOSE:	Initialise the internal sort list
	PARAMETERS:	Range of values
	RETURNS:	
**************************************************************************/

void psiInitSortList(int range);


/**************************************************************************
	FUNCTION:	psiCalcWorldMatrix()
	PURPOSE:	Calculate the matrices ready for drawing an object
	PARAMETERS:	Pointer to root object
	RETURNS:	
**************************************************************************/

void psiCalcWorldMatrix(PSIOBJECT *world);


/**************************************************************************
	FUNCTION:	psiCalcLocalMatrix()
	PURPOSE:	Calculate the matrices ready for getting position of an object
	PARAMETERS:	Pointer to root object
	RETURNS:	
**************************************************************************/

void psiCalcLocalMatrix(PSIOBJECT *world);


/**************************************************************************
	FUNCTION:	psiDrawSegments()
	PURPOSE:	Draw an objects mesh segments
	PARAMETERS:	Pointer to psiData
	RETURNS:	
**************************************************************************/

void psiDrawSegments(PSIDATA *psiData);


/**************************************************************************
	FUNCTION:	psiRegisterDrawFunction()
	PURPOSE:	Register a custom draw function with the library for dynamically sorted models
	PARAMETERS:	Pointer to draw function
	RETURNS:	
**************************************************************************/

void *psiRegisterDrawFunction(void (*drawHandler)(int));


/**************************************************************************
	FUNCTION:	psiRegisterDrawFunction2()
	PURPOSE:	Register a custom draw function with the library for non-dynamically sorted models
	PARAMETERS:	Pointer to draw function
	RETURNS:	
**************************************************************************/

void *psiRegisterDrawFunction2(void (*drawHandler)(int));


/**************************************************************************
	FUNCTION:	psiLoadPIL()
	PURPOSE:	load psi library and ALL models in it, fix up ponters and load textures
	PARAMETERS:	filename.pil
	RETURNS:	pointer to PSIMODEL
**************************************************************************/


void *psiLoadPIL(char *pilName);


/**************************************************************************
	FUNCTION:	psiAllocWorkspace()
	PURPOSE:	allocate workspace for model drawing
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void psiAllocWorkspace();


/**************************************************************************
	FUNCTION:	transformVertexListA()
	PURPOSE:	transform a list of vertices into screen space
	PARAMETERS:	pointer to verts, number of verts, pointer to screen x&y's, pointer to screen z's
	RETURNS:	
**************************************************************************/

void transformVertexListA(VERT *vertPtr, long numVerts, long *transformedVerts, long *transformedDepths);


extern PSIMODEL *psiCheck(char *psiName);

#endif //__ISLPSI_H__
