#ifndef _LIBGTE_H_INCLUDE
#define _LIBGTE_H_INCLUDE

//#define rsin(x) (sin((x/4096.0)*3.14)*4096)
//#define rcos(x) (cos((x/4096.0)*3.14)*4096)
#define rsin(x) (fixed)(sin(((x)*(1.0f/4096.0f))*PI2)*4096)
#define rcos(x) (fixed)(cos(((x)*(1.0f/4096.0f))*PI2)*4096)

typedef struct
{		/* long word type 3D vector */
	long	vx, vy;
	long	vz, pad;
} VECTOR;

typedef struct
{		/* short word type 3D vector */	
	short	vx, vy;
	short	vz, pad;
} SVECTOR;

#endif