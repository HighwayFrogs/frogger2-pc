// include you other stuff first, e.g. global.h or whatever

#include "sonylibs.h"
#include "shell.h"
#include "types.h"
#include "islpsi.h"
//#include <islpsi.h>
#include <psitypes.h>

TMD_P_FT4I *testpol;
POLY_FT4 *testsi;


// external variables needed for drawing
extern long *transformedVertices;
extern VERT *transformedNormals;
//extern PSIMODELCTRL	PSImodelctrl;
extern unsigned long *sortedIndex;
extern int sortCount;
extern long minDepth;


extern int polyCount;
// use psiRegisterDrawFunction to register your own draw function with the library


/**************************************************************************
	FUNCTION:	customDrawSortedPrimitives
	PURPOSE:	draw dynamic sorted polys
	PARAMETERS:	depth to draw at
	RETURNS:	
**************************************************************************/

void customDrawSortedPrimitives(int depth)
{
	register PACKET*		packet;
	register long			*tfv = transformedVertices;
	VERT 					*tfn = transformedNormals;
	register TMD_P_GT4I		*opcd;
	PSIMODELCTRL			*modctrl = &PSImodelctrl;
	int						primsleft,lightmode;
	ULONG					*sorts = sortedIndex;
	ULONG					sortBucket = 0;
//	VERT					*vp = modctrl->VertTop;
								  
	depth >>= 2;

	primsleft = sortCount;
	if (!primsleft)
		return;

	polyCount += primsleft;	

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
#define si ((POLY_FT3*)packet)
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
#define si ((POLY_FT4*)packet)
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
#define si ((POLY_GT3*)packet)
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
#define si ((POLY_GT4*)packet)
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
#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)

			case GPU_COM_TF4SPR :
				{
/*				SHORT		spritez;
				int		width, height;

//				si = (POLY_FT4*)packet;
//				op = (TMD_P_FT4I*)(opcd);

				BEGINPRIM(si, POLY_FT4);

				testpol = op;
				testsi = si;

	// scaling-and-transform-in-one-go code from the Action Man people...
				width = op->v2;
				gte_SetLDDQB(0);			// clear offset control reg (C2_DQB)
				gte_ldv0(&vp[op->v0]);		// Load centre point
				gte_SetLDDQA(width);		// shove sprite width into control reg (C2_DQA)
				gte_rtps();					// do the rtps
				gte_stsxy(&si->x0);			// get screen x and y
				gte_stsz(&spritez);		// get order table z
	// end of scaling-and-transform


				if (spritez < 20) break;

				gte_stopz(&width);		// get scaled width of sprite
				width >>= 17;

 				*(u_long *) & si->r0 = *(u_long *) & op->r0;			// Texture coords / colors
				*(u_long *) & si->u0 = *(u_long *) & op->tu0;
				*(u_long *) & si->u1 = *(u_long *) & op->tu1;
				*(u_long *) & si->u2 = *(u_long *) & op->tu2;
				*(u_long *) & si->u3 = *(u_long *) & op->tu3;

				si->x1 = si->x3=si->x0+width;
				si->x0 = si->x2=si->x0-width;
			
 		 		height = width>>1;//(LONG)(width*(3))/spritez;
			
				si->y2 = si->y3=si->y0+height;
				si->y0 = si->y1=si->y0-height;
				setPolyFT4(si);

				si->code = op->cd | modctrl->semitrans;
		
 				ENDPRIM(si, depth, POLY_FT4);
				op = op->next;*/
			}
			break;

#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_G4*)packet)
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
#define si ((POLY_G3*)packet)
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
		default:
			break;
		}
	}
}
