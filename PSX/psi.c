/************************************************************************************
	PSX CORE (c) 1999 ISL

	psi.c:		Playstation Model (i) Handler

************************************************************************************/

// JH: Define this so that we can turn on and off the dcache usage
#define DCACHE_OPTIMISATION
#define DCACHE 0x1f800000

#include <stddef.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <inline_c.h>
#include <gtemac.h>
#include <stdio.h>
#include "shell.h"
#include "islpsi.h"
#include "psitypes.h"
#include "quatern.h"
#include <islmem.h>
#include <islfile.h>
#include <isltex.h>
#include <islutil.h>
#include "timer.h"
#include "flatpack.h"
//#include "main.h"

#define MALLOC0(S)	memoryAllocateZero(S, __FILE__, __LINE__)

extern ACTORSETANIM globalActors [ 50 ];

typedef struct{
	SHORT x,y,z,w;
}SHORTQUAT;

int totalObjs = 0;

	#define GETX(n)( ((SHORTXY *)( (int)(tfv) +(n) ))->x )
	#define GETY(n)( ((SHORTXY *)( (int)(tfv) +(n) ))->y )
	#define GETV(n)(  tfv[n] )
	#define GETD(n)(  tfd[n] )
	#define GETN(n)( ((VERT *)( (int)(tfn) +(n<<1) )) )

#define gte_stopz_cpu(r)\
asm(\
	"mfc2 %0, $24;"\
	:\
	: "r"(r)\
)

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

// an assembler version of transformVertexList()
void transformVertexListA(VERT *vertPtr, long numVerts, long *transformedVerts, long *transformedDepths);

void ShortquaternionGetMatrix(SHORTQUAT *squat, MATRIX *dmatrix);

void ShortquaternionSlerpMatrix(SHORTQUAT *src1, SHORTQUAT *sp2, ULONG t,MATRIX *dmatrix);


#define Bound(a,x,b) min(b,max(a,x))
/***************************************************************************************************/

//#define SHOWNORM 1
/***************************************************************************************************/

MATRIX	cameraAndGlobalscale;
VECTOR *PSIactorScale = 0;
VECTOR *PSIrootScale = 0;

GsF_LIGHT	flatLight[3];
MATRIX		lightMat;
SVECTOR		lightRotVec;



PSIMODEL *currentPSI;
char *PSIname=0;

//#define transformedNormals  ((long*) getScratchAddr(0))
long *transformedVertices; //__attribute__ ((section("cachedata"))) = 0;
long *transformedDepths; //__attribute__ ((section("cachedata"))) = 0;
VERT *transformedNormals;// __attribute__ ((section("cachedata"))) = 0;

int  tfTotal = 0;

#ifdef _DEBUG
long *transformedScreenN = 0;
LINE_F2	polyLine;
#endif

int	biggestVertexModel=0;
int	biggestPrimModel=0;

int biggestSortDepth=0;
int smallestSortDepth=0;


static int MAXMODELS;

PSIMODEL **psiModelList;
//char psiModelListNames[MAXMODELS][32];
long *psiModelListCRC;
int	psiModelListLen = 0;

long *pilLibraryList[8];
int pilLibraryLen = 0;

#define gte_SetLDDQA( r0 ) __asm__ volatile (		\
	"ctc2	%0, $27"					\
	:							\
	: "r"( r0 ) )

#define gte_SetLDDQB( r0 ) __asm__ volatile (		\
	"ctc2	%0, $28"					\
	:							\
	: "r"( r0 ) )


#define ADD2POINTER(a,b) ( (void *) ( (int)(a) + (int)(b))  )

#define DEFAULTDEPTHSHIFT 1

typedef struct {
	char tname[32];
} PSITEXTURES;

PSITEXTURES	*currentTextureList;

PSIMODELCTRL	PSImodelctrl;

int maxDepthRange=0;

ULONG *sortedIndex=0;//[DEPTHINIT];
int sortCount=0;

long maxDepth=0;
long minDepth=0;
int depthRange=0;

static void (*customDrawFunction)(int) = 0;
static void (*customDrawFunction2)(int) = 0;



void dcachePsiCalcMaxMin ( PSIDATA *psiData, long *tfd )
{
	int		i;
	long	localmaxDepth;
	long	localminDepth;
	ULONG *sortedIndexPtr;

	sortCount = 0;
	localmaxDepth = localminDepth = *tfd;
	
	i = tfTotal;

	while (i)
	{
		if (*tfd < localminDepth)
			localminDepth = *tfd;
		if (*tfd > localmaxDepth)
			localmaxDepth = *tfd;

		tfd ++;
		i --;
	}

	maxDepth = localmaxDepth;
	minDepth = localminDepth;
	depthRange = (localmaxDepth - localminDepth) + 1;

	sortedIndexPtr = sortedIndex;
	for ( i = depthRange; i != 0 ; i-- )
	{
		*(sortedIndexPtr++) = 0;
	}



	if (depthRange > maxDepthRange)
	{
		printf("PROGRAM HALTED. \nDEPTH RANGE EXCEEDED !\nINCREASE MAXDEPTHRANGE TO %d (=%d)\n\n",depthRange,maxDepthRange);
// RFW: Jobe never filled in the object name anyway, and now I've altered actorCreate to use "modelName" to point to the MODEL
// (Which gives much more useful information, (absent name included)
//		printf("Object Mesh Name = >%s<\n",psiData->modelName);
		for (;;);
	}

}

void dcachePsiSortPrimitives ( long *tfv, long *tfd )
{
	register long clipflag asm("$20");
//	register long *tfd asm("$19");
//	register long *tfv  asm("$18");
	register TMD_P_GT4I	*opcd;
	int				*sl;
	int localSortCount;

	long deep;
	int primsleft;
	int localMinDepth = minDepth;

	(TMD_P_GT3I*)sl = &sortedIndex[0];

	primsleft = PSImodelctrl.PrimLeft;
	opcd = (TMD_P_GT4I*)(PSImodelctrl.PrimTop);

	localSortCount = sortCount;

//	tfv =transformedVertices;
//	tfd =transformedDepths;

	while(primsleft)
	{
		switch (opcd->cd & (0xff - 2))
		{
/*-----------------------------------------------------------------------------------------------------------------*/
#define op ((TMD_P_FT3I*)opcd)
			case GPU_COM_TF3:
						
				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles
				
				deep = (GETD(op->v2) - localMinDepth);

				gte_stopz_cpu(clipflag);
					
				if ((clipflag < 0))	// || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					localSortCount ++;
				}
				op ++;
				break;

#undef op			
/*-----------------------------------------------------------------------------------------------------------------*/
#define op ((TMD_P_FT4I*)opcd)
			case GPU_COM_TF4:
			
				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles
				
				deep = (GETD(op->v2) - localMinDepth);

				gte_stopz_cpu(clipflag);
				
				if ((clipflag < 0))	// || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					localSortCount ++;
				}
				op ++;
				break;
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define op ((TMD_P_GT3I*)opcd)
			case GPU_COM_TG3:
				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles

				deep = (GETD(op->v2) - localMinDepth);	// fill in that 8-cycle gap

				gte_stopz_cpu(clipflag);
					
				if ((clipflag < 0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					localSortCount ++;
				}
				
				op ++;
				break;
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define op opcd
			case GPU_COM_TG4:

				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles

				deep = (GETD(op->v2) - localMinDepth);
			
				gte_stopz_cpu(clipflag);
						
				if ((clipflag < 0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					localSortCount ++;
				}

				op ++;
				break;
#undef op

/*-----------------------------------------------------------------------------------------------------------------*/

#define op ((TMD_P_FT4I*)opcd)
			case GPU_COM_TF4SPR :
			
				deep = (GETD(op->v0) - localMinDepth);

//				op->next = sl[deep];
//				sl[deep] = (int)op;
//				localSortCount++;

				op++;
				break;
#undef op

/*-----------------------------------------------------------------------------------------------------------------*/
#define op ((TMD_P_FG4I*)opcd)
			case GPU_COM_G4:
			case GPU_COM_F4:
   		
				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles

				deep = (GETD(op->v2) - localMinDepth);

				gte_stopz_cpu(clipflag);
				
					
				if ((clipflag<0))	// || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					localSortCount ++;
				}
				op ++;
				break;
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define op ((TMD_P_FG3I*)opcd)
			case GPU_COM_G3:
			case GPU_COM_F3:

				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles

				deep = (GETD(op->v2) - localMinDepth);

				gte_stopz_cpu(clipflag);
				
				if ((clipflag < 0))	// || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					localSortCount ++;
				}
				op ++;
				break;
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
			default:
				break;
		}
		primsleft --;
	}
	sortCount = localSortCount;
}







void *psiRegisterDrawFunction(void (*drawHandler)(int))
{
	void *olddraw = (void *)customDrawFunction;

	customDrawFunction = drawHandler;

	return olddraw;
}

void *psiRegisterDrawFunction2(void (*drawHandler)(int))
{
	void *olddraw = (void *)customDrawFunction2;

	customDrawFunction2 = drawHandler;

	return olddraw;
}


/***************************************************************************************************/

static void psiResetModelctrl()
{
	PSImodelctrl.depthoverride = 0;
	PSImodelctrl.specialmode = OFF;
	PSImodelctrl.onmap = 0;
	PSImodelctrl.col.r = 128;
	PSImodelctrl.col.g = 128;
	PSImodelctrl.col.b = 128;
	PSImodelctrl.sprites = YES;
	PSImodelctrl.nearclip = 100;
	PSImodelctrl.farclip = 16384;
	PSImodelctrl.inheritScale = 0;

}

/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
/*
#ifdef _DEBUG
static void DrawNormals()
{
	int i;
	ACTOR *actor;
	VERT *verts;
	VERT *norms;
	VERT *convs;
	VERT newnorm;
	PSIOBJECT *world;
	int loop,obs,j,v,rf,tot;
	SVECTOR *newvects;
	DVECTOR *tfvs,*tfscxy;
	DVECTOR *tfsn = (DVECTOR*)transformedScreenN;

	tfvs = (DVECTOR*)transformedVertices;


	actor = actorList.head.next;
	world = actor->object;
	obs = actor->numObjects;
	i = actor->noofVerts;
	newvects = (SVECTOR*)MALLOC( i*sizeof(SVECTOR));

	verts = actor->object->meshdata->vertop;

	polyLine.r0 = 240;
	polyLine.g0 = 240;

	tot = 0;

	for (loop = 0; loop < obs; loop++)
	{
		world = (PSIOBJECT*)actor->objectTable[loop];

	 	gte_SetRotMatrix(&world->matrixscale);			 
	   	gte_SetTransMatrix(&world->matrixscale);		


		verts = world->meshdata->vertop;
		norms = world->meshdata->nortop;
		v = world->meshdata->norn;
		convs = (VERT*)newvects;

		rf = 128;					 

		for (j=0; j<v; j++)
		{

			convs->vx = (norms->vx/rf) + verts->vx;
			convs->vy = (norms->vy/rf) + verts->vy;
			convs->vz = (norms->vz/rf) + verts->vz;
			verts++;
			norms++;
			convs++;
		}

		transformVertexListB(newvects, world->meshdata->norn, tfsn );
		
		tfsn += v;
		tot += v;

	}
	
	tfsn = (DVECTOR*)transformedScreenN;
	tfvs = (DVECTOR*)transformedVertices;
	
	
	for (i = 0; i<tot; i++)
	{
		polyLine.r0 = 240;
		polyLine.g0 = 240;
		polyLine.b0 = 240;

		polyLine.x0 = tfvs->vx;
		polyLine.y0 = tfvs->vy;
		polyLine.x1 = tfsn->vx;
		polyLine.y1 = tfsn->vy;

		PSIDrawLine(0);
		tfvs++;
		tfsn++;
	}
	FREE(newvects);
#endif
}
*/

/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void psiInitSortList(int range)
{
	if ( range<= maxDepthRange )
		return;

	maxDepthRange = range;

	if (sortedIndex)
		FREE(sortedIndex);
	
	sortedIndex = (ULONG*)MALLOC0( range*sizeof(ULONG));
}


/**************************************************************************
	FUNCTION:	CalcMaxMin	
	PURPOSE:	run thru transformed depth list, find max and min values
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void psiCalcMaxMin(PSIDATA *psiData)
{
	long *tfd = transformedDepths;
	int i;
	
/*
	for (i=0; i<maxDepthRange; i++)
		sortedIndex[i]=0;
*/

	sortCount=0;
	maxDepth = minDepth = *tfd;
	
	i = tfTotal;

	while (i)
	{
		if (*tfd < minDepth)
			minDepth = *tfd;
		if (*tfd > maxDepth)
			maxDepth = *tfd;
		tfd ++;
		i --;
	}

	depthRange = (maxDepth - minDepth) + 1;
	
	for (i = 0; i < depthRange; i ++)
		sortedIndex[i] = 0;
	
	if (depthRange > maxDepthRange)
	{
		printf("PROGRAM HALTED. \nDEPTH RANGE EXCEEDED !\nINCREASE MAXDEPTHRANGE TO %d (=%d)\n\n",depthRange,maxDepthRange);
		printf("Object Mesh Name = >%s<\n",psiData->modelName);
		for (;;);
	}

}
/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void psiSetLight(int lightNum, int r, int g, int b, int x, int y, int z)
{

	flatLight[lightNum].vx = x;
	flatLight[lightNum].vy = y;
	flatLight[lightNum].vz = z;
 	flatLight[lightNum].r  = r;
 	flatLight[lightNum].g  = g;
 	flatLight[lightNum].b  = b;
	GsSetFlatLight( lightNum, &flatLight[lightNum]);


   	lightMat=GsIDMATRIX;
	RotMatrixYXZ_gte(&lightRotVec,&lightMat);
	GsSetLightMatrix2(&lightMat);

}
/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

GsCOORDINATE2 lighting;

void psiInitLights()
{

   	lightMat=GsIDMATRIX;
	lightRotVec.vx = 0;
	lightRotVec.vy = 0;
	lightRotVec.vz = 0;
	RotMatrixYXZ_gte(&lightRotVec,&lightMat);

	psiSetLight(0, 0,0,0, 0,0,0 );
	psiSetLight(1, 0,0,0, 0,0,0 );
	psiSetLight(2, 0,0,0, 0,0,0 );
	
 	GsSetAmbient(1024,1024,1024);
	GsSetLightMode(0);

}

void psiSetAmbient(int r, int g, int b)
{
	GsSetAmbient(r,g,b);
}



/**************************************************************************
	Function 	: psiInitialise()
	Purpose 	: initialises the linked list of actors and local vars
	Parameters 	: Maximum number of models
	Returns 	: none
	Info 		:
**************************************************************************/
void psiInitialise(int maxModels)
{

	psiResetModelctrl();

	psiInitSortList(256);

//bbopt - lighting test
//	psiInitLights();

	//actorList.numEntries = 0;
	//actorList.head.next = actorList.head.prev = &actorList.head;

	transformedVertices = 0;
	transformedDepths = 0;
	transformedNormals = 0;
	biggestVertexModel = 0;
	biggestPrimModel = 0;

	psiModelListLen = 0;
	pilLibraryLen = 0;

	customDrawFunction = 0;
	customDrawFunction2 = 0;

	psiModelList = MALLOC0(sizeof(PSIMODEL *) * maxModels);
	psiModelListCRC =MALLOC0(sizeof(long) * maxModels);
	MAXMODELS = maxModels;
}





/**************************************************************************
	Function 	: psiDestroy()
	Purpose 	: free all memory used by PSI
	Parameters 	: 
	Returns 	: 
	Info 		:
**************************************************************************/
void psiDestroy()
{

//	FREE(transformedNormals);
//	FREE(transformedDepths);
//	FREE(transformedVertices);


//	while (psiModelListLen)
//	{
//	 	psiModelListLen--;
//	 	FREE(psiModelList[psiModelListLen]);
//	}

	psiModelListLen = 0;
	pilLibraryLen = 0;

//	while (pilLibraryLen)
//	{
//		pilLibraryLen--;
//	 	FREE(pilLibraryList[pilLibraryLen]);
//	}

	transformedVertices = 0;
	transformedDepths = 0;
	transformedNormals = 0;
	biggestVertexModel = 0;
	biggestPrimModel = 0;
	
//	if (sortedIndex)
//		FREE(sortedIndex);

	sortedIndex = NULL;
	maxDepthRange = 0;

//	FREE(psiModelList);
//	FREE(psiModelListCRC);

	
}

/**************************************************************************
	FUNCTION:	psiObjectScan
	PURPOSE:	Scan object heirarchy for name
	PARAMETERS:	ASSUMES *name IS UPPER CASE !!
	RETURNS:	
**************************************************************************/

PSIOBJECT *psiObjectScan(PSIOBJECT *obj, char *name)
{


	PSIOBJECT *result;
	
	while (obj)
	{
//bbopt - could we do without this?
		utilUpperStr(obj->meshdata->name);

		if ( strcmp( obj->meshdata->name, name )==0 )
			return obj;
		
		if ( obj->child )
		{
			 result = psiObjectScan(obj->child,name);
			 if (result)
				 return result;
		}

		obj = obj->next;

	}	

	return NULL;
}


/**************************************************************************
	FUNCTION:	SetPSIObject
	PURPOSE:	set up next & child pointers in PSIOBJECT
	PARAMETERS:	
	RETURNS:	
**************************************************************************/




/**************************************************************************
	FUNCTION:	PageTexture
	PURPOSE:	get tpage and clut for this prim's texture
	PARAMETERS:	index number+1
	RETURNS:	pointer to texture TextureType
**************************************************************************/

static TextureType* PageTexture(USHORT ix)
{
	TextureType *sprt;

	if (!ix)
		return 0;

	ix--;

	sprt = textureFindCRCInAllBanks(utilStr2CRC((char*)&currentTextureList[ix]) );
	
	return sprt;

}

/**************************************************************************
	FUNCTION:	FixupPrims
	PURPOSE:	run thru the primitive list, loading textures & pointers
	PARAMETERS:	PSIMODEL address
	RETURNS:	nowt
**************************************************************************/
int TG4count=0;
int TG3count=0;
int TF4count=0;
int TF3count=0;
int F4count=0;
int F3count=0;

#define FIXUP_RGB

static void psiFixupPrims(PSIMODEL *psiModel)
{
	TextureType *sprt;
	int p,a;
	char *primitive;
	UBYTE s;

	//int ix; //temp

		p = psiModel->noofPrims;
		primitive = (char*)psiModel->primOffset;

		while (p)
		{
			
			s = ((TMD_P_GT4I*)primitive)->cd;	// code
			switch (s &(0xff-2))
			{

//bbopt - split because rgb code inserted
/*				case GPU_COM_G3:
				case GPU_COM_F3:	
					a = sizeof(TMD_P_FG3I );
			 		((TMD_P_FG3I*)primitive)->in=(sizeof(TMD_P_FG3I)/4)-1;	//ilen
					((TMD_P_FG3I*)primitive)->out=0xc; 						//olen   
					
					if (((TMD_P_FG3I*)primitive)->dummy & psiTRANSPAR )
						((TMD_P_FG3I*)primitive)->cd |=2;

					//bbopt - to save doing this at draw time
					((TMD_P_FG3I*)primitive)->r0 >>= 1;
					((TMD_P_FG3I*)primitive)->g0 >>= 1;
					((TMD_P_FG3I*)primitive)->b0 >>= 1;
					((TMD_P_FG3I*)primitive)->r1 >>= 1;
					((TMD_P_FG3I*)primitive)->g1 >>= 1;
					((TMD_P_FG3I*)primitive)->b1 >>= 1;
					((TMD_P_FG3I*)primitive)->r2 >>= 1;
					((TMD_P_FG3I*)primitive)->g2 >>= 1;
					((TMD_P_FG3I*)primitive)->b2 >>= 1;
					
					//((TMD_P_FG3I*)primitive)->dummy &= psiDOUBLESIDED; 
					F3count++;							  
					break;
*/
				case GPU_COM_F3:
					a = sizeof(TMD_P_FG3I );
			 		((TMD_P_FG3I*)primitive)->in=(sizeof(TMD_P_FG3I)/4)-1;	//ilen
					((TMD_P_FG3I*)primitive)->out=0xc; 						//olen   
					
					if (((TMD_P_FG3I*)primitive)->dummy & psiTRANSPAR )
						((TMD_P_FG3I*)primitive)->cd |=2;

					//bbopt - to save doing this at draw time
#ifdef FIXUP_RGB
					((TMD_P_FG3I*)primitive)->r0 >>= 1;
					((TMD_P_FG3I*)primitive)->g0 >>= 1;
					((TMD_P_FG3I*)primitive)->b0 >>= 1;
#endif
					
					//((TMD_P_FG3I*)primitive)->dummy &= psiDOUBLESIDED; 
					F3count++;							  
					break;

				case GPU_COM_G3:
					a = sizeof(TMD_P_FG3I );
			 		((TMD_P_FG3I*)primitive)->in=(sizeof(TMD_P_FG3I)/4)-1;	//ilen
					((TMD_P_FG3I*)primitive)->out=0xc; 						//olen   
					
					if (((TMD_P_FG3I*)primitive)->dummy & psiTRANSPAR )
						((TMD_P_FG3I*)primitive)->cd |=2;

					//bbopt - to save doing this at draw time
#ifdef FIXUP_RGB
					((TMD_P_FG3I*)primitive)->r0 >>= 1;
					((TMD_P_FG3I*)primitive)->g0 >>= 1;
					((TMD_P_FG3I*)primitive)->b0 >>= 1;
					((TMD_P_FG3I*)primitive)->r1 >>= 1;
					((TMD_P_FG3I*)primitive)->g1 >>= 1;
					((TMD_P_FG3I*)primitive)->b1 >>= 1;
					((TMD_P_FG3I*)primitive)->r2 >>= 1;
					((TMD_P_FG3I*)primitive)->g2 >>= 1;
					((TMD_P_FG3I*)primitive)->b2 >>= 1;
#endif
					//((TMD_P_FG3I*)primitive)->dummy &= psiDOUBLESIDED; 
					F3count++;							  
					break;


//split cos rgb code insterted
/*				case GPU_COM_G4:
				case GPU_COM_F4:	
					a = sizeof(TMD_P_FG4I );
					((TMD_P_FG4I*)primitive)->in=(sizeof(TMD_P_FG4I)/4)-1;	//ilen
		 			((TMD_P_FG4I*)primitive)->out=0x8; 						//olen   
					
					if (((TMD_P_FG4I*)primitive)->dummy & psiTRANSPAR )
						((TMD_P_FG4I*)primitive)->cd |=2;
					
					//bbopt - to save doing this at draw time
					((TMD_P_FG4I*)primitive)->r0 >>= 1;
					((TMD_P_FG4I*)primitive)->g0 >>= 1;
					((TMD_P_FG4I*)primitive)->b0 >>= 1;
					((TMD_P_FG4I*)primitive)->r1 >>= 1;
					((TMD_P_FG4I*)primitive)->g1 >>= 1;
					((TMD_P_FG4I*)primitive)->b1 >>= 1;
					((TMD_P_FG4I*)primitive)->r2 >>= 1;
					((TMD_P_FG4I*)primitive)->g2 >>= 1;
					((TMD_P_FG4I*)primitive)->b2 >>= 1;
					((TMD_P_FG4I*)primitive)->r3 >>= 1;
					((TMD_P_FG4I*)primitive)->g3 >>= 1;
					((TMD_P_FG4I*)primitive)->b3 >>= 1;

					//((TMD_P_FG4I*)primitive)->dummy &= psiDOUBLESIDED; 
					
					F4count++;
					break;
*/
				case GPU_COM_F4:	
					a = sizeof(TMD_P_FG4I );
					((TMD_P_FG4I*)primitive)->in=(sizeof(TMD_P_FG4I)/4)-1;	//ilen
		 			((TMD_P_FG4I*)primitive)->out=0x8; 						//olen   
					
					if (((TMD_P_FG4I*)primitive)->dummy & psiTRANSPAR )
						((TMD_P_FG4I*)primitive)->cd |=2;
					
					//bbopt - to save doing this at draw time
#ifdef FIXUP_RGB
					((TMD_P_FG4I*)primitive)->r0 >>= 1;
					((TMD_P_FG4I*)primitive)->g0 >>= 1;
					((TMD_P_FG4I*)primitive)->b0 >>= 1;
#endif
					//((TMD_P_FG4I*)primitive)->dummy &= psiDOUBLESIDED; 
					
					F4count++;
					break;

				case GPU_COM_G4:
					a = sizeof(TMD_P_FG4I );
					((TMD_P_FG4I*)primitive)->in=(sizeof(TMD_P_FG4I)/4)-1;	//ilen
		 			((TMD_P_FG4I*)primitive)->out=0x8; 						//olen   
					
					if (((TMD_P_FG4I*)primitive)->dummy & psiTRANSPAR )
						((TMD_P_FG4I*)primitive)->cd |=2;
					
					//bbopt - to save doing this at draw time
#ifdef FIXUP_RGB
					((TMD_P_FG4I*)primitive)->r0 >>= 1;
					((TMD_P_FG4I*)primitive)->g0 >>= 1;
					((TMD_P_FG4I*)primitive)->b0 >>= 1;
					((TMD_P_FG4I*)primitive)->r1 >>= 1;
					((TMD_P_FG4I*)primitive)->g1 >>= 1;
					((TMD_P_FG4I*)primitive)->b1 >>= 1;
					((TMD_P_FG4I*)primitive)->r2 >>= 1;
					((TMD_P_FG4I*)primitive)->g2 >>= 1;
					((TMD_P_FG4I*)primitive)->b2 >>= 1;
					((TMD_P_FG4I*)primitive)->r3 >>= 1;
					((TMD_P_FG4I*)primitive)->g3 >>= 1;
					((TMD_P_FG4I*)primitive)->b3 >>= 1;
#endif
					//((TMD_P_FG4I*)primitive)->dummy &= psiDOUBLESIDED; 
					
					F4count++;
					break;


				case GPU_COM_TF3:	
					a = sizeof(TMD_P_FT3I );
					sprt = PageTexture( ((TMD_P_FT3I*)primitive)->clut ); // clut = texture index number
					
					if (((TMD_P_FT3I*)primitive)->tpage) 
						((TMD_P_FT3I*)primitive)->cd |= 2;
					
					((TMD_P_FT3I*)primitive)->tpage |= sprt->tpage;
					((TMD_P_FT3I*)primitive)->clut = sprt->clut;
					((TMD_P_FT3I*)primitive)->tile = sprt->flags;

					((TMD_P_FT3I*)primitive)->in=(sizeof(TMD_P_FT3I)/4)-1;	//ilen
		 			((TMD_P_FT3I*)primitive)->out=0x7; 						//olen   
					((TMD_P_FT3I*)primitive)->tu0 += sprt->x;
					((TMD_P_FT3I*)primitive)->tu1 += sprt->x;
					((TMD_P_FT3I*)primitive)->tu2 += sprt->x;

					((TMD_P_FT3I*)primitive)->tv0 += sprt->y;
					((TMD_P_FT3I*)primitive)->tv1 += sprt->y;
					((TMD_P_FT3I*)primitive)->tv2 += sprt->y;

					//bbopt - to save doing this at draw time
#ifdef FIXUP_RGB
					((TMD_P_FT3I*)primitive)->r0 >>= 1;
					((TMD_P_FT3I*)primitive)->g0 >>= 1;
					((TMD_P_FT3I*)primitive)->b0 >>= 1;
#endif
					TF3count++;
					//((TMD_P_FG4I*)primitive)->dummy &= psiDOUBLESIDED; 
					break;

				case GPU_COM_TF4SPR:
				case GPU_COM_TF4:	
					a = sizeof(TMD_P_FT4I );
					sprt = PageTexture( ((TMD_P_FT4I*)primitive)->clut ); // clut= texture index number

					if (((TMD_P_FT4I*)primitive)->tpage) 
						((TMD_P_FT4I*)primitive)->cd |= 2;
						
					((TMD_P_FT4I*)primitive)->tpage |= sprt->tpage;
					((TMD_P_FT4I*)primitive)->clut = sprt->clut;
					((TMD_P_FT4I*)primitive)->tile = sprt->flags;

					((TMD_P_FT4I*)primitive)->in=(sizeof(TMD_P_FT4I)/4)-1;	//ilen
		 			((TMD_P_FT4I*)primitive)->out=0x9; 						//olen   

					((TMD_P_FT4I*)primitive)->tu0 += sprt->x;
					((TMD_P_FT4I*)primitive)->tu1 += sprt->x;
					((TMD_P_FT4I*)primitive)->tu2 += sprt->x;
					((TMD_P_FT4I*)primitive)->tu3 += sprt->x;

					((TMD_P_FT4I*)primitive)->tv0 += sprt->y;
					((TMD_P_FT4I*)primitive)->tv1 += sprt->y;
					((TMD_P_FT4I*)primitive)->tv2 += sprt->y;
					((TMD_P_FT4I*)primitive)->tv3 += sprt->y;

					//bbopt - to save doing this at draw time
#ifdef FIXUP_RGB
					((TMD_P_FT4I*)primitive)->r0 >>= 1;
					((TMD_P_FT4I*)primitive)->g0 >>= 1;
					((TMD_P_FT4I*)primitive)->b0 >>= 1;
#endif
					TF4count++;
					//((TMD_P_FG4I*)primitive)->dummy &= psiDOUBLESIDED; 
					break;

				case GPU_COM_TG3:	
					a = sizeof(TMD_P_GT3I );
					sprt = PageTexture( ((TMD_P_GT3I*)primitive)->clut ); // clut= texture index number
					
					if (((TMD_P_GT3I*)primitive)->tpage) 
						((TMD_P_GT3I*)primitive)->cd |= 2;
									
					((TMD_P_GT3I*)primitive)->tpage |= sprt->tpage;
					((TMD_P_GT3I*)primitive)->clut = sprt->clut;
					((TMD_P_GT3I*)primitive)->tile = sprt->flags;

					((TMD_P_GT3I*)primitive)->in=(sizeof(TMD_P_GT3I)/4)-1;	//ilen
		 			((TMD_P_GT3I*)primitive)->out=0x9; 						//olen   

					((TMD_P_GT3I*)primitive)->tu0 += sprt->x;
					((TMD_P_GT3I*)primitive)->tu1 += sprt->x;
					((TMD_P_GT3I*)primitive)->tu2 += sprt->x;

					((TMD_P_GT3I*)primitive)->tv0 += sprt->y;
					((TMD_P_GT3I*)primitive)->tv1 += sprt->y;
					((TMD_P_GT3I*)primitive)->tv2 += sprt->y;

					//bbopt - to save doing this at draw time
#ifdef FIXUP_RGB
					((TMD_P_GT3I*)primitive)->r0 >>= 1;
					((TMD_P_GT3I*)primitive)->g0 >>= 1;
					((TMD_P_GT3I*)primitive)->b0 >>= 1;
					((TMD_P_GT3I*)primitive)->r1 >>= 1;
					((TMD_P_GT3I*)primitive)->g1 >>= 1;
					((TMD_P_GT3I*)primitive)->b1 >>= 1;
					((TMD_P_GT3I*)primitive)->r2 >>= 1;
					((TMD_P_GT3I*)primitive)->g2 >>= 1;
					((TMD_P_GT3I*)primitive)->b2 >>= 1;
#endif

					TG3count++;
					break;

				case GPU_COM_TG4:	
					a = sizeof(TMD_P_GT4I );
					sprt = PageTexture( ((TMD_P_GT4I*)primitive)->clut ); // clut= texture index number

					if (((TMD_P_GT4I*)primitive)->tpage) 
						((TMD_P_GT4I*)primitive)->cd |= 2;
						
					((TMD_P_GT4I*)primitive)->tpage |= sprt->tpage;
					((TMD_P_GT4I*)primitive)->clut = sprt->clut;
					((TMD_P_GT4I*)primitive)->tile = sprt->flags;

					((TMD_P_GT4I*)primitive)->in=(sizeof(TMD_P_GT4I)/4)-1;	//ilen
		 			((TMD_P_GT4I*)primitive)->out=0xc; 						//olen   

					((TMD_P_GT4I*)primitive)->tu0 += sprt->x;
					((TMD_P_GT4I*)primitive)->tu1 += sprt->x;
					((TMD_P_GT4I*)primitive)->tu2 += sprt->x;
					((TMD_P_GT4I*)primitive)->tu3 += sprt->x;

					((TMD_P_GT4I*)primitive)->tv0 += sprt->y;
					((TMD_P_GT4I*)primitive)->tv1 += sprt->y;
					((TMD_P_GT4I*)primitive)->tv2 += sprt->y;
					((TMD_P_GT4I*)primitive)->tv3 += sprt->y;
									
					//bbopt - to save doing this at draw time
#ifdef FIXUP_RGB
					((TMD_P_GT4I*)primitive)->r0 >>= 1;
					((TMD_P_GT4I*)primitive)->g0 >>= 1;
					((TMD_P_GT4I*)primitive)->b0 >>= 1;
					((TMD_P_GT4I*)primitive)->r1 >>= 1;
					((TMD_P_GT4I*)primitive)->g1 >>= 1;
					((TMD_P_GT4I*)primitive)->b1 >>= 1;
					((TMD_P_GT4I*)primitive)->r2 >>= 1;
					((TMD_P_GT4I*)primitive)->g2 >>= 1;
					((TMD_P_GT4I*)primitive)->b2 >>= 1;
					((TMD_P_GT4I*)primitive)->r3 >>= 1;
					((TMD_P_GT4I*)primitive)->g3 >>= 1;
					((TMD_P_GT4I*)primitive)->b3 >>= 1;
#endif
					TG4count++;

					break;


				default:	
					a = 0;
					utilPrintf("FATAL: don't know that kind of prim...($%x)\n",s);
					
					for (;;);
					break;

			}
			primitive += a;
			p--;
		}
}

/**************************************************************************
	FUNCTION:	FixupMesh
	PURPOSE:	turn loaded PSImodel offsets into physical addresses
	PARAMETERS:	mesh address, start of model address
	RETURNS:	nowt
**************************************************************************/
static void psiFixupMesh(PSIMESH *mesh)
{
	int p,i;


	// main pointers										
	(int)mesh->vertop = (int)mesh + (int)mesh->vertop;
	(int)mesh->nortop = (int)mesh + (int)mesh->nortop;

	(int)mesh->scalekeys = (int)mesh + (int)mesh->scalekeys;
	(int)mesh->movekeys = (int)mesh + (int)mesh->movekeys;
	(int)mesh->rotatekeys = (int)mesh + (int)mesh->rotatekeys;


	// sortlist pointers
	p = (int)mesh;

	for (i=0; i<8; i++)	
		(int)mesh->sortlistptr[i] = p + (int)mesh->sortlistptr[i];

	// child and next pointers
	if (mesh->child)
	{
		(int)mesh->child = (int)mesh + (int)mesh->child;
		psiFixupMesh(mesh->child);
	}
	if (mesh->next)
	{
		(int)mesh->next = (int)mesh + (int)mesh->next;
		psiFixupMesh(mesh->next);
	}
}


/**************************************************************************
	FUNCTION:	psiDisplay()
	PURPOSE:	display PSIMODEL stats
	PARAMETERS:	PSIMODEL*
	RETURNS:	nowt
**************************************************************************/
static void psiDisplay(PSIMODEL* psiModel)
{
	utilPrintf("Loading model '%s'\n",psiModel->name);
	utilPrintf("PSI Verion no. %d\n",psiModel->version);
	utilPrintf("%d meshes\n",psiModel->noofmeshes);
	utilPrintf("%d vertices\n",psiModel->noofVerts);
	utilPrintf("%d polys\n",psiModel->noofPrims);
	utilPrintf("%d textures.\n",psiModel->noofTextures);
}

/**************************************************************************
	FUNCTION:	ContructPSIName()
	PURPOSE:	build file name into *.PSI
	PARAMETERS:	filename
	RETURNS:	
**************************************************************************/

char *psiConstructName(char *psiName)
{
	int i;
	
	for (i = 0; i < 32; i ++)
		if ( (psiName[i] == 0) || (psiName[i] == 32) || (psiName[i] == '.') )
			break;

	psiName[i] = '.';
	psiName[i + 1] = 'P';
	psiName[i + 2] = 'S';
	psiName[i + 3] = 'I';
	psiName[i + 4] = 0;

	utilUpperStr(psiName);
	return &psiName[0];
	
}
/**************************************************************************
	FUNCTION:	ContructPSIName()
	PURPOSE:	build file name into *.PSI
	PARAMETERS:	filename
	RETURNS:	
**************************************************************************/

long psiCRCName(char *psiName)
{
	int i,j,slen;
	char str[64];

	utilUpperStr(psiName);
	
	for (i = 0; i < 63; i ++)
	{
		str[i] = psiName[i];
		if ( (psiName[i] == 0) || (psiName[i] == 32) || (psiName[i] == '.') )
			break;
	}
	str[i] = 0;
	slen = i - 1;

	for (i = slen; i >= 0; i --)
	{
		if ( ((str[i] < 'A') || (str[i] > 'Z')) &&
			 ((str[i] < '0') || (str[i] > '9')) )		 break;
	}

	if (i >= 0)
	{
		j = i + 1;
		for (i = 0; i < slen; i ++)
		{
		 str[i] = str[i + j];
		}
	}
	
	return utilStr2CRC(&str[0]);
}


/**************************************************************************
	FUNCTION:	CheckPSI()
	PURPOSE:	see if a psi file is already loaded
	PARAMETERS:	filename.psi
	RETURNS:	pointer to PSIMODEL / null
**************************************************************************/

PSIMODEL *psiCheck(char *psiName)
{
	int i,crc;

	crc = psiCRCName(psiName);

	for (i = 0; i < psiModelListLen; i ++)
	{
		if ( psiModelListCRC[i] == crc )
			return psiModelList[i];
	}

	return NULL;
}


/**************************************************************************
	FUNCTION:	psiFixup()
	PURPOSE:	initialise a psi file, fix up ponters and load textures
	PARAMETERS:	pointer to PSIMODEL
	RETURNS:	pointer to PSIMODEL
**************************************************************************/
PSIMODEL *psiFixup(char *addr)
{
	PSIMESH *mesh;
	int /*lastfilelength,*/i;
	PSIMODEL *psiModel;

	psiModel = (PSIMODEL*)addr;

	currentPSI = psiModel;

	// do check for "PSI",0
	if(strcmp((char*)psiModel,"PSI")!=0){ utilPrintf("%s is not a valid PSI file.\n",addr); return 0; }

	// primitive offset pointer
	psiModel->primOffset += (int)addr;
	
	// list of texture names
	psiModel->textureOffset += (int)addr;
	
	// mesh offset
	psiModel->firstMesh += (int)addr;
	
	//	offset to anim frame list
	psiModel->animSegmentList += (int)addr;

	// malloc transformation vertex,normal and depth buffers to biggest
	// model vert count loaded so far
	i = psiModel->noofVerts;
	if ( i>biggestVertexModel )
	{
		/*
		if (transformedVertices!=0)
		{
			 FREE(transformedVertices);
			 FREE(transformedDepths);
			 FREE(transformedNormals);
		}
		
		transformedVertices = (long*)MALLOC( i*sizeof(SVECTOR));
		transformedDepths = (long*)MALLOC( i*sizeof(SVECTOR));
		transformedNormals = (VERT*)MALLOC( i*sizeof(VERT));
		*/
		biggestVertexModel = i;
	}

//	DisplayPSI(psiModel);

	// number of textures
	(char*)currentTextureList = psiModel->textureOffset;

//	numoftextures = psiModel->noofTextures;
//	for (i=0; i<numoftextures; i++) utilPrintf("%d: %s\n",i,&textureList[i]);

	
	
	// PSI MESH LIST

	mesh = (PSIMESH*)psiModel->firstMesh;

	psiFixupMesh(mesh);

	psiFixupPrims(psiModel);

//	utilPrintf("Load PSI done\n");

	// add to loaded model list (for checking re-loads)
	psiModelList[psiModelListLen] = psiModel;
//	strncpy( psiModelListNames[psiModelListLen], PSIname, 16);
	psiModelListCRC[psiModelListLen] = psiCRCName( PSIname );
	psiModelListLen++;

	return psiModel;
}


/**************************************************************************
	FUNCTION:	psiAllocWorkspace()
	PURPOSE:	allocate workspace for model drawing
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void psiAllocWorkspace()
{
	transformedVertices = (long*)MALLOC0( (biggestVertexModel + 2) * sizeof(long) );
	transformedDepths = (long*)MALLOC0( (biggestVertexModel + 2) * sizeof(long) );
	transformedNormals = (VERT*)MALLOC0( (biggestVertexModel + 2) * sizeof(VERT));
}


/**************************************************************************
	FUNCTION:	LoadPSI()
	PURPOSE:	load a psi file, fix up ponters and load textures
	PARAMETERS:	filename.psi
	RETURNS:	pointer to PSIMODEL
**************************************************************************/


PSIMODEL *psiLoad(char *psiName)
{
	char *addr, *compare;
	//PSIMESH *mesh;
	int lastfilelength;//,i;
	PSIMODEL *psiModel;


	if (sizeof(PSIMODEL) != 192)
		utilPrintf("\nWARNING: PSIMODEL structure length invalid (%d != 192)\n\n",sizeof(PSIMODEL));
	

	psiModel = psiCheck(psiName);		// already loaded ?
	if (psiModel!=NULL)
	{
		//utilPrintf("Already loaded file %s\n",psiName);
		//psiDisplay(psiModel);

		return psiModel;

	}

	PSIname = psiName;

	if (psiModelListLen>=MAXMODELS) 
	{
		utilPrintf("jees ! how many models do you expect me to load ?\n"); 
		return 0; 
	}


	//psiName = psiConstructName(psiName);
	addr = (void *)fileLoad(psiName, &lastfilelength);

	if ( !addr )
		return 0;
	
	// JH: File is only loaded once.....  Here I can set a list of flags things, stuff, etc......

	psiModel = psiFixup(addr);

	return psiModel;
}

/**************************************************************************
	FUNCTION:	psiLoadPIL()
	PURPOSE:	load psi library and ALL models in it, fix up ponters and load textures
	PARAMETERS:	filename.pil
	RETURNS:	pointer to PSIMODEL
**************************************************************************/


void *psiLoadPIL(char *pilName)
{
	char *addr;
	int psiM;
	int *table,*crcs;
	int lastfilelength,i;

	//addr = (void *)fileLoad(pilName, &lastfilelength);
	addr = FindStakFileInAllBanks ( pilName, &lastfilelength );

	//addr = getFileFromStack ( stakFile, pilName, &lastfilelength );

	if(!addr)
	{
		return NULL;
	}

	i = (int)*addr;
	(char*)table = addr+4;

	//utilPrintf("%d models in this library.\n",i);

	while (i)
	{

		i--;
		psiM = *(table++);
		//utilPrintf("#%d $%x (%s)\n",i,psiM,(psiM+(int)addr));
		psiM += (int)addr;
		PSIname = (char*)(psiM);
		(char*)crcs = (char*)(psiM+16);
		//utilPrintf("CRC=%x\n",*crcs);
		psiFixup( (char*)(psiM+20) );
	}
	
	pilLibraryList[pilLibraryLen] = (long*)addr;
	pilLibraryLen++;
	//utilPrintf("Libray Loaded\n");

	return NULL;
	
}

/**************************************************************************
	FUNCTION:	DrawLine()
	PURPOSE:	Add line to packet draw list
	PARAMETERS:	Depth value
	RETURNS:	
**************************************************************************/
/*
LINE_F2	polyLine;

static void PSIDrawLine(ULONG depth)
{
	LINE_F2	*si;
	
	BEGINPRIM(si,LINE_F2);
  	*(ULONG*)&si->r0 = *(ULONG*)&polyLine.r0;
  	*(ULONG*)&si->x0 = *(ULONG*)&polyLine.x0;
  	*(ULONG*)&si->x1 = *(ULONG*)&polyLine.x1;
	si->code = 0x40|PSImodelctrl.semitrans;
  	ENDPRIM(si, depth & 1023, LINE_F2);
}
*/

/**************************************************************************
	FUNCTION:	DrawBox()
	PURPOSE:	Add box to packet draw list
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void psiDrawBox(SHORT x,SHORT y,SHORT w,SHORT h,UBYTE r,UBYTE g,UBYTE b,UBYTE semi,SHORT pri)
{

	POLY_F4 *si;
	
	BEGINPRIM(si, POLY_F4);
	
	si->r0=r;
	si->g0=g;
	si->b0=b;
	if (semi==0)si->code=GPU_COM_F4;
	else		si->code=GPU_COM_F4+2;
	si->x0=si->x2=x;
	si->x1=si->x3=x+w;
	si->y0=si->y1=y;
	si->y2=si->y3=y+h;
	ENDPRIM(si, pri & 1023, POLY_F4);
}

/*
void
Get_Screenxy(VERT *v0,LONG *xy){
 	gte_ldv0(v0);		//load vector
 	gte_rtps();        	//RotTransPers1
	gte_stsxy(xy);	 	//store
}
*/

/**************************************************************************
	FUNCTION:	PSISortPrims
	PURPOSE:	create link list table for model's polys
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

/*static void psiSortPrimitives()
{
	register long *tfd = transformedDepths;
	register long *tfv = transformedVertices;
	register TMD_P_GT4I	*opcd;
	PSIMODELCTRL	*modctrl = &PSImodelctrl;
	int				*sl;
	long			clipflag;


	long deep;
	int prims,primsleft;

	(TMD_P_GT3I*)sl = &sortedIndex[0];

	prims = (int)modctrl->PrimTop;
	primsleft = modctrl->PrimLeft;
	opcd = (TMD_P_GT4I*)(prims);

	while(primsleft)
	{
		switch (opcd->cd & (0xff - 2))
		{
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define op ((TMD_P_FT3I*)opcd)

		case GPU_COM_TF3:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles

				deep = (tfd[op->v2] - minDepth);

				gte_stopz(&clipflag);
					
				if ((clipflag < 0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount ++;
				}
			}
			op ++;
			break;

#undef op			
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define op ((TMD_P_FT4I*)opcd)

		case GPU_COM_TF4:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles
				
				deep = (tfd[op->v2] - minDepth);

				gte_stopz(&clipflag);
				
				if ((clipflag < 0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount ++;
				}
			}
			op ++;
			break;

#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define op ((TMD_P_GT3I*)opcd)

		case GPU_COM_TG3:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles
					
				deep = (tfd[op->v2] - minDepth);

				gte_stopz(&clipflag);
					
				if ((clipflag < 0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount ++;
				}
			}				
			op ++;
			break;

#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define op opcd
		case GPU_COM_TG4:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles

				deep = (tfd[op->v2] - minDepth);
			
				gte_stopz(&clipflag);
						
				if ((clipflag < 0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount ++;
				}
			}
			op ++;
			break;

#undef op

/*-----------------------------------------------------------------------------------------------------------------*/

/*#define op ((TMD_P_FT4I*)opcd)
		case GPU_COM_TF4SPR:
			if((tfd[op->v0] > modctrl->nearclip) && (tfd[op->v0] < modctrl->farclip))
			{
				deep = (tfd[op->v0] - minDepth);

				op->next = sl[deep];
				sl[deep] = (int)op;
				sortCount++;
			}
			op++;
			break;

#undef op

/*-----------------------------------------------------------------------------------------------------------------*/
/*#define op ((TMD_P_FG4I*)opcd)

		case GPU_COM_G4:
		case GPU_COM_F4:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles

				deep = (tfd[op->v2] - minDepth);

				gte_stopz(&clipflag);
				
					
				if ((clipflag<0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount ++;
				}
			}
			op ++;
			break;

#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define op ((TMD_P_FG3I*)opcd)
		case GPU_COM_G3:
		case GPU_COM_F3:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				gte_nclip();	// takes 8 cycles

				deep = (tfd[op->v2] - minDepth);

				gte_stopz(&clipflag);
				
				if ((clipflag < 0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount ++;
				}
			}
			op ++;
			break;

#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
	/*	default:
			break;
		}
		primsleft --;
	}
}
*/

/**************************************************************************
	FUNCTION:	PSIDrawSortedPrims
	PURPOSE:	draw dynamic sorted polys
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

/*static void psiDrawSortedPrimitives(int depth)
{
	register PACKET*		packet;
	register long			*tfv = transformedVertices;
	register long			*tfd = transformedDepths;
	VERT 					*tfn = transformedNormals;
	register TMD_P_GT4I		*opcd;
	PSIMODELCTRL			*modctrl = &PSImodelctrl;
	int						primsleft,lightmode;
	ULONG					*sorts = sortedIndex;
	ULONG					sortBucket = 0;
	VERT					*vp = modctrl->VertTop;
	int						gteH;

	gte_ReadGeomScreen(&gteH);
								  
	primsleft = sortCount;
	if (!primsleft)
		return;

	lightmode = modctrl->lighting;
	opcd = 0;
	while(primsleft)
	{
		if (opcd==0)
		{
			(int)opcd = sorts[sortBucket++];
			continue;
		}

		primsleft--;

		switch (opcd->cd & (0xff-2))
		{
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_FT3*)packet)
#define op ((TMD_P_FT3I*)opcd)

			case GPU_COM_TF3:
			
				BEGINPRIM(si, POLY_FT3);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				gte_stsxy3_ft3(si);

				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb(&op->r0);
						gte_ldv0(&tfn[op->v0]);
						gte_nccs();
						gte_strgb(&si->r0);
						break;
					case DIRECTIONONLY:
						gte_ldv0(&tfn[op->v0]);
						gte_ncs();
						gte_strgb(&si->r0);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *) (&si->r0) = *(u_long *) (&op->r0);
		 		}

				setPolyFT3(si);
				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_FT3);
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)
				
				case GPU_COM_TF4:

				BEGINPRIM(si, POLY_FT4);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_ft4(si);

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);

				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb(&op->r0);
						gte_ldv0(&tfn[op->v0]);
						gte_nccs();			
						gte_strgb(&si->r0);
						break;
					case DIRECTIONONLY:
						gte_ldv0(&tfn[op->v0]);
						gte_ncs();			
						gte_strgb(&si->r0);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here
				}
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);

				setPolyFT4(si);
				si->code = op->cd | modctrl->semitrans;
 				ENDPRIM(si, depth, POLY_FT4);
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_GT3*)packet)
#define op ((TMD_P_GT3I*)opcd)

			case GPU_COM_TG3:

				BEGINPRIM(si, POLY_GT3);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_gt3(si);

				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();			
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				}
				setPolyGT3(si);
				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_GT3);
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_GT4*)packet)
#define op opcd

			case GPU_COM_TG4:
				BEGINPRIM(si, POLY_GT4);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
					
				gte_stsxy3_gt4(si);
				
						
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
		
				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);

						gte_ldrgb(&op->r3);
						gte_ldv0(&tfn[op->v3]);
						gte_nccs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();			
						gte_strgb3(&si->r0, &si->r1, &si->r2);

						gte_ldv0(&tfn[op->v3]);
						gte_ncs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r3) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
						*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
				}
		
				setPolyGT4(si);
				si->code = op->cd | modctrl->semitrans;
 				ENDPRIM(si, depth, POLY_GT4);
				(int)op = op->next;
				break;

#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)

			case GPU_COM_TF4SPR :
			{
				int			width, height;

				BEGINPRIM(si, POLY_FT4);
/*
	We can't use the "quick" scaling method in skinned objects, since we don't know which
	bone the sprite is attached to, hence we can't rtps the vertex and get the scaled width/height,
	so we have to do the scaling based on the distance ourselves.
*/
/*				width = ((op->v2 * gteH) / tfd[op->v0]) / 2;
				height = ((op->v3 * gteH) / tfd[op->v0]) / 4;

 				*(u_long *)&si->r0 = *(u_long *)&op->r0;			// Texture coords / colors
				*(u_long *)&si->u0 = *(u_long *)&op->tu0;
				*(u_long *)&si->u1 = *(u_long *)&op->tu1;
				*(u_long *)&si->u2 = *(u_long *)&op->tu2;
				*(u_long *)&si->u3 = *(u_long *)&op->tu3;

				si->x1 = si->x3 = ((DVECTOR *)tfv)[op->v0].vx + width;
				si->x0 = si->x2 = ((DVECTOR *)tfv)[op->v0].vx - width;
				si->y2 = si->y3 = ((DVECTOR *)tfv)[op->v0].vy + height;
				si->y0 = si->y1 = ((DVECTOR *)tfv)[op->v0].vy - height;
				
				setPolyFT4(si);
				si->code = op->cd | modctrl->semitrans;
 				ENDPRIM(si, depth, POLY_FT4);
				op = op->next;
			}
			break;

#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_G4*)packet)
#define op ((TMD_P_FG4I*)opcd)
			case GPU_COM_G4:
			case GPU_COM_F4:
				
				BEGINPRIM(si, POLY_G4);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
 
				gte_stsxy3_g4(si);

				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						
						gte_ldrgb(&op->r3);
						gte_ldv0(&tfn[op->v3]);
						gte_nccs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();			
						gte_strgb3(&si->r0, &si->r1, &si->r2);

						gte_ldv0(&tfn[op->v3]);
						gte_ncs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;

					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r3) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
						*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
				}


				setPolyG4(si);
				si->code = op->cd | modctrl->semitrans;
 				ENDPRIM(si, depth, POLY_G4);
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_G3*)packet)
#define op ((TMD_P_FG3I*)opcd)
			
			case GPU_COM_G3:
			case GPU_COM_F3:
			
				BEGINPRIM(si, POLY_G3);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				
				gte_stsxy3_g3(si);


				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();			
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				}


				setPolyG3(si);
				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_G3);
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*		default:
			break;
		}
	}
}

/**************************************************************************
	FUNCTION:	PSIDrawPrimitives
	PURPOSE:	add list of polys into ot
	PARAMETERS:	ot (at depth)
	RETURNS:	
**************************************************************************/

/*void psiDrawPrimitives(int depth)
{
	register PACKET*		packet;
	register long *tfv = transformedVertices;
	register long *tfd = transformedDepths;
	register TMD_P_GT4I	*opcd;
	long	 clipflag;
	PSIMODELCTRL	*modctrl = &PSImodelctrl;
	VERT	*vp = modctrl->VertTop;
	int prims,primsleft,lightmode;
	USHORT *sorts;
	VERT 	*tfn = transformedNormals;
	int		gteH;
	
	gte_ReadGeomScreen(&gteH);





	prims = (int)modctrl->PrimTop;
	sorts = modctrl->SortOffs;
	primsleft = modctrl->PrimLeft;
	lightmode = modctrl->lighting;

	while(primsleft)
	{
		
		opcd = (TMD_P_GT4I*)( prims + (*sorts++<<3) );
		switch (opcd->cd & (0xff-2))
		{
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_FT3*)packet)
#define op ((TMD_P_FT3I*)opcd)
		case GPU_COM_TF3:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{
				BEGINPRIM(si, POLY_FT3);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
			
				gte_nclip_b();	// takes 8 cycles


				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

	 			gte_stopz(&clipflag);
			
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;									// Back face culling

				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
				gte_stsxy3_ft3(si);
			
				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb(&op->r0);
						gte_ldv0(&tfn[op->v0]);
						gte_nccs();			
						gte_strgb(&si->r0);
						break;
					case DIRECTIONONLY:
						gte_ldv0(&tfn[op->v0]);
						gte_ncs();			
						gte_strgb(&si->r0);
						break;

					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here
		 		}
				setPolyFT3(si);
				si->code = op->cd | modctrl->semitrans;

 				ENDPRIM(si, depth & 1023, POLY_FT3);
				modctrl->polysdrawn++;
			}
			break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)
		case GPU_COM_TF4:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{
				BEGINPRIM(si, POLY_FT4);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords

				gte_nclip_b();	// takes 8 cycles
		
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stopz(&clipflag);

				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;	// Back face culling
				
				gte_stsxy3_ft4(si);

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);

				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb(&op->r0);
						gte_ldv0(&tfn[op->v0]);
						gte_nccs();			
						gte_strgb(&si->r0);
						break;
					case DIRECTIONONLY:
						gte_ldv0(&tfn[op->v0]);
						gte_ncs();			
						gte_strgb(&si->r0);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here
				}
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);

				setPolyFT4(si);
				si->code = op->cd | modctrl->semitrans;
				
				modctrl->polysdrawn++;
 			
 				ENDPRIM(si, depth & 1023, POLY_FT4);
			}
			break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_GT3*)packet)
#define op ((TMD_P_GT3I*)opcd)
		case GPU_COM_TG3:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))	
			{
				BEGINPRIM(si, POLY_GT3);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords

				gte_nclip_b();	// takes 8 cycles
				
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
					
				gte_stopz(&clipflag);
				
				if (!(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;								// Back face culling

				gte_stsxy3_gt3(si);
			

				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				}
				setPolyGT3(si);
				si->code = op->cd | modctrl->semitrans;
			
				modctrl->polysdrawn++;

				ENDPRIM(si, depth & 1023, POLY_GT3);
			}
			break;
 		
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_GT4*)packet)
#define op opcd
		case GPU_COM_TG4:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{
				BEGINPRIM(si, POLY_GT4);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
			
				gte_nclip_b();	// takes 8 cycles
		
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
					
				gte_stopz(&clipflag);
				
				if (!(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;			// Back face culling

				gte_stsxy3_gt4(si);
				
						
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);


				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
		
		
				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						
						gte_ldrgb(&op->r3);
						gte_ldv0(&tfn[op->v3]);
						gte_nccs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);

						gte_ldv0(&tfn[op->v3]);
						gte_ncs();			
						gte_strgb(&si->r3);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r3) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
						*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
				}
		
				modctrl->polysdrawn++;
			
				setPolyGT4(si);
				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth & 1023, POLY_GT4);
			}
			break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)
		case GPU_COM_TF4SPR :
			if((tfd[op->v0] > modctrl->nearclip) && (tfd[op->v0] < modctrl->farclip))
			{
				int			width, height;

				BEGINPRIM(si, POLY_FT4);
/*
	We can't use the "quick" scaling method in skinned objects, since we don't know which
	bone the sprite is attached to, hence we can't rtps the vertex and get the scaled width/height,
	so we have to do the scaling based on the distance ourselves.
*/
/*				width = ((op->v2 * gteH) / tfd[op->v0]) / 2;
				height = ((op->v3 * gteH) / tfd[op->v0]) / 4;

 				*(u_long *)&si->r0 = *(u_long *)&op->r0;			// Texture coords / colors
				*(u_long *)&si->u0 = *(u_long *)&op->tu0;
				*(u_long *)&si->u1 = *(u_long *)&op->tu1;
				*(u_long *)&si->u2 = *(u_long *)&op->tu2;
				*(u_long *)&si->u3 = *(u_long *)&op->tu3;

				utilPrintf("%d : %d, %d : %d, %d : %d, %d : %d\n ", si->u0, si->v0, si->u1, si->v1, si->u2, si->v2, si->u3, si->v3  );

				si->x1 = si->x3 = ((DVECTOR *)tfv)[op->v0].vx + width;
				si->x0 = si->x2 = ((DVECTOR *)tfv)[op->v0].vx - width;
				si->y2 = si->y3 = ((DVECTOR *)tfv)[op->v0].vy + height;
				si->y0 = si->y1 = ((DVECTOR *)tfv)[op->v0].vy - height;

				utilPrintf("Sprite : %d : %d %d : %d\n ", ((DVECTOR *)tfv)[op->v0].vx, ((DVECTOR *)tfv)[op->v0].vy, width, height);
				
				setPolyFT4(si);
				si->code = op->cd | modctrl->semitrans;
 				ENDPRIM(si, depth & 1023, POLY_FT4);
				op = op->next;
			}
			break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_G4*)packet)
#define op ((TMD_P_FG4I*)opcd)
		case GPU_COM_G4:
		case GPU_COM_F4:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{
				BEGINPRIM(si, POLY_G4);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
 
 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

				// Back face culling
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling
				
				gte_stsxy3_g4(si);


				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						
						gte_ldrgb(&op->r3);
						gte_ldv0(&tfn[op->v3]);
						gte_nccs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);

						gte_ldv0(&tfn[op->v3]);
						gte_ncs();			
						gte_strgb(&si->r3);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r3) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
						*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
				}

				setPolyG4(si);
				si->code = op->cd | modctrl->semitrans;

				modctrl->polysdrawn++;

				ENDPRIM(si, depth & 1023, POLY_G4);
			}
			break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_G3*)packet)
#define op ((TMD_P_FG3I*)opcd)
			
		case GPU_COM_G3:
		case GPU_COM_F3:
			if((tfd[op->v2] > modctrl->nearclip) && (tfd[op->v2] < modctrl->farclip))
			{
				BEGINPRIM(si, POLY_G3);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

 
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

	//			if (clipflag >= 0) break; 								// Back face culling
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling
				
				gte_stsxy3_g3(si);


				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				}


				modctrl->polysdrawn++;

				setPolyG3(si);
				si->code = op->cd | modctrl->semitrans;

				ENDPRIM(si, depth & 1023, POLY_G3);
			}
			break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
	/*		default:
				break;
		}
		primsleft--;
	}
}
*/


/**************************************************************************
	FUNCTION:	PSIDrawSegments
	PURPOSE:	draw all object segments of an actor
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void psiDrawSegments(PSIDATA *psiData)
{
	long		*tfvbase;
	long		*tfdbase;

	register long		*tfv;
	register long		*tfd;
	register PSIOBJECT	*world;
	register SVECTOR	*np;
	PSIMODELCTRL		*modctrl = &PSImodelctrl;
	LONG				depth;
	LONG				s = modctrl->sorttable;
	int					loop, obs, i, j;
	VERT				*tfn;

	world = psiData->object;
	obs = psiData->numObjects;

	// transform all the vertices (by heirarchy)
	// tfv == one long list of transformed vertices

	tfn = transformedNormals - 1;

#ifdef DCACHE_OPTIMISATION
	tfTotal = 0;
	for (loop = 0; loop < obs; loop++)
	{
		world = (PSIOBJECT*)psiData->objectTable[loop];
		tfTotal += world->meshdata->vern;
	}

	/*if(tfTotal <= 126)	// Not 128. TransformVerts rounds up to nearest 3, remember
	{
		tfvbase = (void *)DCACHE;
		tfdbase = (void *)(DCACHE + 512);
	}
	else if(tfTotal <= 255)	// Not 256. TransformVerts rounds up to nearest 3, remember
	{
		tfvbase = (void *)DCACHE;
		tfdbase = transformedDepths;
	}
	else*/
	{
		tfvbase = transformedVertices;
		tfdbase = transformedDepths;
	}


	tfv = tfvbase;
	tfd = tfdbase;
//#else
//	tfv = transformedVertices;
//	tfd = transformedDepths;
#endif

	tfTotal = 0;

	for (loop = 0; loop < obs; loop++)
	{
		world = (PSIOBJECT*)psiData->objectTable[loop];

	 	gte_SetRotMatrix(&world->matrixscale);			 
   	gte_SetTransMatrix(&world->matrixscale);	

		gte_ldv0(&world->meshdata->center);

		gte_rtps();

		modctrl->NormTop = world->meshdata->nortop;

		gte_stszotz(&world->depth);

//		utilPrintf("Bloody Actors : %s  !!!\n", psiData->modelName);
		transformVertexListA(world->meshdata->vertop, world->meshdata->vern, tfv, tfd );

		if (modctrl->lighting)
		{
			gte_SetRotMatrix(&world->matrix);			 
	   		//gte_SetTransMatrix(&world->matrix);	
			
			
			np = (SVECTOR*)world->meshdata->nortop;

			for(i = world->meshdata->norn; i != 0; i--)
			{
				gte_ldv0(&np[i]);
				gte_rtv0();
				tfn++;
				gte_stsv(&tfn[0]);
			}
		}

		tfv += world->meshdata->vern;
		tfd += world->meshdata->vern;
		tfTotal += world->meshdata->vern;
	}


	modctrl->PrimTop = (ULONG*)psiData->primitiveList;

	/*
	j = modctrl->depthShift;

	if (j == 0)
	{
		j = DEFAULTDEPTHSHIFT;
	}
	*/


	if (psiData->flags & ACTOR_DYNAMICSORT)
	{

#ifdef DCACHE_OPTIMISATION
	 	dcachePsiCalcMaxMin ( psiData, tfdbase );
//#else
	 	//psiCalcMaxMin ( psiData );
#endif

		world = (PSIOBJECT*)psiData->objectTable[0];

		if (modctrl->depthoverride)
		{
			depth = modctrl->depthoverride;
		}
		else
		{
			depth = world->depth;
		}
		
		modctrl->PrimLeft = psiData->noofPrims;
		modctrl->VertTop = world->meshdata->vertop;

#ifdef DCACHE_OPTIMISATION
		dcachePsiSortPrimitives ( tfvbase, tfdbase );
//#else
//		psiSortPrimitives();
#endif

		modctrl->polysdrawn = sortCount;

		//if(customDrawFunction)
			//customDrawFunction(depth >> modctrl->depthShift);
		//else
			//psiDrawSortedPrimitives(depth >> modctrl->depthShift);
#ifdef DCACHE_OPTIMISATION
		dcache_LSCAPE_DrawSortedPrimitives ( depth/* >> modctrl->depthShift*/, tfvbase );
//#else
//		LSCAPE_DrawSortedPrimitives ( depth >> modctrl->depthShift );
#endif
	}
	else
	{
		char *compare;

		totalObjs += obs;


		for (loop = 0; loop < obs; loop++)
		{
			world = (PSIOBJECT*)psiData->objectTable[loop];
	
			if (modctrl->depthoverride)
			{
				depth = modctrl->depthoverride;
			}
			else
			{
				depth = world->depth;
			}
	
			modctrl->VertTop = world->meshdata->vertop;
		 	modctrl->SortOffs = world->meshdata->sortlistptr[s];
			modctrl->PrimLeft = world->meshdata->sortlistsize[s];

			//if ( ( compare = strstr ( psiData->modelName, "BACKDROP" ) ) )
			//{
				//depth = 4095;
				//modctrl->depthShift = 0;
			//}

		
			// JH: Don't bother with this object if it's to close to the camera.
			if ( (depth >> modctrl->depthShift) < 10 )
			 continue;


#ifdef DCACHE_OPTIMISATION
			dcacheCustomDrawPrimitives2 ( depth >> modctrl->depthShift, tfvbase, tfdbase );
//#else
//			customDrawPrimitives2 ( depth >> modctrl->depthShift );
#endif
			//customDrawFunction2();
		}
	}
}


static void psiSetRotateKeyFrames(PSIOBJECT *world, ULONG frame)
{		  
	MATRIX		rotmat1;
	SQKEYFRAME	*tmprotatekeys,*tmprotatekeyslast;
	long		keystep;
	long		t;
	PSIMESH		*mesh;
	
	while(world)
	{
		mesh = world->meshdata;
		if ( !mesh )
			return;

		tmprotatekeys = mesh->rotatekeys;
		
		if ((!frame) || (mesh->numRotateKeys<=1))
		{
			ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
		}
		else
		{
			////////////////////////////////////////////
			// bin search
			////////////////////////////////////////////
			keystep = mesh->numRotateKeys / 2;

			tmprotatekeys = &mesh->rotatekeys[keystep];
			
			if ( keystep>1 )
			{
				keystep /= 2;
			}
			
			while (frame != tmprotatekeys->time)
			{

				if (frame > tmprotatekeys->time)
				{
					if ( frame <= tmprotatekeys[1].time )
					{
						tmprotatekeys++;
						break;
					}
					else
					{
						tmprotatekeys += keystep;
					}
				}
				else
				{
					tmprotatekeys -= keystep;
				}

				if ( keystep>1 )
				{
					keystep /= 2;
				}
			}
			////////////////////////////////////////////
			
			if (tmprotatekeys->time == frame) // it's on the keyframe 
			{
				ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
			}
			else // work out the differences 
			{
				tmprotatekeyslast = tmprotatekeys - 1;

				if((tmprotatekeys->vect.x == tmprotatekeyslast->vect.x) && (tmprotatekeys->vect.y == tmprotatekeyslast->vect.y) && (tmprotatekeys->vect.z == tmprotatekeyslast->vect.z) && (tmprotatekeys->vect.w == tmprotatekeyslast->vect.w))
				{
					ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
				}
				else
				{
					t =  tmprotatekeyslast->time;
					t =  ((frame - t) << 12)/(tmprotatekeys->time - t);
					ShortquaternionSlerpMatrix((SHORTQUAT *)&tmprotatekeyslast->vect,
									(SHORTQUAT *)&tmprotatekeys->vect,
									t,
									&world->matrix);
				}
			}
		}
	   
		if((world->rotate.vx) || (world->rotate.vy) || (world->rotate.vz))
		{
			RotMatrixYXZ_gte(&world->rotate,&rotmat1);
			gte_MulMatrix0(&rotmat1,&world->matrix,&world->matrix);
		}

		
		if(world->child)
		{
			psiSetRotateKeyFrames(world->child,frame);
		}
		world = world->next;
	}
}

static void psiSetScaleKeyFrames(PSIOBJECT *world, ULONG frame)
{
	SVKEYFRAME	*tmpscalekeys,*tmpscalekeyslast;
	USHORT		oldframe=frame;
	LONG		t;
	LONG keystep;
	PSIMESH		*mesh;


	while(world)
	{
		mesh = world->meshdata;
		if ( !mesh )
			return;

		tmpscalekeys = mesh->scalekeys;

		if ((!frame) || (mesh->numScaleKeys<=1))
		{
			world->scale.vx = (tmpscalekeys->vect.x)<<2;
			world->scale.vy = (tmpscalekeys->vect.y)<<2;
			world->scale.vz = (tmpscalekeys->vect.z)<<2;
		}
		else
		{
			////////////////////////////////////////////
			// bin search
			////////////////////////////////////////////
			keystep = mesh->numScaleKeys / 2;

			tmpscalekeys = &mesh->scalekeys[keystep];
			
			if ( keystep>1 )
			{
				keystep /= 2;
			}
			
			while (frame != tmpscalekeys->time)
			{

				if (frame > tmpscalekeys->time)
				{
					if ( frame <= tmpscalekeys[1].time )
					{
						tmpscalekeys++;
						break;
					}
					else
					{
						tmpscalekeys += keystep;
					}
				}
				else
				{
					tmpscalekeys -= keystep;
				}

				if ( keystep>1 )
				{
					keystep /= 2;
				}
			}
			////////////////////////////////////////////

			if(tmpscalekeys->time == frame)	// it's on the keyframe 
			{
				world->scale.vx = (tmpscalekeys->vect.x)<<2;
				world->scale.vy = (tmpscalekeys->vect.y)<<2;
				world->scale.vz = (tmpscalekeys->vect.z)<<2;
			}
			else // work out the differences 
			{
				tmpscalekeyslast = tmpscalekeys - 1;

				if((tmpscalekeys->vect.x == tmpscalekeyslast->vect.x) && (tmpscalekeys->vect.y == tmpscalekeyslast->vect.y) && (tmpscalekeys->vect.z == tmpscalekeyslast->vect.z))
				{
					world->scale.vx = (tmpscalekeys->vect.x)<<2;
					world->scale.vy = (tmpscalekeys->vect.y)<<2;
					world->scale.vz = (tmpscalekeys->vect.z)<<2;
				}
				else
				{
					VECTOR temp1;
					VECTOR temp2;
					temp1.vx =((int)(tmpscalekeyslast->vect.x))<<2;
					temp1.vy =((int)(tmpscalekeyslast->vect.y))<<2;
					temp1.vz =((int)(tmpscalekeyslast->vect.z))<<2;
					temp2.vx =((int)(tmpscalekeys->vect.x))<<2;
					temp2.vy =((int)(tmpscalekeys->vect.y))<<2;
					temp2.vz =((int)(tmpscalekeys->vect.z))<<2;

					t = tmpscalekeyslast->time;
					t = ((frame - t) << 12) / (tmpscalekeys->time - t);

					gte_lddp(t);							// load interpolant
					gte_ldlvl(&temp1);						// load source
					gte_ldfc(&temp2);						// load dest
					gte_intpl();							// interpolate (8 cycles)
					gte_stlvnl(&world->scale);				// store interpolated vector

				}

			}
		}	

		if(world->child)
		{
			psiSetScaleKeyFrames(world->child,oldframe);
		}

		world = world->next;
	}
}

static void psiSetMoveKeyFrames(PSIOBJECT *world, ULONG frame)
{
	
	register SVKEYFRAME	*workingkeys,*tmpmovekeys;
	LONG		t;
	VECTOR		source, dest;
	long		keystep;
	PSIMESH 	*mesh;


	while(world)
	{

		mesh = world->meshdata;

		if ( !mesh )
			return;

		tmpmovekeys = mesh->movekeys;//+frame;

		if ((!frame) || (mesh->numMoveKeys<=1) )
		{
			world->matrix.t[0] = (tmpmovekeys->vect.x);
			world->matrix.t[1] = -(tmpmovekeys->vect.y);
			world->matrix.t[2] = (tmpmovekeys->vect.z);
		}
		else
		{
			////////////////////////////////////////////
			// bin search
			////////////////////////////////////////////
			keystep = mesh->numMoveKeys / 2;

			tmpmovekeys = &mesh->movekeys[keystep];
			
			if ( keystep>1 )
			{
				keystep /= 2;
			}
			
			while (frame != tmpmovekeys->time)
			{
				if (frame > tmpmovekeys->time)
				{
					if ( frame <= tmpmovekeys[1].time )
					{
						tmpmovekeys++;
						break;
					}
					else
					{
						tmpmovekeys += keystep;
					}
				}
				else
				{
					tmpmovekeys -= keystep;
				}

				if ( keystep>1 )
				{
					keystep /= 2;
				}
			}
			////////////////////////////////////////////

			if(tmpmovekeys->time == frame)	// it's on the keyframe 
			{
				world->matrix.t[0] = (tmpmovekeys->vect.x);
				world->matrix.t[1] = -(tmpmovekeys->vect.y);
				world->matrix.t[2] = (tmpmovekeys->vect.z);
			}
			else // work out the differences 
			{
				workingkeys = tmpmovekeys - 1;

				if((tmpmovekeys->vect.x == workingkeys->vect.x) && (tmpmovekeys->vect.y == workingkeys->vect.y) && (tmpmovekeys->vect.z == workingkeys->vect.z))
				{
					world->matrix.t[0] = (tmpmovekeys->vect.x);
					world->matrix.t[1] = -(tmpmovekeys->vect.y);
					world->matrix.t[2] = (tmpmovekeys->vect.z);
				}
				else
				{
					t = workingkeys->time;
					t = ((frame - t) << 12) / (tmpmovekeys->time - t);
					
					source.vx = workingkeys->vect.x;
					source.vy = -workingkeys->vect.y;
					source.vz = workingkeys->vect.z;

					dest.vx = tmpmovekeys->vect.x;
					dest.vy = -tmpmovekeys->vect.y;
					dest.vz = tmpmovekeys->vect.z;

					gte_lddp(t);							// load interpolant
					gte_ldlvl(&source);						// load source
					gte_ldfc(&dest);						// load dest
					gte_intpl();							// interpolate (8 cycles)
					gte_stlvnl(&world->matrix.t);			// store interpolated vector
				}
			}
		}	
		if(world->child)
		{
			psiSetMoveKeyFrames(world->child,frame);
		}
		
		world = world->next;
	}
}


void psiSetKeyFrames(PSIOBJECT *world, ULONG frame)
{
	psiSetMoveKeyFrames(world, frame);
	psiSetScaleKeyFrames(world, frame);
	psiSetRotateKeyFrames(world, frame);
}

/*static void psiSetRotateKeyFrames3(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG b, PSIOBJECT *root)
{		  
	MATRIX		rotmat1;
	SQKEYFRAME	*tmprotatekeys,*tmprotatekeyslast;
	long		keystep;
	long		t;
	PSIMESH		*mesh;
	ULONG		frame[2];
	SHORTQUAT	quat[2];
	int			loop;
	VECTOR		result;

	frame[0] = frame0;
	frame[1] = frame1;
	
	while(world)
	{
		mesh = world->meshdata;

		if ( !mesh )
			return;

		for(loop = 0; loop < 2; loop ++)
		{
			tmprotatekeys = mesh->rotatekeys;
			
			if ((!frame[loop]) || (mesh->numRotateKeys<=1))
			{
				// get quaternion rather than a matrix
				quat[loop].x = tmprotatekeys->vect.x;
				quat[loop].y = tmprotatekeys->vect.y;
				quat[loop].z = tmprotatekeys->vect.z;
				quat[loop].w = tmprotatekeys->vect.w;
			}
			else
			{
				////////////////////////////////////////////
				// bin search
				////////////////////////////////////////////
				keystep = mesh->numRotateKeys / 2;

				tmprotatekeys = &mesh->rotatekeys[keystep];
				
				if ( keystep>1 )
				{
					keystep /= 2;
				}
				
				while (frame[loop] != tmprotatekeys->time)
				{

					if (frame[loop] > tmprotatekeys->time)
					{
						if ( frame[loop] <= tmprotatekeys[1].time )
						{
							tmprotatekeys++;
							break;
						}
						else
						{
							tmprotatekeys += keystep;
						}
					}
					else
					{
						tmprotatekeys -= keystep;
					}

					if ( keystep>1 )
					{
						keystep /= 2;
					}
				}
				////////////////////////////////////////////
				
				if (tmprotatekeys->time == frame[loop]) // it's on the keyframe 
				{
					// get quaternion rather than a matrix
					quat[loop].x = tmprotatekeys->vect.x;
					quat[loop].y = tmprotatekeys->vect.y;
					quat[loop].z = tmprotatekeys->vect.z;
					quat[loop].w = tmprotatekeys->vect.w;
				}
				else // work out the differences 
				{
					tmprotatekeyslast = tmprotatekeys - 1;

					if((tmprotatekeys->vect.x == tmprotatekeyslast->vect.x) && (tmprotatekeys->vect.y == tmprotatekeyslast->vect.y) && (tmprotatekeys->vect.z == tmprotatekeyslast->vect.z) && (tmprotatekeys->vect.w == tmprotatekeyslast->vect.w))
					{
						// get quaternion rather than a matrix
						quat[loop].x = tmprotatekeys->vect.x;
						quat[loop].y = tmprotatekeys->vect.y;
						quat[loop].z = tmprotatekeys->vect.z;
						quat[loop].w = tmprotatekeys->vect.w;
					}
					else
					{
						// calculate blended quaternion rather than a matrix

						t =  ((frame[loop] - tmprotatekeyslast->time) << 12)/(tmprotatekeys->time - tmprotatekeyslast->time);
						t =  tmprotatekeyslast->time;
						t =  ((frame[loop] - t) << 12)/(tmprotatekeys->time - t);

						

						gte_ld_intpol_sv1(&tmprotatekeyslast->vect);	// load x, y, z
						gte_ld_intpol_sv0(&tmprotatekeys->vect);		// load x, y, z
						gte_lddp(t);			// load interpolant
						gte_intpl();			// interpolate (8 cycles)
					
						quat[loop].w = (((4096 - t) * tmprotatekeyslast->vect.w) + (t * tmprotatekeys->vect.w)) / 4096;
					
						gte_stlvnl(&result);	// put interpolated x y & z into result

						quat[loop].x = result.vx;
						quat[loop].y = result.vy;
						quat[loop].z = result.vz;
					}
				}
			}
		}

		// get matrix from blended quaternions
		//ShortquaternionSlerpMatrix(&quat[0],&quat[1],b,&world->matrix);
		{
			LONG	s, xs,ys,zs, wx,wy,wz, xx,xy,xz, yy,yz,zz/*, cosom, tempcalc*/;
		/*	VECTOR	source, sqrin, sqrout;

			/*
			// calc cosine (dot product)
			cosom = (quat[0].x * quat[1].x + quat[0].y * quat[1].y + quat[0].z * quat[1].z
			+ quat[0].w * quat[1].w);

			// adjust signs (if necessary)
			
			if (cosom < 0)
			{
				//cosom = -cosom;
				quat[1].x = -quat[1].x;
				quat[1].y = -quat[1].y;
				quat[1].z = -quat[1].z;
				quat[1].w = -quat[1].w;
			}
			*/
			   
			// "from" and "to" quaternions are very close 
			//  ... so we can do a linear interpolation
			
	/*		gte_ld_intpol_sv1(&quat[0]);
			gte_ld_intpol_sv0(&quat[1]);

			gte_lddp(b);			// load interpolant
			gte_intpl();			// interpolate (8 cycles)
			
			// interpolate w manually while we wait for the GTE
			source.vx = ((4096 - b) * quat[0].w);
			source.vx += (b * quat[1].w);
			source.vx /= 4096;

			gte_stlvnl(&sqrin);		// put interpolated x y & z into sqrin

			gte_ldlvl(&sqrin);		// load interpolated x y & z
			gte_sqr0();				// square (5 cycles)
			
			// square w while we wait for the GTE
			source.vy = source.vx * source.vx;

			gte_stlvnl(&sqrout);	// put into sqrout

			s = sqrout.vx + sqrout.vy + sqrout.vz + source.vy;

			s = (s >> 12);
				
			s = (s > 1) ? (33554432 / s) : (33554432);
			
			xs = (sqrin.vx * s) >> 12;
			ys = (sqrin.vy * s) >> 12;
			zs = (sqrin.vz * s) >> 12;

			wx = (source.vx * xs) >> 12;
			wy = (source.vx * ys) >> 12;
			wz = (source.vx * zs) >> 12;

			xx = (sqrin.vx * xs) >> 12;
			xy = (sqrin.vx * ys) >> 12;
			xz = (sqrin.vx * zs) >> 12;

			yy = (sqrin.vy * ys) >> 12;
			yz = (sqrin.vy * zs) >> 12;
			zz = (sqrin.vz * zs) >> 12;

			world->matrix.m[0][0] = 4096 - (yy + zz);
			world->matrix.m[0][1] = xy + wz;
			world->matrix.m[0][2] = xz - wy;

			world->matrix.m[1][0] = xy - wz;
			world->matrix.m[1][1] = 4096 - (xx + zz);
			world->matrix.m[1][2] = yz + wx;

			world->matrix.m[2][0] = xz + wy;
			world->matrix.m[2][1] = yz - wx;
			world->matrix.m[2][2] = 4096 - (xx + yy);
		}

		if((world->rotate.vx) || (world->rotate.vy) || (world->rotate.vz))
		{
			RotMatrixYXZ_gte(&world->rotate,&rotmat1);
			gte_MulMatrix0(&rotmat1,&world->matrix,&world->matrix);
		}
		
		if(world->child)
		{
			psiSetRotateKeyFrames3(world->child,frame0, frame1, b, root);
		}

		if(world != root)
			world = world->next;
		else
			world = 0;
	}
}*/

/*static void psiSetScaleKeyFrames3(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG b, PSIOBJECT *root)
{
	SVKEYFRAME	*tmpscalekeys,*tmpscalekeyslast;
	LONG		t;
	LONG		keystep;
	PSIMESH		*mesh;
	SVECTOR		scale[3];
	ULONG		frame[2];
	int			loop;

	frame[0] = frame0;
	frame[1] = frame1;


	while(world)
	{
		mesh = world->meshdata;

		if ( !mesh )
			return;

		for(loop = 0; loop < 2; loop ++)
		{
			tmpscalekeys = mesh->scalekeys;

			if ((!frame[loop]) || (mesh->numScaleKeys<=1))
			{
				scale[loop].vx = (tmpscalekeys->vect.x)<<2;
				scale[loop].vy = (tmpscalekeys->vect.y)<<2;
				scale[loop].vz = (tmpscalekeys->vect.z)<<2;
			}
			else
			{
				////////////////////////////////////////////
				// bin search
				////////////////////////////////////////////
				keystep = mesh->numScaleKeys / 2;

				tmpscalekeys = &mesh->scalekeys[keystep];
				
				if ( keystep>1 )
				{
					keystep /= 2;
				}
				
				while (frame[loop] != tmpscalekeys->time)
				{

					if (frame[loop] > tmpscalekeys->time)
					{
						if ( frame[loop] <= tmpscalekeys[1].time )
						{
							tmpscalekeys++;
							break;
						}
						else
						{
							tmpscalekeys += keystep;
						}
					}
					else
					{
						tmpscalekeys -= keystep;
					}

					if ( keystep>1 )
					{
						keystep /= 2;
					}
				}
				////////////////////////////////////////////

				if(tmpscalekeys->time == frame[loop])	// it's on the keyframe 
				{
					scale[loop].vx = (tmpscalekeys->vect.x)<<2;
					scale[loop].vy = (tmpscalekeys->vect.y)<<2;
					scale[loop].vz = (tmpscalekeys->vect.z)<<2;
				}
				else // work out the differences 
				{
					tmpscalekeyslast = tmpscalekeys - 1;

					if((tmpscalekeys->vect.x == tmpscalekeyslast->vect.x) && (tmpscalekeys->vect.y == tmpscalekeyslast->vect.y) && (tmpscalekeys->vect.z == tmpscalekeyslast->vect.z))
					{
						scale[loop].vx = (tmpscalekeys->vect.x)<<2;
						scale[loop].vy = (tmpscalekeys->vect.y)<<2;
						scale[loop].vz = (tmpscalekeys->vect.z)<<2;
					}
					else
					{
						VECTOR temp1;
						VECTOR temp2;
						VECTOR dest;
						
						temp1.vx =((int)(tmpscalekeyslast->vect.x));
						temp1.vy =((int)(tmpscalekeyslast->vect.y));
						temp1.vz =((int)(tmpscalekeyslast->vect.z));
						temp2.vx =((int)(tmpscalekeys->vect.x));
						temp2.vy =((int)(tmpscalekeys->vect.y));
						temp2.vz =((int)(tmpscalekeys->vect.z));
						
						t = tmpscalekeyslast->time;
						t = ((frame[loop] - t) << 12) / (tmpscalekeys->time - t);

						gte_lddp(t);							// load interpolant
						gte_ldlvl(&temp1);						// load source
						gte_ldfc(&temp2);						// load dest
						gte_intpl();							// interpolate (8 cycles)
						gte_stlvnl(&dest);				// store interpolated vector
						scale[loop].vx = dest.vx<<2;
						scale[loop].vy = dest.vy<<2;
						scale[loop].vz = dest.vz<<2;

					}

				}
			}
		}

		// interpolate between scale values
		gte_ld_intpol_sv1(&scale[0]);
		gte_ld_intpol_sv0(&scale[1]);
		gte_lddp(b);
		gte_intpl();
		gte_stlvnl(&world->scale);

		if(world->child)
		{
			psiSetScaleKeyFrames3(world->child,frame0, frame1, b, root);
		}

		if(world != root)
			world = world->next;
		else
			world = 0;
	}
}*/

/*static void psiSetMoveKeyFrames3(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG b, PSIOBJECT *root)
{
	
	register SVKEYFRAME	*workingkeys,*tmpmovekeys;
	LONG		t;
	VECTOR		dest;
	long		keystep;
	PSIMESH 	*mesh;
	SVECTOR		move[3];
	int			loop;
	ULONG		frame[2];


	frame[0] = frame0;
	frame[1] = frame1;


	while(world)
	{

		mesh = world->meshdata;

		if ( !mesh )
			return;

		for(loop = 0; loop < 2; loop ++)
		{
			tmpmovekeys = mesh->movekeys;//+frame;

			if ((!frame[loop]) || (mesh->numMoveKeys<=1) )
			{
				move[loop].vx = (tmpmovekeys->vect.x);
				move[loop].vy = -(tmpmovekeys->vect.y);
				move[loop].vz = (tmpmovekeys->vect.z);
			}
			else
			{
				////////////////////////////////////////////
				// bin search
				////////////////////////////////////////////
				keystep = mesh->numMoveKeys / 2;

				tmpmovekeys = &mesh->movekeys[keystep];
				
				if ( keystep>1 )
				{
					keystep /= 2;
				}
				
				while (frame[loop] != tmpmovekeys->time)
				{
					if (frame[loop] > tmpmovekeys->time)
					{
						if ( frame[loop] <= tmpmovekeys[1].time )
						{
							tmpmovekeys++;
							break;
						}
						else
						{
							tmpmovekeys += keystep;
						}
					}
					else
					{
						tmpmovekeys -= keystep;
					}

					if ( keystep>1 )
					{
						keystep /= 2;
					}
				}
				////////////////////////////////////////////

				if(tmpmovekeys->time == frame[loop])	// it's on the keyframe 
				{
					move[loop].vx = (tmpmovekeys->vect.x);
					move[loop].vy = -(tmpmovekeys->vect.y);
					move[loop].vz = (tmpmovekeys->vect.z);
				}
				else // work out the differences 
				{
					workingkeys = tmpmovekeys - 1;

					if((tmpmovekeys->vect.x == workingkeys->vect.x) && (tmpmovekeys->vect.y == workingkeys->vect.y) && (tmpmovekeys->vect.z == workingkeys->vect.z))
					{
						move[loop].vx = (tmpmovekeys->vect.x);
						move[loop].vy = -(tmpmovekeys->vect.y);
						move[loop].vz = (tmpmovekeys->vect.z);
					}
					else
					{
						t = workingkeys->time;
						t = ((frame[loop] - t) << 12) / (tmpmovekeys->time - t);
						
						gte_ld_intpol_sv1(&workingkeys->vect);
						gte_ld_intpol_sv0(&tmpmovekeys->vect);

						gte_lddp(t);							// load interpolant
						gte_intpl();							// interpolate (8 cycles)
						gte_stlvnl(&dest);						// store interpolated vector

						move[loop].vx = dest.vx;
						move[loop].vy = -dest.vy;
						move[loop].vz = dest.vz;
					}
				}
			}
		}

		// interpolate between frame0 and frame1
		
		gte_lddp(b);							// load interpolant
		gte_ld_intpol_sv1(&move[0]);
		gte_ld_intpol_sv0(&move[1]);
		gte_intpl();							// interpolate (8 cycles)
		gte_stlvnl(&world->matrix.t);			// store interpolated vector
	
		if(world->child)
		{
			psiSetMoveKeyFrames3(world->child, frame0, frame1, b, root);
		}
		
		if(world != root)
			world = world->next;
		else
			world = 0;
	}
}*/


/*void psiSetKeyFrames3(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG blend)
{
	psiSetMoveKeyFrames3(world, frame0, frame1, blend, world);
	psiSetScaleKeyFrames3(world, frame0, frame1, blend, world);
	psiSetRotateKeyFrames3(world, frame0, frame1, blend, world);
}*/


/*static void psiSetRotateKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG b)
{		  
	MATRIX		rotmat1;
	SQKEYFRAME	*tmprotatekeys,*tmprotatekeyslast;
	long		keystep;
	long		t;
	PSIMESH		*mesh;
	ULONG		frame[2];
	SHORTQUAT	quat[2];
	int			loop;
	VECTOR		result;

	frame[0] = frame0;
	frame[1] = frame1;
	
	while(world)
	{
		mesh = world->meshdata;

		if ( !mesh )
			return;

		for(loop = 0; loop < 2; loop ++)
		{
			tmprotatekeys = mesh->rotatekeys;
			
			if ((!frame[loop]) || (mesh->numRotateKeys<=1))
			{
				// get quaternion rather than a matrix
				quat[loop].x = tmprotatekeys->vect.x;
				quat[loop].y = tmprotatekeys->vect.y;
				quat[loop].z = tmprotatekeys->vect.z;
				quat[loop].w = tmprotatekeys->vect.w;
			}
			else
			{
				////////////////////////////////////////////
				// bin search
				////////////////////////////////////////////
				keystep = mesh->numRotateKeys / 2;

				tmprotatekeys = &mesh->rotatekeys[keystep];
				
				if ( keystep>1 )
				{
					keystep /= 2;
				}
				
				while (frame[loop] != tmprotatekeys->time)
				{

					if (frame[loop] > tmprotatekeys->time)
					{
						if ( frame[loop] <= tmprotatekeys[1].time )
						{
							tmprotatekeys++;
							break;
						}
						else
						{
							tmprotatekeys += keystep;
						}
					}
					else
					{
						tmprotatekeys -= keystep;
					}

					if ( keystep>1 )
					{
						keystep /= 2;
					}
				}
				////////////////////////////////////////////
				
				if (tmprotatekeys->time == frame[loop]) // it's on the keyframe 
				{
					// get quaternion rather than a matrix
					quat[loop].x = tmprotatekeys->vect.x;
					quat[loop].y = tmprotatekeys->vect.y;
					quat[loop].z = tmprotatekeys->vect.z;
					quat[loop].w = tmprotatekeys->vect.w;
				}
				else // work out the differences 
				{
					tmprotatekeyslast = tmprotatekeys - 1;

					if((tmprotatekeys->vect.x == tmprotatekeyslast->vect.x) && (tmprotatekeys->vect.y == tmprotatekeyslast->vect.y) && (tmprotatekeys->vect.z == tmprotatekeyslast->vect.z) && (tmprotatekeys->vect.w == tmprotatekeyslast->vect.w))
					{
						// get quaternion rather than a matrix
						quat[loop].x = tmprotatekeys->vect.x;
						quat[loop].y = tmprotatekeys->vect.y;
						quat[loop].z = tmprotatekeys->vect.z;
						quat[loop].w = tmprotatekeys->vect.w;
					}
					else
					{
						// calculate blended quaternion rather than a matrix

						t =  ((frame[loop] - tmprotatekeyslast->time) << 12)/(tmprotatekeys->time - tmprotatekeyslast->time);
						t =  tmprotatekeyslast->time;
						t =  ((frame[loop] - t) << 12)/(tmprotatekeys->time - t);

						

						gte_ld_intpol_sv1(&tmprotatekeyslast->vect);	// load x, y, z
						gte_ld_intpol_sv0(&tmprotatekeys->vect);		// load x, y, z
						gte_lddp(t);			// load interpolant
						gte_intpl();			// interpolate (8 cycles)
					
						quat[loop].w = (((4096 - t) * tmprotatekeyslast->vect.w) + (t * tmprotatekeys->vect.w)) / 4096;
					
						gte_stlvnl(&result);	// put interpolated x y & z into result

						quat[loop].x = result.vx;
						quat[loop].y = result.vy;
						quat[loop].z = result.vz;
					}
				}
			}
		}

		// get matrix from blended quaternions
		//ShortquaternionSlerpMatrix(&quat[0],&quat[1],b,&world->matrix);
		{
			LONG	s, xs,ys,zs, wx,wy,wz, xx,xy,xz, yy,yz,zz/*, cosom, tempcalc*/;
	/*		VECTOR	source, sqrin, sqrout;

			/*
			// calc cosine (dot product)
			cosom = (quat[0].x * quat[1].x + quat[0].y * quat[1].y + quat[0].z * quat[1].z
			+ quat[0].w * quat[1].w);

			// adjust signs (if necessary)
			
			if (cosom < 0)
			{
				//cosom = -cosom;
				quat[1].x = -quat[1].x;
				quat[1].y = -quat[1].y;
				quat[1].z = -quat[1].z;
				quat[1].w = -quat[1].w;
			}
			*/
			   
			// "from" and "to" quaternions are very close 
			//  ... so we can do a linear interpolation
			
	/*		gte_ld_intpol_sv1(&quat[0]);
			gte_ld_intpol_sv0(&quat[1]);

			gte_lddp(b);			// load interpolant
			gte_intpl();			// interpolate (8 cycles)
			
			// interpolate w manually while we wait for the GTE
			source.vx = ((4096 - b) * quat[0].w);
			source.vx += (b * quat[1].w);
			source.vx /= 4096;

			gte_stlvnl(&sqrin);		// put interpolated x y & z into sqrin

			gte_ldlvl(&sqrin);		// load interpolated x y & z
			gte_sqr0();				// square (5 cycles)
			
			// square w while we wait for the GTE
			source.vy = source.vx * source.vx;

			gte_stlvnl(&sqrout);	// put into sqrout

			s = sqrout.vx + sqrout.vy + sqrout.vz + source.vy;

			s = (s >> 12);
				
			s = (s > 1) ? (33554432 / s) : (33554432);
			
			xs = (sqrin.vx * s) >> 12;
			ys = (sqrin.vy * s) >> 12;
			zs = (sqrin.vz * s) >> 12;

			wx = (source.vx * xs) >> 12;
			wy = (source.vx * ys) >> 12;
			wz = (source.vx * zs) >> 12;

			xx = (sqrin.vx * xs) >> 12;
			xy = (sqrin.vx * ys) >> 12;
			xz = (sqrin.vx * zs) >> 12;

			yy = (sqrin.vy * ys) >> 12;
			yz = (sqrin.vy * zs) >> 12;
			zz = (sqrin.vz * zs) >> 12;

			world->matrix.m[0][0] = 4096 - (yy + zz);
			world->matrix.m[0][1] = xy + wz;
			world->matrix.m[0][2] = xz - wy;

			world->matrix.m[1][0] = xy - wz;
			world->matrix.m[1][1] = 4096 - (xx + zz);
			world->matrix.m[1][2] = yz + wx;

			world->matrix.m[2][0] = xz + wy;
			world->matrix.m[2][1] = yz - wx;
			world->matrix.m[2][2] = 4096 - (xx + yy);
		}

		if((world->rotate.vx) || (world->rotate.vy) || (world->rotate.vz))
		{
			RotMatrixYXZ_gte(&world->rotate,&rotmat1);
			gte_MulMatrix0(&rotmat1,&world->matrix,&world->matrix);
		}
		
		if(world->child)
		{
			psiSetRotateKeyFrames2(world->child,frame0, frame1, b);
		}
		world = world->next;
	}
}*/

/*static void psiSetScaleKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG b)
{
	SVKEYFRAME	*tmpscalekeys,*tmpscalekeyslast;
	LONG		t;
	LONG		keystep;
	PSIMESH		*mesh;
	SVECTOR		scale[3];
	ULONG		frame[2];
	int			loop;

	frame[0] = frame0;
	frame[1] = frame1;


	while(world)
	{
		mesh = world->meshdata;

		if ( !mesh )
			return;

		for(loop = 0; loop < 2; loop ++)
		{
			tmpscalekeys = mesh->scalekeys;

			if ((!frame[loop]) || (mesh->numScaleKeys<=1))
			{
				scale[loop].vx = (tmpscalekeys->vect.x)<<2;
				scale[loop].vy = (tmpscalekeys->vect.y)<<2;
				scale[loop].vz = (tmpscalekeys->vect.z)<<2;
			}
			else
			{
				////////////////////////////////////////////
				// bin search
				////////////////////////////////////////////
				keystep = mesh->numScaleKeys / 2;

				tmpscalekeys = &mesh->scalekeys[keystep];
				
				if ( keystep>1 )
				{
					keystep /= 2;
				}
				
				while (frame[loop] != tmpscalekeys->time)
				{

					if (frame[loop] > tmpscalekeys->time)
					{
						if ( frame[loop] <= tmpscalekeys[1].time )
						{
							tmpscalekeys++;
							break;
						}
						else
						{
							tmpscalekeys += keystep;
						}
					}
					else
					{
						tmpscalekeys -= keystep;
					}

					if ( keystep>1 )
					{
						keystep /= 2;
					}
				}
				////////////////////////////////////////////

				if(tmpscalekeys->time == frame[loop])	// it's on the keyframe 
				{
					scale[loop].vx = (tmpscalekeys->vect.x)<<2;
					scale[loop].vy = (tmpscalekeys->vect.y)<<2;
					scale[loop].vz = (tmpscalekeys->vect.z)<<2;
				}
				else // work out the differences 
				{
					tmpscalekeyslast = tmpscalekeys - 1;

					if((tmpscalekeys->vect.x == tmpscalekeyslast->vect.x) && (tmpscalekeys->vect.y == tmpscalekeyslast->vect.y) && (tmpscalekeys->vect.z == tmpscalekeyslast->vect.z))
					{
						scale[loop].vx = (tmpscalekeys->vect.x)<<2;
						scale[loop].vy = (tmpscalekeys->vect.y)<<2;
						scale[loop].vz = (tmpscalekeys->vect.z)<<2;
					}
					else
					{
						VECTOR temp1;
						VECTOR temp2;
						VECTOR dest;
						
						temp1.vx =((int)(tmpscalekeyslast->vect.x));
						temp1.vy =((int)(tmpscalekeyslast->vect.y));
						temp1.vz =((int)(tmpscalekeyslast->vect.z));
						temp2.vx =((int)(tmpscalekeys->vect.x));
						temp2.vy =((int)(tmpscalekeys->vect.y));
						temp2.vz =((int)(tmpscalekeys->vect.z));
						
						t = tmpscalekeyslast->time;
						t = ((frame[loop] - t) << 12) / (tmpscalekeys->time - t);

						gte_lddp(t);							// load interpolant
						gte_ldlvl(&temp1);						// load source
						gte_ldfc(&temp2);						// load dest
						gte_intpl();							// interpolate (8 cycles)
						gte_stlvnl(&dest);				// store interpolated vector
						scale[loop].vx = dest.vx<<2;
						scale[loop].vy = dest.vy<<2;
						scale[loop].vz = dest.vz<<2;

					}

				}
			}
		}

		// interpolate between scale values
		gte_ld_intpol_sv1(&scale[0]);
		gte_ld_intpol_sv0(&scale[1]);
		gte_lddp(b);
		gte_intpl();
		gte_stlvnl(&world->scale);

		if(world->child)
		{
			psiSetScaleKeyFrames2(world->child,frame0, frame1, b);
		}

		world = world->next;
	}
}*/

/*static void psiSetMoveKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG b, PSIOBJECT *root)
{
	
	register SVKEYFRAME	*workingkeys,*tmpmovekeys;
	LONG		t;
	VECTOR		dest;
	long		keystep;
	PSIMESH 	*mesh;
	SVECTOR		move[3];
	int			loop;
	ULONG		frame[2];


	frame[0] = frame0;
	frame[1] = frame1;


	while(world)
	{

		mesh = world->meshdata;

		if ( !mesh )
			return;

		for(loop = 0; loop < 2; loop ++)
		{
			tmpmovekeys = mesh->movekeys;//+frame;

			if ((!frame[loop]) || (mesh->numMoveKeys<=1) )
			{
				move[loop].vx = (tmpmovekeys->vect.x);
				move[loop].vy = -(tmpmovekeys->vect.y);
				move[loop].vz = (tmpmovekeys->vect.z);
			}
			else
			{
				////////////////////////////////////////////
				// bin search
				////////////////////////////////////////////
				keystep = mesh->numMoveKeys / 2;

				tmpmovekeys = &mesh->movekeys[keystep];
				
				if ( keystep>1 )
				{
					keystep /= 2;
				}
				
				while (frame[loop] != tmpmovekeys->time)
				{
					if (frame[loop] > tmpmovekeys->time)
					{
						if ( frame[loop] <= tmpmovekeys[1].time )
						{
							tmpmovekeys++;
							break;
						}
						else
						{
							tmpmovekeys += keystep;
						}
					}
					else
					{
						tmpmovekeys -= keystep;
					}

					if ( keystep>1 )
					{
						keystep /= 2;
					}
				}
				////////////////////////////////////////////

				if(tmpmovekeys->time == frame[loop])	// it's on the keyframe 
				{
					move[loop].vx = (tmpmovekeys->vect.x);
					move[loop].vy = -(tmpmovekeys->vect.y);
					move[loop].vz = (tmpmovekeys->vect.z);
				}
				else // work out the differences 
				{
					workingkeys = tmpmovekeys - 1;

					if((tmpmovekeys->vect.x == workingkeys->vect.x) && (tmpmovekeys->vect.y == workingkeys->vect.y) && (tmpmovekeys->vect.z == workingkeys->vect.z))
					{
						move[loop].vx = (tmpmovekeys->vect.x);
						move[loop].vy = -(tmpmovekeys->vect.y);
						move[loop].vz = (tmpmovekeys->vect.z);
					}
					else
					{
						t = workingkeys->time;
						t = ((frame[loop] - t) << 12) / (tmpmovekeys->time - t);
						
						gte_ld_intpol_sv1(&workingkeys->vect);
						gte_ld_intpol_sv0(&tmpmovekeys->vect);

						gte_lddp(t);							// load interpolant
						gte_intpl();							// interpolate (8 cycles)
						gte_stlvnl(&dest);						// store interpolated vector

						move[loop].vx = dest.vx;
						move[loop].vy = -dest.vy;
						move[loop].vz = dest.vz;
					}
				}
			}
		}

		// interpolate between frame0 and frame1
		
		// if this is the root bone (motion bone)
		if(world == root)
		{
			gte_lddp(4096);							// load interpolant
			gte_ld_intpol_sv1(&move[0]);
			gte_ld_intpol_sv0(&move[1]);
			gte_intpl();							// interpolate (8 cycles)
			gte_stlvnl(&world->matrix.t);			// store interpolated vector
		}
		else
		{
			gte_lddp(b);							// load interpolant
			gte_ld_intpol_sv1(&move[0]);
			gte_ld_intpol_sv0(&move[1]);
			gte_intpl();							// interpolate (8 cycles)
			gte_stlvnl(&world->matrix.t);			// store interpolated vector
		}
	

		if(world->child)
		{
			psiSetMoveKeyFrames2(world->child, frame0, frame1, b, root);
		}
		
		world = world->next;
	}
}*/


/*void psiSetKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG blend)
{
	psiSetMoveKeyFrames2(world, frame0, frame1, blend, world);
	psiSetScaleKeyFrames2(world, frame0, frame1, blend);
	psiSetRotateKeyFrames2(world, frame0, frame1, blend);
}*/


/**************************************************************************
	FUNCTION:
	PURPOSE:	calc matrices relative to camera
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

static void psiCalcChildMatrix(PSIOBJECT *world, PSIOBJECT *parent)
{

	/*	// get a copy of the local transformation matrix
		world->matrixscale = world->matrix;
		// scale it by our local scale
		ScaleMatrix(&world->matrixscale, &world->scale);
		// multiply our local matrix by our parents (e.g. now in world space, but unscaled)
		gte_MulMatrix0(&parent->matrix, &world->matrix, &world->matrix);
		
		if(PSImodelctrl.inheritScale)
		{
			// if we want to inherit scale from our parent, multiply by our parents scaled matrix
			gte_MulMatrix0(&parent->matrixscale, &world->matrixscale, &world->matrixscale);
		}
		else
		{
			// multiply by our parents unscaled matrix
			gte_MulMatrix0(&parent->matrix, &world->matrixscale, &world->matrixscale);
			// and by the camera & global scale matrix, cuz we dont inherit it
			gte_MulMatrix0(&cameraAndGlobalscale, &world->matrixscale, &world->matrixscale);
		}

		// transform our position vector by our parents scaled matrix
		gte_SetRotMatrix(&parent->matrixscale);
		gte_SetTransMatrix(&parent->matrixscale);
		gte_ldlvl(&world->matrixscale.t);
		gte_rtirtr();
		gte_stlvl(&world->matrixscale.t);

	if ( world->child )
		psiCalcChildMatrix ( world->child, world );

	if ( world->next )
		psiCalcChildMatrix ( world->next, world );
*/

	// while we have siblings
	while(world)
	{
		// get a copy of the local transformation matrix
		world->matrixscale = world->matrix;
		// scale it by our local scale
		ScaleMatrix(&world->matrixscale, &world->scale);
		// multiply our local matrix by our parents (e.g. now in world space, but unscaled)
		gte_MulMatrix0(&parent->matrix, &world->matrix, &world->matrix);
		
		if(PSImodelctrl.inheritScale)
		{
			// if we want to inherit scale from our parent, multiply by our parents scaled matrix
			gte_MulMatrix0(&parent->matrixscale, &world->matrixscale, &world->matrixscale);
		}
		else
		{
			// multiply by our parents unscaled matrix
			gte_MulMatrix0(&parent->matrix, &world->matrixscale, &world->matrixscale);
			// and by the camera & global scale matrix, cuz we dont inherit it
			gte_MulMatrix0(&cameraAndGlobalscale, &world->matrixscale, &world->matrixscale);
		}

		// transform our position vector by our parents scaled matrix
//bb wrong but funny. People wanted this as a cheat.
//		gte_SetRotMatrix(&parent->matrix);
//		gte_SetTransMatrix(&parent->matrixscale);
		gte_SetRotMatrix(&parent->matrixscale);
		gte_SetTransMatrix(&parent->matrixscale);
		gte_ldlvl(&world->matrixscale.t);
		gte_rtirtr();
		gte_stlvl(&world->matrixscale.t);





		// calculate our children if we have any
		if(world->child)
			psiCalcChildMatrix(world->child, world);

		// go to next object

		world = world->next;
	}
}

/**************************************************************************
	FUNCTION:
	PURPOSE:	calc matrices relative to camera
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void psiCalcWorldMatrix(PSIOBJECT *world)
{
	cameraAndGlobalscale = GsWSMATRIX;

	ScaleMatrix(&cameraAndGlobalscale, PSIactorScale);

	world->matrixscale = world->matrix;

	ScaleMatrix(&world->matrixscale,&world->scale);

	gte_MulMatrix0(&cameraAndGlobalscale, &world->matrixscale, &world->matrixscale);

	gte_SetRotMatrix(&GsWSMATRIX);

	gte_SetTransMatrix(&GsWSMATRIX);

	gte_ldlvl(&world->matrixscale.t);

	gte_rtirtr();

	gte_stlvl(&world->matrixscale.t);

	if ( world->child )
		psiCalcChildMatrix(world->child, world);
}

/**************************************************************************
	FUNCTION:
	PURPOSE:	calc matrices relative to camera
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void psiCalcLocalMatrix(PSIOBJECT *world)
{
	cameraAndGlobalscale = GsIDMATRIX;
	ScaleMatrix(&cameraAndGlobalscale, PSIactorScale);
	world->matrixscale = world->matrix;
	ScaleMatrix(&world->matrixscale,&world->scale);
	gte_MulMatrix0(&cameraAndGlobalscale, &world->matrixscale, &world->matrixscale);
	/*
	gte_SetRotMatrix(&GsIDMATRIX);
	gte_SetTransMatrix(&GsIDMATRIX);
	gte_ldlvl(&world->matrixscale.t);
	gte_rtirtr();
	gte_stlvl(&world->matrixscale.t);
	*/

	if(world->child)
		psiCalcChildMatrix(world->child, world);
}

/**************************************************************************
	FUNCTION:
	PURPOSE:	calc matrices relative to model origin
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
/*
void psiCalcLocalMatrix(PSIOBJECT *world,MATRIX *parentM,MATRIX *parentMS)
{

	while(world)
	{

		if (parentM==0)
		{
			world->matrix.t[0] = 0;
			world->matrix.t[1] = 0;
			world->matrix.t[2] = 0;
		}
	   	
	   	world->matrixscale = world->matrix;
	   	ScaleMatrix(&world->matrixscale,&world->scale);

		if (parentM!=0)
	   	{
			gte_MulMatrix0(parentM, &world->matrix, &world->matrix);
			gte_MulMatrix0(parentM, &world->matrixscale, &world->matrixscale);

			gte_SetRotMatrix(parentMS);
			gte_SetTransMatrix(parentMS);
			gte_ldlvl(&world->matrix.t);
			gte_rtirtr();
			gte_stlvl(&world->matrixscale.t);
		}
		else
		{
			gte_MulMatrix0(PSIrootScale, &world->matrix, &world->matrix);
		}

		if(world->child)
		{
			psiCalcLocalMatrix(world->child,&world->matrix,&world->matrixscale);
		}

		world = world->next;
	}
}
*/

void psiDebug()
{

}

/*
void DrawBoundingBox(ACTOR *actor)
{

	BOUNDINGBOX *corners;
	SVECTOR v;
	long lx,rx,ty,by,ison;
	DVECTOR scrxy1,scrxy2;

	PSISetBoundingRotated(actor,actor->animation.frame,actor->object->rotate.vx,actor->object->rotate.vy,actor->object->rotate.vz);
//	PSISetBounding(actor,0);

	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);

	corners = &actor->bounding;


	polyLine.r0 = 240;
	polyLine.g0 = 240;
	polyLine.b0 = 0;

	v.vx = corners->minX+actor->position.vx;
	v.vy = corners->minY+actor->position.vy;
	v.vz = corners->minZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

	v.vx = corners->maxX+actor->position.vx;
	v.vy = corners->maxY+actor->position.vy;
	v.vz = corners->minZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);


	v.vx = corners->maxX+actor->position.vx;
	v.vy = corners->minY+actor->position.vy;
	v.vz = corners->maxZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

	v.vx = corners->minX+actor->position.vx;
	v.vy = corners->maxY+actor->position.vy;
	v.vz = corners->maxZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		DrawRadiusBox(actor);
}

*/



