/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: dx_sound.cpp
	Programmer	: James Healey
	Date		: 16/08/99

----------------------------------------------------------------------------------------------- */

//***********************************
// System Includes

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <dsound.h>
#include <dinput.h>
#include <stdio.h>

#include <islutil.h>
#include "frogger.h"
#include "pcaudio.h"
#include "dx_sound.h"
#include "controll.h"

//***********************************
// User Includes

#include "..\resource.h"


//***********************************
// User Includes

LPDIRECTSOUND           lpDS			= NULL;
LPDIRECTSOUNDBUFFER		lpdsbPrimary	= NULL;
LPDIRECTSOUND3DLISTENER lpds3DListener	= NULL;


int showSounds = 0;
int siPlaySound = 0;
SAMPLE *dispSample = NULL;

//***********************************
// Function Definitions

static char *DSoundErrorToString ( int dxerror )
{
	switch ( dxerror )
		{
		case DS_OK:
			return "The request completed successfully.";

		case DSERR_ALLOCATED:
			return "The request failed because resources, such as a priority level, were already in use by another caller.";

		case DSERR_ALREADYINITIALIZED:
			return "The object is already initialized.";

		case DSERR_BADFORMAT:
			return "The specified wave format is not supported.";

		case DSERR_BUFFERLOST:
			return "The buffer memory has been lost and must be restored.";

		case DSERR_CONTROLUNAVAIL:
			return "The control (volume, pan, and so forth) requested by the caller is not available.";

		case DSERR_GENERIC:
			return "An undetermined error occurred inside the DirectSound subsystem.";

		case DSERR_INVALIDCALL:
			return "This function is not valid for the current state of this object.";

		case DSERR_INVALIDPARAM:
			return "An invalid parameter was passed to the returning function.";

		case DSERR_NOAGGREGATION:
			return "The object does not support aggregation.";

		case DSERR_NODRIVER:
			return "No sound driver is available for use.";

		case DSERR_OTHERAPPHASPRIO:
			return "Another application has a higher priority level, preventing this call from succeeding";

		case DSERR_OUTOFMEMORY:
			return "The DirectSound subsystem could not allocate sufficient memory to complete the caller's request.";

		case DSERR_PRIOLEVELNEEDED:
			return "The caller does not have the priority level required for the function to succeed.";

		case DSERR_UNINITIALIZED:
			return "The IDirectSound_Initialize method has not been called or has not been called successfully before other methods were called.";

		case DSERR_UNSUPPORTED:
			return "The function called is not supported at this time.";
		}

	return "Unknown Error";
}


int InitDirectSound( HINSTANCE hInst,  HWND hWndMain )
{
	WAVEFORMATEX	wfx;
	DSBUFFERDESC	dsbdesc;
	HRESULT			dsrVal;
	DSCAPS			dsCaps;

	dsrVal = DirectSoundCreate ( NULL, &lpDS, NULL );

	if ( dsrVal != DS_OK )
	{
		utilPrintf("DirectSound Create failed - '%s'\n", DSoundErrorToString(dsrVal));
		lpDS = NULL;
		return 0;
	}

	dsrVal = lpDS->SetCooperativeLevel ( hWndMain, DSSCL_EXCLUSIVE );

	if ( dsrVal != DS_OK )
	{
		utilPrintf("Set Cooperative Level failed - '%s'\n", DSoundErrorToString(dsrVal));
		lpDS->Release();
		lpDS = NULL;
		return 0;
	}

	memset(&dsCaps, 0, sizeof(dsCaps));
	dsCaps.dwSize = sizeof(DSCAPS);
	dsrVal = lpDS->GetCaps(&dsCaps);
	if(dsrVal != DS_OK)
		utilPrintf("bInitSpecificWave: WARNING Could not retrieve device caps\n");
	else
	{
		utilPrintf("\nDirectSound caps:\n");
		DumpDSCaps(&dsCaps);
		utilPrintf("\n");
	}

	ZeroMemory ( &dsbdesc, sizeof ( DSBUFFERDESC ) );
	dsbdesc.dwSize	= sizeof ( DSBUFFERDESC );
	dsbdesc.dwFlags = /*DSBCAPS_CTRL3D |*/ DSBCAPS_PRIMARYBUFFER;

	
	dsrVal = lpDS->CreateSoundBuffer ( &dsbdesc, &lpdsbPrimary, NULL ) ;
	if ( dsrVal != DS_OK )
	{
		utilPrintf("Create Sound Buffer failed - '%s'\n", DSoundErrorToString(dsrVal));
		lpDS->Release();
		lpDS = NULL;
		return 0;
	}

	memset ( &wfx, 0, sizeof ( WAVEFORMATEX ) ); 
	wfx.wFormatTag		= WAVE_FORMAT_PCM;
	wfx.nChannels		= 2;
	wfx.nSamplesPerSec	= 22050; 
	wfx.wBitsPerSample	= 16; 
	wfx.nBlockAlign		= wfx.wBitsPerSample / 8 * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
 
	dsrVal = lpdsbPrimary->SetFormat ( &wfx );

	if ( dsrVal != DS_OK )
	{
		utilPrintf("Set Format failed - '%s'\n", DSoundErrorToString(dsrVal));
		return 0;
	}

/*
    dsrVal = lpdsbPrimary->QueryInterface( IID_IDirectSound3DListener, ( void** ) &lpds3DListener );

	if ( dsrVal != DS_OK )
	{
		utilPrintf("Query Interface For 3d Listener Failed - '%s'\n", DSoundErrorToString(dsrVal));
		return 0;
	}
*/
	InitCDaudio();

	return 1;
}


void ShutDownDirectSound ( void )
{
	ShutdownMusic();

	if( lpDS )
		lpDS->Release();
}

int LoadWavFile( SAMPLE *sample, char *name )
{
	HRESULT			dsrVal;
	HMMIO 			hwav;    // handle to wave file
	MMCKINFO		parent,  // parent chunk
					child;   // child chunk
	WAVEFORMATEX    wfmtx;   // wave format structure


	UCHAR *snd_buffer,       // temporary sound buffer to hold voc data
		  *audio_ptr_1=NULL, // data ptr to first write buffer 
		  *audio_ptr_2=NULL; // data ptr to second write buffer

	unsigned long	audio_length_1=0,  // length of first write buffer
					audio_length_2=0;  // length of second write buffer

	DSBUFFERDESC		dsbd;           // directsound description

	// open the WAV file
	if( (hwav = mmioOpen( name, NULL, MMIO_READ | MMIO_ALLOCBUF )) == NULL )
	{
		utilPrintf("RETURNING : LoadWavFile - Opening Wav File Failure '%s'.\n", name);
		return 0;
	}

	// descend into the RIFF 
	parent.fccType = mmioFOURCC ( 'W', 'A', 'V', 'E' );

	if ( mmioDescend ( hwav, &parent, NULL, MMIO_FINDRIFF ) )
    {
		// close the file
		mmioClose ( hwav, 0 );

		// return error, no wave section
		return 0; 	
    }

	// descend to the WAVEfmt 
	child.ckid = mmioFOURCC ( 'f', 'm', 't', ' ' );

	if ( mmioDescend ( hwav, &child, &parent, 0 ) )
    {
		// close the file
		mmioClose ( hwav, 0 );

		// return error, no format section
		return 0; 	
    }

	// now read the wave format information from file
	if ( mmioRead ( hwav, ( char * ) &wfmtx, sizeof ( wfmtx ) ) != sizeof ( wfmtx ) )
    {
		// close file
		mmioClose ( hwav, 0 );

		// return error, no wave format data
		return 0;
    }

	// make sure that the data format is PCM
	if ( wfmtx.wFormatTag != WAVE_FORMAT_PCM )
    {
		// close the file
		mmioClose ( hwav, 0 );

		// return error, not the right data format
		return 0; 
    }

	// now ascend up one level, so we can access data chunk
	if ( mmioAscend ( hwav, &child, 0 ) )
	{
		// close file
		mmioClose ( hwav, 0 );

		// return error, couldn't ascend
		return 0; 	
	}

	// descend to the data chunk 
	child.ckid = mmioFOURCC ( 'd', 'a', 't', 'a' );

	if ( mmioDescend ( hwav, &child, &parent, MMIO_FINDCHUNK ) )
    {
		// close file
		mmioClose ( hwav, 0 );

		// return error, no data
		return 0; 	
    }

	// finally!!!! now all we have to do is read the data in and
	// set up the directsound buffer

	// allocate the memory to load sound data
	snd_buffer = ( UCHAR * ) AllocMem( child.cksize );

	// read the wave data 
	mmioRead ( hwav, ( char * ) snd_buffer, child.cksize );

	// close the file
	mmioClose ( hwav, 0 );

	// prepare to create sounds buffer
	ZeroMemory ( &dsbd, sizeof ( DSBUFFERDESC ) );
	dsbd.dwSize			= sizeof(DSBUFFERDESC);
	dsbd.dwFlags		= DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC;

	dsbd.dwBufferBytes	= child.cksize;
	dsbd.lpwfxFormat	= &wfmtx;

	sample->len = child.cksize;

	// create the sound buffer

	dsrVal = lpDS->CreateSoundBuffer ( &dsbd, &sample->lpdsBuffer, NULL );
	if ( dsrVal != DS_OK )
	{
		utilPrintf("CreateSoundBuffer failed on file '%s' - '%s'\n", name, DSoundErrorToString(dsrVal));

		// release memory
		FreeMem ( snd_buffer );

		// return error
		return ( -1 );
	}

	// copy data into sound buffer
	if ( sample->lpdsBuffer->Lock ( 0, child.cksize, (void **) &audio_ptr_1, &audio_length_1, (void **)&audio_ptr_2, &audio_length_2, DSBLOCK_FROMWRITECURSOR ) != DS_OK )
		 return(0);

	// copy first section of circular buffer
	memcpy(audio_ptr_1, snd_buffer, audio_length_1);

	// copy last section of circular buffer
	memcpy(audio_ptr_2, (snd_buffer+audio_length_1),audio_length_2);

	// unlock the buffer
	if (sample->lpdsBuffer->Unlock(audio_ptr_1, 
											audio_length_1, 
											audio_ptr_2, 
											audio_length_2)!=DS_OK)
 									 return(0);

	// release the temp buffer
	FreeMem(snd_buffer);

	return 1;
}


int LoadWav( SAMPLE *sample, char *data, int length )
{
	UCHAR *audio_ptr_1=NULL, // data ptr to first write buffer 
		  *audio_ptr_2=NULL; // data ptr to second write buffer

	unsigned long	audio_length_1=0,  // length of first write buffer
					audio_length_2=0;  // length of second write buffer

	DSBUFFERDESC	dsbd;           // directsound description
	WAVEFORMATEX    wfmtx;   // wave format structure
	HRESULT			dsrVal;

	// Read sound format and advance data pointer
	memcpy( (void *)&wfmtx, (const void *)data, sizeof(WAVEFORMATEX) );
	data += sizeof(WAVEFORMATEX);

	// prepare to create sounds buffer
	ZeroMemory ( &dsbd, sizeof ( DSBUFFERDESC ) );
	dsbd.dwSize			= sizeof(DSBUFFERDESC);
	dsbd.dwFlags		= DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC;
	dsbd.dwBufferBytes	= length;
	dsbd.lpwfxFormat	= &wfmtx;

	// create the sound buffer
	dsrVal = lpDS->CreateSoundBuffer ( &dsbd, &sample->lpdsBuffer, NULL );
	if( dsrVal != DS_OK )
	{
		utilPrintf( "CreateSoundBuffer failed on file '%i' - %s\n", sample->uid, DSoundErrorToString(dsrVal) );
		return -1;
	}

	// copy data into sound buffer
	if ( sample->lpdsBuffer->Lock ( 0, length, (void **) &audio_ptr_1, &audio_length_1, (void **)&audio_ptr_2, &audio_length_2, DSBLOCK_FROMWRITECURSOR ) != DS_OK )
		 return(0);

	// copy first section of circular buffer
	memcpy(audio_ptr_1, data, audio_length_1);

	// copy last section of circular buffer
	memcpy(audio_ptr_2, (data+audio_length_1),audio_length_2);

	// unlock the buffer
	if (sample->lpdsBuffer->Unlock(audio_ptr_1, 
											audio_length_1, 
											audio_ptr_2, 
											audio_length_2)!=DS_OK)
 									 return(0);

	return 1;
}


void Update3DListener ( float sourceX, float sourceY, float sourceZ )
{
	lpds3DListener->SetPosition			( 0.0, 0.0, 0.0, DS3D_IMMEDIATE );
	lpds3DListener->SetDopplerFactor	( 1.0f, DS3D_IMMEDIATE );
	lpds3DListener->SetDistanceFactor	( 5.0f, DS3D_IMMEDIATE );

//	lpds3DListener->CommitDeferredSettings

}




void Get3DInterface ( LPDIRECTSOUNDBUFFER lpdsBuffer, LPDIRECTSOUND3DBUFFER lpds3DBuffer )
{
	lpdsBuffer->QueryInterface ( IID_IDirectSound3DBuffer, (void**)&lpds3DBuffer );
}


void ReleaseBuffer ( LPDIRECTSOUNDBUFFER lpdsBuffer )
{
	lpdsBuffer->Release();
}

void Set3DPosition ( LPDIRECTSOUND3DBUFFER lpds3DBuffer, float xPos, float yPos, float zPos )
{
	lpds3DBuffer->SetPosition ( 0.0f, 0.0f, 10.0f, DS3D_IMMEDIATE );
	lpds3DBuffer->SetMinDistance ( 4.0, DS3D_IMMEDIATE );
}

void ShowSounds(void)
{
	RECT r;
	SAMPLE *me;
	HDC hdc;

	r.left = 0;
	r.top = 0;
	r.right = 300;
	r.bottom = rYRes - 1;
	
	DrawTexturedRect(r,D3DRGBA(0,0,0,0.8),NULL,0,0,1,1);

	EndDraw( );
	
	HRESULT res = IDirectDrawSurface4_GetDC(surface[RENDER_SRF], &hdc);
	if ((res == DD_OK))
	{
		char tText[256];
		long tPC = 0;
		HFONT hfnt, hOldFont;      
		hfnt = (HFONT)GetStockObject(ANSI_VAR_FONT); 
		int count=0;

		if (hOldFont = (HFONT)SelectObject(hdc, hfnt)) 
		{
			SetBkMode(hdc, TRANSPARENT);
	
			// Draw the info
			r.left = 32;
			r.top = 32;
			r.right = r.left + 64;
			r.bottom = r.top + 64;

			if( !dispSample )
				dispSample = soundList.head.next;

			me = dispSample;

			SetTextColor(hdc, 0x00ffffff);
			sprintf(tText,"Number of Sounds: %i",soundList.numEntries);
			TextOut(hdc, r.left+70, r.top, tText, strlen(tText));
			r.top += 50;
						
			while (me && (r.bottom < rYRes - 32))	
			{
				r.left = 32;
				r.right = r.left + 64;
				
				// If we're displaying the selected sound, hilight.
				if( !count )
				{
					SetTextColor(hdc, 0x00ffffff);

					if( siPlaySound )
						PlaySample( me, NULL, 0, SAMPLE_VOLUME, -1 );
					siPlaySound = 0;
				}
				else
				{
					SetTextColor(hdc, 0x006464e6);
				}
				
//				sprintf(tText,"%s",me->idName);
//				TextOut(hdc, r.left+70, r.top, tText, strlen(tText));
				
				sprintf(tText,"Length %i",me->len);
				TextOut(hdc, r.left+70, r.top+15, tText, strlen(tText));
				
				me = me->next;
				count++;

				r.top += 25;
				r.bottom += 25;
			}
					
			SelectObject(hdc, hOldFont); 
		}
		
		IDirectDrawSurface4_ReleaseDC(surface[RENDER_SRF], hdc);	
	}
	
	BeginDraw();
}

/*	--------------------------------------------------------------------------------
	Function : DumpDSCaps
	Purpose : dump the contents of a DSCAPS structure to the debug stream
	Parameters : ptr to DSCAPS to dump
	Returns : 
	Info : 
*/

void DumpDSCaps(DSCAPS *caps)
{
	utilPrintf("Flags:\n");
	if (caps->dwFlags & DSCAPS_CERTIFIED)
		utilPrintf("  This driver has been tested and certified by Microsoft.\n");

	if (caps->dwFlags & DSCAPS_CONTINUOUSRATE)
		utilPrintf("  The device supports all sample rates between the dwMinSecondarySampleRate and dwMaxSecondarySampleRate member values. Typically, this means that the actual output rate will be within +/- 10 hertz (Hz) of the requested frequency.\n");

	if (caps->dwFlags & DSCAPS_EMULDRIVER)
		utilPrintf("  The device does not have a DirectSound driver installed, so it is being emulated through the waveform-audio functions. Performance degradation should be expected.\n");

	if (caps->dwFlags & DSCAPS_PRIMARY16BIT)
		utilPrintf("  The device supports primary sound buffers with 16-bit samples.\n");

	if (caps->dwFlags & DSCAPS_PRIMARY8BIT)
		utilPrintf("  The device supports primary buffers with 8-bit samples.\n");

	if (caps->dwFlags & DSCAPS_PRIMARYMONO)
		utilPrintf("  The device supports monophonic primary buffers.\n");

	if (caps->dwFlags & DSCAPS_PRIMARYSTEREO)
		utilPrintf("  The device supports stereo primary buffers.\n");

	if (caps->dwFlags & DSCAPS_SECONDARY16BIT)
		utilPrintf("  The device supports hardware-mixed secondary sound buffers with 16-bit samples.\n");

	if (caps->dwFlags & DSCAPS_SECONDARY8BIT)
		utilPrintf("  The device supports hardware-mixed secondary buffers with 8-bit samples.\n");

	if (caps->dwFlags & DSCAPS_SECONDARYMONO)
		utilPrintf("  The device supports hardware-mixed monophonic secondary buffers.\n");

	if (caps->dwFlags & DSCAPS_SECONDARYSTEREO)
		utilPrintf("  The device supports hardware-mixed stereo secondary buffers.\n");

	utilPrintf("Min sample rate for secondary buffers = %d Hz\n", caps->dwMinSecondarySampleRate);
	utilPrintf("Max sample rate for secondary buffers = %d Hz\n", caps->dwMaxSecondarySampleRate);
	utilPrintf("Max hardware mixing buffers = %d\n", caps->dwMaxHwMixingAllBuffers);
	utilPrintf("Max static sound buffers = %d\n", caps->dwMaxHwMixingStaticBuffers);
	utilPrintf("Max streaming sound buffers = %d\n", caps->dwMaxHwMixingStreamingBuffers);
	utilPrintf("Free hardware mixing buffers = %d\n", caps->dwFreeHwMixingAllBuffers);
	utilPrintf("Free static hardware mixing buffers = %d\n", caps->dwFreeHwMixingStaticBuffers);
	utilPrintf("Free streaming hardware mixing buffers = %d\n", caps->dwFreeHwMixingStreamingBuffers);
	utilPrintf("Max 3D hardware buffers = %d\n", caps->dwMaxHw3DAllBuffers);
	utilPrintf("Max 3D static hardware buffers = %d\n", caps->dwMaxHw3DStaticBuffers);
	utilPrintf("Max 3D streaming hardware buffers = %d\n", caps->dwMaxHw3DStreamingBuffers);
	utilPrintf("Free 3D hardware buffers = %d\n", caps->dwFreeHw3DAllBuffers);
	utilPrintf("Free 3D static hardware buffers = %d\n", caps->dwFreeHw3DStaticBuffers);
	utilPrintf("Free 3D streaming hardware buffers = %d\n", caps->dwFreeHw3DStreamingBuffers);
	utilPrintf("Hardware RAM for static buffers = %.1f Kb\n", ((float)caps->dwTotalHwMemBytes)/1024.0f);
	utilPrintf("Free hardware RAM = %.1f Kb\n", ((float)caps->dwFreeHwMemBytes)/1024.0f);
	utilPrintf("Largest contiguous RAM = %.1f Kb\n", ((float)caps->dwMaxContigFreeHwMemBytes)/1024.0f);
	utilPrintf("Static transfer to hardware rate = %d Kb/sec\n", caps->dwUnlockTransferRateHwBuffers);
	utilPrintf("Software processor cost = %d%%\n", caps->dwPlayCpuOverheadSwBuffers);
}


