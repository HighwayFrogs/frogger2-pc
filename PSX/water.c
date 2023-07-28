#include "sonylibs.h"
#include <islutil.h>
#include <isltex.h>
#include "shell.h"
#include "types.h"

#include "Main.h"
#include "Map_Draw.h"
#include "Water.h"

WATERLIST waterList;

void CreateAndAddWaterObject ( SCENIC *water )
{
	WATER *newItem;

	newItem = ( WATER* ) MALLOC0 ( sizeof ( WATER ) );

	newItem->flags = 0;

	AddWater ( newItem );

	QuatToPSXMatrix ( &water->rot, &newItem->matrix );

	newItem->matrix.t[0] =  water->pos.vx;
	newItem->matrix.t[1] =  water->pos.vy;
	newItem->matrix.t[2] =  water->pos.vz;

//	newItem->flags = newFlags;
	
	utilPrintf("Creating water object : %s\n", water->name);
//	utilUpperStr ( name );

	newItem->fma_water = ( void * ) BFF_FindObject ( BFF_FMA_WORLD_ID, utilStr2CRC ( water->name ) );

	if ( newItem->fma_water )
	{
		SetUpWaterMesh ( newItem->fma_water );
	}
	else
	{
		utilPrintf("Could Not Create Water.\n");
	}
	// ENDIF

}


//void DrawWaterList ( void )
//{
//	WATER *cur, *next;

//	if ( waterList.numEntries == 0 )
//		return;

//	for ( cur = waterList.head.next; cur != &waterList.head; cur = next)
//	{
//		next = cur->next;

//		if ( cur->fma_water )
//		{
//			MapDraw_DrawFMA_Water ( cur );
//		}
//		// ENDIF	    
//	}
	// ENDFOR

//}


void InitWaterLinkedList ( void )
{
	waterList.numEntries = 0;
	waterList.head.next = waterList.head.prev = &waterList.head;
}


void AddWater ( WATER *waterObj )
{
	if ( waterObj->next == NULL )
	{
		waterList.numEntries++;
		waterObj->prev						= &waterList.head;
		waterObj->next						= waterList.head.next;
		waterList.head.next->prev = waterObj;
		waterList.head.next				=	waterObj;
	}
}

void FreeWaterObjectList ( void )
{
	WATER *cur,*next;

	if( !waterList.numEntries )
		return;

	for(cur = waterList.head.next; cur != &waterList.head; cur = next)
	{
		next = cur->next;
		SubWaterObject(cur);
	}
//	FREE(specFXList.head);
}


void SubWaterObject ( WATER *waterObj )
{
	if(waterObj->next == NULL)
		return;

	waterObj->prev->next = waterObj->next;
	waterObj->next->prev = waterObj->prev;
	waterObj->next = NULL;
	waterList.numEntries--;

	FREE(waterObj);
}
