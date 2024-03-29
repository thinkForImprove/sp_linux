﻿/**************************************************************************
* 文件名称: DevBCR_NT0861.cpp
* 文件描述: 条码阅读模块功能处理接口封装
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年7月13日
* 文件版本: 1.0.0.1
**************************************************************************/

#include "DevBCR_NT0861.h"
#include <unistd.h>

static const char *ThisFile = "DevBCR_NT0861.cpp";

/**************************************************************************
*     对外接口调用处理                                                      *
**************************************************************************/
CDevBCR_NT0861::CDevBCR_NT0861(LPCSTR lpDevType) :
    m_pDevImpl(LOG_NAME_DEV, lpDevType)
{
    SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件

    m_wDeviceType = 2;                              // 设备类型
    m_bCancelReadBcr = FALSE;                       // 取消扫码标记初始化:F
    memset(m_nRetErrOLD, 0, sizeof(INT) * 8);
    m_nDevStatOLD = DEV_NOTFOUND;                   // 上一次记录设备状态
    m_bIsReadBcrRunning = FALSE;                    // 是否处于读码中

    m_clErrorDet.SetDevName(EC_DEV_DEF);
    m_clErrorDet.SetHWName(EC_HW_DEF);
}

CDevBCR_NT0861::~CDevBCR_NT0861()
{
    Close();
}

// 释放接口
int CDevBCR_NT0861::Release()
{
    return BCR_SUCCESS;
}

// 打开与设备的连接
int CDevBCR_NT0861::Open(const char *pMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    // AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;

    nRet = m_pDevImpl.OpenDevice(m_stOpenMode.szDevPath[0]);
    if (nRet != IMP_SUCCESS)
    {
        /*Log(ThisModule, __LINE__,
            "打开设备: ->OpenDevice(%s) Fail, ErrCode: %d, Return: %s",
            pMode, nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2BCR(nRet)));*/
        return ConvertImplErrCode2BCR(nRet);
    } else
    {
        // 设置扫码识读模式
        m_pDevImpl.SetSymScanMode(m_stOpenMode.nOtherParam[3]);

        // 扫码读码时是否设置条码类型限制(0:不设置,按硬件所有类型识别)
        if (m_stOpenMode.nOtherParam[4] == 0)
        {
            m_pDevImpl.SetSymDistAllow(EN_CODE_ALL);
        }
    }

    return BCR_SUCCESS;
}

// 关闭与设备的连接
int CDevBCR_NT0861::Close()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    //m_pDevImpl.CloseDevice();
    return BCR_SUCCESS;
}

// 取消
int CDevBCR_NT0861::Cancel(unsigned short usMode)
{
    THISMODULE(__FUNCTION__);

    if (usMode == 0)    // 取消扫码
    {
        Log(ThisModule, __LINE__, "设置取消扫码: usMode = %d.", usMode);
        m_bCancelReadBcr = TRUE;
    }

    return BCR_SUCCESS;
}

// 设备复位
int CDevBCR_NT0861::Reset(unsigned short usParam)
{
    //THISMODULE(__FUNCTION__);

    //INT nRet = IMP_SUCCESS;

    if (m_bIsReadBcrRunning == TRUE)    // 读码中
    {
        m_bCancelReadBcr = FALSE;
    }

    return BCR_SUCCESS;
}

// 取设备状态
int CDevBCR_NT0861::GetStatus(STDEVBCRSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);

    INT nRet = IMP_SUCCESS;
    STDEVBCRSTATUS stDevStat;

    INT nStat = DEV_NOTOPEN;

    CHAR szStatStr[][64] = { "设备正常", "设备未找到", "设备未打开", "设备已打开但断线" };

    nStat = m_pDevImpl.GetDeviceStat();

    // 设备已打开但断线时及时关闭设备连接,
    // 避免重新插线时 因连接占用导致系统分配新的接口给设备
    if (nStat == DEV_OFFLINE)
    {
        m_pDevImpl.CloseDevice();
    }

    // 记录设备状态变化
    if (m_nDevStatOLD != nStat)
    {
        Log(ThisModule, __LINE__, "设备状态有变化: %d[%s] -> %d[%s]",
            m_nDevStatOLD, szStatStr[m_nDevStatOLD], nStat, szStatStr[nStat]);
        m_nDevStatOLD = nStat;
    }

    switch(nStat)
    {
        case DEV_OK:
            stStatus.wDevice = DEVICE_STAT_ONLINE;
            stStatus.wPosition = DEVPOS_STAT_INPOS;
            stStatus.wBcrScanner = SCAN_STAT_ON;
            if (m_bIsReadBcrRunning == TRUE)    // 读码中
            {
                stStatus.wDevice = DEVICE_STAT_BUSY;
                stStatus.wBcrScanner = SCAN_STAT_ON;
            } else
            {
                stStatus.wBcrScanner = SCAN_STAT_OFF;
            }
            break;
        case DEV_NOTFOUND:
            //stStatus.wDevice = DEVICE_STAT_NODEVICE;
            //m_clErrorDet.SetDevErrCode((LPSTR)EC_ERR_DevNotFound);
            //break;
            //stStatus.wBcrScanner = SCAN_STAT_INOP;
        case DEV_OFFLINE:        
            //stStatus.wBcrScanner = SCAN_STAT_INOP;
        case DEV_NOTOPEN:
            stStatus.wDevice = DEVICE_STAT_OFFLINE;
            stStatus.wPosition = DEVPOS_STAT_UNKNOWN;
            stStatus.wBcrScanner = SCAN_STAT_UNKNOWN;
            m_clErrorDet.SetDevErrCode((LPSTR)EC_ERR_DevOffLine);
            break;
    }

    //m_clErrorDet.GetDevHWErrCode(stStatus.szErrCode);

    return BCR_SUCCESS;
}

// 扫码
int CDevBCR_NT0861::ReadBCR(STREADBCRIN stReadIn, STREADBCROUT &stReadOut)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    DWORD dwTimeOut = stReadIn.dwTimeOut;           // 超时时间
    QTime qtTimeCurr = QTime::currentTime();        // 执行前时间
    ULONG ulTimeCount = 0;                          // 超时计数    
    INT nCodeType = 0;                              // 条码类型
    CHAR szSymData[4096] = { 0x00 };
    DWORD dwSymDataSize = sizeof(szSymData);

    m_bIsReadBcrRunning = FALSE;                    // 是否处于读码中

    // 扫码读码时是否设置条码类型限制(1:实时设置)
    if (m_stOpenMode.nOtherParam[4] == 1)
    {
        // 扫码前设置扫码类型禁止允许
        if (stReadIn.wSymType[0] > 0)   // 需要明确指定条码类型扫码
        {

        } else  // 支持所有条码类型扫码
        {
            m_pDevImpl.SetSymDistAllow(EN_CODE_ALL);
        }
    }

    // 开始扫码
    nRet = m_pDevImpl.ScanCodeStart();
    if(nRet != BCR_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "开始扫码: ->ScanCodeStart() Fail, ErrCode: %d, Return: %s",
            nRet, ConvertDevErrCodeToStr(ConvertImplErrCode2BCR(nRet)));
        m_pDevImpl.ScanCodeEnd();
        return ConvertImplErrCode2BCR(nRet);
    }

    m_bIsReadBcrRunning = TRUE;  // 处于读码中

    // 循环检测是否扫描成功
    while(1)
    {
        QCoreApplication::processEvents();

        // 设备断线
        if (m_pDevImpl.GetDeviceStat() != DEV_OK)
        {
            Log(ThisModule, __LINE__,
                "扫码中: 设备断线/异常, Return: %s", ConvertDevErrCodeToStr(ERR_BCR_DEV_HWERR));
            m_pDevImpl.ScanCodeEnd();
            m_bIsReadBcrRunning = FALSE;
            return ERR_BCR_DEV_HWERR;
        }

        // 扫码取消
        if (m_bCancelReadBcr == TRUE)
        {
            m_bCancelReadBcr = FALSE;
            Log(ThisModule, __LINE__,
                "扫码中: 命令取消, Return: %s", ConvertDevErrCodeToStr(ERR_BCR_USER_CANCEL));
            m_pDevImpl.ScanCodeEnd();
            m_bIsReadBcrRunning = FALSE;
            return ERR_BCR_USER_CANCEL;
        }

        // 检查超时
        if (dwTimeOut > 0)  // 指定超时时间
        {
            ulTimeCount = qtTimeCurr.msecsTo(QTime::currentTime()); // 时间差计入超时计数(毫秒)
            if (ulTimeCount >= dwTimeOut)   // 超过超时时间
            {
                Log(ThisModule, __LINE__,
                    "扫描中: 已等待时间[%d] > 指定超时时间[%d], TimeOut, Return: %s",
                    ulTimeCount, dwTimeOut, ConvertDevErrCodeToStr(ERR_BCR_TIMEOUT));
                m_pDevImpl.ScanCodeEnd();
                m_bIsReadBcrRunning = FALSE;
                return ERR_BCR_TIMEOUT;
            }
        }

        // 获取扫码信息(等待时间200)
        nRet = m_pDevImpl.GetScanCode(szSymData, dwSymDataSize, nCodeType, 200);
        if (nRet != BCR_SUCCESS)
        {
            if (m_nRetErrOLD[3] != nRet)
            {
                Log(ThisModule, __LINE__,
                    "扫描中: 获取扫码数据: ->GetScanCode() Fail, ErrCode: %d, 不返回, 进入下一次循环处理...",
                    nRet);
                m_nRetErrOLD[3] = nRet;
            }
        } else
        {
            stReadOut.wSymType = ConvertSymImpl2Dev(nCodeType);
            memcpy(stReadOut.szSymData, szSymData, dwSymDataSize);
            stReadOut.dwSymDataSize = dwSymDataSize;
            stReadOut.wSymDataMode = SYMD_ASCII;
            /*if (stReadIn.wSymDataMode == SYMD_HEX)  // 返回为ASCII,转换为HEX
            {
                stReadOut.dwSymDataSize = DataConvertor::Ascii2HexAscii2Hex(szSymData,dwSymDataSize,
                                                                            stReadOut.szSymData, sizeof(stReadOut.szSymData));
            } else
            {
                memcpy(stReadOut.szSymData, szSymData, dwSymDataSize);
                stReadOut.dwSymDataSize = dwSymDataSize;
            }*/
            break;
        }

        continue;
    }


    m_pDevImpl.ScanCodeEnd();
    m_bIsReadBcrRunning = FALSE;

    return BCR_SUCCESS;
}
// 设置数据
int CDevBCR_NT0861::SetData(unsigned short usType, void *vData)
{
    THISMODULE(__FUNCTION__);

    switch(usType)
    {
        case SET_LIB_PATH:          // 设置动态库路径
        {
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
                m_wDeviceType = (m_stOpenMode.nOtherParam[0] <= 0 ? m_stOpenMode.nOtherParam[0] : m_wDeviceType);
                // 设置命令收发超时
                if (m_stOpenMode.nOtherParam[2] > 0 || m_stOpenMode.nOtherParam[3] > 0)
                {
                    m_pDevImpl.SetSndRcvTimeOut(m_stOpenMode.nOtherParam[2],
                                                m_stOpenMode.nOtherParam[3]);
                }
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
        default:
            break;
    }

    return BCR_SUCCESS;
}

// 获取数据
int CDevBCR_NT0861::GetData(unsigned short usType, void *vData)
{
    THISMODULE(__FUNCTION__);

    switch(usType)
    {
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

    return BCR_SUCCESS;
}

// 获取版本
int CDevBCR_NT0861::GetVersion(unsigned short usType, char* szVer, int nSize)
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

    return BCR_SUCCESS;
}

// 指示灯控制
INT CDevBCR_NT0861::ControlLight(STGLIGHTSCONT stLignt)
{
    return BCR_SUCCESS;
}

// 鸣响控制
INT CDevBCR_NT0861::ControlBeep(STBEEPCONT stBeep)
{
    return BCR_SUCCESS;
}

// Impl错误码转换为BCR错误码
INT CDevBCR_NT0861::ConvertImplErrCode2BCR(INT nRet)
{
#define CASE_RET_DEVCODE(IMP, RET) \
        case IMP: return RET;

    ConvertImplErrCode2ErrDetail(nRet);

    switch(nRet)
    {
        // > 100: Impl处理返回
        CASE_RET_DEVCODE(IMP_SUCCESS, BCR_SUCCESS)                          // 成功
        CASE_RET_DEVCODE(IMP_ERR_LOAD_LIB, ERR_BCR_LIBRARY)                 // 动态库加载失败
        CASE_RET_DEVCODE(IMP_ERR_PARAM_INVALID, ERR_BCR_PARAM_ERR)          // 参数无效
        CASE_RET_DEVCODE(IMP_ERR_READERROR, ERR_BCR_READ_ERR)               // 读数据错误
        CASE_RET_DEVCODE(IMP_ERR_WRITEERROR, ERR_BCR_WRITE_ERR)             // 写数据错误
        CASE_RET_DEVCODE(IMP_ERR_RCVDATA_INVALID, ERR_BCR_RESP_ERR)         // 无效的应答数据
        CASE_RET_DEVCODE(IMP_ERR_RCVDATA_NOTCOMP, ERR_BCR_RESP_NOT_COMP)    // 无效的应答数据
        CASE_RET_DEVCODE(IMP_ERR_SNDCMD_INVALID, ERR_BCR_COMM_RUN)          // 下发命令无效
        CASE_RET_DEVCODE(IMP_ERR_UNKNOWN, ERR_BCR_OTHER)                    // 未知错误
        // <0 : USB/COM接口处理返回
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_NOTOPEN, ERR_BCR_DEV_NOTOPEN)      // (-1) 没打开
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_FAIL, ERR_BCR_COMM_ERR)            // (-2) 通讯错误
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_PARAM, ERR_BCR_PARAM_ERR)          // (-3) 参数错误
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_CANCELED, ERR_BCR_USER_CANCEL)     // (-4) 操作取消
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_READERR, ERR_BCR_READ_ERR)         // (-5) 读取错误
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_WRITE, ERR_BCR_WRITE_ERR)          // (-6) 发送错误
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_RTIMEOUT, ERR_BCR_READ_TIMEOUT)    // (-7) 操作超时
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_WTIMEOUT, ERR_BCR_WRITE_TIMEOUT)   // (-8) 操作超时
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_LIBRARY, ERR_BCR_LIBRARY)          // (-98) 加载通讯库失败
        CASE_RET_DEVCODE(IMP_ERR_DEVPORT_NODEFINED, ERR_BCR_OTHER)          // (-99) 未知错误
        default: return ERR_BCR_OTHER;
    }

    return BCR_SUCCESS;
}

// 根据Impl错误码设置错误错误码字符串
INT CDevBCR_NT0861::ConvertImplErrCode2ErrDetail(INT nRet)
{
#define CASE_SET_DEV_DETAIL(IMP, STR) \
        case IMP: m_clErrorDet.SetDevErrCode((LPSTR)STR); break;

#define CASE_SET_HW_DETAIL(IMP, DEVID) \
        case IMP: \
            m_clErrorDet.SetHWErrCodeInt(IMP, DEVID); break;

    switch(nRet)
    {
        // > 100: Impl处理返回
        //CASE_RET_DEVCODE(IMP_SUCCESS, BCR_SUCCESS)                        // 成功
        CASE_SET_DEV_DETAIL(IMP_ERR_LOAD_LIB, EC_ERR_LibraryLoadFail)       // 动态库加载失败
        CASE_SET_DEV_DETAIL(IMP_ERR_PARAM_INVALID, EC_ERR_ParInvalid)       // 参数无效
        CASE_SET_DEV_DETAIL(IMP_ERR_READERROR, EC_ERR_DataRWErr)            // 读数据错误
        CASE_SET_DEV_DETAIL(IMP_ERR_WRITEERROR, EC_ERR_DataRWErr)           // 写数据错误
        CASE_SET_DEV_DETAIL(IMP_ERR_RCVDATA_INVALID, EC_ERR_RecvData_inv)   // 无效的应答数据
        CASE_SET_DEV_DETAIL(IMP_ERR_RCVDATA_NOTCOMP, EC_ERR_ReadData_NotComp)// 无效的应答数据
        CASE_SET_DEV_DETAIL(IMP_ERR_SNDCMD_INVALID, EC_ERR_HWParInvalid)    // 下发命令无效
        CASE_SET_DEV_DETAIL(IMP_ERR_UNKNOWN, EC_ERR_UnKnownErr)             // 未知错误
        // <0 : USB/COM接口处理返回
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_NOTOPEN, EC_ERR_DevOpenFail)    // (-1) 没打开
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_FAIL, EC_ERR_ConnFail)          // (-2) 通讯错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_PARAM, EC_ERR_ParInvalid)       // (-3) 参数错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_READERR, EC_ERR_DataRWErr)      // (-5) 读取错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_WRITE, EC_ERR_DataRWErr)        // (-6) 发送错误
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_RTIMEOUT, EC_ERR_CommTimeOut)   // (-7) 操作超时
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_WTIMEOUT, EC_ERR_CommTimeOut)   // (-8) 操作超时
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_LIBRARY, EC_ERR_LibraryLoadFail)// (-98) 加载通讯库失败
        CASE_SET_DEV_DETAIL(IMP_ERR_DEVPORT_NODEFINED, EC_ERR_UnKnownErr)   // (-99) 未知错误
    }

    return BCR_SUCCESS;
}

// 条码类型Impl转换为Dev格式
WORD CDevBCR_NT0861::ConvertSymImpl2Dev(INT nSym)
{
    switch(nSym)
    {
        case EN_CODE_Codabar:       return EN_SYM_CODABAR;      // Codabar
        case EN_CODE_Code128:       return EN_SYM_128;          // Code 128
        case EN_CODE_Code39:        return EN_SYM_39;           // Code 39
        case EN_CODE_Code32:        return EN_SYM_39;           // Code 32
        case EN_CODE_Code93:        return EN_SYM_93;           // Code 93
        case EN_CODE_DataMatrix:    return EN_SYM_DATAMATRIX;   // Data Matrix
        case EN_CODE_InterL2OF5:    return EN_SYM_ITF;          // InterLeaved 2 of 5
        case EN_CODE_PDF417:        return EN_SYM_PDF_417;      // PDF417
        case EN_CODE_QR:            return EN_SYM_QRCODE;       // QR
        case EN_CODE_UPCA:          return EN_SYM_UPCA;         // UPC-A
        case EN_CODE_UPCE:          return EN_SYM_UPCE0;        // UPC-E
        case EN_CODE_UPCE1:         return EN_SYM_UPCE1;        // UPC-E1
        case EN_CODE_UPCE8:         return EN_SYM_UPCE1_2;      // UPC-E8
        case EN_CODE_UPCE13:        return EN_SYM_UPCE1_5;      // UPC-E13
        case EN_CODE_Matrix2OF5:    return EN_SYM_DATAMATRIX;   // Matrix 2 of 5
        case EN_CODE_Indust2OF5:    return EN_SYM_STD2OF5;      // Industrial 2 of 5
        default:                    return EN_SYM_UNKNOWN;
    }
}

// 条码类型Dev转换为Impl格式
WORD CDevBCR_NT0861::ConvertSymDev2Impl(INT nSym)
{
    switch(nSym)
    {
        case EN_SYM_CODABAR:        return EN_CODE_Codabar;     // Codabar
        case EN_SYM_128:            return EN_CODE_Code128;     // Code 128
        case EN_SYM_39:             return EN_CODE_Code39;      // Code 39
        case EN_SYM_93:             return EN_CODE_Code93;      // Code 93
        case EN_SYM_DATAMATRIX:     return EN_CODE_DataMatrix;  // Data Matrix
        case EN_SYM_ITF:            return EN_CODE_InterL2OF5;  // InterLeaved 2 of 5
        case EN_SYM_PDF_417:        return EN_CODE_PDF417;      // PDF417
        case EN_SYM_QRCODE:         return EN_CODE_QR;          // QR
        case EN_SYM_UPCA:           return EN_CODE_UPCA;        // UPC-A
        case EN_SYM_UPCE0:          return EN_CODE_UPCE;        // UPC-E
        case EN_SYM_UPCE1:          return EN_CODE_UPCE1;       // UPC-E1
        case EN_SYM_UPCE1_2:        return EN_CODE_UPCE8;       // UPC-E8
        case EN_SYM_UPCE1_5:        return EN_CODE_UPCE13;      // UPC-E13
        //case EN_SYM_DATAMATRIX:     return EN_CODE_Matrix2OF5;  // Matrix 2 of 5
        case EN_SYM_STD2OF5:        return EN_CODE_Indust2OF5;  // Industrial 2 of 5
        default:                    return EN_CODE_UNKNOWN;
    }
}

// -------------------------------- END -----------------------------------
