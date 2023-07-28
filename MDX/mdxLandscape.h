/*

	This file is part of the M libraries,

	File		: 
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef MDXLANDSCAPE_H_INCLUDED
#define MDXLANDSCAPE_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct TAG_MDX_LANDSCAPE
{
	short *faceIndex;
	long  *clipFlags;	
	
	MDX_VECTOR *vertices;
	D3DTLVERTEX *xfmVert;	
	LPDIRECTDRAWSURFACE7 *textures;
	MDX_TEXENTRY **tEntrys;

	MDX_VECTOR bBox[8];
	MDX_MATRIX objMatrix;
	
	unsigned long numVertices;
	unsigned long numFaces;
	char name[64];

	struct TAG_MDX_LANDSCAPE *next,*children;
} MDX_LANDSCAPE;

MDX_LANDSCAPE *ConvertActorToLandscape(MDX_ACTOR *actor);
void FreeLandscape(MDX_LANDSCAPE **me);

#ifdef __cplusplus
}
#endif

#endif