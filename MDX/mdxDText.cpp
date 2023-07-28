/*

	This file is part of the M libraries,

	File		: 
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <math.h>
#include "mgeReport.h"
#include "mdxDDraw.h"
#include "mdxD3D.h"
#include "mdxInfo.h"
#include "mdxTiming.h"
#include "mdxCRC.h"
#include "mdxTexture.h"
#include "mdxMath.h"
#include "mdxObject.h"
#include "mdxActor.h"
#include "mdxLandscape.h"
#include "mdxRender.h"
#include "mdxPoly.h"
#include "mdxDText.h"
#include "mdxProfile.h"
#include "mdxWindows.h"
#include "gelf.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_CONSOLE_SIZE 12

char consoleText[MAX_CONSOLE_SIZE][256];
char curConsoleInput[256] = "";
char oldConsoleInput[256] = "";
unsigned long startPointer = 0;
unsigned long numText = 0;

/*  -------------------------------------------------------------------------------
	Function	: cp
	Purpose		:
	Parameters	: (char *format, ...)
	Returns		: void
	Info		:
*/

void cp(char *format, ...)
{
	va_list			argp;

	static char		buffer[1024];

	if (strlen(format)==0)
		strcpy(buffer,"");
	else
	{
		va_start(argp, format);
		vsprintf(buffer, format, argp);
		va_end(argp);
	}

	strcpy(consoleText[(startPointer+numText)%MAX_CONSOLE_SIZE],buffer);
	
	if (numText<MAX_CONSOLE_SIZE)
		numText++;
	else
		startPointer = ((startPointer+1)%MAX_CONSOLE_SIZE);	
}

enum 
{
	CMD_SET = 0,
	CMD_CLS,
	CMD_EXIT,
	CMD_SHOWOBJECTS,
	CMD_NUMCOMMANDS,
};

char *commandNames[] = 
{
	"set",
	"cls",
	"exit",
	"showobjects",
};

typedef struct TAG_SETDATA
{
	char *name;
	void *data;
	char *type;
} SETDATA;

SETDATA setInfo[] = 
{
	"lighting",&drawLighting,"%lu",
	"phong",&drawPhong,"%lu",
	"numobjects",&numObjDrawn,"%lu",
	"speed",&(timeInfo.speed),"%f",
	NULL,NULL,NULL
};

void ProcessCommand(void)
{
	char command[255];
	char *val1,*val2;
	long i,j,c,length = strlen(curConsoleInput);
	long numSpaces;

	strcpy (command,curConsoleInput);
	if (length)
	{
		numSpaces = 0;
		
		for (i=0; i<length; i++)
			if (command[i]==' ')
			{
				command[i] = 0;
				numSpaces++;
			}
		
		strlwr(command);

		for (i=0; i<CMD_NUMCOMMANDS; i++)
			if (strcmp(command,commandNames[i])==0)
				c = i;
		
		switch(c)
		{
			case CMD_SET:
				if (numSpaces)
				{
					val1 = &command[strlen(command)+1];
					char fString[255] = "%s = ";

					i=0;
					j = -1;

					while (setInfo[i].name!=NULL)
					{
						if (strcmp(setInfo[i].name,val1)==0)
							j = i;
						i++;
					}
					
					if (j==-1)
					{
						cp("Unrecognised Variable [%s]",val1);
						break;
					}

					if (numSpaces==2)
					{
						val2 = &val1[strlen(val1)+1];
						sscanf(val2,setInfo[j].type,setInfo[j].data);
					}

					float mf;
					long ml;
					strcat(fString,setInfo[j].type);
					switch (setInfo[j].type[1])
					{
						case 'f':
							mf = *(float *)setInfo[j].data;
							cp(fString,val1,mf);	
							break;
						case 'l':
							ml = *(long *)setInfo[j].data;
							cp(fString,val1,ml);	
							break;								
					}						
					
				}
				else
				{
					cp("You can set: ");
					i=0;
					
					while (setInfo[i].name!=NULL)
						cp("%s",setInfo[i++].name);							
						
				}
				break;
			case CMD_CLS:
				startPointer = 0;
				numText = 0;				
				break;
			case CMD_EXIT:
				PostQuitMessage(0);
				break;
			case CMD_SHOWOBJECTS:
				{
					char tString[150] = "";
					MDX_ACTOR *cur = actorList;
					while (cur)
					{
						strcat(tString,(const char *)cur->objectController->object->name);
						strcat(tString," : ");
						if (strlen(tString)>100)
						{
							cp("%s",tString);
							strcpy(tString,"");
						}
						cur = cur->next;
					}
				}
				break;
			
			default:
				cp("Unrecognised command.");
				break;
		}

		
	}

	cp("> ");
}
/*	--------------------------------------------------------------------------------
	Function	: 
	Purpose		: 
	Parameters	: 
	Returns		: 
	Info		: 
*/

void UpdateConsole(int charCode)
{
	unsigned long len;
	switch (charCode)
	{
		case VK_ESCAPE:
			strcpy (curConsoleInput,"");
			break;
		case VK_BACK:
			len = strlen(curConsoleInput);			
			curConsoleInput[len-1] = 0;
			break;
		case VK_F3:
			strcpy (curConsoleInput,oldConsoleInput);			
			break;
		case VK_RETURN:
			ProcessCommand();			
			strcpy (oldConsoleInput,curConsoleInput);
			strcpy (curConsoleInput,"");
			break;
		default:
			len = strlen(curConsoleInput);
			curConsoleInput[len] = (char)charCode;
			curConsoleInput[len+1] = 0;
			break;
	}
	strcpy(consoleText[(startPointer+numText-1)%MAX_CONSOLE_SIZE],"> ");
	strcat(consoleText[(startPointer+numText-1)%MAX_CONSOLE_SIZE],curConsoleInput);
}

/*	--------------------------------------------------------------------------------
	Function	: 
	Purpose		: 
	Parameters	: 
	Returns		: 
	Info		: 
*/

void PrintConsole(void)
{
	static LPDIRECTDRAWSURFACE7 tSurface = NULL;

	if (!tSurface)
	{
		MDX_TEXENTRY *t;
		t = GetTexEntryFromCRC(UpdateCRC("console.bmp"));
		if (t)
			tSurface = t->surf;
	}

	RECT r;
	r.top  = 1;
	r.left = 1;
	r.bottom  = 5+16*MAX_CONSOLE_SIZE;
	r.right = 639;
	
	pDirect3DDevice->BeginScene();

	DrawTexturedRect(r,D3DRGBA(1,1,1,0.5),tSurface,FULL_TEXTURE);
	r.top  = 2+16*MAX_CONSOLE_SIZE;
	DrawFlatRect(r,D3DRGBA(0.8,1,0.8,0.5));
	
	pDirect3DDevice->EndScene();

	HDC hdc;
	HRESULT res = IDirectDrawSurface4_GetDC(surface[RENDER_SRF], &hdc);
					
	if (res == DD_OK)
	{
		HFONT hfnt, hOldFont;      
		hfnt = (HFONT)GetStockObject(ANSI_VAR_FONT); 
		if (hOldFont = (HFONT)SelectObject(hdc, hfnt)) 
		{
			for (int i=0; i<numText; i++)
			{		

				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, RGB(0,25,0));
				TextOut(hdc,11,1+i*16, consoleText[(startPointer+i)%MAX_CONSOLE_SIZE], strlen(consoleText[(startPointer+i)%MAX_CONSOLE_SIZE]));

				SetTextColor(hdc, RGB(200,225,200));
				TextOut(hdc,10,i*16, consoleText[(startPointer+i)%MAX_CONSOLE_SIZE], strlen(consoleText[(startPointer+i)%MAX_CONSOLE_SIZE]));
			}
			SelectObject(hdc, hOldFont); 
		}
		IDirectDrawSurface4_ReleaseDC(surface[RENDER_SRF], hdc);
	}
}

#ifdef __cplusplus
}
#endif

		
