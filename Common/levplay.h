/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: levelplaying.h
	Programmer	: David Swift
	Date		: 17/03/99

----------------------------------------------------------------------------------------------- */

#ifndef LEVELPLAYING_H_INCLUDED
#define LEVELPLAYING_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void InitDemoMode();
void RunDemoMode();
void LoadDemoFile(int world, int level);

#ifdef __cplusplus
}
#endif
#endif