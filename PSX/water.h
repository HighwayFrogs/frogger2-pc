#ifndef WATER_H_INCLUDED
#define WATER_H_INCLUDED

typedef struct _WATER
{
	struct _WATER *next,*prev;			// ptr to next / prev platform

	int						flags;
	FMA_WORLD*		fma_water;
	SVECTOR				position;
	MATRIX			  matrix;
} WATER;

typedef struct _WATERLIST
{
	int			numEntries;
	WATER		head;
} WATERLIST;

extern WATERLIST waterList;


void CreateAndAddWaterObject ( SCENIC *water );

void InitWaterLinkedList ( void );

void AddWater ( WATER *waterObj );

void SubWaterObject ( WATER *waterObj );

void FreeWaterObjectList ( void );

#endif
