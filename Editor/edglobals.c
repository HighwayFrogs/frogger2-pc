/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: edglobals.c
	Programmer	: David Swift
	Date		: 04/08/99

----------------------------------------------------------------------------------------------- */


#include "enemies.h"
#include "define.h"
#include "edglobals.h"
#include "edmaths.h"

const char* TILESTATESTRINGS[NUMTILESTATES] = {
	"Normal",	//	TILESTATE_NORMAL,
	"Deadly",	//	TILESTATE_DEADLY,
	"Sink",		//	TILESTATE_SINK,
	"Ice",		//	TILESTATE_ICE,
	"Superhop",	//	TILESTATE_SUPERHOP,
	"Join",		//	TILESTATE_JOIN,
	"Safe",		//	TILESTATE_SAFE,
	"(reserved)",
	"Grapple",	//	TILESTATE_GRAPPLE,
	"Smash",	//	TILESTATE_SMASH
	"Barred",	//	TILESTATE_BARRED
	"Fall forever", // TILESTATE_FALLFOREVER
};

const char* GARIBTYPESTRINGS[] = {
	"SILVERCOIN_GARIB",
	"EXTRALIFE_GARIB",
	"AUTOHOP_GARIB",
	"LONGTONGUE_GARIB",
	"QUICKHOP_GARIB",
	"INVULNERABILITY_GARIB",
	"GOLDCOIN_GARIB",
	"SLOWHOP_GARIB",
};

const char* CAMTYPESTRINGS[] = {
	"Fixed",
	"Fixed, look at frog",
	"Follow frog",
	"Follow frog, no tilt"
};

const char* HELPSTRINGS[] = 
{
	"F     Place Flag\0",
	"K     Kill Path\0",
	"E     Place Enemy\0",
	"P     Place Platform\0",
	"G     Place Garib\0",
	",     Previous\0",
	".     Next\0",
	"A     Toggle Autocamera\0",
	"I     Information\0",
	"3     Create\0",
	"BS    Delete\0",
	"U     Undo\0",
	"1     Save\0",
	"L     Load\0",
	"S     Multiple Select\0",
	"C     Copy\0",
	"V     Paste\0",
	"D     Pick Flag\0",
	"R     Speed for all Flags\0",
	"B     Both Offsets\0",
	"W     Wait Times\0",
	"Space Cancel Selection\0",
	"H     Help Screen\0",
	"9     Give 999 Lives\0",
	"Q     Place Frog on Tile\0",
	"T     Select by Tile\0",
	"Y     Add Tiles to Camera Case\0",
	"N     Entity Name\0",
	"Z     Set Enemy Scale\0",
	"O     Set Enemy Radius\0",
	"0     Kill Everything\0",
	"\n"
};

/* -------------------------------------------------------------------------

	Global editor variables (lots of them)

   ------------------------------------------------------------------------- */

EDITPATH *editPath = NULL;
EDITPATH *pathList = NULL;
EDITGROUP *createList = NULL;
EDITTEXT editText;
BOOL	drawLinks = FALSE;
BOOL	drawNormals = FALSE;
BOOL	drawVectors = FALSE;
BOOL	drawDots = FALSE;
BOOL	invertMouse = FALSE; // getting like Quake here
float	HorizontalWhizziness = 6.0;
float	InOutWhizziness = 12.0; // for heaven's sake
BOOL	clicking = FALSE;
BOOL	showX = FALSE;
EDVECTOR	clickPos;
POINT	oldMousePos;
short	selectedIcon = -1, garibType = 0;
int		createFlags = ENEMY_NEW_FORWARDS;
BOOL	cameraMode = FALSE;

EDITMODE editMode = EDIT_NONE;
GAMETILE *editTile = NULL;
//CREATEENTITY *editCreate = NULL;
EDITPATHNODE *editFlag = NULL;
EDVECTOR editCameraRot = { 0.0, 0.0, 0.0 };

TOOLBAR_SUBMENU submenu = NULL;

char savePath[MAX_PATH] = EDITOR_SAVE_BASE;
char scriptPath[MAX_PATH] = "";

SELECTFUNC selectFunc = NULL;
EDITGROUP *selectionList = NULL;

CREATEENTITY *copyEntity;
short copyFlag = 0;
short helpScreen = 0;
short addTiles = 0;

char createTypes[2][MAXTYPELENGTH] = { "wasp.obe", "pltlilly.obe" };
