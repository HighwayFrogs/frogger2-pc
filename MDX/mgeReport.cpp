/*

	This file is part of the M libraries,

	File		: mliReport.c
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

//#ifdef __cplusplus

//extern "C"
//{
//#endif

#include <windows.h>
#include <crtdbg.h>
#include <stdio.h>

#include "mgeReport.h"

//TODO: rename from mli to mge (Library -> Generic)

//TODO: move following out
// mge - Generic
// mdx - DirectX

char debugFile[MAX_PATH] = "c:\\mdebug.log";

/*	--------------------------------------------------------------------------------
	Function	: dp
	Purpose		: Print debug message
	Parameters	: same as printf
	Returns		: 
	Info		: 
*/

void dp(char *format, ...)
{
	va_list			argp;
	FILE *f;

	static char		debugprintfbuffer[2048];

	va_start(argp, format);
	vsprintf(debugprintfbuffer, format, argp);
	va_end(argp);

#ifdef _DEBUG
	_CrtDbgReport(_CRT_WARN, NULL, NULL, "A3Ddemo", debugprintfbuffer);
#endif
	f = fopen(debugFile, "a+");
	if (f)
	{
		fputs(debugprintfbuffer, f);
		fclose(f);
	}
}

/*	--------------------------------------------------------------------------------
	Function	: rptShowBitfields
	Purpose		: Prints out the capabilities held in a bitfield from specified VALUESTRING
	Parameters	: (unsigned long value, VALUESTRING *me)
	Returns		: number of fields matched
	Info		: 
*/

long rptShowBitfields(unsigned long value, MDX_VALUESTRING *me)
{
	unsigned long matched = 0;
	
	while (me->string)
	{
		if (me->value & value)
		{
			dp(me->string);
			matched++;
		}
		me++;
	}

	return matched;
}

/*	--------------------------------------------------------------------------------
	Function	: rptGetString
	Purpose		: Get a string from a Value, for example and error value
	Parameters	: (unsigned long value, VALUESTRING *me)
	Returns		: string that matches
	Info		: 
*/

char *rptGetString(unsigned long value, MDX_VALUESTRING *me)
{
	while (me->string)
	{
		if (me->value == value)
			return me->string;
		me++;
	}
	return NULL;
}

//#ifdef __cplusplus
//}
//#endif
