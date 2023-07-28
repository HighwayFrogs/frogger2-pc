

#ifndef __PSITYPES_H__
#define __PSITYPES_H__



#define GPU_COM_TF4SPR (0x64)

#define YES		1
#define TRUE	1
#define ON		1

#define NO		0
#define FALSE	0
#define OFF		0


typedef struct {
	u_char r, g, b, code;
} RGBCD;

////////////////////////////////////////////////////////////////////////

//	prim 			struct	 sony type		 prim type			
// POLY_F3;		TMD_P_FG3I	GPU_COM_F3 	   Flat Triangle 			
// POLY_F4;	   	TMD_P_FG4I	GPU_COM_F4	   Flat Quadrangle 
// POLY_G3;		TMD_P_FG3I	GPU_COM_G3 	   Gouraud Triangle 
// POLY_G4;	   	TMD_P_FG4I	GPU_COM_G4	   Gouraud Quadrangle 

// POLY_FT3;	TMD_P_FT3I	GPU_COM_TF3	   Flat Textured Triangle 
// POLY_FT4;	TMD_P_FT4I	GPU_COM_TF4	   Flat Textured Quadrangle 
// POLY_GT3;	TMD_P_GT3I	GPU_COM_TG3	   Gouraud Textured Triangle 
// POLY_GT4;	TMD_P_GT4I	GPU_COM_TG4	   Gouraud Textured Quadrangle 
							

typedef struct {
	long	next;
	u_char	out, in, dummy, cd;
	u_char	r0, g0, b0, code;
	u_char	r1, g1, b1, dummy1;
	u_char	r2, g2, b2, dummy2;
	u_short n0, v0;
	u_short v1, v2;
	int	padto8;
}TMD_P_FG3I;

typedef struct {
	long	next;
	u_char	out, in, dummy, cd;
	u_char	r0, g0, b0, code;
	u_char	r1, g1, b1, dummy1;
	u_char	r2, g2, b2, dummy2;
	u_char	r3, g3, b3, dummy3;
	u_short n0, v0;
	u_short v1, v2;
	u_short v3, dummy4;
	int	padto8;
}TMD_P_FG4I;

typedef struct {
	long	next;
	u_char  out, in, dummy, cd;
	u_char  tu0, tv0;
	u_short clut;
	u_char  tu1, tv1;
	u_short tpage;
	u_char  tu2, tv2;
	u_char  tile,p;
	u_char  r0, g0, b0, code;
	u_short n0, v0;
	u_short v1, v2;
} TMD_P_FT3I;


typedef struct {
	long  next;
	u_char  out, in, dummy, cd;
	u_char  tu0, tv0;			
	u_short clut;
	u_char  tu1, tv1;
	u_short tpage;
	u_char  tu2, tv2;
	u_char  tile,p0;
	u_char  tu3, tv3;
	u_short p1;
	u_char  r0, g0, b0, code;
	
	u_short v0;
	u_short v1, v2;
	u_short v3;
	u_short	n0;
	u_short	padto8;
}TMD_P_FT4I;


typedef struct {
	long	next;
	u_char  out, in, dummy, cd;

	u_char  tu0, tv0;
	u_short clut;

	u_char  tu1, tv1;
	u_short tpage;

	u_char  tu2, tv2;
	u_short v0;

	u_char  r0, g0, b0, code;
	u_char  r1, g1, b1, tile;	  
	u_char  r2, g2, b2, code2;
	u_short v1;
	u_short v2;
	RGBCD avgTexCol;
}TMD_P_GT3I;

typedef struct {
	long next;
	u_char  out, in, dummy, cd;

	u_char  tu0, tv0;
	u_short clut;

	u_char  tu1, tv1;
	u_short tpage;

	u_char  tu2, tv2;
	u_short v0;

	u_char  tu3, tv3;
	u_short v1;

	u_char  r0, g0, b0, code;
	u_char  r1, g1, b1, tile;
	u_char  r2, g2, b2, code2;
	u_char  r3, g3, b3, code3;
	u_short v2, v3;
	RGBCD avgTexCol;
}TMD_P_GT4I;





//#define NUMOFSIDES 		3	   //NB 0=SPRITE 1=LINE 2=TRI 3=QUAD
//#define TEXTURED   		4
#define psiDOUBLESIDED 	1
//#define LIT	 			16
//#define TILEDH			32
//#define TILEDV			64
#define psiTRANSPAR		2
//#define GOURAUD     	512

void dcachePsiCalcMaxMin ( PSIDATA *psiData, long *tfd );

#endif //__PSITYPES_H__