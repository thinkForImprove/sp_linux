/***************************************************************
* 文件名称: DevFIDC_CRT603CZ7.cpp
* 文件描述: 非接模块功能处理接口封装
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2021年10月20日
* 文件版本: 1.0.0.1
****************************************************************/

#include "DevFIDC_CRT603CZ7.h"
#include <unistd.h>

static const char *ThisFile = "DevFIDC_CRT603CZ7.cpp";

/****************************************************************************
*     对外接口调用处理                                                         *
****************************************************************************/
CDevFIDC_CRT603CZ7::CDevFIDC_CRT603CZ7(LPCSTR lpDevType) : m_pDevImpl(LOG_NAME_DEV)
{
    SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件

    m_bCancelReadCard = FALSE;                      // 取消读卡标记初始化:F
    m_bICActive = FALSE;                            // 卡片是否处于激活中初始化:F
}

CDevFIDC_CRT603CZ7::~CDevFIDC_CRT603CZ7()
{
    Close();
}

// 释放接口
int CDevFIDC_CRT603CZ7::Release()
{
    return IDC_SUCCESS;
}

// 打开与设备的连接
int CDevFIDC_CRT603CZ7::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    // AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;
    if ((nRet = m_pDevImpl.OpenDevice((LPSTR)pMode)) != IMP_SUCCESS)
    {
        /*Log(ThisModule, __LINE__,
            "打开设备: ->OpenDevice(%s) Fail, ErrCode: %d, Return: %s",
            pMode, nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));*/
        return ConvertCode_Impl2IDC(nRet);
    }

    return IDC_SUCCESS;
}

// 关闭与设备的连接
int CDevFIDC_CRT603CZ7::Close()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_pDevImpl.CloseDevice();
    return IDC_SUCCESS;
}

// 取消
int CDevFIDC_CRT603CZ7::Cancel(unsigned short usMode)
{
    THISMODULE(__FUNCTION__);

    if (usMode == 0)
    {
        Log(ThisModule, __LINE__, "设置取消读卡: usMode = %d.", usMode);
        m_bCancelReadCard = TRUE;   // 取消读卡
    }

    return IDC_SUCCESS;
}

// 设备复位
int CDevFIDC_CRT603CZ7::Reset(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    if(enMediaAct == MEDIA_EJECT)   // 介质退出
    {
        STDEVIDCSTATUS stDevStat;
        nRet = GetStatus(stDevStat);
        if(nRet == IDC_SUCCESS && stDevStat.wMedia == MEDIA_STAT_ENTERING)  // 介质在内部,执行退出
        {
            nRet = MediaControl(enMediaAct);
            if(nRet != IDC_SUCCESS)
            {
                Log(ThisModule, __LINE__, "复位: 退卡: ->MeidaControl(%d) Fail, ErrCode: %d, Return: %s",
                    enMediaAct, nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                return ConvertCode_Impl2IDC(nRet);
            }
        }

        //ReaderReset
    }

    return IDC_SUCCESS;
}

// 取设备状态
int CDevFIDC_CRT603CZ7::GetStatus(STDEVIDCSTATUS &stStatus)
{
    //THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);

    //INT nRet = IDC_SUCCESS;

    // 取设备状态
    if (m_pDevImpl.IsDeviceOpen() != TRUE)
    {
        m_bICActive = FALSE;
        stStatus.wDevice = DEVICE_STAT_OFFLINE;
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        stStatus.wChipPower = CHIP_STAT_UNKNOWN;
        return IMP_SUCCESS;
    }

    stStatus.wDevice = DEVICE_STAT_ONLINE;

    // 取卡片状态
    INT nCardStat = 0;
    nCardStat = m_pDevImpl.GetCardStatus();
    if (nCardStat < IMP_SUCCESS)
    {
        m_bICActive = FALSE;
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        stStatus.wDevice = DEVICE_STAT_OFFLINE;    // 返回值非0认为断线
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        return ConvertCode_Impl2IDC(nCardStat);
        //return ERR_IDC_DEV_HWERR;
    }

    // 卡状态Check
    switch(nCardStat)
    {
        case CARD_STAT_HAVE:        // 有卡
        {
            if (m_bICActive != TRUE)    // 非激活状态
            {
                stStatus.wMedia = MEDIA_STAT_ENTERING;      // 介质在出口
                stStatus.wChipPower = CHIP_STAT_POWEREDOFF; // 存在未上电
            } else
            {
                stStatus.wMedia = MEDIA_STAT_PRESENT;       // 介质在内部
                stStatus.wChipPower = CHIP_STAT_ONLINE;     // 存在并上电
            }
            break;
        }
        case CARD_STAT_NOHAVE:      // 无卡
        {
            m_bICActive = FALSE;
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;        // 无卡
            stStatus.wChipPower = CHIP_STAT_NOCARD;         // 无卡
            break;
        }
        case CARD_STAT_UNKNOWN:     // 未知状态
        {
            m_bICActive = FALSE;
            stStatus.wDevice = DEVICE_STAT_OFFLINE;         // 断线
            stStatus.wMedia = MEDIA_STAT_UNKNOWN;           // 状态未知
            stStatus.wChipPower = CHIP_STAT_UNKNOWN;        // 状态未知
            break;
        }
        default:
            m_bICActive = FALSE;
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;        // 无卡
            stStatus.wChipPower = CHIP_STAT_NOCARD;         // 无卡
            break;
    }

    return IDC_SUCCESS;
}

// 介质控制
int CDevFIDC_CRT603CZ7::MediaControl(MEDIA_ACTION enMediaAct, unsigned long ulParam)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    if(enMediaAct == MEDIA_EJECT)   // 介质退出
    {
        nRet = m_pDevImpl.SetEjectCard();
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "介质控制: 退卡: 断电: ->SetEjectCard() Fail, ErrCode: %d, Return: %s",
                nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
            return ConvertCode_Impl2IDC(nRet);
        }
    } else
    if (enMediaAct == MEDIA_ACCEPT_IC)      // IC卡进卡
    {
        return AcceptMedia((DWORD)ulParam, 0);
    } else
    if (enMediaAct == MEDIA_ACCEPT_IDCARD)  // 身份证进卡
    {
        return AcceptMedia((DWORD)ulParam, 1);
    } else
    {
        Log(ThisModule, __LINE__, "介质控制: 不支持的入参[%d], Return: %s",
            enMediaAct, ConvertDevErrCodeToStr(ERR_IDC_PARAM_ERR));
        return ERR_IDC_PARAM_ERR;
    }

    return IDC_SUCCESS;
}

// 介质读写
int CDevFIDC_CRT603CZ7::MediaReadWrite(MEDIA_RW_MODE enRWMode, STMEDIARW &stMediaData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;

    if (enRWMode == MEDIA_READ) // 读
    {
        if ((stMediaData.dwRWType & RW_CHIP) == RW_CHIP) // 读芯片(激活上电)
        {
            nRet = m_pDevImpl.SetReaderPowerOn((UCHAR*)stMediaData.stData[3].szData, (INT&)stMediaData.stData[3].wSize);
            if(nRet != IDC_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "介质读写: 读芯片: 上电: ->SetReaderPowerOn() Fail, ErrCode: %d, Return: %d",
                    nRet, ConvertCode_Impl2IDC(nRet));
                return ConvertCode_Impl2IDC(nRet);
            }
            stMediaData.stData[3].wResult = RW_RESULT_SUCC;
            m_bICActive = TRUE;
        } else
        if ((stMediaData.dwRWType & RW_FRONTIMAGE) == RW_FRONTIMAGE ||
            (stMediaData.dwRWType & RW_BACKIMAGE) == RW_BACKIMAGE)       // 读身份证
        {
            return ReadIDCardData(stMediaData);
        } else
        {
            Log(ThisModule, __LINE__, "介质读写: 不支持的入参[stMediaData.dwRWType = %d], Return: %s",
                stMediaData.dwRWType, ConvertDevErrCodeToStr(ERR_IDC_PARAM_ERR));
            return ERR_IDC_PARAM_ERR;
        }
    } else
    {
        Log(ThisModule, __LINE__, "介质读写: 不支持的入参[MEDIA_RW_MODE = %d], Return: %s",
            enRWMode, ConvertDevErrCodeToStr(ERR_IDC_PARAM_ERR));
        return ERR_IDC_PARAM_ERR;
    }

    return IDC_SUCCESS;
}

// 芯片读写
int CDevFIDC_CRT603CZ7::ChipReadWrite(CHIP_RW_MODE enChipMode, STCHIPRW &stChipData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    if (enChipMode == CHIP_POW_COLD ||  // 冷复位
        enChipMode == CHIP_POW_WARM)    // 热复位
    {
        nRet = m_pDevImpl.SetReaderPowerOn((UCHAR*)stChipData.stData[1].szData, (INT&)stChipData.stData[1].dwSize);
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: 冷复位|热复位: 上电: ->SetReaderPowerOn() Fail, ErrCode: %d, Return: %d",
                nRet, ConvertCode_Impl2IDC(nRet));
            return ConvertCode_Impl2IDC(nRet);
        }
        m_bICActive = TRUE;
    } else
    if (enChipMode == CHIP_POW_OFF)     // 断电
    {
        nRet = m_pDevImpl.SetEjectCard();
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: 断电: ->SetEjectCard() Fail, ErrCode: %d, Return: %s",
                nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
            return ConvertCode_Impl2IDC(nRet);
        }
        m_bICActive = FALSE;
    } else
    if (enChipMode == CHIP_IO_T0 ||     // T0协议通信
        enChipMode == CHIP_IO_T1)       // T1协议通信
    {
        nRet = m_pDevImpl.SendReaderAPDU((UCHAR*)stChipData.stData[0].szData, stChipData.stData[0].dwSize,
                                         (UCHAR*)stChipData.stData[1].szData, (INT&)stChipData.stData[1].dwSize);
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: T0协议通信|T1协议通信: ->SendReaderAPDU(%s, %d) Fail, ErrCode: %d, Return: %d",
                stChipData.stData[0].szData, stChipData.stData[0].dwSize,
                nRet, ConvertCode_Impl2IDC(nRet));
            return ConvertCode_Impl2IDC(nRet);
        }
    } else
    {
        Log(ThisModule, __LINE__, "芯片读写: 不支持的入参[enChipMode = %d], Return: %s",
            enChipMode, ConvertDevErrCodeToStr(ERR_IDC_PARAM_ERR));
        return ERR_IDC_PARAM_ERR;
    }

    return IDC_SUCCESS;
}

// 等待放卡处理
INT CDevFIDC_CRT603CZ7::AcceptMedia(DWORD dwTimeOut, WORD wCardType)
{
    THISMODULE(__FUNCTION__);

    ULONG ulTimeCount = 0;
    STDEVIDCSTATUS stDevStat;
    INT nRet = IMP_SUCCESS;

    QTime qtTimeCurr1, qtTimeCurr2;
    qtTimeCurr1 = QTime::currentTime();     // 取吸卡执行前时间

    // 设置读卡器操作模式: RF模式/身份证模式
    /*if (wCardType == 0)
    {
        m_pDevImpl.SetReaderMode(CARDER_MODE_RF);
    } else
    {
        m_pDevImpl.SetReaderMode(CARDER_MODE_IDCARD);
    }*/

    // 循环检测是否放入卡
    while(1)
    {
        QCoreApplication::processEvents();
        if (m_bCancelReadCard == TRUE)      // 取消读卡
        {
            m_bCancelReadCard = FALSE;
            return ERR_IDC_USER_CANCEL;
        }

        // 取当前状态
        nRet = GetStatus(stDevStat);
        if(nRet == IDC_SUCCESS)
        {
            if (stDevStat.wMedia == MEDIA_STAT_PRESENT ||   // 介质在内部
                stDevStat.wMedia == MEDIA_STAT_ENTERING)    // 介质在出口
            {
                return IDC_SUCCESS;
            }
        } else
        {
            Log(ThisModule, __LINE__,
                "介质控制: 进卡: 取当前状态: ->GetStatus() Fail, ErrCode=%d, Return: %s",
                nRet, ConvertDevErrCodeToStr(nRet));
            return nRet;
        }

        qtTimeCurr2 = QTime::currentTime();     // 取吸卡执行后时间
        if (dwTimeOut > 0)
        {
            ulTimeCount = qtTimeCurr1.msecsTo(qtTimeCurr2); // 时间差计入超时计数(毫秒)
        }

        // 检查超时
        if (dwTimeOut > 0)  // 指定超时时间
        {
            if (ulTimeCount >= dwTimeOut)   // 超过超时时间
            {
                Log(ThisModule, __LINE__,
                    "介质控制: 进卡: 已等待时间[%d] > 指定超时时间[%d], TimeOut, Return: %s",
                    ulTimeCount, dwTimeOut, ConvertDevErrCodeToStr(ERR_IDC_INSERT_TIMEOUT));
                return ERR_IDC_INSERT_TIMEOUT;
            }
        }

        usleep(1000 * 100);     // 休眠100毫秒

        continue;
    }

    return IDC_SUCCESS;
}

// 读身份证数据
INT CDevFIDC_CRT603CZ7::ReadIDCardData(STMEDIARW &stMediaData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    // 11. 设置读卡器操作模式: 身份证模式
    nRet = m_pDevImpl.SetReaderMode(CARDER_MODE_IDCARD);
    if(nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "设置读卡器操作模式: 身份证模式: ->SetReaderMode(%d) Fail, ErrCode: %d, Return: %d",
            CARDER_MODE_IDCARD, nRet, ConvertCode_Impl2IDC(nRet));
        return ConvertCode_Impl2IDC(nRet);
    }

    // 设置二代证头像保存路径
    CHAR szHeadImg[256] = { 0x00 };
    sprintf(szHeadImg, "%s/%s",
            m_stImageParam.szIDCardImgSavePath, m_stImageParam.szIDCardImgName);
    nRet = m_pDevImpl.SetIDCardHeadImgPath(szHeadImg);
    if(nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "设置二代证头像保存路径: ->SetIDCardHeadImgPath(%s) Fail, ErrCode: %d, Return: %d",
            szHeadImg, nRet, ConvertCode_Impl2IDC(nRet));
        return ConvertCode_Impl2IDC(nRet);
    }

    // 29. 读二代证信息
    STCRTDEFIDINFO stIDCardData;
    nRet = m_pDevImpl.GetIDCardInfo(&stIDCardData);
    if(nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "读二代证信息: ->GetIDCardInfo() Fail, ErrCode: %d, Return: %d",
            nRet, ConvertCode_Impl2IDC(nRet));
        return ConvertCode_Impl2IDC(nRet);
    }

    // 33. 获取二代证指纹信息
    BYTE byFingerData[1024 * 2] = { 0x00 };
    INT nFingerLen = 0;
    nRet = m_pDevImpl.GetIDCardFinger(byFingerData, nFingerLen);
    if(nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取二代证指纹信息: ->GetIDCardFinger() Fail, ErrCode: %d, Return: %d",
            nRet, ConvertCode_Impl2IDC(nRet));
        return ConvertCode_Impl2IDC(nRet);
    }

    CHAR szIDCardBuffer[1024 * 10] = { 0x00 };
    MSET_0(stIDCardData.szImgData);
    MCPY_NOLEN(stIDCardData.szImgData, szHeadImg);
    bIDCardDataToBuff(stIDCardData, (LPCSTR)byFingerData, nFingerLen, szIDCardBuffer);

    if ((stMediaData.dwRWType & RW_FRONTIMAGE) == RW_FRONTIMAGE)
    {
        memcpy(stMediaData.stData[4].szData, szIDCardBuffer, strlen(szIDCardBuffer));
        stMediaData.stData[4].wSize = strlen(szIDCardBuffer);
        stMediaData.stData[4].wResult = RW_RESULT_SUCC;
    } else
    {
        memcpy(stMediaData.stData[5].szData, szIDCardBuffer, strlen(szIDCardBuffer));
        stMediaData.stData[5].wSize = strlen(szIDCardBuffer);
        stMediaData.stData[5].wResult = RW_RESULT_SUCC;
    }

    return IDC_SUCCESS;
}

// 设置数据
int CDevFIDC_CRT603CZ7::SetData(unsigned short usType, void *vData)
{
    THISMODULE(__FUNCTION__);

    switch(usType)
    {
        case SET_LIB_PATH:          // 设置动态库路径
        {
            if (vData != nullptr)
            {
                m_pDevImpl.SetLibPath((LPCSTR)vData);
            }
            break;
        }
        case SET_DEV_RECON:         // 设置断线重连标记为T
        {
            return m_pDevImpl.SetReConFlag(TRUE);
        }
        case SET_DEV_OPENMODE:      // 设置设备打开模式
        {
            break;
        }
        case SET_BEEP_CONTROL:      // 设备鸣响控制
        {
            if (vData != nullptr)
            {
                ControlBeep(*((LPSTBEEPCONT)vData));
            }
            break;
        }
        case SET_GLIGHT_CONTROL:    // 指示灯控制
        {
            if (vData != nullptr)
            {
                ControlLight(*((LPSTGLIGHTSCONT)vData));
            }
            break;
        }
        case SET_IMAGE_PAR:         // 设置图像参数
        {
            if (vData != nullptr)
            {
                memcpy(&m_stImageParam, (LPSTIMAGEPAR)vData, sizeof(STIMAGEPAR));
            }
            break;
        }
        default:
            break;
    }

    return IDC_SUCCESS;
}

// 获取数据
int CDevFIDC_CRT603CZ7::GetData(unsigned short usType, void *vData)
{
    THISMODULE(__FUNCTION__);

    return IDC_SUCCESS;
}

// 获取版本
int CDevFIDC_CRT603CZ7::GetVersion(unsigned short usType, char* szVer, int nSize)
{
    THISMODULE(__FUNCTION__);

    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_FW:        // 固件版本号
                m_pDevImpl.GetReaderVersion(szVer, nSize, VER_FW);
                break;
            default:
                break;
        }
    }

    return IDC_SUCCESS;
}

// 指示灯控制
INT CDevFIDC_CRT603CZ7::ControlLight(STGLIGHTSCONT stLignt)
{
    if (stLignt.wContMode == 0)     // 自动
    {
        m_pDevImpl.SetLightAuto(0);
    } else
    {
        m_pDevImpl.SetLightAuto(1);

        INT nYellow = 0, nBlue = 0, nGreen = 0, nRed = 0;
        WORD nStat = 0;
        if (stLignt.enLigAct == GLIGHTS_ACT_OPEN)   // 打开
        {
            nStat = 1;
        } else
        if (stLignt.enLigAct == GLIGHTS_ACT_CLOSE)  // 关闭
        {
            nStat = 0;
        } else                                      // 闪烁
        {
            nStat = 2;
        }

        if ((stLignt.enLigType & GLIGHTS_TYPE_YELLOW) == GLIGHTS_TYPE_YELLOW)
        {
            nYellow = nStat;
        }
        if ((stLignt.enLigType & GLIGHTS_TYPE_BLUE) == GLIGHTS_TYPE_BLUE)
        {
            nBlue = nStat;
        }
        if ((stLignt.enLigType & GLIGHTS_TYPE_GREEN) == GLIGHTS_TYPE_GREEN)
        {
            nGreen = nStat;
        }
        if ((stLignt.enLigType & GLIGHTS_TYPE_RED) == GLIGHTS_TYPE_RED)
        {
            nRed = nStat;
        }
        m_pDevImpl.SetLightStat(nYellow, nBlue, nGreen, nRed);
    }

    return IDC_SUCCESS;
}

// 鸣响控制
INT CDevFIDC_CRT603CZ7::ControlBeep(STBEEPCONT stBeep)
{
    if (stBeep.wContMode == 0)  // 自动鸣响
    {
        m_pDevImpl.SetBeepAuto(TRUE);
    } else
    {
        m_pDevImpl.SetBeepAuto(FALSE);
        m_pDevImpl.SetBeepCont(stBeep.wBeepMsec);
    }

    return IDC_SUCCESS;
}

// Impl错误码转换为IDC错误码
INT CDevFIDC_CRT603CZ7::ConvertCode_Impl2IDC(INT nRet)
{
#define CRT603_CASE_RET_DEVCODE(IMP, RET) \
        case IMP: return RET;

    switch(nRet)
    {
        CRT603_CASE_RET_DEVCODE(IMP_SUCCESS, IDC_SUCCESS)                      // 成功
        CRT603_CASE_RET_DEVCODE(IMP_ERR_LOAD_LIB, ERR_IDC_LIBRARY)             // 动态库加载失败
        CRT603_CASE_RET_DEVCODE(IMP_ERR_PARAM_INVALID, ERR_IDC_PARAM_ERR)      // 参数无效
        CRT603_CASE_RET_DEVCODE(IMP_ERR_UNKNOWN,ERR_IDC_OTHER)                 // 未知错误
        CRT603_CASE_RET_DEVCODE(IMP_ERR_NOTOPEN, ERR_IDC_DEV_NOTOPEN)          // 设备未Open
        CRT603_CASE_RET_DEVCODE(IMP_ERR_NODEVICE, ERR_IDC_DEV_NOTFOUND)        // 未找到有效设备
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F1, ERR_IDC_OTHER)                 // An internal consistency check failed(内部一致性检查失败");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F2, ERR_IDC_USER_CANCEL);          // The action was cancelled by an SCardCancel request(该行动因一项取消请求而取消");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F3, ERR_IDC_PARAM_ERR);            // The supplied handle was invalid(提供的句柄无效");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F4, ERR_IDC_OTHER)                 // One or more of the supplied parameters could not be properly interpreted(无法正确解释提供的一个或多个参数");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F5, ERR_IDC_OTHER)                 // Registry startup information is missing or invalid(注册表启动信息丢失或无效");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F6, ERR_IDC_OTHER)                 // Not enough memory available to complete this command(内存不足，无法完成此命令");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F7, ERR_IDC_OTHER)                 // An internal consistency timer has expired(内部一致性计时器已过期");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F8, ERR_IDC_OTHER)                 // The specified reader name is not recognized(无法识别指定的读取器名称");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F9, ERR_IDC_OTHER)                 // The specified reader name is not recognized(无法识别指定的读取器名称");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F10, ERR_IDC_OTHER)                // The user-specified timeout value has expired(用户指定的超时值已过期");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F11, ERR_IDC_OTHER)                // The operation requires a Smart Card, but no Smart Card is currently in the device(该操作需要智能卡，但设备中当前没有智能卡");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F12, ERR_IDC_OTHER)                // The operation requires a Smart Card, but no Smart Card is currently in the device(该操作需要智能卡，但设备中当前没有智能卡");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F13, ERR_IDC_OTHER)                // The specified smart card name is not recognized(无法识别指定的智能卡名称");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F14, ERR_IDC_OTHER)                // The system could not dispose of the media in the requested manner(系统无法按请求的方式处置介质");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F15, ERR_IDC_OTHER)                // The requested protocols are incompatible with the protocol currently in use with the smart card(请求的协议与智能卡当前使用的协议不兼容");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F16, ERR_IDC_OTHER)                // The reader or smart card is not ready to accept commands(读卡器或智能卡尚未准备好接受命令");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F17, ERR_IDC_OTHER)                // One or more of the supplied parameters values could not be properly interpreted(无法正确解释提供的一个或多个参数值");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F18, ERR_IDC_OTHER)                // The action was cancelled by the system, presumably to log off or shut down(该操作被系统取消，可能是为了注销或关闭");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F19, ERR_IDC_OTHER)                // An internal communications error has been detected(检测到内部通信错误");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F20, ERR_IDC_OTHER)                // An internal error has been detected, but the source is unknown(检测到内部错误，但来源未知");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F21, ERR_IDC_OTHER)                // An ATR obtained from the registry is not a valid ATR string(从注册表获取的ATR不是有效的ATR字符串");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F22, ERR_IDC_OTHER)                // An attempt was made to end a non-existent transaction(试图结束一项不存在的交易");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F23, ERR_IDC_OTHER)                // The specified reader is not currently available for use(指定的读卡器当前不可用");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F24, ERR_IDC_OTHER)                // The operation has been aborted to allow the server application to exit(操作已中止，以允许服务器应用程序退出");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F25, ERR_IDC_OTHER)                // The PCI Receive buffer was too small(PCI接收缓冲区太小");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F26, ERR_IDC_OTHER)                // The reader driver does not meet minimal requirements for support(读卡器驱动程序不满足最低支持要求");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F27, ERR_IDC_OTHER)                // The reader driver did not produce a unique reader name(读卡器驱动程序没有生成唯一的读卡器名称");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F28, ERR_IDC_OTHER)                // The smart card does not meet minimal requirements for support(智能卡不满足最低支持要求");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F29, ERR_IDC_OTHER)                // The Smart card resource manager is not running(智能卡资源管理器未运行");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F30, ERR_IDC_OTHER)                // The Smart card resource manager has shut down(智能卡资源管理器已关闭");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F31, ERR_IDC_OTHER)                // An unexpected card error has occurred(发生了意外的卡错误");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F32, ERR_IDC_OTHER)                // No Primary Provider can be found for the smart card(找不到智能卡的主要提供商");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F33, ERR_IDC_OTHER)                // The requested order of object creation is not supported(不支持请求的对象创建顺序");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F34, ERR_IDC_OTHER)                // This smart card does not support the requested feature(此智能卡不支持请求的功能");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F35, ERR_IDC_OTHER)                // The identified directory does not exist in the smart card(智能卡中不存在标识的目录");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F36, ERR_IDC_OTHER)                // The identified file does not exist in the smart card(智能卡中不存在识别的文件");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F37, ERR_IDC_OTHER)                // The supplied path does not represent a smart card directory(提供的路径不代表智能卡目录");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F38, ERR_IDC_OTHER)                // The supplied path does not represent a smart card file(提供的路径不代表智能卡文件");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F39, ERR_IDC_OTHER)                // Access is denied to this file(拒绝访问此文件");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F40, ERR_IDC_OTHER)                // The smartcard does not have enough memory to store the information(智能卡内存不足，无法存储信息");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F41, ERR_IDC_OTHER)                // There was an error trying to set the smart card file object pointer(试图设置智能卡文件对象指针时出错");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F42, ERR_IDC_OTHER)                // The supplied PIN is incorrect(提供的PIN不正确");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F43, ERR_IDC_OTHER)                // An unrecognized error code was returned from a layered component(分层组件返回了无法识别的错误代码");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F44, ERR_IDC_OTHER)                // The requested certificate does not exist(请求的证书不存在");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F45, ERR_IDC_OTHER)                // The requested certificate could not be obtained(无法获取请求的证书");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F46, ERR_IDC_OTHER)                // Cannot find a smart card reader(找不到智能卡读卡器");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F47, ERR_IDC_OTHER)                // A communications error with the smart card has been detected(  Retry the operation(检测到智能卡存在通信错误");请重试该操作");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F48, ERR_IDC_OTHER)                // The requested key container does not exist on the smart card(智能卡上不存在请求的密钥容器");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F49, ERR_IDC_OTHER)                // The Smart card resource manager is too busy to complete this operation(智能卡资源管理器太忙，无法完成此操作");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F101, ERR_IDC_OTHER)               // The Smart card resource manager is too busy to complete this operation(智能卡资源管理器太忙，无法完成此操作");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F102, ERR_IDC_OTHER)               // The smart card is not responding to a reset(智能卡对重置没有响应");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F103, ERR_IDC_OTHER)               // Power has been removed from the smart card, so that further communication is not possible(智能卡已断电，因此无法进行进一步通信");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F104, ERR_IDC_OTHER)               // The smart card has been reset, so any shared state information is invalid(智能卡已重置，因此任何共享状态信息都无效");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F105, ERR_IDC_OTHER)               // The smart card has been removed, so that further communication is not possible(智能卡已被移除，因此无法进行进一步通信");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F106, ERR_IDC_OTHER)               // Access was denied because of a security violation(由于安全违规，访问被拒绝");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F107, ERR_IDC_OTHER)               // The card cannot be accessed because the wrong PIN was presented(无法访问该卡，因为提供了错误的PIN");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F108, ERR_IDC_OTHER)               // The card cannot be accessed because the maximum number of PIN entry attempts has been reached(无法访问该卡，因为已达到尝试输入PIN码的最大次数");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F109, ERR_IDC_OTHER)               // The end of the smart card file has been reached(已到达智能卡文件的末尾");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F110, ERR_IDC_USER_CANCEL)         // The action was cancelled by the user(该操作已被用户取消");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F111, ERR_IDC_OTHER)               // No PIN was presented to the smart card(智能卡上未显示PIN");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F112, ERR_IDC_OTHER)               // The requested item could not be found in the cache(在缓存中找不到请求的项");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F113, ERR_IDC_OTHER)               // The requested cache item is too old and was deleted from the cache(请求的缓存项太旧，已从缓存中删除");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F114, ERR_IDC_OTHER)               // The new cache item exceeds the maximum per-item size defined for the cache(新缓存项超过了为缓存定义的最大每项大小");
        CRT603_CASE_RET_DEVCODE(IMP_ERR_DEV_F99, ERR_IDC_OTHER)                // Function returned unknown error code(函数返回未知错误代码");
        default: return ERR_IDC_OTHER;
    }

    return IDC_SUCCESS;
}

// 分类型证件数据组合
BOOL CDevFIDC_CRT603CZ7::bIDCardDataToBuff(STCRTDEFIDINFO stIDCardData, LPCSTR lpcFingerData, DWORD dwFigerSize,
                                           LPSTR lpDestData)
{
    sprintf(lpDestData,
            "Name=%s|Sex=%s|Nation=%s|Born=%s|IDCardNo=%s|Address=%s|GrantDept=%s|UserLifeBegin=%s|"
            "UserLifeEnd=%s|PhotoFileName=%s",
            QString(stIDCardData.szName).trimmed().toStdString().c_str(),           // 姓名
            QString(stIDCardData.szSex).trimmed().toStdString().c_str(),            // 性别
            QString(stIDCardData.szNation).trimmed().toStdString().c_str(),         // 民族
            QString(stIDCardData.szBornDay).trimmed().toStdString().c_str(),        // 出生日期
            QString(stIDCardData.szIDNum).trimmed().toStdString().c_str(),          // 身份证号码
            QString(stIDCardData.szAddress).trimmed().toStdString().c_str(),        // 住址信息
            QString(stIDCardData.szIssued).trimmed().toStdString().c_str(),         // 签发机关
            QString(stIDCardData.szBeginValidity).trimmed().toStdString().c_str(),  // 有效日期开始
            QString(stIDCardData.szEndValidity).trimmed().toStdString().c_str(),    // 有效日期结束
            QString(stIDCardData.szImgData).toStdString().c_str()                   // 头像信息文件
           );

    // 指纹处理
    if (lpcFingerData != nullptr && dwFigerSize > 0)
    {
        sprintf(lpDestData + strlen(lpDestData), "|Finger1=");
        for (int i = 0; i < dwFigerSize / 2; i ++)
        {
            sprintf(lpDestData + strlen(lpDestData), "%02x", (BYTE)(lpcFingerData[i]));
        }
        sprintf(lpDestData + strlen(lpDestData), "|Finger2=");
        for (int i = dwFigerSize / 2; i < dwFigerSize; i ++)
        {
            sprintf(lpDestData + strlen(lpDestData), "%02x", (BYTE)(lpcFingerData[i]));
        }
    }

    return TRUE;
}
