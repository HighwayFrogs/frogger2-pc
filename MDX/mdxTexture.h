/*

	This file is part of the M libraries,

	File		: 
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef MDXTEXTURE_H_INCLUDED
#define MDXTEXTURE_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#define TEXTURE_NORMAL 0
#define TEXTURE_AI	1
#define TEXTURE_PROCEDURAL 2
#define TEXTURE_NOTEXTURE 3
#define TEXTURE_AC	4
typedef struct TAG_MDX_TEXENTRY
{
	long CRC;
	char name[32];
//	char bank[64];
	short *data, *gelfData;
	long *softData;
	char keyed;
	long type;
	LPDIRECTDRAWSURFACE7 surf;
	float xPos,yPos;
	long xSize,ySize;
	long refCount;

	unsigned long updated;
	unsigned long lastFrame;
	unsigned long lastGameFrame;
	unsigned long numFrames;	
	float *frameTimes;

	// *ASL* 15/06/2000
	void			**frames;		// LPDIRECTDRAWSURFACE7 *frames;

	struct TAG_MDX_TEXENTRY *next,*prev;	
} MDX_TEXENTRY;

typedef struct TAG_MDX_TEXPAGE
{
	LPDIRECTDRAWSURFACE7 surf;
	unsigned long numTex;

	struct TAG_MDX_TEXPAGE *next;	
} MDX_TEXPAGE;

typedef struct _TEXTURE_HEADER
{
	DWORD flags;
	DWORD CRC;
	short dim[2];
	char name[12];
} TEXTURE_HEADER;

#define TEXTURE_BASE "textures\\"

extern MDX_TEXENTRY *texList;
extern MDX_TEXENTRY *cDispTexture;
extern float textureAdjustDivider;
extern LPDIRECTDRAWSURFACE7 testS;

MDX_TEXENTRY *GetTexEntryFromCRC (long CRC);
unsigned long LoadTexBank(char *bank, char *baseDir);
unsigned long LoadTexBankFile(char *bank, char *baseDir);
void FreeAllTextureBanks( );
void InitWater(char *file);
void UpdateWater(void);
void ShowTextures(void);
void mdxSetBackdropToTex(MDX_TEXENTRY *t);
void UpdateAnimatingTextures(void);
void GrabSurfaceToTexture(long x, long y, MDX_TEXENTRY *texture, LPDIRECTDRAWSURFACE7 srf);
void PrintTextureInfo(void);
void WriteTexturesToFile(void);

extern long surfacesMade;
extern char showMemDebug;

#ifdef __cplusplus
}
#endif

#endif