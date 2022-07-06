#pragma once

//////////////////////////////////////////////////////////////////////////
#include "string.h"
//////////////////////////////////////////////////////////////////////////
//WIN和LINUX不一样的定义，32位和64位不一样字节大小的
#ifdef QT_WIN32
typedef unsigned long                   DWORD;
#define SPETCPATH  "C:/CFES/ETC"
#define LOGPATH    "D:/LOG"
#else
typedef unsigned int                    DWORD;
#define SPETCPATH  "/usr/local/CFES/ETC"
#define LOGPATH    "/usr/local/LOG"
#endif

//////////////////////////////////////////////////////////////////////////
// LFS定义
typedef void               *HANDLE;
typedef void               *LPVOID;
typedef char               *LPSTR;

typedef DWORD              *LPDWORD;
typedef unsigned short      WORD;
typedef WORD               *LPWORD;
typedef unsigned char       BYTE;
typedef BYTE               *LPBYTE;
typedef long                LONG;
typedef int                 BOOL;
typedef BOOL               *LPBOOL;

typedef unsigned short      USHORT;
typedef char                CHAR;
typedef short               SHORT;
typedef unsigned long       ULONG;
typedef unsigned char       UCHAR;
typedef SHORT              *LPSHORT;
typedef LPVOID             *LPLPVOID;
typedef ULONG              *LPULONG;
typedef USHORT             *LPUSHORT;

typedef HANDLE              HPROVIDER;
typedef ULONG               REQUESTID;
typedef REQUESTID          *LPREQUESTID;

typedef HANDLE              HAPP;
typedef HAPP               *LPHAPP;

typedef USHORT              HSERVICE;
typedef HSERVICE           *LPHSERVICE;

typedef LONG                HRESULT;
typedef HRESULT            *LPHRESULT;

#ifdef QT_LINUX
typedef void               *HWND;
typedef HWND               *LPHWND;
#endif

// 其他定义
typedef char               *PSZ;
typedef float               FLOAT;
typedef FLOAT              *PFLOAT;
typedef int                *LPINT;
typedef long               *LPLONG;
typedef void               *PVOID;
typedef int                 INT;
typedef unsigned int        UINT;
typedef UINT               *PUINT;
typedef const char         *LPCSTR;
typedef wchar_t             WCHAR;
typedef WCHAR              *LPWSTR;

/* Types use for passing & returning polymorphic values */
typedef UINT                WPARAM;
typedef LONG                LPARAM;
typedef LONG                LRESULT;

//////////////////////////////////////////////////////////////////////////
#ifndef _INC_WINDOWS
#ifndef _SYSTEMTIME_
typedef struct _SYSTEMTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;
#endif
//////////////////////////////////////////////////////////////////////////
// 通用宏定义
#define SAFEDELPOINT(p)         {if(nullptr != (p)) { delete[] (p); (p) = nullptr;}}
#define TRUE                    (1)
#define FALSE                   (0)
#ifndef NULL
#define NULL                    (0)
#endif

#define MAX_PATH                (255)
#define MAX_EXT                 (255)
#define INFINITE                (0xFFFFFFFF)


#define MAKEWORD(a, b)          ((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))
#define LOWORD(l)               ((WORD)(l))
#define HIWORD(l)               ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)               ((BYTE)(w))
#define HIBYTE(w)               ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define ZeroMemory(a, l)        memset((a), 0x00, (l))

#endif
//////////////////////////////////////////////////////////////////////////
