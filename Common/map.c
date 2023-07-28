 /*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: map.c
	Programmer	: James Healey
	Date		: 18/06/99

----------------------------------------------------------------------------------------------- */


#include <islmem.h>
#include <islfile.h>
#include <stdio.h>
#include <islutil.h>

#include "Define.h"

#include "LayOut.h"
#include "MemLoad.h"
#include "NewPsx.h"

#include "map.h"
#include "cam.h"

#include "maths.h"
#include "flatpack.h"

#ifdef DREAMCAST_VERSION
#include "main.h"
#endif

int numBabies = 0;
int tileCount = 0;
int scenicCount = 0;

void *collisionFile = NULL;
void *entityFile = NULL;
void *scenicFile = NULL;

GAMETILE	*gTStart[4];
GAMETILE	*bTStart[5];

GAMETILE	*firstTile;
SCENIC		*firstScenic;


void LoadScenics ( int collBank )
{
	int c;
	int fileLength;
	unsigned char* p;
	int *ptrAddr;
	char file[64];
	SCENIC *cur;

#ifdef PSX_VERSION
	strcpy( file, "" );
#endif

#ifdef PC_VERSION
	strcpy( file, "SCENICS\\" );
#endif

	switch ( collBank )
	{
		case GARDENLEV1_COL:
			strcat(file,"GARDEN1.SCE");
			break;
		case GARDENLEV2_COL:
			strcat(file,"GARDEN2.SCE");
			break;
		case GARDENMULTI_COL:
			strcat(file,"GARDENMULTI.SCE");
			break;

		case ANCIENTMASTER_COL:
			strcat(file,"ANCIENTSBOSS.SCE");
			break;
		case ANCIENTLEV1_COL:
			strcat(file,"ANCIENTS1.SCE");
			break;
		case ANCIENTLEV2_COL:
			strcat(file,"ANCIENTS2.SCE");
			break;
		case ANCIENTLEV3_COL:
			strcat(file,"ANCIENTS3.SCE");
			break;
		case ANCIENTMULTI_COL:	
			strcat(file,"ANCIENTSMULTI.SCE");
			break;

		case SPACEMASTER_COL:
			strcat(file,"SPACEBOSS.SCE");
			break;
		case SPACEMASTERA_COL:
			strcat(file,"SPACEBOSSA.SCE");
			break;
		case SPACELEV1_COL:
			strcat(file,"SPACE1.SCE");
			break;
		case SPACELEV2_COL:
			strcat(file,"SPACE2.SCE");
			break;
		case SPACELEV3_COL:
			strcat(file,"SPACE3.SCE");
			break;
		case SPACEBONUS_COL:
			strcat(file,"SPACEBONUS.SCE");
			break;
		case SPACEMULTI_COL:
			strcat(file,"SPACEMULTI.SCE");
			break;

		case CITYMASTER_COL:
			strcat(file,"CITYBOSS.SCE");
			break;
		case CITYLEV1_COL:
			strcat(file,"CITY1.SCE");
			break;
		case CITYLEV2_COL:
			strcat(file,"CITY2.SCE");
			break;
		case CITYLEV3_COL:
			strcat(file,"CITY3.SCE");
			break;
		case CITYBONUS_COL:
			strcat(file,"CITYBONUS.SCE");
			break;
		case CITYMULTI_COL:
			strcat(file,"CITYMULTI.SCE");
			break;

		case SUBTERRANEANMASTER_COL:
			strcat(file,"SUBBOSS.SCE");
			break;
		case SUBTERRANEANLEV1_COL:
			strcat(file,"SUB1.SCE");
			break;
		case SUBTERRANEANLEV2_COL:
			strcat(file,"SUB2.SCE");
			break;
		case SUBTERRANEANLEV3_COL:
			strcat(file,"SUB3.SCE");
			break;
		case SUBTERRANEANBONUS_COL:
			strcat(file,"SUBBONUS.SCE");
			break;
		case SUBTERRANEANMULTI_COL:
			strcat(file,"SUBMULTI.SCE");
			break;

		case LABORATORYMASTERA_COL:
			strcat(file,"LABBOSSA.SCE");
			break;
		case LABORATORYMASTERB_COL:
			strcat(file,"LABBOSSB.SCE");
			break;
		case LABORATORYMASTERC_COL:
			strcat(file,"LABBOSSC.SCE");
			break;
		case LABORATORYLEV1_COL:
			strcat(file,"LAB1.SCE");
			break;
		case LABORATORYLEV2_COL:
			strcat(file,"LAB2.SCE");
			break;
		case LABORATORYLEV3_COL:
			strcat(file,"LAB3.SCE");
			break;
		case LABORATORYBONUS_COL:
			strcat(file,"LABBONUS.SCE");
			break;
		case LABORATORYMULTI1_COL:
			strcat(file,"LABMULTI1.SCE");
			break;
		case LABORATORYMULTI2_COL:
			strcat(file,"LABMULTI2.SCE");
			break;
		case LABORATORYMULTI3_COL:
			strcat(file,"LABMULTI3.SCE");
			break;

		case HALLOWEENMASTER_COL:
			strcat(file,"HALLOWEENBOSS.SCE");
			break;
		case HALLOWEENMASTERB_COL:
			strcat(file,"HALLOWEENBOSSB.SCE");
			break;
		case HALLOWEENLEV1_COL:
			strcat(file,"HALLOWEEN1.SCE");
			break;
		case HALLOWEENLEV2_COL:
			strcat(file,"HALLOWEEN2.SCE");
			break;
		case HALLOWEENLEV3_COL:
			strcat(file,"HALLOWEEN3.SCE");
			break;
		case HALLOWEENBONUS_COL:
			strcat(file,"HALLOWEENBONUS.SCE");
			break;
		case HALLOWEENMULTI_COL:
			strcat(file,"HALLOWEENMULTI.SCE");
			break;

//		case SWAMPYWORLDMASTER_COL:
//			strcat(file,"HALLOWEENBOSS.SCE");
//			break;
//		case SWAMPYWORLDMASTERB_COL:
//			strcat(file,"HALLOWEENBOSSB.SCE");
//			break;
//		case SWAMPYWORLDLEV1_COL:
//			strcat(file,"HALLOWEEN1.SCE");
//			break;
//		case SWAMPYWORLDLEV2_COL:
//			strcat(file,"HALLOWEEN2.SCE");
//			break;
//		case SWAMPYWORLDLEV3_COL:
//			strcat(file,"HALLOWEEN3.SCE");
//			break;
//		case SWAMPYWORLDBONUS_COL:
//			strcat(file,"HALLOWEENBONUS.SCE");
//			break;
//		case SWAMPYWORLDMULTI_COL:
//			strcat(file,"HALLOWEENMULTI.SCE");
//			break;

		case RETROLEV1_COL:
			strcat(file,"SRETRO1.SCE");
			break;
		case RETROLEV2_COL:
			strcat(file,"SRETRO2.SCE");
			break;
		case RETROLEV3_COL:
			strcat(file,"SRETRO3.SCE");
			break;
		case RETROLEV4_COL:
			strcat(file,"SRETRO4.SCE");
			break;
		case RETROLEV5_COL:
			strcat(file,"SRETRO5.SCE");
			break;
		case RETROLEV6_COL:
			strcat(file,"SRETRO6.SCE");
			break;
		case RETROLEV7_COL:
			strcat(file,"SRETRO7.SCE");
			break;
		case RETROLEV8_COL:
			strcat(file,"SRETRO8.SCE");
			break;
		case RETROLEV9_COL:
			strcat(file,"SRETRO9.SCE");
			break;
		case RETROLEV10_COL:
			strcat(file,"SRETRO10.SCE");
			break;
		case RETROMASTER_COL:
			strcat(file,"SRETROBOSS.SCE");
			break;
		case RETROMULTI1_COL:
			strcat(file,"SRETROMULTI1.SCE");
			break;
		case RETROMULTI2_COL:
			strcat(file,"SRETROMULTI2.SCE");
			break;
		case RETROMULTI3_COL:
			strcat(file,"SRETROMULTI3.SCE");
			break;
		case RETROMULTI4_COL:
			strcat(file,"SRETROMULTI4.SCE");
			break;
		case RETROMULTI5_COL:
			strcat(file,"SRETROMULTI5.SCE");
			break;
		case RETROMULTI6_COL:
			strcat(file,"SRETROMULTI6.SCE");
			break;
		case RETROMULTI7_COL:
			strcat(file,"SRETROMULTI7.SCE");
			break;
		case RETROMULTI8_COL:
			strcat(file,"SRETROMULTI8.SCE");
			break;
		case RETROMULTI9_COL:
			strcat(file,"SRETROMULTI9.SCE");
			break;
		case RETROMULTI10_COL:
			strcat(file,"SRETROMULTI10.SCE");
			break;
		
		case FRONTEND1_COL:
			strcat(file,"START.SCE");
			break;
		case FRONTEND2_COL:
			strcat(file,"LEVELA.SCE");
			break;
		case FRONTEND3_COL:
			strcat(file,"LEVELB.SCE");
			break;
		case FRONTEND4_COL:
			strcat(file,"MULTISEL.SCE");
			break;
		case FRONTEND5_COL:
			strcat(file,"LANGUAGE.SCE");
			break;
		case TEST1_COL:	strcat(file,"TEST1.SCE"); break;
		case TEST2_COL:	strcat(file,"TEST2.SCE"); break;
		case TEST3_COL:	strcat(file,"TEST3.SCE"); break;
		case TEST4_COL:	strcat(file,"TEST4.SCE"); break;
		case TEST5_COL:	strcat(file,"TEST5.SCE"); break;
		case TEST6_COL:	strcat(file,"TEST6.SCE"); break;
	}

#ifdef PSX_VERSION
	if(scenicFile)
		FREE(scenicFile);

	scenicFile = FindStakFileInAllBanks ( file, &fileLength );
#endif


	//scenicFile = getFileFromStack ( stakFile, file, &fileLength);

#ifdef PC_VERSION
	scenicFile = fileLoad(file, &fileLength);
#endif

	p = (unsigned char*)scenicFile;

	// Get a pointer to the addr and get scenic count from it...
	ptrAddr			= ( int* )p;
	scenicCount = ( int ) *ptrAddr;

	// Move on 4 bytes so that we can get the number of babies.
	p += 4;
	c = 0;

	firstScenic = ( SCENIC* )p;// scenicFile;
	cur = firstScenic;

	for ( c = 0; cur; c++, cur = cur->next )
	{
		if ( ( int ) cur->next != -1 )
			cur->next = ( SCENIC* ) ( ( int ) cur->next + ( int ) firstScenic );
		else
				// If not a valid pointer then set the pointer to NULL
			cur->next = 0;
		
		cur->pos.vx *= SCALE;
		cur->pos.vy *= SCALE;
		cur->pos.vz *= SCALE;

//		utilPrintf("%s, %d, %d, %d, %d\n", cur->name, cur->pos.vx, cur->pos.vy, cur->pos.vz, scenicCount);
	}
}

void LoadLevelEntities ( int worldID, int levelID )
{
	int fileLength;
	char fileName[256];

#ifdef DREAMCAST_VERSION
	sprintf ( fileName, "MAPS\\ENTITY_%d_%d.dat", worldID, levelID );
#endif

#ifdef PSX_VERSION
	sprintf ( fileName, "ENTITY-%d-%d.DAT", worldID, levelID );
#endif

#ifdef PC_VERSION
	sprintf ( fileName, "MAPS\\ENTITY-%d-%d.DAT", worldID, levelID );
#endif

#ifdef PSX_VERSION
	if(entityFile)
		FREE(entityFile);

	entityFile = FindStakFileInAllBanks ( fileName, &fileLength );

#endif


	//entityFile = getFileFromStack ( stakFile, fileName, &fileLength);

#ifdef PC_VERSION
	entityFile = ( void * ) fileLoad ( fileName, &fileLength );
#endif

	if ( !entityFile )
		return;

	if ( !MemLoadEntities ( entityFile ) )
	{
		utilPrintf ( "MemLoadEntities failed\n" );
	}

	//bb
	if(entityFile)
		FREE(entityFile);
}

void LoadMapBank ( int collBank )
{
//	DBPRINTF(("Loading Map Banks, Collision And Scenics.\n"));

	LoadCollision ( collBank );
	LoadScenics		( collBank );
}

void SetupCollisionTiles(void **buffer)
{
	GAMETILE *cur;
	int babStart[5];	// Temp holders for the baby game tiles....
	int c, d, pl;
	int *ptrAddr;
	unsigned char *p;

	p = (unsigned char*)(*buffer);

	// Get a pointer to the addr and get tile count from it...
	ptrAddr		= ( int* ) p;//firstTile;
	tileCount = ( int ) *ptrAddr;
	p += 4;

	// read player start tiles
	for (pl=0; pl<4; pl++)
	{
		gTStart[pl] = *(GAMETILE**)p;
		p += 4;
	}

	// Get a pointer to the addr and get the number of babies.....
	ptrAddr		= ( int* ) p;//firstTile;
	numBabies	= ( int ) *ptrAddr;
	p += 4;

	for ( c = 0; c < numBabies; c++ )
	{
		// Get a pointer to the addr and get the game tile.....
		babStart[c] = *(int*)p;

		// Move on 4 bytes so that we can get the next baby game tile....
		p += 4;

	}
	// ENDFOR - for ( c = 0; c < numBabies; c++ )
	
	firstTile = (GAMETILE*)p;
	// These need to be done last, so that we get the correct firstTile offset in the file.....

	//  Set the offsets for the memory pointers in memory for the baby game tiles....
	for ( c = 0; c < numBabies; c++ )
	{
		bTStart [c] = &firstTile[(int)babStart[c]];
	}

	//  And lets do the same for the frogger start tiles
	for (pl=0; pl<4; pl++)
		gTStart[pl] = &firstTile[(int)gTStart[pl]];


	c = 0;
	cur = firstTile;

	for ( c = 0; c < tileCount; c++, cur++ )
	{
		// If the memory pointer is not -1 then set them up so that they offset into the correct place in the tile list...

		for (d=0; d<4; d++)
		{
			if ((int)cur->tilePtrs[d] != -1)
				cur->tilePtrs[d] = &firstTile[(int)cur->tilePtrs[d]];
			else
				cur->tilePtrs[d] = 0;
		}
		
		//if (c == (tileCount-1))
		//	cur->next = 0;
		//else
			//cur->next = &firstTile[c+1];
		
		ScaleVector(&cur->centre, SCALE);
	}

	*buffer = cur;
}

void SetupCameraBoxes(void **buffer)
{
	int i = 0;
	CAM_PLANE* plane;
	CAM_BOX* box;
	char* p = (char*)(*buffer);	// pointer to the first thing after the thing, kind of thing

	cameraBoxes.numPlanes = *(int*)p;
	p += 4;


	cameraBoxes.planes = plane = (CAM_PLANE*)p;
	for (i=cameraBoxes.numPlanes; i; plane++,i--)
	{
		plane->k *= 10;	// scale k value..
	}
	p = (char*)plane;
/*
	cameraBoxes.planes = (CAM_PLANE*)p;
	p += (cameraBoxes.numPlanes*sizeof(CAM_PLANE));
*/
	cameraBoxes.numBoxes = *(int*)p;
	p += 4;

	cameraBoxes.boxes = box = (CAM_BOX*)p;
	for (i=cameraBoxes.numBoxes; i; box++,i--)
	{
		box->planes = &cameraBoxes.planes[(int)box->planes];
	}
	p = (char*)box;

	*buffer = p;
}

void LoadCollision ( int collBank )
{
	char file[64];
	void *p;
	int fileLength;

#ifdef PSX_VERSION
	strcpy( file, "" );
#endif

#ifdef PC_VERSION
	strcpy( file, "COLLISION\\" );
#endif

	switch ( collBank )
	{
		case GARDENLEV1_COL:
			strcat(file,"GARDEN1.COL");
			break;
		case GARDENLEV2_COL:
			strcat(file,"GARDEN2.COL");
			break;
		case GARDENMULTI_COL:
			strcat(file,"GARDENMULTI.COL");
			break;

		case ANCIENTMASTER_COL:
			strcat(file,"ANCIENTSBOSS.COL");
			break;
		case ANCIENTLEV1_COL:
			strcat(file,"ANCIENTS1.COL");
			break;
		case ANCIENTLEV2_COL:
			strcat(file,"ANCIENTS2.COL");
			break;
		case ANCIENTLEV3_COL:
			strcat(file,"ANCIENTS3.COL");
			break;
		case ANCIENTMULTI_COL:	
			strcat(file,"ANCIENTSMULTI.COL");
			break;

		case SPACEMASTER_COL:
			strcat(file,"SPACEBOSS.COL");
			break;
		case SPACEMASTERA_COL:
			strcat(file,"SPACEBOSSA.COL");
			break;
		case SPACELEV1_COL:
			strcat(file,"SPACE1.COL");
			break;
		case SPACELEV2_COL:
			strcat(file,"SPACE2.COL");
			break;
		case SPACELEV3_COL:
			strcat(file,"SPACE3.COL");
			break;
		case SPACEBONUS_COL:
			strcat(file,"SPACEBONUS.COL");
			break;
		case SPACEMULTI_COL:
			strcat(file,"SPACEMULTI.COL");
			break;

		case CITYMASTER_COL:
			strcat(file,"CITYBOSS.COL");
			break;
		case CITYLEV1_COL:
			strcat(file,"CITY1.COL");
			break;
		case CITYLEV2_COL:
			strcat(file,"CITY2.COL");
			break;
		case CITYLEV3_COL:
			strcat(file,"CITY3.COL");
			break;
		case CITYBONUS_COL:
			strcat(file,"CITYBONUS.COL");
			break;
		case CITYMULTI_COL:
			strcat(file,"CITYMULTI.COL");
			break;

		case SUBTERRANEANMASTER_COL:
			strcat(file,"SUBBOSS.COL");
			break;
		case SUBTERRANEANMASTERB_COL:
			strcat(file,"SUBTEST.COL");		// <--- interesting, new, fragrant
			break;
		case SUBTERRANEANLEV1_COL:
			strcat(file,"SUB1.COL");
			break;
		case SUBTERRANEANLEV2_COL:
			strcat(file,"SUB2.COL");
			break;
		case SUBTERRANEANLEV3_COL:
			strcat(file,"SUB3.COL");
			break;
		case SUBTERRANEANBONUS_COL:
			strcat(file,"SUBBONUS.COL");
			break;
		case SUBTERRANEANMULTI_COL:
			strcat(file,"SUBMULTI.COL");
			break;

		case LABORATORYMASTERA_COL:
			strcat(file,"LABBOSSA.COL");
			break;
		case LABORATORYMASTERB_COL:
			strcat(file,"LABBOSSB.COL");
			break;
		case LABORATORYMASTERC_COL:
			strcat(file,"LABBOSSC.COL");
			break;
		case LABORATORYLEV1_COL:
			strcat(file,"LAB1.COL");
			break;
		case LABORATORYLEV2_COL:
			strcat(file,"LAB2.COL");
			break;
		case LABORATORYLEV3_COL:
			strcat(file,"LAB3.COL");
			break;
		case LABORATORYBONUS_COL:
			strcat(file,"LABBONUS.COL");
			break;
		case LABORATORYMULTI1_COL:
			strcat(file,"LABMULTI1.COL");
			break;
		case LABORATORYMULTI2_COL:
			strcat(file,"LABMULTI2.COL");
			break;
		case LABORATORYMULTI3_COL:
			strcat(file,"LABMULTI3.COL");
			break;

		case HALLOWEENMASTER_COL:
			strcat(file,"HALLOWEENBOSS.COL");
			break;
		case HALLOWEENMASTERB_COL:
			strcat(file,"HALLOWEENBOSSB.COL");
			break;
		case HALLOWEENLEV1_COL:
			strcat(file,"HALLOWEEN1.COL");
			break;
		case HALLOWEENLEV2_COL:
			strcat(file,"HALLOWEEN2.COL");
			break;
		case HALLOWEENLEV3_COL:
			strcat(file,"HALLOWEEN3.COL");
			break;
		case HALLOWEENBONUS_COL:
			strcat(file,"HALLOWEENBONUS.COL");
			break;
		case HALLOWEENMULTI_COL:
			strcat(file,"HALLOWEENMULTI.COL");
			break;

//		case SWAMPYWORLDMASTER_COL:
//			strcat(file,"HALLOWEENBOSS.COL");
//			break;
//		case SWAMPYWORLDMASTERB_COL:
//			strcat(file,"HALLOWEENBOSSB.COL");
//			break;
//		case SWAMPYWORLDLEV1_COL:
//			strcat(file,"HALLOWEEN1.COL");
//			break;
//		case SWAMPYWORLDLEV2_COL:
//			strcat(file,"HALLOWEEN2.COL");
//			break;
//		case SWAMPYWORLDLEV3_COL:
//			strcat(file,"HALLOWEEN3.COL");
//			break;
//		case SWAMPYWORLDBONUS_COL:
//			strcat(file,"HALLOWEENBONUS.COL");
//			break;
//		case SWAMPYWORLDMULTI_COL:
//			strcat(file,"HALLOWEENMULTI.COL");
//			break;

		case RETROLEV1_COL:
			strcat(file,"SRETRO1.COL");
			break;
		case RETROLEV2_COL:
			strcat(file,"SRETRO2.COL");
			break;
		case RETROLEV3_COL:
			strcat(file,"SRETRO3.COL");
			break;
		case RETROLEV4_COL:
			strcat(file,"SRETRO4.COL");
			break;
		case RETROLEV5_COL:
			strcat(file,"SRETRO5.COL");
			break;
		case RETROLEV6_COL:
			strcat(file,"SRETRO6.COL");
			break;
		case RETROLEV7_COL:
			strcat(file,"SRETRO7.COL");
			break;
		case RETROLEV8_COL:
			strcat(file,"SRETRO8.COL");
			break;
		case RETROLEV9_COL:
			strcat(file,"SRETRO9.COL");
			break;
		case RETROLEV10_COL:
			strcat(file,"SRETRO10.COL");
			break;
		case RETROMASTER_COL:
			strcat(file,"SRETROBOSS.COL");
			break;
		case RETROMULTI1_COL:
			strcat(file,"SRETROMULTI1.COL");
			break;
		case RETROMULTI2_COL:
			strcat(file,"SRETROMULTI2.COL");
			break;
		case RETROMULTI3_COL:
			strcat(file,"SRETROMULTI3.COL");
			break;
		case RETROMULTI4_COL:
			strcat(file,"SRETROMULTI4.COL");
			break;
		case RETROMULTI5_COL:
			strcat(file,"SRETROMULTI5.COL");
			break;
		case RETROMULTI6_COL:
			strcat(file,"SRETROMULTI6.COL");
			break;
		case RETROMULTI7_COL:
			strcat(file,"SRETROMULTI7.COL");
			break;
		case RETROMULTI8_COL:
			strcat(file,"SRETROMULTI8.COL");
			break;
		case RETROMULTI9_COL:
			strcat(file,"SRETROMULTI9.COL");
			break;
		case RETROMULTI10_COL:
			strcat(file,"SRETROMULTI10.COL");
			break;

		case FRONTEND1_COL:
			strcat(file,"START.COL");
			break;
		case FRONTEND2_COL:
			strcat(file,"LEVELA.COL");
			break;
		case FRONTEND3_COL:
			strcat(file,"LEVELB.COL");
			break;
		case FRONTEND4_COL:
			strcat(file,"MULTISEL.COL");
			break;
		case FRONTEND5_COL:
			strcat(file,"LANGUAGE.COL");
			break;

		case TEST1_COL:	strcat(file,"TEST1.COL"); break;
		case TEST2_COL:	strcat(file,"TEST2.COL"); break;
		case TEST3_COL:	strcat(file,"TEST3.COL"); break;
		case TEST4_COL:	strcat(file,"TEST4.COL"); break;
		case TEST5_COL:	strcat(file,"TEST5.COL"); break;
		case TEST6_COL:	strcat(file,"TEST6.COL"); break;
	}

#ifdef PSX_VERSION
	if(collisionFile)
		FREE(collisionFile);

	collisionFile = FindStakFileInAllBanks ( file, &fileLength );

#endif

	//collisionFile = getFileFromStack ( stakFile, file, &fileLength);

#ifdef PC_VERSION
	collisionFile = ( void* ) fileLoad ( file, &fileLength );
#endif

	p = collisionFile;

	SetupCollisionTiles(&p);

	// avoid trying to read camera boxes if we've already read everything
	if (((int)p-(int)collisionFile) < fileLength)		
		SetupCameraBoxes(&p);
}

void FreeEntityBank ( void )
{
	//utilPrintf("Freeing Entity Bank : MAP\n");
	
	if (entityFile)
	{
		FREE ( ( void* ) entityFile );
		entityFile=NULL;
	}
}


void FreeMap ( void )
{
	//utilPrintf("Freeing Map Bank : MAP\n");
	if (collisionFile)
	{
		FREE ( ( void* ) collisionFile );
		collisionFile = NULL;
		
		memset(&cameraBoxes, 0, sizeof(CAM_BOX_LIST));
	}
}

void FreeScenic ( void )
{
	//utilPrintf("Freeing Scenic Bank : MAP\n");
	if (scenicFile)
	{
		FREE ( ( void* ) scenicFile );
		scenicFile = NULL;
	}
}
