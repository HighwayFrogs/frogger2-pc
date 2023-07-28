/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: savegame.h
	Programmer	: David Swift
	Date		: 11 May 00

----------------------------------------------------------------------------------------------- */

#ifndef SAVEGAME_H_INCLUDED
#define SAVEGAME_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


#define SAVEGAME_VERSION		1

typedef struct _PsxCardHeaderType
{
	char	magic[2];		// Always 'SC'
	char	type;			// Icon type, always 0x11
	char	blockEntry;		// Number of slots (1)
	char 	title[64];		// Shift-JIS title
	char 	reserved[28];		// Reserved
	char	clut[32];		// Icon palette
	char	icons[3][128];	// Icon data (only first one)
} PsxCardHeaderType;

#ifdef PSX_VERSION
#define PSXCARDHEADER_SIZE (sizeof(PsxCardHeaderType))
#else
#define PSXCARDHEADER_SIZE 0
#endif


typedef struct _SAVEGAME_HEADER
{
	unsigned char version, versioncheck;
	unsigned char story_world, story_level;
	char playername[8];
	unsigned char language, controller, vol_musicvol, vol_effect;
	unsigned char char_unlock;
	unsigned char lives;
	char res1[2];
	unsigned char flags[32], res2[12];
} SAVEGAME_HEADER;

typedef struct _SAVEGAME_LEVELINFO
{
	unsigned char flags, garibs, res1[2];
	char winner[8];
	unsigned long bestTime;
	unsigned char res2[16];
} SAVEGAME_LEVELINFO;

#define SAVEGAME_HEADERSIZE		sizeof(SAVEGAME_HEADER)
#define SAVEGAME_LEVELS			64	// this will change when we get the levels sorted!
#define SAVEGAME_SIZE			(SAVEGAME_HEADERSIZE+sizeof(SAVEGAME_LEVELINFO)*SAVEGAME_LEVELS)



void *MakeSaveGameBlock(void** ptr, unsigned long *size);
int LoadSaveGameBlock(void* data, unsigned long size);

#ifdef __cplusplus
}
#endif

#endif