/************************************************************************************
	PSX CORE (c) 1999 ISL

	quatern.c:		Quaternion rotation routines

************************************************************************************/

#include <stddef.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <inline_c.h>
#include "shell.h"

#include "quatern.h"
#include "islpsi.h"
#include "psi.h"

typedef struct{
	SHORT x,y,z,w;
}SHORTQUAT;



#define IQEPSILON	4096
#define IQHALFPI	6434


/**************************************************************************
	FUNCTION:	quaternionSlerp()
	PURPOSE:	interpolate between two quaternions
	PARAMETERS:	quaternion #1, quaternion #2, interpolant (normalised), dest quaternion
	RETURNS:	
**************************************************************************/

/*
void quaternionSlerp(IQUATERNION *src1, IQUATERNION *sp2, ULONG t, IQUATERNION *dquat)
{
	LONG		omega, cosom, sinom, sclp, sclq, pdist,ndist;
	LONG		w;
	LONG		src1w,src1x,src1y,src1z;
	LONG		sp2w,sp2x,sp2y,sp2z;
	VECTOR		sqrin,sqrout;

	src1w = src1->w;
	src1x = src1->x;
	src1y = src1->y;
	src1z = src1->z;

	sp2w = sp2->w;
	sp2x = sp2->x;
	sp2y = sp2->y;
	sp2z = sp2->z;

	sqrin.vx = src1x - sp2x;
	sqrin.vy = src1y - sp2y;
	sqrin.vz = src1z - sp2z;
	
	gte_ldlvl(&sqrin);
	w = src1w - sp2w;
	gte_nop();
	gte_sqr0_b();
	w = w * w;
	gte_stlvnl(&sqrout);
		
	pdist = sqrout.vx + sqrout.vy + sqrout.vz + w;

	sqrin.vx = src1x + sp2x;
	sqrin.vy = src1y + sp2y;
	sqrin.vz = src1z + sp2z;

	gte_ldlvl(&sqrin);
	w = src1w + sp2w;
	gte_nop();
	gte_sqr0_b();
	w = w * w;
	gte_stlvnl(&sqrout);

	ndist = sqrout.vx + sqrout.vy + sqrout.vz + w;

	if (pdist > ndist)
	{
		// -ve arc is shorter, so interpolate along that
		sp2x = -sp2x;
		sp2y = -sp2y;
		sp2z = -sp2z;
		sp2w = -sp2w;
	}
	
	cosom = (src1x * sp2x + src1y * sp2y + src1z * sp2z + src1w * sp2w) >> 12;

	if ((4096 + cosom) > IQEPSILON)
	{
		if ((4096 - cosom) > IQEPSILON)
		{
			omega = arccos(cosom);

			sinom = rsin((omega * 652) >> 12); // conversion for radians
			sclp = rsin((((((4096 - t) * omega)) / sinom) * 652) >> 12);
			sclq = rsin((((t * omega) / sinom) * 652) >> 12);
			

			//sinom = rsin((omega * 100) / 628); // conversion for radians
			//sclp = rsin((((((4096 - t) * omega)) / sinom) * 100) / 628);
			//sclq = rsin((((t * omega) / sinom) * 100) / 628);

		}
		else
		{
			sclp = 4096 - t;
			sclq = t;
		}
		dquat->x = ((sclp * src1x) + (sclq * sp2x)) >> 12;
		dquat->y = ((sclp * src1y) + (sclq * sp2y)) >> 12;
		dquat->z = ((sclp * src1z) + (sclq * sp2z)) >> 12;
		dquat->w = ((sclp * src1w) + (sclq * sp2w)) >> 12;
	}
	else
	{
		sclp = rsin(((4096 - t) * IQHALFPI) >> 12);
		sclq = rsin((t * IQHALFPI) >> 12);
		dquat->x = (sclp * src1x + sclq * -src1y) >> 12;
		dquat->y = (sclp * src1y + sclq * src1x) >> 12;
		dquat->z = (sclp * src1z + sclq * -src1w) >> 12;
		dquat->w = src1z;
	}
}
*/

/**************************************************************************
	FUNCTION:	quaternionGetMatrix()
	PURPOSE:	Convert quarternion to rotation matrix
	PARAMETERS:	Source quaternion, dest matrix
	RETURNS:	
**************************************************************************/

void quaternionGetMatrix(IQUATERNION *squat, MATRIX *dmatrix)
{
	LONG	s, xs,ys,zs, wx,wy,wz, xx,xy,xz, yy,yz,zz,tempcalc;
	LONG	squatw;
	VECTOR	sqrin, sqrout;

	sqrin.vx = squat->x;
	sqrin.vy = squat->y;
	sqrin.vz = squat->z;

	gte_ldlvl(&sqrin);
	squatw = squat->w;
	gte_nop();
	gte_sqr0_b();
	squatw = squatw * squatw;
	gte_stlvnl(&sqrout);

	tempcalc = ((sqrout.vx + sqrout.vy + sqrout.vz + squatw) >> 12);
	squatw = squat->w;
	
	s = (tempcalc > 1) ? (33554432 / tempcalc) : (33554432);
	
	xs = (sqrin.vx * s) >> 12;
	ys = (sqrin.vy * s) >> 12;
	zs = (sqrin.vz * s) >> 12;

	wx = (squatw * xs) >> 12;
	wy = (squatw * ys) >> 12;
	wz = (squatw * zs) >> 12;

	xx = (sqrin.vx * xs) >> 12;
	xy = (sqrin.vx * ys) >> 12;
	xz = (sqrin.vx * zs) >> 12;

	yy = (sqrin.vy * ys) >> 12;
	yz = (sqrin.vy * zs) >> 12;
	zz = (sqrin.vz * zs) >> 12;


	dmatrix->m[0][0] = 4096 - (yy + zz);
	dmatrix->m[0][1] = xy + wz;
	dmatrix->m[0][2] = xz - wy;

	dmatrix->m[1][0] = xy - wz;
	dmatrix->m[1][1] = 4096 - (xx + zz);
	dmatrix->m[1][2] = yz + wx;

	dmatrix->m[2][0] = xz + wy;
	dmatrix->m[2][1] = yz - wx;
	dmatrix->m[2][2] = 4096 - (xx + yy);

	//doesn't actually use any of these
  //	dmatrix->matrix[0][3] = 0;
  //	dmatrix->matrix[1][3] = 0;
  //	dmatrix->matrix[2][3] = 0;
  //	dmatrix->matrix[3][3] = 4096;
  //	dmatrix->matrix[3][0] = 0;
  //	dmatrix->matrix[3][1] = 0;
  //	dmatrix->matrix[3][2] = 0;
}

void quaternionSlerpMatrix(IQUATERNION *src1, IQUATERNION *sp2, ULONG t,MATRIX *dmatrix)
{
	LONG		s, xs,ys,zs, wx,wy,wz, xx,xy,xz, yy,yz,zz,tempcalc;
	LONG		w;
	VECTOR		sqrin,sqrout;
	VECTOR		source,dest;
	LONG		cosom;

	source.vx = src1->x;
	source.vy = src1->y;
	source.vz = src1->z;

    // calc cosine
	cosom = (source.vx * sp2->x + source.vy * sp2->y + source.vz * sp2->z
	+ src1->w * sp2->w);

    // adjust signs (if necessary)
	
	if (cosom < 0)
	{
		cosom = -cosom;
		dest.vx = - sp2->x;
		dest.vy = - sp2->y;
		dest.vz = - sp2->z;
		w = - sp2->w;
	}
	else
	{
		dest.vx = sp2->x;
		dest.vy = sp2->y;
		dest.vz = sp2->z;
		w = sp2->w;
    }
    
	// "from" and "to" quaternions are very close 
	//  ... so we can do a linear interpolation
	
	gte_ldlvl(&source);		// load source
	gte_ldfc(&dest);		// load dest
	gte_lddp(t);			// load interpolant
	gte_intpl();			// interpolate (8 cycles)
	
	source.vx = src1->w;	// put source w into source
	dest.vx = w;			// put dest w into dest
		
	gte_stlvnl(&sqrin);		// put interpolated x y & z into sqrin

	gte_ldlvl(&source);		// load source w
	gte_ldfc(&dest);		// load dest w
	gte_intpl();			// interpolate (8 cycles)
	gte_stlvnl(&dest);		// put interpolated w into dest

	gte_ldlvl(&sqrin);		// load interpolated x y & z
	gte_sqr0();				// square (5 cycles)
	gte_stlvnl(&sqrout);	// put into sqrout

	gte_ldlvl(&dest);		// load interpolated w
	gte_sqr0();				// square (5 cycles)

	tempcalc = sqrout.vx + sqrout.vy + sqrout.vz;

	gte_stlvnl(&source);	// put into source

	tempcalc = ((tempcalc + source.vx) >> 12);
		
	s = (tempcalc > 1) ? (33554432 / tempcalc) : (33554432);
	
	xs = (sqrin.vx * s) >> 12;
	ys = (sqrin.vy * s) >> 12;
	zs = (sqrin.vz * s) >> 12;

	wx = (dest.vx * xs) >> 12;
	wy = (dest.vx * ys) >> 12;
	wz = (dest.vx * zs) >> 12;

	xx = (sqrin.vx * xs) >> 12;
	xy = (sqrin.vx * ys) >> 12;
	xz = (sqrin.vx * zs) >> 12;

	yy = (sqrin.vy * ys) >> 12;
	yz = (sqrin.vy * zs) >> 12;
	zz = (sqrin.vz * zs) >> 12;

	dmatrix->m[0][0] = 4096 - (yy + zz);
	dmatrix->m[0][1] = xy + wz;
	dmatrix->m[0][2] = xz - wy;

	dmatrix->m[1][0] = xy - wz;
	dmatrix->m[1][1] = 4096 - (xx + zz);
	dmatrix->m[1][2] = yz + wx;

	dmatrix->m[2][0] = xz + wy;
	dmatrix->m[2][1] = yz - wx;
	dmatrix->m[2][2] = 4096 - (xx + yy);
}

/*
void quatSlerp2(IQUATERNION *from, IQUATERNION  *to, ULONG t, IQUATERNION *res)
{
	LONG	toX,toY,toZ,toW;
	LONG	omega, cosom, sinom, scale0, scale1;

    // calc cosine
	cosom = from->x * to->x + from->y * to->y + from->z * to->z
	+ from->w * to->w;

    // adjust signs (if necessary)
	if (cosom < 0)
	{
		cosom = -cosom;
		toX = - to->x;
		toY = - to->y;
		toZ = - to->z;
		toW = - to->w;
	}
	else
	{
		toX = to->x;
		toY = to->y;
		toZ = to->z;
		toW = to->w;
    }

    // calculate coefficients

    if ( (4096 - cosom) > IQEPSILON )
	{
		// standard case (slerp)
        omega = arccos(cosom);
        sinom = rsin((omega * 652) >> 12);
        scale0 = rsin((((((4096 - t) * omega)) / sinom) * 652) >> 12);
		scale1 = rsin((((t * omega) / sinom) * 652) >> 12);
		

	}
	else
	{        
    // "from" and "to" quaternions are very close 
	    //  ... so we can do a linear interpolation
		scale0 = 4096 - t;
        scale1 = t;
    }
	// calculate final values
	res->x = (scale0 * from->x + scale1 * toX) >> 12;
	res->y = (scale0 * from->y + scale1 * toY) >> 12;
	res->z = (scale0 * from->z + scale1 * toZ) >> 12;
	res->w = (scale0 * from->w + scale1 * toW) >> 12;
}
*/


/**************************************************************************
	FUNCTION:	quaternionGetMatrix()
	PURPOSE:	Convert quarternion to rotation matrix
	PARAMETERS:	Source quaternion, dest matrix
	RETURNS:	
**************************************************************************/

void ShortquaternionGetMatrix(SHORTQUAT *squat, MATRIX *dmatrix)
{
	LONG	s, xs,ys,zs, wx,wy,wz, xx,xy,xz, yy,yz,zz,tempcalc;
	LONG	squatw;
	VECTOR	sqrin, sqrout;

	sqrin.vx = squat->x;
	sqrin.vy = squat->y;
	sqrin.vz = squat->z;

	gte_ldlvl(&sqrin);
	squatw = squat->w;
	gte_nop();
	gte_sqr0_b();
	squatw = squatw * squatw;
	gte_stlvnl(&sqrout);

	tempcalc = ((sqrout.vx + sqrout.vy + sqrout.vz + squatw) >> 12);
	squatw = squat->w;
	
	s = (tempcalc > 1) ? (33554432 / tempcalc) : (33554432);
	
	xs = (sqrin.vx * s) >> 12;
	ys = (sqrin.vy * s) >> 12;
	zs = (sqrin.vz * s) >> 12;

	wx = (squatw * xs) >> 12;
	wy = (squatw * ys) >> 12;
	wz = (squatw * zs) >> 12;

	xx = (sqrin.vx * xs) >> 12;
	xy = (sqrin.vx * ys) >> 12;
	xz = (sqrin.vx * zs) >> 12;

	yy = (sqrin.vy * ys) >> 12;
	yz = (sqrin.vy * zs) >> 12;
	zz = (sqrin.vz * zs) >> 12;


	dmatrix->m[0][0] = 4096 - (yy + zz);
	dmatrix->m[0][1] = xy + wz;
	dmatrix->m[0][2] = xz - wy;

	dmatrix->m[1][0] = xy - wz;
	dmatrix->m[1][1] = 4096 - (xx + zz);
	dmatrix->m[1][2] = yz + wx;

	dmatrix->m[2][0] = xz + wy;
	dmatrix->m[2][1] = yz - wx;
	dmatrix->m[2][2] = 4096 - (xx + yy);

	//doesn't actually use any of these
  //	dmatrix->matrix[0][3] = 0;
  //	dmatrix->matrix[1][3] = 0;
  //	dmatrix->matrix[2][3] = 0;
  //	dmatrix->matrix[3][3] = 4096;
  //	dmatrix->matrix[3][0] = 0;
  //	dmatrix->matrix[3][1] = 0;
  //	dmatrix->matrix[3][2] = 0;
}

void ShortquaternionSlerpMatrix(SHORTQUAT *src1, SHORTQUAT *sp2, ULONG t,MATRIX *dmatrix)
{
	LONG		s, xs,ys,zs, wx,wy,wz, xx,xy,xz, yy,yz,zz,tempcalc;
	LONG		w;
	VECTOR		sqrin,sqrout;
	VECTOR		source,dest;
	LONG		cosom;

	source.vx = src1->x;
	source.vy = src1->y;
	source.vz = src1->z;

    // calc cosine
	cosom = (source.vx * sp2->x + source.vy * sp2->y + source.vz * sp2->z
	+ src1->w * sp2->w);

    // adjust signs (if necessary)
	
	if (cosom < 0)
	{
		//cosom = -cosom;
		dest.vx = - sp2->x;
		dest.vy = - sp2->y;
		dest.vz = - sp2->z;
		w = - sp2->w;
	}
	else
	{
		dest.vx = sp2->x;
		dest.vy = sp2->y;
		dest.vz = sp2->z;
		w = sp2->w;
    }
    
	// "from" and "to" quaternions are very close 
	//  ... so we can do a linear interpolation
	
	gte_ldlvl(&source);		// load source
	gte_ldfc(&dest);		// load dest
	gte_lddp(t);			// load interpolant
	gte_intpl();			// interpolate (8 cycles)
	
	source.vx = src1->w;	// put source w into source
	dest.vx = w;			// put dest w into dest
		
	gte_stlvnl(&sqrin);		// put interpolated x y & z into sqrin

	gte_ldlvl(&source);		// load source w
	gte_ldfc(&dest);		// load dest w
	gte_intpl();			// interpolate (8 cycles)
	gte_stlvnl(&dest);		// put interpolated w into dest

	gte_ldlvl(&sqrin);		// load interpolated x y & z
	gte_sqr0();				// square (5 cycles)
	gte_stlvnl(&sqrout);	// put into sqrout

	gte_ldlvl(&dest);		// load interpolated w
	gte_sqr0();				// square (5 cycles)

	tempcalc = sqrout.vx + sqrout.vy + sqrout.vz;

	gte_stlvnl(&source);	// put into source

	tempcalc = ((tempcalc + source.vx) >> 12);
		
	s = (tempcalc > 1) ? (33554432 / tempcalc) : (33554432);
	
	xs = (sqrin.vx * s) >> 12;
	ys = (sqrin.vy * s) >> 12;
	zs = (sqrin.vz * s) >> 12;

	wx = (dest.vx * xs) >> 12;
	wy = (dest.vx * ys) >> 12;
	wz = (dest.vx * zs) >> 12;

	xx = (sqrin.vx * xs) >> 12;
	xy = (sqrin.vx * ys) >> 12;
	xz = (sqrin.vx * zs) >> 12;

	yy = (sqrin.vy * ys) >> 12;
	yz = (sqrin.vy * zs) >> 12;
	zz = (sqrin.vz * zs) >> 12;

	dmatrix->m[0][0] = 4096 - (yy + zz);
	dmatrix->m[0][1] = xy + wz;
	dmatrix->m[0][2] = xz - wy;

	dmatrix->m[1][0] = xy - wz;
	dmatrix->m[1][1] = 4096 - (xx + zz);
	dmatrix->m[1][2] = yz + wx;

	dmatrix->m[2][0] = xz + wy;
	dmatrix->m[2][1] = yz - wx;
	dmatrix->m[2][2] = 4096 - (xx + yy);
}
