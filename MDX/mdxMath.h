#ifndef MDXMATH_H_INCLUDED
#define MDXMATH_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#define PI			(3.141592654)
#define PI_OVER_2	(3.141592654/2.0F)
#define PI_OVER_4	(3.141592654/4.0F)
#define PI2			(3.141592654*2.0F)

typedef struct _MDX_VECTOR
{
	float	vx,vy,vz;
}MDX_VECTOR;

typedef struct
{
	signed char	v[4];
}MDX_BYTEVECTOR;

typedef struct
{
	short	v[3];
}MDX_SHORTVECTOR;

typedef struct
{
	unsigned short	v[2];
}MDX_USHORT2DVECTOR;

typedef struct
{
	short	v[2];
}MDX_SHORT2DVECTOR;

typedef struct
{
	float	x, y, z, w;
}MDX_QUATERNION;

typedef struct
{
	float	matrix[4][4];
}MDX_MATRIX;

#define MAX_STACK_SIZE	16

typedef struct
{

	float	stack[MAX_STACK_SIZE][4][4];
	short	stackPosition;
	float	result[4][4];
}MDX_MATRIXSTACK;

extern MDX_MATRIXSTACK	matrixStack;

extern MDX_VECTOR upV, rightV, inV;

void guTranslateF(float a[4][4], float dx, float dy, float dz);
//void guMtxCatF(float *b, float *a, float *ret);
//void guMtxCatF(float *b, float *a, float *ret);

#define guMtxCatF(a,b,ret) {\
	(ret)[ 0] = *((a)+0) * *((b)+0) + *((a)+1) * *((b)+4) + *((a)+2) * *((b)+8); \
	(ret)[ 1] = *((a)+0) * *((b)+1) + *((a)+1) * *((b)+5) + *((a)+2) * *((b)+9); \
	(ret)[ 2] = *((a)+0) * *((b)+2) + *((a)+1) * *((b)+6) + *((a)+2) * *((b)+10); \
	(ret)[ 3] = 0; \
	(ret)[ 4] = *((a)+4) * *((b)+0) + *((a)+5) * *((b)+4) + *((a)+6) * *((b)+8); \
	(ret)[ 5] = *((a)+4) * *((b)+1) + *((a)+5) * *((b)+5) + *((a)+6) * *((b)+9); \
	(ret)[ 6] = *((a)+4) * *((b)+2) + *((a)+5) * *((b)+6) + *((a)+6) * *((b)+10); \
	(ret)[ 7] = 0; \
	(ret)[ 8] = *((a)+8) * *((b)+0) + *((a)+9) * *((b)+4) + *((a)+10) * *((b)+8); \
	(ret)[ 9] = *((a)+8) * *((b)+1) + *((a)+9) * *((b)+5) + *((a)+10) * *((b)+9); \
	(ret)[10] = *((a)+8) * *((b)+2) + *((a)+9) * *((b)+6) + *((a)+10) * *((b)+10); \
	(ret)[11] = 0;\
	(ret)[12] = *((a)+12) * *((b)+0) + *((a)+13) * *((b)+4) + *((a)+14) * *((b)+8) + *((b)+12);\
	(ret)[13] = *((a)+12) * *((b)+1) + *((a)+13) * *((b)+5) + *((a)+14) * *((b)+9) + *((b)+13);\
	(ret)[14] = *((a)+12) * *((b)+2) + *((a)+13) * *((b)+6) + *((a)+14) * *((b)+10) + *((b)+14);\
	(ret)[15] = 1;\
}

#define PushMatrix(matrix)\
{ \
	float *ret,*a,*b;\
	ret = (float *)matrixStack.stack[matrixStack.stackPosition+1];\
	a = (float *)matrix;\
	b = (float *)matrixStack.stack[matrixStack.stackPosition];\
	(ret)[ 0] = *((a)+0) * *((b)+0) + *((a)+1) * *((b)+4) + *((a)+2) * *((b)+8); \
	(ret)[ 1] = *((a)+0) * *((b)+1) + *((a)+1) * *((b)+5) + *((a)+2) * *((b)+9); \
	(ret)[ 2] = *((a)+0) * *((b)+2) + *((a)+1) * *((b)+6) + *((a)+2) * *((b)+10); \
	(ret)[ 3] = 0; \
	(ret)[ 4] = *((a)+4) * *((b)+0) + *((a)+5) * *((b)+4) + *((a)+6) * *((b)+8); \
	(ret)[ 5] = *((a)+4) * *((b)+1) + *((a)+5) * *((b)+5) + *((a)+6) * *((b)+9); \
	(ret)[ 6] = *((a)+4) * *((b)+2) + *((a)+5) * *((b)+6) + *((a)+6) * *((b)+10); \
	(ret)[ 7] = 0; \
	(ret)[ 8] = *((a)+8) * *((b)+0) + *((a)+9) * *((b)+4) + *((a)+10) * *((b)+8); \
	(ret)[ 9] = *((a)+8) * *((b)+1) + *((a)+9) * *((b)+5) + *((a)+10) * *((b)+9); \
	(ret)[10] = *((a)+8) * *((b)+2) + *((a)+9) * *((b)+6) + *((a)+10) * *((b)+10); \
	(ret)[11] = 0;\
	(ret)[12] = *((a)+12) * *((b)+0) + *((a)+13) * *((b)+4) + *((a)+14) * *((b)+8) + *((b)+12);\
	(ret)[13] = *((a)+12) * *((b)+1) + *((a)+13) * *((b)+5) + *((a)+14) * *((b)+9) + *((b)+13);\
	(ret)[14] = *((a)+12) * *((b)+2) + *((a)+13) * *((b)+6) + *((a)+14) * *((b)+10) + *((b)+14);\
	(ret)[15] = 1;\
	matrixStack.stackPosition++; \
}

void guScaleF(float a[4][4], float dx, float dy, float dz);
void guLookAtF (float m[4][4],float xEye, float yEye, float zEye,float xAt, float yAt, float zAt,float xUp, float yUp, float zUp, long updateView);
void guMtxXFMF(float m[4][4],float srcX,float srcY,float srcZ,float *destX,float *destY,float *destZ);


#define EPSILON		((float)0.00001)
#define HALFPI		((float)1.570796326794895)

#define DotProduct(v1,v2)	((((v1)->vx)*((v2)->vx)) + (((v1)->vy)*((v2)->vy)) + (((v1)->vz)*((v2)->vz)))
#define mdxMagnitudeSquared(vect) ((vect)->vx*(vect)->vx + (vect)->vy*(vect)->vy + (vect)->vz*(vect)->vz)
#define mdxMagnitude(vect) ((float)sqrt((vect)->vx*(vect)->vx + (vect)->vy*(vect)->vy + (vect)->vz*(vect)->vz))
#define CrossProduct(result,operand1,operand2) {(result)->vx = (operand1)->vy * (operand2)->vz - (operand1)->vz * (operand2)->vy; (result)->vy = (operand1)->vz * (operand2)->vx - (operand1)->vx * (operand2)->vz; (result)->vz = (operand1)->vx * (operand2)->vy - (operand1)->vy * (operand2)->vx;}
#define SetQuaternion(q1,q2) (*(q1)) = (*(q2))
#define ZeroQuaternion(quat) SetQuaternion(quat,&zeroQuat)
#define Bound(a,x,b) Min(b,Max(a,x))
#define Max(p1,p2)	(((p1) > (p2)) ? (p1) : (p2))
#define Min(p1,p2)	(((p1) > (p2)) ? (p2) : (p1))

#define PopMatrix() (matrixStack.stackPosition--)
#define SetVector(v1,v2)	(*(v1)) = (*(v2))
#define MatrixSet(m1,m2) (*((MDX_MATRIX *)m1)) = (*((MDX_MATRIX *)m2))
#define LinearInterp(result,from,to,interp) {(result)->vx = (from)->vx + (((to)->vx - (from)->vx) * interp);(result)->vy = (from)->vy + (((to)->vy - (from)->vy) * interp);(result)->vz = (from)->vz + (((to)->vz - (from)->vz) * interp);}
#define mdxAddToVector(res,v1) {(res)->vx += (v1)->vx;(res)->vy += (v1)->vy;(res)->vz += (v1)->vz;}
#define SubVector(res,v1,v2) {(res)->vx = (v1)->vx - (v2)->vx;(res)->vy = (v1)->vy - (v2)->vy;(res)->vz = (v1)->vz - (v2)->vz;}
#define AddVector(res,v1,v2) {(res)->vx = (v1)->vx + (v2)->vx;(res)->vy = (v1)->vy + (v2)->vy;(res)->vz = (v1)->vz + (v2)->vz;}
#define ScaleVector(v1,scale) {(v1)->vx *= scale;(v1)->vy *= scale;(v1)->vz *= scale;}

void QuaternionToMatrix(MDX_QUATERNION *squat, MDX_MATRIX *dmatrix);
void GetQuaternionFromRotation(MDX_QUATERNION *destQ,MDX_QUATERNION *srcQ);
void QuatSlerp(MDX_QUATERNION *src1, MDX_QUATERNION *sp2, float t, MDX_QUATERNION *dquat);
void QuaternionMultiply(MDX_QUATERNION *dest,MDX_QUATERNION *src1,MDX_QUATERNION *src2);
void MatrixToQuaternion(MDX_MATRIX *smatrix, MDX_QUATERNION *dquat);
void RotateVectorByRotation(MDX_VECTOR *result,MDX_VECTOR *vect,MDX_QUATERNION *rot);
void GetRotationFromQuaternion(MDX_QUATERNION *destQ,MDX_QUATERNION *srcQ);
void InitMaths(void);

float mdxFindShortestAngle(float val1, float val2);

extern MDX_VECTOR curAt;
extern MDX_VECTOR curEye;
extern MDX_QUATERNION zeroQuat;

void MatrixStackInitialise();
void Normalise(MDX_VECTOR *vect);
extern long changedView;

#ifdef __cplusplus
}
#endif

#endif
