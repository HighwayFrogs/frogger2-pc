#ifndef ISL_MEM_H
#define ISL_MEM_H

#include <memory.h>
#include <windows.h>
#include <mdxException.h>

#define MALLOC(x) AllocMem(x)//malloc(x)
#define FREE(x) FreeMem(x)/*free(x)*/,(x)=NULL
#define memoryAllocateZero(size, file, line) calloc((size), 1)
#define memoryShow()
#define memoryShowStats()


#endif