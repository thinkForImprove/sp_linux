/***************************************************************
* 文件名称: DevIDC_CRT350N.cpp
* 文件描述: 读卡器模块功能处理接口封装
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2021年10月20日
* 文件版本: 1.0.0.1
****************************************************************/

#include "DevIDC_CRT350N.h"
#include <unistd.h>

static const char *ThisFile = "DevIDC_CRT350N.cpp";

/****************************************************************************
*     对外接口调用处理                                                         *
****************************************************************************/
CDevIDC_CRT350N::CDevIDC_CRT350N(LPCSTR lpDevType) :
    m_pDevImpl(LOG_NAME_DEV, lpDevType),
    m_clErrorDet((LPSTR)"1")      // INI设定CRT350读卡器类型编号为1
{
    SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件

    m_bCancelReadCard = FALSE;                      // 取消读卡标记初始化:F
    m_bICActive = FALSE;                            // 卡片是否处于激活中初始化:F
    m_bICPress = FALSE;                             // 卡片是否处于触点接触状态初始化:F
    MSET_0(m_szResetParam);                         // 复位辅助参数
    MSET_0(m_nAuxParam);                            // 设备辅助参数
    sprintf(m_szResetParam, "32400201200");
    m_nTamperSensorStat = IMPL_TAMPER_UNKNOWN;      // 防盗钩状态初始化:未知
    memset(m_nRetErrOLD, 0, sizeof(INT) * 8);
    m_bIsTeaseCardProtectRun = FALSE;               // 防逗卡保护是否启动初始化:F
    //m_dwTeaseCardProtRunDate = 0;                   // 记录防逗卡保护启动时间初始化:0
    m_wTeaseReInCardCount = 0;                      // 记录防逗卡进卡计数初始化:0
    m_bIsSkimmingHave = FALSE;                      // 检知发现异物标记初始化:F
}

CDevIDC_CRT350N::~CDevIDC_CRT350N()
{
    Close();
}

// 释放接口
int CDevIDC_CRT350N::Release()
{
    return IDC_SUCCESS;
}

// 打开与设备的连接
int CDevIDC_CRT350N::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    // AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;

    // 组织USB入参字符串
    CHAR szUsbPar[64] = { 0x00 };
    sprintf(szUsbPar, "USB:%s,%s", m_stOpenMode.szHidVid, m_stOpenMode.szHidPid);
    nRet = m_pDevImpl.OpenDevice(szUsbPar, m_szResetParam, strlen(m_szResetParam));
    if (nRet != IMP_SUCCESS)
    {
        /*Log(ThisModule, __LINE__,
            "打开设备: ->OpenDevice(%s) Fail, ErrCode: %d, Return: %s",
            pMode, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));*/
        return ConvertImplErrCode2IDC(nRet);
    }

    return IDC_SUCCESS;
}

// 关闭与设备的连接
int CDevIDC_CRT350N::Close()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_pDevImpl.CloseDevice();
    return IDC_SUCCESS;
}

// 取消
int CDevIDC_CRT350N::Cancel(unsigned short usMode)
{
    THISMODULE(__FUNCTION__);

    if (usMode == 0)    // 取消读卡
    {
        Log(ThisModule, __LINE__, "设置取消读卡: usMode = %d.", usMode);
        m_bCancelReadCard = TRUE;
    }

    return IDC_SUCCESS;
}

// 设备复位
int CDevIDC_CRT350N::Reset(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    EN_DEV_INIT enCardAction = INIT_NOACTION;
    CHAR szBuffer[32] = { 0x00 };

    m_bIsTeaseCardProtectRun = FALSE;   // 防逗卡保护清除
    m_wTeaseReInCardCount = 0;          // 记录防逗卡进卡计数清零

    // 防盗钩下压状态时,需要通过防盗钩释放指令清除错误状态才能继续初始化
    if (AND_IS1(m_nTamperSensorStat, IMPL_TAMPER_PRESS))
    {
        Log(ThisModule, __LINE__, "复位: 防盗钩已下压, 准备释放...");

        nRet = m_pDevImpl.SetTamperOperAtion(TAM_SETRELEASE);
        if(nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "复位: 防盗钩已下压: 执行释放: ->SetTamperOperAtion(%d) Fail, ErrCode: %d, Return: %s",
                TAM_SETRELEASE, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
            m_clErrorDet.SetDevErrCode((LPSTR)EC_IDC_DEV_TamperRelease);
            return ConvertImplErrCode2IDC(nRet);
        }

        Log(ThisModule, __LINE__, "防盗钩已释放, 开始初始化...");
    }

    if(enMediaAct == MEDIA_EJECT)       // 介质退出
    {
        enCardAction = INIT_EJECT;
        sprintf(szBuffer, "复位: 退卡:");
    } else
    if(enMediaAct == MEDIA_RETRACT)     // 介质回收
    {
        enCardAction = INIT_RETAIN;
        sprintf(szBuffer, "复位: 吞卡:");
    } else                              // 无动作
    {
        enCardAction = INIT_NOACTION;
        sprintf(szBuffer, "复位: 卡保持:");
    }

    nRet = m_pDevImpl.DeviceInit(enCardAction, m_szResetParam, strlen(m_szResetParam),
                                 m_stOpenMode.nOtherParam[13]);
    if(nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "%s ->DeviceInit(%d, %s, %d, %d) Fail, ErrCode: %d, Return: %s",
            szBuffer, enCardAction, m_szResetParam, strlen(m_szResetParam),
            m_stOpenMode.nOtherParam[3], nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
        return ConvertImplErrCode2IDC(nRet);
    }

    return IDC_SUCCESS;
}

// 取设备状态
int CDevIDC_CRT350N::GetStatus(STDEVIDCSTATUS &stStatus)
{
    //THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;
    STDEVIDCSTATUS stDevStat;

    /* stStatus.wOtherCode[0]: 用于标记读卡器后出口是否有卡
     * stStatus.wOtherCode[1]: 1防逗卡保护已生效
     */

    // 取设备状态
    if (m_pDevImpl.IsDeviceOpen() != TRUE)
    {
        m_bICActive = FALSE;
        stStatus.wDevice = DEVICE_STAT_OFFLINE;
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        stStatus.wChipPower = CHIP_STAT_UNKNOWN;
        m_clErrorDet.SetDevErrCode((LPSTR)EC_DEV_DevOffLine);
        return ERR_IDC_OTHER;
    }

    // 防逗卡保护已生效
    if (IsTeaseCardProtect() != 0)
    {
        stStatus.wOtherCode[1] = 1;
        m_bICActive = FALSE;
        stStatus.wDevice = DEVICE_STAT_HWERROR;
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        stStatus.wChipPower = CHIP_STAT_UNKNOWN;        
        m_clErrorDet.SetDevErrCode(EC_IDC_DEV_TeaseCard);
        return ERR_IDC_OTHER;
    }

    stStatus.wDevice = DEVICE_STAT_ONLINE;

    // 防逗卡保护中，检测卡取消

    // 取卡片状态
    INT nStat[12];
    memset(nStat, -1, sizeof(nStat));
    nRet = m_pDevImpl.GetDeviceStat(GET_STAT_ALL, nStat);
    if (nRet != IMP_SUCCESS)
    {
        m_bICActive = FALSE;
        stStatus.wDevice = DEVICE_STAT_OFFLINE;
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        stStatus.wChipPower = CHIP_STAT_UNKNOWN;
        return IMP_SUCCESS;
    }

    // Check 卡状态
    switch(nStat[1])
    {
        case IMPL_STAT_CARD_NOTHAVE:    // 无卡
        {
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
            stStatus.wChipPower = CHIP_STAT_NOCARD;
            m_bICActive = FALSE;
            break;
        }
        case IMPL_STAT_CARD_ISEXPORT:   // 卡在出口
        {
            stStatus.wMedia = MEDIA_STAT_ENTERING;      // 介质在出口
            stStatus.wChipPower = CHIP_STAT_POWEREDOFF; // 存在未上电
            m_bICActive = FALSE;
            break;
        }
        case IMPL_STAT_CARD_ISINSIDE:   // 卡在内部
        {
            stStatus.wMedia = MEDIA_STAT_PRESENT;       // 介质在内部
            if (m_bICActive == TRUE)
            {
                stStatus.wChipPower = CHIP_STAT_ONLINE; // 存在并上电
            } else
            {
                stStatus.wChipPower = CHIP_STAT_POWEREDOFF;// 存在未上电
            }
            break;
        }
        default:                        // 其他卡状态
        {
            m_bICActive = FALSE;
            stStatus.wMedia = MEDIA_STAT_UNKNOWN;
            stStatus.wChipPower = CHIP_STAT_UNKNOWN;
            m_bICActive = FALSE;
            break;
        }
    }

    // 闸门状态
    m_wShutStat = (WORD)nStat[3];

    // 后出口是否有卡
    stStatus.wOtherCode[0] = ((WORD)nStat[4] == IMPL_STAT_CARD_ISAFTER_EXPORT ? 1 : 0);

    if (nStat[2] == IMPL_STAT_MEET_DOWN)    // 触点压下
    {
        m_bICPress = TRUE;
    } else
    {
        m_bICPress = FALSE;
    }

    m_clErrorDet.GetDevHWErrCode(stStatus.szErrCode);

    return IDC_SUCCESS;
}

// 介质控制
int CDevIDC_CRT350N::MediaControl(MEDIA_ACTION enMediaAct, unsigned long ulParam)
{
    THISMODULE(__FUNCTION__);

    #define MEDSTR enMediaAct == MEDIA_EJECT ? "退卡" : (enMediaAct == MEDIA_RETRACT ? "回收" : "卡移动")
    #define MEDTYPE enMediaAct == MEDIA_EJECT ? CARD_EJECT : \
                    (enMediaAct == MEDIA_RETRACT ? CARD_RETAIN : CARD_MOVE)

    INT nRet = IDC_SUCCESS;

    if (enMediaAct == MEDIA_EJECT ||    // 介质退出
        enMediaAct == MEDIA_RETRACT ||  // 介质回收
        enMediaAct == MEDIA_MOVE)       // 介质移动
    {
        if (m_bICActive == TRUE)    // 芯片上电中,先断电
        {
            nRet = m_pDevImpl.ChipOperation(CHIP_DEACTIVE);
            if(nRet != IDC_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "介质控制: %s(芯片上电状态): 断电: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %s",
                    MEDSTR, CHIP_DEACTIVE, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
                return ConvertImplErrCode2IDC(nRet);
            }
            m_bICActive = FALSE;
        }

        if (m_bICPress == TRUE)    // 触点接触状态,先释放接触
        {
            nRet = m_pDevImpl.ChipOperation(CHIP_RELEASE);
            if (nRet != IDC_SUCCESS)
            {
                if (enMediaAct == MEDIA_MOVE && ulParam == 9999)    // 不输出Log
                {
                    Log(ThisModule, __LINE__,
                        "介质控制: %s(触点接触状态): 释放接触: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %s",
                        MEDSTR, CHIP_RELEASE, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
                    return ConvertImplErrCode2IDC(nRet);
                }
            }
            m_bICPress = FALSE;
        }

        nRet = m_pDevImpl.MediaOperation(MEDTYPE);
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "介质控制: %s: ->MediaOperation(%d) Fail, ErrCode: %d, Return: %s",
                MEDSTR, MEDTYPE, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
            return ConvertImplErrCode2IDC(nRet);
        }
    } else
    if (enMediaAct == MEDIA_ACCEPT_IC ||    // IC卡进卡
        enMediaAct == MEDIA_ACCEPT)         // 磁条卡进卡
    {
        // 进卡检查
        return AcceptMedia(enMediaAct, ulParam);
    } else
    {
        Log(ThisModule, __LINE__, "介质控制: 不支持的入参[%d], Return: %s",
            enMediaAct, ConvertDevErrCodeToStr(ERR_IDC_PARAM_ERR));
        return ERR_IDC_PARAM_ERR;
    }

    return IDC_SUCCESS;
}

// 介质读写
int CDevIDC_CRT350N::MediaReadWrite(MEDIA_RW_MODE enRWMode, STMEDIARW &stMediaData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    DWORD dwMag = MAG_NULL;
    CHAR szBuff[1024 * 3] = { 0x00 };
    INT nBuffSize = sizeof(szBuff);
    WORD wReadCnt = 0;
    WORD wTrackOnly = 0;

    if (enRWMode == MEDIA_READ) // 读
    {
        if (AND_IS1(stMediaData.dwRWType, RW_TRACK1) ||
            AND_IS1(stMediaData.dwRWType, RW_TRACK2) ||
            AND_IS1(stMediaData.dwRWType, RW_TRACK3) ||
            AND_IS1(stMediaData.dwRWType, RW_CHIP))
        {
            // ----------------读磁道处理----------------
            if (AND_IS1(stMediaData.dwRWType, RW_TRACK1))
            {
                dwMag += MAG_READ_TRACK1;
                wReadCnt ++;
                wTrackOnly = 0;
            }
            if (AND_IS1(stMediaData.dwRWType, RW_TRACK2))
            {
                dwMag += MAG_READ_TRACK2;
                wReadCnt ++;
                wTrackOnly = 1;
            }
            if (AND_IS1(stMediaData.dwRWType, RW_TRACK3))
            {
                dwMag += MAG_READ_TRACK3;
                wReadCnt ++;
                wTrackOnly = 2;
            }

            if (wReadCnt == 0)
            {
                //stMediaData.Clear();
                Log(ThisModule, __LINE__, "介质读写: 入参检查没有可用磁道参数[%d], 不执行读磁操作",
                    stMediaData.dwRWType);
            } else  // 读磁道
            {
                if (m_bICActive == TRUE)    // 芯片上电中,先断电
                {
                    nRet = m_pDevImpl.ChipOperation(CHIP_DEACTIVE);
                    if(nRet != IDC_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "介质读写: 读磁(芯片上电状态): 断电: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %s",
                            CHIP_DEACTIVE, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
                        return ConvertImplErrCode2IDC(nRet);
                    }
                    m_bICActive = FALSE;
                }

                if (m_bICPress == TRUE)    // 触点接触状态,先释放接触
                {
                    nRet = m_pDevImpl.ChipOperation(CHIP_RELEASE);
                    if(nRet != IDC_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "介质读写: 读磁(触点接触状态): 释放接触: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %s",
                            CHIP_RELEASE, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
                        return ConvertImplErrCode2IDC(nRet);
                    }
                    m_bICPress = FALSE;
                }

                nRet = m_pDevImpl.MagneticRead(dwMag, szBuff, nBuffSize);
                if(nRet != IMP_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "介质读写: 读磁: ->MagneticRead(%d) Fail, ErrCode: %d, Return: %d",
                        dwMag, nRet, ConvertImplErrCode2IDC(nRet));
                    stMediaData.Clear();
                    return ConvertImplErrCode2IDC(nRet);
                }

                if (wReadCnt == 1)  // 只读一个磁道
                {
                    // 组织证件数据写入应答
                    stMediaData.SetData(wTrackOnly, RW_RESULT_SUCC, szBuff);
                } else  // 读多个磁道,解析
                {
                    // 解析磁道数据(分割,分割符"~")
                    QString qtStr = QString(szBuff);
                    QStringList qtList = qtStr.split("~");
                    QString qtSplitstr[3];
                    for(INT i = 0; i < qtList.size(); i++)
                    {
                        qtSplitstr[i] = qtList[i];
                    }

                    if (AND_IS1(stMediaData.dwRWType, RW_TRACK1))
                    {
                        // 组织证件数据写入应答
                        if (qtSplitstr[0].toStdString().length() < 1)
                        {
                            stMediaData.DataClear(0);
                        } else
                        {
                            stMediaData.SetData(0, RW_RESULT_SUCC, (LPSTR)qtSplitstr[0].toStdString().c_str());
                        }

                    }
                    if (AND_IS1(stMediaData.dwRWType, RW_TRACK2))
                    {
                        // 组织证件数据写入应答
                        if (qtSplitstr[1].toStdString().length() < 1)
                        {
                            stMediaData.DataClear(1);
                        } else
                        {
                            stMediaData.SetData(1, RW_RESULT_SUCC, (LPSTR)qtSplitstr[1].toStdString().c_str());
                        }
                    }
                    if (AND_IS1(stMediaData.dwRWType, RW_TRACK3))
                    {
                        // 组织证件数据写入应答
                        if (qtSplitstr[1].toStdString().length() < 1)
                        {
                            stMediaData.DataClear(2);
                        } else
                        {
                            stMediaData.SetData(2, RW_RESULT_SUCC, (LPSTR)qtSplitstr[2].toStdString().c_str());
                        }
                    }
                }
            }

            // ----------------读芯片处理----------------
            if (AND_IS1(stMediaData.dwRWType, RW_CHIP))
            {
                stMediaData.DataClear(3);

                // 芯片卡走位+触点接触操作
                nRet = m_pDevImpl.ChipOperation(CHIP_PRESS);
                if(nRet != IMP_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "介质读写: 读芯片: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %d",
                        SND_CMD_CHIP_PRESS, nRet, ConvertImplErrCode2IDC(nRet));
                    stMediaData.Clear();
                    return ConvertImplErrCode2IDC(nRet);
                }
                m_bICPress = TRUE;

                // 芯片卡激活
                MSET_0(szBuff);
                nRet = m_pDevImpl.ChipOperation(CHIP_ACTIVE, szBuff, &nBuffSize);
                if(nRet != IMP_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "介质读写: 读芯片: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %d",
                        SND_CMD_CHIP_PRESS, nRet, ConvertImplErrCode2IDC(nRet));
                    stMediaData.Clear();
                    return ConvertImplErrCode2IDC(nRet);
                }
                stMediaData.SetData(3, RW_RESULT_SUCC, szBuff, nBuffSize);
                m_bICActive = TRUE;
            }
        } else
        {
            Log(ThisModule, __LINE__, "介质读写: 不支持的入参[stMediaData.wRWType = %d], Return: %s",
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
int CDevIDC_CRT350N::ChipReadWrite(CHIP_RW_MODE enChipMode, STCHIPRW &stChipData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    if (enChipMode == CHIP_POW_COLD ||  // 冷复位
        enChipMode == CHIP_POW_WARM)    // 热复位
    {
        if (m_bICPress == FALSE)    // 触点未接触状态,设置接触
        {
            nRet = m_pDevImpl.ChipOperation(CHIP_PRESS);
            if(nRet != IDC_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "芯片读写: 冷复位|热复位: 触点压下: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %s",
                    CHIP_PRESS, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
                return ConvertImplErrCode2IDC(nRet);
            }
            m_bICPress = FALSE;
        }

        // 芯片卡冷复位/激活
        stChipData.stData[1].szData;
        INT nDataSize = sizeof(stChipData.stData[1].szData);
        nRet = m_pDevImpl.ChipOperation(CHIP_ACTIVE, stChipData.stData[1].szData,
                                        /*sizeof(stChipData.stData[1].szData)*/
                                        &nDataSize);
        if(nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: 冷复位|热复位: 上电: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %d",
                CHIP_ACTIVE, nRet, ConvertImplErrCode2IDC(nRet));
            stChipData.Clear();
            return ConvertImplErrCode2IDC(nRet);
        }
        stChipData.stData[1].dwSize = nDataSize;//strlen(stChipData.stData[1].szData);
        m_bICActive = TRUE;
    } else
    if (enChipMode == CHIP_POW_OFF)     // 断电
    {
        nRet = m_pDevImpl.ChipOperation(CHIP_DEACTIVE);
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: 断电: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %s",
                CHIP_DEACTIVE, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
            return ConvertImplErrCode2IDC(nRet);
        }
        m_bICActive = FALSE;
    } else
    if (enChipMode == CHIP_IO_T0 ||     // T0协议通信
        enChipMode == CHIP_IO_T1)       // T1协议通信
    {
        nRet = m_pDevImpl.ChipProtocol(CHIP_COMMAUTO,
                                       stChipData.stData[0].szData, stChipData.stData[0].dwSize,
                                       stChipData.stData[1].szData, (DWORD&)stChipData.stData[1].dwSize);
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "芯片读写: T0协议通信|T1协议通信: ->SendReaderAPDU(%s, %d) Fail, ErrCode: %d, Return: %d",
                stChipData.stData[0].szData, stChipData.stData[0].dwSize,
                nRet, ConvertImplErrCode2IDC(nRet));
            return ConvertImplErrCode2IDC(nRet);
        }
        m_bICActive = TRUE;
    } else
    {
        Log(ThisModule, __LINE__, "芯片读写: 不支持的入参[enChipMode = %d], Return: %s",
            enChipMode, ConvertDevErrCodeToStr(ERR_IDC_PARAM_ERR));
        return ERR_IDC_PARAM_ERR;
    }

    return IDC_SUCCESS;
}

// 设置数据
int CDevIDC_CRT350N::SetData(unsigned short usType, void *vData)
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
                // 设置命令收发超时
                if (m_stOpenMode.nOtherParam[0] > 0 || m_stOpenMode.nOtherParam[1] > 0)
                {
                    m_pDevImpl.SetSndRcvTimeOut(m_stOpenMode.nOtherParam[0],
                                                m_stOpenMode.nOtherParam[1]);
                }
                SetResetParam();    // 设置复位/初始化辅助参数
            }
            break;
        }
        case SET_BEEP_CONTROL:      // 设备鸣响控制
        {
            break;
        }
        case SET_GLIGHT_CONTROL:    // 指示灯控制
        {
            break;
        }
        case SET_DEV_PREIC:         // 设置进卡检查模式
        {
            if (vData != nullptr)
            {
                //return m_pDevImpl.SetPredictIC(*((WORD*)(vData)));
                m_wInCardParam = *((WORD*)(vData));
            }
            break;
        }
        case SET_DEV_RETAINCNT:     // 设置硬件回收计数
        {
            if (vData == nullptr)
            {
                return m_pDevImpl.SetDevRetainCnt(0);
            } else
            {
                return m_pDevImpl.SetDevRetainCnt(*((WORD*)(vData)));
            }
        }
        case SET_DEV_AUXPARAM:      // 设置硬件辅助参数
        {
            memcpy(m_nAuxParam, vData, sizeof(INT) * 64);
            break;
        }
        case SET_DEV_WOBBLE_OPEN:   // 设置开启抖动进卡支持
        {
            Log(ThisModule, __LINE__, "开启抖动进卡");
            m_pDevImpl.CardAcceptASync(INCARD_SHAKE, 1);
            break;
        }
        case SET_DEV_WOBBLE_CLOSE:   // 设置关闭抖动进卡支持
        {
            Log(ThisModule, __LINE__, "关闭抖动进卡");
            m_pDevImpl.CardAcceptASync(INCARD_SHAKE, 0);
            break;
        }
        case SET_SND_NOTICE_2DEV:   // 发送通知消息到DevXXX
        {
            if (vData != nullptr)
            {
                if (*((WORD*)(vData)) == 1) // 检知有异物
                {
                    Log(ThisModule, __LINE__, "设置检知有异物标记:T");
                    m_bIsSkimmingHave = TRUE; // 检知发现异物标记
                } else
                if (*((WORD*)(vData)) == 0) // 检知无异物
                {
                    Log(ThisModule, __LINE__, "设置检知有异物标记:F");
                    m_bIsSkimmingHave = FALSE; // 检知发现异物标记
                }
            }
            break;
        }
        default:
            break;
    }

    return IDC_SUCCESS;
}

// 获取数据
int CDevIDC_CRT350N::GetData(unsigned short usType, void *vData)
{
    THISMODULE(__FUNCTION__);

    switch(usType)
    {
        case GET_DEV_FRAUDDETE:     // 防逗卡保护是否生效中
        {
            return IsTeaseCardProtect();
        }        
        case GET_DEV_ERRCODE:       // 取DevXXX错误码
        {
            if (vData != nullptr)
            {
                //MCPY_NOLEN((LPSTR)vData, m_clErrorDet.GetSPErrCode());
                m_clErrorDet.GetSPErrCode((LPSTR)vData);
            }
            break;
        }
        default:
            break;
    }

    return IDC_SUCCESS;
}

// 获取版本
int CDevIDC_CRT350N::GetVersion(unsigned short usType, char* szVer, int nSize)
{
    THISMODULE(__FUNCTION__);

    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_FW:        // 固件版本号
            {
                if (szVer != nullptr && nSize > 0)
                {
                    m_pDevImpl.GetVersion(0, szVer, nSize);
                }
                break;
            }
            default:
                break;
        }
    }

    return IDC_SUCCESS;
}

// 进卡处理
INT CDevIDC_CRT350N::AcceptMedia(MEDIA_ACTION enMediaAct, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    ULONG ulTimeCount = 0;
    STDEVIDCSTATUS stDevStat;
    //STDEVIDCSTATUS stDevStatTmp;
    INT nRet = IMP_SUCCESS;
    EN_INCARD_PAR enInCardParam = INCARD_ALL;
    BOOL bIsAfterInCard = FALSE;                // 是否后出口进卡
    WORD wMediaStatLast = MEDIA_STAT_UNKNOWN;   // 记录上一次介质状态

    QTime qtTimeCurr1, qtTimeCurr2;
    qtTimeCurr1 = QTime::currentTime();     // 取吸卡执行前时间

    // 检测读卡器防盗钩状态
    if (m_nAuxParam[0] == 1)    // INI配置启用防盗钩
    {
        nRet = m_pDevImpl.GetTamperStat(m_nTamperSensorStat);
        if(nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "进卡处理: 防盗钩状态: ->GetTamperStat() Fail, ErrCode: %d, Return: %s",
                nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
            return ConvertImplErrCode2IDC(nRet);
        }

        if (m_nTamperSensorStat == IMPL_TAMPER_PRESS)   // 未释放
        {
            Log(ThisModule, __LINE__, "介质控制: 防盗钩未释放: 读卡器异型口被用手或者其他东西挡住");
            m_clErrorDet.SetDevErrCode((LPSTR)EC_IDC_DEV_TamperNotRelease);
            return ERR_IDC_DETEC_TAMPER;
        }
    }

    // 取当前状态
    stDevStat.Clear();
    nRet = GetStatus(stDevStat);
    if (nRet != IDC_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "进卡处理: 取当前状态: ->GetStatus() Fail, ErrCode: %d, Return: %s",
            nRet, ConvertDevErrCodeToStr(nRet));
        return nRet;
    }

    if (stDevStat.wMedia == MEDIA_STAT_PRESENT)     // 介质在内部
    {
        BOOL bIsRunRelease = FALSE;
        if (m_bICActive == TRUE)    // 卡片处于激活(上电)中
        {
            nRet = m_pDevImpl.ChipOperation(CHIP_DEACTIVE);
            if(nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "进卡处理: 在内部(上电状态): 执行断电: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %s",
                    CHIP_DEACTIVE, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
                return ConvertImplErrCode2IDC(nRet);
            }
            m_bICActive = FALSE;
            bIsRunRelease = TRUE;
        }

        if (m_bICPress == TRUE || bIsRunRelease == TRUE)    // 卡片处于触点接触状态
        {
            nRet = m_pDevImpl.ChipOperation(CHIP_RELEASE);
            if(nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "进卡处理: 在内部(触点接触状态): 释放接触: ->ChipOperation(%d) Fail, ErrCode: %d, Return: %s",
                    CHIP_RELEASE, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
                return ConvertImplErrCode2IDC(nRet);
            }
            m_bICPress = FALSE;
            bIsRunRelease = FALSE;
        }

        return IDC_SUCCESS;
    } else
    if (stDevStat.wMedia == MEDIA_STAT_ENTERING)    // 介质在出口:重进卡
    {
        wMediaStatLast = MEDIA_STAT_ENTERING;
        nRet = m_pDevImpl.CardReAccept();
        if(nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "进卡处理: 在出口: 重进卡: ->CardReAccept() Fail, ErrCode: %d, Return: %s",
                nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
            return ConvertImplErrCode2IDC(nRet);
        }
        return IDC_SUCCESS;
    } else  // 无卡: 下发进卡命令
    {
        wMediaStatLast = MEDIA_STAT_NOTPRESENT;
        switch(m_wInCardParam)      // 按INI指定进卡参数+入参设定
        {
            case 0: // 所有卡
                enInCardParam = INCARD_ALL;
                break;
            case 1: // 检查磁卡或芯片卡
                enInCardParam = (enMediaAct == MEDIA_ACCEPT) ? INCARD_MAG3IC : INCARD_IC;
                break;
            case 2: // 检查磁卡和芯片卡
                enInCardParam = (enMediaAct == MEDIA_ACCEPT) ? INCARD_MAG2IC : INCARD_IC;
                break;
            default:
                enInCardParam = (enMediaAct == MEDIA_ACCEPT) ? INCARD_MAG3IC : INCARD_IC;
                break;
        }

        nRet = m_pDevImpl.CardAcceptASync(enInCardParam);
        if(nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "进卡处理: 下发进卡命令: ->CardAcceptASync(%d) Fail, ErrCode: %d, Return: %s",
                enInCardParam, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2IDC(nRet)));
            return ConvertImplErrCode2IDC(nRet);
        }
    }

    // 循环检测是否放入卡
    while(1)
    {
        QCoreApplication::processEvents();
        if (m_bCancelReadCard == TRUE)      // 取消进卡
        {
            m_bCancelReadCard = FALSE;

            // 取当前状态
            /*stDevStat.Clear();
            GetStatus(stDevStat);
            if (stDevStat.wOtherCode[0] == IMPL_STAT_CARD_ISAFTER_EXPORT)
            {
                bIsAfterInCard = TRUE;  // 后出口进卡
            }*/
            m_pDevImpl.CardAcceptASync(INCARD_NOT); // 禁止进卡

            return ERR_IDC_USER_CANCEL;
        }

        if (m_bIsSkimmingHave == TRUE)      // 检知发现异物
        {
            Log(ThisModule, __LINE__,
                "进卡处理: 进卡检查中: 检知发现异物: 终止进卡处理, Return: %s",
                ConvertDevErrCodeToStr(ERR_IDC_DETEC_TAMPER));
            m_pDevImpl.CardAcceptASync(INCARD_NOT); // 禁止进卡

            return ERR_IDC_DETEC_TAMPER;
        }

        // 取当前状态
        stDevStat.Clear();
        nRet = GetStatus(stDevStat);
        if (nRet == IDC_SUCCESS)
        {
            if (stDevStat.wMedia == MEDIA_STAT_PRESENT)     // 介质在内部
            {
                break;
            } else
            {
                if (wMediaStatLast == MEDIA_STAT_ENTERING &&
                    stDevStat.wMedia == MEDIA_STAT_NOTPRESENT)   // 卡口+内部无卡
                {
                    if (m_nAuxParam[1] == 1)   // 支持启动防逗卡保护
                    {
                        m_wTeaseReInCardCount ++;   // 防逗卡进卡次数+1
                        if (m_wTeaseReInCardCount >= m_nAuxParam[2])   // 防逗卡进卡次数 >= 上限
                        {
                            m_bIsTeaseCardProtectRun = TRUE;  // 防逗卡保护启动
                            m_qtTeaseCardProtRunDate = QTime::currentTime();                   // 记录防逗卡保护启动时间
                            Log(ThisModule, __LINE__,
                                "进卡处理: 进卡: 反复进卡次数[%d]超限[>=%d]: 启动防逗卡保护, Return: %s",
                                m_wTeaseReInCardCount, m_nAuxParam[2],
                                ConvertDevErrCodeToStr(ERR_IDC_DETEC_FRAUD));
                            m_pDevImpl.CardAcceptASync(INCARD_NOT); // 禁止进卡
                            return ERR_IDC_DETEC_FRAUD;
                        }
                    }
                }
            }
            wMediaStatLast = stDevStat.wMedia;
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
                    "进卡处理: 进卡: 已等待时间[%d] > 指定超时时间[%d], TimeOut, Return: %s",
                    ulTimeCount, dwTimeOut, ConvertDevErrCodeToStr(ERR_IDC_INSERT_TIMEOUT));
                m_pDevImpl.CardAcceptASync(INCARD_NOT); // 禁止进卡
                return ERR_IDC_INSERT_TIMEOUT;
            }
        }

        usleep(1000 * 50);     // 休眠100毫秒

        continue;
    }

    m_pDevImpl.CardAcceptASync(INCARD_NOT); // 禁止进卡

    //return IDC_SUCCESS;
    return bIsAfterInCard == TRUE ? ERR_IDC_INCARD_AFTER : IDC_SUCCESS;
}

// 指示灯控制
INT CDevIDC_CRT350N::ControlLight(STGLIGHTSCONT stLignt)
{
    return IDC_SUCCESS;
}

// 鸣响控制
INT CDevIDC_CRT350N::ControlBeep(STBEEPCONT stBeep)
{
    return IDC_SUCCESS;
}

// 设置复位/初始化辅助参数
INT CDevIDC_CRT350N::SetResetParam()
{
    // m_stDevOpenMode.nOtherParam[0]: 掉电时卡的控制方式,缺省2
    // 0:退卡到前出口; 1:写磁时不退卡,其他情况退卡; 2:无动作(不退卡); 3:退卡到前出口,30秒未取则吞卡; 4: 吞卡
    switch(m_stOpenMode.nOtherParam[10])
    {
        case 0: m_szResetParam[5] = '0'; break;
        case 1: m_szResetParam[5] = '1'; break;
        case 2: m_szResetParam[5] = '2'; break;
        case 3: m_szResetParam[5] = '3'; break;
        case 4: m_szResetParam[5] = '5'; break;
    }

    // m_stDevOpenMode.nOtherParam[1]: 掉电时写卡命令处理(0:停止写; 1:不停止写, 缺省0)
    switch(m_stOpenMode.nOtherParam[11])
    {
        case 0: m_szResetParam[6] = '0'; break;
        case 1: m_szResetParam[6] = '1'; break;
    }

    // m_stDevOpenMode.nOtherParam[2]: 初始化时是否测试闸门开关(0:不测试, 1:测试, 缺省0)
    switch(m_stOpenMode.nOtherParam[12])
    {
        case 0: m_szResetParam[7] = '1'; break;
        case 1: m_szResetParam[7] = '0'; break;
    }

    return IDC_SUCCESS;
}

// 防逗卡检测处理
// 0:防逗卡保护未启动, 1:防逗卡保护已启动
INT CDevIDC_CRT350N::IsTeaseCardProtect()
{
    if (m_nAuxParam[1] == 1)                            // 支持防逗卡检测
    {
        if (m_bIsTeaseCardProtectRun == TRUE)           // 防逗卡保护已启动
        {
            if (m_nAuxParam[3] <= 0)                    // 防逗卡保护持续时间 <= 0
            {
                return 1;
            } else
            {
                if (m_qtTeaseCardProtRunDate.msecsTo(QTime::currentTime()) <
                    m_nAuxParam[3] * 1000)
                {
                    return 1;
                } else
                {
                    m_bIsTeaseCardProtectRun = FALSE;
                    m_wTeaseReInCardCount= 0;           // 记录防逗卡进卡计数清零
                    return 0;
                }
            }
        }
    }

    return 0;
}

// Impl错误码转换为IDC错误码
INT CDevIDC_CRT350N::ConvertImplErrCode2IDC(INT nRet)
{
#define CASE_RET_DEVCODE(IMP, RET) \
        case IMP: return RET;

    ConvertImplErrCode2ErrDetail(nRet);

    switch(nRet)
    {
        // > 100: Impl处理返回
        CASE_RET_DEVCODE(IMP_SUCCESS, IDC_SUCCESS)                          // 成功
        CASE_RET_DEVCODE(IMP_ERR_LOAD_LIB, ERR_IDC_LIBRARY)                 // 动态库加载失败
        CASE_RET_DEVCODE(IMP_ERR_PARAM_INVALID, ERR_IDC_PARAM_ERR)          // 参数无效
        CASE_RET_DEVCODE(IMP_ERR_READERROR, ERR_IDC_READ_ERR)               // 读数据错误
        CASE_RET_DEVCODE(IMP_ERR_WRITEERROR, ERR_IDC_WRITE_ERR)             // 写数据错误
        CASE_RET_DEVCODE(IMP_ERR_RCVDATA_INVALID, ERR_IDC_RESP_ERR)         // 无效的应答数据
        CASE_RET_DEVCODE(IMP_ERR_RCVDATA_NOTCOMP, ERR_IDC_RESP_NOT_COMP)    // 无效的应答数据
        CASE_RET_DEVCODE(IMP_ERR_UNKNOWN, ERR_IDC_OTHER)                    // 未知错误
        // <0 : USB/COM接口处理返回
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_NOTOPEN, ERR_IDC_DEV_NOTOPEN)      // (-1) 没打开
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_FAIL, ERR_IDC_COMM_ERR)            // (-2) 通讯错误
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_PARAM, ERR_IDC_PARAM_ERR)          // (-3) 参数错误
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_CANCELED, ERR_IDC_USER_CANCEL)     // (-4) 操作取消
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_READERR, ERR_IDC_READ_ERR)         // (-5) 读取错误
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_WRITE, ERR_IDC_WRITE_ERR)          // (-6) 发送错误
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_RTIMEOUT, ERR_IDC_READ_TIMEOUT)    // (-7) 操作超时
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_WTIMEOUT, ERR_IDC_WRITE_TIMEOUT)   // (-8) 操作超时
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_LIBRARY, ERR_IDC_LIBRARY)          // (-98) 加载通讯库失败
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_NODEFINED, ERR_IDC_OTHER)          // (-99) 未知错误
        // 0~100: 硬件设备返回
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_00, ERR_IDC_PARAM_ERR)              // 命令编码未定义
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_01, ERR_IDC_PARAM_ERR)              // 命令参数错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_02, ERR_IDC_COMM_RUN)               // 命令无法执行
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_03, ERR_IDC_UNSUP_CMD)              // 硬件不支持
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_04, ERR_IDC_PARAM_ERR)              // 命令数据错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_05, ERR_IDC_MED_STAT_ERR)           // IC触点未释放
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_06, ERR_IDC_OTHER)                  // 密钥不存在
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_07, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_08, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_09, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_10, ERR_IDC_MED_JAMMED)             // 卡堵塞
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_11, ERR_IDC_DEV_HWERR)              // Shuter错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_12, ERR_IDC_OTHER)                  // 传感错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_13, ERR_IDC_MED_LONG)               // 不规则卡长度(过长)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_14, ERR_IDC_MED_SHORT)              // 不规则卡长度(过短)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_15, ERR_IDC_OTHER)                  // FRAM错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_16, ERR_IDC_DEV_HWERR)              // 卡位置移动
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_17, ERR_IDC_MED_JAMMED)             // 重进卡时卡堵塞
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_18, ERR_IDC_OTHER)                  // SW1,SW2错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_19, ERR_IDC_OTHER)                  // 卡没有从后端插入
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_20, ERR_IDC_READ_ERR)               // 读磁卡错误(奇偶校验错)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_21, ERR_IDC_READ_ERR)               // 读磁卡错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_22, ERR_IDC_WRITE_ERR)              // 写磁卡错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_23, ERR_IDC_READ_ERR)               // 读磁卡错误(没有数据内容,只有STX起始符,ETX结束符和LRC)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_24, /*ERR_IDC_READ_ERR*/ERR_IDC_MED_INV)// 读磁卡错误(没有磁条或没有编码-空白轨道)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_25, ERR_IDC_WRITE_ERR)              // 写磁卡校验错误(品质错误)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_26, ERR_IDC_READ_ERR)               // 读磁卡错误(没有SS)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_27, ERR_IDC_READ_ERR)               // 读磁卡错误(没有ES)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_28, ERR_IDC_READ_ERR)               // 读磁卡错误(LRC错误)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_29, ERR_IDC_WRITE_ERR)              // 写磁卡校验错误(数据不一致)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_30, ERR_IDC_DEV_HWERR)              // 电源掉电
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_31, ERR_IDC_DEV_HWERR)              // DSR信号为OFF
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_32, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_40, ERR_IDC_MED_PULLOUT)            // 吞卡时卡拔走
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_41, ERR_IDC_DEV_STAT_ERR)           // IC触点或触点传感器错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_42, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_43, ERR_IDC_DEV_HWERR)              // 无法走到IC卡位
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_44, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_45, ERR_IDC_DEV_HWERR)              // 卡机强制弹卡
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_46, ERR_IDC_USER_ERR)               // 前端卡未在指定时间内取走
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_47, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_48, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_49, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_50, ERR_IDC_DEV_HWERR)              // 回收卡计数溢出
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_51, ERR_IDC_DEV_HWERR)              // 马达错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_52, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_53, ERR_IDC_OTHER)                  // 数字解码读错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_54, ERR_IDC_DEV_HWERR)              // 防盗钩移动错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_55, ERR_IDC_COMM_RUN)               // 防盗钩已经设置,命令不能执行
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_56, ERR_IDC_DEV_STAT_ERR)           // 芯片检测传感器错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_57, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_58, ERR_IDC_DEV_HWERR)              // 防盗钩正在移动
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_59, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_60, ERR_IDC_OTHER)                  // IC卡或SAM卡Vcc条件异常
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_61, ERR_IDC_COMM_ERR)               // IC卡或SAM卡ATR通讯错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_62, ERR_IDC_OTHER)                  // IC卡或SAM卡在当前激活条件下ATR无效
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_63, ERR_IDC_COMM_ERR)               // IC卡或SAM卡通讯过程中无响应
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_64, ERR_IDC_COMM_ERR)               // IC卡或SAM卡通讯错误(除无响应外)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_65, ERR_IDC_OTHER)                   // IC卡或SAM卡未激活
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_66, ERR_IDC_OTHER)                  // IC卡或SAM卡不支持(仅对于非EMV激活)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_67, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_68, ERR_IDC_UNKNOWN)                //
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_69, ERR_IDC_OTHER)                  // IC卡或SAM卡不支持(仅对于EMV激活)
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_76, ERR_IDC_COMM_ERR)               // ESU模块和卡机通讯错误
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_95, ERR_IDC_DEV_HWERR)              // ESU模块损坏或无连接
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_99, ERR_IDC_DEV_HWERR)              // ESU模块过流
        CASE_RET_DEVCODE(IMP_ERR_DEVRET_B0, ERR_IDC_COMM_RUN)               // 未接收到初始化命令
        default: return ERR_IDC_OTHER;
    }

    return IDC_SUCCESS;
}

// 根据Impl错误码设置错误错误码字符串
INT CDevIDC_CRT350N::ConvertImplErrCode2ErrDetail(INT nRet)
{
#define CASE_SET_DEV_DETAIL(IMP, STR) \
        case IMP: m_clErrorDet.SetDevErrCode((LPSTR)STR); break;

#define CASE_SET_HW_DETAIL(IMP, DSTR, HSTR) \
        case IMP: \
            m_clErrorDet.SetDevErrCode((LPSTR)DSTR); \
            m_clErrorDet.SetHWErrCodeStr((LPSTR)HSTR, (LPSTR)"1"); break;

    switch(nRet)
    {
        // > 100: Impl处理返回
        //CASE_RET_DEVCODE(IMP_SUCCESS, IDC_SUCCESS)                        // 成功
        CASE_SET_DEV_DETAIL(IMP_ERR_LOAD_LIB, EC_DEV_LibraryLoadFail)       // 动态库加载失败
        CASE_SET_DEV_DETAIL(IMP_ERR_PARAM_INVALID, EC_DEV_ParInvalid)       // 参数无效
        CASE_SET_DEV_DETAIL(IMP_ERR_READERROR, EC_DEV_DataRWErr)            // 读数据错误
        CASE_SET_DEV_DETAIL(IMP_ERR_WRITEERROR, EC_DEV_DataRWErr)           // 写数据错误
        CASE_SET_DEV_DETAIL(IMP_ERR_RCVDATA_INVALID, EC_DEV_RecvData_inv)   // 无效的应答数据
        CASE_SET_DEV_DETAIL(IMP_ERR_RCVDATA_NOTCOMP, EC_DEV_ReadData_NotComp)// 无效的应答数据
        CASE_SET_DEV_DETAIL(IMP_ERR_UNKNOWN, EC_DEV_UnKnownErr)             // 未知错误
        // <0 : USB/COM接口处理返回
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_NOTOPEN, EC_DEV_DevOpenFail)    // (-1) 没打开
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_FAIL, EC_DEV_ConnFail)          // (-2) 通讯错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_PARAM, EC_DEV_ParInvalid)       // (-3) 参数错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_READERR, EC_DEV_DataRWErr)      // (-5) 读取错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_WRITE, EC_DEV_DataRWErr)        // (-6) 发送错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_RTIMEOUT, EC_DEV_CommTimeOut)   // (-7) 操作超时
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_WTIMEOUT, EC_DEV_CommTimeOut)   // (-8) 操作超时
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_LIBRARY, EC_DEV_LibraryLoadFail)// (-98) 加载通讯库失败
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_NODEFINED, EC_DEV_UnKnownErr)   // (-99) 未知错误
        // 0~100: 硬件设备返回
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_00, EC_DEV_Unsup_CMD, "0000000")  // 命令编码未定义
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_01, EC_DEV_HWParInvalid, "0000001")// 命令参数错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_02, EC_DEV_DevHWErr, "0000002")   // 命令无法执行
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_03, EC_DEV_DevNotSupp, "0000003") // 硬件不支持
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_04, EC_DEV_HWParInvalid, "0000004")// 命令数据错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_05, EC_DEV_DevHWErr, "0000005")   // IC触点未释放
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_06, EC_DEV_DevHWErr, "0000006")   // 密钥不存在
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_07, EC_DEV_UnKnownErr, "0000007") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_08, EC_DEV_UnKnownErr, "0000008") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_09, EC_DEV_UnKnownErr, "0000009") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_10, EC_DEV_MedJammed, "0000010")  // 卡堵塞
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_11, EC_DEV_DevHWErr, "0000011")   // Shuter错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_12, EC_DEV_DevHWErr, "0000012")   // 传感错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_13, EC_DEV_MedInvalid, "0000013") // 不规则卡长度(过长)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_14, EC_DEV_MedInvalid, "0000014") // 不规则卡长度(过短)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_15, EC_DEV_DevHWErr, "0000015")   // FRAM错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_16, EC_DEV_DevHWErr, "0000016")   // 卡位置移动
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_17, EC_DEV_MedJammed, "0000017")  // 重进卡时卡堵塞
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_18, EC_DEV_DevHWErr, "0000018")   // SW1,SW2错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_19, EC_DEV_DevHWErr, "0000019")   // 卡没有从后端插入
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_20, EC_DEV_DevHWErr, "0000020")   // 读磁卡错误(奇偶校验错)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_21, EC_DEV_DevHWErr, "0000021")   // 读磁卡错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_22, EC_DEV_DevHWErr, "0000022")   // 写磁卡错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_23, EC_DEV_DevHWErr, "0000023")   // 读磁卡错误(没有数据内容,只有STX起始符,ETX结束符和LRC)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_24, EC_DEV_DevHWErr, "0000024")   // 读磁卡错误(没有磁条或没有编码-空白轨道)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_25, EC_DEV_DevHWErr, "0000025")   // 写磁卡校验错误(品质错误)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_26, EC_DEV_DevHWErr, "0000026")   // 读磁卡错误(没有SS)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_27, EC_DEV_DevHWErr, "0000027")   // 读磁卡错误(没有ES)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_28, EC_DEV_DevHWErr, "0000028")   // 读磁卡错误(LRC错误)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_29, EC_DEV_DevHWErr, "0000029")   // 写磁卡校验错误(数据不一致)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_30, EC_DEV_DevHWErr, "0000030")   // 电源掉电
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_31, EC_DEV_DevHWErr, "0000031")   // DSR信号为OFF
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_32, EC_DEV_UnKnownErr, "0000032") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_40, EC_DEV_DevHWErr, "0000040")   // 吞卡时卡拔走
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_41, EC_DEV_DevHWErr, "0000041")   // IC触点或触点传感器错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_42, EC_DEV_UnKnownErr, "0000042") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_43, EC_DEV_DevHWErr, "0000043")   // 无法走到IC卡位
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_44, EC_DEV_UnKnownErr, "0000044") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_45, EC_DEV_DevHWErr, "0000045")   // 卡机强制弹卡
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_46, EC_DEV_DevHWErr, "0000046")   // 前端卡未在指定时间内取走
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_47, EC_DEV_UnKnownErr, "0000047") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_48, EC_DEV_UnKnownErr, "0000048") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_49, EC_DEV_UnKnownErr, "0000049") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_50, EC_DEV_DevHWErr, "0000050")   // 回收卡计数溢出
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_51, EC_DEV_DevHWErr, "0000051")   // 马达错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_52, EC_DEV_UnKnownErr, "0000052") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_53, EC_DEV_DevHWErr, "0000053")   // 数字解码读错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_54, EC_DEV_DevHWErr, "0000054")   // 防盗钩移动错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_55, EC_DEV_DevHWErr, "0000055")   // 防盗钩已经设置,命令不能执行
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_56, EC_DEV_DevHWErr, "0000056")   // 芯片检测传感器错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_57, EC_DEV_UnKnownErr, "0000057") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_58, EC_DEV_DevHWErr, "0000058")   // 防盗钩正在移动
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_59, EC_DEV_UnKnownErr, "0000059") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_60, EC_DEV_DevHWErr, "0000060")   // IC卡或SAM卡Vcc条件异常
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_61, EC_DEV_DevHWErr, "0000061")   // IC卡或SAM卡ATR通讯错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_62, EC_DEV_DevHWErr, "0000062")   // IC卡或SAM卡在当前激活条件下ATR无效
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_63, EC_DEV_DevHWErr, "0000063")   // IC卡或SAM卡通讯过程中无响应
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_64, EC_DEV_DevHWErr, "0000064")   // IC卡或SAM卡通讯错误(除无响应外)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_65, EC_DEV_DevHWErr, "0000065")   // IC卡或SAM卡未激活
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_66, EC_DEV_DevHWErr, "0000066")   // IC卡或SAM卡不支持(仅对于非EMV激活)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_67, EC_DEV_UnKnownErr, "0000067") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_68, EC_DEV_UnKnownErr, "0000068") //
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_69, EC_DEV_MedNotSupp, "0000069") // IC卡或SAM卡不支持(仅对于EMV激活)
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_76, EC_DEV_DevHWErr, "0000076")   // ESU模块和卡机通讯错误
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_95, EC_DEV_DevHWErr, "0000095")   // ESU模块损坏或无连接
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_99, EC_DEV_DevHWErr, "0000099")   // ESU模块过流
        CASE_SET_HW_DETAIL(IMP_ERR_DEVRET_B0, EC_DEV_DevHWErr, "00000B0")   // 未接收到初始化命令
    }

    return IDC_SUCCESS;
}
