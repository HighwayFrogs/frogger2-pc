/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: DefaultData.h
	Programmer	: James Healey
	Date		: 14/06/99

----------------------------------------------------------------------------------------------- */

#ifndef DEFAULTDATA_H_INCLUDED
#define DEFAULTDATA_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//************************************************************************************************
//	Defines

#define MAX_SAVE_SLOTS			5

#define MAX_HISCORE_SLOTS		5

//************************************************************************************************
//	Type Defines

typedef struct _SAVE_SLOT
{
	char name[2];
	long score;
} SAVE_SLOT;

typedef struct _SCORENAME
{
	char name[4];
} SCORENAME;

typedef struct _HISCORE
{
	char	name[2];
	long	score;
} HISCORE;

//************************************************************************************************
//	Global Externs

extern SAVE_SLOT saveSlots [ MAX_SAVE_SLOTS ];

extern SCORENAME	scoreNames	[ MAX_HISCORE_SLOTS ]; 
extern HISCORE		hiScoreData [ MAX_HISCORE_SLOTS ];


#ifdef __cplusplus
}
#endif
#endif