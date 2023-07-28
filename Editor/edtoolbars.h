/*	-----------------------------------------------------------------
	Toolbar defs
	-----------------------------------------------------------------

	To add a new toolbar command:
	
	1.	Add a TB_xxx constant to the TOOLBARCOMMANDS enumeration below
	2.	Add an entry to either TOOLBARBUTTONS or one of the TOOLBAR_SUBMENU
		arrays. Make sure the array is terminated by a { -1, -1 } entry.
		Entries are { icon, command }. To enable a submenu icon, add TB_ON
		to the icon value. Seperators are { -1, 0 }.
	3.	Add an entry in the (huge!) switch statement in ToolbarSelect() in
		"editor.c".

*/

#ifndef TOOLBARS_H_INCLUDED
#define TOOLBARS_H_INCLUDED

#include "edittypes.h"

#define TBFLAG 0x80000000
#define TB_ON  0x8000

enum TOOLBARCOMMANDS
{
	TB_NONE = 0,
	TB_SAVE,
	TB_LOAD,
	TB_CLEARLISTS,
	TB_CLEAR_GARIBS,
	TB_CLEAR_ENEMIES,
	TB_CLEAR_PLATFORMS,
	TB_CLEAR_CREATEENEMY,
	TB_CLEAR_CREATEPLATFORM,
	TB_DELETE_SELECTED,
	TB_TEST,
	TB_PLACEFLAG,
	TB_CLEARPATH,
	TB_PICKFLAG,
	TB_UNDO,
	TB_PLACEGARIB,
	TB_PREV,
	TB_NEXT,
	TB_PLACEENEMY,
	TB_PLACEPLATFORM,
	TB_AUTOCAMERA,
	TB_SET_SPEED,
	TB_SET_OFFSET,
	TB_SET_OFFSET2,
	TB_SET_WAIT,
	TB_SET_ID,
	TB_SETPATHSPEED,
	TB_SET_ENTITYTYPE,
	TB_INFO,
	TB_DRAW_NORMALS,
	TB_DRAW_LINKS,
	TB_DRAW_VECTORS,
	TB_DRAW_DOTS,
	TB_RECTSELECT,
	TB_TILESELECT,
	TB_TILESTATE_NEXT,
	TB_TILESTATE_PREV,
	TB_EMPTYTILESELECTION,
	TB_DELETE_FLAG,
	TB_INVERTMOUSE,
	TB_INOUTWHIZZY,
	TB_HORIZWHIZZY,
	TB_999LIVES,
	TB_TESTING,
	TB_TEST_ENEMYRECT,
	TB_ASSIGNPATH,
	TB_SCALE,
	TB_RADIUS,
	TB_ANIMSPEED,
	TB_FLAG_PLAT_FORWARDS,
	TB_FLAG_PLAT_BACKWARDS,
	TB_FLAG_PLAT_CYCLE,
	TB_FLAG_PLAT_PINGPONG,
	TB_FLAG_PLAT_MOVEUP,
	TB_FLAG_PLAT_MOVEDOWN,
	TB_FLAG_PLAT_STEPACTIVATE,
	TB_FLAG_PLAT_CRUMBLE,
	TB_FLAG_PLAT_REGENERATE,
	TB_FLAG_PLAT_NOWALKUNDER,
	TB_FLAG_PLAT_KILLFROG,
	TB_FLAG_NME_RADIUSCOLLISION,
	TB_FLAG_NME_WATCHFROG,
	TB_FLAG_NME_SNAPFROG,
	TB_FLAG_NME_RANDOMSPEED,
	TB_FLAG_NME_FACEFORWARDS,
	TB_FLAG_NME_PUSHESFROG,
	TB_RESET_ENEMYFLAGS,
	TB_RESET_PLATFORMFLAGS,
	TB_MAKECAMERA_FOLLOWFROG,
	TB_MAKECAMERA_WATCHFROG,
	TB_MAKECAMERA_STATIC,
	TB_MAKECAMERA_FOLLOWFROG_NOTILT,
	TB_COPY_SELECTION,
	TB_PLACEFROG,
	TB_SET_BOTH_OFFSETS,
	TB_SET_ALL_PATHSPEEDS,
	TB_SET_ALL_OFFSETS,
	TB_SET_ALL_OFFSET2S,
	TB_SET_ALL_BOTHOFFSETS,
	TB_SET_ALL_WAITTIMES,
	TB_PLACEPLACEHOLDER,
	TB_SET_VALUE1,
	TB_PASTE_SELECTION,
	TB_CLEAR_SELECTION,
	TB_HELPSCREEN,
	TB_FLAG_NME_HOMING,
	TB_FLAG_NME_ROTATEPATH_XZ,
	TB_FLAG_NME_ROTATEPATH_XY,
	TB_FLAG_NME_ROTATEPATH_ZY,
	TB_TEST_SCRIPTING,
	TB_INTERP_SPEEDS,
	TB_CAMERACASE_ADDTILES,
	TB_CAMERACASE_SETDIRECTION,
	TB_CAMERACASE_SETFOV,
	TB_KILL_EVERYTHING,
	TB_FLAG_PLAT_FACEFORWARDS,
	TB_FLAG_NME_SNAPTILES,
	TB_FLAG_NME_MOVEONMOVE,
	TB_CAMERACASE_SETSPEED,
	TB_FLAG_MAKERIPPLES,
	TB_FLAG_SMOKE_STATIC,
	TB_FLAG_SPARKBURST,
	TB_FLAG_NME_VENT,
	TB_FLAG_GREENGLOOP,
	TB_ATTACHEFFECT,
	TB_FLAG_NME_NODAMAGE,
	TB_FLAG_NME_FLAPPYTHING,
	TB_FLAG_NME_ONEHITKILL,
	TB_FLAG_NME_SLERPPATH,
	TB_FLAG_BATSWARM,
	TB_FLAG_BUBBLES,
	TB_FLAG_SMOKE_GROWS,
	TB_FLAG_FASTEFFECT,
	TB_FLAG_MEDIUMEFFECT,
	TB_FLAG_SLOWEFFECT,
	TB_FLAG_SMOKEBURST,
	TB_FLAG_TINTRED,
	TB_FLAG_TINTGREEN,
	TB_FLAG_TINTBLUE,
	TB_FLAG_FIERYSMOKE,
	TB_FLAG_BUTTERFLYSWARM,
	TB_FLAG_LASER,
	TB_FLAG_RANDOMCREATE,
	TB_FLAG_TRAIL,
	TB_FLAG_NME_BABYFROG,
	TB_FLAG_NME_RANDOMMOVE,
	TB_AUTOMAP_BABIES,
	TB_FLAG_NME_SHADOW,
	TB_FLAG_PLAT_SHADOW,
	TB_FLAG_NME_TILEHOMING,
	TB_FLAG_PLAT_SLERPPATH,
	TB_MAKECAMERA_WATCHTILE,
	TB_FLAG_BILLBOARDTRAIL,
	TB_FLAG_LIGHTNING,
	TB_MAKECAMERA_LOOKDIR,
	TB_FLAG_SPACETHING1,
	TB_FLAG_SPARKLYTRAIL,
	TB_FLAG_GLOW,
	TB_FLAG_TWINKLE,
	TB_SET_FLAGSOUND,
	TB_SET_ORIENTATION,
	TB_SET_PSXSHIFT,
	TB_SET_PSXHACK,
	TB_SAVE_EXTRASPECIAL,
	TB_FLAG_NME_BATTLEMODE,

	TBNUMCOMMANDS
};

// Dynamically changing icon - don't forget to increment this if you add a button before it!
#define CAMERA_BUTTON 23

#ifdef __cplusplus
extern "C" {
#endif

extern TOOLBAR_ENTRY TOOLBARBUTTONS[], TOOLBAR_MULTI[], TOOLBAR_ENEMY[], TOOLBAR_PLATFORM[],
	TOOLBAR_GARIB[], TOOLBAR_INFO[], TOOLBAR_FLAG[], TOOLBAR_TILES[], TOOLBAR_TESTING[],
	TOOLBAR_CLEAR[], TOOLBAR_PLACEHOLDER[], TOOLBAR_CAMERA[], TOOLBAR_EFFECTS[];

extern char *tooltips[TBNUMCOMMANDS];

#ifdef __cplusplus
}
#endif
#endif
