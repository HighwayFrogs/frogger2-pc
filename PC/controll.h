/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: Controll.h
	Programmer	: Andrew Eder
	Date		: 12/1/98

----------------------------------------------------------------------------------------------- */

#ifndef CONTROLL_H_INCLUDED
#define CONTROLL_H_INCLUDED

#include <dinput.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* -------------------------------------------------------------------- */

extern unsigned long	playerInputPause;
extern unsigned long	playerInputPause2;

extern BOOL keysEnabled;

typedef struct
{
	short player;
	DWORD button;
	DWORD key;

} KEYENTRY;

#define KEYPRESS(keyDIK)	(keyTable[keyDIK] & 0x80)

extern BYTE keyTable[256];

extern unsigned rKeying;
extern unsigned rPlaying;
extern int checkMenuKeys;
extern KEYENTRY keymap[56];

/* ---------- Function prototypes ------------------- */

BOOL InitInputDevices();
void DeInitInputDevices();
void ProcessUserInput();
void ResetParameters();
BOOL SetupControllers(HWND hwnd);
void MakeKeyMap( );
void RecordKeyInit(unsigned long worldNum, unsigned long levelNum);
void PlayKeyInit(unsigned long worldNum, unsigned long levelNum);
void PlayKeyDone(void);

void StartControllerView();
void RunControllerView();

/* -------------------------------------------------- */

#ifdef __cplusplus
}
#endif


#endif
