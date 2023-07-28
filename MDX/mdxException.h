#ifndef MDXEXCEPTION_H_INCLUDED
#define MDXEXCEPTION_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

int Eval_Exception (int n_except,LPEXCEPTION_POINTERS exceptinf);

#define EXCEPTION_HANDLE(x) {__try {x} __except (Eval_Exception(GetExceptionCode(),GetExceptionInformation())){}};

void *Alloc_Mem(unsigned long size,char *file, long line);
void Free_Mem(void *me,char *file, long line);
void Show_Mem(void);


#define AllocMem(x) Alloc_Mem(x,__FILE__,__LINE__)
#define FreeMem(x) Free_Mem(x,__FILE__,__LINE__)

#ifdef __cplusplus
}
#endif

#endif