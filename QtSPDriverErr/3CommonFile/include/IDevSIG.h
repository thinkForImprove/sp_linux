#pragma once
/***************************************************************
* 文件名称：IDevSIG.h
* 文件描述：声明Camera底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/
#include "../XFS_SIG/CommonInfo.h"

#if defined(IDEVSIG_LIBRARY)
#define DEVSIG_EXPORT Q_DECL_EXPORT
#else
#define DEVSIG_EXPORT Q_DECL_IMPORT
#endif

#define CAM_CAMERAS_SIZE       (8)
#define MAX_LEN_FWVERSION      (255)   // 最大版本信息长度
//////////////////////////////////////////////////////////////////////////
// 返回码
#define CAM_SUCCESS             0       //操作成功!
#define ERR_OTHER               -1      //未知错误!
#define ERR_TIMEOUT             -2      //超时错误
#define ERR_OPENSERIAL          -3      //打开串口失败
#define ERR_SENDDATA            -4      //向串口发送数据失败
#define ERR_INVALID_PARA        -5      //接口参数错误!
#define ERR_NOFINDDLL           -6      //找不到动态链接库
#define ERR_LOADDLL             -7      //动态链接库加载错误
#define ERR_DEVBUZY             -8      //设备忙
#define ERR_DEVOFF              -9      //设备掉线
#define ERR_USERNOSIGN          -10     //客户未签名!
#define ERR_IMAGEFORMAT         -11     //无法识别图片文件格式!
#define ERR_SAVEIMAGE           -12     //保存图片文件出错!
#define ERR_CREATESIGNFILE      -13     //无法创建签名文件!
#define ERR_UNCONNECTDEV        -14     //无法连接设备!
#define ERR_RANGEWINDOWSIZE     -15     //前面框设置超出父窗口的大小范围!
#define ERR_RANGEPARAM          -16     //调用参数超出范围!
#define ERR_IMPORTKEY           -17     //灌注密钥出错!
#define ERR_ENCSIGNATURE        -18     //加密签名出错!
#define ERR_NOSTARTSIGN         -19     //未启动签名!
#define ERR_NOENDSIGN           -20     //未结束签名!
#define ERR_INVALID_HWND        -21     //无效的窗口句柄
#define ERR_UNSUPPVERSION       -22     //不支持的版本

//////////////////////////////////////////////////////////////////////////
typedef struct tag_dev_cam_status
{
    WORD   fwDevice[CAM_CAMERAS_SIZE];
    WORD   fwMedia[CAM_CAMERAS_SIZE];
    WORD   fwCameras[CAM_CAMERAS_SIZE];
    char   szErrCode[8];    // 三位的错误码

    tag_dev_cam_status() { Init(); }
    void Init()
    {
        memset(this, 0x00, sizeof(tag_dev_cam_status));
    }
} DEVCAMSTATUS, *LPDEVCAMSTATUS;

//////////////////////////////////////////////////////////////////////////
//#define DVECAM_NO_VTABLE  __declspec(novtable)
//////////////////////////////////////////////////////////////////////////
struct IDevSIG
{
    // 释放接口
    virtual void Release() = 0;
    // 打开连接
    virtual int Open() = 0;
    // 关闭连接
    virtual int Close() = 0;
    // 复位
    virtual int Reset() = 0;
    // 取状态
    virtual int GetStatus(DEVCAMSTATUS &stStatus) = 0;
    /************************************************************
    ** 功能：获取固件版本
    ** 输入：无
    ** 输出：pFWVersion：保存固件版本
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetFWVersion(LPSTR pFWVersion) = 0;
    // 设置属性
    virtual int SetProperty(const SIGCONFIGINFO stSigConfig) = 0;
    // 打开窗口
    virtual int Display(WORD wAction, WORD wX, WORD wY, WORD wWidth, WORD wHeight, WORD wHpixel, WORD wVpixel, LPSTR pszTexData, WORD wAlwaysShow, DWORD dwTimeout) = 0;
    // 获取签名
    virtual int GetSignature(LPSTR pszKey, LPSTR pszEncKey, LPSTR pszPictureFile, LPSTR pszSignatureFile, DWORD wnd, WORD wEncypt, LPSTR pszCamData, LPWSTR pswzUnicodeCamData) = 0;
    // 清除签名
    virtual int ClearSignature() = 0;
};

extern "C" DEVSIG_EXPORT long CreateIDevSIG(LPCSTR lpDevType, IDevSIG *&pDev);
//////////////////////////////////////////////////////////////////////////
