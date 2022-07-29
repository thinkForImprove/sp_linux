/***************************************************************************
* 文件名称: DevHCAM_JDY5001A0809.cpp
* 文件描述：摄像功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年6月6日
* 文件版本：1.0.0.1
****************************************************************************/

#include <unistd.h>

#include "DevCAM_JDY5001A0809.h"

static const char *ThisFile = "DevCAM_JDY5001A0809.cpp";

/***************************************************************************
*     对外接口调用处理                                                         *
***************************************************************************/

CDevCAM_JDY5001A0809::CDevCAM_JDY5001A0809(LPCSTR lpDevType) :
    m_pDevImpl(LOG_NAME_DEV, lpDevType)//,
    //m_clErrorDet((LPSTR)"1")      // 设定JDY5001A0809类型编号为1
{
    SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件

    m_enDisplayStat = EN_DISP_ENDING;       // Display命令状态
    m_bReCon = FALSE;                       // 是否断线重连状态
    m_qSharedMemData = nullptr;             // 共性内存句柄
    MSET_0(m_szSharedDataName);             // 共享内存名
    m_ulSharedDataSize = 0;                 // 共享内存大小
    m_bCancel = FALSE;                      // 是否下发取消命令
    m_nRefreshTime = 30;                    // 摄像数据获取间隔
    m_nDevStatOLD = DEV_NOTFOUND;           // 保留上一次状态变化
    MSET_XS(m_nSaveStat, 0, sizeof(INT) * 12); // 保存必要的状态信息
    MSET_XS(m_nRetErrOLD, 0, sizeof(INT) * 12);// 处理错误值保存
    m_nClipMode = EN_CLIP_NO;               // 图像镜像模式转换
    m_wDisplayOpenMode = DISP_OPEN_MODE_THREAD;// Display窗口打开模式(必须设定):线程支持
    memset(m_bDevPortIsHaveOLD, FALSE, sizeof(BOOL) * 2);// 记录设备是否连接上)
}

CDevCAM_JDY5001A0809::~CDevCAM_JDY5001A0809()
{
    Close();
}

// 打开连接
int CDevCAM_JDY5001A0809::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();    // 断线重连时会重复调用,注销避免出现Log冗余
    //AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    m_pDevImpl.CloseDevice();

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
        nRet = m_pDevImpl.OpenDevice(m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0]);
        if (nRet != IMP_SUCCESS)
        {
            if (m_nRetErrOLD[0] != nRet)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: ->OpenDevice(%s, %s) Fail, ErrCode: %s, Return: %s.",
                    m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0],
                    m_pDevImpl.ConvertCode_Impl2Str(nRet),
                    ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
                m_nRetErrOLD[0] = nRet;
            }
            return ConvertImplErrCode2CAM(nRet);
        }
    }
    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "断线重连: 打开设备: : ->OpenDevice(%s, %s) Succ.",
            m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0]);
        m_bReCon = FALSE; // 是否断线重连状态: 初始F
    } else
    {
        Log(ThisModule, __LINE__,  "打开设备: ->OpenDevice(%s, %s) Succ.",
            m_stOpenMode.szHidVid[0], m_stOpenMode.szHidPid[0]);
    }

    // 设置摄像模式
    //m_pDevImpl.SetVideoCaptMode(m_stVideoParam);
    SetVideoMode(m_stVideoParam);

    // 取摄像参数
    STVIDEOPAMAR stVideoPar;
    m_pDevImpl.GetVideoCaptMode(stVideoPar);
    Log(ThisModule, __LINE__,
        "打开设备: 取摄像参数如下:  宽度:%.2f, 高度:%.2f, 帧率:%.2f, 亮度:%.2f, 对比度:%.2f, 饱和度:%.2f, 色调:%.2f, 曝光:%.2f",
        stVideoPar.duWidth, stVideoPar.duHeight, stVideoPar.duFPS, stVideoPar.duBright,
        stVideoPar.duContrast, stVideoPar.duSaturation, stVideoPar.duHue, stVideoPar.duExposure);

    return CAM_SUCCESS;
}

// 关闭连接
int CDevCAM_JDY5001A0809::Close()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_enDisplayStat = EN_DISP_ENDING;
    DisConnSharedMemory();
    m_pDevImpl.CloseDevice();

    return CAM_SUCCESS;
}

// 设备复位
int CDevCAM_JDY5001A0809::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    nRet = m_pDevImpl.ResetDevice();
    if(nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "设备复位: ->ResetDevice() Fail, ErrCode: %s, Return: %s",
            nRet, m_pDevImpl.ConvertCode_Impl2Str(nRet),
            ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
        return ConvertImplErrCode2CAM(nRet);
    }

    return CAM_SUCCESS;
}

// 取设备状态
int CDevCAM_JDY5001A0809::GetStatus(STDEVCAMSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nStat = DEV_NOTOPEN;

    CHAR szStatStr[][64] = { "设备正常", "设备未找到", "设备未打开", "设备已打开但断线" };

    nStat = m_pDevImpl.GetDeviceStatus();

    // 记录设备状态变化
    if (m_nDevStatOLD != nStat)
    {
        Log(ThisModule, __LINE__, "设备状态有变化: %d[%s] -> %d[%s]",
            m_nDevStatOLD, szStatStr[m_nDevStatOLD], nStat, szStatStr[nStat]);
        m_nDevStatOLD = nStat;
        if ((nStat == DEV_NOTFOUND || nStat == DEV_OFFLINE || nStat == DEV_NOTOPEN) &&
            m_enDisplayStat != EN_DISP_ENDING)
        {
            Log(ThisModule, __LINE__, "设备连接断开: 有窗口显示中, 设置关闭.");
            m_enDisplayStat = EN_DISP_ENDING;
        }
    }

    switch(nStat)
    {
        case DEV_OK:
            stStatus.wDevice = DEVICE_STAT_ONLINE;
            stStatus.wMedia[0] = CAM_MEDIA_STAT_OK;
            stStatus.fwCameras[0] = CAM_CAMERA_STAT_OK;
            break;
        case DEV_NOTFOUND:
            stStatus.wDevice = DEVICE_STAT_NODEVICE;
            stStatus.wMedia[0] = CAM_MEDIA_STAT_UNKNOWN;
            stStatus.fwCameras[0] = CAM_CAMERA_STAT_UNKNOWN;
            break;
        case DEV_OFFLINE:
        case DEV_NOTOPEN:
            stStatus.wDevice = DEVICE_STAT_OFFLINE;
            stStatus.wMedia[0] = CAM_MEDIA_STAT_UNKNOWN;
            stStatus.fwCameras[0] = CAM_CAMERA_STAT_UNKNOWN;
            break;
    }

    // 摄像窗口只在状态连接正常情况下上报错误事件
    // 状态连接异常有统一上报方式
    if (m_nSaveStat[0] == 1)
    {
        if (nStat == DEV_OK)
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
int CDevCAM_JDY5001A0809::Cancel(unsigned short usMode)
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
int CDevCAM_JDY5001A0809::Display(STDISPLAYPAR stDisplayIn, void *vParam/* = nullptr*/)
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
int CDevCAM_JDY5001A0809::TakePicture(STTAKEPICTUREPAR stTakePicIn, void *vParam/* = nullptr*/)
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
int CDevCAM_JDY5001A0809::SetData(unsigned short usType, void *vData/* = nullptr*/)
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
        case SET_DEV_RECON:             // 设置断线重连标记为T
        {
            if (m_bReCon == FALSE)
            {
                Log(ThisModule, __LINE__, "设备重连 Start......");
                m_bReCon = TRUE;
            }
            break;
        }
        case SET_DEV_OPENMODE:          // 设置设备打开模式
        {
            if (vData != nullptr)
            {
                memcpy(&(m_stOpenMode), ((LPSTDEVICEOPENMODE)vData), sizeof(STDEVICEOPENMODE));
                m_nRefreshTime = ((LPSTDEVICEOPENMODE)vData)->nOtherParam[1];
            }
            break;
        }
        case SET_DEV_VIDEOMODE:         // 设置设备摄像模式
        {
            if (vData != nullptr)
            {
                memcpy(&(m_stVideoParam), ((LPSTVIDEOPARAM)vData), sizeof(STVIDEOPAMAR));
                // 图像镜像模式转换
                if (m_stVideoParam.nOtherParam[0] == 1)
                {
                    m_nClipMode = EN_CLIP_LR;
                } else
                if (m_stVideoParam.nOtherParam[0] == 2)
                {
                    m_nClipMode = EN_CLIP_UD;
                } else
                if (m_stVideoParam.nOtherParam[0] == 3)
                {
                    m_nClipMode = EN_CLIP_UDLR;
                } else
                {
                    m_nClipMode = EN_CLIP_NO;
                }
            }
            break;
        }
        default:
            break;
    }

    return CAM_SUCCESS;
}

// 获取数据
int CDevCAM_JDY5001A0809::GetData(unsigned short usType, void *vData)
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
        default:
            break;
    }

    return CAM_SUCCESS;
}

// 版本号(1DevCam版本/2固件版本/3其他)
int CDevCAM_JDY5001A0809::GetVersion(unsigned short usType, char* szVer, int nSize)
{

    return CAM_SUCCESS;
}

// 设置摄像参数
INT CDevCAM_JDY5001A0809::SetVideoMode(STVIDEOPAMAR stVM)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHAR szPrtLog[1024] = "";

#define SET_VIDEO_MODE(IN, TY, STR) \
    if (IN == SET_VM_INV) \
    { \
        sprintf(szPrtLog + strlen(szPrtLog), "摄像%s[%.2lf]无效,不设置; ", STR, IN); \
    } else \
    { \
        if (m_pDevImpl.SetVideoCaptMode(TY, IN) == IMP_SUCCESS) \
        { \
            sprintf(szPrtLog + strlen(szPrtLog), "摄像%s[%.2lf]设置成功; ", STR, IN); \
        } else \
        { \
            sprintf(szPrtLog + strlen(szPrtLog), "摄像%s[%.2lf]设置失败; ", STR, IN); \
        } \
    }


    //SET_VIDEO_MODE(stVM.duWidth, VM_WIDTH, "宽度");
    //SET_VIDEO_MODE(stVM.duHeight, VM_HEIGHT, "高度");
    //SET_VIDEO_MODE(stVM.duFPS, VM_FPS, "帧率");
    SET_VIDEO_MODE(stVM.duBright, VM_BRIGHT, "亮度");
    SET_VIDEO_MODE(stVM.duContrast, VM_CONTRAST, "对比度");
    SET_VIDEO_MODE(stVM.duSaturation, VM_SATURATION, "饱和度");
    SET_VIDEO_MODE(stVM.duHue, VM_HUE, "色调");
    SET_VIDEO_MODE(stVM.duExposure, VM_EXPOSURE, "曝光");

    Log(ThisModule, __LINE__, "设置摄像参数: %s", szPrtLog);

    return CAM_SUCCESS;
}

// Impl错误码转换为CAM错误码
INT CDevCAM_JDY5001A0809::ConvertImplErrCode2CAM(INT nRet)
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
            CASE_RET_DEVCODE(IMP_ERR_VIDPID_NOTFOUND, ERR_CAM_NODEVICE)         // VID/PID未找到
            CASE_RET_DEVCODE(IMP_ERR_VIDEOIDX_NOTFOUND, ERR_CAM_NODEVICE)       // 未找到摄像设备索引号
            CASE_RET_DEVCODE(IMP_ERR_GET_IMGDATA_FAIL, ERR_CAM_OTHER)           // 取帧图像数据失败
            CASE_RET_DEVCODE(IMP_ERR_GET_IMGDATA_ISNULL, ERR_CAM_OTHER)         // 取帧图像数据为空
            CASE_RET_DEVCODE(IMP_ERR_GET_IMGDATA_BUFFER, ERR_CAM_OTHER)         // 取帧图像数据空间异常
            CASE_RET_DEVCODE(IMP_ERR_GET_IMGDATA_CHANGE, ERR_CAM_OTHER)         // 帧图像数据转换失败
            CASE_RET_DEVCODE(IMP_ERR_SAVE_IMAGE_FILE, ERR_CAM_OTHER)            // 帧图像数据保存到文件失败

            default: return ERR_CAM_OTHER;
        }
    }

    return CAM_SUCCESS;
}

// 根据Impl错误码设置错误错误码字符串
INT CDevCAM_JDY5001A0809::ConvertImplErrCode2ErrDetail(INT nRet)
{
#define CASE_SET_DEV_DETAIL(IMP, STR) \
        case IMP: m_clErrorDet.SetDevErrCode((LPSTR)STR); break;

#define CASE_SET_HW_DETAIL(IMP, DSTR, HSTR, DEVID) \
        case IMP: \
            m_clErrorDet.SetDevErrCode((LPSTR)DSTR); \
            m_clErrorDet.SetHWErrCodeStr((LPSTR)HSTR, DEVID); break;

    switch(nRet)
    {
        CASE_SET_DEV_DETAIL(IMP_ERR_VIDPID_NOTFOUND, EC_DEV_DevNotFound)    // VID/PID未找到
        CASE_SET_DEV_DETAIL(IMP_ERR_VIDEOIDX_NOTFOUND, EC_DEV_DevNotFound)  // 未找到摄像设备索引号
        CASE_SET_DEV_DETAIL(IMP_ERR_GET_IMGDATA_FAIL, EC_CAM_DEV_GetImageErr)// 取帧图像数据失败
        CASE_SET_DEV_DETAIL(IMP_ERR_GET_IMGDATA_ISNULL, EC_CAM_DEV_GetImageErr)// 取帧图像数据为空
        CASE_SET_DEV_DETAIL(IMP_ERR_GET_IMGDATA_BUFFER, EC_CAM_DEV_GetImageErr)// 取帧图像数据空间异常
        CASE_SET_DEV_DETAIL(IMP_ERR_GET_IMGDATA_CHANGE, EC_CAM_DEV_GetImageErr)// 帧图像数据转换失败
        CASE_SET_DEV_DETAIL(IMP_ERR_SAVE_IMAGE_FILE, EC_CAM_DEV_SaveImageErr)// 帧图像数据保存到文件失败
    }

    return CAM_SUCCESS;
}

// 摄像窗口打开前处理(重写)
INT CDevCAM_JDY5001A0809::VideoCameraOpenFrontRun(STDISPLAYPAR stDisplayIn)
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
    nRet = m_pDevImpl.SetVideoCaptWH(wWidth, wHeight);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "摄像窗口处理: 建立窗口: 设置摄像窗口图像数据分辨率为 %d*%d 失败, ErrCode: %s.",
            wWidth, wHeight, ConvertDevErrCodeToStr(nRet));
    }

    INT nW = 0, nH = 0;
    nRet = m_pDevImpl.GetVideoCaptWH(nW, nH);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "摄像窗口处理: 建立窗口: 取当前摄像窗口图像数据分辨率: ->GetVideoCaptWH() Fail, ErrCode: %s",
            ConvertDevErrCodeToStr(nRet));
    } else
    {
        Log(ThisModule, __LINE__,
            "摄像窗口处理: 建立窗口: 当前摄像窗口图像数据分辨率为 %d*%d...", nW, nH);
    }

    return CAM_SUCCESS;
}

// 打开设备摄像画面(重写)
INT CDevCAM_JDY5001A0809::VideoCameraOpen(STDISPLAYPAR stDisplayIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    return CAM_SUCCESS;
}

// 关闭设备摄像画面(重写)
INT CDevCAM_JDY5001A0809::VideoCameraClose()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    return CAM_SUCCESS;
}

// 暂停设备摄像画面(重写)
INT CDevCAM_JDY5001A0809::VideoCameraPause()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 恢复设备摄像画面(重写)
INT CDevCAM_JDY5001A0809::VideoCameraResume()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 获取窗口显示数据(重写)
INT CDevCAM_JDY5001A0809::GetViewImage(LPSTIMGDATA lpImgData, INT nWidth, INT nHeight, DWORD dwParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return m_pDevImpl.GetVideoImage(lpImgData, nWidth, nHeight, dwParam);
}

// 拍照前运行处理(重写)
INT CDevCAM_JDY5001A0809::TakePicFrontRun(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    return CAM_SUCCESS;
}

// 拍照后运行处理(重写)
INT CDevCAM_JDY5001A0809::TakePicAfterRun(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 获取检测结果(重写)
BOOL CDevCAM_JDY5001A0809::GetLiveDetectResult()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return TRUE;
}

// 保存图像(重写)
INT CDevCAM_JDY5001A0809::TakePicSaveImage(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    nRet = m_pDevImpl.SaveImageFile(stTakePicIn.szFileName);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "摄像拍照: ->SaveImageFile(%s) Fail, ErrCode: %s, Return: %s",
            stTakePicIn.szFileName, m_pDevImpl.ConvertCode_Impl2Str(nRet),
            ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(nRet)));
        return ConvertImplErrCode2CAM(nRet);
    } else
    {
        Log(ThisModule, __LINE__, "摄像拍照: ->SaveImageFile(%s) Succ",
            stTakePicIn.szFileName);
    }


    return CAM_SUCCESS;
}

// -------------------------------------- END --------------------------------------
