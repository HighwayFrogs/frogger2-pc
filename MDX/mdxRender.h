/*

	This file is part of the M libraries,

	File		: 
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef MDXRENDER_H_INCLUDED
#define MDXRENDER_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif
extern MDX_TEXENTRY *phong;
extern MDX_TEXENTRY *lightMap;
extern float globalXLU;
extern float globalXLU2;
extern MDX_MATRIX vMatrix;
extern unsigned long drawingSwampy;
extern unsigned long numObjDrawn;
extern float clx0, cly0, clx1, cly1;
extern long DIST, FOV;
extern float farClip, nearClip, horizClip, vertClip;
extern MDX_TEXENTRY *overrideTex;
extern float fogStart;
extern float fogRange;

extern long noClip;

extern float halfWidth;
extern float halfHeight;
extern float oneOver[];

#define BIAS 127
#define fftol(v) (((((*((long *)v))&0x007fffff)<<8)|0x80000000)>>((BIAS-(((*((long *)v))&0x7f800000)>>23))+31))


void InitOneOverTable(void);
void InitDOFOneOverTable(int min, int max, int range);
void SwapOneOver(long num);
void DrawObject(MDX_OBJECT *obj, int skinned, MDX_MESH *masterMesh);
void DrawLandscape(MDX_LANDSCAPE *me);
void DrawLandscape2(MDX_LANDSCAPE *me);
void DrawLandscape3(MDX_LANDSCAPE *me);
void SetupDOF(long min, long max, float range);

void PCRenderObject (MDX_OBJECT *obj);
void PCRenderModgyObject (MDX_OBJECT *obj);
void PCPrepareModgySheenObject (MDX_OBJECT *obj, MDX_MESH *me, float m[4][4]);
void PCRenderModgyObject2(MDX_OBJECT *obj);
void PCRenderObjectPhong (MDX_OBJECT *obj);

void PCRenderModgySheenObject (MDX_OBJECT *obj);
void PCPrepareModgyObject (MDX_OBJECT *obj, MDX_MESH *me, float m[4][4]);
void PCPrepareObject (MDX_OBJECT *obj, MDX_MESH *me, float m[4][4]);
void PCPrepareSkinnedObjectOutline(MDX_OBJECT *obj, MDX_MESH *mesh, float m[4][4]);
void PCCalcModgeValues(MDX_OBJECT *obj);
void XfmPoint(MDX_VECTOR *vTemp2,MDX_VECTOR *in,MDX_MATRIX *d);
void SetupRenderer(long xRes, long yRes);
void PCRenderObjectOutline (MDX_OBJECT *obj);
void AdjustObjectOutline(void);

void __fastcall Clip3DPolygon (D3DTLVERTEX in[3], MDX_TEXENTRY *tEntry);
extern unsigned long noClipping;
extern MDX_TEXENTRY *sheenTex;
extern MDX_VECTOR sheenEye;
extern unsigned long wrapCoords;

#ifdef __cplusplus
}
#endif

#endif