//#define TRUE_OBJECT_FOG

/*

	This file is part of the M libraries,

	File		: 
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <crtdbg.h>
#include <stdio.h>

#include "mdxDDraw.h"
#include "mdxD3D.h"
#include "mdxMath.h"
#include "mdxTexture.h"
#include "mdxObject.h"
#include "mdxActor.h"
#include "mdxCRC.h"
#include "mdxTiming.h"
#include "mgeReport.h"
#include "gelf.h"
#include "mdxLandscape.h"
#include "mdxRender.h"
#include "mdxPoly.h"
#include "mdxProfile.h"
#include "math.h"

#ifdef __cplusplus
extern "C"
{
#endif

unsigned long drawingSwampy;
long noClip = 0;

#define MAX_OBJECT_VERTICES 8000
#define SETALPHA(rgba, x) ((((long)(x)) << 24) | ((rgba & 0x00ffffff)))
#define MODALPHA(rgba, x) (((((long)(x*(rgba>>24))>>8)) << 24) | ((rgba & 0x00ffffff)))
#define BETWEEN(x,a,b) ((x>a) && (x<b))

float globalXLU;
float globalXLU2;
MDX_MATRIX vMatrix;
MDX_VECTOR tV[MAX_OBJECT_VERTICES];
float tMx[MAX_OBJECT_VERTICES];
float tMy[MAX_OBJECT_VERTICES];
float tMz[MAX_OBJECT_VERTICES];
MDX_VECTOR tN[MAX_OBJECT_VERTICES];

short facesON[3] = {0,1,2};

float nearClip = 10;
float farClip = 1200;

float horizClip = 3000;
float vertClip = 2500;

long DIST=-10;
long FOV=450 / (480.0/480.0);
float oneOver[65535];
float posAddr[10000];

float fogStart = 400;
float fogRange = 1.0/((1200.0-400.0));
#define MAX_ADDER 1

#define FOG(x) (1.0- (x-fogStart)*fogRange) 
#define FOGVAL(x) (((long)(x*0xff))<<24)

float halfWidth;
float halfHeight;
unsigned long noClipping = 0;

// Outcode enum
enum {OUTCODE_LEFT, OUTCODE_RIGHT, OUTCODE_TOP, OUTCODE_BOTTOM};

// Outcode macros for clipping.
#define OUTCODEF_LEFT	(1<<OUTCODE_LEFT)
#define OUTCODEF_RIGHT	(1<<OUTCODE_RIGHT)
#define OUTCODEF_TOP	(1<<OUTCODE_TOP)
#define OUTCODEF_BOTTOM	(1<<OUTCODE_BOTTOM)

// Calculate the actual outcode
#define CALC_OUTCODE(x,y,x0,y0,x1,y1) \
						((((y)>=(y1))<<OUTCODE_BOTTOM)| \
						 (((y)<=(y0))<<OUTCODE_TOP)| \
						 (((x)>=(x1))<<OUTCODE_RIGHT)| \
						 (((x)<=(x0))<<OUTCODE_LEFT))

// The 2D clip Volume.
float clx0 = 0,//320-160,
      cly0 = 0,//240-120,
	  clx1 = 639,//320+160,
	  cly1 = 479;//240+120;

// Subtracted Square!
//#define sbsqr(x,y) ((x-y)*(x-y))

float xl = 1;

void SetupRenderer(long xRes, long yRes)
{
	clx0 = 0;
	cly0 = 0;
	clx1 = xRes - 1;
	cly1 = yRes - 1;
	FOV = 450; 			
}


/*---------------------------------------------------------------------------------------------
	Function	: calcIntVertex
	Parameters	: (D3DTLVERTEX *vOut, int outcode, D3DTLVERTEX *v0,D3DTLVERTEX *v1, float cx0, float cy0, float cx1, float cy1)
	Returns		: inline int 
*/

float   rCol,gCol,bCol;

inline int __fastcall calcIntVertex(D3DTLVERTEX *vOut, int outcode, D3DTLVERTEX *v0,D3DTLVERTEX *v1, float cx0, float cy0, float cx1, float cy1)
{
	float segLen, totalLen, vt;
	long r1,g1,b1,a1;
	long r2,g2,b2,a2;

	switch(outcode)
	{
		case OUTCODE_LEFT:
			segLen = cx0-v0->sx;
			totalLen = v1->sx-v0->sx;
			
			vt = segLen/totalLen;
			
			a1 = RGBA_GETALPHA(v0->color);
			r1 = RGBA_GETRED(v0->color);
			g1 = RGBA_GETGREEN(v0->color);
			b1 = RGBA_GETBLUE(v0->color);

			a2 = RGBA_GETALPHA(v1->color);
			r2 = RGBA_GETRED(v1->color);
			g2 = RGBA_GETGREEN(v1->color);
			b2 = RGBA_GETBLUE(v1->color);
			
			vOut->sx = cx0;

			vOut->sy = v0->sy+((v1->sy-v0->sy)*vt);
			break;
		case OUTCODE_RIGHT:
			segLen = cx1-v0->sx;
			totalLen = v1->sx-v0->sx;
			vt = segLen/totalLen;
			a1 = RGBA_GETALPHA(v0->color);
			r1 = RGBA_GETRED(v0->color);
			g1 = RGBA_GETGREEN(v0->color);
			b1 = RGBA_GETBLUE(v0->color);

			a2 = RGBA_GETALPHA(v1->color);
			r2 = RGBA_GETRED(v1->color);
			g2 = RGBA_GETGREEN(v1->color);
			b2 = RGBA_GETBLUE(v1->color);
			vOut->sx = cx1;
			vOut->sy = v0->sy+((v1->sy-v0->sy)*vt);
			break;
		case OUTCODE_TOP:
			segLen = cy0-v0->sy;
			totalLen = v1->sy-v0->sy;
			vt = segLen/totalLen;
			a1 = RGBA_GETALPHA(v0->color);
			r1 = RGBA_GETRED(v0->color);
			g1 = RGBA_GETGREEN(v0->color);
			b1 = RGBA_GETBLUE(v0->color);

			a2 = RGBA_GETALPHA(v1->color);
			r2 = RGBA_GETRED(v1->color);
			g2 = RGBA_GETGREEN(v1->color);
			b2 = RGBA_GETBLUE(v1->color);
			vOut->sx = v0->sx+((v1->sx-v0->sx)*vt);
			vOut->sy = cy0;
			break;
		case OUTCODE_BOTTOM:
			segLen = cy1-v0->sy;
			totalLen = v1->sy-v0->sy;
			vt = segLen/totalLen;
			a1 = RGBA_GETALPHA(v0->color);
			r1 = RGBA_GETRED(v0->color);
			g1 = RGBA_GETGREEN(v0->color);
			b1 = RGBA_GETBLUE(v0->color);

			a2 = RGBA_GETALPHA(v1->color);
			r2 = RGBA_GETRED(v1->color);
			g2 = RGBA_GETGREEN(v1->color);
			b2 = RGBA_GETBLUE(v1->color);

			vOut->sx = v0->sx+((v1->sx-v0->sx)*vt);
			vOut->sy = cy1;
			break;		
	}
	
	vOut->tu = (v0->tu+((v1->tu-v0->tu)*vt));
    vOut->tv = (v0->tv+((v1->tv-v0->tv)*vt));
	vOut->sz = (v0->sz+((v1->sz-v0->sz)*vt));
	vOut->rhw = 1;///vOut->sz;
	
	//vOut->specular = v0->specular;//((long)(((long)v0->specular>>24)+((((long)v1->specular>>24)-((long)v0->specular>>24))*vt)))<<24;
	
	
	vOut->color = RGBA_MAKE ((long)(r1+(r2-r1)*vt),(long)(g1+(g2-g1)*vt),(long)(b1+(b2-b1)*vt),(long)(a1+(a2-a1)*vt));

	a1 = RGBA_GETALPHA(v0->specular);
// Not required unless we have some specular values!
//	r1 = RGBA_GETRED(v0->specular);
//	g1 = RGBA_GETGREEN(v0->specular);
//	b1 = RGBA_GETBLUE(v0->specular);

	a2 = RGBA_GETALPHA(v1->specular);

// Not required unless we have some specular values!
//	r2 = RGBA_GETRED(v1->specular);
//	g2 = RGBA_GETGREEN(v1->specular);
//	b2 = RGBA_GETBLUE(v1->specular);

	vOut->specular = RGBA_MAKE (0,0,0,(long)(a1+(a2-a1)*vt));

	return !((vOut->sx==v0->sx)&&(vOut->sy==v0->sy));
}



void __fastcall Clip3DPolygon (D3DTLVERTEX in[3], MDX_TEXENTRY *tEntry)
{
	register
//	D3DTLVERTEX		vList[10];
	int				out0, out1;
	D3DTLVERTEX 	*v0ptr, *v1ptr, vBuf1[8], vBuf2[8], *vIn, *vOut, *vTmp,*vOut2,*vIn2;
	int				vInCount, vOutCount;
	int				sideLp, vertLp, sideMask;
	short			faceList[8*3];
	short			*fL;
	long			i,j;
	// Set up in & out buffers											
	
	vIn = vBuf1;
	
	vIn[0] = in[0];
	vIn[1] = in[1];
	vIn[2] = in[2];
	
	//memcpy (vIn,in,sizeof(D3DTLVERTEX)*3);
	
	vInCount = 3;
	vOut = vBuf2;
	
	vOutCount = 0;

	for(sideLp=0; sideLp<4; sideLp++)
	{
		sideMask = 1<<sideLp;
		vOut2 = vOut;
		vIn2 = vIn;
		for(vertLp=0; vertLp<vInCount; vertLp++)
		{
			v0ptr = vIn2;
			
			if ((vertLp+1)<vInCount)
				v1ptr = vIn2+1;
			else
				v1ptr = vIn;

			out0 = CALC_OUTCODE(v0ptr->sx,v0ptr->sy, clx0,cly0,clx1,cly1);
			out1 = CALC_OUTCODE(v1ptr->sx,v1ptr->sy, clx0,cly0,clx1,cly1);
			if ((out0 & sideMask)==0)		// v0 on
			{
				*vOut2 = *v0ptr;
				vOut2++;
				vOutCount++;
				if (out1 & sideMask)		// v0 on, v1 off
				{
					// add intersection to output
					if (calcIntVertex(vOut2, sideLp, v0ptr, v1ptr, clx0,cly0,clx1,cly1))
					{
						vOutCount++;
						vOut2++;
					}
				}
			}
			else
			{
				if (!(out1 & sideMask))	// v0 off, v1 on
				{
					// add intersection to output
					if (calcIntVertex(vOut2, sideLp, v1ptr, v0ptr, clx0,cly0,clx1,cly1))
					{
						vOutCount++;
						vOut2++;
					}
				}
			}

			vIn2++;
		}
		vTmp = vIn;					// Swap in & out bufs
		vIn = vOut;
		vOut = vTmp;
		vInCount = vOutCount;
		vOutCount = 0;
	}

	j=0;
	fL = faceList;
	for (i=1; i<(vInCount-1); i++)
	{
		*fL = 0;
		*(fL+1) = i;
		*(fL+2) = i+1;
		fL+=3;
	}
	
	j = (i-1)*3;
	if (j)
	{
		PushPolys(vIn,vInCount,faceList,j,tEntry);
	}
}


void XfmPoint(MDX_VECTOR *vTemp2,MDX_VECTOR *in,MDX_MATRIX *d)
{
	float c[4][4],oozd;

	if (d)
	{
		guMtxCatF((float *)d->matrix,(float *)vMatrix.matrix,(float *)c);
		guMtxXFMF(c,in->vx,in->vy,in->vz,&(vTemp2->vx),&(vTemp2->vy),&(vTemp2->vz));
	}
	else
		guMtxXFMF(vMatrix.matrix,in->vx,in->vy,in->vz,&(vTemp2->vx),&(vTemp2->vy),&(vTemp2->vz));
	
	if  (((vTemp2->vz+DIST)>nearClip) &&
		((vTemp2->vz+DIST)<farClip || noClip) &&
		((vTemp2->vx)>-horizClip) &&
		((vTemp2->vx)<horizClip) &&
		((vTemp2->vy)>-vertClip) &&
		((vTemp2->vy)<vertClip))
	{
		oozd = -FOV * *(oneOver+fftol((((long *)vTemp2)+2))+DIST);

//		long x = (long)vTemp2->vz+DIST;
//		float oozd = -FOV * oneOver[x];
			
		vTemp2->vx = halfWidth+(vTemp2->vx * oozd);
		vTemp2->vy = halfHeight+(vTemp2->vy * oozd);
	}
	else
		vTemp2->vz = 0;
}

void InitOneOverTable(void)
{
	int i;
	float rDiff = rXRes/640.0F;
	for(i=1; i<65535; i++)
		oneOver[i] = (rDiff)/(float)i;
	halfWidth = rXRes/2.0F;
	halfHeight = rYRes/2.0F;
}

void SetupDOF(long min, long max, float range)
{
	unsigned long i;
	for (i=0; i<10000; i++)
	{
		float adder = MAX_ADDER;
		if ((i>min) && (i<max))
		{
			adder = 0;
			if (i<min+range)
				adder = (((min+range)-i)*MAX_ADDER)/range;

			if (i>max-range)
				adder = ((i-(max-range))*MAX_ADDER)/range;
		}

		posAddr[i] = adder;
	}
}



void PCPrepareObject (MDX_OBJECT *obj, MDX_MESH *me, float m[4][4])
{
	float f[4][4],oozd,*g;
	MDX_VECTOR *in;
	MDX_VECTOR *vTemp2;
	long i;

	in = obj->mesh->vertices;

	guMtxCatF((float *)m,(float *)vMatrix.matrix,(float *)f);
	
	vTemp2 = tV;
	g = (float *)f;

	for (i = obj->mesh->numVertices; i; i--)
	{
		
		vTemp2->vz = (*(g+2)*in->vx)+(*(g+4+2)*in->vy)+(*(g+8+2)*in->vz)+*(g+12+2);
		
		if (((vTemp2->vz+DIST)>nearClip) &&
			(((vTemp2->vz+DIST)<farClip || noClip)))
		{
			oozd = -FOV * *(oneOver+fftol((((long *)vTemp2)+2))+DIST);
		
			vTemp2->vx = halfWidth + ((*(g)*in->vx)+(*(g+4)*in->vy)+(*(g+8)*in->vz)+*(g+12))*oozd;
			vTemp2->vy = halfHeight + ((*(g+1)*in->vx)+(*(g+4+1)*in->vy)+(*(g+8+1)*in->vz)+*(g+12+1))*oozd;
		}
		else
			vTemp2->vz = 0;

		
		vTemp2++;
		in++;
	}
}

void PCCalcModgeValues(MDX_OBJECT *obj)
{
	float *mTemp,*mTemp2,*mTemp3;
	MDX_VECTOR *in;
	long i;

	in = obj->mesh->vertices;
	mTemp = tMx;
	mTemp2 = tMz;
	mTemp3 = tMy;

	for (i=0; i<obj->mesh->numVertices; i++)
	{
		*mTemp  = (sinf((in->vz*(1.5/35)+sinf((in->vx*(1.3/49))+timeInfo.frameCount*(2.0/56.0)))+timeInfo.frameCount*(1.5/70.0))+1)*0.1;
		*mTemp2 = (cosf((in->vx*(1.5/49)+cosf((in->vz*(1.3/29))+timeInfo.frameCount*(2.0/66.0)))+timeInfo.frameCount*(1.5/66.0))+1)*0.1;
		*mTemp3 = (sinf(	(in->vz*(1.2/26))+cosf((in->vx*(1.3/49))+timeInfo.frameCount*(1.0/26.0))+timeInfo.frameCount*(1.0/26.0))+1)*5;
		mTemp++;
		mTemp2++;
		mTemp3++;
		in++;
	}
}



void PCPrepareModgyObject (MDX_OBJECT *obj, MDX_MESH *me, float m[4][4])
{
	float f[4][4],*mTemp,ty,oozd;
	MDX_VECTOR *in;
	MDX_VECTOR *vTemp2;
	long i;

	in = obj->mesh->vertices;

	guMtxCatF((float *)m,(float *)vMatrix.matrix,(float *)f);
	
	vTemp2 = tV;
	mTemp = tMy;
	
	for (i=0; i<obj->mesh->numVertices; i++)
	{
		ty = in->vy + *mTemp;

		vTemp2->vx = (f[0][0]*in->vx)+(f[1][0]*ty)+(f[2][0]*in->vz)+(f[3][0]);
		vTemp2->vy = (f[0][1]*in->vx)+(f[1][1]*ty)+(f[2][1]*in->vz)+(f[3][1]);
		vTemp2->vz = (f[0][2]*in->vx)+(f[1][2]*ty)+(f[2][2]*in->vz)+(f[3][2]);

		if (((vTemp2->vz+DIST)>nearClip) &&
			(((vTemp2->vz+DIST)<farClip || noClip) &&
			((vTemp2->vx)>-horizClip) &&
			((vTemp2->vx)<horizClip) &&
			((vTemp2->vy)>-vertClip) &&
			((vTemp2->vy)<vertClip)))
		{
			oozd = -FOV * *(oneOver+fftol((((long *)vTemp2)+2))+DIST);

//			long x = (long)vTemp2->vz+DIST;
//			float oozd = -FOV * oneOver[x];
			
			vTemp2->vx = halfWidth+(vTemp2->vx * oozd);
			vTemp2->vy = halfHeight+(vTemp2->vy * oozd);
		}
		else
			vTemp2->vz = 0;

		vTemp2++;
		mTemp++;
		in++;
	}
}

MDX_VECTOR sheenPos = {0,100,-1700};
MDX_VECTOR sheenEye = {0,100,-1700};
MDX_USHORT2DVECTOR sheenTC[2000];

void PCPrepareModgySheenObject (MDX_OBJECT *obj, MDX_MESH *me, float m[4][4])
{
	MDX_VECTOR *in,vT,vT1,vT2;
	MDX_VECTOR *vTemp2 = obj->mesh->vertices;;
	float ang,len,mul;
	long i;

	short *face = &obj->mesh->faceIndex[0].v[0];

	in = obj->mesh->vertices;

//	vTemp2 = tV;
	sheenEye.vy = 0;
	for (i=0; i<obj->mesh->numFaces*3; i++)
	{
		in = &vTemp2[*face];

		vT.vx = (m[0][0]*in->vx)+(m[1][0]*in->vy)+(m[2][0]*in->vz)+(m[3][0]);
		vT.vy = (m[0][1]*in->vx)+(m[1][1]*in->vy)+(m[2][1]*in->vz)+(m[3][1]);
		vT.vz = (m[0][2]*in->vx)+(m[1][2]*in->vy)+(m[2][2]*in->vz)+(m[3][2]);

		
		SubVector (&vT1,&vT,&sheenEye);
		SubVector (&vT2,&sheenPos,&sheenEye);
		len = mdxMagnitude(&vT1);

		vT1.vy = 0;
		vT2.vy = 0;
		
		Normalise(&vT1);
		Normalise(&vT2);

		
		ang = DotProduct(&vT1,&vT2);

		if (ang>0)
		{
			mul = len*sin(acos(ang)*2)*2;
		
			if (mul>1000)
				mul = 1000;


			sheenTC[i].v[0] = 1024-(mul);
			sheenTC[i].v[1] = 1024-(mul);
		}
		else
			sheenTC[i].v[0] = sheenTC[i].v[1] = 24;

		face++;		
		in++;
	}
}

void PCPrepareObjectNormals(MDX_OBJECT *obj, MDX_MESH *mesh, float m[4][4])
{
	float f[4][4];
	MDX_VECTOR *in;
	MDX_VECTOR *vTemp2;
	long i;
	
	guMtxCatF((float *)m,(float *)vMatrix.matrix,(float *)f);

	vTemp2 = tN;
	in = obj->mesh->vertexNormals;
	
			
	if (obj->mesh->vertexNormals)
	{
		// Go thru the affected vertices, and xfm the corresponding vertices
		for (i=0; i<obj->mesh->numVertices; i++)
		{
			// Transform by our matrix
			vTemp2->vx = (f[0][0]*in->vx)+(f[1][0]*in->vy)+(f[2][0]*in->vz);
			vTemp2->vy = (f[0][1]*in->vx)+(f[1][1]*in->vy)+(f[2][1]*in->vz);
			vTemp2->vz = (f[0][2]*in->vx)+(f[1][2]*in->vy)+(f[2][2]*in->vz);
			vTemp2++;
			in++;
		}
	}
}




void PCPrepareSkinnedObject(MDX_OBJECT *obj, MDX_MESH *mesh, float m[4][4])
{
	float f[16];
	MDX_VECTOR *in;
	MDX_VECTOR *vTemp2;
	MDX_VECTOR *vtxS = mesh->vertices;
	long i,j;
	long *eV;
	float oozd;

	guMtxCatF((float *)m,(float *)vMatrix.matrix,(float *)f);
	eV = ((long *)(obj->effectedVerts));

	// Go thru the affected vertices, and xfm the corresponding vertices
	for (i=obj->numVerts; i; i--)
	{
		j = *eV++;
		
		vTemp2 = tV+j;
		in = vtxS+j;

		vTemp2->vx = (f[0]*in->vx)+(f[4]*in->vy)+(f[8]*in->vz)+(f[12]);
		vTemp2->vy = (f[1]*in->vx)+(f[5]*in->vy)+(f[9]*in->vz)+(f[13]);
		vTemp2->vz = (f[2]*in->vx)+(f[6]*in->vy)+(f[10]*in->vz)+(f[14]);
		oozd = vTemp2->vz+DIST; 
		// Transform to screen space
		if ((oozd>nearClip) &&
			(oozd<farClip))
		{
			oozd = -FOV * *(oneOver+fftol((((long *)vTemp2)+2))+DIST);
			vTemp2->vx = halfWidth + (vTemp2->vx * oozd);
			vTemp2->vy = halfHeight+ (vTemp2->vy * oozd);			
		}
		else
			vTemp2->vz = 0;
	}
}

void PCPrepareSkinnedFlatObject(MDX_OBJECT *obj, MDX_MESH *mesh, float m[4][4])
{
	float f[16],*g;
	MDX_VECTOR *in;
	MDX_VECTOR *vTemp2;
	MDX_VECTOR *vtxS = mesh->vertices;
	long i,j;
	long *eV;
	float oozd;

	guMtxCatF((float *)m,(float *)vMatrix.matrix,(float *)f);
	eV = ((long *)(obj->effectedVerts));

	g = ((float *)vMatrix.matrix);
	// Go thru the affected vertices, and xfm the corresponding vertices
	for (i=obj->numVerts; i; i--)
	{
		j = *eV++;
		
		vTemp2 = tV+j;
		in = vtxS+j;

		vTemp2->vx = (f[0]*in->vx)+(f[4]*in->vy)+(f[8]*in->vz)+(f[12]);
		vTemp2->vy = -55;
		vTemp2->vz = (f[2]*in->vx)+(f[6]*in->vy)+(f[10]*in->vz)+(f[14]);
		
		oozd = vTemp2->vz+DIST; 
		// Transform to screen space
		if ((oozd>nearClip) &&
			((oozd<farClip) &&
			((vTemp2->vx)>-horizClip) &&
			((vTemp2->vx)<horizClip) &&
			((vTemp2->vy)>-vertClip) &&
			((vTemp2->vy)<vertClip)))
		{
			oozd = -FOV * *(oneOver+fftol((((long *)vTemp2)+2))+DIST);
			vTemp2->vx = halfWidth + (vTemp2->vx * oozd);
			vTemp2->vy = halfHeight+ (vTemp2->vy * oozd);			
		}
		else
			vTemp2->vz = 0;
	}
}


void PCPrepareSkinnedObjectOutline(MDX_OBJECT *obj, MDX_MESH *mesh, float m[4][4])
{
	float f[16];
	MDX_VECTOR *in;
	MDX_VECTOR *vTemp2;
	MDX_VECTOR *vtxS = mesh->vertices;
	long i,j;
	long *eV;
	float oozd;

	guMtxCatF((float *)m,(float *)vMatrix.matrix,(float *)f);
	eV = ((long *)(obj->effectedVerts));

	// Go thru the affected vertices, and xfm the corresponding vertices
	for (i=obj->numVerts; i; i--)
	{
		j = *eV++;
		
		vTemp2 = tV+j;
		in = vtxS+j;

		vTemp2->vx = (f[0]*in->vx)+(f[4]*in->vy)+(f[8]*in->vz)+(f[12]);
		vTemp2->vy = (f[1]*in->vx)+(f[5]*in->vy)+(f[9]*in->vz)+(f[13]);
		vTemp2->vz = (f[2]*in->vx)+(f[6]*in->vy)+(f[10]*in->vz)+(f[14]);
		oozd = vTemp2->vz+DIST;
		
		// Transform to screen space
		if ((oozd>nearClip) &&
			((oozd<farClip) &&
			((vTemp2->vx)>-horizClip) &&
			((vTemp2->vx)<horizClip) &&
			((vTemp2->vy)>-vertClip) &&
			((vTemp2->vy)<vertClip)))
		{
			oozd = -FOV * *(oneOver+fftol((((long *)vTemp2)+2))+DIST);
			vTemp2->vx = halfWidth + (vTemp2->vx * oozd)+2;
			vTemp2->vy = halfHeight+ (vTemp2->vy * oozd)-1;
			vTemp2->vz+=6;
		}
		else
			vTemp2->vz = 0;
	}
}


void PCPrepareSkinnedObjectNormals(MDX_OBJECT *obj, MDX_MESH *mesh, float m[4][4])
{
	float f[4][4];
	MDX_VECTOR *in;
	MDX_VECTOR *vTemp2;
	long i,j;
	
	guMtxCatF((float *)m,(float *)vMatrix.matrix,(float *)f);

	if (mesh->vertexNormals)
	{
		// Go thru the affected vertices, and xfm the corresponding vertices
		for (i=0; i<obj->numVerts; i++)
		{
			j = ((long *)(obj->effectedVerts))[i];
			if (j==0)
				j=0;
			// Setup Our pointers
			vTemp2 = &tN[j];
			in = &mesh->vertexNormals[j];
	
			// Transform by our matrix
			vTemp2->vx = (f[0][0]*in->vx)+(f[1][0]*in->vy)+(f[2][0]*in->vz);
			vTemp2->vy = (f[0][1]*in->vx)+(f[1][1]*in->vy)+(f[2][1]*in->vz);
			vTemp2->vz = (f[0][2]*in->vx)+(f[1][2]*in->vy)+(f[2][2]*in->vz);
		}
	}
}

void __fastcall PCPrepareLandscape (MDX_LANDSCAPE *me)
{
	MDX_VECTOR *in;
	D3DTLVERTEX *vTemp2;
	short *tFace;
	long i,j,nFOV;
	float oozd;
	float a0,b0,c0,d0;
	float a1,b1,c1,d1;
	float a2,b2,c2,d2,tFog;
	float frange;

	a0 = vMatrix.matrix[0][0];
	a1 = vMatrix.matrix[0][1];
	a2 = vMatrix.matrix[0][2];

	b0 = vMatrix.matrix[1][0];
	b1 = vMatrix.matrix[1][1];
	b2 = vMatrix.matrix[1][2];

	c0 = vMatrix.matrix[2][0];
	c1 = vMatrix.matrix[2][1];
	c2 = vMatrix.matrix[2][2];

	d0 = vMatrix.matrix[3][0];
	d1 = vMatrix.matrix[3][1];
	d2 = vMatrix.matrix[3][2];
	
	nFOV = -FOV;
	
	if (changedView)
	{
		in = me->vertices;
		vTemp2 = me->xfmVert;
		tFace = me->faceIndex;
		frange = 0xff*fogRange;	

		for (i=0,j=me->numFaces*3; j>0; i++,j--)
		{
			in = &me->vertices[*tFace];
			
			vTemp2->sz = (a2*in->vx)+(b2*in->vy)+(c2*in->vz)+d2;

			if (((vTemp2->sz+DIST)>nearClip) &&
			(((vTemp2->sz+DIST)<farClip)))
			{
				vTemp2->sx = (a0*in->vx)+(b0*in->vy)+(c0*in->vz)+d0;
				vTemp2->sy = (a1*in->vx)+(b1*in->vy)+(c1*in->vz)+d1;
			
				oozd = nFOV * *(oneOver+fftol((((long *)vTemp2)+2))+DIST);

				vTemp2->sx = halfWidth+(vTemp2->sx * oozd);
				vTemp2->sy = halfHeight+(vTemp2->sy * oozd);
				
				tFog = 0;
				
				if (vTemp2->sz>fogStart)
				{
					tFog = (0xff-(vTemp2->sz-fogStart)*frange) ;
					if (tFog>0xff)
						tFog = 0xff;
				}
					
	
				vTemp2->specular = ((long)tFog)<<24;
				
				vTemp2->sz *= 0.00025F;
			}
			else
				vTemp2->sz = 0;

			
			vTemp2++;
			tFace++;
			in++;
		}
	}
}

void __fastcall PCPrepareLandscape2 (MDX_LANDSCAPE *me)
{
	MDX_VECTOR *in;
	D3DTLVERTEX *vTemp2;
	short *tFace;
	long i,x,j;
	float oozd,y;
	float a0,b0,c0,d0;
	float a1,b1,c1,d1;
	float a2,b2,c2,d2;

	a0 = vMatrix.matrix[0][0];
	a1 = vMatrix.matrix[0][1];
	a2 = vMatrix.matrix[0][2];

	b0 = vMatrix.matrix[1][0];
	b1 = vMatrix.matrix[1][1];
	b2 = vMatrix.matrix[1][2];

	c0 = vMatrix.matrix[2][0];
	c1 = vMatrix.matrix[2][1];
	c2 = vMatrix.matrix[2][2];

	d0 = vMatrix.matrix[3][0];
	d1 = vMatrix.matrix[3][1];
	d2 = vMatrix.matrix[3][2];

	in = me->vertices;
	vTemp2 = me->xfmVert;
	tFace = me->faceIndex;
	
	for (i=0,j=me->numFaces*3; j>0; i++,j--)
	{
		in = &me->vertices[*tFace];
		
		vTemp2->sx = (a0*in->vx)+(b0*in->vy)+(c0*in->vz)+d0;
		vTemp2->sy = (a1*in->vx)+(b1*in->vy)+(c1*in->vz)+d1;
		vTemp2->sz = (a2*in->vx)+(b2*in->vy)+(c2*in->vz)+d2;

		if (((vTemp2->sz+DIST)>nearClip) &&
		(((vTemp2->sz+DIST)<farClip) &&
		((vTemp2->sx)>-horizClip) &&
		((vTemp2->sx)<horizClip) &&
		((vTemp2->sy)>-vertClip) &&
		((vTemp2->sy)<vertClip)))
			{
			x = (long)vTemp2->sz + DIST;
			oozd = -FOV * *(oneOver+x);
			y = *(posAddr+x);
			vTemp2->sx = y + halfWidth+(vTemp2->sx * oozd);
			vTemp2->sy = y + halfHeight+(vTemp2->sy * oozd);
		
		}
		else
			vTemp2->sz = 0;

		vTemp2++;
		tFace++;
		in++;
	}
}

void __fastcall PCPrepareLandscape3 (MDX_LANDSCAPE *me)
{
	MDX_VECTOR *in;
	D3DTLVERTEX *vTemp2;
	short *tFace;
	long i,x,j;
	float oozd,y;
	float a0,b0,c0,d0;
	float a1,b1,c1,d1;
	float a2,b2,c2,d2;

	a0 = vMatrix.matrix[0][0];
	a1 = vMatrix.matrix[0][1];
	a2 = vMatrix.matrix[0][2];

	b0 = vMatrix.matrix[1][0];
	b1 = vMatrix.matrix[1][1];
	b2 = vMatrix.matrix[1][2];

	c0 = vMatrix.matrix[2][0];
	c1 = vMatrix.matrix[2][1];
	c2 = vMatrix.matrix[2][2];

	d0 = vMatrix.matrix[3][0];
	d1 = vMatrix.matrix[3][1];
	d2 = vMatrix.matrix[3][2];

	in = me->vertices;
	vTemp2 = me->xfmVert;
	tFace = me->faceIndex;
	
	for (i=0,j=me->numFaces*3; j>0; i++,j--)
	{
		in = &me->vertices[*tFace];
		
		vTemp2->sx = (a0*in->vx)+(b0*in->vy)+(c0*in->vz)+d0;
		vTemp2->sy = (a1*in->vx)+(b1*in->vy)+(c1*in->vz)+d1;
		vTemp2->sz = (a2*in->vx)+(b2*in->vy)+(c2*in->vz)+d2;

		if (((vTemp2->sz+DIST)>nearClip) &&
		(((vTemp2->sz+DIST)<farClip) &&
		((vTemp2->sx)>-horizClip) &&
		((vTemp2->sx)<horizClip) &&
		((vTemp2->sy)>-vertClip) &&
		((vTemp2->sy)<vertClip)))
			{
			x = (long)vTemp2->sz + DIST;
			oozd = -FOV * *(oneOver+x);
			y = *(posAddr+x);
			vTemp2->sx = halfWidth+(vTemp2->sx * oozd) - y;
			vTemp2->sy =/* y -*/ halfHeight+(vTemp2->sy * oozd) - y;
			vTemp2->specular = D3DRGBA(0,0,0,1);
		}
		else
			vTemp2->sz = 0;

		vTemp2++;
		tFace++;
		in++;
	}
}

#define FBETWEEN(ptr,idx,clip2) (!(((*(((long *)ptr)))) & 0x8000000) && (*(((float *)ptr))<clip2))

void PCRenderLandscape(MDX_LANDSCAPE *me)
{
	unsigned long x1on,y1on;
	short facesON[3] = {0,1,2};
	D3DTLVERTEX *v = me->xfmVert;
	MDX_TEXENTRY **tEnt = me->tEntrys;
	

	for (int i=me->numFaces; i; i--)
	{
		if (v->sz && (v+1)->sz && (v+2)->sz)
		{
			y1on =	BETWEEN(v[0].sy,cly0,cly1) +
					BETWEEN(v[1].sy,cly0,cly1) +
					BETWEEN(v[2].sy,cly0,cly1);
		   	if (y1on)
			{
				x1on = BETWEEN(v[0].sx,clx0,clx1) +
					   BETWEEN(v[1].sx,clx0,clx1) +
					   BETWEEN(v[2].sx,clx0,clx1);
				
				if (x1on)
				{
					if ((x1on+y1on==6))
					{
						PushPolys(v,3,facesON,3,(*tEnt));
					}
					else
					{
						Clip3DPolygon(v,*tEnt);
					}
				}	
			}
		}
			tEnt++;
			v+=3;		
	}
}

void AdjustObjectOutline(void)
{
	MDX_VECTOR *vTemp2 = tV;

	for (int i=0; i<400; i++)
	{
		vTemp2->vx -= 4;
		vTemp2->vy += 2;
		vTemp2++;
	}
}

unsigned long numObjDrawn;

void DrawLandscape(MDX_LANDSCAPE *me)
{

	
	if (CheckBoundingBox(me->bBox,&me->objMatrix)==0)
	{
		PCPrepareLandscape(me);
		PCRenderLandscape(me);
		numObjDrawn++;
	}

	if (me->next)
		DrawLandscape(me->next);

	if (me->children)
		DrawLandscape(me->children);


}

void DrawLandscape2(MDX_LANDSCAPE *me)
{
	if (CheckBoundingBox(me->bBox,&me->objMatrix)==0)
	{
		PCPrepareLandscape2(me);
		PCRenderLandscape(me);
		
		numObjDrawn++;
	}

	if (me->next)
		DrawLandscape2(me->next);

	if (me->children)
		DrawLandscape2(me->children);


}

void DrawLandscape3(MDX_LANDSCAPE *me)
{
	if (CheckBoundingBox(me->bBox,&me->objMatrix)==0)
	{
		PCPrepareLandscape3(me);
		PCRenderLandscape(me);
		
		numObjDrawn++;
	}

	if (me->next)
		DrawLandscape3(me->next);

	if (me->children)
		DrawLandscape3(me->children);


}

unsigned long wrapCoords = 0;

void DrawObject(MDX_OBJECT *obj, int skinned, MDX_MESH *masterMesh)
{
	SaveFrame;

	if (obj->flags & OBJECT_FLAGS_MODGE || wrapCoords)
		SwapFrame(MA_FRAME_WRAP);

	if (obj->flags & OBJECT_FLAGS_XLU)
		SwapFrame(MA_FRAME_XLU);

	if (obj->flags & OBJECT_FLAGS_ADDITIVE)
		SwapFrame(MA_FRAME_ADDITIVE);		

	if (obj->flags & OBJECT_FLAGS_GLOW)
	{
		MDX_VECTOR v,cVect,oVect;
		float horiz,horiz2;

		v.vx = obj->objMatrix.matrix[3][0];
		v.vy = obj->objMatrix.matrix[3][1];
		v.vz = obj->objMatrix.matrix[3][2];
					
		SubVector(&cVect,&curAt,&curEye);
		SubVector(&oVect,&curEye,&v);
		cVect.vy = 0;
		oVect.vy = 0;
		Normalise(&cVect);
		Normalise(&oVect);
			
		horiz = fabs(0.7+acos(DotProduct(&cVect,&oVect))*2.5);
		horiz = 0.2+fabs(cosf(horiz))*0.8;
		horiz2 = fabs(1.2+acos(DotProduct(&cVect,&oVect))*2.5);
		horiz2 = 0.2+fabs(sinf(horiz2))*0.8;
		AddHalo(&v,horiz,horiz2,SETALPHA(*((long *)(&(obj->mesh->gouraudColors->x))),(long)(0.8*0xff)),obj->xlu);
		
		SwapFrame(MA_FRAME_GLOW);		
	}

	// If we are a skinned object then we just need to prepare all the skinned vertices, so do that for this sub-object.
	if (skinned)
	{
		if (skinned<3)
		{
			if (obj->flags & OBJECT_FLAGS_FLATSHADOW)
				PCPrepareSkinnedFlatObject(obj, masterMesh,  obj->objMatrix.matrix);
			else
				PCPrepareSkinnedObject(obj, masterMesh,  obj->objMatrix.matrix);
		}
		else
			PCPrepareSkinnedObjectOutline(obj, masterMesh,  obj->objMatrix.matrix);

		if (skinned == 2)
			PCPrepareSkinnedObjectNormals(obj, masterMesh, obj->objMatrix.matrix);
	}
	else
	{ 
		if (!(obj->flags & OBJECT_FLAGS_CLIPPED) || (noClipping))
		{
			numObjectsDrawn++;
	
			if (obj->mesh)
			{
				globalXLU2 = (((float)obj->xlu) / ((float)0xff)) * globalXLU;
				
				if ((obj->flags & OBJECT_FLAGS_MODGE) || (obj->flags & OBJECT_FLAGS_WAVE))
					PCCalcModgeValues(obj);					
					
				if (obj->flags & OBJECT_FLAGS_WAVE)
				{
					PCPrepareModgyObject(obj, obj->mesh,  obj->objMatrix.matrix);
					
					if (obj->flags & OBJECT_FLAGS_SHEEN)
						PCPrepareModgySheenObject(obj, obj->mesh,  obj->objMatrix.matrix);
				}
				else
					PCPrepareObject(obj, obj->mesh,  obj->objMatrix.matrix);

				
				if (obj->phong)
				{
					phong = obj->phong;
					PCPrepareObjectNormals(obj, obj->mesh,  obj->objMatrix.matrix);
					PCRenderObjectPhong(obj);
				}
				else
					if (obj->flags & OBJECT_FLAGS_MODGE)
					{
						if (obj->flags & OBJECT_FLAGS_WAVE)
						{
							if (obj->flags & OBJECT_FLAGS_SHEEN)
								PCRenderModgySheenObject(obj);
							else
								PCRenderModgyObject(obj);
						}
						else
							PCRenderModgyObject2(obj);
					}
					else
						PCRenderObject(obj);

			}
		}
	}

	RestoreFrame;

	// Recurse.
	if(obj->children)
		DrawObject(obj->children, skinned, masterMesh);

	if(obj->next)
		DrawObject(obj->next, skinned, masterMesh);
}

MDX_TEXENTRY *phong;
MDX_TEXENTRY *lightMap;
MDX_TEXENTRY *overrideTex;

void PCRenderObjectSlow (MDX_OBJECT *obj)
{
	long i,j;
	unsigned short fce[3] = {0,1,2};		
	MDX_QUATERNION *c1,*c2,*c3;
	D3DTLVERTEX v[3],*vTemp;
	MDX_SHORTVECTOR *facesIdx;
	unsigned long x1on,x2on,x3on,y1on,y2on,y3on;
	unsigned long v0,v1,v2;
	unsigned long v0a,v1a,v2a;
	long alphaVal;
	MDX_TEXENTRY *tex;
	MDX_TEXENTRY **tex2;
	MDX_VECTOR *tV0,*tV1,*tV2, *tN0,*tN1,*tN2;
	MDX_QUATERNION *cols;
	
	facesIdx = obj->mesh->faceIndex;
	tex2 = obj->mesh->textureIDs;
	cols = obj->mesh->gouraudColors;
	alphaVal = (long)(globalXLU2*255.0);

	for (j=0, i=0; i<obj->mesh->numFaces; i++, j+=3)
	{
		// Get information from the mesh!
		v0 = facesIdx->v[0];
		v1 = facesIdx->v[1];
		v2 = facesIdx->v[2];
		
		
		tV0 = &tV[v0];
		tV1 = &tV[v1];
		tV2 = &tV[v2];

		tN0 = &tN[v0];
		tN1 = &tN[v1];
		tN2 = &tN[v2];
		
		if (overrideTex)
			tex = overrideTex;
		else
			tex = (MDX_TEXENTRY *)(*tex2);
		
		// If we are to be drawn.
		if (((tV0->vz) && (tV1->vz) && (tV2->vz)) && (tex))
		{
			// Get rest of info from mesh
			v0a = j;
			v1a = j+1;
			v2a = j+2;

			c1 = &(cols[v0a]);
			c2 = &(cols[v1a]);
			c3 = &(cols[v2a]);
			
			// Fill out D3DVertices...
			vTemp = v;
				
			vTemp->sx = tV0->vx;
			vTemp->sy = tV0->vy;
			vTemp->sz = (tV0->vz) * 0.00025F;
			vTemp->rhw = 1/vTemp->sz;
			
			vTemp->color = MODALPHA(*((unsigned long *)(&(c1->x))),alphaVal);
			vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (obj->mesh->faceTC[v0a].v[0]*0.000975F);
			vTemp->tv = (obj->mesh->faceTC[v0a].v[1]*0.000975F);
		
			//vTemp->tv = (tN0->vy+0.5);
			//vTemp->tu = (tN0->vx+0.5);
			vTemp++;

			vTemp->sx = tV1->vx;
			vTemp->sy = tV1->vy;
			vTemp->sz = (tV1->vz) * 0.00025F;
			vTemp->rhw = 1/vTemp->sz;
			vTemp->color = MODALPHA(*((unsigned long *)(&(c2->x))),alphaVal);
			vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (obj->mesh->faceTC[v1a].v[0]*0.000975F);
			vTemp->tv = (obj->mesh->faceTC[v1a].v[1]*0.000975F);
			//vTemp->tv = (tN1->vy+0.5);
			//vTemp->tu = (tN1->vx+0.5);
			vTemp++;
			
			vTemp->sx = tV2->vx;
			vTemp->sy = tV2->vy;
			vTemp->sz = (tV2->vz) * 0.00025F;
			vTemp->rhw = 1/vTemp->sz;
			
			vTemp->color = MODALPHA(*((unsigned long *)(&(c3->x))),alphaVal);
			vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (obj->mesh->faceTC[v2a].v[0]*0.000975F);
			vTemp->tv = (obj->mesh->faceTC[v2a].v[1]*0.000975F);
			//vTemp->tv = (tN2->vy+0.5);
			//vTemp->tu = (tN2->vx+0.5);
			
			x1on = BETWEEN(v[0].sx,clx0,clx1);
			x2on = BETWEEN(v[1].sx,clx0,clx1);
			x3on = BETWEEN(v[2].sx,clx0,clx1);
			y1on = BETWEEN(v[0].sy,cly0,cly1);
			y2on = BETWEEN(v[1].sy,cly0,cly1);
			y3on = BETWEEN(v[2].sy,cly0,cly1);

			if ((x1on || x2on || x3on) && (y1on || y2on || y3on))
			{
				if ((x1on && x2on && x3on) && (y1on && y2on && y3on))
				{
					PushPolys(v,3,facesON,3,tex);
				}
				else
				{
					Clip3DPolygon(v,tex);
				}
			}
		}
		
		// Update our pointers
		facesIdx++;
		tex2++;
	}
}

void PCRenderObjectPhong (MDX_OBJECT *obj)
{
	long i,j;
	unsigned short fce[3] = {0,1,2};		
	MDX_QUATERNION *c1,*c2,*c3;
	D3DTLVERTEX v[3],*vTemp;
	MDX_SHORTVECTOR *facesIdx;
	unsigned long x1on,x2on,x3on,y1on,y2on,y3on;
	unsigned long v0,v1,v2;
	unsigned long v0a,v1a,v2a;
	long alphaVal;
	MDX_TEXENTRY *tex;
	MDX_TEXENTRY **tex2;
	MDX_VECTOR *tV0,*tV1,*tV2, *tN0,*tN1,*tN2;
	MDX_QUATERNION *cols;
	
	facesIdx = obj->mesh->faceIndex;
	tex2 = obj->mesh->textureIDs;
	cols = obj->mesh->gouraudColors;
	alphaVal = (long)(globalXLU2*255.0);

	for (j=0, i=0; i<obj->mesh->numFaces; i++, j+=3)
	{
		// Get information from the mesh!
		v0 = facesIdx->v[0];
		v1 = facesIdx->v[1];
		v2 = facesIdx->v[2];
		
		
		tV0 = &tV[v0];
		tV1 = &tV[v1];
		tV2 = &tV[v2];

		tN0 = &tN[v0];
		tN1 = &tN[v1];
		tN2 = &tN[v2];

		
		tex = (MDX_TEXENTRY *)(*tex2);
		
		// If we are to be drawn.
		if (((tV0->vz) && (tV1->vz) && (tV2->vz)) && (tex))
		{
			// Get rest of info from mesh
			v0a = j;
			v1a = j+1;
			v2a = j+2;

			c1 = &(cols[v0a]);
			c2 = &(cols[v1a]);
			c3 = &(cols[v2a]);
			
			// Fill out D3DVertices...
			vTemp = v;
				
			vTemp->sx = tV0->vx;
			vTemp->sy = tV0->vy;
			vTemp->sz = (tV0->vz) * 0.00025F;
			vTemp->rhw = 1/vTemp->sz;
			
			vTemp->color = MODALPHA(*((unsigned long *)(&(c1->x))),alphaVal);
			vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (obj->mesh->faceTC[v0a].v[0]*0.000975F);
			vTemp->tv = (obj->mesh->faceTC[v0a].v[1]*0.000975F);
			
			//vTemp->tv = (tN0->vy+0.5);
			//vTemp->tu = (tN0->vx+0.5);
			vTemp++;

			vTemp->sx = tV1->vx;
			vTemp->sy = tV1->vy;
			vTemp->sz = (tV1->vz) * 0.00025F;
			vTemp->rhw = 1/vTemp->sz;
			vTemp->color = MODALPHA(*((unsigned long *)(&(c2->x))),alphaVal);
			vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (obj->mesh->faceTC[v1a].v[0]*0.000975F);
			vTemp->tv = (obj->mesh->faceTC[v1a].v[1]*0.000975F);
			//vTemp->tv = (tN1->vy+0.5);
			//vTemp->tu = (tN1->vx+0.5);
			vTemp++;
			
			vTemp->sx = tV2->vx;
			vTemp->sy = tV2->vy;
			vTemp->sz = (tV2->vz) * 0.00025F;
			vTemp->rhw = 1/vTemp->sz;
			
			vTemp->color = MODALPHA(*((unsigned long *)(&(c3->x))),alphaVal);
			vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (obj->mesh->faceTC[v2a].v[0]*0.000975F);
			vTemp->tv = (obj->mesh->faceTC[v2a].v[1]*0.000975F);
			//vTemp->tv = (tN2->vy+0.5);
			//vTemp->tu = (tN2->vx+0.5);
			
				x1on = BETWEEN(v[0].sx,clx0,clx1);
			x2on = BETWEEN(v[1].sx,clx0,clx1);
			x3on = BETWEEN(v[2].sx,clx0,clx1);
			y1on = BETWEEN(v[0].sy,cly0,cly1);
			y2on = BETWEEN(v[1].sy,cly0,cly1);
			y3on = BETWEEN(v[2].sy,cly0,cly1);

			if ((x1on || x2on || x3on) && (y1on || y2on || y3on))
			{
				if ((x1on && x2on && x3on) && (y1on && y2on && y3on))
				{
					PushPolys(v,3,facesON,3,tex);

					// Push Lightmap polys - only for single pass cards.
					vTemp = v;

					vTemp->tv = (tN0->vy+0.5);
					vTemp->tu = (tN0->vx+0.5);
					vTemp->color = 0xffffffff;
					vTemp++;
					vTemp->tv = (tN1->vy+0.5);
					vTemp->tu = (tN1->vx+0.5);
					vTemp->color = 0xffffffff;
					vTemp++;
					vTemp->tv = (tN2->vy+0.5);
					vTemp->tu = (tN2->vx+0.5);
					vTemp->color = 0xffffffff;
					
					SaveFrame;
					SwapFrame(MA_FRAME_PHONG);
					PushPolys(v,3,facesON,3,phong);
					SwapFrame(MA_FRAME_LIGHTMAP);
					PushPolys(v,3,facesON,3,lightMap);
					RestoreFrame;
				}
				else
				{
					Clip3DPolygon(v,tex);
				}
			}
		}
		
		// Update our pointers
		facesIdx++;
		tex2++;
	}
}


void PCRenderModgyWaterObject (MDX_OBJECT *obj)
{
	long i,j;
	unsigned short fce[3] = {0,1,2};		
	MDX_QUATERNION *c1,*c2,*c3;
	D3DTLVERTEX v[3],*vTemp;
	MDX_SHORTVECTOR *facesIdx;
	unsigned long x1on,x2on,x3on,y1on,y2on,y3on;
	unsigned long v0,v1,v2;
	unsigned long v0a,v1a,v2a;
	float m1x,m2x,m3x;
	float m1z,m2z,m3z;
	long alphaVal;
	MDX_TEXENTRY *tex;
	MDX_TEXENTRY **tex2;
	MDX_VECTOR *tV0,*tV1,*tV2, *tN0,*tN1,*tN2;
	MDX_QUATERNION *cols;
	
	facesIdx = obj->mesh->faceIndex;
	tex2 = obj->mesh->textureIDs;
	cols = obj->mesh->gouraudColors;
	alphaVal = (long)(globalXLU2*255.0);

	for (j=0, i=0; i<obj->mesh->numFaces; i++, j+=3)
	{
		// Get information from the mesh!
		v0 = facesIdx->v[0];
		v1 = facesIdx->v[1];
		v2 = facesIdx->v[2];
		
		
		tV0 = &tV[v0];
		tV1 = &tV[v1];
		tV2 = &tV[v2];

		tN0 = &tN[v0];
		tN1 = &tN[v1];
		tN2 = &tN[v2];
		
		m1x = tMx[v0];
		m2x = tMx[v1];
		m3x = tMx[v2];
		m1z = tMz[v0];
		m2z = tMz[v1];
		m3z = tMz[v2];
		
		tex = (MDX_TEXENTRY *)(*tex2);
		
		// If we are to be drawn.
		if (((tV0->vz) && (tV1->vz) && (tV2->vz)) && (tex))
		{
			// Get rest of info from mesh
			v0a = j;
			v1a = j+1;
			v2a = j+2;

			c1 = &(cols[v0a]);
			c2 = &(cols[v1a]);
			c3 = &(cols[v2a]);
			
			// Fill out D3DVertices...
			vTemp = v;
				
			vTemp->sx = tV0->vx;
			vTemp->sy = tV0->vy;
			vTemp->sz = (tV0->vz) * 0.00025F;
			vTemp->rhw = 1/vTemp->sz;
			
			vTemp->color = MODALPHA(*((unsigned long *)(&(c1->x))),alphaVal);
			vTemp->specular = D3DRGBA(0,0,0,1);
			vTemp->tu = (obj->mesh->faceTC[v0a].v[0]*0.000975F) + m1x;
			vTemp->tv = (obj->mesh->faceTC[v0a].v[1]*0.000975F) + m1z;
			
			//vTemp->tv = (tN0->vy+0.5);
			//vTemp->tu = (tN0->vx+0.5);
			vTemp++;

			vTemp->sx = tV1->vx;
			vTemp->sy = tV1->vy;
			vTemp->sz = (tV1->vz) * 0.00025F;
			vTemp->rhw = 1/vTemp->sz;
			vTemp->color = MODALPHA(*((unsigned long *)(&(c2->x))),alphaVal);
			vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (obj->mesh->faceTC[v1a].v[0]*0.000975F) + m2x;
			vTemp->tv = (obj->mesh->faceTC[v1a].v[1]*0.000975F) + m2z;
			//vTemp->tv = (tN1->vy+0.5);
			//vTemp->tu = (tN1->vx+0.5);
			vTemp++;
			
			vTemp->sx = tV2->vx;
			vTemp->sy = tV2->vy;
			vTemp->sz = (tV2->vz) * 0.00025F;
			vTemp->rhw = 1/vTemp->sz;
			
			vTemp->color = MODALPHA(*((unsigned long *)(&(c3->x))),alphaVal);
			vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (obj->mesh->faceTC[v2a].v[0]*0.000975F) + m3x;
			vTemp->tv = (obj->mesh->faceTC[v2a].v[1]*0.000975F) + m3z;
			//vTemp->tv = (tN2->vy+0.5);
			//vTemp->tu = (tN2->vx+0.5);
			
			x1on = BETWEEN(v[0].sx,clx0,clx1);
			x2on = BETWEEN(v[1].sx,clx0,clx1);
			x3on = BETWEEN(v[2].sx,clx0,clx1);
			y1on = BETWEEN(v[0].sy,cly0,cly1);
			y2on = BETWEEN(v[1].sy,cly0,cly1);
			y3on = BETWEEN(v[2].sy,cly0,cly1);

			if ((x1on || x2on || x3on) && (y1on || y2on || y3on))
			{
				if ((x1on && x2on && x3on) && (y1on && y2on && y3on))
				{
					PushPolys(v,3,facesON,3,tex);
				}
				else
				{
					Clip3DPolygon(v,tex);
				}
			}
		}
		
		// Update our pointers
		facesIdx++;
		tex2++;
	}
}

void PCRenderModgyObject (MDX_OBJECT *obj)
{
	long i,j;
	unsigned short fce[3] = {0,1,2};		
	MDX_QUATERNION *c1,*c2,*c3;
	D3DTLVERTEX v[3],*vTemp;
	MDX_SHORTVECTOR *facesIdx;
	unsigned long x1on,x2on,x3on,y1on,y2on,y3on;
	unsigned long v0,v1,v2;
	unsigned long v0a,v1a,v2a;
	float m1x,m2x,m3x,tFog;
	float m1z,m2z,m3z,cVal;
	long alphaVal;
	MDX_TEXENTRY *tex;
	MDX_TEXENTRY **tex2;
	MDX_VECTOR *tV0,*tV1,*tV2, *tN0,*tN1,*tN2;
	MDX_QUATERNION *cols;
	
	facesIdx = obj->mesh->faceIndex;
	tex2 = obj->mesh->textureIDs;
	cols = obj->mesh->gouraudColors;
	alphaVal = (long)(globalXLU2*255.0);

	for (j=0, i=0; i<obj->mesh->numFaces; i++, j+=3)
	{
		// Get information from the mesh!
		v0 = facesIdx->v[0];
		v1 = facesIdx->v[1];
		v2 = facesIdx->v[2];
		
		
		tV0 = &tV[v0];
		tV1 = &tV[v1];
		tV2 = &tV[v2];

		tN0 = &tN[v0];
		tN1 = &tN[v1];
		tN2 = &tN[v2];
		
		m1x = tMx[v0];
		m2x = tMx[v1];
		m3x = tMx[v2];
		m1z = tMz[v0];
		m2z = tMz[v1];
		m3z = tMz[v2];

//		if (overrideTex)
//			tex = overrideTex;
//		else
			tex = (MDX_TEXENTRY *)(*tex2);
		
		// If we are to be drawn.
		if (((tV0->vz) && (tV1->vz) && (tV2->vz)) && (tex))
		{
			// Get rest of info from mesh
			v0a = j;
			v1a = j+1;
			v2a = j+2;

			c1 = &(cols[v0a]);
			c2 = &(cols[v1a]);
			c3 = &(cols[v2a]);
			
			// Fill out D3DVertices...
			vTemp = v;
			
			tFog = FOG(tV0->vz);
			if (tFog>1) tFog = 1;
			if (tFog<0) tFog = 0;
			vTemp->specular = FOGVAL(tFog);			
						
			vTemp->sx = tV0->vx;
			vTemp->sy = tV0->vy;
			vTemp->sz = (tV0->vz) * 0.00025F;
			vTemp->rhw = 1;///vTemp->sz;

			cVal = 1;//cVal = fabs((m1x+m1z)*2.5);		
			//vTemp->color = D3DRGBA(cVal,cVal,1,0.4+(m1x+m1z));
			vTemp->color = (*((unsigned long *)c1)&0x00FFFFFF)|(50+(unsigned long)((m1x+m1z)*400)<<24);

			//vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (obj->mesh->faceTC[v0a].v[0]*0.000975F) + m1x*0.4;
			vTemp->tv = (obj->mesh->faceTC[v0a].v[1]*0.000975F) + m1z*0.4;
			
			//vTemp->tv = (tN0->vy+0.5);
			//vTemp->tu = (tN0->vx+0.5);
			vTemp++;

			tFog = FOG(tV1->vz);
			if (tFog>1) tFog = 1;
			if (tFog<0) tFog = 0;
			vTemp->specular = FOGVAL(tFog);			
			vTemp->sx = tV1->vx;
			vTemp->sy = tV1->vy;
			vTemp->sz = (tV1->vz) * 0.00025F;
			vTemp->rhw = 1;///vTemp->sz;
			cVal = 1;//fabs((m2x+m2z)*2.5);		
			//vTemp->color = D3DRGBA(cVal,cVal,1,0.4+(m2x+m2z));
			vTemp->color = (*((unsigned long *)c2)&0x00FFFFFF)|(50+(unsigned long)((m2x+m2z)*400)<<24);
			vTemp->tu = (obj->mesh->faceTC[v1a].v[0]*0.000975F) + m2x*0.4;
			vTemp->tv = (obj->mesh->faceTC[v1a].v[1]*0.000975F) + m2z*0.4;
			//vTemp->tv = (tN1->vy+0.5);
			//vTemp->tu = (tN1->vx+0.5);
			vTemp++;
			
			tFog = FOG(tV2->vz);
			if (tFog>1) tFog = 1;
			if (tFog<0) tFog = 0;
			vTemp->specular = FOGVAL(tFog);			
			vTemp->sx = tV2->vx;
			vTemp->sy = tV2->vy;
			vTemp->sz = (tV2->vz) * 0.00025F;
			vTemp->rhw = 1;///vTemp->sz;
			cVal = 1;//cVal = fabs((m3x+m3z)*2.5);		
			//vTemp->color = D3DRGBA(cVal,cVal,1,0.4+(m3x+m3z));
			vTemp->color = (*((unsigned long *)c3)&0x00FFFFFF)|(50+(unsigned long)((m3x+m3z)*400)<<24);
			vTemp->tu = (obj->mesh->faceTC[v2a].v[0]*0.000975F) + m3x *0.4;
			vTemp->tv = (obj->mesh->faceTC[v2a].v[1]*0.000975F) + m3z *0.4;
			//vTemp->tv = (tN2->vy+0.5);
			//vTemp->tu = (tN2->vx+0.5);
			
			x1on = BETWEEN(v[0].sx,clx0,clx1);
			x2on = BETWEEN(v[1].sx,clx0,clx1);
			x3on = BETWEEN(v[2].sx,clx0,clx1);
			y1on = BETWEEN(v[0].sy,cly0,cly1);
			y2on = BETWEEN(v[1].sy,cly0,cly1);
			y3on = BETWEEN(v[2].sy,cly0,cly1);

			if ((x1on || x2on || x3on) && (y1on || y2on || y3on))
			{
				if ((x1on && x2on && x3on) && (y1on && y2on && y3on))
				{
					PushPolys(v,3,facesON,3,tex);
				}
				else
					Clip3DPolygon(v,tex);
			}
		}
		
		// Update our pointers
		facesIdx++;
		tex2++;
	}
}

MDX_TEXENTRY *sheenTex;
void PCRenderModgySheenObject (MDX_OBJECT *obj)
{
	long i,j;
	unsigned short fce[3] = {0,1,2};		
	MDX_QUATERNION *c1,*c2,*c3;
	D3DTLVERTEX v[3],*vTemp;
	MDX_SHORTVECTOR *facesIdx;
	unsigned long x1on,x2on,x3on,y1on,y2on,y3on;
	unsigned long v0,v1,v2;
	unsigned long v0a,v1a,v2a;
	float m1x,m2x,m3x;
	float m1z,m2z,m3z,cVal;
	long alphaVal;
	MDX_TEXENTRY *tex;
	MDX_TEXENTRY **tex2;
	MDX_VECTOR *tV0,*tV1,*tV2, *tN0,*tN1,*tN2;
	MDX_QUATERNION *cols;
	
	facesIdx = obj->mesh->faceIndex;
	tex2 = obj->mesh->textureIDs;
	cols = obj->mesh->gouraudColors;
	alphaVal = (long)(globalXLU2*255.0);

	for (j=0, i=0; i<obj->mesh->numFaces; i++, j+=3)
	{
		// Get information from the mesh!
		v0 = facesIdx->v[0];
		v1 = facesIdx->v[1];
		v2 = facesIdx->v[2];
		
		
		tV0 = &tV[v0];
		tV1 = &tV[v1];
		tV2 = &tV[v2];

		tN0 = &tN[v0];
		tN1 = &tN[v1];
		tN2 = &tN[v2];
		
		m1x = tMx[v0];
		m2x = tMx[v1];
		m3x = tMx[v2];
		m1z = tMz[v0];
		m2z = tMz[v1];
		m3z = tMz[v2];

//		if (overrideTex)
		tex = sheenTex;
//		else
//			tex = (MDX_TEXENTRY *)(*tex2);
		
		// If we are to be drawn.
		if (((tV0->vz) && (tV1->vz) && (tV2->vz)) && (tex))
		{
			// Get rest of info from mesh
			v0a = j;
			v1a = j+1;
			v2a = j+2;

			c1 = &(cols[v0a]);
			c2 = &(cols[v1a]);
			c3 = &(cols[v2a]);
			
			// Fill out D3DVertices...
			vTemp = v;
			
			vTemp->specular = D3DRGBA(0,0,0,1);									
			vTemp->sx = tV0->vx;
			vTemp->sy = tV0->vy;
			vTemp->sz = (tV0->vz) * 0.00025F;
			vTemp->rhw = 1;///vTemp->sz;

			cVal = 1;//cVal = fabs((m1x+m1z)*2.5);		
			vTemp->color = D3DRGBA(1,1,1,1);
			//vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (sheenTC[v0a].v[0]*0.000975F);
			vTemp->tv = (sheenTC[v0a].v[1]*0.000975F);
			
			//vTemp->tv = (tN0->vy+0.5);
			//vTemp->tu = (tN0->vx+0.5);
			vTemp++;

			vTemp->specular = D3DRGBA(0,0,0,1);			
			vTemp->sx = tV1->vx;
			vTemp->sy = tV1->vy;
			vTemp->sz = (tV1->vz) * 0.00025F;
			vTemp->rhw = 1;///vTemp->sz;
			cVal = 1;//fabs((m2x+m2z)*2.5);		
			vTemp->color = D3DRGBA(1,1,1,1);
			vTemp->tu = (sheenTC[v1a].v[0]*0.000975F);
			vTemp->tv = (sheenTC[v1a].v[1]*0.000975F);
			//vTemp->tv = (tN1->vy+0.5);
			//vTemp->tu = (tN1->vx+0.5);
			vTemp++;
			
			vTemp->specular = D3DRGBA(0,0,0,1);			
			vTemp->sx = tV2->vx;
			vTemp->sy = tV2->vy;
			vTemp->sz = (tV2->vz) * 0.00025F;
			vTemp->rhw = 1;///vTemp->sz;
			cVal = 1;//cVal = fabs((m3x+m3z)*2.5);		
			vTemp->color = D3DRGBA(1,1,1,1);
			vTemp->tu = (sheenTC[v2a].v[0]*0.000975F);
			vTemp->tv = (sheenTC[v2a].v[1]*0.000975F);
			//vTemp->tv = (tN2->vy+0.5);
			//vTemp->tu = (tN2->vx+0.5);
			
			x1on = BETWEEN(v[0].sx,clx0,clx1);
			x2on = BETWEEN(v[1].sx,clx0,clx1);
			x3on = BETWEEN(v[2].sx,clx0,clx1);
			y1on = BETWEEN(v[0].sy,cly0,cly1);
			y2on = BETWEEN(v[1].sy,cly0,cly1);
			y3on = BETWEEN(v[2].sy,cly0,cly1);

			if ((x1on || x2on || x3on) && (y1on || y2on || y3on))
			{
				if ((x1on && x2on && x3on) && (y1on && y2on && y3on))
				{
					PushPolys(v,3,facesON,3,tex);
				}
				else
					Clip3DPolygon(v,tex);
			}
		}
		
		// Update our pointers
		facesIdx++;
		tex2++;
	}
}

void PCRenderModgyObject2(MDX_OBJECT *obj)
{
	long i,j;
	unsigned short fce[3] = {0,1,2};		
	MDX_QUATERNION *c1,*c2,*c3;
	D3DTLVERTEX v[3],*vTemp;
	MDX_SHORTVECTOR *facesIdx;
	unsigned long x1on,x2on,x3on,y1on,y2on,y3on;
	unsigned long v0,v1,v2;
	unsigned long v0a,v1a,v2a;
	float m1x,m2x,m3x,tFog;
	float m1z,m2z,m3z,cVal;
	long alphaVal;
	MDX_TEXENTRY *tex;
	MDX_TEXENTRY **tex2;
	MDX_VECTOR *tV0,*tV1,*tV2, *tN0,*tN1,*tN2;
	MDX_QUATERNION *cols;
	
	facesIdx = obj->mesh->faceIndex;
	tex2 = obj->mesh->textureIDs;
	cols = obj->mesh->gouraudColors;
	alphaVal = (long)(globalXLU2*255.0);

	for (j=0, i=0; i<obj->mesh->numFaces; i++, j+=3)
	{
		// Get information from the mesh!
		v0 = facesIdx->v[0];
		v1 = facesIdx->v[1];
		v2 = facesIdx->v[2];
		
		
		tV0 = &tV[v0];
		tV1 = &tV[v1];
		tV2 = &tV[v2];

		tN0 = &tN[v0];
		tN1 = &tN[v1];
		tN2 = &tN[v2];
		
		m1x = tMx[v0];
		m2x = tMx[v1];
		m3x = tMx[v2];
		m1z = tMz[v0];
		m2z = tMz[v1];
		m3z = tMz[v2];
		
		tex = (MDX_TEXENTRY *)(*tex2);
		
		// If we are to be drawn.
		if (((tV0->vz) && (tV1->vz) && (tV2->vz)) && (tex))
		{
			// Get rest of info from mesh
			v0a = j;
			v1a = j+1;
			v2a = j+2;

			c1 = &(cols[v0a]);
			c2 = &(cols[v1a]);
			c3 = &(cols[v2a]);
			
			// Fill out D3DVertices...
			vTemp = v;
			
			tFog = FOG(tV0->vz);
			if (tFog>1) tFog = 1;
			if (tFog<0) tFog = 0;
			vTemp->specular = FOGVAL(tFog);			
						
			vTemp->sx = tV0->vx;
			vTemp->sy = tV0->vy;
			vTemp->sz = (tV0->vz) * 0.00025F;
			vTemp->rhw = 1;///vTemp->sz;

			cVal = 1;//cVal = fabs((m1x+m1z)*2.5);		
		//	vTemp->colo5r = D3DRGBA(cVal,cVal,1,0.2+(m1x+m1z)*2);
		//	vTemp->color = D3DRGBA(cVal,cVal,1,1);
			vTemp->color = MODALPHA(*((unsigned long *)(&(c1->x))),alphaVal);
			//vTemp->specular = D3DRGBA(0,0,0,0);
			vTemp->tu = (obj->mesh->faceTC[v0a].v[0]*0.000975F) + m1x*0.4;
			vTemp->tv = (obj->mesh->faceTC[v0a].v[1]*0.000975F) + m1z*0.4;
			
			//vTemp->tv = (tN0->vy+0.5);
			//vTemp->tu = (tN0->vx+0.5);
			vTemp++;

			tFog = FOG(tV1->vz);
			if (tFog>1) tFog = 1;
			if (tFog<0) tFog = 0;
			vTemp->specular = FOGVAL(tFog);			
			vTemp->sx = tV1->vx;
			vTemp->sy = tV1->vy;
			vTemp->sz = (tV1->vz) * 0.00025F;
			vTemp->rhw = 1;///vTemp->sz;
			cVal = 1;//fabs((m2x+m2z)*2.5);		
		//	vTemp->color = D3DRGBA(cVal,cVal,1,0.2+(m2x+m2z)*2);
		//	vTemp->color = D3DRGBA(cVal,cVal,1,1);
			vTemp->color = MODALPHA(*((unsigned long *)(&(c2->x))),alphaVal);
			vTemp->tu = (obj->mesh->faceTC[v1a].v[0]*0.000975F) + m2x*0.4;
			vTemp->tv = (obj->mesh->faceTC[v1a].v[1]*0.000975F) + m2z*0.4;
			//vTemp->tv = (tN1->vy+0.5);
			//vTemp->tu = (tN1->vx+0.5);
			vTemp++;
			
			tFog = FOG(tV2->vz);
			if (tFog>1) tFog = 1;
			if (tFog<0) tFog = 0;
			vTemp->specular = FOGVAL(tFog);			
			vTemp->sx = tV2->vx;
			vTemp->sy = tV2->vy;
			vTemp->sz = (tV2->vz) * 0.00025F;
			vTemp->rhw = 1;///vTemp->sz;
			cVal = 1;//cVal = fabs((m3x+m3z)*2.5);		
		//	vTemp->color = D3DRGBA(cVal,cVal,1,0.2+(m3x+m3z)*2);
		//Temp->color = D3DRGBA(cVal,cVal,1,1);
			vTemp->color = MODALPHA(*((unsigned long *)(&(c3->x))),alphaVal);
			vTemp->tu = (obj->mesh->faceTC[v2a].v[0]*0.000975F) + m3x *0.4;
			vTemp->tv = (obj->mesh->faceTC[v2a].v[1]*0.000975F) + m3z *0.4;
			//vTemp->tv = (tN2->vy+0.5);
			//vTemp->tu = (tN2->vx+0.5);
			
			x1on = BETWEEN(v[0].sx,clx0,clx1);
			x2on = BETWEEN(v[1].sx,clx0,clx1);
			x3on = BETWEEN(v[2].sx,clx0,clx1);
			y1on = BETWEEN(v[0].sy,cly0,cly1);
			y2on = BETWEEN(v[1].sy,cly0,cly1);
			y3on = BETWEEN(v[2].sy,cly0,cly1);

			if ((x1on || x2on || x3on) && (y1on || y2on || y3on))
			{
				if ((x1on && x2on && x3on) && (y1on && y2on && y3on))
				{
					PushPolys(v,3,facesON,3,tex);
				}
				else
					Clip3DPolygon(v,tex);
			}
		}
		
		// Update our pointers
		facesIdx++;
		tex2++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PCRenderObjectOutline (MDX_OBJECT *obj)
{
	long i;//,j;
	//unsigned short fce[3] = {0,1,2};		
	//MDX_QUATERNION *c1,*c2,*c3;
	//D3DTLVERTEX v[3],*vTemp;
	MDX_SHORTVECTOR *facesIdx;
	unsigned long x1on,y1on;
	//unsigned long v0,v1,v2;
//	unsigned long v0a,v1a,v2a;
	//long alphaVal;
	MDX_TEXENTRY *tex;
	MDX_TEXENTRY **tex2;
	MDX_VECTOR *tV0,*tV1,*tV2;
	D3DTLVERTEX *dVtx;
	float tFog;
	//MDX_QUATERNION *cols;
	
	dVtx = obj->mesh->d3dVtx;
	facesIdx = obj->mesh->faceIndex;
	tex2 = obj->mesh->textureIDs;
	//cols = obj->mesh->gouraudColors;
	//alphaVal = (long)(globalXLU2*255.0);

	tex = GetTexEntryFromCRC(UpdateCRC("blck.bmp"));
		
	for (i=obj->mesh->numFaces; i; i--)
	{
							
		tV0 = tV+*(((short *)facesIdx->v));
		tV1 = tV+*(((short *)facesIdx->v)+1);
		tV2 = tV+*(((short *)facesIdx->v)+2);

		
		// If we are to be drawn.
		if (((tV0->vz) && (tV1->vz) && (tV2->vz)) && (tex))
		{
			tFog = FOG(tV0->vz);
			if (tFog>1) tFog = 1;
			if (tFog<0) tFog = 0;
			(dVtx)->specular = FOGVAL(tFog);			
			(dVtx)->sx = tV0->vx;
			(dVtx)->sy = tV0->vy;
			(dVtx)->sz = tV0->vz * 0.00025F;
			
			tFog = FOG(tV1->vz);
			if (tFog>1) tFog = 1;
			if (tFog<0) tFog = 0;
			(dVtx+1)->specular = FOGVAL(tFog);			
			(dVtx+1)->sx = tV1->vx;
			(dVtx+1)->sy = tV1->vy;
			(dVtx+1)->sz = tV1->vz * 0.00025F;
			
			tFog = FOG(tV2->vz);
			if (tFog>1) tFog = 1;
			if (tFog<0) tFog = 0;
			(dVtx+2)->specular = FOGVAL(tFog);			
			(dVtx+2)->sx = tV2->vx;
			(dVtx+2)->sy = tV2->vy;
			(dVtx+2)->sz = tV2->vz * 0.00025F;
			
			y1on = BETWEEN(tV0->vy,cly0,cly1) +
				   BETWEEN(tV1->vy,cly0,cly1) +
				   BETWEEN(tV2->vy,cly0,cly1);
			
			if (y1on)
			{
				x1on = BETWEEN(tV0->vx,clx0,clx1) +
					   BETWEEN(tV1->vx,clx0,clx1) +
					   BETWEEN(tV2->vx,clx0,clx1);
				
				if (x1on)
				{
					if ((x1on+y1on==6))
					{
						PushPolys(dVtx,3,facesON,3,tex);
					}
					else
					{
						Clip3DPolygon(dVtx,tex);
					}
				}
			}
		}
		
		// Update our pointers
		facesIdx++;
		tex2++;
		dVtx+=3;
	}
}

long cFog;

void PCRenderObject (MDX_OBJECT *obj)
{
	long i;//,j;
	MDX_SHORTVECTOR *facesIdx;
	unsigned long x1on,y1on;
//	unsigned long v0a,v1a,v2a;
	MDX_TEXENTRY *tex;
	MDX_TEXENTRY **tex2;
	MDX_VECTOR *tV0,*tV1,*tV2;
	D3DTLVERTEX *dVtx;
	float tFog;
	
	dVtx = obj->mesh->d3dVtx;
	facesIdx = obj->mesh->faceIndex;
	tex2 = obj->mesh->textureIDs;

	if (overrideTex)
	{
		tex = overrideTex;
		
		for (i=obj->mesh->numFaces; i; i--)
		{
								
			tV0 = tV+*(((short *)facesIdx->v));
			tV1 = tV+*(((short *)facesIdx->v)+1);
			tV2 = tV+*(((short *)facesIdx->v)+2);

			
			// If we are to be drawn.
			if (((tV0->vz) && (tV1->vz) && (tV2->vz)) && (tex))
			{
				tFog = FOG(tV0->vz);
				if (tFog>1) tFog = 1;
				if (tFog<0) tFog = 0;
				(dVtx)->specular = FOGVAL(tFog);			
				(dVtx)->sx = tV0->vx;
				(dVtx)->sy = tV0->vy;
				(dVtx)->sz = tV0->vz * 0.00025F;
				
				tFog = FOG(tV1->vz);
				if (tFog>1) tFog = 1;
				if (tFog<0) tFog = 0;
				(dVtx+1)->specular = FOGVAL(tFog);			
				(dVtx+1)->sx = tV1->vx;
				(dVtx+1)->sy = tV1->vy;
				(dVtx+1)->sz = tV1->vz * 0.00025F;
				
				tFog = FOG(tV2->vz);
				if (tFog>1) tFog = 1;
				if (tFog<0) tFog = 0;
				(dVtx+2)->specular = FOGVAL(tFog);			
				(dVtx+2)->sx = tV2->vx;
				(dVtx+2)->sy = tV2->vy;
				(dVtx+2)->sz = tV2->vz * 0.00025F;
				
				y1on = BETWEEN(tV0->vy,cly0,cly1) +
					   BETWEEN(tV1->vy,cly0,cly1) +
					   BETWEEN(tV2->vy,cly0,cly1);
				
				if (y1on)
				{
					x1on = BETWEEN(tV0->vx,clx0,clx1) +
						   BETWEEN(tV1->vx,clx0,clx1) +
						   BETWEEN(tV2->vx,clx0,clx1);
					
					if (x1on)
					{
						if ((x1on+y1on==6))
						{
							PushPolys(dVtx,3,facesON,3,tex);
						}
						else
						{
							Clip3DPolygon(dVtx,tex);
						}
					}
				}
			}
			
			// Update our pointers
			facesIdx++;
			tex2++;
			dVtx+=3;
		}
	}
	else
	{
		if (obj->mesh->numFaces)
		{
			tV1 = tV+*(((short *)facesIdx->v));
			tFog = FOG(tV1->vz);
			if (tFog>1) tFog = 1;
			if (tFog<0) tFog = 0;
			cFog = FOGVAL(tFog);
		}
				
		for (i=obj->mesh->numFaces; i; i--)
		{
			tV0 = tV+*(((short *)facesIdx->v));
			tV1 = tV+*(((short *)facesIdx->v)+1);
			tV2 = tV+*(((short *)facesIdx->v)+2);
		
			tex = (MDX_TEXENTRY *)(*tex2);
			
			// If we are to be drawn.
			if (((tV0->vz) && (tV1->vz) && (tV2->vz)) && (tex))
			{
		
#ifndef TRUE_OBJECT_FOG
				(dVtx)->specular = cFog;			
#else
				tFog = FOG(tV0->vz);
				if (tFog>1) tFog = 1;
				if (tFog<0) tFog = 0;
				(dVtx)->specular = FOGVAL(tFog);
#endif				
				(dVtx)->sx = tV0->vx;
				(dVtx)->sy = tV0->vy;
				(dVtx)->sz = tV0->vz * 0.00025F;
				
#ifndef TRUE_OBJECT_FOG
				(dVtx+1)->specular = cFog;			
#else
				tFog = FOG(tV1->vz);
				if (tFog>1) tFog = 1;
				if (tFog<0) tFog = 0;
				(dVtx+1)->specular = FOGVAL(tFog);
#endif
				(dVtx+1)->sx = tV1->vx;
				(dVtx+1)->sy = tV1->vy;
				(dVtx+1)->sz = tV1->vz * 0.00025F;

				
#ifndef TRUE_OBJECT_FOG
				(dVtx+2)->specular = cFog;			
#else
				tFog = FOG(tV2->vz);
				if (tFog>1) tFog = 1;
				if (tFog<0) tFog = 0;
				(dVtx+2)->specular = FOGVAL(tFog);
#endif	
				(dVtx+2)->sx = tV2->vx;
				(dVtx+2)->sy = tV2->vy;
				(dVtx+2)->sz = tV2->vz * 0.00025F;
		
				y1on = BETWEEN(tV0->vy,cly0,cly1) +
					   BETWEEN(tV1->vy,cly0,cly1) +
					   BETWEEN(tV2->vy,cly0,cly1);
				
				if (y1on)
				{
					x1on = BETWEEN(tV0->vx,clx0,clx1) +
						   BETWEEN(tV1->vx,clx0,clx1) +
						   BETWEEN(tV2->vx,clx0,clx1);
					
					if (x1on)
					{
						if ((x1on+y1on==6))
							{PushPolys(dVtx,3,facesON,3,tex);}
						else
							{Clip3DPolygon(dVtx,tex);}
					}
				}
		
			}
			
			// Update our pointers
			facesIdx++;
			tex2++;
			dVtx+=3;
		}
	}
}

#ifdef __cplusplus
}
#endif
