#include "types.h"

VECTOR MakeVECTOR(long x, long y, long z)
{
	VECTOR Ret;
	Ret.vx = x;
	Ret.vy = y;
	Ret.vz = z;
	return Ret;
}

SVECTOR MakeSVECTOR(short x, short y, short z)
{
	SVECTOR Ret;
	Ret.vx = x;
	Ret.vy = y;
	Ret.vz = z;
	return Ret;
}

FVECTOR MakeFVECTOR(fixed x, fixed y, fixed z)
{
	FVECTOR Ret;
	Ret.vx = x;
	Ret.vy = y;
	Ret.vz = z;
	return Ret;
}

//FLVECTOR MakeFLVECTOR(fixed x, fixed y, fixed z)
//{
//	FLVECTOR Ret;
//	Ret.vx = x/4096.0F;
//	Ret.vy = y/4096.0F;
//	Ret.vz = z/4096.0F;
//	return Ret;
//}
