/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: savegame.c
	Programmer	: David Swift
	Date		: 11 May 00

----------------------------------------------------------------------------------------------- */

#include <stdlib.h>
#include <islmem.h>
#include <islutil.h>
#include "main.h"
#include "savegame.h"
#include "layout.h"
#include "frogger.h"
#include "menus.h"
#include "game.h"


#ifdef PSX_VERSION
extern char saveicon[];  // Included TIM data created from a 16-col BMP by TIMUTIL.EXE
#endif



/*	--------------------------------------------------------------------------------
	Function		: SaveGameInfo
	Parameters		: 
	Returns			: int
	Info			: Saves the current game state to the memory card/hard drive
*/

void *MakeSaveGameBlock(void** ptr, unsigned long *size)
{
	unsigned char *data;
	SAVEGAME_HEADER *header;
	SAVEGAME_LEVELINFO *levelinfo;
	LEVEL_VISUAL_DATA* levelvis;
	int world, level, count;

//	*size = SAVEGAME_SIZE;
	*size = SAVEGAME_SIZE+PSXCARDHEADER_SIZE; //PSXCARDHEADER_SIZE = 0 on pc/dc
//	data = *ptr = MALLOC0(SAVEGAME_SIZE);
	data = *ptr = MALLOC0(SAVEGAME_SIZE+PSXCARDHEADER_SIZE);
	//memset(data, 0, SAVEGAME_SIZE);


	//fill in PsxCardHeader
#ifdef PSX_VERSION
	{
		TIM_IMAGE	tim;
		PsxCardHeaderType *cardHeader = data;

		cardHeader->magic[0] = 'S';			// Sony header format
		cardHeader->magic[1] = 'C';
		cardHeader->type = 0x11;
		cardHeader->blockEntry = 1;
		memset(cardHeader->title, 0, 64);
		asciiStringToSJIS("Frogger 2", cardHeader->title);
		memset(cardHeader->reserved, 0, 28);
//		saveicon = fileLoad("saveicon.tim", NULL);
		OpenTIM((u_long *)saveicon);
		ReadTIM(&tim);
		memcpy(cardHeader->clut, tim.caddr, 32);
		cardHeader->clut[0] = cardHeader->clut[1] = 0x00;
		memcpy(cardHeader->icons, tim.paddr, 128);
	}
#endif


	// Set header info
//bb - now got a PsxCardHeader struct at the beginning, (psx only)
	header = (SAVEGAME_HEADER*) (data + PSXCARDHEADER_SIZE);
	header->version = SAVEGAME_VERSION;

	strcpy( header->playername, player[0].name );
	// TODO: language, controller setup & audio settings here
	header->lives = player[0].lives;
	
	header->vol_musicvol = globalMusicVol;
	header->vol_effect = globalSoundVol;
	
	// Set per-level info
//bb - now got a PsxCardHeader struct at the beginning, (psx only)
//	levelinfo = (SAVEGAME_LEVELINFO*)(data + SAVEGAME_HEADERSIZE);
	levelinfo = (SAVEGAME_LEVELINFO*)(data + SAVEGAME_HEADERSIZE + PSXCARDHEADER_SIZE);
	count = 0;

	for (world=0; world<(MAX_WORLDS-1); world++)
		for (level=0; level<worldVisualData[world].numLevels; level++)
		{
			levelvis = &worldVisualData[world].levelVisualData[level];

//sb horrible bodge to remember if training level has been completed!!!
			levelinfo->flags = (levelvis->levelOpen ? 1:0) + (levelvis->levelCompleted ? 2:0) + (worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].levelCompleted ? 4:0) + (levelvis->levelBeaten ? 8:0) + ((gameState.difficulty == DIFFICULTY_HARD) ? 16:0) + ((gameState.difficulty == DIFFICULTY_EASY) ? 32:0);
			levelinfo->bestTime = levelvis->parTime;
			levelinfo->garibs = levelvis->maxCoins;
			
			strncpy(levelinfo->winner, levelvis->parName, 8);

			levelinfo++, count++;
		}

	return data;
}

/*	--------------------------------------------------------------------------------
	Function		: LoadGameInfo
	Parameters		: 
	Returns			: int
	Info			: Saves the current game state to the memory card/hard drive
*/

int LoadSaveGameBlock(void* ptr, unsigned long size)
{
	unsigned char* data = ptr;
	SAVEGAME_HEADER *header;
	SAVEGAME_LEVELINFO *levelinfo;
	LEVEL_VISUAL_DATA* levelvis;
	int world, level, count;

	// Set header info
//bb - psx card header infront (psx only)
	header = (SAVEGAME_HEADER*) (data + PSXCARDHEADER_SIZE);
	
	if (header->version != SAVEGAME_VERSION)
	{
		utilPrintf("Failed loading savegame block due to incorrect revision number\n");
		return 0;
	}
	
	strcpy( player[0].name, header->playername );
	// TODO: language, controller setup & audio settings here
	if( header->lives )
		player[0].lives = header->lives;
	else
		player[0].lives = 3;

	globalMusicVol = header->vol_musicvol;
	globalSoundVol = header->vol_effect;

	// Set per-level info
//bb
//	levelinfo = (SAVEGAME_LEVELINFO*)(data + SAVEGAME_HEADERSIZE);
	levelinfo = (SAVEGAME_LEVELINFO*)(data + SAVEGAME_HEADERSIZE + PSXCARDHEADER_SIZE);
	count = 0;

	for (world=0; world<(MAX_WORLDS-1); world++)
	{
		for (level=0; level<worldVisualData[world].numLevels; level++)
		{
			levelvis = &worldVisualData[world].levelVisualData[level];

			levelvis->levelOpen = levelinfo->flags & 1;
			if((level < worldVisualData[world].numSinglePlayerLevels) && (levelvis->levelOpen))
				worldVisualData[world].worldOpen = WORLD_OPEN;
			levelvis->levelCompleted = (levelinfo->flags & 2) ? 1 : 0;
			levelvis->levelBeaten = (levelinfo->flags & 8) ? 1 : 0;
//sb horrible bodge to remember if training level has been completed!!!
			worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].levelCompleted = (levelinfo->flags & 4) ? 1 : 0;

			if(levelinfo->flags & 16)
				gameState.difficulty = DIFFICULTY_HARD;
			else if(levelinfo->flags & 32)
				gameState.difficulty = DIFFICULTY_EASY;
			
			levelvis->maxCoins = levelinfo->garibs;
			levelvis->parTime = levelinfo->bestTime;
			
			strncpy(levelvis->parName, levelinfo->winner, 8); levelvis->parName[8] = 0;

			levelinfo++, count++;
		}
	}

	return 1;
}
