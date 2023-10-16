/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: CDAudioManager.h
	Programmer	: Andy Eder
	Date		: 15/08/2023 21:14:00

    NOTE:
    
    I added CDAudioManager (in 2023) in an effort to workaround the limitations and bugs that
    exist when using the old MCI interface on Modern Windows / PCs / hardware.

----------------------------------------------------------------------------------------------- */

#ifndef CDAUDIOMANAGER_H_INCLUDED
#define CDAUDIOMANAGER_H_INCLUDED

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CDAM_NUM_TRACKS             13

#define CDAM_ERR_BASE               0

#define CDAM_OK                     0
#define CDAM_NOERROR                0
#define CDAM_ERR_NOERROR            0
#define CDAM_ERR_UNKNOWN            (CDAM_ERR_BASE + 1)
#define CDAM_ERR_INVALIDTRACKNUM    (CDAM_ERR_BASE + 2)


// CDAudioManager related data types
typedef struct
{
    BYTE track;
    BYTE minute;
    BYTE second;
    BYTE frame;
} TMSF;

typedef struct
{
    BYTE minute;
    BYTE second;
    BYTE frame;
    BYTE padding;
} MSF;

typedef struct TAGCDAM_MCI_REQUEST
{
    DWORD               requestType;
    DWORD               mciFlags;
    MCI_GENERIC_PARMS   mciGenericParms;
    MCI_PLAY_PARMS      mciPlayParms;
    MCI_STATUS_PARMS    mciStatusParms;

    struct TAGCDAM_MCI_REQUEST *prev;
    struct TAGCDAM_MCI_REQUEST *next;
} CDAM_MCI_REQUEST;

typedef struct
{
    DWORD               numEntries;
    CDAM_MCI_REQUEST    head;
} CDAM_MCI_REQUEST_LIST;

typedef struct
{
    MSF                 length;
    DWORD               hasAudioFile;
    char                szAudioFile[32];
} CDAM_TRACKINFO;

typedef struct
{
    MCIDEVICEID         deviceID;

    DWORD               currTrackNum;
    TMSF                currTrackPosTMSF;
    MSF                 currTrackLenMSF;

    TMSF                pauseTMSF;
    DWORD               requestedTrack;
    DWORD               isPaused;
} CDAMData;

// CDAudioManager related data
extern CDAMData *gCDAMData;
extern CDAM_TRACKINFO *gCDAMTrackInfo[CDAM_NUM_TRACKS];

// Forward declarations
BOOL CDAudioManager_Startup();
void CDAudioManager_Shutdown();
DWORD WINAPI CDAudioManager_AudioThreadFunction(void *pData);

DWORD CDAudioManager_Play(DWORD requestedTrack, DWORD loop);
DWORD CDAudioManager_Stop();
DWORD CDAudioManager_Pause();
DWORD CDAudioManager_Resume();

#ifdef __cplusplus
}
#endif

#endif
