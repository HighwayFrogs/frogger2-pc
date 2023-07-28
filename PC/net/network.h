/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: network.h
	Programmer	: David Swift
	Date		:

----------------------------------------------------------------------------------------------- */

#ifndef __NETWORK_H
#define __NETWORK_H

#include <dplay.h>
#include "netchat.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Message identifiers - the first byte of our data packets
// note that message size varies depending on context

enum APPMSGTYPE
{
	APPMSG_LEVELSEL,	// player has selected a new level (in the front-end)
	APPMSG_START,		// start the game!
	APPMSG_CHAT,		// chat message (text)
	APPMSG_UPDATE,		// game update
	APPMSG_PING,		// ping
	APPMSG_PINGREPLY,	// ping reply
	APPMSG_READY,		// this player is ready (synched etc.)
	APPMSG_HOSTREADY,	// the host is ready to sync
	APPMSG_DEATH,		// this player was deaded

	GAMEMSG_START
};

typedef struct _NETPLAYER
{
	DPID	dpid;		// DirectPlay player identifier - passed to IDirectPlay to query address stuff
//	bool	isHost;		// true if this player is the host, false otherwise
	bool	isReady;
	char	start;		// starttile index to use for this player
	char	name[16];

	int		score;

	unsigned long lastUpdateMsg;	// the tick count of the last update msg
	
	struct _NETPLAYER *next, *prev;
} NETPLAYER;

extern LPDIRECTPLAY4A dplay;
extern DPID		dpidLocalPlayer;
extern HANDLE	hLocalPlayerEvent;
extern char		sessionName[256];
extern char		playerName[16];
extern bool		isHost;
extern NETPLAYER netPlayerList[4];

typedef int (*NET_MESSAGEHANDLER)(void *data, unsigned long size, NETPLAYER *player);

void NetStartGame();

NET_MESSAGEHANDLER NetInstallMessageHandler(NET_MESSAGEHANDLER handler);
void NetProcessMessages();
void SetupNetPlayerList();

int GetPlayerNumFromID(DPID id);
int NetBroadcastMessage(void *data, unsigned long size);
int NetBroadcastUrgentMessage(void *data, unsigned long size);

void NetShowMessage(const char* string, CHAT_FORMAT f);
const char *GetDirectPlayError(HRESULT hRes);

#ifdef __cplusplus
}
#endif

#endif