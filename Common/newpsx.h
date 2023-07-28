#ifndef NEW_PSX_H
#define NEW_PSX_H

#include <isltex.h>

#define SCALE 10

//#define sqrt(x)		(utilSqrt(x)>>16)
//#define sqrtF(x)	(utilSqrt(x)>>22)
#define Fsqrt(x)	(utilSqrt(x)>>4)
#define FsqrtF(x)	(utilSqrt(x)>>10)

//#define cosf(x)		(0)
//#define sinf(x)		(0)

//pc extern TextureAnimType* newBabyAnim;
//pc extern TextureType*     newBabyAnimDest;


#endif
