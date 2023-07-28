/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: netchat.cpp
	Programmer	: David Swift
	Date		: 

	Does the chatting and level select bit at the start of the game

----------------------------------------------------------------------------------------------- */

#include <windows.h>
#include <dplay.h>
#include <dplobby.h>
#include <richedit.h>
#include <stdio.h>

#include <ddraw.h>
#include <d3d.h>
#include <mdx.h>		// because MDX is evil and all-pervasive

#include "islutil.h"
#include "main.h"
#include "../resource.h"

#include "netchat.h"
#include "network.h"
#include "netgame.h"

// common files
#include "layout.h"
#include "lang.h"
#include "frogger.h"

COLORREF systemColor = RGB(0,0x80,0);	// green
COLORREF errorColor = RGB(0xFF,0,0);		// red
HWND hwndChat, hwndChatEdit;
HICON chatIcon, bigChatIcon;

#define NUM_MULTI_LEVELS 4

struct MULTILEVEL { int world, level; };

MULTILEVEL multiLevels[NUM_MULTI_LEVELS] = {
	{ WORLDID_TEST, 0 },
	{ WORLDID_TEST, 1 },
	{ WORLDID_TEST, 2 },
	{ WORLDID_TEST, 3 },
};

struct	MSG_LEVELSEL
{
	UBYTE msg, level;
};

struct	MSG_GAMESETUP
{
	UBYTE id;

	struct {
		DPID dpid;
		UBYTE character, flags, start;
	} players[4];

	short world, level;
};


int ChatHandler(void *data, unsigned long size, NETPLAYER *player);
//void ChatShowMessage(const char* string, CHAT_FORMAT f);

void StartPlayers(HWND hDlg)
{	
	MSG_GAMESETUP msg;

	msg.id = APPMSG_START;

	for (int s=0; s<4; s++)
	{
		msg.players[s].dpid = netPlayerList[s].dpid;
		msg.players[s].character = s;
		msg.players[s].flags = 0;
		msg.players[s].start = s;

		player[s].character = s;
		netPlayerList[s].start = s;
	}

	int level;
	HWND hctrl = GetDlgItem(hDlg, IDC_LEVEL);

	level = SendMessage(hctrl, CB_GETCURSEL, 0, 0);
	level = SendMessage(hctrl, CB_GETITEMDATA, level, 0);

	if (level<0 || level>=NUM_MULTI_LEVELS)
		level = 0;

	msg.id = APPMSG_START;
	msg.level = multiLevels[level].level;
	msg.world = multiLevels[level].world;

	NetBroadcastUrgentMessage(&msg, sizeof(msg));

	player[0].worldNum = msg.world;
	player[0].levelNum = msg.level;
}

void SetupPlayers(MSG_GAMESETUP *msg)
{
	int pl, i;

	for (pl=0; pl<4; pl++)
	{
		for (i=0; i<4; i++)
			if (msg->players[i].dpid == netPlayerList[pl].dpid)
			{
				player[pl].character = msg->players[i].character;
				netPlayerList[pl].start = msg->players[i].start;
				break;
			}
	}

	player[0].worldNum = msg->world;
	player[0].levelNum = msg->level;
}

// -----------------------------------------------------------------------------------------


bool SetupChatDialog(HWND hdlg)
{
	int i;
	hwndChatEdit = GetDlgItem(hdlg, IDC_RICHCHAT);
	hwndChat = hdlg;

	PARAFORMAT pf;

	pf.cbSize = sizeof(pf);
	pf.dwMask = PFM_OFFSET;
	pf.dxOffset = 256;
	SendMessage(hwndChatEdit, EM_SETPARAFORMAT, 0, (LPARAM)&pf);

	if (isHost)
		ChatShowMessage(GAMESTRING(STR_NET_HOST), CHAT_SYSTEM);

	HWND hList = GetDlgItem(hdlg, IDC_LEVEL);

	for (i=0; i<NUM_MULTI_LEVELS; i++)
	{
		LEVEL_VISUAL_DATA *data;
		char *levelname; //[8];
		int index;

		// get level names from worldvisualdata

		data = &worldVisualData[multiLevels[i].world].levelVisualData[multiLevels[i].level];

		if (data->levelOpen)
		{
			//sprintf(levelname, "%d:%d", multiLevels[i].world, multiLevels[i].level);
			levelname = GAMESTRING(data->description_str);

			index = SendMessage(hList, CB_ADDSTRING, 0, (DWORD)levelname);
			SendMessage(hList, CB_SETITEMDATA, index, i);
		}
	}

	chatIcon = (HICON)LoadImage(mdxWinInfo.hInstance, MAKEINTRESOURCE(IDI_FROGGER2), IMAGE_ICON, 16, 16, 0);
	SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM)chatIcon);
	bigChatIcon = (HICON)LoadImage(mdxWinInfo.hInstance, MAKEINTRESOURCE(IDI_FROGGER2), IMAGE_ICON, 32, 32, 0);
	SendMessage(hdlg, WM_SETICON, ICON_BIG, (LPARAM)bigChatIcon);

	SendMessage(hList, CB_SETCURSEL, 0, 0);

	// Timer 1 is used for checking the player list and footling through the message buffers
	SetTimer(hdlg, 1, 250, NULL);

	SetDlgItemText(hdlg, IDCANCEL,	GAMESTRING(STR_PCSETUP_CANCEL));
	SetDlgItemText(hdlg, IDC_START,	GAMESTRING(STR_NET_READY));
	SetDlgItemText(hdlg, IDC_PLAYERSLABEL, GAMESTRING(STR_NET_PLAYERS));
	
	SetWindowText(hdlg, GAMESTRING(STR_NET_STARTAGAME));

	return true;
}

void ChatShowMessage(const char* string, CHAT_FORMAT f)
{
	if (!hwndChatEdit) return;

	CHARFORMAT fmt;
	fmt.cbSize = sizeof(fmt);
	fmt.dwMask = CFM_COLOR|CFM_BOLD;
	fmt.dwEffects = 0;
	
	switch (f)
	{
	case CHAT_NORMAL:	fmt.dwEffects |= CFE_AUTOCOLOR; break;
	case CHAT_SYSTEM:	fmt.dwEffects |= CFE_BOLD; fmt.crTextColor = systemColor; break;
	case CHAT_ERROR:	fmt.dwEffects |= CFE_BOLD; fmt.crTextColor = errorColor; break;
	}

	SendMessage(hwndChatEdit, EM_SETSEL, 0x7FFFFFFF, 0x7FFFFFFF);
	SendMessage(hwndChatEdit, EM_REPLACESEL, FALSE, (DWORD)"\n");
	SendMessage(hwndChatEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&fmt);
	SendMessage(hwndChatEdit, EM_REPLACESEL, FALSE, (DWORD)string);
	SendMessage(hwndChatEdit, EM_SCROLLCARET, 0, 0);

}

void UpdateChatWindow()
{
	int pl, count = 0;
	HWND hlist = GetDlgItem(hwndChat, IDC_PLAYERLIST);

	SendMessage(hlist, LB_RESETCONTENT, 0, 0);

	for (pl=0; pl<4; pl++)
	{
		if (netPlayerList[pl].dpid)
		{
			char dpname[256], name[256];
			DWORD size = 256;

			if (DP_OK == dplay->GetPlayerName(netPlayerList[pl].dpid, &dpname, &size));
			{
				strcpy(name, ((DPNAME*)dpname)->lpszShortNameA);

				if (netPlayerList[pl].score)
					sprintf(name+strlen(name), " (%d win%s!)",
						netPlayerList[pl].score,
						netPlayerList[pl].score==1?"":"s");

				SendMessage(hlist, LB_ADDSTRING, 0, (DWORD)name);

				//SendMessage(hlist, LB_ADDSTRING, 0, (DWORD)((DPNAME*)dpname)->lpszShortNameA);
			}

			count++;
		}
	}

	EnableWindow(GetDlgItem(hwndChat, IDC_LEVEL), isHost);

#ifdef _DEBUG
	EnableWindow(GetDlgItem(hwndChat, IDC_START), isHost);
#else
	EnableWindow(GetDlgItem(hwndChat, IDC_START), isHost && count>=2);
#endif
}


BOOL CALLBACK dialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		SetupChatDialog(hDlg);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SEND:
			{
				char buffer[1024], text[1024];
				HWND hEdit = GetDlgItem(hDlg, IDC_CHATENTER);

				if (!GetWindowText(hEdit, text, 1024))
					break;

				SetWindowText(hEdit, "");
				strcpy(buffer, "<");
				strcat(buffer, playerName);
				strcat(buffer, "> ");
				strncat(buffer, text, 1024);

				ChatShowMessage(buffer, CHAT_NORMAL);

				*(int*)buffer = APPMSG_CHAT;
				strncpy(buffer+4, text, 1020);

				dplay->Send(dpidLocalPlayer, DPID_ALLPLAYERS, DPSEND_GUARANTEED, buffer, strlen(text)+4+1);
				return TRUE;
			}

		case IDCANCEL:
			dplay->Close();
			EndDialog(hDlg, 1);
			return TRUE;

		case IDC_START:
			if (isHost) {
				HRESULT res;
				DWORD	dwSize;

				// ... drumroll please ..
				utilPrintf("NET: starting the game... drumroll please ...\n");

				dwSize = 0;
				res = dplay->GetSessionDesc(NULL, &dwSize);

				UBYTE *buffer = new UBYTE[dwSize];
				res = dplay->GetSessionDesc(buffer, &dwSize);

				if (res == DP_OK)
				{
					DPSESSIONDESC2 *dpsd = (DPSESSIONDESC2*)buffer;

					dpsd->dwFlags |= DPSESSION_NEWPLAYERSDISABLED|DPSESSION_JOINDISABLED;
					res = dplay->SetSessionDesc(dpsd, 0);
					if (res)
						utilPrintf("Net: couldn't set session desc: %s\n", GetDirectPlayError(res));
				}
				else
					utilPrintf("Net: couldn't get session desc: %s\n", GetDirectPlayError(res));

				delete buffer;

				StartPlayers(hDlg);

				EndDialog(hDlg, 0);
			}
			return TRUE;

		case IDC_LEVEL:
			if (HIWORD(wParam)==CBN_SELCHANGE)
			{
				MSG_LEVELSEL msg = { APPMSG_LEVELSEL, 0 };
				msg.level = SendDlgItemMessage(hDlg, IDC_LEVEL, CB_GETCURSEL, 0, 0);

				NetBroadcastUrgentMessage(&msg, sizeof(msg));
			}
			return TRUE;
		}
		break;

	case WM_TIMER:
		NetProcessMessages();
		UpdateChatWindow();
		return TRUE;

	case WM_DESTROY:
		KillTimer(hDlg, 1);
		DestroyIcon(chatIcon);
		DestroyIcon(bigChatIcon);
		return TRUE;
	}

	return FALSE;
}

int RunNetChatWindow(HWND parent)
{
	int res;
	LoadLibrary("Riched32.dll");

	// re-enable join and new player, if previously disabled
	DPSESSIONDESC2 dpsd;
	ZeroMemory(&dpsd, sizeof(dpsd));
	dpsd.dwSize = sizeof(dpsd);
	dpsd.dwFlags = 0;;

	dplay->SetSessionDesc(&dpsd, 0);

	NetInstallMessageHandler(ChatHandler);
	res = DialogBox(mdxWinInfo.hInstance, MAKEINTRESOURCE(IDD_MULTI_START), parent, dialogProc);
	return res == 0;
}

int ChatHandler(void *data, unsigned long size, NETPLAYER *player)
{
	bool wasHost = false;

	if (isHost && !wasHost)
	{
		DWORD dwSize = 0;
		HRESULT res = dplay->GetSessionDesc(NULL, &dwSize);

		UBYTE *buffer = new UBYTE[dwSize];
		res = dplay->GetSessionDesc(buffer, &dwSize);

		if (res == DP_OK)
		{
			DPSESSIONDESC2 *dpsd = (DPSESSIONDESC2*)buffer;

			dpsd->dwFlags &= ~(DPSESSION_NEWPLAYERSDISABLED|DPSESSION_JOINDISABLED);
			res = dplay->SetSessionDesc(dpsd, 0);
			if (res)
				utilPrintf("Net: couldn't set session desc: %s\n", GetDirectPlayError(res));
		}
		else
			utilPrintf("Net: couldn't get session desc: %s\n", GetDirectPlayError(res));
	}

	switch (*(unsigned char*)data)
	{
	case APPMSG_LEVELSEL:
		{
			MSG_LEVELSEL *msg = (MSG_LEVELSEL*)data;
			SendDlgItemMessage(hwndChat, IDC_LEVEL, CB_SETCURSEL, msg->level, 0);
		}
		return 0;

	case APPMSG_CHAT:
		{
			char buffer[1024], name[256];
			DWORD size = 256;

			dplay->GetPlayerName(player->dpid, &name, &size);
			
			strcpy(buffer, "<");
			strcat(buffer, ((DPNAME*)name)->lpszShortNameA);
			strcat(buffer, "> ");
			strncat(buffer, ((char*)data) + 4, 1024);

			ChatShowMessage(buffer, CHAT_NORMAL);
		}
		return 0;

	case APPMSG_START:
		{
			SetupPlayers((MSG_GAMESETUP*)data);

			utilPrintf("NET: starting the joined game... drumroll please ...\n");
			EndDialog(hwndChat, 0);
		}
		return 0;
	}

	wasHost = isHost;

	return 1;
}
