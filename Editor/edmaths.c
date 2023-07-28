#include "define.h"
#include "edmaths.h"
#include "maths.h"

void GetTilePos(EDVECTOR *v, GAMETILE *tile)
{
	v->vx = tile->centre.vx * 0.1f;
	v->vy = tile->centre.vy * 0.1f;
	v->vz = tile->centre.vz * 0.1f;
}

void EdMakeUnit(EDVECTOR *v)
{
	float s = 1.0f/(float)sqrt(v->vx*v->vx+v->vy*v->vy+v->vz*v->vz);
	ScaleVector(v, s);
}

float DistanceBetweenPointsSquared(EDVECTOR *v1, EDVECTOR *v2)
{
	return (v1->vx-v2->vx)*(v1->vx-v2->vx)+(v1->vy-v2->vy)*(v1->vy-v2->vy)+(v1->vz-v2->vz)*(v1->vz-v2->vz);
}
