/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islvideo.c			Video player

************************************************************************************/


#include <sys/types.h>
#include <libsn.h>
#include <r3000.h>
#include <asm.h>
#include <kernel.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libcd.h>
#include <libspu.h>
#include <libpress.h>
#include <libsnd.h>
#include <islmem.h>
#include <islpad.h>

//#include <global.h>
#include "islvideo.h"

/* ---------------------------------------------------------------------------
 * - CONSTANTS
 * ---------------------------------------------------------------------------
 */

// Define for timing and testing functions.
//#define TESTING

// Define for extra debug info.
//#define _DEBUG

/*

#ifdef NTSC

#define	FRAME_Y			240
#define SCREEN_X		0
#define SCREEN_Y		0	

#else

#define	FRAME_Y			256
#define SCREEN_X		0
#define SCREEN_Y		18	

#endif // NTSC
*/

#if GOLDCD==0
	#define FIND_VLCBUF 1
#endif


static int FRAME_Y;
static int SCREEN_X;
static int SCREEN_Y;

static unsigned short	*vlc_table_ptr;
static u_long			*ringBuf;

// In 4-audio-track STR's, the audio tracks are (0,1,2,3)
// In Normal STR's, the audio track is (1)

char video_audio_channel = 1;
char video_is_playing = 0;


// Maximum number of retries.
#define MAX_RETRY				5

// Form1 sector size in bytes.
#define FORM1_SIZE				2048 

// Ring buffer size in sectors.
#define RING_SIZE				64

// Maximum possible VLC buffer size (bytes) = 128K + (Header) = 129K.
#define MAX_VLCBUF_SIZE			132096

// Slice width in pixels.
#define SLICE_WIDTH				16

// Words/Pixel for 24-bit and 16-bit modes.
#define PPW24					3 / 2
#define PPW16					1    

// DCT modes for 24-bit and 16-bit (STP = 1) streaming.
#define DCT_MODE24				3
#define DCT_MODE16				2

#define bound(val, n)			((((val) - 1) / (n) + 1) * (n))
#define bound16PPW(val, ppw)	(bound((val), 16 * ppw))
#define bound16(val)			(bound((val), 16))

// Speed of volume fade.
#define FADE_VOL_SPD			8

// RunLevel header size in bytes.
#define RL_HEADER_SIZE			1024

/* ---------------------------------------------------------------------------
 * - DATA TYPE AND STRUCTURE DECLARATIONS
 * ---------------------------------------------------------------------------
 */

typedef struct {
	volatile short		is24Bit;						// = 1 when 24-bit streaming.
	volatile u_short	scrWidth;						// Screen width resolution.
	volatile u_short	x;								// Str X, Y.
	volatile u_short	y;
	volatile u_short	width;							// Str width and height.
	volatile u_short	height;
	volatile u_long		endFrame;                  		// End frame. 
	volatile short		volume;							// ADPCM Volume.
	volatile u_long		frameCount;						// Frame count.
	RECT				rect[2];
	RECT				slice;							// Slice decoded by DecDCTout().
	volatile short		rewindSwitch;					// = 1 when end of stream.
	volatile short		frameDone;						// = 1 when decoding and drawing done.
	volatile short		rectId;
	volatile short		vlcId;							// VLC buffer id.
	volatile short		imageId;						// Image buffer id.
	u_long				*vlcBuffer[2];					// VLC run-level buffers.
	u_long				*imageBuffer[2];				// Strip image buffers.
	DISPENV				disp[2];						// Display envs.
} DecEnv;

/* ---------------------------------------------------------------------------
 * - GLOBAL DEFINITIONS 
 * ---------------------------------------------------------------------------
 */
 
#ifdef TESTING
static int		t1, t2;							// Timers.

static DRAWENV	draw[2];						// DrawEnvs for FntPrint.
#endif


#if defined(TESTING)||defined(FIND_VLCBUF)
static short	firstFrame = 1,					// Frame skipping check.
				numSkipped = 0,
				prevFrameNum = 0;			
#endif


#ifdef FIND_VLCBUF
static u_short	maxRunLevel = 0;
#endif

static DecEnv	dec;							// Streaming Env.

/* ---------------------------------------------------------------------------
 * - PRIVATE FUNCTION PROTOTYPES
 * ---------------------------------------------------------------------------
 */

static void StrDecDCToutCB(void);
static void StrInit(StrDataType *str);
static void StrEnd(void);
static void StrClearVRAM(void);
static short StrKickCD(CdlLOC *loc);
static u_long *StrGetNextFrame(void);
static void StrVLCFrame(u_long *frame);
static void	StrDrawDecodeSync(void);
static short StrFadeOutVol(short *fadeVol);
static long GetDCToutSize(void);
static u_short ScalePPW(u_short n);
static u_short GetDCT_MODE(void);

#ifdef FIND_VLCBUF
static void PrintVLCBufSize(void); 
#endif


/* ---------------------------------------------------------------------------
 * - FUNCTION DEFINITIONS
 * ---------------------------------------------------------------------------
 */

void videoSetAudioChannel(int channel)
{
	u_char param[4];
	CdlFILTER theFilter;

	video_audio_channel = channel;

	if(video_is_playing)
	{
		theFilter.file=1;
		theFilter.chan=video_audio_channel;
		CdControlF(CdlSetfilter, (u_char *)&theFilter);

		// Aha! Right, this allows us to select audio channel without making the video turn off
		param[0] = CdlModeSpeed|CdlModeRT|CdlModeSF;
		CdControlB(CdlSetmode, param, 0);
	}
	utilPrintf("vid channel set to %d\n",channel);
}


short videoPlayStream(StrDataType *str, int palMode, short (*keyHandler)(void))
{


/* - Type:	PUBLIC
 * -
 * - Param:	str = (In) Stream info. palMode = pal screen res (vertical)
 * -		keyHandler = (In) Read controller function (Returns non-zero if exit).
 * -
 * - Ret:	PLAYSTR_ERROR = Error.
 * -		PLAYSTR_END = End of stream exit.
 * -		PLAYSTR_USER = User exit.
 * -
 * - Usage:	Play a stream.
 */


	CdlFILE		fp;

	u_long		*frame;

	short		frameRetry = 0;
	short		strQuit = 0;
	short		fadeVol = 0;

	if(palMode)
	{
		FRAME_Y = 256;
		SCREEN_X = 0;
		SCREEN_Y = 18;
	}
	else
	{
		FRAME_Y = 240;
		SCREEN_X = 0;
		SCREEN_Y = 0;
	}

	vlc_table_ptr = MALLOC(sizeof(DECDCTTAB));
    DecDCTvlcBuild(&vlc_table_ptr[0]);			// expand compressed vlc parameter data


	if (!CdSearchFile(&fp, str->strName))
	{
#ifdef _DEBUG
		utilPrintf("ERROR: MOVIE -- Could not find stream file '%s'.\n", str->strName);
#endif
		FREE(vlc_table_ptr);
		return PLAYSTR_ERROR;
	}


	StrInit(str);

	if (!StrKickCD(&fp.pos))
	{
#ifdef _DEBUG
		utilPrintf("ERROR: MOVIE -- Couldn't start the CD!!\n");
#endif
		StrEnd();
		
		FREE(ringBuf);
		FREE(dec.imageBuffer[1]);
		FREE(dec.imageBuffer[0]);
		FREE(dec.vlcBuffer[1]);
		FREE(dec.vlcBuffer[0]);
		FREE(vlc_table_ptr);
		return PLAYSTR_ERROR;
	}


	while (!(frame = StrGetNextFrame()))	// If we can't get frame exit!
	{		

		if (++frameRetry == MAX_RETRY) 
		{
#ifdef _DEBUG
			utilPrintf("ERROR: MOVIE -- Couldn't get first frame!!\n");
#endif
			StrEnd();
			FREE(ringBuf);
			FREE(dec.imageBuffer[1]);
			FREE(dec.imageBuffer[0]);
			FREE(dec.vlcBuffer[1]);
			FREE(dec.vlcBuffer[0]);
			FREE(vlc_table_ptr);
			return PLAYSTR_ERROR;
		}
	}
	

	frameRetry = 0;
	StrVLCFrame(frame);

	videoSetAudioChannel(video_audio_channel);
	while (!dec.rewindSwitch && !strQuit)
	{


		DecDCTin(dec.vlcBuffer[dec.vlcId], GetDCT_MODE());
		DecDCTout(dec.imageBuffer[dec.imageId], GetDCToutSize());


		if (!(frame = StrGetNextFrame()))
		{

			if (++frameRetry == MAX_RETRY)
				strQuit = PLAYSTR_ERROR;
			
		}
		else
		{
			frameRetry = 0;
		}


#ifdef TESTING
		t2 = VSync(1);					// Time VLC.
		StrVLCFrame(frame);
		t2 = VSync(1) - t2;
#else 
		StrVLCFrame(frame);
#endif


		// Check for exit. 
		padHandleInput();
		if (!fadeVol && (*keyHandler)())
			fadeVol = dec.volume;


		// Fade the volume down to zero. Then trigger exit.
		if (fadeVol)
			strQuit = StrFadeOutVol(&fadeVol);


		StrDrawDecodeSync();


#ifdef TESTING
		DrawSync(0);
		FntPrint("\nVLC Time = %d\n", t2);
		FntPrint("Total Time = %d\n", VSync(1) - t1);	// Timing.
		FntFlush(-1);
#endif


		DrawSync(0);
		VSync(0);


#ifdef TESTING
		t1 = VSync(1);									// Start timer.
#endif


		PutDispEnv(&dec.disp[dec.rectId]);


#ifdef TESTING
		PutDrawEnv(&draw[dec.rectId]);					// Put DrawEnv for FntPrint.
#endif
	}


	StrEnd();


	if (dec.rewindSwitch)								// Check for end of stream exit.
		strQuit =  PLAYSTR_END;


#ifdef _DEBUG	
	if (strQuit == PLAYSTR_ERROR) 
		utilPrintf("ERROR: MOVIE -- Couldn't get frame from ring buffer!!\n");
#endif


#ifdef FIND_VLCBUF
	// If stream played completely print the maximum VLC buffer size.
	if (strQuit == PLAYSTR_END)
		PrintVLCBufSize(); 
#endif

	FREE(ringBuf);
	FREE(dec.imageBuffer[1]);
	FREE(dec.imageBuffer[0]);
	FREE(dec.vlcBuffer[1]);
	FREE(dec.vlcBuffer[0]);
	FREE(vlc_table_ptr);
	return strQuit;
}

/* ------------------------------------------------------------------------ */

static void StrDecDCToutCB(void)
{


/* - Type:	CALLBACK
 * -
 * - Usage:	Receives decoded image data from the MDEC and loads it to VRAM.
 */


	// In 24bit mode call StCdInterrupt() manually.
	if (dec.is24Bit)
	{
		extern int StCdIntrFlag;

		if(StCdIntrFlag)
		{
			StCdInterrupt();
			StCdIntrFlag = 0;
		}
	}

	LoadImage(&dec.slice, dec.imageBuffer[dec.imageId]);
	dec.imageId ^= 1;
	dec.slice.x += dec.slice.w;

	if (dec.slice.x < (dec.rect[dec.rectId].x + dec.rect[dec.rectId].w))
	{

		DecDCTout(dec.imageBuffer[dec.imageId], GetDCToutSize());

	}
	else
	{

		dec.frameDone = 1;
	}
}

/* ------------------------------------------------------------------------ */

static void StrInit(StrDataType *str) {


/* - Type:	PRIVATE
 * -
 * - Param:	str = (In) = Stream Info.
 * -
 * - Usage: Init internal vars, the MDEC and allocate the buffers.
 */


	short	x, y, w, h;
	short	scrWidth;
	u_long	vlcBufSize,
			imageBufSize;	

	video_is_playing = 1;

	x = str->x;
	y = str->y;	
	w = str->width;
	h = str->height;
	scrWidth = str->scrWidth;


#ifdef TESTING
	// Init frame skipping checks.
	firstFrame = 1;
	numSkipped = 0;
	prevFrameNum = 0;			

	// Init DrawEnvs for FntPrint.
	SetDefDrawEnv(&draw[0], 0,   0,       scrWidth, FRAME_Y);
	SetDefDrawEnv(&draw[1], 0,   FRAME_Y, scrWidth, FRAME_Y);
	setRGB0(&draw[0], 0, 0, 0);
	setRGB0(&draw[1], 0, 0, 0);
	draw[0].isbg = draw[1].isbg = 1;
#endif


#ifdef FIND_VLCBUF
	maxRunLevel = 0;
#endif


	dec.is24Bit = str->mode; 
	dec.scrWidth = scrWidth;
	dec.x = x;
	dec.y = y;
	dec.width = w;
	dec.height = h;
	dec.endFrame = str->endFrame;
	dec.volume = str->volume;
	dec.frameCount = 0;
	dec.rewindSwitch = 0;
	dec.frameDone = 0;
	dec.rectId = 0;
	dec.vlcId = 0;
	dec.imageId = 0;
	

	// Init the display environments.
	SetDefDispEnv(&dec.disp[0], 0,   FRAME_Y, scrWidth, FRAME_Y);
	SetDefDispEnv(&dec.disp[1], 0,   0,       scrWidth, FRAME_Y);
	setRECT(&dec.disp[0].screen, SCREEN_X, SCREEN_Y, 0, FRAME_Y);	
	setRECT(&dec.disp[1].screen, SCREEN_X, SCREEN_Y, 0, FRAME_Y);	
	dec.disp[0].isrgb24 = dec.disp[1].isrgb24 = (str->mode == STR_MODE24);


	if (str->drawBorders || dec.is24Bit)			// Clear VRAM.
		StrClearVRAM();


	// Set correct display mode before we begin drawing.
	VSync(0);
	PutDispEnv(&dec.disp[dec.rectId]);


	setRECT(&dec.rect[0], ScalePPW(x), y, ScalePPW(w), h);  
	setRECT(&dec.rect[1], ScalePPW(x), y + FRAME_Y, ScalePPW(w), h);
	dec.slice = dec.rect[dec.rectId];
	dec.slice.w = ScalePPW(SLICE_WIDTH);

	
	// Alloc VLC and image buffers (amount allocated depends on frame size,
	// streaming mode and maximum runlevel size).
//#ifdef FIND_VLCBUF
//	vlcBufSize = MAX_VLCBUF_SIZE;
//#else
	if (str->vlcBufSize)
		vlcBufSize = str->vlcBufSize;
	else
		vlcBufSize = MAX_VLCBUF_SIZE;
//#endif // FIND_VLCBUF


	dec.vlcBuffer[0] = (u_long *) MALLOC(vlcBufSize);
	dec.vlcBuffer[1] = (u_long *) MALLOC(vlcBufSize);


	if (dec.is24Bit)
		imageBufSize = SLICE_WIDTH * 3 * h;
	else
		imageBufSize = SLICE_WIDTH * 2 * h;

	dec.imageBuffer[0] = (u_long *) MALLOC(imageBufSize);
	dec.imageBuffer[1] = (u_long *) MALLOC(imageBufSize); 


	// Init the MDEC.
	DecDCTReset(0);
	DecDCTvlcSize(0);
	DecDCToutCallback((void (*)()) StrDecDCToutCB);
	ringBuf = (u_long *) MALLOC(RING_SIZE * FORM1_SIZE);
	StSetRing(ringBuf, RING_SIZE);


	// Start no matter the frame number!
	StSetStream(dec.is24Bit, 0, 0xffffffff, 0, 0);


	SpuSetCommonCDVolume((dec.volume*0x7fff)/127,(dec.volume*0x7fff)/127);
}

/* ------------------------------------------------------------------------ */

static void StrEnd(void) {


/* - Type:	PRIVATE
 * -
 * - Usage: Clean up.
 */

	video_is_playing = 0;
	StUnSetRing();
	CdControlB(CdlPause, 0, 0);
	DecDCToutCallback(0);            

	if (dec.is24Bit)			// Clear VRAM to remove any 24bit graphics.
		StrClearVRAM();

#ifdef _DEBUG
	utilPrintf("FrameCount %d\n", dec.frameCount);
#endif
}

/* ------------------------------------------------------------------------ */

static void StrClearVRAM(void) {


/* - Type:	PRIVATE
 * -
 * - Usage: Useful before and after streaming in 24bit mode	to ensure
 * - 		24bit data is not displayed in 16bit mode (and vice versa). 
 */


	RECT	clrRect;


	setRECT(&clrRect, 0, 0, ScalePPW(dec.scrWidth), FRAME_Y);
	ClearImage(&clrRect, 0, 0, 0);
	clrRect.y = FRAME_Y;
	ClearImage(&clrRect, 0, 0, 0);
}

/* ------------------------------------------------------------------------ */

static short StrKickCD(CdlLOC *loc) {


/* - Type:	PRIVATE
 * -
 * - Param:	loc = (In) location. 
 * -
 * - Ret:	0 on error, 1 otherwise.	
 * -
 * - Usage: Start the CD. 
 */

	
	short	seekRetry = 0,
			readRetry = 0;	


	do {

		while (!CdControl(CdlSeekL, (u_char *)loc, 0)) {
			
			if (++seekRetry == MAX_RETRY)
				return 0;
		}


		seekRetry = 0;

		if (readRetry++ == MAX_RETRY)
			return 0;
				

	} while (!CdRead2(CdlModeStream|CdlModeSpeed|CdlModeRT));


	return 1;
}

/* ------------------------------------------------------------------------ */

static u_long *StrGetNextFrame(void) {


/* - Type:	PRIVATE
 * -
 * - Ret:	Address of next frame or NULL on error.
 * -
 * - Usage:	Gets next frame from the ring buffer. Also checks for the
 * -		end of the stream. If the end has been reached rewindSwitch
 * -		is set. If frame is not obtained from the ring timeout and
 * -		return NULL.
 */


	long		timer = WAIT_TIME;
	u_long		*addr;                      
	StHEADER	*sector;                         

    
    // Get the next frame from the ring buffer.
	while (StGetNext(&addr, (u_long**) &sector)) {
			
			if (--timer == 0)
				return NULL;
	}


	dec.frameCount++;				// Update internal frame count.


#if defined(TESTING)||defined(FIND_VLCBUF)

	// Check for frame skipping.
	if (firstFrame) {
		prevFrameNum = sector->frameCount;
		firstFrame = 0;

	} else if ((sector->frameCount - prevFrameNum) != 1) {
		numSkipped++;	
	}
		
	prevFrameNum = sector->frameCount;
	FntPrint("Skipped =  %d\n", numSkipped);
#endif


#ifdef _DEBUG
	if (dec.frameCount == 1)
		utilPrintf("MOVIE: w = %d, h = %d, sectors = %d\n", 
				sector->width, sector->height, sector->nSectors);

	FntPrint("Frame No. = %d\n", dec.frameCount);
#endif

			
	// End of stream checks.
	if (sector->frameCount > dec.endFrame) {
	
#ifdef _DEBUG	
		utilPrintf("ERROR: MOVIE -- Found sector frame count over > end frame (%d > %d)\n",
				sector->frameCount, dec.endFrame);
#endif
		dec.rewindSwitch = 1;
		return addr;
	}
	

	if (sector->frameCount < dec.frameCount) {
	
#ifdef _DEBUG	
		utilPrintf("ERROR: MOVIE -- Found sector frame count < internal frame count. (%d < %d)\n",
			 sector->frameCount, dec.endFrame);
#endif
		dec.rewindSwitch = 1;
		return addr;
	}


	if (sector->width != dec.width || sector->height != dec.height) {
	
#ifdef _DEBUG	
		utilPrintf("ERROR: MOVIE -- Found bad sized frame (%d x %d) is not (%d x %d).\n",
				sector->width, sector->height, dec.width, dec.height);
#endif
		dec.rewindSwitch = 1;
		return addr;
	}

	
    return addr;                          
}

/* ------------------------------------------------------------------------ */

static void StrVLCFrame(u_long *frame) {


/* - Type:	PRIVATE
 * -
 * - Param:	frame = (In) Address of frame BS.
 * -
 * - Usage:	VLC decodes the frame's BS. Unlock frame from the ring buffer
 * -		once decoded.
 */


	// If no frame to decode return.
	if (frame == NULL) {
#ifdef _DEBUG
		utilPrintf("ERROR: MOVIE -- No frame to VLC decode!!\n");
#endif
		return;
	}


#ifdef TESTING
		// Print BS Encoded Format (i.e. Version 2 or 3). 
		FntPrint("BS Ver = %d\n", *((u_short *) frame + 3) );
#endif


#ifdef FIND_VLCBUF
	if (DecDCTBufSize(frame) > maxRunLevel)
		maxRunLevel = DecDCTBufSize(frame);
#endif


	dec.vlcId ^= 1;
	//DecDCTvlc(frame, dec.vlcBuffer[dec.vlcId]);
	DecDCTvlc2(frame, dec.vlcBuffer[dec.vlcId],&vlc_table_ptr[0]);
	StFreeRing(frame);
}

/* ------------------------------------------------------------------------ */

static void	StrDrawDecodeSync(void) {


/* - Type:	PRIVATE
 * -
 * - Usage:	Wait for current frame's decoding and drawing to be finished.
 */


	long	timer = WAIT_TIME;


	while (!dec.frameDone) {

		if (--timer == 0) {
#ifdef _DEBUG
			utilPrintf("ERROR: MOVIE -- MDEC decode timeout!!\n");
#endif
			break;
		}
	}

	
	dec.frameDone = 0;

	dec.rectId ^= 1;
	dec.slice.x = dec.rect[dec.rectId].x;
	dec.slice.y = dec.rect[dec.rectId].y;
}

/* ------------------------------------------------------------------------ */

static short StrFadeOutVol(short *fadeVol) {


/* - Type:	PRIVATE
 * -
 * - Param:	fadeVol = (In/Out) Current volume.
 * -
 * - Ret:	1 = Volume completely faded (therefore finish playing the stream).
 * - 		0 = Otherwise.
 * -		
 * -
 * - Usage: Fade out the volume.
 */


	if ((*fadeVol -= FADE_VOL_SPD) < 0)
		*fadeVol = 0;


	SpuSetCommonCDVolume((*fadeVol*0x7fff)/127,(*fadeVol*0x7fff)/127);


	if (!*fadeVol)
		return PLAYSTR_USER;			// Flag user exit.
	else
		return 0;
}

/* ------------------------------------------------------------------------ */

static long GetDCToutSize(void) {


/* - Type:	PRIVATE
 * -
 * - Ret:	DecDCTout size in longs for current playback mode.
 */


	u_short	h = dec.height;

	
	if (dec.is24Bit)
		return (((SLICE_WIDTH * PPW24) * bound16(h)) >> 1);
	else
		return (((SLICE_WIDTH * PPW16) * bound16(h)) >> 1);
}

/* ------------------------------------------------------------------------ */

static u_short ScalePPW(u_short n) {


/* - Type:	PRIVATE
 * -
 * - Param:	n = (In) A number.
 * -
 * - Ret:	n scaled by the correct PPW (words/pixel). This depends
 * -		on the current playback mode.
 */


	if (dec.is24Bit)
		return (n * PPW24);
	else
		return (n * PPW16);
}

/* ------------------------------------------------------------------------ */

static u_short GetDCT_MODE(void) {


/* - Type:	PRIVATE
 * -
 * - Ret:	The current DCT mode.	
 */


	return ((dec.is24Bit) ? DCT_MODE24 : DCT_MODE16);
}

/* ------------------------------------------------------------------------ */

#ifdef FIND_VLCBUF

static void PrintVLCBufSize(void) {


/* - Type:	PRIVATE
 * -
 * - Usage: Prints the maximum VLC buffer size to play a stream.	
 */


	utilPrintf("\nMaximum VLC Buffer (RunLevel) Size\n");
	utilPrintf("----------------------------------\n");
	utilPrintf("Max RunLevel size = %d bytes.\n", maxRunLevel << 2);
	utilPrintf("Max size of each VLC buffer = %d bytes.\n\n", (maxRunLevel << 2) + RL_HEADER_SIZE);

	// If frames were skipped warn of possible incorrect result! 
	if (numSkipped) {
		utilPrintf("Warning:\n");
		utilPrintf("\tSkipped %d frames during playback!\n", numSkipped);
		utilPrintf("\tTherefore the maximum VLC buffer size may be incorrect!\n\n"); 
	}
}

#endif // FIND_VLCBUF

/* ------------------------------------------------------------------------ */
