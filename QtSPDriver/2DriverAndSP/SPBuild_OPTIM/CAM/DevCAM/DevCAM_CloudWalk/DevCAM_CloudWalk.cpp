/***************************************************************************
* 文件名称: DevCAM_CloudWalk.cpp
* 文件描述：摄像功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年6月6日
* 文件版本：1.0.0.1
****************************************************************************/

#include <unistd.h>

#include "DevCAM_CloudWalk.h"
#include "device_port.h"
#include "file_access.h"

static const char *ThisFile = "DevCAM_CloudWalk.cpp";

/***************************************************************************
*     对外接口调用处理                                                         *
***************************************************************************/
CDevCAM_CloudWalk::CDevCAM_CloudWalk(LPCSTR lpDevType) :
    m_pDevImpl(LOG_NAME_DEV, lpDevType)//,
    //m_clErrorDet((LPSTR)"1")      // 设定CloudWalk类型编号为1
{
    SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件

    m_stOpenMode.Clear();                   // 设备打开方式
    m_stVideoParam.Clear();                 // 设备摄像模式
    //m_stDetectInitPar;                    // 设备Open参数
    //CErrorDetail                    m_clErrorDet;                   // 错误码处理类实例
    m_bReCon = FALSE;                       // 是否断线重连状态
    m_nDevStatOLD = 0;                      // 保留上一次状态变化
    MSET_XS(m_nRetErrOLD, 0, sizeof(INT) * 12);// 处理错误值保存
    m_nClipMode = 0;                        // 图像镜像模式转换
    m_stDisplayReso.width = 0;              // Display命令指定分辨率
    m_stDisplayReso.height = 0;             // Display命令指定分辨率
    m_wVoiceSup = 0;                        // 是否支持语音提示
    MSET_0(m_szVoiceFile);                  // 语音文件路径
    MSET_XS(&m_nFrameResoWH, 0, sizeof(INT) * 2);              // 截取画面帧的分辨率(0:Width, 1:Height)
    MSET_0(m_szPersonImgFile);              // 特殊处理: 人脸图像名
    MSET_0(m_szPersonNirImgFile);           // 特殊处理: 人脸红外图像名
    m_stStatusOLD.Clear();                  // 记录上一次状态
}

CDevCAM_CloudWalk::~CDevCAM_CloudWalk()
{
    Close();
}

// 打开连接
int CDevCAM_CloudWalk::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();    // 断线重连时会重复调用,注销避免出现Log冗余
    //AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    m_pDevImpl.CloseDevice();

    if (m_stOpenMode.wOpenMode == 0)    // 序号方式打开
    {
        if (CDevicePort::SearchVideoXIsHave(m_stOpenMode.nHidVid[0]) == DP_RET_NOTHAVE ||
            CDevicePort::SearchVideoXIsHave(m_stOpenMode.nHidVid[1]) == DP_RET_NOTHAVE)
        {
            if (m_nRetErrOLD[3] != ERR_CAM_NODEVICE)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: 检查设备: 序号方式打开: 序号[%d, %d] 未找到, Return: %s.",
                    m_stOpenMode.nHidVid[0], m_stOpenMode.nHidVid[1],
                    ConvertDevErrCodeToStr(ERR_CAM_NODEVICE));
                m_nRetErrOLD[3] = ERR_CAM_NODEVICE;
            }
            return ERR_CAM_NODEVICE;
        }
    } else                              // VidPid方式打开
    {
        if (CDevicePort::SearchDeviceVidPidIsHave(m_stOpenMode.szHidVid[0],
                                                  m_stOpenMode.szHidPid[0]) == DP_RET_NOTHAVE ||
            CDevicePort::SearchDeviceVidPidIsHave(m_stOpenMode.szHidVid[1],
                                                  m_stOpenMode.szHidPid[1]) == DP_RET_NOTHAVE)
        {
            if (m_nRetErrOLD[3] != ERR_CAM_NODEVICE)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: 检查设备: VidPid方式打开: [%s:%s, %s,%s] 未找到, Return: %s.",
                    m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0],
                    m_stOpenMode.szHidVid[1], m_stOpenMode.szHidPid[1],
                    ConvertDevErrCodeToStr(ERR_CAM_NODEVICE));
                m_nRetErrOLD[3] = ERR_CAM_NODEVICE;
            }
            return ERR_CAM_NODEVICE;
        }
    }

    // 连接共享内存
    nRet = ConnSharedMemory(m_szSharedDataName);
    if (nRet != IMP_SUCCESS)
    {
        if (m_nRetErrOLD[1] != nRet)
        {
            Log(ThisModule, __LINE__,
                "打开设备: 连接共享内存: ->ConnSharedMemory(%s) Fail, ErrCode: %s, Return: %s.",
                m_szSharedDataName,
                nRet == 1 ? "句柄实例化失败" : (nRet == 2 ? "无法连接指定内存" : "连接内存成功但内存大小<1"),
                ConvertDevErrCodeToStr(ERR_CAM_OTHER));
            m_nRetErrOLD[1] = nRet;
        }
        return ERR_CAM_OTHER;
    }

    // 打开设备
    if (m_pDevImpl.IsDeviceOpen() != TRUE)
    {
        nRet = m_pDevImpl.OpenDevice(m_stDetectInitPar);
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

    if (m_stOpenMode.wOpenMode == 0)
    {
        Log(ThisModule, __LINE__,
            "打开设备(序号方式): ->OpenDevice(%d, %d) Succ.",
            m_stOpenMode.nHidVid[0], m_stOpenMode.nHidVid[1],
            m_bReCon == TRUE ? "断线重连: " : "");
    } else
    {
        Log(ThisModule, __LINE__,
            "打开设备(VidPid方式): ->OpenDevice(%s:%s, %s:%s) Succ.",
            m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0],
            m_stOpenMode.szHidVid[1], m_stOpenMode.szHidPid[1],
            m_bReCon == TRUE ? "断线重连: " : "");
    }

    m_bReCon = FALSE; // 是否断线重连状态: 初始F

    // 设置进行人脸追踪显示, 返回值不处理
    m_pDevImpl.EnableFaceTrace(TRUE);

    // 语音提示开启
    if (m_wVoiceSup != 0 && strlen(m_szVoiceFile) > 0)
    {
        m_pDevImpl.EnableVoiceTip(TRUE, m_szVoiceFile);
    } else
    {
        m_pDevImpl.EnableVoiceTip(FALSE, nullptr);
    }

    // 枚举当前设备上的相机所支持的分辨率记录Log, 返回值不处理
    nGetbEnumReso();

    return CAM_SUCCESS;
}

// 关闭连接
int CDevCAM_CloudWalk::Close()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_enDisplayStat = EN_DISP_ENDING;
    DisConnSharedMemory();
    m_pDevImpl.CloseDevice();

    return CAM_SUCCESS;
}

// 设备复位
int CDevCAM_CloudWalk::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //INT nRet = IMP_SUCCESS;

    m_enDisplayStat = EN_DISP_ENDING;

    // 关闭所有相机
    m_pDevImpl.CloseAllCameras();

    // 停止连续活体检测
    m_pDevImpl.StopLiveDetect();

    return CAM_SUCCESS;
}

// 取设备状态
int CDevCAM_CloudWalk::GetStatus(STDEVCAMSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    BOOL bDevPort1IsHave = FALSE;   // 设备连接1是否存在
    BOOL bDevPort2IsHave = FALSE;   // 设备连接2是否存在

    if (m_stOpenMode.wOpenMode == 0)    // 序号方式打开
    {
        if (CDevicePort::SearchVideoXIsHave(m_stOpenMode.nHidVid[0]) != DP_RET_NOTHAVE)
        {
            bDevPort1IsHave = TRUE;
        }
        if (CDevicePort::SearchVideoXIsHave(m_stOpenMode.nHidVid[1]) != DP_RET_NOTHAVE)
        {
            bDevPort2IsHave = TRUE;
        }
    } else                              // VidPid方式打开
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
int CDevCAM_CloudWalk::Cancel(unsigned short usMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (usMode == 0)    // 取消拍照
    {
        Log(ThisModule, __LINE__, "设置取消标记: usMode = %d.", usMode);
        m_bCancel = TRUE;
    }

    m_bCancel = TRUE;
}

// 摄像窗口处理
int CDevCAM_CloudWalk::Display(STDISPLAYPAR stDisplayIn, void *vParam/* = nullptr*/)
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
int CDevCAM_CloudWalk::TakePicture(STTAKEPICTUREPAR stTakePicIn, void *vParam/* = nullptr*/)
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
int CDevCAM_CloudWalk::SetData(unsigned short usType, void *vData/* = nullptr*/)
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
                m_stDetectInitPar.nModelMode = (SHORT)(((LPSTDEVICEOPENMODE)vData)->nOtherParam[10]);                   // 模型加载方式
                m_stDetectInitPar.nLivenessMode = (SHORT)(((LPSTDEVICEOPENMODE)vData)->nOtherParam[11]);                // 活体类型
                m_stDetectInitPar.nLicenseType = (SHORT)(((LPSTDEVICEOPENMODE)vData)->nOtherParam[12]);                 // 授权类型
                m_stDetectInitPar.nGpu = (SHORT)(((LPSTDEVICEOPENMODE)vData)->nOtherParam[13]);                         // 是否使用GPU
                m_stDetectInitPar.nMultiThread = (SHORT)(((LPSTDEVICEOPENMODE)vData)->nOtherParam[14]);                 // 是否多线程检测
                MCPY_NOLEN(m_stDetectInitPar.szConfigFile, ((LPSTDEVICEOPENMODE)vData)->szOtherParams[10]);             // 算法矩阵文件
                MCPY_NOLEN(m_stDetectInitPar.szFaceDetectFile, ((LPSTDEVICEOPENMODE)vData)->szOtherParams[11]);         // 人脸检测模型
                MCPY_NOLEN(m_stDetectInitPar.szKeyPointDetectFile, ((LPSTDEVICEOPENMODE)vData)->szOtherParams[12]);     // 关键点检测模型
                MCPY_NOLEN(m_stDetectInitPar.szKeyPointTrackFile, ((LPSTDEVICEOPENMODE)vData)->szOtherParams[13]);      // 关键点跟踪模型
                MCPY_NOLEN(m_stDetectInitPar.szFaceQualityFile, ((LPSTDEVICEOPENMODE)vData)->szOtherParams[14]);        // 人脸质量评估模型
                MCPY_NOLEN(m_stDetectInitPar.szFaceLivenessFile, ((LPSTDEVICEOPENMODE)vData)->szOtherParams[15]);       // 活体模型
                MCPY_NOLEN(m_stDetectInitPar.szFaceKeyPointTrackFile, ((LPSTDEVICEOPENMODE)vData)->szOtherParams[16]);  // 人脸检测模型

                m_wVoiceSup = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[15];                     // 是否支持语音提示
                if (m_wVoiceSup > 0)
                {
                    MCPY_NOLEN(m_szVoiceFile, ((LPSTDEVICEOPENMODE)vData)->szOtherParams[16]);  // 语音文件路径
                }
                m_nFrameResoWH[0] = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[16];               // 截取画面帧的分辨率(0:Width, 1:Height)
                m_nFrameResoWH[1] = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[17];
                m_pDevImpl.SetSDKVersion(((LPSTDEVICEOPENMODE)vData)->nOtherParam[18]);         // 设置SDK版本
                m_nRefreshTime = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[19];                  // 摄像刷新时间
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
int CDevCAM_CloudWalk::GetData(unsigned short usType, void *vData)
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
int CDevCAM_CloudWalk::GetVersion(unsigned short usType, char* szVer, int nSize)
{

    switch(usType)
    {
        case GET_VER_FW:            // 固件版本
        {
            if (m_stOpenMode.wOpenMode == 0)    // 序号方式打开
            {
                if (m_pDevImpl.GetFirmwareVersion(m_stOpenMode.nHidVid[0], szVer + strlen(szVer)) == IMP_SUCCESS)
                {
                    if (strlen(szVer) > 0)
                    {
                        sprintf(szVer, "%s", " ; ");
                    }
                }

                m_pDevImpl.GetFirmwareVersion(m_stOpenMode.nHidVid[1], szVer + strlen(szVer));
            }

            if (m_stOpenMode.wOpenMode == 1)    // VidPid方式打开
            {
                if (m_pDevImpl.GetFirmwareVersionEx(m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0], szVer + strlen(szVer)) == IMP_SUCCESS)
                {
                    if (strlen(szVer) > 0)
                    {
                        sprintf(szVer, "%s", " ; ");
                    }
                }
                m_pDevImpl.GetFirmwareVersionEx(m_stOpenMode.szHidVid[1], m_stOpenMode.szHidPid[1], szVer + strlen(szVer));
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

// 枚举当前设备上的相机所支持的分辨率
INT CDevCAM_CloudWalk::nGetbEnumReso()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CWResolution *stResoLists = nullptr, *stResoListTmp = nullptr;
    std::string  stdReso = "";
    if (m_stOpenMode.wOpenMode == 0)    // 序号打开方式
    {
        // 按序号枚举当前设备上的相机所支持的分辨率
        if (m_pDevImpl.EnumResolutions(m_stOpenMode.nHidVid[0], &stResoLists) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "按序号[%d]枚举当前设备上的相机所支持的分辨率失败, 不记录Log", m_stOpenMode.nHidVid[0]);
            return ERR_CAM_OTHER;
        }

        stResoListTmp = stResoLists;
        while(stResoListTmp != nullptr)
        {
            stdReso.append("W:");
            stdReso.append(std::to_string(stResoListTmp->width));
            stdReso.append(" H:");
            stdReso.append(std::to_string(stResoListTmp->height));
            stdReso.append("|");
            stResoListTmp = stResoListTmp->next;
        }
        Log(ThisModule, __LINE__, "按序号[%d]枚举当前设备上的相机所支持的分辨率如下: %s",
            m_stOpenMode.nHidVid[0], stdReso.c_str());
    } else
    {
        // 按VID/PID枚举当前设备上的相机所支持的分辨率
        CWResolution *stResoLists = nullptr, *stResoListTmp = nullptr;
        if (m_pDevImpl.EnumResolutionsEx(m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0],
                                         &stResoLists)!= IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "按VID[%s]/PID[%s]枚举当前设备上的相机所支持的分辨率失败,不记录Log",
                m_stOpenMode.szHidVid[0], m_stOpenMode.szHidVid[0]);
            return FALSE;
        }

        stResoListTmp = stResoLists;
        std::string  stdReso = "";
        while(stResoListTmp != nullptr)
        {
            stdReso.append("W:");
            stdReso.append(std::to_string(stResoListTmp->width));
            stdReso.append(" H:");
            stdReso.append(std::to_string(stResoListTmp->height));
            stdReso.append("|");
            stResoListTmp = stResoListTmp->next;
        }
        Log(ThisModule, __LINE__,
            "按VID[%s]/PID[%s]枚举当前设备上的相机所支持的分辨率如下: %s",
            m_stOpenMode.szHidVid[0], m_stOpenMode.szHidVid[0], stdReso.c_str());
    }

    return TRUE;
}

// Impl错误码转换为CAM错误码
INT CDevCAM_CloudWalk::ConvertImplErrCode2CAM(INT nRet)
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
            CASE_RET_DEVCODE(IMP_ERR_DEV_SUCCESS, CAM_SUCCESS)              // 成功
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002001, ERR_CAM_PARAM_ERR)       // 输入参数错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002002, ERR_CAM_NODEVICE)        // 相机不存在
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002003, ERR_CAM_OPENFAIL)        // 相机打开失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002004, ERR_CAM_OTHER)           // 相机关闭失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002005, ERR_CAM_NODEVICE)        // 非云从相机
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002006, ERR_CAM_OTHER)           // 读取芯片授权码失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002007, ERR_CAM_OPENFAIL)        // 创建相机句柄失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002008, ERR_CAM_OPENFAIL)        // 相机句柄为空
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002009, ERR_CAM_OTHER)           // 不支持的相机类型
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002010, ERR_CAM_OTHER)           // 参数指针为空
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002011, ERR_CAM_OTHER)           // 成像参数读取失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002012, ERR_CAM_OTHER)           // 成像参数设置失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002013, ERR_CAM_OTHER)           // 相机参数读取失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002014, ERR_CAM_OTHER)           // 相机参数设置失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002015, ERR_CAM_OTHER)           // 枚举相机设备失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002016, ERR_CAM_OTHER)           // 枚举相机分辨率失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002017, ERR_CAM_OTHER)           // 相机运行中
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002018, ERR_CAM_UNKNOWN)         // 操作未知的错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002019, ERR_CAM_OTHER)           // 设置相机休眠失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002020, ERR_CAM_OTHER)           // 设置相机唤醒失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75002021, ERR_CAM_OFFLINE)         // 相机设备断开
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000108, ERR_CAM_PARAM_ERR)       // 输入参数错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000109, ERR_CAM_PARAM_ERR)       // 无效的句柄
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000110, ERR_CAM_OTHER)           // 读取芯片授权码失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000111, ERR_CAM_OTHER)           // 检测器未初始化
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000112, ERR_CAM_OTHER)           // 未知错误catch
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000113, ERR_CAM_OTHER)           // 设置人脸检测回调函数错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000114, ERR_CAM_OTHER)           // 设置活体检测回调函数错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000115, ERR_CAM_OTHER)           // 设置日志信息回调函数错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000116, ERR_CAM_DATA_FAIL)       // 抠取人脸图像错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000117, ERR_CAM_OTHER)           // 创建com服务错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000118, ERR_CAM_OPENFAIL)        // 重复创建检测器
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000119, ERR_CAM_OTHER)           // base64解码错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000120, ERR_CAM_LIVEDETECT)      // 检测到换脸
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000121, ERR_CAM_OTHER)           // 创建检测器错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_75000123, ERR_CAM_LIVEDETECT)      // 检测到多张人脸
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121000, CAM_SUCCESS)             // 成功or合法
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121001, ERR_CAM_OTHER)           // 空图像
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121002, ERR_CAM_OTHER)           // 图像格式不支持
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121003, ERR_CAM_OTHER)           // 没有人脸
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121004, ERR_CAM_OTHER)           // ROI设置失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121005, ERR_CAM_OTHER)           // 最小最大人脸设置失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121006, ERR_CAM_OTHER)           // 数据范围错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121007, ERR_CAM_OTHER)           // 未授权
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121008, ERR_CAM_OTHER)           // 尚未初始化
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121009, ERR_CAM_OTHER)           // 加载检测模型失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121010, ERR_CAM_OTHER)           // 加载关键点模型失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121011, ERR_CAM_OTHER)           // 加载质量评估模型失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121012, ERR_CAM_OTHER)           // 加载活体检测模型失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121013, ERR_CAM_LIVEDETECT)      // 检测失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121014, ERR_CAM_OTHER)           // 提取关键点失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121015, ERR_CAM_OTHER)           // 对齐人脸失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121016, ERR_CAM_OTHER)           // 质量评估失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121017, ERR_CAM_OTHER)           // 活体检测错误
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121018, ERR_CAM_OTHER)           // 时间戳不匹配
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121019, ERR_CAM_OTHER)           // 获取检测参数失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121020, ERR_CAM_OTHER)           // 设置检测参数失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121021, ERR_CAM_OTHER)           // 获取版本信息失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121022, ERR_CAM_OTHER)           // 删除文件或文件夹失败
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121030, ERR_CAM_OTHER)           // 人脸检测默认值
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121031, ERR_CAM_OTHER)           // color图像通过检测
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121032, ERR_CAM_LIVEDETECT)      // 人脸距离太近
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121033, ERR_CAM_LIVEDETECT)      // 人脸距离太远
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121034, ERR_CAM_LIVEDETECT)      // 人脸角度不满足要求
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121035, ERR_CAM_LIVEDETECT)      // 人脸清晰度不满足要求
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121036, ERR_CAM_LIVEDETECT)      // 检测到闭眼，仅在设置参数时eyeopen为true且检测到闭眼
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121037, ERR_CAM_LIVEDETECT)      // 检测到张嘴，仅在设置参数时mouthopen为true且检测到多人时返回
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121038, ERR_CAM_LIVEDETECT)      // 检测到人脸过亮，仅在设置参数时brightnessexc为true且人脸过亮时返回
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121039, ERR_CAM_LIVEDETECT)      // 检测到人脸过暗，仅在设置参数时brightnessins为true且人脸过暗时返回
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121040, ERR_CAM_LIVEDETECT)      // 检测到人脸置信度过低，仅在设置参数时confidence为true且人脸置信度过低时返回
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121041, ERR_CAM_LIVEDETECT)      // 检测到人脸存在遮挡，仅在设置参数时occlusion为true且检测到遮挡时返回
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121042, ERR_CAM_LIVEDETECT)      // 检测到黑框眼镜，仅在设置参数时blackspec为true且检测到黑框眼镜时返回
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121043, ERR_CAM_LIVEDETECT)      // 检测到墨镜，仅在设置参数时sunglass为true且检测到墨镜时返回
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121044, ERR_CAM_LIVEDETECT)      // 检测到口罩,仅在设置参数时proceduremask>-1且检测到口罩时返回
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121080, ERR_CAM_OTHER)           // 活体检测默认值
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121081, CAM_SUCCESS)             // 活体
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121082, ERR_CAM_LIVEDETECT)      // 非活体
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121083, ERR_CAM_LIVEDETECT)      // 人脸肤色检测未通过
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121084, ERR_CAM_LIVEDETECT)      // 可见光和红外人脸不匹配
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121085, ERR_CAM_LIVEDETECT)      // 红外输入没有人脸
            CASE_RET_DEVCODE(IMP_ERR_DEV_26121086, ERR_CAM_LIVEDETECT)      // 可见光输入没有人脸
            default: return ERR_CAM_OTHER;
        }
    }
    return CAM_SUCCESS;
}

// 根据Impl错误码设置错误错误码字符串
INT CDevCAM_CloudWalk::ConvertImplErrCode2ErrDetail(INT nRet)
{
#define CASE_SET_DEV_DETAIL(IMP, STR) \
        case IMP: m_clErrorDet.SetDevErrCode((LPSTR)STR); break;

#define CASE_SET_HW_DETAIL(IMP, DSTR, DEVID) \
            m_clErrorDet.SetDevErrCode((LPSTR)DSTR); \
            m_clErrorDet.SetHWErrCodeHex(IMP, DEVID); break;

    switch(nRet)
    {
        case IMP_ERR_DEV_SUCCESS:   // 成功
        case IMP_ERR_DEV_26121000:  // 成功or合法
        case IMP_ERR_DEV_26121081:  // 活体
            break;
        default:
            CASE_SET_HW_DETAIL(nRet, EC_DEV_HWErr, m_stOpenMode.nOtherParam[0])
    }

    return CAM_SUCCESS;
}

// Impl错误码转换为CAM活检状态码
INT CDevCAM_CloudWalk::ConvertImplLiveErr2Dev(INT nRet)
{
    switch(nRet) // 活检图像检查错误记录到全局变量
    {
        case IMP_ERR_DEV_26121086:              // 26121086:可见光输入没有人脸
            return LIVE_ERR_VIS_NOFACE;             // 可见光未检测到人脸
        case IMP_ERR_DEV_26121041:              // 检测到人脸存在遮挡
        case IMP_ERR_DEV_26121042:              // 检测到黑框眼镜
        case IMP_ERR_DEV_26121043:              // 检测到墨镜
        case IMP_ERR_DEV_26121044:              // 检测到口罩
            return LIVE_ERR_FACE_SHELTER;           // 人脸有遮挡(五官有遮挡,戴镜帽等)
        case IMP_ERR_DEV_26121032:              // 人脸距离太近
            return LIVE_ERR_FACE_TOONEAR;           // 人脸离摄像头太近
        case IMP_ERR_DEV_26121033:              // 人脸距离太远
            return LIVE_ERR_FACE_TOOFAR;            // 人脸离摄像头太远
        case IMP_ERR_DEV_26121034:              // 人脸角度不满足要求
            return LIVE_ERR_FACE_ANGLE_FAIL;        // 人脸角度不满足要求(低头/抬头/侧脸等)
        case IMP_ERR_DEV_26121035:              // 人脸清晰度不满足要求
            return LIVE_ERR_FACE_SHAKE;             // 检测到人脸晃动/模糊
        case IMP_ERR_DEV_26121036:              // 检测到闭眼
            return LIVE_ERR_FACE_EYECLOSE;          // 检测到闭眼
        case IMP_ERR_DEV_26121037:              // 检测到张嘴
            return LIVE_ERR_FACE_MOUTHOPEN;         // 检测到张嘴
        case IMP_ERR_DEV_26121082:              // 非活体
            return LIVE_ERR_IS_UNLIVE;              // 检测到非活体
        case IMP_ERR_DEV_26121085:              // 红外输入没有人脸
            return LIVE_ERR_NIS_NOFACE;             // 红外光未检测到人脸
        case IMP_ERR_DEV_75000123:              // 检测到多张人脸
            return LIVE_ERR_FACE_MULTIPLE;          // 检测到多张人脸
        default:
            return LIVE_ERR_UNKNOWN;                // 其他/未知错误
    }
}

// 活体检测结果生成图像
BOOL CDevCAM_CloudWalk::LiveDetectToImage(CWLiveImage live, LPSTR lpSaveFileName, WORD wSaveType)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHAR szFileNameJPG[MAX_PATH] = { 0x00 };
    CHAR szFileNameBASE64[MAX_PATH] = { 0x00 };
    CHAR szFileNameBMP[MAX_PATH] = { 0x00 };
    BOOL bSaveJPG = FALSE;
    BOOL bSaveBASE64 = FALSE;
    BOOL bSaveBMP = FALSE;

    if ((wSaveType & PIC_JPG) == PIC_JPG ||
        (wSaveType & PIC_BASE64) == PIC_BASE64)
    {
        sprintf(szFileNameJPG, "%s", lpSaveFileName);

        cv::Mat mat(live.height, live.width, CV_8UC3, live.data);
        if (cv::imwrite(szFileNameJPG, mat) == TRUE)
        {
            Log(ThisModule, __LINE__, "活体检测结果生成图像: JPG: ->生成图片[%s]成功", szFileNameJPG);
            bSaveJPG = TRUE;

            if ((wSaveType & PIC_BASE64) == PIC_BASE64)
            {
                sprintf(szFileNameBASE64, "%s", lpSaveFileName);
                std::string szBase64;
                szBase64.clear();

                nRet = m_pDevImpl.Image2Base64Format(szFileNameJPG, &szBase64);
                if (nRet == IMP_SUCCESS)
                {
                    if (szBase64.size() < 1)
                    {
                        Log(ThisModule, __LINE__,
                            "活体检测结果生成图像: JPG转BASE64: ->Image2Base64Format(%s) Fail，返回data<1", szFileNameJPG);
                        bSaveBASE64 = FALSE;
                    } else
                    {
                        Log(ThisModule, __LINE__,
                            "活体检测结果生成图像: JPG转BASE64: ->Image2Base64Format(%s)成功", szFileNameJPG);
                        if (FileAccess::WriteDataToFile(szFileNameBASE64, (char*)szBase64.c_str(), szBase64.size()) != TRUE)
                        {
                            Log(ThisModule, __LINE__,
                                "活体检测结果生成图像: JPG转BASE64: 保存BASE64数据到文件: ->WriteDataToFile(%s) Fail",
                                szFileNameBASE64);
                            bSaveBASE64 = FALSE;
                        } else
                        {
                            bSaveBASE64 = TRUE;
                        }
                    }
                } else
                {
                    Log(ThisModule, __LINE__, "showLive()->bImage2Base64Format(): [%s]转BASE64 Data失败", szFileNameJPG);
                    bSaveBASE64 = FALSE;
                }
            }
        } else
        {
            Log(ThisModule, __LINE__,
                "活体检测结果生成图像: JPG转BASE64: ->Image2Base64Format(%s) Fail, ErrCode: %s",
                m_pDevImpl.ConvertCode_Impl2Str(nRet));
            bSaveJPG = FALSE;
        }
    }

    if ((wSaveType & PIC_BMP) == PIC_BMP)
    {
        sprintf(szFileNameBMP, "%s", lpSaveFileName);

        cv::Mat mat(live.height, live.width, CV_8UC3, live.data);
        if (cv::imwrite(szFileNameBMP, mat) == TRUE)
        {
            Log(ThisModule, __LINE__, "活体检测结果生成图像: BMP: ->生成图片[%s]成功", szFileNameBMP);
            bSaveBMP = TRUE;
        } else
        {
            Log(ThisModule, __LINE__, "活体检测结果生成图像: BMP: ->生成图片[%s]成功", szFileNameBMP);
            bSaveBMP = TRUE;
        }
    }

    if (((wSaveType & PIC_BASE64) == PIC_BASE64 && bSaveBASE64 == FALSE) ||
        ((wSaveType & PIC_JPG) == PIC_JPG && bSaveJPG == FALSE) ||
        ((wSaveType & PIC_BMP) == PIC_BMP && bSaveBMP == FALSE))
    {
        return FALSE;
    }

    return TRUE;
}

// 摄像窗口打开前处理(重写)
INT CDevCAM_CloudWalk::VideoCameraOpenFrontRun(STDISPLAYPAR stDisplayIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_stDisplayReso.width = stDisplayIn.wWidth;
    m_stDisplayReso.height = stDisplayIn.wHeight;

    return CAM_SUCCESS;
}

// 打开设备摄像画面(重写)
INT CDevCAM_CloudWalk::VideoCameraOpen(WORD wWidth, WORD wHeight)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    if (m_stOpenMode.wOpenMode == 0)    // 序号方式打开
    {
        // 按序号开启相机
        nRet = m_pDevImpl.OpenCamera(m_stOpenMode.nHidVid[0], 0, wWidth, wHeight);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打开设备摄像画面(可见光:序号方式): ->OpenCamera(%d, %d, %d, %d) Fail, ErrCode: %s, Return: %s.",
                m_stOpenMode.nHidVid[0], 0, wWidth, wHeight, m_pDevImpl.ConvertCode_Impl2Str(nRet),
                ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
            return ConvertImplErrCode2CAM(nRet);
        }
        nRet = m_pDevImpl.OpenCamera(m_stOpenMode.nHidVid[1], 1, wWidth, wHeight);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打开设备摄像画面(红外光:序号方式): ->OpenCamera(%d, %d, %d, %d) Fail, ErrCode: %s, Return: %s.",
                m_stOpenMode.nHidPid[0], 0, wWidth, wHeight, m_pDevImpl.ConvertCode_Impl2Str(nRet),
                ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
            return ConvertImplErrCode2CAM(nRet);
        }
    } else                          // VidPid方式打开
    {
        // 开启相机(按VID/PID)
        nRet = m_pDevImpl.OpenCameraEx(m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0], 0, wWidth, wHeight);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打开设备摄像画面(可见光:VidPid方式): ->OpenCamera(%s, %s, %d, %d, %d) Fail, ErrCode: %s, Return: %s.",
                m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0], 0, wWidth, wHeight,
                m_pDevImpl.ConvertCode_Impl2Str(nRet), ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
            return ConvertImplErrCode2CAM(nRet);
        }
        nRet = m_pDevImpl.OpenCameraEx(m_stOpenMode.szHidVid[1], m_stOpenMode.szHidPid[1], 1, wWidth, wHeight);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "打开设备摄像画面(红外光:VidPid方式): ->OpenCamera(%s, %s, %d, %d, %d) Fail, ErrCode: %s, Return: %s.",
                m_stOpenMode.szHidVid[1], m_stOpenMode.szHidPid[1], 0, wWidth, wHeight,
                m_pDevImpl.ConvertCode_Impl2Str(nRet), ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
            return ConvertImplErrCode2CAM(nRet);
        }
    }

    return CAM_SUCCESS;
}

// 关闭设备摄像画面(重写)
INT CDevCAM_CloudWalk::VideoCameraClose()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 关闭所有相机
    nRet = m_pDevImpl.CloseAllCameras();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "关闭设备摄像画面: ->CloseAllCameras() Fail, ErrCode: %s, Return: %s.",
            m_pDevImpl.ConvertCode_Impl2Str(nRet), ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
        return ConvertImplErrCode2CAM(nRet);
    }
    // 停止连续活体检测
    m_pDevImpl.StopLiveDetect();

    return CAM_SUCCESS;
}

// 暂停设备摄像画面(重写)
INT CDevCAM_CloudWalk::VideoCameraPause()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 恢复设备摄像画面(重写)
INT CDevCAM_CloudWalk::VideoCameraResume()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 获取窗口显示数据(重写)
INT CDevCAM_CloudWalk::GetViewImage(LPSTIMGDATA lpImgData, INT nWidth, INT nHeight, DWORD dwParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return m_pDevImpl.GetVideoData(lpImgData, nWidth, nHeight, dwParam);
}

// 拍照前运行处理(重写)
INT CDevCAM_CloudWalk::TakePicFrontRun()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 活检前确认
    m_pDevImpl.StopLiveDetect();

    // 开始活体检测
    nRet = m_pDevImpl.StartLiveDetect(TRUE);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "开启活体检测: ->StartLiveDetect(TRUE) Fail, ErrCode: %s, Return: %s.",
            m_pDevImpl.ConvertCode_Impl2Str(nRet),
            ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
        return ConvertImplErrCode2CAM(nRet);
    }

    return CAM_SUCCESS;
}

// 拍照后运行处理(重写)
INT CDevCAM_CloudWalk::TakePicAfterRun()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 终止活体检测
    m_pDevImpl.StopLiveDetect();

    return CAM_SUCCESS;
}

// 获取检测结果(重写)
BOOL CDevCAM_CloudWalk::GetLiveDetectResult()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return m_pDevImpl.GetIsLiveSucc();
}

// 保存图像(重写)
INT CDevCAM_CloudWalk::TakePicSaveImage(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;
    CWLiveImage live_Person; // 人脸摄像数据
    CWLiveImage live_Room;   // 全景摄像数据
    BOOL bRet = TRUE;

    BOOL bIsPersonImg = FALSE;
    CHAR szPersonFile[256] = { 0x00 };

    if (stTakePicIn.wCameraAction == DEV_CAM_MODE_ROOM)    // 全景摄像
    {
        nRet = m_pDevImpl.CaptureCurrentFrame(m_stDisplayReso, FALSE/*T指定Base64编码格式输出*/, &live_Room);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "活体成功后保存图像: 取全景图像数据: ->CaptureCurrentFrame() Fail, ErrCode: %s, Return: %s.",
                m_pDevImpl.ConvertCode_Impl2Str(nRet),
                ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
            return ConvertImplErrCode2CAM(nRet);
        } else
        {
            Log(ThisModule, __LINE__,
                "活体成功后保存图像: 取全景图像数据: ->CaptureCurrentFrame() Succ, ErrCode: %s.",
                m_pDevImpl.ConvertCode_Impl2Str(nRet));
        }

        bRet = LiveDetectToImage(live_Room, stTakePicIn.szFileName, stTakePicIn.wPicType);

        if (strlen(m_szPersonImgFile) > 0)  // 特殊处理,需要生成人脸图
        {
            bIsPersonImg = TRUE;
            CHAR szFileTmp[4][256] = { 0x00 };
            FileDir::_splitpath(stTakePicIn.szFileName, szFileTmp[0], szFileTmp[1], szFileTmp[2], szFileTmp[3]);
            sprintf(szPersonFile, "%s/%s%s", szFileTmp[1], m_szPersonImgFile, szFileTmp[3]);
        }
    }

    if (stTakePicIn.wCameraAction == DEV_CAM_MODE_PERSON ||    // 人脸摄像
        bIsPersonImg == TRUE)
    {
        nRet = m_pDevImpl.GetBestFace(2.0f, FALSE/*T指定Base64编码格式输出*/, &live_Person);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "活体成功后保存图像: 取人脸图像数据: ->GetBestFace() Fail, ErrCode: %s, Return: %s.",
                m_pDevImpl.ConvertCode_Impl2Str(nRet),
                ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
            return ConvertImplErrCode2CAM(nRet);
        } else
        {
            Log(ThisModule, __LINE__,
                "活体成功后保存图像: 取人脸图像数据: ->GetBestFace() Succ, ErrCode: %s.",
                m_pDevImpl.ConvertCode_Impl2Str(nRet));
        }

        if (bIsPersonImg == TRUE)
        {
            bRet = LiveDetectToImage(live_Person, szPersonFile, stTakePicIn.wPicType);
        } else
        {
            bRet = LiveDetectToImage(live_Person, stTakePicIn.szFileName, stTakePicIn.wPicType);
        }
    }

    // 入参生成全景或人脸图像时,允许生成人脸红外图像
    if ((stTakePicIn.wCameraAction == DEV_CAM_MODE_ROOM ||      // 全景摄像
         stTakePicIn.wCameraAction == DEV_CAM_MODE_PERSON) &&   // 人脸摄像
        strlen(m_szPersonNirImgFile) > 0)                       // 需要生成红外图像
    {
        CHAR szNirFile[MAX_PATH] = { 0x00 };    // 保存文件名
        CWLiveImage live_Nis;                   // 红外摄像数据

        if (m_pDevImpl.GetNirImgSup() != TRUE)
        {
            Log(ThisModule, __LINE__, "活体成功后保存图像: 当前SDK版本不支持[获取人脸红外数据]功能,不生成红外图像");
        } else
        {
            nRet = m_pDevImpl.GetBestFace(2.0f, FALSE/*T指定Base64编码格式输出*/, &live_Nis, 2);
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "活体成功后保存图像: 取人脸红外图像数据: ->GetBestFace() Fail, ErrCode: %s, Return: %s.",
                    m_pDevImpl.ConvertCode_Impl2Str(nRet),
                    ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
                return ConvertImplErrCode2CAM(nRet);
            } else
            {
                Log(ThisModule, __LINE__,
                    "活体成功后保存图像: 取人脸红外图像数据: ->GetBestFace() Succ, ErrCode: %s.",
                    m_pDevImpl.ConvertCode_Impl2Str(nRet));
            }

            CHAR szFileTmp[4][256] = { 0x00 };
            FileDir::_splitpath(stTakePicIn.szFileName, szFileTmp[0], szFileTmp[1], szFileTmp[2], szFileTmp[3]);
            sprintf(szNirFile, "%s/%s%s%s", szFileTmp[1], szFileTmp[2], m_szPersonNirImgFile, szFileTmp[3]);

            bRet = LiveDetectToImage(live_Nis, szNirFile, stTakePicIn.wPicType);
        }
    }

    if (bRet == FALSE)
    {
        return ERR_CAM_SAVE_IMAGE;
    }

    return CAM_SUCCESS;
}

// -------------------------------------- END --------------------------------------
