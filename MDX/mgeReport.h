/*

	This file is part of the M libraries,

	File		: mliReport.h
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef MDBREPORT_H_INCLUDED
#define MDBREPORT_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include "windef.h"


extern char debugFile[MAX_PATH];

// A value, string pair for reporting purposes!
typedef struct TAG_MDX_VALUESTRING
{
	unsigned long value;
	char *string;
}MDX_VALUESTRING;

/*	--------------------------------------------------------------------------------
	Function	: dp
	Purpose		: Print debug message
	Parameters	: same as printf
	Returns		: 
	Info		: 
*/

void dp(char *format, ...);

/*	--------------------------------------------------------------------------------
	Function	: rptShowBitfields
	Purpose		: Prints out the capabilities held in a bitfield from specified VALUESTRING
	Parameters	: (unsigned long value, VALUESTRING *me)
	Returns		: number of fields matched
	Info		: 
*/

long rptShowBitfields(unsigned long value, MDX_VALUESTRING *me);

/*	--------------------------------------------------------------------------------
	Function	: rptGetString
	Purpose		: Get a string from a Value, for example and error value
	Parameters	: (unsigned long value, VALUESTRING *me)
	Returns		: string that matches
	Info		: 
*/

char *rptGetString(unsigned long value, MDX_VALUESTRING *me);

#ifdef __cplusplus
}
#endif

#endif