#ifndef _EDCOMMAND_INCLUDD_H
#define _EDCOMMAND_INCLUDED_H

#include "edglobals.h"

#ifdef __cplusplus
extern "C" {
#endif

// Editor commands
extern void EditorLoad(const char* path);
extern void EditorSave(const char* path);
extern void EditorUndo();

// Toolbar
extern void ToolbarSelect(int toolOpt);

// Level editing
extern void PlaceGarib(GAMETILE *tile, int garibType);
extern void PlaceEnemy();
extern void PlacePlatform();
extern void PlacePlaceholder( );
extern void RemoveEntities(int type);
extern void FreeEnemyList(void);
extern void FreePlatformList(void);

// Selections
extern void UpdateSelection(void);
extern void SelectMode(SELECTFUNC func);
extern void SetSelectionName(const char* str);
extern void AssignSamePath(EDITGROUP *group);
extern void ScaleEntitySelection(EDITGROUP *group);
extern void DeleteEntitySelection(void);
extern void CopyEntitySelection(EDITGROUP *group);
extern void AddTilesToCameraCase(int ax, int ay, int bx, int by);

// Edit variable
extern void EditVariable(char *name, EDITVAR t, void *var);
extern void FUNC_SetPathSpeed(const char *str);

#define ENABLEICON(entry, flag) (entry).icon = (flag) ? (entry).icon | TB_ON : (entry).icon & ~TB_ON


void Select_Create(int ax, int ay, int bx, int by);
void Select_Tiles(int ax, int ay, int bx, int by);
void Select_Flags(int ax, int ay, int bx, int by);

#ifdef __cplusplus
}
#endif
#endif
