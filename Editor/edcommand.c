/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: edcommand.c
	Programmer	: David Swift
	Date		: 04/08/99

----------------------------------------------------------------------------------------------- */

#include <ultra64.h>
#include <stdio.h>

#include <islutil.h>
#include <islmem.h>

#include "edmaths.h"

#include "editdefs.h"
#include "edcommand.h"
#include "edglobals.h"
#include "edtoolbars.h"
#include "edcamera.h"
#include "editfile.h"
#include "script.h"

#include "edcreate.h"

#include "event.h"
#include "specfx.h"
#include "particle.h"
#include "enemies.h"
#include "platform.h"
#include "collect.h"
#include "cam.h"
#include "layout.h"
#include "pcaudio.h"
#include "babyfrog.h"
#include "pcsprite.h"

/* --------------------------------------------------------------
	Forward declarations
*/

void Select_Create(int ax, int ay, int bx, int by);
void Select_Tiles(int ax, int ay, int bx, int by);
void Select_Flags(int ax, int ay, int bx, int by);

void SetOrientation(const char*);
void SetFlagSound(const char*);
void SetPathSpeed(const char*);
void SetPathOffset(const char*);
void SetPathOffset2(const char*);
void SetAllPathSpeeds(const char*);
void SetAllOffsets(const char*);
void SetAllOffset2s(const char*);
void SetPathBothOffsets(const char*);
void SetAllBothOffsets(const char*);
void SetPathWaitTime(const char*);
void SetAllWaitTimes( const char *str );
void SetCamDirVal(const char* str);
void ResetEnemyCreateFlags( );
void ResetPlatformCreateFlags( );
void InterpolatePathSpeeds( );
void DeleteFlags( );

void FUNC_SetScaleValue( const char *str );
void FUNC_SetRadiusValue( const char *str );
void FUNC_SetAnimSpeedValue( const char *str );
void FUNC_SetValue1( const char *str );

void SetSelectionID(const char *str);

unsigned int FlagFromCommand( int command );
unsigned int FXFlagFromCommand( int command );
void UpdateFlagHilights( );
void UpdateFlagSelection( );
void UpdateStatusString();

void AutomapBabies( );

/*	--------------------------------------------------------------------------------
	Lots of little "set" functions

*/

void SetPSXshift(CREATEENTITY*node, int shift) { node->PSX_shift = (char)shift; }
void SetPSXhack(CREATEENTITY*node, int hack) { node->PSX_hack = (char)hack; }

/*	-------------------------------------------------------------------------------- */


void CreateAndLoadScript(const char *filename)
{
	char *c;
	int i;

	EditorCreateEntities();
	InitTriggerList();
	LoadTestScript(filename);

	strcpy(scriptPath, filename);

	// strip off anything after last '\'
	for (c = scriptPath, i = 0; *c; c++, i++);		// c points to the last char + 1
	while (i--)
	{
		c--;
		if (*c == '\\') { *(c+1) = 0; break; }	
	}
}

unsigned int FXFlagFromCommand( int command )
{
	switch( command )
	{
	case TB_FLAG_MAKERIPPLES:			return EF_RIPPLE_RINGS;
	case TB_FLAG_SMOKE_STATIC:			return EF_SMOKE_STATIC;
	case TB_FLAG_SPARKBURST:			return EF_SPARKBURST;
	case TB_FLAG_GREENGLOOP:			return EF_GREENGLOOP;
	case TB_FLAG_BATSWARM:				return EF_BATSWARM;
	case TB_FLAG_BUBBLES:				return EF_BUBBLES;
	case TB_FLAG_SMOKE_GROWS:			return EF_SMOKE_GROWS;
	case TB_FLAG_FIERYSMOKE:			return EF_FIERYSMOKE;
	case TB_FLAG_SMOKEBURST:			return EF_SMOKEBURST;
	case TB_FLAG_FASTEFFECT:			return EF_FAST;
	case TB_FLAG_MEDIUMEFFECT:			return EF_MEDIUM;
	case TB_FLAG_SLOWEFFECT:			return EF_SLOW;
	case TB_FLAG_TINTRED:				return EF_TINTRED;
	case TB_FLAG_TINTGREEN:				return EF_TINTGREEN;
	case TB_FLAG_TINTBLUE:				return EF_TINTBLUE;
	case TB_FLAG_BUTTERFLYSWARM:		return EF_BUTTERFLYSWARM;
	case TB_FLAG_LASER:					return EF_LASER;
	case TB_FLAG_RANDOMCREATE:			return EF_RANDOMCREATE;
	case TB_FLAG_TRAIL:					return EF_TRAIL;
	case TB_FLAG_BILLBOARDTRAIL:		return EF_BILLBOARDTRAIL;
	case TB_FLAG_LIGHTNING:				return EF_LIGHTNING;
	case TB_FLAG_SPACETHING1:			return EF_SPACETHING1;
	case TB_FLAG_SPARKLYTRAIL:			return EF_SPARKLYTRAIL;
	case TB_FLAG_GLOW:					return EF_GLOW;
	case TB_FLAG_TWINKLE:				return EF_TWINKLE;
	}
	return 0;
}

unsigned int FlagFromCommand( int command )
{
	switch( command )
	{
	case TB_FLAG_PLAT_FORWARDS:			return PLATFORM_NEW_FORWARDS;
	case TB_FLAG_PLAT_BACKWARDS:		return PLATFORM_NEW_BACKWARDS;
	case TB_FLAG_PLAT_CYCLE:			return PLATFORM_NEW_CYCLE;
	case TB_FLAG_PLAT_PINGPONG:			return PLATFORM_NEW_PINGPONG;
	case TB_FLAG_PLAT_MOVEUP:			return PLATFORM_NEW_MOVEUP;
	case TB_FLAG_PLAT_MOVEDOWN:			return PLATFORM_NEW_MOVEDOWN;
	case TB_FLAG_PLAT_STEPACTIVATE:		return PLATFORM_NEW_STEPONACTIVATED;
	case TB_FLAG_PLAT_CRUMBLE:			return PLATFORM_NEW_CRUMBLES;
	case TB_FLAG_PLAT_REGENERATE:		return PLATFORM_NEW_REGENERATES;
	case TB_FLAG_PLAT_NOWALKUNDER:		return PLATFORM_NEW_NOWALKUNDER;
	case TB_FLAG_PLAT_KILLFROG:			return PLATFORM_NEW_KILLSFROG;
	case TB_FLAG_PLAT_FACEFORWARDS:		return PLATFORM_NEW_FACEFORWARDS;
	case TB_FLAG_PLAT_SLERPPATH:		return PLATFORM_NEW_SLERPPATH;
	case TB_FLAG_NME_RADIUSCOLLISION:	return ENEMY_NEW_RADIUSBASEDCOLLISION;
	case TB_FLAG_NME_WATCHFROG:			return ENEMY_NEW_WATCHFROG;
	case TB_FLAG_NME_SNAPFROG:			return ENEMY_NEW_SNAPFROG;
	case TB_FLAG_NME_RANDOMSPEED:		return ENEMY_NEW_RANDOMSPEED;
	case TB_FLAG_NME_FACEFORWARDS:		return ENEMY_NEW_FACEFORWARDS;
	case TB_FLAG_NME_PUSHESFROG:		return ENEMY_NEW_PUSHESFROG;
	case TB_FLAG_NME_HOMING:			return ENEMY_NEW_HOMING;
	case TB_FLAG_NME_ROTATEPATH_XZ:		return ENEMY_NEW_ROTATEPATH_XZ;
	case TB_FLAG_NME_ROTATEPATH_XY:		return ENEMY_NEW_ROTATEPATH_XY;
	case TB_FLAG_NME_ROTATEPATH_ZY:		return ENEMY_NEW_ROTATEPATH_ZY;
	case TB_FLAG_NME_SNAPTILES:			return ENEMY_NEW_SNAPTILES;
	case TB_FLAG_NME_MOVEONMOVE:		return ENEMY_NEW_MOVEONMOVE;
	case TB_FLAG_NME_VENT:				return ENEMY_NEW_VENT;
	case TB_FLAG_NME_NODAMAGE:			return ENEMY_NEW_NODAMAGE;
	case TB_FLAG_NME_FLAPPYTHING:		return ENEMY_NEW_FLAPPYTHING;
	case TB_FLAG_NME_ONEHITKILL:		return ENEMY_NEW_ONEHITKILL;
	case TB_FLAG_NME_SLERPPATH:			return ENEMY_NEW_SLERPPATH;
	case TB_FLAG_NME_BABYFROG:			return ENEMY_NEW_BABYFROG;
	case TB_FLAG_NME_RANDOMMOVE:		return ENEMY_NEW_RANDOMMOVE;
	case TB_FLAG_NME_SHADOW:			return ENEMY_NEW_SHADOW;
	case TB_FLAG_PLAT_SHADOW:			return PLATFORM_NEW_SHADOW;
	case TB_FLAG_NME_TILEHOMING:		return ENEMY_NEW_TILEHOMING;
	case TB_FLAG_NME_BATTLEMODE:		return ENEMY_NEW_BATTLEMODE;
	}
	return 0;
}

/*	------------------------------------------------------------------------------ */

void SetEditorMode(int mode, int icon, TOOLBAR_SUBMENU menu)
{
	editMode = mode;
	selectedIcon = icon;
	showX = TRUE;
	submenu = menu;
	FreeEditGroup(&selectionList);
}

void SpecialEditorSave(const char* str)
{
	releaseQuality = 1;
	EditorSave(str);
	releaseQuality = 0;
}

/*	--------------------------------------------------------------------------------
	Function		: ToolbarSelect
	Parameters		: int
	Returns			: 
*/

void ToolbarSelect(int command)
{
	EDVECTOR edv;

	switch (command)
	{
	case TB_SAVE: // save
		EditVariable("Save path", EDITVAR_PASSTOFUNCTION, EditorSave);
		strcpy(editText.string, savePath);
		break;

	case TB_SAVE_EXTRASPECIAL: // extra special save
		EditVariable("Save special", EDITVAR_PASSTOFUNCTION, SpecialEditorSave);
		strcpy(editText.string, savePath);
		break;

	case TB_LOAD: // load
		copyFlag = 0;
		EditVariable("Load path", EDITVAR_PASSTOFUNCTION, EditorLoad);
		strcpy(editText.string, savePath);
		break;

	case TB_TEST: // test
		EditorCreateEntities();
		break;

	case TB_TEST_SCRIPTING: // test scripting!
		EditVariable("Test script", EDITVAR_PASSTOFUNCTION, CreateAndLoadScript);
		strcpy(editText.string, scriptPath);
		break;
	
	case TB_PLACEFROG:
		SetEditorMode(EDIT_PLACE_FROG, command, NULL);
		break;

	case TB_RECTSELECT:
		SetEditorMode(0, command, NULL);
		SelectMode(Select_Create);
		break;

	case TB_TILESELECT:
		SetEditorMode(0, command, NULL);
		SelectMode(Select_Tiles);
		break;

	case TB_PICKFLAG:
		SetEditorMode(0, command, NULL);
		SelectMode(Select_Flags);
		break;

	case TB_INFO:
		submenu = TOOLBAR_INFO;
		break;

	case TB_DRAW_LINKS:
		drawLinks = !drawLinks;
		ENABLEICON(TOOLBAR_INFO[0], drawLinks);
		break;

	case TB_DRAW_NORMALS:
		drawNormals = !drawNormals;
		ENABLEICON(TOOLBAR_INFO[1], drawNormals);
		break;

	case TB_DRAW_VECTORS:
		drawVectors = !drawVectors;
		ENABLEICON(TOOLBAR_INFO[2], drawVectors);
		break;

	case TB_DRAW_DOTS:
		drawDots = !drawDots;
		ENABLEICON(TOOLBAR_INFO[3], drawDots);
		break;

	case TB_INVERTMOUSE:
		invertMouse = !invertMouse;
		ENABLEICON(TOOLBAR_INFO[5], invertMouse);
		break;

	case TB_INOUTWHIZZY:
		EditVariable("In-out whizziness", EDITVAR_FLOAT, &InOutWhizziness);
		break;

	case TB_HORIZWHIZZY:
		EditVariable("Horizontal whooshitude", EDITVAR_FLOAT, &HorizontalWhizziness);
		break;

	case TB_CLEARLISTS:
		/*editMode = EDIT_CLEAR_LISTS;
		selectedIcon = command;
		showX = FALSE;*/
		submenu = TOOLBAR_CLEAR;
		break;

	case TB_CLEAR_GARIBS:
		FreeGaribList();
		InitGaribList();
		strcpy(statusMessage, "Emptied garib list");
		break;

	case TB_CLEAR_ENEMIES:
		FreeEnemyLinkedList();
		strcpy(statusMessage, "Emptied enemy list");
		break;

	case TB_CLEAR_PLATFORMS:
		FreePlatformLinkedList();
		strcpy(statusMessage, "Emptied platform list");
		break;

	case TB_CLEAR_CREATEENEMY:
		RemoveEntities(CREATE_ENEMY);
		strcpy(statusMessage, "Removed enemy creation flags");
		break;

	case TB_CLEAR_CREATEPLATFORM:
		RemoveEntities(CREATE_PLATFORM);
		strcpy(statusMessage, "Removed platform creation flags");
		break;

	case TB_DELETE_SELECTED:
		if (selectionList)
		{
			DeleteEntitySelection();
			UpdateSelection();
		}
		break;

	case TB_PLACEFLAG:	// place flags
		SetEditorMode(EDIT_MAKE_PATH, command, TOOLBAR_FLAG);
		break;

	case TB_CLEARPATH: // remove flags
		FreeEditPath(editPath);
		editPath = NULL;
		break;


	case TB_UNDO:	// undo
		EditorUndo();
		break;
	
	case TB_PLACEGARIB: // place garib
		SetEditorMode(EDIT_PLACE_GARIB, command, NULL);
		break;

	case TB_NEXT: // next garib type
		if ((++garibType) == NUM_GARIB_TYPES) garibType = 0;
		break;

	case TB_PREV: // prev garib type
		if ((--garibType) < 0) garibType = NUM_GARIB_TYPES - 1;
		break;

	case TB_PLACEENEMY: // place enemy
		SetEditorMode(EDIT_PLACE_ENEMY, command, TOOLBAR_ENEMY);
		break;

	case TB_PLACEPLATFORM: // place platform
		SetEditorMode(EDIT_PLACE_PLATFORM, command, TOOLBAR_PLATFORM);
		break;

	case TB_PLACEPLACEHOLDER: // place platform
		SetEditorMode(EDIT_PLACE_PLACEHOLDER, command, TOOLBAR_PLACEHOLDER);
		break;

	case TB_ATTACHEFFECT:
		editMode = EDIT_EFFECTS;
		selectedIcon = command;
		showX = TRUE;
		submenu = TOOLBAR_EFFECTS;
		break;

	case TB_AUTOCAMERA: // return camera to auto
		if (cameraMode)	cameraMode = FALSE;
		controlCamera = 0;
		TOOLBARBUTTONS[CAMERA_BUTTON].icon = 22;
		InitCamera();
		break;

	case TB_SET_SPEED: // set flag speed
		if (selectionList) EditVariable("Speed", EDITVAR_PASSTOFUNCTION, SetPathSpeed);
		break;

	case TB_SET_OFFSET: // set 'offset'
		if (selectionList) EditVariable("Offset", EDITVAR_PASSTOFUNCTION, SetPathOffset);
		break;

	case TB_SET_OFFSET2: // set 'offset2'
		if (selectionList) EditVariable("Offset2", EDITVAR_PASSTOFUNCTION, SetPathOffset2);
		break;

	case TB_SET_BOTH_OFFSETS: // set both offsets to the same value
		if (selectionList) EditVariable("Both Offsets", EDITVAR_PASSTOFUNCTION, SetPathBothOffsets);
		break;

	case TB_SET_WAIT:
		if (selectionList) EditVariable("Wait time", EDITVAR_PASSTOFUNCTION, SetPathWaitTime);
		break;

	case TB_SET_ID:
		if (selectionList) EditVariable("ID", EDITVAR_PASSTOFUNCTION, SetSelectionID);
		break;

	case TB_SETPATHSPEED: // set speeds
		EditVariable("Speed for all flags", EDITVAR_PASSTOFUNCTION, FUNC_SetPathSpeed);
		break;

	case TB_SET_ENTITYTYPE: // choose enemy/plat type
		if (selectionList)
		{
			EditVariable("actor type", EDITVAR_PASSTOFUNCTION, SetSelectionName);
			editText.charLimit = MAXTYPELENGTH - 1;
		}
		break;

	case TB_TILESTATE_NEXT:
		sprintf(statusMessage, "Set tile state to %s",
			TILESTATESTRINGS[NextTileSelectionState(selectionList)]);
		break;
		
	case TB_TILESTATE_PREV:
		sprintf(statusMessage, "Set tile state to %s",
			TILESTATESTRINGS[PrevTileSelectionState(selectionList)]);
		break;

	case TB_MAKECAMERA_FOLLOWFROG:
		if (selectionList)
		{
			SetVectorS(&edv, &camDist);
			ScaleVector(&edv, 0.1f);
			EditorAddCameraCase(selectionList, LOOK_AT_FROG, &edv);
			UpdateSelection( );
		}
		break;

	case TB_MAKECAMERA_FOLLOWFROG_NOTILT:
		if (selectionList)
		{
			SetVectorS(&edv, &camDist);
			ScaleVector(&edv, 0.1f);
			EditorAddCameraCase(selectionList, LOOK_AT_FROG_NOTILT, &edv);
			UpdateSelection( );
		}
		break;

	case TB_MAKECAMERA_WATCHFROG:
		if (selectionList)
		{
			SetVectorF(&edv, &currCamSource);
			ScaleVector(&edv, 0.1f);
			EditorAddCameraCase(selectionList, FIXED_SOURCE, &edv);
			UpdateSelection( );
		}
		break;

	case TB_MAKECAMERA_STATIC:
		if (selectionList)
		{
			SetVectorF(&edv, &currCamSource);
			ScaleVector(&edv, 0.1f);
			EditorAddCameraCase(selectionList, STATIC_CAMERA, &edv);
			UpdateSelection( );
		}
		break;

	case TB_MAKECAMERA_WATCHTILE:
		if (selectionList)
		{
			SetVectorF(&edv, &currCamSource);
			ScaleVector(&edv, 0.1f);
			EditorAddCameraCase(selectionList, LOOK_AT_TILE, &edv);
			UpdateSelection( );
		}
		break;

	case TB_MAKECAMERA_LOOKDIR:
		if (selectionList)
		{
			EDVECTOR dir, a, b;
			CREATEENTITY *cam;
			
			SetVectorF(&edv, &currCamSource);
			ScaleVector(&edv, 0.1f);
			cam = EditorAddCameraCase(selectionList, LOOK_IN_DIR, &edv);
			
			SetVectorF(&a, &currCamTarget);
			SetVectorF(&b, &currCamSource);
			SubVector(&dir, &a, &b);
			EdMakeUnit( &dir );

			cam->startNode = 0;
			cam->startNode |= ((char)(dir.vx*0x7F))&0x000000FF;
			cam->startNode |= (((char)(dir.vy*0x7F))<<8)&0x0000FF00;
			cam->startNode |= (((char)(dir.vz*0x7F))<<16)&0x00FF0000;
			UpdateSelection( );
		}
		break;

	case TB_999LIVES:
		player[0].lives = 999;
		break;

	case TB_TESTING:
		submenu = TOOLBAR_TESTING;
		strcpy(statusMessage, "Please don't feed the sheep");
		break;

	case TB_ASSIGNPATH:
		AssignSamePath(selectionList);
		UpdateSelection();
		break;

	case TB_SCALE:
		if( selectionList )
			EditVariable( "Scale selection", EDITVAR_PASSTOFUNCTION, FUNC_SetScaleValue );
		break;

	case TB_RADIUS:
		if( selectionList )
			EditVariable( "Set radius", EDITVAR_PASSTOFUNCTION, FUNC_SetRadiusValue );
		break;

	case TB_ANIMSPEED:
		if( selectionList )
			EditVariable( "Set Animation Speed", EDITVAR_PASSTOFUNCTION, FUNC_SetAnimSpeedValue );
		break;

	case TB_RESET_ENEMYFLAGS:
		ResetEnemyCreateFlags( );
		break;

	case TB_RESET_PLATFORMFLAGS:
		ResetPlatformCreateFlags( );
		break;

	case TB_COPY_SELECTION:
		CopyEntitySelection(selectionList);
		break;

	case TB_SET_ALL_PATHSPEEDS:
		if (selectionList) EditVariable("All Speeds", EDITVAR_PASSTOFUNCTION, SetAllPathSpeeds);
		break;

	case TB_SET_ALL_OFFSETS:
		if (selectionList) EditVariable("All Offsets", EDITVAR_PASSTOFUNCTION, SetAllOffsets);
		break;

	case TB_SET_ALL_OFFSET2S:
		if (selectionList) EditVariable("All Offset2s", EDITVAR_PASSTOFUNCTION, SetAllOffset2s);
		break;

	case TB_SET_ALL_BOTHOFFSETS:
		if (selectionList) EditVariable("All Both Offsets", EDITVAR_PASSTOFUNCTION, SetAllBothOffsets);
		break;

	case TB_SET_ALL_WAITTIMES:
		if (selectionList) EditVariable("All Wait Times", EDITVAR_PASSTOFUNCTION, SetAllWaitTimes);
		break;

	case TB_SET_VALUE1:	// a miscellaneous value
		if (selectionList) EditVariable("Value 1", EDITVAR_PASSTOFUNCTION, FUNC_SetValue1);
		break;

	case TB_SET_PSXSHIFT:
		EditVariable("PSX sort hack", EDITVAR_GROUPFUNCINT, SetPSXshift);
		break;

	case TB_SET_PSXHACK:
		EditVariable("PSX shite hack", EDITVAR_GROUPFUNCINT, SetPSXhack);
		break;

	case TB_PASTE_SELECTION:
		copyFlag = (copyFlag) ? 0 : 1;
		break;

	case TB_CLEAR_SELECTION:
		if( selectionList )
		{
			FreeEditGroup(&selectionList);
			selectionList = MakeEditGroup();
			UpdateSelection( );
		}
		break;

	case TB_HELPSCREEN:
		helpScreen = (helpScreen) ? 0 : 1;
		break;

	case TB_INTERP_SPEEDS:
		if( selectionList ) InterpolatePathSpeeds( );
		break;

	case TB_DELETE_FLAG:
		if( selectionList ) DeleteFlags( );
		break;

	case TB_CAMERACASE_ADDTILES:
		if( selectionList ) addTiles = (addTiles) ? 0 : 1;
		break;

	case TB_CAMERACASE_SETDIRECTION:
		if (selectionList) EditVariable("Direction for camera (0 for ANY)", EDITVAR_PASSTOFUNCTION, SetCamDirVal);
		break;

	case TB_CAMERACASE_SETFOV:
		if (selectionList) EditVariable("Field of view", EDITVAR_PASSTOFUNCTION, FUNC_SetScaleValue);
		strcpy(editText.string, "45.0");
		break;

	case TB_CAMERACASE_SETSPEED:
		if (selectionList) EditVariable("Camera Speed", EDITVAR_PASSTOFUNCTION, FUNC_SetAnimSpeedValue);
		strcpy(editText.string, "1.0");
		break;

	case TB_AUTOMAP_BABIES:
		AutomapBabies( );
		break;

	case TB_SET_FLAGSOUND:
		if (selectionList) EditVariable("flag sound", EDITVAR_PASSTOFUNCTION, SetFlagSound);
		break;

	case TB_SET_ORIENTATION:
		if (selectionList) EditVariable("entity facing", EDITVAR_PASSTOFUNCTION, SetOrientation);
		break;

	case TB_KILL_EVERYTHING:
		FreeAmbientSoundList();
		FreeSpecFXList( );
		InitSpecFXList( );
		ResetBabies( );
		FreePlatformLinkedList();
		FreeEnemyLinkedList();
		FreeGaribList();
		InitGaribList();
		FreeTransCameraList();
		FreePathList();
		KillAllTriggers( );
		FreeSpriteList( );
		InitSpriteList( );
		InitSpriteSortArray( );
		FreeParticleList( );
		InitParticleList( );
		RemoveEntities( CREATE_ENEMY );
		RemoveEntities( CREATE_PLATFORM );
		RemoveEntities( CREATE_CAMERACASE );
		RemoveEntities( CREATE_GARIB );
		RemoveEntities( CREATE_PLACEHOLDER );
		FreeEditGroup( &selectionList );

		strcpy(statusMessage, "Everything is Dead");
		break;

	case TB_FLAG_MAKERIPPLES:
	case TB_FLAG_SMOKE_STATIC:
	case TB_FLAG_SPARKBURST:
	case TB_FLAG_GREENGLOOP:
	case TB_FLAG_BATSWARM:
	case TB_FLAG_BUBBLES:
	case TB_FLAG_SMOKE_GROWS:
	case TB_FLAG_FASTEFFECT:
	case TB_FLAG_MEDIUMEFFECT:
	case TB_FLAG_SLOWEFFECT:
	case TB_FLAG_SMOKEBURST:
	case TB_FLAG_FIERYSMOKE:
	case TB_FLAG_TINTRED:
	case TB_FLAG_TINTGREEN:
	case TB_FLAG_TINTBLUE:
	case TB_FLAG_BUTTERFLYSWARM:
	case TB_FLAG_LASER:
	case TB_FLAG_RANDOMCREATE:
	case TB_FLAG_TRAIL:
	case TB_FLAG_BILLBOARDTRAIL:
	case TB_FLAG_LIGHTNING:
	case TB_FLAG_SPACETHING1:
	case TB_FLAG_SPARKLYTRAIL:
	case TB_FLAG_GLOW:
	case TB_FLAG_TWINKLE:
		{
			EDITGROUPNODE *node;
			unsigned int fl = FXFlagFromCommand(command);
			// Update flags for selected entities
			if (!selectionList) return;
			for (node = selectionList->nodes; node; node=node->link)
				if( ((CREATEENTITY*)node->thing)->effects & fl )
					((CREATEENTITY*)node->thing)->effects &= ~fl;
				else
					((CREATEENTITY*)node->thing)->effects |= fl;
		}
		break;

	/* Put all flag toggle commands here */
	case TB_FLAG_PLAT_FORWARDS:
	case TB_FLAG_PLAT_BACKWARDS:
	case TB_FLAG_PLAT_CYCLE:
	case TB_FLAG_PLAT_PINGPONG:
	case TB_FLAG_PLAT_MOVEUP:
	case TB_FLAG_PLAT_MOVEDOWN:
	case TB_FLAG_PLAT_STEPACTIVATE:
	case TB_FLAG_PLAT_CRUMBLE:
	case TB_FLAG_PLAT_REGENERATE:
	case TB_FLAG_PLAT_NOWALKUNDER:
	case TB_FLAG_PLAT_KILLFROG:
	case TB_FLAG_PLAT_FACEFORWARDS:
	case TB_FLAG_PLAT_SHADOW:
	case TB_FLAG_PLAT_SLERPPATH:
	case TB_FLAG_NME_RADIUSCOLLISION:
	case TB_FLAG_NME_WATCHFROG:
	case TB_FLAG_NME_SNAPFROG:
	case TB_FLAG_NME_RANDOMSPEED:
	case TB_FLAG_NME_FACEFORWARDS:
	case TB_FLAG_NME_PUSHESFROG:
	case TB_FLAG_NME_HOMING:
	case TB_FLAG_NME_ROTATEPATH_XZ:
	case TB_FLAG_NME_ROTATEPATH_XY:
	case TB_FLAG_NME_ROTATEPATH_ZY:
	case TB_FLAG_NME_SNAPTILES:
	case TB_FLAG_NME_MOVEONMOVE:
	case TB_FLAG_NME_VENT:
	case TB_FLAG_NME_NODAMAGE:
	case TB_FLAG_NME_FLAPPYTHING:
	case TB_FLAG_NME_SLERPPATH:
	case TB_FLAG_NME_ONEHITKILL:
	case TB_FLAG_NME_BABYFROG:
	case TB_FLAG_NME_RANDOMMOVE:
	case TB_FLAG_NME_SHADOW:
	case TB_FLAG_NME_TILEHOMING:
	case TB_FLAG_NME_BATTLEMODE:
		{
			EDITGROUPNODE *node;
			unsigned int fl = FlagFromCommand(command);
			if( createFlags & fl )
				createFlags &= ~fl;
			else
				createFlags |= fl;

			// Update flags for selected entities
			if (!selectionList) return;
			for (node = selectionList->nodes; node; node=node->link)
				((CREATEENTITY*)node->thing)->flags = createFlags;
		}
		break;
	}

	UpdateFlagHilights( );
	UpdateStatusString();
}


void UpdateFlagHilights( )
{
	long i;
	unsigned int fl = 0, 
		effectFlags = ~0; // All bits set

	EDITGROUPNODE *node;

	if( submenu )
	{
		if( submenu != TOOLBAR_EFFECTS )
		{
			for (i = 0; submenu[i].command != -1; i++)
			{
				fl = FlagFromCommand(submenu[i].command);

				if(!fl) continue;

				if( createFlags & fl )
					submenu[i].icon |= 0x8000;
				else
					submenu[i].icon &= ~0x8000;
			}
		}
		else if( selectionList )
		{
			// Set all common effect flags
			for (node = selectionList->nodes; node; node=node->link)
				effectFlags &= ((CREATEENTITY*)node->thing)->effects;

			for (i = 0; submenu[i].command != -1; i++)
			{
				fl = FXFlagFromCommand(submenu[i].command);

				if(!fl) continue;

				if( effectFlags & fl )
					submenu[i].icon |= 0x8000;
				else
					submenu[i].icon &= ~0x8000;
			}
		}
	}

	for (i = 0; TOOLBARBUTTONS[i].command != -1; i++)
	{
		fl = FlagFromCommand(TOOLBARBUTTONS[i].command);

		if(!fl) continue;

		if( createFlags & fl )
			TOOLBARBUTTONS[i].icon |= 0x8000;
		else
			TOOLBARBUTTONS[i].icon &= ~0x8000;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: EditorUndo
	Parameters		: 
	Returns			: 
*/

void EditorUndo()
{
	EDITPATHNODE *node, *prev;

	if (editPath && editPath->nodes)
	{
		if (editPath->nodes->link)
		{
			for (node = editPath->nodes; node->link; prev = node, node = node->link);
			prev->link = NULL; free(node);
		}
		else
		{
			free(editPath->nodes);
			editPath->nodes = NULL;
		}
	}
}


/*	--------------------------------------------------------------------------------
	Function		: ResetEntityFlags
	Parameters		: 
	Returns			: 
*/
void ResetEnemyCreateFlags( )
{
	EDITGROUPNODE *node;
	
	if (!selectionList) return;

	for (node = selectionList->nodes; node; node = node->link)
		((CREATEENTITY*)node->thing)->flags = ENEMY_NEW_FORWARDS;

	createFlags = ENEMY_NEW_FORWARDS;
}

void ResetPlatformCreateFlags( )
{
	EDITGROUPNODE *node;
	
	if (!selectionList) return;

	for (node = selectionList->nodes; node; node = node->link)
		((CREATEENTITY*)node->thing)->flags = PLATFORM_NEW_FORWARDS;

	createFlags = ENEMY_NEW_FORWARDS;
}

/*	--------------------------------------------------------------------------------
	Function		: CopyEntitySelection
	Parameters		: 
	Returns			: 
*/
void DeleteEntity( CREATEENTITY **e )
{
	if( *e )
	{
		free( e );
		*e = NULL;
		e = NULL;
	}
}

void CopyEntitySelection( EDITGROUP *group )
{
	int count, flags;
	EDITGROUPNODE *node;
	CREATEENTITY *create;
	int type;
	char name[MAXTYPELENGTH];
	float scale, radius, animSpeed;

	// If no group, or group is empty, or multiple types of entity are selected, do not copy
	if( !group )
	{
		DeleteEntity( &copyEntity );
		return;
	}

	count = CountGroupMembers(group);
	if( !count ) 
	{
		DeleteEntity( &copyEntity );
		return;
	}

	node = group->nodes;
	type = ((CREATEENTITY*)node->thing)->thing;
	for (node = node->link; node; node = node->link)
		if (((CREATEENTITY*)node->thing)->thing != type)
		{
			DeleteEntity(&copyEntity);
			return;
		}

	// If selection is OK, take the union of the flags and fill out the copy entity
	node = group->nodes;
	create = (CREATEENTITY*)node->thing;
	flags = create->flags;
	strcpy(name, create->type);
	// Only copy these once - an average wouldn't make sense
	scale = create->scale;
	radius = create->radius;
	animSpeed = create->animSpeed;

	for (node = node->link; node; node = node->link)
	{
		create = (CREATEENTITY*)node->thing;
		flags &= create->flags;
		if (strcmp(create->type, name) != 0) name[0] = 0;
	}

	if( !copyEntity )
		copyEntity = calloc(1,sizeof(CREATEENTITY)); //(CREATEENTITY *)JallocAlloc( sizeof(CREATEENTITY),YES,"copyEntity" );

	strcpy(copyEntity->type, name);
	copyEntity->thing = type;
	copyEntity->flags = flags;
	copyEntity->scale = scale;
	copyEntity->radius = radius;
	copyEntity->animSpeed = animSpeed;

	createFlags = flags;
}

/*	--------------------------------------------------------------------------------
	Function		: RemoveEntities
	Parameters		: 
	Returns			: 
*/

void RemoveEntities(int type)
{
	EDITGROUPNODE *node, *next;

	for (node = createList->nodes; node; node=next)
	{
		next = node->link;
		
		if (((CREATEENTITY*)node->thing)->thing == type) EditorSubCreateNode(node->thing);
	}
}


/*	--------------------------------------------------------------------------------
	Function		: SelectMode
	Parameters		: SELECTFUNC
	Returns			: 
*/

void SelectMode(SELECTFUNC func)
{
	submenu = NULL;
	editMode = EDIT_RECT_SELECT;
	selectFunc = func;
	if (selectionList) FreeEditGroup(&selectionList);
	selectionList = MakeEditGroup();
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateSelection
	Parameters		: void
	Returns			: 
*/

void UpdateSelection(void)
{
	int count, flags;
	EDITGROUPNODE *node;
	CREATEENTITY *create;
	EDITPATH *path;
	int type;
	BOOL multi;
	char name[MAXTYPELENGTH];

	count = CountGroupMembers(selectionList);

	FreeEditPath(editPath); editPath = NULL;

	sprintf(statusMessage, "Selected %d items", count);

	if (count == 0)
		return;
	else
	{
		node = selectionList->nodes;
		type = ((CREATEENTITY*)node->thing)->thing;

		for (node = node->link, multi = FALSE; node; node = node->link)
			if (((CREATEENTITY*)node->thing)->thing != type)
			{
				multi = TRUE; break;
			}

		if (multi)
		{
			// multiple selection
			submenu = TOOLBAR_MULTI;
			sprintf(statusMessage, "Multiple selection of %d items", count);
		}
		else
		{
			// set common stuff
			node = selectionList->nodes;
			create = (CREATEENTITY*)node->thing;
			flags = create->flags;
			path = create->path;
			strcpy(name, create->type);

			for (node = node->link, multi = FALSE; node; node = node->link)
			{
				create = (CREATEENTITY*)node->thing;
				flags &= create->flags;
				if (strcmp(create->type, name) != 0) name[0] = 0;
				if (create->path != path) path = NULL;
			}
			
			createFlags = flags;
			editPath = path; if (path) path->refs++;
			strcpy(createTypes[type], name);

			switch (type)
			{
			case CREATE_ENEMY:
				submenu = TOOLBAR_ENEMY;
				if( count == 1 )
					sprintf(statusMessage, 
							"Enemy: %s: ID %i: Radius %.2f Value1: %.2f Scale: %.2f Depth: %d",
							name, create->ID, create->radius, create->value1, create->scale, create->PSX_shift );
				else
					sprintf(statusMessage, "Selected %d enemies: %s", count, name );
				break;
			case CREATE_PLATFORM:
				submenu = TOOLBAR_PLATFORM;
				if( count == 1 )
					sprintf(statusMessage, 
							"Platform: %s: ID %i: AnimSpeed: %.2f Scale: %.2f Depth: %d",
							name, create->ID, create->animSpeed, create->scale, create->PSX_shift );
				else
					sprintf(statusMessage, "Selected %d platforms: %s", count, name );
				break;
			case CREATE_PLACEHOLDER:
				submenu = TOOLBAR_PLACEHOLDER;
				if( count == 1 )
					sprintf(statusMessage, "Placeholder: ID %i", create->ID );
				else
					sprintf(statusMessage, "Selected %d placeholders", count );
				break;
			case CREATE_GARIB:
				submenu = TOOLBAR_GARIB;
				if( count == 1 )
					sprintf(statusMessage,
							"Garib: %s: O1 %.2f: O2 %.2f", 
							GARIBTYPESTRINGS[create->flags], create->path->nodes->offset, create->path->nodes->offset2 );
				else
					sprintf(statusMessage, "Selected %d garibs", count );
				break;
			case CREATE_CAMERACASE:
				submenu = TOOLBAR_CAMERA;
				if( count == 1 )
					sprintf( statusMessage,	"Camera transition: %s, dir %d", CAMTYPESTRINGS[create->flags & 0xF], (create->flags>>16));
				else
					sprintf( statusMessage,	"Selected %d camera transitions", count );
				break;
			default:
				submenu = TOOLBAR_MULTI;
				break;
			}

			UpdateFlagHilights( );
		}
	}
}

/*	--------------------------------------------------------------------------------
	Function		: UpdateFlagSelection
	Parameters		: 
	Returns			: 
*/
void UpdateFlagSelection( )
{
	int count;

	if( !selectionList )
		return;

	count = CountGroupMembers(selectionList);

	sprintf(statusMessage, "Selected %d flags", count);

	if( !count )
		return;


	// Multiple flags probably share no useful data (floating point values)
	if( count == 1 )
	{
		int f;
		EDITPATHNODE *node, *n;
		SAMPLE *sample;

		node = (EDITPATHNODE *)selectionList->nodes->thing;
		
		for (f = 0, n = editPath->nodes; n; n = n->link, f++) if (node == n) break;
		
//		if( (sample = FindSample(node->sample)) )
//			sprintf( statusMessage, "Flag #%d: Speed %.2f, Offset %.2f, Offset2 %.2f, Wait %i, Sample %s", f, node->speed, node->offset, node->offset2, node->waitTime, sample->idName );
//		else
			sprintf( statusMessage, "Flag #%d: Speed %.2f, Offset %.2f, Offset2 %.2f, Wait %i", f, node->speed, node->offset, node->offset2, node->waitTime );
	}

	submenu = TOOLBAR_FLAG;
}

/*	--------------------------------------------------------------------------------
	Function		: Select_Create
	Parameters		: 
	Returns			: 
*/
void Select_Create(int ax, int ay, int bx, int by)
{
	ToggleRectSelection(ax, ay, bx, by, createList, selectionList);
	UpdateSelection();
}

const char* TileStateName(int state)
{
	if (state < NUMTILESTATES)
		return TILESTATESTRINGS[state];
	else
	if (state & TILESTATE_CONVEYOR)
		return "Conveyor";

	return NULL;
}

void Select_Tiles(int ax, int ay, int bx, int by)
{
	EDITGROUPNODE *node;
	GAMETILE *tile;
	int count, state;

	ToggleRectTileSelection(ax, ay, bx, by, selectionList);

	if (!selectionList->nodes)
	{
		strcpy(statusMessage, "No tiles selected");
		return;
	}

	node = selectionList->nodes;
	tile = (GAMETILE*)node->thing;
	state = tile->state;
	count = 1;

	for (node = node->link; node; node = node->link)
	{
		tile = (GAMETILE*)node->thing;
		if (tile->state != state)
		{
			strcpy(statusMessage, "Multiple tile types selected");
			state = -1;
			break;	
		}
		count++;
	}

	if (state != -1)
		sprintf(statusMessage, "Selected %d '%s' tile%s", count, TileStateName(state), (count==1)?"":"s");

	submenu = TOOLBAR_TILES;
}

void Select_Flags(int ax, int ay, int bx, int by)
{
	ToggleRectFlagSelection(ax, ay, bx, by, selectionList, editPath);
	UpdateFlagSelection( );
}

void AddTilesToCameraCase(int ax, int ay, int bx, int by)
{
	ToggleRectTileSelection(ax, ay, bx, by, ((CREATEENTITY *)selectionList->nodes->thing)->group );
	submenu = TOOLBAR_CAMERA;
}

/*	--------------------------------------------------------------------------------
	Function		: Select_Create
	Parameters		: 
	Returns			: 
*/

void SetSelectionName(const char* str)
{
	EDITGROUPNODE *node;
	for (node = selectionList->nodes; node; node=node->link)
		strcpy(((CREATEENTITY*)node->thing)->type, str);
}

/*	--------------------------------------------------------------------------------
	Function		: Select_Create
	Parameters		: 
	Returns			: 
*/

void SetOrientation(const char* str)
{
	EDITGROUPNODE *node; unsigned char value = atoi(str);
	for (node = selectionList->nodes; node; node=node->link)
		((CREATEENTITY*)node->thing)->facing = value;
}

/*	--------------------------------------------------------------------------------
	Function		: SetFlagSound
	Parameters		: 
	Returns			: 
*/
void SetFlagSound(const char* str)
{
	unsigned long uid;
	EDITGROUPNODE *node; 
	char name[16];
	int count=0;

	while( str[count] != '\0' && str[count] != '.' )
	{
		name[count] = str[count];
		count++;
	}
	name[count] = '\0';
	strlwr(name);

	uid = utilStr2CRC(name);

	for (node = selectionList->nodes, count=0; node; node = node->link, count++)
		((EDITPATHNODE*)node->thing)->sample = uid;
	sprintf(statusMessage, "Set flag sample to %s for %d flags", name, count);
}



/*	--------------------------------------------------------------------------------
	Function		: AssignSamePath
	Parameters		: 
	Returns			: 
*/

void AssignSamePath(EDITGROUP *group)
{
	EDITPATH *path;
	EDITGROUPNODE *node;
	CREATEENTITY *create;
	int checked = 0, assigned = 0;
	
	node = group->nodes;
	if (!(node && node->link)) return;
	path = ((CREATEENTITY*)node->thing)->path;

	for (node = node->link; node; node = node->link, checked++)
	{
		create = (CREATEENTITY*)node->thing;

		if (create->path != path)
		{
			int n;
			GAMETILE *startTile;
			EDITPATHNODE *start = create->path->nodes;
		
			// Find starting gametile
			for (n = create->startNode; n; n--, start = start->link);
			
			startTile = start->tile;

			for (n = 0, start = path->nodes; start; n++, start = start->link)
			{
				if (start->tile == startTile)
				{
					FreeEditPath(create->path);
					create->path = path; path->refs++;
					create->startNode = n;
					assigned++;
					break;
				}
			}
		}
	}

	sprintf(statusMessage, "Assigned to %d (of %d)", assigned, checked);
}

/*	--------------------------------------------------------------------------------
	Function		: FUNC_SetValue1
	Parameters		: 
	Returns			: 
*/

void FUNC_SetValue1( const char *str )
{
	EDITGROUPNODE *node;
	float value1;
	
	if (!selectionList) return;

	value1 = (float)atof(str);
	for (node = selectionList->nodes; node; node = node->link)
		((CREATEENTITY*)node->thing)->value1 = value1;
}

void SetSelectionID(const char *str)
{
	EDITGROUPNODE *node;
	int id;
	
	if (!selectionList) return;

	id = atoi(str);
	for (node = selectionList->nodes; node; node = node->link)
		((CREATEENTITY*)node->thing)->ID = id;
}

/*	--------------------------------------------------------------------------------
	Function		: FUNC_SetScaleValue
	Parameters		: 
	Returns			: 
*/

void FUNC_SetScaleValue( const char *str )
{
	EDITGROUPNODE *node;
	float scale;
	
	if (!selectionList) return;

	scale = (float)atof(str);
	for (node = selectionList->nodes; node; node = node->link)
		((CREATEENTITY*)node->thing)->scale = scale;
}

/*	--------------------------------------------------------------------------------
	Function		: FUNC_SetScaleValue
	Parameters		: 
	Returns			: 
*/

void FUNC_SetAnimSpeedValue( const char *str )
{
	EDITGROUPNODE *node;
	float animSpeed;
	
	if (!selectionList) return;

	animSpeed = (float)atof(str);
	for (node = selectionList->nodes; node; node = node->link)
		((CREATEENTITY*)node->thing)->animSpeed = animSpeed;
}

/*	--------------------------------------------------------------------------------
	Function		: FUNC_SetRadiusValue
	Parameters		: 
	Returns			: 
*/

void FUNC_SetRadiusValue( const char *str )
{
	EDITGROUPNODE *node;
	float radius;
	
	if (!selectionList) return;

	radius = (float)atof(str);
	for (node = selectionList->nodes; node; node = node->link)
		((CREATEENTITY*)node->thing)->radius = radius;
}

/*	--------------------------------------------------------------------------------
	Function		: DeleteEntitySelection
	Parameters		: 
	Returns			: 
*/

void DeleteEntitySelection(void)
{
	EDITGROUPNODE *node;

	if (!selectionList) return;

	for (node = selectionList->nodes; node; node = node->link)
	{
		EditorSubCreateNode((CREATEENTITY*)node->thing);
	}

	FreeEditGroup(&selectionList);
	selectionList = MakeEditGroup();
}

int GetNodeIndex(EDITPATH *path, EDITPATHNODE *foo)
{
	int i;
	EDITPATHNODE *node;

	if (!path) return -1;

	for (i=0, node = path->nodes; node; node = node->link, i++)
		if (node==foo) return i;

	return -1;
}

/*	--------------------------------------------------------------------------------
	Function		: SetSelection
	Parameters		: 
	Returns			: 
*/

void SetSelection(CREATEENTITY *entity, EDVECTOR *iconPos)
{
	// Replace selection list with item
	FreeEditGroup( &selectionList );
	selectionList = MakeEditGroup( );
	AddGroupMember( entity, iconPos, selectionList );
	UpdateSelection( );
}

/*	--------------------------------------------------------------------------------
	Function		: PlaceEnemy
	Parameters		: 
	Returns			: 
*/

void PlaceEnemy(EDITPATHNODE *node)
{
	CREATEENTITY *e;
	EDVECTOR v;
	int flags = 0;

	if (!editPath->nodes) return;
	if (editPath->nodes->link) { // if there's more than one node
		if (node->link) // if not last flag
			flags |= ENEMY_NEW_FORWARDS;
		else
			flags |= ENEMY_NEW_BACKWARDS;
	}

	e = EditorAddCreateNode(createTypes[0], flags, 0, CREATE_ENEMY,
		editPath, GetNodeIndex(editPath, node));

	SetVectorS(&v, &node->tile->centre);
	SetSelection(e, &v);
}

/*	--------------------------------------------------------------------------------
	Function		: PlacePlatform
	Parameters		: 
	Returns			: 
*/

void PlacePlatform(EDITPATHNODE *node)
{
	CREATEENTITY *e;
	EDVECTOR v;
	int flags = 0;

	if (!editPath->nodes) return;
	if (editPath->nodes->link) { // if there's more than one node
		if (node->link) // if not last flag
			flags |= ENEMY_NEW_FORWARDS;
		else
			flags |= ENEMY_NEW_BACKWARDS;
	}

	e = EditorAddCreateNode(createTypes[1], flags, 0, CREATE_PLATFORM,
			editPath, GetNodeIndex(editPath, node));

	SetVectorS(&v, &node->tile->centre);
	SetSelection(e, &v);
}

void PlacePlaceholder(EDITPATHNODE *node)
{
	CREATEENTITY *e;
	EDVECTOR v;
	if (!editPath->nodes) return;

	e = EditorAddCreateNode("placeholder", 0, 0, CREATE_PLACEHOLDER,
			editPath, GetNodeIndex(editPath, node));

	SetVectorS(&v, &node->tile->centre);
	SetSelection(e, &v);
}

/*	--------------------------------------------------------------------------------
	Function		: PlaceGarib
	Parameters		: 
	Returns			: 
*/

void PlaceGarib(GAMETILE *tile, int garibType)
{
	CREATEENTITY *e;
	EDVECTOR v;
	EDITPATH *path;

	path = CreateEditPath();
	EditorAddFlag(tile, path);
	path->nodes->offset = 15;
	e = EditorAddCreateNode("", garibType, 0, CREATE_GARIB, path, NULL);
	SetVectorF(&v, &tile->centre);
	SetSelection(e, &v);
	FreeEditPath(path);
}


/*	-------------------------------------------------------------------------------- */

void SetPathSpeed(const char* str)
{
	EDITGROUPNODE *node; int count; float value = (float)atof(str);
	for (node = selectionList->nodes, count=0; node; node = node->link, count++)
		((EDITPATHNODE*)node->thing)->speed = value;
	sprintf(statusMessage, "Set speed to %f for %d flags", value, count);
}

void SetPathOffset(const char* str)
{
	EDITGROUPNODE *node; int count; float value = (float)atof(str);
	for (node = selectionList->nodes, count=0; node; node = node->link, count++)
		((EDITPATHNODE*)node->thing)->offset = value;
	sprintf(statusMessage, "Set offset to %f for %d flags", value, count);
}

void SetPathOffset2(const char* str)
{
	EDITGROUPNODE *node; int count; float value = (float)atof(str);
	for (node = selectionList->nodes, count=0; node; node = node->link, count++)
		((EDITPATHNODE*)node->thing)->offset2 = value;
	sprintf(statusMessage, "Set offset2 to %f for %d flags", value, count);
}

void SetPathBothOffsets( const char *str )
{
	EDITGROUPNODE *node; int count; float value = (float)atof(str);
	for (node = selectionList->nodes, count=0; node; node = node->link, count++)
	{
		((EDITPATHNODE*)node->thing)->offset = value;
		((EDITPATHNODE*)node->thing)->offset2 = value;
	}
	sprintf(statusMessage, "Set both offsets to %f for %d flags", value, count);
}

void SetPathWaitTime(const char* str)
{
	EDITGROUPNODE *node; int count; int value = atoi(str);
	for (node = selectionList->nodes, count=0; node; node = node->link, count++)
		((EDITPATHNODE*)node->thing)->waitTime = value;
	sprintf(statusMessage, "Set wait time to %d for %d flags", value, count);
}

void InterpolatePathSpeeds( )
{
	EDITGROUPNODE *snode, *enode;
	long count;
	float diff;

	snode = selectionList->nodes;
	// Count entries and set end node
	for (enode = snode, count=0; enode->link; enode = enode->link, count++);

	// Difference between speeds of each flag
	diff = (((EDITPATHNODE *)enode->thing)->speed - ((EDITPATHNODE *)snode->thing)->speed) / (float)count;

	for( ; snode->link; snode = snode->link )
		((EDITPATHNODE *)snode->link->thing)->speed = ((EDITPATHNODE *)snode->thing)->speed + diff;

	sprintf(statusMessage, "Set acceleration to %.4f for %d flags\n", diff, count+1);
}

void SetAllPathSpeeds(const char* str)
{
	EDITGROUPNODE *enode; 
	EDITPATHNODE *fnode;
	int count;
	float value = (float)atof(str);

	for (enode = selectionList->nodes, count=0; enode; enode = enode->link, count++)
		for (fnode = ((CREATEENTITY*)enode->thing)->path->nodes; fnode; fnode = fnode->link)
			fnode->speed = value;

	sprintf(statusMessage, "Set speed to %f for %d paths", value, count);
}

void SetCamDirVal(const char* str)
{
	EDITGROUPNODE *foo; int val, *flags;

	val=atoi(str);

	for (foo=selectionList->nodes; foo; foo=foo->link)
	{
		flags = &((CREATEENTITY*)foo->thing)->flags;
		*flags = (*flags & 0xFFFF) + (val << 16);
	}
}

void SetAllWaitTimes(const char* str)
{
	EDITGROUPNODE *enode; 
	EDITPATHNODE *fnode;
	int count;
	int value = atoi(str);

	for (enode = selectionList->nodes, count=0; enode; enode = enode->link, count++)

		for (fnode = ((CREATEENTITY*)enode->thing)->path->nodes; fnode; fnode = fnode->link)
			fnode->waitTime = value;

	sprintf(statusMessage, "Set wait time to %d for %d paths", value, count);
}

void SetAllOffsets(const char* str)
{
	EDITGROUPNODE *enode;
	EDITPATHNODE *fnode;
	int count; 
	float value = (float)atof(str);

	for (enode = selectionList->nodes, count=0; enode; enode = enode->link, count++)
		for (fnode = ((CREATEENTITY*)enode->thing)->path->nodes; fnode; fnode = fnode->link)
			fnode->offset = value;

	sprintf(statusMessage, "Set offset to %f for %d paths", value, count);
}

void SetAllOffset2s(const char* str)
{
	EDITGROUPNODE *enode;
	EDITPATHNODE *fnode;
	int count;
	float value = (float)atof(str);

	for (enode = selectionList->nodes, count=0; enode; enode = enode->link, count++)
		for (fnode = ((CREATEENTITY*)enode->thing)->path->nodes; fnode; fnode = fnode->link)
			fnode->offset2 = value;

	sprintf(statusMessage, "Set offset2 to %f for %d paths", value, count);
}

void SetAllBothOffsets(const char* str)
{
	EDITGROUPNODE *enode;
	EDITPATHNODE *fnode;
	int count; 
	float value = (float)atof(str);

	for (enode = selectionList->nodes, count=0; enode; enode = enode->link, count++)
		for (fnode = ((CREATEENTITY*)enode->thing)->path->nodes; fnode; fnode = fnode->link)
		{
			fnode->offset = value;
			fnode->offset2 = value;
		}

	sprintf(statusMessage, "Set both path offsets to %f for %d entities", value, count);
}

void DeleteFlags( )
{
	EDITGROUPNODE *node;
	long count;
	
	if( !editPath ) return;

	for( node = selectionList->nodes, count=0; node; node = node->link, count++ )
		EditorSubFlag( (EDITPATHNODE*)node->thing, editPath );

	sprintf(statusMessage, "Removed %d flags from path", count);
}

/*	--------------------------------------------------------------------------------
	Function		: EditorSave
	Parameters		: const char *
	Returns			: 
*/

void EditorSave(const char* path)
{
	char filename[80];
	int i, worldID, levelID;

	strcpy(savePath, path);

	i = strlen(path);
	if (savePath[i-1] != '\\') /* must have trailing \ */
	{
		savePath[i++] = '\\'; savePath[i] = 0;
	}
	
	worldID = worldVisualData[player[0].worldNum].worldID;
	levelID = worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].levelID;

	sprintf(filename, "%s" ENTITY_DUMP_FILE, savePath, worldID, levelID);
	if (SaveCreateList(filename, createList))
	{
		sprintf(statusMessage, "Saved editor file %s", filename);
	}
	else
		sprintf(statusMessage, "Error saving to %s", filename);
}

/*	--------------------------------------------------------------------------------
	Function		: EditorLoad
	Parameters		: const char *
	Returns			: 
*/

void EditorLoad(const char* path)
{
	char filename[80];
	int i, worldID, levelID;

	strcpy(savePath, path);

	i = strlen(path);
	if (savePath[i-1] != '\\') /* must have trailing \ */
	{
		savePath[i++] = '\\'; savePath[i] = 0;
	}

	worldID = worldVisualData[player[0].worldNum].worldID;
	levelID = worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].levelID;

	sprintf(filename, "%s" ENTITY_DUMP_FILE, savePath, worldID, levelID);

	if (LoadCreateList(filename))
	{
		sprintf(statusMessage, "Loaded editor file %s", filename);
	}
	else
		sprintf(statusMessage, "Error loading %s", filename);
}


/*	--------------------------------------------------------------------------------
	Function		: EditVariable
	Parameters		: 
	Returns			: 
*/

void EditVariable(char *name, EDITVAR t, void *var)
{
	editText.var = name;
	editText.type = t;
	editText.variable = var;
	editText.charLimit = 80;
	
	switch (t)
	{
	case EDITVAR_STRING:
		strncpy(editText.string, var, 127);
		editText.string[127] = 0;
		break;

	case EDITVAR_INT:
		_itoa(*(int *)var, editText.string, 10);
		break;

	case EDITVAR_FLOAT:
		sprintf(editText.string, "%.2f", *(float *)var);
		break;

	case EDITVAR_GROUPFUNCINT:
	case EDITVAR_GROUPFUNCFLOAT:
	case EDITVAR_GROUPFUNCSTR:
	case EDITVAR_PASSTOFUNCTION:
		editText.string[0] = 0;
		break;
	}

	editText.active = TRUE;

	UpdateStatusString();
}

/*	--------------------------------------------------------------------------------
	Function		: 
	Parameters		: 
	Returns			: 
*/

void FUNC_SetPathSpeed(const char *str)
{
	float speed;
	EDITPATHNODE *node;
	int counter;

	if (!editPath || !editPath->nodes) return;

	if (str[0] == '+')
	{
		speed = (float)atof(str + 1);
		for (node = editPath->nodes, counter = 0; node; node = node->link, counter++)
			node->speed += speed;
		sprintf(statusMessage, "Increased speed by %f for %i nodes", speed, counter);
	}
	else if (str[0] == '-')
	{
		speed = (float)atof(str + 1);
		for (node = editPath->nodes, counter = 0; node; node = node->link, counter++)
			node->speed -= speed;
		sprintf(statusMessage, "Decreased speed by %f for %i nodes", speed, counter);
	}
	else
	{
		speed = (float)atof(str);
		for (node = editPath->nodes, counter = 0; node; node = node->link, counter++)
			node->speed = speed;
		sprintf(statusMessage, "Set speed to %f for %i nodes", speed, counter);
	}
}


/*	--------------------------------------------------------------------------------
	Function		: AutomapBabies
	Info			: Create baby enemies from bTStart tiles
	Parameters		: 
	Returns			: 
*/
void AutomapBabies( )
{
/*
	ENEMY *nme;
	EDITPATHNODE *path;
	CREATEENTITY *e;
	int i, flags = 0;

	ResetBabies( );

	for( i=0; i<numBabies; i++ )
	{
		FreeEditPath( editPath );
		editPath = CreateEditPath();
		path = EditorAddFlag( bTStart[i], editPath );
		path->speed = 0.3;

		e = EditorAddCreateNode( "froglet.obe", ENEMY_NEW_BABYFROG, 0, CREATE_ENEMY, editPath, 0 );
		
		// Value1 used to be the respawn tile number, but is now the uid of a placeholder to respawn on
		e->value1 = 0;//((DWORD)path->tile - (DWORD)firstTile) / sizeof(GAMETILE);
	}
*/

	strcpy(statusMessage, "NOT CURRENTLY WORKING");
}
