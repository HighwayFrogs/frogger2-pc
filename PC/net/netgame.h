/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: netgame.h
	Programmer	: David Swift
	Date		:

----------------------------------------------------------------------------------------------- */

#ifndef __NETGAME_H
#define __NETGAME_H

#include "overlays.h"
#include "network.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef int (*NETGAME_LOOP)(void);

extern unsigned long		gameStartTime;

extern NET_MESSAGEHANDLER	netgameHandler;
extern NETGAME_LOOP			netgameLoopFunc;

void NetgameStartGame();
void NetgameRun();
void NetgameDeath();
void NetgameHostGame();
void NetgameStatusMessage(const char* msg);

#ifdef __cplusplus
}
#endif

#endif