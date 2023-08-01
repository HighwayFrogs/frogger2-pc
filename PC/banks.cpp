/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: banks.h
	Programmer	: David Swift
	Date		: 21/3/00

----------------------------------------------------------------------------------------------- */

#include <windows.h>
#include "banks.h"
#include "mdx.h"
#include "layout.h"
#include "Main.h"
#include "actor2.h"
#include "game.h"
#include "define.h"
#include <islmem.h>
#include <islutil.h>
#include <stdio.h>
#include <islfile.h>
#include "savegame.h"

char saveName[32];

int compressedTexBanks = 1;

TextureBankType *textureBanks [ MAX_TEXTURE_BANKS ];

void LoadObjectBank(int objectBank)					
{
	char fileName[MAX_PATH];

	switch ( objectBank )
	{
		case INGAMEGENERIC_OBJ_BANK:
			strcpy(fileName, "gamegeneric"); break;

// Singleplayer object banks
		case FROGGER_OBJ_BANK:
			strcpy(fileName, "frogger"); break;

		case LILLI_OBJ_BANK:
			strcpy(fileName, "lilli"); break;
			
		case TAD_OBJ_BANK:
			strcpy(fileName, "tad"); break;

		case SWAMPY_OBJ_BANK:
			strcpy(fileName, "swampy"); break;

		case TWEE_OBJ_BANK:
			strcpy(fileName, "twee"); break;

		case WART_OBJ_BANK:
			strcpy(fileName, "wart"); break;

		case ROOBIE_OBJ_BANK:
			strcpy(fileName, "roobie"); break;

		case TANK_OBJ_BANK:
			strcpy(fileName, "tank"); break;

// Small multiplayer obj banks
		case MFROGGER_OBJ_BANK:
			strcpy(fileName, "mfrogger"); break;

		case MLILLI_OBJ_BANK:
			strcpy(fileName, "mlilli"); break;
			
		case MTAD_OBJ_BANK:
			strcpy(fileName, "mtad"); break;

		case MSWAMPY_OBJ_BANK:
			strcpy(fileName, "mswampy"); break;

		case MTWEE_OBJ_BANK:
			strcpy(fileName, "mtwee"); break;

		case MWART_OBJ_BANK:
			strcpy(fileName, "mwart"); break;

		case MROOBIE_OBJ_BANK:
			strcpy(fileName, "mroobie"); break;

		case MTANK_OBJ_BANK:
			strcpy(fileName, "mtank"); break;

// Garden
		case GARDENMASTER_OBJ_BANK:
			strcpy(fileName, "gardenmaster");	break;

		case GARDENLEV1_OBJ_BANK:
			strcpy(fileName, "garden1"); break;

		case GARDENLEV2_OBJ_BANK:
			strcpy(fileName, "garden1"); break;

		case GARDENMULTI_OBJ_BANK:
			strcpy(fileName, "gardenmulti"); break;

// Ancient

		case ANCIENTMASTER_OBJ_BANK:
			strcpy(fileName, "ancientsmaster"); break;

		case ANCIENTLEV1_OBJ_BANK:				
			strcpy(fileName, "ancients1"); break;

		case ANCIENTLEV2_OBJ_BANK:				
			strcpy(fileName, "ancients2"); break;

		case ANCIENTLEV3_OBJ_BANK:				
			strcpy(fileName, "ancients3"); break;

		case ANCIENTBOSS_OBJ_BANK:				
			strcpy(fileName, "ancientsboss"); break;

		case ANCIENTMULTI_OBJ_BANK:
			strcpy(fileName, "ancientsmulti"); break;

// Space

		case SPACEMASTER_OBJ_BANK:
			strcpy(fileName, "spacemaster"); break;

		case SPACELEV1_OBJ_BANK:
			strcpy( fileName, "space1" ); break;

		case SPACELEV2_OBJ_BANK:
			strcpy(fileName, "space2"); break;

		case SPACELEV3_OBJ_BANK:
			strcpy(fileName, "space3"); break;

		case SPACEBOSS_OBJ_BANK:
			strcpy(fileName, "spaceboss"); break;

		case SPACEMULTI_OBJ_BANK:
			strcpy(fileName, "spacemulti"); break;

// City

		case CITYMASTER_OBJ_BANK:
			strcpy(fileName, "citymaster"); break;

		case CITYBOSS_OBJ_BANK:
			strcpy (fileName,"cityboss"); break;

		case CITYLEV1_OBJ_BANK:
			strcpy(fileName, "city1"); break;

		case CITYLEV2_OBJ_BANK:
			strcpy(fileName, "city2"); break;

		case CITYLEV3_OBJ_BANK:
			strcpy(fileName, "city3"); break;

		case CITYMULTI_OBJ_BANK:
			strcpy(fileName, "citymulti"); break;

// Subterranean

		case SUBTERRANEANMASTER_OBJ_BANK:
			strcpy(fileName, "submaster"); break;

		case SUBTERRANEANBOSS_OBJ_BANK:
			strcpy (fileName, "subboss"); break;

		case SUBTERRANEANBOSSB_OBJ_BANK:
			strcpy (fileName, "subtest"); break;

		case SUBTERRANEANLEV1_OBJ_BANK:
			strcpy(fileName, "sub1"); break;

		case SUBTERRANEANLEV2_OBJ_BANK:
			strcpy(fileName, "sub2"); break;

		case SUBTERRANEANLEV3_OBJ_BANK:
			strcpy(fileName, "sub2"); break;

		case SUBTERRANEANMULTI_OBJ_BANK:
			strcpy(fileName, "submulti"); break;

// Laboratory

		case LABORATORYMASTER_OBJ_BANK:
			strcpy(fileName, "labmaster"); break;

		case LABORATORYLEV1_OBJ_BANK:
			strcpy(fileName, "lab1"); break;

		case LABORATORYLEV2_OBJ_BANK:
			strcpy(fileName, "lab2"); break;

		case LABORATORYLEV3_OBJ_BANK:
			strcpy(fileName, "lab3"); break;

		case LABORATORYBOSSA_OBJ_BANK:
			strcpy(fileName, "labbossa"); break;
	
		case LABORATORYBOSSB_OBJ_BANK:
			strcpy (fileName, "labbossb"); break;

		case LABORATORYBOSSC_OBJ_BANK:
			strcpy (fileName, "labbossc"); break;

		case LABORATORYMULTI1_OBJ_BANK:
			strcpy(fileName, "labmulti1"); break;
		case LABORATORYMULTI2_OBJ_BANK:
			strcpy(fileName, "labmulti2"); break;
		case LABORATORYMULTI3_OBJ_BANK:
			strcpy(fileName, "labmulti3"); break;

// Halloween

		case HALLOWEENMASTER_OBJ_BANK:
			strcpy(fileName, "halloweenmaster"); break;

		case HALLOWEENLEV1_OBJ_BANK:
			strcpy(fileName, "halloween1"); break;

		case HALLOWEENLEV2_OBJ_BANK:
			strcpy(fileName, "halloween2"); break;

		case HALLOWEENLEV3_OBJ_BANK:
			strcpy(fileName, "halloween3"); break;

		case HALLOWEENBOSS_OBJ_BANK:
			strcpy(fileName, "halloweenboss"); break;

		case HALLOWEENBOSSB_OBJ_BANK:
			strcpy(fileName, "halloweenbossb");	break;

		case HALLOWEENMULTI_OBJ_BANK:
			strcpy(fileName, "halloweenmulti"); break;

// Swampys world

//		case SWAMPYWORLDMASTER_OBJ_BANK:
//			strcpy(fileName, "halloweenmaster"); break;

//		case SWAMPYWORLDLEV1_OBJ_BANK:
//			strcpy(fileName, "halloween1"); break;

//		case SWAMPYWORLDLEV2_OBJ_BANK:
//			strcpy(fileName, "halloween2"); break;

//		case SWAMPYWORLDLEV3_OBJ_BANK:
//			strcpy(fileName, "halloween3"); break;

//		case SWAMPYWORLDBOSS_OBJ_BANK:
//			strcpy(fileName, "halloweenboss"); break;

//		case SWAMPYWORLDBOSSB_OBJ_BANK:
//			strcpy(fileName, "halloweenbossb");	break;

//		case SWAMPYWORLDMULTI_OBJ_BANK:
//			strcpy(fileName, "halloweenmulti"); break;

// Super-retro
		case RETROLEV1_OBJ_BANK:
			strcpy(fileName,"sretro1");
			break;
		case RETROLEV2_OBJ_BANK:
			strcpy(fileName,"sretro2");
			break;
		case RETROLEV3_OBJ_BANK:
			strcpy(fileName,"sretro3");
			break;
		case RETROLEV4_OBJ_BANK:
			strcpy(fileName,"sretro4");
			break;
		case RETROLEV5_OBJ_BANK:
			strcpy(fileName,"sretro5");
			break;
		case RETROLEV6_OBJ_BANK:
			strcpy(fileName,"sretro6");
			break;
		case RETROLEV7_OBJ_BANK:
			strcpy(fileName,"sretro7");
			break;
		case RETROLEV8_OBJ_BANK:
			strcpy(fileName,"sretro8");
			break;
		case RETROLEV9_OBJ_BANK:
			strcpy(fileName,"sretro9");
			break;
		case RETROLEV10_OBJ_BANK:
			strcpy(fileName,"sretro10");
			break;
		case RETROMASTER_OBJ_BANK:
			strcpy(fileName,"sretromaster");
			break;		
		case RETROMULTI1_OBJ_BANK:
			strcpy(fileName, "sretromulti1"); break;
		case RETROMULTI2_OBJ_BANK:
			strcpy(fileName, "sretromulti2"); break;
		case RETROMULTI3_OBJ_BANK:
			strcpy(fileName, "sretromulti3"); break;
		case RETROMULTI4_OBJ_BANK:
			strcpy(fileName, "sretromulti4"); break;
		case RETROMULTI5_OBJ_BANK:
			strcpy(fileName, "sretromulti5"); break;
		case RETROMULTI6_OBJ_BANK:
			strcpy(fileName, "sretromulti6"); break;
		case RETROMULTI7_OBJ_BANK:
			strcpy(fileName, "sretromulti7"); break;
		case RETROMULTI8_OBJ_BANK:
			strcpy(fileName, "sretromulti8"); break;
		case RETROMULTI9_OBJ_BANK:
			strcpy(fileName, "sretromulti9"); break;
		case RETROMULTI10_OBJ_BANK:
			strcpy(fileName, "sretromulti10"); break;

// Front-end

		case FRONTEND_OBJ_BANK:
//			strcpy(fileName, "titles"); break;
			return;

		case FRONTEND1_OBJ_BANK:
			strcpy(fileName, "start"); break;

		case FRONTEND2_OBJ_BANK:
			strcpy(fileName, "levela"); break;

		case FRONTEND3_OBJ_BANK:
			strcpy(fileName, "levelb"); break;

		case FRONTEND4_OBJ_BANK:
			strcpy(fileName, "multisel"); break;

		case FRONTEND5_OBJ_BANK:
			strcpy(fileName, "language"); break;

		case TEST1_OBJ_BANK: strcpy(fileName, "test1"); break;
		case TEST2_OBJ_BANK: strcpy(fileName, "test2"); break;
		case TEST3_OBJ_BANK: strcpy(fileName, "test3"); break;
		case TEST4_OBJ_BANK: strcpy(fileName, "test4"); break;
		case TEST5_OBJ_BANK: strcpy(fileName, "test5"); break;
		case TEST6_OBJ_BANK: strcpy(fileName, "test6"); break;

		default:
			utilPrintf("Didn't load object bank '%d'\n", objectBank);
			return;
	}

	//newobj(fileName);

	LoadObjBank(fileName,baseDirectory);
}

void LoadTextureBank(int textureBank)					
{
	char fileName[256];

	switch ( textureBank )
	{
		case GARDEN_TEX_BANK:
			strcpy( fileName, "Garden" );
			break;

		case ANCIENT_TEX_BANK:
			strcpy( fileName, "ancient" );
			break;

		case SPACE_TEX_BANK:				
			strcpy( fileName, "space" );
			break;

		case CITY_TEX_BANK:				
			strcpy(fileName, "city");
			break;

		case SUBTERRANEAN_TEX_BANK:
			strcpy(fileName, "subter");
			break;

		case LABORATORY_TEX_BANK:
			strcpy(fileName, "lab");
			break;

		case HALLOWEEN_TEX_BANK:
			strcpy(fileName, "Halloween");
			break;

//		case SWAMPYWORLD_TEX_BANK:
//			strcpy(fileName, "SwampyWorld");
//			break;
		
		case SUPERRETRO_TEX_BANK:
			strcpy(fileName, "Super");
			break;

		case FRONT_TEX_BANK:
			strcpy(fileName, "front");
			break;
		
		case TITLES_TEX_BANK:
		case TITLESGENERIC_TEX_BANK:
			strcpy( fileName, "titles" );
			break;

		case FRONTEND_TEX_BANK:				
			strcpy( fileName, "titles" );
			break;

		case INGAMEGENERIC_TEX_BANK:
			strcpy( fileName, "System" );
			break;

		default:
			return;
	}

	if (compressedTexBanks)
		LoadTexBankFile(fileName,baseDirectory);
	else
		LoadTexBank(fileName,baseDirectory);
	return;
}

/*
void newobj(char* fn)
{
	int i;char*c,*d;
	____ _____ ___ fn, ______ ___ 0; __(_____);	_(_____), _(______));
	____ ________ ___ _____	+7,	_(______); ______; ______--,
	_d(________) ___ _d(_____) _______;	____ _____ ___ fn, ________ 
	___ "newobj\\"; __(________); __(_(_____)) ___ __(_(________)) _______;
}
*/

void LoadGame(void)
{
	char file[MAX_PATH];
	void *info; int size;
//	FILE *fp;

#ifdef PC_DEMO
	return;
#endif

	strcpy(file, baseDirectory);
	strcat(file, "SaveData\\savegame.dat");
	
	info = fileLoad(file, &size);
	if (!info)
	{
		utilPrintf("Couldn't load savegame %s\n", file);
		return;
	}

	LoadSaveGameBlock(info, size);
	FREE(info);

	utilPrintf("Loaded game from %s\n", file);
}

void SaveGame(void)
{
	char file[MAX_PATH];
	void *info; unsigned long size;
	FILE *fp;

#ifdef PC_DEMO
	return;
#endif

	strcpy(file, baseDirectory);
	strcat(file, "\\SaveData\\savegame.dat");
	
	fp = fopen(file,"wb");
	if (!fp)
	{
		utilPrintf("Couldn't save savegame %s\n", file);
		return;
	}

	MakeSaveGameBlock(&info, &size);
	fwrite(info, size, 1, fp);
	FREE(info);

	fclose(fp);	

	utilPrintf("Saved game to %s\n", file);
}

/*	--------------------------------------------------------------------------------
	Function		: CreateLevelObjects
	Purpose			: creates the objects for the world / level
	Parameters		: 
	Returns			: void
	Info			: 
*/
void CreateLevelObjects(unsigned long worldID,unsigned long levelID)
{
	ACTOR2 *theActor;
	SCENIC *ts = firstScenic;
	int actCount = 0;
	char tmp[5];
	long i;
	
	// go through and add items
	while (ts)
	{
//		float tv;

		stringChange(ts->name);

		if((gstrcmp(ts->name,"backdrop.obe") == 0) || (gstrcmp(ts->name,"backdrop.psi") == 0) )
		{
			backGnd = CreateAndAddActor (ts->name,ts->pos.vx,ts->pos.vy,ts->pos.vz,INIT_ANIMATION);
			if (backGnd)
			{
				backGnd->actor->size.vx = ToFixed(5);
				backGnd->actor->size.vy = ToFixed(5);
				backGnd->actor->size.vz = ToFixed(5);
			}
//			actList = actList->next;
//			if (actList)
//				actList->prev = NULL;
		}
		else
		{

			if((gstrcmp(ts->name,"world.obe") != 0) && (gstrcmp(ts->name,"world.psi") != 0))
			{
				for( i=0; i<4; i++ )
					tmp[i] = ts->name[i];
				tmp[4] = '\0';

// JIM: This line of code basically says 'don't add water objects in software mode'. WHY?
//				if (rHardware || gstrcmp( tmp, "wat_\0" ))
//				{
					if( !(theActor = CreateAndAddActor (ts->name,ts->pos.vx,ts->pos.vy,ts->pos.vz,INIT_ANIMATION)) )
					{
						ts = ts->next;
						continue;
					}

					dp("Added actor '%s'\n",ts->name);

					/*
					if(gstrcmp(ts->name,"world.obe") == 0)
					{
						theActor->flags = ACTOR_DRAW_ALWAYS;
						globalLevelActor = theActor;
					}*/

					// If a water object, draw always
					if( !gstrcmp( tmp, "wat_" ))
					{
						theActor->flags = ACTOR_WATER | ACTOR_DRAW_ALWAYS | ACTOR_ADDITIVE;
						((MDX_ACTOR *)(theActor->actor->actualActor))->objectController->object->flags =  OBJECT_FLAGS_WAVE | OBJECT_FLAGS_MODGE | OBJECT_FLAGS_ADDITIVE;
						((MDX_ACTOR *)(theActor->actor->actualActor))->flags |= ACTOR_SLIDY;
						
						if (ts->name[4]=='f')
							theActor->flags |= ACTOR_SLIDYTEX;

					}
					else if( !gstrcmp( tmp, "slu_" ) )
					{
						theActor->flags = ACTOR_WATER | ACTOR_SLUDGE | ACTOR_DRAW_ALWAYS;
						((MDX_ACTOR *)(theActor->actor->actualActor))->objectController->object->flags =  OBJECT_FLAGS_MODGE;
						((MDX_ACTOR *)(theActor->actor->actualActor))->flags |= ACTOR_SLIDY;
						if (ts->name[4]=='f')
							theActor->flags |= ACTOR_SLIDYTEX;

					}
					else if( !gstrcmp( tmp, "lea_" ) )
					{
						theActor->flags = ACTOR_WATER | ACTOR_LEAVES | ACTOR_DRAW_ALWAYS;
						((MDX_ACTOR *)(theActor->actor->actualActor))->objectController->object->flags =  OBJECT_FLAGS_WAVE;						
						((MDX_ACTOR *)(theActor->actor->actualActor))->flags |= ACTOR_SLIDY;
						if (ts->name[4]=='f')
							theActor->flags |= ACTOR_SLIDYTEX;

					}
					else if ( !gstrcmp( tmp, "xxa_"))
					{
						theActor->flags = ACTOR_ADDITIVE;
						((MDX_ACTOR *)(theActor->actor->actualActor))->objectController->object->flags =  OBJECT_FLAGS_ADDITIVE;
					}

		// JH : rot is not a member
		//			tv = ts->rot.y;
		//			ts->rot.y = ts->rot.z;
		//			ts->rot.z = tv;

		//			GetQuaternionFromRotation (&theActor->actor->qRot,&ts->rot);

					theActor->actor->qRot = ts->rot;	// rot is a quat! Yay!

					actorAnimate(theActor->actor,0,YES,NO,90,0);
					
					// ds - what the HELL?

					/*if(ts->name[0] == 'a')
					{
						float rMin,rMax,rNum;
						if(ts->name[2] == '_')
						{
							rMin = ts->name[1] - 30;
							if(rMin = 0) 
								rMin = 10;
							rMin /= 10.0;
							actorAnimate(theActor->actor,0,YES,NO,rMin, 0);
						}
						else if(ts->name[3] == '_')
						{
							rMin = ts->name[1] - 30;
							if(rMin == 0) 
								rMin = 10;
							rMin /= 10.0;
						
							rMax = ts->name[1] - 30;
							if (rMax == 0) 
								rMax = 10;
							rMax /= 10.0;

							rNum = Random(1000);
							rNum= rMin + ((rNum * (rMax - rMin)) / 1000);
												
							actorAnimate(theActor->actor,0,YES,NO,rNum, 0);
						}
					}*/

					theActor->actor->size.vx = 4096;
					theActor->actor->size.vy = 4096;
					theActor->actor->size.vz = 4096;
					actCount++;
//				}
			}
		}
		ts = ts->next;
	}

	utilPrintf("\n\n** ADDED %d ACTORS **\n\n",actCount);
}

