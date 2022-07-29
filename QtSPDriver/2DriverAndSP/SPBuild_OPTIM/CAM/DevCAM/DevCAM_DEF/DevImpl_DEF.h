/***************************************************************************
* 文件名称: DevImpl_DEF.h
* 文件描述：摄像功能底层封装类共通 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年6月6日
* 文件版本：1.0.0.1
****************************************************************************/

#ifndef DEVIMPL_DEF_H
#define DEVIMPL_DEF_H

#include "QtTypeDef.h"
#include "malloc.h"

/***************************************************************************
// 返回值/错误码　宏定义 (0~-300共通定义)
***************************************************************************/
#define IMP_ERR_DEF_NUM                 300
#define IMP_SUCCESS                     0           // 成功
#define IMP_ERR_LOAD_LIB                -1          // 动态库加载错误
#define IMP_ERR_PARAM_INVALID           -2          // 参数无效
#define IMP_ERR_UNKNOWN                 -3          // 未知错误
#define IMP_ERR_NOTOPEN                 -4          // 设备未Open
#define IMP_ERR_OPENFAIL                -5          // 设备打开失败/未打开
#define IMP_ERR_NODEVICE                -6          // 未找到有效设备
#define IMP_ERR_OFFLINE                 -7          // 设备断线
#define IMP_ERR_NODETECT                -8          // 未检测到活体
#define IMP_ERR_INTEREXEC               -9          // 接口执行错误
#define IMP_ERR_SET_VMODE               -10         // 摄像参数设置错误
#define IMP_ERR_SET_RESO                -11         // 分辨率设置失败
#define IMP_ERR_SHAREDMEM_RW            -12         // 共享内存读写失败
#define IMP_ERR_MEMORY                  -13         // 内存错误

// 检查是否共通定义错误码
#define CHK_ERR_ISDEF(e)                ((e <= 0 && e >= -300) ? TRUE : FALSE)

/***************************************************************************
// 无分类　宏定义
***************************************************************************/
#define LOG_NAME                    "DevImpl_JDY5001A0809.log"   // 缺省日志名

enum EN_DEVSTAT
{
    DEV_OK          = 0,    // 设备正常
    DEV_NOTFOUND    = 1,    // 设备未找到
    DEV_NOTOPEN     = 2,    // 设备未打开
    DEV_OFFLINE     = 3,    // 设备已打开但断线
    DEV_UNKNOWN     = 4,    // 设备状态未知
};

enum EN_VIDEOMODE
{
    VM_WIDTH        = 0,    // 宽度
    VM_HEIGHT       = 1,    // 高度
    VM_FPS          = 2,    // 帧率(帧/秒)
    VM_BRIGHT       = 3,    // 亮度(-255 ~ 255)(RGB相关)
    VM_CONTRAST     = 4,    // 对比度
    VM_SATURATION   = 5,    // 饱和度
    VM_HUE          = 6,    // 色调
    VM_EXPOSURE     = 7,    // 曝光
};

typedef struct ST_IMAGE_DATA
{
    INT nWidth;                 // 图像宽
    INT nHeight;                // 图像高
    INT nFormat;                // 图像通道数
    UCHAR *ucImgData;           // 图像数据Buffer
    ULONG ulImagDataLen;        // 图像数据BufferSize
    INT nOtherParam[24];        // 其他参数

    ST_IMAGE_DATA()
    {
        memset(this, 0x00, sizeof(ST_IMAGE_DATA));
    }

    void Clear()
    {
        if (ucImgData != nullptr)
        {
            free(ucImgData);
            ucImgData = nullptr;
        }
        memset(this, 0x00, sizeof(ST_IMAGE_DATA));
    }
} STIMGDATA, *LPSTIMGDATA;

// CDevImpl共通类
class CDevImpl_DEF
{
public:
    CDevImpl_DEF() {}
    ~CDevImpl_DEF() {}

public:
    // Impl共通定义错误码转换解释字符串
    INT DEF_ConvertCode_Impl2Str(INT nErrCode, LPSTR lpRetStr)
    {
    #define PUB_CASE_CODE_STR(IMP, CODE, STR) \
        case IMP: \
            sprintf(lpRetStr, "%d|%s", CODE, STR); \
            break;

        switch(nErrCode)
        {
            PUB_CASE_CODE_STR(IMP_SUCCESS, nErrCode, "成功")
            PUB_CASE_CODE_STR(IMP_ERR_LOAD_LIB, nErrCode, "动态库加载错误")
            PUB_CASE_CODE_STR(IMP_ERR_PARAM_INVALID, nErrCode, "参数无效")
            PUB_CASE_CODE_STR(IMP_ERR_UNKNOWN, nErrCode, "未知错误")
            PUB_CASE_CODE_STR(IMP_ERR_NOTOPEN, nErrCode, "设备未Open")
            PUB_CASE_CODE_STR(IMP_ERR_OPENFAIL, nErrCode, "设备打开失败/未打开")
            PUB_CASE_CODE_STR(IMP_ERR_NODEVICE, nErrCode, "未找到有效设备")
            PUB_CASE_CODE_STR(IMP_ERR_OFFLINE, nErrCode, "设备断线")
            PUB_CASE_CODE_STR(IMP_ERR_NODETECT, nErrCode, "未检测到活体")
            PUB_CASE_CODE_STR(IMP_ERR_INTEREXEC, nErrCode, "接口执行错误")
            PUB_CASE_CODE_STR(IMP_ERR_SET_VMODE, nErrCode, "摄像参数设置错误")
            PUB_CASE_CODE_STR(IMP_ERR_SET_RESO, nErrCode, "分辨率设置失败")
            PUB_CASE_CODE_STR(IMP_ERR_SHAREDMEM_RW, nErrCode, "共享内存读写失败")
            PUB_CASE_CODE_STR(IMP_ERR_MEMORY, nErrCode, "内存错误")
            default :
                sprintf(lpRetStr, "%d|%s", nErrCode, "未知Code");
                break;
        }

        return 0;
    }
};


#endif // DEVIMPL_DEF_H

// -------------------------------------- END --------------------------------------
