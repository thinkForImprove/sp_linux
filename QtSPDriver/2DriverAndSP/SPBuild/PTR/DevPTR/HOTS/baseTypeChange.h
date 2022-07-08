#ifndef BASETYPECHANGE_H
#define BASETYPECHANGE_H

//#ifdef AFX_H
//#include "afx.h"
//#endif
#include <stdlib.h>
//T.B.D.#include "lfsheader/lfsapi.h"
//T.B.D.typedef SYSTEMTIME *LPSYSTEMTIME;       //wb
//T.B.D.#define _MAX_PATH           260 /* max. length of full pathname */

#define MAX_PATH            260
#define TRUE                true
#define FALSE               false
//#define OK                  true
//#define NG                  false

#define __stdcall
/* T.B.D.#define stdcall
#define __fastcall
#define __declspec(x)
#define __cdecl
#define _cdecl
#define __cdecl__
*/
#ifdef _UNICODE                     //wangbin
#define __T(x) L ## x
#define _T(x) __T(x)
#else                               //wangbin
#define _T(x) x                     //wangbin
#endif
#ifdef _UNICODE
#define TEXT(x) L##x
#else
#define TEXT(x) x
#endif
#define LFSDDESCRIPTION_LEN                     256
#define LFSDSYSSTATUS_LEN                       256
//#define _SH_DENYWR                              0x20
#define LZERROR_BADINHANDLE                     0
#define LZERROR_GLOBALLOC                       1
#define FORMAT_MESSAGE_ALLOCATE_BUFFER          0x00000100
#define FORMAT_MESSAGE_FROM_SYSTEM              0x00001000
#define FORMAT_MESSAGE_IGNORE_INSERTS           0x00000200
#define ERROR_FILE_NOT_FOUND                    0
#define ERROR_PATH_NOT_FOUND                    1
#define ERROR_INVALID_DATA                      2
#define ERROR_CURRENT_DIRECTORY                 3
#define ERROR_ALREADY_EXISTS                    4
#define HFILE_ERROR                             -1
#define ENOENT                                  1
#define ENXIO                                   2
#define EFAULT                                  3
#define ENODEV                                  4
#define ENOTDIR                                 5
#define ERROR_IO_PENDING                        4
#define LANG_NEUTRAL                            0
#define SUBLANG_DEFAULT                         0
#define INFINITE                                1
#define RAS_SP_EVT_PARAM_TRACE                  0x20
#define RAS_SP_PARAM_TRACE_1                    0X20
#define RAS_SP_PARAM_TRACE_0                    0X20
#define RAS_SP_INF_OUT_PARAM_TRACE              0x20
#define RAS_SP_OUT_PARAM_TRACE                  0x20
#define RAS_SP_EXEEVT_PARAM_TRACE               0x20
#define MAX_COMPUTERNAME_LENGTH                 0
#define RAS_BHT_OUTPUT_BUFFER                   0
#define SPICNTL_COMMAND_REJECTED                0
#define RAS_SP_INF_IN_PARAM_TRACE               0
#define RAS_SP_IN_PARAM_TRACE                   0
#define EXCEPTION_NONCONTINUABLE                0x1
#define EVENTLOG_ERROR_TYPE                     0x0001
#define DLL_PROCESS_ATTACH                      1
#define DLL_PROCESS_DETACH                      0
#define DLL_THREAD_ATTACH                       2
#define DLL_THREAD_DETACH                       3
#define ERROR_INVALID_HANDLE                    6L
#define GMEM_MODIFY                             0x0080
#define GMEM_ZEROINIT                           0x0040
#define CREATE_ALWAYS                           2
#define PAGE_READWRITE                          0x04
#define WAIT_FAILED                             (DWORD)0XFFFFFFFF
#define FILE_MAP_ALL_ACCESS                     2
#define KEY_ALL_ACCESS                          2
#define _fmemcpy                                memcpy
#define OFS_MAXPATHNAME                         128
#define OF_CREATE                               0x00001000
#define OF_WRITE                                0x00000001
#define REG_SZ                                  (1)
#define GMEM_DISCARDABLE                        0x0010
#define SECTION_MAP_WRITE                       0x0002
#define SMTO_ABORTIFHUNG                       0x0002
#define EXCEPTION_ACCESS_VIOLATION              0xC0000005
#define WAIT_OBJECT_0                           0x00000000L
#define WAIT_TIMEOUT                            0x00000102L
#define CALLBACK                                __attribute__((stdcall))
#define WS_OVERLAPPEDWINDOW                             1
#define ERROR_ACCESS_DENIED                         -1
#define PURGE_TXABORT                               0
#define PURGE_RXABORT                               1
#define LPTR                                        (0x0000 | 0x0040)
#define KEY_WRITE                                   1
#define REG_DWORD                                   1
//T.B.D.#define _MBC_LEAD                               0x00000052
//T.B.D.#define _MBC_SINGLE                             0
//T.B.D.#define _MBC_ILLEGAL                            -1
#define far
#define FAR                                     far
#define OF_EXIST                                0x00000049
#define OF_READWRITE                            0x00000002
#define GHND                                    0x00000062
#define GMEM_SHARE                              0x00000061
#define OF_READ                                 0x00000000
#define WM_CLOSE                                0x00000064
#define GMEM_FIXED                              0x00000063
#define GMEM_MOVEABLE                           0x0020
#define FILE_ATTRIBUTE_DIRECTORY                0x00000080
#define FILE_SHARE_READ                         0x00000066
#define OF_DELETE                               0x00000065
#define FILE_SHARE_WRITE                        0x00000067
#define FILE_DEVICE_UNKNOWN                     0x00000068
#define METHOD_BUFFERED                         0x00000069
#define FILE_ANY_ACCESS                         0x00000070
#define CTL_CODE(DeviceType, Function, Method, Access) (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#define EVENTLOG_WARNING_TYPE                   2
#define MAXCHAR                                 256
#define MAXBYTE                                 256
#define MAXWORD                                 256
#define EV_RXCHAR                               0
#define EV_ERR                                  1
#define CE_RXPARITY                             2
#define CE_OVERRUN                              3
#define CE_FRAME                                4
#define EVENTLOG_INFORMATION_TYPE               5
#define ERROR_INSUFFICIENT_BUFFER               6
#define SPDRP_HARDWAREID                        7
#define DIGCF_PRESENT                           8
#define DIGCF_ALLCLASSES                        9
#define CREATE_SUSPENDED                        10
#define CE_MODE                                 11
#define PURGE_TXCLEAR                           12
#define PURGE_RXCLEAR                           13
#define FILE_CURRENT                            1
#define REG_OPTION_NON_VOLATILE                 0



#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif
//#define max(a,b)            (((a) > (b)) ? (a) : (b))
//#define min(a,b)            (((a) < (b)) ? (a) : (b))
typedef void                *HCERTSTORE/*,*LPTHREAD_START_ROUTINE*/, *TIMERPROC;
typedef void                *HANDLE, *HDEVINFO, *HMIXEROBJ;
typedef int                 BOOL,       HFILE,     INT,       INT32, PSID;
typedef int                 *LPBOOL,    *PBOOL,   *LPINT;
typedef unsigned int        UINT,       WPARAM, DWORD, DWORD_PTR;
typedef unsigned int        *LPUINT,    *PUINT, *LPDWORD, *PDWORD, *PDWORD_PTR;
typedef char                CHAR,       XCHAR;
typedef char                *LPCHAR,    *PCHAR,    *LPSTR,     *PSTR;
typedef unsigned char       BYTE,       BOOLEAN,   UCHAR;
typedef unsigned char       *LPBYTE,    *PBYTE,    *LPUCHAR,   *PUCHAR;
typedef const char              *LPCSTR,    *PCSTR, CSring, *PCXSTR;
typedef short               SHORT;
typedef short               *LPSHORT;
typedef unsigned short      USHORT,     ATOM,      LANGID,     WORD;
typedef unsigned short      *LPUSHORT,  *PUSHORT,   *PWORD,     *LPWORD;
typedef long                LONG,        INT_PTR,   LPARAM,     LRESULT, FARPROC;
typedef long                *LPLONG,    *PLONG;
typedef unsigned long       ULONG,      REGSAM,     SIZE_T,      COLORREF,   UINT_PTR,    LCID,     ULONG_PTR ;
typedef unsigned long       *LPULONG,   *PULONG,   *PLCID;
typedef long long           LONGLONG,    USN;
typedef long long           *LPLONGLONG;
typedef unsigned long long  ULONGLONG;
typedef unsigned long long  *LPULONGLONG;
typedef void                VOID;
typedef void                *LPVOID,    *HGLOBAL,  *PVOID,     *SC_LOCK;
typedef const void          *LPCVOID;
typedef LPVOID              *LPLPVOID;
typedef float               FLOAT;
typedef ULONG_PTR HCRYPTPROV;
typedef ULONG_PTR HCRYPTHASH;
typedef ULONG_PTR HCRYPTKEY;
typedef float               *LPFLOAT,   *PFLOAT;
typedef HANDLE              HKEY,       HBITMAP,   HBRUSH,     HCURSOR,     HDC,         HFONT,       HICON,    HINSTANCE,    HMODULE,     HPALETTE,     HWND,  SC_HANDLE,     HGDIOBJ;
typedef HANDLE              *PHKEY,    *LPHANDLE,  *PHANDLE, HAPP, HPROVIDER;
typedef wchar_t             WCHAR;
typedef wchar_t             *LPWSTR;
typedef unsigned int UINT32;
typedef short        INT16;
typedef unsigned short UINT16;
#define CLSID GUID


//#define _UNICODE
#ifdef _UNICODE
typedef const wchar_t *LPCTSTR, *PCTSTR;
typedef wchar_t        *PTSTR, *LPTSTR;
typedef wchar_t        TCHAR;
#else
typedef const char *LPCTSTR, *PCTSTR;
typedef char        *PCHAR, *LPTSTR;
typedef char        TCHAR;
#endif

typedef const wchar_t       *LPCWSTR;
typedef LONG                 HRESULT;
typedef HRESULT               *LPHRESULT;
typedef ULONG REQUESTID;
typedef REQUESTID *LPREQUESTID;
typedef USHORT HSERVICE;
typedef HSERVICE *LPHSERVICE;

const HKEY HKEY_USERS = (HKEY)"HKEY_USERS";
const HKEY HKEY_LOCAL_MACHINE = (HKEY)"HKEY_LOCAL_MACHINE";
const HKEY INVALID_HANDLE_VALUE   = (HKEY)"INVALID_HANDLE_VALUE";
const HKEY HKEY_CLASSES_ROOT = (HKEY)"HKEY_CLASSES_ROOT";                   //mcu
const HKEY HKEY_CURRENT_USER = (HKEY)"HKEY_CURRENT_USER";                   //mcu
const DWORD GENERIC_READ =                         1;
const DWORD GENERIC_WRITE =                      2;
const DWORD OPEN_EXISTING =                      3;
const DWORD FILE_ATTRIBUTE_NORMAL =                      4;
const DWORD FILE_FLAG_OVERLAPPED =                      5;
const REGSAM KEY_READ =                      0;

typedef int a_list;
typedef HWND HLOCAL;


//typedef union _ULARGE_INTEGER {
//  struct {
//    DWORD LowPart;
//    DWORD HighPart;
//  };
//  struct {
//    DWORD LowPart;
//    DWORD HighPart;
//  } u;
//  ULONGLONG QuadPart;
//} ULARGE_INTEGER,
// *PULARGE_INTEGER;
typedef struct _FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;
typedef struct _COMSTAT   // cst
{

    DWORD fCtsHold : 1;   // Tx waiting for CTS signal

    DWORD fDsrHold : 1;   // Tx waiting for DSR signal

    DWORD fRlsdHold : 1;  // Tx waiting for RLSD signal

    DWORD fXoffHold : 1;  // Tx waiting, XOFF char rec''d

    DWORD fXoffSent : 1;  // Tx waiting, XOFF char sent

    DWORD fEof : 1;       // EOF character sent

    DWORD fTxim : 1;      // character waiting for Tx

    DWORD fReserved : 25; // reserved

    DWORD cbInQue;        // bytes in input buffer

    DWORD cbOutQue;       // bytes in output buffer

} COMSTAT, *LPCOMSTAT;

//typedef struct _LPOFSTRUCT
//{


//}LPOFSTRUCT;

/*wb del start for redefination start
typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME,*LPSYSTEMTIME;
wb del end*/

typedef struct _OVERLAPPED
{
    DWORD Internal;
    DWORD InternalHigh;
    DWORD Offset;
    DWORD OffsetHigh;
    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED, *LPOVERLAPPED_COMPLETION_ROUTINE;


typedef struct _CRYPTOAPI_BLOB
{
    DWORD cbData;
    BYTE  *pbData;
} CRYPT_INTEGER_BLOB, *PCRYPT_INTEGER_BLOB,
CRYPT_UINT_BLOB, *PCRYPT_UINT_BLOB,
CRYPT_OBJID_BLOB, *PCRYPT_OBJID_BLOB,
CERT_NAME_BLOB, CERT_RDN_VALUE_BLOB,
*PCERT_NAME_BLOB, *PCERT_RDN_VALUE_BLOB,
CERT_BLOB, *PCERT_BLOB, CRL_BLOB,
*PCRL_BLOB, DATA_BLOB, *PDATA_BLOB,
CRYPT_DATA_BLOB, *PCRYPT_DATA_BLOB,
CRYPT_HASH_BLOB, *PCRYPT_HASH_BLOB,
CRYPT_DIGEST_BLOB, *PCRYPT_DIGEST_BLOB,
CRYPT_DER_BLOB, PCRYPT_DER_BLOB,
CRYPT_ATTR_BLOB, *PCRYPT_ATTR_BLOB;

typedef struct _CRYPT_ALGORITHM_IDENTIFIER
{
    LPSTR            pszObjId;
    CRYPT_OBJID_BLOB Parameters;
} CRYPT_ALGORITHM_IDENTIFIER, *PCRYPT_ALGORITHM_IDENTIFIER;


typedef struct _CRYPT_BIT_BLOB
{
    DWORD cbData;
    BYTE  *pbData;
    DWORD cUnusedBits;
} CRYPT_BIT_BLOB, *PCRYPT_BIT_BLOB;

//typedef struct _SP_DEVINFO_DATA {
//  DWORD     cbSize;
//  GUID      ClassGuid;
//  DWORD     DevInst;
//  ULONG_PTR Reserved;
//} SP_DEVINFO_DATA, *PSP_DEVINFO_DATA;


typedef struct _CERT_PUBLIC_KEY_INFO
{
    CRYPT_ALGORITHM_IDENTIFIER Algorithm;
    CRYPT_BIT_BLOB             PublicKey;
} CERT_PUBLIC_KEY_INFO, *PCERT_PUBLIC_KEY_INFO;


typedef struct _CERT_EXTENSION
{
    LPSTR            pszObjId;
    BOOL             fCritical;
    CRYPT_OBJID_BLOB Value;
} CERT_EXTENSION, *PCERT_EXTENSION;
typedef struct _CERT_INFO
{
    DWORD                      dwVersion;
    CRYPT_INTEGER_BLOB         SerialNumber;
    CRYPT_ALGORITHM_IDENTIFIER SignatureAlgorithm;
    CERT_NAME_BLOB             Issuer;
    FILETIME                   NotBefore;
    FILETIME                   NotAfter;
    CERT_NAME_BLOB             Subject;
    CERT_PUBLIC_KEY_INFO       SubjectPublicKeyInfo;
    CRYPT_BIT_BLOB             IssuerUniqueId;
    CRYPT_BIT_BLOB             SubjectUniqueId;
    DWORD                      cExtension;
    PCERT_EXTENSION            rgExtension;
} CERT_INFO, *PCERT_INFO;

typedef struct _CERT_CONTEXT
{
    DWORD dwCerEncodingType;
    BYTE  *pbCertEncoded;
    DWORD cbCertEncoded;
    PCERT_INFO pCertInfo;
    HCERTSTORE HcertStore;


} CERT_CONTEXT, *PCERT_CONTEXT;

typedef struct _SECURITY_ATTRIBUTES
{
    DWORD  nLength;
    LPVOID lpSecurityDescriptor;
    BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef const CERT_CONTEXT *PCCERT_CONTEXT;

typedef struct _OFSTRUCT
{
    BYTE cBytes;
    BYTE fFixeDisk;
    WORD nErrCode;
    WORD Reserved1;
    WORD Reserved2;
    CHAR szPathName[OFS_MAXPATHNAME];

} OFSTRUCT, *LPOFSTRUCT, *POFSTRUCT;



//调试时追加
//typedef int WIN32_FIND_DATA;
typedef struct _WIN32_FIND_DATA
{
    DWORD dwFileAttributes; //文件属性
    FILETIME ftCreationTime; // 文件创建时间
    FILETIME ftLastAccessTime; // 文件最后一次访问时间
    FILETIME ftLastWriteTime; // 文件最后一次修改时间
    DWORD nFileSizeHigh; // 文件长度高32位
    DWORD nFileSizeLow; // 文件长度低32位
    DWORD dwReserved0; // 系统保留
    DWORD dwReserved1; // 系统保留
    TCHAR cFileName[ MAX_PATH ]; // 长文件名
    TCHAR cAlternateFileName[ 14 ]; // 8.3格式文件名
} WIN32_FIND_DATA, *PWIN32_FIND_DATA;
typedef WIN32_FIND_DATA *LPWIN32_FIND_DATA;

typedef struct _PROCESS_INFORMATION
{
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION, *LPPROCESS_INFORMATION;

typedef struct _STARTUPINFO
{
    DWORD cb;
    LPTSTR lpReserved;
    LPTSTR lpDesktop;
    LPTSTR lpTitle;
    DWORD dwX;
    DWORD dwY;
    DWORD dwXSize;
    DWORD dwYSize;
    DWORD dwXCountChars;
    DWORD dwYCountChars;
    DWORD dwFillAttribute;
    DWORD dwFlags;
    WORD wShowWindow;
    WORD cbReserved2;
    LPBYTE lpReserved2;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
} STARTUPINFO, *LPSTARTUPINFO;

typedef struct tagPOINT
{
    LONG x;
    LONG y;
} POINT;

typedef struct _GLYPHMETRICS
{
    UINT  gmBlackBoxX;     //指定完全包围字体结构的最小矩阵的宽度
    UINT  gmBlackBoxY;     //指定完全包围字体结构的最小矩阵的高度
    POINT gmptGlyphOrigin; //指定完全包围字体结构的最小矩阵左上角的点坐标
    short gmCellIncX;      //指定当前的起点到下一个字符的起点的水平距离
    short gmCellIncY;      //...垂直距离
} GLYPHMETRICS, *LPGLYPHMETRICS;

typedef   struct   _SP_DEVINFO_DATA
{
    DWORD   cbSize;
    //  GUID   ClassGuid;
    DWORD   DevInst;
    ULONG_PTR   Reserved;
}   SP_DEVINFO_DATA,   *PSP_DEVINFO_DATA;


typedef struct _COMMTIMEOUTS
{
    DWORD ReadIntervalTimeout;
    DWORD ReadTotalTimeoutMultiplier;
    DWORD ReadTotalTimeoutConstant;
    DWORD WriteTotalTimeoutMultiplier;
    DWORD WriteTotalTimeoutConstant;
} COMMTIMEOUTS, *LPCOMMTIMEOUTS;
//typedef void *LPSECURITY_ATTRIBUTES;
#define LFSAPI                                  __attribute__((__cdecl__))
#define WINAPI                                  __attribute__((__cdecl__))
#define CONST                              const
#define LFS_MGR_DBUS_NAME                       "org.lfs.manager"
#define LFS_MGR_DBUS_INTF_NAME                  "org.lfs.windows_compatible_intf"

#define WM_USER                                 (0x0400)
#define NO_ERROR                                 (0)
//struct __POSITION{};                                  //wangbin
//typedef __POSITION *POSITION;                         //wangbin
typedef int POSITION;                                   //wangbin
#define CBR_9600 9600
#define CBR_115200 115200
#define CBR_57600 57600
#define CBR_38400 38400
#define CBR_19200 19200
#define CBR_4800 4800
#define CBR_2400 2400
#define CBR_1200 1200
#define ERROR_SUCCESS  0L


typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;


typedef struct _FIXED
{
    short   value;
    WORD    fract;
} FIXED;

typedef struct _MAT2
{
    FIXED eM11;
    FIXED eM12;
    FIXED eM21;
    FIXED eM22;
} MAT2, *LPMAT2;

#define near
#define NEAR                                     near
typedef struct tagTEXTMETRICA
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
} TEXTMETRICA, *PTEXTMETRICA, NEAR *NPTEXTMETRICA, FAR *LPTEXTMETRICA;
typedef struct tagTEXTMETRICW
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    WCHAR       tmFirstChar;
    WCHAR       tmLastChar;
    WCHAR       tmDefaultChar;
    WCHAR       tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
} TEXTMETRICW, *PTEXTMETRICW, NEAR *NPTEXTMETRICW, FAR *LPTEXTMETRICW;
#ifdef UNICODE
typedef TEXTMETRICW TEXTMETRIC;
typedef PTEXTMETRICW PTEXTMETRIC;
typedef NPTEXTMETRICW NPTEXTMETRIC;
typedef LPTEXTMETRICW LPTEXTMETRIC;
#else
typedef TEXTMETRICA TEXTMETRIC;
typedef PTEXTMETRICA PTEXTMETRIC;
typedef NPTEXTMETRICA NPTEXTMETRIC;
typedef LPTEXTMETRICA LPTEXTMETRIC;
#endif // UNICODE

typedef DWORD (__stdcall *LPTHREAD_START_ROUTINE)(/*[in]*/ LPVOID lpThreadParameter);
typedef struct _devicemode
{
    WORD  dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
} DEVMODE, LPDEVMODE ;
//#define TIMERPROC void(CALLBACK EXPORT *lpfnTimer) (HWND,UINT ,YINT ,DWORD)

typedef struct _OSVERSIONINFO
{
    DWORD dwOSVersionInfoSize;//指定该数据结构的字节大小
    DWORD dwMajorVersion;//操作系统的主版本号
    DWORD dwMinorVersion;//操作系统的副版本号

    DWORD dwBuildNumber;//操作系统的创建号
    DWORD dwPlatformId;//操作系统ID号
    TCHAR szCSDVersion[ 128 ];//关于操作系统的一些附加信息

} OSVERSIONINFO, *LPOSVERSIONINFO;
//SIU

#define IOCTL_BATTERY_QUERY_TAG 0
#define BATTERY_TAG_INVALID 0
#define IOCTL_BATTERY_QUERY_STATUS 0
#define BATTERY_DISCHARGING 0
#define KEY_EXECUTE 0
#define DIGCF_DEVICEINTERFACE 0
#define SPINT_ACTIVE 0
#define ANYSIZE_ARRAY 9
#define RTS_CONTROL_DISABLE 0x00
#define DTR_CONTROL_ENABLE 0x01
#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
DECLARE_HANDLE(HMIXER);
#define MIXER_SHORT_NAME_CHARS 22
#define MIXER_LONG_NAME_CHARS 22
#define MAXPNAMELEN 22
#define USB_PRM_NO_RSP 0
#define USB_DRV_FN_DATASEND 0
#define USB_PRM_USUALLY 0
#define USB_DRV_FN_DATASENDRCV 0
#define USB_PRM_USUALLY 0
#define MIXER_SETCONTROLDETAILSF_VALUE 0
#define MIXER_GETLINECONTROLSF_ONEBYTYPE 0
#define MIXERCONTROL_CONTROLTYPE_VOLUME 0
#define MIXER_GETLINEINFOF_COMPONENTTYPE 0
#define MIXERLINE_COMPONENTTYPE_DST_SPEAKERS 0
#define MMSYSERR_NOERROR 0
#define USB_EX_PRM_NO_TRACE 0
#define MIXERCONTROL_CONTROLTYPE_MUTE 0
//T.B.D.#define MIXER_GETLINECONTROLSF_ONEBYTYPE  0x00000002L
#define KEY_QUERY_VALUE             (0x0001)
#define FILE_FLAG_SEQUENTIAL_SCAN   0x08000000
typedef UINT MMVERSION;
typedef HMIXER FAR *LPHMIXER;
typedef UINT MMRESULT;


typedef struct tagMIXERLINE
{
    DWORD       cbStruct;               /* size of MIXERLINE structure */
    DWORD       dwDestination;          /* zero based destination index */
    DWORD       dwSource;               /* zero based source index (if source) */
    DWORD       dwLineID;               /* unique line id for mixer device */
    DWORD       fdwLine;                /* state/information about line */
    DWORD_PTR   dwUser;                 /* driver specific information */
    DWORD       dwComponentType;        /* component type line connects to */
    DWORD       cChannels;              /* number of channels line supports */
    DWORD       cConnections;           /* number of connections [possible] */
    DWORD       cControls;              /* number of controls at this line */
    WCHAR       szShortName[MIXER_SHORT_NAME_CHARS];
    WCHAR       szName[MIXER_LONG_NAME_CHARS];
    struct
    {
        DWORD       dwType;                 /* MIXERLINE_TARGETTYPE_xxxx */
        DWORD       dwDeviceID;             /* target device ID of device type */
        WORD        wMid;                   /* of target device */
        WORD        wPid;                   /*      " */
        MMVERSION   vDriverVersion;         /*      " */
        WCHAR       szPname[MAXPNAMELEN];   /*      " */
    } Target;
} MIXERLINE, *PMIXERLINE, *LPMIXERLINE;

typedef struct _BATTERY_WAIT_STATUS
{
    ULONG BatteryTag;
    ULONG Timeout;
    ULONG PowerState;
    ULONG LowCapacity;
    ULONG HighCapacity;
} BATTERY_WAIT_STATUS, *PBATTERY_WAIT_STATUS;

typedef struct _BATTERY_STATUS
{
    ULONG PowerState;
    ULONG Capacity;
    ULONG Voltage;
    LONG Rate;
} BATTERY_STATUS, *PBATTERY_STATUS ;

typedef struct _SP_DEVICE_INTERFACE_DATA
{
    DWORD cbSize;
    GUID InterfaceClassGuid;
    DWORD Flags;
    ULONG_PTR Reserved;
} SP_DEVICE_INTERFACE_DATA, *PSP_DEVICE_INTERFACE_DATA;

typedef struct _SP_DEVICE_INTERFACE_DETAIL_DATA
{
    DWORD cbSize;
    TCHAR DevicePath[ANYSIZE_ARRAY];
} SP_DEVICE_INTERFACE_DETAIL_DATA, *PSP_DEVICE_INTERFACE_DETAIL_DATA;

typedef struct tagMIXERCONTROL
{
    DWORD           cbStruct;           /* size in bytes of MIXERCONTROL */
    DWORD           dwControlID;        /* unique control id for mixer device */
    DWORD           dwControlType;      /* MIXERCONTROL_CONTROLTYPE_xxx */
    DWORD           fdwControl;         /* MIXERCONTROL_CONTROLF_xxx */
    DWORD           cMultipleItems;     /* if MIXERCONTROL_CONTROLF_MULTIPLE set */
    WCHAR           szShortName[MIXER_SHORT_NAME_CHARS];
    WCHAR           szName[MIXER_LONG_NAME_CHARS];
    union
    {
        struct
        {
            LONG    lMinimum;           /* signed minimum for this control */
            LONG    lMaximum;           /* signed maximum for this control */
        } DUMMYSTRUCTNAME;
        struct
        {
            DWORD   dwMinimum;          /* unsigned minimum for this control */
            DWORD   dwMaximum;          /* unsigned maximum for this control */
        } DUMMYSTRUCTNAME2;
        DWORD       dwReserved[6];
    } Bounds;
    union
    {
        DWORD       cSteps;             /* # of steps between min & max */
        DWORD       cbCustomData;       /* size in bytes of custom data */
        DWORD       dwReserved[6];      /* !!! needed? we have cbStruct.... */
    } Metrics;
} MIXERCONTROL, *PMIXERCONTROL, *LPMIXERCONTROL;

typedef struct
{
    LONG fValue;
} MIXERCONTROLDETAILS_BOOLEAN;

typedef struct tMIXERCONTROLDETAILS
{
    DWORD           cbStruct;       /* size in bytes of MIXERCONTROLDETAILS */
    DWORD           dwControlID;    /* control id to get/set details on */
    DWORD           cChannels;      /* number of channels in paDetails array */
    union
    {
        HWND        hwndOwner;      /* for MIXER_SETCONTROLDETAILSF_CUSTOM */
        DWORD       cMultipleItems; /* if _MULTIPLE, the number of items per channel */
    } DUMMYUNIONNAME;
    DWORD           cbDetails;      /* size of _one_ details_XX struct */
    LPVOID          paDetails;      /* pointer to array of details_XX structs */
} MIXERCONTROLDETAILS, *PMIXERCONTROLDETAILS, FAR *LPMIXERCONTROLDETAILS;

typedef struct tMIXERCONTROLDETAILS_UNSIGNED
{
    DWORD           dwValue;
}       MIXERCONTROLDETAILS_UNSIGNED,
*PMIXERCONTROLDETAILS_UNSIGNED,
FAR *LPMIXERCONTROLDETAILS_UNSIGNED;

typedef struct
{
    DWORD cbStruct;
    DWORD dwLineID;
    union
    {
        DWORD dwControlID;
        DWORD dwControlType;
    };
    DWORD cControls;
    DWORD cbmxctrl;
    LPMIXERCONTROL pamxctrl;
} MIXERLINECONTROLS, *PMIXERLINECONTROLS, FAR *LPMIXERLINECONTROLS;

//MCU
typedef struct _RECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT;
#ifndef IN
#define IN
#endif

#ifdef OUT
#define OUT
#endif

//T.B.D.#pragma pack(push,1)
typedef struct tagBITMAPFILEHEADER
{
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    DWORD  biWidth;
    DWORD  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    DWORD  biXPelsPerMeter;
    DWORD  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD
{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;
#pragma pack()

#define SPDRP_DRIVER 0
#define ERROR_OUTOFMEMORY                       188           //llinux
#define ERROR_TOO_MANY_OPEN_FILES               11          //llinux
#define ERROR_INVALID_NAME                      12
#define ERROR_INVALID_FUNCTION                  9
#define THREAD_PRIORITY_NORMAL                   0
#define WAIT_ABANDONED                          0x00000103L

#endif // BASETYPECHANGE_H
