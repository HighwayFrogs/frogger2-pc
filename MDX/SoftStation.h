// -----------------------------------------------------------------------
//  This file is part of the SoftStation Project, (c) 2000 ISL
//
//     File : SoftStation.h
//  Purpose : top-level library header
// -----------------------------------------------------------------------


// --------------------
// Constants and macros

// ** MMX defines

// ** Define this to use the "MaskShiftOr" rather than the default "MulOr" for
// ** 888->555/565 mmx blit. The default algorithm has a slight performance
// ** increase over the "MaskShiftOr". [Not tested on PentiumI MMX or Pro]
//#define SSMMX_BLIT_MASKSHIFTOR

// ** Define SSMMX_MINWIDTH_USE to use a minimum amount threshold on all mmx
// ** fillers. SSMMX_MINWIDTH_NUMPIXELS defines the actual pixel width threshold.
//#define SSMMX_MINWIDTH_USE			
#define SSMMX_MINWIDTH_NUMPIXELS	12


// define this to gather stats
#define SSGATHERSTATS

// scene flags - [called with ssEndScene]
#define SSENDSCENE_NOBLIT			(1<<0)	// set if you don't want the blit to occur at ssEndScene

// texture map hints - [called with ssSetTexture]
#define SSTEXHINT_NOTRANS			(1<<0)	// set if you are sure the texture map isn't transparent
#define SSTEXHINT_WRAPUVS			(1<<1)	// set if you want the texture uvs to wrap around - (used to create sliding textures)

// render states
enum ESSRenderState {
	SSRENDERSTATE_SHADEMODE,				// set the shading mode (see ESSShadeMode)
	SSRENDERSTATE_ALPHAMODE,				// set the alpha mode (see ESSAlphaMode)
	SSRENDERSTATE_COLOURMODEL,				// set the colour model (see ESSColourModel)
	SSRENDERSTATE_CLIPPING,					// set the clipping state (see ESSClipping)
	SSRENDERSTATE_CULLING,					// set the desired culling mode (see ESSCulling)
};


// _SHADEMODE values
enum ESSShadeMode {
	SSSHADEMODE_FLAT,
	SSSHADEMODE_GOURAUD,

	SSSHADEMODE_FORCE32BIT = 0x7fffffff
};


// _ALPHAMODE values
enum ESSAlphaMode {
	SSALPHAMODE_NONE,
	SSALPHAMODE_ADD,
	SSALPHAMODE_SUB,
	SSALPHAMODE_SEMI,

	SSALPHAMODE_FORCE32BIT = 0x7fffffff
};


// _COLOURMODEL values
enum ESSColourModel {
	SSCOLOURMODEL_RGB,
	SSCOLOURMODEL_RAMP,

	SSCOLOURMODEL_FORCE32BIT = 0x7fffffff
};


// _CLIPPING values
enum ESSClipping {
	SSCLIPPING_ON,
	SSCLIPPING_OFF,

	SSCLIPPING_FORCE32BIT = 0x7fffffff
};


// _PIXELFORMAT values
enum ESSPixelFormat {
	SSPIXELFORMAT_555,
	SSPIXELFORMAT_565,

	SSPIXELFORMAT_FORCE32BIT = 0x7fffffff
};


// _CULLING values
enum ESSCulling {
	SSCULLING_NONE,
	SSCULLING_CCW,
	SSCULLING_CW,

	SSCULLING_FORCE32BIT = 0x7fffffff
};

// make a 16bit pixel from a 24bit RGB triple
#define SSRGB16(R,G,B)  ((ssPixelFormat == SSPIXELFORMAT_555) ? (((R>>3)<<10) | ((G>>3)<<5) | (B>>3)) : (((R>>3)<<11) | ((G>>2)<<5) | (B>>3)))

// clip interpolator precision
#define SSCLIP_PRECISION		10
#define SSCLIP_FPRECISION		0.0009765625f

// RGBA interpolation precision
#define SSRGBA_PRECISION		8
//#define SSRGBA_PRECISION		14				// non mmx version of this library used to use this value

#define SSRGBA_PRECISIONX2		(SSRGBA_PRECISION-1)
#define SSRGBA_PRECISIONSHIFT	(SSRGBA_PRECISION+8-5)
#define SSRGBA_PRECISIONSHIFTX2	(SSRGBA_PRECISIONSHIFT-1)

// UV interpolation / coordinate precision
#define SSUV_INTERPPRECISION	8
#define SSUV_COORDPRECISION		0
//#define SSUV_INTERPPRECISION	10				// non mmx version of this library used to use this value
//#define SSUV_COORDPRECISION	12				// non mmx version of this library used to use this value

// UV total precision
#define SSUV_TOTALPRECISION			(SSUV_INTERPPRECISION+SSUV_COORDPRECISION)
#define SSUV_TOTALPRECISIONCARRY	(1<<SSUV_TOTALPRECISION)
#define SSUV_TOTALPRECISIONMASK		(SSUV_TOTALPRECISIONCARRY-1)

// UV remainder
#define SSUV_REMAINDER				(SSUV_COORDPRECISION>>1)

// macro to make shifted (u,v) coordinates
#define SSMAKEUV(UV)				(((UV)<<SSUV_COORDPRECISION)+SSUV_REMAINDER)


// -----------------------------
// Types, Structures and Classes

// basic types
#ifndef uchar
typedef unsigned char	uchar;
#endif
#ifndef ushort
typedef unsigned short	ushort;
#endif
#ifndef ulong
typedef unsigned long	ulong;
#endif
typedef int				ssFixed28;


// alpha blending callback
typedef void (*TSSAlphaBlend)(ushort *scanBase, int width);


// a polygon vertex
typedef struct _TSSVertex {
	int			x,y;					// position
	int			r,g,b;					// colour components
	int			u,v;					// texture coordinates (in texels << SSUVCOORD_PRECISION)
} TSSVertex;


// a polygon edge
typedef struct _TSSPolyEdge {
	int			x;
	int			xStep;
	int			numerator;
	int			denominator;
	int			errorTerm;
	int			y;
	int			height;
	int			r,g,b;					// colour channels
	int			rd,gd,bd;				// colour channel deltas
	int			u,v, ud,vd;				// texture coordinates/deltas
} TSSPolyEdge;


// -------
// Globals

extern int				ssViewXPos, ssViewYPos, ssViewWidth, ssViewHeight;		// viewport parameters;
extern uchar			*ssSurfacePtr;											// ptr to start of surface memory
extern int				ssPitch;												// surface pitch
extern ulong			*ssTexPixels;											// texture pixel data
extern int				ssTexWidth, ssTexHeight;								// texture dimensions
extern unsigned int		ssTexHints;												// texture hints
extern int				ssTexWidthShift;										// texture width shift
extern ushort			scanBuffer[2048];										// scanline buffer
extern TSSAlphaBlend	ssAlphaBlend;											// alpha blend callback
extern int				ssColourKey16;											// 16bit colour key values
extern ushort			*ssScanlinePtr;											// ptr to the current scanline
extern ulong			*ssMMXBufferPtr;										// ptr to mmx data aligned screen buffer
extern int				ssMMXWidth;												// mmx buffer width
extern int				ssMMXHeight;											// mmx buffer height
extern __int64			ssMMXSemiAlphaBuffer[1024];								// mmx semi alpha blend buffer

#ifdef SSGATHERSTATS
extern int				ssNoofPixels;											// #pixels filled on this poly
#endif

// render states
extern ESSShadeMode		ssShadeMode;											// current shading mode
extern ESSAlphaMode		ssAlphaMode;											// current alpha mode
extern ESSColourModel	ssColourModel;											// current colour model
extern ESSClipping		ssClipping;												// current clipping state
extern ESSPixelFormat	ssPixelFormat;											// desired pixel format


// ----------
// Prototypes

// internal alpha blend functions
void ssAlphaBlend_MMX_Semi(ulong *bufferBase, int width);
void ssAlphaBlend_MMX_Semi_ColourKey(ulong *bufferBase, int width);


/* --------------------------------------------------------------------------------
   Function : ssInit
   Purpose : initialise the SoftStation library
   Parameters : pixel format
   Returns : 
   Info : 
*/

void ssInit(ESSPixelFormat pixelFormat);


/* --------------------------------------------------------------------------------
   Function : ssShutdown
   Purpose : shutdown the SoftStation library
   Parameters : 
   Returns : 
   Info : 
*/

void ssShutdown();


/* --------------------------------------------------------------------------------
   Function : ssBeginScene
   Purpose : begin a rendered scene
   Parameters : ptr to start of screen, #bytes between scanlines, pixel width, height
   Returns : 
   Info : pixel width and height are used to determine the buffer dimensions
*/

void ssBeginScene(void *screen, int pitch, int pixelWidth, int pixelHeight);


/* --------------------------------------------------------------------------------
   Function : ssEndScene
   Purpose : end a rendered scene
   Parameters : end scene flags [see above]. default: none
   Returns : 
   Info : pass SSENDSCENE_NOBLIT to skip the render buffer blit, although it is then
		  your responsibility to make the call to ssBlitToScreen explicitly.
*/

void ssEndScene(int flags = 0);


/* --------------------------------------------------------------------------------
   Function : ssSetViewport
   Purpose : set the viewport parameters
   Parameters : topleft(x,y), width, height
   Returns : 
   Info : 
*/

void ssSetViewport(int xTopLeft, int yTopLeft, int width, int height);


/* --------------------------------------------------------------------------------
   Function : ssClearViewport
   Purpose : clear the current viewport
   Parameters : 
   Returns : 
   Info : valid only inbetween ssBeginScene and ssEndScene
*/

void ssClearViewport();


/* --------------------------------------------------------------------------------
   Function : ssDrawPrimitive
   Purpose : draw a primitive
   Parameters : vertices, #vertices (3 or 4)
   Returns : 
   Info : 
*/

void ssDrawPrimitive(TSSVertex *vertices, int noofVertices);


/* --------------------------------------------------------------------------------
   Function : ssSetRenderState
   Purpose : set a render state
   Parameters : state, new value
   Returns : 
   Info : 
*/

void ssSetRenderState(ESSRenderState state, int value);


/* --------------------------------------------------------------------------------
   Function : ssSetTexture
   Purpose : set the current texture info
   Parameters : pixel data, width, height, texture map hints [see top], default: none
   Returns : 
   Info : 
*/

void ssSetTexture(void *pixels, int width, int height, int hints = 0);


/* -----------------------------------------------------------------------
   Function : ssCeil28
   Purpose : ceiling function for 28:4 fixed point
   Parameters : value 
   Returns : ceiling
   Info : 
*/

int ssCeil28(ssFixed28 value);


/* -----------------------------------------------------------------------
   Function : ssFloorDivMod
   Purpose : find the divisor and remainder from a division
   Parameters : 
   Returns : 
   Info : 
*/

void ssFloorDivMod(int numerator, int denominator, int *floorRef, int *modRef);


/* --------------------------------------------------------------------------------
   Function : ssBlitToScreen()
   Purpose : if using mmx blits to the 16bit screen, else does nothing
   Parameters : 
   Returns : 
   Info : 
*/

void ssBlitToScreen();


/* --------------------------------------------------------------------------------
   Function : ssSetBrightness
   Purpose : set brightness for all rgbs
   Parameters : brightness factor, 0 = linear
   Returns : 
   Info : 
*/

void ssSetBrightness(float bright);


/* --------------------------------------------------------------------------------
   Function : ssUseMMX
   Purpose : use mmx rendering if available
   Parameters : use mmx = 1, no mmx = 0
   Returns : 
   Info : 
*/

void ssUseMMX(int mmxFlag);


/* --------------------------------------------------------------------------------
   Function : ssIsUsingMMX
   Purpose : are we using mmx rendering?
   Parameters : 
   Returns : using mmx = 1, no mmx = 0
   Info : 
*/

int ssIsUsingMMX();
