/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: main.cpp
	Programmer	: 
	Date		:

	Note - I've moved the draw loop to drawloop.cpp and drawloop.h because main.cpp has become
	overly congested

----------------------------------------------------------------------------------------------- */

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <math.h>
#include <islutil.h>
#include <islpad.h>

#include <anim.h>
#include <stdio.h>

#include "game.h"
#include "types2d.h"
#include "frogger.h"
#include "levplay.h"
#include "tongue.h"
#include "babyfrog.h"
#include "hud.h"
#include "frontend.h"
#include "menus.h"
#include "textover.h"
#include "multi.h"
#include "layout.h"
#include "platform.h"
#include "enemies.h"
#include "event.h"
#include "main.h"
#include "newpsx.h"
#include "Main.h"
#include "actor2.h"
#include "bbtimer.h"
#include "maths.h"
#include "E3_Demo.h"
#include "menus.h"
#include "editor.h"
#include "drawloop.h"
#include "fadeout.h"
#include "pcsprite.h"
#include "pcgfx.h"
#include "backdrop.h"
#include "ptexture.h"
#include "dx_sound.h"
#include "banks.h"
#include "controll.h"
#include "pcmisc.h"
#include "pcaudio.h"
#include "lang.h"
#include "mdx.h"
#include "mdxException.h"
#include "softstation.h"

#include "..\resource.h"
#include "fxBlur.h"
#include "net\netconnect.h"	// ds - pending new network code
#include "net\netgame.h"


UINT msgAutoplayDisable = 0;
psFont *font = 0;
psFont *fontSmall = 0;
psFont *fontWhite = 0;
MDX_FONT *pcFont = 0;
MDX_FONT *pcFontSmall = 0;
MDX_FONT *pcFontWhite = 0;
long drawLandscape = 1;
long drawGame = 1;
long textEntry = 0;	
char textString[255] = "---";
int networkGame = 0;
int winActive = 1;

#ifdef FINAL_MASTER
char baseDirectory[MAX_PATH] = "";
#else
char baseDirectory[MAX_PATH] = "X:\\TeamSpirit\\pcversion\\";
#endif

char cdromDrive[4] = "";

char lButton = 0, rButton = 0;
int editorOk = 0;

float camY = 100,camZ = 100;

long resolution = (640<<16) + 480;
long slideSpeeds[4] = {0,16,32,64};

long fogEnable = 0;
bool configDialog = false;

void GetArgs(char *arglist);

/*	--------------------------------------------------------------------------------
	Function		: FindFroggerCD
	Parameters		: 
	Returns			: int
	Info			: 
*/

int FindFrogger2CD(void)
{
	char drives[1024], *d = drives;
	int len;

	utilPrintf("Locating Frogger2 CD-ROM... ");

	len = GetLogicalDriveStrings(1024, drives);
	while (len)
	{
		if (GetDriveType(d) == DRIVE_CDROM)
		{
			utilPrintf("%s ", d);

			char label[20];
			DWORD serial, maxLen, flags;

			GetVolumeInformation(d, label, 20, &serial, &maxLen, &flags, NULL, 0);
			if (strcmp(label, "FROGGER2") == 0)
			{
				utilPrintf("Volume '%s', serial 0x%08x : ok!\n", label, serial);
				strncpy(cdromDrive, d, 3);
				return 1;
			}
		}

		while (len && *d) len--, d++;
		if (len) d++, len--;
	}

	utilPrintf("Couldn't find Frogger2 CD.. failing!\n");
	return 0;
}

/*	--------------------------------------------------------------------------------
	Function		: GetRegistryInformation(void)
	Parameters		: 
	Returns			: int
	Info			: Gets setup stuff (e.g. install dir) from the registry
*/

int GetRegistryInformation(void)
{
	HKEY hkey;
	DWORD val, len = MAX_PATH;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_KEY, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
	{
		// If demo install (no default reg settings) then try to get current directory as base
#ifdef PC_DEMO
		int n;
		char *c;

		// Set up the base directory to be the location of the executable
		GetModuleFileName(NULL, baseDirectory, MAX_PATH);

		for (c=baseDirectory,n=0; *c; c++,n++);
		for (c--;n;n--,c--)	if (*c == '\\') break;
		*(c++) = '\\'; *c = 0;
#endif
		utilPrintf("Couldn't open registry key\n"); return 0;
	}
	else
	{
		if (RegQueryValueEx(hkey, "InstallDir", NULL, NULL, (unsigned char*)baseDirectory, &len) == ERROR_SUCCESS)
		{
			if (baseDirectory[strlen(baseDirectory) - 1] != '\\')
				strcat(baseDirectory, "\\");

			utilPrintf("Using base directory: %s\n", baseDirectory);
		}
		else
			utilPrintf("Couldn't read InstallDir value from registry\n");

		if (RegQueryValueEx(hkey, "Resolution", NULL, NULL, (unsigned char*)&val, &len) == ERROR_SUCCESS)
			resolution = val;

		if (RegQueryValueEx(hkey, "Fullscreen", NULL, NULL, (unsigned char*)&val, &len) == ERROR_SUCCESS)
			rFullscreen = val;

		len = 255;
		RegQueryValueEx(hkey, "VideoDevice", NULL, NULL, (unsigned char*)rVideoDevice, &len);

		char regLanguage[16];
		len = 16;
		if (RegQueryValueEx(hkey, "Language", NULL, NULL, (unsigned char*)regLanguage, &len) == ERROR_SUCCESS)
		{
			const char* languages[LANG_NUMLANGS] = { "English", "Français", "Deutsch", "Italiano", "US" };
			int lang;

			for (lang=0; lang<LANG_NUMLANGS; lang++)
				if (stricmp(regLanguage, languages[lang]) == 0)
				{
					gameTextLang = lang;
					break;
				}
		}

		RegCloseKey(hkey);
	}

	return 1;
}

int SetRegistryInformation(void)
{
	HKEY hkey;
	char temp[MAX_PATH];
	DWORD val;
	HRESULT res;

	if ((res = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_KEY, 0, 0, 0, KEY_WRITE, NULL, &hkey, NULL)) != ERROR_SUCCESS)
	{
		utilPrintf("Couldn't create registry key %s\n", temp); return 0;
	}
	else
	{
		// Save base directory
		RegSetValueEx(hkey, "InstallDir", NULL, REG_SZ, (unsigned char*)baseDirectory, strlen(baseDirectory) + 1);

		// Save working resolution
		val = resolution;
		RegSetValueEx(hkey, "Resolution", NULL, REG_DWORD, (unsigned char*)&val, sizeof(DWORD));

		// Save "fullscreen?"
		val = rFullscreen;
		RegSetValueEx(hkey, "Fullscreen", NULL, REG_DWORD, (unsigned char*)&val, sizeof(DWORD));

		// Save video device
		RegSetValueEx(hkey, "VideoDevice", NULL, REG_SZ, (unsigned char*)rVideoDevice, strlen(rVideoDevice)+1);

		RegCloseKey(hkey);
	}

	return 1;
}

/*	--------------------------------------------------------------------------------
	Function		: GetArgs
	Purpose			: Process Cmd Line Args
	Parameters		: 
	Returns			: 
	Info			: 
*/
void GetArgs(char *arglist)
{
	bool cmdMode;

	do
	{
		if (*arglist=='+' || *arglist=='-' || *arglist=='/')
		{
			cmdMode = 1;
			while (cmdMode)
				switch(toupper(*(++arglist)))
				{
					case 'C':
						configDialog = true;
						break;

/*
						swingCam = !swingCam;
						utilPrintf("Swinging camera %s\n",swingCam?"enabled":"disabled");
						break;
*/
					case 'R':
						rKeying = 1;
						break;

					case 'P':
						rPlaying = 1;
						break;

					case 'a':
						useAudio = !useAudio;
						utilPrintf("Audio %s\n",useAudio?"enabled":"disabled");
						break;

					case 'D':
						playDemos = !playDemos;
						utilPrintf("Demos %s\n",playDemos?"enabled":"disabled");
						break;

					case 'K':
						debugKeys = !debugKeys;
						displayDebugInfo = debugKeys;
						utilPrintf("Debug keys %s\n",debugKeys?"enabled":"disabled");
						break;

					case 'S':
						sortMode = MA_SORTBACKFRONT;
						break;

					case 'I':
						screenshotEnable = !screenshotEnable;
						utilPrintf("Screenshot mode is %s\n",screenshotEnable?"enabled":"disabled");
						break;
/*
					case 'M':
						e3multi = !e3multi;
						break;
*/
					case 'E':
						showMemDebug = !showMemDebug;
						break;

					case ' ':
					case 0:
						cmdMode = 0;
						break;

					case 'N':
						networkGame = 1;	// Start a non-lobbied network game
						break;
				}
		}
	} while ((*arglist)++);
}


int InstallChecker(HWND hParent)
{
// ds - this never really worked, but for release we MIGHT want to do something special to check the install. Perhaps.

/*	char path[MAX_PATH];
	char *c; int n;
	GetModuleFileName(NULL, path, MAX_PATH);

	for (c=path,n=0; *c; c++,n++);
	for (c--;n;n--,c--)	if (*c == '\\') break;
	*(c++) = '\\'; *c = 0;

	if (stricmp(baseDirectory, path) != 0)
	{
		if (strnicmp(path, "X:\\", 3) == 0)
		{
			if (MessageBox(hParent,
				"You are running Frogger2 from the X: drive while a version is installed on your hard drive. "
				"Do you want to use X:\\TeamSpirit\\pcversion as your base directory for this session?",
				"Warning", MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				strcpy(baseDirectory, "X:\\TeamSpirit\\pcversion\\");
			}
		}
#ifndef _DEBUG
		else if (MessageBox(hParent,
			"You are running Frogger2 from a different directory to where it was installed. "
			"This will often cause unexpected results and we recommend you re-install Frogger2 "
			"before playing\n\n"
			"Are you sure you want to continue?",
			"Warning",
			MB_YESNO | MB_ICONQUESTION) != IDYES)
				return 0;
#endif
	}
*/
	return 1;
}





/*	-------------------------------------------------------------------------------
	Function:	MainWndProc
	Params:		As WNDPROC
	returns:	0 for success, other to pass on to mdx default handler
*/

LRESULT CALLBACK MyInitProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
//	WIN32_FIND_DATA fData;
//	HANDLE			fHandle;
//	char			fName[MAX_PATH];
//	char			fPath[MAX_PATH];
//	long			idx,data;
//	FILE			*fp;

    switch(msg)
	{
		case WM_INITDIALOG:
		{
			SetWindowText(GetDlgItem(hWnd, IDC_TXT_VIDEO), GAMESTRING(STR_PCSETUP_VIDEO));
			SetWindowText(GetDlgItem(hWnd, IDC_CONTROLS), GAMESTRING(STR_PCSETUP_CONTROLS));
			SetWindowText(GetDlgItem(hWnd, IDC_TXT_RESOLUTION), GAMESTRING(STR_PCSETUP_RESOLUTION));
			SetWindowText(GetDlgItem(hWnd, IDC_WINDOW), GAMESTRING(STR_PCSETUP_WINDOWED));
			SetWindowText(GetDlgItem(hWnd, IDOK), GAMESTRING(STR_PCSETUP_OK));
			SetWindowText(GetDlgItem(hWnd, IDCANCEL), GAMESTRING(STR_PCSETUP_CANCEL));

			SendMessage(GetDlgItem(hWnd,IDC_WINDOW),BM_SETCHECK,!rFullscreen,0);

			if (!InstallChecker(hWnd))
				EndDialog(hWnd, 0);

			return FALSE;			
		}		

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_CONTROLS:
					SetupControllers(hWnd);
					return TRUE;

				case IDOK:
				{
					HWND hres = GetDlgItem(hWnd, IDC_SCREENRES);
					int sel = SendMessage(hres, CB_GETCURSEL, 0, 0);
					
					if (sel >= 0)
						resolution = SendMessage(hres, CB_GETITEMDATA, (WPARAM)sel, 0);
					else
						resolution = (640<<16) + 480;

					rFullscreen = !SendMessage(GetDlgItem(hWnd,IDC_WINDOW),BM_GETCHECK,0,0);

					SendMessage ( GetDlgItem(hWnd,IDC_LIST3),WM_GETTEXT,16,(long)saveName);

					return FALSE;				
				}
			}
		}
	}

	return FALSE;
}

/*	--------------------------------------------------------------------------------
	Function	: WinMain
	Purpose		: Application startup and shutdown
	Parameters	: the usual...
	Returns		: success
*/
void TextInput( char c )
{
	long numc;

	for(numc = 0;(textString[numc] != 0) && (textString[numc] != '-');numc++);

	if (c == 8) // backspace
	{
		if (numc>0)
			textString[numc-1] = '-';
	}
	else if (c == 13) // enter
	{
		for(numc = 0;numc < NAME_LENGTH;numc++)
			if(textString[numc] == '-')
				textString[numc] = 0;
		textEntry = 0;
	}
	else
	{
		if(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == ' ') || (c == '!'))
		{
			if (numc < NAME_LENGTH)
			{
				textString[numc] = c;
				if(numc == NAME_LENGTH - 1)
					textString[numc+1] = 0;
				else
					textString[numc+1] = '-';
			}
		}
	}
}

/*	-------------------------------------------------------------------------------
	Function:	MainWndProc
	Params:		As WNDPROC
	returns:	0 for success, other to pass on to mdx default handler
*/

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;
	switch (msg)
	{
	case WM_ACTIVATEAPP:
		if(wParam)
		{
			winActive = 1;
			for(i = 0;i < NUM_SRF;i++)
				if(surface[i])
					surface[i]->Restore();
		}
		else
			winActive = 0;
		break;

	case WM_LBUTTONDOWN:
		lButton = 1;
		break;
	
	case WM_LBUTTONUP:
		lButton = 0;
		break;

	case WM_RBUTTONDOWN:
		rButton = 1;
		break;
	
	case WM_RBUTTONUP:
		rButton = 0;
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if (debugKeys)
		{
			if (wParam == VK_F11)
			{
				showSounds = !showSounds;
				return 0;
			}
			
			if (wParam == VK_NUMLOCK)
			{
				WriteTexturesToFile();
				return 0;
			}

			if( showSounds )
			{
				if( dispSample )
				{
					if( wParam == VK_UP )
						dispSample = dispSample->prev;
					else if( wParam == VK_DOWN )
						dispSample = dispSample->next;
				}

				if( wParam == VK_RETURN )
					siPlaySound = 1;

				if( dispSample == &soundList.head )
					dispSample = soundList.head.next;

				return 0;
			}

#ifndef FINAL_MASTER
			if (wParam == VK_F10)
			{
				editorOk = !editorOk;
				keysEnabled = !keysEnabled;
				dkPressed += editorOk;
			}
#endif
		}
		break;

	case WM_CHAR:

		if (textEntry > 0)
		{
			TextInput((char)wParam);
		}
/*
		if( chatFlags & CHAT_INPUT )
		{
			ChatInput((char)wParam);
			return 0;
		}
*/

#ifndef FINAL_MASTER
		if (editorOk)	// only when editor is set up to "grab" keyboard data
		{
			EditorKeypress((char)wParam);
			return 0;
		}
#endif
		return 1;
		break;

	case MM_MCINOTIFY:
	{
		// Loop cd track when it finishes
		switch( (int)wParam )
		{
		case MCI_NOTIFY_SUCCESSFUL:
			LoopSong();
			break;
		}

		break;
	}

	default:
		if (msg && msg==msgAutoplayDisable)	
			return TRUE; // cancel autoplay

		return 1;
	}

	return 0;
}


long turbo = 4096;

// Stop things pinging in pause mode
long pFrameModifier = 0;
long beenPaused = 0;
long lastFrames,lastTicks;


void CopyActorList()
{
	ACTOR2 *c;

	for (c = actList; c; c = c->next)
	{
		if( c->actor )
			c->distanceFromFrog = DistanceBetweenPointsSS( &c->actor->position, &frog[0]->actor->position );

		if (c->actor->actualActor)
		{
			MDX_ACTOR *a = (MDX_ACTOR*)c->actor->actualActor;
			long slideVal;
			slideVal = ((c->flags>>5) & 3);
			
			if (slideVal)
				SlideObjectTextures(a->objectController->object,(slideSpeeds[slideVal]*gameSpeed)>>12);

			if (c->flags&ACTOR_SLOWSLIDE)
				SlideObjectTextures(a->objectController->object,gameSpeed>>8);

			if( a->objectController->object->flags & OBJECT_FLAGS_CLIPPED )
				c->clipped = 1;
			else
				c->clipped = 0;

			if (!c->draw)
			{
				a->visible = 0;
				a->draw = 0;
				continue;
			}
			else
			{
				a->visible = 1;
				a->draw = 1;
			}

			a->pos.vx = c->actor->position.vx * 0.1f;
			a->pos.vy = c->actor->position.vy * 0.1f;
			a->pos.vz = c->actor->position.vz * 0.1f;

			a->scale.vx = c->actor->size.vx * (1.0f/4096.0f);
			a->scale.vy = c->actor->size.vy * (1.0f/4096.0f);
			a->scale.vz = c->actor->size.vz * (1.0f/4096.0f);

			a->depthOff = c->depthShift;
			
			if (c->actor->qRot.w || c->actor->qRot.x || c->actor->qRot.y || c->actor->qRot.z)
			{
				a->qRot.x = c->actor->qRot.x * (1.0f/4096.0f);
				a->qRot.y = c->actor->qRot.y * (1.0f/4096.0f);
				a->qRot.z = c->actor->qRot.z * (1.0f/4096.0f);
				a->qRot.w = c->actor->qRot.w  * (1.0f/4096.0f);
			}
		}
	}
}

/*	--------------------------------------------------------------------------------
	Function	: LoopFunc
	Purpose		: Application main loop
	Parameters	: void
	Returns		: success
*/
long LoopFunc(void)
{
	if((gameState.mode!=PAUSE_MODE) || ((quittingLevel) && (pauseConfirmMode == 0)))
	{
		lastActFrameCount = actFrameCount;
		
		actFrameCount = (timeInfo.frameCount-(pFrameModifier)) * (float)(turbo/4096);

		gameSpeed = 4096*timeInfo.speed * (float)(turbo/4096);
		lastFrames = timeInfo.frameCount;
		lastTicks = timeInfo.tickCount;

		if (beenPaused)
		{
			pFrameModifier += (actFrameCount - lastActFrameCount);		
			actFrameCount = lastActFrameCount;
			gameSpeed = pauseGameSpeed;
			beenPaused = 0;
		}

		pauseGameSpeed = gameSpeed;
	}
	else
	{
		beenPaused = 1;
		gameSpeed = 0;
		timeInfo.speed = 0;
		timeInfo.frameCount = lastFrames;
		timeInfo.tickCount = lastTicks;
	}

	ProcessUserInput();

	if (networkGame)
	{
		StartTimer(10,"Network");
		NetgameRun();
		EndTimer(10);
	}
	else
	{
		StartTimer(4,"Gameloop");
		GameLoop();
		EndTimer(4);
	}

	StartTimer(11,"UpdateStuff");
	CopyActorList();
	EndTimer(11);

#ifndef FINAL_MASTER
	StartTimer(12,"Editor");
	if (editorOk)
		RunEditor();
	EndTimer(12);
#endif

/*
	if( KEYPRESS(DIK_F7) && chatFlags )
	{
		if( chatFlags & CHAT_INPUT )
		{
			chatFlags &= ~CHAT_INPUT;
		}
		else
		{
			if( !chatInput.msg )
				chatInput.msg = new char[MAX_CSLENGTH];

			chatInput.msgLen = 0;
			chatFlags |= CHAT_INPUT;
		}
	}
*/

//	StartTimer(13,"WaterUpdate");
//	UpdateWater();
//	EndTimer(13);

	DrawLoop();

	return 0;
}

/*	--------------------------------------------------------------------------------
	Function	: GameStartup
	Purpose		: Game startup
	Parameters	: void
	Returns		: success
*/

int GameStartup()
{
	MDX_TEXENTRY *t;
	char path[MAX_PATH];

//	SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, TRUE, NULL, 0 );

	InitProfile();
	InitDirectSound( mdxWinInfo.hInstance, mdxWinInfo.hWndMain );
	InitMaths();
	gelfInit();
	InitCRCTable();
	InitOneOverTable();
	InitFrames();
	InitFontSystem();
	ClearTimers();
	InitSampleList();

	sprintf(path, "%stextures\\font\\bigfont.bmp", baseDirectory);
	pcFont = InitFont(path);

	if( rHardware || rXRes > 320 )
		sprintf(path, "%stextures\\font\\smallfont.bmp", baseDirectory);
	else
		sprintf(path, "%stextures\\font\\softfont.bmp", baseDirectory);

	pcFontSmall = InitFont(path);

	sprintf(path, "%stextures\\font\\monofont.bmp", baseDirectory);
	pcFontWhite = InitFont(path);

	LoadTexBank("Phong",baseDirectory);

	if (t = GetTexEntryFromCRC(UpdateCRC("phong.bmp")))
		phong = t;

	if (t = GetTexEntryFromCRC(UpdateCRC("light.bmp")))
		lightMap = t;

	font = (psFont *)pcFont;
	fontSmall = (psFont *)pcFontSmall;
	fontWhite = (psFont *)pcFontWhite;
	
	fxInitBlur();

//	sprintf(path,"%stextures\\ProcData\\",baseDirectory);
	//InitWater(path);

	/*SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);*/
	InitTiming(60.0);

#ifndef FINAL_MASTER
	InitEditor();
#endif

	SPRITECLIPLEFT = clx0;
	SPRITECLIPTOP = cly0;
	SPRITECLIPRIGHT	= clx1;
	SPRITECLIPBOTTOM = cly1;

	return 1;
}

int GameShutdown()
{
	if( frog[0] )
		UndoChangeModel( frog[0]->actor );

	SaveGame();

	ShutdownNetworkGame();

#ifndef FINAL_MASTER
	ShutdownEditor();
#endif

	fontWhite = NULL;
	fontSmall = NULL;
	font = NULL;

	ShutdownFont(pcFontWhite);
	pcFontWhite = NULL;
	ShutdownFont(pcFontSmall);
	pcFontSmall = NULL;
	ShutdownFont(pcFont);
	pcFont = NULL;

	FreeAllLists();
	fxFreeBlur( );
	DeInitInputDevices();
	D3DShutdown();
	DDrawShutdown();	
	ShutDownDirectSound( );
	gelfShutdown();

	SetRegistryInformation();
//	SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, FALSE, NULL, 0 );

	return 0;
}

/*	--------------------------------------------------------------------------------
	Function	: WinMain
	Purpose		: Application startup and shutdown
	Parameters	: the usual...
	Returns		: success
*/
int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	long xRes,yRes;
	int res;
	
	SYSTEMTIME currTime;
	GetLocalTime(&currTime);
	utilPrintf("\n------------- Starting Frogger2 ----------------\n"
		"Session started %02d/%02d/%d %02d:%02d:%02d\n",
		currTime.wDay, currTime.wMonth, currTime.wYear,
		currTime.wHour, currTime.wMinute, currTime.wSecond);

	// Set up worldvisualdata array (poo)
	// perhaps not the most clear place to do this - ds
	memcpy(worldVisualData,origWorldVisualData,sizeof(worldVisualData));

	msgAutoplayDisable = RegisterWindowMessage(TEXT("QueryCancelAutoPlay")); 
	
	GetRegistryInformation();
	GetArgs(lpCmdLine);
	gameTextInit("MAPS\\LANGUAGE.FLA", LANG_NUM_STRINGS, LANG_NUMLANGS, gameTextLang);

	// Load the game here, mostly for network mode - ds
	LoadGame();

#ifndef PC_DEMO
#ifdef FINAL_MASTER
	while (!FindFrogger2CD())
	{
		if (MessageBox(NULL, GAMESTRING(STR_PCSETUP_INSERTCD), "Frogger2",
			MB_ICONEXCLAMATION|MB_RETRYCANCEL) == IDCANCEL)
			return -1;
	}
#else
	FindFrogger2CD();
#endif
#endif

	SetUserVideoProc(MyInitProc);
	// Init common controls
	InitCommonControls();

	// Init windows
	if (!WindowsInitialise(hInstance,"Frogger2",1))
		return 1;

	// Init network game, then do a weirdass check - ds
	res = StartNetworkGame(mdxWinInfo.hWndMain, networkGame);

	if ((networkGame && res==0) || res == -1)
		return 1;
	else if (res == 1)
		networkGame = 1;

	InitInputDevices();

	while (1)
	{
		// Init DDraw Object
		if (DDrawInitObject (configDialog, resolution) == -1)
			return 1;

/*
		// *ASL* 13/06/2000
		switch (resolution)
		{
		case 0:
			xRes = 320;
			yRes = 240;
			break;
		case 1:
			xRes = 640;
			yRes = 480;
			break;
		case 2:
			xRes = 800;
			yRes = 600;
			break;
		case 3:
			xRes = 1024;
			yRes = 768;
			break;
		case 4:
			xRes = 1280;
			yRes = 1024;
			break;
		default:
			xRes = 640;
			yRes = 480;
			break;
		}
*/

		xRes = (resolution & 0xFFFF0000) >> 16;
		yRes = (resolution & 0xFFFF);

		OVERLAY_X = xRes/4096.0;
		OVERLAY_Y = yRes/4096.0;
		
		// Setup our sufaces and Direct3D, break out of check loop on success
		if (DDrawCreateSurfaces (mdxWinInfo.hWndMain, xRes, yRes, 16,TRUE, 16))
		{
			// Setup D3D
			if (D3DInit(xRes, yRes))
			{
				SetupRenderer(xRes, yRes);
				break;
			}
		}

		// we must have failed .. shut things down and loop around
		// we really want to print a helpful message here, but, er, right now I don't.
		// Might be nice to dim out modes that failed too, if I can be bothered
		D3DShutdown();
		DDrawShutdown();

		ShowWindow(mdxWinInfo.hWndMain, 0);

		MessageBox(NULL,
			GAMESTRING(STR_PCSETUP_BADVIDEO), "Frogger2",
			MB_ICONEXCLAMATION|MB_OK);

		configDialog = true;
	}

	GameStartup();

	if (networkGame)
		NetgameStartGame();
	else
		CommonInit();
	
	mdxSetUserWndProc(MainWndProc);
	
	// tell MDX what callback function to use
	AppLoop = LoopFunc;

	RunWindowsLoop();

	return GameShutdown();
}
