/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: audio.h
	Programmer	: Andy Eder
	Date		: 28/06/99 10:37:57

----------------------------------------------------------------------------------------------- */

#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED
 


#include "types.h"
#include <islsfx2.h> 
#include "types.h"
#include "actor2.h"
#include <libcd.h>
#include <islxa.h>

// //***********************************
// // Defines
 
// Hmm - three flags. Worth it?
#define SFXFLAGS_GENERIC		(1<<0)
#define SFXFLAGS_3D_SAMPLE		(1<<1)
#define SFXFLAGS_LOOP			(1<<2)

 
#define AMBIENT_VOLUME	255
#define SAMPLE_VOLUME	255
 
#define AUDIOTRK_GAMEOVER			100
#define AUDIOTRK_LEVELCOMPLETE		101
#define AUDIOTRK_LEVELCOMPLETELOOP	102

extern char *musicNames[];
extern XAFileType *xaFileData[2];


 
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

	NUM_GENERIC_SFX,
};
 



typedef struct _SAMPLE
{
	unsigned long			flags;
	unsigned long			uid;

	SfxSampleType			*snd;
	int						handle;
} SAMPLE;

 
extern SAMPLE **sfx_anim_map;

#define MAX_SAMPLES 100
#define MAX_VOICES 12
 
typedef struct _SOUNDLIST
{
	SfxBankType		*genericBank;
	SfxBankType		*levelBank;
	SfxBankType		*loopBank;
	SfxBankType		*voiceBank[4];

	short count;

	SAMPLE			array[MAX_SAMPLES];
} SOUNDLIST;
 
 
 
typedef struct TAG_AMBIENT_SOUND
{
	struct TAG_AMBIENT_SOUND *next,*prev;

	SVECTOR		pos;
	ACTOR		*follow;

	struct _SAMPLE *sample;

	short		volume;
	short		pitch;
	long		radius;

	long		freq;
	long		randFreq;
	long		counter;
	int			handle;

}AMBIENT_SOUND;
 
 
typedef struct
{
	int array;
	AMBIENT_SOUND head;
	int numEntries;

}AMBIENT_SOUND_LIST; 
 
 extern SAMPLE *genSfx[];
void StopSample(SAMPLE *sample);
 
 
extern SAMPLE *FindSample( unsigned long uid );
 
 
void MAIN_PrintXAData ( const XAFileType * const XATrack );

#define InitVoices(path, len)	0
#define IsSongPlaying			XAcheckPlay

extern void PauseAudio( );
extern void UnPauseAudio( );

SAMPLE *FindVoice( unsigned long uid, int pl );
//#define FindVoice(uid, pl) FindSample(uid)
#define PlayVoice(pl, sound) PlaySample(FindVoice(utilStr2CRC(sound),pl), NULL, 0, SAMPLE_VOLUME, -1)

void PrepareSong ( short worldID,  short loop );

//#define PlayVoice(pl, sound) PlaySample(FindSample(utilStr2CRC(sound)), &frog[0]->actor->position, 0, SAMPLE_VOLUME, -1)

#endif
