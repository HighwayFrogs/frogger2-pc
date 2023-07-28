#ifndef FIXED_H
#define FIXED_H

#define FIXED_SCALE
#define FIXED_INT
#define FIXED_INT

#define FDiv(X, Y) NewFDiv(X, Y, __FILE__, __LINE__)

typedef long fixed;

typedef struct
{
	fixed vx,vy,vz;
}FVECTOR;

//fixed ToFixed(long f);
#define ToFixed(x) ( ((int)(x)) << 12)

#ifdef __cplusplus
extern "C" {
#endif

fixed FMul(fixed x, fixed y);
fixed NewFDiv(fixed x, fixed y, char *file, int line);
fixed FDiv2(fixed x, fixed y);

#ifdef __cplusplus
}
#endif
#endif
