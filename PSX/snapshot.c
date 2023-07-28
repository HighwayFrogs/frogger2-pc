
#include <islmem.h>
#include "shell.h"
#include "snapshot.h"

#ifdef _WINDOWS //PP: redefine RECT to something that doesn't conflict with Windows' RECT
#define RECT RECTPS
#endif /*def _WINDOWS*/

#if PALMODE==1
	#define SCN_WIDTH 512
	#define SCN_HEIGHT 256
#else
	#define SCN_WIDTH 512
	#define SCN_HEIGHT 240
#endif

// 0-255
typedef struct {
    unsigned char        Red;
    unsigned char        Green;
    unsigned char        Blue;
    unsigned char        Reserved;
} BMP_RGBType;

// For version 3
typedef struct {
    unsigned short	Identifier;          // "BM"
    unsigned int	FileSize;
    unsigned short	Reserved1;
    unsigned short	Reserved2;
    unsigned int	DataOffset;
} BMP_HeaderType;

typedef struct {
    unsigned int	HeaderSize;             // 40
    unsigned int	Width;
    unsigned int	Height;
    unsigned short	NoOfImagePlanes;        // always 1
    unsigned short	BitsPerPixel;           // 1,4,8 or 24
    unsigned int	Compression;            // 0,1,2 (0-none, 1-8 bit RLE, 2-4 bit RLE)
    unsigned int	SizeOfBitmap;           // in bytes
    unsigned int	HRes;                   // }
    unsigned int	VRes;                   // }- pixels per meter
    unsigned int	Colours;
    unsigned int	SignificantColours;
} BMP_HeaderInfoType;

#define IMG_SIZE (SCN_WIDTH * SCN_HEIGHT * 3)
#define FILE_SIZE (IMG_SIZE + sizeof(BMP_HeaderType) + sizeof(BMP_HeaderInfoType))

unsigned char BMP_SaveHeader[sizeof(BMP_HeaderType)]=
{
	0x42, 0x4D,
    (FILE_SIZE & 0xff),(FILE_SIZE >>8) & 0xff, (FILE_SIZE>>16)&0xff, 0,
    0, 0,
	0, 0,
    0x36, 0, 0, 0           // data offset
};

unsigned char BMP_SaveHeaderInfo[sizeof(BMP_HeaderInfoType)]=
{
	0x28, 0, 0, 0,		// header size = 40
    (SCN_WIDTH & 0xff), (SCN_WIDTH >> 8) & 0xff, 0, 0,       // width = 320
    (SCN_HEIGHT & 0xff), (SCN_HEIGHT >> 8) & 0xff, 0, 0,

    0x01, 0,            // 1 plane
	0x18, 0,			// 24-bpp
	0, 0, 0, 0,			// 0 compression

    (IMG_SIZE & 0xff),(IMG_SIZE >>8) & 0xff, (IMG_SIZE>>16)&0xff, 0,  // 320 * 240 * 3

    0x13, 0x0B, 0, 0,
	0x13, 0x0B, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0
};
/*
// Example call (note final backslash)
	if(padData.debounce[4] & PAD_START)
	{
		SnapShot("C:\\PSX\\CRTEST\\SNAPSHOT\\");
	}
*/
int SnapShot(char *root)
{
#if GOLDCD==NO

    int i,j;
    int file;
	short *memptr;
	short *linestart;
    char *output;
	char	filename[80];
    int value;
    int temp = 0;
    short *mem2;
    char *memc;
	RECT rect;


	for(i=0; i<=9999; i++)
	{
		sprintf(filename, "%sGRAB%04d.BMP",root,i);
		if ((file=PCopen(filename,1,0))==-1)
			break;
		else
			PCclose(file);
	}
	if (i>9999)
	{
		utilPrintf("Error trying to save %s\n", filename);
		return 0;
	}

    memptr = (short *)MALLOC(SCN_WIDTH * 2);
    output = MALLOC(SCN_WIDTH*3);

	DrawSync(0);

    if(!memptr || !output)
	{
		utilPrintf("Error malloccing screenshot\n");
		return 0;
	}

	if ((file = PCcreat(filename,1))==-1)
	{
		utilPrintf("Error creating file %s\n", filename);
		return 0;
	}

    PCwrite(file,BMP_SaveHeader,14);
    PCwrite(file,BMP_SaveHeaderInfo,sizeof(BMP_HeaderInfoType));

	rect.x = 0;
	rect.y = 0;
	rect.w = SCN_WIDTH;
	rect.h = 1;

    for (j=0;j<SCN_HEIGHT;j++)
	{
		rect.y = (SCN_HEIGHT-1-j);
		StoreImage(&rect,(unsigned long *)memptr);

	    mem2 = memptr;
        memc = output;
        for (i=0;i<SCN_WIDTH;i++)
		{
            *memc++ = (((*mem2) >>10)<<3) & 0xff;
            *memc++ = (((*mem2) >>5)<<3) & 0xff;
            *memc++ = ((*mem2) << 3) & 0xff;
            mem2++;
        }
	    PCwrite(file,output,SCN_WIDTH*3);
	}


    FREE(memptr);
    FREE(output);

	PCclose(file);

	utilPrintf("Grabbed screen %s\n", filename);

	return 1;
#endif
}

