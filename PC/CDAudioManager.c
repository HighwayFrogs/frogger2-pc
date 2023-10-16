/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: CDAudioManager.c
	Programmer	: Andy Eder
	Date		: 15/08/2023 21:14:00

    NOTE:
    
    I added CDAudioManager (in 2023) in an effort to workaround the limitations and bugs that
    exist when using the old MCI interface on modern Windows / PCs / hardware.

----------------------------------------------------------------------------------------------- */

#include <Windows.h>
#include <d3d.h>

#include "islutil.h"
#include "islmem.h"
#include "Main.h"
#include "pcaudio.h"
#include "CDAudioManager.h"

//
// Forward declarations
//

DWORD CDAudioManager_PlayFileSample(DWORD requestedTrack, DWORD isLooping);
DWORD CDAudioManager_StopFileSample();
DWORD CDAudioManager_PauseFileSample();
DWORD CDAudioManager_ResumeFileSample();

DWORD CDAudioManager_PlayMCI(DWORD requestedTrack, DWORD isLooping);
DWORD CDAudioManager_StopMCI();
DWORD CDAudioManager_PauseMCI();
DWORD CDAudioManager_ResumeMCI();


//
// CDAudioManager data
//

#define MCI_CD_FRAMES_PER_SECOND            75

const char *gCDAMTrackFolder = "music\\";

const char *gCDAMTrackFilenames[CDAM_NUM_TRACKS] =
{
	NULL,                       // [00] NOTRACK
	NULL,                       // [01] NOTRACK
	"01 Garden.wav",            // [02] GARDEN_CDAUDIO
	"02 Ancients.wav",          // [03] ANCIENTS_CDAUDIO
	"03 Space.wav",             // [04] SPACE_CDAUDIO
	"04 Subterranean.wav",      // [05] SUBTERRANEAN_CDAUDIO
	"05 Laboratory.wav",        // [06] LABORATORY_CDAUDIO
	"06 Halloween.wav",         // [07] HALLOWEEN_CDAUDIO
	"07 retro.wav",             // [08] SUPERRETRO_CDAUDIO
	"08 Title.wav",             // [09] FRONTEND_CDAUDIO
	"09 level complete.wav",    // [10] LEVELCOMPLETE_CDAUDIO
	"10 gameover.wav",          // [11] GAMEOVER_CDAUDIO
	"11 EOL.wav"                // [12] LEVELCOMPLETELOOP_CDAUDIO
};


CRITICAL_SECTION gCDAMRequestListCriticalSection = { 0 };
CDAM_MCI_REQUEST_LIST gCDAMRequestList = { 0 };

CDAMData *gCDAMData = NULL;

CDAM_TRACKINFO *gCDAMTrackInfo[CDAM_NUM_TRACKS] = { 0 };

HANDLE gCDAMAudioThreadHandle = NULL;
BOOL gCDAMRunAudioThread = FALSE;
DWORD gCDAMAudioIdlePeriod = 200;   //250;


CDAM_MCI_REQUEST *CDAudioManager_AddRequest()
{
    // SHOULD *ONLY* BE CALLED FROM WITHIN AN ASSOCIATED CRITICAL SECTION WHEN CD AUDIO THREAD IS RUNNING!

    CDAM_MCI_REQUEST *ptr;
    CDAM_MCI_REQUEST *request;

	request = (CDAM_MCI_REQUEST *)MALLOC(sizeof(CDAM_MCI_REQUEST));
	ptr = &gCDAMRequestList.head;

	request->prev = ptr;
	request->next = ptr->next;
	ptr->next->prev = request;
	ptr->next = request;
	
    gCDAMRequestList.numEntries++;

	return request;
}


void CDAudioManager_RemoveRequest(CDAM_MCI_REQUEST *request)
{
    // SHOULD *ONLY* BE CALLED FROM WITHIN AN ASSOCIATED CRITICAL SECTION WHEN CD AUDIO THREAD IS RUNNING!

	if (!request || !request->next)
    {
        return;
    }

	request->prev->next = request->next;
	request->next->prev = request->prev;
	gCDAMRequestList.numEntries--;

	FREE(request);
}


void CDAudioManager_InitRequestList()
{
    gCDAMRequestList.head.next = gCDAMRequestList.head.prev = &gCDAMRequestList.head;
	gCDAMRequestList.numEntries = 0;
}


void CDAudioManager_FreeRequestList()
{
	while (gCDAMRequestList.head.next && gCDAMRequestList.head.next != &gCDAMRequestList.head)
	{
		CDAudioManager_RemoveRequest(gCDAMRequestList.head.next);
	}
}


BOOL CDAudioManager_FileExists(const char* baseDir, const char* filename)
{
    char fullPath[512];
    DWORD fileAttributes = 0;

    ZeroMemory(fullPath, 512);
	strcat(fullPath, baseDir);
	strcat(fullPath, gCDAMTrackFolder);
	strcat(fullPath, filename);

    fileAttributes = GetFileAttributes(fullPath);

    return (fileAttributes != INVALID_FILE_ATTRIBUTES) && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}


BOOL CDAudioManager_Startup()
{
    int i = 0;

    utilPrintf("CDAudioManager_Initialise() called...\n");

    // Allocate memory for our shared thread data (might need to think about properly synchronizing this...)
    gCDAMData = (CDAMData *)MALLOC(sizeof(CDAMData));
    if (gCDAMData == NULL)
    {
        utilPrintf("CDAudioManager_Initialise() - Unable to allocate memory for CD audio worker thread data structures!\n");
        return FALSE;
    }

    memset(gCDAMData, 0, sizeof(CDAMData));

    // Allocate memory for track lengths
    //  - NOTE: we have to spin up our CD audio thread and request track lengths there (not on this main thread!)
    //  
    //  [00] NULL (NO CDAUDIO)
    //  [01] NULL (NO CDAUDIO)
    //  [02] GARDEN_CDAUDIO
    //  [03] ANCIENTS_CDAUDIO
    //  [04] SPACE_CDAUDIO
    //  [05] SUBTERRANEAN_CDAUDIO
    //  [06] LABORATORY_CDAUDIO
    //  [07] HALLOWEEN_CDAUDIO
    //  [08] SUPERRETRO_CDAUDIO
    //  [09] FRONTEND_CDAUDIO
    //  [20] LEVELCOMPLETE_CDAUDIO
    //  [11] GAMEOVER_CDAUDIO
    //  [12] LEVELCOMPLETELOOP_CDAUDIO

    // Loop through the known CD audio tracks and look to see if an audio file exists for that track
    for (i = 0; i < CDAM_NUM_TRACKS; i++)
    {
        gCDAMTrackInfo[i] = (i > 1) ? (CDAM_TRACKINFO *)MALLOC(sizeof(CDAM_TRACKINFO)) : NULL;

        if (gCDAMTrackInfo[i] != NULL)
        {
            // Get the filename and check to see if it actually exists on this system/install
            strcpy(gCDAMTrackInfo[i]->szAudioFile, gCDAMTrackFilenames[i]);
            gCDAMTrackInfo[i]->hasAudioFile = CDAudioManager_FileExists(baseDirectory, gCDAMTrackInfo[i]->szAudioFile);
        }
    }

    // Create the critical section(s) to control cross-thread shared data access
    if (!InitializeCriticalSectionAndSpinCount(&gCDAMRequestListCriticalSection, 0x00000400))
    {
        utilPrintf("CDAudioManager_Initialise() - Unable to initialize CD audio worker thread access control object!\n");
        return FALSE;
    }

    // Setup and init shared data
    CDAudioManager_InitRequestList();

    // Attempt to create a thread purely for CD audio handling
    gCDAMAudioThreadHandle = CreateThread(NULL, 0, CDAudioManager_AudioThreadFunction, gCDAMData, 0, NULL);
    if (gCDAMAudioThreadHandle == NULL)
    {
        utilPrintf("CDAudioManager_Initialise() - Unable to create CD audio worker thread!\n");
        return FALSE;
    }

    // Set flag to run main processing loop
    gCDAMRunAudioThread = TRUE;

    return TRUE;
}


void CDAudioManager_Shutdown()
{
    int i = 0;

    utilPrintf("CDAudioManager_Shutdown() called...\n");

    // Set flag to terminate main processing loop
    gCDAMRunAudioThread = FALSE;

    // Wait for the thread to finish, then release and cleanup
    WaitForSingleObject(gCDAMAudioThreadHandle, INFINITE);
    if (gCDAMAudioThreadHandle != NULL)
    {
        CloseHandle(gCDAMAudioThreadHandle);
        gCDAMAudioThreadHandle = NULL;
    }

    // Release the critical section(s)
    DeleteCriticalSection(&gCDAMRequestListCriticalSection);

    // Free memory used for storing track lengths
    for (i = 0; i < CDAM_NUM_TRACKS; i++)
    {
        if (gCDAMTrackInfo[i] != NULL)
        {
            FREE(gCDAMTrackInfo[i]);
        }
    }

    // Free other shared data
    CDAudioManager_FreeRequestList();

    // Free memory used for shared thread data
    FREE(gCDAMData);
}


DWORD CDAudioManager_Play(DWORD requestedTrack, DWORD loop)
{
    if (requestedTrack > 1)
    {
        return (gCDAMTrackInfo[requestedTrack]->hasAudioFile) ?
            CDAudioManager_PlayFileSample(requestedTrack, loop) :
            CDAudioManager_PlayMCI(requestedTrack, loop);
    }

    return CDAM_ERR_INVALIDTRACKNUM;
}


DWORD CDAudioManager_Stop()
{
    if (gCDAMData->currTrackNum > 1)
    {
        return (gCDAMTrackInfo[gCDAMData->currTrackNum]->hasAudioFile) ?
            CDAudioManager_StopFileSample() :
            CDAudioManager_StopMCI();
    }

    return CDAM_OK;
}


DWORD CDAudioManager_Pause()
{
    if (gCDAMData->currTrackNum > 1)
    {
        return (gCDAMTrackInfo[gCDAMData->currTrackNum]->hasAudioFile) ?
            CDAudioManager_PauseFileSample() :
            CDAudioManager_PauseMCI();
    }

    return CDAM_ERR_INVALIDTRACKNUM;
}


DWORD CDAudioManager_Resume()
{
    if (gCDAMData->currTrackNum > 1)
    {
        return (gCDAMTrackInfo[gCDAMData->currTrackNum]->hasAudioFile) ?
            CDAudioManager_ResumeFileSample() :
            CDAudioManager_ResumeMCI();
    }

    return CDAM_ERR_INVALIDTRACKNUM;
}


//
// Functions to play back audio from file sample
//

DWORD CDAudioManager_PlayFileSample(DWORD requestedTrack, DWORD isLooping)
{
    gCDAMData->currTrackNum     = requestedTrack;
    gCDAMData->currTrackLenMSF  = gCDAMTrackInfo[requestedTrack]->length;
    gCDAMData->isPaused         = 0;

    return PlayCDTrackFromFile(requestedTrack, isLooping);
}


DWORD CDAudioManager_StopFileSample()
{
    gCDAMData->requestedTrack   = 0;
    gCDAMData->currTrackNum     = 0;
    gCDAMData->isPaused         = 0;

    StopSong();
    return CDAM_OK;
}


DWORD CDAudioManager_PauseFileSample()
{
    gCDAMData->isPaused         = 1;

    return CDAM_OK;
}


DWORD CDAudioManager_ResumeFileSample()
{
    gCDAMData->isPaused         = 0;

    LoopSong();
    return CDAM_OK;
}


//
// Functions to play back CD audio track (using MCI)
//

DWORD CDAudioManager_PlayMCI(DWORD requestedTrack, DWORD isLooping)
{
    MSF *pMSF   = &gCDAMTrackInfo[requestedTrack]->length;
    DWORD from  = MCI_MAKE_TMSF(requestedTrack, 0, 0, 0);
    DWORD to    = MCI_MAKE_TMSF(requestedTrack, pMSF->minute, pMSF->second, pMSF->frame);

    CDAM_MCI_REQUEST *playRequest = NULL;

    /////////////////////////////////////////////////////////////////////////>>

    EnterCriticalSection(&gCDAMRequestListCriticalSection);

    playRequest = CDAudioManager_AddRequest();
    if (playRequest != NULL)
    {
        utilPrintf("CDAudioManager << Producing PLAY Request >>\n");

        // Set MCI message type
        playRequest->requestType = MCI_PLAY;
        // Set flags
        playRequest->mciFlags = MCI_FROM | MCI_TO;
        // Set params
        memset(&playRequest->mciPlayParms, 0, sizeof(MCI_PLAY_PARMS));
        playRequest->mciPlayParms.dwCallback = (DWORD_PTR)NULL;
        playRequest->mciPlayParms.dwFrom = from;
        playRequest->mciPlayParms.dwTo = to;
    }

    LeaveCriticalSection(&gCDAMRequestListCriticalSection);

    /////////////////////////////////////////////////////////////////////////>>

    return CDAM_OK;
}


DWORD CDAudioManager_StopMCI()
{
    CDAM_MCI_REQUEST *stopRequest = NULL;

    /////////////////////////////////////////////////////////////////////////>>

    EnterCriticalSection(&gCDAMRequestListCriticalSection);

    stopRequest = CDAudioManager_AddRequest();
    if (stopRequest != NULL)
    {
        utilPrintf("CDAudioManager << Producing STOP Request >>\n");

        // Set MCI message type
        stopRequest->requestType = MCI_STOP;
        // Set flags
        stopRequest->mciFlags = 0;
        // Set params
        memset(&stopRequest->mciGenericParms, 0, sizeof(MCI_GENERIC_PARMS));
    }

    LeaveCriticalSection(&gCDAMRequestListCriticalSection);

    /////////////////////////////////////////////////////////////////////////>>

    return 0;
}


DWORD CDAudioManager_PauseMCI()
{
    CDAM_MCI_REQUEST *pauseRequest = NULL;

    /////////////////////////////////////////////////////////////////////////>>

    EnterCriticalSection(&gCDAMRequestListCriticalSection);

    pauseRequest = CDAudioManager_AddRequest();
    if (pauseRequest != NULL)
    {
        utilPrintf("CDAudioManager << Producing PAUSE Request >>\n");

        // Set MCI message type
        pauseRequest->requestType = MCI_PAUSE;
        // Set flags
        pauseRequest->mciFlags = 0;
        // Set params
        memset(&pauseRequest->mciGenericParms, 0, sizeof(MCI_GENERIC_PARMS));
    }

    LeaveCriticalSection(&gCDAMRequestListCriticalSection);

    /////////////////////////////////////////////////////////////////////////>>

    return 0;
}


DWORD CDAudioManager_ResumeMCI()
{
    CDAM_MCI_REQUEST *resumeRequest = NULL;

    /////////////////////////////////////////////////////////////////////////>>

    EnterCriticalSection(&gCDAMRequestListCriticalSection);

    resumeRequest = CDAudioManager_AddRequest();
    if (resumeRequest != NULL)
    {
        utilPrintf("CDAudioManager << Producing RESUME Request >>\n");

        // Set MCI message type
        resumeRequest->requestType = MCI_RESUME;
        // Set flags
        resumeRequest->mciFlags = MCI_FROM;
        // Set params
        memset(&resumeRequest->mciPlayParms, 0, sizeof(MCI_PLAY_PARMS));
        resumeRequest->mciPlayParms.dwFrom = MCI_MAKE_TMSF(gCDAMData->pauseTMSF.track, gCDAMData->pauseTMSF.minute, gCDAMData->pauseTMSF.second, gCDAMData->pauseTMSF.frame);
    }

    LeaveCriticalSection(&gCDAMRequestListCriticalSection);

    /////////////////////////////////////////////////////////////////////////>>

    return 0;
}


DWORD CDAudioManager_ConvertTMSFToMilliseconds(TMSF tmsf)
{
    DWORD milliseconds = 0;
    
    milliseconds += tmsf.frame * 1000 / MCI_CD_FRAMES_PER_SECOND;
    milliseconds += tmsf.second * 1000;
    milliseconds += tmsf.minute * 60000;
    
    return milliseconds;
}


void CDAudioManager_ConvertMillisecondsToTMSF(DWORD milliseconds, TMSF *tmsf)
{
    tmsf->frame = (BYTE)((milliseconds % 1000) * MCI_CD_FRAMES_PER_SECOND / 1000);
    tmsf->second = (BYTE)((milliseconds / 1000) % 60);
    tmsf->minute = (BYTE)((milliseconds / 60000) % 60);
    tmsf->track = 1; // Assuming you're working with the first track
}


DWORD CDAudioManager_ConvertMSFToMilliseconds(MSF msf)
{
    DWORD milliseconds = 0;
    
    milliseconds += msf.frame * 1000 / MCI_CD_FRAMES_PER_SECOND;
    milliseconds += msf.second * 1000;
    milliseconds += msf.minute * 60000;
    
    return milliseconds;
}


DWORD WINAPI CDAudioManager_AudioThreadFunction(void *pData)
{
    int i = 0;
    CDAMData *pCDAMData = (CDAMData*)pData;

    MCIERROR dwResult = 0;
    MCI_OPEN_PARMS mciOpenParms = { 0 };
    MCI_SET_PARMS mciSetParms = { 0 };

    MCI_GENERIC_PARMS mciGenericParms = { 0 };
    MCI_STATUS_PARMS mciStatusParms = { 0 };
    MCI_STATUS_PARMS mciStatusParmsTrackLen = { 0 };
    DWORD trackPosInMs = 0, trackLenInMs = 0;
    int frameDelta = 0;

    CDAM_MCI_REQUEST *request = NULL;

	// Open the CD audio device by specifying the device name.
	mciOpenParms.lpstrDeviceType = "cdaudio";

	if (dwResult = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD_PTR)&mciOpenParms))
	{
		if(dwResult == MCIERR_MUST_USE_SHAREABLE)
		{
			utilPrintf("CDAMAudioThreadFunction() - Failed to open device %s, trying shareable...\n", mciOpenParms.lpstrDeviceType);
			if (dwResult = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE, (DWORD_PTR)&mciOpenParms))
			{
				utilPrintf("CDAMAudioThreadFunction() - Failed to open device %s\n", mciOpenParms.lpstrDeviceType);
				// Failed to open device. Don't close it; just return error.
				return 1;
			}
		}
        else
        {
            return 2;
        }
	}

	// The device opened successfully; get the device ID.
    pCDAMData->deviceID = mciOpenParms.wDeviceID;

	// Set the time format to track/minute/second/frame (TMSF).
	mciSetParms.dwTimeFormat = MCI_FORMAT_TMSF;

	if (dwResult = mciSendCommand(pCDAMData->deviceID, MCI_SET,MCI_SET_TIME_FORMAT, (DWORD_PTR)&mciSetParms))
	{
		utilPrintf("CDAMAudioThreadFunction() - Failed to set time format for cd device\n");
		mciSendCommand(pCDAMData->deviceID, MCI_CLOSE, 0, 0);
		return 3;
	}

    // Grab track lengths before we kick off the main audio processing loop
    for (i = 0; i < CDAM_NUM_TRACKS; i++)
    {
        if (gCDAMTrackInfo[i] != NULL)
        {
            mciStatusParmsTrackLen.dwCallback = (DWORD_PTR)NULL;
            mciStatusParmsTrackLen.dwItem = MCI_STATUS_LENGTH;
            mciStatusParmsTrackLen.dwTrack = i;
            
            if (mciSendCommand(pCDAMData->deviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD_PTR)&mciStatusParmsTrackLen))
            {
		        utilPrintf("CDAMAudioThreadFunction() - Failed to get track lengths for audio tracks\n");
		        mciSendCommand(pCDAMData->deviceID, MCI_CLOSE, 0, 0);
                return 4;
            }

            memcpy(&gCDAMTrackInfo[i]->length, &mciStatusParmsTrackLen.dwReturn, sizeof(MSF));
            
            //>>
            // [ANDYE] Hack to avoid end of CD audio glitching! Not sure how robust this is...
            if (i == 12)
            {
                frameDelta = MCI_CD_FRAMES_PER_SECOND / (1000 / gCDAMAudioIdlePeriod);
                if ((gCDAMTrackInfo[i]->length.frame - frameDelta) < 0)
                {
                    gCDAMTrackInfo[i]->length.second--;
                    gCDAMTrackInfo[i]->length.frame = MCI_CD_FRAMES_PER_SECOND - (frameDelta - gCDAMTrackInfo[i]->length.frame);
                }
                else
                {
                    gCDAMTrackInfo[i]->length.frame -= frameDelta;
                }
            }
            //>>
        }
    }

    // Run the processing loop continuously until notified to terminate
    while (gCDAMRunAudioThread)
    {
        if (pCDAMData->deviceID)
        {
            /////////////////////////////////////////////////////////////////>>
            /////////////////////////////////////////////////////////////////>>

            // Wait until we have thread-safe access to the CDAM request list
            EnterCriticalSection(&gCDAMRequestListCriticalSection);

            // Check to see if we have any outstanding requests that need processing
            if (gCDAMRequestList.numEntries > 0)
            {
                // Acquire a ptr to the tail (oldest) item on the request list
                request = gCDAMRequestList.head.prev;

                if (request)
                {
                    if (request->requestType == MCI_PLAY)
                    {
                        dwResult = mciSendCommand(pCDAMData->deviceID, MCI_PLAY, request->mciFlags, (DWORD_PTR)&request->mciPlayParms);
                        utilPrintf("CDAMAudioThreadFunction() - Consuming PLAY Request >> %s\n", (!dwResult) ? ("SUCCESS") : ("FAILED"));
                        if (dwResult)
                        {
                            // [ANDYE] TODO...?
                        }
                        else
                        {
                            pCDAMData->requestedTrack = MCI_TMSF_TRACK(request->mciPlayParms.dwFrom);
                            pCDAMData->isPaused = 0;
                        }
                    }
                    else if (request->requestType == MCI_STOP)
                    {
                        dwResult = mciSendCommand(pCDAMData->deviceID, MCI_STOP, request->mciFlags | MCI_WAIT, (DWORD_PTR)&request->mciGenericParms);
                        utilPrintf("CDAMAudioThreadFunction() - Consuming STOP Request >> \n", (!dwResult) ? ("SUCCESS") : ("FAILED"));
                        if (dwResult)
                        {
                            // [ANDYE] TODO...?
                        }
                        else
                        {
                            pCDAMData->requestedTrack = 0;
                            pCDAMData->isPaused = 0;
                        }
                    }
                    else if (request->requestType == MCI_PAUSE)
                    {
                        //dwResult = mciSendCommand(pCDAMData->deviceID, MCI_PAUSE, request->mciFlags, (DWORD_PTR)&request->mciGenericParms);
                        dwResult = mciSendCommand(pCDAMData->deviceID, MCI_STOP, request->mciFlags | MCI_WAIT, (DWORD_PTR)&request->mciGenericParms);
                        utilPrintf("CDAMAudioThreadFunction() - Consuming PAUSE Request >> \n", (!dwResult) ? ("SUCCESS") : ("FAILED"));
                        if (dwResult)
                        {
                            // [ANDYE] TODO...?
                        }
                        else
                        {
                            memcpy(&gCDAMData->pauseTMSF, &pCDAMData->currTrackPosTMSF, sizeof(TMSF));
                            pCDAMData->isPaused = 1;
                        }
                    }
                    else if (request->requestType == MCI_RESUME)
                    {
                        dwResult = mciSendCommand(pCDAMData->deviceID, MCI_PLAY, request->mciFlags, (DWORD_PTR)&request->mciPlayParms);
                        utilPrintf("CDAMAudioThreadFunction() - Consuming RESUME Request >> \n", (!dwResult) ? ("SUCCESS") : ("FAILED"));
                        if (dwResult)
                        {
                            // [ANDYE] TODO...?
                        }
                        else
                        {
                            pCDAMData->requestedTrack = MCI_TMSF_TRACK(request->mciPlayParms.dwFrom);
                            pCDAMData->isPaused = 0;
                        }
                    }
                    else
                    {
                		utilPrintf("CDAMAudioThreadFunction() - Consuming UNKNOWN Request\n");
                    }

                    // This request has been processed, remoe from the request list
                    CDAudioManager_RemoveRequest(request);

                    if (dwResult)
                    {
                        continue;
                    }
                }
            }

            // Let other threads know we are done with the CDAM request list
            LeaveCriticalSection(&gCDAMRequestListCriticalSection);

            /////////////////////////////////////////////////////////////////>>
            /////////////////////////////////////////////////////////////////>>

            if ((pCDAMData->requestedTrack < 2) || pCDAMData->isPaused)
            {
                Sleep(gCDAMAudioIdlePeriod);
                continue;
            }

            // Get current track number
            mciStatusParms.dwItem = MCI_STATUS_CURRENT_TRACK;
            if (mciSendCommand(pCDAMData->deviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatusParms))
            {
                // Handle error
                return 4;
            }
            pCDAMData->currTrackNum = mciStatusParms.dwReturn;

            //>>

            // Has the curently playing track changed?
            if (pCDAMData->currTrackNum != pCDAMData->requestedTrack)
            {
                // Change of track detected
                utilPrintf("<<<< TRACK CHANGE FROM %02d TO %02d >>>>\n", pCDAMData->requestedTrack, pCDAMData->currTrackNum);
                CDAudioManager_StopMCI();
                LoopSong();
                continue;
            }

            //>>

            // Get current track position
            trackPosInMs = 0;
            mciStatusParms.dwItem = MCI_STATUS_POSITION;
            if (mciSendCommand(pCDAMData->deviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatusParms))
            {
                // Handle error
                return 5;
            }
            memcpy(&pCDAMData->currTrackPosTMSF, &mciStatusParms.dwReturn, sizeof(TMSF));
            trackPosInMs = CDAudioManager_ConvertTMSFToMilliseconds(pCDAMData->currTrackPosTMSF);

            // Get the current track length
            trackLenInMs = 0;
#if 0
            mciStatusParms.dwItem = MCI_STATUS_LENGTH;
            mciStatusParms.dwTrack = pCDAMData->currTrackPosTMSF.track;
            if (mciSendCommand(pCDAMData->deviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, (DWORD_PTR)&mciStatusParms))
            {
                // Handle error
                return 6;
            }
            memcpy(&pCDAMData->currTrackLenMSF, &mciStatusParms.dwReturn, sizeof(MSF));
            trackLenInMs = CDAudioManager_ConvertMSFToMilliseconds(pCDAMData->currTrackLenMSF);
#else
            if (pCDAMData->currTrackPosTMSF.track > 1)
            {
                pCDAMData->currTrackLenMSF = gCDAMTrackInfo[pCDAMData->currTrackPosTMSF.track]->length;
                trackLenInMs = CDAudioManager_ConvertMSFToMilliseconds(pCDAMData->currTrackLenMSF);
            }
            else
            {
                Sleep(gCDAMAudioIdlePeriod);
                continue;
            }
#endif

            //if (gCDAMAudioIdlePeriod >= 250)
            //{
            //    utilPrintf("<< [Trk %02d] %02d:%02d:%02d (%02d:%02d:%02d) >>\n",
            //        pCDAMData->currTrackPosTMSF.track, pCDAMData->currTrackPosTMSF.minute, pCDAMData->currTrackPosTMSF.second, pCDAMData->currTrackPosTMSF.frame,
            //        pCDAMData->currTrackLenMSF.minute, pCDAMData->currTrackLenMSF.second, pCDAMData->currTrackLenMSF.frame);
            //}

            //>>

            //if (gCDAMAudioIdlePeriod >= 250)
            //{
            //      utilPrintf("<< [Trk %02d] %d / %d >>\n", pCDAMData->currTrackPosTMSF.track, trackPosInMs, trackLenInMs);
            //}

            if (trackPosInMs >= trackLenInMs)
            {
                // End of track reached
                utilPrintf("<<<< END OF TRACK %02d REACHED >>>>\n", pCDAMData->currTrackNum);
                //CDAudioManager_StopMCI();
                LoopSong();
                continue;
            }
        }

        Sleep(gCDAMAudioIdlePeriod);
    }

    return 0;
}
