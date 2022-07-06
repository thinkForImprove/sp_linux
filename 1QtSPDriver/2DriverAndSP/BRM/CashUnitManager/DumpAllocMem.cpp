//DumpAllocMem.cpp
#include "StdAfx.h"
#if defined(DEBUG) || defined(_DEBUG)

#include <crtdbg.h>

#pragma warning(disable: 4291)

void *_cdecl operator new (size_t nSize, const char *lpszFileName, int nLine)
{
    return _malloc_dbg(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
}

void __cdecl operator delete (void *p)
{
    _free_dbg(p, _NORMAL_BLOCK);
}

void __cdecl operator delete (void *pData, LPCSTR /* lpszFileName */, int /* nLine */)
{
    ::operator delete (pData);
}

void _cdecl ExitDumpProc()
{
    HANDLE hFile = CreateFile("ReportError.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE, NULL);
    SetFilePointer(hFile, 0, NULL, FILE_END);
    _CrtSetReportFile(_CRT_WARN, hFile);
    _CrtSetReportFile(_CRT_ERROR, hFile);
    _CrtSetReportFile(_CRT_ASSERT, hFile);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
    _CrtMemDumpAllObjectsSince(NULL);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    CloseHandle(hFile);
}

struct DUMP_STRUCT
{
    DUMP_STRUCT()
    {
        atexit(ExitDumpProc);
    }
};
DUMP_STRUCT __DumpStruct;

#endif //defined(DEBUG) || defined(_DEBUG)