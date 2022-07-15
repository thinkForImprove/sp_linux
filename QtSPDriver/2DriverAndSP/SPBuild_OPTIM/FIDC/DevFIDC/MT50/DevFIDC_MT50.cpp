/***************************************************************
* 文件名称: DevFIDC_MT50.cpp
* 文件描述: 非接模块功能处理接口封装
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2021年10月20日
* 文件版本: 1.0.0.1
****************************************************************/

#include "DevFIDC_MT50.h"
#include <unistd.h>

static const char *ThisFile = "DevFIDC_MT50.cpp";

/****************************************************************************
*     对外接口调用处理                                                         *
****************************************************************************/
CDevFIDC_MT50::CDevFIDC_MT50(LPCSTR lpDevType) : m_pDevImpl(LOG_NAME_DEV)
{
    SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件

    m_bCancelReadCard = FALSE;                      // 取消读卡标记初始化:F
    m_bICActive = FALSE;                            // 卡片是否处于激活中初始化:F
}

CDevFIDC_MT50::~CDevFIDC_MT50()
{
    Close();
}

// 释放接口
int CDevFIDC_MT50::Release()
{
    return IDC_SUCCESS;
}

// 打开与设备的连接
int CDevFIDC_MT50::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    // AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;

    // 字符串VID/PID转数字
    INT nVid = (INT)DataConvertor::string_to_ulong(m_stOpenMode.szHidVid[0],
                                                   strlen(m_stOpenMode.szHidVid[0]), 16);
    INT nPid = (INT)DataConvertor::string_to_ulong(m_stOpenMode.szHidPid[0],
                                                   strlen(m_stOpenMode.szHidPid[0]), 16);

    if ((nRet = m_pDevImpl.OpenDeviceUSB(nVid, nPid, m_stOpenMode.wProtocol[0])) != IMP_SUCCESS)
    {
        /*Log(ThisModule, __LINE__,
            "打开设备: ->OpenDevice(%s) Fail, ErrCode: %d, Return: %s",
            pMode, nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));*/
        return ConvertCode_Impl2IDC(nRet);
    }

    return IDC_SUCCESS;
}

// 关闭与设备的连接
int CDevFIDC_MT50::Close()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_pDevImpl.CloseDevice();
    return IDC_SUCCESS;
}

// 取消
int CDevFIDC_MT50::Cancel(unsigned short usMode)
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
int CDevFIDC_MT50::Reset(MEDIA_ACTION enMediaAct, unsigned short usParam)
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
int CDevFIDC_MT50::GetStatus(STDEVIDCSTATUS &stStatus)
{
    //THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);

    INT nRet = IDC_SUCCESS;
    STDEVIDCSTATUS stDevStat;


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
    switch(nCardStat)
    {
        case IMP_ERR_DEV_00DEH:         // 有卡未上电
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
        case IMP_ERR_DEV_00A3H:         // libusb 拒绝访问，可能权限不足，可用管理员权限再试
        case IMP_ERR_DEV_00A4H:         // libusb 找不到设备(设备可能已经掉线)
        case IMP_ERR_DEV_00A5H:         // libusb 未找到设备
        case IMP_ERR_DEV_00A7H:         // libusb 操作超时
        {
            m_bICActive = FALSE;
            stStatus.wDevice = DEVICE_STAT_OFFLINE;
            stStatus.wMedia = MEDIA_STAT_UNKNOWN;
            stStatus.wChipPower = CHIP_STAT_UNKNOWN;
            break;
        }
        case IMP_ERR_DEV_00DDH:         // 无卡
        default:
        {
            m_bICActive = FALSE;
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
            stStatus.wChipPower = CHIP_STAT_NOCARD;
            break;
        }
    }

    return IDC_SUCCESS;
}

// 介质控制
int CDevFIDC_MT50::MediaControl(MEDIA_ACTION enMediaAct, unsigned long ulParam)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    if(enMediaAct == MEDIA_EJECT)   // 介质退出
    {
        nRet = m_pDevImpl.SetReaderPowerOff();
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "介质控制: 退卡: 断电: ->SetReaderPowerOff() Fail, ErrCode: %d, Return: %s",
                nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
            return ConvertCode_Impl2IDC(nRet);
        }
    } else
    if (enMediaAct == MEDIA_ACCEPT_IC) // IC卡进卡
    {
        // 进卡检查
        return AcceptMedia(ulParam);
    } else
    {
        Log(ThisModule, __LINE__, "介质控制: 不支持的入参[%d], Return: %s",
            enMediaAct, ConvertDevErrCodeToStr(ERR_IDC_PARAM_ERR));
        return ERR_IDC_PARAM_ERR;
    }

    return IDC_SUCCESS;
}

// 介质读写
int CDevFIDC_MT50::MediaReadWrite(MEDIA_RW_MODE enRWMode, STMEDIARW &stMediaData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    if (enRWMode == MEDIA_READ) // 读
    {
        if ((stMediaData.dwRWType & RW_CHIP)== RW_CHIP) // 读芯片(激活上电)
        {
            nRet = m_pDevImpl.SetReaderPowerOn(m_stOpenMode.nOtherParam[0],
                                               (UCHAR*)stMediaData.stData[3].szData,
                                               (INT&)stMediaData.stData[3].wSize);
            if(nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "介质读写: 读芯片: 上电: ->SetReaderPowerOn(%d) Fail, ErrCode: %d, Return: %d",
                    m_stOpenMode.nOtherParam[0], nRet, ConvertCode_Impl2IDC(nRet));
                stMediaData.Clear();
                return ConvertCode_Impl2IDC(nRet);
            }
            stMediaData.stData[3].wResult = RW_RESULT_SUCC;
            m_bICActive = TRUE;
        } else
        if ((stMediaData.dwRWType & RW_TRACK1)== RW_TRACK1 ||
            (stMediaData.dwRWType & RW_TRACK2)== RW_TRACK2 ||
            (stMediaData.dwRWType & RW_TRACK3)== RW_TRACK3)      // 读身份证
        {

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
int CDevFIDC_MT50::ChipReadWrite(CHIP_RW_MODE enChipMode, STCHIPRW &stChipData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    if (enChipMode == CHIP_POW_COLD ||  // 冷复位
        enChipMode == CHIP_POW_WARM)    // 热复位
    {
        nRet = m_pDevImpl.SetReaderPowerOn(m_stOpenMode.nOtherParam[0],
                                          (UCHAR*)stChipData.stData[1].szData,
                                          (INT&)stChipData.stData[1].dwSize);
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: 冷复位|热复位: 上电: ->SetReaderPowerOn(%d) Fail, ErrCode: %d, Return: %d",
                m_stOpenMode.nOtherParam[0], nRet, ConvertCode_Impl2IDC(nRet));
            return ConvertCode_Impl2IDC(nRet);
        }
        m_bICActive = TRUE;
    } else
    if (enChipMode == CHIP_POW_OFF)     // 断电
    {
        nRet = m_pDevImpl.SetReaderPowerOff();
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: 断电: ->SetReaderPowerOff() Fail, ErrCode: %d, Return: %s",
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

// 设置数据
int CDevFIDC_MT50::SetData(unsigned short usType, void *vData)
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
            if (vData != nullptr)
            {
                memcpy(&(m_stOpenMode), ((LPSTDEVICEOPENMODE)vData), sizeof(STDEVICEOPENMODE));
            }
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
int CDevFIDC_MT50::GetData(unsigned short usType, void *vData)
{
    //THISMODULE(__FUNCTION__);

    return IDC_SUCCESS;
}

// 获取版本
int CDevFIDC_MT50::GetVersion(unsigned short usType, char* szVer, int nSize)
{
    THISMODULE(__FUNCTION__);

    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_FW:        // 固件版本号
                m_pDevImpl.GetFWVersion(szVer, nSize);
                break;
            default:
                break;
        }
    }

    return IDC_SUCCESS;
}

// 等待放卡处理
INT CDevFIDC_MT50::AcceptMedia(DWORD dwTimeOut)
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
INT CDevFIDC_MT50::ControlLight(STGLIGHTSCONT stLignt)
{
    return IDC_SUCCESS;
}

// 鸣响控制
INT CDevFIDC_MT50::ControlBeep(STBEEPCONT stBeep)
{
    m_pDevImpl.SetReaderBeep(stBeep.wBeepMsec, stBeep.wBeepInterval, stBeep.wBeepCount);

    return IDC_SUCCESS;
}

// Impl错误码转换为IDC错误码
INT CDevFIDC_MT50::ConvertCode_Impl2IDC(INT nRet)
{
#define MT50_CASE_RET_DEVCODE(IMP, RET) \
        case IMP: return RET;

    switch(nRet)
    {
        MT50_CASE_RET_DEVCODE(IMP_SUCCESS, IDC_SUCCESS)                      // 成功
        MT50_CASE_RET_DEVCODE(IMP_ERR_LOAD_LIB, ERR_IDC_LIBRARY)             // 动态库加载失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_PARAM_INVALID, ERR_IDC_PARAM_ERR)      // 参数无效
        MT50_CASE_RET_DEVCODE(IMP_ERR_UNKNOWN,ERR_IDC_OTHER)                 // 未知错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_NOTOPEN, ERR_IDC_DEV_NOTOPEN)          // 设备未Open
        MT50_CASE_RET_DEVCODE(IMP_ERR_NODEVICE, ERR_IDC_DEV_NOTFOUND)        // 未找到有效设备
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0001H, ERR_IDC_MED_NOTFOUND);      // 在操作区域内无卡
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0002H, ERR_IDC_OTHER);             // 卡片CRC错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0003H, ERR_IDC_OTHER);             // 数值溢出
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0004H, ERR_IDC_OTHER);             // 验证不成功
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0005H, ERR_IDC_OTHER);             // 卡片奇偶校验错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0006H, ERR_IDC_OTHER);             // 与M1卡卡片通讯错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0008H, ERR_IDC_OTHER);             // 防冲突过程中读系列号错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0009H, ERR_IDC_OTHER);             // 波特率修改失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_000AH, ERR_IDC_OTHER);             // 卡片没有通过验证
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_000BH, ERR_IDC_OTHER);             // 从卡片接收到的位数错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_000CH, ERR_IDC_OTHER);             // 从卡片接收到的字节数错误（仅仅读函数有效）
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0016H, ERR_IDC_OTHER);             // 调用request函数出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0017H, ERR_IDC_OTHER);             // 调用select函数出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0018H, ERR_IDC_OTHER);             // 调用anticoll函数出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0019H, ERR_IDC_OTHER);             // 调用read函数出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_001AH, ERR_IDC_OTHER);             // 调用write函数出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_001BH, ERR_IDC_OTHER);             // 调用增值函数出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_001CH, ERR_IDC_OTHER);             // 调用减值函数出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_001DH, ERR_IDC_OTHER);             // 调用重载函数出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_001EH, ERR_IDC_OTHER);             // 调用loadkey函数出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_002AH, ERR_IDC_OTHER);             // 命令错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_002BH, ERR_IDC_OTHER);             // PC同reader之间通讯错误，比如BCC错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_002CH, ERR_IDC_OTHER);             // PC同reader之间通讯命令码错误(设备不支持这条指令)
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0041H, ERR_IDC_OTHER);             // 4442卡错误计数等于0（锁卡）
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0042H, ERR_IDC_OTHER);             // 4442卡密码校验失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0043H, ERR_IDC_OTHER);             // 4442卡读失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0044H, ERR_IDC_OTHER);             // 4442卡写失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0045H, ERR_IDC_OTHER);             // 4442卡读写地址越界
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0060H, ERR_IDC_READ_ERR);          // 接收出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0061H, ERR_IDC_OTHER);             // 输入偏移地址与长度超出读写范围
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0062H, ERR_IDC_OTHER);             // 不是102卡
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0063H, ERR_IDC_OTHER);             // 密码校验错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0070H, ERR_IDC_OTHER);             // 接收参数出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0071H, ERR_IDC_OTHER);             // 校验码出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0072H, ERR_IDC_OTHER);             // 命令执行失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0073H, ERR_IDC_OTHER);             // 命令执行超时
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0074H, ERR_IDC_OTHER);             // 错误通信模式
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0075H, ERR_IDC_OTHER);             // 无磁条卡
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_008AH, ERR_IDC_OTHER);             // 校验和错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_008BH, ERR_IDC_OTHER);             // 卡型错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_008CH, ERR_IDC_OTHER);             // 拔卡错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_008DH, ERR_IDC_OTHER);             // 通用错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_008EH, ERR_IDC_OTHER);             // 命令头错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_008FH, ERR_IDC_OTHER);             // 数据长度错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_0099H, ERR_IDC_OTHER);             // FLASH读写错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_1001H, ERR_IDC_OTHER);             // 不支持接触用户卡
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_1002H, ERR_IDC_OTHER);             // 接触用户卡未插到位
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_1003H, ERR_IDC_OTHER);             // 接触用户卡已上电
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_1004H, ERR_IDC_OTHER);             // 接触用户卡未上电
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_1005H, ERR_IDC_OTHER);             // 接触用户卡上电失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_1006H, ERR_IDC_OTHER);             // 操作接触用户卡数据无回应
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_1007H, ERR_IDC_OTHER);             // 操作接触用户卡数据出现错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_2001H, ERR_IDC_OTHER);             // 不支持PSAM卡
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_2002H, ERR_IDC_OTHER);             // PSAM卡未插到位
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_2003H, ERR_IDC_OTHER);             // PSAM卡已上电
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_2004H, ERR_IDC_OTHER);             // PSAM卡未上电
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_2005H, ERR_IDC_OTHER);             // PSAM卡上电失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_2006H, ERR_IDC_OTHER);             // 操作PSAM卡数据无回应
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_2007H, ERR_IDC_OTHER);             // 操作PSAM卡数据出现错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_3001H, ERR_IDC_OTHER);             // 不支持非接触用户卡
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_3004H, ERR_IDC_OTHER);             // 非接触用户卡未激活
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_3005H, ERR_IDC_OTHER);             // 非接触用户卡激活失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_3006H, ERR_IDC_OTHER);             // 操作非接触用户卡无回应（等待超时）
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_3007H, ERR_IDC_OTHER);             // 操作非接触用户卡数据出现错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_3008H, ERR_IDC_OTHER);             // 非接触用户卡halt失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_3009H, ERR_IDC_OTHER);             // 有多张卡在感应区
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6001H, ERR_IDC_OTHER);             // 不支持逻辑操作
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6020H, ERR_IDC_OTHER);             // 卡片类型不对（卡状态6A82）
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6021H, ERR_IDC_OTHER);             // 余额不足（卡状态9401）
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6022H, ERR_IDC_OTHER);             // 卡片功能不支持（卡状态6A81）
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6023H, ERR_IDC_OTHER);             // 扣款失败（卡状态9302）
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6030H, ERR_IDC_OTHER);             // 卡片未启用
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6031H, ERR_IDC_OTHER);             // 卡片不在有效期
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6032H, ERR_IDC_OTHER);             // 交易明细无此记录
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6033H, ERR_IDC_OTHER);             // 交易明细记录未处理完成
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6040H, ERR_IDC_OTHER);             // 需要做防拔处理
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6041H, ERR_IDC_OTHER);             // 防拔处理中出错, 非原来卡
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_6042H, ERR_IDC_OTHER);             // 交易中断, 没有资金损失
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00A1H, ERR_IDC_OTHER);             // libusb 返回输入输出错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00A2H, ERR_IDC_PARAM_ERR);         // libusb 参数错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00A3H, ERR_IDC_DEV_NOTOPEN);       // libusb 拒绝访问，可能权限不足，可用管理员权限再试
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00A4H, ERR_IDC_DEV_NOTOPEN);       // libusb 找不到设备(设备可能已经掉线)
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00A5H, ERR_IDC_DEV_NOTOPEN);       // libusb 未找到设备
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00A6H, ERR_IDC_OTHER);             // libusb 资源忙，可能设备已经被占用
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00A7H, ERR_IDC_OTHER);             // libusb 操作超时
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00A8H, ERR_IDC_OTHER);             // libusb 溢出错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00A9H, ERR_IDC_OTHER);             // libusb 管道错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00AAH, ERR_IDC_OTHER);             // libusb 系统调用中断(可能是由于信号)
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00ABH, ERR_IDC_OTHER);             // libusb 内存不足
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00ACH, ERR_IDC_OTHER);             // libusb 在这个平台上不支持的操作(linux)
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00B1H, ERR_IDC_OTHER);             // 通讯超时
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00B2H, ERR_IDC_OTHER);             // 无效的通讯句柄
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00B3H, ERR_IDC_OTHER);             // 打开串口错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00B4H, ERR_IDC_OTHER);             // 串口已经打开
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00B5H, ERR_IDC_OTHER);             // 获取通讯端口状态错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00B6H, ERR_IDC_OTHER);             // 设置通讯端口状态错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00B7H, ERR_IDC_OTHER);             // 从读写器读取数据出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00B8H, ERR_IDC_OTHER);             // 向读写器写入数据出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00B9H, ERR_IDC_OTHER);             // 设置串口通讯波特率错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00C1H, ERR_IDC_OTHER);             // STX错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00C2H, ERR_IDC_OTHER);             // ETX错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00C3H, ERR_IDC_OTHER);             // BCC错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00C4H, ERR_IDC_OTHER);             // 命令的数据长度大于最大长度
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00C5H, ERR_IDC_OTHER);             // 数据值错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00C6H, ERR_IDC_OTHER);             // 错误的协议类型
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00C7H, ERR_IDC_OTHER);             // 错误的设备类型
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00C8H, ERR_IDC_OTHER);             // 错误的USB通讯设备类
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00C9H, ERR_IDC_OTHER);             // 设备正在通讯中或者是设备已经关闭
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00CAH, ERR_IDC_OTHER);             // 设备通讯忙，函数正在操作可能还没返回
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00CBH, ERR_IDC_OTHER);             // 接收到的设备返回的数据长度不对
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00D1H, ERR_IDC_OTHER);             // 获取身份证
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00D2H, ERR_IDC_OTHER);             // 身份证读卡
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00D3H, ERR_IDC_OTHER);             // 身份证校验
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00D4H, ERR_IDC_OTHER);             // 内存分配失
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00D5H, ERR_IDC_OTHER);             // 调用ICONV
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00D6H, ERR_IDC_OTHER);             // 调用iconv
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00D7H, ERR_IDC_OTHER);             // 调用libwlt.so库出错
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00D8H, ERR_IDC_OTHER);             // 传入的WLT错误
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00D9H, ERR_IDC_OTHER);             // 打开文件失
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00DAH, ERR_IDC_OTHER);             // 文件不存在
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00DBH, ERR_IDC_OTHER);             // 磁条卡数据
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00DCH, ERR_IDC_OTHER);             // 未识别卡类
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00DDH, ERR_IDC_MED_NOTFOUND);      // 无卡
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00DEH, ERR_IDC_OTHER);             // 有卡未上电
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00DFH, ERR_IDC_OTHER);             // 卡无应答
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00E0H, ERR_IDC_OTHER);             // 刷卡命令超时
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00E1H, ERR_IDC_OTHER);             // 磁条卡刷卡失败
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00E2H, ERR_IDC_OTHER);             // 磁条卡刷卡模式未开启
        MT50_CASE_RET_DEVCODE(IMP_ERR_DEV_00E3H, ERR_IDC_OTHER);             // 发送APDU错误
        default: return ERR_IDC_OTHER;
    }

    return IDC_SUCCESS;
}
