/************************************************************************************
	ACTION MAN PS	(c) 1998-9 ISL

	timer.c:		Debug timing functions

************************************************************************************/

#include "timer.h"
#include <isltex.h>
#include <islfont.h>
#include "shell.h"
#include "sonylibs.h"
#include "main.h"
#include "layout.h"

/////////////////////////////////////////////////////////////////////
char	timerActive = 0;
int 	globalTimer[TIMER_NUMTIMERS];
int 	startTimer[TIMER_NUMTIMERS];
int 	prevTimer[TIMER_NUMTIMERS];
char	*timerName[TIMER_NUMTIMERS] = {TIMER_NAMES};
/////////////////////////////////////////////////////////////////////

static void dispGraph(short x,short y, short w, short h, UBYTE r, UBYTE g, UBYTE b)
{
//	POLY_F4 	*si = (POLY_F4 *) GsOUT_PACKET_P; 
	POLY_F4 	*si; 

// 	if (TOOMANYPOLYS(5))
// 		return;

	BEGINPRIM(si, POLY_F4);
	setPolyF4(si);

	si->x0 = x;
	si->y0 = y;
	si->x1 = x+w;
	si->y1 = y;
	si->x2 = x;
	si->y2 = y+h;
	si->x3 = x+w;
	si->y3 = y+h;
	si->r0 = r;
	si->g0 = g;
	si->b0 = b;
	si->code = GPU_COM_F4;

	ENDPRIM(si, 0, POLY_F4);

//	PUTPACKETINTABLE(si, (GsOTA*)(PolyList->org), POLYF4_LEN);
//	si++;
//	(POLY_F4*)GsOUT_PACKET_P = si;
}


/**************************************************************************
	FUNCTION:	timerDisplay()
	PURPOSE:	Display timer readings if enabled
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void timerDisplay()
{
	int		loop, subtotal = 0;
	char	str[20];

	int x=-236;
	int y=-100;
	int h;

	TIMER_START(TIMER_TIMERS);
	if (timerActive)
	{
//		h=PSFONT_GetSmallStringHeight(font,"");
		h=10;

		// Timers
		for(loop=1; loop<TIMER_NUMTIMERS; loop++)
		{
//			fontPrint(fontSmall, x,y, timerName[loop], (loop&1)?(90):(130),(loop&1)?(90):(130),(loop&1)?(90):(130));
			fontPrintScaled(fontSmall, x,y, timerName[loop], 90,90,90,4096);
			sprintf(str, "%d", prevTimer[loop]);
//			fontPrint(fontSmall, x+300,y, str, (loop&1)?(90):(130),(loop&1)?(90):(130),(loop&1)?(90):(130));
			fontPrintScaled(fontSmall, x+300,y, str, 90,90,255,4096);
			dispGraph(x+180,y+4, (100*prevTimer[loop])/(1+prevTimer[TIMER_TOTAL]),2, (loop&1)?(100):(200),(loop&1)?(100):(200),0);
			dispGraph(x+178,y+2, 104,6, 0,0,0);
			subtotal += prevTimer[loop];

			y+=h;
		}


		// Sub total
		fontPrintScaled(fontSmall, x,y, "SUB-TOTAL", 90,90,255,4096);
		sprintf(str, "%d%%", (100*subtotal)/(1+prevTimer[TIMER_TOTAL]));
		fontPrintScaled(fontSmall, x+300,y, str, 90,90,255,4096);

		// Total
		fontPrintScaled(fontSmall, x,95, timerName[TIMER_TOTAL], 90,90,255,4096);
//		sprintf(str, "%d [%d]", prevTimer[TIMER_TOTAL],framerate);
		sprintf(str, "%d [%d]", prevTimer[TIMER_TOTAL], gameSpeed);
		fontPrintScaled(fontSmall, 100,95, str, 90,90,255,4096);
//		subtotal = (100*prevTimer[TIMER_TOTAL])/(((PALMODE)?(310):(260))*TITAN_DEFAULT_FRAME_LOCK);
#define IDEAL_FRAME_RATE 2
		subtotal = (100*prevTimer[TIMER_TOTAL])/(((PALMODE)?(310):(260))*IDEAL_FRAME_RATE);


		if (subtotal>100)
			dispGraph(-50,101, subtotal,8, 255,0,0);
		else
			dispGraph(-50,101, subtotal,8, 128,0,0);
		dispGraph(-52,100, 104,10, 0,0,0);

	}
	TIMER_STOP(TIMER_TIMERS);

//	if (padData.debounce[4] & PAD_R1)
//		timerActive = !timerActive;
}
