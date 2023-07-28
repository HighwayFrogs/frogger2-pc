#ifndef __ISLPAD_H__
#define __ISLPAD_H__

#ifdef __cplusplus
extern "C" {
#endif

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

void padInitialise(unsigned char multiTap);
void padHandler();


#ifdef __cplusplus
}
#endif
#endif