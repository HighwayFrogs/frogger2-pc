/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: audio.c
	Programmer	: Random Healey
	Date		: 28/06/99 10:37:45

----------------------------------------------------------------------------------------------- */

// #include <ultra64.h>
// #include "..\resource.h"
// #include "incs.h"
#include <islmem.h>
#include <libspu.h>
#include <libsnd.h>

#include "audio.h"

#include "main.h"

#include "memload.h"
#include "frogger.h"
#include "layout.h"
#include "menus.h"
#include "game.h"
#include "cam.h"

int lastSound = -1;

char *musicNames[] = {	"CDA.XA",
						"CDB.XA",
					 };

int musicList[] = { 0,//
					1,//
					2,//
					6,//
					3,//
					4,//
					5,//
					6,//
					7,
					0,
					1,
					2,
};//


#define SFX_BASE		"SFX\\"

 
//#define MAX_AMBIENT_SFX		50
#define DEFAULT_SFX_DIST	700*SCALE

//This value means the pitch can go up to 100000Hz like the PC. This might be a 
//problem for the PSX but I don't think it goes much above 20000Hz, which seems alright. May
//cause problems in the future though, so check!!
#define PITCH_STEP		190 //(DSBFREQUENCY_MAX/256)   //MM NEEDS CHANGING!!!!!

 
//#define VOLUME_MIN		-5000
//#define VOLUME_PERCENT (VOLUME_MIN/100)
//#define PITCH_STEP		(DSBFREQUENCY_MAX/256)
 
SAMPLE voiceArray[4][MAX_VOICES];

SAMPLE *genSfx[NUM_GENERIC_SFX];
// SAMPLE **sfx_anim_map = NULL;

XAFileType	*xaFileData[2];
 
// UINT mciDevice = 0;
 
SOUNDLIST soundList;					// Actual Sound Samples List
AMBIENT_SOUND_LIST	ambientSoundList;
//BUFFERLIST bufferList;					// Buffered Sound Samples List
 
 
// void AddSample( SAMPLE *sample );
// void RemoveSample( SAMPLE *sample );
// void AddBufSample( BUFSAMPLE *sample );
// void RemoveBufSample( BUFSAMPLE *sample );
 
// void SubAmbientSound(AMBIENT_SOUND *ambientSound);


int GetSoundVols(SVECTOR *pos,int *vl,int *vr,long radius,unsigned long vol);

int UpdateLoopingSample(AMBIENT_SOUND *sample)
{
	int vl,vr,vs;
	short pitch;

	if (!sample)
	{
		utilPrintf("Sample Not Valid!!!!!!\n");
		return;
	}
	
	if(GetSoundVols(&sample->pos,&vl,&vr,sample->radius,sample->volume) == -1)
	{
		if(sample->handle >= 0)
		{
			sfxStopChannel(sample->handle);
			sample->handle = -1;
		}
		return;
	}
	
	if(sample->pitch == -1)
	{
		pitch = 0;
	}
	else
	{
		pitch = sample->pitch * PITCH_STEP;
	}

	if((sample->handle == -1) && (sample->sample) && (sample->sample->snd))
	{
		vs = VSync(1);
		while((lastSound>=0) && (SpuGetKeyStatus(1<<lastSound)==SPU_ON_ENV_OFF) && (VSync(1)<vs+3));

 		lastSound = sample->handle = sfxPlaySample(sample->sample->snd, vl,vr, pitch);
		return;
	}

	if(pitch)
		sfxSetChannelPitch(sample->handle,pitch);
	sfxSetChannelVolume(sample->handle, vl, vr);
}
 
// SAMPLE *CreateAndAddSample( char *path, char *file );
 
// void SetSampleFormat ( SAMPLE *sample );
// void CleanBufferSamples( void );
 
// void PrepareSong( char num, char slot );
// void StopSong( );
// DWORD playCDTrack( HWND hWndNotify, BYTE bTrack );
// DWORD stopCDTrack( HWND hWndNotify );
 
 
/*	--------------------------------------------------------------------------------
	Function		:StartSound
	Purpose		    :  Initialises the PSX sound chip and the soundlibraries
	Parameters		: none
	Returns			: 
	Info			: Should only be called once at the start of the game loop. 
*/
void StartSound()
{
	sfxInitialise(0);
 	sfxStartSound();
 	sfxOn();
 	sfxSetGlobalVolume(255);// set to max volume 
}



/*	--------------------------------------------------------------------------------
	Function		:StopSound
	Purpose		    :  Turns off the PSX sound chip and the soundlibraries
	Parameters		: none
	Returns			: 
	Info			: Should only be called once at the end of the game loop. 
*/
void StopSound()
{
	FreeSampleList();
//free the sfxBankType pointers
	FREE(soundList.genericBank);
	FREE(soundList.levelBank);
	FREE(soundList.loopBank);
	sfxOff();
	sfxStopSound();
	sfxDestroy();
}





/*	--------------------------------------------------------------------------------
	Function		: InitSampleList
	Purpose			: Initialises the sample list
	Parameters		: 
	Returns			: 
	Info			: 
*/
void InitSampleList( )
{
	//Init the sample list for the real samples.

	int i;
	if(soundList.genericBank)
	{
		sfxUnloadSampleBank(soundList.genericBank);
		sfxRemoveSampleBank(soundList.genericBank,1);
		soundList.genericBank = NULL;
	}
	if(soundList.levelBank)
	{
		sfxUnloadSampleBank(soundList.levelBank);
		sfxRemoveSampleBank(soundList.levelBank,1);
		soundList.levelBank = NULL;
	}
	if(soundList.loopBank)
	{
		sfxUnloadSampleBank(soundList.loopBank);
		sfxRemoveSampleBank(soundList.loopBank,1);
		soundList.loopBank = NULL;
	}
	for(i = 0;i < 4;i++)
	{
		if(soundList.voiceBank[i])
		{
			sfxUnloadSampleBank(soundList.voiceBank[i]);
			sfxRemoveSampleBank(soundList.voiceBank[i],1);
			soundList.voiceBank[i] = NULL;
		}
	}

	soundList.count = 0;
}


/*	--------------------------------------------------------------------------------
	Function		: LoadSfxSet
	Purpose			: Loads the soundbank and adds samples to list
	Parameters		: path to be loaded, adn whether the bank is the generic bank
	Returns			: Success
	Info			: 
*/
int LoadSfxSet(char *path, SfxBankType **sfxBank,int flags,SAMPLE *array,short *count)
{
	int i;
//	SfxBankType *sfxBank=0;
	SfxSampleType *snd;

	*sfxBank = 0;

	*sfxBank = sfxLoadSampleBank(path);	
	if(!(*sfxBank))
	{
	 	utilPrintf("Can't load sound bank %s\n", path);
		return 0;
	}

	snd =(SfxSampleType *) (*sfxBank)->sample;

	for  ( i=0; i<(*sfxBank)->numSamples; i++)
	{
		if (!snd)
		{
			utilPrintf("oops...can't find sample %i in bank %s\n",i,path);
			continue;
		}
		snd = sfxDownloadSample(snd);
		CreateAndAddSample(snd,flags,array,count);

		*snd++;
	}
/*
	if (generic > 0)
		soundList.genericBank = sfxBank;
	else if(generic < 0)
		soundList.loopBank = sfxBank;
	else
		soundList.levelBank = sfxBank;
*/
 	sfxDestroySampleBank( *sfxBank );		

	return 1;
}





/*	--------------------------------------------------------------------------------
	Function		: LoadSfx
	Purpose			: Loads a sound bank, and puts all the samples in the sample list
	Parameters		: number of world sound bank to load
	Returns			: Success?
	Info			: 
*/
short voiceCount[4];
int LoadSfx(long worldID )
{
	char *path;
	int len,j;
	
 	len = strlen(SFX_BASE);
 	path = (char *)MALLOC0( len+32 );

	strcat( path, SFX_BASE );

	// Load all generic samples first and put in array
//	strcat( path, "GENERIC\\SFX" );

//	LoadSfxSet(path, &soundList.genericBank,0,&soundList.array[0],&soundList.count);

	if(worldID != -1)
	{
		for(j = 0;j < NUM_FROGS;j++)
		{
			path[len] = '\0';
			voiceCount[j] = 0;
			strcat(path,frogPool[player[j].character].fileName);
			strcat(path,"\\SFX");
			LoadSfxSet(path, &soundList.voiceBank[j],0,&voiceArray[j][0],&voiceCount[j]);
		}
	}

	path[len] = '\0';


	if ( worldID == 0xffff )
		return 0;
	// ENDIF

	utilPrintf ( "WorldID Is : %d\n", worldID );
//load the non-looping level samples

	if(worldID == -1)
		strcat(path,"SYSTEM\\");
	else if(gameState.multi == SINGLEPLAYER)
	{
		switch( worldID )
		{
			case WORLDID_GARDEN: strcat( path, "GARDEN\\" ); break;
			case WORLDID_ANCIENT: strcat( path, "ANCIENTS\\" ); break;
			case WORLDID_SPACE: strcat( path, "SPACE\\" ); break;
			case WORLDID_CITY: strcat( path, "CITY\\" ); break;
			case WORLDID_SUBTERRANEAN: strcat( path, "SUB\\" ); break;
			case WORLDID_LABORATORY: strcat( path, "LAB\\" ); break;
			case WORLDID_HALLOWEEN: strcat( path, "HALLOWEEN\\" ); break;
			case WORLDID_SUPERRETRO: strcat( path, "SUPERRETRO\\" ); break;
			case WORLDID_FRONTEND: strcat( path, "FRONTEND\\" ); break;
		}
	}
	else
		strcat(path,"MULTI\\");

 	len = strlen(path);

	strcat( path, "SFX" );

	LoadSfxSet(path, &soundList.levelBank,0,&soundList.array[0],&soundList.count);

	genSfx[GEN_FROG_HOP] = FindSample(utilStr2CRC("hopongrass"));
	genSfx[GEN_SUPER_HOP] = FindSample(utilStr2CRC("hop2"));
	genSfx[GEN_DOUBLE_HOP] = FindSample(utilStr2CRC("doublehop"));
	genSfx[GEN_COLLECT_BABY] = FindSample(utilStr2CRC("getbabyfrog"));
	genSfx[GEN_COLLECT_COIN] = FindSample(utilStr2CRC("pickupcoin"));
	genSfx[GEN_BABYHAPPY] = FindSample(utilStr2CRC("babyhappy"));
	genSfx[GEN_BABYSAD] = FindSample(utilStr2CRC("babysad"));
	genSfx[GEN_BABYCRY] = FindSample(utilStr2CRC("babycry"));
	genSfx[GEN_BABYREPLY] = FindSample(utilStr2CRC("babyreply"));
	genSfx[GEN_TELEPORT] = FindSample(utilStr2CRC("teleport"));

	genSfx[GEN_POWERUP] = FindSample(utilStr2CRC("powerup"));

	genSfx[GEN_CLOCKTICK] = FindSample(utilStr2CRC("clocktick"));
	genSfx[GEN_POWERTICK] = FindSample(utilStr2CRC("puptick"));

	genSfx[GEN_DEATHDROWN] = FindSample(utilStr2CRC("frogdrown1"));
	genSfx[GEN_DEATHCRUSH] = FindSample(utilStr2CRC("frogcrush"));
	genSfx[GEN_DEATHEXPLODE] = FindSample(utilStr2CRC("frogexplode"));
	genSfx[GEN_DEATHFALL] = FindSample(utilStr2CRC("frogfall"));
	genSfx[GEN_DEATHMOWED] = FindSample(utilStr2CRC("frogmowed"));
	genSfx[GEN_DEATHGIB] = FindSample(utilStr2CRC("frog_gib"));
	genSfx[GEN_DEATHCHOP] = FindSample(utilStr2CRC("frogchop"));
	genSfx[GEN_DEATHELECTRIC] = FindSample(utilStr2CRC("electrocute"));
	genSfx[GEN_DEATHFIRE] = FindSample(utilStr2CRC("burnbum"));


// JH: Quite Possibly fix it!!!!!!!!!!!!!!!!!!!!!!!!!!
	path[len] = '\0';


//load the looping level samples

/*	if(worldID != -1)
	{
		len = strlen(path);

		strcat( path, "LOOP" );

		LoadSfxSet(path, &soundList.loopBank,SFXFLAGS_LOOP,&soundList.array[0],&soundList.count);
	}
*/			
	FREE( path );

	return 1;
}




/*	--------------------------------------------------------------------------------
	Function		:CreateAndAddSamples
	Purpose			: Adds a sample to the sample list
	Parameters		: The sound effect to be added to the sample structure
	Returns			: 
	Info			: 
*/
SAMPLE *CreateAndAddSample(SfxSampleType *snd,int flags,SAMPLE *array,short *count)
{
	SAMPLE *newItem;
	
	if(( soundList.count >= MAX_SAMPLES ) || ( !( newItem = array + *count)))
		return NULL;

	//sfx = (SAMPLE *)MALLOC0(sizeof(SAMPLE));

	newItem->uid = snd->nameCRC;
	newItem->snd = snd;
	newItem->flags = flags;

	(*count)++;

	return newItem;
}

/*	--------------------------------------------------------------------------------
	Function		: FindSample
	Purpose			: Search the list for the required sound
	Parameters		: ID
	Returns			: Sample or NULL
	Info			: 
*/
SAMPLE *FindSample( unsigned long uid )
{
	int i;

	if (!uid)
		return NULL;

	for(i = 0;i < soundList.count;i++)
	{
		if(soundList.array[i].uid == uid)
			return &soundList.array[i];
	}

	return NULL;	
}
 

void FreeSampleBlock( )
{
	soundList.count = 0;
}


/*	--------------------------------------------------------------------------------
	Function		:FreeSampleList
	Purpose			:Clears the whole sample list and the PSX sound memory
	Parameters		: 
	Returns			: 
	Info			: 
*/
void FreeSampleList( )
{
	SAMPLE *cur,*next;

	if(soundList.count)
		utilPrintf("Freeing linked list : SAMPLE : (%d elements)\n",soundList.count);

	FreeSampleBlock( );

	// initialise list for future use
	InitSampleList();
}


								 


/*	--------------------------------------------------------------------------------
	Function		: PlaySample
	Purpose			: plays a sample
	Parameters		: ID, position, radius, volume, pitch
	Returns			: success?
	Info			: Pass in a valid vector to get attenuation, and a radius to override the default
*/
int sfxwaittime = 5;
int PlaySample( SAMPLE *sample, SVECTOR *pos, long radius, short volume, short pitch )
{
 	int vl,vr;
	int vs;

	if (!sample)
	{
		utilPrintf("Sample Not Valid!!!!!!\n");
		return 0;
	}


//Stuff to ensure calls to PlaySample can be the same for PC and PSX
	if (pitch ==-1)
	{
		pitch = 0;
	}
	else
	{
		pitch *= PITCH_STEP;
	}
//end of stuff

	if( pos )
	{
		if(GetSoundVols(pos,&vl,&vr,radius,volume) == -1)
			return -1;
	}
	else
	{
		vr = vl = (volume*globalSoundVol)/MAX_SOUND_VOL;
	}

	if ( !sample->snd )
		return 0;

	vs = VSync(1);
	while((lastSound>=0) && (SpuGetKeyStatus(1<<lastSound)==SPU_ON_ENV_OFF) && (VSync(1)<vs+sfxwaittime));

//	if(sample->flags)
//		utilPrintf("playing looping sample the wrong way!!!!!!!!\n");
	
	sample->handle = lastSound = sfxPlaySample( sample->snd, vl,vr, pitch);
	if(lastSound<0)
		utilPrintf("SOUND NOT WORKED (%i RETURNED)\n",lastSound);
	return lastSound;
}
































 /*	--------------------------------------------------------------------------------
 	Function 	: AddAmbientSound
 	Purpose 	: Create an ambient sound
 	Parameters 	: 
 	Returns 	: 
 	Info 		:
 */
 AMBIENT_SOUND *AddAmbientSound(SAMPLE *sample, SVECTOR *pos, long radius, short vol, short pitch, float freq, float randFreq, ACTOR *follow )
 {
 	AMBIENT_SOUND *ptr;
 	AMBIENT_SOUND *ambientSound;
 
 	if( !sample ) return NULL;
 
 	ambientSound = (AMBIENT_SOUND *)MALLOC0(sizeof(AMBIENT_SOUND));
 	ptr = &ambientSoundList.head;
 
 	if( pos ) SetVectorSS( &ambientSound->pos, pos );
 	if( follow ) ambientSound->follow = follow;
 
 	ambientSound->volume = vol;
 	ambientSound->sample = sample;
 	ambientSound->pitch = pitch;
 	ambientSound->radius = radius;// * SCALE;//does this need scaling????
 
 	ambientSound->freq = freq*60;
 	ambientSound->randFreq = randFreq*60;
 
 	ambientSound->prev = ptr;
 	ambientSound->next = ptr->next;
 	ptr->next->prev = ambientSound;
 	ptr->next = ambientSound;
 	ambientSoundList.numEntries++;

	ambientSound->handle = -1;
 
 	return ambientSound;
 }
 
 
 /*	--------------------------------------------------------------------------------
 	Function 	: UpdateAmbientSounds
 	Purpose 	: Start sound effect if close enough, if timeout, etc
 	Parameters 	: 
 	Returns 	: 
 	Info 		:
 */
 void UpdateAmbientSounds()
 {
 	AMBIENT_SOUND *amb,*amb2;
 	SVECTOR *pos;
 
 	// Update each ambient in turn
 	for( amb = ambientSoundList.head.next; amb != &ambientSoundList.head; amb = amb2 )
 	{
 		amb2 = amb->next;
 
 		// Timeout, so play a new sound
 		if( actFrameCount < amb->counter )
 			continue;
 
 		// If it is attached to a platform, make ambient follow that platform
 		if( amb->follow )
 			SetVectorSS( &amb->pos, &amb->follow->position );
 
 		// If sound doesn't have a source
 //		if( MagnitudeSquared(&amb->pos) )	//PUT BACK IN?!?!
 //			pos = NULL;	//PUT BACK IN?!?!
 //		else	//PUT BACK IN?!?!
 			pos = &amb->pos;
 
		if(amb->sample->snd->pad & 1)
			UpdateLoopingSample(amb);
		else
			PlaySample(amb->sample, &amb->pos, amb->radius, amb->volume, amb->pitch );
 
 		// Freq and randFreq are cunningly pre-multiplied by 60
 		amb->counter = actFrameCount + amb->freq + ((amb->randFreq)?Random(amb->randFreq):0);
 	}
 }
 
 
 
 
 
 void InitAmbientSoundList()
 {
 	ambientSoundList.head.next = ambientSoundList.head.prev = &ambientSoundList.head;
 	ambientSoundList.numEntries = 0;
 }
 
 
 
 
 
 void SubAmbientSound(AMBIENT_SOUND *ambientSound)
 {
 	if(ambientSound->handle != -1)
		sfxStopChannel(ambientSound->handle);
 
 	ambientSound->prev->next = ambientSound->next;
 	ambientSound->next->prev = ambientSound->prev;
 	ambientSoundList.numEntries--;
 
 	FREE( ambientSound );
 }
 
 
 void FreeAmbientSoundList( )
 {
 	AMBIENT_SOUND *cur,*next;

	utilPrintf("Freeing Schmambient sound list\n");
 
 	// check if any elements in list
 	if( !ambientSoundList.numEntries )
 		return;
 
 	// traverse enemy list and free elements
/*
 	for( cur = ambientSoundList.head.next; cur != &ambientSoundList.head; cur = next )
 	{
 		next = cur->next;
 
// 		SubAmbientSound( cur );
	 	if(cur->handle != -1)
			sfxStopChannel(cur->handle);
 	}
*/ 
 	// initialise list for future use
 	InitAmbientSoundList();
}








/*
void MAIN_PrintXAData(const XAFileType * const XATrack)
{
	if (XATrack->status)
	{
		utilPrintf("endpos = %d\n",XATrack->endPos);
		utilPrintf("loop = %d\n",XATrack->loop);
		utilPrintf("startpos = %d\n",XATrack->startPos);
		utilPrintf("status = %d\n",XATrack->status);
		utilPrintf("volume = %d\n",XATrack->vol);
		utilPrintf("cdpos %d, %d, %d, %d\n",XATrack->fileInfo.pos.minute,
										XATrack->fileInfo.pos.second,
										XATrack->fileInfo.pos.sector,
										XATrack->fileInfo.pos.track);
		utilPrintf("xa size = %d\n",XATrack->fileInfo.size);
		utilPrintf("xa name = %s\n",XATrack->fileInfo.name);
	}
	else
	{
		utilPrintf("No XA file currently being played.\n");
	}
}
*/




void bb_InitXA(void)
{
#if GOLDCD==1
	int i=0;
	
	for(i=0; i<2; i++)
		xaFileData[i] = XAgetFileInfo(musicNames[i]);
#endif
}


/*	--------------------------------------------------------------------------------
	Function		: PrepareSongForLevel
	Purpose			: loads and starts playback of song for specified level
	Parameters		: short,short
	Returns			: void
	Info			: 
*/
int songPlaying = 0;
void PrepareSong ( short worldID,  short loop )
{
#if GOLDCD==1

	//default use first xa file
	int chan;
	int xaNum = 0;

	switch ( worldID )
	{
		case AUDIOTRK_GAMEOVER:				worldID = 10; xaNum = 1; break;
		case AUDIOTRK_LEVELCOMPLETE:		worldID = 9; xaNum = 1; break;
		case AUDIOTRK_LEVELCOMPLETELOOP:	worldID = 11; xaNum = 1; break;
	}

	chan = musicList  [ worldID ];


	StopSong();
	XAstart(1);
	songPlaying = 1;
	utilPrintf			( "World ID : %d \n", worldID );
	XAplayChannel		( xaFileData[xaNum], chan, loop, 100 );
	SetMusicVolume();
	//MAIN_PrintXAData	( xaFileData [ 0 ] );

#endif

}



void StopSong( )
{
#if GOLDCD==1
	if(songPlaying)
	{
		XAstop();
		songPlaying = 0;
	}
#endif
}


long PAN_MAX = 4096*10;
int GetSoundVols(SVECTOR *pos,int *vl,int *vr,long radius,unsigned long vol)
{
	fixed att, dist;
	SVECTOR diff;
	long pan = 0;
	SVECTOR m;
	SVECTOR tempSvect;

	
	vol = (vol * globalSoundVol)/MAX_SOUND_VOL;

	if(vol == 0)
		return -1;

	att = (radius)?radius/*/SCALE*/:DEFAULT_SFX_DIST;
	att = att<<12;

	SubVectorSSF(&diff, pos, &currCamSource);
//	SubVectorSSS( &diff, pos, &frog[0]->actor->position );
	// Volume attenuation - check also for radius != 0 and use instead of default
	dist = MagnitudeS(&diff);
	if( dist > (att) )
		return -1;

	vol = (FDiv(att-dist,att)*vol)>>12;

	tempSvect.vx = -pos->vx;
	tempSvect.vy = -pos->vy;
	tempSvect.vz = pos->vz;

	gte_ldv0(&tempSvect);
	gte_rtps();
	gte_stsxy((long *)&m.vx);
	gte_stsz(&m.vz);	//screen z/4 as otz
	m.vz >>= 2;

	dist = MagnitudeS(&m);
	if(dist)
	{
		pan = (m.vx*PAN_MAX)/dist;
		pan *= 4096;
	}
	else
		pan = 0;

	if(pan >= 0)
	{
		*vl = ((PAN_MAX - pan)*vol)/PAN_MAX;
		*vr = vol;
	}
	else
	{
		*vl = vol;
		*vr = ((PAN_MAX + pan)*vol)/PAN_MAX;
	}
}

SAMPLE *FindVoice( unsigned long uid, int pl )
{
	int j;

	for(j = 0;j < voiceCount[pl];j++)
		if(voiceArray[pl][j].uid == uid)
			return &voiceArray[pl][j];

	return NULL;
}

void StopSample(SAMPLE *sample)
{
	if((sample) && (sample->handle != -1))
	{
		sfxStopChannel(sample->handle);
		sample->handle = -1;
	}
}

void PauseAudio( )
{
	// CD Pause: Possibly should check return value... Nah, can't be bothered.
#if GOLDCD==1
	SpuSetCommonCDVolume(0, 0);
	xaFileData[0]->vol = xaFileData[1]->vol = 0;
//	CdControl( CdlPause, NULL, NULL );
#endif

	// Mute sound
	SsSetMute( 1 );
}

void UnPauseAudio( )
{
	// CD Resume
	SetMusicVolume();
	// Unmute sound
	SsSetMute( 0 );
}
