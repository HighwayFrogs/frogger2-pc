#ifndef MEMLOAD_H_INCLUDED
#define MEMLOAD_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

extern int MemLoadEntities(const void* data);
extern int MemLoadEvents(const void* data, long size);

extern int MemLoadInt(unsigned char**);
extern short MemLoadWord(unsigned char**);
//extern char *MemLoadString(UBYTE **p);

#define MEMGETBYTE(p)	(*((*p)++))
//#define MEMGETFLOAT(p) ((float)MEMGETINT(p) / (float)0x10000)
#define MEMGETINT(p)	MemLoadInt(p)
#define MEMGETWORD(p)	MemLoadWord(p)
#define MEMGETFIXED(p) (MEMGETINT(p)>>4 )  


#ifdef __cplusplus
}
#endif
#endif