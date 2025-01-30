/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: Controll.c
	Programmer	: Andrew Eder
	Date		: 11/9/98

----------------------------------------------------------------------------------------------- */

#include <math.h>
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>
#include <fstream>
#include <islutil.h>
#include <dplay.h>
#include <dplobby.h>
#include <dinput.h>
#include "..\resource.h"

#include "lang.h"
#include "cam.h"
#include "game.h"
#include "frontend.h"
#include "Main.h"
#include "menus.h"
#include "frogger.h"
#include "language.h"
#include "controll.h"
#include "islpad.h"
#include "mdx.h"
#include "layout.h"
#include "backdrop.h"
#include "dx_sound.h"
#include "hud.h"

/*	------------------------------------------------------------------------
	Global stuff
*/

BYTE keyTable[256];

PadDataType padData;	// PSX-Combatibuuule


/*	------------------------------------------------------------------------
	Controll-specific stuff
*/

#define NUM_CONTROLS	8		// number of in-game controls needed
#define DEAD_ZONE		500		// threshold at which we consider a joypad 'pressed', since PC pads are all analogue
#define MAXJOYPADS 4			// Support at most 4 joypads
#define MAXBUTTONS 6			// max buttons used on joypad ... hmm...
#define KEYBOARDBUFFER	40		// length of DIDEVICEOBJECTDATA array for keyboard
#define JOYPADPOLLRATE	20		// amount of time (ms) to sleep every loop of the joypad processing loop

// Gamepad

char keyFileName[] = "frogkeys.map";

struct CONTROLSETUP
{
	const char* name;
	unsigned long padCode;
};

CONTROLSETUP controlDesc[NUM_CONTROLS] = 
{
	{ "Forwards",	0 },
	{ "Backwards",	1 },
	{ "Left",		2 }, 
	{ "Right",		3 },
	{ "Superhop",	4 },
	{ "Croak",		5 },
	{ "Start",		6 },
	{ "Tongue",		7 }
};

//	"Camera Left",
//	"Camera Down",
//	"Camera Up",
//	"Camera Right",
//	"Trigger",
//	"Left Shoulder",
//	"Right Shoulder"

LPDIRECTINPUT7			lpDI		= NULL;
LPDIRECTINPUTDEVICE7	lpKeyb		= NULL;
LPDIRECTINPUTDEVICE7	lpJoypad[4] = { NULL, NULL, NULL, NULL };

unsigned short oldJoypadButtons[4];
unsigned short joypadButtons[4];
unsigned short joypadDebounce[4];

#ifdef JOYPADTHREAD
bool runJoypadThread = true;
HANDLE hJoypadThread = NULL, hJoyMutex = NULL, hJoyEvent = NULL;
#endif

void StopKeying(void);

char rKeyFile[MAX_PATH] = "";
unsigned rKeying = 0;
unsigned rKeyOK = 0;				
unsigned rPlaying = 0;
unsigned rPlayOK = 0;
long rEndFrame;

// -------------------------------------------------

extern unsigned rKeying;
extern unsigned rPlaying;

int numJoypads = 0;

KEYENTRY keymap[56], defaultKeymap[56] = 
{
	{ 0, PAD_UP, DIK_UP },
	{ 0, PAD_DOWN, DIK_DOWN },
	{ 0, PAD_LEFT, DIK_LEFT },
	{ 0, PAD_RIGHT, DIK_RIGHT },
	{ 0, PAD_CROSS, DIK_RETURN },
	{ 0, PAD_SQUARE, DIK_RCONTROL },
	{ 0, PAD_START, DIK_ESCAPE },
	{ 0, PAD_CIRCLE, DIK_RSHIFT },
	{ 0, PAD_TRIANGLE, DIK_ESCAPE },
	{ 0, PAD_L2, DIK_NUMPAD4 },
	{ 0, PAD_R2, DIK_NUMPAD6 },
	{ 0, PAD_SELECT, DIK_NUMPAD0 },
	{ 0, PAD_L1, DIK_NUMPAD1 },
	{ 0, PAD_R1, DIK_R },

	{ 1, PAD_UP, DIK_Q },
	{ 1, PAD_DOWN, DIK_A },
	{ 1, PAD_LEFT, DIK_S },
	{ 1, PAD_RIGHT, DIK_D },
	{ 1, PAD_CROSS, DIK_TAB },
	{ 1, PAD_SQUARE, DIK_LSHIFT },
	{ 1, PAD_START, 0 },
	{ 1, PAD_CIRCLE, DIK_Q },
	{ 1, PAD_TRIANGLE, DIK_ESCAPE },
	{ 1, PAD_L2, 0 },
	{ 1, PAD_R2, 0 },
	{ 1, PAD_SELECT, 0 },
	{ 1, PAD_L1, 0 },
	{ 1, PAD_R1, 0 },

	{ 2, PAD_UP, DIK_8 },
	{ 2, PAD_DOWN, DIK_I },
	{ 2, PAD_LEFT, DIK_O },
	{ 2, PAD_RIGHT, DIK_P },
	{ 2, PAD_CROSS, DIK_7 },
	{ 2, PAD_SQUARE, DIK_U },
	{ 2, PAD_START, 0 },
	{ 2, PAD_CIRCLE, 0 },
	{ 2, PAD_TRIANGLE, DIK_ESCAPE },
	{ 2, PAD_L2, 0 },
	{ 2, PAD_R2, 0 },
	{ 2, PAD_SELECT, 0 },
	{ 2, PAD_L1, 0 },
	{ 2, PAD_R1, 0 },

	{ 3, PAD_UP, DIK_NUMPAD5 },
	{ 3, PAD_DOWN, DIK_NUMPAD2 },
	{ 3, PAD_LEFT, DIK_NUMPAD1 },
	{ 3, PAD_RIGHT, DIK_NUMPAD3 },
	{ 3, PAD_CROSS, DIK_NUMPADENTER },
	{ 3, PAD_SQUARE, 0 },
	{ 3, PAD_START, 0 },
	{ 3, PAD_CIRCLE, 0 },
	{ 3, PAD_TRIANGLE, DIK_ESCAPE },
	{ 3, PAD_L2, 0 },
	{ 3, PAD_R2, 0 },
	{ 3, PAD_SELECT, 0 },
	{ 3, PAD_L1, 0 },
	{ 3, PAD_R1, 0 }
};

unsigned int joymap[MAXBUTTONS] = { PAD_CROSS, PAD_SQUARE, PAD_CIRCLE, PAD_TRIANGLE, PAD_START };

// controller type for each player
#define KEYBOARD 0x200
#define GAMEPAD	 0x100		// controller # in low byte

struct CONTROLLERINFO
{
	char name[80];
	unsigned short id;
};

CONTROLLERINFO *controllerInfo;	// allocated and deallocated by controller dialog

// used by controller setup to decide which thing is what
unsigned short controllers[4] = { KEYBOARD, KEYBOARD, KEYBOARD, KEYBOARD };	

unsigned long	playerInputPause;
unsigned long	playerInputPause2;

BOOL keysEnabled = TRUE;

/*	------------------------------------------------------------------------
	Forward declarations
*/

BOOL InitKeyboardControl();
BOOL InitJoystickControl();
void DeInitKeyboardControl();
void DeInitJoystick();
BOOL SetupControllerDlg(HWND hdlg);
BOOL CALLBACK DLGKeyMapDialogue(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);
int GetButtonDialog(LPDIRECTINPUTDEVICE lpDI, HWND hParent);
int SetupKeyboardDialog(int player, HWND hParent);

DWORD WINAPI JoypadThreadProc(LPVOID);

/*	------------------------------------------------------------------------ */

const char* DIErrorStr(HRESULT err)
{
	switch(err)
	{
	case DIERR_INPUTLOST:
		return "Access to the input device has been lost";
	case DIERR_NOTACQUIRED:
		return "The operation cannot be performed unless the device is acquired.";
	case DIERR_NOTINITIALIZED:
		return "This object has not been initialized.";
	}

	return "Unrecognised error code";
}


/*	--------------------------------------------------------------------------------
	Function	: RecordKeyInit
	Purpose		: initialises key recording
	Parameters	: 
	Returns		: BOOL - TRUE on success, else FALSE
	Info		: 
*/

void RecordKeyInit(unsigned long worldNum, unsigned long levelNum)
{
	FILE *fp;
	
	if (rKeyOK)
		return;

	sprintf(rKeyFile,"%s\\demos\\record-%lu-%lu.key",baseDirectory,worldNum,levelNum);
	fp = fopen(rKeyFile,"wb");
	if (fp)
	{
		rKeyOK = 1;
		fclose(fp);
		utilPrintf("Record keys START...\n");
	}
}

/*	--------------------------------------------------------------------------------
	Function	: InitInputDevices
	Purpose		: initialises input devices
	Parameters	: 
	Returns		: BOOL - TRUE on success, else FALSE
	Info		: 
*/

void RecordButtons(unsigned long b0,unsigned long pnum)
{
	FILE *fp = fopen(rKeyFile,"a+b");
	if (fp)
	{
		fwrite(&actFrameCount,4,1,fp);
		fwrite(&b0,4,1,fp);
		fwrite(&pnum,4,1,fp);
		fflush(fp);
		fclose(fp);
	}
}


/*	--------------------------------------------------------------------------------
	Function	: GetControllerSetup
	Purpose		: saves the current controller setup
	Parameters	: 
	Returns		: BOOL - TRUE on success, else FALSE
	Info		: 
*/
void KeyboardDump(HKEY hkey, bool write)
{
	unsigned char* buffer[56*3];
	unsigned char* p = (unsigned char*)buffer;
	DWORD len;
	int b = 0;

	if (write)
	{
		for (int i=0;i<56;i++)
		{
			*(p++) = keymap[i].player;
			
			for (b=0;b<16;b++)
				if ((1<<b) == keymap[i].button) break;

			*(p++) = b;
			*(p++) = keymap[i].key;
		}
		// these are binary values I dont know if we should store those in an ini

		RegSetValueEx(hkey, "Keymap", NULL, REG_BINARY, (unsigned char*)buffer, 56*3);
	}
	else
	{
		bool success = false;

		len = 56*3;
		if (RegQueryValueEx(hkey, "Keymap", NULL, NULL, (unsigned char*)buffer, &len) == ERROR_SUCCESS)
		{
			if (len < 56*3)
				utilPrintf("KeyboardDump(read): Buffer full of rubbish, ignoring\n");
			else
			{
				for (int i=0;i<56;i++)
				{
					keymap[i].player = *(p++);
					keymap[i].button = 1 << *(p++);
					keymap[i].key = *(p++);
				}
				success = true;
			}
		}

		if (!success)
			memcpy(keymap, defaultKeymap, 56*sizeof(KEYENTRY));
	}
}


/*	--------------------------------------------------------------------------------
	Function	: ResetControllerSetup
	Purpose		: resets the controller setup
	Returns		: BOOL - TRUE on success, else FALSE
*/
void ResetControllerSetup(void)
{
	for (int pl=0; pl<4; pl++)
		controllers[pl] = (pl < numJoypads) ? (GAMEPAD+pl) : KEYBOARD;

	memcpy(keymap, defaultKeymap, 56*sizeof(KEYENTRY));
}


/*	--------------------------------------------------------------------------------
	Function	: GetControllerSetup
	Purpose		: saves the current controller setup
	Parameters	: 
	Returns		: BOOL - TRUE on success, else FALSE
*/
int GetControllerSetup(void)
{
	HKEY hkey;
	char name[9] = "Control ";

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_KEY, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		for (int pl=0; pl<4; pl++)
		{
			char value[10];
			DWORD len = 16;
			name[7] = pl+'1';
			
			if (RegQueryValueEx(hkey, name, NULL, NULL, (unsigned char*)value, &len) == ERROR_SUCCESS)
			{
				if (strnicmp(value, "joypad", 6) == 0)
				{
					if (value[7] >= '1' && value[7] <= '9')
						controllers[pl] = GAMEPAD + (value[7] - '1');
					else
						controllers[pl] = GAMEPAD;
				}
				else
				{
					controllers[pl] = KEYBOARD;
				}
			}

			KeyboardDump(hkey, 0);
		}
		RegCloseKey(hkey);
	}
	else
	{
		// set default keymap
		memcpy(keymap, defaultKeymap, 56*sizeof(KEYENTRY));
	}

	return 1;
}

int SaveControllerSetup(void)
{
	HKEY hkey;
	char name[9] = "Control ";

	if ((RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_KEY, 0, 0, 0, KEY_WRITE, NULL, &hkey, NULL)) == ERROR_SUCCESS)
	{
		for (int pl=0; pl<4; pl++)
		{
			char value[10];
			name[7] = pl+'1';

			if (controllers[pl] == KEYBOARD)
				RegSetValueEx(hkey, name, NULL, REG_SZ, (unsigned char*)"Keyboard", 9);
			else
			{
				strcpy(value, "Joypad ");
				value[6] = (controllers[pl] & (GAMEPAD-1)) + '1';
				RegSetValueEx(hkey, name, NULL, REG_SZ, (unsigned char*)value, 8);
			}

			KeyboardDump(hkey, 1);
		}
		RegCloseKey(hkey);
	}

	return 1;
}

/*	--------------------------------------------------------------------------------
	Function	: InitInputDevices
	Purpose		: initialises input devices
	Parameters	: 
	Returns		: BOOL - TRUE on success, else FALSE
	Info		: 
*/
BOOL InitInputDevices()
{
	HRESULT hRes;

	hRes = DirectInputCreateEx(mdxWinInfo.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput7, (void**)&lpDI, NULL);
	if(FAILED(hRes))
		return FALSE;

	if(!InitKeyboardControl())
	{
		DeInitInputDevices();
		return FALSE;
	}

	if(!InitJoystickControl())
	{
		DeInitJoystick();
	}

	GetControllerSetup();

	// Read control map
/*
	ifstream in;

	in.open( keyFileName, ios::nocreate, filebuf::sh_read );
	if( in.is_open() )
	{
		for( int i=0; i<56; i++ )
			in >> keymap[i].key;

		in.close();
	}
*/	
	return TRUE;
}

/*	--------------------------------------------------------------------------------
	Function	: DeInitInputDevices
	Purpose		: de-initialises input devices
	Parameters	: 
	Returns		: void
	Info		: 
*/
void DeInitInputDevices()
{
	DeInitKeyboardControl();
	DeInitJoystick();

	if(lpDI)
	{
		lpDI->Release();
		lpDI = NULL;
	}

	SaveControllerSetup();
}

/*	--------------------------------------------------------------------------------
	Function	: EnumJoypadProc
	Purpose		: Enumerates DI devices to find a gamepad (for now gets the first instance)
	Parameters	: 
	Returns		:
*/
BOOL CALLBACK EnumJoypadProc(LPCDIDEVICEINSTANCE dev, LPVOID foundDev)
{
	LPCDIDEVICEINSTANCE found;

	utilPrintf("Found device: %s\n", dev->tszInstanceName);

	found = ((LPCDIDEVICEINSTANCE)foundDev) + numJoypads;

	memcpy((void*)found, dev, sizeof(DIDEVICEINSTANCE));
	
	numJoypads++;

	return DIENUM_CONTINUE;
}
 

/*	--------------------------------------------------------------------------------
	Function	: InitJoystickControl();
	Purpose		: initialises DirectInput joystick/pad control
	Parameters	: none
	Returns		: TRUE on success - else FALSE
	Info		: 
*/

BOOL InitJoystickControl()
{
	HRESULT hRes;
	DIDEVICEINSTANCE dev[4];
	LPDIRECTINPUTDEVICE7 lpJoy;
	int j;

	ZeroMemory(&dev, sizeof(dev));
	
	lpDI->EnumDevices(DIDEVTYPE_JOYSTICK, EnumJoypadProc, (void*)&dev, DIEDFL_ATTACHEDONLY);

	for (j = 0; j < numJoypads; j++)
	{
		if (!dev[j].dwSize) break;

		hRes = lpDI->CreateDeviceEx(dev[j].guidInstance, IID_IDirectInputDevice7, (void**)&lpJoy, NULL);
		if(FAILED(hRes))
		{
			utilPrintf("Failed to create IDirectInputDevice7 device for joypad\n");
			return FALSE;
		}

		hRes = lpJoy->SetDataFormat(&c_dfDIJoystick);
		if(FAILED(hRes))
		{
			utilPrintf("Error setting data format\n");
			return FALSE;
		}

		hRes = lpJoy->SetCooperativeLevel(mdxWinInfo.hWndMain,DISCL_EXCLUSIVE | DISCL_FOREGROUND);
		if(FAILED(hRes))
			return FALSE;

		 // set the range of the joystick axis
		DIPROPRANGE diprg; 

		diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		diprg.diph.dwHow        = DIPH_BYOFFSET; 
		diprg.lMin              = -1000; 
		diprg.lMax              = +1000; 

		diprg.diph.dwObj = DIJOFS_X;    // set the x-axis range
		hRes = lpJoy->SetProperty( DIPROP_RANGE, &diprg.diph );
		if ( FAILED(hRes) ) 
			return FALSE;

		diprg.diph.dwObj = DIJOFS_Y;    // set the y-axis range
		hRes = lpJoy->SetProperty( DIPROP_RANGE, &diprg.diph );
		if ( FAILED(hRes) ) 
			return FALSE;

		lpJoypad[j] = lpJoy;

		controllers[j] = GAMEPAD + j;
	}


#ifdef JOYPADTHREAD
	DWORD threadID;

	hJoyMutex = CreateMutex(NULL, FALSE, NULL);
	hJoyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	CreateThread(NULL, NULL, JoypadThreadProc, NULL, 0, &threadID);

	utilPrintf("Created Joypad thread with thread ID 0x%08x\n", threadID);
#endif

	return TRUE;
}

#ifdef JOYPADTHREAD
/*	--------------------------------------------------------------------------------
	Function	: JoypadThreadProc()
	Purpose		: Polls the joypad hundreds of millions of times a second until it explodes
	Parameters	: none
	Returns		: none
	Info		: 

	Analogue joypads don't generate interrupts and must be polled every frame; unfortunately
	because the framerate changes all the time we'll sometimes miss a press completely. By
	polling the joypad in a different thread, we can pretty much choose our time resolution.

	We have to avoid running this bit of code while the controller code in the main thread
	is processing our data

								* * * WORK IN PROGRESS * * *
*/
DWORD WINAPI JoypadThreadProc(LPVOID param)
{
	while (runJoypadThread)
	{
		if (WaitForSingleObject(hJoyMutex, 1000) == WAIT_TIMEOUT) continue;

		for (int i=0; i<4; i++)
		{
			if (controllers[i] & GAMEPAD)
			{
				LPDIRECTINPUTDEVICE2 lpJoy = lpJoypad[controllers[i] & 0xFF];
				DIJOYSTATE joy;
				HRESULT hRes;

				if (!lpJoy) break;

				lpJoy->Acquire();

				hRes = lpJoy->Poll();
				if (hRes == DI_OK || hRes == DI_NOEFFECT)
				{
					hRes = lpJoy->GetDeviceState(sizeof(joy), &joy);
					if (FAILED(hRes)) continue;

					unsigned long b = 0;

					for (int m=0; m < MAXBUTTONS; m++)
						if (joy.rgbButtons[m]) b |= joymap[m];
					
					if (joy.lX < -DEAD_ZONE)		b |= PAD_LEFT;
					else if (joy.lX > DEAD_ZONE)	b |= PAD_RIGHT;

					if (joy.lY < -DEAD_ZONE)		b |= PAD_UP;
					else if (joy.lY > DEAD_ZONE)	b |= PAD_DOWN;

					joypadDebounce[i] |= (~oldJoypadButtons[i]) & b;
					joypadButtons[i] |= b;

					//lpJoypad->UnAcquire();
				}
			}
		}

		ReleaseMutex(hJoyMutex);
		PulseEvent(hJoyEvent);
		Sleep(JOYPADPOLLRATE);
	}

	return 0;
}
#endif

void OldProcessJoypad()
{
	for(int i=0; i<4; i++ )
	{
		if (controllers[i] & GAMEPAD)
		{
			LPDIRECTINPUTDEVICE7 lpJoy = lpJoypad[controllers[i] & 0xFF];
			DIJOYSTATE joy;
			HRESULT hRes;

			if (!lpJoy) break;

			lpJoy->Acquire();

			hRes = lpJoy->Poll();
			if (hRes == DI_OK || hRes == DI_NOEFFECT)
			{
				hRes = lpJoy->GetDeviceState(sizeof(joy), &joy);
				if (FAILED(hRes))
				{
					utilPrintf("GetDeviceState() failed\n");
					return;
				}

				unsigned long b = 0;

				for (int m=0; m < MAXBUTTONS; m++)
					if (joy.rgbButtons[m]) b |= joymap[m];
				
				if (joy.lX < -DEAD_ZONE)
					b |= PAD_LEFT;
				else if (joy.lX > DEAD_ZONE)
					b |= PAD_RIGHT;

				//if ((b & (CONT_LEFT|CONT_RIGHT)) && (b & (CONT_UP|CONT_DOWN)))
				//	b &= ~(CONT_LEFT|CONT_RIGHT|CONT_DOWN|CONT_UP);	// diagonals do nothing

				if (joy.lY < -DEAD_ZONE)
					b |= PAD_UP;
				else if (joy.lY > DEAD_ZONE)
					b |= PAD_DOWN;

				if (b)
				{
					padData.digital[i] |= b;
					//pressed = 1;

					//if (rPlaying) StopKeying();
				}

				//lpJoystick->UnAcquire();
			}
		}
	}
}

/*	--------------------------------------------------------------------------------
	Function	: DeInitJoystick
	Purpose		: Releases joystick(s)
	Parameters	: none
	Returns		: none
	Info		: 
*/
void DeInitJoystick()
{
	int j;

#ifdef JOYPADTHREAD
	runJoypadThread = false;
	
	if (WaitForSingleObject(hJoypadThread, 1000) == WAIT_TIMEOUT)
	{
		utilPrintf("Couldn't shut down joypad thread; terminating\n");
		TerminateThread(hJoypadThread, -1);
	}

	CloseHandle(hJoyMutex);
#endif


	for (j=0; j<MAXJOYPADS; j++)
		if(lpJoypad[j])
		{
			lpJoypad[j]->Unacquire();
			lpJoypad[j]->Release();
			lpJoypad[j] = NULL;
		}


}


/*	--------------------------------------------------------------------------------
	Function	: InitKeyboardControl();
	Purpose		: initialises DirectInput keyboard control
	Parameters	: none
	Returns		: TRUE on success - else FALSE
	Info		: 
*/

BOOL InitKeyboardControl()
{
	HRESULT hRes;

	hRes = lpDI->CreateDeviceEx(GUID_SysKeyboard, IID_IDirectInputDevice7, (void**)&lpKeyb, NULL);
	if(FAILED(hRes))
		return FALSE;

	hRes = lpKeyb->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(hRes))
		return FALSE;

	hRes = lpKeyb->SetCooperativeLevel(mdxWinInfo.hWndMain,DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	if(FAILED(hRes))
		return FALSE;
	
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = KEYBOARDBUFFER;

    hRes = lpKeyb->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );

	hRes = lpKeyb->Acquire();
	if(FAILED(hRes))
		return FALSE;

	return TRUE;
}

int menuKey[7] = {DIK_UP,DIK_DOWN,DIK_LEFT,DIK_RIGHT,DIK_RETURN,0,DIK_ESCAPE};
int checkMenuKeys = 0;
void ProcessKeyboardInput()
{

	// read keyboard data
    DIDEVICEOBJECTDATA didod[KEYBOARDBUFFER];  // Receives buffered data 
    DWORD dwElements;
    HRESULT hr;
	int i;

    hr = DIERR_INPUTLOST;

    while ( DI_OK != hr )
    {
        dwElements = KEYBOARDBUFFER;
        hr = lpKeyb->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),didod,&dwElements,0);
        if (hr != DI_OK) 
        {
            hr = lpKeyb->Acquire();
            if ( FAILED(hr) )  
                return;
		}
	}
		
    // Study each of the buffer elements and process them.
    for (i = 0; i < dwElements; i++) 
    {
		if (didod[i].dwData & 0x80)
		{
			DWORD key = didod[i].dwOfs;

			for (int k = 0; k<4*14; k++)
				if( keymap[k].key == key )
				{
					int pl = keymap[k].player;

					joypadDebounce[pl] |= keymap[k].button;
					//joypadButtons[pl] |= keymap[i].button;
				}
		}
	}

	// We also want the keyboard state because it's used in a few places.. go figure
	hr = lpKeyb->GetDeviceState(sizeof(keyTable),&keyTable);

	for (i = 0; i<4 * 14; i++)
		if( keymap[i].key > 0 && KEYPRESS(keymap[i].key) )
		{
			//controllerdata[keymap[i].player].button |= keymap[i].button;
			padData.digital[keymap[i].player] |= keymap[i].button;
		}

	if(checkMenuKeys)
	{
		for(i = 0;i < 7;i++)
		{
			if((menuKey[i]) && (KEYPRESS(menuKey[i])))
				padData.digital[0] |= keymap[i].button;
		}
	}


	if( debugMode )
	{
		if (KEYPRESS(DIK_NUMPAD7))
		{
			fog.max*=1.2;
//			horizClip*=1.2;
//			vertClip*=1.2;
			utilPrintf("farClip is %f\n",farClip);

		}

		if (KEYPRESS(DIK_NUMPAD9) && fog.max > 5)
		{
			fog.max/=1.2;
//			horizClip/=1.2;
//			vertClip/=1.2;
			utilPrintf("farClip is %f\n",farClip);
		}

		if (KEYPRESS(DIK_F1))
			camDist.vy += FRoundRandomHack(20*gameSpeed);

		if (KEYPRESS(DIK_F2))
			camDist.vy -= FRoundRandomHack(20*gameSpeed);

		if (KEYPRESS(DIK_F3))
		{
			if (KEYPRESS(DIK_LCONTROL))
				camSideOfs += 20*gameSpeed;
			else
				camDist.vz += FRoundRandomHack(20*gameSpeed);
		}

		if (KEYPRESS(DIK_F4))
		{
			if (KEYPRESS(DIK_LCONTROL))
				camSideOfs -= 20*gameSpeed;
			else
				camDist.vz -= FRoundRandomHack(20*gameSpeed);
		}

		if( KEYPRESS(DIK_F8) )
			displayingTile = !displayingTile;

		if(KEYPRESS(DIK_F12))
			PostMessage(mdxWinInfo.hWndMain,WM_QUIT, 0, 0);
	}
}

/*	--------------------------------------------------------------------------------
	Function	: DeInitKeyboardControl()
	Purpose		: cleans up DirectInput
	Parameters	: none
	Returns		: none
	Info		: 
*/
void DeInitKeyboardControl()
{
	if(lpKeyb)
	{
		lpKeyb->Unacquire();
		lpKeyb->Release();
		lpKeyb = NULL;
	}
}




/*	--------------------------------------------------------------------------------
	Function	: ProcessUserInput
	Purpose		: processes keyboard input / mouse
	Parameters	: HWND
	Returns		: void
	Info		: 
*/
void ProcessUserInput()
{
	long i;
	int pressed = 0;
	
	unsigned short oldDigital[8];

	if (consoleDraw || textureDraw || showSounds)
		return;

	// Start record keys if we're in record mode and (*sigh*) just
	// after the first frame of a single-player mode level
	if (rKeying && gameState.mode == INGAME_MODE && frameCount == 2)
		RecordKeyInit(player[0].worldNum, player[0].levelNum);

// ds - COPY **ALL** NETWORK-SPECIFIC STUFF TO NETWORK-SPECIFIC FILES!

//	if ( chatFlags & CHAT_INPUT)
//		return;

	// reset states
	for (i=0; i<8; i++)
	{
		oldDigital[i] = padData.digital[i];
		padData.debounce[i] = 0;
		padData.digital[i] = 0;
	}

	if (windowActive)
		ProcessKeyboardInput();

#ifdef JOYPADTHREAD
	// NOTE - this is all complete shite! Ignore it for the time being
	// eats up clock cycles like a rhino on heat
	// (rhinos are reknowned for their cycle-eating frenzies)
	// (at least in my imagination) - ds

	// Joypad must have been polled at least once, so wait for it just in case
	WaitForSingleObject(hJoyEvent, 1000);

	// And now get the mutex so it won't change anything while we're using its
	// variables... perhaps a bit of an efficiency fart here - ds
	WaitForSingleObject(hJoyMutex, 1000);
#else	
	// use old joypad processing stuff
	OldProcessJoypad();
#endif

	for (i = 0; i<4; i++)
	{
		if (keysEnabled && !showSounds)
		{
			padData.digital[i] |= joypadButtons[i];
			padData.debounce[i] |= ((~oldDigital[i]) & padData.digital[i]) | joypadDebounce[i];
		}

		// recordkeys
		if ((rKeyOK) && (padData.digital[i] != oldDigital[i]))
		{
			if (padData.debounce[0] & PAD_START)
			{
				utilPrintf("Record keys STOP!\n");
				rKeyOK = 0;
			}
			else
				RecordButtons(padData.digital[i],i);
		}

		oldJoypadButtons[i] = joypadButtons[i];

		joypadDebounce[i] = 0;
		joypadButtons[i] = 0;
	}

#ifdef JOYPADTHREAD
	ReleaseMutex(hJoyMutex);
#endif
}


/*	--------------------------------------------------------------------------------
	Function		: ResetParameters
	Purpose			: resets parameters
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ResetParameters()
{
}


BOOL SetupControllerDlg(HWND hdlg)
{
	int n = numJoypads + 1;
	char playerName[32];
	HWND combo;

	SetWindowText(GetDlgItem(hdlg, ID_SETUP), GAMESTRING(STR_PCSETUP_SETUP));
	SetWindowText(GetDlgItem(hdlg, IDOK), GAMESTRING(STR_PCSETUP_OK));
	SetWindowText(GetDlgItem(hdlg, IDCANCEL), GAMESTRING(STR_PCSETUP_CANCEL));

	SetWindowText(GetDlgItem(hdlg, IDC_TXT_CONTROLS), GAMESTRING(STR_PCSETUP_CONTROLS));
	SetWindowText(/*GetDlgItem(hdlg, IDD_CONTROLS)*/hdlg, GAMESTRING(STR_PCSETUP_CONTROLS));
	SetWindowText(GetDlgItem(hdlg, ID_DEFAULT), GAMESTRING(STR_PCSETUP_DEFAULTS));

	SetWindowText(GetDlgItem(hdlg, IDC_P1), "1UP");
	SetWindowText(GetDlgItem(hdlg, IDC_P2), "2UP");
	SetWindowText(GetDlgItem(hdlg, IDC_P3), "3UP");
	SetWindowText(GetDlgItem(hdlg, IDC_P4), "4UP");

	controllerInfo = (CONTROLLERINFO*)malloc(sizeof(CONTROLLERINFO) * n);

	strcpy(controllerInfo[0].name, GAMESTRING(STR_PCSETUP_KEYBOARD));
	controllerInfo[0].id = KEYBOARD;

	for (int c = 0; c < numJoypads && lpJoypad[c]; c++)
	{
		DIDEVICEINSTANCE di;
		di.dwSize = sizeof(DIDEVICEINSTANCE);

		lpJoypad[c]->GetDeviceInfo(&di);

		strcpy(controllerInfo[c+1].name, di.tszInstanceName);
		controllerInfo[c+1].id = GAMEPAD + c;
	}

	for (int player = 0; player < 4; player++)
	{
		combo = GetDlgItem(hdlg, IDC_PLAYER1 + player);
		for (int i = 0; i < n; i++)
			SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)controllerInfo[i].name);

		if (controllers[player] == KEYBOARD)
			SendMessage(combo, CB_SETCURSEL, 0, 0);
		else
			SendMessage(combo, CB_SETCURSEL, (controllers[player]&(GAMEPAD-1))+1, 0);
	}

	return TRUE;
}


/*	--------------------------------------------------------------------------------
	Function	: ControllerDlgProc
	Purpose		: Dialog procedure for controller setup
	Parameters	: 
	Returns		: 
	Info		: 
*/

BOOL CALLBACK ControllerDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int selectedPlayer = -1;

	switch (msg)
	{
	case WM_INITDIALOG:
		return SetupControllerDlg(hdlg);

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			// Set appropriate controller data
			{
				for (int player = 0; player < 4; player++)
				{
					HWND combo = GetDlgItem(hdlg, IDC_PLAYER1 + player);
					int sel = SendMessage(combo, CB_GETCURSEL, 0, 0);
					controllers[player] = controllerInfo[sel].id;
				}
			}

			// fall through...
		case IDCANCEL:
			EndDialog(hdlg, wParam);
			free(controllerInfo);
			return TRUE;

		case ID_SETUP:
			if (selectedPlayer >= 0)
				SetupKeyboardDialog(selectedPlayer, hdlg);
			return TRUE;

		case ID_DEFAULT:
			if (MessageBox(hdlg, GAMESTRING(STR_PCSETUP_DEFAULTKEYS), GAMESTRING(STR_PCSETUP_CONTROLS), MB_ICONQUESTION|MB_YESNO) == IDYES)
			{
				ResetControllerSetup();
				for (int player = 0; player < 4; player++)
				{
					HWND combo = GetDlgItem(hdlg, IDC_PLAYER1 + player);
					if (controllers[player] == KEYBOARD)
						SendMessage(combo, CB_SETCURSEL, 0, 0);
					else
						SendMessage(combo, CB_SETCURSEL, (controllers[player]&(GAMEPAD-1))+1, 0);
				}
			}
			return TRUE;
		}

		// When selection changes, make sure no other combo boxes have this controller
		// selected (unless KEYBOARD is selected)
		case CBN_SELENDOK:
			{
				HWND combo;
				int cb, controller, id;
				
				selectedPlayer = cb = LOWORD(wParam) - IDC_PLAYER1;
				controller = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				id = controllerInfo[controller].id;

				if (id == KEYBOARD)
				{
					EnableWindow(GetDlgItem(hdlg, ID_SETUP), 1);
					return TRUE;
				}
				else
					EnableWindow(GetDlgItem(hdlg, ID_SETUP), 0);

				for (int player=0; player<4; player++)
				{
					if (player == cb) continue;
					combo = GetDlgItem(hdlg, IDC_PLAYER1 + player);
					int s = SendMessage(combo, CB_GETCURSEL, 0, 0);

					if (id == controllerInfo[s].id)
						SendMessage(combo, CB_SETCURSEL, 0, 0);
				}

				return TRUE;
			}

	}

	return FALSE;
}

/*	--------------------------------------------------------------------------------
	Function	: SetupControllers
	Purpose		: Set up control devices
	Parameters	: 
	Returns		: TRUE unless something goes horribly wrong
	Info		: 
*/
BOOL SetupControllers(HWND hwnd)
{
	return DialogBox(mdxWinInfo.hInstance, MAKEINTRESOURCE(IDD_CONTROLS), hwnd,
		(DLGPROC)ControllerDlgProc);
}

/*	--------------------------------------------------------------------------------
	Function		: SetupKeyboardDialog
	Purpose			: Make a dialogue that maps command to keys
	Parameters		: 
	Returns			: 
	Info			: 
*/
BOOL CALLBACK DLGKeyMapDialogue(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);


int SetupKeyboardDialog(int player, HWND hParent)
{
	return DialogBoxParam(mdxWinInfo.hInstance, MAKEINTRESOURCE(IDD_KEYMAPBOX), hParent,
		(DLGPROC)DLGKeyMapDialogue, (LPARAM)player);
}

BOOL CALLBACK DLGKeyMapDialogue(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	long i, j, code, item, usedKey;
	char itmTxt[256];
	HWND list;
	static DWORD keyIndex = 0;

    switch(msg)
	{
		case WM_INITDIALOG:
		{
			RECT meR;
			int player = (int)lParam;
			int tabstop = 80;

			keyIndex = 14 * player;
			
			SetWindowText(GetDlgItem(hDlg, IDOK), GAMESTRING(STR_PCSETUP_OK));
			SetWindowText(GetDlgItem(hDlg, IDCANCEL), GAMESTRING(STR_PCSETUP_CANCEL));

			wsprintf(itmTxt, "%s : %s %d", GAMESTRING(STR_PCSETUP_KEYBOARD), GAMESTRING(STR_PLAYER), (player+1));
			SetWindowText(hDlg, itmTxt);

			GetWindowRect(hDlg, &meR);
			SetWindowPos(hDlg,HWND_TOPMOST,(GetSystemMetrics(SM_CXSCREEN)-(meR.right-meR.left))/2,(GetSystemMetrics(SM_CYSCREEN)-(meR.bottom-meR.top))/2, 0,0,SWP_NOSIZE);

			list = GetDlgItem(hDlg,IDC_KEYMAPLIST);

			SendMessage(list, LB_SETTABSTOPS, 1, (LPARAM)&tabstop);


			for( i=0; i<NUM_CONTROLS; i++ )
			{
				switch(i)
				{
				case 0: strcpy( itmTxt, GAMESTRING(STR_UP) ); break;
				case 1: strcpy( itmTxt, GAMESTRING(STR_DOWN) ); break;
				case 2: strcpy( itmTxt, GAMESTRING(STR_LEFT) ); break;
				case 3: strcpy( itmTxt, GAMESTRING(STR_RIGHT) ); break;
				case 4: strcpy( itmTxt, GAMESTRING(STR_SUPERHOP) ); break;
				case 5: strcpy( itmTxt, GAMESTRING(STR_CROAK) ); break;
				case 6: strcpy( itmTxt, GAMESTRING(STR_START) ); break;
				case 7: strcpy( itmTxt, GAMESTRING(STR_TONGUE) ); break;
				}
				strcat( itmTxt, "\t" );
				strcat( itmTxt, DIKStrings[keymap[keyIndex+i].key][gameTextLang] );
				SendMessage( list,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)itmTxt );
			}

 			return TRUE;
		}

        case WM_CLOSE:
			keyIndex = 0;
			EndDialog(hDlg,TRUE);
            return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_KEYMAPLIST:
					switch(HIWORD(wParam))
					{
						case LBN_SELCHANGE:
							// Get index of new selection
							item = SendDlgItemMessage(hDlg,IDC_KEYMAPLIST,LB_GETCURSEL,(WPARAM)0,(LPARAM)0);
							if(item == LB_ERR || item < 0 || item >= NUM_CONTROLS )
								break;

							if ((code = GetButtonDialog(lpKeyb, hDlg)) == -1)
								break;

							// Check if another player has a control mapped already
							for( usedKey=-1, j=0; j<56; j+=14 )
							{
								if( j != keyIndex )
								{
									for( i=0; i<NUM_CONTROLS; i++ )
									{
										if( keymap[j+i].key == code )
										{
											usedKey = j/14;
											break;
										}
									}
								}
							}
							if( usedKey != -1 )
							{
								char errorString[1024];

								sprintf( errorString, GAMESTRING(STR_CONFIG_KEYUSED), usedKey+1 );
								MessageBox( hDlg, errorString, GAMESTRING(STR_PCSETUP_CONTROLS), MB_ICONEXCLAMATION|MB_OK );
								break;
							}

							for (i=0; i<NUM_CONTROLS; i++)
								if (keymap[keyIndex+i].key == code)
									keymap[keyIndex+i].key = 0;

							// Set DInput key in keymap
							keymap[keyIndex+item].key = code;

							// Reset and remake the key list
							list = GetDlgItem(hDlg,IDC_KEYMAPLIST);
							SendMessage( list,LB_RESETCONTENT,(WPARAM)0,(LPARAM)0 );

							for( i=0; i<NUM_CONTROLS; i++ )
							{
								switch(i)
								{
								case 0: strcpy( itmTxt, GAMESTRING(STR_UP) ); break;
								case 1: strcpy( itmTxt, GAMESTRING(STR_DOWN) ); break;
								case 2: strcpy( itmTxt, GAMESTRING(STR_LEFT) ); break;
								case 3: strcpy( itmTxt, GAMESTRING(STR_RIGHT) ); break;
								case 4: strcpy( itmTxt, GAMESTRING(STR_SUPERHOP) ); break;
								case 5: strcpy( itmTxt, GAMESTRING(STR_CROAK) ); break;
								case 6: strcpy( itmTxt, GAMESTRING(STR_START) ); break;
								case 7: strcpy( itmTxt, GAMESTRING(STR_TONGUE) ); break;
								}
								strcat( itmTxt, "\t" );
								strcat( itmTxt, DIKStrings[keymap[keyIndex+i].key][gameTextLang] );
								SendMessage( list,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)itmTxt );
							}
/*
							for( i=0; i<NUM_CONTROLS; i++ )
							{
								strcpy( itmTxt, controlDesc[i].name );
								strcat( itmTxt, "\t" );
								strcat( itmTxt, DIKStrings[keymap[keyIndex+i].key][gameTextLang] );
								SendMessage( list,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)itmTxt );
							}
*/
							SendDlgItemMessage(hDlg,IDC_KEYMAPLIST,LB_SETCURSEL,(WPARAM)-1,(LPARAM)0);

							break;
					}
					break;

				case IDCANCEL:
				{
					std::ifstream in;
					in.open(keyFileName, std::ifstream::in);
					if( in.is_open() )
					{
						for( i=0; i<56; i++ )
							in >> keymap[i].key;

						in.close();
					}
					else
					{
//						if (MessageBox(hDlg, GAMESTRING(STR_PCSETUP_DEFAULTKEYS), GAMESTRING(STR_PCSETUP_CONTROLS), MB_ICONQUESTION|MB_YESNO) == IDYES)
//						{
							ResetControllerSetup();
							for (int player = 0; player < 4; player++)
							{
								HWND combo = GetDlgItem(hDlg, IDC_PLAYER1 + player);
								if (controllers[player] == KEYBOARD)
									SendMessage(combo, CB_SETCURSEL, 0, 0);
								else
									SendMessage(combo, CB_SETCURSEL, (controllers[player]&(GAMEPAD-1))+1, 0);
							}
//						}
					}

					keyIndex = 0;
					EndDialog(hDlg,FALSE);
					break;
				}

				case IDOK:
				{
					std::ofstream out;
					out.open(keyFileName, std::ofstream::out);
					if( out.is_open() )
					{
						for( i=0; i<56; i++ )
							out << keymap[i].key << ' ';

						out.close( );
					}

					keyIndex = 0;
					EndDialog(hDlg,TRUE);
					break;
				}
			}

			return TRUE;
	}

	return FALSE;
}

/*	--------------------------------------------------------------------------------
	Function		: GetButtonDialog
	Purpose			: brings up a little dialog prompting to press a button; wait for a keypress and return immediately
	Parameters		: LPDIRECTINPUT, HWND parent
	Returns			: scan code of key
	Info			: 
*/

BOOL CALLBACK ButtonDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int *button;

	switch (msg)
	{
		case WM_INITDIALOG:
			SetWindowText(GetDlgItem(hDlg, IDC_PRESSKEY), GAMESTRING(STR_PCSETUP_PRESSKEY));
			SetWindowText(GetDlgItem(hDlg, IDCANCEL), GAMESTRING(STR_PCSETUP_CANCEL));

			button = (int*)lParam;
            
			SetTimer( hDlg, 0, 100, NULL );	// poll every 100ms
			return FALSE;	// DON'T set keyboard focus - ds

		case WM_KEYDOWN:
			// fall through..

		case WM_TIMER:
			if (lpKeyb->GetDeviceState(sizeof(keyTable),&keyTable) == DI_OK)
			{
				int i;

				for(i=1; i<256; i++ )
					if( KEYPRESS( i ) )
						break;

				if (i < 256)
				{
					*button = i;
                    KillTimer(hDlg, 0);
					EndDialog(hDlg, TRUE);
				}
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				EndDialog(hDlg, FALSE);
				break;
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

int GetButtonDialog(LPDIRECTINPUTDEVICE lpDID, HWND hParent)
{
	int button = -1;

	DialogBoxParam(mdxWinInfo.hInstance, MAKEINTRESOURCE(IDD_KEYPRESS), hParent, (DLGPROC) ButtonDialogProc, (LPARAM)&button);

	return button;
}
