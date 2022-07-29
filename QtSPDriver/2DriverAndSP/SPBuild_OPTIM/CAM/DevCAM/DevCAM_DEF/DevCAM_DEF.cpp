/***************************************************************************
* 文件名称: DevHCAM_JDY5001A0809.cpp
* 文件描述：摄像功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年6月6日
* 文件版本：1.0.0.1
****************************************************************************/

#include "DevCAM_DEF.h"

CDevCAM_DEF::CDevCAM_DEF()
{
    m_enDisplayStat = EN_DISP_ENDING;                   // Display命令状态
    m_qSharedMemData = nullptr;                         // 共性内存句柄
    MSET_0(m_szSharedDataName);                         // 共享内存名
    m_ulSharedDataSize = 0;                             // 共享内存大小
    m_bCancel = FALSE;                                  // 是否下发取消命令
    m_nRefreshTime = 30;                                // 摄像数据获取间隔
    MSET_XS(m_nSaveStat, 0, sizeof(INT) * 12);          // 保存必要的状态信息
    m_wDisplayOpenMode = DISP_OPEN_MODE_THREAD;         // Display窗口打开模式
    m_nDisplayGetVideoMaxErrCnt = 500;    // display采用图像帧方式刷新时,取图像帧数据接口错误次数上限
}

CDevCAM_DEF::~CDevCAM_DEF()
{
    m_enDisplayStat = EN_DISP_ENDING;
    DisConnSharedMemory();
}

// 摄像窗口处理
INT CDevCAM_DEF::InnerDisplay(STDISPLAYPAR stDisplayIn, void *vParam/* = nullptr*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

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

            // 摄像窗口打开前处理
            nRet = VideoCameraOpenFrontRun(stDisplayIn);
            if (nRet != CAM_SUCCESS)
            {
                return nRet;
            }

            // 打开设备摄像画面
            nRet = VideoCameraOpen(stDisplayIn);
            if (nRet != CAM_SUCCESS)
            {
                return nRet;
            }

            // Display窗口打开模式为线程方式
            if (m_wDisplayOpenMode == DISP_OPEN_MODE_THREAD)
            {
                // 启动窗口处理线程
                m_nThreadRet = CAM_SUCCESS;
                m_enDisplayStat = EN_DISP_CREATE;
                if (!m_thRunDisplay.joinable())
                {
                    m_thRunDisplay = std::thread(&CDevCAM_DEF::ThreadRunDisplay, this,
                                                 stDisplayIn.wWidth, stDisplayIn.wHeight,
                                                 m_nRefreshTime);
                    if (m_thRunDisplay.joinable())
                    {
                        m_thRunDisplay.detach();   // 进程分离
                    } else
                    {
                        Log(ThisModule, __LINE__,
                            "摄像窗口处理: 建立窗口: 窗口未建立, 创建窗口处理线程失败: Return: %s",
                            ConvertDevErrCodeToStr(ERR_CAM_OTHER));
                        m_enDisplayStat = EN_DISP_ENDING;
                        VideoCameraClose();     // 关闭设备摄像画面
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
                            VideoCameraClose(); // 关闭设备摄像画面
                            return ERR_CAM_TIMEOUT;
                        }
                    }

                    if (m_nThreadRet != CAM_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "摄像窗口处理: 建立窗口失败, ErrCode: %s, Return: %s",
                            ConvertDevErrCodeToStr(m_nThreadRet), ConvertDevErrCodeToStr(m_nThreadRet));
                        VideoCameraClose(); // 关闭设备摄像画面
                        return m_nThreadRet;
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
            VideoCameraPause(); // 暂停设备摄像画面
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
            VideoCameraResume();    // 恢复设备摄像画面
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
            VideoCameraClose();     // 关闭设备摄像画面
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
            VideoCameraClose();     // 关闭设备摄像画面
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
INT CDevCAM_DEF::InnerTakePicture(STTAKEPICTUREPAR stTakePicIn, void *vParam/* = nullptr*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    QTime qtTimeCurr;
    qtTimeCurr = QTime::currentTime();     // 拍照执行前时间

    // 拍照前运行处理
    nRet = TakePicFrontRun(stTakePicIn);
    if (nRet != CAM_SUCCESS)
    {
        return nRet;
    }

    // 根据超时时间，循环验证TakePic是否结束
    ULONG ulTimeCount = 0;
    while(1)
    {
        QCoreApplication::processEvents();

        if (m_bCancel == TRUE)  // 命令取消
        {
            Log(ThisModule, __LINE__, "摄像拍照: 命令取消: Return: %s",
                nRet, ConvertDevErrCodeToStr(ERR_CAM_USER_CANCEL));
            TakePicAfterRun(stTakePicIn);
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
                TakePicAfterRun(stTakePicIn);
                return ERR_CAM_TIMEOUT;
            }
        }

        // 获取检测结果
        if (GetLiveDetectResult() == TRUE)
        {
            nRet = TakePicSaveImage(stTakePicIn);
            if (nRet != CAM_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "摄像拍照: ->SaveImageFile(%s) Fail, ErrCode: %s, Return: %s",
                    stTakePicIn.szFileName, ConvertDevErrCodeToStr(nRet),
                    ConvertDevErrCodeToStr(nRet));
                TakePicAfterRun(stTakePicIn);
                return nRet;
            } else
            {
                Log(ThisModule, __LINE__, "摄像拍照: ->SaveImageFile(%s) Succ",
                    stTakePicIn.szFileName);
                break;
            }
        } else
        {
            ;
        }

        usleep(1000 * 30);
    }

    TakePicAfterRun(stTakePicIn);

    return CAM_SUCCESS;
}


// 连接共享内存
INT CDevCAM_DEF::ConnSharedMemory(LPSTR lpSharedName)
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
INT CDevCAM_DEF::DisConnSharedMemory()
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
INT CDevCAM_DEF::DEF_ConvertImplErrCode2CAM(INT nRet)
{
#define DEF_CASE_RET_DEVCODE(IMP, RET) \
        case IMP: return RET;

    DEF_ConvertImplErrCode2ErrDetail(nRet);

    switch(nRet)
    {
        DEF_CASE_RET_DEVCODE(IMP_SUCCESS, CAM_SUCCESS)                      // 成功
        DEF_CASE_RET_DEVCODE(IMP_ERR_LOAD_LIB, ERR_CAM_LIBRARY)             // 动态库加载错误
        DEF_CASE_RET_DEVCODE(IMP_ERR_PARAM_INVALID, ERR_CAM_PARAM_ERR)      // 参数无效
        DEF_CASE_RET_DEVCODE(IMP_ERR_UNKNOWN, ERR_CAM_UNKNOWN)              // 未知错误
        DEF_CASE_RET_DEVCODE(IMP_ERR_NOTOPEN, ERR_CAM_NOTOPEN)              // 设备未Open
        DEF_CASE_RET_DEVCODE(IMP_ERR_OPENFAIL, ERR_CAM_OPENFAIL)            // 设备打开失败/未打开
        DEF_CASE_RET_DEVCODE(IMP_ERR_NODEVICE, ERR_CAM_NODEVICE)            // 未找到有效设备
        DEF_CASE_RET_DEVCODE(IMP_ERR_OFFLINE, ERR_CAM_OFFLINE)              // 设备断线
        DEF_CASE_RET_DEVCODE(IMP_ERR_NODETECT, ERR_CAM_LIVEDETECT)          // 未检测到活体
        DEF_CASE_RET_DEVCODE(IMP_ERR_INTEREXEC, ERR_CAM_OTHER)              // 接口执行错误
        DEF_CASE_RET_DEVCODE(IMP_ERR_SET_VMODE, ERR_CAM_OTHER)              // 摄像参数设置错误
        DEF_CASE_RET_DEVCODE(IMP_ERR_SET_RESO, ERR_CAM_OTHER)               // 分辨率设置失败
        DEF_CASE_RET_DEVCODE(IMP_ERR_SHAREDMEM_RW, ERR_CAM_OTHER)           // 共享内存读写失败
        DEF_CASE_RET_DEVCODE(IMP_ERR_MEMORY, ERR_CAM_OTHER)                 // 内存错误
        default: return ERR_CAM_OTHER;
    }

    return CAM_SUCCESS;
}

// 根据Impl错误码设置错误错误码字符串
INT CDevCAM_DEF::DEF_ConvertImplErrCode2ErrDetail(INT nRet)
{
#define CASE_SET_DEV_DETAIL(IMP, STR) \
        case IMP: m_clErrorDet.SetDevErrCode((LPSTR)STR); break;

    switch(nRet)
    {
        CASE_SET_DEV_DETAIL(IMP_ERR_LOAD_LIB, EC_DEV_LibraryLoadFail)       // 动态库加载错误
        CASE_SET_DEV_DETAIL(IMP_ERR_PARAM_INVALID, EC_DEV_ParInvalid)       // 参数无效
        CASE_SET_DEV_DETAIL(IMP_ERR_UNKNOWN, EC_DEV_UnKnownErr)             // 未知错误
        CASE_SET_DEV_DETAIL(IMP_ERR_NOTOPEN, EC_DEV_DevOffLine)             // 设备未Open
        CASE_SET_DEV_DETAIL(IMP_ERR_OPENFAIL, EC_DEV_DevOpenFail)           // 设备打开失败/未打开
        CASE_SET_DEV_DETAIL(IMP_ERR_NODEVICE, EC_DEV_DevNotFound)           // 未找到有效设备
        CASE_SET_DEV_DETAIL(IMP_ERR_OFFLINE, EC_DEV_DevOffLine)             // 设备断线
        CASE_SET_DEV_DETAIL(IMP_ERR_NODETECT, EC_CAM_DEV_NotLiveDetect)     // 未检测到活体
        CASE_SET_DEV_DETAIL(IMP_ERR_INTEREXEC, EX_DEV_InTerExec)            // 接口执行错误
        CASE_SET_DEV_DETAIL(IMP_ERR_SET_VMODE, EC_DEV_OtherErr)             // 摄像参数设置错误
        CASE_SET_DEV_DETAIL(IMP_ERR_SET_RESO, EC_DEV_OtherErr)              // 分辨率设置失败
        CASE_SET_DEV_DETAIL(IMP_ERR_SHAREDMEM_RW, EC_DEV_ShareMemRW)        // 共享内存读写失败
        CASE_SET_DEV_DETAIL(IMP_ERR_MEMORY, EC_DEV_MemoryErrr)              // 内存错误
    }

    return CAM_SUCCESS;
}


// 摄像窗口打开前处理
INT CDevCAM_DEF::VideoCameraOpenFrontRun(STDISPLAYPAR stDisplayIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 打开设备摄像画面
INT CDevCAM_DEF::VideoCameraOpen(STDISPLAYPAR stDisplayIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 关闭设备摄像画面
INT CDevCAM_DEF::VideoCameraClose()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 暂停设备摄像画面
INT CDevCAM_DEF::VideoCameraPause()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 恢复设备摄像画面
INT CDevCAM_DEF::VideoCameraResume()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 获取窗口显示数据
INT CDevCAM_DEF::GetViewImage(LPSTIMGDATA lpImgData, INT nWidth, INT nHeight, DWORD dwParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 拍照前运行处理
INT CDevCAM_DEF::TakePicFrontRun(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 拍照后运行处理
INT CDevCAM_DEF::TakePicAfterRun(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}



// 获取检测结果
BOOL CDevCAM_DEF::GetLiveDetectResult()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 保存图像
INT CDevCAM_DEF::TakePicSaveImage(STTAKEPICTUREPAR stTakePicIn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return CAM_SUCCESS;
}

// 窗口处理进程
void CDevCAM_DEF::ThreadRunDisplay(WORD wDisplayW, WORD wDisplayH, DWORD dwRefreshTime)
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
        "窗口处理进程[%ld]启动, 图像获取间隔[%d]毫秒...", std::this_thread::get_id(), dwRefreshTime);

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
            usleep(1000 * dwRefreshTime);
            continue;
        } else
        {
            //nRet = m_pDevImpl.GetVideoImage(stImgData, wDisplayW, wDisplayH, m_nClipMode);
            nRet = GetViewImage(stImgData, wDisplayW, wDisplayH, 0);
            if (nRet != IMP_SUCCESS)
            {
                nRetErrCount ++;
                if (nRetErrCount > m_nDisplayGetVideoMaxErrCnt)
                {
                    Log(ThisModule, __LINE__,
                        "窗口处理进程[%ld]: 窗口状态为显示[%d]: 获取帧数据: ->GetViewImage() Fail, "
                        "错误次数%d超过上限%d, ErrCode: %s, 线程结束.",
                        std::this_thread::get_id(), m_enDisplayStat, nRetErrCount,
                        m_nDisplayGetVideoMaxErrCnt, ConvertDevErrCodeToStr(nRet));
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
                        m_nThreadRet = ERR_CAM_SHAREDMEM;
                        Log(ThisModule, __LINE__,
                            "窗口处理进程[%ld]: 窗口状态为显示[%d]: 共享内存空间[%ld]无法容纳图像数据[%ld], ErrCode: %s, 线程结束.",
                            std::this_thread::get_id(), m_enDisplayStat, m_qSharedMemData->size(), stImgData->ulImagDataLen,
                            ConvertDevErrCodeToStr(m_nThreadRet));

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
                    m_nThreadRet = ERR_CAM_SHAREDMEM;
                    Log(ThisModule, __LINE__,
                        "窗口处理进程[%ld]: 窗口状态为显示[%d]: 共享内存实例为null, ErrCode: %s, 线程结束.",
                        std::this_thread::get_id(), m_enDisplayStat,
                        ConvertDevErrCodeToStr(m_nThreadRet));
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
                m_nThreadRet = CAM_SUCCESS;
            }
        }

        usleep(1000 * dwRefreshTime);
    }

    delete stImgData;
    m_enDisplayStat = EN_DISP_ENDING;

    return;
}

// -------------------------------------- END --------------------------------------
