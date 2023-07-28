#include <islmem.h>
#include <islfile.h>

#include "Shell.h"
#include "SonyLibs.h"

#include "Frontend.h"

#include "BackDrop.h"

BACKDROP backDrop;

extern USHORT EXPLORE_black_CLUT;

void FreeBackdrop ( void );

#define ScreenGetBuffer()			( (currentDisplayPage==displayPage)?0:1 )

void InitBackdrop ( char * filename )
{
	char newFileName[64];

	FreeBackdrop();
	if (backDrop.init)
		return;

	backDrop.init = 1;

#if PALMODE==1
	backDrop.rect.x = 512;			// leave the y till the update
	backDrop.rect.y = 256;
	backDrop.rect.w = 512; 
	backDrop.rect.h = 256;
#else
	backDrop.rect.x = 512;			// leave the y till the update
	backDrop.rect.y = 256+8;
	backDrop.rect.w = 512; 
	backDrop.rect.h = 256-16;
#endif

	/*DrawSync(0);
	ClearImage2(&backDrop.rect, 0,0,0);
	DrawSync(0);
	backDrop.rect.y = 256;
	DrawSync(0);
	ClearImage2(&backDrop.rect, 0,0,0);
	DrawSync(0);*/


	sprintf ( newFileName, "%s.FLA", filename );
	backDrop.data = fileLoad ( newFileName, NULL );

	DrawSync(0);
	LoadImage ( &backDrop.rect, (ULONG *) backDrop.data );
	DrawSync(0);

	backDrop.rect.y = 0;
	// set the drawing clscols to NOT clearscreen
//	INTERUPT_ScreenClear(INTERUPT_CLEAR_OFF);
}

void DrawBackDrop ( int execute, int drawBothScreens )
{
	displayPageType *targetpage;

	DR_MOVE *si = (void *)currentDisplayPage->primPtr;


#if PALMODE==1
	static RECT rect = {512,256,512,256};
#else
	static RECT rect = {512,256+8,512,256-16};
#endif

	targetpage = (currentDisplayPage==displayPage)?(&displayPage[1]):(&displayPage[0]);

	if ( ( backDrop.init ) && ( backDrop.data ) )
	{
		if ( execute )
		{
 			currentDisplayPage = (currentDisplayPage==displayPage)?(&displayPage[1]):(&displayPage[0]);
//			currentDisplayPage = displayPage;
			ClearOTagR(currentDisplayPage->ot, 1024);
			currentDisplayPage->primPtr = currentDisplayPage->primBuffer;

			gte_SetRotMatrix(&GsWSMATRIX);
			gte_SetTransMatrix(&GsWSMATRIX);

 			DrawSync(0);
 			VSync(2);
			PutDispEnv(&currentDisplayPage->dispenv);
 			PutDrawEnv(&currentDisplayPage->drawenv);
 			DrawOTag(currentDisplayPage->ot+(1024-1));

		}
		// ENDIF


		if(drawBothScreens >= 0)
		{
			BEGINPRIM(si, DR_MOVE);
		
			SetDrawMove ( si, &rect, 0, targetpage->dispenv.disp.y );

			ENDPRIM(si, 500, DR_MOVE);	// 0 or 1023
		}

		if ( drawBothScreens )
		{
			BEGINPRIM(si, DR_MOVE);

			SetDrawMove ( si, &rect, 0, targetpage->dispenv.disp.y+256 );

			ENDPRIM(si, 500, DR_MOVE);	// 0 or 1023
		}
		// ENDIF

		if ( execute )
		{
 			currentDisplayPage = (currentDisplayPage==displayPage)?(&displayPage[1]):(&displayPage[0]);
 			ClearOTagR(currentDisplayPage->ot, 1024);
 			currentDisplayPage->primPtr = currentDisplayPage->primBuffer;


 			DrawSync(0);
 			VSync(2);
 		/*	PutDispEnv(&currentDisplayPage->dispenv);
 			PutDrawEnv(&currentDisplayPage->drawenv);
 			DrawOTag(currentDisplayPage->ot+(1024-1));*/
		}
		// ENDIF
	}
	// ENIDF

}

void FreeBackdrop ( void )
{
	//utilPrintf("Freeing Backdrop, naughty........................\n");
	backDrop.init = 0;

	FREE(backDrop.data);

	backDrop.data = NULL;

//	displayPage[0].drawenv.isbg = displayPage[1].drawenv.isbg = 1;
}