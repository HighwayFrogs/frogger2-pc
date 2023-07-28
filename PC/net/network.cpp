/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: network.cpp
	Programmer	: David Swift
	Date		: 

----------------------------------------------------------------------------------------------- */

#include <windows.h>
#include <dplay.h>
#include <dplobby.h>

#include "main.h"
#include "islutil.h"
#include "resource.h"

#include "network.h"
#include "netchat.h"
#include "netgame.h"
#include "net.h"

#include "frogger.h"
#include "frogmove.h"
#include "multi.h"
#include "hud.h"

#include "lang.h"

void *recieveBuffer;
DWORD recieveBufferSize = 1024;

LPDIRECTPLAY4A dplay		= NULL;

DPID	dpidLocalPlayer;
HANDLE	hLocalPlayerEvent;

char	sessionName[256]	= "Frogger2";
char	playerName[16]		= "Player";
bool	isHost			= false;

//void HandleApplicationMessage(LPDPMSG_GENERIC, DWORD, DPID, DPID);
void HandleSystemMessage(LPDPMSG_GENERIC, DWORD, DPID, DPID);

NET_MESSAGEHANDLER messageHandler;	// callback for interpreting game messages

NETPLAYER netPlayerList[MAX_FROGS];



const char *GetDirectPlayError(HRESULT hRes)
{
	static char szTempStr[12];

	switch(hRes)
	{
		case DP_OK: return ("DP_OK");
		case DPERR_ALREADYINITIALIZED: return ("DPERR_ALREADYINITIALIZED");
		case DPERR_ACCESSDENIED: return ("DPERR_ACCESSDENIED");
		case DPERR_ACTIVEPLAYERS: return ("DPERR_ACTIVEPLAYERS");
		case DPERR_BUFFERTOOSMALL: return ("DPERR_BUFFERTOOSMALL");
		case DPERR_CANTADDPLAYER: return ("DPERR_CANTADDPLAYER");
		case DPERR_CANTCREATEGROUP: return ("DPERR_CANTCREATEGROUP");
		case DPERR_CANTCREATEPLAYER: return ("DPERR_CANTCREATEPLAYER");
		case DPERR_CANTCREATESESSION: return ("DPERR_CANTCREATESESSION");
		case DPERR_CAPSNOTAVAILABLEYET: return ("DPERR_CAPSNOTAVAILABLEYET");
		case DPERR_EXCEPTION: return ("DPERR_EXCEPTION");
		case DPERR_GENERIC: return ("DPERR_GENERIC");
		case DPERR_INVALIDFLAGS: return ("DPERR_INVALIDFLAGS");
		case DPERR_INVALIDOBJECT: return ("DPERR_INVALIDOBJECT");
	//	case DPERR_INVALIDPARAM: return ("DPERR_INVALIDPARAM");	 dup value
		case DPERR_INVALIDPARAMS: return ("DPERR_INVALIDPARAMS");
		case DPERR_INVALIDPLAYER: return ("DPERR_INVALIDPLAYER");
		case DPERR_INVALIDGROUP: return ("DPERR_INVALIDGROUP");
		case DPERR_NOCAPS: return ("DPERR_NOCAPS");
		case DPERR_NOCONNECTION: return ("DPERR_NOCONNECTION");
	//	case DPERR_NOMEMORY: return ("DPERR_NOMEMORY");		dup value
		case DPERR_OUTOFMEMORY: return ("DPERR_OUTOFMEMORY");
		case DPERR_NOMESSAGES: return ("DPERR_NOMESSAGES");
		case DPERR_NONAMESERVERFOUND: return ("DPERR_NONAMESERVERFOUND");
		case DPERR_NOPLAYERS: return ("DPERR_NOPLAYERS");
		case DPERR_NOSESSIONS: return ("DPERR_NOSESSIONS");
		case DPERR_PENDING: return ("DPERR_PENDING");
		case DPERR_SENDTOOBIG: return ("DPERR_SENDTOOBIG");
		case DPERR_TIMEOUT: return ("DPERR_TIMEOUT");
		case DPERR_UNAVAILABLE: return ("DPERR_UNAVAILABLE");
		case DPERR_UNSUPPORTED: return ("DPERR_UNSUPPORTED");
		case DPERR_BUSY: return ("DPERR_BUSY");
		case DPERR_USERCANCEL: return ("DPERR_USERCANCEL");
		case DPERR_NOINTERFACE: return ("DPERR_NOINTERFACE");
		case DPERR_CANNOTCREATESERVER: return ("DPERR_CANNOTCREATESERVER");
		case DPERR_PLAYERLOST: return ("DPERR_PLAYERLOST");
		case DPERR_SESSIONLOST: return ("DPERR_SESSIONLOST");
		case DPERR_UNINITIALIZED: return ("DPERR_UNINITIALIZED");
		case DPERR_NONEWPLAYERS: return ("DPERR_NONEWPLAYERS");
		case DPERR_INVALIDPASSWORD: return ("DPERR_INVALIDPASSWORD");
		case DPERR_CONNECTING: return ("DPERR_CONNECTING");
		case DPERR_CONNECTIONLOST: return ("DPERR_CONNECTIONLOST");
		case DPERR_UNKNOWNMESSAGE: return ("DPERR_UNKNOWNMESSAGE");
		case DPERR_CANCELFAILED: return ("DPERR_CANCELFAILED");
		case DPERR_INVALIDPRIORITY: return ("DPERR_INVALIDPRIORITY");
		case DPERR_NOTHANDLED: return ("DPERR_NOTHANDLED");
		case DPERR_CANCELLED: return ("DPERR_CANCELLED");
		case DPERR_ABORTED: return ("DPERR_ABORTED");
		case DPERR_BUFFERTOOLARGE: return ("DPERR_BUFFERTOOLARGE");
		case DPERR_CANTCREATEPROCESS: return ("DPERR_CANTCREATEPROCESS");
		case DPERR_APPNOTSTARTED: return ("DPERR_APPNOTSTARTED");
		case DPERR_INVALIDINTERFACE: return ("DPERR_INVALIDINTERFACE");
		case DPERR_NOSERVICEPROVIDER: return ("DPERR_NOSERVICEPROVIDER");
		case DPERR_UNKNOWNAPPLICATION: return ("DPERR_UNKNOWNAPPLICATION");
		case DPERR_NOTLOBBIED: return ("DPERR_NOTLOBBIED");
		case DPERR_SERVICEPROVIDERLOADED: return ("DPERR_SERVICEPROVIDERLOADED");
		case DPERR_ALREADYREGISTERED: return ("DPERR_ALREADYREGISTERED");
		case DPERR_NOTREGISTERED: return ("DPERR_NOTREGISTERED");
		case DPERR_AUTHENTICATIONFAILED: return ("DPERR_AUTHENTICATIONFAILED");
		case DPERR_CANTLOADSSPI: return ("DPERR_CANTLOADSSPI");
		case DPERR_ENCRYPTIONFAILED: return ("DPERR_ENCRYPTIONFAILED");
		case DPERR_SIGNFAILED: return ("DPERR_SIGNFAILED");
		case DPERR_CANTLOADSECURITYPACKAGE: return ("DPERR_CANTLOADSECURITYPACKAGE");
		case DPERR_ENCRYPTIONNOTSUPPORTED: return ("DPERR_ENCRYPTIONNOTSUPPORTED");
		case DPERR_CANTLOADCAPI: return ("DPERR_CANTLOADCAPI");
		case DPERR_NOTLOGGEDIN: return ("DPERR_NOTLOGGEDIN");
		case DPERR_LOGONDENIED: return ("DPERR_LOGONDENIED");
	}

	// For errors not in the list, return HRESULT string
	wsprintf(szTempStr,"0x%08X",hRes);
	return szTempStr;
}


void NetShowMessage(const char* string, CHAT_FORMAT f)
{	
	ChatShowMessage(string, f);
	NetgameStatusMessage(string);
}


NETPLAYER *GetPlayerFromID(DPID id)
{
	for (int p=0; p<MAX_FROGS; p++)
		if (netPlayerList[p].dpid == id)
			return &netPlayerList[p];

	return NULL;
}

int GetPlayerNumFromID(DPID id)
{
	for (int p=0; p<MAX_FROGS; p++)
		if (netPlayerList[p].dpid == id)
			return p;

	return NULL;
}

const char *NetGetPlayerName(int pl)
{
	return netPlayerList[pl].name;
}

int AddNetPlayer(DPID id)
{
	int i;
	NETPLAYER *player;
	HRESULT res;
	DPCAPS dpc;

	dpc.dwSize = sizeof(dpc);
	res = dplay->GetPlayerCaps(id, &dpc, 0);

	for (i=0, player = &netPlayerList[0]; i<MAX_FROGS; i++, player++)
	{
		if (!player->dpid)
		{
			player->dpid = id;

			DWORD size = 0;
			dplay->GetPlayerName(id, 0, &size);

			UBYTE *buf = new UBYTE[size];
			dplay->GetPlayerName(id, buf, &size);

			DPNAME *name = (DPNAME*)buf;

			strncpy(player->name, name->lpszShortNameA, 15);

			delete buf;

			//player->isHost = ((dpc.dwFlags & DPCAPS_ISHOST) != 0);
			//player->player = i;
			NUM_FROGS++;
			return i;
		}
	}

	return -1;
}

int RemoveNetPlayer(DPID id)
{
	int p, q;
	NETPLAYER *player;

	for (p=0, player=&netPlayerList[0]; p<MAX_FROGS; p++, player++)
		if (netPlayerList[p].dpid == id)
		{
			if (frog[p])
				frog[p]->draw = 0;

			if (mpl[p].penalText)
				mpl[p].penalText->draw = 0;

			if (multiHud.backChars[p])
			{
				multiHud.backChars[p]->draw = false;
				multiHud.penaliseText[p]->draw = false;
				multiHud.penalOver[p]->draw = false;

				multiHud.trophies[p][0]->draw = false;
				multiHud.trophies[p][1]->draw = false;
				multiHud.trophies[p][2]->draw = false;
				mpl[p].nameText->draw = false;
				mpl[p].numText->draw = false;
			}

			q = p+1;
			while (q<NUM_FROGS)
			{
				// here's a bit of comedy code... - ds

				multiHud.backChars[p] = multiHud.backChars[q];
				multiHud.penaliseText[p] = multiHud.penaliseText[q];
				multiHud.penalOver[p] = multiHud.penalOver[q];
				memcpy(multiHud.trophies[p], multiHud.trophies[q], sizeof(SPRITEOVERLAY*)*3);

				netPlayerList[p]	= netPlayerList[q];
				currTile[p]	= currTile[q];
				destTile[p]	= destTile[q];
				frog[p]		= frog[q];
				player[p]	= player[q];
				mpl[p]		= mpl[q];
				gTStart[p]	= gTStart[q];

				p++,q++;
			}

			
			netPlayerList[p].dpid = 0;
			NUM_FROGS--;
			return 1;
		}

	return 0;
}

BOOL CALLBACK PlayerListEnum(DPID id, DWORD type, LPCDPNAME lpname, DWORD flags, LPVOID context)
{
	if (!GetPlayerFromID(id))
		AddNetPlayer(id);

	return TRUE;
}

void SetupNetPlayerList()
{
	for (int i=0; i<MAX_FROGS; i++)
	{
		netPlayerList[i].dpid = 0;
		//netPlayerList[i].player = -1;
	}

	NUM_FROGS=0;

	if (dplay)
	{
		dplay->EnumPlayers(NULL, PlayerListEnum, 0, DPENUMPLAYERS_ALL);
	}
}

NET_MESSAGEHANDLER NetInstallMessageHandler(NET_MESSAGEHANDLER handler)
{
	NET_MESSAGEHANDLER old = messageHandler;

	messageHandler = handler;

	return old;
}

/*	--------------------------------------------------------------------------------
	Function		: ReceiveMessage
	Purpose			: receive a message from a player
	Parameters		: LPDPLAYINFO
	Returns			: HRESULT
	Info			: 
*/
HRESULT ReceiveMessages()
{
	DPID idFrom,idTo;
	HRESULT	hRes;
	DWORD recieveSize;

	if (!recieveBuffer)
		recieveBuffer = AllocMem(recieveBufferSize);

	// loop to read all msg in queue
	while(1)
	{
		// read msgs from any player, including system player
		idFrom = 0;
		idTo = 0;

		DWORD messageSize = recieveBufferSize;

		hRes = dplay->Receive(&idFrom, &idTo, DPRECEIVE_ALL, recieveBuffer, &messageSize);

		// not enough room - resize buffer
		if(hRes == DPERR_BUFFERTOOSMALL)
		{
			if (recieveBuffer) FreeMem(recieveBuffer);

			if (!(recieveBuffer = AllocMem(messageSize)))
				return DPERR_OUTOFMEMORY;
			
			recieveBufferSize = messageSize;
		}
		else
		if (SUCCEEDED(hRes)) /*&& (messageSize >= sizeof(DPMSG_GENERIC)))*/
		{
			// check for system message
			if (idFrom == DPID_SYSMSG)
				HandleSystemMessage((LPDPMSG_GENERIC)recieveBuffer, messageSize, idFrom, idTo);
			else
			{
				NETPLAYER *player = GetPlayerFromID(idFrom);

				if (messageHandler && player)
					messageHandler((VOID*)recieveBuffer, messageSize, player);
	
				//HandleApplicationMessage((LPDPMSG_GENERIC)recieveBuffer, messageSize, idFrom, idTo);
			}
		}
		else
			break;
	};

	return DP_OK;
}

void NetProcessMessages()
{
	if (dplay)
	{
		// handle pending messages
		ReceiveMessages();
	}
}

/*	--------------------------------------------------------------------------------
	Function		: HandleSystemMessage
	Purpose			: handles a system message for multiplay
	Parameters		: LPDPLAYINFO,LPDPMSG_GENERIC,DWORD,DPIP,DPID
	Returns			: void
	Info			: 
*/
void HandleSystemMessage(LPDPMSG_GENERIC lpMsg,DWORD dwMsgSize,DPID idFrom,DPID idTo)
{
	long i;
	char message[256];

	switch(lpMsg->dwType)
	{
		case DPSYS_CREATEPLAYERORGROUP:
		{
			LPDPMSG_CREATEPLAYERORGROUP lp = (LPDPMSG_CREATEPLAYERORGROUP) lpMsg;

			wsprintf(message, GAMESTRING(STR_NET_JOINED), lp->dpnName.lpszShortNameA);
			NetShowMessage(message, CHAT_SYSTEM);
			utilPrintf("NET: %s\n", message);

			AddNetPlayer(lp->dpId);
		}
		break;

		case DPSYS_DESTROYPLAYERORGROUP:
		{
			LPDPMSG_DESTROYPLAYERORGROUP lp = (LPDPMSG_DESTROYPLAYERORGROUP) lpMsg;

			wsprintf(message, GAMESTRING(STR_NET_LEFT), lp->dpnName.lpszShortNameA);
			NetShowMessage(message, CHAT_SYSTEM);
			utilPrintf("NET: %s\n", message);

			RemoveNetPlayer(lp->dpId);
		}
		break;

/*
		case DPSYS_SESSIONLOST:
		{
			LPDPMSG_SESSIONLOST lp = (LPDPMSG_SESSIONLOST)lpMsg;

			wsprintf(message, "The connection to the game has been broken");
			NetShowMessage(message, CHAT_ERROR);
			utilPrintf("NET: %s\n", message);
		}
		break;
*/

		case DPSYS_HOST:
		{
			wsprintf(message, GAMESTRING(STR_NET_HOST));
			NetShowMessage(message, CHAT_SYSTEM);
			utilPrintf("NET: %s\n", message);
			// we are now the host
			isHost = true;

			NetgameHostGame();
		}
		break;
	}
}

// --------------------------------------------------------------------------
// NetBroadcastMessage & co
// Send broadcast messages (generally game-generated messages) asynchronously

// A send timeout of 500ms seems to work reliably enough over the net with a
// modem and everything. 
int NetBroadcastMessage(void *data, unsigned long size)
{
	HRESULT res;

	res = dplay->SendEx(dpidLocalPlayer, DPID_ALLPLAYERS,
		DPSEND_ASYNC|DPSEND_NOSENDCOMPLETEMSG,
		data, size, 0, 500, NULL, NULL);

	return res;
}

int NetBroadcastUrgentMessage(void *data, unsigned long size)
{
	HRESULT res;

	res = dplay->SendEx(dpidLocalPlayer, DPID_ALLPLAYERS,
		DPSEND_GUARANTEED|DPSEND_ASYNC,
		data, size, 0, 0, NULL, NULL);

	return res;
}

void NetStartGame()
{

	//.. net-specific stuff after the game's started!
	// (i.e. timer synchronisation and stuff)
}

/*	ds - more hacked network shit

const DWORD APPMSG_UPDATEGAME	= 0;			// message type for update of game
const DWORD APPMSG_GAMECHAT     = 1;			// message type for in game chat
const DWORD APPMSG_SYNCHGAME	= 2;			// message type for update of game
const DWORD APPMSG_SYNCHPING	= 3;			// message type for ping message

const DWORD	MAX_STRLEN			= 200;			// max size of a temporary string
unsigned long networkPlay = 0;
DPLAYINFO DPInfo;

// {37097B21-5635-11d3-8FA8-00A0C9DB6D13}
static const GUID DPCHAT_GUID = { 0x37097b21, 0x5635, 0x11d3, { 0x8f, 0xa8, 0x0, 0xa0, 0xc9, 0xdb, 0x6d, 0x13 } };

unsigned long nextSynchAt;
unsigned long actTickCountModifier = 0;
unsigned long synchSpeed = 60 * 1;
unsigned long pingOffset = 40;
unsigned long synchRecovery = 1;

	if(networkPlay && (gameState.mode == INGAME_MODE))
	{
		
		if (!synchedOK)
		{
			if (DPInfo.bIsHost)
				InitialServerSynch();
			else
				InitialPlayerSynch();
			
			InitTiming(60);
			synchedOK = 1;
			nextSynchAt = synchSpeed;
		}
		else
		{
			SendUpdateMessage();

			if (DPInfo.bIsHost)
			{
				if (actFrameCount >	nextSynchAt)
				{
					nextSynchAt = timeInfo.frameCount + synchSpeed;
					SendSynchMessage();
				}
			}
			else
			{
				if ((actFrameCount+pingOffset) > nextSynchAt)
				{
					nextSynchAt = timeInfo.frameCount + synchSpeed;
					SendPingMessage();
				}

				if (synchedFrameCount>0)
				{
					synchedFrameCount-=synchRecovery;
					actTickCountModifier+=synchRecovery;
					timeInfo.tickModifier = -(long)actTickCountModifier;
				}
				else
				{
					synchedFrameCount+=synchRecovery;
					actTickCountModifier-=synchRecovery;
					timeInfo.tickModifier = -(long)actTickCountModifier;
				}
			}
		}
	}
*/


