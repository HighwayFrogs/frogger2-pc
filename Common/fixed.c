#include "fixed.h"

// fixed ToFixed(long f)
// {
// 	return f*4096;
// }

fixed FMul(fixed x, fixed y)
{
	int a = x>>12;
	int b = x&4095;
	
	int c = y>>12;
	int d = y&4095;

	int res1 = (d*b)>>12;
	int res2 = d*a;
	int res3 = c*b;
	int res4 = (c*a)<<12;

	return res1 + res2 + res3 + res4;
}

fixed NewFDiv(fixed x, fixed y, char *file, int line)
{
//	int invDiv = 16777216L / y;
//
//	int a = x >>12;
//	int b = x &4095;
//	
//	int c = invDiv>>12;
//	int d = invDiv&4095;
//
//	int res1 = (d*b)>>12;
//	int res2 = d*a;
//	int res3 = c*b;
//	int res4 = (c*a)<<12;
//
//	return  res1 + res2 + res3 + res4;

	int invDiv = 2147483648L / y;

	int a = x >>12;
	int b = x &4095;
	
	int c = invDiv>>12;
	int d = invDiv&4095;

	int res1 = (d*b)>>19;
	int res2 = (d*a)>>7;
	int res3 = (c*b)>>7;
	int res4 = (c*a)<<5;

	return  res1 + res2 + res3 + res4;
}

fixed FDiv2(fixed x, fixed y)
{
	if ( y ==0 )
		return 0;
	
	while(y>16777216L)
	{
		x>>=1;
		y>>=1;
	}

	return FDiv(x,y);
}
