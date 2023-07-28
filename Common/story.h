/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: sprite.h
	Programmer	: Jim Hubbard
	Date		: 25/02/00

----------------------------------------------------------------------------------------------- */

#ifndef STORY_H_INCLUDED
#define STORY_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_STORY_LEVELS 17

enum
{
	FMV_NONE,
	FMV_HASBRO_LOGO,
	FMV_BLITZ_LOGO,
	FMV_INTRO,
	FMV_FROGGER_LILLIE_BABIES,
	FMV_SWAMPY_PLAN,
	FMV_SWAMPY_ESCAPES,
	FMV_LILLIE_TO_GATE,
	FMV_BOTH_AT_GATE,
	FMV_LILLIE_COMPUTER,
	FMV_SWAMPY_COMMERCIAL,
	FMV_TELEPORT_TO_EARTH,
	FMV_ENTER_HAUNTED_HOUSE,
	FMV_LILLIE_IN_CAGE,
	FMV_VICTORY,
	NUM_FMV_SEQUENCES,
};

typedef struct
{
	char	name[16];
	int		len;
}FMV_DATA;

extern short storySequenceLevelToChapter[];
extern FMV_DATA fmv[NUM_FMV_SEQUENCES];

typedef struct
{
	long worldNum,levelNum,another,character,fmv;
	char iconName[32];
}GAME_LEVEL;

extern GAME_LEVEL storySequence[];

void StoryStartLevel(void);

#ifdef __cplusplus
}
#endif

#endif
