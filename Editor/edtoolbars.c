/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: toolbars.c
	Programmer	: David Swift
	Date		: 04/08/99

----------------------------------------------------------------------------------------------- */

#include <ultra64.h>

#include "edtoolbars.h"
#include "platform.h"
#include "enemies.h"

TOOLBAR_ENTRY TOOLBARBUTTONS[] =
{
	{ 8,		TB_SAVE },
	{ 9,		TB_LOAD },
	{ 10,		TB_TEST },
	{ 21,		TB_CLEARLISTS },
	{ 15,		TB_UNDO },
	{ -1, 0 }, // SEPERATOR
	{ 61,		TB_RECTSELECT },
	{ 46,		TB_TILESELECT },
	{ 77,		TB_COPY_SELECTION },
	{ 81,		TB_PASTE_SELECTION },
	{ 82,		TB_CLEAR_SELECTION },
	{ -1, 0 },
	{ 11,		TB_PLACEFLAG },
	{ 13,		TB_CLEARPATH },
	{ 19,		TB_PLACEENEMY },
	{ 20,		TB_PLACEPLATFORM },
	{ 79,		TB_PLACEPLACEHOLDER },
	{ 106,		TB_ATTACHEFFECT },
	{ -1, 0 },
	{ 16,		TB_PLACEGARIB },
	{ 17,		TB_PREV },
	{ 18,		TB_NEXT },
	{ -1, 0 },
	{ 22,		TB_AUTOCAMERA },
	{ -1, 0 },
	{ 32,		TB_SET_ENTITYTYPE },
	{ 21,		TB_DELETE_SELECTED },
	{ 39,		TB_SET_ID },
	{ 29,		TB_PICKFLAG },
	{ -1, 0 },
	{ 125,		TB_AUTOMAP_BABIES },
	{ -1, 0 },
	{ 56,		TB_INFO },
	{ 83,		TB_HELPSCREEN },
	{ 47,		TB_TESTING },
	{ -1, 0 },
	{ 95,		TB_KILL_EVERYTHING },
	{ -1, -1 } // end of list
};

TOOLBAR_ENTRY TOOLBAR_MULTI[] =
{
	{ 32 + TB_ON,	TB_SET_ENTITYTYPE },
	{ 21 + TB_ON,	TB_DELETE_SELECTED },
	{ 39 + TB_ON,	TB_SET_ID },
	{ -1, -1 } // end of list
};

TOOLBAR_ENTRY TOOLBAR_ENEMY[] =
{
	{ 33 + TB_ON,	TB_FLAG_PLAT_FORWARDS },
	{ 34,			TB_FLAG_PLAT_BACKWARDS },
	{ 35,			TB_FLAG_PLAT_CYCLE },
	{ 36,			TB_FLAG_PLAT_PINGPONG },
	{ 37,			TB_FLAG_PLAT_MOVEUP },
	{ 38,			TB_FLAG_PLAT_MOVEDOWN },
	{ 110,			TB_FLAG_NME_SLERPPATH },
	{ 71,			TB_FLAG_NME_RADIUSCOLLISION },
	{ 48,			TB_FLAG_NME_WATCHFROG },
	{ 49,			TB_FLAG_NME_SNAPFROG },
	{ 98,			TB_FLAG_NME_SNAPTILES },
	{ 69 /*dude!*/,	TB_FLAG_NME_FACEFORWARDS },
	{ 70,			TB_FLAG_NME_PUSHESFROG },
	{ 84,			TB_FLAG_NME_HOMING },
	{ 99,			TB_FLAG_NME_MOVEONMOVE },
	{ 85,			TB_FLAG_NME_ROTATEPATH_XZ },
	{ 87,			TB_FLAG_NME_ROTATEPATH_XY },
	{ 88,			TB_FLAG_NME_ROTATEPATH_ZY },
	{ 103,			TB_FLAG_NME_VENT },
	{ 107,			TB_FLAG_NME_NODAMAGE },
	{ 108,			TB_FLAG_NME_FLAPPYTHING },
	{ 123,			TB_FLAG_NME_BABYFROG },
	{ 126,			TB_FLAG_NME_SHADOW },
	{ 127,			TB_FLAG_NME_TILEHOMING },
	{ 142,			TB_FLAG_NME_BATTLEMODE },
	{ -1, 0 },
	{ 139 + TB_ON,	TB_SET_ORIENTATION },
	{ 67 + TB_ON,	TB_ASSIGNPATH },
	{ 68 + TB_ON,	TB_SCALE },
	{ 71 + TB_ON,	TB_RADIUS },
	{ 72 + TB_ON,	TB_ANIMSPEED },
	{ 26 + TB_ON,	TB_SET_ALL_OFFSETS },
	{ 27 + TB_ON,	TB_SET_ALL_OFFSET2S },
	{ 78 + TB_ON,	TB_SET_ALL_BOTHOFFSETS },
	{ 28 + TB_ON,	TB_SET_ALL_PATHSPEEDS },
	{ 24 + TB_ON,	TB_SET_ALL_WAITTIMES },
	{ 80 + TB_ON,	TB_SET_VALUE1 },
	{ 73,			TB_RESET_ENEMYFLAGS },
	{ -1, -1 } // end of list
};

TOOLBAR_ENTRY TOOLBAR_PLATFORM[] =
{
	{ 33 + TB_ON,	TB_FLAG_PLAT_FORWARDS },
	{ 34,			TB_FLAG_PLAT_BACKWARDS },
	{ 35,			TB_FLAG_PLAT_CYCLE },
	{ 36,			TB_FLAG_PLAT_PINGPONG },
	{ 37,			TB_FLAG_PLAT_MOVEUP },
	{ 38,			TB_FLAG_PLAT_MOVEDOWN },
	{ 40,			TB_FLAG_PLAT_STEPACTIVATE },
	{ 41,			TB_FLAG_PLAT_CRUMBLE },
	{ 42,			TB_FLAG_PLAT_REGENERATE },
	{ 43,			TB_FLAG_PLAT_NOWALKUNDER },
	{ 44,			TB_FLAG_PLAT_KILLFROG },
	{ 69,			TB_FLAG_PLAT_FACEFORWARDS },
	{ 126,			TB_FLAG_PLAT_SHADOW },
	{ 110,			TB_FLAG_PLAT_SLERPPATH },
	{ -1, 0 },
	{ 139 + TB_ON,	TB_SET_ORIENTATION },
	{ 67 + TB_ON,	TB_ASSIGNPATH },
	{ 68 + TB_ON,	TB_SCALE },
	{ 71 + TB_ON,	TB_RADIUS },
	{ 72 + TB_ON,	TB_ANIMSPEED },
	{ 26 + TB_ON,	TB_SET_ALL_OFFSETS },
	{ 27 + TB_ON,	TB_SET_ALL_OFFSET2S },
	{ 78 + TB_ON,	TB_SET_ALL_BOTHOFFSETS },
	{ 28 + TB_ON,	TB_SET_ALL_PATHSPEEDS },
	{ 24 + TB_ON,	TB_SET_ALL_WAITTIMES },
	{ 80 + TB_ON,	TB_SET_VALUE1 },
	{ -1, 0 },
	{ 73,			TB_RESET_PLATFORMFLAGS },
	{ -1, -1 }	// end of list
};

TOOLBAR_ENTRY TOOLBAR_EFFECTS[] = 
{
	{ 94,			TB_FLAG_MAKERIPPLES },
	{ 96,			TB_FLAG_SMOKE_STATIC },
	{ 111,			TB_FLAG_SMOKE_GROWS },
	{ 97,			TB_FLAG_SPARKBURST },
	{ 100,			TB_FLAG_BUBBLES },
	{ 118,			TB_FLAG_FIERYSMOKE },
	{ 104,			TB_FLAG_GREENGLOOP },
	{ 105,			TB_FLAG_SMOKEBURST },
	{ 86,			TB_FLAG_BATSWARM },
	{ 119,			TB_FLAG_BUTTERFLYSWARM },
	{ 120,			TB_FLAG_LASER },
	{ 122,			TB_FLAG_TRAIL },
	{ 131,			TB_FLAG_BILLBOARDTRAIL },
	{ 132,			TB_FLAG_LIGHTNING },
	{ 134,			TB_FLAG_SPACETHING1 },
	{ 135,			TB_FLAG_SPARKLYTRAIL },
	{ 136,			TB_FLAG_GLOW },
	{ 137,			TB_FLAG_TWINKLE },
	{ -1, 0 },
	{ 121,			TB_FLAG_RANDOMCREATE },
	{ -1, 0 },
	{ 112,			TB_FLAG_FASTEFFECT },
	{ 113,			TB_FLAG_MEDIUMEFFECT },
	{ 114,			TB_FLAG_SLOWEFFECT },
	{ -1, 0 },
	{ 115,			TB_FLAG_TINTRED },
	{ 116,			TB_FLAG_TINTGREEN },
	{ 117,			TB_FLAG_TINTBLUE },
	{ -1, 0 },
	{ 140 + TB_ON,	TB_SET_PSXSHIFT },
	{ 141 + TB_ON,	TB_SET_PSXHACK },
	{ -1, -1 }
};

TOOLBAR_ENTRY TOOLBAR_PLACEHOLDER[] =
{
	{ 21 + TB_ON,	TB_DELETE_SELECTED },
	{ 39 + TB_ON,	TB_SET_ID },
	{ 29 + TB_ON,	TB_PICKFLAG },
	{ -1, 0 },
	{ 67 + TB_ON,	TB_ASSIGNPATH },
	{ 26 + TB_ON,	TB_SET_ALL_OFFSETS },
	{ 27 + TB_ON,	TB_SET_ALL_OFFSET2S },
	{ 78 + TB_ON,	TB_SET_ALL_BOTHOFFSETS },
	{ 28 + TB_ON,	TB_SET_ALL_PATHSPEEDS },
	{ 24 + TB_ON,	TB_SET_ALL_WAITTIMES },
	{ 80 + TB_ON,	TB_SET_VALUE1 },
	{ -1, -1 }	
};

TOOLBAR_ENTRY TOOLBAR_CLEAR[] =
{
	{ 16 + 0x8000, TB_CLEAR_GARIBS  },
	{ 19 + 0x8000, TB_CLEAR_ENEMIES },
	{ 20 + 0x8000, TB_CLEAR_PLATFORMS },
	{ 5 + 0x8000, TB_CLEAR_CREATEENEMY },
	{ 6 + 0x8000, TB_CLEAR_CREATEPLATFORM },
	{ -1, -1 } // end of list
};

TOOLBAR_ENTRY TOOLBAR_GARIB[] =
{
	{ 21 + TB_ON,	TB_DELETE_SELECTED  },
	{ -1, 0 },
	{ 64 + TB_ON, 0 },
	{ 65 + TB_ON, 0 },
	{ 66 + TB_ON, 0 },
	{ -1, 0 },
	{ 26 + TB_ON,	TB_SET_ALL_OFFSETS },
	{ 27 + TB_ON,	TB_SET_ALL_OFFSET2S },
	{ 78 + TB_ON,	TB_SET_ALL_BOTHOFFSETS },
	{ 80 + TB_ON,	TB_SET_VALUE1 },
	{ -1, -1 } // end of list
};

TOOLBAR_ENTRY TOOLBAR_INFO[] =
{
	{ 57,	TB_DRAW_LINKS },
	{ 58,	TB_DRAW_NORMALS },
	{ 59,	TB_DRAW_VECTORS },
	{ 60,	TB_DRAW_DOTS },
	{ -1, 0 },
	{ 54,	TB_INVERTMOUSE },
	{ 52,	TB_INOUTWHIZZY },
	{ 53,	TB_HORIZWHIZZY },
	{ -1, 0 },
	{ 51,	TB_999LIVES },
	{ 22,	TB_PLACEFROG },
	{ -1, 0 },
	{ 90,	TB_TEST_SCRIPTING },
	{ -1, -1 } // end of list
};

TOOLBAR_ENTRY TOOLBAR_FLAG[] =
{
	{ 21 + TB_ON,	TB_DELETE_FLAG },
	{ -1, 0 },
	{ 29 + TB_ON,	TB_PICKFLAG },
	{ -1, 0 },
	{ 25 + TB_ON,	TB_SET_SPEED },
	{ 26 + TB_ON,	TB_SET_OFFSET },
	{ 27 + TB_ON,	TB_SET_OFFSET2 },
	{ 78 + TB_ON,	TB_SET_BOTH_OFFSETS },
	{ 24 + TB_ON,	TB_SET_WAIT },
	{ -1, 0 },
	{ 138 + TB_ON,	TB_SET_FLAGSOUND },
	{ -1, 0 },
	{ 28 + TB_ON,	TB_SETPATHSPEED },
	{ 91,			TB_INTERP_SPEEDS },
	{ -1, -1 } // end of list
};

TOOLBAR_ENTRY TOOLBAR_TILES[] =
{
	{ 21 + TB_ON,	TB_EMPTYTILESELECTION },
	{ 17 + TB_ON,	TB_TILESTATE_NEXT },
	{ 18 + TB_ON,	TB_TILESTATE_PREV },
	{ -1, 0 },
	{ 74 + TB_ON,	TB_MAKECAMERA_FOLLOWFROG },
	{ 128 + TB_ON,	TB_MAKECAMERA_FOLLOWFROG_NOTILT },
	{ 75 + TB_ON,	TB_MAKECAMERA_WATCHFROG },
	{ 76 + TB_ON,	TB_MAKECAMERA_STATIC },
	{ 130 + TB_ON,	TB_MAKECAMERA_WATCHTILE },
	{ 133 + TB_ON,	TB_MAKECAMERA_LOOKDIR },
	{ -1, -1 } // end of list
};

TOOLBAR_ENTRY TOOLBAR_CAMERA[] = 
{
	{ 92 + TB_ON,	TB_CAMERACASE_ADDTILES },
	{ 93 + TB_ON,	TB_CAMERACASE_SETDIRECTION },
	{ 101 + TB_ON,	TB_CAMERACASE_SETFOV },
	{ 102 + TB_ON,	TB_CAMERACASE_SETSPEED },

	{ -1, -1 }
};

TOOLBAR_ENTRY TOOLBAR_TESTING[] =
{
	{ 64 + TB_ON, 0 },
	{ 65 + TB_ON, 0 },
	{ 66 + TB_ON, 0 },
	{ -1, -1 } // end of list
};


/*
*	Must be in order of enumeration
*/
char *tooltips[TBNUMCOMMANDS] = 
{
	"",						//TB_NONE,
	"Save DAT File",		//TB_SAVE,
	"Load DAT File",		//TB_LOAD,
	"Clear Lists",			//TB_CLEARLISTS,
	"Clear Garibs",			//TB_CLEAR_GARIBS,
	"Clear Enemies",		//TB_CLEAR_ENEMIES,
	"Clear Platforms",		//TB_CLEAR_PLATFORMS,
	"Clear Create Enemy",	//TB_CLEAR_CREATEENEMY,
	"Clear Create Platform",//TB_CLEAR_CREATEPLATFORM,
	"Delete Selected",		//TB_DELETE_SELECTED,
	"Test Level",			//TB_TEST,
	"Place Flag",			//TB_PLACEFLAG,
	"Clear Path",			//TB_CLEARPATH,
	"Pick Flag",			//TB_PICKFLAG,
	"Undo Last Command",	//TB_UNDO,
	"Place Garib",			//TB_PLACEGARIB,
	"Previous",				//TB_PREV,
	"Next",					//TB_NEXT,
	"Place Enemy",			//TB_PLACEENEMY,
	"Place Platform",		//TB_PLACEPLATFORM,
	"Auto Camera",			//TB_AUTOCAMERA,
	"Set Speed",			//TB_SET_SPEED,
	"Set Offset",			//TB_SET_OFFSET,
	"Set Offset2",			//TB_SET_OFFSET2,
	"Set Wait",				//TB_SET_WAIT,
	"Set ID",				//TB_SET_ID,
	"Set Pathspeed",		//TB_SETPATHSPEED,
	"Set Entity Type",		//TB_SET_ENTITYTYPE,
	"Information",			//TB_INFO,
	"Draw Normals",			//TB_DRAW_NORMALS,
	"Draw Links",			//TB_DRAW_LINKS,
	"Draw Vectors",			//TB_DRAW_VECTORS,
	"Draw Dots",			//TB_DRAW_DOTS,
	"Select by Area",		//TB_RECTSELECT,
	"Select by Tile",		//TB_TILESELECT,
	"Next Tilestate",		//TB_TILESTATE_NEXT,
	"Previous Tilestate",	//TB_TILESTATE_PREV,
	"Clear Tile Selection",	//TB_EMPTYTILESELECTION,
	"Delete Flag",			//TB_DELETE_FLAG,
	"Invert Mouse Y-axis",	//TB_INVERTMOUSE,
	"In-Out Whizzy",		//TB_INOUTWHIZZY,
	"Horizontal Whizzy",	//TB_HORIZWHIZZY,
	"Give 999 Lives",		//TB_999LIVES,
	"Testing",				//TB_TESTING,
	"Test Enemy Rectangle",	//TB_TEST_ENEMYRECT,
	"Assign same path to selection", //TB_ASSIGNPATH,
	"Scale Selected",		//TB_SCALE,
	"Enemy Radius",			//TB_RADIUS,
	"Animation Speed",		//TB_ANIMSPEED
	"Path Forwards",		//TB_FLAG_PLAT_FORWARDS,
	"Path Backwards",		//TB_FLAG_PLAT_BACKWARDS,
	"Path Cycle",			//TB_FLAG_PLAT_CYCLE,
	"Path Pingpong",		//TB_FLAG_PLAT_PINGPONG,
	"Move Up",				//TB_FLAG_PLAT_MOVEUP,
	"Move Down",			//TB_FLAG_PLAT_MOVEDOWN,
	"Step on Activate",		//TB_FLAG_PLAT_STEPACTIVATE,
	"Crumbles",				//TB_FLAG_PLAT_CRUMBLE,
	"Regenerates",			//TB_FLAG_PLAT_REGENERATE,
	"No Walk Under",		//TB_FLAG_PLAT_NOWALKUNDER,
	"Platform Kills Frog",	//TB_FLAG_PLAT_KILLFROG,
	"Radius Collision",		//TB_FLAG_NME_RADIUSCOLLISION,
	"Watch Frog",			//TB_FLAG_NME_WATCHFROG,
	"Snap at Frog",			//TB_FLAG_NME_SNAPFROG,
	"Random Speed",			//TB_FLAG_NME_RANDOMSPEED,
	"Face Forwards",		//TB_FLAG_NME_FACEFORWARDS,
	"Pushes Frog",			//TB_FLAG_NME_PUSHESFROG
	"Reset Enemy Flags",	//TB_RESET_ENEMYFLAGS
	"Reset Platform Flags",	//TB_RESET_PLATFORMFLAGS
	"Create moving camera",		//TB_MAKECAMERA_FOLLOWFROG
	"Create fixed camera, watches frog",	//TB_MAKECAMERA_WATCHFROG
	"Create static camera",		//TB_MAKECAMERA_STATIC
	"Create moving camera, no tilting",
	"Copy Selected",		//TB_COPY_SELECTION
	"Place frog",			//TB_PLACEFROG
	"Set Both Offsets",		//TB_SET_BOTH_OFFSETS
	"Set Selection Pathspeeds",	//TB_SET_ALL_PATHSPEEDS,
	"Set Selection Offsets",	//TB_SET_ALL_OFFSETS,
	"Set Selection Offset2s",	//TB_SET_ALL_OFFSET2S
	"Set Selection Both Offsets",	//TB_SET_ALL_BOTHOFFSETS
	"Set Selection Wait Times",	// TB_SET_ALL_WAITTIMES
	"Place Placeholder",	// TB_PLACEPLACEHOLDER
	"Set Misc Value 1",		// TB_SET_VALUE1
	"Paste Selected",		// TB_PASTE_SELECTION
	"Clear Selection",		// TB_CLEAR_SELECTION
	"Help Screen",			// TB_HELPSCREEN
	"Homing",				// TB_FLAG_NME_HOMING
	"Circle path in XZ",	// TB_FLAG_NME_ROTATEPATH_XZ
	"Circle path in XY",	// TB_FLAG_NME_ROTATEPATH_XY
	"Circle path in ZY",	// TB_FLAG_NME_ROTATEPATH_ZY
	"Test scripting",		// TB_TEST_SCRIPTING
	"Interpolate Path Speeds",	// TB_INTERP_SPEEDS
	"Add Tiles to Camera Case",	//TB_CAMERACASE_ADDTILES
	"Set direction for camera",	// TB_CAMERACASE_SETDIRECTION
	"Set field of view",		// TB_CAMERACASE_ADDTILES
	"Purge",				// TB_KILL_EVERYTHING
	"Face Forwards",		//TB_FLAG_PLAT_FACEFORWARDS
	"Snap at Tiles",		// TB_FLAG_NME_SNAPTILES
	"Move on Move",			// TB_FLAG_NME_MOVEONMOVE
	"Set Camera Speed",		// TB_CAMERACASE_SETSPEED
	"Entity makes Ripples",	//TB_FLAG_MAKERIPPLES,
	"Entity makes Smoke",	//TB_FLAG_MAKESMOKE,
	"Entity makes Sparks",	//TB_FLAG_MAKESPARKS,
	"Vent NME",				// TB_FLAG_NME_VENTUP
	"Green Goop",			//TB_FLAG_GREENGLOOP,
	"Special FX Selection",	//TB_ATTACHEFFECT
	"No Damage (This is a Dove)", //TB_FLAG_NME_NODAMAGE,
	"Flappy Thing",			//TB_FLAG_NME_FLAPPYTHING,
	"One Hit Kill",			// TB_FLAG_NME_ONEHITKILL
	"Slerp Path",			// TB_FLAG_NME_SLERPPATH
	"Swarm of Bats",		// TB_FLAG_FLYSWARM
	"Entity makes Bubbles",	// TB_FLAG_BUBBLES
	"Smoke that grows",		// TB_FLAG_SMOKE_GROWS,
	"Fast effect",			// TB_FLAG_FASTEFFECT,
	"Medium effect",		// TB_FLAG_MEDIUMEFFECT,
	"Slow effect",			// TB_FLAG_SLOWEFFECT,
	"Smoke Bursts",			// TB_FLAG_SMOKEBURST
	"Tint Red",				// TB_FLAG_TINTRED
	"Tint Green",
	"Tint Blue",
	"Fiery Smoke",			// 	TB_FLAG_FIERYSMOKE,
	"Butterflies",			//	TB_FLAG_BUTTERFLYSWARM,
	"Laser",				//TB_FLAG_LASER },
	"Slightly Random Creation",	//TB_FLAG_RANDOMCREATE },
	"Motion Trails",		//TB_FLAG_TRAIL
	"Baby Frog",			//TB_FLAG_NME_BABYFROG
	"Random Movement",		// TB_FLAG_NME_RANDOMMOVE
	"Automap Babies",		// TB_AUTOMAP_BABIES
	"Enemy has a Shadow",	// TB_FLAG_NME_SHADOW
	"Platform has a Shadow",// TB_FLAG_PLAT_SHADOW
	"Homing using Tiles",//TB_FLAG_NME_TILEHOMING
	"Slerpy path",			// TB_FLAG_PLAT_SLERPPATH
	"Camera watches last Tile", //TB_MAKECAMERA_WATCHTILE
	"Billboarded Trail",	//TB_FLAG_BILLBOARDTRAIL
	"Lightning",			// TB_FLAG_LIGHTNING
	"Camera looks Direction",	//TB_MAKECAMERA_LOOKDIR
	"Swirly space beastie",	// TB_FLAG_SPACETHING1
	"Sparkly trail",		// TB_FLAG_SPARKLYTRAIL
	"Glow",					// TB_FLAG_GLOW
	"Twinkle",				// TB_FLAG_TWINKLE
	"Sound for flag",		// TB_SET_FLAGSOUND
	"Faces Direction",		// TB_SET_ORIENTATION
	"PSX Sorting hack",		// TB_SET_PSXSHIFT
	"PSX extra hack",		// TB_SET_PSXSHIFT
	"Save extraspecial map",// TB_SAVE_EXTRASPECIAL
	"Battlemode Enemy",		// TB_FLAG_NME_BATTLEMODE
};

