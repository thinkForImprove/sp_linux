#ifndef DUMP_ALLOC_MEM_H
#define DUMP_ALLOC_MEM_H

#if defined(DEBUG) || defined(_DEBUG)

void *_cdecl operator new (size_t nSize, const char *lpszFileName, int nLine);
void __cdecl operator delete (void *p, const char *lpszFileName, int nLine);
void __cdecl operator delete (void *p);

#define DEBUG_NEW new(THIS_FILE, __LINE__)



#endif  //defined(DEBUG) || defined(_DEBUG)

#endif  //DUMP_ALLOC_MEM_H