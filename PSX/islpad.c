/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islpad.c			Controller pad routines

************************************************************************************/

#include <stddef.h>
#include <libetc.h>
#include <libpad.h>
#include <stdio.h>
#include "islpad.h"

enum {									// Supported pad types
	PADTYPE_NONE,
	PADTYPE_DIGITAL,
	PADTYPE_ANALOG,
	PADTYPE_DUALSHOCK,
};

typedef struct _PadPacket {
     short			digital;
     unsigned char	rightX;
     unsigned char	rightY;
     unsigned char	leftX;
     unsigned char	leftY;
} PadPacket;

typedef struct _PadDataType2 {
	unsigned char	buffer[2][34];				// HW packet buffers
	int				state[8];					// Port HW state
	unsigned short	digitalPrev[8];				// Previous d-pad reading
	int				newShock[8];
} PadDataType2;

static PadDataType2		padData2;
static unsigned char	MULTITAP_SUPPORT;


PadDataType				padData;

static unsigned char	HWalign[6]={0,1,0xFF,0xFF,0xFF,0xFF};
unsigned char			analog[8];
unsigned short			pad[8], debounce[8];
unsigned short			leftx[8], lefty[8];
unsigned short			rightx[8], righty[8];
unsigned short			bothdebounces, bothpads;



// (deadmult is 4096/(128-deadzone)
#define DEADZONE 50
#define DEADMULT 53
#define ABS(x) (x > 0) ? (x) : (-x)

static void NormaliseJoy2(int x,int y,short *nx, short *ny)
{
	int ax,ay;
	x = -(127 - x);
	y = -(127 - y);
	ax = ABS(x);
	ay = ABS(y);

	if(ax < DEADZONE && ay < DEADZONE)
	{
		*nx = 0;
		*ny = 0;
	}
	else if (ax <= ay)
	{
//		*ny = (ay - DEADZONE) * 4096 / (128 - DEADZONE);
		*ny = (ay - DEADZONE) * DEADMULT;
		*nx = (ax * (int)(*ny)) >>7;
	}
	else
	{
//		*nx = (ax - DEADZONE) * 4096 / (128 - DEADZONE);
		*nx = (ax - DEADZONE) * DEADMULT;
		*ny = (ay * (int)(*nx)) >>7;
	}
	if(*nx >4096) *nx = 4096;
	if(*ny >4096) *ny = 4096;

	if(x < 0)
		*nx = -*nx;
	if(y < 0)
		*ny = -*ny;
}



/**************************************************************************
	FUNCTION:	padInitialise()
	PURPOSE:	Initialise pads
	PARAMETERS:	multi-tap on/off
	RETURNS:	
**************************************************************************/

void padInitialise(unsigned char multiTap)
{
	int		loop;

	MULTITAP_SUPPORT = multiTap;

	if(MULTITAP_SUPPORT)
		PadInitMtap(padData2.buffer[0],padData2.buffer[1]);
	else
		PadInitDirect(padData2.buffer[0],padData2.buffer[1]);

	VSync(5);
	PadStartCom();
	padData.numPads[0] = 1;
	padData.numPads[1] = 1;
	padData.analogAccel = 4;	// default acceleration for analog emulation
	VSync(5);
	for(loop=0; loop<8; loop++)
	{
		padData.digital[loop] = 0;
		padData.debounce[loop] = 0;
		padData.analogX[loop] = 128;
		padData.analogY[loop] = 128;
		padData.analogX2[loop] = 128;
		padData.analogY2[loop] = 128;
		padData.analogXS[loop] = 0;
		padData.analogYS[loop] = 0;
		padData.analogX2S[loop] = 0;
		padData.analogY2S[loop] = 0;

		padData2.digitalPrev[loop] = 0;
		padData2.newShock[loop] = 10;
	}
}


static inline unsigned char padGetTypeFromPacket(unsigned char id)
{
	switch(id)
	{
	case 4:
		return PADTYPE_DIGITAL;
	case 7:
		return PADTYPE_ANALOG;
	}
	return PADTYPE_NONE;
}


static void padHandlePort(int port)
{
	int			padLp, HWport, padNo, state, currPad;
	PadPacket	*packet;
	unsigned char		id;

	if (*padData2.buffer[port])
	{
		for(padLp=0; padLp<4; padLp++)
		{
			padNo = padLp|(port<<2);
			padData.present[padNo] = PADTYPE_NONE;
			padData2.state[padNo] = PadStateDiscon;
			padData.digital[padNo] = 0;
		}
		return;
	}

	if(MULTITAP_SUPPORT)
	{
		if (PadChkMtap(port<<4)==1)					// SORT THIS OUT - COULD BE GREEDY
		{
			padData.numPads[port] = 4;
		}
		else
		{
			padData.numPads[port] = 1;
		}
	}
	else
	{
		padData.numPads[port] = 1;
	}


	for(padLp=0; padLp<padData.numPads[port]; padLp++)
	{
		padNo = padLp|(port<<2);
		HWport = padLp|(port<<4);
		state = PadGetState(HWport);
		switch(state)										// Handle (dis)connection states
		{
		case PadStateDiscon:
			if(padData2.newShock[padNo] < 1)
			{
#ifdef _DEBUG
				//printf("  Controller disconnected\n");
#endif
  				padData.present[padNo] = PADTYPE_NONE;
				padData2.state[padNo] = PadStateDiscon;
			}
			continue;
		case PadStateFindPad:
			if(padData2.newShock[padNo] < 1)
			{
#ifdef _DEBUG
				//printf("  Find controller connection (checking)\n");
#endif
				padData2.state[padNo] = PadStateFindPad;
			}
			continue;
		case PadStateFindCTP1:
			if (padData2.state[padNo]!=PadStateStable)
			{
#ifdef _DEBUG
				//printf("  Check for controller connection with controllers other than DUAL SHOCK (Complete the acquisition of controller information)\n");
#endif
				padData2.state[padNo] = PadStateStable;
				currPad = PadInfoMode(HWport, InfoModeCurID, 0);
				switch(currPad)
				{
				case 4:
#ifdef _DEBUG
					//printf("%d: Standard Pad Connected\n", padNo);
#endif
					padData.present[padNo] = PADTYPE_DIGITAL;
					padData.digital[padNo] = 0;
					break;
				case 7:
#ifdef _DEBUG
					//printf("%d: Old Analog Pad (Red Mode) Connected\n", padNo);
#endif
					padData.present[padNo] = PADTYPE_ANALOG;
					padData.digital[padNo] = 0;
					break;
				}
				continue;
			}
			break;
		case PadStateReqInfo:
#ifdef _DEBUG
			//printf("  Actuator information being retrieved (data being retrieved)\n");
#endif
			continue;
		case PadStateStable:
			if (padData2.state[padNo]!=PadStateStable)
			{
#ifdef _DEBUG
				//printf("  Retrieval of actuator information completed, or library-controller communication completed\n");
#endif
				padData2.state[padNo] = PadStateStable;
				currPad = PadInfoMode(HWport, InfoModeCurExID, 0);
				if(currPad)
				{
#ifdef _DEBUG
					//printf("%d: Found DUAL SHOCK controller\n", padNo);
#endif
					padData.present[padNo] = PADTYPE_DUALSHOCK;
					padData.digital[padNo] = 0;
					padData2.newShock[padNo] = 10;
					PadSetMainMode(HWport,0,3);
				}
				else
				{
#ifdef _DEBUG
					//printf("  Found unsupported extended controller\n");
#endif
					padData.present[padNo] = PADTYPE_NONE;
				}
				continue;
			}
			break;
		}
		if (padData.numPads[port]==4)
		{
			packet = (PadPacket *)&(padData2.buffer[port][(padLp*8)+2+2]);
			id = padData2.buffer[port][(padLp*8)+1+2]>>4;
		}
		else
		{
			packet = (PadPacket *)&(padData2.buffer[port][(padLp*8)+2+0]);
			id = padData2.buffer[port][(padLp*8)+1+0]>>4;
		}
		currPad = padData.present[padNo];
		if (currPad!=PADTYPE_DUALSHOCK)				// Dodgy old analog controller morphs into other ones
		{
			padData.present[padNo] = padGetTypeFromPacket(id);
			if (currPad!=padData.present[padNo])
			{
				switch(padData.present[padNo])
				{
				case PADTYPE_DIGITAL:
#ifdef _DEBUG
					//printf("%d: Morphing controller became digital\n", padNo);
#endif
					break;
				case PADTYPE_ANALOG:
#ifdef _DEBUG
					//printf("%d: Morphing controller became red analog\n", padNo);
#endif
					break;
				case PADTYPE_NONE:
#ifdef _DEBUG
					//printf("%d: Morphing controller became unsupported\n", padNo);
#endif
					break;
				}
			}
		}
		if (padData.present[padNo]!=PADTYPE_NONE)
		{
			padData.digital[padNo] = ~packet->digital;
			padData.analogX[padNo] = packet->leftX;
			padData.analogY[padNo] = packet->leftY;
			padData.analogX2[padNo] = packet->rightX;
			padData.analogY2[padNo] = packet->rightY;
			
			if(padData.analog[padNo])
			{
				// if we really are an analog pad, do some smoothing stuff
				NormaliseJoy2(packet->leftX, packet->leftY, &padData.analogXS[padNo], &padData.analogYS[padNo]);
				NormaliseJoy2(packet->rightX, packet->rightY, &padData.analogX2S[padNo], &padData.analogY2S[padNo]);
			}
			else
			{
				// analog emulation for digital pads

				if(padData.digital[padNo] & PAD_LEFT)
				{
					padData.analogXS[padNo] += (((-4096) - padData.analogXS[padNo]) / padData.analogAccel);
					if(padData.analogXS[padNo] > -4096)
						padData.analogXS[padNo] --;
				}
				else
				{
					if(padData.digital[padNo] & PAD_RIGHT)
					{
						padData.analogXS[padNo] += ((4096 - padData.analogXS[padNo]) / padData.analogAccel);
						if(padData.analogXS[padNo] < 4096)
							padData.analogXS[padNo] ++;
					}
					else
					{
						padData.analogXS[padNo] += ((0 - padData.analogXS[padNo]) / padData.analogAccel);
						if(padData.analogXS[padNo] > 0)
							padData.analogXS[padNo] --;
						if(padData.analogXS[padNo] < 0)
							padData.analogXS[padNo] ++;
					}
				}

				if(padData.digital[padNo] & PAD_UP)
				{
					padData.analogYS[padNo] += (((-4096) - padData.analogYS[padNo]) / padData.analogAccel);
					if(padData.analogYS[padNo] > -4096)
						padData.analogYS[padNo] --;
				}
				else
				{
					if(padData.digital[padNo] & PAD_DOWN)
					{
						padData.analogYS[padNo] += ((4096 - padData.analogYS[padNo]) / padData.analogAccel);
						if(padData.analogYS[padNo] < 4096)
							padData.analogYS[padNo] ++;
					}
					else
					{
						padData.analogYS[padNo] += ((0 - padData.analogYS[padNo]) / padData.analogAccel);
						if(padData.analogYS[padNo] > 0)
							padData.analogYS[padNo] --;
						if(padData.analogYS[padNo] < 0)
							padData.analogYS[padNo] ++;
					}
				}
			}
		}
		else
			padData.digital[padNo] = 0;
	}	
}


/**************************************************************************
	FUNCTION:	padHandleInput()
	PURPOSE:	Handle pad reading/connection etc.
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void padHandleInput()
{
	int				loop;
	unsigned short	temp;

	padHandlePort(0);
	padHandlePort(1);

	for(loop = 0; loop < 8; loop ++)
	{
		temp = (padData.digital[loop] ^ padData2.digitalPrev[loop]);
		padData.debounce[loop] = (temp & padData.digital[loop]);
		padData2.digitalPrev[loop] = padData.digital[loop];
	}
}


/**************************************************************************
	FUNCTION:	padHandleShock()
	PURPOSE:	Handle pad dual shock stuff
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void padHandleShock()
{
	int		HWport, loop;

	for(loop=0; loop<8; loop++)
	{
		padData.analog[loop] = (padData.present[loop]==PADTYPE_ANALOG)||(padData.present[loop]==PADTYPE_DUALSHOCK);

		if (padData.present[loop]==PADTYPE_DUALSHOCK)
		{
			if (padData2.newShock[loop])
			{
				if (padData2.newShock[loop]==1)
				{
					padData.motor[loop][0] = 0;
					padData.motor[loop][1] = 0;
					HWport = ((loop & 3)|((loop>>2)<<4));
					PadSetAct(HWport, padData.motor[loop], 2);
					PadSetActAlign(HWport,HWalign);
				}
				padData2.newShock[loop]--;
			}
			else
			{
				if ((padData.shock[loop]>>16)!=padData.motor[loop][1])
				{
					if (padData.shockDiv[loop]==0)
						padData.shockDiv[loop] = 1;
					padData.currShock[loop] += (padData.shock[loop]-padData.currShock[loop])/padData.shockDiv[loop];
					padData.motor[loop][1] = (padData.currShock[loop]>>16);
				}
				if (padData.buzzTime[loop]>0)
				{
					padData.buzzTime[loop]--;
					padData.motor[loop][0] = 1;
				}
				else
					padData.motor[loop][0] = 0;
			}
		}
		else
		{
			padData.motor[loop][0] = 0;
			padData.motor[loop][1] = 0;
		}
	}
}

