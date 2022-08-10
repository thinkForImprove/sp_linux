#ifndef _COMMON_OS_COMPATIBLE_DEF_H_
#define _COMMON_OS_COMPATIBLE_DEF_H_


#include <time.h>
#include <stdio.h>
#include <syslog.h>
#include "QtTypeDef.h"

#ifdef WIN32
// data type
typedef __int8                                          __int8_t;
typedef unsigned __int8                                 __uint8_t;
typedef __int16                                         __int16_t;
typedef unsigned __int16                                __uint16_t;
typedef __int32                                         __int32_t;
typedef unsigned __int32                                __uint32_t;
typedef __int64                                         __int64_t;
typedef unsigned __int64                                __uint64_t;

// path separator
#define PATH_SPT_CHR					'\\'
#define PATH_SPT_STR					"\\"

#define IS_FULL_PATH(PathStr) ((PathStr != NULL) && (strlen(PathStr) > 1) && (PathStr[1] == ':'))

#else

#ifndef MAX_PATH
#define MAX_PATH					(260)
#endif // MAX_PATH
#define TRUE                                            (1)
#define FALSE                                           (0)

// Syslog 优先级声明(数值越小，事件越严重)
#define EVENTLOG_EMERG_TYPE     LOG_EMERG       // 0:严重/紧急(内核崩溃等)
#define EVENTLOG_ALERT_TYPE     LOG_ALERT       // 1:报告(必须立即采取措施等)
#define EVENTLOG_CRIT_TYPE      LOG_CRIT        // 2:严重(涉及严重的硬件或软件操作失败)
#define EVENTLOG_ERROR_TYPE     LOG_ERR         // 3:错误(阻止某个功能或者模块不能正常工作)
#define EVENTLOG_WARNING_TYPE   LOG_WARNING     // 4:警告(对可能出现问题的情况进行警告)
#define EVENTLOG_NOTICE_TYPE    LOG_NOTICE      // 5:通知(正常但又重要的消息。用于提醒，常用于与安全相关的消息)
#define EVENTLOG_INFO_TYPE      LOG_INFO        // 6:提示
#define EVENTLOG_DEBUG_TYPE     LOG_DEBUG       // 7:调试

/*typedef void*                                           HANDLE;
typedef char                                            CHAR;
typedef wchar_t                                         WCHAR;
typedef unsigned char                                   UCHAR, BYTE, *LPBYTE;
typedef short                                           SHORT;
typedef unsigned short                                  USHORT, WORD, *LPWORD;
typedef int                                             INT;
typedef unsigned int                                    UINT;
typedef __int32_t                                       INT32;
//typedef __uint32_t                                      UINT32, DWORD, *LPDWORD;
typedef __int64_t                                       INT64, LONGLONG;
typedef __uint64_t                                      UINT64, ULONGLONG;
typedef int                                             BOOL, *LPBOOL;
typedef char*                                           LPSTR;
typedef const char*                                     LPCSTR;
typedef wchar_t*                                        LPWSTR;
typedef const wchar_t*                                  LPCWSTR;

//#ifdef __x86_64__
typedef __int64_t                                       INT_PTR, LONG_PTR;
typedef __uint64_t                                      UINT_PTR, ULONG_PTR;
//#elif __i386
//typedef __int32_t                                       INT_PTR, LONG_PTR;
//typedef __uint32_t                                      UINT_PTR, ULONG_PTR;
//#endif
*/
/*#ifndef _SYSTEMTIME_DEF
#define _SYSTEMTIME_DEF
typedef struct _SYSTEMTIME_ {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME, *PSYSTEMTIME;
#endif*/

#ifndef INFINITE
#define INFINITE                                        ((unsigned long)-1)
#endif // INFINITE

#define PATH_SPT_CHR					'/'
#define PATH_SPT_STR					"/"

#define IS_FULL_PATH(PathStr) ((PathStr != NULL) && (strlen(PathStr) > 0) && (PathStr[0] == '/'))

#endif // WIN32


#endif // _COMMON_OS_COMPATIBLE_DEF_H_
