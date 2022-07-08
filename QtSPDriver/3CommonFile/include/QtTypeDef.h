#pragma once

//////////////////////////////////////////////////////////////////////////
#include "string.h"
#include <stdio.h>
//////////////////////////////////////////////////////////////////////////
//WIN和LINUX不一样的定义，32位和64位不一样字节大小的
#ifdef QT_WIN32
typedef unsigned long           DWORD;
#define SPETCPATH  "C:/CFES/ETC"
#define LOGPATH    "D:/LOG"
#else
typedef unsigned int           DWORD;
#define SPETCPATH  "/usr/local/CFES/ETC"
#define LOGPATH    "/usr/local/LOG"
#define WSAPCFGPATH "/home/projects/wsap/cfg"      //test#5
#define APVIRTUALPATH  "/home/cfes/CFESAgent/ini"  //30-00-00-00(FS#00001)
#define SPETCPATHFORXH  "/tmp/Config/"              //test#28
#endif

//进程间通信共享内存相关命名
#define IOMC_SEND_SHARE_MEMORY_KEY_NAME      "IOMCSMSEND"
#define IOMC_SEND_SHARE_MEMORY_MUTEX_NAME    "IOMCSMSENDMUTEX"
#define IOMC_RECV_SHARE_MEMORY_KEY_NAME      "IOMCSMRECV"
#define IOMC_RECV_SHARE_MEMORY_MUTEX_NAME    "IOMCSMRECVMUTEX"
#define IOMC_PROC_MUTEX_NAME                 "IOMCPROCMUTEX"

//////////////////////////////////////////////////////////////////////////
// LFS定义
typedef void                   *HANDLE;
typedef void                   *LPVOID;
typedef char                   *LPSTR;

typedef DWORD                  *LPDWORD;
typedef unsigned short          WORD;
typedef WORD                   *LPWORD;
typedef unsigned char           BYTE;
typedef BYTE                   *LPBYTE;
typedef long                    LONG;
typedef int                     BOOL;
typedef BOOL                   *LPBOOL;

typedef unsigned short          USHORT;
typedef char                    CHAR;
typedef short                   SHORT;
typedef unsigned long           ULONG;
typedef unsigned char           UCHAR;
typedef unsigned char          *LPUCHAR;
typedef SHORT                  *LPSHORT;
typedef LPVOID                 *LPLPVOID;
typedef ULONG                  *LPULONG;
typedef USHORT                 *LPUSHORT;

typedef HANDLE                  HPROVIDER;
typedef ULONG                   REQUESTID;
typedef REQUESTID              *LPREQUESTID;

typedef HANDLE                  HAPP;
typedef HAPP                   *LPHAPP;

typedef USHORT                  HSERVICE;
typedef HSERVICE               *LPHSERVICE;

typedef LONG                    HRESULT;
typedef HRESULT                *LPHRESULT;

#ifdef QT_LINUX
typedef void                   *HWND;
typedef HWND                   *LPHWND;
#endif

// 其他定义
typedef char                   *PSZ;
typedef float                   FLOAT;
typedef FLOAT                  *PFLOAT;
typedef int                    *LPINT;
typedef long                   *LPLONG;
typedef void                   *PVOID;
typedef int                     INT;
typedef unsigned int            UINT;
typedef UINT                   *PUINT;
typedef const char             *LPCSTR;
typedef wchar_t                 WCHAR;
typedef WCHAR                  *LPWSTR;
typedef const wchar_t          *LPCWSTR;
typedef double                  DOUBLE;

typedef __int32_t               INT32;
typedef __uint32_t              UINT32;
typedef __int64_t               INT64, LONGLONG;
typedef __uint64_t              UINT64, ULONGLONG;

//#ifdef __x86_64__
typedef __int64_t               INT_PTR, LONG_PTR;
typedef __uint64_t              UINT_PTR, ULONG_PTR;

/* Types use for passing & returning polymorphic values */
typedef UINT                    WPARAM;
typedef LONG                    LPARAM;
typedef LONG                    LRESULT;

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

#define MAKELONG(a, b)          ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define MAKEWORD(a, b)          ((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))
#define LOWORD(l)               ((WORD)(l))
#define HIWORD(l)               ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)               ((BYTE)(w))
#define HIBYTE(w)               ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define EXWORDHL(X)             (WORD)((((WORD)LOBYTE(X))<<8) | (HIBYTE(X)))
#define ZeroMemory(a, l)        memset((a), 0x00, (l))
#define GETBIT(a, b)            ((WORD)(a) >> (WORD)(b) & 0x01)

// 定义COLORREF结构
typedef DWORD                       COLORREF;
#define RGB(r,g,b)                  ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#define GetRValue(rgb)              (LOBYTE(rgb))
#define GetGValue(rgb)              (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)              (LOBYTE((rgb) >> 16))

// 初始化
#define MSET_0(a) memset(a, 0x00, sizeof(a));
#define MSET_S(a, s) memset(a, 0x00, s);
#define MSET_X0(a, X) memset(a, X, sizeof(a));
#define MSET_XS(a, X, s) memset(a, X, s);

// 比较
#define MCMP_IS0(a, b) \
    (memcmp(a, b, strlen(b)) == 0 && memcmp(a, b, strlen(a)) == 0)

// 复制(不指定长度)
#define MCPY_NOLEN(d, s) \
    memset(d, 0x00, sizeof(d)); \
    memcpy(d, s, strlen(s) > sizeof(d) ? sizeof(d) - 1 : strlen(s));

// 复制(指定长度)
#define MCPY_LEN(d, s, l) \
    memset(d, 0x00, sizeof(d)); \
    memcpy(d, s, l);

// 与或非处理
#define AND_IS1(s, d) ((s & d) == d)

// 转换: 0.1毫米单位转1毫米单位
#define M01M2MM(MM) (INT)((MM + 5) / 10)

// 毫米转像素数(入参: MM 0.1毫米单位, DPI)
#define MM2PX(MM, DPI) \
    (INT)((float)(MM * 1.0 / 10.0 / 25.4 * (float)(DPI * 1.0)) + 0.5 / 10.0)

// 一个字节(char)按8位获取数据定义
#define BIT0(a)     ((a & 0x01) == 0x01)  // 第一位
#define BIT1(a)     ((a & 0x02) == 0x02)
#define BIT2(a)     ((a & 0x04) == 0x04)
#define BIT3(a)     ((a & 0x08) == 0x05)
#define BIT4(a)     ((a & 0x10) == 0x10)
#define BIT5(a)     ((a & 0x20) == 0x20)
#define BIT6(a)     ((a & 0x40) == 0x40)
#define BIT7(a)     ((a & 0x80) == 0x80)

#endif
//////////////////////////////////////////////////////////////////////////
