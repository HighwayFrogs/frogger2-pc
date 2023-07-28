//#include "directx.h"

//#include <stdio.h>
#include <islutil.h>

#include "ultra64.h"
#include "memload.h"
#include "evenfunc.h"
#include "script.h"

//#include "jalloc.h"
#include "path.h"
#include "enemies.h"
#include "babyfrog.h"
#include "platform.h"
#include "collect.h"
#include "Cam.h"
//#include "newstuff.h"
#include "newpsx.h"
#include <islmem.h>
#include "Main.h"
#include "maths.h"
#include "game.h"

#ifdef PC_VERSION
#include <pcaudio.h>
#else
#include "audio.h"
#include "psiactor.h"
#endif

/*	-------------------------------------------------------------------------------- */

#ifdef PC_VERSION
#define MEMLOAD_PRINT_ENTITIES
#endif

#define MEMLOAD_ENTITY_VERSION 16
#define MEMLOAD_SCRIPT_VERSION 2

typedef enum { CREATE_ENEMY, CREATE_PLATFORM, CREATE_GARIB, CREATE_CAMERACASE, CREATE_PLACEHOLDER } CREATETYPE;

int memload_fast = 0;

#ifdef PC_VERSION
#define NUM_SORTHACKED_OBJECTS 18
#define SORTHACK_OFFSET -50

char *sortHackNames[] = 
{
	"ada_info", "lillies", "gator", "turtle2", "turtle3", "plt4log", "plt5log",
	"barup", "baruptwo", "barupthree", "beetle", "roach", "louse", "pltpump", "pltlilly",
	"coff2n", "coff3n", "coff4n",
};
#endif

/*	-------------------------------------------------------------------------------- */

#define MEMGETBYTE(p) (*((*p)++))

int MemLoadInt(UBYTE **p)		// get a little-endian integer
{
	unsigned int i;

	i = (unsigned int)*((*p)++);
	i += ((unsigned int)*((*p)++) << 8);
	i += ((unsigned int)*((*p)++) << 16);
	i += ((unsigned int)*((*p)++) << 24);

	return i;
}

short MemLoadWord(UBYTE **p)		// get a little-endian word
{
	unsigned short i;

	i = (unsigned int)*((*p)++);
	i += ((unsigned int)*((*p)++) << 8);

	return (short)i;
}

char *MemLoadString(UBYTE **p)
{
	char *ptr;
	int size;
	
	size = MEMGETBYTE(p);
	ptr = (char*)MALLOC0(size + 1);
	memcpy(ptr, *p, size);
	ptr[size] = 0;

	(*p) += size;

	return ptr;
}

/*	--------------------------------------------------------------------------------
    Function		: MemLoadEntities
	Parameters		: void*, long
	Returns			: int

	Load enemies, platforms etc.
*/

short createExtraLife;
int MemLoadEntities(const void* data)
{
	UBYTE thing;
	UBYTE *p = (UBYTE*)data;
	PATH *pathList;
	int n, count;
	int totalPathNodes, totalEnemies, totalPlatforms, totalGaribs, totalCameraCases, totalPlaceholders;

	createExtraLife = 1;
	// Version check - only load files with the current version
	n = MEMGETBYTE(&p);

	if (n & 0x40)
	{
		memload_fast = 1;
		n &= ~0x40;
		utilPrintf("MemLoadEntities() --- SUPERFAST VERSION ---\n");
	}
	else
	{
		memload_fast = 0;
		utilPrintf("MemLoadEntities() ---------------------\n");
//		while(1);
	}

	if (n != MEMLOAD_ENTITY_VERSION ) {
		utilPrintf(
			"--------------------------------------------------------------------------------------\n"
			"ERROR: Attempting to load incorrect version (%03d) of level file (should be %03d)\n"
			"Update levels by loading into the editor and saving out again!\n"
			"--------------------------------------------------------------------------------------\n",
			n, MEMLOAD_ENTITY_VERSION);
		return 0;
	}

	totalPathNodes = MEMGETWORD(&p);
	AllocatePathList(totalPathNodes);

	count = MEMGETWORD(&p);

	utilPrintf("Loading %d paths..\n", count);

	pathList = (PATH*)MALLOC0(count*sizeof(PATH));

	for (n=0; n<count; n++)
	{
		PATH *path;
		PATHNODE *node;
		VECTOR v;
		int numNodes;

		numNodes = MEMGETINT(&p);

		path = &pathList[n];
		path->nodes = node = GetPathNodes(numNodes);
		path->numNodes = numNodes;

		while (numNodes--)
		{
			if (memload_fast)
			{
				int index = MEMGETINT(&p);
				node->worldTile = &firstTile[index];
			}
			else
			{
				v.vx = MEMGETINT(&p)>>16;
				v.vy = MEMGETINT(&p)>>16;
				v.vz = MEMGETINT(&p)>>16;
 				v.vx *=	SCALE;
 				v.vy *=	SCALE;
 				v.vz *= SCALE;
				node->worldTile = FindNearestTileV(v);
			}
			node->offset = MEMGETFIXED(&p) *SCALE;
			node->offset2 = MEMGETFIXED(&p) *SCALE;
			node->speed = MEMGETFIXED(&p);//mm *SCALE?

			node->waitTime = MEMGETINT(&p);
			node->sample = FindSample((unsigned long)MEMGETINT(&p));
			node++;
		}
	}

	totalEnemies = MEMGETWORD(&p);
	totalPlatforms = MEMGETWORD(&p);
	totalGaribs = MEMGETWORD(&p);
	totalCameraCases = MEMGETWORD(&p);
	totalPlaceholders = MEMGETWORD(&p);

	AllocNmeBlock( totalEnemies+totalPlaceholders );
	AllocPlatformBlock( totalPlatforms );
	AllocCamBlock( totalCameraCases );
	AllocGaribBlock( totalGaribs );

	count = MEMGETWORD(&p);

#ifdef DEBUG_MEMLOAD
	utilPrintf("MemLoadEntities: %d items\n", count);
#endif

	while (count--)
	{
		thing = MEMGETBYTE(&p);

		switch (thing)
		{
		case CREATE_ENEMY:
		case CREATE_PLATFORM:
		case CREATE_PLACEHOLDER:
			{
				signed char type[32], depthShift, dffClipping;
//bb
//				int count, flags, numNodes, startNode, n, ID, effects, pathIndex;
				unsigned char facing;
				int flags, startNode, n, ID, effects, pathIndex, objFlags, j;
				fixed scale, radius, animSpeed, value1;
				PATH *path;
				ENEMY *enemy;
				PLATFORM *platform;
				ACTOR2 *act = NULL;

				n = MEMGETBYTE(&p);
				memcpy(type, p, n); type[n] = 0; p+= n;

				flags	= MEMGETINT(&p);
				ID		= MEMGETINT(&p);
				scale	= MEMGETFIXED(&p);
				radius	= MEMGETFIXED(&p)*10;
				animSpeed = MEMGETINT(&p)>>8;
				value1 = MEMGETFIXED(&p);

				facing = MEMGETBYTE(&p);
				objFlags = MEMGETINT(&p);

				//4 psx char flags
				depthShift = MEMGETBYTE(&p);
				dffClipping = MEMGETBYTE(&p);
				p += 2;	// PSX shift, dffClipping, & 2 empty bytes

				effects = MEMGETINT(&p);

//mike frig to get rid of butterfly trails!?!?

				if((gstrcmp(type,"bfly.obe") == 0)&&(effects&4))
					effects &= ~4;

				startNode = MEMGETINT(&p);

				pathIndex = MEMGETWORD(&p);
				path = (PATH*)MALLOC0(sizeof(PATH));
				memcpy(path, &pathList[pathIndex], sizeof(PATH));

				path->startNode = startNode;

				switch (thing)
				{
				case CREATE_ENEMY:
#ifdef MEMLOAD_PRINT_ENTITIES
					utilPrintf("Enemy: %s (ID %d)..", type, ID);
#endif

#ifdef DREAMCAST_VERSION
					if(strstr(type,"froglet"))
					{
						for(i=0;i<numBabies;i++)
						{
							if(!babyList[i].baby)
							{
								sprintf(type,"froglet0%d.psi",i+1);
								break;
							}
						}
					}
#endif
					enemy = CreateAndAddEnemy(type,flags,ID,path,animSpeed,facing);
					if (!enemy)
						utilPrintf("failed!\n");
					else
						act = enemy->nmeActor;
					break;

				case CREATE_PLACEHOLDER:
#ifdef MEMLOAD_PRINT_ENTITIES
						utilPrintf("Placeholder ID %d..", ID);
#endif
						enemy = &enemyList.array[enemyList.count];
						enemy->path = path;
						enemy->active = 0;
						enemy->uid = ID;
						enemy->nmeActor = act = NULL;
						AddEnemy(enemy);
					break;

				case CREATE_PLATFORM:
					if (gstrcmp(type,"blank.obe") != 0)
					{
#ifdef MEMLOAD_PRINT_ENTITIES
						utilPrintf("Platform: %s (ID %d)..", type, ID);
#endif

						platform = CreateAndAddPlatform(type,flags,ID,path,animSpeed,facing);
						if (!platform)
							utilPrintf("failed!\n");
						else
							act = platform->pltActor;

					}
					break;
				}

				if (act)
				{
					if( thing == CREATE_ENEMY )
					{
						if( !(enemy->flags & ENEMY_NEW_BABYFROG) )
							act->effects = effects;
						else
							scale = FMul( scale, BABY_SCALE );
					}

					act->radius = radius;

 					act->actor->size.vx = scale;
					act->actor->size.vy = scale;
 					act->actor->size.vz = scale;

					act->animSpeed = animSpeed;
					act->value1 = value1;

					act->depthShift = depthShift;
					act->dffClipping = dffClipping;

#ifdef PC_VERSION
					if( !rHardware )
					{
						char name[16], *c = type, *d = name;
						while( *c != '.' ) *d++ = *c++;
						*d = '\0';

						if( player[0].worldNum == WORLDID_SUPERRETRO )
						{
							act->depthShift += SORTHACK_OFFSET/5;
						}
						else
						{
							for( j=0; j<NUM_SORTHACKED_OBJECTS; j++ )
							{
								if( !(stricmp(name,sortHackNames[j])) )
								{
									act->depthShift += SORTHACK_OFFSET;
									break;
								}
							}
						}
					}
#endif
				}

#ifdef MEMLOAD_PRINT_ENTITIES
				utilPrintf("ok\n");
#endif
				break;
			}

		case CREATE_GARIB:
			{
				SVECTOR v;
				FVECTOR w;
				GAMETILE *tile;
				fixed offs;

				n = MEMGETBYTE(&p);

				if (memload_fast)
				{
					int index = MEMGETINT(&p);
					tile = &firstTile[index];
				}
				else
				{
					v.vx = MEMGETINT(&p)>>16;
					v.vy = MEMGETINT(&p)>>16;
					v.vz = MEMGETINT(&p)>>16;

   					v.vx *=	SCALE;
   					v.vy *=	SCALE;
   					v.vz *= SCALE;

					tile = FindNearestTileS(v);
				}

				offs = MEMGETFIXED(&p);

				SetVectorFF(&w, &tile->normal);
//bb
//				ScaleVector(&w, offs);
				ScaleVectorFF(&w, offs * SCALE );
				AddVectorSSF(&v, &tile->centre, &w);

				if((n != EXTRALIFE_GARIB) || (gameState.difficulty == DIFFICULTY_EASY) || (createExtraLife))
					CreateNewGarib(v, n);
				if(n == EXTRALIFE_GARIB)
					createExtraLife = 1 - createExtraLife;
				break;
			}

		case CREATE_CAMERACASE:
			{
				TRANSCAMERA *tcam;
				GAMETILE *tile;
				SVECTOR v,w;
				int flags, numNodes, speed, target;
				fixed FOV;
				
				flags = MEMGETINT(&p);
 				v.vx = MEMGETINT(&p)>>16;
 				v.vy = MEMGETINT(&p)>>16;
 				v.vz = MEMGETINT(&p)>>16;
				v.vx *=	SCALE;
				v.vy *=	SCALE;
				v.vz *= SCALE;
						
				FOV = MEMGETFIXED(&p);
				speed = MEMGETINT(&p)>>16;

				target = MEMGETINT(&p);

				numNodes = MEMGETINT(&p);
				while (numNodes--)
				{
// 					w.vx = MEMGETFLOAT(&p)*SCALE;
// 					w.vy = MEMGETFLOAT(&p)*SCALE;
// 					w.vz = MEMGETFLOAT(&p)*SCALE;
					if (memload_fast)
					{
						int index = MEMGETINT(&p);
						tile = &firstTile[index];
					}
					else
					{
						w.vx = MEMGETINT(&p)>>16;
						w.vy = MEMGETINT(&p)>>16;
						w.vz = MEMGETINT(&p)>>16;

   						w.vx *=	SCALE;
   						w.vy *=	SCALE;
   						w.vz *= SCALE;

						tile = FindNearestTileS(w);
					}

					if( (tcam = CreateAndAddTransCamera(tile, (unsigned char)(flags >> 16), &v, (unsigned char)(flags & 0xFF))) )
					{
						tcam->FOV = FOV;
						tcam->speed = ToFixed(speed);
						tcam->camLookAt.vx =  ((int)((signed char*)&target)[0]) << 4;
						tcam->camLookAt.vy =  ((int)((signed char*)&target)[1]) << 4;
						tcam->camLookAt.vz =  ((int)((signed char*)&target)[2]) << 4;
						MakeUnit(&tcam->camLookAt);
						ScaleVector(&tcam->camLookAt,1000);
					}
				}
				break;
			}
		}
	 }

	FREE(pathList);

	utilPrintf("Success!\n----------------------------------------------------\n");

	return 1;
}

/*	--------------------------------------------------------------------------------
    Function		: MemLoadEvents
	Parameters		: void*, long
	Returns			: int

	Load events and triggers from an ETC output file
*/

int MemLoadEvents(const void* data, long size)
{
	int ver;//, s;
	UBYTE *p = (UBYTE*)data;

	if (*p != MEMLOAD_SCRIPT_VERSION) {
		ver = *p;
		utilPrintf("ERROR: Attempting to load incorrect version (%03d) of script (should be %03d)\n",
			ver, MEMLOAD_SCRIPT_VERSION);
		return 0;
	}

	return (Interpret(p + 1));
}

