/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: netgame.cpp
	Programmer	: David Swift
	Date		:

----------------------------------------------------------------------------------------------- */

// Net headers
#include "network.h"
#include "netgame.h"
#include "netrace.h"

// PC headers
#include "main.h"
#include "islpad.h"
#include "islutil.h"
#include "fadeout.h"

// Common headers
#include "frontend.h"
#include "game.h"
#include "frogger.h"
#include "frogmove.h"
#include "cam.h"
#include "enemies.h"
#include "specfx.h"
#include "overlays.h"
#include "lang.h"
#include "multi.h"
#include "hud.h"

#define UPDATE_PERIOD		10		// in 60ths of a second (actFrameCounts)
#define PING_PERIOD			200		// how often we should ping once we're synchronised

#define STARTGAME_COUNT		(4*60)	// how much time we allow for the countdown

unsigned long	nextUpdate = 0, nextPing = 0, gameStartTime = 0;
bool			hostSync, wasSync, hostReady, gameReady;
TEXTOVERLAY		*netMessage, *netStatus;

NETGAME_LOOP		netgameLoopFunc = NULL;
NET_MESSAGEHANDLER	netgameHandler = NULL;

#define NETMSG_TIMEOUT			1000 //ms

char *netStatusText = NULL;
DWORD netStatusTime = 0;

#define NETSTATUS_TIMEOUT		3000
#define NETSTATUS_FADEOUT		1000

// ---------------------------------------------------------------------------------------
// Update messages
// all of these are identified by a UBYTE message ID tag entry.. member.. thing

#define BIT(x)	(1<<(x))

#define NETUPD_SUPERHOP		BIT(0)
#define NETUPD_DOUBLEJUMP	BIT(1)
#define NETUPD_DEAD			BIT(4)
#define NETUPD_SAFE			BIT(5)
#define NETUPD_READY		BIT(6)

typedef struct
{
	unsigned char	type;
	short			fromTile, toTile;
	DWORD			tickCount;		// When in game frames this event happened.
	DWORD			flags;

} MSG_UPDATEGAME,*LPMSG_UPDATEGAME;

struct	MSG_PING
{
	UBYTE appmsg_ping;
	unsigned long time;
};

struct	MSG_PINGREPLY
{
	UBYTE appmsg_pingreply;
	unsigned long reply, time0;
};

struct	MSG_STARTGAME
{
	UBYTE appmsg_start;
	unsigned long	gameStartTime;		// when the game is due to start, according to actFrameCount
};

struct MSG_WONGAME
{
	UBYTE appmsg_wongame;
	unsigned long	gameWonTime;
};




#define GetTileNo(t) (((DWORD)tile-(DWORD)firstTile)/sizeof(GAMETILE))

int WaitForGameReady();
int SendUpdateMessage();
void SendPing();
int NetgameMessageDispatch(void *data, unsigned long size, NETPLAYER *player);
void NetgameBegin();

void OnPing(MSG_PING*ping, NETPLAYER *player);
void OnPingReply(MSG_PINGREPLY* pingreply, NETPLAYER *player);
void OnUpdate(LPMSG_UPDATEGAME lpMsg, NETPLAYER *pl);
void OnDeath(NETPLAYER *player);

// ---------------------------------------------------------------------------------

/*	--------------------------------------------------------------------------------
	Function		: NetgameStartGame
	Purpose			: starts a network game; for now, always a race
	Parameters		: 
	Returns			: 
*/
void NetgameStartGame()
{
	int pl;

	gameState.mode = INGAME_MODE;
	gameState.multi = MULTIREMOTE;
	gameState.single = ARCADE_MODE;
	gameState.difficulty = DIFFICULTY_NORMAL;

	for (pl=1; pl<NUM_FROGS; pl++)
	{
		if (!netPlayerList[pl].dpid)
			break;
	}
	NUM_FROGS = pl;

	NetInstallMessageHandler(NetgameMessageDispatch);
	NetRaceInit();

	GTInit( &modeTimer, 1 );
	InitLevel(player[0].worldNum, player[0].levelNum);

	// Reorder start tiles for the players
	
	GAMETILE *startTiles[4];
	memcpy(startTiles, gTStart, sizeof(GAMETILE*)*4);

	if (isHost)
	{
		hostSync = hostReady = true;
		UBYTE msg = APPMSG_HOSTREADY;
		NetBroadcastUrgentMessage(&msg, 1);
	}
	else
	{
		for (pl=0; pl<NUM_FROGS; pl++)
		{
			gTStart[pl] = startTiles[netPlayerList[pl].start];
			SetFroggerStartPos(gTStart[pl], pl);
		}

		hostSync = hostReady = false;
	}

	netStatusText = new char[1024];
	netStatusText[0] = 0;
	netStatus = CreateAndAddTextOverlay(32, 3800, netStatusText, 0, 0, fontSmall, 0);

	NetgameBegin();
}

void NetgameBegin()
{
	int pl;

	nextUpdate = 0;
	gameReady = false;

	for (pl=1; pl<NUM_FROGS; pl++)
	{
		netPlayerList[pl].lastUpdateMsg = 0;
		//netPlayerList[pl].isReady = false;
	}

	if (isHost)
	{
		unsigned char msg;
		
		msg = APPMSG_READY;
		NetBroadcastUrgentMessage(&msg, 1);

		multiHud.centreText->text = GAMESTRING(STR_NET_WAITINGPLAYERS);
	}
	else
	{
		hostSync = wasSync = false;
	}
}

void NetgameHostGame()
{
	if (!gameReady)
	{
		unsigned char msg;

		msg = APPMSG_HOSTREADY;
		NetBroadcastUrgentMessage(&msg, 1);

		hostReady = hostSync = true;
	}
}


void NetgameStatusMessage(const char* msg)
{
	if (netStatus)
	{
		netStatusTime = timeInfo.tickCount;
		strncpy(netStatusText, msg, 1023);

		netStatus->draw = 1;
		netStatus->a = 255;
	}
}

/*	--------------------------------------------------------------------------------
	Function		: NetgameRun
	Purpose			: network-game specific version of GameLoop()

	Updates only the bits of the game that we need for multiplayer; makes sure everything's
	ready and sychronised before running the game proper
*/

void NetgameGameloop()
{
	if (timeInfo.tickCount > netStatusTime+NETSTATUS_TIMEOUT)
		netStatus->draw = 0;
	else if (timeInfo.tickCount > netStatusTime+NETSTATUS_FADEOUT)
		netStatus->a = (255*(NETSTATUS_TIMEOUT-timeInfo.tickCount+netStatusTime))/(NETSTATUS_TIMEOUT-NETSTATUS_FADEOUT);

	if (gameReady)
	{
		// Make sure we don't send update messages EVERY frame, in case we're running at, like,
		// 8 million frames per second, or some junk
		if (actFrameCount >= nextUpdate)
		{
			SendUpdateMessage();
			nextUpdate += UPDATE_PERIOD;
		}

		if (actFrameCount > nextPing)
		{
			SendPing();
			nextPing += PING_PERIOD;
		}

		for (int i=1; i<NUM_FROGS; i++)
		{
			FroggerHop(i);
			if (player[i].jumpTime > 4096)
			{
				//if (gameSpeed > 0)
				player[i].jumpTime = -1;
				SetVectorSS(&frog[i]->actor->position, &currTile[i]->centre);
				actorAnimate(frog[i]->actor, FROG_ANIM_BREATHE, YES, YES, FROG_BREATHE_SPEED, NO);
			}
		}

		bool wasDead = player[0].frogState & FROGSTATUS_ISDEAD;

		if( endTimer.time ) // If finished the race then wait before replaying
		{
			GTUpdate( &endTimer, -1 );

			if( !endTimer.time )
			{
				multiHud.centreText->draw = 0;

				if (matchWinner >= 0)
					netPlayerList[matchWinner].score++;	// yay!

				StartMultiWinGame( );
			}

			return;
		}
		else if (netgameLoopFunc) netgameLoopFunc();

		// generate a death message if the player ever *becomes* dead
		if (player[0].frogState & FROGSTATUS_ISDEAD && !wasDead)
			NetgameDeath();

	}
	else
	{
		WaitForGameReady();
	}

	if (hostSync)
	{
		UpdateEnemies();
		UpdateSpecialEffects();
	}
}

void NetgameRun()
{

	if (!dplay) return;

	wasSync = hostSync;
	NetProcessMessages();

	// ... otherwise ...

	if (gameState.mode == INGAME_MODE)
	{
		NetgameGameloop();
	
		frameCount++;
		player[0].inputPause = 0;
	}
	else
	{
		GameLoop();

		if (gameState.mode == INGAME_MODE)
			NetgameBegin();
	}
}



int WaitForGameReady()
{
	if (!hostReady)
	{
		multiHud.centreText->text = GAMESTRING(STR_NET_WAITINGHOST);
		return 0;
	}

	if (!hostSync)
	{
		multiHud.centreText->text = GAMESTRING(STR_NET_SYNCH);

		SendPing();
		return 0;
	}
	else if (!wasSync)
	{
		// We've just become synchronised, so tell everybody else
		unsigned char msg = APPMSG_READY;
		NetBroadcastUrgentMessage(&msg, 1);

		UpdateAllEnemies();	// sync all our enemies with the host

		utilPrintf("Net: synchronised with host okay\n");

		multiHud.centreText->text = GAMESTRING(STR_NET_WAITINGPLAYERS);
	}

	gameReady = true;

	for (int pl=1; pl<NUM_FROGS; pl++)
	{
		if (!netPlayerList[pl].dpid) break;
		
		if (!netPlayerList[pl].isReady)
		{
			gameReady = false;
			break;
		}
	}

	if (gameReady)
	{
		utilPrintf("Net: all players ready\n");

		//multiHud.centreText->draw = 0;

		if (isHost)
		{
			MSG_STARTGAME start;

			gameStartTime = actFrameCount + STARTGAME_COUNT;

			start.appmsg_start = APPMSG_START;
			start.gameStartTime = gameStartTime;	// game starts in five seconds

			NetBroadcastUrgentMessage(&start, sizeof(start));

			utilPrintf("Net: starting game at T%d\n", gameStartTime);
		}

		return 1;
	}
	else
		return 0;
}

// ------------------------------------------------------------------------
// OnPing()
// Replies to a PING message, returning the current time on THIS machine and
// the timestamp (unchanged) from the other machine.

void OnPing(MSG_PING*ping, NETPLAYER *player)
{
	MSG_PINGREPLY reply;

	reply.appmsg_pingreply = APPMSG_PINGREPLY;
	reply.reply = timeGetTime()-timeInfo.firstTicks;
	reply.time0 = ping->time;

	dplay->Send(dpidLocalPlayer, player->dpid, 0, &reply, sizeof(reply));
}

void SendPing()
{
	if (!isHost)
	{
		MSG_PING ping;

		ping.appmsg_ping = APPMSG_PING;
		ping.time = timeGetTime();

		dplay->Send(dpidLocalPlayer, DPID_SERVERPLAYER, 0, &ping, sizeof(ping));
	}
}

// ------------------------------------------------------------------------
// OnPingReply()
// Synchronises clocks with another machine; sets the timeinfo so that actFrameCount
// on this machine will be the same (give or take a bit) as the other machine

void OnPingReply(MSG_PINGREPLY* pingreply, NETPLAYER *player)
{
	unsigned long currTime = timeGetTime();
	long latency = ((long)currTime - (long)pingreply->time0) / 2;

	timeInfo.tickCount = (pingreply->reply + latency);
	timeInfo.firstTicks = (long)currTime - (long)timeInfo.tickCount;

	utilPrintf("Net: clock sync, count was:\t%d\n", actFrameCount);

	gameSpeed = 2048;
	actFrameCount = timeInfo.tickCount*(timeInfo.frameSpeed/1000.0F);

	utilPrintf("Net: actframecount is now:\t%d\n", actFrameCount);

	hostSync = true;
}

/*	--------------------------------------------------------------------------------
	Function		: NetgameMessageDispatch
	Purpose			: callback to dispatch net messages to the right function
	Parameters		: data, size, player
	Returns			: error code
*/

int NetgameMessageDispatch(void *data, unsigned long size, NETPLAYER *player)
{
	if (netgameHandler)
		if (netgameHandler(data, size, player) == 0)
			return 0;

	switch (*(unsigned char*)data)
	{
	case APPMSG_UPDATE:
		OnUpdate((LPMSG_UPDATEGAME)data, player);
		break;

	case APPMSG_PING:
		OnPing((MSG_PING*)data, player);
		break;

	case APPMSG_PINGREPLY:
		if (!hostSync)
			OnPingReply((MSG_PINGREPLY*)data, player);
		break;

	case APPMSG_READY:
		utilPrintf("Net: Player ID %08x is ready\n", player->dpid);
		player->isReady = true;
		break;

	case APPMSG_HOSTREADY:
		utilPrintf("Net: Host is ready to sync\n");
		hostReady = true;

		if (hostSync)
		{
			utilPrintf("Net: synching to new host..\n");
			hostSync = false;
		}
		break;

	case APPMSG_DEATH:
		OnDeath(player);
		break;

	case APPMSG_START:
		gameStartTime = ((MSG_STARTGAME*)data)->gameStartTime;
		break;

	default:
		return -1;
	}

	return 0;
}


/*	--------------------------------------------------------------------------------
	Function		: SendUpdateMessage
	Purpose			: sends an update message
	Parameters		: Player number
	Returns			: 
*/

int SendUpdateMessage()
{
	MSG_UPDATEGAME updateMessage;
	HRESULT	hRes;
	GAMETILE *tile = (destTile[0]?destTile[0]:currTile[0]);

	//if (!padData.digital[0])
	//	return DPERR_ABORTED;

	// build message	
	updateMessage.type = APPMSG_UPDATE;
	//updateMessage.buttons = padData.digital[0];
	updateMessage.toTile = GetTileNo(tile);
	updateMessage.tickCount = timeInfo.tickCount;
	
	updateMessage.flags =
		(player[0].safe.time?NETUPD_SAFE:0) |
		(player[0].dead.time?NETUPD_DEAD:0) |
		(mpl[0].ready?NETUPD_READY:0);
	
	if (player[0].hasDoubleJumped)
		updateMessage.flags |= NETUPD_DOUBLEJUMP;
	else if (player[0].isSuperHopping)
		updateMessage.flags |= NETUPD_SUPERHOP;

	return NetBroadcastMessage(&updateMessage, sizeof(updateMessage));
}


/*	--------------------------------------------------------------------------------
	Function		: OnUpdate
	Purpose			: interpret a game update from the network
	Parameters		: 
	Returns			: 
*/
void OnUpdate(LPMSG_UPDATEGAME lpMsg, NETPLAYER *pl)
{
	if (lpMsg->tickCount < pl->lastUpdateMsg)
		return;
	
	int i = GetPlayerNumFromID(pl->dpid);
	GAMETILE *tile = &firstTile[lpMsg->toTile];

//	padData.digital[i] = lpMsg->buttons;
//	padData[i].tickOn = lpMsg->tickCount;

//	dp("Recieved message for tile %lu\n",lpMsg->tileNum);

	// Calculate frog jump if it's not 'in' the current tile
	if (tile != currTile[i])
	{
		FVECTOR fv;

		if (currTile[i])
		{
			SubVectorFSS(&fv, &tile->centre, &currTile[i]->centre);
			MakeUnit(&fv);
			OrientateFS(&frog[i]->actor->qRot, &fv, &currTile[i]->normal);
		}

		//SetVector(&frog[i]->actor->position, &tile->centre);
		currTile[i] = tile;
		frog[i]->draw = 1;

		if (player[i].dead.time)
		{
			SetVectorSS(&frog[i]->actor->position, &tile->centre);
		}
		else
		{
			if (lpMsg->flags & NETUPD_SUPERHOP)
			{
				CalculateFrogJumpS(&frog[i]->actor->position, &tile->centre, &tile->normal, superhopHeight, superHopFrames, i);
				actorAnimate(frog[i]->actor, FROG_ANIM_SUPERHOP, NO, NO, (player[i].jumpSpeed*15)>>4, 0);
			}
			else if (lpMsg->flags & NETUPD_DOUBLEJUMP)
			{
				CalculateFrogJumpS(&frog[i]->actor->position, &tile->centre, &tile->normal, doublehopHeight, doubleHopFrames, i);
				actorAnimate(frog[i]->actor,FROG_ANIM_SOMERSAULT,NO,NO,75,0);
			}
			else
			{
				CalculateFrogJumpS(&frog[i]->actor->position, &tile->centre, &tile->normal, hopHeight, standardHopFrames, i);
				actorAnimate(frog[i]->actor,FROG_ANIM_HOP,NO,NO,frogAnimSpeed,0);
			}
		}
	}

	player[i].safe.time = (lpMsg->flags&NETUPD_SAFE)?1:0;
	player[i].dead.time = (lpMsg->flags&NETUPD_DEAD)?1:0;

	mpl[i].ready = (lpMsg->flags&NETUPD_READY) != 0;

	pl->lastUpdateMsg = lpMsg->tickCount;
}

/*	--------------------------------------------------------------------------------
	Function		: OnDeath
	Purpose			: Kills a multiplayer frog (adds penalty, sets win conditions, whatever's necessary)
	Parameters		: 
	Returns			: 
*/
void OnDeath(NETPLAYER *player)
{
	int pl = GetPlayerNumFromID(player->dpid);
	KillMPFrog(pl);
}

void NetgameDeath()
{
	UBYTE msg = APPMSG_DEATH;
	NetBroadcastUrgentMessage(&msg, 1);
}
