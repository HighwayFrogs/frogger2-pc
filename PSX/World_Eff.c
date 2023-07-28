#include "sonylibs.h"
#include <islutil.h>
#include <isltex.h>
#include "shell.h"
#include "types.h"
#include "main.h"
#include "map_draw.h"
#include "actor2.h"
#include "maths.h"
#include "layout.h"
#include "frontend.h"
#include "sprite.h"
#include "frogger.h"
#include "game.h"
#include "psxsprite.h"
#include "temp_psx.h"

#include "World_Eff.h"

// JH: Let's define a name so that we can use sprite's in the scenic drawing routines.
#define NEW_SPRITE_MAPS

#define ADDITIVE		(1<<0)
#define SUBTRACTIVE		(1<<1)
#define USLIDING		(1<<4)
#define VSLIDING		(1<<5)
#define PLUSSLIDING		(1<<6)
#define MINUSSLIDING	(1<<7)
#define JIGGLE			(1<<8)
#define MODGY			(1<<9)
#define SLIDESPEED		(1<<10)
#define SLIDESPEED2		(1<<11)
#define SLOWSLIDE		(1<<12)

#define VRAM_STARTX			512
#define VRAM_PAGECOLS		8
#define VRAM_PAGEROWS		2
#define VRAM_PAGES			16
#define VRAM_PAGEW			32
#define VRAM_PAGEH			32
#define VRAM_SETX(X)		(X)
#define VRAM_SETY(Y)		((Y)*VRAM_PAGEW)
#define VRAM_SETXY(X,Y)		((X)+((Y)*VRAM_PAGEW))
#define VRAM_SETPAGE(P)		((P)<<16)
#define VRAM_GETX(HND)		((HND) & (VRAM_PAGEW-1))
#define VRAM_GETY(HND)		(((HND)/VRAM_PAGEW) & (VRAM_PAGEW-1))
#define VRAM_GETXY(HND)		((HND) & 0xffff)
#define VRAM_GETPAGE(HND)	((HND)>>16)
#define VRAM_CALCVRAMX(HND)	(512+((VRAM_GETPAGE(HND)%(VRAM_PAGECOLS))*64)+(VRAM_GETX(HND)*2))
#define VRAM_CALCVRAMY(HND)	(((VRAM_GETPAGE(HND)/(VRAM_PAGECOLS))*256)+(VRAM_GETY(HND)*8))


long slideSpeeds[] = {0, 1, 2, 4};

SVECTOR jiggledVerts[250];


extern int polyCount;

extern USHORT EXPLORE_black_CLUT;

#define gte_stotz_cpu(r)\
asm(\
  "mfc2 %0, $7;"\
  :\
  : "r" (r)\
)

#define addPrimLen(ot,p,len,tempvar) \
{\
	tempvar = *(u_long *)(ot); \
    (*(u_long *)(ot) = (u_long)(p)); \
	tempvar |= (len<<24); \
	*(unsigned long *)(p) = tempvar; \
}

//#define WATERANIM_1A (u+((rcos(frame<<10)+4096)>>5))|((v+((rsin(frame<<10)+4096)>>5))<<4)
//#define WATERANIM_2B (u+((rsin(frame<<10)+4096)>>5))|((v+((rcos(frame<<10)+4096)>>5))<<8)

//#define WATERANIM_1  ( ( u )  ) | ( ( v + ( ( 4096 ) >> 11 ) ) << 8 )
//#define WATERANIM_2  ( ( u )  ) | ( ( v + ( ( 4096 ) >> 11 ) ) << 8 )


//#define WATER_TRANS_CODE (((ULONG)2)<<24)
//#define WATER_TRANS_CLUT (0x80000000)

#define MIN_MAP_DEPTH (10)
	
#define MAX_MAP_DEPTH (1000)




SCENICOBJ *CreateAndAddScenicObject(SCENIC *sc)
{
	SCENICOBJ *newItem;

	newItem = ( SCENICOBJ* ) MALLOC0 ( sizeof ( SCENICOBJ ) );

	AddScenicObj ( newItem );

	newItem->flags = 0;

	//utilPrintf("SCENIC %s, ROT %d %d %d %d\n", sc->name, sc->rot.x, sc->rot.y, sc->rot.z, sc->rot.w);

	QuatToPSXMatrix(&sc->rot, &newItem->matrix);

	newItem->matrix.t[0] =  sc->pos.vx;
	newItem->matrix.t[1] =  sc->pos.vy;
	newItem->matrix.t[2] =  sc->pos.vz;
	
	//utilPrintf("Creating Scenic Object : %s\n", sc->name);

	newItem->fmaObj = ( void * ) BFF_FindObject ( BFF_FMA_WORLD_ID, utilStr2CRC ( sc->name ) );

	if ( newItem->fmaObj )
		SetUpWaterMesh ( newItem->fmaObj );
	else
		utilPrintf("Could Not Create Scenic Object.\n");

	if( sc->name[0] == 'S' && sc->name[1] == 'P')
	{
		switch( sc->name[2] )
		{
			case 'L': newItem->flags |= SLIDESPEED; break;
			case 'M': newItem->flags |= SLIDESPEED2; break;
			case 'F': newItem->flags |= (SLIDESPEED | SLIDESPEED2); break;
		}
	}
	return newItem;
}



void DrawScenicObjList ( void )
{
	int i;
	SCENICOBJ *cur;
	FMA_MESH_HEADER **mesh;

	if ( scenicObjList.numEntries == 0 )
		return;

	for ( cur = scenicObjList.head.next; cur != &scenicObjList.head; cur = cur->next)
	{
		if ( cur->fmaObj )
		{
			mesh = ADD2POINTER(cur->fmaObj,sizeof(FMA_WORLD));

			for ( i = cur->fmaObj->n_meshes; i != 0; i--, mesh++ )
			{
				MapDraw_SetScenicMatrix(*mesh, cur);

				if ( MapDraw_ClipCheck ( *mesh ) )
					DrawScenicObj ( *mesh, cur->flags );
			}
		}
	}
}


void InitScenicObjLinkedList ( void )
{
	scenicObjList.numEntries = 0;
	scenicObjList.head.next = scenicObjList.head.prev = &scenicObjList.head;
}


void AddScenicObj ( SCENICOBJ *scenicObj )
{
	if ( scenicObj->next == NULL )
	{
		scenicObjList.numEntries++;
		scenicObj->prev								= &scenicObjList.head;
		scenicObj->next								= scenicObjList.head.next;
		scenicObjList.head.next->prev	= scenicObj;
		scenicObjList.head.next				=	scenicObj;
	}
}

void FreeScenicObjectList ( void )
{
	SCENICOBJ *cur,*next;

	if( !scenicObjList.numEntries )
		return;

	for ( cur = scenicObjList.head.next; cur != &scenicObjList.head; cur = next )
	{
		next = cur->next;
		SubScenicObject(cur);
	}
}


void SubScenicObject ( SCENICOBJ *scenicObj )
{
	if(scenicObj->next == NULL)
		return;

	scenicObj->prev->next = scenicObj->next;
	scenicObj->next->prev = scenicObj->prev;
	scenicObj->next = NULL;
	scenicObjList.numEntries--;

//	FREE(scenicObj);
	scenicObj = NULL;
}

unsigned long oldperiod=0, oldfr=0;
void DrawScenicObj ( FMA_MESH_HEADER *mesh, int flags )
{
	register u_long t1 asm("$16");
	register u_long t2 asm("$20");

	register PACKET * packet asm("$17");

	register char *opcd asm("$18");

	long *tfv;
	long *tfd;

	#define GETX(n)( ((SHORTXY *)( (int)(tfv) +(n) ))->x )
	#define GETY(n)( ((SHORTXY *)( (int)(tfv) +(n) ))->y )
	#define GETV(n)(  *(u_long *)( (int)(tfv) +(n) ) )
	#define GETD(n)(  *(u_long *)( (int)(tfd) +(n) ) )

	register long depth asm("$16");

	unsigned long *ot = currentDisplayPage->ot+mesh->extra_depth;

	int i, j;

	int min_depth = MIN_MAP_DEPTH;// + mesh->extra_depth;
	int max_depth = (fog.max>>4);// + mesh->extra_depth;

	// Store UV modge vals for a poly
	short mVs[8];

	// Is this an sp(m/f/l) type slidy thing? If so, what speed?
	unsigned short pcStyleSlide;
	int sldSpd;
	unsigned long period1, period2, fr;

	if((gameState.mode != PAUSE_MODE ) && (gameState.mode != GAMEOVER_MODE))
	{
		pcStyleSlide = ((flags>>10)&3);

		period1 = frameCount<<5; period2 = frameCount<<7;
		oldperiod = period1;

		fr = frame<<5;

		// Multiply speed if we have a multiple
		if( pcStyleSlide )
			sldSpd = ((frame>>1)*slideSpeeds[pcStyleSlide])%32;
		else
			sldSpd = (frame>>1)%32;

		// Reverse if told to
		if( mesh->flags & MINUSSLIDING )
			sldSpd = -sldSpd;
	}
	else
	{
		sldSpd = 0;
		period1 = oldperiod;
		period2 = oldperiod<<2;
		fr = oldfr;
	}

	if(mesh->n_verts <= 126)	// Not 128. TransformVerts rounds up to nearest 3, remember
	{
		tfv = (void *)0x1f800000;
		tfd = (void *)(0x1f800000 + 512);
	}
	else if(mesh->n_verts <= 255)	// Not 256. TransformVerts rounds up to nearest 3, remember
	{
		tfv = (void *)0x1f800000;
		tfd = transformedDepths;
	}
	else
	{
		tfv = transformedVertices;
		tfd = transformedDepths;
	}

	if( max_depth > 1024 - mesh->extra_depth )
		max_depth = 1024 - mesh->extra_depth;
	
	if( mesh->flags & JIGGLE )
	{
		// Interesting. The framecount shift controls the periiod of oscillation,
		// and the downshift at the end decreases the amplitude from huge.
		for ( i = 0; i < mesh->n_verts; i++ )
		{
			jiggledVerts[i].vx = mesh->verts[i].vx + (rcos(period1 + (mesh->verts[i].vx & mesh->verts[i].vz))>>8);
			jiggledVerts[i].vy = mesh->verts[i].vy + (rsin(period2 + (mesh->verts[i].vx ^ mesh->verts[i].vz))>>7);
			jiggledVerts[i].vz = mesh->verts[i].vz + (rcos(period1 + (mesh->verts[i].vx | mesh->verts[i].vz))>>8);
		}

		transformVertexListA ( jiggledVerts, mesh->n_verts, tfv, tfd );
	}
	else
	{
		transformVertexListA(mesh->verts, mesh->n_verts, tfv, tfd);
	}

#define si ((POLY_GT4*)packet)
#define op ((FMA_GT4 *)opcd)

	op = mesh->gt4s;
	packet = (PACKET *)currentDisplayPage->primPtr;
	polyCount += mesh->n_gt4s;

	for ( i = mesh->n_gt4s; i != 0; i--, op++ )
	{
		gte_ldsz4 ( GETD ( op->vert0 ), GETD ( op->vert1 ), GETD ( op->vert2 ), GETD ( op->vert3 ) );
	  gte_avsz4();
		gte_stotz_cpu ( depth );

		if ( depth > min_depth && depth < max_depth )
		{
			if( ( ( GETV ( op->vert0 ) & 0xff80ff00 ) + 0x00800100 ) &
					( ( GETV ( op->vert1 ) & 0xff80ff00 ) + 0x00800100 ) &
					( ( GETV ( op->vert2 ) & 0xff80ff00 ) + 0x00800100 ) &
					( ( GETV ( op->vert3 ) & 0xff80ff00 ) + 0x00800100 ) & 0x01000200 )
			{
				continue;
			}

			gte_ldsxy3 ( GETV ( op->vert0 ), GETV ( op->vert1 ), GETV ( op->vert2 ) );

			addPrimLen ( ot + ( depth + mesh->shift), si, 12, t2 );

			// Precalc modge values for this quad
			if ( mesh->flags & MODGY )
			{
				int vv;
				for( j=3; j>=0; j-- )
				{
					vv = fr + (mesh->verts[*(&op->vert0+j)].vx * mesh->verts[*(&op->vert0+j)].vz);
					mVs[j<<1]		= rsin(vv)>>10;
					mVs[(j<<1)+1]	= rcos(vv)>>10;
				}
			}

			*(u_long *)  (&si->u0) = *(u_long *) (&op->u0);		// Texture coords
			*(u_long *)  (&si->u1) = *(u_long *) (&op->u1);

			if( mesh->flags & USLIDING )
			{
				si->u0 += sldSpd;
				si->u1 += sldSpd;
			}
			if( pcStyleSlide || (mesh->flags & VSLIDING) )
			{
				si->v0 += sldSpd;
				si->v1 += sldSpd;
			}

			if ( mesh->flags & MODGY )
			{
				si->u0 += mVs[0];
				si->v0 += mVs[1];
				si->u1 += mVs[2];
				si->v1 += mVs[3];
			}
			
			gte_stsxy3_gt4(si);
								
			*(u_long *)  (&si->u2) = *(u_long *) (&op->u2);
			*(u_long *)  (&si->u3) = *(u_long *) (&op->u3);

			if ( mesh->flags & USLIDING )
			{
				si->u2 += sldSpd;
				si->u3 += sldSpd;
			}
			if( pcStyleSlide || (mesh->flags & VSLIDING) )
			{
				si->v2 += sldSpd;
				si->v3 += sldSpd;
			}

			if ( mesh->flags & MODGY )
			{
				si->u2 += mVs[4];
				si->v2 += mVs[5];
				si->u3 += mVs[6];
				si->v3 += mVs[7];
			}

			*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
			*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
			*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
			*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);

			*(u_long *)  (&si->x3) = *(u_long *) ( &GETV ( op->vert3 ) );

			si->code	= op->code | ( op->pad2 & 2 );
			si->tpage = op->tpage | ( op->pad2 & 96 );
			if( globalClut ) si->clut = globalClut;

			packet = ADD2POINTER ( packet, sizeof ( POLY_GT4 ) );
		}
	}
#undef op
#undef si

#define si ((POLY_GT3*)packet)
#define op ((FMA_GT3 *)opcd)

	op = mesh->gt3s;

	polyCount += mesh->n_gt3s;

	for ( i = mesh->n_gt3s; i != 0; i--, op++ )
	{
		gte_ldsz3 ( GETD ( op->vert0 ), GETD ( op->vert1 ), GETD ( op->vert2 ) );
	   	gte_avsz3();
		gte_stotz_cpu ( depth );

		if ( depth > min_depth && depth < max_depth )
		{
			if(
			((GETV(op->vert0) & 0xff80ff00) + 0x00800100)
			&
			((GETV(op->vert1) & 0xff80ff00) + 0x00800100)
			&
			((GETV(op->vert2) & 0xff80ff00) + 0x00800100)
			& 0x01000200
			)
			continue;

			gte_ldsxy3 ( GETV ( op->vert0 ), GETV ( op->vert1 ), GETV ( op->vert2 ) );
			addPrimLen ( ot + ( depth + mesh->shift), si, 9, t2 );

			// Precalc modge values for this vertex
			if ( mesh->flags & MODGY )
			{
				int vv;
				for( j=2; j>=0; j-- )
				{
					vv = fr+(mesh->verts[*(&op->vert0+j)].vx * mesh->verts[*(&op->vert0+j)].vz);
					mVs[j<<1]		= rsin(vv)>>10;
					mVs[(j<<1)+1]	= rcos(vv)>>10;
				}
			}

			*(u_long *)  (&si->u0) = *(u_long *) (&op->u0);
			*(u_long *)  (&si->u1) = *(u_long *) (&op->u1);

			if ( mesh->flags & USLIDING )
			{
				si->u0 += sldSpd;
				si->u1 += sldSpd;
			}
			if( pcStyleSlide || (mesh->flags & VSLIDING) )
			{
				si->v0 += sldSpd;
				si->v1 += sldSpd;
			}

			if ( mesh->flags & MODGY )
			{
				si->u0 += mVs[0];
				si->v0 += mVs[1];
				si->u1 += mVs[2];
				si->v1 += mVs[3];
			}
			
			gte_stsxy3_gt3(si);
			*(u_long *)  (&si->u2) = *(u_long *) (&op->u2);

			if ( mesh->flags & USLIDING )
				si->u2 += sldSpd;
			if( pcStyleSlide || (mesh->flags & VSLIDING) )
				si->v2 += sldSpd;

			if ( mesh->flags & MODGY )
			{
				si->u2 += mVs[4];
				si->v2 += mVs[5];
			}

			*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
			*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
			*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);

			si->code	= op->code | ( op->pad2 & 2 );
			si->tpage = op->tpage | ( op->pad2 & 96 );

			packet = ADD2POINTER ( packet, sizeof ( POLY_GT3 ) );
		}
	}
#undef op
#undef si

	currentDisplayPage->primPtr = (char *)packet;


#define si ((POLY_FT4*)packet)
#define op ((FMA_SPR *)opcd)

	op = mesh->sprs;

	for(i = mesh->n_sprs; i != 0; i--,op++)
	{
		LONG spritez, width, height;
		SVECTOR tempVect;

		BEGINPRIM(si, POLY_FT4);

		setPolyFT4(si);

		tempVect.vx = -op->x;
		tempVect.vy = -op->y;
		tempVect.vz = -op->z;

		width = 32;

		gte_SetLDDQB(0);		// clear offset control reg (C2_DQB)
		gte_ldv0(&tempVect);
		gte_SetLDDQA(width);	// shove sprite width into control reg (C2_DQA)
		gte_rtps();				// do the rtps
		gte_stsxy(&si->x0);		// get screen x and y
		gte_stsz(&spritez);		// get screen z

		if ( spritez <= 0 || spritez >= fog.max ) 
			continue;

		width = ((op->w * SCALEX * fmaActorScale.vx)>>12) / spritez;
		if(width < 2 || width > 256)
			continue;

 		si->x1 = si->x3 = si->x0 + width;
 		si->x0 = si->x2 = si->x0 - width;
   		if (si->x1<=-256) continue;
   		if (si->x0>=256) continue;

		height = ((op->h * SCALEY * fmaActorScale.vx)>>12) / spritez;

		si->y2 = si->y3 = si->y0 + height;
		si->y0 = si->y1 = si->y0 - height;
   		if (si->y2<= -128) continue;
   		if (si->y0>= 128) continue;

		si->r0 = op->r0;
		si->g0 = op->g0;
		si->b0 = op->b0;

		si->u0 = op->u0;
		si->v0 = op->v0;

		si->u1 = op->u1;
		si->v1 = op->v1;

		si->u2 = op->u2;
		si->v2 = op->v2;

		si->u3 = op->u3;
		si->v3 = op->v3;

		si->tpage = op->tpage;
		si->clut = op->clut;

		si->code = GPU_COM_TF4;

		ENDPRIM(si, spritez>>4, POLY_FT4);
	}

#undef si
#undef op
}


// For space - NOT a bitmap!
void PrintStaticBackdrop( FMA_MESH_HEADER *mesh )
{
	register u_long t1 asm("$16");
	register u_long t2 asm("$20");

	register PACKET * packet asm("$17");

	register char *opcd asm("$18");

	long *tfv;
	long *tfd;

	#define GETX(n)( ((SHORTXY *)( (int)(tfv) +(n) ))->x )
	#define GETY(n)( ((SHORTXY *)( (int)(tfv) +(n) ))->y )
	#define GETV(n)(  *(u_long *)( (int)(tfv) +(n) ) )
	#define GETD(n)(  *(u_long *)( (int)(tfd) +(n) ) )

	register long depth asm("$16");

	unsigned long *ot = currentDisplayPage->ot + mesh->extra_depth,
											min_depth = MIN_MAP_DEPTH,
											max_depth = fog.max>>2;

	int i, j;

	if(mesh->n_verts <= 126)	// Not 128. TransformVerts rounds up to nearest 3, remember
	{
		tfv = (void *)0x1f800000;
		tfd = (void *)(0x1f800000 + 512);
	}
	else if(mesh->n_verts <= 255)	// Not 256. TransformVerts rounds up to nearest 3, remember
	{
		tfv = (void *)0x1f800000;
		tfd = transformedDepths;
	}
	else
	{
		tfv = transformedVertices;
		tfd = transformedDepths;
	}

	if( max_depth > 1024 - mesh->extra_depth )
		max_depth = 1024 - mesh->extra_depth;

	transformVertexListA(mesh->verts, mesh->n_verts, tfv, tfd);

	packet = (PACKET *)currentDisplayPage->primPtr;

#define si ((POLY_GT4*)packet)
#define op ((FMA_GT4 *)opcd)

	op = mesh->gt4s;
	polyCount += mesh->n_gt4s;

	for( i = mesh->n_gt4s; i != 0; i--, op++ )
	{
		gte_ldsz4 ( GETD ( op->vert0 ), GETD ( op->vert1 ), GETD ( op->vert2 ), GETD ( op->vert3 ) );
	   	gte_avsz4();
		gte_stotz_cpu ( depth );

		if( depth > max_depth )
			depth = max_depth-1;

		if( depth > min_depth )
		{
			if( ( ( GETV ( op->vert0 ) & 0xff80ff00 ) + 0x00800100 ) &
					( ( GETV ( op->vert1 ) & 0xff80ff00 ) + 0x00800100 ) &
					( ( GETV ( op->vert2 ) & 0xff80ff00 ) + 0x00800100 ) &
					( ( GETV ( op->vert3 ) & 0xff80ff00 ) + 0x00800100 ) & 0x01000200 )
			{
				continue;
			}

			gte_ldsxy3 ( GETV ( op->vert0 ), GETV ( op->vert1 ), GETV ( op->vert2 ) );

			addPrimLen ( currentDisplayPage->ot + 1023, si, 12, t2 );

			*(u_long *)  (&si->u0) = *(u_long *) (&op->u0);		// Texture coords
			*(u_long *)  (&si->u1) = *(u_long *) (&op->u1);

			
			gte_stsxy3_gt4(si);
								
			*(u_long *)  (&si->u2) = *(u_long *) (&op->u2);
			*(u_long *)  (&si->u3) = *(u_long *) (&op->u3);
			*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
			*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
			*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
			*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);

			*(u_long *)  (&si->x3) = *(u_long *) ( &GETV ( op->vert3 ) );

			//si->code	= op->code;
			//si->tpage	= op->tpage;
			//if( globalClut ) si->clut = globalClut;

			packet = ADD2POINTER ( packet, sizeof ( POLY_GT4 ) );
		}
	}
#undef op
#undef si

#define si ((POLY_GT3*)packet)
#define op ((FMA_GT3 *)opcd)

	op = mesh->gt3s;

	polyCount += mesh->n_gt3s;

	for ( i = mesh->n_gt3s; i != 0; i--, op++ )
	{
		gte_ldsz3 ( GETD ( op->vert0 ), GETD ( op->vert1 ), GETD ( op->vert2 ) );
	   	gte_avsz3();
		gte_stotz_cpu ( depth );

		if( depth > max_depth )
			depth = max_depth-1;

		if ( depth > min_depth )
		{
			if(
			((GETV(op->vert0) & 0xff80ff00) + 0x00800100)
			&
			((GETV(op->vert1) & 0xff80ff00) + 0x00800100)
			&
			((GETV(op->vert2) & 0xff80ff00) + 0x00800100)
			& 0x01000200
			)
			continue;

			gte_ldsxy3 ( GETV ( op->vert0 ), GETV ( op->vert1 ), GETV ( op->vert2 ) );
			addPrimLen ( currentDisplayPage->ot + 1023, si, 9, t2 );

			*(u_long *)  (&si->u0) = *(u_long *) (&op->u0);
			*(u_long *)  (&si->u1) = *(u_long *) (&op->u1);

			gte_stsxy3_gt3(si);
			*(u_long *)  (&si->u2) = *(u_long *) (&op->u2);

			*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
			*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
			*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);

			//si->code	= op->code;
			//si->tpage	= op->tpage;

			packet = ADD2POINTER ( packet, sizeof ( POLY_GT3 ) );
		}
	}
#undef op
#undef si

	currentDisplayPage->primPtr = (char *)packet;
}
