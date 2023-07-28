/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: netconnect.h
	Programmer	: 
	Date		:

----------------------------------------------------------------------------------------------- */

#ifndef NETCONNECT_H
#define NETCONNECT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <dplay.h>

int StartNetworkGame(HWND hwnd, int flag);
void ShutdownNetworkGame();

#ifdef __cplusplus
}
#endif

#endif