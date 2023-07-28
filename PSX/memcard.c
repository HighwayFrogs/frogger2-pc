#include "map_draw.h"
#include "world_eff.h"
#include <islpad.h>
#include <isltex.h>
#include <islcard.h>
#include "main.h"
#include "layout.h"
#include "frogger.h"
#include "savegame.h"
#include "water.h"
#include "actor2.h"
#include "temp_psx.h"
#include "cam.h"
#include "textures.h"
#include "map_draw.h"
#include "timer.h"
#include "memcard.h"
#include "lang.h"
#include "audio.h"
#include <Libmcrd.h>

#include "game.h"
#include "menus.h"
#include "options.h"


#define MIN min
#define MAX max

extern TextureType	*buttonSprites[6];

int framePause = 0;
static int	optChosen, optFrame, optSaveAlready, optSaveSlot;
static char	optStr[256];

extern SAVE_INFO saveInfo;
extern int useMemCard;
char memmessage[256];
char slotNumStr[4];


enum {
	SAVEMENU_CHECK,
	SAVEMENU_FULL,
	SAVEMENU_NOCARD,
	SAVEMENU_SAVE,
	SAVEMENU_COMPLETE,
	SAVEMENU_FAIL,
	SAVEMENU_FORMATYN,
	SAVEMENU_FORMAT,
	SAVEMENU_FORMATFAIL,
	SAVEMENU_NEEDFORMAT,
	SAVEMENU_LOAD,
	SAVEMENU_LOADERROR,
	SAVEMENU_LOADCOMPLETE,
	SAVEMENU_SAVEYN,
	SAVEMENU_CHANGED,
	SAVEMENU_OVERWRITE,
	SAVEMENU_FORMATCOMPLETE,
};


int waitCheck;
int delayTimer;
int cardChanged = NO;
#define DELAY_TIME 180

/* SAVE PAGE ***********************************************************************************************************/


static void saveMenuSave()
{
	if(gameState.mode == LEVELCOMPLETE_MODE)
		SimpleMessage(GAMESTRING(STR_MCARD_AUTOSAVING), MIN(128,optFrame*43));
	else
		SimpleMessage(GAMESTRING(STR_MCARD_SAVING), MIN(128,optFrame*43));
	if (optFrame++>4)
	{
		if (gameSaveHandleSave() != CARDWRITE_OK)
			saveInfo.saveStage = SAVEMENU_FAIL;
		else
		{
			if(gameState.mode == LEVELCOMPLETE_MODE)
			{
				saveInfo.saveFrame = 0;
				cardChanged = NO;
			}
			else
				saveInfo.saveStage = SAVEMENU_COMPLETE;
			delayTimer = 0;
		}
		StartChooseOption();
	}
}

int gameSaveGetCardStatus()
{
	long	cmds, result;

	switch(MemCardSync(1, &cmds, &result))
	{
	case -1:	// Start off status request
		MemCardExist(0);
		break;
	case 1:		// Return status result
		if (cmds==McFuncExist)
			return result;
		break;
	}
	return -1;
}

#if PALMODE==1
#define SAVE_FILENAME "BESLES-02853FROGGER2"
#else
#define SAVE_FILENAME "BASLUS-01172FROGGER2"
#endif

int gameSaveHandleSave()
{
	void *info;
	int res;
	unsigned long size;

	MakeSaveGameBlock(&info, &size);
	utilPrintf("Saving %d\n", size);
	res = cardWrite(SAVE_FILENAME, info, size);

	switch(res)
	{
		case CARDWRITE_OK:
		{
			utilPrintf("No errors\n");
			break;
		}
		case CARDWRITE_NOCARD:
		{
			utilPrintf("No card in slot\n");
			break;
		}
		case CARDWRITE_BADCARD:
		{
			utilPrintf("Bad card in slot\n");
			break;
		}
		case CARDWRITE_NOTFORMATTED:
		{
			utilPrintf("Card unformatted\n");
			break;
		}
		case CARDWRITE_FULL:
		{
			utilPrintf("Card is full\n");
			break;
		}
	} //end 


	FREE(info);

	return res;
}

int gameSaveHandleLoad(int justChecking)
{
	void *info;
	int size;
	int res;

	if(justChecking)
	{
		utilPrintf("Checking Save Game\n");
		res = cardRead(SAVE_FILENAME, 0, SAVEGAME_SIZE+PSXCARDHEADER_SIZE); //checking, not loading
		switch(res)
		{
			case CARDREAD_OK:
			{
				utilPrintf("No errors\n");
				break;
			}
			case CARDREAD_NOCARD:
			{
				utilPrintf("No card in slot\n");
				break;
			}
			case CARDREAD_BADCARD:
			{
				utilPrintf("Bad card in slot\n");
				break;
			}
			case CARDREAD_NOTFORMATTED:
			{
				utilPrintf("Card unformatted\n");
				break;
			}
			case CARDREAD_NOTFOUND:
			{
				utilPrintf("No game save data found\n");
				break;
			}
			case CARDREAD_CORRUPT:
			{
				utilPrintf("Game save data corrupted\n");
				break;
			}
			case CARDREAD_NOTFOUNDANDFULL:
			{
				utilPrintf("No game save data found and card is full\n");
				break;
			}
		} //end 


		return res;
	}

	//make buffer
	info = MALLOC0(SAVEGAME_SIZE+PSXCARDHEADER_SIZE);
	if(!info)
	{
		utilPrintf("\n\nMALLOC ERROR DURING LOAD\n\n");
		return;
	}

	utilPrintf("Loading Save Game %d\n", SAVEGAME_SIZE+PSXCARDHEADER_SIZE);
	res = cardRead(SAVE_FILENAME, info, SAVEGAME_SIZE+PSXCARDHEADER_SIZE);

	//bb - skip over PsxCardHeader
	//NO - responsibility of updated LoadSaveGameBlock (like MakeSaveGameBlock).

	switch(res)
	{
		case CARDREAD_OK:
		{
			utilPrintf("No errors\n");
			LoadSaveGameBlock(info, size);
			break;
		}
		case CARDREAD_NOCARD:
		{
			utilPrintf("No card in slot\n");
			break;
		}
		case CARDREAD_BADCARD:
		{
			utilPrintf("Bad card in slot\n");
			break;
		}
		case CARDREAD_NOTFORMATTED:
		{
			utilPrintf("Card unformatted\n");
			break;
		}
		case CARDREAD_NOTFOUND:
		{
			utilPrintf("No game save data found\n");
			break;
		}
		case CARDREAD_CORRUPT:
		{
			utilPrintf("Game save data corrupted\n");
			break;
		}
		case CARDREAD_NOTFOUNDANDFULL:
		{
			utilPrintf("No game save data found and card is full\n");
			break;
		}

	} //end 

	//free buffer
	FREE(info);

	return res;
}

void StartChooseOption()
{
	optFrame = 1;
	optChosen = 0;
}


int FontInSpace(int x, int y, char *str, int w, int l, uchar r, uchar g, uchar b)
{
	char	buf[200], *bufPtr;
	int		loop, numLines;


	sprintf(memmessage,str,slotNumStr);

	numLines = fontFitToWidth(fontSmall, w, memmessage, buf);
	if (numLines<l)
		y += (l-numLines)*6;
	bufPtr = buf;
	for(loop=0; loop<numLines; loop++)
	{
		fontPrintScaled(fontSmall, x,y, bufPtr, r,g,b,4096);
		bufPtr += strlen(bufPtr)+1;
		y += 12;
	}
	return numLines;
}

void SimpleMessage(char *msg, uchar rgb)
{
	char	*bufPtr;
	int		y, loop, numLines;
	POLY_F4	*f4;
	DR_MODE	*dm;


	sprintf(memmessage,msg,slotNumStr);

	numLines = fontFitToWidth(fontSmall, 420, memmessage, optStr);
	bufPtr = optStr;
	y = max(-100,-67-numLines*6);

	BEGINPRIM(f4, POLY_F4);
	setXYWH(f4, -256,y-7, 512,numLines*12+15);
	setRGB0(f4, rgb/2,rgb/2,rgb/2);
	setPolyF4(f4);
	setSemiTrans(f4, 1);
	ENDPRIM(f4, 5, POLY_F4);
	BEGINPRIM(dm, DR_MODE);
	SetDrawMode(dm, 0,1, (SEMITRANS_SUB-1)<<5,0);
	ENDPRIM(dm, 5, DR_MODE);

	for(loop=0; loop<numLines; loop++)
	{
		fontPrintScaled(fontSmall, -fontExtentWScaled(fontSmall, bufPtr,4096)/2,y, bufPtr, rgb,rgb,rgb,4096);
		fontSmall->alpha = 3;
		fontPrintScaled(fontSmall, -fontExtentWScaled(fontSmall, bufPtr,4096)/2,y+2, bufPtr, rgb,rgb,rgb,4096);
		fontSmall->alpha = 2;
		bufPtr += strlen(bufPtr)+1;
		y += 12;
	}
}


int ChooseOption(char *msg, char *msg1, char *msg2)
{
	int		x,y, f[2], f2, loop, p,numLines = 0;
	uchar	r[4],g[4],b[4];
	POLY_F4	*f4;
	DR_MODE	*dm;


	framePause++;
	f2 = optFrame<<5;
	for(loop=0; loop<4; loop++)
	{
		r[loop] = MIN(f2,100); g[loop] = MIN(f2,25); b[loop] = 0;
	}
	switch(optChosen)
	{
	case 0:		// Fade up
		f[0] = (optFrame<<5)-0;
		f[1] = MAX((optFrame<<4)-50,0);
		optFrame++;
		break;
	case 2:		// Fade down except 2/circle
		f2 = MAX(optFrame-10,0);
		r[1] = 100; g[1] = 25; b[1] = 0;
		f[0] = (f2<<4)-0;
		f[1] = MAX((f2<<4)-50,0);
		optFrame--;
		break;
	case 3:		// Fade down except 3/cross
		f2 = MAX(optFrame-10,0);
		r[2] = 100; g[2] = 25; b[2] = 0;
		f[0] = (f2<<4)-0;
		f[1] = MAX((f2<<4)-50,0);
		optFrame--;
		break;
	}

	f2 = MIN(f[1],128);
	SimpleMessage(msg, f2);

	x = -160;
	y = -77+38*2;

	if ((p = ((optFrame<<2) & 63))>31)
		p = 31-(p-31);
	if (msg2!=NULL)
	{
		f2 = MIN(f[1],127);
		FontInSpace(x,y, msg2, 380,2, f2,f2,f2);		// 2/triangle
		fontSmall->alpha = 3;
		FontInSpace(x,y+2, msg2, 380,2, f2,f2,f2);		// 2/triangle
		fontSmall->alpha = 2;
		fontDispSprite(buttonSprites[0], x-50,y+1, f2*2,f2*2,f2*2,2);
		if(f2 > 120)
			fontDispSprite(buttonSprites[0], x-50,y+3, 0,0,0,1);
	}
	y += 48;

	f2 = MIN(f[1],127);
	numLines = FontInSpace(x,y, msg1, 380,2, f2,f2,f2);		// 3/Cross
	fontSmall->alpha = 3;
	numLines = FontInSpace(x,y+2, msg1, 380,2, f2,f2,f2);		// 3/Cross
	fontSmall->alpha = 2;
	fontDispSprite(buttonSprites[2], x-50,y+1, f2*2,f2*2,f2*2, 2);
	if(f2 > 120)
		fontDispSprite(buttonSprites[2], x-50,y+3, 0,0,0, 1);
	y += 40;
												// Help buttons
	BEGINPRIM(f4, POLY_F4);
	if(msg2)
	{
		setXYWH(f4, -256,-77+38*2-7, 512,numLines*12 + 48 + 20);
	}
	else
	{
		setXYWH(f4, -256,-77+38*2-7+48+5, 512,numLines*12 + 15);
	}
	setRGB0(f4, f2/2,f2/2,f2/2);
	setPolyF4(f4);
	setSemiTrans(f4, 1);
	ENDPRIM(f4, 5, POLY_F4);
	BEGINPRIM(dm, DR_MODE);
	SetDrawMode(dm, 0,1, (SEMITRANS_SUB-1)<<5,0);
	ENDPRIM(dm, 5, DR_MODE);

/*
	if (!optChosen)
	{
		x = -14*(1+(msg2!=NULL))-fontExtentWScaled(fontSmall, GAMESTRING(STR_SELECTOPTION),4096)/2-5;
		if (msg2!=NULL)
		{
			fontDispSprite(buttonSprites[0], x,y-3, 255,255,255, 2);
			fontDispSprite(buttonSprites[0], x,y-1, 0,0,0, 1);
			x += 28;
		}
		fontDispSprite(buttonSprites[2], x,y-3, 255,255,255, 2);
		fontDispSprite(buttonSprites[2], x,y-1, 0,0,0, 1);
		x += 28;
		x += 10;
		fontPrintScaled(fontSmall, x,y, GAMESTRING(STR_SELECTOPTION), 128,128,128,4096);
		fontSmall->alpha = 3;
		fontPrintScaled(fontSmall, x,y+2, GAMESTRING(STR_SELECTOPTION), 128,128,128,4096);
		fontSmall->alpha = 2;
	}
*/

	if (!optChosen)								// Make selection
	{
		if (optFrame>20)
		{
			if ((msg2!=NULL) && (padData.debounce[0] & PAD_TRIANGLE))
			{
				optChosen = 2;
				optFrame = 25;
				PlaySample(genSfx[GEN_SUPER_HOP],NULL,0,SAMPLE_VOLUME,-1);
			}
			if (padData.debounce[0] & PAD_CROSS)
			{
				optChosen = 3;
				optFrame = 25;
				PlaySample(genSfx[GEN_SUPER_HOP],NULL,0,SAMPLE_VOLUME,-1);
			}
		}
	}
	else
	{
		if (optFrame==1)						// Fade complete, save and/or continue
		{
			return optChosen;
			optFrame = 0;
		}
	}

	return 0;
}

void StartChooseLoadSave(int load)
{
/*	if (XAgetStatus())
	{
		if(XAcheckPlay())
			CdControl( CdlStop, NULL, NULL );
	}*/
	saveInfo.saveFrame = 1;
	saveInfo.saveChosen = 0;
	saveInfo.saveStage = SAVEMENU_CHECK;
	saveInfo.load = load;
	framePause++;
	StartChooseOption();
	if(padData.numPads[0] == 4)
		sprintf(slotNumStr,"1-A");
	else
		sprintf(slotNumStr,"1");
	waitCheck = 0;
}



/* FULL CARD PAGE ******************************************************************************************************/

static void saveMenuFull()
{
	switch(ChooseOption(GAMESTRING(STR_MCARD_FULL), GAMESTRING(STR_MCARD_RECHECK), GAMESTRING(STR_MCARD_PROCEED_WITHOUT_SAVE)))
	{
	case 3:
		saveInfo.saveStage = SAVEMENU_CHECK;
		StartChooseOption();
		break;
	case 2:
		saveInfo.saveFrame = 0;
		useMemCard = 0;
		break;
	}
	switch(gameSaveGetCardStatus())
	{
   	case McErrCardNotExist:
   	case McErrCardInvalid:
		saveInfo.saveStage = SAVEMENU_NOCARD;
		StartChooseOption();
		break;
	}
}

/* NO CARD PAGE ********************************************************************************************************/

int memtemp = 1;
static void saveMenuNoCard()
{
	int choice;

	choice = ChooseOption(GAMESTRING(STR_MCARD_NOCARD), GAMESTRING(STR_MCARD_RECHECK), GAMESTRING(STR_MCARD_PROCEED_WITHOUT_SAVE));
	switch(choice)
	{
	case 3:
		saveInfo.saveStage = SAVEMENU_CHECK;
		StartChooseOption();
		break;
	case 2:
		saveInfo.saveFrame = 0;
		useMemCard = 0;
		break;
	}
/*
	switch(gameSaveGetCardStatus())
	{
   	case McErrNone:
   	case McErrNewCard:
		saveInfo.saveStage = SAVEMENU_CHECK;
		StartChooseOption();
		break;
	}
*/
}

static void saveMenuLoad()
{
	SimpleMessage(GAMESTRING(STR_MCARD_LOADING), MIN(128,optFrame*43));
	if (optFrame++>4)
	{
		if (gameSaveHandleLoad(NO) != CARDREAD_OK)
		{
			saveInfo.saveStage = SAVEMENU_LOADERROR;
		}
		else
		{
			saveInfo.saveStage = SAVEMENU_LOADCOMPLETE;
			delayTimer = 0;
		}
		StartChooseOption();
	}
}

/* CHECK CARD PAGE *****************************************************************************************************/
static void saveMenuCheck()
{
	int res;

	SimpleMessage(GAMESTRING(STR_MCARD_CHECK), MIN(128,optFrame*43));
	if (optFrame++>4)
	{
		switch(gameSaveHandleLoad(YES))
		{
		case CARDREAD_OK:						// Loaded fine
		case CARDREAD_CORRUPT:
			optSaveAlready = 1;
			if(saveInfo.load)
			{
				saveInfo.saveStage = SAVEMENU_LOAD;
				StartChooseOption();
			}
			else if(cardChanged)
			{
				saveInfo.saveStage = SAVEMENU_OVERWRITE;
				StartChooseOption();
			}
			else
			{
				saveInfo.saveStage = SAVEMENU_SAVE;
				StartChooseOption();
			}
			break;
		case CARDREAD_NOTFOUND:		// Data not found, space on card
//		case -2:					// Corrupt data
			optSaveAlready = 0;
			saveInfo.saveStage = SAVEMENU_SAVEYN;
			StartChooseOption();
			break;
   		case CARDREAD_NOCARD:		// No card
   		case CARDREAD_BADCARD:		// Bad card
			saveInfo.saveStage = SAVEMENU_NOCARD;
			StartChooseOption();
			break;
		case CARDREAD_NOTFOUNDANDFULL:					// Data not found, full card
			saveInfo.saveStage = SAVEMENU_FULL;
			StartChooseOption();
			break;
		case CARDREAD_NOTFORMATTED: 		// Card not formatted
			saveInfo.saveStage = SAVEMENU_FORMATYN;
			StartChooseOption();
			break;
//		case CARDREAD_CORRUPT:
			//if(saveInfo.load)
			//{
//				saveInfo.saveStage = SAVEMENU_LOAD;
				//StartChooseOption();
			//}
			//break;
		}
	}
}

/* SAVE COMPLETE PAGE **************************************************************************************************/

static void saveMenuComplete()
{
//	if (((options.mode == -1) && (delayTimer++ > DELAY_TIME)) || (ChooseOption(GAMESTRING(STR_MCARD_COMPLETE), GAMESTRING(STR_MCARD_CONTINUE), NULL)))
	if (ChooseOption(GAMESTRING(STR_MCARD_COMPLETE), GAMESTRING(STR_MCARD_CONTINUE), NULL))
	{
		saveInfo.saveFrame = 0;
		cardChanged = NO;
	}
}

static void saveMenuLoadComplete()
{
	SimpleMessage(GAMESTRING(STR_MCARD_LOADCOMPLETE), MIN(128,optFrame++*43));
	if(delayTimer++ > DELAY_TIME)
		saveInfo.saveFrame = 0;

//	if (/*(delayTimer++ > DELAY_TIME) || */(ChooseOption(GAMESTRING(STR_MCARD_LOADCOMPLETE), GAMESTRING(STR_MCARD_CONTINUE), NULL)))
//		saveInfo.saveFrame = 0;
}

static void saveMenuFormatComplete()
{
	if (/*(delayTimer++ > DELAY_TIME) ||*/ (ChooseOption(GAMESTRING(STR_MCARD_FORMATCOMPLETE), GAMESTRING(STR_MCARD_CONTINUE), NULL)))
	{
		saveInfo.saveStage = SAVEMENU_SAVEYN;
		StartChooseOption();
	}
}

static void saveMenuLoadError()
{
	switch(ChooseOption(GAMESTRING(STR_MCARD_LOADERROR), GAMESTRING(STR_MCARD_RECHECK), GAMESTRING(STR_MCARD_PROCEED_WITHOUT_SAVE)))
	{
	case 3:
		saveInfo.saveStage = SAVEMENU_CHECK;
		StartChooseOption();
		break;
	case 2:
		saveInfo.saveFrame = 0;
		useMemCard = 0;
		break;
	}
}

/* SAVE FAIL PAGE ******************************************************************************************************/

static void saveMenuFail()
{
	if (ChooseOption(GAMESTRING(STR_MCARD_SAVEERROR), GAMESTRING(STR_MCARD_RETRYSAVE), NULL))
	{
		saveInfo.saveStage = SAVEMENU_CHECK;
		StartChooseOption();
	}
/*	switch(gameSaveGetCardStatus())
	{
   	case McErrCardNotExist:
   	case McErrCardInvalid:
		saveInfo.saveStage = SAVEMENU_NOCARD;
		StartChooseOption();
		break;
	}
*/
}

static void saveMenuOverwrite()
{
	switch(ChooseOption(GAMESTRING(STR_MCARD_OVERWRITE), GAMESTRING(STR_YES), GAMESTRING(STR_NO)))
	{
	case 3:
		saveInfo.saveStage = SAVEMENU_SAVE;
		StartChooseOption();
		break;
	case 2:
		saveInfo.saveStage = SAVEMENU_CHANGED;
		StartChooseOption();
		break;
	}
	switch(gameSaveGetCardStatus())
	{
   	case McErrCardNotExist:
   	case McErrCardInvalid:
		saveInfo.saveStage = SAVEMENU_NOCARD;
		StartChooseOption();
		break;
	}
}

/* FORMAT CONFIRMATION PAGE ********************************************************************************************/

static void saveMenuFormatYN()
{
	switch(ChooseOption(GAMESTRING(STR_MCARD_UNFORMAT), GAMESTRING(STR_MCARD_DONTFORMAT), GAMESTRING(STR_MCARD_FORMATCARD)))
	{
	case 2:
		saveInfo.saveStage = SAVEMENU_FORMAT;
		StartChooseOption();
		break;
	case 3:
		saveInfo.saveStage = SAVEMENU_NEEDFORMAT;
		StartChooseOption();
		break;
	}
	switch(gameSaveGetCardStatus())
	{
   	case McErrCardNotExist:
   	case McErrCardInvalid:
		saveInfo.saveStage = SAVEMENU_NOCARD;
		StartChooseOption();
		break;
	}
}


static void saveMenuChanged()
{
	switch(ChooseOption(GAMESTRING(STR_MCARD_CHANGED), GAMESTRING(STR_MCARD_CHECK_THIS_CARD), GAMESTRING(STR_MCARD_PROCEED_WITHOUT_SAVE)))
	{
	case 3:
		saveInfo.saveStage = SAVEMENU_CHECK;
		StartChooseOption();
		break;
	case 2:
		saveInfo.saveFrame = 0;
		useMemCard = 0;
		break;
	}
}


static void saveMenuSaveYN()
{
	switch(ChooseOption(GAMESTRING(STR_MCARD_NEEDS_ONE_BLOCK), GAMESTRING(STR_MCARD_CREATE_SAVE), GAMESTRING(STR_MCARD_PROCEED_WITHOUT_SAVE)))
	{
	case 3:
		saveInfo.saveStage = SAVEMENU_SAVE;
		StartChooseOption();
		break;
	case 2:
		saveInfo.saveFrame = 0;
		useMemCard = 0;
		break;
	}
	switch(gameSaveGetCardStatus())
	{
   	case McErrCardNotExist:
   	case McErrCardInvalid:
		saveInfo.saveStage = SAVEMENU_NOCARD;
		StartChooseOption();
		break;
	}
}

/* FORMAT PAGE *********************************************************************************************************/

static void saveMenuFormat()
{
	SimpleMessage(GAMESTRING(STR_MCARD_FORMATTING), MIN(128,optFrame*43));
	if (optFrame++>4)
	{
		if (cardFormat())
		{
			saveInfo.saveStage = SAVEMENU_FORMATFAIL;
			StartChooseOption();
		}
		else
		{
			saveInfo.saveStage = SAVEMENU_FORMATCOMPLETE;
			delayTimer = 0;
			StartChooseOption();
		}
	}
}

/* FORMAT FAILURE PAGE *************************************************************************************************/

static void saveMenuFormatFail()
{
	if (ChooseOption(GAMESTRING(STR_MCARD_FORMATERROR), GAMESTRING(STR_MCARD_RECHECK), NULL))
	{
		saveInfo.saveStage = SAVEMENU_CHECK;
		StartChooseOption();
	}
/*
	switch(gameSaveGetCardStatus())
	{
   	case McErrCardNotExist:
   	case McErrCardInvalid:
		saveInfo.saveStage = SAVEMENU_NOCARD;
		StartChooseOption();
		break;
	}
*/
}


void saveMenuNeedFormat()
{
	switch(ChooseOption(GAMESTRING(STR_MCARD_NEEDFORMAT), GAMESTRING(STR_MCARD_RECHECK), GAMESTRING(STR_MCARD_PROCEED_WITHOUT_SAVE)))
	{
	case 3:
		saveInfo.saveStage = SAVEMENU_CHECK;
		StartChooseOption();
		break;
	case 2:
		saveInfo.saveFrame = 0;
		useMemCard = 0;
		break;
	}
	switch(gameSaveGetCardStatus())
	{
   	case McErrCardNotExist:
   	case McErrCardInvalid:
		saveInfo.saveStage = SAVEMENU_NOCARD;
		StartChooseOption();
		break;
	}
}

int MEMWAIT = 10;
void ChooseLoadSave()
{
	int alpha = fontSmall->alpha;


	if(padData.numPads[0] == 4)
		sprintf(slotNumStr,"1-A");
	else
		sprintf(slotNumStr,"1");

	if((saveInfo.load == 0) && (saveInfo.saveStage != SAVEMENU_COMPLETE))
	{
		switch(gameSaveGetCardStatus())
		{
   		case McErrNewCard:
			if(cardChanged == NO)
			{
				saveInfo.saveStage = SAVEMENU_CHANGED;
				StartChooseOption();
				cardChanged = YES;
			}
			break;
		}
		VSync(0);
	}

	if(waitCheck < MEMWAIT)
	{
		waitCheck++;
		return;
	}
	fontSmall->alpha = 2;
	switch(saveInfo.saveStage)
	{
	case SAVEMENU_CHECK:
		saveMenuCheck();
		break;
	case SAVEMENU_FULL:
		saveMenuFull();
		break;
	case SAVEMENU_NOCARD:
		saveMenuNoCard();
		break;
	case SAVEMENU_SAVE:
		saveMenuSave();
		break;
	case SAVEMENU_COMPLETE:
		saveMenuComplete();
		break;
	case SAVEMENU_FAIL:
		saveMenuFail();
		break;
	case SAVEMENU_FORMATYN:
		saveMenuFormatYN();
		break;
	case SAVEMENU_FORMAT:
		saveMenuFormat();
		break;
	case SAVEMENU_FORMATFAIL:
		saveMenuFormatFail();
		break;
	case SAVEMENU_NEEDFORMAT:
		saveMenuNeedFormat();
		break;
	case SAVEMENU_LOAD:
		saveMenuLoad();
		break;
	case SAVEMENU_LOADERROR:
		saveMenuLoadError();
		break;
	case SAVEMENU_LOADCOMPLETE:
		saveMenuLoadComplete();
		break;
	case SAVEMENU_SAVEYN:
		saveMenuSaveYN();
		break;
	case SAVEMENU_CHANGED:
		saveMenuChanged();
		break;
	case SAVEMENU_OVERWRITE:
		saveMenuOverwrite();
		break;
	case SAVEMENU_FORMATCOMPLETE:
		saveMenuFormatComplete();
		break;	
	}
	fontSmall->alpha = alpha;
}

void SaveGame(void)
{
	if(useMemCard)
		StartChooseLoadSave(NO);
}

void LoadGame(void)
{
	if(useMemCard)
		StartChooseLoadSave(YES);
}


