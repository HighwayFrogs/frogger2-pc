/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: flatpack.c
	Programmer	: David Swift (from code by Richard Hackett)
	Date		: 25 May 00

----------------------------------------------------------------------------------------------- */

#ifndef FLATPACK_H
#define FLATPACK_H

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_STAKFILES 10

	typedef struct _STAKFILE
{
	char *stakFile;
} STAKFILE;

extern STAKFILE stakFiles [ NUM_STAKFILES ];

char *getFileFromStack(char *stackFile, char *name, int *length);

void LoadStakFile								( int stakBank );
char *FindStakFileInAllBanks ( char *name, int *length );
void FreeStakFiles ( void );

#ifdef __cplusplus
}
#endif
#endif