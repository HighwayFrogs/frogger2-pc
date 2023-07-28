/*

	This file is part of the M libraries,

	File		: 
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef MDXTIMING_H_INCLUDED
#define MDXTIMING_H_INCLUDED

#ifdef __cplusplus

extern "C"
{
#endif


typedef struct 
{
	long firstTicks;		// was unsigned long -ds

	unsigned long tickCount;
	unsigned long frameCount;

	long tickModifier, maxTickIncrement;

	float frameSpeed;
	float speed;

} MDX_TIMING_VAL; 

extern MDX_TIMING_VAL timeInfo;

#define CFRAME (timeInfo.frameCount)

/*	--------------------------------------------------------------------------------
	Function	: 
	Purpose		: 
	Parameters	: 
	Returns		: 
	Info		: 
*/

void InitTiming(float frameSpeed);

/*	--------------------------------------------------------------------------------
	Function	: 
	Purpose		: 
	Parameters	: 
	Returns		: 
	Info		: 
*/

void UpdateTiming(void);

#ifdef __cplusplus
}
#endif

#endif