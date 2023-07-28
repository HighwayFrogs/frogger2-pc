
#include "sonylibs.h"
#include "shell.h"

#include "islpsi.h"
//y#include <islpsi.h>
#include <psitypes.h>

#include "enemies.h"
#include <islutil.h>
#include "timer.h"
#include "psxsprite.h"


#define GETX(n)( ((SHORTXY *)( (int)(tfv) +(n) ))->x )
#define GETY(n)( ((SHORTXY *)( (int)(tfv) +(n) ))->y )
#define GETV(n)(  tfv[n] )
#define GETD(n)(  tfd[n] )
#define GETN(n)( ((VERT *)( (int)(tfn) +(n<<1) )) )

extern long *transformedVertices;
extern long *transformedDepths;
extern VERT *transformedNormals;
extern PSIMODELCTRL	PSImodelctrl;

extern unsigned long *sortedIndex;
extern int sortCount;

#define WATER_TRANS_CODE (((ULONG)2)<<24)
#define WATER_TRANS_CLUT (0x80000000)

int polyCount = 0;

short actorShiftDepth;

// SL: This is the pre-made up all black palette.
const USHORT EXPLORE_black_ref_palette[16] =
//								{	0x4210,0x4210,0x4210,0x4210,	// rgb = 127	// this version does no trans!
								{	0x8000,0x4210,0x4210,0x4210,	// rgb = 127
									0x4210,0x4210,0x4210,0x4210,
									0x4210,0x4210,0x4210,0x4210,
									0x4210,0x4210,0x4210,0x4210
								};

USHORT EXPLORE_black_CLUT;

unsigned short globalClut;

/**************************************************************************
	FUNCTION:	customDrawPrimitives2
	PURPOSE:	add list of polys into ot using sortlists
	PARAMETERS:	ot (at depth)
	RETURNS:	
**************************************************************************/

/*void customDrawPrimitives2 ( int depth )
{
	register PACKET		*packet;

	register long			*tfv = transformedVertices;
	register long			*tfd = transformedDepths;

	VERT 							*tfn = transformedNormals;
	register TMD_P_GT4I	*opcd;
	long				clipflag;
	PSIMODELCTRL		*modctrl = &PSImodelctrl;
	VERT				*vp = modctrl->VertTop;
	int					prims,primsleft;
	USHORT				*sorts;
//	long				*tfd = transformedDepths;
	
	int						gteH;


	depth=depth>>2;
	depth += actorShiftDepth;

///bb this was missing!
	gte_ReadGeomScreen(&gteH);

	prims = (int)modctrl->PrimTop;
	sorts = modctrl->SortOffs;
	primsleft = modctrl->PrimLeft;

	polyCount += primsleft;	

	while(primsleft)
	{	
		opcd = (TMD_P_GT4I*)( prims + (*sorts++<<3) );
		switch (opcd->cd & (0xff-2))
		{
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_FT3*)packet)
#define op ((TMD_P_FT3I*)opcd)
			case GPU_COM_TF3:

				BEGINPRIM(si, POLY_FT3);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
			
				gte_nclip_b();	// takes 8 cycles

				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

	 			gte_stopz(&clipflag);
			
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;									// Back face culling

				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
				gte_stsxy3_ft3(si);
			
				//no lighting
				*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here

				setPolyFT3(si);
				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF
				si->code = op->cd | modctrl->semitrans;

 				ENDPRIM(si, depth, POLY_FT3);
				modctrl->polysdrawn++;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)
			case GPU_COM_TF4:

				BEGINPRIM(si, POLY_FT4);
   			
//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords

				gte_nclip_b();	// takes 8 cycles
		
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stopz(&clipflag);

				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;	// Back face culling
				
				gte_stsxy3_ft4(si);

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);

				//no lighting
				*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here

				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);

				setPolyFT4(si);
				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF
				si->code = op->cd | modctrl->semitrans;
				
				modctrl->polysdrawn++;
 			
 				ENDPRIM(si, depth, POLY_FT4);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_GT3*)packet)
#define op ((TMD_P_GT3I*)opcd)
			case GPU_COM_TG3:
			
				BEGINPRIM(si, POLY_GT3);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords

				gte_nclip_b();	// takes 8 cycles
				
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
					
				gte_stopz(&clipflag);
				
				if (!(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;								// Back face culling

				gte_stsxy3_gt3(si);
			

				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				//no lighting
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);

				setPolyGT3(si);
				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF
				si->code = op->cd | modctrl->semitrans;
			
				modctrl->polysdrawn++;

				ENDPRIM(si, depth, POLY_GT3);
				break;
 		
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_GT4*)packet)
#define op opcd
			case GPU_COM_TG4:
		
				BEGINPRIM(si, POLY_GT4);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
			
				gte_nclip_b();	// takes 8 cycles
		
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
					
				gte_stopz(&clipflag);
				
				if (!(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;
//				if (clipflag >= 0) break;								// Back face culling

				gte_stsxy3_gt4(si);
				
						
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);


				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
		
		
				//no lighting
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);

				modctrl->polysdrawn++;
			
				setPolyGT4(si);
				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF
				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_GT4);
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
		We can't use the "quick" scaling method in psi objects, since we don't know which
		bone the sprite is attached to, hence we can't rtps the vertex and get the scaled width/height,
		so we have to do the scaling based on the distance ourselves.
	*/

//					width = (((op->v2 * gteH) / (tfd[op->v0]<<2))*2)/3;
//					height = (((op->v3 * gteH) / (tfd[op->v0]<<2))*2)/6;

					//width = ((op->v2 * gteH) / (tfd[op->v0]));
					//height = ((op->v3 * gteH) / (tfd[op->v0]));

		/*			width = (op->v2 * SCALEX) / (tfd[op->v0]*20);
					height = (op->v3 * SCALEY) / (tfd[op->v0]*20);

					// JH : Temp Fix
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
					si->code |= modctrl->semitrans;
			
	 				ENDPRIM(si, depth, POLY_FT4);
				}
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_G4*)packet)
#define op ((TMD_P_FG4I*)opcd)
			case GPU_COM_G4:
			case GPU_COM_F4:
				
				BEGINPRIM(si, POLY_G4);
   			
//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
 
 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

				// Back face culling
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling
				
				gte_stsxy3_g4(si);


//bbxx - it's drawing all F4's as G4's.
//bbopt - perhaps a separate case for the F4's
//will be faster, cos of less rgtb copies
				//no lighting
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);

				setPolyG4(si);
//				if ( globalClut )
//				{
//					si->clut	= globalClut;
//				}

				// ENDIF
				si->code = op->cd | modctrl->semitrans;

				modctrl->polysdrawn++;

				ENDPRIM(si, depth, POLY_G4);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_G3*)packet)
#define op ((TMD_P_FG3I*)opcd)
			
			case GPU_COM_G3:
			case GPU_COM_F3:

				BEGINPRIM(si, POLY_G3);
   			
//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

 
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
//				si->r0 = op->r0 >> 1;
 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

	//			if (clipflag >= 0) break; 								// Back face culling
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling
				
				gte_stsxy3_g3(si);


//bbxx - F3's only have 1 rgb
//bbopt - perhaps a separate case for the F4's
//will be faster, cos of less rgtb copies
				//no lighting
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);

				modctrl->polysdrawn++;

				setPolyG3(si);
//				if ( globalClut )
//				{
//					si->clut	= globalClut;
//				}

				// ENDIF
				si->code = op->cd | modctrl->semitrans;

				ENDPRIM(si, depth, POLY_G3);
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

extern long globalFadeVal;

/*void LSCAPE_DrawSortedPrimitives ( int depth )
{
	register PACKET			*packet;//, *packetNext;

	register long			*tfv = transformedVertices;
	register long			*tfd = transformedDepths;

//	VERT 							*tfn = transformedNormals;
	register TMD_P_GT4I		*opcd;
	PSIMODELCTRL			*modctrl = &PSImodelctrl;
	int						primsleft,lightmode;
	ULONG					*sorts = sortedIndex;
//	ULONG					*sorts = modctrl->SortOffs;
	ULONG					sortBucket = 0;
//	VERT					*vp = modctrl->VertTop;
	LONG					fogdepth;
//	unsigned short			oldtpage;
	long					fogFade;
//	DR_MODE					*dr_mode;
//	u_long t1;

	int						gteH;


	gte_ReadGeomScreen(&gteH);

	depth = depth >> 2;

	depth += actorShiftDepth;

	// SL: get the fogdepth, and modge it accordingly.
	fogdepth = 10000;//(depth << (2+1/*LSCAPE_Data.depthshift*//*));*/

//	if(fogdepth > LSCAPE_Data.fogendz)
//		return;

	// SL: work out the fog fade. in advance. It's the same for the whole model, so this is fine here.
/*	fogFade=globalFadeVal;//255-((255*(fogdepth-3000/*-LSCAPE_Data.fogstartz*//*))/(3500-3000/*LSCAPE_Data.fogendz-LSCAPE_Data.fogstartz*//*));
	// SL: shift everythang into the same rez as the landscape, etc...
//	depth = (depth >> LSCAPE_Data.depthshift) & MAXDEPTH;

	
	primsleft = sortCount;
	if (!primsleft)
		return;

//	polyCount += primsleft;	

	lightmode = modctrl->lighting;
	opcd = 0;
//	TimerStart(&poly);
	//utilPrintf("Polys To Draw : %d\n", primsleft);


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
#define siNext ((POLY_FT3*)packetNext)
#define op ((TMD_P_FT3I*)opcd)

			case GPU_COM_TF3:
				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_FT3);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *) (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *) (&si->u1) = *(u_long *) (&op->tu1);
				*(u_long *) (&si->u2) = *(u_long *) (&op->tu2);

				gte_stsxy3_ft3(si);
			
				// JH : Temp Fix.................

				*(u_long *) (&si->r0) = *(u_long *) (&op->r0);
// 				si->r0 = ( op->r0 << 7 ) >> 8;
// 				si->g0 = ( op->g0 << 7 ) >> 8;
// 				si->b0 = ( op->b0 << 7 ) >> 8;


				// SL: put in the additive poly...
				setPolyFT3(si);

				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF

				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_FT3);

				// SL: move the prim pointer on one...
				op = op->next;
				}
				break;
#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_FT4*)packet)
#define siNext ((POLY_FT4*)packetNext)
#define op ((TMD_P_FT4I*)opcd)
				
			case GPU_COM_TF4:

				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_FT4);
   			
//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_ft4(si);

				*(u_long *) (&si->x3) = *(u_long *) (&tfv[op->v3]);

				// JH : Temp Fix
				*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here

// 				si->r0 = ( op->r0 * 128 ) >> 8;
// 				si->g0 = ( op->g0 * 128 ) >> 8;
// 				si->b0 = ( op->b0 * 128 ) >> 8;

				*(u_long *) (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *) (&si->u3) = *(u_long *) (&op->tu3);

				// SL: put in the additive poly...
				setPolyFT4(si);

				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF

				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_FT4);

				op = op->next;
				}
				break;
#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_GT3*)packet)
#define siNext ((POLY_GT3*)packetNext)
#define op ((TMD_P_GT3I*)opcd)

			case GPU_COM_TG3:

				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_GT3);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_gt3(si);

				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				// JH : Temp Fix.........

				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);

// 				si->r0 = ( op->r0 * 128 ) >> 8;
// 				si->g0 = ( op->g0 * 128 ) >> 8;
// 				si->b0 = ( op->b0 * 128 ) >> 8;
// 
// 				si->r1 = ( op->r1 * 128 ) >> 8;
// 				si->g1 = ( op->g1 * 128 ) >> 8;
// 				si->b1 = ( op->b1 * 128 ) >> 8;
// 
// 				si->r2 = ( op->r2 * 128 ) >> 8;
// 				si->g2 = ( op->g2 * 128 ) >> 8;
// 				si->b2 = ( op->b2 * 128 ) >> 8;

				// SL: put in the additive poly...
				setPolyGT3(si);

				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF


				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_GT3);

				op = op->next;
				}
				break;
#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
/*#define si ((POLY_GT4*)packet)
#define siNext ((POLY_GT4*)packetNext)
#define op opcd

			case GPU_COM_TG4:
				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_GT4);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
					

				gte_stsxy3_gt4(si);
				
						
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
		
				// JH Temp Fix.......................
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
		
// 				si->r0 = ( op->r0 << 7 ) >> 8;
// 				si->g0 = ( op->g0 << 7 ) >> 8;
// 				si->b0 = ( op->b0 << 7 ) >> 8;
// 
// 				si->r1 = ( op->r1 << 7 ) >> 8;
// 				si->g1 = ( op->g1 << 7 ) >> 8;
// 				si->b1 = ( op->b1 << 7 ) >> 8;
// 
// 				si->r2 = ( op->r2 << 7 ) >> 8;
// 				si->g2 = ( op->g2 << 7 ) >> 8;
// 				si->b2 = ( op->b2 << 7 ) >> 8;
// 
// 				si->r3 = ( op->r3 << 7 ) >> 8;
// 				si->g3 = ( op->g3 << 7 ) >> 8;
// 				si->b3 = ( op->b3 << 7 ) >> 8;

				// SL: put in the additive poly...
				setPolyGT4(si);

				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF

				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_GT4);

				(int)op = op->next;
				}
				break;

#undef si
#undef siNext
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
				//width = ((op->v2 * gteH) / tfd[op->v0]);
				//height = ((op->v3 * gteH) / tfd[op->v0]);

		/*			width = (((op->v2 * gteH) / (tfd[op->v0]<<2))*2)/3;
					height = (((op->v3 * gteH) / (tfd[op->v0]<<2))*2)/6;

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
			default:
				break;
		}
	}
//	TimerStop(&poly);
//	utilPrintf("Timer Taken To Draw Polys : %d\n", poly.total);
}
*/


void DrawSortedPrimitivesFaded ( int depth )
{
	register PACKET			*packet, *packetNext;
	register long			*tfv = transformedVertices;
	register long			*tfd = transformedDepths;
//	VERT 							*tfn = transformedNormals;
	register TMD_P_GT4I		*opcd;
	PSIMODELCTRL			*modctrl = &PSImodelctrl;
	int						primsleft;//,lightmode;
	ULONG					*sorts = sortedIndex;
//	ULONG					*sorts = modctrl->SortOffs;
	ULONG					sortBucket = 0;
//	VERT					*vp = modctrl->VertTop;
//	LONG					fogdepth;
	unsigned short			oldtpage;
//	long					fogFade;
//	DR_MODE					*dr_mode;
//	u_long t1;

	int						gteH;


	gte_ReadGeomScreen(&gteH);

	depth=depth>>2;

	depth += actorShiftDepth;

	// SL: work out the fog fade. in advance. It's the same for the whole model, so this is fine here.
//	fogFade=globalFadeVal;

	// SL: shift everythang into the same rez as the landscape, etc...
	//	depth = (depth >> LSCAPE_Data.depthshift) & MAXDEPTH;

	
	primsleft = sortCount;
	if (!primsleft)
		return;

	polyCount += primsleft;	

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
#define si ((POLY_FT3*)packet)
#define siNext ((POLY_FT3*)packetNext)
#define op ((TMD_P_FT3I*)opcd)

			case GPU_COM_TF3:
				BEGINPRIM(si, POLY_FT3);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *) (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *) (&si->u1) = *(u_long *) (&op->tu1);
				*(u_long *) (&si->u2) = *(u_long *) (&op->tu2);

				gte_stsxy3_ft3(si);
			
				*(u_long *) (&si->r0) = *(u_long *) (&op->r0);

 				// SL: modge the RGBs accordingly
 				si->r0 = ((globalFadeVal*(short)si->r0)/128);
 				si->g0 = ((globalFadeVal*(short)si->g0)/128);
 				si->b0 = ((globalFadeVal*(short)si->b0)/128);

 				// SL: put in the additive poly...
 				setPolyFT3(si);
 				// SL: store the pre-modification tpage...
 				oldtpage = si->tpage;
 				// make it additive...
 				si->code |= 2;
 				si->tpage |= 32;
 				// SL: put it in t' table
 				ENDPRIM(si, depth, POLY_FT3);

 				// SL: hokay, setup the next poly...

 				/*BEGINPRIM(siNext, POLY_FT3);

 				// SL: copy in all the values using long copies...
 				*(u_long *)  (&siNext->x0) = *(u_long *) (&si->x0);
 				*(u_long *)  (&siNext->x1) = *(u_long *) (&si->x1);
 				*(u_long *)  (&siNext->x2) = *(u_long *) (&si->x2);
 				*(u_long *)  (&siNext->u0) = *(u_long *) (&si->u0);		// Texture coords
 				*(u_long *)  (&siNext->u1) = *(u_long *) (&si->u1);
 				*(u_long *)  (&siNext->u2) = *(u_long *) (&si->u2);
 				*(u_long *)  (&siNext->r0) = *(u_long *) (&si->r0);			// SL: RGBs

 				// SL: set it as the right type
 				setPolyFT3(siNext);
 				// SL: make it subtractive
 				siNext->clut = EXPLORE_black_CLUT;
 				siNext->code |= 2;
 				siNext->tpage = oldtpage | 64;

 				// SL: put it in t' table
 				ENDPRIM(siNext, depth, POLY_FT3);*/

				op = op->next;
				break;
#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT4*)packet)
#define siNext ((POLY_FT4*)packetNext)
#define op ((TMD_P_FT4I*)opcd)
				
			case GPU_COM_TF4:

				BEGINPRIM(si, POLY_FT4);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_ft4(si);

				*(u_long *) (&si->x3) = *(u_long *) (&tfv[op->v3]);

				*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here

				*(u_long *) (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *) (&si->u3) = *(u_long *) (&op->tu3);

 				// SL: modge the RGBs accordingly
 				si->r0 = ((globalFadeVal*(short)si->r0)/128);
 				si->g0 = ((globalFadeVal*(short)si->g0)/128);
 				si->b0 = ((globalFadeVal*(short)si->b0)/128);

 				// SL: put in the additive poly...
 				setPolyFT4(si);
 				// SL: store the pre-modification tpage...
 				oldtpage = si->tpage;
 				// make it additive...
 				si->code |= 2;
 				si->tpage |= 32;
 				// SL: put it in t' table
 				ENDPRIM(si, depth, POLY_FT4);

 				// SL: hokay, setup the next poly...
 				/*BEGINPRIM(siNext, POLY_FT4);

 				// SL: copy in all the values using long copies...
 				*(u_long *)  (&siNext->x0) = *(u_long *) (&si->x0);
 				*(u_long *)  (&siNext->x1) = *(u_long *) (&si->x1);
 				*(u_long *)  (&siNext->x2) = *(u_long *) (&si->x2);
 				*(u_long *)  (&siNext->x3) = *(u_long *) (&si->x3);
 				*(u_long *)  (&siNext->u0) = *(u_long *) (&si->u0);		// Texture coords
 				*(u_long *)  (&siNext->u1) = *(u_long *) (&si->u1);
 				*(u_long *)  (&siNext->u2) = *(u_long *) (&si->u2);
 				*(u_long *)  (&siNext->u3) = *(u_long *) (&si->u3);
 				*(u_long *)  (&siNext->r0) = *(u_long *) (&si->r0);			// SL: RGBs

 				// SL: set it as the right type
 				setPolyFT4(siNext);
 				// SL: make it subtractive
 				siNext->clut = EXPLORE_black_CLUT;
 				siNext->code |= 2;
 				siNext->tpage = oldtpage | 64;

 				// SL: put it in t' table
 				ENDPRIM(siNext, depth, POLY_FT4);*/

    		op = op->next;
				break;
#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_GT3*)packet)
#define siNext ((POLY_GT3*)packetNext)
#define op ((TMD_P_GT3I*)opcd)

			case GPU_COM_TG3:

				BEGINPRIM(si, POLY_GT3);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_gt3(si);

				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);

 				// SL: modge the RGBs accordingly
 				si->r0 = ((globalFadeVal*(short)si->r0)/128);
 				si->g0 = ((globalFadeVal*(short)si->g0)/128);
 				si->b0 = ((globalFadeVal*(short)si->b0)/128);
 				si->r1 = ((globalFadeVal*(short)si->r1)/128);
 				si->g1 = ((globalFadeVal*(short)si->g1)/128);
 				si->b1 = ((globalFadeVal*(short)si->b1)/128);
 				si->r2 = ((globalFadeVal*(short)si->r2)/128);
 				si->g2 = ((globalFadeVal*(short)si->g2)/128);
 				si->b2 = ((globalFadeVal*(short)si->b2)/128);

 				// SL: put in the additive poly...
 				setPolyGT3(si);
 				// SL: store the pre-modification tpage...
 				oldtpage = si->tpage;
 				// make it additive...
 				si->code |= 2;
 				si->tpage |= 32;
 				// SL: put it in t' table
 				ENDPRIM(si, depth, POLY_GT3);

 				// SL: hokay, setup the next poly...
 				/*BEGINPRIM(siNext, POLY_GT3);

 				// SL: copy in all the values using long copies...
 				*(u_long *)  (&siNext->x0) = *(u_long *) (&si->x0);
 				*(u_long *)  (&siNext->x1) = *(u_long *) (&si->x1);
 				*(u_long *)  (&siNext->x2) = *(u_long *) (&si->x2);
 				*(u_long *)  (&siNext->u0) = *(u_long *) (&si->u0);		// Texture coords
 				*(u_long *)  (&siNext->u1) = *(u_long *) (&si->u1);
 				*(u_long *)  (&siNext->u2) = *(u_long *) (&si->u2);
 				*(u_long *)  (&siNext->r0) = *(u_long *) (&si->r0);			// SL: RGBs
 				*(u_long *)  (&siNext->r1) = *(u_long *) (&si->r1);			// SL: RGBs
 				*(u_long *)  (&siNext->r2) = *(u_long *) (&si->r2);			// SL: RGBs

 				// SL: set it as the right type
 				setPolyGT3(siNext);
 				// SL: make it subtractive
 				siNext->clut = EXPLORE_black_CLUT;
 				siNext->code |= 2;
 				siNext->tpage = oldtpage | 64;

 				// SL: put it in t' table
 				ENDPRIM(siNext, depth, POLY_GT3);*/
				op = op->next;
				break;
#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_GT4*)packet)
#define siNext ((POLY_GT4*)packetNext)
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
		
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
		
 				// SL: modge the RGBs accordingly
 				si->r0 = ((globalFadeVal*(short)si->r0)/128);
 				si->g0 = ((globalFadeVal*(short)si->g0)/128);
 				si->b0 = ((globalFadeVal*(short)si->b0)/128);
 				si->r1 = ((globalFadeVal*(short)si->r1)/128);
 				si->g1 = ((globalFadeVal*(short)si->g1)/128);
 				si->b1 = ((globalFadeVal*(short)si->b1)/128);
 				si->r2 = ((globalFadeVal*(short)si->r2)/128);
 				si->g2 = ((globalFadeVal*(short)si->g2)/128);
 				si->b2 = ((globalFadeVal*(short)si->b2)/128);
 				si->r3 = ((globalFadeVal*(short)si->r3)/128);
 				si->g3 = ((globalFadeVal*(short)si->g3)/128);
 				si->b3 = ((globalFadeVal*(short)si->b3)/128);

 				// SL: put in the additive poly...
 				setPolyGT4(si);
 				// SL: store the pre-modification tpage...
 				oldtpage = si->tpage;
 				// make it additive...
 				si->code |= 2;
 				si->tpage |= 32;
 				// SL: put it in t' table
 				ENDPRIM(si, depth, POLY_GT4);

 				// SL: hokay, setup the next poly...
 				/*BEGINPRIM(siNext, POLY_GT4);

 				// SL: copy in all the values using long copies...
 				*(u_long *)  (&siNext->x0) = *(u_long *) (&si->x0);
 				*(u_long *)  (&siNext->x1) = *(u_long *) (&si->x1);
 				*(u_long *)  (&siNext->x2) = *(u_long *) (&si->x2);
 				*(u_long *)  (&siNext->x3) = *(u_long *) (&si->x3);
 				*(u_long *)  (&siNext->u0) = *(u_long *) (&si->u0);		// Texture coords
 				*(u_long *)  (&siNext->u1) = *(u_long *) (&si->u1);
 				*(u_long *)  (&siNext->u2) = *(u_long *) (&si->u2);
 				*(u_long *)  (&siNext->u3) = *(u_long *) (&si->u3);
 				*(u_long *)  (&siNext->r0) = *(u_long *) (&si->r0);			// SL: RGBs
 				*(u_long *)  (&siNext->r1) = *(u_long *) (&si->r1);			// SL: RGBs
 				*(u_long *)  (&siNext->r2) = *(u_long *) (&si->r2);			// SL: RGBs
 				*(u_long *)  (&siNext->r3) = *(u_long *) (&si->r3);			// SL: RGBs

 				// SL: set it as the right type
 				setPolyGT4(siNext);
 				// SL: make it subtractive
 				siNext->clut = EXPLORE_black_CLUT;
 				siNext->code |= 2;
 				siNext->tpage = oldtpage | 64;

 				// SL: put it in t' table
 				ENDPRIM(siNext, depth, POLY_GT4);*/

				(int)op = op->next;
				break;

#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/

#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)

			case GPU_COM_TF4SPR :

				if((tfd[op->v0] > modctrl->nearclip) && (tfd[op->v0] < modctrl->farclip))
				{
					int			width, height;

					BEGINPRIM(si, POLY_FT4);

	/*
		We can't use the "quick" scaling method in psi objects, since we don't know which
		bone the sprite is attached to, hence we can't rtps the vertex and get the scaled width/height,
		so we have to do the scaling based on the distance ourselves.
	*/

					//width = ((op->v2 * gteH) / tfd[op->v0]) / 2;
					//height = ((op->v3 * gteH) / tfd[op->v0]) / 4;

					width = (((op->v2 * gteH) / (tfd[op->v0]<<2))*2)/3;
					height = (((op->v3 * gteH) / (tfd[op->v0]<<2))*2)/6;

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

					si->code |= modctrl->semitrans;
			
	 				ENDPRIM(si, depth & 1023, POLY_FT4);
					op = op->next;
				}
				break;
#undef si
#undef op
			default:
				break;
		}
	}
//	TimerStop(&poly);
//	utilPrintf("Timer Taken To Draw Polys : %d\n", poly.total);
}


void dcache_LSCAPE_DrawSortedPrimitives ( int depth, long *tfv )
{
	register PACKET			*packet;//, *packetNext;

	register long			*tfd = transformedDepths;

//	VERT 							*tfn = transformedNormals;
	register TMD_P_GT4I		*opcd;
	PSIMODELCTRL			*modctrl = &PSImodelctrl;
	int						primsleft,lightmode;
	ULONG					*sorts = sortedIndex;
//	ULONG					*sorts = modctrl->SortOffs;
	ULONG					sortBucket = 0;
//	VERT					*vp = modctrl->VertTop;
	LONG					fogdepth;
//	unsigned short			oldtpage;
	long					fogFade;
//	DR_MODE					*dr_mode;
//	u_long t1;

	int						gteH;
	long				clipflag;


	gte_ReadGeomScreen(&gteH);

	depth = depth >> 2;

	depth += actorShiftDepth;

	// SL: get the fogdepth, and modge it accordingly.
	fogdepth = 10000;//(depth << (2+1/*LSCAPE_Data.depthshift*/));

//	if(fogdepth > LSCAPE_Data.fogendz)
//		return;

	// SL: work out the fog fade. in advance. It's the same for the whole model, so this is fine here.
	fogFade=globalFadeVal;//255-((255*(fogdepth-3000/*-LSCAPE_Data.fogstartz*/))/(3500-3000/*LSCAPE_Data.fogendz-LSCAPE_Data.fogstartz*/));
	// SL: shift everythang into the same rez as the landscape, etc...
//	depth = (depth >> LSCAPE_Data.depthshift) & MAXDEPTH;

	
	primsleft = sortCount;
	if (!primsleft)
		return;

//	polyCount += primsleft;	

	lightmode = modctrl->lighting;
	opcd = 0;
//	TimerStart(&poly);
	//utilPrintf("Polys To Draw : %d\n", primsleft);


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
#define si ((POLY_FT3*)packet)
#define siNext ((POLY_FT3*)packetNext)
#define op ((TMD_P_FT3I*)opcd)

			case GPU_COM_TF3:
				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_FT3);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *) (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *) (&si->u1) = *(u_long *) (&op->tu1);
				*(u_long *) (&si->u2) = *(u_long *) (&op->tu2);

				gte_stsxy3_ft3(si);
			
				// JH : Temp Fix.................

				*(u_long *) (&si->r0) = *(u_long *) (&op->r0);
// 				si->r0 = ( op->r0 << 7 ) >> 8;
// 				si->g0 = ( op->g0 << 7 ) >> 8;
// 				si->b0 = ( op->b0 << 7 ) >> 8;


				// SL: put in the additive poly...
				setPolyFT3(si);

				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF

				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_FT3);

				// SL: move the prim pointer on one...
				op = op->next;
				}
				break;
#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT4*)packet)
#define siNext ((POLY_FT4*)packetNext)
#define op ((TMD_P_FT4I*)opcd)
				
			case GPU_COM_TF4:

				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_FT4);
   			
//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_ft4(si);

				*(u_long *) (&si->x3) = *(u_long *) (&tfv[op->v3]);

				// JH : Temp Fix
				*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here

// 				si->r0 = ( op->r0 * 128 ) >> 8;
// 				si->g0 = ( op->g0 * 128 ) >> 8;
// 				si->b0 = ( op->b0 * 128 ) >> 8;

				*(u_long *) (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *) (&si->u3) = *(u_long *) (&op->tu3);

				// SL: put in the additive poly...
				setPolyFT4(si);

				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF

				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_FT4);

				op = op->next;
				}
				break;
#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_GT3*)packet)
#define siNext ((POLY_GT3*)packetNext)
#define op ((TMD_P_GT3I*)opcd)

			case GPU_COM_TG3:

				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_GT3);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_gt3(si);

				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				// JH : Temp Fix.........

				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);

// 				si->r0 = ( op->r0 * 128 ) >> 8;
// 				si->g0 = ( op->g0 * 128 ) >> 8;
// 				si->b0 = ( op->b0 * 128 ) >> 8;
// 
// 				si->r1 = ( op->r1 * 128 ) >> 8;
// 				si->g1 = ( op->g1 * 128 ) >> 8;
// 				si->b1 = ( op->b1 * 128 ) >> 8;
// 
// 				si->r2 = ( op->r2 * 128 ) >> 8;
// 				si->g2 = ( op->g2 * 128 ) >> 8;
// 				si->b2 = ( op->b2 * 128 ) >> 8;

				// SL: put in the additive poly...
				setPolyGT3(si);

				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF


				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_GT3);

				op = op->next;
				}
				break;
#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_G3*)packet)
#define op ((TMD_P_FG3I*)opcd)

			case GPU_COM_G3:

				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_G3);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				/**(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);*/

				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);


				//*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				// JH : Temp Fix.........

 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling

				gte_stsxy3_g3(si);

				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);


// 				si->r0 = ( op->r0 * 128 ) >> 8;
// 				si->g0 = ( op->g0 * 128 ) >> 8;
// 				si->b0 = ( op->b0 * 128 ) >> 8;
// 
// 				si->r1 = ( op->r1 * 128 ) >> 8;
// 				si->g1 = ( op->g1 * 128 ) >> 8;
// 				si->b1 = ( op->b1 * 128 ) >> 8;
// 
// 				si->r2 = ( op->r2 * 128 ) >> 8;
// 				si->g2 = ( op->g2 * 128 ) >> 8;
// 				si->b2 = ( op->b2 * 128 ) >> 8;

				// SL: put in the additive poly...
				setPolyG3(si);


				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_G3);

				op = op->next;
				}
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_G4*)packet)
#define op ((TMD_P_FG4I*)opcd)

			case GPU_COM_G4:

				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_G4);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				/**(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);*/

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);

				//*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				// JH : Temp Fix.........

 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling

				gte_stsxy3_g4(si);

				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);



// 				si->r0 = ( op->r0 * 128 ) >> 8;
// 				si->g0 = ( op->g0 * 128 ) >> 8;
// 				si->b0 = ( op->b0 * 128 ) >> 8;
// 
// 				si->r1 = ( op->r1 * 128 ) >> 8;
// 				si->g1 = ( op->g1 * 128 ) >> 8;
// 				si->b1 = ( op->b1 * 128 ) >> 8;
// 
// 				si->r2 = ( op->r2 * 128 ) >> 8;
// 				si->g2 = ( op->g2 * 128 ) >> 8;
// 				si->b2 = ( op->b2 * 128 ) >> 8;

				// SL: put in the additive poly...
				setPolyG4(si);


				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_G4);

				op = op->next;
				}
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/



#define si ((POLY_F3*)packet)
#define op ((TMD_P_FG3I*)opcd)

			case GPU_COM_F3:

				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_F3);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				/**(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);*/

				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);


				//*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				// JH : Temp Fix.........

 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling

				gte_stsxy3_f3(si);

//				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
//				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);


// 				si->r0 = ( op->r0 * 128 ) >> 8;
// 				si->g0 = ( op->g0 * 128 ) >> 8;
// 				si->b0 = ( op->b0 * 128 ) >> 8;
// 
// 				si->r1 = ( op->r1 * 128 ) >> 8;
// 				si->g1 = ( op->g1 * 128 ) >> 8;
// 				si->b1 = ( op->b1 * 128 ) >> 8;
// 
// 				si->r2 = ( op->r2 * 128 ) >> 8;
// 				si->g2 = ( op->g2 * 128 ) >> 8;
// 				si->b2 = ( op->b2 * 128 ) >> 8;

				// SL: put in the additive poly...
				setPolyF3(si);


				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_F3);

				op = op->next;
				}
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_G4*)packet)
#define op ((TMD_P_FG4I*)opcd)

			case GPU_COM_F4:

				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_G4);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				/**(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);*/

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);

				//*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				// JH : Temp Fix.........

 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling

				gte_stsxy3_f4(si);

				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);



// 				si->r0 = ( op->r0 * 128 ) >> 8;
// 				si->g0 = ( op->g0 * 128 ) >> 8;
// 				si->b0 = ( op->b0 * 128 ) >> 8;
// 
// 				si->r1 = ( op->r1 * 128 ) >> 8;
// 				si->g1 = ( op->g1 * 128 ) >> 8;
// 				si->b1 = ( op->b1 * 128 ) >> 8;
// 
// 				si->r2 = ( op->r2 * 128 ) >> 8;
// 				si->g2 = ( op->g2 * 128 ) >> 8;
// 				si->b2 = ( op->b2 * 128 ) >> 8;

				// SL: put in the additive poly...
				setPolyF4(si);


				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_G4);

				op = op->next;
				}
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/









#define si ((POLY_GT4*)packet)
#define siNext ((POLY_GT4*)packetNext)
#define op opcd

			case GPU_COM_TG4:
				if((tfd[op->v0] > modctrl->nearclip-80) && (tfd[op->v0] < modctrl->farclip))
				{
					//polyCount ++;	
				BEGINPRIM(si, POLY_GT4);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
					

				gte_stsxy3_gt4(si);
				
						
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
		
				// JH Temp Fix.......................
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
		
				setPolyGT4(si);

				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF

				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_GT4);

				(int)op = op->next;
				}
				break;

#undef si
#undef siNext
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/

#define si ((POLY_FT4*)packet)
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
				//width = ((op->v2 * gteH) / tfd[op->v0]);
				//height = ((op->v3 * gteH) / tfd[op->v0]);

					width = (((op->v2 * gteH) / (tfd[op->v0]<<2))*2)/3;
					height = (((op->v3 * gteH) / (tfd[op->v0]<<2))*2)/6;

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
			default:
				break;
		}
	}
//	TimerStop(&poly);
//	utilPrintf("Timer Taken To Draw Polys : %d\n", poly.total);
}

void dcacheCustomDrawPrimitives2 ( int depth, long *tfv, long *tfd )
{
	register PACKET		*packet;

	VERT 							*tfn = transformedNormals;
	register TMD_P_GT4I	*opcd;
	long				clipflag;
	PSIMODELCTRL		*modctrl = &PSImodelctrl;
	VERT				*vp = modctrl->VertTop;
	int					prims,primsleft;
	USHORT				*sorts;
//	long				*tfd = transformedDepths;
	
	int						gteH;


	depth=depth>>2;
	depth += actorShiftDepth;

///bb this was missing!
	gte_ReadGeomScreen(&gteH);

	prims = (int)modctrl->PrimTop;
	sorts = modctrl->SortOffs;
	primsleft = modctrl->PrimLeft;

	polyCount += primsleft;	

	while(primsleft)
	{	
		opcd = (TMD_P_GT4I*)( prims + (*sorts++<<3) );
		switch (opcd->cd & (0xff-2))
		{
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT3*)packet)
#define op ((TMD_P_FT3I*)opcd)
			case GPU_COM_TF3:

				BEGINPRIM(si, POLY_FT3);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
			
				gte_nclip_b();	// takes 8 cycles

				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

	 			gte_stopz(&clipflag);
			
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;									// Back face culling

				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
				gte_stsxy3_ft3(si);
			
				//no lighting
				*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here

				setPolyFT3(si);
				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF
				si->code = op->cd | modctrl->semitrans;

 				ENDPRIM(si, depth, POLY_FT3);
				modctrl->polysdrawn++;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)
			case GPU_COM_TF4:

				BEGINPRIM(si, POLY_FT4);
   			
//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords

				gte_nclip_b();	// takes 8 cycles
		
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stopz(&clipflag);

				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;	// Back face culling
				
				gte_stsxy3_ft4(si);

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);

				//no lighting
				*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here

				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);

				setPolyFT4(si);
				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF
				si->code = op->cd | modctrl->semitrans;
				
				modctrl->polysdrawn++;
 			
 				ENDPRIM(si, depth, POLY_FT4);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_GT3*)packet)
#define op ((TMD_P_GT3I*)opcd)
			case GPU_COM_TG3:
			
				BEGINPRIM(si, POLY_GT3);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords

				gte_nclip_b();	// takes 8 cycles
				
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
					
				gte_stopz(&clipflag);
				
				if (!(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;								// Back face culling

				gte_stsxy3_gt3(si);
			

				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				//no lighting
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);

				setPolyGT3(si);
				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF
				si->code = op->cd | modctrl->semitrans;
			
				modctrl->polysdrawn++;

				ENDPRIM(si, depth, POLY_GT3);
				break;
 		
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_GT4*)packet)
#define op opcd
			case GPU_COM_TG4:
		
				BEGINPRIM(si, POLY_GT4);

//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
			
				gte_nclip_b();	// takes 8 cycles
		
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
					
				gte_stopz(&clipflag);
				
				if (!(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;
//				if (clipflag >= 0) break;								// Back face culling

				gte_stsxy3_gt4(si);
				
						
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);


				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
		
		
				//no lighting
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);

				modctrl->polysdrawn++;
			
				setPolyGT4(si);
				if ( globalClut )
				{
					si->clut	= globalClut;
				}
				// ENDIF
				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_GT4);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)
			case GPU_COM_TF4SPR :

				if((tfd[op->v0] > modctrl->nearclip) && (tfd[op->v0] < modctrl->farclip))
				{
					int			width, height;

					BEGINPRIM(si, POLY_FT4);

	/*
		We can't use the "quick" scaling method in psi objects, since we don't know which
		bone the sprite is attached to, hence we can't rtps the vertex and get the scaled width/height,
		so we have to do the scaling based on the distance ourselves.
	*/

//					width = ((op->v2 * gteH) / (tfd[op->v0]));
//					height = ((op->v3 * gteH) / (tfd[op->v0]));

//					width = (((op->v2 * gteH) / (tfd[op->v0]<<2))*2)/3;
//					height = (((op->v3 * gteH) / (tfd[op->v0]<<2))*2)/6;

					width = (op->v2 * SCALEX) / (tfd[op->v0]*20);
					height = (op->v3 * SCALEY) / (tfd[op->v0]*20);

					// JH : Temp Fix
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
					si->code |= modctrl->semitrans;
			
	 				ENDPRIM(si, depth, POLY_FT4);
				}
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_G4*)packet)
#define op ((TMD_P_FG4I*)opcd)
			case GPU_COM_G4:
				
				BEGINPRIM(si, POLY_G4);
   			
//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
 
 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

				// Back face culling
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling
				
				gte_stsxy3_g4(si);


//bbxx - it's drawing all F4's as G4's.
//bbopt - perhaps a separate case for the F4's
//will be faster, cos of less rgtb copies
				//no lighting
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);

				setPolyG4(si);
//				if ( globalClut )
//				{
//					si->clut	= globalClut;
//				}

				// ENDIF
				si->code = op->cd | modctrl->semitrans;

				modctrl->polysdrawn++;

				ENDPRIM(si, depth, POLY_G4);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_G3*)packet)
#define op ((TMD_P_FG3I*)opcd)
			
			case GPU_COM_G3:

				BEGINPRIM(si, POLY_G3);
   			
//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

 
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
//				si->r0 = op->r0 >> 1;
 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

	//			if (clipflag >= 0) break; 								// Back face culling
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling
				
				gte_stsxy3_g3(si);


//bbxx - F3's only have 1 rgb
//bbopt - perhaps a separate case for the F4's
//will be faster, cos of less rgtb copies
				//no lighting
				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);

				modctrl->polysdrawn++;

				setPolyG3(si);
//				if ( globalClut )
//				{
//					si->clut	= globalClut;
//				}

				// ENDIF
				si->code = op->cd | modctrl->semitrans;

				ENDPRIM(si, depth, POLY_G3);
				break;
#undef si
#undef op
#define si ((POLY_F4*)packet)
#define op ((TMD_P_FG4I*)opcd)
			case GPU_COM_F4:
				
				BEGINPRIM(si, POLY_F4);
   			
//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
 
 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

				// Back face culling
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling
				
				gte_stsxy3_g4(si);


//bbxx - it's drawing all F4's as G4's.
//bbopt - perhaps a separate case for the F4's
//will be faster, cos of less rgtb copies
				//no lighting
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
//				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
//				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
//				*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);

				setPolyF4(si);
//				if ( globalClut )
//				{
//					si->clut	= globalClut;
//				}

				// ENDIF
				si->code = op->cd | modctrl->semitrans;

				modctrl->polysdrawn++;

				ENDPRIM(si, depth, POLY_F4);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_F3*)packet)
#define op ((TMD_P_FG3I*)opcd)
			
			case GPU_COM_F3:

				BEGINPRIM(si, POLY_F3);
   			
//				gte_ldsxy3(GETV(op->v0), GETV(op->v1), GETV(op->v2));		// Load 1st three vertices
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

 
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
//				si->r0 = op->r0 >> 1;
 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

	//			if (clipflag >= 0) break; 								// Back face culling
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling
				
				gte_stsxy3_g3(si);


//bbxx - F3's only have 1 rgb
//bbopt - perhaps a separate case for the F4's
//will be faster, cos of less rgtb copies
				//no lighting
//				*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
//				*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);

				modctrl->polysdrawn++;

				setPolyF3(si);
//				if ( globalClut )
//				{
//					si->clut	= globalClut;
//				}

				// ENDIF
				si->code = op->cd | modctrl->semitrans;

				ENDPRIM(si, depth, POLY_F3);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
			default:
				break;
		}
		primsleft--;
	}
}


