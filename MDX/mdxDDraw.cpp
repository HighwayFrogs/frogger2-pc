
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <math.h>
#include <stdio.h>
#include "mgeReport.h"
#include "mdxDDraw.h"
#include "mdxD3D.h"
#include "mdxInfo.h"
#include "mdxTiming.h"
#include "mdxCRC.h"
#include "mdxTexture.h"
#include "mdxMath.h"
#include "mdxObject.h"
#include "mdxActor.h"
#include "mdxLandscape.h"
#include "mdxRender.h"
#include "mdxPoly.h"
#include "mdxDText.h"
#include "mdxProfile.h"
#include "mdxWindows.h"
#include "gelf.h"
#include "commctrl.h"
#include "mdxException.h"
#include "resource.h"
#include "mdxFile.h"

#define NUM_LANGUAGES 5

const float ONE_OVER_1K_X_1K = 1.0f / (1024 * 1024);

const char *languageText[NUM_LANGUAGES] = {"English","Francais","Deutsch","Italiano","Svierge"};
const char *softwareString = "Software Rendering";
const char *softwareDriver = "Blitz Games SoftStation";

unsigned long selIdx;

DDPIXELFORMAT defaultPixelFormat;

LPDIRECTDRAW7			pDirectDraw7;
LPDIRECTDRAWCLIPPER		pClipper;
unsigned long			rXRes, rYRes, rBitDepth, r565 , rHardware, rFullscreen = 1, rScale = 1, rFlipOK = 1, rAltZBuffer = 0;
HWND					rWin;
char					rVideoDevice[256];

LPDIRECTDRAWSURFACE7	surface[NUM_SRF] = {NULL,NULL,NULL,NULL};
LPDIRECTDRAWSURFACE7	backdrop = NULL;

WNDPROC userDlgProc = NULL;

WNDPROC SetUserVideoProc(WNDPROC proc)
{
	WNDPROC oldDlgProc = userDlgProc;
	userDlgProc = proc;

	return oldDlgProc;
}

struct MDX_DXDEVICE
{
	GUID *guid;
	DDCAPS caps;
	char *desc;
	char *name;
	DWORD idx;
};

MDX_DXDEVICE dxDeviceList[10];
unsigned long dxNumDevices = 0;

void SetupDefaultPixelFormat()
{
	// Default pixel format. (This is the format which we expect to receive data, because this is the format loaded from files.)
	// It's RGB1555.
	DDINIT(defaultPixelFormat);
	defaultPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
	defaultPixelFormat.dwRGBBitCount = 16;
	defaultPixelFormat.dwRGBAlphaBitMask = 0x8000; // 1
	defaultPixelFormat.dwRBitMask = 0x7c00; // 5
	defaultPixelFormat.dwGBitMask = 0x03e0; // 5
	defaultPixelFormat.dwBBitMask = 0x001f; // 5

}

BOOL WINAPI  EnumDDDevices(GUID FAR* lpGUID, LPSTR lpDriverDesc, LPSTR lpDriverName, LPVOID lpContext, HMONITOR mon)
{
    LPDIRECTDRAW7	lpDD;
    DDCAPS			ddCaps;
	DDDEVICEIDENTIFIER2 ddId;

//	if (!_CrtCheckMemory())
//	{
//		dp("Heap is corrupt!\n");
//		__asm {int 3};
//	}

	if (DirectDrawCreateEx(lpGUID, (LPVOID *)&lpDD, IID_IDirectDraw7, NULL)!=DD_OK)				// Couldn't create DirectDraw device...
		return DDENUMRET_OK;										// so continue on to the next one

//	if (!_CrtCheckMemory())
//	{
//		dp("Heap is corrupt!\n");
//		__asm {int 3};
//	}

	DDINIT(ddCaps);													// Init caps struct
	
 	HRESULT capsResult;
	capsResult = lpDD->GetCaps(&ddCaps, NULL);					// Get the caps for the device
 	if (capsResult!=DD_OK)										// Get the caps for the device 	if (lpDD->GetCaps(&ddCaps, NULL)!=DD_OK)						// Get the caps for the device
	{
		lpDD->Release();											// Oops couldn't get it, release...
		return DDENUMRET_OK;										// And continue looking.
	}

//	if (!_CrtCheckMemory())
//	{
//		dp("Heap is corrupt!\n");
//		__asm {int 3};
//	}

	lpDD->GetDeviceIdentifier(&ddId,0);

	dxDeviceList[dxNumDevices].desc = (char *) AllocMem(sizeof(char)*strlen (ddId.szDescription)+1);
	dxDeviceList[dxNumDevices].name = (char *) AllocMem(sizeof(char)*strlen (ddId.szDriver)+1);
	
	strcpy(dxDeviceList[dxNumDevices].desc, ddId.szDescription);
	strcpy(dxDeviceList[dxNumDevices].name, ddId.szDriver);

	dxDeviceList[dxNumDevices].caps = ddCaps;						// Implicit structure copy.

//	if (!_CrtCheckMemory())
//	{
//		dp("Heap is corrupt!\n");
//		__asm {int 3};
//	}

    if (lpGUID)													// This is NULL for the primary display device
	{
	    dxDeviceList[dxNumDevices].guid = (GUID *) AllocMem(sizeof(GUID));
		memcpy(dxDeviceList[dxNumDevices].guid, lpGUID, sizeof(GUID));		
	}
	else
		dxDeviceList[dxNumDevices].guid = NULL;

	dxNumDevices++;

	lpDD->Release();

    return DDENUMRET_OK;
}

/*	---------------------------------------------------------
	Info:	Dave's lovely happy video mode enumeration stuff
*/

struct VIDEOMODEINFO
{
	HWND hcombo;
	DWORD totalVidMem;
	DWORD wantedRes;
};

HRESULT WINAPI VideoModeCallback(LPDDSURFACEDESC2 desc, LPVOID context)
{
	char mode[256];
	int index;
	DWORD videomode;
	VIDEOMODEINFO *info = (VIDEOMODEINFO*)context;
	
	HWND hcmb = (HWND)info->hcombo;

	DWORD rgbBitCount = desc->ddpfPixelFormat.dwRGBBitCount;
	DWORD bytes = (desc->dwHeight * desc->lPitch);
	dp("%d x %d (%d-bit, %dHz), ~%d bytes (%0.3fMB)\n", desc->dwWidth, desc->dwHeight, rgbBitCount, desc->dwRefreshRate, bytes, bytes * ONE_OVER_1K_X_1K);

	if (bytes < info->totalVidMem)
	{
		sprintf(mode, "%dx%d (%d-bit, %dHz)", desc->dwWidth, desc->dwHeight, desc->ddpfPixelFormat.dwRGBBitCount, desc->dwRefreshRate);
		videomode = (desc->dwWidth << 16)|(desc->dwHeight);

		index = SendMessage(hcmb, CB_ADDSTRING, 0, (LPARAM)mode);
		SendMessage(hcmb, CB_SETITEMDATA, (WPARAM)index, (LPARAM)videomode);

		if (videomode == info->wantedRes)
			SendMessage(hcmb, CB_SETCURSEL, (WPARAM)index, 0);
	}

	return DDENUMRET_OK;
}


BOOL FillVideoModes(HWND hdlg, GUID *lpGUID, DWORD resolution)
{
	HWND hctrl = GetDlgItem(hdlg, IDC_SCREENRES);
    LPDIRECTDRAW7	lpDD;
	DWORD total, free;
	DDSCAPS2 ddsc;
	DDCAPS ddc;
	HRESULT res;
	VIDEOMODEINFO info;

	ZeroMemory(&ddsc, sizeof(ddsc));
	ddsc.dwCaps = DDSCAPS_3DDEVICE|DDSCAPS_PRIMARYSURFACE;

	if (lpGUID == (GUID*)-1)
		lpGUID = NULL;

	dp("--- Enumerating video modes ---\n");

	SendMessage(hctrl, CB_RESETCONTENT, 0, 0);

	if (DirectDrawCreateEx(lpGUID, (LPVOID *)&lpDD, IID_IDirectDraw7, NULL) !=DD_OK)
		return FALSE;

	info.hcombo = hctrl;
	info.totalVidMem = 0xFFFFFFFF;
	info.wantedRes = resolution;

	if ((res = lpDD->GetAvailableVidMem(&ddsc, &total, &free)) == DD_OK)
	{
		if (total)
		{
			dp("Total video memory: %d (%0.2fMB)\n", total, total * ONE_OVER_1K_X_1K);
			info.totalVidMem = total;
		}
	}

	lpDD->EnumDisplayModes(DDEDM_REFRESHRATES, NULL, (LPVOID)&info, VideoModeCallback);

	// Check if ini specified res is compatible with users display
	// by checking if the desired resolution
	// I found that most of them work on windowed mode even if not perfectly - raq
	if (SendMessage(hctrl, CB_GETCURSEL, 0, 0) == CB_ERR)
	{
		char mode[32];
		sprintf(mode, "%dx%d (incompatible)", (resolution >> 16) & 0xFFFF, (resolution & 0xFFFF));
		DWORD videomode = resolution;

		int index = SendMessage(hctrl, CB_ADDSTRING, 0, (LPARAM)mode);
		SendMessage(hctrl, CB_SETITEMDATA, (WPARAM)index, (LPARAM)videomode);
		SendMessage(hctrl, CB_SETCURSEL, (WPARAM)index, 0);

		MessageBox(hdlg, "Resolution specified in the ini file might not be compatible with your display's resolution.", "Display error", MB_ICONEXCLAMATION|MB_OK);
	}

	ZeroMemory(&ddc, sizeof(ddc));
	ddc.dwSize = sizeof(ddc);
	lpDD->GetCaps(&ddc, NULL);

	hctrl = GetDlgItem(hdlg, IDC_WINDOW);
	
	if (ddc.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
	{
		EnableWindow(hctrl, 1);
	}
	else
	{
		SendMessage(hctrl, BM_SETCHECK, BST_UNCHECKED, 0);
		EnableWindow(hctrl, 0);
	}

	dp("-------------------------------\n");

	lpDD->Release();
	return TRUE;

}

/*
BOOL FillVideoModes(HWND hdlg, GUID *lpGUID)
{
	const POINT res[4] = { {640,480), { 800,600 }, {1024,768} };
	HWND hctrl = GetDlgItem(hdlg, IDC_SCREENRES);
	int m;

	SendMessage(hctrl, CB_RESETCONTENT, 0, 0);

	for (m=0; m<3; m++)
	{
		char mode[10];
		DWORD videomode = (res[mode].x<<16)|(res[mode].y);

		sprintf(mode, "%dx%d", desc->dwWidth, desc->dwHeight);

		int index = SendMessage(hcmb, CB_ADDSTRING, 0, (LPARAM)mode);
		SendMessage(hcmb, CB_SETITEMDATA, videomode, (LPARAM)videomode);
	}

	return TRUE;
}
*/


BOOL CALLBACK HardwareProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char *col0txt = "Description";
	char *col1txt = "Driver DLL";
	const LV_COLUMN c1 = {LVCF_FMT | LVCF_TEXT | LVCF_WIDTH,LVCFMT_LEFT,100,col1txt,255,0};
	const LV_COLUMN c2 = {LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM,LVCFMT_LEFT,350,col0txt,255,1};

	LV_ITEM i1 = {LVIF_TEXT,0,0,0,0,NULL,255};

	static int	initFlag;
	static DWORD resolution;
	unsigned	i,lastIdx;
	HWND		list;


	// this dialog appears to handle the video radio buttons
	if (userDlgProc)
	{
		BOOL result = userDlgProc(hwndDlg,uMsg,wParam, lParam);
		if (result) return result;
	}

    switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			// starting to initialise.. ignore notifications
			initFlag = 0;
			resolution = lParam;

			RECT meR;
			LV_ITEM itm;
			int index = 0;	

			GetWindowRect(hwndDlg, &meR);
			ShowWindow(hwndDlg,SW_SHOW);
			SetWindowPos(hwndDlg, HWND_NOTOPMOST, (GetSystemMetrics(SM_CXSCREEN)-(meR.right-meR.left))/2,(GetSystemMetrics(SM_CYSCREEN)-(meR.bottom-meR.top))/2, 0,0,SWP_NOSIZE);
			list = GetDlgItem(hwndDlg,IDC_LIST2);
		
			for (i=NUM_LANGUAGES; i>0; i--)
				SendMessage ( GetDlgItem(hwndDlg,IDC_LANGUAGE),LB_INSERTSTRING,0,(unsigned int)languageText[i-1]);

			SendMessage ( GetDlgItem(hwndDlg,IDC_LANGUAGE),LB_SETCURSEL,0,0);

			SendMessage (list,LVM_INSERTCOLUMN,0,(long)&c1);
			SendMessage (list,LVM_INSERTCOLUMN,0,(long)&c2);
				
			for (i=0; i<dxNumDevices; i++)
			{
				
				if ((dxDeviceList[i].guid == (GUID *)-1) || (dxDeviceList[i].caps.dwCaps & DDCAPS_3D))
				{
					i1.iItem = i; 
					i1.pszText = dxDeviceList[i].desc;
					i1.lParam = i; 
					itm.iSubItem = 0;
								
					lastIdx = dxDeviceList[i].idx = SendMessage (list,LVM_INSERTITEM,0,(long)&i1);

					itm.iItem = i; 
					itm.lParam = i; 
					itm.pszText = dxDeviceList[i].name;
					itm.iSubItem = 1;
					
					SendMessage (list,LVM_SETITEM,lastIdx,(long)&itm);

					if (strcmp(dxDeviceList[i].desc, rVideoDevice) == 0)
						index = i;
				}
			}
			ListView_SetItemState(list, index, LVIS_SELECTED | LVIS_FOCUSED, 0x00FF);

/*
			// if the hardware renderer is currently selected..
			if (stricmp(dxDeviceList[index].desc, softwareString) != NULL)
			{
				// do not allow the 320x240 resolution
				if (SendMessage(GetDlgItem(hwndDlg, IDC_320), BM_GETCHECK, 0, 0))
				{
					SendMessage(GetDlgItem(hwndDlg, IDC_320), BM_SETCHECK, 0, 0);
					SendMessage(GetDlgItem(hwndDlg, IDC_640), BM_SETCHECK, 1, 0);
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_320), FALSE);
			}
*/
			dp("Getting video modes for device %d.\n", index + 1);
			FillVideoModes(hwndDlg, dxDeviceList[index].guid, resolution);

			// initialised.. notifications valid
			initFlag = 1;

			return TRUE;
			break;
		}

		case WM_CLOSE:
			PostQuitMessage(0);
			EndDialog ( hwndDlg, FALSE );
            return TRUE;
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					PostQuitMessage(0);
					EndDialog ( hwndDlg, FALSE );
					break;
	
				case IDOK:
				{
					for (i=0; (long)i<SendMessage(GetDlgItem(hwndDlg,IDC_LIST2),LVM_GETITEMCOUNT,0,0); i++)
						if (SendMessage (GetDlgItem(hwndDlg,IDC_LIST2),LVM_GETITEMSTATE,i,LVIS_SELECTED))
							selIdx = i;

					if (SendMessage (GetDlgItem(hwndDlg,IDC_WINDOW),BM_GETCHECK,0,0) == BST_CHECKED)
					{
						dp("Running Windowed!\n");
						rFullscreen = 0;
					}

					EndDialog ( hwndDlg, TRUE );
					break;
				}

				default:
					return FALSE;
			}
			break;

		case WM_NOTIFY:

			// ready to accept control notifications?
			if (initFlag == 0)
				break;
			// is this from the video device list box?
			if (((NMHDR *)lParam)->idFrom == IDC_LIST2)
			{
				switch (((NMHDR *)lParam)->code)
				{
					// user changed selection..
					case LVN_ITEMCHANGED:
						if (((NMLISTVIEW *)lParam)->uChanged & LVIF_STATE && ((NMLISTVIEW *)lParam)->uNewState & LVIS_SELECTED)
						{
							// to what?
/*							ListView_GetItemText(((NMHDR *)lParam)->hwndFrom, ((NMLISTVIEW *)lParam)->iItem, 0, text, 32);
							
						
							if (stricmp(text, softwareString) == NULL)
							{
								// User selected software renderer

								// allow 320x240 resolution
								EnableWindow(GetDlgItem(hwndDlg, IDC_320), TRUE);
							}
							else
							{
								// User selected hardware renderer

								// do not allow 320x240 resolution
								if (SendMessage(GetDlgItem(hwndDlg, IDC_320), BM_GETCHECK, 0, 0))
								{
									SendMessage(GetDlgItem(hwndDlg, IDC_320), BM_SETCHECK, 0, 0);
									SendMessage(GetDlgItem(hwndDlg, IDC_640), BM_SETCHECK, 1, 0);
								}
								EnableWindow(GetDlgItem(hwndDlg, IDC_320), FALSE);
							}
*/
							NMLISTVIEW* nmlv = (NMLISTVIEW*)lParam;

							FillVideoModes(hwndDlg, dxDeviceList[nmlv->iItem].guid, resolution);
						}
						break;

					// user double-clicked on video selection..
					case NM_DBLCLK:
						// use current configuration and exit
						PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
						break;
				}
			}
			break;
	}

	return FALSE;
}


/*	--------------------------------------------------------------------------------
	Function	: DDrawInitObject
	Purpose		: Initialise Directdraw objects
	Parameters	: Guid of device
	Returns		: error code
	Info		: 
*/

unsigned long DDrawInitObject (DWORD resolution)
{
	HRESULT		res;
	DDCAPS		ddCaps;
	DWORD i;

	if (dxNumDevices == 0)
	{
		// Create a base DirectDraw object
		DirectDrawEnumerateEx(EnumDDDevices, 0, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);

		dxDeviceList[dxNumDevices].desc = (char *) AllocMem(sizeof(char)*(strlen (softwareString)+1));
		dxDeviceList[dxNumDevices].name = (char *) AllocMem(sizeof(char)*(strlen (softwareDriver)+1));
		
		strcpy(dxDeviceList[dxNumDevices].desc, softwareString);
		strcpy(dxDeviceList[dxNumDevices].name, softwareDriver);

		dxDeviceList[dxNumDevices++].guid = (GUID *)-1;
	}

	for (i=0; i<=dxNumDevices; i++)
		dp("Device #%d: %s (Driver: %s)\n", i + 1, dxDeviceList[i].desc, dxDeviceList[i].name);

	for (i=0; i<dxNumDevices; i++)
		if ((strcmp(dxDeviceList[i].desc, rVideoDevice) == 0))
			break;
		
	if (i<dxNumDevices)
		dp ("%s\n%s\n",dxDeviceList[i].name,dxDeviceList[i].desc);

	if (!DialogBoxParam(mdxWinInfo.hInstance, MAKEINTRESOURCE(IDD_VIDEODEVICE),NULL,(DLGPROC)HardwareProc, resolution))
		return -1;

	for (i=0; i<dxNumDevices; i++)
		if ((dxDeviceList[i].idx == selIdx) && ((dxDeviceList[i].caps.dwCaps & DDCAPS_3D) || (dxDeviceList[i].guid == (GUID *)-1)))
			break;
	dp ("%s\n%s\n",dxDeviceList[i].name,dxDeviceList[i].desc);

	strcpy(rVideoDevice, dxDeviceList[i].desc);

	if (dxDeviceList[i].guid == (GUID *)-1)
	{
		rHardware = 0;

		if ((res = DirectDrawCreateEx(NULL, (LPVOID *)&pDirectDraw7,IID_IDirectDraw7, NULL)) != DD_OK)
		{
			dp("Failed creating DirectDraw object.\n");
			ddShowError(res);
			return 1;
		}
		
//		MPR_Init();
	
	}
	else
	{
		rHardware = 1;

		if ((res = DirectDrawCreateEx(dxDeviceList[i].guid, (LPVOID *)&pDirectDraw7,IID_IDirectDraw7, NULL)) != DD_OK)
		{
			dp("Failed creating DirectDraw object.\n");
			ddShowError(res);
			return 1;
		}
	}

	DDINIT(ddCaps);													
 	if ((res = pDirectDraw7->GetCaps(&ddCaps, NULL))!=DD_OK)			
	{
		dp("Failed getting DirectDraw4 caps.\n");
		ddShowError(res);
		return 1;
	}

	// We now have a valid object! zero for success!
	return 0;
}


/*	--------------------------------------------------------------------------------
	Function	: DDrawCreateSurfaces
	Purpose		: Create all our surfaces
	Parameters	: xRes,yRes & Bitdepth, whether we will want a 3D capable render surface, and whether we would want a zbuffer
	Returns		: success
	Info		: 
*/

unsigned long DDrawCreateSurfaces(HWND window, unsigned long xRes, unsigned long yRes, unsigned long bitDepth, unsigned long want3D, unsigned long zBits)
{
	DDSURFACEDESC2	ddsd;
	HRESULT			res;
	unsigned long	l;
	DDPIXELFORMAT	zBufferFormat;
	unsigned long	bitCounter = 0, zMask = 0;

	rBitDepth =	bitDepth;
	rXRes = xRes;
	rYRes = yRes;

	// To run fullscreen - exclusive, ensure bitdepth is nonzero
	if (rFullscreen)
	{
		SetWindowLong(mdxWinInfo.hWndMain, GWL_STYLE, WS_POPUP);
		
		SetWindowPos(window,HWND_TOP,0,0,
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN),
			SWP_SHOWWINDOW|SWP_FRAMECHANGED);

		ShowWindow(mdxWinInfo.hWndMain,SW_SHOWMAXIMIZED);

		// Fullscreen Exclusive
		if ((res = pDirectDraw7->SetCooperativeLevel(window, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE)) != DD_OK)
		{
			dp("Failed setting cooperative level. (Fullscreen Mode)\n");
			ddShowError(res);
			return 0;
		}

		// Set display
		dp("Bit depth: %d\n",rBitDepth);
		dp("Xres: %d\n",rXRes);
		dp("Yres: %d\n",rYRes);
		if ((res = pDirectDraw7->SetDisplayMode(rXRes, rYRes, rBitDepth,0,0)) != DD_OK)
		{
			dp("Failed setting display mode. (Fullscreen Mode)\n");
			ddShowError(res);
			return 0;
		}
	
  		// Create a primary surface
		DDINIT(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
		ddsd.dwBackBufferCount = 1;
		if ((res = pDirectDraw7->CreateSurface(&ddsd, &surface[PRIMARY_SRF], NULL))!= DD_OK)
		{
			dp("Failed creating primary surface\n");
			ddShowError(res);
			return 0;
		}
		
		surfacesMade++;
		
		DDINIT(ddsd);
		ddsd.ddsCaps.dwCaps =  DDSCAPS_BACKBUFFER | rHardware?DDSCAPS_3DDEVICE:0;
		if ((res = surface[PRIMARY_SRF]->GetAttachedSurface (&ddsd.ddsCaps, &surface[RENDER_SRF])) != DD_OK)
		{
			dp("Failed getting render surface\n");
			ddShowError(res);
			return 0;
		}
	}
	else
	{
		ShowWindow(mdxWinInfo.hWndMain,SW_SHOW);

		if ((res = pDirectDraw7->SetCooperativeLevel(window, DDSCL_NORMAL)) != DD_OK)
		{
			dp("Failed setting cooperative level. (Windowsed Mode)\n");
			ddShowError(res);
			return 0;
		}

		// Create a primary surface
		DDINIT(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
		if ((res = pDirectDraw7->CreateSurface(&ddsd, &surface[PRIMARY_SRF], NULL))!= DD_OK)
		{
			dp("Failed creating primary surface\n");
			ddShowError(res);
			return 0;
		}
		
		surfacesMade++;
	
		// Create an additional (secondary) render surface
		DDINIT(ddsd);
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwWidth  = rXRes;
		ddsd.dwHeight = rYRes; 
		if (rHardware)
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
		else
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		if ((res = pDirectDraw7->CreateSurface(&ddsd, &surface[RENDER_SRF], NULL))!= DD_OK)
		{
			dp("Failed creating hidden surface\n");
			ddShowError(res);
			return 0;
		}

		surfacesMade++;

	}

	
	// Get Some info for the surface
	DDINIT(ddsd);
	if ((res = surface[PRIMARY_SRF]->GetSurfaceDesc(&ddsd)) != DD_OK)
	{
		dp("Failed creating primary surface description\n");
		ddShowError(res);
		return 0;
	}

	// Test the green mask to see how many bits it is.
 	l = (int)ddsd.ddpfPixelFormat.dwGBitMask;
	while ( (!(l&1)) && (l) )
		  l >>= 1;
	r565 = FALSE;
	
	// Create a render surface
	
	// Create a zbuffer if asked
	if (zBits && rHardware)
	{
		// Calculate the zbuffer z-mask
		for (bitCounter = 0; bitCounter < zBits; bitCounter++)
		{
			zMask |= (1 << bitCounter);
		}

		// Define the zbuffer pixel format
		DDINIT(zBufferFormat);
		zBufferFormat.dwFlags = DDPF_ZBUFFER;
		zBufferFormat.dwZBufferBitDepth = zBits;	// Possible values: 8, 16, 24, 32
		zBufferFormat.dwZBitMask = zMask;

		// Setup required zbuffer surface parameters
		DDINIT(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
		ddsd.dwWidth = rXRes;
		ddsd.dwHeight = rYRes;

		// Now set the pixel format for the surface
		ddsd.ddpfPixelFormat = zBufferFormat;

		// Is the user trying to use the original or alternative z-buffer creation method?
		ddsd.ddpfPixelFormat.dwRGBZBitMask = (rAltZBuffer) ? (0) : (zMask);
		
		if ((res = pDirectDraw7->CreateSurface(&ddsd, &surface[ZBUFFER_SRF], NULL)) != DD_OK)
		{
			dp("Error creating Z-buffer surface\n");
			ddShowError(res);
			return 0;
		}

		surfacesMade++;

		DDrawAttachSurface (RENDER_SRF,ZBUFFER_SRF);
	}

/*	DDINIT(ddsd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	ddsd.dwWidth  = 640;
	ddsd.dwHeight = 480; 
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
	if ((res = pDirectDraw7->CreateSurface(&ddsd, &surface[SPARE_SRF], NULL))!= DD_OK)
	{
		dp("Failed creating spare surface\n");
		ddShowError(res);
		return 0;
	}
	
*/	
	DDrawSetupWindow (window,TRUE);

	return 1;
}

/*	--------------------------------------------------------------------------------
	Function	: DDrawInitObject
	Purpose		: Initialise Directdraw objects
	Parameters	: Guid of device
	Returns		: success
	Info		: 
*/

unsigned long DDrawShutdown (void)
{
	HRESULT res;
	if ((res = pDirectDraw7->SetCooperativeLevel(rWin, DDSCL_NORMAL)) != DD_OK)
	{
		dp("Failed setting cooperative level. (Fullscreen Mode)\n");
		ddShowError(res);
	}

	if (surface[ZBUFFER_SRF])
	{
		surface[ZBUFFER_SRF]->Release();
		surfacesMade--;
	}

	if (surface[SPARE_SRF])
	{
		surface[SPARE_SRF]->Release();
		surfacesMade--;
	}

	// Delete primary (And implicitly the render, if we're using flipping) surfaces
	if (surface[PRIMARY_SRF])
	{
		surface[PRIMARY_SRF]->Release();
		surfacesMade--;
	}

	if (!rFullscreen && surface[RENDER_SRF])
	{
		surface[RENDER_SRF]->Release();
		surfacesMade--;
	}

	pDirectDraw7->Release();
	return 1;
}


/*	--------------------------------------------------------------------------------
	Function	: DDrawAttachSurface
	Purpose		: Attach two surfaces
	Parameters	: A is surface that B attaches to.
	Returns		: success
	Info		: 
*/

unsigned long DDrawAttachSurface(unsigned long srfA, unsigned long srfB)
{
	HRESULT res;
	if ((res = surface[srfA]->AddAttachedSurface(surface[srfB])) != DD_OK)
	{
		dp("Error attaching surface %lu to %lu\n");
		ddShowError(res);
		return FALSE;
	}

	return TRUE;
}

/*	--------------------------------------------------------------------------------
	Function	: DDrawSetupWindow
	Purpose		: Setup for widowing
	Parameters	: The window that will contain the view of the primary surface
	Returns		: success
	Info		: 
*/

unsigned long DDrawSetupWindow(HWND window, unsigned long scaled)
{
	HRESULT res;
	
	if (!surface[0])
	{
		dp("Cannot setup window before you create at least one surface");
		return 0;
	}
	
	rWin = window;
	rScale = scaled;
	
	// Make a clipper
	if ((res = pDirectDraw7->CreateClipper (0,&pClipper,NULL)) !=DD_OK)
	{
		dp("Failed creating clipper.\n");
		ddShowError(res);
		return 0;
	}

	// Set one end to the window
	if (pClipper->SetHWnd (0,window)!=DD_OK) 
	{
		dp("Failed setting cliiper to hwnd.\n");
		ddShowError(res);
		return 0;
	}

	// And the other to the surface!
	if (surface[PRIMARY_SRF]->SetClipper (pClipper)!=DD_OK)
	{
		dp("Failed setting clipper to surface.\n");
		ddShowError(res);
		return 0;
	}

	return 1;
}

/*	--------------------------------------------------------------------------------
	Function	: DDrawSetupWindow
	Purpose		: Setup for widowing
	Parameters	: The window that will contain the view of the primary surface
	Returns		: success
	Info		: 
*/

void DDrawFlip(void)
{
	// Flip the back buffer to the primary surface
	if (rFullscreen)
	{
		if (rFlipOK)
			surface[PRIMARY_SRF]->Flip(NULL,DDFLIP_WAIT);
		else
			surface[PRIMARY_SRF]->Blt(NULL,surface[RENDER_SRF],NULL,DDBLT_WAIT,NULL);
	}
	else
	{
		RECT clientR,windowR;

		// Or if we are windowed, copy it.. Since thay won't be attached
		GetClientRect(rWin,&clientR);
		GetWindowRect(rWin,&windowR);
		
		windowR.top+=GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYSIZEFRAME);
		windowR.left+=GetSystemMetrics(SM_CXSIZEFRAME);
			
		// Setup for scaling
		if (rScale)
		{
			windowR.bottom-=GetSystemMetrics(SM_CYSIZEFRAME);
			windowR.right-=GetSystemMetrics(SM_CXSIZEFRAME);
			clientR.right = rXRes;
			clientR.bottom = rYRes;
		}
		else
		{
			windowR.bottom = clientR.bottom+windowR.top;
			windowR.right = clientR.right+windowR.left;
			if (clientR.right>(long)rXRes) clientR.right = (long)rXRes;
			if (clientR.bottom>(long)rYRes) clientR.bottom = (long)rYRes;
		}

		// Blt it, I suppose if we aren't scaling I could used BltFast, but is it worth it? I dont think so
		surface[PRIMARY_SRF]->Blt(&windowR,surface[RENDER_SRF],&clientR,DDBLT_WAIT,NULL);
	}
}

/*	--------------------------------------------------------------------------------
	Function	: ClearSurface
	Purpose		: Clear a surface (Depth or color)
	Parameters	: Surface Number, value to fill with, DDBLT_DEPTHFILL or DDBLT_COLORFILL
	Returns		: success
	Info		: 
*/

void DDrawClearSurface(unsigned long srfN, unsigned long value, unsigned long fillType)
{
	DDBLTFX			m;
	DDINIT(m);

	// Setup, fillDepth and fill color ar currently a union to the same four bytes, but it may not stay that way forever!
	m.dwFillDepth = m.dwFillColor = value;

	// Fill it, innefecient, I would recomend not waiting!
	surface[srfN]->Blt(NULL,NULL,NULL,DDBLT_WAIT | fillType,&m);
}

inline char CalculateMaskLength(DWORD mask)
{
	char length = 0;

	// Find first.
	while (!(mask & 1) && mask)
		mask >>= 1;

	while (mask)
	{
		length++;
		mask >>= 1;
	}

	return length;
}

inline char CalculateMaskStart(DWORD mask)
{
	char start = 0;
	while (!(mask & 1) && mask)
	{
		mask >>= 1;
		start++;
	}

	return start;
}

void* ConvertPixelDataToSurfaceFormat(void **data, DWORD width, DWORD height, LPDIRECTDRAWSURFACE7 surface)
{
	DDSURFACEDESC2 ddsd;
	HRESULT result;
	void* oldData;
	long inputBytesPerPixel, outputBytesPerPixel;

	DDINIT(ddsd);
	result = surface->GetSurfaceDesc(&ddsd);
	if (result != DD_OK)
	{
		dp("Failed to convert pixel data - GetSurfaceDesc() returned status %d.", result);
		return NULL;
	}

	if (!(ddsd.dwFlags & DDSD_PIXELFORMAT))
	{
		dp("Failed to convert pixel data - DDSURFACEDESC2 did not have a pixel format. (Flags: %d)", ddsd.dwFlags);
		return NULL;
	}

	if (!(ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB))
	{
		dp("Failed to convert pixel data - DDSURFACEDESC2 pixel format was not RGB. (Flags: %d)", ddsd.ddpfPixelFormat.dwFlags);
		return NULL;
	}

	inputBytesPerPixel = ((defaultPixelFormat.dwRGBBitCount - 1) / 8) + 1;
	outputBytesPerPixel = ((ddsd.ddpfPixelFormat.dwRGBBitCount - 1) / 8) + 1;

	if (inputBytesPerPixel == outputBytesPerPixel)
	{
		ConvertPixelData(*data, *data, width, height, &defaultPixelFormat, &ddsd.ddpfPixelFormat);
		return NULL;
	}
	else
	{
		// The size changed so we need a new buffer.
		oldData = (void*) *data;
		*data = (char*)AllocMem(outputBytesPerPixel * width * height);
		ConvertPixelData(oldData, *data, width, height, &defaultPixelFormat, &ddsd.ddpfPixelFormat);
		return oldData;
	}
}

void ConvertPixelData(void *src, void* dst, DWORD width, DWORD height, DDPIXELFORMAT *inPixelFormat, DDPIXELFORMAT *outPixelFormat)
{
	DWORD inAlphaBitMask, outAlphaBitMask;
	long inMaskStartA, inMaskStartR, inMaskStartG, inMaskStartB;
	long outMaskStartA, outMaskStartR, outMaskStartG, outMaskStartB;
	long diffMaskLengthA, diffMaskLengthR, diffMaskLengthG, diffMaskLengthB;
	char *inBuf, *outBuf;
	long inputBytesPerPixel, outputBytesPerPixel;

	// Verify pixel formats ok.
	if (!(inPixelFormat->dwFlags & DDPF_RGB))
	{
		dp("Failed to convert pixel data - Input pixel format was not RGB. (Flags: %d)", inPixelFormat->dwFlags);
		return;
	}

	if (!(outPixelFormat->dwFlags & DDPF_RGB))
	{
		dp("Failed to convert pixel data - Output pixel format was not RGB. (Flags: %d)", outPixelFormat->dwFlags);
		return;
	}

	inAlphaBitMask = (inPixelFormat->dwFlags & DDPF_ALPHAPIXELS) ? inPixelFormat->dwRGBAlphaBitMask : 0;
	outAlphaBitMask = (outPixelFormat->dwFlags & DDPF_ALPHAPIXELS) ? outPixelFormat->dwRGBAlphaBitMask : 0;

	// Calculate pixel format conversion information.
	inMaskStartA = CalculateMaskStart(inAlphaBitMask);
	inMaskStartR = CalculateMaskStart(inPixelFormat->dwRBitMask);
	inMaskStartG = CalculateMaskStart(inPixelFormat->dwGBitMask);
	inMaskStartB = CalculateMaskStart(inPixelFormat->dwBBitMask);

	outMaskStartA = CalculateMaskStart(outAlphaBitMask);
	outMaskStartR = CalculateMaskStart(outPixelFormat->dwRBitMask);
	outMaskStartG = CalculateMaskStart(outPixelFormat->dwGBitMask);
	outMaskStartB = CalculateMaskStart(outPixelFormat->dwBBitMask);

	diffMaskLengthA = CalculateMaskLength(outAlphaBitMask) - CalculateMaskLength(inAlphaBitMask);
	diffMaskLengthR = CalculateMaskLength(outPixelFormat->dwRBitMask) - CalculateMaskLength(inPixelFormat->dwRBitMask);
	diffMaskLengthG = CalculateMaskLength(outPixelFormat->dwGBitMask) - CalculateMaskLength(inPixelFormat->dwGBitMask);
	diffMaskLengthB = CalculateMaskLength(outPixelFormat->dwBBitMask) - CalculateMaskLength(inPixelFormat->dwBBitMask);

	inputBytesPerPixel = ((inPixelFormat->dwRGBBitCount - 1) / 8) + 1;
	outputBytesPerPixel = ((outPixelFormat->dwRGBBitCount - 1) / 8) + 1;

	// Convert pixel data.
	inBuf = (char*)src;
	outBuf = (char*)dst;
	for (long pos = (width * height); pos; pos--)
	{
		// 1) Get pixel data from input buffer & format.
		DWORD value = 0;
		memcpy(&value, inBuf, inputBytesPerPixel);
		DWORD alpha = ((value & inAlphaBitMask) >> inMaskStartA);
		DWORD red = (value & inPixelFormat->dwRBitMask) >> inMaskStartR;
		DWORD green = (value & inPixelFormat->dwGBitMask) >> inMaskStartG;
		DWORD blue = (value & inPixelFormat->dwBBitMask) >> inMaskStartB;
//		if (pos == (width * height))
//			dp("First Pixel: %d, [R=%d,G=%d,B=%d,A=%d] [R=%d/%d,G=%d/%d,B=%d/%d,A=%d/%d] -> ", value, red, green, blue, alpha, outMaskStartR, diffMaskLengthR, outMaskStartG, diffMaskLengthG, outMaskStartB, diffMaskLengthB, outMaskStartA, diffMaskLengthA);

		// 2) Convert pixel data to new format.
		red = mdxSignedShiftLeft(red, diffMaskLengthR);
		green = mdxSignedShiftLeft(green, diffMaskLengthG);
		blue = mdxSignedShiftLeft(blue, diffMaskLengthB);
		if (!inAlphaBitMask && outAlphaBitMask)
			alpha = outAlphaBitMask >> outMaskStartA; // Fully enable alpha bits.
		else
			alpha = mdxSignedShiftLeft(alpha, diffMaskLengthA);

//		if (pos == (width * height))
//			dp("[R=%d,G=%d,B=%d,A=%d]\n", red, green, blue, alpha);

		value = (alpha << outMaskStartA) | (red << outMaskStartR) | (green << outMaskStartG) | (blue << outMaskStartB);

		// 3) Write converted pixel data to output buffer.
		memcpy(outBuf, &value, outputBytesPerPixel);
		inBuf += inputBytesPerPixel;
		outBuf += outputBytesPerPixel;
	}
}

/*	--------------------------------------------------------------------------------
	Function	: CopyDataToSurface
	Purpose		: Copy data (matching the screen format) from a buffer to the screen
	Parameters	: data to copy, surface to copy to
	Returns		: success
	Info		: 
*/
void CopyDataToSurface(void *data, LPDIRECTDRAWSURFACE7 surface)
{
	HRESULT res;
	DWORD bytesPerPixel;
	DDSURFACEDESC2 ddsd;
	DDINIT(ddsd);

	res = surface->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_WRITEONLY|DDLOCK_NOSYSLOCK, 0);
	bytesPerPixel = ((ddsd.ddpfPixelFormat.dwRGBBitCount - 1) / 8) + 1;
	
	if (res == DD_OK)
	{
		long rows = ddsd.dwHeight;
		UCHAR *p = (UCHAR*)ddsd.lpSurface, *q = (UCHAR*)data;

		while (rows--)
		{
			memcpy(p, q, ddsd.dwWidth * bytesPerPixel);
			
			p += ddsd.lPitch;
			q += ddsd.dwWidth * bytesPerPixel;
		}

		surface->Unlock(NULL);
	}
	else
		ddShowError(res);
}

/*	--------------------------------------------------------------------------------
	Function	: mdxDrawBackdrop
	Purpose		: 
	Parameters	: 
	Returns		: 
	Info		: 
*/
void mdxDrawBackdrop()
{
	DDBLTFX	m; DDINIT(m);
	HRESULT res;
	if (backdrop)
		res = surface[RENDER_SRF]->Blt(NULL, backdrop, NULL, DDBLT_WAIT, &m);
}

void mdxSetBackdropToTex(MDX_TEXENTRY *t)
{
	if (t)
		backdrop = t->surf;
}


int isBMP(const char* filename)
{
	int n = strlen(filename);
	if (n<5) return 0;
	char *c = (char*)filename + (n-4);
	return (strcmp(c, ".bmp") == 0);
}


/*	--------------------------------------------------------------------------------
	Function	: mdxLoadBackdrop
	Purpose		: 
	Parameters	: 
	Returns		: 
	Info		: 
*/
void mdxLoadBackdrop(const char* filename)
{
	DDSURFACEDESC2 ddsd; DDINIT(ddsd);
	HRESULT res;

	int xDim,yDim,gelf;
	int pptr = -1;

	void *fdata;
	BYTE* data;
	void* oldData;
	
	if (isBMP(filename))
	{
		data = (BYTE*)gelfLoad_BMP((char*)filename,NULL,(void**)&pptr,&xDim,&yDim,NULL,GELF_IFORMAT_16BPP555,GELF_IMAGEDATA_TOPDOWN);
		if (!data) {
			dp("mdxLoadBackdrop(): Failed loading .BMP\n");
			return;
		}
		gelf = 1;
	}
	else
	{
		fdata = mdxFileLoad(filename, NULL, NULL);
		if (!fdata) {
			dp("mdxLoadBackdrop(): Failed loading raw bitmap\n");
			return;
		}
		
		TEXTURE_HEADER *h = (TEXTURE_HEADER*)fdata;
		data = ((BYTE*)fdata + sizeof(TEXTURE_HEADER));
		gelf = 0;

		xDim = h->dim[0]; yDim = h->dim[1];
	}

	oldData = ConvertPixelDataToSurfaceFormat((void**)&data, xDim, yDim, surface[RENDER_SRF]);

	surface[RENDER_SRF]->GetSurfaceDesc(&ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;

/*
	// Get driver caps
	pDirectDraw7->GetCaps( &ddcaps, NULL );

	// If we can have surfaces wider than the primary (hardware determined) then create
	// a surface the size of the bitmap.
	if( ddcaps.dwCaps2 & DDCAPS2_WIDESURFACES )
	{
*/	
	ddsd.dwWidth = xDim;
	ddsd.dwHeight = yDim;

	/*	if (rHardware)
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	else*/
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

	mdxFreeBackdrop();

	if ((res = pDirectDraw7->CreateSurface(&ddsd, &backdrop, NULL)) == DD_OK)
	{
		surfacesMade++;
		CopyDataToSurface(data, backdrop);
	}
	else
	{
		dp("Error creating backdrop surface\n");
		ddShowError(res);
	}

	if (gelf)
	{
		free(data);
		if (oldData)
			FreeMem(oldData);
	}
	else
	{
		FreeMem(fdata);
		if (oldData) // We free data here instead of oldData, since fdata was free'd.
			FreeMem(data);
		
	}
}

/*	--------------------------------------------------------------------------------
	Function	: mdxFreeBackdrop
	Purpose		: 
	Parameters	: 
	Returns		: 
	Info		: 
*/
void mdxFreeBackdrop()
{
	if (backdrop)
	{
		backdrop->Release();
		surfacesMade--;
	}
}

#define REDVAL(x) ((((x>>11)&0x1f) * 0xff)/0x1f)
#define GREENVAL(x) ((((x>>5)&0x3f) * 0xff)/0x3f)
#define BLUEVAL(x) ((((x>>0)&0x1f) * 0xff)/0x1f)
int picnum = 0;

void ScreenShot()
{
	char	filename[MAX_PATH];
	FILE *file;
	short *screen;
	long pitch;
	DWORD x, y;
	unsigned short pixel;
	unsigned char	col;
	unsigned char	line[1280 * 4];
	int	linePos;

	DDSURFACEDESC2		ddsd;
	DDINIT(ddsd);
	



	dp("==================\n");
	dp("Taking screen shot\n");


	sprintf(filename, "c:\\frogshot%04d_%lux%lu.raw", picnum++,rXRes,rYRes);
	file =	fopen(filename, "wb");
	
	if(!file)
	{
		dp("FILEERROR: could not open file:\n");
		return;
	}

	if (surface[RENDER_SRF]->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,0) != DD_OK) return;
	
	screen = (short *)ddsd.lpSurface;
	pitch = ddsd.lPitch/2;

	y = 0;
	while (y<rYRes)
	{
		linePos = 0;
		
		for(x = 0; x < rXRes; x++)
		{
			pixel = screen[x + pitch * y];

			col = REDVAL(pixel);
			line[linePos++] = col;
			col = GREENVAL(pixel);
			line[linePos++] = col;
			col = BLUEVAL(pixel);
			line[linePos++] = col;
		}

		y++;

		fwrite(line, rXRes*3,1,file);	
		
	}

	surface[RENDER_SRF]->Unlock(NULL);	

	fclose(file);	
}

