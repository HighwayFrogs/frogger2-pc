#ifndef _ISLUTIL_H_INCLUDE
#define _ISLUTIL_H_INCLUDE

#include "crc32.h"
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef FINAL_MASTER
#define utilPrintf(x)		// disables debug output
#else
int utilPrintf(char* fmt, ...);
#endif

unsigned long utilSqrt(unsigned long num);


//int utilCalcAngle(int adj, int opp);

// jhubbard
#define utilCalcAngle(adj,opp) ((int)((atan2(adj,opp) / 6.283185308) * 4096.0))
#define utilStr2CRC(x) UpdateCRC(x)

#ifdef __cplusplus
}
#endif

#endif
