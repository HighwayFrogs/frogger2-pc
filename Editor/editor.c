/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: editor.c
	Programmer	: David Swift
	Date		: 28/06/99 15:53:22

----------------------------------------------------------------------------------------------- */

#include <gelf.h>
//#include <ultra64.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>

#include <islmem.h>
#include <winbase.h>
#include <winnt.h>
#include <Windows.h>

#include "edmaths.h"
#include "editfile.h"
#include "editdefs.h"
#include "edcommand.h"
#include "edglobals.h"
#include "edtoolbars.h"
#include "editor.h"

#include "collect.h"
#include "cam.h"
#include "define.h"
#include "enemies.h"
#include "frogger.h"
#include "frogmove.h"
#include "islutil.h"
#include "Main.h"
#include "platform.h"

#include "controll.h"
#include "mdx.h"
#include "pcmisc.h"
#include "pcsprite.h"

/* ----------------------------------------------------------------------------------------------- */

const char* ENTITYTYPESTRINGS[] = {
	"Enemy", "Platform", "Garib"
};

//static const char *EDITVARSTRINGS[] = { "char*", "int","float" };

//char EDITOR_SHORTCUTS_A[] = "1234567890qwertyuiop";
//char EDITOR_SHORTCUTS_B[] = "asdfghjklzxcvbnm";

KMAP EDKEYMAP[] = 
{
	{ 'f',	TB_PLACEFLAG },
	{ 'k',	TB_CLEARPATH },
	{ 'e',	TB_PLACEENEMY },
	{ 'p',	TB_PLACEPLATFORM },
	{ 'g',	TB_PLACEGARIB },
	{ 'a',	TB_AUTOCAMERA },
	{ 'i',	TB_INFO },
	{ '3',	TB_TEST },
	{ '4',	TB_TEST_SCRIPTING },
	{ 'u',	TB_UNDO },
	{ '1',	TB_SAVE },
	{ 'l',	TB_LOAD },
	{ 's',	TB_RECTSELECT },
	{ 'c',	TB_COPY_SELECTION },
	{ 'v',	TB_PASTE_SELECTION },
	{ 'd',	TB_PICKFLAG },
	{ 'r',	TB_SET_ALL_PATHSPEEDS },
	{ 'b',	TB_SET_ALL_BOTHOFFSETS },
	{ 'w',	TB_SET_ALL_WAITTIMES },
	{ ' ',	TB_CLEAR_SELECTION },
	{ 'h',	TB_HELPSCREEN },
	{ '9',	TB_999LIVES },
	{ 'q',	TB_PLACEFROG },
	{ 't',	TB_TILESELECT },
	{ 'y',	TB_CAMERACASE_ADDTILES },
	{ 'n',	TB_SET_ENTITYTYPE },
	{ 'z',	TB_SCALE },
	{ 'o',	TB_RADIUS },
	{ '0',	TB_KILL_EVERYTHING },
	{ 'j',	TB_AUTOMAP_BABIES },
	{ 'x',  TB_SET_ID },
	{ 'm',	TB_ANIMSPEED },
	{ '.',	TB_CAMERACASE_SETDIRECTION },
	{ '5',  TB_SAVE_EXTRASPECIAL },
	{ -1, -1 }
};
/* ----------------------------------------------------------------------------------------------- */

RECT statusRect = { 0, 380, 640, 480-60 };
RECT borderRect = { 0, 376, 640, 380 };
RECT toolbarRect = { 0, 480-60, 640, 480 };

static int commonFlagIcons[8] = {
	PLATFORM_NEW_FORWARDS,
	PLATFORM_NEW_BACKWARDS, 
	PLATFORM_NEW_CYCLE,
	PLATFORM_NEW_PINGPONG,
	PLATFORM_NEW_MOVEUP,
	PLATFORM_NEW_MOVEDOWN,
	0
};

static int platformFlagIcons[8] = {
	PLATFORM_NEW_STEPONACTIVATED,
	PLATFORM_NEW_CRUMBLES,
	PLATFORM_NEW_REGENERATES,
	PLATFORM_NEW_NOWALKUNDER,
	0, 0, 0, 0
};


struct TAGSTATUSWINDOW
{
	RECT r;
	char text[128], currentSelection[128];
} statusWin;

struct {
	POINT a, b;
	BOOL valid;
} selectRect;

typedef void (* EDITVARFUNC)(const char *) ;
typedef void (* SELECTFUNC)(int, int, int, int);

unsigned long mod = 0;
static float womble = 0;

POINT mousePos;

/*	-------------------------------------------------------------------------------------------
	prototypes
*/

void DrawStatusInfo(void);
void ShowTileInfo(GAMETILE *tile);
void PlaceEditX(int x, int y);
void EditorMouseDown(int x, int y);
void EditorMouseUp(int x, int y);
void EditorMouseMove(int x, int y);
void EditorMoveCamera();
void EditorUpdateCamera();
void GetEditorCameraRotations(void);
void SaveEditorState(void);
void LoadEditorState(void);
int ToolbarMouseOver(int);
void ToolbarMouseDown(int);
GAMETILE *FindClickedTile(int x, int y);
CREATEENTITY *FindCreateNode(int x, int y);
EDITPATHNODE* FindFlag(int x, int y);
EDITPATHNODE* FindFlagOnTile(GAMETILE *tile);
void DrawEditorArrow(int x1, int y1, int x2, int y2, float width, D3DCOLOR col);

/*	--------------------------------------------------------------------------------
	Function		: SaveEditorState
	Parameters		: 
	Returns			: 
*/

static char *BooleanString(int flag) {
	return flag ? "True" : "False";
}

void SaveEditorState()
{
	WritePrivateProfileString("Editor Flags", "invert_mouse", BooleanString(invertMouse), iniFilePath);
	WritePrivateProfileString("Editor Flags", "draw_links", BooleanString(drawLinks), iniFilePath);
	WritePrivateProfileString("Editor Flags", "draw_normals", BooleanString(drawNormals), iniFilePath);
	WritePrivateProfileString("Editor Flags", "draw_vectors", BooleanString(drawVectors), iniFilePath);
	WritePrivateProfileString("Editor Flags", "draw_dots", BooleanString(drawDots), iniFilePath);
}

/*	--------------------------------------------------------------------------------
	Function		: LoadEditorState
	Parameters		: 
	Returns			: 
*/

void LoadEditorState()
{
	char tempStr[16];

	GetPrivateProfileString("Editor Flags", "invert_mouse", "False", tempStr, 16, iniFilePath);
	invertMouse = stricmp(tempStr, "True") == 0;

	GetPrivateProfileString("Editor Flags", "draw_links", "False", tempStr, 16, iniFilePath);
	drawLinks = stricmp(tempStr, "True") == 0;

	GetPrivateProfileString("Editor Flags", "draw_normals", "False", tempStr, 16, iniFilePath);
	drawNormals = stricmp(tempStr, "True") == 0;

	GetPrivateProfileString("Editor Flags", "draw_vectors", "False", tempStr, 16, iniFilePath);
	drawVectors = stricmp(tempStr, "True") == 0;

	GetPrivateProfileString("Editor Flags", "draw_dots", "False", tempStr, 16, iniFilePath);
	drawDots = stricmp(tempStr, "True") == 0;
}

/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

int CheckEditIcons()
{
	// This is a bit of a hack but abusing the fact that there is already an editicons list
	// and it will be changed by a subsequent call to LoadEditIcons 
	// if the icon is present we set the editicons[i].surface = 1 otherwise 0
	char texturePath[MAX_PATH];
	int i;
	int success = 1;
	DWORD attribs;

	for (i = 0; i < EDITORTEXTURES; i++)	
	{
		sprintf(texturePath, "%s" TEXTURE_BASE "editor\\editor%d.bmp", baseDirectory, i+1);
		attribs = GetFileAttributes(texturePath);
		editicons[i].surface = (attribs != ((DWORD)-1) && !(attribs & FILE_ATTRIBUTE_DIRECTORY));
		if (!editicons[i].surface) success = 0;
	}

	return success;
}

/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: whether loading all of the icons were successfully loaded
	Info			: 
*/

void LoadEditIcons()
{
	char texturePath[MAX_PATH];
	int i;

	for (i=0; i<EDITORTEXTURES; i++)
	{
		//Copy the base directory and texture subdirectory into our path
		sprintf(texturePath, "%s" TEXTURE_BASE "editor\\editor%d.bmp", baseDirectory, i+1);

		editicons[i].surface = LoadEditorTexture(texturePath);
		
/*old:
		// Load texture
		textureData = (short *)gelfLoad_BMP(texturePath, NULL, (void**)-1, 
			&xDim,&yDim,NULL,GELF_IFORMAT_16BPP555,GELF_IMAGEDATA_TOPDOWN);

		// Create surface and blit
		temp = D3DCreateTexSurface(xDim, yDim, 0xf81f, 0, 1);
		//DDrawCopyToSurface(temp,(unsigned short *)textureData, 0, xDim, yDim);

		//editicons[i].surface = D3DCreateTexSurface(xDim, yDim, 0xf81f, 0, 1);
		//while (editicons[i].surface->BltFast(0,0,temp,NULL,NULL)!=DD_OK);

		editicons[i].surface = temp;
		temp->lpVtbl->AddRef(temp);
		temp->lpVtbl->Release(temp);
*/
	}

	utilPrintf("Loaded %d pages of editor textures\n", EDITORTEXTURES);
}
           
/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

void DrawMouse(void)
{
	float s = 16;
	RECT r = { mousePos.x, mousePos.y, mousePos.x+16, mousePos.y+16 };

	if (lButton)
	{
		if (controlCamera) return;
		s = 12;
	}
	
	//GetCursorPos(&p);

	//DrawFlatRect(r, D3DRGBA(1,1,1,1));

	DrawTexturedRect(
		r, D3DRGBA(1,1,1,0.9),
		editicons[0].surface,
		0, 0, 0.25f, 0.25f);
}

/*	--------------------------------------------------------------------------------
	Function		: InitEditor
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

void InitEditor(void)
{
	static RECT statusSize = { 0, 350, 640, 480 };

	statusWin.r = statusSize;
	statusWin.text[0] = 0;
	strcpy(statusWin.currentSelection, "[no current tile]");

	selectRect.valid = FALSE;

	editPath = NULL;
	createList = MakeEditGroup();

	ZeroVector(&editCameraRot);
	//editCameraRot.vx = -(float)PI/4.0;

	//ZeroVector(&editCameraDir);

	LoadEditIcons();
	LoadEditorState();
}

/*	--------------------------------------------------------------------------------
	Function		: ShutdownEditor
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

void ShutdownEditor(void)
{
	SaveEditorState();
	FreeEditPath(editPath);
	FreeCreateList();
}

/*	--------------------------------------------------------------------------------
	Function		: RunEditor
	Purpose			: Runs the editor
	Parameters		: 
	Returns			: 
	Info			: 
*/

void RunEditor(void)
{
	POINT p;

	//if (editText.active) return;

	// Get mouse pos and scale it to screen coords
	GetCursorPos(&mousePos);
/*
	if (winMode)
	{
		ScreenToClient(mdxWinInfo.hWndMain, &mousePos);
		GetClientRect(mdxWinInfo.hWndMain, &r);

		mousePos.x = (rXRes * mousePos.x) / r.right;
		mousePos.y = (rYRes * mousePos.y) / r.bottom;
	}
*/
	p = mousePos;

	EditorMoveCamera();

	// recalc viewing matrix FOR HEAVENS SAKE
	CalcViewMatrix(0);
	
	if (showX) PlaceEditX(p.x, p.y);

	if (lButton)
	{
		if (!clicking)
			EditorMouseDown(p.x, p.y);
	}
	else
	{
		if (clicking)
		{
			//GetCursorPos(&p);
			EditorMouseUp(p.x, p.y);
			clicking = FALSE;
		}
	}

	if (p.x != oldMousePos.x || p.y != oldMousePos.y) // && p.y < toolbarRect.top)
		EditorMouseMove(p.x, p.y);

	oldMousePos = p;
}

void UpdateStatusString()
{
	if (editText.active) {
		sprintf(statusWin.text, "%s = %s_",
			editText.var,
			//EDITVARSTRINGS[editText.type],
			editText.string);
	}
	else if (cameraMode)
	{
		strcpy(statusWin.text, "Moving camera - press ESC to cancel");
	}
	else
	{
		switch (editMode)
		{
		case EDIT_PLACE_GARIB:
			sprintf(statusWin.text, "Place garib: %s", GARIBTYPESTRINGS[garibType]);
			break;

		case EDIT_PLACE_ENEMY:
			if( copyFlag )
				sprintf(statusWin.text, "Paste enemy on path: %s", createTypes[0]);
			else
				sprintf(statusWin.text, "Create enemy on path: %s", createTypes[0]);
			break;

		case EDIT_PLACE_PLATFORM:
			if( copyFlag )
				sprintf(statusWin.text, "Paste platform on path: %s", createTypes[1]);
			else
				sprintf(statusWin.text, "Create platform on path: %s", createTypes[1]);
			break;

		case EDIT_PLACE_PLACEHOLDER:
			sprintf(statusWin.text, "Create placeholder on path" );
			break;

		case EDIT_MAKE_PATH:
			strcpy(statusWin.text, "Making path");
			break;

		case EDIT_PLACE_FROG:
			strcpy(statusWin.text, "Placing frog");
			break;

		default:
			statusWin.text[0] = 0;
		}
	}
}

void PlaceEditX(int x, int y)
{
	int r = 100*100;

	if (!(editTile = FindClickedTile(x, y))) return;
	//SetVectorS(&clickPos, &editTile->centre);
	GetTilePos(&clickPos, editTile);
}


void DoEditCamera(void)
{
	cameraMode = TRUE;
	TOOLBARBUTTONS[CAMERA_BUTTON].icon = 23;
	selectedIcon = -1;
	controlCamera = TRUE;

	SetCursorPos(320, 240);
	GetEditorCameraRotations();
	EditorUpdateCamera();
	UpdateStatusString();
}

/*	--------------------------------------------------------------------------------
	Function		: EditorMouseMove
	Purpose			: 
	Parameters		: int, int
	Returns			: 
	Info			: 
*/
void RotateVector2DXYZ(EDVECTOR *dest, EDVECTOR *source, float angle, BYTE axis)
{
	EDVECTOR temp;
	float cosa, sina;

	cosa = (float)cos(angle);
	sina = (float)sin(angle);

	temp = *source;

	switch(axis)
	{
		case X:
			temp.vy = (cosa * source->vy) + (-sina * source->vz);
			temp.vz = (cosa * source->vz) + (sina * source->vy);
			break;
		case Y:
			temp.vz = (cosa * source->vz) + (-sina * source->vx);
			temp.vx = (cosa * source->vx) + (sina * source->vz);
			break;
		case Z:
			temp.vx = (cosa * source->vx) + (-sina * source->vy);
			temp.vy = (cosa * source->vy) + (sina * source->vx);
			break;
	}

	SetVector(dest, &temp);
}

void EditorMouseMove(int x, int y)
{
	float rot;
	EDVECTOR v;

	if (cameraMode && (lButton || rButton))
	{
//		POINT mp;
		POINT p = { 320, 240 };

/*		if (winMode)
		{
			GetCursorPos(&mp);

			ScreenToClient(winInfo.hWndMain, &mp);
			x = mp.x; y = mp.y;

			ClientToScreen(winInfo.hWndMain, &p);
			SetCursorPos(p.x, p.y);
		}
		else*/
		{
			SetCursorPos(320, 240);
		}

		if (rButton)
		{
			if (lButton)
			{
				v.vx = (float)(x - 320) * 8192;
				v.vy = 0.0;
				v.vz = (float)(y - 240) * 8192;

				RotateVector2DXYZ(&v, &v, editCameraRot.vx, X);
				RotateVector2DXYZ(&v, &v, editCameraRot.vy, Y);
				AddToVector(&currCamSource, &v);
				EditorUpdateCamera();
			}
			else 
			{
				editCameraRot.vy += (320-x) * (float)(3.14159 / 2 / 360);
				rot = (240-y) * (float)(3.14159 / 2 / 480);
				if (invertMouse) rot = -rot;
				editCameraRot.vx += rot;
				EditorUpdateCamera();
			}
		}
		else if (lButton)
		{
			v.vy = -(float)(y - 240) * 8192;
			v.vx = (float)(x - 320) * 8192;
			v.vz = 0.0;

			RotateVector2DXYZ(&v, &v, editCameraRot.vx, X);
			RotateVector2DXYZ(&v, &v, editCameraRot.vy, Y);
			AddToVector(&currCamSource, &v);
			EditorUpdateCamera();
		}
	}
	else if (rButton && !cameraMode)
	{
		DoEditCamera();
	}
	else if (lButton && editMode == EDIT_RECT_SELECT)
	{
		selectRect.b.x = x;
		selectRect.b.y = y;
	}
}

void EditorMoveCamera()
{
	float offsx = 0, offsy = 0;
	EDVECTOR v;
	BOOL foo = FALSE;

	if (KEYPRESS(DIK_UP))
		{ offsy = -InOutWhizziness; foo = TRUE; }
	if (KEYPRESS(DIK_DOWN))  
		{ offsy = InOutWhizziness; foo = TRUE; }
	if (KEYPRESS(DIK_LEFT))	 
		{ offsx = -HorizontalWhizziness; foo = TRUE; }
	if (KEYPRESS(DIK_RIGHT)) 
		{ offsx = HorizontalWhizziness; foo = TRUE; }

	if (KEYPRESS(DIK_LSHIFT) || KEYPRESS(DIK_RSHIFT)) { offsx *= 2; offsy *= 2; }

	if (foo && !controlCamera)
		DoEditCamera();

	if (foo)
	{
		v.vy = 0.0;
		v.vx = offsx * 40960;
		v.vz = offsy * 40960;

		RotateVector2DXYZ(&v, &v, editCameraRot.vx, X);
		RotateVector2DXYZ(&v, &v, editCameraRot.vy, Y);
 
		AddToVector(&currCamSource, &v);

		//SetCursorPos(320,240);
	}

	if (controlCamera)
		EditorUpdateCamera();
}

void ToolbarMouseDown(int toolOpt)
{
	int command = 0;

	if (toolOpt >= 100)
	{
		if (submenu)
		{
			toolOpt -= 100;
			while (toolOpt--)
			{
				if (submenu[command].command == -1) return;
				command++;
			}
			ToolbarSelect(submenu[command].command);
		}
		else
			return; 
	}
	else
	{
		while (toolOpt--)
		{
			if (TOOLBARBUTTONS[command].command == -1) return;
			command++;
		}
		ToolbarSelect(TOOLBARBUTTONS[command].command);
	}
}

/*	--------------------------------------------------------------------------------
	Function		: ToolbarMouseOver
	Purpose			: 
	Parameters		: 
	Returns			: enumerated command
	Info			: 
*/
int ToolbarMouseOver(int toolOpt)
{
	int command = 0;

	if (toolOpt >= 100)
	{
		if (submenu)
		{
			toolOpt -= 100;
			while (toolOpt--)
			{
				if (submenu[command].command == -1) return -1;
				command++;
			}
			return submenu[command].command;
		}
		else
			return -1; 
	}
	else
	{
		while (toolOpt--)
		{
			if (TOOLBARBUTTONS[command].command == -1) return -1;
			command++;
		}
	}

	return TOOLBARBUTTONS[command].command;
}

/*	--------------------------------------------------------------------------------
	Function		: EditorMouseDown
	Purpose			: 
	Parameters		: int, int
	Returns			: 
	Info			: 
*/

void EditorMouseDown(int x, int y)
{
	EDITPATHNODE *path;
	GAMETILE *tile;

	if (y > statusRect.top)
		ToolbarMouseDown(x / 16 + ((y - toolbarRect.top) / 18) * 100);
	else if (!cameraMode)
	{
		switch (editMode)
		{
		case EDIT_PLACE_GARIB:
			if (!(tile = FindClickedTile(x, y))) break;
			//v = tile->centre; v.vy += 20;
			//CreateNewGarib(v, garibType, &firstTile[1], 0.0f);
			PlaceGarib(tile, garibType);
			break;

		case EDIT_MAKE_PATH:
			tile = FindClickedTile(x, y);
			if (tile)
			{
				if (!editPath) editPath = CreateEditPath();
				if (KEYPRESS(DIK_LSHIFT) || KEYPRESS(DIK_RSHIFT))
					EditorAddFlags(tile, editPath);
				else
					EditorAddFlag(tile, editPath);
			}
			break;

		case EDIT_PLACE_PLATFORM:
			path = FindFlagOnTile(editTile);
			if( !path )
			{
				FreeEditPath( editPath );
				editPath = CreateEditPath();
				path = EditorAddFlag(editTile, editPath);
			}
			PlacePlatform(path);
			UpdateSelection( );
			break;

		case EDIT_PLACE_PLACEHOLDER:
			path = FindFlagOnTile(editTile);
			if( !path )
			{
				FreeEditPath( editPath );
				editPath = CreateEditPath();
				path = EditorAddFlag(editTile, editPath);
			}
			PlacePlaceholder(path);
			UpdateSelection( );
			break;

		case EDIT_PLACE_ENEMY:
			path = FindFlagOnTile(editTile);
			if( !path )
			{
				FreeEditPath( editPath );
				editPath = CreateEditPath();
				path = EditorAddFlag(editTile, editPath);
			}
			PlaceEnemy(path);
			UpdateSelection( );
			break;

		case EDIT_RECT_SELECT:
			selectRect.a.x = x;
			selectRect.a.y = y;
			selectRect.b = selectRect.a;
			selectRect.valid = TRUE;
			break;

		case EDIT_PLACE_FROG:
			if (!(tile = FindClickedTile(x,y))) break;
			SetVector(&frog[0]->actor->position, &tile->centre);
			
			if (currPlatform[0])
			{
				currPlatform[0]->carrying = NULL;
				currPlatform[0]->flags &= ~PLATFORM_NEW_CARRYINGFROG;
				currPlatform[0] = NULL;
			}
			currTile[0] = tile;
			gTStart[0] = tile;
			player[0].frogState = 0;
			break;
		}
	}

	clicking = TRUE;
}

/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

void EditorMouseUp(int x, int y)
{
	if (selectRect.valid) 
	{
		selectRect.valid = FALSE;
		
		if (selectRect.a.x == selectRect.b.x && selectRect.a.y == selectRect.b.y)
		{
			selectRect.a.x -= 2; selectRect.b.x += 2;
			selectRect.a.y -= 2; selectRect.b.y += 2;
		}

		if( addTiles && selectionList && selectionList->nodes && selectionList->nodes->thing &&
			((CREATEENTITY *)selectionList->nodes->thing)->thing == CREATE_CAMERACASE )
		{
			AddTilesToCameraCase( selectRect.a.x, selectRect.a.y, selectRect.b.x, selectRect.b.y );
		}
		else if (selectFunc)
		{
			selectFunc(selectRect.a.x, selectRect.a.y, selectRect.b.x, selectRect.b.y);
			UpdateStatusString();
		}
	}
}

void DrawEditorArrow(int x1, int y1, int x2, int y2, float width, D3DCOLOR col)
{
	float a, b;
	D3DTLVERTEX vtx[3];
	short arse[3] = { 0, 2, 1 };

	memset(vtx, 0, sizeof(D3DTLVERTEX)*3);

	if (x1==x2 && y1==y2) return;	// foolish

	if (x1 < 0 || x1 > 640 || x2 < 0 || x2 > 640 ||
		y1 < 0 || y1 > 480 || y2 < 0 || y2 > 480) return;	// pah

	a = (x2 - x1) * width; if (a < -1 || a > 1) a = (a < 0) ? -1 : 1;
	b = (y2 - y1) * width; if (b < -1 || b > 1) b = (b < 0) ? -1 : 1;

	vtx[0].dvSX = x2;
	vtx[0].dvSY = y2;
	vtx[0].dvSZ = 0.0001f;
	vtx[0].dcColor = col;
	vtx[0].dcSpecular = D3DRGBA(0,0,0,1);

	vtx[1].dvSX = x1 - b;
	vtx[1].dvSY = y1 + a;
	vtx[1].dvSZ = 0.0001f;
	vtx[1].dcColor = col;
	vtx[1].dcSpecular = D3DRGBA(0,0,0,1);

	vtx[2].dvSX = x1 + b;
	vtx[2].dvSY = y1 - a;
	vtx[2].dvSZ = 0.0001f;
	vtx[2].dcColor = col;
	vtx[2].dcSpecular = D3DRGBA(0,0,0,1);

	//PushPolys(vtx, 3, &arse, 1, 0);
	//PushPolys(v,vC,fce,fC,h)	Clip3DPolygon(vtx, 0);
	DrawPoly( D3DPT_TRIANGLELIST, 
		D3DFVF_TLVERTEX,
		vtx, 3, 
		arse, 3,
		D3DDP_WAIT);
}

/*	--------------------------------------------------------------------------------
	Function		: DrawEditorIcon
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

void DrawEditorIcon(int x, int y, int n, BOOL disabled)
{
	int page, row, col;
	RECT r = { x, y, x+16, y+16 };

	if (n < 0) return;

	page = n >> 4;
	row = (n & 15) >> 2;
	col = (n & 3);

	DrawTexturedRect(
		r,
		disabled ? D3DRGB(0.5f,0.5f,0.5f) : D3DRGB(1,1,1),
		editicons[page].surface,
		col * EDITORICONSIZE, row * EDITORICONSIZE,
		(col+1) * EDITORICONSIZE, (row+1) * EDITORICONSIZE
	);
}

void DrawAllTiles(void)
{
	GAMETILE *cur;
	D3DCOLOR col;
	EDVECTOR tmp,v,v2,v3;
	int i, t;

	cur = firstTile;
	t = tileCount;
	
	while (t--)
	{
		GetTilePos(&tmp, cur);
		EdXfmPoint(&v, &tmp);
		
		if (v.vz)
		{
			for (i=0; i<4; i++)
			{
				if (drawLinks)
					if (cur->tilePtrs[i])
					{
						if (cur->state == TILESTATE_JOIN || cur->tilePtrs[i]->state == TILESTATE_JOIN)
							col = D3DRGB(0.0,0.4,1.0);
						else
							col = D3DRGB(1,0.2,0.4);

						GetTilePos(&tmp, cur->tilePtrs[i]);
						EdXfmPoint(&v2, &tmp);
						if (v2.vz)
							DrawEditorArrow(v.vx, v.vy,v2.vx, v2.vy, 0.05f, col);
					}
				
				if (drawVectors)
				{
					SetVectorF(&v3, &cur->dirVector[i]);
					ScaleVector(&v3, 20.0);
					AddToVector(&v3, &tmp);
					
					EdXfmPoint(&v2, &v3);
				
					if (v2.vz)
						DrawEditorArrow(v.vx, v.vy,v2.vx, v2.vy, 0.1f, D3DRGB(0,0,1));

					if (i==0)
						DrawEditorIcon(v2.vx-7, v2.vy-7, 45, FALSE);
				}


			}

			if (drawNormals)
			{
				SetVectorF(&v3, &cur->normal);
				ScaleVector(&v3,20.0);
				AddToVector(&v3, &tmp);

				EdXfmPoint(&v2, &v3);
				
				if (v2.vz)
					DrawEditorArrow(v.vx, v.vy,v2.vx, v2.vy, 0.15f, D3DRGB(0,0,0));
			}

			if (drawDots)
				DrawEditorIcon(v.vx - 7, v.vy - 7, 60, FALSE);
		}
		
		cur++;
	}
}

/*	--------------------------------------------------------------------------------
	Function		: DrawEditor
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

void DrawEditPath(EDITPATH *ep, BOOL shadey)
{
	EDITPATHNODE *path, *next;
	EDVECTOR v, w, a;

	for (path = ep->nodes; path; path = next)
	{
		next = path->link;
		/* if there's a link from this, draw a line */
		if (next) {
			GetTilePos(&a, path->tile);
			EdXfmPoint(&v, &a);

			GetTilePos(&a, next->tile);
			EdXfmPoint(&w, &a);

			if (w.vz != 0 && v.vz != 0)
			{
				SubFromVector(&w, &v);
				ScaleVector(&w, 0.5);
				AddToVector(&w, &v);

				//DrawALine(v.vx, v.vy, w.vx, w.vy, D3DRGBA(1,0,0,shadey?0.5:1.0));
				DrawEditorArrow(v.vx, v.vy, w.vx, w.vy, 0.1f, D3DRGBA(1,0,0,shadey?0.5:1.0));
				DrawEditorIcon(w.vx - 3, w.vy - 3, 4, shadey);
			}
		}

		GetTilePos(&a, path->tile);
		EdXfmPoint(&v, &a);
		if (v.vz == 0) continue;
		
		v.vy -= path->level * 16;

		// draw checkered flag for start flag (and I KNOW that's wrong)
		/*if (path->start)
			DrawEditorIcon(v.vx - 12, v.vy - 15, 3, shadey);
		else*/
		
		DrawEditorIcon(v.vx - 12, v.vy - 15, 2, shadey);
	}
}

void DrawEditor(void)
{
	EDVECTOR v;
	EDITGROUPNODE *node, *flag;
	CREATEENTITY *create;
	RECT r;

	// CREATE_ types
	static const int createicons[5] = { 5, 6, 7, 63, 79 };

	// Draw "X marks the spot"
	if (showX && (clickPos.vx != INT_MAX))
	{
		EdXfmPoint(&v, &clickPos);
		if (v.vz != 0)
			DrawEditorIcon(v.vx - 8, v.vy - 8, 1, FALSE);
	}

	if (drawLinks || drawVectors || drawNormals || drawDots) DrawAllTiles();

	if (editPath) DrawEditPath(editPath, FALSE);

	// Draw "create entity" flags
	for (node = createList->nodes; node; node = node->link)
	{
		create = (CREATEENTITY*)node->thing;
		EdXfmPoint(&v, &node->pos);
		if (v.vz == 0) continue;

		DrawEditorIcon(v.vx - 12, v.vy - 15,
			createicons[create->thing], (create->path != editPath));
		//DrawEditPath(create->path, TRUE);
	}

	if (selectionList)
		for (node = selectionList->nodes; node; node = node->link)
		{
			EdXfmPoint(&v, &node->pos);
			if (v.vz != 0)
				DrawEditorIcon(v.vx - 8, v.vy - 8, 62, FALSE);

				if (selectFunc == Select_Create)
				// Mark all affected tiles of a camera case
				if( ((CREATEENTITY *)node->thing)->thing == CREATE_CAMERACASE )
				{
					for( flag = ((CREATEENTITY *)node->thing)->group->nodes; flag; flag = flag->link )
					{
						EdXfmPoint(&v, &flag->pos);
						if (v.vz != 0)
							DrawEditorIcon(v.vx - 8, v.vy - 8, 89, FALSE);
					}
				}
		}

	DrawStatusInfo();

	if (selectRect.valid)
	{
		r.left = selectRect.a.x;
		r.right = selectRect.b.x;
		r.top = selectRect.a.y;
		r.bottom = selectRect.b.y;
		DrawFlatRect(r, D3DRGBA(sin(womble)*0.5+0.5, 0, cos(womble)*0.5+0.5, 0.5));
	}

	womble+=0.2f;

//	if (!winMode)
		DrawMouse();
}


/*	--------------------------------------------------------------------------------
	Function		: DrawStatusInfo
	Parameters		: 
	Returns			: 
*/

void DrawStatusInfo(void)
{
	HRESULT res;
	HDC hdc;
	int i, icon, command;
	POINT p;
	RECT r;

	DrawFlatRect(statusRect, D3DRGBA(0, 0, 0.1, 0.5));
	DrawFlatRect(borderRect, D3DRGBA(0, 0, 0.1, 0.7));
	DrawFlatRect(toolbarRect, D3DRGBA(0, 0, 0.1, 0.7));

	res = IDirectDrawSurface4_GetDC(surface[RENDER_SRF], &hdc);
	if (res == DD_OK)
	{
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, 0x00ffffff); //Color of status window text
		TextOut(hdc, statusRect.left+5, statusRect.top+5, statusWin.text, strlen(statusWin.text));
		TextOut(hdc, statusRect.left+5, statusRect.top+20, statusWin.currentSelection, strlen(statusWin.currentSelection));
		TextOut(hdc, statusRect.left+5, toolbarRect.bottom-20, statusMessage, strlen(statusMessage));

		//GetCursorPos(&p);
		p = mousePos;

		if (p.y > statusRect.top)
		{
			command = ToolbarMouseOver(p.x / 16 + ((p.y - toolbarRect.top) / 18) * 100);

			SetTextAlign( hdc, TA_RIGHT );
	
			if( command != -1 )
			{
				SetTextColor(hdc, 0x00ffffff); //Congratulations. We now have readable tooltips.
				TextOut( hdc, statusRect.right,
						toolbarRect.bottom-20, 
						tooltips[command],
						strlen(tooltips[command]) );
			}
			else
			{
				SetTextColor(hdc, 0x00ffffff); //Ditto to the above comment.
				TextOut( hdc, statusRect.right,
						toolbarRect.bottom-20, 
						"Press H for help",
						16 );
			}
		}

		if( helpScreen )
		{
			int half;
			for( i=0; HELPSTRINGS[i][0] != '\n'; i++ );

			half = (i/2)+1;
			r.left = 120;
			r.right = 520;
			r.top = 20;
			r.bottom = half*22;
			SetTextAlign( hdc, TA_LEFT );
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, 0x00ffffff);
			for( --i; (i+1); i-- ) // Oh yeah baby!
				TextOut(hdc, (i<half)?r.left+5:r.left+((r.right-r.left)/2)+5,
						(i<half)?r.top+5+(i*20):r.top+5+((i-half)*20),
						HELPSTRINGS[i],
						strlen(HELPSTRINGS[i]));
		}

		IDirectDrawSurface4_ReleaseDC(surface[RENDER_SRF], hdc);
	}
	else
	{
		utilPrintf("Error getting surface DC: %d", res);
		return;
	}

	for (i=0; TOOLBARBUTTONS[i].command != -1; i++) {
		icon = TOOLBARBUTTONS[i].icon;
		if (icon == -1) continue;
		if( TOOLBARBUTTONS[i].command == TB_PASTE_SELECTION ) // Euuww, kludge!
			DrawEditorIcon(2 + i*16, toolbarRect.top + 2, icon, !copyFlag);
		else
			DrawEditorIcon(2 + i*16, toolbarRect.top + 2, icon,
				(selectedIcon != TOOLBARBUTTONS[i].command));
	}

	if (submenu)
	{
		for (i=0; submenu[i].command != -1; i++) {
			icon = submenu[i].icon;
			if (icon == -1)
				continue; // skip seperators
			else if( (icon & 0x8000) || (submenu[i].command == TB_CAMERACASE_ADDTILES && addTiles) )
				DrawEditorIcon(2 + i*16, toolbarRect.top+20, icon & 0x7fff, FALSE);
			else
				DrawEditorIcon(2 + i*16, toolbarRect.top+20, icon, TRUE);
		}
	}

/*
	if (editMode == EDIT_PLACE_ENEMY || editMode == EDIT_PLACE_PLATFORM)
	{
		DrawEditorIcon(2, toolbarRect.top+20, 32, FALSE);
		for (icon=1; icon<8; icon++, flag) {
			DrawEditorIcon(2 + icon*16, toolbarRect.top+20, icon+32,
				(createFlags & commonFlagIcons[icon-1]) == 0);
		}

		if (editMode == EDIT_PLACE_PLATFORM)
		{
			for (icon=0; icon<8; icon++, flag) {
				DrawEditorIcon(2 + 8 * 16 + icon*16, toolbarRect.top+20, icon+40,
					(createFlags & platformFlagIcons[icon]) == 0);
			}
		}
	}
*/
}


/*	--------------------------------------------------------------------------------
	Function		: FindSomething
	Parameters		: 
	Returns			: 
*/

CREATEENTITY *FindCreateNode(int x, int y)
{
	EDITGROUPNODE *node, *closest;
	EDVECTOR v;
	int dist, closestDist;

	closest = NULL; closestDist = 0x10000;

	for (node = createList->nodes; node; node = node->link)
	{
		EdXfmPoint (&v, &node->pos);
		if (!v.vz) continue;

		v.vx -= x; v.vy -= y; v.vz = 0;
		
		dist = MagnitudeSquared(&v);

		if (dist < closestDist)
		{
			closest = node;
			closestDist = dist;
		}
	}

	return (CREATEENTITY*)closest->thing;
}	

/*	--------------------------------------------------------------------------------
	Function		: FindFlag
	Parameters		: 
	Returns			: 
*/

EDITPATHNODE* FindFlag(int x, int y)
{
	EDITPATHNODE *flag, *closest;
	EDVECTOR v, a;
	int dist, closestDist;

	if (!editPath) return NULL;

	closest = NULL; closestDist = 0x10000;

	for (flag = editPath->nodes; flag; flag = flag->link)
	{
		SetVectorF(&a, &flag->tile->centre);
		EdXfmPoint (&v, &a);
		if (!v.vz) continue;

		v.vx -= x; v.vy -= y; v.vz = 0;
		
		dist = MagnitudeSquared(&v);

		if (dist < closestDist)
		{
			closest = flag;
			closestDist = dist;
		}
	}

	return closest;
}

EDITPATHNODE* FindFlagOnTile(GAMETILE *tile)
{
	EDITPATHNODE *flag;

	if( !editPath )
		return NULL;

	for (flag = editPath->nodes; flag; flag = flag->link)
		if (flag->tile == tile)	return flag;

	return NULL;
}

/*	--------------------------------------------------------------------------------
	Function		: FindClickedTile
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

GAMETILE *FindClickedTile(int x, int y)
{
	GAMETILE *t, *closest;
	EDVECTOR v, a;
	int dist, closestDist, n;

	closest = NULL; closestDist = 100*100;

	for (t=firstTile, n=tileCount; n; t++, n--)
	{
		GetTilePos(&a, t);
		EdXfmPoint (&v, &a);
		if (!v.vz) continue;

		v.vx -= x; v.vy -= y; //v.vz = 0;

		if (v.vx > 100 || v.vx < -100 || v.vy > 100 || v.vy < -100) continue;

		dist = v.vx*v.vx + v.vy*v.vy;

		if (dist < closestDist)
		{
			closest = t;
			closestDist = dist;
		}
	}

	return closest;
}

/*	--------------------------------------------------------------------------------
	Function		: EditorUpdateCamera
	Parameters		: 
	Returns			:
	Notes			:

	Legacy code meant that our camera had to be calculated with lookAt... since we've
	actually got a viewing matrix now, this could be updated to use proper rotations
	more control! More power! More spurious expenditure of effort...
*/
void EditorUpdateCamera()
{
	EDVECTOR v = { 0.0, 0.0, -40960 };
	EDVECTOR up = { 0.0, 1.0, 0.0 };

	// Work out point to look at

	//SetVectorS(&v, &currCamTarget);
	//SubFromVector(&v, &currCamSource);
	RotateVector2DXYZ(&v, &v, editCameraRot.vx, X);
	RotateVector2DXYZ(&v, &v, editCameraRot.vy, Y);
	
	AddToVector(&v, &currCamSource);
	
	currCamTarget.vx = (fixed)v.vx;
	currCamTarget.vy = (fixed)v.vy;
	currCamTarget.vz = (fixed)v.vz;

	// Calculate "up" vector

	RotateVector2DXYZ(&up, &up, editCameraRot.vx, X);
	RotateVector2DXYZ(&up, &up, editCameraRot.vy, Y);

	camVect.vx = (fixed)(up.vx * 4096);
	camVect.vy = (fixed)(up.vy * 4096);
	camVect.vz = (fixed)(up.vz * 4096);
}


void GetEditorCameraRotations(void)
{
	EDVECTOR v, a, b;
	float foo, bar;

	SetVectorS(&a, &currCamSource);
	SetVectorS(&b, &currCamTarget);
	SubVector(&v, &a, &b);

	bar = atan2(v.vx, v.vz);
	RotateVector2DXYZ(&v, &v, -bar, Y);
	foo = -atan2(v.vy, v.vz);

	editCameraRot.vx = foo;
	editCameraRot.vy = bar;
}


/*	--------------------------------------------------------------------------------
	Function		: EditorKeypress
	Parameters		: 
	Returns			: 
*/

void EditorKeypress(char c)
{
	char *p;
	int i;
	double d;

	if (editText.active)
	{
		p = editText.string;
		if (c == 8) // backspace
		{
			if (!*p) 
			{
				editText.active = 0;
				UpdateStatusString( );
				return;
			}
			while (*p) p++;
			*(p - 1) = 0;
		}
		else if (c == 13) // enter
		{
			switch (editText.type)
			{
			case EDITVAR_INT:
				*(int *)editText.variable = atoi(editText.string);
				sprintf(statusMessage, "Set %s to %d", editText.var, *(int*)editText.variable);
				break;

			case EDITVAR_STRING:
				strcpy(editText.variable, editText.string);
				sprintf(statusMessage, "Set %s to '%s'", editText.var, editText.string);
				break;

			case EDITVAR_FLOAT:
				d = atof(editText.string);
				*(float *)editText.variable = d;
				sprintf(statusMessage, "Set %s to %f", editText.var, *(float*)editText.variable);
				break;

			case EDITVAR_PASSTOFUNCTION:
				{
					EDITVARFUNC editFunc;
					editFunc = (EDITVARFUNC)editText.variable;
					editFunc(editText.string);
					//FUNC_SetPathSpeed(editText.string);
					break;
				}

			case EDITVAR_GROUPFUNCINT:
				ForAllGroupNodesInt(selectionList, editText.string, (CREATEFUNCINT)editText.variable);
				break;
				
			case EDITVAR_GROUPFUNCFLOAT:
				ForAllGroupNodesFloat(selectionList, editText.string, (CREATEFUNCFLOAT)editText.variable);
				break;

			case EDITVAR_GROUPFUNCSTR:
				ForAllGroupNodesStr(selectionList, editText.string, (CREATEFUNCSTR)editText.variable);
				break;
			}
			editText.active = FALSE;
		}
		else if (c == 27)
		{
			editText.active = FALSE;
		}
		else
		{
			for (i = 0; *p; p++, i++); // count elements & find last char
			if (i < editText.charLimit)
			{
				*(p++) = c;
				*p = 0;
			}
		}
		UpdateStatusString();
	}
	else
	{
		if( c == 8 ) // Delete keys - remove selected, then clear flag path
		{
			if( selectionList )
				ToolbarSelect( TB_DELETE_SELECTED );
			else
				ToolbarSelect( TB_CLEARPATH );
		}
		else if ( c == 27 && cameraMode )
		{
			cameraMode = FALSE;
			UpdateStatusString();
		}
		else
		{
			for( i=0; EDKEYMAP[i].command != -1; i++ )
				if( c == EDKEYMAP[i].key )
				{
					ToolbarSelect(EDKEYMAP[i].command);
					return;
				}
			/*
			for (i = 0, p = EDITOR_SHORTCUTS_A; *p; p++, i++)
			{
				if (!TOOLBARBUTTONS[i].command) i++;
				if (*p == c) { ToolbarMouseDown(i); return; }
			}
			if (submenu)
				for (i = 100, p = EDITOR_SHORTCUTS_B; *p; p++, i++)
				{
					if (!submenu[i-100].command) i++;
					if (*p == c) { ToolbarMouseDown(i);	return; }
				}
				*/
		}
	}
}
