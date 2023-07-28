/************************************************************************************
	PSX CORE (c) 1999 ISL

	quatern.h:		Quaternion rotation routines

************************************************************************************/

#ifndef _QUATERN_H_
#define _QUATERN_H_


typedef struct
{
	LONG	x, y, z, w;
}IQUATERNION;

typedef struct
{
	LONG	matrix[4][4];
}MATRIXI;


/**************************************************************************
	FUNCTION:	quaternionSlerp()
	PURPOSE:	interpolate between two quaternions
	PARAMETERS:	quaternion #1, quaternion #2, interpolant (normalised), dest quaternion
	RETURNS:	
**************************************************************************/

//void quaternionSlerp(IQUATERNION *src1, IQUATERNION *sp2, ULONG t, IQUATERNION *dquat);
//void quatSlerp2(IQUATERNION *from, IQUATERNION  *to, ULONG t, IQUATERNION *res);


/**************************************************************************
	FUNCTION:	quaternionGetMatrix()
	PURPOSE:	Convert quarternion to rotation matrix
	PARAMETERS:	Source quaternion, dest matrix
	RETURNS:	
**************************************************************************/

void quaternionGetMatrix(IQUATERNION *squat, MATRIX *dmatrix);

void quaternionSlerpMatrix(IQUATERNION *src1, IQUATERNION *sp2, ULONG t,MATRIX *dmatrix);



#endif
