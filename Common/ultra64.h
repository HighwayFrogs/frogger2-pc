																																																																																			
/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1994, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright  law.  They  may not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 *************************************************************************/

/**************************************************************************
 *
 *  $Revision: 1.10 $
 *  $Date: 1997/02/11 08:37:33 $
 *  $Source: /disk6/Master/cvsmdev2/PR/include/ultra64.h,v $
 *
 **************************************************************************/

#ifndef _ULTRA64_H_
#define _ULTRA64_H_

#define Gfx char
#define Mtx	char		//void
#define Bitmap void
// #define Vtx float

#define uSprite unsigned char
#define Sprite unsigned char

#define u8  unsigned char					/* unsigned  8-bit */
#define u16 unsigned short					/* unsigned  16-bit */
#define u32 unsigned long					/* unsigned  32-bit */
//#define u64 unsigned long long				/* unsigned  64-bit */
#define u64 unsigned _int64					/* unsigned  64-bit */

#define s8  signed char						/* unsigned  8-bit */
#define s16 signed short					/* unsigned  16-bit */
#define s32 signed long						/* unsigned  32-bit */
//#define s64 signed long long				/* unsigned  64-bit */
#define s64 _int64							/* unsigned  64-bit */

#define vu8  volatile unsigned char			/* unsigned  8-bit */
#define vu16 volatile unsigned short		/* unsigned  16-bit */
#define vu32 volatile unsigned long			/* unsigned  32-bit */
#define vu64 volatile unsigned long long	/* unsigned  64-bit */

#define vs8  volatile signed char			/* unsigned  8-bit */
#define vs16 volatile signed short			/* unsigned  16-bit */
#define vs32 volatile signed long			/* unsigned  32-bit */
#define vs64 volatile signed long long		/* unsigned  64-bit */

// #define f32 float
#define f64 double

/*************************************************************************
 * Common definitions
 */
#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef NULL
#define NULL    0
#endif


#ifdef PSX_VERSION

#ifndef BOOL
#define BOOL    int
#endif

#ifndef ULONG
#define ULONG    unsigned long
#endif

#ifndef USHORT
#define USHORT    unsigned short
#endif

#endif


#endif
