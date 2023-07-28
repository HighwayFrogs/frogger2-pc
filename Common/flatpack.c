/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: flatpack.c
	Programmer	: David Swift (from code by Richard Hackett)
	Date		: 25 May 00

----------------------------------------------------------------------------------------------- */

#include <islutil.h>
#include <islmem.h>
#include <islfile.h>

#include "Shell.h"
#include "SonyLibs.h"

#include "layout.h"
#include "flatpack.h"

//#ifdef PSX_VERSION
//#include <libcrypt.h>
//#endif


//#define DECRYPTION 0

int numStakFiles = 0;

STAKFILE stakFiles [ NUM_STAKFILES ] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

/*	--------------------------------------------------------------------------------
	Function		: getFileFromStack()
	Purpose			: Get ptr to (sub)file from inside flatstack file
	Parameters		: Ptr to flatstack file, name of (sub)file, ptr to length int
	Returns			: Ptr to base of required file
*/

char *getFileFromStack(char *stackFile, char *name, int *length)
{
	unsigned long 	crc;
	int				loop, numFiles, *iPtr;

	crc = utilStr2CRC(name);
	iPtr = (int *)stackFile;
	numFiles = iPtr[1];
	for(loop=0; loop<numFiles; loop++)
	{
		if (iPtr[2+loop*3]==crc)
		{
			utilPrintf("STACK FILE: %s\n", name);
			if (length!=NULL)
				*length = iPtr[2+loop*3+2];
			return stackFile+iPtr[2+loop*3+1];
		}
	}
	return NULL;
}

char *FindStakFileInAllBanks ( char *name, int *length )
{
	int c;
	char *stakFile;

	for ( c = 0; c < NUM_STAKFILES; c++ )
	{
		if ( stakFiles [ c ].stakFile )
		{
			stakFile = getFileFromStack ( stakFiles [ c ].stakFile, name, length );

			if ( stakFile )
				return stakFile;

		}
	}
	// ENDIF

	return NULL;
}


void LoadStakFile ( int stakBank )
{
	int fileLength;
	char file[64];
	int i;

	strcpy ( file, "STAKFILES\\" );

	switch ( stakBank )
	{
		case FROGGER_STAK:
				strcat ( file, "FROGGER.STK" );				
			break;
		case LILLIE_STAK:
				strcat ( file, "LILLIE.STK" );				
			break;
		case TAD_STAK:
				strcat ( file, "TAD.STK" );				
			break;
		case SWAMPY_STAK:
				strcat ( file, "SWAMPY.STK" );				
			break;
		case TREEBY_STAK:
				strcat ( file, "TREEBY.STK" );				
			break;
		case WART_STAK:
				strcat ( file, "TOAD.STK" );				
			break;
		case ROOBIE_STAK:
				strcat ( file, "ROOBIE.STK" );				
			break;
		case TANK_STAK:
				strcat ( file, "TANK.STK" );				
			break;


		case MFROGGER_STAK:
						strcat ( file, "MFROGGER.STK" );					
				break;
		case MLILLIE_STAK:
							strcat ( file,  "MLILLIE.STK" );					
				break;
		case MTAD_STAK:
							strcat ( file,  "MTAD.STK" );					
				break;
		case MSWAMPY_STAK:
						strcat ( file,  "MSWAMPY.STK" );					
				break;
		case MTREEBY_STAK:
							strcat ( file,  "MTREEBY.STK" );					
				break;
		case MWART_STAK:
							strcat ( file,  "MTOAD.STK" );					
				break;
		case MROOBIE_STAK:
						strcat ( file,  "MROOBIE.STK" );					
				break;
		case MTANK_STAK:
					strcat ( file,  "MTANK.STK" );					
				break;


		case GENERIC_STAK:
				strcat ( file, "GENERIC.STK" );				
			break;

		case GARDEN1_STAK:
				strcat ( file, "GARDEN1.STK" );				
			break;

		case ANCIENTS1_STAK:
				strcat ( file, "ANCIENTS1.STK" );				
			break;

		case ANCIENTS2_STAK:
				strcat ( file, "ANCIENTS2.STK" );				
			break;

		case ANCIENTS3_STAK:
				strcat ( file, "ANCIENTS3.STK" );				
			break;

		case FRONTEND1_STAK:
				strcat ( file, "FRONTEND.STK" );				
			break;

		case FRONTEND4_STAK:
				strcat ( file, "TRAINING.STK" );				
			break;

		case SPACE1_STAK:
				strcat ( file, "SPACE1.STK" );				
			break;

		case SPACE2_STAK:
				strcat ( file, "SPACE2.STK" );				
			break;

		case SPACE3_STAK:
				strcat ( file, "SPACE3.STK" );				
			break;

		case SUB1_STAK:
				strcat ( file, "SUB1.STK" );				
			break;

		case SUB2_STAK:
				strcat ( file, "SUB2.STK" );				
			break;

		case SUB3_STAK:
				strcat ( file, "SUB3.STK" );				
			break;

		case HALLOWEEN1_STAK:
				strcat ( file, "HALLOWEEN1.STK" );				
			break;

		case HALLOWEEN2_STAK:
				strcat ( file, "HALLOWEEN2.STK" );				
			break;

		case HALLOWEEN3_STAK:
				strcat ( file, "HALLOWEEN3.STK" );				
			break;

		case LAB1_STAK:
				strcat ( file, "LAB1.STK" );				
			break;

		case LAB2_STAK:
				strcat ( file, "LAB2.STK" );				
			break;

		case LAB3_STAK:
				strcat ( file, "LAB3.STK" );				
			break;

		case RETRO1_STAK:
				strcat ( file, "RETRO1.STK" );				
			break;
		case RETRO2_STAK:
				strcat ( file, "RETRO2.STK" );				
			break;
		case RETRO3_STAK:
				strcat ( file, "RETRO3.STK" );				
			break;
		case RETRO4_STAK:
				strcat ( file, "RETRO4.STK" );				
			break;
		case RETRO5_STAK:
				strcat ( file, "RETRO5.STK" );				
			break;
		case RETRO6_STAK:
				strcat ( file, "RETRO6.STK" );				
			break;
		case RETRO7_STAK:
				strcat ( file, "RETRO7.STK" );				
			break;
		case RETRO8_STAK:
				strcat ( file, "RETRO8.STK" );				
			break;
		case RETRO9_STAK:
				strcat ( file, "RETRO9.STK" );				
			break;
		case RETRO10_STAK:
				strcat ( file, "RETRO10.STK" );				
			break;
		case RETRO11_STAK:
				strcat ( file, "RETRO11.STK" );				
			break;
		case RETRO12_STAK:
				strcat ( file, "RETRO12.STK" );				
			break;
		case RETRO13_STAK:
				strcat ( file, "RETRO13.STK" );				
			break;
		case RETRO14_STAK:
				strcat ( file, "RETRO14.STK" );				
			break;

		case ANCIENTSMULTI_STAK:
				strcat ( file, "ANCIENTSMULTI.STK" );				
			break;

		case CITYMULTI_STAK:
				strcat ( file, "CITYMULTI.STK" );				
			break;

		case LABMULTI1_STAK:
				strcat ( file, "LABMULTI1.STK" );				
			break;
		case LABMULTI2_STAK:
				strcat ( file, "LABMULTI2.STK" );				
			break;
		case LABMULTI3_STAK:
				strcat ( file, "LABMULTI3.STK" );				
			break;

		case RETROMULTI1_STAK:
				strcat ( file, "RETROMULTI1.STK" );				
			break;
		case RETROMULTI2_STAK:
				strcat ( file, "RETROMULTI2.STK" );				
			break;
		case RETROMULTI3_STAK:
				strcat ( file, "RETROMULTI3.STK" );				
			break;
		case RETROMULTI4_STAK:
				strcat ( file, "RETROMULTI4.STK" );				
			break;
	}
	// ENDSWITCH - collBank

	if ( stakFiles [ numStakFiles ].stakFile )
		FREE ( stakFiles [ numStakFiles ].stakFile );

	stakFiles [ numStakFiles++ ].stakFile = (char*)fileLoad ( file, &fileLength);
//#ifdef PSX_VERSION
//#if PAL_MODE==1
//#if DECRYPTION==1
//#if GOLDCD==1
//	if(stakBank == FRONTEND1_STAK)
//	{
//		Decrypt(stakFiles[numStakFiles - 1].stakFile);
//	}
/*
#else
	if(stakBank == FRONTEND1_STAK)
	{
		for(i = 0;i < 2048;i++)
		{
			stakFiles[numStakFiles - 1].stakFile[i] = Random(256);
		}
	}
*/
//#endif
//#endif
//#endif
//#endif
}


void FreeStakFiles ( void )
{
	int c = 0;

	for ( c = 0; c < NUM_STAKFILES; c++ )
	{
		if ( stakFiles [ c ].stakFile )
			FREE ( stakFiles [ c ].stakFile );

		stakFiles [ c ].stakFile = NULL;
	}
	// ENDIF

	numStakFiles = 0;

}