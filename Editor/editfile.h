#ifndef EDITFILE_H_INCLUDED
#define EDITFILE_H_INCLUDED

#include "edittypes.h"

#ifdef __cplusplus
extern "C" {
#endif
BOOL SaveGaribList(const char *filename);
BOOL LoadGaribList(const char *filename);
BOOL SaveCreateList(const char *filename, EDITGROUP *list);
BOOL LoadCreateList(const char *filename);

extern int releaseQuality;

#ifdef __cplusplus
}
#endif
#endif