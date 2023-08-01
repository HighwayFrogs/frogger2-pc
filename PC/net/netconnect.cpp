/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: netconnect.cpp
	Programmer	: David Swift
	Date		:

	Based loosely around the MS-supplied dpconnect.cpp only marginally less obnoxious
	No, only kidding, it's terrible really.

----------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <windows.h>
#include <dplay.h>
#include <dplobby.h>

#include <ddraw.h>
#include <d3d.h>
#include <mdx.h>		// because MDX is evil and all-pervasive

#include "islutil.h"
#include "main.h"
#include "../resource.h"
#include "netconnect.h"
#include "network.h"
#include "netchat.h"

#include "lang.h"

#define MAX_PLAYER_NAME     14
#define MAX_SESSION_NAME    256

// from dpconnect.cpp ...

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }

struct DPSessionInfo
{
	GUID guidSession;
	TCHAR szSession[MAX_SESSION_NAME];
	DPSessionInfo* next;
};

DPSessionInfo               DPSIHead;
BOOL                        searchingForSessions;

IDirectPlayLobby3A			*dpLobby = NULL;


// {D0D2A940-5803-11d4-8832-00A0244E2381}
static const GUID Frogger2_GUID =
{ 0xd0d2a940, 0x5803, 0x11d4, { 0x88, 0x32, 0x0, 0xa0, 0x24, 0x4e, 0x23, 0x81 } };

// ------ Local functions ---------

bool SetupNetworking(HWND hwnd);
bool SelectProvider(HWND hwnd);
bool FindNetGame(HWND hwnd);
HRESULT CreateNetGame(HWND hDlg);
HRESULT JoinNetGame(HWND hDlg);
bool CheckLobby(HWND hwnd);

/*	--------------------------------------------------------------------------------
	Function	: SetupNetworking
	Purpose		: sets up DirectPlay
	Parameters	: 
	Returns		: 
*/
bool SetupNetworking()
{
	HRESULT res;
	HKEY hkey;

	// init COM library
	if ((res = CoInitialize(NULL)) != S_OK)
	{
		utilPrintf("SetupNetworking(): Failed initialising COM!\n");
		return false;
	}

	if ((res = CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_ALL,
		IID_IDirectPlay4A, (void**)&dplay)) != S_OK)
	{
		utilPrintf("SetupNetworking(): Failed creating DirectPlay4A interface\n");
		return false;
	}

	// TODO: move to ini
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_KEY, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		DWORD len;

		len = 16;
		RegQueryValueEx(hkey, "netplayer", NULL, NULL, (unsigned char*)playerName, &len);

		len = 128;
		RegQueryValueEx(hkey, "netgame", NULL, NULL, (unsigned char*)sessionName, &len);

		RegCloseKey(hkey);
	}

    // Setup the g_DPSIHead circular linked list
    ZeroMemory( &DPSIHead, sizeof( DPSessionInfo ) );
    DPSIHead.next = &DPSIHead;

	// ... set any other network-spiffic stuff up too ...

	return true;
}

/*	--------------------------------------------------------------------------------
	Function	: ShutdownNetworkGame
	Purpose		: 
	Parameters	: 
	Returns		: 
*/

void ShutdownNetworkGame()
{
	// .. shut down anything networky ..

	if (dplay)
	{
		dplay->Close();
		dplay->Release();
	}

}



void SaveNetgameSetup()
{
	HKEY hkey;
	HRESULT res;

	if ((res = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_KEY, 0, 0, 0, KEY_WRITE, NULL, &hkey, NULL)) == ERROR_SUCCESS)
	{
		RegSetValueEx(hkey, "netplayer", NULL, REG_SZ, (unsigned char*)playerName, strlen(playerName) + 1);
		RegSetValueEx(hkey, "netgame", NULL, REG_SZ, (unsigned char*)sessionName, strlen(sessionName) + 1);
	}
}

/*	--------------------------------------------------------------------------------
	Function	: StartNetworkGame
	Purpose		: sets up and configures a network game
	Parameters	: flags
	Returns		: success
*/
int StartNetworkGame(HWND hwnd, int flag)
{
	bool connected = (dplay != NULL);

	if (!connected)
	{
		if (!SetupNetworking())	// if we can't set networking up at all, fail
			return 0;

		// if the flag's set, we're definitely running networked
		if (flag)
		{
			// .. so we need to get LAN/modem/etc. settings
			
			if (SelectProvider(hwnd))
				if (FindNetGame(hwnd))
					connected = true;
		}
		else
		{
			if (CheckLobby(hwnd))
				connected = true;
		}
	}

	if (connected)
	{
		SaveNetgameSetup();

		SetupNetPlayerList();
		return 1;
	}

	// otherwise, fail
	ShutdownNetworkGame();
	return 0;
}

/*	--------------------------------------------------------------------------------
	Function	: CheckLobby
	Purpose		: checks if we're running through a lobby and connects if we are
	Parameters	: 
	Returns		: success
*/

bool CheckLobby(HWND hwnd)
{
	HRESULT hr;
	DWORD dwSize = 0;
	DPLCONNECTION *dplConnection;

	if (FAILED(hr = CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER,
        IID_IDirectPlayLobby3A, (VOID**)&dpLobby)))
    return hr;

	hr = dpLobby->GetConnectionSettings(0, NULL, &dwSize);
	if (FAILED(hr) && (DPERR_BUFFERTOOSMALL != hr))
	{
		dpLobby->Release();

		if (DPERR_NOTLOBBIED == hr)
			utilPrintf("net: Not running lobbied\n");
		else
			utilPrintf("net: Error getting lobby connection settings: %d\n");

		return false; 
	}

	dplConnection = (DPLCONNECTION*)new UBYTE[dwSize];
	if (FAILED(hr = dpLobby->GetConnectionSettings(0, dplConnection, &dwSize)))
	{
		utilPrintf("net: Error getting lobby connection settings: %d\n", hr);
		delete dplConnection;
		return false;
	};

	// Set our session flags
	dplConnection->lpSessionDesc->dwFlags |= 
			DPSESSION_KEEPALIVE|DPSESSION_MIGRATEHOST|DPSESSION_DIRECTPLAYPROTOCOL;
 
	// Let lobby know our connection flags
	if ( FAILED( hr = dpLobby->SetConnectionSettings( 
			0, 0, dplConnection ) ) )
		return false;

	isHost = (dplConnection->dwFlags == DPLCONNECTION_CREATESESSION);

	if ( FAILED( hr = dpLobby->ConnectEx( 0, IID_IDirectPlay4A, 
        (VOID**)&dplay, NULL ) ) )
    return false;

	// Create a player
	DPNAME dpname;
	memcpy(&dpname, dplConnection->lpPlayerName, sizeof(DPNAME));
	
	if ( FAILED( hr = dplay->CreatePlayer( &dpidLocalPlayer, &dpname, 
		NULL, NULL, 0, isHost?DPPLAYER_SERVERPLAYER:0) ) )
		return hr;
	
	delete dplConnection;

	return true;
}

/*	--------------------------------------------------------------------------------
	Function	: SelectProvider
	Purpose		: Enumerate and select a DirectPlay provider
	Parameters	: 
	Returns		: success
*/
BOOL CALLBACK EnumConnections(LPCGUID lpguidSP, LPVOID connection, DWORD connectionSize,
							  const DPNAME* name, DWORD flags, LPVOID context)
{
	int index;
	HRESULT res;
	HWND list = (HWND)context;
	LPDIRECTPLAY4A dp;

	if ((res = CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_ALL,
		IID_IDirectPlay4A, (void**)&dp)) != S_OK)
		return FALSE;
	
	res = dp->InitializeConnection(connection, 0);
	dp->Release();	// don't need any more..

	if (res != DP_OK) return TRUE;

	index = (int)SendMessage(list, LB_ADDSTRING, 0, (LPARAM)name->lpszShortNameA);

	BYTE* data = new BYTE[connectionSize];
	memcpy(data, connection, connectionSize);

	SendMessage(list, LB_SETITEMDATA, index, (LPARAM)data);

	return TRUE;
}

BOOL CALLBACK dlgProvider(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int count, index;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			HWND hctrl = GetDlgItem(hdlg, IDC_CONNECTIONLIST);
			dplay->EnumConnections(&Frogger2_GUID, EnumConnections, (void*)hctrl,
				DPCONNECTION_DIRECTPLAY | DPCONNECTION_DIRECTPLAYLOBBY);

			SendMessage(GetDlgItem(hdlg, IDC_CONNECTIONLIST), LB_SETCURSEL, 0, 0);

			hctrl = GetDlgItem(hdlg, IDC_PLAYERNAME);
			SetWindowText(hctrl, playerName);
			SendMessage(hctrl, EM_SETSEL, 0, -1);
			SendMessage(hctrl, EM_LIMITTEXT, 15, 0);

			SetWindowText(hdlg, GAMESTRING(STR_NET_NETWORKGAME));

			SetDlgItemText(hdlg, IDC_PLAYERLABEL, GAMESTRING(STR_NET_PLAYERNAME));
			SetDlgItemText(hdlg, IDC_CONNECTIONLABEL, GAMESTRING(STR_NET_CONNECTION));
			SetDlgItemText(hdlg, IDOK, GAMESTRING(STR_PCSETUP_OK));
			SetDlgItemText(hdlg, IDCANCEL, GAMESTRING(STR_PCSETUP_CANCEL));
		}		
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				HWND hctrl = GetDlgItem(hdlg, IDC_CONNECTIONLIST);
				index = SendMessage(hctrl, LB_GETCURSEL, 0, 0);

				void *connection = (void*)SendMessage(hctrl, LB_GETITEMDATA, index, 0);

				if (CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_ALL,
					IID_IDirectPlay4A, (void**)&dplay) == S_OK)
				{
					dplay->InitializeConnection(connection, 0);
				}

				EndDialog(hdlg, true);
				return true;
			}

		case IDCANCEL:
			EndDialog(hdlg, false);
			return true;

		case IDC_PLAYERNAME:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				int len = SendMessage((HWND)lParam, WM_GETTEXT, 32, (LPARAM)&playerName);
				EnableWindow(GetDlgItem(hdlg, IDOK), (len>0));
				return true;
			}
		}
		break;

	case WM_DESTROY:
		{
			// clean up connection info
			HWND hctrl = GetDlgItem(hdlg, IDC_CONNECTIONLIST);
			count = SendMessage(hctrl, LB_GETCOUNT, 0, 0);
			for(index=0; index<count; index++)
			{
				void *ptr = (VOID*)SendMessage(hctrl, LB_GETITEMDATA, index, 0 );
				if (ptr) delete [] ptr;
			}
		}
		return true;
	}
	return false;
}

bool SelectProvider(HWND hwnd)
{
	return DialogBox(mdxWinInfo.hInstance, MAKEINTRESOURCE(IDD_MULTI_CONNECT), hwnd, (DLGPROC) dlgProvider);
}


/*	--------------------------------------------------------------------------------
	Function	: FindNetGame
	Purpose		: Find a network game of Frogger2
	Parameters	: 
	Returns		: success
*/

BOOL CALLBACK EnumSessions(const DPSESSIONDESC2 *pdpsd, LPDWORD lptimeout, DWORD flags, LPVOID context)
{
    DPSessionInfo* pDPSINew = NULL;

	if (flags & DPESC_TIMEDOUT || pdpsd->dwFlags & DPSESSION_JOINDISABLED)
		return FALSE;

    // Found a good session, save it
    pDPSINew = new DPSessionInfo; 
    if( NULL == pDPSINew )
        return FALSE;

    ZeroMemory( pDPSINew, sizeof(DPSessionInfo) );

    // Copy the information into pDPSINew
    pDPSINew->guidSession = pdpsd->guidInstance;
    sprintf( pDPSINew->szSession, "%s (%d/%d)", pdpsd->lpszSessionNameA, 
             pdpsd->dwCurrentPlayers, pdpsd->dwMaxPlayers );

    // Add pDPSINew to the circular linked list, g_pDPSIFirst
    pDPSINew->next = DPSIHead.next;
    DPSIHead.next = pDPSINew;

	return TRUE;
}

void InitSessionsDialog(HWND hdlg)
{
    HWND          hWndListBox = GetDlgItem( hdlg, IDC_GAMELIST );

    SendMessage( hWndListBox, LB_RESETCONTENT, 0, 0 );
}

void SessionCleanup(HWND hdlg)
{
    DPSessionInfo* pDPSI = DPSIHead.next;
    DPSessionInfo* pDPSIDelete;

    while ( pDPSI != &DPSIHead )
    {
        pDPSIDelete = pDPSI;       
        pDPSI = pDPSI->next;

        SAFE_DELETE( pDPSIDelete );
    }

    // Re-link the DPSIHead circular linked list
    DPSIHead.next= &DPSIHead;
}

bool ShowNetGames(HWND hDlg, bool start)
{
    HRESULT        hr;
    DPSESSIONDESC2 dpsd;
    DPSessionInfo* pDPSISelected = NULL;
    int            nItemSelected;
    GUID           guidSelectedSession;
	HWND hlist = GetDlgItem(hDlg, IDC_GAMELIST);
    BOOL           bFindSelectedGUID;
    BOOL           bFoundSelectedGUID;

	ZeroMemory(&dpsd, sizeof(DPSESSIONDESC2));
	dpsd.dwSize = sizeof(DPSESSIONDESC2);
	dpsd.guidApplication = Frogger2_GUID;

	if (start)
	{
	    // Try to keep the same session selected unless it goes away or 
		// there is no real session currently selected
		bFindSelectedGUID  = FALSE;
		bFoundSelectedGUID = TRUE;

		nItemSelected = SendMessage( hlist, LB_GETCURSEL, 0, 0 );
		if( nItemSelected != LB_ERR )
		{
			pDPSISelected = (DPSessionInfo*) SendMessage( hlist, LB_GETITEMDATA, 
														  nItemSelected, 0 );
			if( pDPSISelected != NULL )
			{
				guidSelectedSession = pDPSISelected->guidSession;
				bFindSelectedGUID = TRUE;
			}
		}
		
		// Tell listbox not to redraw itself since the contents are going to change
		SendMessage( hlist, WM_SETREDRAW, FALSE, 0 );

		SessionCleanup(hDlg);

		hr = dplay->EnumSessions(&dpsd, 0, &EnumSessions, GetDlgItem(hDlg, IDC_GAMELIST),
			(start)?DPENUMSESSIONS_ASYNC|DPENUMSESSIONS_ALL:DPENUMSESSIONS_STOPASYNC);
		if( FAILED(hr) )
		{
			if( hr == DPERR_USERCANCEL )
			{
				// The user canceled the DirectPlay connection dialog, 
				// so stop the search
				if( searchingForSessions )
				{
					SendMessage( hDlg, WM_COMMAND, IDC_SEARCH, 0 );
					CheckDlgButton( hDlg, IDC_SEARCH, BST_UNCHECKED );
				}

				return S_OK;
			}
			else 
			{
				InitSessionsDialog( hDlg );
				if ( hr == DPERR_CONNECTING )
					return S_OK;

				return hr;
			}
		}

		// Add the enumerated sessions to the listbox
		if( DPSIHead.next!= &DPSIHead )
		{
			// Clear the contents from the list box and enable the join button
			SendMessage( hlist, LB_RESETCONTENT, 0, 0 );
			EnableWindow( GetDlgItem( hDlg, IDC_JOIN ), TRUE );
        
			DPSessionInfo* pDPSI = DPSIHead.next;
			while ( pDPSI != &DPSIHead )
			{
				// Add session to the list box
				int nIndex = SendMessage( hlist, LB_ADDSTRING, 0, 
										  (LPARAM)pDPSI->szSession );
				SendMessage( hlist, LB_SETITEMDATA, nIndex, (LPARAM)pDPSI );

				if( bFindSelectedGUID )
				{
					// Look for the session the was selected before
					if( pDPSI->guidSession == guidSelectedSession )
					{
						SendMessage( hlist, LB_SETCURSEL, nIndex, 0 );
						bFoundSelectedGUID = TRUE;
					}
				}

				pDPSI = pDPSI->next;
			}

			if( !bFindSelectedGUID || !bFoundSelectedGUID )
				SendMessage( hlist, LB_SETCURSEL, 0, 0 );
		}
		else
		{
			// There are no active session, so just reset the listbox
			InitSessionsDialog( hDlg );
		}

		// Tell listbox to redraw itself now since the contents have changed
		SendMessage( hlist, WM_SETREDRAW, TRUE, 0 );
		InvalidateRect( hlist, NULL, FALSE );
	}
	else
	{
        hr = dplay->EnumSessions( &dpsd, 0, &EnumSessions, 
                                  NULL,  DPENUMSESSIONS_STOPASYNC );
	}

	return true;
}

BOOL CALLBACK dlgSession( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( msg )
    {
        case WM_INITDIALOG:
            {
                searchingForSessions = TRUE;
				SetDlgItemText( hDlg, IDC_SEARCH, GAMESTRING(STR_NET_SEARCHING));
				CheckDlgButton( hDlg, IDC_SEARCH, BST_CHECKED);
				SetTimer( hDlg, 1, 250, NULL );
				ShowNetGames(hDlg, true);

				SetWindowText(hDlg, GAMESTRING(STR_NET_FINDAGAME));

				SetDlgItemText(hDlg, IDCANCEL, GAMESTRING(STR_PCSETUP_CANCEL));
				SetDlgItemText(hDlg, IDC_SELECTA, GAMESTRING(STR_NET_CLICKJOIN));
				SetDlgItemText(hDlg, IDC_CREATE, GAMESTRING(STR_NET_CREATE));
				SetDlgItemText(hDlg, IDC_JOIN, GAMESTRING(STR_NET_JOIN));

				// Disable the join button until sessions are found
				EnableWindow( GetDlgItem( hDlg, IDC_JOIN ), FALSE );
				InitSessionsDialog(hDlg);

            }
            break;

        case WM_TIMER:
			ShowNetGames(hDlg, true);
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_SEARCH:
                    searchingForSessions = !searchingForSessions;

                    if (searchingForSessions)
                    {
                        // Start the timer, and start the async enumeratation
                        SetDlgItemText( hDlg, IDC_SEARCH, GAMESTRING(STR_NET_SEARCHING) );
                        SetTimer( hDlg, 1, 250, NULL );

						ShowNetGames(hDlg, true);
                    }
                    else
                    {
                        // Stop the timer, and stop the async enumeration
                        KillTimer( hDlg, 1 );
						ShowNetGames(hDlg, false);
                        SetDlgItemText( hDlg, IDC_SEARCH, GAMESTRING(STR_NET_FIND) );

						InitSessionsDialog(hDlg);
                    }
                    break;


                case IDC_GAMELIST:
                    if( HIWORD(wParam) != LBN_DBLCLK )
                        break;
                    // Fall through

                case IDC_JOIN:
                    if (SUCCEEDED(hr = JoinNetGame(hDlg)))
					{
						EndDialog(hDlg, 0);
					}
                    break;

                case IDC_CREATE:
					if (SUCCEEDED(CreateNetGame(hDlg)))
						EndDialog(hDlg, 0);
                    break;

                case IDCANCEL: // The close button was press
                    EndDialog(hDlg, -1);
                    break;
/*
                case IDC_BACK: // "Cancel" button was pressed
                    EndDialog(hDlg, 1);
                    break;
*/
                default:
                    return FALSE; // Message not handled
            }
            break;
        
        case WM_DESTROY:
            KillTimer(hDlg, 1);
            SessionCleanup(hDlg);
			//DPConnect_SessionsDlgCleanup();
            break;

        default:
            return FALSE; // Message not handled
    }

    // Message was handled
    return TRUE; 
}


bool FindNetGame(HWND hwnd)
{
	return (DialogBox(mdxWinInfo.hInstance, MAKEINTRESOURCE(IDD_MULTI_GAMES), hwnd, (DLGPROC) dlgSession) == 0);
}

/*	--------------------------------------------------------------------------------
	Function	: CreateNetGame
	Purpose		: Creates a network game of Frogger2
	Parameters	: 
	Returns		: success
*/


BOOL CALLBACK dlgCreate(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int len;

	switch (msg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_NAME, sessionName);
		SendDlgItemMessage(hDlg, IDC_NAME, EM_LIMITTEXT, 15, 0);

		SetDlgItemText(hDlg, IDOK,		GAMESTRING(STR_PCSETUP_OK));
		SetDlgItemText(hDlg, IDCANCEL,	GAMESTRING(STR_PCSETUP_CANCEL));
		SetDlgItemText(hDlg, IDC_GAMENAME_LABEL,	GAMESTRING(STR_NET_GAMENAME));
		SetWindowText(hDlg, GAMESTRING(STR_NET_CREATEAGAME));
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			if ((len = GetDlgItemText(hDlg, IDC_NAME, sessionName, 256)) > 0)
				EndDialog(hDlg, 0);
			break;

		case IDCANCEL:
			EndDialog(hDlg, -1);
			break;

		default:
			return FALSE;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

HRESULT JoinNetGame(HWND hDlg)
{
	DPSESSIONDESC2	dpsd;
	DPNAME			dpname;
	HRESULT			hr;
	HWND		   hWndListBox = GetDlgItem( hDlg, IDC_GAMELIST );
	DPSessionInfo* pDPSISelected = NULL;
	int			nItemSelected;

	nItemSelected = SendMessage( hWndListBox, LB_GETCURSEL, 0, 0 );
	
	if (nItemSelected < 0) return S_FALSE;

	pDPSISelected = (DPSessionInfo*) SendMessage( hWndListBox, LB_GETITEMDATA, 
												  nItemSelected, 0 );
	// Setup the DPSESSIONDESC2, and get the session guid from 
	// the selected listbox item
	ZeroMemory( &dpsd, sizeof(dpsd) );
	dpsd.dwSize		  = sizeof(dpsd);
	dpsd.guidInstance	= pDPSISelected->guidSession;
	dpsd.guidApplication = Frogger2_GUID;

	// Join the session
	isHost = FALSE;
	if( FAILED( hr = dplay->Open( &dpsd, DPOPEN_JOIN ) ) )
		return hr;

	// Create player based on g_strLocalPlayerName.  
	// Store the player's DPID in g_LocalPlayerDPID.
	// Also all DirectPlay messages for this player will signal g_hDPMessageEvent
	ZeroMemory( &dpname, sizeof(DPNAME) );
	dpname.dwSize		 = sizeof(DPNAME);
	dpname.lpszShortNameA = playerName;
	
	if( FAILED( hr = dplay->CreatePlayer( &dpidLocalPlayer, &dpname, 
										  hLocalPlayerEvent, NULL, 0, 0 ) ) )
		return hr;

	return S_OK;
}

HRESULT CreateNetGame(HWND hwnd)
{
    DPSESSIONDESC2	dpsd;
    DPNAME			dpname;
	HRESULT			res;

	if (DialogBox(mdxWinInfo.hInstance, MAKEINTRESOURCE(IDD_MULTI_CREATE), hwnd, (DLGPROC) dlgCreate) != 0)
		return -1;

    ZeroMemory( &dpsd, sizeof(dpsd) );
    dpsd.dwSize           = sizeof(dpsd);
    dpsd.guidApplication  = Frogger2_GUID;
    dpsd.lpszSessionNameA = sessionName;
    dpsd.dwMaxPlayers     = 4;
    dpsd.dwFlags          = DPSESSION_KEEPALIVE|DPSESSION_MIGRATEHOST|DPSESSION_DIRECTPLAYPROTOCOL;

	isHost = true;

	if ((res = dplay->Open(&dpsd, DPOPEN_CREATE)) != DP_OK)
	{
		utilPrintf("CreateNetGame(): dplay->Open() failed\n");
		return res;
	}

	ZeroMemory(&dpname, sizeof(dpname));
	dpname.dwSize			= sizeof(dpname);
	dpname.lpszShortNameA	= playerName;

	if ((res = dplay->CreatePlayer(&dpidLocalPlayer, &dpname, 0, NULL, 0, DPPLAYER_SERVERPLAYER)) != DP_OK)
	{
		dplay->Close();
		utilPrintf("CreateNetGame(): Failed creating player\n");
		return res;
	}

	// .. hoorah, we've created a game and a player!
	// we can go onto whatever thing we want to go onto next

	return S_OK;
}

