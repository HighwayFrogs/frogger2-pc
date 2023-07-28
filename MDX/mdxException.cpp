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
#include "mdxFont.h"
#include "commctrl.h"
#include "gelf.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C"
{
#endif

int Eval_Exception (int n_except,LPEXCEPTION_POINTERS exceptinf)
{
	MSG msg;

	if (rFullscreen)
	{
		ShowWindow(mdxWinInfo.hWndMain,SW_SHOWMINIMIZED);

		while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}
		pDirectDraw7->SetCooperativeLevel(mdxWinInfo.hWndMain, DDSCL_NORMAL);
		pDirectDraw7->Release();
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

typedef struct TAG_MEMINFO
{
	char file[MAX_PATH];
	long line;

	void *data;
	long size;

	long number;
	long numprints;

	TAG_MEMINFO *next,*prev;
} MEMINFO;

MEMINFO *memList = NULL;
unsigned long numAllocs = 0;

void *Alloc_Mem(unsigned long size,char *file, long line)
{
	MEMINFO *nMem = new MEMINFO;
	nMem->prev = NULL;
	nMem->next = memList;
	if (memList)
		memList->prev = nMem;
	memList = nMem;


	nMem->data = (void *)new char[size];
	memset(nMem->data,0,size);
	nMem->size = size;
	nMem->numprints = 0;

	strcpy (nMem->file,file);
	nMem->line = line;
	
	nMem->number = numAllocs++;

	return nMem->data;
}

void Show_Mem(void)
{
	MEMINFO *cur = memList;
	while (cur)
	{
		dp("Memory Block number #%lu - Size:%lu   File:%s   Line:%lu - %lu prints   Data:0x%xl\n",cur->number,cur->size,cur->file,cur->line,cur->numprints++,cur->data);
		cur = cur->next;
	}
}

void Free_Mem(void *me,char *file, long line)
{
	MEMINFO *cur = memList;

	if (cur)
	{
		while (cur && cur->data != me )
			cur = cur->next;

		if (!cur)
			dp("DATA BLOCK NOT FOUND - File: %s   Line: %lu\n",file,line);
		else
		{
			if (memList == cur)
				memList = cur->next;
			else
				cur->prev->next = cur->next;

			if (cur->next)
				cur->next->prev = cur->prev;
		}

		delete cur;
	}
	else
		dp("Free Before Alloc - File: %s   Line: %lu\n",file,line);
	delete me;
}

#ifdef __cplusplus
}
#endif
