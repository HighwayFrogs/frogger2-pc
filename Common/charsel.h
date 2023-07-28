/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: charsel.h
	Programmer	: David Swift
	Date		: 27/03/00 

----------------------------------------------------------------------------------------------- */

#ifndef CHARSEL_H_INCLUDED
#define CHARSEL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CHARSELOPTIONS
{
	int character, world, level;
} CHARSELOPTIONS;

void StartCharacterSelectMode(CHARSELOPTIONS* character1, CHARSELOPTIONS* character2);
void RunCharacterSelectMode(void);

#ifdef __cplusplus
}
#endif

#endif