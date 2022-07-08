#include "DevCRD_ACTU6SG5.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>

static const char *ThisModule = "DevCRD_ACTU6SG5.cpp";

//////////////////////////////////////////////////////////////////////////

CDevCRD_ACTU6SG5::CDevCRD_ACTU6SG5() : m_devACTU6SG5(LOG_NAME_DEVCRD)
{
    SetLogFile(LOG_NAME_DEVCRD, ThisModule);  // 设置日志文件名和错误发生的文件
}

CDevCRD_ACTU6SG5::~CDevCRD_ACTU6SG5()
{
    Close();
}

void CDevCRD_ACTU6SG5::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    return;
}

// 建立CRD连接
int CDevCRD_ACTU6SG5::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // 建立连接
    if (m_devACTU6SG5.IsDeviceOpen() != TRUE)
    {
        if ((nRet = m_devACTU6SG5.DeviceOpen((LPSTR)lpMode)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "建立连接: ->DeviceOpen(%s) Fail, ErrCode=%d, Return %s.",
                lpMode, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    }

    return CRD_SUCCESS;
}

// CRD初始化
int CDevCRD_ACTU6SG5::Init(INT eActFlag)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return CRD_SUCCESS;
}

// 关闭CRD连接
int CDevCRD_ACTU6SG5::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_devACTU6SG5.DeviceClose();
    Log(ThisModule, __LINE__, "关闭连接: ->DeviceClose() Succ, Return %s.",
        ConvertErrCodeToStr(CRD_SUCCESS));

    return CRD_SUCCESS;
}

// 设备复位
int CDevCRD_ACTU6SG5::Reset(int nMode, int nParam)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    USHORT usReset = 0;

    if (nMode == RESET_NOACTION)    // 无动作
    {
        usReset = MODE_RESET_NOACTIVE;
    } else
    if (nMode == RESET_EJECT)       // 退卡
    {
        usReset = MODE_RESET_EJECT;
    } else
    if (nMode == RESET_RETAIN)      // 回收
    {
        usReset = MODE_RESET_RETAIN;
    } else
    {
        Log(ThisModule, __LINE__, "设备复位: 入参[%d]无效, Return %s.",
            nMode, nRet, ConvertErrCodeToStr(ERR_CRD_PARAM_ERR));
        return ERR_CRD_PARAM_ERR;
    }

    // 下发复位
    if ((nRet = m_devACTU6SG5.DeviceReset(usReset)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备复位: ->DeviceReset(%d) Fail, ErrCode=%d, Return %s.",
            usReset, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return CRD_SUCCESS;
}

// 读取设备状态
int CDevCRD_ACTU6SG5::GetDevStat(STCRDDEVSTATUS &stStat)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);


    INT nRet = IMP_SUCCESS;
    INT nStatus[12];
    memset(nStatus, 0, sizeof(INT) * 12);

    stStat.Clear();

    // 获取状态    
    nRet = m_devACTU6SG5.GetDeviceStat(0, nStatus);
    /*if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读取设备状态: ->GetDeviceStat() Fail, ErrCode=%d, Return %s.",
            nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }*/

    // ----------------状态解析----------------
    // 设备状态
    if (nStatus[0] == DEV_STAT_OFFLINE)     // 设备断线
    {
        stStat.wDevice = DEVICE_STAT_OFFLINE;
    } else
    if (nStatus[0] == DEV_STAT_HWERR)      // 设备故障
    {
        stStat.wDevice = DEVICE_STAT_HWERROR;
    } else
    {
        stStat.wDevice = DEVICE_STAT_ONLINE;
    }

    // UKEY箱解析
    stStat.stUnitInfo.usUnitType[0] = UNIT_STORAGE;
    stStat.stUnitInfo.usUnitType[1] = UNIT_STORAGE;
    stStat.stUnitInfo.usUnitType[2] = UNIT_RETRACT;
    if (nStatus[1] == STAT_NOHAVE)    // UKEY箱1不存在
    {
        stStat.stUnitInfo.wUnitStat[0] = UNITINFO_STAT_MISSING;
    } else
    if (nStatus[1] == STAT_UNKNOWN)    // UKEY箱1状态未知
    {
        stStat.stUnitInfo.wUnitStat[0] = UNITINFO_STAT_UNKNOWN;
    } else
    {
        if (nStatus[1] == STAT_EMPTY) // UKEY箱1无卡
        {
            stStat.stUnitInfo.wUnitStat[0] = UNITINFO_STAT_EMPTY; // 空
        } else
        {
            if (nStatus[1] == STAT_FULL) // UKEY箱1卡满
            {
                stStat.stUnitInfo.wUnitStat[0] = UNITINFO_STAT_FULL;
            } else
            {
                stStat.stUnitInfo.wUnitStat[0] = UNITINFO_STAT_LOW;   // 少
            }
        }
        stStat.stUnitInfo.wUnitCnt ++;
    }
    if (nStatus[2] == STAT_NOHAVE)    // UKEY箱2不存在
    {
        stStat.stUnitInfo.wUnitStat[1] = UNITINFO_STAT_MISSING;
    } else
    if (nStatus[2] == STAT_UNKNOWN)    // UKEY箱2状态未知
    {
        stStat.stUnitInfo.wUnitStat[1] = UNITINFO_STAT_UNKNOWN;
    } else
    {
        if (nStatus[2] == STAT_EMPTY) // UKEY箱2无卡
        {
            stStat.stUnitInfo.wUnitStat[1] = UNITINFO_STAT_EMPTY; // 空
        } else
        {
            if (nStatus[2] == STAT_FULL) // UKEY箱2卡满
            {
                stStat.stUnitInfo.wUnitStat[1] = UNITINFO_STAT_FULL;
            } else
            {
                stStat.stUnitInfo.wUnitStat[1] = UNITINFO_STAT_LOW;   // 少
            }
        }
        stStat.stUnitInfo.wUnitCnt ++;
    }

    // 回收箱解析
    if (nStatus[3] == STAT_NOHAVE)    // 回收箱箱1不存在
    {
        stStat.stUnitInfo.wUnitStat[2] = UNITINFO_STAT_MISSING;
    } else
    if (nStatus[3] == STAT_UNKNOWN)    // 回收箱箱1状态未知
    {
        stStat.stUnitInfo.wUnitStat[2] = UNITINFO_STAT_UNKNOWN;
    } else
    {
        if (nStatus[3] == STAT_EMPTY) // 回收箱箱1无卡
        {
            stStat.stUnitInfo.wUnitStat[2] = UNITINFO_STAT_EMPTY; // 空
        } else
        {
            if (nStatus[3] == STAT_FULL) // 回收箱箱1卡满
            {
                stStat.stUnitInfo.wUnitStat[2] = UNITINFO_STAT_FULL;
            } else
            {
                stStat.stUnitInfo.wUnitStat[2] = UNITINFO_STAT_EMPTY;//UNITINFO_STAT_HIGH;   // 将满
            }
        }
        stStat.stUnitInfo.wUnitCnt ++;
    }

    // 单元状态
    if (stStat.stUnitInfo.wUnitCnt == 0) // 无UKEY箱
    {
        stStat.wDispensr = DISP_STAT_STOP;
    } else
    if (stStat.stUnitInfo.wUnitCnt == 1) // 有1个UKEY箱存在
    {
        stStat.wDispensr = DISP_STAT_STATE;
    } else
    {
        stStat.wDispensr = DISP_STAT_OK;
    }

    // 传输状态
    stStat.wTransport = TRANS_STAT_OK;

    // 介质状态
    if (nStatus[4] == STAT_ISHAVE)   // 出口有卡
    {
        stStat.wMedia = MEDIA_STAT_ENTERING;
    } else
    if (nStatus[5] == STAT_ISHAVE)   // 扫描位置有卡
    {
        stStat.wMedia = MEDIA_STAT_PRESENT;
    } else
    {
        stStat.wMedia = MEDIA_STAT_NOTPRESENT;
    }

    // 门状态
    stStat.wShutter = SHUTTER_STAT_NOTSUPP;

    // 设备位置状态
    stStat.wDevicePos = DEVPOS_STAT_INPOS;

    // 反欺诈模块状态
    stStat.wAntiFraudMod = ANFRAUD_STAT_NOTSUPP;

    // 卡移动状态
    stStat.wCardMoveStat = CAREMOVE_STAT_NOTSUPP;

    return ConvertErrorCode(nRet);//CRD_SUCCESS;
}

// 读取设备信息
int CDevCRD_ACTU6SG5::GetUnitInfo(STCRDUNITINFO &stInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_CRD_UNSUP_CMD;
}

// 发卡
int CDevCRD_ACTU6SG5::DispenseCard(const int nUnitNo, const int nMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    USHORT usInPar = 0;

    // 参数组合
    if (nUnitNo == 1)
    {
        usInPar = MODE_MOVE_UKEY1_FSCAN;    // 缺省: 移动操作:箱1UKEY移动到前端扫描位置
        if (nMode == DC_BOX2FSCAN)          // 移动操作:箱UKEY移动到前端扫描位置
        {
            usInPar = MODE_MOVE_UKEY1_FSCAN;
        } else
        if (nMode == DC_BOX2DOOR)           // 移动操作:箱1UKEY直接移动到门口
        {
            usInPar = MODE_MOVE_UKEY1_DOOR;
        } else
        if (nMode == DC_BOX2FSCAN_NOS)
        {
            usInPar = MODE_MOVE_UKEY1_FSCAN;
        }
    } else
    if (nUnitNo == 2)
    {
        usInPar = MODE_MOVE_UKEY2_FSCAN;    // 缺省: 移动操作:箱2UKEY移动到前端扫描位置
        if (nMode == DC_BOX2FSCAN)          // 移动操作:箱2UKEY移动到前端扫描位置
        {
            usInPar = MODE_MOVE_UKEY2_FSCAN;
        } else
        if (nMode == DC_BOX2DOOR)           // 移动操作:箱2UKEY直接移动到门口
        {
            usInPar = MODE_MOVE_UKEY2_DOOR;
        } else
        if (nMode == DC_BOX2FSCAN_NOS)
        {
            usInPar = MODE_MOVE_UKEY2_FSCAN;
        }
    } else
    {
        Log(ThisModule, __LINE__, "设备发卡: Input Param BoxNo[%d] Invalid, Return %s.",
            nUnitNo, ConvertErrCodeToStr(ERR_CRD_PARAM_ERR));
        return ERR_CRD_PARAM_ERR;
    }

    // 调用接口
    if ((nRet = m_devACTU6SG5.MoveMedia(usInPar)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备发卡: ->MoveMedia(%d) Fail, ErrCode=%d, Return %s.",
            usInPar, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return CRD_SUCCESS;
}

// 弹卡
int CDevCRD_ACTU6SG5:: EjectCard(const int nMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    USHORT usInPar = 0;

    // 参数组合
    if (nMode == DC_FSCAN2DOOR)         // 移动操作:UKEY从扫描位置发送到门口
    {
        usInPar = MODE_MOVE_SCAN2DOOR;
    } else
    if (nMode == DC_BOX2DOOR)           // 移动操作:UKEY从出口到扫描位置
    {
        usInPar = MODE_MOVE_DOOR2SCAN;
    } else
    {
        Log(ThisModule, __LINE__, "设备弹卡: Input Param[%d] Invalid, Return %s.",
            nMode, ConvertErrCodeToStr(ERR_CRD_PARAM_ERR));
        return ERR_CRD_PARAM_ERR;
    }

    // 调用接口
    if ((nRet = m_devACTU6SG5.MoveMedia(usInPar)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备弹卡: ->MoveMedia(%d) Fail, ErrCode=%d, Return %s.",
            usInPar, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return CRD_SUCCESS;
}

// 回收卡
int CDevCRD_ACTU6SG5::RetainCard(const int nMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // 调用接口
    if ((nRet = m_devACTU6SG5.MoveMedia(MODE_MOVE_UKEY_RETAIN)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备回收卡: ->MoveMedia(%d) Fail, ErrCode=%d, Return %s.",
            MODE_MOVE_UKEY_RETAIN, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return CRD_SUCCESS;
}

// 设置数据
int CDevCRD_ACTU6SG5::SetData(void *vData, int nSize,  WORD wDataType)
{
    switch(wDataType)
    {
        case DTYPE_SET_SNDTIMEOUT:      // 设置报文发送超时时间
        {
            m_devACTU6SG5.SetTimeOut(*((UINT*)(vData)), 0);
            break;
        }
        case DTYPE_SET_RCVTIMEOUT:      // 设置报文接收超时时间
        {
            m_devACTU6SG5.SetTimeOut(0, *((UINT*)(vData)));
            break;
        }
        case DTYPE_SET_POWEROFFMODE:// 掉电处理模式
        {
            INT nParam[12];
            if (nSize == 1)
            {
                return m_devACTU6SG5.SetDeviceParam(MODE_ST_POWOFF_NOACT, nParam);
            } else
            if (nSize == 1)
            {
                return m_devACTU6SG5.SetDeviceParam(MODE_ST_POWOFF_RETAIN, nParam);
            } else
            if (nSize == 1)
            {
                return m_devACTU6SG5.SetDeviceParam(MODE_ST_POWOFF_EJECT, nParam);
            }
            break;
        }
        default:
            break;
    }

    return CRD_SUCCESS;
}

// 获取数据
int CDevCRD_ACTU6SG5::GetData(void *vData, int *nSize, WORD wDataType)
{
    switch(wDataType)
    {
        case DATATYPE_INIT:
            break;
        case DTYPE_GET_UKEYNO:  // 读UKEY编号
            return GetScanUKey((LPSTR)vData, nSize);
        default:
            break;
    }

    return CRD_SUCCESS;
}

// 获取版本号(1DevCRD版本/2固件版本/3设备软件版本/4其他)
void CDevCRD_ACTU6SG5::GetVersion(char* szVer, long lSize, ushort usType)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if (usType == 2)
    {
        m_devACTU6SG5.GetDeviceFW((LPSTR)szVer, lSize);
    }

    return;
}

// 信息扫描
INT CDevCRD_ACTU6SG5::GetScanUKey(LPSTR lpData, INT *nDataLen)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // 清除上一次扫描信息
    if ((nRet = m_devACTU6SG5.ScanMedia(MODE_SCAN_CLR_INFO, lpData, nDataLen)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "信息扫描: 清除上一次扫描信息: ->ScanMedia(%d) Fail, ErrCode=%d, Return %s.",
            MODE_SCAN_CLR_INFO, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    // 启动扫描
    if ((nRet = m_devACTU6SG5.ScanMedia(MODE_SCAN_START, lpData, nDataLen)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "信息扫描: 启动扫描: ->ScanMedia(%d) Fail, ErrCode=%d, Return %s.",
            MODE_SCAN_START, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    // 获取扫描信息
    /*if ((nRet = m_devACTU6SG5.ScanMedia(MODE_SCAN_GET_INFO, lpData, nDataLen)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "信息扫描: 获取扫描信息: ->ScanMedia(%d) Fail, ErrCode=%d, Return %s.",
            MODE_SCAN_GET_INFO, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }*/

    return CRD_SUCCESS;
}

// Impl错误码转换为DevCRD错误码
INT CDevCRD_ACTU6SG5::ConvertErrorCode(INT nRet)
{
    switch(nRet)
    {
        case IMP_ERR_LOAD_LIB           :// 动态库加载失败
            return ERR_CRD_LOAD_LIB;
        case IMP_ERR_PARAM_INVALID      :// 参数无效
            return ERR_CRD_PARAM_ERR;
        case IMP_ERR_READERROR           :// 读数据错误
            return ERR_CRD_READERROR;
        case IMP_ERR_WRITEERROR          :// 写数据错误
            return ERR_CRD_WRITEERROR;
        case IMP_ERR_UNKNOWN             :// 未知错误
            return ERR_CRD_OTHER;
        case IMP_ERR_SNDDATA_INVALID     :// 无效的下发数据
            return ERR_CRD_DATA_INVALID;
        case IMP_ERR_RCVDATA_INVALID     :// 无效的应答数据
            return ERR_CRD_DATA_INVALID;
        case IMP_ERR_DEVPORT_NOTOPEN     :// (-1) 没打开
            return ERR_CRD_NOT_OPEN;
        case IMP_ERR_DEVPORT_FAIL        :// (-2) 通讯错误
            return ERR_CRD_COMM_ERR;
        case IMP_ERR_DEVPORT_PARAM       :// (-3) 参数错误
            return ERR_CRD_PARAM_ERR;
        case IMP_ERR_DEVPORT_CANCELED    :// (-4) 操作取消
            return ERR_CRD_USER_CANCEL;
        case IMP_ERR_DEVPORT_READERR     :// (-5) 读取错误
            return ERR_CRD_READERROR;
        case IMP_ERR_DEVPORT_WRITE       :// (-6) 发送错误
            return ERR_CRD_WRITEERROR;
        case IMP_ERR_DEVPORT_RTIMEOUT    :// (-7) 操作超时
            return ERR_CRD_READTIMEOUT;
        case IMP_ERR_DEVPORT_WTIMEOUT    :// (-8) 操作超时
            return ERR_CRD_WRITETIMEOUT;
        case IMP_ERR_DEVPORT_LIBRARY        :// (-98) 加载通讯库失败
            return ERR_CRD_LOAD_LIB;
        case IMP_ERR_DEVPORT_NODEFINED   :// (-99) 未知错误
            return ERR_CRD_OTHER;
        case IMP_ERR_DEVRET_CMD_INVLID      :// 未定义的命令
            return ERR_CRD_UNSUP_CMD;
        case IMP_ERR_DEVRET_CMD_PARAM       :// 命令参数错误
            return ERR_CRD_PARAM_ERR;
        case IMP_ERR_DEVRET_CMD_UKEY_NOHAVE :// 卡箱无卡
            return ERR_CRD_UNIT_EMPTY;
        case IMP_ERR_DEVRET_CMD_PASS_BLOCK  :// 通道堵塞
            return ERR_CRD_JAMMED;
        case IMP_ERR_DEVRET_CMD_SENSOR_FAIL :// 传感器坏
            return ERR_CRD_HWERR;
        case IMP_ERR_DEVRET_CMD_ELEV_NOBOT  :// 升降梯不在底部
            return ERR_CRD_HWERR;
        case IMP_ERR_DEVRET_CMD_CMD_NORUN   :// 命令不能执行
            return ERR_CRD_HWERR;
        case IMP_ERR_DEVRET_CMD_SCAN_FAIL   :// 获取扫描信息失败
            return ERR_CRD_SCAN;
        case IMP_ERR_DEVRET_CMD_RETAIN_FULL :// 回收箱满
            return ERR_CRD_UNIT_FULL;
        case IMP_ERR_DEVRET_CMD_REPASS_BLOCK:// 回收通道堵塞
            return ERR_CRD_JAMMED;
        case IMP_ERR_DEVRET_CMD_VAL_OVER    :// 设置值超出范围
            return ERR_CRD_PARAM_ERR;
        case IMP_ERR_DEVRET_CMD_REBOX_NOHAVE:// 无回收箱
            return ERR_CRD_UNIT_NOTFOUND;
        case IMP_ERR_DEVRET_CMD_ELEV_NOSCAN :// 升降梯不在扫描位
            return ERR_CRD_HWERR;
        case IMP_ERR_DEVRET_CMD_POWER_FAIL  :// 电压异常,过低或过高
            return ERR_CRD_HWERR;
        case IMP_ERR_DEVRET_CMD_ELEC_FAIL   :// 升降电机模块故障
            return ERR_CRD_HWERR;
        case IMP_ERR_DEVRET_CMD_SELF_FAIL   :// 自适应出错
            return ERR_CRD_HWERR;
        case IMP_ERR_DEVRET_CMD_ELEVM_FAIL  :// 电梯模块故障
            return ERR_CRD_HWERR;
        case IMP_ERR_DEVRET_CMD_CAM_FAIL    :// 凸轮模块故障
            return ERR_CRD_HWERR;
        case IMP_ERR_DEVRET_CMD_BOX_NOHAVE  :// 没有卡箱或卡箱没有到位
            return ERR_CRD_UNIT_NOTFOUND;
        default:
            return ERR_CRD_OTHER;
    }
}

// DevCRD错误码含义
CHAR* CDevCRD_ACTU6SG5::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(nRet)
    {
        case CRD_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "操作成功");
            return m_szErrStr;
        case ERR_CRD_INSERT_TIMEOUT :
            sprintf(m_szErrStr, "%d|%s", nRet, "进卡超时");
            return m_szErrStr;
        case ERR_CRD_USER_CANCEL    :
            sprintf(m_szErrStr, "%d|%s", nRet, "用户取消");
            return m_szErrStr;
        case ERR_CRD_COMM_ERR       :
            sprintf(m_szErrStr, "%d|%s", nRet, "通讯错误");
            return m_szErrStr;
        case ERR_CRD_JAMMED         :
            sprintf(m_szErrStr, "%d|%s", nRet, "堵卡");
            return m_szErrStr;
        case ERR_CRD_OFFLINE        :
            sprintf(m_szErrStr, "%d|%s", nRet, "脱机");
            return m_szErrStr;
        case ERR_CRD_NOT_OPEN       :
            sprintf(m_szErrStr, "%d|%s", nRet, "没有打开");
            return m_szErrStr;
        case ERR_CRD_UNIT_FULL      :
            sprintf(m_szErrStr, "%d|%s", nRet, "箱满");
            return m_szErrStr;
        case ERR_CRD_UNIT_EMPTY     :
            sprintf(m_szErrStr, "%d|%s", nRet, "箱空");
            return m_szErrStr;
        case ERR_CRD_UNIT_NOTFOUND  :
            sprintf(m_szErrStr, "%d|%s", nRet, "箱不存在");
            return m_szErrStr;
        case ERR_CRD_HWERR          :
            sprintf(m_szErrStr, "%d|%s", nRet, "硬件故障");
            return m_szErrStr;
        case ERR_CRD_STATUS_ERR     :
            sprintf(m_szErrStr, "%d|%s", nRet, "状态出错");
            return m_szErrStr;
        case ERR_CRD_UNSUP_CMD      :
            sprintf(m_szErrStr, "%d|%s", nRet, "不支持的指令");
            return m_szErrStr;
        case ERR_CRD_PARAM_ERR      :
            sprintf(m_szErrStr, "%d|%s", nRet, "参数错误");
            return m_szErrStr;
        case ERR_CRD_READTIMEOUT    :
            sprintf(m_szErrStr, "%d|%s", nRet, "读数据超时");
            return m_szErrStr;
        case ERR_CRD_WRITETIMEOUT   :
            sprintf(m_szErrStr, "%d|%s", nRet, "写数据超时");
            return m_szErrStr;
        case ERR_CRD_READERROR      :
            sprintf(m_szErrStr, "%d|%s", nRet, "读数据错");
            return m_szErrStr;
        case ERR_CRD_WRITEERROR     :
            sprintf(m_szErrStr, "%d|%s", nRet, "写数据错");
            return m_szErrStr;
        case ERR_CRD_LOAD_LIB       :
            sprintf(m_szErrStr, "%d|%s", nRet, "动态库错误");
            return m_szErrStr;
        case ERR_CRD_DATA_INVALID   :
            sprintf(m_szErrStr, "%d|%s", nRet, "无效数据");
            return m_szErrStr;
        case ERR_CRD_SCAN           :
            sprintf(m_szErrStr, "%d|%s", nRet, "扫描错");
            return m_szErrStr;
        case ERR_CRD_OTHER          :
            sprintf(m_szErrStr, "%d|%s", nRet, "其他错误/未知错误");
            return m_szErrStr;
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义错误");
            return m_szErrStr;
    }
}

//////////////////////////////////////////////////////////////////////////
