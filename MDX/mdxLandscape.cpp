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
#include "mgeReport.h"
#include "mdxLandscape.h"
#include "mdxPoly.h"
#include "gelf.h"
#include "mdxException.h"

#ifdef __cplusplus
extern "C"
{
#endif

MDX_LANDSCAPE *ConvertDataToLandscape(short *data, unsigned long xs, unsigned long ys, float scaleFlat, float scaleVert, LPDIRECTDRAWSURFACE7 tex)
{
	MDX_LANDSCAPE *me = (MDX_LANDSCAPE *) AllocMem(sizeof(MDX_LANDSCAPE));
	unsigned long numVerts = xs * ys;
	unsigned long numFaces = ((xs-1) * (ys-1))*2;
	unsigned long i,j;
	me->vertices = (MDX_VECTOR *) AllocMem(sizeof(MDX_VECTOR)*numVerts);
	me->textures = (LPDIRECTDRAWSURFACE7 *) AllocMem(sizeof(LPDIRECTDRAWSURFACE7)*numFaces);
	
	me->faceIndex = (short *) AllocMem(sizeof(short)*numFaces);
	me->xfmVert = (D3DTLVERTEX *) AllocMem(sizeof(D3DTLVERTEX)*numFaces*3);
		
	for (i=0; i<numFaces; i++)
		me->textures[i] = tex;

	for (i=0; i<xs; i++)
		for (j=0; j<ys; j++)
		{
			unsigned long pos = i+j*xs;
			me->vertices[pos].vx = (i * scaleFlat) / xs;
			me->vertices[pos].vz = (j * scaleFlat) / ys;
			me->vertices[pos].vy = ((data[i+j*xs] & 0x001f) * scaleVert) / 0x1f;			
		}

	me->next = NULL;
	me->children = NULL;
	return me;	
}

MDX_LANDSCAPE *ConvertObjectToLandscape(MDX_OBJECT *obj)
{
	MDX_LANDSCAPE *me = (MDX_LANDSCAPE *) AllocMem(sizeof(MDX_LANDSCAPE));
	unsigned long i;

	me->vertices = 0;
	me->faceIndex = 0;	
	me->xfmVert = 0;
	me->textures = 0;
	me->tEntrys = 0;

	strcpy(me->name,(const char *)obj->name);

	// Copy the vertices and the faces
	if (me->numVertices = obj->mesh->numVertices)
	{
		me->vertices = (MDX_VECTOR *) AllocMem(sizeof(MDX_VECTOR)*me->numVertices);
			
		for (int i=0; i<me->numVertices; i++)
		{
			guMtxXFMF(obj->objMatrix.matrix,obj->mesh->vertices[i].vx,obj->mesh->vertices[i].vy,obj->mesh->vertices[i].vz,
									&me->vertices[i].vx,&me->vertices[i].vy,&me->vertices[i].vz);
		}
	}

	memcpy (&me->objMatrix,&obj->objMatrix,sizeof(MDX_MATRIX));
	
	if (me->numFaces = obj->mesh->numFaces)
	{
		short *tFace;
		me->faceIndex = (short *) AllocMem(sizeof(short)*me->numFaces * 3);
		//me->clipFlags = new long[me->numFaces * 3];
		me->xfmVert = (D3DTLVERTEX *) AllocMem(sizeof(D3DTLVERTEX) * me->numFaces * 3);
		me->textures = (LPDIRECTDRAWSURFACE7 *) AllocMem(sizeof(LPDIRECTDRAWSURFACE7)*me->numFaces);
		me->tEntrys = (MDX_TEXENTRY **) AllocMem (sizeof(MDX_TEXENTRY *)*me->numFaces * 3);

		memcpy (me->faceIndex,obj->mesh->faceIndex,sizeof(short)*me->numFaces*3);

		tFace = me->faceIndex;

		for (i=0; i<obj->mesh->numFaces*3; i++)
		{
			me->xfmVert[i].tu = obj->mesh->faceTC[i].v[0] * 0.000975F;
			me->xfmVert[i].tv = obj->mesh->faceTC[i].v[1] * 0.000975F;
			
			me->xfmVert[i].rhw = 1;
			me->xfmVert[i].specular = D3DRGBA(0,0,0,0);
			me->xfmVert[i].color = (*((long *)(&(obj->mesh->gouraudColors[i].x))));// &0x00ffffff) | D3DRGBA(0,0,0,0.5);

		//	me->xfmVert[i].color = (*((long *)(&(obj->mesh->gouraudColors[i].x))) &0x00ffffff) | D3DRGBA(0,0,0,0.2);

			if (obj->mesh->textureIDs[i/3])
			{
				me->textures[i/3] = obj->mesh->textureIDs[i/3]->surf;
				me->tEntrys[i/3] = obj->mesh->textureIDs[i/3];				
			}
			else
			{
				me->textures[i/3] = NULL;
				me->tEntrys[i/3] = NULL;
			}
			tFace++;
		}
	}
	
	// Copy the untransformed bounding box for culling purposes
	if (obj->bBox)
		memcpy (&me->bBox,obj->bBox,sizeof(MDX_VECTOR)*8);
	
    me->next = me->children = NULL;		
	if (obj->next)
		me->next = ConvertObjectToLandscape(obj->next);

	if (obj->children)
		me->children = ConvertObjectToLandscape(obj->children);

	return me;
}

MDX_LANDSCAPE *ConvertActorToLandscape(MDX_ACTOR *actor)
{
	// Calculate the object matrices for each sub object (We'll pre-transform them)
	XformActor(actor,1);
	return ConvertObjectToLandscape(actor->objectController->object);
}


void FreeLandscape(MDX_LANDSCAPE **me)
{
	if (!*me)
		return;
	
	MDX_LANDSCAPE *next = (*me)->next;
	MDX_LANDSCAPE *children = (*me)->children;
	
	if ((*me)->vertices)
		FreeMem ((*me)->vertices);
	//if ((*me)->clipFlags)
	//	delete (*me)->clipFlags;
	if ((*me)->faceIndex)
		FreeMem ((*me)->faceIndex);
	if ((*me)->xfmVert)
		FreeMem ((*me)->xfmVert);
	if ((*me)->textures)
		FreeMem ((*me)->textures);
	if ((*me)->tEntrys)
		FreeMem ((*me)->tEntrys);

	FreeMem (*me);
	*me = NULL;

	FreeLandscape(&next);
	FreeLandscape(&children);
}

#ifdef __cplusplus
}
#endif
