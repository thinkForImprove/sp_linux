/***************************************************************************
* 文件名称: DevCAM_TCF261.cpp
* 文件描述：摄像功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年6月6日
* 文件版本：1.0.0.1
****************************************************************************/

#include "DevCAM_TCF261.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>

#include "device_port.h"
#include "file_access.h"

static const char *ThisFile = "DevCAM_TCF261.cpp";

extern CDevImpl_TCF261_Task cTask;

/***************************************************************************
*     对外接口调用处理                                                         *
***************************************************************************/
CDevCAM_TCF261::CDevCAM_TCF261(LPCSTR lpDevType) :
    m_pDevImpl(LOG_NAME_DEV, lpDevType)//,
    //m_clErrorDet((LPSTR)"1")      // 设定CloudWalk类型编号为1
{
    SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件

    m_stOpenMode.Clear();                           // 设备打开方式
    m_stVideoParam.Clear();                         // 设备摄像模式
    m_bReCon = FALSE;                               // 是否断线重连状态
    memset(m_nRetErrOLD, 0, sizeof(INT) * 12);      // 处理错误值保存
    m_nClipMode = 0;                                // 图像镜像模式转换
    memset(m_nFrameResoWH, 0, sizeof(INT) * 2);     // 截取画面帧的分辨率(0:Width, 1:Height)
    MSET_0(m_szPersonImgFile);                      // 特殊处理: 人脸图像名
    MSET_0(m_szPersonNirImgFile);                   // 特殊处理: 人脸红外图像名
    m_stStatusOLD.Clear();                          // 记录上一次状态
    m_nImplStatOLD = 0;                             // 保留上一次获取的Impl状态变化
    m_wDisplayOpenMode = DISP_OPEN_MODE_SDK;        // Display窗口打开模式(必须设定):SDK支持
    cTask.cDevTCF261 = &m_pDevImpl;                 // 全局变量，用于抓拍回调函数对类成员的处理
    memset(m_bDevPortIsHaveOLD, FALSE, sizeof(BOOL) * 2);// 记录设备是否连接上)
}

CDevCAM_TCF261::~CDevCAM_TCF261()
{
    Close();
}

// 打开连接
int CDevCAM_TCF261::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();    // 断线重连时会重复调用,注销避免出现Log冗余
    //AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    INT nDevPort1IsHave = 0;   // 设备连接1是否存在
    INT nDevPort2IsHave = 0;   // 设备连接2是否存在

    m_pDevImpl.CloseDevice();

    // 检查VidPid是否存在
    if (m_stOpenMode.wOpenMode == 1)
    {
        nDevPort1IsHave = CDevicePort::SearchDeviceVidPidIsHave(m_stOpenMode.szHidVid[0],
                                                                m_stOpenMode.szHidPid[0]);
        nDevPort2IsHave = CDevicePort::SearchDeviceVidPidIsHave(m_stOpenMode.szHidVid[1],
                                                                m_stOpenMode.szHidPid[1]);

        m_bDevPortIsHaveOLD[0] = (nDevPort1IsHave == DP_RET_NOTHAVE ? FALSE : TRUE);
        m_bDevPortIsHaveOLD[1] = (nDevPort2IsHave == DP_RET_NOTHAVE ? FALSE : TRUE);

        if (nDevPort1IsHave == DP_RET_NOTHAVE || nDevPort2IsHave == DP_RET_NOTHAVE)
        {
            if (m_nRetErrOLD[1] != ERR_CAM_NODEVICE)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: 检查设备: VidPid[%s:%s%s, %s:%s%s], Return: %s.",
                    m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0],
                    nDevPort1IsHave == DP_RET_NOTHAVE ? "未连接" : "已连接",
                    m_stOpenMode.szHidVid[1], m_stOpenMode.szHidPid[1],
                    nDevPort2IsHave == DP_RET_NOTHAVE ? "未连接" : "已连接",
                    ConvertDevErrCodeToStr(ERR_CAM_NODEVICE));
                m_nRetErrOLD[1] = ERR_CAM_NODEVICE;
            }
            return ERR_CAM_NODEVICE;
        }
    }

    // 打开设备
    if (m_pDevImpl.IsDeviceOpen() != TRUE)
    {
        nRet = m_pDevImpl.OpenDevice();
        if (nRet != IMP_SUCCESS)
        {
            if (m_nRetErrOLD[0] != nRet)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: ->OpenDevice() Fail, ErrCode: %s, Return: %s.",
                    m_pDevImpl.ConvertCode_Impl2Str(nRet),
                    ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
                m_nRetErrOLD[0] = nRet;
            }
            return ConvertImplErrCode2CAM(nRet);
        }
    }

    Log(ThisModule, __LINE__,
        "%s打开设备: ->OpenDevice(%d, %d) Succ.",
        m_bReCon == TRUE ? "断线重连: " : "",
        m_stOpenMode.nHidVid[0], m_stOpenMode.nHidVid[1]);
    m_bReCon = FALSE; // 是否断线重连状态: 初始F

    return CAM_SUCCESS;
}

// 关闭连接
int CDevCAM_TCF261::Close()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_enDisplayStat = EN_DISP_ENDING;
    m_pDevImpl.CloseDevice();

    return CAM_SUCCESS;
}

// 设备复位
int CDevCAM_TCF261::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //INT nRet = IMP_SUCCESS;

    m_enDisplayStat = EN_DISP_ENDING;

    // 停止活检
    m_pDevImpl.StopLiveDetecte();

    // 关闭预览窗口
    //m_pDevImpl.CloseWindow();

    return CAM_SUCCESS;
}

// 取设备状态
int CDevCAM_TCF261::GetStatus(STDEVCAMSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    BOOL bDevPort1IsHave = FALSE;   // 设备连接1是否存在
    BOOL bDevPort2IsHave = FALSE;   // 设备连接2是否存在

    if (m_stOpenMode.wOpenMode == 1)
    {
        if (CDevicePort::SearchDeviceVidPidIsHave(m_stOpenMode.szHidVid[0],
                                                  m_stOpenMode.szHidPid[0]) != DP_RET_NOTHAVE)
        {
            bDevPort1IsHave = TRUE;
        }
        if (CDevicePort::SearchDeviceVidPidIsHave(m_stOpenMode.szHidVid[1],
                                                  m_stOpenMode.szHidPid[1]) != DP_RET_NOTHAVE)
        {
            bDevPort2IsHave = TRUE;
        }

        if (m_bDevPortIsHaveOLD[0] != bDevPort1IsHave)
        {
            Log(ThisModule, __LINE__, "数据线VID:PID[%s:%s]%s",
                m_stOpenMode.szHidVid[0],m_stOpenMode.szHidPid[0],
                bDevPort1IsHave == TRUE ? "已连接" : "已断开");
            m_bDevPortIsHaveOLD[0] = bDevPort1IsHave;
        }

        if (m_bDevPortIsHaveOLD[1] != bDevPort2IsHave)
        {
            Log(ThisModule, __LINE__, "数据线VID:PID[%s:%s]%s",
                m_stOpenMode.szHidVid[1],m_stOpenMode.szHidPid[1],
                bDevPort2IsHave == TRUE ? "已连接" : "已断开");
            m_bDevPortIsHaveOLD[1] = bDevPort2IsHave;
        }

        if (bDevPort1IsHave == TRUE && bDevPort2IsHave == TRUE)     // 两根线正常接入
        {
            if (m_pDevImpl.IsDeviceOpen() != TRUE)  // 设备未Open
            {
                stStatus.wDevice = DEVICE_STAT_OFFLINE;
                stStatus.wMedia[0] = CAM_MEDIA_STAT_UNKNOWN;
                stStatus.fwCameras[0] = CAM_CAMERA_STAT_UNKNOWN;
            } else
            {
                stStatus.wDevice = DEVICE_STAT_ONLINE;
                stStatus.wMedia[0] = CAM_MEDIA_STAT_OK;
                stStatus.fwCameras[0] = CAM_CAMERA_STAT_OK;
            }
        } else
        {
            if (bDevPort1IsHave != TRUE || bDevPort2IsHave != TRUE)     // 两根线有一根线拔出
            {
                if (m_pDevImpl.IsDeviceOpen() != TRUE)  // 设备未Open
                {
                    stStatus.wDevice = DEVICE_STAT_NODEVICE;
                    stStatus.wMedia[0] = CAM_MEDIA_STAT_UNKNOWN;
                    stStatus.fwCameras[0] = CAM_CAMERA_STAT_UNKNOWN;
                } else
                {
                    stStatus.wDevice = DEVICE_STAT_OFFLINE;
                    stStatus.wMedia[0] = CAM_MEDIA_STAT_OK;
                    stStatus.fwCameras[0] = CAM_CAMERA_STAT_OK;
                }
            }
        }
    }

    if (stStatus.wDevice == DEVICE_STAT_ONLINE)
    {
        // 通过设备接口获取设备状态
        INT nStat = m_pDevImpl.GetStatus();
        if (m_nImplStatOLD != nStat)
        {
            Log(ThisModule, __LINE__, "设备状态接口获取状态值有变化: %d->%d", m_nImplStatOLD, nStat);
            m_nImplStatOLD = nStat;
        }

        // 设备状态分析处理
        switch(nStat)
        {
            case FR_RET_SUCC:   // 成功
            case FR_RET_ARDY:   // 已经打开设备
                stStatus.wDevice = DEVICE_STAT_ONLINE;
                stStatus.wMedia[0] = CAM_MEDIA_STAT_OK;
                stStatus.fwCameras[0] = CAM_CAMERA_STAT_OK;
                break;
            case FR_RET_BUSY:   // 设备繁忙
                stStatus.wDevice = DEVICE_STAT_BUSY;
                stStatus.wMedia[0] = CAM_MEDIA_STAT_OK;
                stStatus.fwCameras[0] = CAM_CAMERA_STAT_OK;
                break;
            case FR_RET_NDEV:   // 打开设备失败
            case FR_RET_NINI:   // 未打开设备
            case FR_RET_NLNK:   // 设备断开
                stStatus.wDevice = DEVICE_STAT_OFFLINE;
                stStatus.wMedia[0] = CAM_MEDIA_STAT_UNKNOWN;
                stStatus.fwCameras[0] = CAM_CAMERA_STAT_UNKNOWN;
                break;
            default:
                stStatus.wDevice = DEVICE_STAT_HWERROR;
                stStatus.wMedia[0] = CAM_MEDIA_STAT_UNKNOWN;
                stStatus.fwCameras[0] = CAM_CAMERA_STAT_UNKNOWN;
                break;
        }
    }

    // 连接断开时, 有窗口显示要关闭
    if ((stStatus.wDevice == DEVICE_STAT_NODEVICE || stStatus.wDevice == DEVICE_STAT_OFFLINE) &&
        m_enDisplayStat != EN_DISP_ENDING)
    {
        Log(ThisModule, __LINE__, "设备连接断开: 有窗口显示中, 设置关闭.");
        m_enDisplayStat = EN_DISP_ENDING;
        m_pDevImpl.StopLiveDetecte();
        m_pDevImpl.CloseWindow();
    }

    // 记录设备状态变化
    if (m_stStatusOLD.Diff(stStatus) == TRUE)
    {
        Log(ThisModule, __LINE__, "设备状态有变化: Device:%d->%d%s|Media:%d->%d%s|Cameras:%d->%d%s",
            m_stStatusOLD.wDevice, stStatus.wDevice, (m_stStatusOLD.wDevice != stStatus.wDevice ? " *" : ""),
            m_stStatusOLD.wMedia[0], stStatus.wMedia[0], (m_stStatusOLD.wMedia[0] != stStatus.wMedia[0] ? " *" : ""),
            m_stStatusOLD.fwCameras[0], stStatus.fwCameras[0], (m_stStatusOLD.fwCameras[0] != stStatus.fwCameras[0] ? " *" : ""));
        m_stStatusOLD.Copy(stStatus);
    }

    // 摄像窗口只在状态连接正常情况下上报错误事件
    // 状态连接异常有统一上报方式
    if (m_nSaveStat[0] == 1)
    {
        if (stStatus.wDevice == DEVICE_STAT_ONLINE)
        {
            stStatus.nOtherCode[0] = m_nSaveStat[0];
            stStatus.nOtherCode[1] = m_nSaveStat[1];
            stStatus.nOtherCode[2] = m_nSaveStat[2];
        }
        MSET_XS(m_nSaveStat, 0, sizeof(INT) * 12);
    }

    return CAM_SUCCESS;
}

// 命令取消
int CDevCAM_TCF261::Cancel(unsigned short usMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (usMode == 0)    // 取消拍照
    {
        Log(ThisModule, __LINE__, "设置取消标记: usMode = %d.", usMode);
        m_bCancel = TRUE;
    }

    return CAM_SUCCESS;
}

// 摄像窗口处理
int CDevCAM_TCF261::Display(STDISPLAYPAR stDisplayIn, void *vParam/* = nullptr*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    nRet = InnerDisplay(stDisplayIn, vParam);
    if (nRet != CAM_SUCCESS)
    {
        return nRet;
    }

    return CAM_SUCCESS;
}

// 拍照
int CDevCAM_TCF261::TakePicture(STTAKEPICTUREPAR stTakePicIn, void *vParam/* = nullptr*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    nRet = InnerTakePicture(stTakePicIn, vParam);
    if (nRet != CAM_SUCCESS)
    {
        return nRet;
    }

    return CAM_SUCCESS;
}

// 设置数据
int CDevCAM_TCF261::SetData(unsigned short usType, void *vData/* = nullptr*/)
{
    THISMODULE(__FUNCTION__);

    switch(usType)
    {
        case SET_DEV_INIT:              // 设置初始化数据
        {
            if (vData != nullptr)
            {
                MCPY_NOLEN(m_szSharedDataName, ((LPSTINITPARAM)vData)->szParStr[0]);
                m_ulSharedDataSize = ((LPSTINITPARAM)vData)->lParLong[0];
                MCPY_NOLEN(m_szPersonImgFile, ((LPSTINITPARAM)vData)->szParStr[10]);    // 特殊处理: 人脸图像名
                MCPY_NOLEN(m_szPersonNirImgFile, ((LPSTINITPARAM)vData)->szParStr[11]); // 特殊处理: 人脸红外图像名
            }
            break;
        }
        case SET_LIB_PATH:          // 设置动态库路径
        {
            if (vData != nullptr)
            {
                m_pDevImpl.SetLibPath((LPCSTR)vData);
            }
            break;
        }
        case SET_DEV_RECON:             // 设置断线重连标记为T
        {
            return m_pDevImpl.SetReConFlag(TRUE);
        }
        case SET_DEV_OPENMODE:          // 设置设备打开模式
        {
            if (vData != nullptr)
            {
                memcpy(&(m_stOpenMode), ((LPSTDEVICEOPENMODE)vData), sizeof(STDEVICEOPENMODE));
                // 解析设备检测参数
                m_nFrameResoWH[0] = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[2];               // 截取画面帧的分辨率(0:Width, 1:Height)
                m_nFrameResoWH[1] = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[3];
                m_nRefreshTime = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[1];                  // 摄像刷新时间
            }
            break;
        }
        case SET_DEV_VIDEOMODE:         // 设置设备摄像模式
        {
            break;
        }
        default:
            break;
    }

    return CAM_SUCCESS;
}

// 获取数据
int CDevCAM_TCF261::GetData(unsigned short usType, void *vData)
{
    switch(usType)
    {
        case GET_DEV_ERRCODE:       // 取DevXXX错误码
        {
            if (vData != nullptr)
            {
                m_clErrorDet.GetSPErrCode((LPSTR)vData);
            }
            break;
        }
        case GET_DEV_LIVESTAT:      // 取活检状态
        {
            return ConvertImplLiveErr2Dev(m_pDevImpl.GetLiveErrCode());
        }
        default:
            break;
    }

    return CAM_SUCCESS;
}

// 版本号(1DevCam版本/2固件版本/3其他)
int CDevCAM_TCF261::GetVersion(unsigned short usType, char* szVer, int nSize)
{

    switch(usType)
    {
        case GET_VER_FW:            // 固件版本
        {
            if (m_pDevImpl.IsDeviceOpen() == TRUE)
            {
                m_pDevImpl.GetFirmwareVersion(szVer, (WORD)nSize);
            }
            break;
        }
        case GET_VER_SOFT:          // 软件版本
        {
            break;
        }
        case GET_VER_LIB:           // SDK动态库版本
        {
            break;
        }
        case GET_VER_SERIAL:        // 序列号
        {
            break;
        }
        case GET_VER_PN:            // 设备编号
        {
            break;
        }
        case GET_VER_SN:            // 设备出厂编号
        {
            break;
        }
        default:
            break;
    }

    return CAM_SUCCESS;
}

// Impl错误码转换为CAM错误码
INT CDevCAM_TCF261::ConvertImplErrCode2CAM(INT nRet)
{
#define CASE_RET_DEVCODE(IMP, RET) \
        case IMP: return RET;

    if (CHK_ERR_ISDEF(nRet) == TRUE)
    {
        return DEF_ConvertImplErrCode2CAM(nRet);
    } else
    {
        ConvertImplErrCode2ErrDetail(nRet + IMP_ERR_DEF_NUM);
        switch(nRet)
        {
            CASE_RET_DEVCODE(IMP_ERR_DEV_SUCCESS, CAM_SUCCESS)              // 成功
            CASE_RET_DEVCODE(IMP_ERR_DEV_RUNFAIL, ERR_CAM_OTHER)            // 执行失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_PARAM, ERR_CAM_PARAM_ERR)          // 参数错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_OPENFAIL, ERR_CAM_OPENFAIL)        // 打开设备失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_NOTOPEN, ERR_CAM_NOTOPEN)          // 未打开设备
            CASE_RET_DEVCODE(IMP_ERR_DEV_BUSY, ERR_CAM_OTHER)               // 设备繁忙
            CASE_RET_DEVCODE(IMP_ERR_DEV_DATA, ERR_CAM_OTHER)               // 图像数据不正确
            CASE_RET_DEVCODE(IMP_ERR_DEV_OFFLINE, ERR_CAM_OFFLINE)          // 设备断开
            CASE_RET_DEVCODE(IMP_ERR_DEV_LOADDLL, ERR_CAM_LIBRARY)          // 加载设备库失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_LOADALGO, ERR_CAM_LIBRARY)         // 加载算法库失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_MEMORY, ERR_CAM_OTHER)             // 内存分配失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_ISOPEN, ERR_CAM_OTHER)             // 已经打开设备
            CASE_RET_DEVCODE(IMP_ERR_DEV_AUTH, ERR_CAM_OTHER)               // 算法授权失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_IPC, ERR_CAM_OTHER)                // IPC错误
            default: return ERR_CAM_OTHER;
        }
    }
    return CAM_SUCCESS;
}

// 根据Impl错误码设置错误错误码字符串
INT CDevCAM_TCF261::ConvertImplErrCode2ErrDetail(INT nRet)
{
#define CASE_SET_DEV_DETAIL(IMP, STR) \
        case IMP: m_clErrorDet.SetDevErrCode((LPSTR)STR); break;

#define CASE_SET_HW_DETAIL(IMP, DSTR, DEVID) \
            m_clErrorDet.SetDevErrCode((LPSTR)DSTR); \
            m_clErrorDet.SetHWErrCodeStr(IMP, DEVID); break;
    CHAR szBuff[8] = { 0x00 };

    switch(nRet)
    {
        case IMP_ERR_DEV_SUCCESS:   // 成功
            break;
        default:
        {
            sprintf(szBuff, "F%06d", nRet);
            CASE_SET_HW_DETAIL(szBuff, EC_DEV_HWErr, m_stOpenMode.nOtherParam[0])
        }
    }

    return CAM_SUCCESS;
}

// Impl错误码转换为CAM活检状态码
INT CDevCAM_TCF261::ConvertImplLiveErr2Dev(INT nRet)
{
    switch(nRet) // 活检图像检查错误记录到全局变量
    {
        case FR_CALLBACK_RESULT_NOFACE:         // 没有检测到人脸
            return LIVE_ERR_VIS_NOFACE;             // 可见光未检测到人脸
        case FR_CALLBACK_RESULT_HEADPOS:        // 头部姿态不正常
            return LIVE_ERR_FACE_ANGLE_FAIL;        // 人脸角度不满足要求(低头/抬头/侧脸等)
        case FR_CALLBACK_RESULT_MOTIVE:         // 有运动模糊
            return LIVE_ERR_FACE_SHAKE;             // 检测到人脸晃动/模糊
        case FR_CALLBACK_RESULT_EMOTION:        // 有闭眼张嘴等表情动作
            return LIVE_ERR_FACE_EYECLOSE;          // 检测到闭眼
        case FR_CALLBACK_RESULT_MULTIFADE:      // 检测到多个人脸
            return LIVE_ERR_FACE_MULTIPLE;          // 检测到多张人脸
        case FR_CALLBACK_RESULT_BRIGHT:         // 亮度不符合
        case FR_CALLBACK_RESULT_NOTCENTER:      // 人脸未居中
        case FR_CALLBACK_RESULT_UNKNOW:         // 未知错误
        case FR_CALLBACK_RESULT_FAIL:           // 活体检测失败
        case FR_CALLBACK_RESULT_TIMEOUT:        // 活体检测超时
        case FR_CALLBACK_RESULT_ERROR:          // 从设备取图失败
        case FR_CALLBACK_RESULT_CANCEL:         // 活体检测取消
            return LIVE_ERR_UNKNOWN;                // 其他/未知错误
        default:
            return LIVE_ERR_UNKNOWN;                // 其他/未知错误
    }
}

// 摄像窗口打开前处理(重写)
INT CDevCAM_TCF261::VideoCameraOpenFrontRun(STDISPLAYPAR stDisplayIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 打开设备摄像画面(重写)
INT CDevCAM_TCF261::VideoCameraOpen(STDISPLAYPAR stDisplayIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    nRet = m_pDevImpl.CreateWindow(stDisplayIn.hWnd, stDisplayIn.wX, stDisplayIn.wY,
                                   stDisplayIn.wWidth, stDisplayIn.wHeight);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "打开设备摄像画面: ->CreateWindow(%d, %d, %d, %d, %d) Fail, ErrCode: %s, Return: %s.",
            stDisplayIn.hWnd, stDisplayIn.wX, stDisplayIn.wY, stDisplayIn.wWidth,
            stDisplayIn.wHeight, m_pDevImpl.ConvertCode_Impl2Str(nRet),
            ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
        return ConvertImplErrCode2CAM(nRet);
    }

    m_enDisplayStat = EN_DISP_SHOWING;

    return CAM_SUCCESS;
}

// 关闭设备摄像画面(重写)
INT CDevCAM_TCF261::VideoCameraClose()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 停止连续活体检测
    m_pDevImpl.StopLiveDetecte();

    // 关闭所有相机
    nRet = m_pDevImpl.CloseWindow();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "关闭设备摄像画面: ->CloseWindow() Fail, ErrCode: %s, Return: %s.",
            m_pDevImpl.ConvertCode_Impl2Str(nRet),
            ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
        return ConvertImplErrCode2CAM(nRet);
    }

    m_enDisplayStat = EN_DISP_ENDING;

    return CAM_SUCCESS;
}

// 暂停设备摄像画面(重写)
INT CDevCAM_TCF261::VideoCameraPause()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 暂停设备摄像画面
    nRet = m_pDevImpl.PauseAndPlaya(TRUE);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "暂停设备摄像画面: ->PauseAndPlaya(TRUE) Fail, ErrCode: %s, Return: %s.",
            m_pDevImpl.ConvertCode_Impl2Str(nRet),
            ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
        return ConvertImplErrCode2CAM(nRet);
    }

    m_enDisplayStat = EN_DISP_PAUSE;

    return CAM_SUCCESS;
}

// 恢复设备摄像画面(重写)
INT CDevCAM_TCF261::VideoCameraResume()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 恢复设备摄像画面
    nRet = m_pDevImpl.PauseAndPlaya(FALSE);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "恢复设备摄像画面: ->PauseAndPlaya(FALSE) Fail, ErrCode: %s, Return: %s.",
            m_pDevImpl.ConvertCode_Impl2Str(nRet),
            ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
        return ConvertImplErrCode2CAM(nRet);
    }

    m_enDisplayStat = EN_DISP_SHOWING;

    return CAM_SUCCESS;
}

// 获取窗口显示数据(重写)
INT CDevCAM_TCF261::GetViewImage(LPSTIMGDATA lpImgData, INT nWidth, INT nHeight, DWORD dwParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 拍照前运行处理(重写)
INT CDevCAM_TCF261::TakePicFrontRun(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 活检前确认
    m_pDevImpl.StopLiveDetecte();

    // 开始活体检测
    nRet = m_pDevImpl.StartLiveDetecte(stTakePicIn.wCameraAction == DEV_CAM_MODE_PERSON ? FALSE : TRUE, "",
                                       stTakePicIn.szFileName);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "开启活体检测: ->StartLiveDetecte(%d, %s, %s) Fail, ErrCode: %s, Return: %s.",
            stTakePicIn.wCameraAction == DEV_CAM_MODE_PERSON ? FALSE : TRUE, "",
            stTakePicIn.szFileName, m_pDevImpl.ConvertCode_Impl2Str(nRet),
            ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
        return ConvertImplErrCode2CAM(nRet);
    }

    return CAM_SUCCESS;
}

// 拍照后运行处理(重写)
INT CDevCAM_TCF261::TakePicAfterRun(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 终止活体检测
    m_pDevImpl.StopLiveDetecte();

    return CAM_SUCCESS;
}

// 获取检测结果(重写)
BOOL CDevCAM_TCF261::GetLiveDetectResult()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return m_pDevImpl.GetIsLiveSucc();
}

// 保存图像(重写)
INT CDevCAM_TCF261::TakePicSaveImage(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    /*if (stTakePicIn.wCameraAction == DEV_CAM_MODE_ROOM)    // 全景摄像
    {
       if (strlen(m_szPersonImgFile) > 0)  // 特殊处理,需要生成人脸图
        {
            bIsPersonImg = TRUE;
            CHAR szFileTmp[4][256] = { 0x00 };
            FileDir::_splitpath(stTakePicIn.szFileName, szFileTmp[0], szFileTmp[1], szFileTmp[2], szFileTmp[3]);
            sprintf(szPersonFile, "%s/%s%s", szFileTmp[1], m_szPersonImgFile, szFileTmp[3]);
        }
    }*/

    // 入参生成全景或人脸图像时,允许生成人脸红外图像
    /*if ((stTakePicIn.wCameraAction == DEV_CAM_MODE_ROOM ||      // 全景摄像
         stTakePicIn.wCameraAction == DEV_CAM_MODE_PERSON) &&   // 人脸摄像
        strlen(m_szPersonNirImgFile) > 0)                       // 需要生成红外图像
    {
        Log(ThisModule, __LINE__, "活体成功后保存图像: 当前SDK版本不支持[获取人脸红外数据]功能,不生成红外图像");
    }

    if (bRet == FALSE)
    {
        return ERR_CAM_SAVE_IMAGE;
    }*/

    return CAM_SUCCESS;
}

// -------------------------------------- END --------------------------------------
