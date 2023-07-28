#include <string.h>
#include <islutil.h>
//#include "global.h"
#include "map_draw.h"
#include "world.h"

BFF_Header* level_bff = NULL;
FMA_WORLD* fma_world = NULL;


void DrawWorld()
{
	if(fma_world)
	{
//bbopt - doesn't do much here
//		char* oldStackPointer;
//		oldStackPointer = SetSp(0x1f800400);
		MapDraw_DrawFMA_World(fma_world);
//		SetSp(oldStackPointer);
	}
}
