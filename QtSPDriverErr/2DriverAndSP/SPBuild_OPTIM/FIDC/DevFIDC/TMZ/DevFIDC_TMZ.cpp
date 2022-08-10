/***************************************************************
* 文件名称: DevFIDC_TMZ.cpp
* 文件描述: 非接模块功能处理接口封装
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2021年10月20日
* 文件版本: 1.0.0.1
****************************************************************/

#include "DevFIDC_TMZ.h"
#include <unistd.h>

static const char *ThisFile = "DevFIDC_TMZ.cpp";
#define FIDC_SLOT_NUM 0x31

/****************************************************************************
*     对外接口调用处理                                                         *
****************************************************************************/
CDevFIDC_TMZ::CDevFIDC_TMZ(LPCSTR lpDevType) : m_pDevImpl(LOG_NAME_DEV)
{
    SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件

    m_bCancelReadCard = FALSE;                      // 取消读卡标记初始化:F
    m_bICActive = FALSE;                            // 卡片是否处于激活中初始化:F
}

CDevFIDC_TMZ::~CDevFIDC_TMZ()
{
    Close();
}

// 释放接口
int CDevFIDC_TMZ::Release()
{
    return IDC_SUCCESS;
}

// 打开与设备的连接
int CDevFIDC_TMZ::Open(const char *pMode)
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
int CDevFIDC_TMZ::Close()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_pDevImpl.CloseDevice();
    return IDC_SUCCESS;
}

// 取消
int CDevFIDC_TMZ::Cancel(unsigned short usMode)
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
int CDevFIDC_TMZ::Reset(MEDIA_ACTION enMediaAct, unsigned short usParam)
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
    }

    return IDC_SUCCESS;
}

// 取设备状态
int CDevFIDC_TMZ::GetStatus(STDEVIDCSTATUS &stStatus)
{
    //THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);

    INT nRet = IDC_SUCCESS;
    STDEVIDCSTATUS stDevStat;

    // 取设备状态(时间长)
    /*UCHAR ucDevStat = 0x00;
    nRet = m_pDevImpl.GetDeviceStatus(ucDevStat);
    if (nRet != IMP_SUCCESS)
    {
        m_bICActive = FALSE;
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        stStatus.wDevice = DEV_STAT_OFFLINE;    // 返回值非0认为断线
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        return ERR_IDC_HWERR;
    } else
    {
        stStatus.wDevice = DEV_STAT_ONLINE;
        switch(ucDevStat)
        {
            //case DEV_STAT_OK:   // 正常
            //case DEV_STAT_CONT_ABNOR:           // 接触卡通道异常
            case DEV_STAT_NOCONT_ABNOR:         // 非接卡通道异常
                stStatus.wDevice = DEV_STAT_HWERROR;
                break;
            //case DEV_STAT_NO_CONT_ABNOR:        // 接触卡和非接卡通道异常
            //case DEV_STAT_SECUR_ABNOR:          // 安全模块通道异常
            //case DEV_STAT_CONT_SECUR_ABNOR:     // 接触卡和安全模块通道异常
            //case DEV_STAT_NOCONT_SECUR_ABNOR:   // 非接卡和安全模块通道异常
            //case DEV_STAT_NO_CONT_SECUR_ABNOR:  // 接触卡、非接卡和安全模块通道异常
        }
    }*/

    if (m_pDevImpl.IsDeviceOpen() != TRUE)
    {
        m_bICActive = FALSE;
        stStatus.wDevice = DEVICE_STAT_OFFLINE;
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        stStatus.wChipPower = CHIP_STAT_UNKNOWN;
        return IMP_SUCCESS;
    }

    // 取卡片状态
    INT nCardStat = 0;
    nRet = m_pDevImpl.GetCardStatus(nCardStat, FCPU_SLOT_ONLY);
    if (nRet != IMP_SUCCESS)
    {
        m_bICActive = FALSE;
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        stStatus.wDevice = DEVICE_STAT_OFFLINE;    // 返回值非0认为断线
        stStatus.wChipPower = CHIP_STAT_UNKNOWN;
        return ERR_IDC_DEV_HWERR;
    }

    stStatus.wDevice = DEVICE_STAT_ONLINE;

    // 卡状态Check
    switch(nCardStat)
    {
        case CARD_STAT_POWER_OFF:       // 有卡未上电(未激活,或者非接有卡状态)
            stStatus.wChipPower = CHIP_STAT_POWEREDOFF; // 存在未上电
            if (m_bICActive != TRUE)    // 非激活状态
            {
                stStatus.wMedia = MEDIA_STAT_ENTERING;  // 介质在出口
            } else
            {
                stStatus.wMedia = MEDIA_STAT_PRESENT;   // 介质在内部
                stStatus.wChipPower = CHIP_STAT_ONLINE; // 存在并上电
            }
            break;
        case CARD_STAT_POWER_ON:        // 有卡已上电(非接卡暂无该状态)
            stStatus.wMedia = MEDIA_STAT_PRESENT;       // 介质在内部
            stStatus.wChipPower = CHIP_STAT_ONLINE;     // 存在并上电
            break;
        case CARD_STAT_POWER_FAIL:      // 卡上电失败(激活失败)
        case CARD_STAT_DATA_NORESP:     // 操作卡片数据无回应（超时）
        case CARD_STAT_DATA_ERR:        // 操作卡片数据出现错误
            m_bICActive = FALSE;
            stStatus.wMedia = MEDIA_STAT_UNKNOWN;       // 介质未知
            stStatus.wChipPower = CHIP_STAT_HWERROR;    // 存在且故障中
            break;
        case CARD_STAT_NOHAVE:          // 无卡
            stStatus.wChipPower = CHIP_STAT_NOCARD;     // 无卡
        case CARD_STAT_TYPE_NOTSUPP:    // 不支持卡类型
            stStatus.wChipPower = CHIP_STAT_NOTSUPP;    // 不支持
        default:
            m_bICActive = FALSE;
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;    // 介质不存在
            break;
    }

    return IDC_SUCCESS;
}

// 介质控制
int CDevFIDC_TMZ::MediaControl(MEDIA_ACTION enMediaAct, unsigned long ulParam)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    if(enMediaAct == MEDIA_EJECT)   // 介质退出
    {
        nRet = m_pDevImpl.SetCPUPowerOff(FCPU_SLOT_ONLY);
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "介质控制: 退卡: 断电: ->SetCPUPowerOff(%d) Fail, ErrCode: %d, Return: %s",
                FCPU_SLOT_ONLY, nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
            return ConvertCode_Impl2IDC(nRet);
        }
    } else
    if (enMediaAct == MEDIA_ACCEPT_IC) // IC卡进卡
    {
        // 进卡检查
        return AcceptMedia((DWORD)ulParam);
    } else
    {
        Log(ThisModule, __LINE__, "介质控制: 不支持的入参[%d], Return: %s",
            enMediaAct, ConvertDevErrCodeToStr(ERR_IDC_PARAM_ERR));
        return ERR_IDC_PARAM_ERR;
    }

    return IDC_SUCCESS;
}

// 介质读写
int CDevFIDC_TMZ::MediaReadWrite(MEDIA_RW_MODE enRWMode, STMEDIARW &stMediaData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;

    if (enRWMode == MEDIA_READ) // 读
    {
        if ((stMediaData.dwRWType & RW_CHIP) == RW_CHIP) // 读芯片(激活上电)
        {
            UCHAR ucCardType = 0;
            UCHAR ucSnr[4] = {0};
            UCHAR ucSnrLen = sizeof(ucSnr);

            nRet = m_pDevImpl.SetCPUPowerOn(FCPU_SLOT_ONLY, 100, ucCardType, ucSnr, ucSnrLen,
                                            (UCHAR*)stMediaData.stData[3].szData, (INT&)stMediaData.stData[3].wSize);
            if(nRet != IDC_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "介质读写: 读芯片: 上电: ->SetCPUPowerOn(%d, %d, %d) Fail, ErrCode: %d, Return: %d",
                    FCPU_SLOT_ONLY, 100, ucCardType, nRet, ConvertCode_Impl2IDC(nRet));
                return ConvertCode_Impl2IDC(nRet);
            }
            stMediaData.stData[3].wResult = RW_RESULT_SUCC;
            m_bICActive = TRUE;
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
int CDevFIDC_TMZ::ChipReadWrite(CHIP_RW_MODE enChipMode, STCHIPRW &stChipData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    if (enChipMode == CHIP_POW_COLD ||  // 冷复位
        enChipMode == CHIP_POW_WARM)    // 热复位
    {
        UCHAR ucCardType = 0;
        UCHAR ucSnr[4] = {0};
        UCHAR ucSnrLen = sizeof(ucSnr);

        nRet = m_pDevImpl.SetCPUPowerOn(FCPU_SLOT_ONLY, 100, ucCardType, ucSnr, ucSnrLen,
                                        (UCHAR*)stChipData.stData[1].szData, (INT&)stChipData.stData[1].dwSize);
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: 冷复位|热复位: 上电: ->SetCPUPowerOn(%d, %d, %d) Fail, ErrCode: %d, Return: %d",
                FCPU_SLOT_ONLY, 100, ucCardType, nRet, ConvertCode_Impl2IDC(nRet));
            return ConvertCode_Impl2IDC(nRet);
        }
        m_bICActive = TRUE;
    } else
    if (enChipMode == CHIP_POW_OFF)     // 断电
    {
        nRet = m_pDevImpl.SetCPUPowerOff(FCPU_SLOT_ONLY);
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: 断电: ->SetCPUPowerOff(%d) Fail, ErrCode: %d, Return: %s",
                FCPU_SLOT_ONLY, nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
            return ConvertCode_Impl2IDC(nRet);
        }
        m_bICActive = FALSE;
    } else
    if (enChipMode == CHIP_IO_T0 ||     // T0协议通信
        enChipMode == CHIP_IO_T1)       // T1协议通信
    {
        nRet = m_pDevImpl.SendCPUCardAPDU(FCPU_SLOT_ONLY,
                                          (UCHAR*)stChipData.stData[0].szData, stChipData.stData[0].dwSize,
                                          (UCHAR*)stChipData.stData[1].szData, (INT&)stChipData.stData[1].dwSize);
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: T0协议通信|T1协议通信: ->SendCPUCardAPDU(%d, %s, %d) Fail, ErrCode: %d, Return: %d",
                FCPU_SLOT_ONLY, stChipData.stData[0].szData, stChipData.stData[0].dwSize,
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

// 设置数据
int CDevFIDC_TMZ::SetData(unsigned short usType, void *vData)
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
        default:
            break;
    }

    return IDC_SUCCESS;
}

// 获取数据
int CDevFIDC_TMZ::GetData(unsigned short usType, void *vData)
{
    THISMODULE(__FUNCTION__);

    return IDC_SUCCESS;
}

// 获取版本
int CDevFIDC_TMZ::GetVersion(unsigned short usType, char* szVer, int nSize)
{
    THISMODULE(__FUNCTION__);

    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_FW:        // 固件版本号
                m_pDevImpl.GetFWVersion(szVer);
                break;
            default:
                break;
        }
    }

    return IDC_SUCCESS;
}

// 等待放卡处理
INT CDevFIDC_TMZ::AcceptMedia(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    ULONG ulTimeCount = 0;
    STDEVIDCSTATUS stDevStat;
    INT nRet = IMP_SUCCESS;

    QTime qtTimeCurr1, qtTimeCurr2;
    qtTimeCurr1 = QTime::currentTime();     // 取吸卡执行前时间

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

// 指示灯控制
INT CDevFIDC_TMZ::ControlLight(STGLIGHTSCONT stLignt)
{
    if (stLignt.enLigAct == GLIGHTS_ACT_OPEN)   // 打开
    {
        m_pDevImpl.SetLEDCtrl(1, stLignt.wDelay);
    } else
    if (stLignt.enLigAct == GLIGHTS_ACT_CLOSE)  // 关闭
    {
        m_pDevImpl.SetLEDCtrl(0, stLignt.wDelay);
    } else                                      // 闪烁
    {
        m_pDevImpl.SetLEDCtrl(2, stLignt.wDelay);
    }
    return IDC_SUCCESS;
}

// 鸣响控制
INT CDevFIDC_TMZ::ControlBeep(STBEEPCONT stBeep)
{
    WORD wMesc = stBeep.wBeepMsec > 100 ? stBeep.wBeepMsec / 100 : 1;
    m_pDevImpl.SetDeviceBeep(wMesc, stBeep.wBeepInterval, stBeep.wBeepCount);
    return IDC_SUCCESS;
}

// Impl错误码转换为IDC错误码
INT CDevFIDC_TMZ::ConvertCode_Impl2IDC(INT nRet)
{
    switch(nRet)
    {
        case IMP_SUCCESS:               // 成功
            return IDC_SUCCESS;
        case IMP_ERR_LOAD_LIB:          // 动态库加载失败
            return ERR_IDC_LIBRARY;
        case IMP_ERR_PARAM_INVALID:     // 参数无效
            return ERR_IDC_PARAM_ERR;
        case IMP_ERR_UNKNOWN:           // 未知错误
            return ERR_IDC_OTHER;
        case IMP_ERR_NOTOPEN:           // 设备未Open
            return ERR_IDC_DEV_NOTOPEN;
        // <0: Device返回
        case IMP_DEV_ERR_F31:           // 取消PIN输入
        case IMP_DEV_ERR_F32:           // 键盘超时
        case IMP_DEV_ERR_F33:           // 输入密码长度错误
        case IMP_DEV_ERR_F85:           // 键盘不支持
        case IMP_DEV_ERR_F86:           // 键盘返回数据格式错误
        case IMP_DEV_ERR_F97:           // 无效句柄
            return ERR_IDC_OTHER;
        case IMP_DEV_ERR_F98:           // 设备异常断开
            return ERR_IDC_DEV_NOTOPEN;
        case IMP_DEV_ERR_F99:           // 获取设备状态参数错误
        case IMP_DEV_ERR_F100:          // 设置设备状态参数错误
            return ERR_IDC_PARAM_ERR;
        case IMP_DEV_ERR_F101:          // 设置通讯事件错误
        case IMP_DEV_ERR_F113:          // 读数据错误
            return ERR_IDC_READ_ERR;
        case IMP_DEV_ERR_F114:          // 写数据错误
            return ERR_IDC_WRITE_ERR;
        case IMP_DEV_ERR_F115:          // 命令头错误
        case IMP_DEV_ERR_F116:          // 命令尾错误
        case IMP_DEV_ERR_F117:          // 数据错位
        case IMP_DEV_ERR_F118:          // 校验位错误
            return ERR_IDC_RESP_NOT_COMP;
        case IMP_DEV_ERR_F119:          // 超时错误(上层软件超时返回，没有等待到硬件返回数据)
            return ERR_IDC_TIMEOUT;
        case IMP_DEV_ERR_F129:          // 数据分配空间错误(内存错误)
        case IMP_DEV_ERR_F130:          // 长度错误
        case IMP_DEV_ERR_F131:          // 传入数据格式错误
        case IMP_DEV_ERR_F144:          // 设备不支持该操作(动态库未加载)
        case IMP_DEV_ERR_F145:          // 二代证错误
        case IMP_DEV_ERR_F146:          // 无权限(一般为文件操作，可能是设置的路径问题)
        case IMP_DEV_ERR_F147:          // 解码库加载失败
        case IMP_DEV_ERR_F148:          // 身份证解码错误
        case IMP_DEV_ERR_F149:          // 其他错误
        case IMP_DEV_ERR_F161:          // T57卡交互数据异常
            return ERR_IDC_OTHER;
        case IMP_DEV_ERR_F162:          // 无T57卡
            return ERR_IDC_MED_NOTFOUND;
        case IMP_DEV_ERR_F163:          // T57卡操作作卡片数据出现错误无回应
        case IMP_DEV_ERR_F164:          // T57卡参数设置失败
        case IMP_DEV_ERR_F165:          // T57卡密码认证没通过
            return ERR_IDC_OTHER;
        case IMP_DEV_ERR_F4097:         // 设备功能不支持或参数不支持
        case IMP_DEV_ERR_F4098:         // 命令执行错误
        case IMP_DEV_ERR_F8193:         // 写EEPROM失败
        case IMP_DEV_ERR_F8194:         // 读EEPROM失败
        case IMP_DEV_ERR_F12289:        // 不支持卡类型
            return ERR_IDC_OTHER;
        case IMP_DEV_ERR_F12290:// 无卡
            return ERR_IDC_MED_NOTFOUND;
        case IMP_DEV_ERR_F12291:        // 有卡已上电
        case IMP_DEV_ERR_F12292:        // 有卡未上电(或非接有卡状态)
        case IMP_DEV_ERR_F12293:        // 卡上电失败
            return ERR_IDC_CHIP_ACTIVE_FAIL;
        case IMP_DEV_ERR_F12294:        // 操作卡片数据无回应,超时(接触式存储卡无响应)
        case IMP_DEV_ERR_F12295:        // 操作卡片数据出现错误
        case IMP_DEV_ERR_F12296:        // 非接卡Halt失败
            return ERR_IDC_OTHER;
        case IMP_DEV_ERR_F12297:        // 多张非接卡
            return ERR_IDC_MED_MULTICARD;
        case IMP_DEV_ERR_F16386:        // 设备底层超时未响应返回
        case IMP_DEV_ERR_F20481:        // 磁头未开启
        case IMP_DEV_ERR_F20482:        // 刷卡失败
        case IMP_DEV_ERR_F20483:        // 刷卡超时
            return ERR_IDC_OTHER;
        default:
            return ERR_IDC_OTHER;
    }

    return IDC_SUCCESS;
}
