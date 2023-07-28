/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: backdrop.h
	Programmer	: David
	Date		: 
	Purpose		: Backdrop handling functions

----------------------------------------------------------------------------------------------- */

#ifndef BACKDROP_H_INCLUDED
#define BACKDROP_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif


int InitBackdrop(const char *filename);
int InitCDBackdrop(const char *filename);
void FreeBackdrop(void);
void DrawBackdrop(void);

void InitLoadingScreen( const char *filename );
void UpdateLoadingScreen( short addprog );
void FreeLoadingScreen( );

#ifdef __cplusplus
}
#endif


#endif
