#ifndef COMMONINFO_H
#define COMMONINFO_H

#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include <string.h>

#define SIGNATURE_DATA_SIZE         2*1024*1024
#define LOG_SAVE_PATH               "/usr/local/LOG/SIG/DATA"
#define SIGNATURE_FILE_SAVE_PATH    "/usr/local/CFES/DATA/FORM/SIG"

// 设备类型
#define IDEV_TYPE_TSD               "TSD"
#define IDEV_TYPE_TPK               "TPK"

// 民生银行
#define PSGINFOFILE                 "/usr/local/LOG/SIG/DATA/PSG.des"
#define IMAGEINFOFILE               "/usr/local/LOG/SIG/DATA/IMAGE.png"
#define HIDEPICFILE                 "/usr/local/CFES/DATA/FORM/SIG/hidePic.png"
#define HIDESIGNFILE                "/usr/local/CFES/DATA/FORM/SIG/hideSign.txt"
#define SAVEIMAGEFILE               "/usr/local/CFES/DATA/FORM/SIG/ImageArray.txt"
#define APILOGFILE                  "/usr/local/LOG/SIG_API_LOG.txt"

#define MAX_ERRCODE                 (11)

// 定义COLORREF结构
typedef DWORD                       COLORREF;
#define RGB(r,g,b)                  ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#define GetRValue(rgb)              (LOBYTE(rgb))
#define GetGValue(rgb)              (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)              (LOBYTE((rgb) >> 16))

enum DEVICE_STATUS
{
    DEVICE_ONLINE                   = 0,
    DEVICE_OFFLINE                  = 1,
    DEVICE_HWERROR                  = 2
};

enum CAM_STATUS
{
    STATUS_NOTSUPP                  = 0,
    STATUS_OK                       = 1,
    STATUS_INOP                     = 2,
    STATUS_UNKNOWN                  = 3
};

enum CAM_WINDOW
{
    CAM_CREATE                      = 0,
    CAM_DESTROY                     = 1,
    CAM_PAUSE                       = 2,
    CAM_RESUME                      = 3,
    CAM_ERASE                       = 4,
    CAM_UNKNOWN                     = 5
};

enum MEDIA_STATUS
{
    MEDIA_OK                        = 0,
    MEDIA_BUSY                      = 1,
    MEDIA_UNKNOWN                   = 2,
    MEDIA_NOTSUPP                   = 3
};

// 加密算法
enum SIG_ALGORITHM
{
    ENCRYPT_NONE                    = 0,
    ENCRYPT_SM4,
    ENCRYPT_DES,
    ENCRYPT_AES,
    ENCRYPT_3DES
};

// 密钥用途
enum KEY_USE
{
    KEY_NONE,
    KEY_MASTER,                     // 直接灌注主密钥
    KEY_DECMASTER,                  // 解密后灌注主密钥
    KEY_ENCTRACK,                   // 轨迹数据加密密钥
    KEY_SM2PUBLIC                   // SM2公钥
};

// 加密密钥索引
enum KEY_INDEX
{
    KEY_PLAIN = -1,                 //明文

    KEY_INDEX_0                     = 0,
    KEY_INDEX_1,
    KEY_INDEX_2,
    KEY_INDEX_3,
    KEY_INDEX_4,
    KEY_INDEX_5,
    KEY_INDEX_6,
    KEY_INDEX_7,
    KEY_INDEX_8
};

// 颜色
#define COLOR_BLACK                       (0)
#define COLOR_WHITE                       (1)
#define COLOR_DARKGREY                    (2)
#define COLOR_GREY                        (3)
#define COLOR_LIGHTGREY                   (4)
#define COLOR_RED                         (5)
#define COLOR_GREEN                       (6)
#define COLOR_BLUE                        (7)
#define COLOR_CYAN                        (8)
#define COLOR_YELLOW                      (9)



// 签名窗口参数信息
typedef struct st_sig_showwin_info
{
    DWORD   hWnd;                   // 窗口ID
    WORD    wX;                     // 坐标X
    WORD    wY;                     // 坐标Y
    WORD    wWidth;                 // 窗口宽
    WORD    wHeight;                // 窗口高
    WORD    wHpixel;                // 像素
    WORD    wVpixel;                // 像素
    LPSTR   pszTexData;             // 背景文字及图片

    st_sig_showwin_info()
    {
        init();
    }

    void init()
    {
        memset(this, 0x00, sizeof(st_sig_showwin_info));
    }

}SIGHOWWININFO, *LPSIGSHOWWININFO;

// 电子签名配置信息
typedef struct st_sig_config_info
{
    WORD            wDeviceType;                                    // 设备类型
    CHAR            szDevDllName[MAX_PATH];                         // 库名称
    WORD            wEncrypt;                                       // 加密属性
    CHAR            pszPsgPath[MAX_PATH];                           // 签名数据路径
    CHAR            szImagePath[MAX_PATH];                          // 签名图片路径
    WORD            wPenWidth;                                      // 设置笔迹宽度
    WORD            wPenMultiple;                                   // 设置笔迹放大倍数
    WORD            wPenColor;                                      // 设置笔迹颜色
    COLORREF        wTextColor;                                     // 文本颜色
    COLORREF        wBackColor;                                     // 背景颜色
    WORD            wIsUseBackColor;                                // 是否使用背景颜色
    WORD            wTransparency;                                  // 设置背景透明度(0~255)
    WORD            wAlwaysShow;                                    // 一直显示
    WORD            wSaveTime;                                      // SignaturePic备份保存天数
    CHAR            pszKey[10];                                     // 默认主密钥
    CHAR            pszEncKey[10];                                  // 默认子密钥  
    CHAR            pszSignImagePath[MAX_PATH];                     // 存放下发过的签名图像的路径
    WORD            wAPILog;                                        // 设置启用接口日志
    CHAR            pszLogFile[MAX_PATH];                           // 日志存放路径

    st_sig_config_info()
    {
        init();
    }

    void init()
    {
        memset(this, 0x00, sizeof(st_sig_config_info));
    }

}SIGCONFIGINFO, *LPSIGCONFIGINFO;

#endif // COMMONINFO_H
