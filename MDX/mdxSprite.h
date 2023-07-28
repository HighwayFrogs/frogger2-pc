
#ifndef MDXSPRITE_H_INCLUDED
#define MDXSPRITE_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

extern unsigned short spIndices[];

void DrawAlphaSprite (float x, float y, float z, float xs, float ys, float u1, float v1, float u2, float v2, MDX_TEXENTRY *tex, DWORD colour );
void DrawAlphaSpriteRotating(MDX_VECTOR *pos,float angle,float x, float y, float z, float xs, float ys, float u1, float v1, float u2, float v2, MDX_TEXENTRY *tex, DWORD colour );

void DrawSpriteOverlayRotating( float x, float y, float z, float xs, float ys, float u1, float v1, float u2, float v2, float angle, MDX_TEXENTRY *tex, DWORD colour );

#ifdef __cplusplus
}
#endif

#endif