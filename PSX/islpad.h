/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islpad.h			Controller pad routines

************************************************************************************/

#ifndef __ISLPAD_H__
#define __ISLPAD_H__


// bit-fields for reading digital buttons
#define PAD_SELECT		(1<<0)
#define PAD_L3			(1<<1)
#define PAD_R3			(1<<2)
#define PAD_START		(1<<3)
#define PAD_UP			(1<<4)
#define PAD_RIGHT		(1<<5)
#define PAD_DOWN		(1<<6)
#define PAD_LEFT		(1<<7)			// Pad digital buttons
#define PAD_L2			(1<<8)
#define PAD_R2			(1<<9)
#define PAD_L1			(1<<10)
#define PAD_R1			(1<<11)
#define PAD_TRIANGLE	(1<<12)
#define PAD_CIRCLE		(1<<13)
#define PAD_CROSS		(1<<14)
#define PAD_SQUARE		(1<<15)

// data structure for reading pad data
typedef struct _PadDataType {
	unsigned char	present[8];					// Controllers present
	unsigned char	analog[8];					// Analogue-type controllers present
	unsigned char	numPads[2];					// Number of pads in port
	unsigned short	digital[8];					// Current d-pad reading
	unsigned short	debounce[8];				// d-pad edge reading
	unsigned short	analogX[8];					// Current left analog X reading
	unsigned short	analogY[8];					// Current left analog Y reading
	unsigned short	analogX2[8];				// Current right analog X reading
	unsigned short	analogY2[8];				// Current right analog Y reading
	short			analogXS[8];				// Current left analog X reading (smoothed)
	short			analogYS[8];				// Current left analog Y reading (smoothed)
	short			analogX2S[8];				// Current right analog X reading (smoothed)
	short			analogY2S[8];				// Current right analog Y reading (smoothed)
	unsigned char	motor[8][2];				// Dual shock motor values
	int				shock[8];					// 2nd shock dest values
	int				currShock[8];				// 2nd shock current values
	int				shockDiv[8];				// 2nd shock inertia divisor
	int				buzzTime[8];				// Countdown time for 1st shock
	int				shockEnable;
	int				analogAccel;				// acceleration for analog emulation
} PadDataType;

// extern declaration of pad data structure
extern PadDataType		padData;


// Dual-shock macros

// Stop dual shock vibration
#define SHOCK_STOP(p)		if (padData.shockEnable) { padData.motor[(p)][0] = padData.motor[(p)][1] = 0; \
							padData.shock[(p)] = 0; \
							padData.buzzTime[(p)] = 0; }

// Start 1st shock motor
#define SHOCK_BUZZ(p)		if (padData.shockEnable) { padData.buzzTime[(p)] = INT_LARGEST; }

// Stop 1st shock motor
#define SHOCK_NOBUZZ(p)		if (padData.shockEnable) { padData.buzzTime[(p)] = 0; }

// Start 1st shock motor for given time
#define SHOCK_BUZZTIME(p,t)	if (padData.shockEnable) { padData.buzzTime[(p)] = (t); }

// Set 2nd shock motor value
#define SHOCK_SET(p,v)		if (padData.shockEnable) { padData.motor[(p)][1] = (v); \
							padData.currShock[(p)] = padData.shock[(p)] = (v)<<16; }

// Slide 2nd shock motor to value
#define SHOCK_SLIDE(p,v,d)	if (padData.shockEnable) { padData.shock[(p)] = (v)<<16; \
							padData.shockDiv[(p)] = (d); }


/**************************************************************************
	FUNCTION:	padInitialise()
	PURPOSE:	Initialise pads
	PARAMETERS:	multi-tap support on/off
	RETURNS:	
**************************************************************************/

void padInitialise(unsigned char multiTap);


/**************************************************************************
	FUNCTION:	padHandleInput()
	PURPOSE:	Handle pad reading/connection etc.
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void padHandleInput();


/**************************************************************************
	FUNCTION:	padHandleShock()
	PURPOSE:	Handle pad dual shock stuff
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void padHandleShock();


#endif
