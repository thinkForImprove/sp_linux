/***************************************************************************
* 文件名称: DevCAM_ZLF1000A3.cpp
* 文件描述：摄像功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年6月6日
* 文件版本：1.0.0.1
****************************************************************************/

#include <unistd.h>
#include <qimage.h>
#include <sys/syscall.h>

#include "DevCAM_ZLF1000A3.h"
#include "opencv2/opencv.hpp"
#include "device_port.h"

static const char *ThisFile = "DevCAM_ZLF1000A3.cpp";

/***************************************************************************
*     对外接口调用处理                                                         *
***************************************************************************/
CDevCAM_ZLF1000A3::CDevCAM_ZLF1000A3(LPCSTR lpDevType) :
    m_pDevImpl(LOG_NAME_DEV, lpDevType)
{
     SetLogFile(LOG_NAME_DEV, ThisFile);  // 设置日志文件名和错误发生的文件

     m_stOpenMode.Clear();                           // 设备打开方式
     m_stVideoParam.Clear();                         // 设备摄像模式
     m_bReCon = FALSE;                               // 是否断线重连状态
     memset(m_nRetErrOLD, 0, sizeof(INT) * 12);      // 处理错误值保存
     m_nClipMode = 0;                                // 图像镜像模式转换
     memset(m_nFrameResoWH, 0, sizeof(INT) * 2);     // 截取画面帧的分辨率(0:Width, 1:Height)
     m_stStatusOLD.Clear();                          // 记录上一次状态
     m_nImplStatOLD = 0;                             // 保留上一次获取的Impl状态变化
     m_wDisplayOpenMode = DISP_OPEN_MODE_THREAD;     // Display窗口打开模式(必须设定):线程支持
     memset(m_bDevPortIsHaveOLD, FALSE, sizeof(BOOL) * 2);// 记录设备是否连接上)
}

CDevCAM_ZLF1000A3::~CDevCAM_ZLF1000A3()
{
    Close();
}

// 打开连接
int CDevCAM_ZLF1000A3::Open(LPCSTR lpMode)
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

        m_bDevPortIsHaveOLD[0] = (nDevPort1IsHave == DP_RET_NOTHAVE ? FALSE : TRUE);

        if (nDevPort1IsHave == DP_RET_NOTHAVE)
        {
            if (m_nRetErrOLD[1] != ERR_CAM_NODEVICE)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: 检查设备: VidPid[%s:%s%s], Return: %s.",
                    m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0],
                    nDevPort1IsHave == DP_RET_NOTHAVE ? "未连接" : "已连接",
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

    // 连接共享内存
    nRet = ConnSharedMemory(m_szSharedDataName);
    if (nRet != IMP_SUCCESS)
    {
        if (m_nRetErrOLD[2] != nRet)
        {
            Log(ThisModule, __LINE__,
                "打开设备: 连接共享内存: ->ConnSharedMemory(%s) Fail, ErrCode: %s, Return: %s.",
                m_szSharedDataName,
                nRet == 1 ? "句柄实例化失败" : (nRet == 2 ? "无法连接指定内存" : "连接内存成功但内存大小<1"),
                ConvertDevErrCodeToStr(ERR_CAM_OTHER));
            m_nRetErrOLD[2] = nRet;
        }
        return ERR_CAM_OTHER;
    }

    Log(ThisModule, __LINE__, "%s打开设备: ->OpenDevice(%d, %d) Succ.",
        m_bReCon == TRUE ? "断线重连: " : "", m_stOpenMode.nHidVid[0], m_stOpenMode.nHidVid[1]);
    m_bReCon = FALSE; // 是否断线重连状态: 初始F

    // 枚举当前设备所支持的分辨率
    nGetbEnumReso();

    return CAM_SUCCESS;
}

// 关闭连接
int CDevCAM_ZLF1000A3::Close()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_enDisplayStat = EN_DISP_ENDING;
    DisConnSharedMemory();
    m_pDevImpl.CloseDevice();

    return CAM_SUCCESS;
}

// 设备复位
int CDevCAM_ZLF1000A3::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    m_enDisplayStat = EN_DISP_ENDING;

    // 关闭设备并重新打开
    m_pDevImpl.CloseDevice();
    if ((nRet = m_pDevImpl.OpenDevice()) != IMP_SUCCESS)
    {
        return ConvertImplErrCode2CAM(nRet);
    }

    return CAM_SUCCESS;
}

// 取设备状态
int CDevCAM_ZLF1000A3::GetStatus(STDEVCAMSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    BOOL bDevPort1IsHave = FALSE;   // 设备连接1是否存在

    // 设备数据线连接检查
    if (m_stOpenMode.wOpenMode == 1)
    {
        if (CDevicePort::SearchDeviceVidPidIsHave(m_stOpenMode.szHidVid[0],
                                                  m_stOpenMode.szHidPid[0]) != DP_RET_NOTHAVE)
        {
            bDevPort1IsHave = TRUE;
        }

        if (m_bDevPortIsHaveOLD[0] != bDevPort1IsHave)
        {
            Log(ThisModule, __LINE__, "数据线VID:PID[%s:%s]%s",
                m_stOpenMode.szHidVid[0],m_stOpenMode.szHidPid[0],
                bDevPort1IsHave == TRUE ? "已连接" : "已断开");
            m_bDevPortIsHaveOLD[0] = bDevPort1IsHave;
        }
    }

    if (bDevPort1IsHave == TRUE)     // 正常接入
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

    if (stStatus.wDevice == DEVICE_STAT_ONLINE)
    {
        // 获取设备状态并检查
        INT nStat = m_pDevImpl.GetDevStatus();
        switch(nStat)
        {
            case DEVICE_OK:         // 启动且正在运行
            case DEVICE_OPENED:     // 设备已打开
                stStatus.wDevice   = DEVICE_STAT_ONLINE;
                stStatus.wMedia[0] = CAM_MEDIA_STAT_OK;
                stStatus.fwCameras[0] = CAM_CAMERA_STAT_OK;
                break;
            case DEVICE_NOT_FOUND:   // 未找到设备
                stStatus.wDevice   = DEVICE_STAT_NODEVICE;
                break;
            case DEVICE_CONNECTED:   // 设备已连接未打开
                stStatus.wDevice   = DEVICE_STAT_OFFLINE;
                stStatus.wMedia[0] = CAM_MEDIA_STAT_UNKNOWN;
                stStatus.fwCameras[0] = CAM_CAMERA_STAT_UNKNOWN;
                break;
            default:
                stStatus.wDevice   = DEVICE_STAT_HWERROR;
                stStatus.wMedia[0] = CAM_MEDIA_STAT_UNKNOWN;
                stStatus.fwCameras[0] = CAM_CAMERA_STAT_UNKNOWN;
                break;
        }
    }

    if ((stStatus.wDevice == DEVICE_STAT_NODEVICE || stStatus.wDevice == DEVICE_STAT_OFFLINE) &&
        m_enDisplayStat != EN_DISP_ENDING)
    {
        Log(ThisModule, __LINE__, "设备连接断开: 有窗口显示中, 设置关闭.");
        m_enDisplayStat = EN_DISP_ENDING;
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
int CDevCAM_ZLF1000A3::Cancel(unsigned short usMode)
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
int CDevCAM_ZLF1000A3::Display(STDISPLAYPAR stDisplayIn, void *vParam/* = nullptr*/)
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
int CDevCAM_ZLF1000A3::TakePicture(STTAKEPICTUREPAR stTakePicIn, void *vParam/* = nullptr*/)
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
int CDevCAM_ZLF1000A3::SetData(unsigned short usType, void *vData/* = nullptr*/)
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
                // display采用图像帧方式刷新时,取图像帧数据接口错误次数上限
                m_nDisplayGetVideoMaxErrCnt = ((LPSTINITPARAM)vData)->nParInt[2];
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
                m_nFrameResoWH[0] = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[2];               // 截取画面帧的分辨率(0:Width, 1:Height)
                m_nFrameResoWH[1] = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[3];
                m_nRefreshTime = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[1];                  // 摄像刷新时间
                m_pDevImpl.SetApplyMode(
                            ((LPSTDEVICEOPENMODE)vData)->nOtherParam[10] == 0 ?
                             ZL_APPLYMODE_DOC : ZL_APPLYMODE_PER);                             // 设备指定使用模式
                m_pDevImpl.SetDrawCutRect(
                            ((LPSTDEVICEOPENMODE)vData)->nOtherParam[11] == 0 ? FALSE : TRUE); // 图像帧是否绘制切边区域
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
int CDevCAM_ZLF1000A3::GetData(unsigned short usType, void *vData)
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
            break;
        }
        default:
            break;
    }

    return CAM_SUCCESS;
}

// 版本号(1DevCam版本/2固件版本/3其他)
int CDevCAM_ZLF1000A3::GetVersion(unsigned short usType, char* szVer, int nSize)
{
    switch(usType)
    {
        case GET_VER_FW:            // 固件版本
        {

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

// 枚举当前设备上的相机所支持的分辨率
INT CDevCAM_ZLF1000A3::nGetbEnumReso()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 获取分辨率支持列表
    UINT unResoBuf[30][2];
    INT nResoCnt = m_pDevImpl.GetResolut(unResoBuf);
    if (nResoCnt > 0)
    {
        CHAR szResoPrt[1024] = { 0x00 };
        for (INT i = 0; i < nResoCnt; i ++)
        {
            sprintf(szResoPrt + strlen(szResoPrt), "%d*%d|",
                    unResoBuf[i][0], unResoBuf[i][1]);
        }
        Log(ThisFile, __LINE__, "设备支持的分辨率列表: %s", szResoPrt);
    } else
    {
        Log(ThisFile, __LINE__, "获取分辨率列表: GetResolut() = %d", nResoCnt);
    }

    return TRUE;
}

// Impl错误码转换为CAM错误码
INT CDevCAM_ZLF1000A3::ConvertImplErrCode2CAM(INT nRet)
{
#define CASE_RET_DEVCODE(IMP, RET) \
        case IMP: return RET;

    if (CHK_ERR_ISDEF(nRet) == TRUE)
    {
        return DEF_ConvertImplErrCode2CAM(nRet);
    } else
    {
        ConvertImplErrCode2ErrDetail(nRet);
        switch(nRet)
        {
            CASE_RET_DEVCODE(IMP_ERR_DEV_NOTOPEN, ERR_CAM_NOTOPEN)          // 设备未打开
            CASE_RET_DEVCODE(IMP_ERR_DEV_GETVDATA, ERR_CAM_LOAD_IMAGE)      // 获取视频/图像数据失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_PARAM, ERR_CAM_PARAM_ERR)          // SDK接口参数错误
            default: return ERR_CAM_OTHER;
        }
    }
    return CAM_SUCCESS;
}

// 根据Impl错误码设置错误错误码字符串
INT CDevCAM_ZLF1000A3::ConvertImplErrCode2ErrDetail(INT nRet)
{
#define CASE_SET_DEV_DETAIL(IMP, STR) \
        case IMP: m_clErrorDet.SetDevErrCode((LPSTR)STR); break;

#define CASE_SET_HW_DETAIL(IMP, DSTR, DEVID) \
            m_clErrorDet.SetDevErrCode((LPSTR)DSTR); \
            m_clErrorDet.SetHWErrCodeHex(IMP, DEVID); break;

    switch(nRet)
    {
        case IMP_ERR_DEV_NOTOPEN:       // 设备未打开
            CASE_SET_HW_DETAIL(nRet, EC_DEV_SDKFail, m_stOpenMode.nOtherParam[0])
        case IMP_ERR_DEV_GETVDATA:      // 获取视频/图像数据失败
            CASE_SET_HW_DETAIL(nRet, EC_DEV_SDKFail, m_stOpenMode.nOtherParam[0])
        case IMP_ERR_DEV_PARAM:         // SDK接口参数错误
            CASE_SET_HW_DETAIL(nRet, EC_DEV_SDKFail, m_stOpenMode.nOtherParam[0])
        default:
            CASE_SET_HW_DETAIL(nRet, EC_DEV_SDKFail, m_stOpenMode.nOtherParam[0])
    }

    return CAM_SUCCESS;
}

// 摄像窗口打开前处理(重写)
INT CDevCAM_ZLF1000A3::VideoCameraOpenFrontRun(STDISPLAYPAR stDisplayIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    WORD wWidth = 0;
    WORD wHeight = 0;

    if (stDisplayIn.wHpixel == 0 || stDisplayIn.wVpixel == 0)
    {
        wWidth = stDisplayIn.wWidth;
        wHeight = stDisplayIn.wHeight;
    } else
    {
        wWidth = stDisplayIn.wHpixel;
        wHeight = stDisplayIn.wVpixel;
    }

    // 设置硬件分辨率
    nRet = m_pDevImpl.SetResolut(wWidth, wHeight);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "摄像窗口处理: 建立窗口: 设置摄像窗口图像数据分辨率为 %d*%d 失败, ErrCode: %s.",
            wWidth, wHeight, ConvertDevErrCodeToStr(nRet));
    }

    return CAM_SUCCESS;
}

// 打开设备摄像画面(重写)
INT CDevCAM_ZLF1000A3::VideoCameraOpen(STDISPLAYPAR stDisplayIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    return CAM_SUCCESS;
}

// 关闭设备摄像画面(重写)
INT CDevCAM_ZLF1000A3::VideoCameraClose()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 暂停设备摄像画面(重写)
INT CDevCAM_ZLF1000A3::VideoCameraPause()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 恢复设备摄像画面(重写)
INT CDevCAM_ZLF1000A3::VideoCameraResume()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 获取窗口显示数据(重写)
INT CDevCAM_ZLF1000A3::GetViewImage(LPSTIMGDATA lpImgData, INT nWidth, INT nHeight, DWORD dwParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return m_pDevImpl.GetVideoImage(lpImgData, nWidth, nHeight, dwParam);
}

// 拍照前运行处理(重写)
INT CDevCAM_ZLF1000A3::TakePicFrontRun(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 拍照后运行处理(重写)
INT CDevCAM_ZLF1000A3::TakePicAfterRun(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 获取检测结果(重写)
BOOL CDevCAM_ZLF1000A3::GetLiveDetectResult()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return TRUE;
}

// 保存图像(重写)
INT CDevCAM_ZLF1000A3::TakePicSaveImage(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    nRet = m_pDevImpl.SaveImage(stTakePicIn.szFileName);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisFile, __LINE__, "保存图像: ->SaveImage(%s) Fail, ErrCode: %d, Return: %s",
            stTakePicIn.szFileName, m_pDevImpl.ConvertCode_Impl2Str(nRet),
            ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
        return ConvertImplErrCode2CAM(nRet);
    }

    return CAM_SUCCESS;
}

// -------------------------------------- END --------------------------------------
