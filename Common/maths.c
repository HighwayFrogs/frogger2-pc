 /*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.
	
		File		: maths.c
		Programmer	: 
		Date		: 29/04/99 11:08:07
		
----------------------------------------------------------------------------------------------- */

#include <islutil.h>
#include <limits.h>
#include "ultra64.h"
#include "maths.h"
#include "newpsx.h"
#include "layout.h"


#ifdef PC_VERSION
#include <pcmisc.h>
#endif

#ifdef DREAMCAST_VERSION
#include <gte.h>
#include "main.h"
#endif

#define EulFrmS	     0
#define EulFrmR	     1
#define EulFrm(ord)  ((unsigned)(ord)&1)
#define EulRepNo     0
#define EulRepYes    1
#define EulRep(ord)  (((unsigned)(ord)>>1)&1)
#define EulParEven   0
#define EulParOdd    1
#define EulPar(ord)  (((unsigned)(ord)>>2)&1)
#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulAxI(ord)  ((int)(EulSafe[(((unsigned)(ord)>>3)&3)]))
#define EulAxJ(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)  ((EulRep(ord)==EulRepNo)?EulAxK(ord):EulAxI(ord))
    /* EulGetOrd unpacks all useful information about order simultaneously. */
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>=1;s=o&1;o>>=1;\
    n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
    /* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
    /* Static axes */
#define EulOrdXYZs    EulOrd(X,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(X,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(X,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(X,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(Y,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(Y,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(Y,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(Y,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(Z,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(Z,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(Z,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(Z,EulParOdd,EulRepYes,EulFrmS)
    /* Rotating axes */
#define EulOrdZYXr    EulOrd(X,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(X,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(X,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(X,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(Y,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(Y,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(Y,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(Y,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(Z,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(Z,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(Z,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(Z,EulParOdd,EulRepYes,EulFrmR)


//short bb_acostable[4097];	//0->4096 inclusive. Stores PSX angles
//short bb_acostable[8193];	//-4096->4096 inclusive. Stores PSX angles

#define COPYVECTOR(U,V){	(U)->vx=(V)->vx; \
							(U)->vy=(V)->vy; \
							(U)->vz=(V)->vz; }

void fixedRotateVectorByRotationS(FVECTOR *result, SVECTOR *vect, IQUATERNION *rot)
{
	fixed	m,n,sinTheta,cosTheta;
	FVECTOR	mVec,pVec,vVec;

//bbopt
//   	m  = FMul(vect->vx, rot->x);
//   	m += FMul(vect->vy, rot->y);
//   	m += FMul(vect->vz, rot->z);
  	m  = (vect->vx * rot->x) >>12;
  	m += (vect->vy * rot->y) >>12;
  	m += (vect->vz * rot->z) >>12;

	// quats are in 4096 format

//	m/=4096;	// I think this is needed

//bbopt
//   	mVec.vx = FMul(m, rot->x);
//   	mVec.vy = FMul(m, rot->y);
//   	mVec.vz = FMul(m, rot->z);
  	mVec.vx = (m * rot->x) >>12;
  	mVec.vy = (m * rot->y) >>12;
  	mVec.vz = (m * rot->z) >>12;

	SubVectorFSF(&pVec,vect,&mVec);
	
	// This version does scale down
	CrossProductFFF(&vVec, (FVECTOR *)rot, &pVec);

	m = MagnitudeF(&pVec);

	if(m == 0)
	{
		COPYVECTOR(result,vect);
	}
	else
	{
		n = MagnitudeF(&vVec);
		if(n)
		{
//			m = m*4096;
//			m /= n;
			m = FDiv(m, n);
//			SCALEVECTOR(&vVec,m);
			vVec.vx = FMul(vVec.vx, m);
			vVec.vy = FMul(vVec.vy, m);
			vVec.vz = FMul(vVec.vz, m);
		}

		cosTheta = rcos(rot->w);	// could the problem be a radians to fixed problem?
		sinTheta = rsin(rot->w);

//		result->vz = (mVec.vx + ((cosTheta * pVec.vx)/4096) + ((sinTheta * vVec.vx)/4096) );
//		result->vy = (mVec.vy + ((cosTheta * pVec.vy)/4096) + ((sinTheta * vVec.vy)/4096) );
//		result->vz = (mVec.vz + ((cosTheta * pVec.vz)/4096) + ((sinTheta * vVec.vz)/4096) );

//bbopt
  		result->vx = mVec.vx + FMul(cosTheta, pVec.vx) + FMul(sinTheta, vVec.vx);
  		result->vy = mVec.vy + FMul(cosTheta, pVec.vy) + FMul(sinTheta, vVec.vy);
  		result->vz = mVec.vz + FMul(cosTheta, pVec.vz) + FMul(sinTheta, vVec.vz);
//  		result->vx = mVec.vx + ((cosTheta * pVec.vx)>>12) + ((sinTheta, vVec.vx)>>12);
//  		result->vy = mVec.vy + ((cosTheta * pVec.vy)>>12) + ((sinTheta, vVec.vy)>>12);
//  		result->vz = mVec.vz + ((cosTheta * pVec.vz)>>12) + ((sinTheta, vVec.vz)>>12);
	}
}

void fixedQuaternionMultiply(IQUATERNION *dest,IQUATERNION *src1,IQUATERNION *src2)
{
	LONG	dp;
	IQUATERNION temp;

	dp = (src1->x*src2->x + src1->y*src2->y + src1->z*src2->z);

// For the benefit of people doing optimisations and breaking things,
// I'll reiterate the point made in the function header
// DESTINATION CAN BE THE SAME AS SOURCE, Hence the temp store (I'll add a dest != src version)

//	temp.x = (src1->w*src2->x + src2->w*src1->x + src1->y*src2->z - src1->z*src2->y)/4096;
//	temp.y = (src1->w*src2->y + src2->w*src1->y + src1->z*src2->x - src1->x*src2->z)/4096;
//	temp.z = (src1->w*src2->z + src2->w*src1->z + src1->x*src2->y - src1->y*src2->x)/4096;
	temp.x = (src1->w*src2->x + src2->w*src1->x + src1->y*src2->z - src1->z*src2->y)>>12;
	temp.y = (src1->w*src2->y + src2->w*src1->y + src1->z*src2->x - src1->x*src2->z)>>12;
	temp.z = (src1->w*src2->z + src2->w*src1->z + src1->x*src2->y - src1->y*src2->x)>>12;

//	dest->w= (src1->w*src2->w - dp)/4096;
	dest->w= (src1->w*src2->w - dp)>>12;
	dest->x=temp.x;
	dest->y=temp.y;
	dest->z=temp.z;
}

// void CrossProductFFF(FVECTOR *result, FVECTOR *operand1, FVECTOR *operand2)
// {
// 	result->vx = FMul(operand1->vy, operand2->vz) - FMul(operand1->vz, operand2->vy);
// 	result->vy = FMul(operand1->vz, operand2->vx) - FMul(operand1->vx, operand2->vz);
// 	result->vz = FMul(operand1->vx, operand2->vy) - FMul(operand1->vy, operand2->vx);
// }

// fixed DotProductFF(FVECTOR *v1, FVECTOR *v2)
// {
// 	return ( FMul(v1->vx, v2->vx)
// 		  +  FMul(v1->vy, v2->vy)  
// 		  +  FMul(v1->vz, v2->vz) );
// }

LONG arccos(LONG cos)
{
#ifndef PSX_VERSION
 	return (long)(acos(cos/4096.0F)*(4096/PI2));
#endif

// 	cos >>= 2;
// 
// 	if(cos<0)
// 		return 2048 - acostable[cos +1024];
// 	else
// 		return acostable[cos];


 	if(cos<-4096)
	{
//		utilPrintf("Bad Call arccos(%d)\n", cos);
 		cos = -4096;
	}

 	if(cos>4096)
	{
//		utilPrintf("Bad Call arccos(%d)\n", cos);
 		cos = 4096;
	}

//	return bb_acostable[cos+4096];

	
#ifdef DREAMCAST_VERSION
	if(cos>=0)
	{
		return acostable[cos];
	}
	else
	{
		return 2048-acostable[-cos];
	}
#else
	//table stores acos of 0->4096.
	//-4096->0 is upside down/mirrored
	if(cos>=0)
	{
		//[ANDYE]	return bb_acostable[cos];
		return 0;
	}
	else
	{
		//[ANDYE]	return 2048-bb_acostable[-cos];
		return 0;
	}
#endif
}


/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
ULONG rseed=398623;

int Random(ULONG lim)
{

	if(lim == 0)
		return 0;

	rseed = rseed^(rseed<<1);
	if ((rseed & 0x80000000)==0)
		rseed++;
	else
		if (rseed == 0x80000000)
			rseed = 0;

	return (rseed%lim);
}

/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
void SeedRandom()
{
	rseed = 398623;
}


/*	--------------------------------------------------------------------------------
	Function 	: GetRotationFromQuaternion()
	Purpose 	: converts a quaternion into a rotation of theta degrees about an axis
	Parameters 	: destination, source
	Returns 	: nun
	Info 		:
*/
// void GetRotationFromQuaternion(QUATERNION *destQ,QUATERNION *srcQ)
// {
// }

void fixedGetQuaternionFromRotation(IQUATERNION *destQ,IQUATERNION *srcQ)
{
	fixed thetaOver2;
	fixed sinThetaOver2;

	thetaOver2 = srcQ->w/2;
	sinThetaOver2 = rsin(thetaOver2);

	destQ->w = rcos(thetaOver2);
// 	destQ->x = (sinThetaOver2 * srcQ->x)/4096;
// 	destQ->y = (sinThetaOver2 * srcQ->y)/4096;
// 	destQ->z = (sinThetaOver2 * srcQ->z)/4096;
	destQ->x = (sinThetaOver2 * srcQ->x)>>12;
	destQ->y = (sinThetaOver2 * srcQ->y)>>12;
	destQ->z = (sinThetaOver2 * srcQ->z)>>12;
}

void fixedRotateVectorByRotation(FVECTOR *result, FVECTOR *vect, IQUATERNION *rot)
{

	fixed	m,n,sinTheta,cosTheta;
	FVECTOR	mVec,pVec,vVec;

//bbopt
//   	m  = FMul(vect->vx, rot->x);
//   	m += FMul(vect->vy, rot->y);
//   	m += FMul(vect->vz, rot->z);
  	m  = (vect->vx * rot->x) >>12;
  	m += (vect->vy * rot->y) >>12;
  	m += (vect->vz * rot->z) >>12;

	// quats are in 4096 format

//	m/=4096;	// I think this is needed

//bbopt
//   	mVec.vx = FMul(m, rot->x);
//   	mVec.vy = FMul(m, rot->y);
//   	mVec.vz = FMul(m, rot->z);
  	mVec.vx = (m * rot->x) >>12;
  	mVec.vy = (m * rot->y) >>12;
  	mVec.vz = (m * rot->z) >>12;

	SubVectorFFF(&pVec,vect,&mVec);
	
	// This version does scale down
	CrossProductFFF(&vVec, (FVECTOR *)rot, &pVec);

	m = MagnitudeF(&pVec);

	if(m == 0)
	{
		COPYVECTOR(result,vect);
	}
	else
	{
		n = MagnitudeF(&vVec);
		if(n)
		{
//			m = m*4096;
//			m /= n;
			m = FDiv(m, n);
//			SCALEVECTOR(&vVec,m);
			vVec.vx = FMul(vVec.vx, m);
			vVec.vy = FMul(vVec.vy, m);
			vVec.vz = FMul(vVec.vz, m);
		}

		cosTheta = rcos(rot->w);	// could the problem be a radians to fixed problem?
		sinTheta = rsin(rot->w);

//		result->vz = (mVec.vx + ((cosTheta * pVec.vx)/4096) + ((sinTheta * vVec.vx)/4096) );
//		result->vy = (mVec.vy + ((cosTheta * pVec.vy)/4096) + ((sinTheta * vVec.vy)/4096) );
//		result->vz = (mVec.vz + ((cosTheta * pVec.vz)/4096) + ((sinTheta * vVec.vz)/4096) );

//bbopt
  		result->vx = mVec.vx + FMul(cosTheta, pVec.vx) + FMul(sinTheta, vVec.vx);
  		result->vy = mVec.vy + FMul(cosTheta, pVec.vy) + FMul(sinTheta, vVec.vy);
  		result->vz = mVec.vz + FMul(cosTheta, pVec.vz) + FMul(sinTheta, vVec.vz);
//  		result->vx = mVec.vx + ((cosTheta * pVec.vx)>>12) + ((sinTheta, vVec.vx)>>12);
//  		result->vy = mVec.vy + ((cosTheta * pVec.vy)>>12) + ((sinTheta, vVec.vy)>>12);
//  		result->vz = mVec.vz + ((cosTheta * pVec.vz)>>12) + ((sinTheta, vVec.vz)>>12);
	}
}


/*	--------------------------------------------------------------------------------
	Function 	: DistanceBetweenPoints2D()
	Purpose 	: finds the distance between two points
	Parameters 	: vector 1, vector 2
	Returns 	: distance
	Info 		: destination CAN be same as source
*/
fixed DistanceBetweenPoints2D(VECTOR *v1, VECTOR *v2)
{
	VECTOR tempVect;

	SubVector2D(&tempVect,v1,v2);
	return Magnitude2DV(&tempVect);
}

/*	--------------------------------------------------------------------------------
	Function 	: DistanceBetweenPoints2D()
	Purpose 	: finds the distance between two points
	Parameters 	: vector 1, vector 2
	Returns 	: distance
	Info 		: destination CAN be same as source
*/

/*	--------------------------------------------------------------------------------
	Function 	: DistanceBetweenPoints()
	Purpose 	: finds the distance between two points
	Parameters 	: vector 1, vector 2
	Returns 	: distance
	Info 		:
*/
fixed DistanceBetweenPointsVV(VECTOR *v1, VECTOR *v2)
{
	VECTOR tempVect;

	SubVectorVVV(&tempVect,v1,v2);
	return MagnitudeV(&tempVect);
}

fixed DistanceBetweenPointsSS(SVECTOR *v1, SVECTOR *v2)
{
// 	VECTOR tempVect;
// 	SubVectorVSS(&tempVect,v1,v2);
// 	return MagnitudeV(&tempVect);

 	VECTOR tempVect;
 	SubVectorVSS(&tempVect,v1,v2);
  	return Fsqrt(tempVect.vx*tempVect.vx
 			   +tempVect.vy*tempVect.vy
 			   +tempVect.vz*tempVect.vz);
}

fixed DistanceBetweenPointsSV(SVECTOR *v1, VECTOR *v2)
{
// 	VECTOR tempVect;
// 	SubVectorVSV(&tempVect,v1,v2);
// 	return MagnitudeV(&tempVect);

 	VECTOR tempVect;
 	SubVectorVSV(&tempVect,v1,v2);
   	return Fsqrt(tempVect.vx*tempVect.vx
  			   +tempVect.vy*tempVect.vy
  			   +tempVect.vz*tempVect.vz);
}

fixed DistanceBetweenPointsSF(SVECTOR *v1, FVECTOR *v2)
{
	FVECTOR tempVect;

	//ScaleVectorFF ( &tempVect, 4096 );
	SubVectorFSF(&tempVect,v1,v2);
	return MagnitudeF(&tempVect);
}

fixed DistanceBetweenPointsFS(FVECTOR *v1, SVECTOR *v2)
{
	FVECTOR tempVect;

	SubVectorFFS(&tempVect,v1,v2);
	return MagnitudeF(&tempVect);
}

fixed DistanceBetweenPointsFF(FVECTOR *v1, FVECTOR *v2)
{
	FVECTOR tempVect;

	SubVectorFFF(&tempVect,v1,v2);
	return MagnitudeF(&tempVect);
}

/*	--------------------------------------------------------------------------------
	Function 	: DistanceBetweenPointsSqr()
	Purpose 	: finds the distance SQUARED between two points
	Parameters 	: vector 1, vector 2
	Returns 	: distance
	Info 		:
*/

long DistanceBetweenPointsSqrSS(SVECTOR *v1, SVECTOR *v2)
{
	VECTOR tempVect;

	SubVectorVSS(&tempVect,v1,v2);

/*	something to do with overflows .. problems here!

	if ((abs(tempVect.vx) > 2000) ||
		(abs(tempVect.vy) > 2000) ||
		(abs(tempVect.vz) > 2000))
	{
		return LONG_MAX;
	}
*/

	return (long)tempVect.vx*(long)tempVect.vx + (long)tempVect.vy*(long)tempVect.vy + (long)tempVect.vz*(long)tempVect.vz;
}

long DistanceBetweenPointsSqrFS(FVECTOR *v1, SVECTOR *v2)
{
	FVECTOR tempVect;

	SubVectorFFS(&tempVect,v1,v2);
	return (FMul(tempVect.vx,tempVect.vx)>>12) + (FMul(tempVect.vy,tempVect.vy)>>12) + (FMul(tempVect.vz,tempVect.vz)>>12);
}

/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/




/*	--------------------------------------------------------------------------------
	Function 	: IsPointWithinBox
	Purpose 	: determine whether a point is within a box
	Parameters 	: box* vector*
	Returns 	: BOOL
	Info 		:
*/
BOOL IsPointWithinBox(BOX *box, VECTOR *pos)
{
	if((pos->vx >= box->pos.vx) && (pos->vx <= box->pos.vx + box->size.vx) &&
	   (pos->vy >= box->pos.vy) && (pos->vy <= box->pos.vy + box->size.vy) &&
	   (pos->vz >= box->pos.vz) && (pos->vz <= box->pos.vz + box->size.vz))
	   return TRUE;

	return FALSE;
}

/*	--------------------------------------------------------------------------------
	Function 	: IsPointWithinBox
	Purpose 	: determine whether a point is within a box
	Parameters 	: box* vector*
	Returns 	: BOOL
	Info 		:
*/
BOOL IsPointWithinBoxPlusHeight(BOX *box, VECTOR *pos,fixed height)
{
	if((pos->vx >= box->pos.vx) && (pos->vx <= box->pos.vx + box->size.vx) &&
	   (pos->vy >= box->pos.vy) && (pos->vy <= box->pos.vy + box->size.vy + (height>>12)) &&
	   (pos->vz >= box->pos.vz) && (pos->vz <= box->pos.vz + box->size.vz))
	   return TRUE;

	return FALSE;
}

/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
BOOL IsPointWithinBox2D(BOX *box, VECTOR *pos)
{
	if((pos->vx >= box->pos.vx) && (pos->vx <= box->pos.vx + box->size.vx) &&
	   (pos->vz >= box->pos.vz) && (pos->vz <= box->pos.vz + box->size.vz))
	   return TRUE;

	return FALSE;
}

/*	--------------------------------------------------------------------------------
	Function 	: RotateVector2D
	Purpose 	: rotates the X and Z points around the Y axis, by given angle
	Parameters 	: result vector, source vector, angle
	Returns 	: 
	Info 		: destination CAN be same as source
*/
void RotateVector2D(FVECTOR *dest, FVECTOR *source, fixed angle)
{
	FVECTOR temp;
	fixed cosangle;
	fixed sinangle;

	angle = -angle;

//bb
////mm	cosangle = ToFixed(cosf(angle/4096.0F));
//	sinangle = ToFixed(sinf(angle/4096.0F));
	cosangle = rcos(angle);
	sinangle = rsin(angle);

	temp.vy = source->vy;
	temp.vx = FMul(cosangle, source->vx) + FMul(-sinangle, source->vz);
	temp.vz = FMul(cosangle, source->vz) + FMul(sinangle, source->vx);


	SetVectorFF(dest, &temp);
}

/*	--------------------------------------------------------------------------------
	Function 	: RotateVector2D
	Purpose 	: rotates the X and Z points around the Y axis, by given angle
	Parameters 	: result vector, source vector, angle
	Returns 	: 
	Info 		: destination CAN be same as source
*/
void RotateVector2DX(FVECTOR *dest, FVECTOR *source, fixed angle)
{
	FVECTOR	temp;
	fixed cosangle;
	fixed sinangle;

	
	cosangle = rcos(angle);
	sinangle = rsin(angle);


	temp.vx = source->vx;
	temp.vy = FMul(cosangle,source->vy) + FMul(-sinangle,source->vz);
	temp.vz = FMul(cosangle,source->vz) + FMul(sinangle,source->vy);

// 	temp.v[axis] = source->v[axis];
// 	temp.v[x] = (source->v[x] * cosf(angle/4096.0F)) + (source->v[z] * -sinf(angle/4096.0F));
// 	temp.v[z] = (source->v[z] * cosf(angle/4096.0F)) + (source->v[x] *  sinf(angle/4096.0F));

	SetVectorFF(dest, &temp);
}

  
/*	--------------------------------------------------------------------------------
	Function 	: FlipVector()
	Purpose 	: flips a vector
	Parameters 	: pointer to vector
	Returns 	: none
	Info 		: destination CAN be same as source
*/
void FlipVector(VECTOR *vector)
{
	vector->vx = -vector->vx;
	vector->vy = -vector->vy;
	vector->vz = -vector->vz;
}

/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
void FlipVector2D(VECTOR *vector)
{
	vector->vx = -vector->vx;
	vector->vz = -vector->vz;
}

/*	--------------------------------------------------------------------------------
	Function 	: FindShortestAngle()
	Purpose 	: returns the smallest angle between 2 directions (rads)
	Parameters 	: angle1, angle2
	Returns 	: angle
	Info 		:
*/
fixed FindShortestAngle(fixed val1, fixed val2)
{
	fixed temp;

//bb
//	if(Fabs((val1 - val2)/4096.0F) > PI)	
//	if(Fabs(val1-val2) > fPI)
	if(Fabs(val1-val2) > 12868)
	{
		if(val1 > val2)
		{
			temp = val2 - val1;
		}
		else
		{
			temp = val1 - val2;
		}
//bb
//		temp += ToFixed(PI * 2);
//		temp += fPI*2;
		temp += 51472;
		return temp;
	}
	else
	{
//bb
//	   return ToFixed(Fabs((val1 - val2)/4096.0F));
	   return Fabs(val1 - val2);
	}
}

/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
fixed FindShortestAngleSigned(fixed ang1,fixed ang2)
{
//bb
// 	fixed temp = ang1 - ang2;
// 	if((temp > ToFixed(-PI)) && (temp <= ToFixed(PI)))
// 		return temp;
// 	else if(temp > ToFixed(PI))
// 		return ang1 - ang2 - ToFixed(2*PI);
// 	else
// 		return ang1 + ToFixed(2*PI) - ang2;

// 	fixed temp = ang1 - ang2;
// 	if( (temp>-fPI) && (temp<=fPI) )
// 		return temp;
// 	else if(temp > fPI)
// 		return ang1 - ang2 - (2*fPI);
// 	else
// 		return ang1 + (2*fPI) - ang2;

	fixed temp = ang1 - ang2;
	if( (temp>-2048) && (temp<=2048) )
		return temp;
	else if(temp > 2048)
		return ang1 - ang2 - 4096;
	else
		return ang1 + 4096 - ang2;
}

/*	--------------------------------------------------------------------------------
	Function 	: Magnitude()
	Purpose 	: find the magnitude of a vector
	Parameters 	: pointer to vector
	Returns 	: magnitude
	Info 		:
*/

// fixed MagnitudeV(VECTOR *vect)
// {
// 
//  	return Fsqrt(vect->vx*vect->vx
//  					   +vect->vy*vect->vy
//  					   +vect->vz*vect->vz);
// 
// /*
// //slower(!)
// 	VECTOR v;
// 
// 	gte_ldlvl(vect);
// 	gte_sqr0();
// 	gte_stlvnl(&v);
// 
// 	return Fsqrt(v.vx + v.vy + v.vz);
// */
// 
// /*
// 	VECTOR v = *vect;
// 
// 	gte_ldlvl(&v);
// 	gte_sqr0();
// 	gte_stlvnl(&v);
// 
// 	return Fsqrt(v.vx + v.vy + v.vz);
// */
// }

fixed MagnitudeS(SVECTOR *vect)
{
// 	VECTOR tmp;
// 	SetVectorVS(&tmp, vect);
// 	return MagnitudeV(&tmp);

 	VECTOR tmp;
 	SetVectorVS(&tmp, vect);
  	return Fsqrt(tmp.vx*tmp.vx
  				+tmp.vy*tmp.vy
  				+tmp.vz*tmp.vz);
}

fixed MagnitudeF(FVECTOR *v)
{
	ULONG x,y,z,calc;
	ULONG l;
	ULONG num;
	ULONG up;
	ULONG s=0;

	num=0;

	x=labs(v->vx);
	num|=x;

	y=labs(v->vy);
	num|=y;

	z=labs(v->vz);
	num|=z;

	while(num > 26754)
	{
		num = num>>2;
		s+=2;
	}
	x=(x)>>s;
	y=(y)>>s;
	z=(z)>>s;

	calc=x*x+y*y+z*z;
	l = utilSqrt(calc);
	up=16-s;

	l=l>>up;

	return(l);
}

//float fl_Magnitude(FLVECTOR *vect)
//{
//	return sqrt(vect->vx*vect->vx + vect->vy*vect->vy + vect->vz*vect->vz);
//}

//could opt, but not needed?
fixed Magnitude2DF(FVECTOR *v)
{
	ULONG x,z,calc;
	ULONG l;
	ULONG num;
	ULONG up;
	ULONG s=0;

	num=0;

	x=labs(v->vx);
	num|=x;

	z=labs(v->vz);
	num|=z;

	while(num > 26754)
	{
		num = num>>2;
		s+=2;
	}
	x=(x)>>s;
	z=(z)>>s;

	calc=x*x+z*z;
	l = utilSqrt(calc);
	up=16-s;

	l=l>>up;

	return(l);
}

//	return FsqrtF(  FMul(vect->vx,vect->vx)
//				   +FMul(vect->vz,vect->vz) );


fixed Magnitude2DV(VECTOR *vect)
{
//	return ToFixed(sqrtf( (float)vect->vx * (float)vect->vx
//						 +(float)vect->vz * (float)vect->vz ));
	return Fsqrt( vect->vx * vect->vx
				 +vect->vz * vect->vz );
}

/*	--------------------------------------------------------------------------------
	Function 	: MakeUnit()
	Purpose 	: makes a vector unit magnitude
	Parameters 	: pointer to vector
	Returns 	: none
	Info 		:
*/


void MakeUnit(FVECTOR *vect)
{
//	int tb,s,d;
	int tb,d;
	long n;


//	long m = MagnitudeF(vect);
//bbopt - reuse some of these below!
	long m;



	ULONG x,y,z,calc;
	ULONG num;
	ULONG up;
//	ULONG s=0;
	int s=0;

//bbopt - don't set to 0 or do first or
//	num=0;
	x=Fabs(vect->vx);
//	num|=x;
	num=x;

	y=Fabs(vect->vy);
	num|=y;

	z=Fabs(vect->vz);
	num|=z;

//bbopt - use this for later
	n = num;

	while(num > 26754)
	{
		num = num>>2;
		s+=2;
	}
	x=(x)>>s;
	y=(y)>>s;
	z=(z)>>s;

	calc=x*x+y*y+z*z;
//bbopt
// 	l = utilSqrt(calc);
// 	up=16-s;
// 	m=l>>up;
	up=16-s;
	m = utilSqrt(calc) >> up;

	//end of using
	//x,y,z,calc,l,num,up,
	//s is used again, (was separate var before these 2 funcs were merged)




	if(m != 0)
	{
		// find top bit
//bbopt - worked out above
//		n=Fabs(vect->vx)|Fabs(vect->vy)|Fabs(vect->vz);

		for(tb=31; !(n&0x80000000); tb--)
		{
//bbopt - <<=
//			n=n<<1;
			n<<=1;
		}

		if(tb>18)
		{
			s=30-tb;
			d=tb-18;
			vect->vx= (vect->vx<<s)/(m>>d);
			vect->vy= (vect->vy<<s)/(m>>d);
			vect->vz= (vect->vz<<s)/(m>>d);
		}
		else
		{
			vect->vx= (vect->vx<<12)/m;
			vect->vy= (vect->vy<<12)/m;
			vect->vz= (vect->vz<<12)/m;
		}
	}
}


/*	--------------------------------------------------------------------------------
	Function 	: MakeUnit()
	Purpose 	: makes a vector unit magnitude
	Parameters 	: pointer to vector
	Returns 	: none
	Info 		:
*/
void MakeUnit2D(FVECTOR *vect)
{
	fixed m = Magnitude2DF(vect);

	if(m != ToFixed(0))
	{
		vect->vx = FDiv(vect->vx, m);
		vect->vz = FDiv(vect->vz, m);
	}
}

/*	--------------------------------------------------------------------------------
	Function 	: Aabs()
	Purpose 	: calculates a valid angle (makes sure it is between 0 and PI*2)
	Parameters 	: angle
	Returns 	: absolute angle
	Info 		:
*/
fixed Aabs(fixed angle)
{
//bb
//	angle = angle > ToFixed(2 * PI) ? (angle-ToFixed(2*PI)) : angle;
//	angle = (angle<ToFixed(0)) ? (angle+=ToFixed(2*PI)) : angle;
//	angle = (angle>2*fPI) ? (angle-2*fPI) : angle;
//	angle = (angle<0) 	  ? (angle+=2*fPI) : angle;
	angle = (angle>4096) ? (angle-4096) : angle;
	angle = (angle<0) 	  ? (angle+=4096) : angle;

	return angle;
}

/*	--------------------------------------------------------------------------------
	Function 	: arse
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
fixed SineWave(fixed freq,fixed phase,int base)
{
	fixed temp = ToFixed(base) + phase;
//bb
//	return ToFixed(sinf(FDiv(temp,freq)/4096.0F));
	return rsin(FDiv(temp,freq));
}

fixed SineWave2(fixed freq,fixed phase)
{
//bb
//	return ToFixed(sinf( FDiv(phase*2*PI,freq)/4096.0F ));
//	return rsin(FDiv( FMul(phase,fPI)*2, freq ));
	return rsin(FDiv( FMul(phase,fPI)*2, freq ));
}


void SetVectorLength(VECTOR *vect,fixed length)
{
	fixed m = FDiv(MagnitudeV(vect), length);

	if(m != ToFixed(0))
	{
		vect->vx = FDiv(ToFixed(vect->vx), m) >>12;
		vect->vy = FDiv(ToFixed(vect->vy), m) >>12;
		vect->vz = FDiv(ToFixed(vect->vz), m) >>12;
	}
}


//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//matrix functions follow


/*	--------------------------------------------------------------------------------
	Function 	: InitMatrixStack()
	Purpose 	: clears the matrix stack and resets the position pointer
	Parameters 	: none
	Returns 	: none
	Info 		:
*/
void InitMatrixStack()
{
}

/*	--------------------------------------------------------------------------------
	Function 	: InitMatrixStack()
	Purpose 	: clears the matrix stack and resets the position pointer
	Parameters 	: none
	Returns 	: none
	Info 		:
*/
void InitRMatrixStack()
{
}



/*	--------------------------------------------------------------------------------
	Function 	: SetMatrix()
	Purpose 	: copies one matrix into another
	Parameters 	: fixed*, fixed*
	Returns 	: none
	Info 		:
*/
/*	--------------------------------------------------------------------------------
	Function 	: LoadMatrix()
	Purpose 	: replaces the matrix at the top of the stack with this one 
	Parameters 	: fixed *
	Returns 	: none
	Info 		:
*/
void LoadMatrix(fixed *matrix)
{
// 	if(matrixStack.stackPosition < 0)
// 		matrixStack.stackPosition = 0;
// 
// 	SetMatrix(*(matrixStack.stack[matrixStack.stackPosition]), matrix);
}

/*	--------------------------------------------------------------------------------
	Function 	: PushMatrix()
	Purpose 	: pushes a matrix onto the stack
	Parameters 	: fixed *
	Returns 	: none
	Info 		:
*/

/*	--------------------------------------------------------------------------------
	Function 	: PopMatrix()
	Purpose 	: pops a matrix from the top of the stack
	Parameters 	: none
	Returns 	: none
	Info 		:
*/
/*void PopMatrix()
{
	if(matrixStack.stackPosition == -1)
	{
		dprintf"MatrixStack underflow\n"));
		Crash("POP MATRIX");
	}

	matrixStack.stackPosition--;

}
  */
/*	--------------------------------------------------------------------------------
	Function 	: ComputeResultMatrix()
	Purpose 	: multiplies all the matrices currently on the stack, puts result into .result
	Parameters 	: none
	Returns 	: none
	Info 		:
*/
void ComputeResultMatrix()
{
// 	int	x;
// 		
// 	SetMatrix(*matrixStack.result, *matrixStack.stack[matrixStack.stackPosition]);
// 	return;
// 	for(x = 1; x <= matrixStack.stackPosition; x++)
// 	{
// 		guMtxCatF(matrixStack.stack[x], matrixStack.result, matrixStack.result);
// 	}
// 

}


/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/

/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
//int lookforbug = -1;



// void RotateVectorByQuaternionFV(FVECTOR *result,VECTOR *vect,QUATERNION *q)
// {
// }

/*	--------------------------------------------------------------------------------
	Function 	: RotateVectorByQuaternion
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		: destination CAN be same as source
*/
// void RotateVectorByRotation(FVECTOR *result, FVECTOR *vect, QUATERNION *rot)
// {
// }


/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
// void NormalToQuaternion(QUATERNION *q, FVECTOR *normal)
// {
// }

/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
/*void CalculateQuatForPlane(ACTOR *actor,QUATERNION *qAim,PLANE *plane)
{
/*}*/

/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
/*
void CalculateQuatForPlane2(fixed yRot, QUATERNION *qAim, FVECTOR *normal)
{
}	  
*/
void CalculateQuatForPlane2(fixed yRot, IQUATERNION *qAim, FVECTOR *normal)
{

 	IQUATERNION tempRot,tempQ;
 	FVECTOR tempVect;
 	
/* 	if(normal->vy == 4096)
 	{
// 		vertQ.w = Aabs(ToFixed(2*PI)-yRot)/4096.0;// + PI);
 		vertQ.w = Aabs(4096-yRot);// + PI);
 		tempRot.w = 0;					//quat float
 		tempRot.x = 0;
 		tempRot.y = 4096;
 		tempRot.z = 0;
 	}
	else*/
 	if(normal->vy == -4096)
 	{
// 		vertQ.w = Aabs(ToFixed(2*PI)-yRot)/4096.0;// + PI);
 		vertQ.w = Aabs(4096-yRot);// + PI);
 		tempRot.w = 2048;					//quat float
 		tempRot.x = 4096;
 		tempRot.y = 0;
 		tempRot.z = 0;
 	}
 	else
 	{
 		vertQ.w = 0;	//yRot;
 		SetVector2DFF(&tempVect,normal);
 		tempVect.vy = 0;
 		MakeUnit2D(&tempVect);
 		CrossProductVertical((FVECTOR *)&tempRot,&tempVect);
 		tempRot.w = arccos(normal->vy);
 	}

 	fixedGetQuaternionFromRotation(qAim,&vertQ);
 	fixedGetQuaternionFromRotation(&tempQ,&tempRot);
 	fixedQuaternionMultiply(qAim,&tempQ,qAim);
}



#define QEPSILON	1
//#define QHALFPI		6434

//bbopt ?
void IQuatSlerp(IQUATERNION *src1, IQUATERNION *sp2, fixed t, IQUATERNION *dquat)
{
	fixed omega, cosom, sinom, sclp, sclq, pdist,ndist;
	fixed x,y,z,w;
	IQUATERNION	negs2, *src2;

	x = src1->x - sp2->x;
	y = src1->y - sp2->y;
	z = src1->z - sp2->z;
	w = src1->w - sp2->w;
	pdist = FMul(x,x) + FMul(y,y) + FMul(z,z) + FMul(w,w);

	x = src1->x + sp2->x;
	y = src1->y + sp2->y;
	z = src1->z + sp2->z;
	w = src1->w + sp2->w;
	ndist = FMul(x,x) + FMul(y,y) + FMul(z,z) + FMul(w,w);

	if (pdist < ndist)
	{
		// +ve arc is shorter, so interpolate along that
		src2 = sp2;
	}
	else
	{
		// -ve arc is shorter, so interpolate along that
		negs2.x = -sp2->x;
		negs2.y = -sp2->y;
		negs2.z = -sp2->z;
		negs2.w = -sp2->w;
		src2 = &negs2;
	}

	cosom = FMul(src1->x,src2->x) + FMul(src1->y,src2->y) + FMul(src1->z,src2->z) + FMul(src1->w,src2->w);

//	if ((((float)1.0)+cosom) > QEPSILON)
	if((4096+cosom) > QEPSILON)
	{
		if((4096-cosom) > QEPSILON)
		{
// 			omega = (float)acosBetter(cosom);
// 			sinom = (float)1.0/sinf(omega);
// 			sclp = sinf((1.0F-(t/4096.0F))*omega) * sinom;
// 			sclq = sinf((t/4096.0F)*omega)*sinom;
			omega = arccos(cosom);
			sinom = FDiv2(4096,rsin(omega));		// ????? Look at..........
			sclp = FMul( rsin(FMul(4096-t,omega)) , sinom );
			sclq = FMul( rsin(FMul(t,omega)) , sinom );
		}
		else
		{
//			sclp = (float)1.0-(t/4096.0F);
//			sclq = t/4096.0F;
			sclp = 4096-t;
			sclq = t;
		}

		dquat->x = FMul(sclp,src1->x) + FMul(sclq,src2->x);
		dquat->y = FMul(sclp,src1->y) + FMul(sclq,src2->y);
		dquat->z = FMul(sclp,src1->z) + FMul(sclq,src2->z);
		dquat->w = FMul(sclp,src1->w) + FMul(sclq,src2->w);
	}
	else
	{
		dquat->x = -src1->y;
		dquat->y = src1->x;
		dquat->z = -src1->w;
		dquat->w = src1->z;
//		sclp = sinf((1.0-(t/4096.0F))*QHALFPI);
//		sclq = sinf((t/4096.0F)*QHALFPI);
//bb	sclp = rsin(FMul(4096-t,1024));
//		sclq = rsin(FMul(t,1024));
		sclp = rsin( (4096-t)/4 );
		sclq = rsin( t/4 );

		dquat->x = FMul(sclp,src1->x) + FMul(sclq,dquat->x);
		dquat->y = FMul(sclp,src1->y) + FMul(sclq,dquat->y);
		dquat->z = FMul(sclp,src1->z) + FMul(sclq,dquat->z);
	}
}

void RotateVectorByQuaternionFF(FVECTOR *result, FVECTOR *vect, IQUATERNION *q)
{
	IQUATERNION rot;

	// ds - I'm sure there's a far more efficient way of doing this..
	fixedGetRotationFromQuaternion(&rot,q);
	fixedRotateVectorByRotation(result,vect,&rot);
}

// void BBTEST_RotVecByQuatFF(FVECTOR *result, FVECTOR *vect, IQUATERNION *q)
// {
// 	IQUATERNION rot;
// 
// //	printf("\nBBTEST_RotVecByQuatFF(%d %d %d, %d %d %d %d)\n", vect->vx, vect->vy, vect->vz,
// //															  q->x, q->y, q->z, q->w );
// 
// 	fixedGetRotationFromQuaternion(&rot,q);
// //	printf("rot   %d %d %d %d\n", rot.x, rot.y, rot.z, rot.w);
// //	fixedRotateVectorByRotation(result,vect,&rot);
// 	BBTEST_fixedRotVecByRot(result,vect,&rot);
// //	printf("result  %d %d %d\n", result->vx, result->vy, result->vz);
// }

void fixedGetRotationFromQuaternion(IQUATERNION *destQ, IQUATERNION *srcQ)
{
	fixed theta,sinThetaOver2,m;

	theta = 2 * arccos(srcQ->w);
	sinThetaOver2 = rsin(theta/2);
	
	destQ->w = theta;
	if(sinThetaOver2 != 0)
	{
		//bbopt - 1/sinThetaOver2 and FMul?
		destQ->x = FDiv(srcQ->x,sinThetaOver2);
		destQ->y = FDiv(srcQ->y,sinThetaOver2);
		destQ->z = FDiv(srcQ->z,sinThetaOver2);

		m = FsqrtF( FMul(destQ->x,destQ->x) + FMul(destQ->y,destQ->y) + FMul(destQ->z,destQ->z) );
		if(m != 0)
		{
			//bbopt - 1/m and FMul?
			destQ->x = FDiv(destQ->x,m);
			destQ->y = FDiv(destQ->y,m);
			destQ->z = FDiv(destQ->z,m);
		}
	}
	else
	{
		destQ->x = 0;
		destQ->y = 4096;
		destQ->z = 0;
	}
}







/* Convert quaternion to Euler angles (in radians). */
void Eul_FromQuat(SVECTOR* dest, IQUATERNION* q)
{
    MATRIXI M;
    fixed Nq = FMul(q->x,q->x) + FMul(q->y,q->y)
    		 + FMul(q->z,q->z) + FMul(q->w,q->w);
    fixed s = (Nq > 0) ? FDiv(8192,Nq) : 0;
    fixed xs = FMul(q->x,s),	ys = FMul(q->y,s),	zs = FMul(q->z,s);
    fixed wx = FMul(q->w,xs),	wy = FMul(q->w,ys),	wz = FMul(q->w,zs);
    fixed xx = FMul(q->x,xs),	xy = FMul(q->x,ys),	xz = FMul(q->x,zs);
    fixed yy = FMul(q->y,ys),	yz = FMul(q->y,zs),	zz = FMul(q->z,zs);
    
    M.matrix[0][0] = 4096 - (yy + zz);
    M.matrix[0][1] = xy - wz;
    M.matrix[0][2] = xz + wy;
    
    M.matrix[1][0] = xy + wz;
    M.matrix[1][1] = 4096 - (xx + zz);
    M.matrix[1][2] = yz - wx;
    
    M.matrix[2][0] = xz - wy;
    M.matrix[2][1] = yz + wx;
    M.matrix[2][2] = 4096 - (xx + yy);
    
    M.matrix[3][0]=M.matrix[3][1]=M.matrix[3][2]=
    M.matrix[0][3]=M.matrix[1][3]=M.matrix[2][3]=0;
    M.matrix[3][3]=4096;

    Eul_FromHMatrix(dest, &M);
}

void Eul_FromHMatrix(SVECTOR* dest, MATRIXI* M)
{
    int i,j,k,h,n,s,f;

    EulGetOrd(EulOrdZYZr,i,j,k,h,n,s,f);		//GOOD

    if(s==1)
    {
		fixed sy = utilSqrt(M->matrix[i][j] * M->matrix[i][j]
						   +M->matrix[i][k] * M->matrix[i][k]) >>4;
		if(sy > 1)
		{
		    dest->vx = utilCalcAngle(M->matrix[i][j], M->matrix[i][k]);
		    dest->vy = utilCalcAngle(sy, M->matrix[i][i]);
		    dest->vz = utilCalcAngle(M->matrix[j][i], -M->matrix[k][i]);
		}
		else
		{
		    dest->vx = utilCalcAngle(-M->matrix[j][k], M->matrix[j][j]);
		    dest->vy = utilCalcAngle(sy, M->matrix[i][i]);
		    dest->vz = 0;
			dest->vy +=2048;
		}
    }
    else
    {
		fixed cy = utilSqrt(M->matrix[i][i] * M->matrix[i][i]
						   +M->matrix[j][i] * M->matrix[j][i]) >>4;
		if(cy > 1)
		{
		    dest->vx = utilCalcAngle(M->matrix[k][j], M->matrix[k][k]);
		    dest->vy = utilCalcAngle(-M->matrix[k][i], cy);
		    dest->vz = utilCalcAngle(M->matrix[j][i], M->matrix[i][i]);
		}
		else
		{
		    dest->vx = utilCalcAngle(-M->matrix[j][k], M->matrix[j][j]);
		    dest->vy = utilCalcAngle(-M->matrix[k][i], cy);
		    dest->vz = 0;
		}
    }
    if(n==EulParOdd)
    {
    	dest->vx = -dest->vx;
    	dest->vy = -dest->vy;
    	dest->vz = -dest->vz;
    }

    if(f==EulFrmR)
    {
    	short t = dest->vx;
    	dest->vx = dest->vz;
    	dest->vz = t;
    }
}

// Construct quaternion from Euler angles (in radians). 
//Quat Eul_ToQuat(EulerAngles ea)
void Eul_ToQuat(IQUATERNION* qu, SVECTOR* src)
{
	SVECTOR v = *src;	//bb - work on copy
    fixed a[3], ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;
    EulGetOrd(EulOrdZYZr,i,j,k,h,n,s,f);

    if(f==EulFrmR)
    {
    	short t = v.vx;
    	v.vx = v.vz;
    	v.vz = t;
    }

    if(n==EulParOdd)
    	v.vy = -v.vy;

    ti = v.vx>>1; tj = v.vy>>1; th = v.vz>>1;
    ci = rcos(ti);  cj = rcos(tj);  ch = rcos(th);
    si = rsin(ti);  sj = rsin(tj);  sh = rsin(th);
    cc = FMul(ci,ch); cs = FMul(ci,sh); sc = FMul(si,ch); ss = FMul(si,sh);
    if (s==EulRepYes)
    {
		a[i] = FMul(cj, (cs+sc));	// Could speed up with 
		a[j] = FMul(sj, (cc+ss));	// trig identities. 
		a[k] = FMul(sj, (cs-sc));
		qu->w = FMul(cj, (cc-ss));
    }
    else
    {
		a[i] = FMul(cj,sc) - FMul(sj,cs);
		a[j] = FMul(cj,ss) + FMul(sj,cc);
		a[k] = FMul(cj,cs) - FMul(sj,sc);
		qu->w = FMul(cj,cc) + FMul(sj,ss);
    }

    if(n==EulParOdd)
    	a[j] = -a[j];

    qu->x = a[X];
    qu->y = a[Y];
    qu->z = a[Z];
}

void RotateVectorByXZRotation(FVECTOR *result, FVECTOR *vect, IQUATERNION *rot)
{
	fixed m,n,sinTheta,cosTheta;
	FVECTOR mVec,pVec,vVec;

	m = FMul(vect->vx, rot->x);
	m += FMul(vect->vz, rot->z);

	mVec.vx = FMul(m, rot->x);
	mVec.vy = 0;
	mVec.vz = FMul(m, rot->z);

	SubVectorFFF(&pVec,vect,&mVec);

	vVec.vx = -FMul(rot->z, pVec.vx);
	vVec.vy =  FMul(rot->z, pVec.vy) - FMul(rot->x, pVec.vz);
	vVec.vz =  FMul(rot->x, pVec.vz);
	
	//CrossProduct(&vVec,(VECTOR *)rot,&pVec);

	m = MagnitudeF(&pVec);

	if(m == 0)
		SetVectorFF(result,vect);
	else
	{
		n = MagnitudeF(&vVec);
		if(n)
		{
//bb
//			m /= n;
			m = FDiv2(m,n);
			ScaleVectorFF(&vVec,m);
		}

//		cosTheta = cosf(rot->w);
//		sinTheta = sinf(rot->w);
		cosTheta = rcos(rot->w);
		sinTheta = rsin(rot->w);

		result->vx = mVec.vx + FMul(cosTheta,pVec.vx) + FMul(sinTheta,vVec.vx);
		result->vy = FMul(cosTheta,pVec.vy) + FMul(sinTheta,vVec.vy);
		result->vz = mVec.vz + FMul(cosTheta,pVec.vz) + FMul(sinTheta,vVec.vz);
	}
}

void GetQuaternionFromXZRotation(IQUATERNION *destQ, IQUATERNION *srcQ)
{
	fixed thetaOver2;
	fixed sinThetaOver2;

	thetaOver2 = srcQ->w>>1;
	sinThetaOver2 = rsin(thetaOver2);

	destQ->w = rcos(thetaOver2);
	destQ->x = FMul(sinThetaOver2,srcQ->x);
	destQ->y = 0;
	destQ->z = FMul(sinThetaOver2,srcQ->z);
}

void Init_BB_AcosTable()
{
// 	int i;
// 	for(i=-4096; i<=4096; i++)
// 	{	//					    0->1		  rad 2 psx
// 		bb_acostable[i+4096] = (acos(i/4096.0) *4096.0)/(44.0/7.0);
// 
// 		if(!(i%64))
// 			utilPrintf("%d\n",i);
// 	}
}

void NormalToQuaternion(IQUATERNION *q, SVECTOR *normal)
{
	if(normal->vx)
	{
//		q->z = FsqrtF( FDiv(4096, (4096 + FDiv(FMul(normal->vz*normal->vz),FMul(normal->vx*normal->vx)))) );
//		utilPrintf("%lu : %lu\n", normal->vz, normal->vx);
//		q->z = FsqrtF( FDiv(4096, (4096 + FDiv(FMul(normal->vz,normal->vz),FMul(normal->vx,normal->vx)))));
		q->z = FsqrtF( FDiv2(4096, (4096 + FDiv2(FMul(normal->vz,normal->vz),FMul(normal->vx,normal->vx)))));
		if(normal->vx > 0)
			q->z = -q->z;
		q->x = -(normal->vz*q->z) / normal->vx;
		q->w = arccos(normal->vy)/2;

		q->x *= rsin(q->w);
		q->z *= rsin(q->w);
		q->w = rcos(q->w);
	}
	else if(normal->vz)
	{
		q->z = 0;
		q->w = arccos(normal->vy)/2;

		if(normal->vz > 0)
			q->x = rsin(q->w);
		else
			q->x = -rsin(q->w);
		q->w = rcos(q->w);
	}
	else if(normal->vy > 0)
	{
		q->x = 0;
		q->z = 0;
		q->w = 4096;
	}
	else
	{
		q->x = 0;
		q->z = -4096;
		q->w = 0;
	}
	q->y = 0;
}

/*	-------------------------------------------------------------------------------------
	Function:	NormaliseQuaternion

	Based on MakeUnit only in 4 dimensions!
*/
void NormaliseQuaternion(IQUATERNION *vect)
{
	int tb,d;
	long n;
	long m;

	ULONG x,y,z,w,calc;
	ULONG num;
	ULONG up;
	int s=0;

	x=Fabs(vect->x);
	num=x;

	y=Fabs(vect->y);
	num|=y;

	z=Fabs(vect->z);
	num|=z;

	w=Fabs(vect->w);
	num|=w;

	n = num;

	while(num > 26754)
	{
		num = num>>2;
		s+=2;
	}
	x=(x)>>s;
	y=(y)>>s;
	z=(z)>>s;

	calc=x*x+y*y+z*z+w*w;
	up=16-s;
	m = utilSqrt(calc) >> up;

	if(m != 0)
	{
		for(tb=31; !(n&0x80000000); tb--)
		{
			n<<=1;
		}

		if(tb>18)
		{
			s=30-tb;
			d=tb-18;
			vect->x= (vect->x<<s)/(m>>d);
			vect->y= (vect->y<<s)/(m>>d);
			vect->z= (vect->z<<s)/(m>>d);
			vect->w= (vect->w<<s)/(m>>d);
		}
		else
		{
			vect->x= (vect->x<<12)/m;
			vect->y= (vect->y<<12)/m;
			vect->z= (vect->z<<12)/m;
			vect->w= (vect->w<<12)/m;
		}
	}
}

/*void NormaliseQuaternion(IQUATERNION* q)
{
	fixed a_sqr;

//	MakeUnit((FVECTOR*)&q->x);
	
	a_sqr = (q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);

	if (a_sqr)
	{
		//if (a_sqr < (65536-256) || a_sqr > (65536+256))	// friendly threshold
		//{
			a_sqr = utilSqrt(a_sqr)>>12;

			q->x = FDiv(q->x, a_sqr);
			q->y = FDiv(q->y, a_sqr);
			q->z = FDiv(q->z, a_sqr);
			q->w = FDiv(q->w, a_sqr);
		//}
	}
	else
	{
		q->w = 4096;
	}


	if (q->w < -4096)
		q->w = -4096;
	else if (q->w > 4096)
		q->w = 4096;

	if (a_sqr)
	{
		mg = FDiv((q->w<<12),(utilSqrt(a_sqr)));

		q->x = (mg * q->x)>>12;
		q->y = (mg * q->y)>>12;
		q->z = (mg * q->z)>>12;
	}

	//drift = ((q->w*q->w)>>12 + a_sqr) - 4096;
	//q->w = FsqrtF(4096 - a_sqr);
}
*/


#ifdef PSX_VERSION
void QuatToPSXMatrix(IQUATERNION* q, MATRIX* M)
{
//	quaternionGetMatrix(q, M);

    
	fixed Nq = FMul(q->x,q->x) + FMul(q->y,q->y)
    		 + FMul(q->z,q->z) + FMul(q->w,q->w);
    fixed s = (Nq > 0) ? FDiv(8192,Nq) : 0;
    fixed xs = FMul(q->x,s),	ys = FMul(q->y,s),	zs = FMul(q->z,s);
    fixed wx = FMul(q->w,xs),	wy = FMul(q->w,ys),	wz = FMul(q->w,zs);
    fixed xx = FMul(q->x,xs),	xy = FMul(q->x,ys),	xz = FMul(q->x,zs);
    fixed yy = FMul(q->y,ys),	yz = FMul(q->y,zs),	zz = FMul(q->z,zs);
 
	M->m[0][0] = ((zz + yy) - 4096);
    M->m[1][0] = -(xy + wz);
    M->m[2][0] = xz - wy;
    
    M->m[0][1] = (xy - wz);
    M->m[1][1] = (4096 - (zz + xx));
    M->m[2][1] = -(yz + wx);
    
    M->m[0][2] = xz + wy;
    M->m[1][2] = yz - wx;
    M->m[2][2] = ((yy+xx) - 4096);

    M->t[0]=
    M->t[1]=
    M->t[2]=0;
}
#endif

/*	--------------------------------------------------------------------------------
	Function 	: 
	Purpose 	: 
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
BOOL SlideVectorToVectorFF(FVECTOR *vect,FVECTOR *dest,fixed speed)
{
	FVECTOR tempVect;
	fixed m;

	speed = FMul(speed,gameSpeed);

	SubVectorFFF(&tempVect,dest,vect);
	m = MagnitudeF(&tempVect);
	if(m < speed)
	{
		SetVectorFF(vect,dest);
		return TRUE;
	}
	else
	{
		if (m == 0)
		{
			SetVectorFF(vect, dest);
			return TRUE;
		}

		m = FDiv(speed,m);
		ScaleVectorFF(&tempVect,m);
		AddToVectorFF(vect,&tempVect);
//		AddToOneScaledVector(vect,&tempVect,FDiv(speed,m));
		return FALSE;
	}
}

#ifdef PC_VERSION
void TransformPoint(SVECTOR *pos,short *scrX,short *scrY)
{
	MDX_VECTOR p,scr;

	SetVectorRS(&p, pos);
	ScaleVector(&p,0.1f);
	XfmPoint(&scr,&p,NULL);
	*scrX = (short)((scr.vx*4096.0f)/rXRes);
	*scrY = (short)((scr.vy*4096.0f)/rYRes);
}
#else
void TransformPoint(SVECTOR *pos,short *scrX,short *scrY)
{
	SVECTOR tempSvect,m;

	tempSvect.vx = -pos->vx;
	tempSvect.vy = -pos->vy;
	tempSvect.vz = pos->vz;

	gte_ldv0(&tempSvect);
	gte_rtps();
	gte_stsxy((long *)&m.vx);

#ifdef DREAMCAST_VERSION
	*scrX = (m.vx*4096.0)/640;
	*scrY = (m.vy*4096.0)/480;
#else
	*scrX = (m.vx + 224)*8;
	*scrY = (m.vy + 120)*(17 - PALMODE);
#endif
}
#endif
