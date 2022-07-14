/***************************************************************
* 文件名称: DevHCAM_JDY5001A0809.cpp
* 文件描述：摄像功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年6月6日
* 文件版本：1.0.0.1
****************************************************************/

#include <unistd.h>

#include "DevHCAM_JDY5001A0809.h"

static const char *ThisFile = "DevHCAM_JDY5001A0809.cpp";

/****************************************************************************
*     对外接口调用处理                                                         *
****************************************************************************/

CDevHCAM_JDY5001A0809::CDevHCAM_JDY5001A0809(LPCSTR lpDevType) :
    m_pDevImpl(LOG_NAME_DEV, lpDevType),
    m_clErrorDet((LPSTR)"1")      // 设定JDY5001A0809类型编号为1
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
}

CDevHCAM_JDY5001A0809::~CDevHCAM_JDY5001A0809()
{
    Close();
}

// 打开连接
int CDevHCAM_JDY5001A0809::Open(LPCSTR lpMode)
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
int CDevHCAM_JDY5001A0809::Close()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_enDisplayStat = EN_DISP_ENDING;
    DisConnSharedMemory();
    m_pDevImpl.CloseDevice();

    return CAM_SUCCESS;
}

// 设备复位
int CDevHCAM_JDY5001A0809::Reset()
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
int CDevHCAM_JDY5001A0809::GetStatus(STDEVCAMSTATUS &stStatus)
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
int CDevHCAM_JDY5001A0809::Cancel(unsigned short usMode)
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
int CDevHCAM_JDY5001A0809::Display(STDISPLAYPAR stDisplayIn, void *vParam/* = nullptr*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;
    WORD wWidth = 0;
    WORD wHeight = 0;

    if (stDisplayIn.wAction == CAMERA_OPEN)         // 执行打开窗口
    {
        if (m_enDisplayStat == EN_DISP_SHOWING)         // 窗口显示中，直接return
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 建立窗口: 窗口处于显示中, 不处理, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            return CAM_SUCCESS;
        } else
        if (m_enDisplayStat == EN_DISP_ENDING)          // 窗口未建立
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 建立窗口: 窗口未建立, 创建窗口处理线程...",
                ConvertDevErrCodeToStr(CAM_SUCCESS));

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

            // 启动窗口处理进程
            m_nThreadRet = IMP_SUCCESS;
            m_enDisplayStat = EN_DISP_CREATE;
            if (!m_thRunDisplay.joinable())
            {
                m_thRunDisplay = std::thread(&CDevHCAM_JDY5001A0809::ThreadRunDisplay, this,
                                             stDisplayIn.wWidth, stDisplayIn.wHeight);
                if (m_thRunDisplay.joinable())
                {
                    m_thRunDisplay.detach();   // 进程分离
                } else
                {
                    Log(ThisModule, __LINE__,
                        "摄像窗口处理: 建立窗口: 窗口未建立, 创建窗口处理线程失败: Return: %s",
                        ConvertDevErrCodeToStr(ERR_CAM_OTHER));
                    m_enDisplayStat = EN_DISP_ENDING;
                    return ERR_CAM_OTHER;
                }
            }

            // 根据超时时间，循环验证线程是否启动
            QTime qtTimeCurr = QTime::currentTime();     // 拍照执行前时间
            ULONG ulTimeCount = 0;
            while(1)
            {
                QCoreApplication::processEvents();

                if (stDisplayIn.dwTimeOut > 0)   // >0有超时;否则不超时
                {
                    ulTimeCount = qtTimeCurr.msecsTo(QTime::currentTime()); // 时间差计入超时计数(毫秒)
                    if (ulTimeCount >= stDisplayIn.dwTimeOut)
                    {
                        Log(ThisModule, __LINE__,
                            "摄像窗口处理: 建立窗口: 等待时间[%ld] > 指定超时时间[%ld]: 超时: Return: %s",
                            ulTimeCount, stDisplayIn.dwTimeOut, ConvertDevErrCodeToStr(ERR_CAM_TIMEOUT));
                        return ERR_CAM_TIMEOUT;
                    }
                }

                if (m_nThreadRet != IMP_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "摄像窗口处理: 建立窗口失败, ErrCode: %s, Return: %s",
                        m_pDevImpl.ConvertCode_Impl2Str(m_nThreadRet),
                        ConvertDevErrCodeToStr(ConvertImplErrCode2CAM(m_nThreadRet)));
                    return ConvertImplErrCode2CAM(m_nThreadRet);
                } else
                {
                    if (m_enDisplayStat == EN_DISP_SHOWING)
                    {
                        Log(ThisModule, __LINE__, "摄像窗口处理: 建立窗口完成...");
                        break;
                    }
                }

                usleep(1000 * 30);
            }

            return CAM_SUCCESS;
        } else                                          // 窗口暂停中
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 建立窗口: 窗口已建立暂停中, 不处理, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            return CAM_SUCCESS;
        }
    } else
    if (stDisplayIn.wAction == CAMERA_PAUSE)        // 执行窗口暂停
    {
        if (m_enDisplayStat == EN_DISP_SHOWING)         // 窗口显示中
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 摄像暂停: 窗口界面显示中, 设置为暂停, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            m_enDisplayStat = EN_DISP_PAUSE;
            return CAM_SUCCESS;
        } else
        if (m_enDisplayStat == EN_DISP_ENDING)          // 窗口未建立
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 摄像暂停: 窗口未建立, 不处理, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            return CAM_SUCCESS;
        } else                                          // 窗口暂停中
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 摄像暂停: 窗口界面处于暂停中, 不处理, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            return CAM_SUCCESS;
        }
    } else
    if (stDisplayIn.wAction == CAMERA_RESUME)       // 执行窗口恢复
    {
        if (m_enDisplayStat == EN_DISP_SHOWING)         // 窗口显示中
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 暂停恢复: 窗口界面处于显示中, 不处理, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            return CAM_SUCCESS;
        } else
        if (m_enDisplayStat == EN_DISP_ENDING)          // 窗口未建立
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 暂停恢复: 窗口未建立, 不处理, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            return CAM_SUCCESS;
        } else                                          // 窗口暂停中
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 暂停恢复: 窗口界面处于暂停中, 设置恢复显示, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            m_enDisplayStat = EN_DISP_SHOWING;
            return CAM_SUCCESS;
        }
    } else
    if (stDisplayIn.wAction == CAMERA_CLOSE)        // 执行窗口关闭
    {
        if (m_enDisplayStat == EN_DISP_SHOWING)         // 窗口显示中
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 窗口关闭: 窗口界面处于显示中, 设置关闭, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            m_enDisplayStat = EN_DISP_ENDING;
            return CAM_SUCCESS;
        } else
        if (m_enDisplayStat == EN_DISP_ENDING)          // 窗口未建立
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 窗口关闭: 窗口未建立, 不处理, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            return CAM_SUCCESS;
        } else                                          // 窗口暂停中
        {
            Log(ThisModule, __LINE__,
                "摄像窗口处理: 窗口关闭: 窗口界面处于暂停中, 设置关闭, Return: %s",
                ConvertDevErrCodeToStr(CAM_SUCCESS));
            m_enDisplayStat = EN_DISP_ENDING;
            return CAM_SUCCESS;
        }
    } else
    {
        Log(ThisModule, __LINE__,
            "摄像窗口处理: 入参Action[%d]无效, Return: %s",
            stDisplayIn.wAction, ConvertDevErrCodeToStr(ERR_CAM_PARAM_ERR));
        return ERR_CAM_PARAM_ERR;
    }

    return CAM_SUCCESS;
}

// 拍照
int CDevHCAM_JDY5001A0809::TakePicture(STTAKEPICTUREPAR stTakePicIn, void *vParam/* = nullptr*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    QTime qtTimeCurr;
    qtTimeCurr = QTime::currentTime();     // 拍照执行前时间

    // 根据超时时间，循环验证TakePic是否结束
    ULONG ulTimeCount = 0;
    while(1)
    {
        QCoreApplication::processEvents();

        if (m_bCancel == TRUE)  // 命令取消
        {
            Log(ThisModule, __LINE__, "摄像拍照: 命令取消: Return: %s",
                nRet, ConvertDevErrCodeToStr(ERR_CAM_USER_CANCEL));
            return ERR_CAM_USER_CANCEL;
        }

        if (stTakePicIn.dwTimeOut > 0)   // >0有超时;否则不超时
        {
            ulTimeCount = qtTimeCurr.msecsTo(QTime::currentTime()); // 时间差计入超时计数(毫秒)
            if (ulTimeCount >= stTakePicIn.dwTimeOut)
            {
                Log(ThisModule, __LINE__,
                    "摄像拍照: 等待时间[%ld] > 指定超时时间[%ld]: 超时: Return: %s",
                    ulTimeCount, stTakePicIn.dwTimeOut, ConvertDevErrCodeToStr(ERR_CAM_TIMEOUT));
                return ERR_CAM_TIMEOUT;
            }
        }

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
            break;
        }        

        usleep(1000 * 30);
    }

    return CAM_SUCCESS;
}

// 设置数据
int CDevHCAM_JDY5001A0809::SetData(unsigned short usType, void *vData/* = nullptr*/)
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
                m_nRefreshTime = ((LPSTINITPARAM)vData)->nParInt[0];
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
int CDevHCAM_JDY5001A0809::GetData(unsigned short usType, void *vData)
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
int CDevHCAM_JDY5001A0809::GetVersion(unsigned short usType, char* szVer, int nSize)
{

    return CAM_SUCCESS;
}

// 设置摄像参数
INT CDevHCAM_JDY5001A0809::SetVideoMode(STVIDEOPAMAR stVM)
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

// 连接共享内存
INT CDevHCAM_JDY5001A0809::ConnSharedMemory(LPSTR lpSharedName)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_qSharedMemData = new QSharedMemory(lpSharedName);
    if (m_qSharedMemData == nullptr)
    {
        return 1;
    }
    if (m_qSharedMemData->attach(QSharedMemory::ReadWrite) != true)    // 读写方式关联
    {
        if (m_qSharedMemData->isAttached() != true)   // 判断共享内存的关联状态
        {
            return 2;
        }
    }

    if (m_qSharedMemData->size() < 1)
    {
        return 3;
    }

    return CAM_SUCCESS;
}

// 断开共享内存
INT CDevHCAM_JDY5001A0809::DisConnSharedMemory()
{
    if (m_qSharedMemData != nullptr)
    {
        if (m_qSharedMemData->isAttached())   // 检测程序当前是否关联共享内存
            m_qSharedMemData->detach();      // 解除关联
        delete m_qSharedMemData;
        m_qSharedMemData = nullptr;
    }

    return CAM_SUCCESS;
}

// Impl错误码转换为CAM错误码
INT CDevHCAM_JDY5001A0809::ConvertImplErrCode2CAM(INT nRet)
{
#define CASE_RET_DEVCODE(IMP, RET) \
        case IMP: return RET;

    ConvertImplErrCode2ErrDetail(nRet);

    switch(nRet)
    {
        CASE_RET_DEVCODE(IMP_SUCCESS, CAM_SUCCESS)                          // 成功
        CASE_RET_DEVCODE(IMP_ERR_LOAD_LIB, ERR_CAM_LIBRARY)                 // 动态库加载失败
        CASE_RET_DEVCODE(IMP_ERR_PARAM_INVALID, ERR_CAM_PARAM_ERR)          // 参数无效
        CASE_RET_DEVCODE(IMP_ERR_UNKNOWN, ERR_CAM_UNKNOWN)                  // 未知错误

        CASE_RET_DEVCODE(IMP_ERR_DEVICE_NOTFOUND, ERR_CAM_NODEVICE)         // 设备未找到
        CASE_RET_DEVCODE(IMP_ERR_VIDPID_NOTFOUND, ERR_CAM_NODEVICE)         // VID/PID未找到
        CASE_RET_DEVCODE(IMP_ERR_VIDEOIDX_NOTFOUND, ERR_CAM_NODEVICE)       // 未找到摄像设备索引号
        CASE_RET_DEVCODE(IMP_ERR_DEVICE_OPENFAIL, ERR_CAM_NOTOPEN)          // 设备打开失败/未打开
        CASE_RET_DEVCODE(IMP_ERR_DEVICE_OFFLINE, ERR_CAM_OFFLINE)           // 设备断线
        CASE_RET_DEVCODE(IMP_ERR_GET_IMGDATA_FAIL, ERR_CAM_OTHER)           // 取帧图像数据失败
        CASE_RET_DEVCODE(IMP_ERR_GET_IMGDATA_ISNULL, ERR_CAM_OTHER)         // 取帧图像数据为空
        CASE_RET_DEVCODE(IMP_ERR_GET_IMGDATA_BUFFER, ERR_CAM_OTHER)         // 取帧图像数据空间异常
        CASE_RET_DEVCODE(IMP_ERR_GET_IMGDATA_CHANGE, ERR_CAM_OTHER)         // 帧图像数据转换失败
        CASE_RET_DEVCODE(IMP_ERR_SAVE_IMAGE_FILE, ERR_CAM_OTHER)            // 帧图像数据保存到文件失败
        CASE_RET_DEVCODE(IMP_ERR_SET_VMODE, ERR_CAM_OTHER)                  // 摄像参数设置错误
        CASE_RET_DEVCODE(IMP_ERR_SET_RESO, ERR_CAM_OTHER)                   // 分辨率设置失败
        CASE_RET_DEVCODE(IMP_ERR_SHAREDMEM_RW, ERR_CAM_OTHER)               // 共享内存读写失败
        default: return ERR_CAM_OTHER;
    }

    return CAM_SUCCESS;
}

// 根据Impl错误码设置错误错误码字符串
INT CDevHCAM_JDY5001A0809::ConvertImplErrCode2ErrDetail(INT nRet)
{
#define CASE_SET_DEV_DETAIL(IMP, STR) \
        case IMP: m_clErrorDet.SetDevErrCode((LPSTR)STR); break;

#define CASE_SET_HW_DETAIL(IMP, DSTR, HSTR) \
        case IMP: \
            m_clErrorDet.SetDevErrCode((LPSTR)DSTR); \
            m_clErrorDet.SetHWErrCodeStr((LPSTR)HSTR, (LPSTR)"1"); break;

    switch(nRet)
    {
        CASE_SET_DEV_DETAIL(IMP_ERR_LOAD_LIB, EC_DEV_LibraryLoadFail)       // 动态库加载失败
        CASE_SET_DEV_DETAIL(IMP_ERR_PARAM_INVALID, EC_DEV_ParInvalid)       // 参数无效
        CASE_SET_DEV_DETAIL(IMP_ERR_UNKNOWN, EC_DEV_UnKnownErr)             // 未知错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVICE_NOTFOUND, EC_DEV_DevNotFound)    // 设备未找到
        CASE_SET_DEV_DETAIL(IMP_ERR_VIDPID_NOTFOUND, EC_DEV_DevNotFound)    // VID/PID未找到
        CASE_SET_DEV_DETAIL(IMP_ERR_VIDEOIDX_NOTFOUND, EC_DEV_DevNotFound)  // 未找到摄像设备索引号
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVICE_OPENFAIL, EC_DEV_DevOpenFail)    // 设备打开失败/未打开
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVICE_OFFLINE, EC_DEV_DevOffLine)      // 设备断线
        CASE_SET_DEV_DETAIL(IMP_ERR_GET_IMGDATA_FAIL, EC_CAM_DEV_GetImageErr)// 取帧图像数据失败
        CASE_SET_DEV_DETAIL(IMP_ERR_GET_IMGDATA_ISNULL, EC_CAM_DEV_GetImageErr)// 取帧图像数据为空
        CASE_SET_DEV_DETAIL(IMP_ERR_GET_IMGDATA_BUFFER, EC_CAM_DEV_GetImageErr)// 取帧图像数据空间异常
        CASE_SET_DEV_DETAIL(IMP_ERR_GET_IMGDATA_CHANGE, EC_CAM_DEV_GetImageErr)// 帧图像数据转换失败
        CASE_SET_DEV_DETAIL(IMP_ERR_SAVE_IMAGE_FILE, EC_CAM_DEV_SaveImageErr)// 帧图像数据保存到文件失败
        CASE_SET_DEV_DETAIL(IMP_ERR_SET_VMODE, EC_DEV_OtherErr)             // 摄像参数设置错误
        CASE_SET_DEV_DETAIL(IMP_ERR_SET_RESO, EC_DEV_OtherErr)              // 分辨率设置失败
        CASE_SET_DEV_DETAIL(IMP_ERR_SHAREDMEM_RW, EC_DEV_ShareMemRW)        // 共享内存读写失败
    }

    return CAM_SUCCESS;
}

// 窗口处理进程
void CDevHCAM_JDY5001A0809::ThreadRunDisplay(WORD wDisplayW, WORD wDisplayH)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;
    INT nRetOLD = IMP_SUCCESS;
    INT nRetErrCount = 0;
    INT nWidthOLD = 0, nHeightOLD = 0, nFormatOLD = 0;

    LPSTIMGDATA stImgData = new ST_IMAGE_DATA();
    stImgData->Clear();

    EN_DISPLAY_STAT enDispStatOLD;

    Log(ThisModule, __LINE__,
        "窗口处理进程[%ld]启动...", std::this_thread::get_id());

    while(1)
    {
        if (m_enDisplayStat == EN_DISP_ENDING)  // 窗口状态为结束
        {
            Log(ThisModule, __LINE__,
                "窗口处理进程[%ld]: 窗口状态为结束[%d],进程退出.",
                std::this_thread::get_id(), m_enDisplayStat);
            enDispStatOLD = EN_DISP_ENDING;
            break;
        } else
        if (m_enDisplayStat == EN_DISP_PAUSE)   // 窗口状态为暂停
        {
            if (enDispStatOLD != m_enDisplayStat)
            {
                Log(ThisModule, __LINE__,
                    "窗口处理进程[%ld]: 窗口状态为暂停[%d], 窗口暂停显示.",
                    std::this_thread::get_id(), m_enDisplayStat);
                enDispStatOLD = m_enDisplayStat;
            }
            usleep(1000 * m_nRefreshTime);
            continue;
        } else
        {
            nRet = m_pDevImpl.GetVideoImage(stImgData, wDisplayW, wDisplayH, m_nClipMode);
            if (nRet != IMP_SUCCESS)
            {
                nRetErrCount ++;
                if (nRetErrCount > 5)
                {
                    Log(ThisModule, __LINE__,
                        "窗口处理进程[%ld]: 窗口状态为显示[%d]: 获取帧数据: ->GetVideoImage() Fail(%d次), ErrCode: %s, 线程结束.",
                        std::this_thread::get_id(), m_enDisplayStat, nRetErrCount,
                        m_pDevImpl.ConvertCode_Impl2Str(nRet));
                    m_nThreadRet = nRet;
                    // 设置错误事件参数
                    if (m_enDisplayStat == EN_DISP_SHOWING)
                    {
                        m_nSaveStat[0] = 1;     // 摄像窗口(Display)
                        m_nSaveStat[1] = EN_EVENTID_SOFTERR;    // 软件错误
                        m_nSaveStat[2] = EN_EVENTACT_NOACTION;  // 可自动恢复
                    }
                    break;
                }

            } else
            {
                nRetErrCount = 0;
                if (nWidthOLD != stImgData->nWidth || nHeightOLD != stImgData->nHeight ||
                    nFormatOLD != stImgData->nFormat)
                {
                    Log(ThisModule, __LINE__,
                        "窗口处理进程[%ld]: 图像数据参数变化: 宽[%d->%d], 高[%d->%d], 格式[%d->%d]",
                        std::this_thread::get_id(), nWidthOLD, stImgData->nWidth, nHeightOLD,
                        stImgData->nHeight, nFormatOLD,stImgData->nFormat);
                    nWidthOLD = stImgData->nWidth;
                    nHeightOLD = stImgData->nHeight;
                    nFormatOLD = stImgData->nFormat;
                }

                if (m_qSharedMemData != nullptr)
                {
                    // 检查共享内存空间
                    if (m_qSharedMemData->size() < stImgData->ulImagDataLen)
                    {
                        m_enDisplayStat = EN_DISP_ENDING;
                        m_nThreadRet = IMP_ERR_SHAREDMEM_RW;
                        Log(ThisModule, __LINE__,
                            "窗口处理进程[%ld]: 窗口状态为显示[%d]: 共享内存空间[%ld]无法容纳图像数据[%ld], ErrCode: %s, 线程结束.",
                            std::this_thread::get_id(), m_enDisplayStat, m_qSharedMemData->size(), stImgData->ulImagDataLen,
                            m_pDevImpl.ConvertCode_Impl2Str(m_nThreadRet));

                        // 设置错误事件参数
                        if (m_enDisplayStat == EN_DISP_SHOWING)
                        {
                            m_nSaveStat[0] = 1;     // 摄像窗口(Display)
                            m_nSaveStat[1] = EN_EVENTID_SOFTERR;    // 软件错误
                            m_nSaveStat[2] = EN_EVENTACT_CONFIG;    // 检查配置
                        }
                        break;
                    }

                    if (m_qSharedMemData->isAttached())
                    {
                        memcpy(m_qSharedMemData->data(), stImgData->ucImgData, stImgData->ulImagDataLen);
                        m_qSharedMemData->unlock();
                    }
                } else
                {
                    // 共享内存为空,结束线程
                    m_enDisplayStat = EN_DISP_ENDING;
                    m_nThreadRet = IMP_ERR_SHAREDMEM_RW;
                    Log(ThisModule, __LINE__,
                        "窗口处理进程[%ld]: 窗口状态为显示[%d]: 共享内存实例为null, ErrCode: %s, 线程结束.",
                        std::this_thread::get_id(), m_enDisplayStat,
                        m_pDevImpl.ConvertCode_Impl2Str(m_nThreadRet));
                    // 设置错误事件参数
                    if (m_enDisplayStat == EN_DISP_SHOWING)
                    {
                        m_nSaveStat[0] = 1;     // 摄像窗口(Display)
                        m_nSaveStat[1] = EN_EVENTID_SOFTERR;    // 软件错误
                        m_nSaveStat[2] = EN_EVENTACT_CONFIG;    // 检查配置
                    }
                    break;
                }

                if (m_enDisplayStat == EN_DISP_CREATE)
                {
                    m_enDisplayStat = EN_DISP_SHOWING;
                }
                m_nThreadRet = IMP_SUCCESS;
            }
        }

        usleep(1000 * m_nRefreshTime);
    }

    delete stImgData;
    m_enDisplayStat = EN_DISP_ENDING;

    return;
}

