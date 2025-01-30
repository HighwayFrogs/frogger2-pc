/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: audio.h
	Programmer	: Jim Hubbard
	Date		: 24/03/00

----------------------------------------------------------------------------------------------- */

#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

#include <windows.h>
#include <dsound.h>
#include <mdx.h>

#include "types.h"
#include "actor.h"



#ifdef __cplusplus
extern "C"
{
#endif


//***********************************
// Defines

#define SFXFLAGS_3D_SAMPLE		(1<<0)
#define SFXFLAGS_LOOP			(1<<1)

extern int AMBIENT_VOLUME;
extern int SAMPLE_VOLUME;

// All generic sfx
enum
{
	GEN_FROG_HOP,
	GEN_SUPER_HOP,
	GEN_DOUBLE_HOP,
	GEN_COLLECT_BABY,
	GEN_COLLECT_COIN,
	GEN_BABYHAPPY,
	GEN_BABYSAD,
	GEN_BABYCRY,
	GEN_BABYREPLY,
	GEN_TELEPORT,
	GEN_POWERUP,
	GEN_CLOCKTICK,
	GEN_POWERTICK,

	GEN_DEATHDROWN,
	GEN_DEATHCRUSH,
	GEN_DEATHEXPLODE,
	GEN_DEATHFALL,
	GEN_DEATHGIB,
	GEN_DEATHMOWED,
	GEN_DEATHCHOP,
	GEN_DEATHELECTRIC,
	GEN_DEATHFIRE,

	NUM_GENERIC_SFX
};

enum
{
	VOICE_TONGUE,
	VOICE_CROAK,
	VOICE_BELCH1,
	VOICE_BELCH2,
	VOICE_ANNOYED,
	VOICE_SLIDE,
	VOICE_HAPPY,
	VOICE_SCARED,
	VOICE_BORED,
	VOICE_LETSGO,

	VOICE_DEATHNORMAL,
	VOICE_DEATHDROWN1,

	NUM_VOICES
};

enum
{
	NOTRACK = 0,
	GARDEN_CDAUDIO			= 2,
	ANCIENTS_CDAUDIO,
	SPACE_CDAUDIO,
//	CITY_CDAUDIO,
	SUBTERRANEAN_CDAUDIO,
	LABORATORY_CDAUDIO,
	HALLOWEEN_CDAUDIO,
	SUPERRETRO_CDAUDIO,
	FRONTEND_CDAUDIO,
	LEVELCOMPLETE_CDAUDIO,
	GAMEOVER_CDAUDIO,
	LEVELCOMPLETELOOP_CDAUDIO,

	NUM_CD_TRACKS,
};

#define AUDIOTRK_GAMEOVER		100
#define AUDIOTRK_LEVELCOMPLETE	101
#define AUDIOTRK_LEVELCOMPLETELOOP 102

//***********************************
// Type Defines

typedef struct _SAMPLE
{
	struct _SAMPLE			*next, *prev;

//	char					*idName;
	unsigned long			flags;
	unsigned long			uid;

	LPWAVEFORMATEX			lpWavFmt;
	BYTE					*data;
	DWORD					len;

	LPDIRECTSOUNDBUFFER		lpdsBuffer;	
    LPDIRECTSOUND3DBUFFER   lpds3DBuffer;	// DirectSound 3D buffer interface

} SAMPLE;

extern SAMPLE **sfx_anim_map;

typedef struct _SOUNDLIST
{
	int				numEntries;
	SAMPLE			head;

} SOUNDLIST;


typedef struct _BUFSAMPLE
{
	struct _BUFSAMPLE *next, *prev;
	unsigned long			uid;
	LPDIRECTSOUNDBUFFER lpdsBuffer;

} BUFSAMPLE;


typedef struct _BUFFERLIST
{
	int			numEntries;
	BUFSAMPLE	head;

} BUFFERLIST;


typedef struct TAG_AMBIENT_SOUND
{
	struct TAG_AMBIENT_SOUND *next,*prev;

	SVECTOR		pos;
	MDX_ACTOR	*follow;

	struct _SAMPLE *sample;
	LPDIRECTSOUNDBUFFER		lpdsBuffer;

	short		volume;
	short		pitch;
	long		radius;

	long		freq;
	long		randFreq;
	long		counter;

}AMBIENT_SOUND;


typedef struct
{
	AMBIENT_SOUND head;
	int numEntries;

}AMBIENT_SOUND_LIST;


typedef struct
{
	unsigned long length, uid, loop;

} SAMPLE_HEADER;


// These are the original file-names determined from the backups. ~Knee.
extern const char *gCDAMTrackFolder;
extern const char *gCDAMTrackFilenames[NUM_CD_TRACKS];
extern SAMPLE *genSfx[];
extern SAMPLE voices[4];

extern SOUNDLIST soundList;					// Actual Sound Samples List

extern int oldVolume;	// previously set value for volume

//***********************************
// Function Prototypes

extern void FindSfxMapping( unsigned long uid, ACTOR *actor );

extern void LoadSfx( unsigned long worldID );

extern void InitSampleList( );
extern void FreeSampleList( );

extern void InitAmbientSoundList( );
extern void FreeAmbientSoundList( );
AMBIENT_SOUND *AddAmbientSound(SAMPLE *sample, SVECTOR *pos, long radius, short vol, short pitch, int freq, int randFreq, MDX_ACTOR *follow );

extern void FreeBufSampleList( );

extern SAMPLE *FindSample( unsigned long uid );
extern SAMPLE *FindVoice( unsigned long uid, int pl );

extern SAMPLE *CreateSampleFromWavFile( char *folder, char *file, long looped );

extern void CleanBufferSamples ( void );
extern void SetSampleFormat ( SAMPLE *sample );


extern int PlaySample(SAMPLE *sample,SVECTOR *pos,long radius,short volume,short pitch);
extern LPDIRECTSOUNDBUFFER PlaySampleMusic( SAMPLE *sample, SVECTOR *pos, long radius, short volume, short pitch );
int StopSample( SAMPLE *sample );
void PlaySfxMappedSample( MDX_ACTOR *act );
extern void UpdateAmbientSounds();

extern void InitVoices( char *path, int len );
#define PlayVoice(pl, sound) PlaySample(FindVoice(UpdateCRC(sound),pl), NULL, 0, SAMPLE_VOLUME, -1)

extern void ResetMusicVolume();

extern void PrepareSong(short num, short loop);
extern void LoopSong();
extern void StopSong();
extern int IsSongPlaying();

extern void PauseAudio();
extern void UnPauseAudio();

int InitCDaudio();
int ShutdownMusic();
int GetCDVolume();
void SetCDVolume(int vol);

DWORD PlayCDTrackFromFile(int track, long loop);

#ifdef __cplusplus
}
#endif

#endif
