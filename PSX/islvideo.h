/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islvideo.c			Video player

************************************************************************************/
 
#ifndef __ISLVIDEO_H__
#define __ISLVIDEO_H__


/* ---------------------------------------------------------------------------
 * - CONSTANTS
 * ---------------------------------------------------------------------------
 */
 
// Streaming modes.
#define STR_MODE24				1
#define STR_MODE16				0


// Border modes.
#define STR_BORDERS_OFF			0
#define STR_BORDERS_ON			1


// PlayStream return values.
#define	PLAYSTR_END				1
#define	PLAYSTR_USER			2
#define	PLAYSTR_ERROR			3

/* ---------------------------------------------------------------------------
 * - DATA TYPE AND STRUCTURE DECLARATIONS
 * ---------------------------------------------------------------------------
 */

typedef struct _StrDataType
{
		char		*strName;					// Stream file name. 
		short		mode;						// 24-Bit or 16-Bit streaming.
		short		drawBorders;				// != 0 if borders on.
		u_short		scrWidth;					// Screen res width.
		u_short		x;							// X,Y position. 
		u_short		y;
		u_short		width;						// Stream width and height.
		u_short		height;
		u_long		endFrame;					// Last frame No.
		u_long		vlcBufSize;					// Size of each VLC buffer (including header).
		u_short		volume;						// Left and Right ADPCM volume.
} StrDataType;

/* ---------------------------------------------------------------------------
 * - PUBLIC FUNCTION PROTOTYPES
 * ---------------------------------------------------------------------------
 */
 
short videoPlayStream(StrDataType *str, int palMode, short (*keyHandler)(void));

void videoSetAudioChannel(int channel);

/* ------------------------------------------------------------------------ */

#endif // __ISLVIDEO_H__
