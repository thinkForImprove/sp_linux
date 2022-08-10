/***************************************************************
* 文件名称: DevImpl_JDY5001A0809.h
* 文件描述: 封装摄像模块底层指令,提供控制接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
****************************************************************/

#include "DevImpl_JDY5001A0809.h"

// open()必需头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// close()必需头文件
#include <unistd.h>
// ioctl()必需头文件
#include <sys/ioctl.h>

#include <linux/videodev2.h>



static const char *ThisFile = "DevImpl_JDY5001A0809.cpp";

CDevImpl_JDY5001A0809::CDevImpl_JDY5001A0809()
{
    SetLogFile(LOG_NAME, ThisFile);                 // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_JDY5001A0809::CDevImpl_JDY5001A0809(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);                    // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_JDY5001A0809::CDevImpl_JDY5001A0809(LPSTR lpLog, LPCSTR lpDevType)
{
    SetLogFile(lpLog, ThisFile, lpDevType);         // 设置日志文件名和错误发生的文件
    MSET_0(m_szDevType);
    MCPY_NOLEN(m_szDevType, strlen(lpDevType) > 0 ? lpDevType : "JDY-5001A-0809");
    Init();
}

// 参数初始化
void CDevImpl_JDY5001A0809::Init()
{
    MSET_0(m_szDevType);                            // 设备类型
    m_nDevHandle = 0;                               // 设备句柄
    m_bDevOpenOk = FALSE;                           // 设备是否Open
    m_bReCon = FALSE;                               // 是否断线重连状态
    memset(m_nRetErrOLD, 0, sizeof(INT) * 8);
}

CDevImpl_JDY5001A0809::~CDevImpl_JDY5001A0809()
{
    //vUnLoadLibrary();
}

/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::OpenDevice(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    m_nDevHandle = open(lpMode, O_RDWR | O_NONBLOCK);
    if (m_nDevHandle < 0)
    {
        if (m_nRetErrOLD[0] != m_nDevHandle)
        {
            Log(ThisModule, __LINE__,
                "打开设备: ->open(%s) Fail, ErrCode: %d, Return: %s.",
                lpMode, m_nDevHandle, ConvertCode_Impl2Str(IMP_ERR_OPEN_FAIL));
            m_nRetErrOLD[0] = m_nDevHandle;
        }
        return IMP_ERR_OPEN_FAIL;
    }

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "断线重连: 打开设备(%s): Succ.", lpMode);
        m_bReCon = FALSE; // 是否断线重连状态: 初始F
    } else
    {
        Log(ThisModule, __LINE__,  "打开设备(%s) Succ.", lpMode);
    }

    //memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::CloseDevice()
{
    if (m_bDevOpenOk == TRUE)
    {
        if (m_nDevHandle < 0)
        {
            return IMP_SUCCESS;
        }
        close(m_nDevHandle);
    }

    m_bDevOpenOk = FALSE;

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 设备是否Open
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
BOOL CDevImpl_JDY5001A0809::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

/************************************************************
 * 功能: 复位设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::ResetDevice()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 取设备状态
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::GetDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (m_bDevOpenOk == TRUE)
    {
        return DEV_OK;
    } else
    {
        return DEV_NOTOPEN;
    }

    INT nRet = IMP_SUCCESS;
    struct v4l2_capability stDevCaps;
    nRet = ioctl(m_nDevHandle, VIDIOC_QUERYCAP, &stDevCaps);


    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 设置视频捕获模式
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT SetVideoCaptureMode(INT nWidth, INT nHeight)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    INT nWidthTmp = (nWidth / 16 * 16) + ((nWidth % 16) / 2 >= 8 ? 16 : 0);
    INT nHeightTmp = (nHeight / 16 * 16) + ((nHeight % 16) / 2 >= 8 ? 16 : 0);

    struct v4l2_format stVideoFmt;

    stVideoFmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stVideoFmt.fmt.pix.width = nWidthTmp;
    stVideoFmt.fmt.pix.width = nHeightTmp;
    stVideoFmt.

    return IMP_SUCCESS;
}

//----------------------------------对外参数设置接口----------------------------------
// 设置断线重连标记
INT CDevImpl_JDY5001A0809::SetReConFlag(BOOL bFlag)
{
    THISMODULE(__FUNCTION__);

    if (m_bReCon == FALSE && bFlag == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 Start......");
    }
    m_bReCon = bFlag;

    return IMP_SUCCESS;
}

// Impl错误码转换解释字符串
LPSTR CDevImpl_JDY5001A0809::ConvertCode_Impl2Str(INT nErrCode)
{
    return nullptr;
}

