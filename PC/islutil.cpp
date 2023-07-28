#include "islutil.h"

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <windows.h>

#define DEBUG_FILE "C:\\frogger2.log"

//returns fixed format

// ---------------------------------------------------------------------------------------------
// Function: fast_sqrt
// Purpose : Perform a fast square root of a number
// Input   : Pointer to the source float
// Output  : Square root of source float
// ---------------------------------------------------------------------------------------------
float fast_sqrt(float *src)
{
	float retval = 0.0f;

	__asm {
		mov		ebx, [src]
		mov		eax, [ebx]
		sub		eax, 0x3F800000
		sar		eax, 1
		add		eax, 0x3F800000
		mov		[retval], eax
	}

	return(retval);
}

float n;
float *n2 = &n;
float retval = 0.0f;
	
unsigned long utilSqrt(unsigned long num)
{
	n = num;
	__asm {
		mov		ebx, [n2]
		mov		eax, [ebx]
		sub		eax, 0x3F800000
		sar		eax, 1
		add		eax, 0x3F800000
		mov		[retval], eax
	}
	return (unsigned long)(retval*65536);
//	return (unsigned long)(sqrt(num)*4096);
//	return (unsigned long)(fast_sqrt(&n)*65536);
}

//return angle -2048 to +2048
/*int utilCalcAngle(int adj, int opp)
{
	return (int)((atan2(adj,opp) / 6.283185308) * 4096.0);
}*/

#ifndef FINAL_MASTER

int utilPrintf(char* fmt, ...)				
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	
	OutputDebugString(buffer);

	FILE *f = fopen(DEBUG_FILE, "a+ct");
	if (f)
	{
		fputs(buffer, f);
		fclose(f);
	}

	return 0;
}

#endif