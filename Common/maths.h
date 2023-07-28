#ifndef __MATHS_H
#define __MATHS_H

#include "types.h"

#define Fabs(a) ((a) > 0 ? (a) : -(a))

//#define PI			(3.141592654)
//#define PI_OVER_2	(3.141592654/2.0F)
//#define PI_OVER_4	(3.141592654/4.0F)
//#define PI2			(3.141592654*2.0F)

#define fPI			(12868)
#define fPI_OVER_2	(6434)
#define fPI_OVER_4	(3217)
#define fPI2		(25736)




#define DotProduct2D(v1,v2)	((((v1)->vx)*((v2)->vx)) + (((v1)->vz)*((v2)->vz)))

#define ScaleVector(v1,scale) {(v1)->vx *= scale;(v1)->vy *= scale;(v1)->vz *= scale;}
#define ScaleVector2D(v1,scale) {(v1)->vx *= scale;(v1)->vz *= scale;}
#define ScaleVectorFF(v1,scale) { (v1)->vx = FMul((v1)->vx, scale),\
								  (v1)->vy = FMul((v1)->vy, scale),\
								  (v1)->vz = FMul((v1)->vz, scale); }

#define ScaleVectorSF(v1,scale) { (v1)->vx = (FMul(((v1)->vx<<12), scale))>>12,\
								  (v1)->vy = (FMul(((v1)->vy<<12), scale))>>12,\
								  (v1)->vz = (FMul(((v1)->vz<<12), scale))>>12; }


#define SetMatrix(m1,m2) (*((MATRIX *)m1)) = (*((MATRIX *)m2))
#define SetQuaternion(q1,q2) (*(q1)) = (*(q2))

#define ZeroVector(vect) (vect)->vx = (vect)->vy = (vect)->vz = 0
#define ZeroVectorF(vect) {(vect)->vx=0; (vect)->vy=0; (vect)->vz=0;}
#define ZeroVector2D(vect) SetVector2D(vect,&zero)
#define ZeroQuaternion(quat) SetQuaternion(quat,&zeroQuat)

#define LOAD_VECTOR(p, x, y, z) { p.vx = (x); p.vy = (y); p.vz = (z);}
#define LOAD_BYTEVECTOR(p, x, y, z) {p.vx = (x); p.vy = (y); p.vz = (z);}
#define LOAD_SHORTVECTOR(p, x, y, z) {p.vx = (x); p.vy = (y); p.vz = (z);}
#define LOAD_SHORT2DVECTOR(p, x, y) {p.vx = (x); p.vy = (y);}

#define Bound(a,x,b) Min(b,Max(a,x))

#define acosBetter(x)	(acos(x))

#define PushRMatrix(matrix)	{ guMtxCatF((matrix), rMatrixStack.stack[rMatrixStack.stackPosition++], rMatrixStack.stack[rMatrixStack.stackPosition]); rMatrixStack.stackPosition++; } 
#define PopRMatrix() rMatrixStack.stackPosition--

#define LinearInterp(result,from,to,interp) {(result)->vx = (from)->vx + (((to)->vx - (from)->vx) * interp);(result)->vy = (from)->vy + (((to)->vy - (from)->vy) * interp);(result)->vz = (from)->vz + (((to)->vz - (from)->vz) * interp);}
#define LinearInterpFSSF(result,from,to,interp) {(result)->vx = ToFixed((from)->vx) + (FMul(ToFixed((to)->vx - (from)->vx),(interp)));(result)->vy = ToFixed((from)->vy) + (FMul(ToFixed((to)->vy - (from)->vy),(interp)));(result)->vz = ToFixed((from)->vz) + (FMul(ToFixed((to)->vz - (from)->vz),(interp)));}

#define MAX_STACK_SIZE	16


#ifdef __cplusplus
extern "C" {
#endif


extern short bb_acostable[4097];	//0->4096 inclusive. Stores PSX angles
//extern short bb_acostable[8193];

extern VECTOR zero;
extern IQUATERNION vertQ;
extern FVECTOR upVec;
extern FVECTOR inVec;
extern FVECTOR outVec;
extern FVECTOR rightVec;
extern FVECTOR oneVec;




BOOL IntervalsOverlap(fixed lo1,fixed hi1,fixed lo2,fixed hi2);
int Random(ULONG val);
void SeedRandom();
fixed FindShortestAngle(fixed val1, fixed val2);
fixed FindShortestAngleSigned(fixed ang1,fixed ang2);
fixed Aabs(fixed angle);
void RotateVector2D(FVECTOR *dest, FVECTOR *source, fixed angle);
void AddScaledVector(VECTOR *result,VECTOR *operand1,fixed scale1,VECTOR *operand2,fixed scale2);
void RotateVector2DX(FVECTOR *dest, FVECTOR *source, fixed angle);
BOOL IsPointWithinBox(BOX *box, VECTOR *pos);
BOOL IsPointWithinBoxPlusHeight(BOX *box, VECTOR *pos,fixed height);
void FlipVector(VECTOR *vector);
void FlipVector(VECTOR *vector);
fixed SineWave(fixed freq,fixed phase,int base);
fixed SineWave2(fixed freq,fixed phase);
BOOL SphereIntersectsBox(BOX *box,VECTOR *pos,fixed radius);
BOOL IsPointWithinBox2D(BOX *box, VECTOR *pos);

void CalculateQuatForPlane2(fixed yRot, IQUATERNION *qAim, FVECTOR *normal);

void InitMatrixStack();
void ComputeResultMatrix();
void LoadMatrix(fixed *matrix);
void FlipVector2D(VECTOR *vector);

//fixed MagnitudeV(VECTOR *vect);
#define MagnitudeV(v) Fsqrt((v)->vx*(v)->vx + (v)->vy*(v)->vy + (v)->vz*(v)->vz)

fixed MagnitudeS(SVECTOR *vect);
fixed MagnitudeF(FVECTOR *v);
fixed Magnitude2DF(FVECTOR *vect);
fixed Magnitude2DV(VECTOR *vect);
void MakeUnit(FVECTOR *vect);
void MakeUnit2DF(FVECTOR *vect);
void SetVectorLength(VECTOR *vect,fixed length);

LONG arccos(LONG cos);
void fixedRotateVectorByRotation(FVECTOR *result, FVECTOR *vect, IQUATERNION *rot);
void fixedRotateVectorByRotationS(FVECTOR *result, SVECTOR *vect, IQUATERNION *rot);

void fixedGetQuaternionFromRotation(IQUATERNION *destQ,IQUATERNION *srcQ);
void fixedQuaternionMultiply(IQUATERNION *dest,IQUATERNION *src1,IQUATERNION *src2);

void IQuatSlerp(IQUATERNION *src1, IQUATERNION *sp2, fixed t, IQUATERNION *dquat);
void RotateVectorByQuaternionFF(FVECTOR *result, FVECTOR *vect, IQUATERNION *q);
void fixedGetRotationFromQuaternion(IQUATERNION *destQ, IQUATERNION *srcQ);
void Eul_FromQuat(SVECTOR* dest, IQUATERNION* q);
void Eul_FromHMatrix(SVECTOR* dest, MATRIXI* M);
void Eul_ToQuat(IQUATERNION* q, SVECTOR* v);


void TransformPoint(SVECTOR *pos,short *scrX,short *scrY);




//void SubVectorVVV(VECTOR  *result, VECTOR  *operand1, VECTOR  *operand2);
//void SubVectorVSV(VECTOR  *result, SVECTOR *operand1, VECTOR  *operand2);
//void SubVectorVSS(VECTOR  *result, SVECTOR *operand1, SVECTOR *operand2);
//
//void SubVectorSSS(SVECTOR *result, SVECTOR *operand1, SVECTOR *operand2);
//void SubVectorSSF(SVECTOR *result, SVECTOR *operand1, FVECTOR *operand2);

//void SubVectorFSS(FVECTOR *result, SVECTOR *operand1, SVECTOR *operand2);
//void SubVectorFSF(FVECTOR *result, SVECTOR *operand1, FVECTOR *operand2);
//void SubVectorFFS(FVECTOR *result, FVECTOR *operand1, SVECTOR *operand2);
//void SubVectorFFF(FVECTOR *result, FVECTOR *operand1, FVECTOR *operand2);

//void SetVectorVV(VECTOR*    v1, VECTOR*   v2);
//void SetVectorVF(VECTOR*    v1, FVECTOR*  v2);
//void SetVectorVS(VECTOR*    v1, SVECTOR*  v2);
//
//void SetVectorSV(SVECTOR*   v1, VECTOR*   v2);
//void SetVectorSS(SVECTOR*   v1, SVECTOR*  v2);
//void SetVectorSF(SVECTOR*   v1, FVECTOR*  v2);
//
//void SetVectorFS(FVECTOR*   v1, SVECTOR*  v2);
//void SetVectorFF(FVECTOR*   v1, FVECTOR*  v2);
//void SetVectorFV(FVECTOR*   v1, VECTOR*   v2);

// void AddVectorSFS(SVECTOR *dest, FVECTOR* v1, SVECTOR* v2);
// void AddVectorSSF(SVECTOR *dest, SVECTOR* v1, FVECTOR* v2);
// void AddVectorSSS(SVECTOR *dest, SVECTOR* v1, SVECTOR* v2);
// void AddVectorSFF(SVECTOR *dest, FVECTOR* v1, FVECTOR* v2);
// void AddVectorFFS(FVECTOR *dest, FVECTOR* v1, SVECTOR* v2);
// void AddVectorVSF(VECTOR *dest, SVECTOR* v1, FVECTOR* v2);

//void CrossProduct(FVECTOR *result, FVECTOR *operand1, FVECTOR *operand2);
//void CrossProductFFF(FVECTOR *result, FVECTOR *operand1, FVECTOR *operand2);

//void CrossProductVertical(FVECTOR *result, FVECTOR *operand2);

//fixed DotProduct(FVECTOR* v1, FVECTOR* v2);
//fixed DotProductFF(FVECTOR *v1,  FVECTOR *v2);

// void SetVector2D(VECTOR *v1,VECTOR *v2);
// void SetVector2DFF(FVECTOR *v1,FVECTOR *v2);

// void SubVector2D(VECTOR *result,VECTOR *operand1,VECTOR *operand2);
// void SubFromVector2D(VECTOR *result,VECTOR *operand);
// void SubFromVectorVS(VECTOR  *result, SVECTOR *v1);
// void SubFromVectorSS(SVECTOR *result, SVECTOR *v1);
// void SubFromVectorSF(SVECTOR *result, FVECTOR *v1);
// void SubFromVectorFF(FVECTOR *result, FVECTOR *v1);
// void SubVector2DSSS(SVECTOR *result, SVECTOR *operand1, SVECTOR *operand2);

// void AddToVectorVS(VECTOR  *res, SVECTOR *v1);
// void AddToVectorSS(SVECTOR *res, SVECTOR *v1);
// void AddToVectorFF(FVECTOR *res, FVECTOR *v1);
// void AddToVectorFS(FVECTOR *res, SVECTOR *v1);
// void AddToVectorFV(FVECTOR *res, VECTOR *v1);
// void AddToVectorSF(SVECTOR *res, FVECTOR *v1);
// void AddVector2D(VECTOR *result,VECTOR *operand1,VECTOR *operand2);
// void AddScaledVector2D(VECTOR *result,VECTOR *operand1,fixed scale1,VECTOR *operand2,fixed scale2);
// void AddOneScaledVector(VECTOR *result,VECTOR *operand1,VECTOR *operand2,fixed scale2);
// void AddToVector2D(VECTOR *result,VECTOR *operand);
// void AddToOneScaledVector(VECTOR *result,VECTOR *operand2,fixed scale2);
// void AddToOneScaledVector2D(VECTOR *result,VECTOR *operand2,fixed scale2);
// void AddOneScaledVector2D(VECTOR *result,VECTOR *operand1,VECTOR *operand2,fixed scale2);

fixed DistanceBetweenPointsVV(VECTOR *v1, VECTOR *v2);
fixed DistanceBetweenPointsSS(SVECTOR *v1, SVECTOR *v2);
fixed DistanceBetweenPointsSV(SVECTOR *v1, VECTOR *v2);
fixed DistanceBetweenPointsFS(FVECTOR *v1, SVECTOR *v2);
fixed DistanceBetweenPointsSF(SVECTOR *v1, FVECTOR *v2);
fixed DistanceBetweenPoints2D(VECTOR *v1, VECTOR *v2);
fixed DistanceBetweenPointsFF(FVECTOR *v1, FVECTOR *v2);

BOOL SlideVectorToVectorFF(FVECTOR *vect,FVECTOR *dest,fixed speed);

long DistanceBetweenPointsSqrSS(SVECTOR *v1, SVECTOR *v2);
long DistanceBetweenPointsSqrFS(FVECTOR *v1, SVECTOR *v2);

void RotateVectorByXZRotation(FVECTOR *result, FVECTOR *vect, IQUATERNION *rot);
void GetQuaternionFromXZRotation(IQUATERNION *destQ, IQUATERNION *srcQ);

void NormaliseQuaternion(IQUATERNION *quat);	// normalise to a unit quaternion

void Init_BB_AcosTable(void);
void NormalToQuaternion(IQUATERNION *q, SVECTOR *normal);

#ifdef PSX_VERSION
#ifndef DREAMCAST_VERSION
void QuatToPSXMatrix(IQUATERNION* q, MATRIX* m);
#endif
#endif



#define SubVectorVVV(result, operand1, operand2)   (result)->vx = (operand1)->vx - (operand2)->vx,\
												   (result)->vy = (operand1)->vy - (operand2)->vy,\
												   (result)->vz = (operand1)->vz - (operand2)->vz

#define SubVectorVSV(result, operand1, operand2)   (result)->vx = (operand1)->vx - (operand2)->vx,\
												   (result)->vy = (operand1)->vy - (operand2)->vy,\
												   (result)->vz = (operand1)->vz - (operand2)->vz

#define SubVectorVSS(result, operand1, operand2)   (result)->vx = (operand1)->vx - (operand2)->vx,\
												   (result)->vy = (operand1)->vy - (operand2)->vy,\
												   (result)->vz = (operand1)->vz - (operand2)->vz

#define SubVectorSSS(result, operand1, operand2)   (result)->vx = (operand1)->vx - (operand2)->vx,\
												   (result)->vy = (operand1)->vy - (operand2)->vy,\
												   (result)->vz = (operand1)->vz - (operand2)->vz

#define SubVectorSSF(result, operand1, operand2)   (result)->vx = (operand1)->vx - ((operand2)->vx>>12),\
												   (result)->vy = (operand1)->vy - ((operand2)->vy>>12),\
												   (result)->vz = (operand1)->vz - ((operand2)->vz>>12)

#define SubVectorFSS(result, operand1, operand2)   (result)->vx = ((operand1)->vx - (operand2)->vx)<<12,\
												   (result)->vy = ((operand1)->vy - (operand2)->vy)<<12,\
												   (result)->vz = ((operand1)->vz - (operand2)->vz)<<12 

#define SubVectorFSF(result, operand1, operand2)   (result)->vx = ((operand1)->vx<<12) - (operand2)->vx,\
												   (result)->vy = ((operand1)->vy<<12) - (operand2)->vy,\
												   (result)->vz = ((operand1)->vz<<12) - (operand2)->vz

#define SubVectorFFS(result, operand1, operand2)   (result)->vx = (operand1)->vx - ((operand2)->vx<<12),\
												   (result)->vy = (operand1)->vy - ((operand2)->vy<<12),\
												   (result)->vz = (operand1)->vz - ((operand2)->vz<<12)

#define SubVectorFFF(result, operand1, operand2)   (result)->vx = (operand1)->vx - (operand2)->vx,\
												   (result)->vy = (operand1)->vy - (operand2)->vy,\
												   (result)->vz = (operand1)->vz - (operand2)->vz

#define SubVector2D(result, operand1, operand2)		(result)->vx=(operand1)->vx-(operand2)->vx,\
													(result)->vz=(operand1)->vz-(operand2)->vz

#define SubVector2DSSS(result, operand1, operand2)	(result)->vx=(operand1)->vx-(operand2)->vx,\
													(result)->vz=(operand1)->vz-(operand2)->vz

#define SubFromVector2D(result, operand)	(result)->vx-=(operand)->vx,\
											(result)->vz-=(operand)->vz

#define SubFromVectorVS(result, v1) 		(result)->vx-=(v1)->vx,\
											(result)->vy-=(v1)->vy,\
											(result)->vz-=(v1)->vz

#define SubFromVectorSS(result, v1)			(result)->vx-=(v1)->vx,\
											(result)->vy-=(v1)->vy,\
											(result)->vz-=(v1)->vz

#define SubFromVectorSF(result, v1)			(result)->vx-=(short)((v1)->vx>>12),\
											(result)->vy-=(short)((v1)->vy>>12),\
											(result)->vz-=(short)((v1)->vz>>12)

#define SubFromVectorFS(result, v1)			(result)->vx-=((fixed)(v1)->vx)<<12,\
											(result)->vy-=((fixed)(v1)->vy)<<12,\
											(result)->vz-=((fixed)(v1)->vz)<<12

#define SubFromVectorFF(result, v1)			(result)->vx-=(v1)->vx,\
											(result)->vy-=(v1)->vy,\
											(result)->vz-=(v1)->vz


#define SetVectorVS(v1, v2)						   (v1)->vx = (v2)->vx,\
												   (v1)->vy = (v2)->vy,\
												   (v1)->vz = (v2)->vz

#define SetVectorSV(v1, v2)						   (v1)->vx = (short)(v2)->vx,\
												   (v1)->vy = (short)(v2)->vy,\
												   (v1)->vz = (short)(v2)->vz

#define SetVectorSS(v1, v2)						   (v1)->vx = (v2)->vx,\
												   (v1)->vy = (v2)->vy,\
												   (v1)->vz = (v2)->vz

#define SetVectorSF(v1, v2)						   (v1)->vx = (short)((v2)->vx>>12),\
												   (v1)->vy = (short)((v2)->vy>>12),\
												   (v1)->vz = (short)((v2)->vz>>12)

#define SetVectorFS(v1, v2)						   (v1)->vx = (v2)->vx<<12,\
												   (v1)->vy = (v2)->vy<<12,\
												   (v1)->vz = (v2)->vz<<12

#define NewSetVectorFS(v1, v2)	(v1)->vx = (v2)->vx,\
												   (v1)->vy = (v2)->vy,\
												   (v1)->vz = (v2)->vz

#define SetVectorFF(v1, v2)						   (v1)->vx = (v2)->vx,\
												   (v1)->vy = (v2)->vy,\
												   (v1)->vz = (v2)->vz

#define SetVectorFV(v1, v2)						   (v1)->vx = (v2)->vx<<12,\
												   (v1)->vy = (v2)->vy<<12,\
												   (v1)->vz = (v2)->vz<<12


#define AddVectorSSS(dest, v1, v2)				   (dest)->vx = (v1)->vx + (v2)->vx,\
												   (dest)->vy = (v1)->vy + (v2)->vy,\
												   (dest)->vz = (v1)->vz + (v2)->vz

#define AddVectorSFS(dest, v1, v2)				   (dest)->vx = ((v1)->vx>>12) + (v2)->vx,\
												   (dest)->vy = ((v1)->vy>>12) + (v2)->vy,\
												   (dest)->vz = ((v1)->vz>>12) + (v2)->vz

#define AddVectorSSF(dest, v1, v2)				   (dest)->vx = (v1)->vx + ((v2)->vx>>12),\
												   (dest)->vy = (v1)->vy + ((v2)->vy>>12),\
												   (dest)->vz = (v1)->vz + ((v2)->vz>>12)

#define AddVectorSFF(dest, v1, v2)				   (dest)->vx = ((v1)->vx + (v2)->vx)>>12,\
												   (dest)->vy = ((v1)->vy + (v2)->vy)>>12,\
												   (dest)->vz = ((v1)->vz + (v2)->vz)>>12

#define AddVectorFFS(dest, v1, v2)				   (dest)->vx = (v1)->vx + ((v2)->vx<<12),\
												   (dest)->vy = (v1)->vy + ((v2)->vy<<12),\
												   (dest)->vz = (v1)->vz + ((v2)->vz<<12)

#define AddVectorFFF(dest, v1, v2)				   (dest)->vx = (v1)->vx + (v2)->vx,\
												   (dest)->vy = (v1)->vy + (v2)->vy,\
												   (dest)->vz = (v1)->vz + (v2)->vz

#define AddVectorVSF(dest, v1, v2)				   (dest)->vx = (v1)->vx + ((v2)->vx>>12),\
												   (dest)->vy = (v1)->vy + ((v2)->vy>>12),\
												   (dest)->vz = (v1)->vz + ((v2)->vz>>12)


#define CrossProductFFF(result, operand1, operand2) (result)->vx = FMul((operand1)->vy,(operand2)->vz) - FMul((operand1)->vz,(operand2)->vy),\
													 (result)->vy = FMul((operand1)->vz,(operand2)->vx) - FMul((operand1)->vx,(operand2)->vz),\
													 (result)->vz = FMul((operand1)->vx,(operand2)->vy) - FMul((operand1)->vy,(operand2)->vx)

#define CrossProductVertical(result, operand2)	(result)->vx =  (operand2)->vz,\
												(result)->vy = 0,\
												(result)->vz = -(operand2)->vx


#define DotProductFF(v1, v2) (FMul((v1)->vx,(v2)->vx) + FMul((v1)->vy,(v2)->vy) + FMul((v1)->vz,(v2)->vz))

#define DotProductSS(v1, v2) ((v1)->vx*(v2)->vx+(v1)->vy*(v2)->vy+(v1)->vz*(v2)->vz)

#define SetVector2D(v1, v2)		(v1)->vx = (v2)->vx, (v1)->vz = (v2)->vz

#define SetVector2DFF(v1, v2)	(v1)->vx = (v2)->vx, (v1)->vz = (v2)->vz



#define AddToVectorVS(res, v1)	(res)->vx += (v1)->vx,\
								(res)->vy += (v1)->vy,\
								(res)->vz += (v1)->vz

#define AddToVectorSS(res, v1)	(res)->vx += (v1)->vx,\
								(res)->vy += (v1)->vy,\
								(res)->vz += (v1)->vz

#define AddToVectorFF(res, v1)	(res)->vx += (v1)->vx,\
								(res)->vy += (v1)->vy,\
								(res)->vz += (v1)->vz

#define AddToVectorFS(res, v1)	(res)->vx += (v1)->vx<<12,\
								(res)->vy += (v1)->vy<<12,\
								(res)->vz += (v1)->vz<<12

#define AddToVectorFV(res, v1)	(res)->vx += (v1)->vx<<12,\
								(res)->vy += (v1)->vy<<12,\
								(res)->vz += (v1)->vz<<12

#define AddToVectorSF(res, v1)	(res)->vx += (short)((v1)->vx>>12),\
								(res)->vy += (short)((v1)->vy>>12),\
								(res)->vz += (short)((v1)->vz>>12)

#define AddVector2D(result, operand1, operand2)					(result)->vx = (operand1)->vx + (operand2)->vx,\
																(result)->vz = (operand1)->vz + (operand2)->vz

#define AddScaledVector2D(result, operand1, scale1, operand2, scale2)	(result)->vx = (((scale1)*(operand1)->vx) + ((scale2)*(operand2)->vx))>>12,\
																		(result)->vz = (((scale1)*(operand1)->vz) + ((scale2)*(operand2)->vz))>>12

#define AddOneScaledVector(result, operand1, operand2, scale2)	(result)->vx = (operand1)->vx + (((scale2)*(operand2)->vx)>>12),\
																(result)->vy = (operand1)->vy + (((scale2)*(operand2)->vy)>>12),\
																(result)->vz = (operand1)->vz + (((scale2)*(operand2)->vz)>>12)

#define AddToVector2D(result, operand)							(result)->vx += (operand)->vx,\
																(result)->vz += (operand)->vz

#define AddToOneScaledVector(result, operand2, scale2)			(result)->vx = (result)->vx + FMul(scale2,(operand2)->vx),\
																(result)->vy = (result)->vy + FMul(scale2,(operand2)->vy),\
																(result)->vz = (result)->vz + FMul(scale2,(operand2)->vz)

/*
#define AddToOneScaledVector(result, operand2, scale2)			(result)->vx = (result)->vx + (((scale2)*(operand2)->vx)>>12),\
																(result)->vy = (result)->vy + (((scale2)*(operand2)->vy)>>12),\
																(result)->vz = (result)->vz + (((scale2)*(operand2)->vz)>>12)

*/
#define AddToOneScaledVector2D( result, operand2, scale2)		(result)->vx = (result)->vx + (((scale2)*(operand2)->vx)>>12),\
																(result)->vz = (result)->vz + (((scale2)*(operand2)->vz)>>12)

#define AddOneScaledVector2D(result, operand1, operand2, scale2)	(result)->vx = (operand1)->vx + (((scale2)*(operand2)->vx)>>12),\
																	(result)->vz = (operand1)->vz + (((scale2)*(operand2)->vz)>>12)


#ifdef __cplusplus
}
#endif

#endif

