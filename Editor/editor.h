#ifndef EDITOR_H_INCLUDED
#define EDITOR_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "actor2.h"

extern unsigned long mod;
#define FRED_SHIFT   (1<<0)
#define FRED_CONTROL (1<<1)
#define FRED_ALT     (1<<2)
#define FRED_CAPS    (1<<3)

void RunEditor(void);
void DrawEditor(void);
void InitEditor(void);
void ShutdownEditor(void);

void EditorKeypress(char c);

int CheckEditIcons(void);

#ifdef __cplusplus
}
#endif
#endif