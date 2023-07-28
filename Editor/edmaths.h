#ifndef __EDMATHS_H
#define __EDMATHS_H

#include "edittypes.h"
#include "define.h"
#include "math.h"

#define GAMEFLOAT(f)	(fixed)((f)*4096)
#define EDFLOAT(x)		(float)((x)*(1.0f/4096.0f))

#define DotProduct(v1,v2)	((((v1)->vx)*((v2)->vx)) + (((v1)->vy)*((v2)->vy)) + (((v1)->vz)*((v2)->vz)))
#define MagnitudeSquared(vect) ((vect)->vx*(vect)->vx + (vect)->vy*(vect)->vy + (vect)->vz*(vect)->vz)
#define EdMagnitude(vect) ((float)sqrt((vect)->vx*(vect)->vx + (vect)->vy*(vect)->vy + (vect)->vz*(vect)->vz))
#define AddVector(res,v1,v2) {(res)->vx = (v1)->vx + (v2)->vx;(res)->vy = (v1)->vy + (v2)->vy;(res)->vz = (v1)->vz + (v2)->vz;}
#define AddToVector(res,v1) {(res)->vx += (v1)->vx;(res)->vy += (v1)->vy;(res)->vz += (v1)->vz;}
#define SubFromVector(res,v1) {(res)->vx -= (v1)->vx;(res)->vy -= (v1)->vy;(res)->vz -= (v1)->vz;}
#define SubVector(res,v1,v2) {(res)->vx = (v1)->vx - (v2)->vx;(res)->vy = (v1)->vy - (v2)->vy;(res)->vz = (v1)->vz - (v2)->vz;}

#define AddGToVector(res,v1) {(res)->vx += (v1)->v;(res)->vy += (v1)->vy;(res)->vz += (v1)->vz;}

//#define ScaleVector(v1,scale) {(v1)->v[X] *= scale;(v1)->v[Y] *= scale;(v1)->v[Z] *= scale;}
//#define ZeroVector(vect) (vect)->v[X] = (vect)->v[Y] = (vect)->v[Z] = 0

#define SetVector(v1,v2)	(*(v1)) = (*(v2))
#define SetVectorF(v,fv)	((v)->vx=(float)(fv)->vx*(1.0f/4096.0f),(v)->vy=(float)(fv)->vy*(1.0f/4096.0f),(v)->vz=(float)(fv)->vz*(1.0f/4096.0f))
#define SetVectorS(v,v2)	((v)->vx=(float)(v2)->vx,(v)->vy=(float)(v2)->vy,(v)->vz=(float)(v2)->vz)
#define FSetVector(v,v2)	((v)->vx=(fixed)((v2)->vx*4096)),(v)->vy=(fixed)((v2)->vy*4096),(v)->vz=((fixed)(v2)->vz*4096))
#define SSetVector(v,v2)	(v)->vx=(short)((v2)->vx),(v)->vy=(short)((v2)->vy),(v)->vz=(short)((v2)->vz)


#ifdef __cplusplus
extern "C" {
#endif

void GetTilePos(EDVECTOR *v, GAMETILE *tile);

float DistanceBetweenPointsSquared(EDVECTOR *v1, EDVECTOR *v2);
void EdMakeUnit(EDVECTOR *v);

#ifdef __cplusplus
}
#endif

#endif

