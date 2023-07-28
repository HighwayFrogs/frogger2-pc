#include "tongue.h"
#include "map_draw.h"
#include "psxtongue.h"
#include "cam.h"

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


void DrawTongueSegment(SVECTOR *vt, TextureType *tEntry, unsigned char r, unsigned char g, unsigned char b )
{
	long *tfv;
	long *tfd;
	int i;
	POLY_FT4 *ft4;
	POLY_F4  *f4;

	//transform the 3d coords into screen xyz
//	tfv = (long*)transformedVertices;
//	tfd = (long*)transformedDepths;
//	transformVertexListA(vt, 4, tfv, tfd);
//	for(i=0; i<4; i++)
//		tfd[i] = tfd[i] >> 2;

	//set up a poly
/*	BEGINPRIM(ft4, POLY_FT4);
	setPolyFT4(ft4);
	*(long*)(&ft4->x0) = *(long*)(&vt[0].vx);
	*(long*)(&ft4->x1) = *(long*)(&vt[1].vx);
	*(long*)(&ft4->x2) = *(long*)(&vt[2].vx);
	*(long*)(&ft4->x3) = *(long*)(&vt[3].vx);
	ft4->r0 = 128;
	ft4->g0 = 128;
	ft4->b0 = 128;
	ft4->u0 = tEntry->u0;
	ft4->v0 = tEntry->v0;
	ft4->u1 = tEntry->u1;
	ft4->v1 = tEntry->v1;
	ft4->u2 = tEntry->u2;
	ft4->v2 = tEntry->v2;
	ft4->u3 = tEntry->u3;
	ft4->v3 = tEntry->v3;
	ft4->tpage = tEntry->tpage;
	ft4->clut  = tEntry->clut;
	ENDPRIM(ft4, 1, POLY_FT4);
*/
	BEGINPRIM(f4, POLY_F4);
	setPolyF4(f4);
	f4->x0 = vt[0].vx;
	f4->y0 = vt[0].vy;
	f4->x1 = vt[1].vx;
	f4->y1 = vt[1].vy;
	f4->x2 = vt[2].vx;
	f4->y2 = vt[2].vy;
	f4->x3 = vt[3].vx;
	f4->y3 = vt[3].vy;
	f4->r0 = r;
	f4->g0 = g;
	f4->b0 = b;
//	ENDPRIM(f4, 1, POLY_F4);
	ENDPRIM(f4, (vt[0].vz+vt[1].vz+vt[2].vz+vt[3].vz)>>4, POLY_F4);
}





void CalcTongueNodes(SVECTOR *vT, int pl, int i)
{
	TONGUE *t = &tongue[pl];
	FVECTOR p1, p2, pos, m, normal, tempV;
	IQUATERNION q, cross;
	fixed p;
	SVECTOR tempSvect;
	MATRIX rMtrx;
	long sz;

	// Calculate matrix to face to screen
	SetVectorFF(&pos, &t->segment[i]);
	SubVectorFFF(&normal, &currCamSource, &pos);
	MakeUnit(&normal);
	CrossProductFFF((FVECTOR*)&cross, &normal, &upVec);
	MakeUnit((FVECTOR*)&cross);
	p = DotProductFF(&normal, &upVec);
	cross.w = -arccos(p);
	fixedGetQuaternionFromRotation(&q, &cross);
	QuatToPSXMatrix(&q, &rMtrx);

	rMtrx.t[0] = -pos.vx>>12;
	rMtrx.t[1] = -pos.vy>>12;
	rMtrx.t[2] = pos.vz>>12;

	// Rotate point 1 to screen then around frog in local space
	p1.vx = (-12000+(i*1024))*SCALE;
	p1.vy = p1.vz = 0;
	ApplyMatrixLV( &rMtrx, &p1, &p1 );
	RotateVectorByQuaternionFF( &p2, &p1, &frog[pl]->actor->qRot );
	SetVectorSF( &vT[0], &p2 );

	// Rotate point 1 to screen then around frog in local space
	p1.vx = (12000+(i*1024))*SCALE;
	p1.vy = p1.vz = 0;
	ApplyMatrixLV( &rMtrx, &p1, &p1 );
	RotateVectorByQuaternionFF( &p2, &p1, &frog[pl]->actor->qRot );
	SetVectorSF( &vT[1], &p2 );

	// Translate and transform point 1
	SetVectorSS( &tempSvect, &vT[0] );
	tempSvect.vx += rMtrx.t[0]; tempSvect.vy += rMtrx.t[1]; tempSvect.vz += rMtrx.t[2];
	gte_SetTransMatrix(&GsWSMATRIX);
	gte_SetRotMatrix(&GsWSMATRIX);
	gte_ldv0(&tempSvect);
	gte_rtps();
	gte_stsxy(&vT[0].vx);
	gte_stszotz(&sz);
	vT[0].vz = sz;

	// Translate and transform point 2
	SetVectorSS( &tempSvect, &vT[1] );
	tempSvect.vx += rMtrx.t[0]; tempSvect.vy += rMtrx.t[1]; tempSvect.vz += rMtrx.t[2];
	gte_SetTransMatrix(&GsWSMATRIX);
	gte_SetRotMatrix(&GsWSMATRIX);
	gte_ldv0(&tempSvect);
	gte_rtps();
	gte_stsxy(&vT[1].vx);
	gte_stszotz(&sz);
	vT[1].vz = sz;
}


void DrawTongue( int pl )
{
	unsigned long i=0, index = (tongue[pl].progress*(MAX_TONGUENODES-1)>>12);
	SVECTOR vT[4], vTPrev[2];
	TextureType *tEntry;
	unsigned char r, g, b;

	tEntry = tongue[pl].tex;

	if(/*!tEntry ||*/ index<2)
		return;

	r = (tongueColours[player[pl].character]>>16) & 0xff;
	g = (tongueColours[player[pl].character]>>8) & 0xff;
	b = (tongueColours[player[pl].character]) & 0xff;

	while(i < index)
	{
		//********-[ First 2 points ]-*******
		if(i && vTPrev[0].vz && vTPrev[1].vz)
			memcpy( vT, vTPrev, sizeof(SVECTOR)*2 );			// Previously transformed vertices
		else
			CalcTongueNodes( vT, pl, i );

		//********-[ Next 2 points ]-********
		CalcTongueNodes( &vT[2], pl, i+1 );
		memcpy(vTPrev, &vT[2], sizeof(SVECTOR)*2); 			// Store first 2 vertices of the next segment

		//********-[ Draw the polys ]-********
		if(vT[0].vz && vT[1].vz && vT[2].vz && vT[3].vz)
		{
			DrawTongueSegment(vT, tEntry, r, g, b);
		}

		i++;
	}

	//need to draw some sort of end 'blob' on the tongue?
}

