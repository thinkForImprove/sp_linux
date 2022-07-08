/***************************************************************
* 文件名称: DevIDX_DMTM1M.cpp
* 文件描述: 身份证模块功能处理接口封装
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2023年3月25日
* 文件版本: 1.0.0.1
****************************************************************/

#include "DevIDX_DMTM1M.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>
#include <QSettings>
#include <sys/stat.h>

#include <dirent.h>

static const char *ThisFile = "DevIDX_DMTM1M.cpp";

/****************************************************************************
*     对外接口调用处理                                                         *
****************************************************************************/
CDevIDX_DMTM1M::CDevIDX_DMTM1M() : m_pDevImpl(LOG_NAME_DEV)
{
    SetLogFile(LOG_NAME_DEV, ThisFile);                     // 设置日志文件名和错误发生的文件
    m_bReCon = FALSE;                                       // 是否断线重连状态: 初始F

    m_stStatusOLD.Clear();
    memset(m_szHeadImgSavePath, 0x00, sizeof(m_szHeadImgSavePath));     // 证件头像存放位置
    memset(m_szScanImgSavePath, 0x00, sizeof(m_szScanImgSavePath));     // 扫描图像存放位置
    memset(m_szHeadImgName, 0x00, sizeof(m_szHeadImgName));             // 头像名(空不使用)
    memset(m_szScanImgFrontName, 0x00, sizeof(m_szScanImgFrontName));   // 证件正面图像名(空不使用)
    memset(m_szScanImgBackName, 0x00, sizeof(m_szScanImgBackName));     // 证件背面图像名(空不使用)
    memset(m_szScanImgFrontBuff, 0x00, sizeof(m_szScanImgFrontBuff));   // 证件扫描正面buffer
    memset(m_szScanImgBackBuff, 0x00, sizeof(m_szScanImgBackBuff));     // 证件扫描背面buffer
    m_nScanImgFrontBuffLen = 0;                                         // 证件扫描正面buffer Len
    m_nScanImgBackBuffLen = 0;                                          // 证件扫描背面buffer Len
    m_wScanImgSaveType = SAVE_IMG_JPG;                                  // 证件扫描图像保存类型(BMP/JPG,缺省JPG)
    memset(m_szScanImgSaveTypeS, 0x00, sizeof(m_szScanImgSaveTypeS));   // 证件扫描图像保存类型(BMP/JPG,缺省JPG)
    memcpy(m_szScanImgSaveTypeS, SAVE_IMG_JPG_S, strlen(SAVE_IMG_JPG_S));
    m_fScanImgSaveZoomSc = 2.0;                                         // 证件扫描图像保存图片缩放比例(0.1-3.0,缺省2.0)
    m_wScanImageFrontIsInfor = 1;                                       // 证件扫描图像是否以人像信息面为正面
    m_bCancelReadCard = FALSE;                                          // 取消读卡标记,初始化:F
    m_wBankNo = 0;
}

CDevIDX_DMTM1M::~CDevIDX_DMTM1M()
{
    m_pDevImpl.CloseDevice();
}

// 释放接口
int CDevIDX_DMTM1M::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    m_pDevImpl.CloseDevice();
    return IDC_SUCCESS;
}

// 打开与设备的连接
int CDevIDX_DMTM1M::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    //m_pDevImpl.CloseDevice();

    INT nRet = IMP_SUCCESS;

    // 建立连接
    //if (m_pDevImpl.IsDeviceOpen() != TRUE)
    {
        if ((nRet = m_pDevImpl.OpenDevice()) != IMP_SUCCESS)
        {
            /*Log(ThisModule, __LINE__,
                "打开设备: ->OpenDevice(%s) Fail, ErrCode: %d, Return: %s",
                pMode, nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));*/
            return ConvertCode_Impl2IDC(nRet);
        }
    }

    return IDC_SUCCESS;
}

// 关闭与设备的连接
int CDevIDX_DMTM1M::Close()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_pDevImpl.CloseDevice();
    return IDC_SUCCESS;
}

// 取消
int CDevIDX_DMTM1M::Cancel(unsigned short usMode)
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
int CDevIDX_DMTM1M::Reset(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    INT nAct = 0;
    CHAR szErrBuf[1024] = { 0x00 };

    sprintf(szErrBuf, "设备复位: ");

    // Reset卡动作转换为设备支持
    if (enMediaAct == MEDIA_NOTACTION)  // 无动作
    {
        nAct = CARD_NOACTION;
        sprintf(szErrBuf + strlen(szErrBuf), "设备复位: 参数(无动作): ");
    } else
    if (enMediaAct == MEDIA_EJECT)      // 退卡
    {
        if (usParam == 0)
        {
            nAct = CARD_EJECTMENT;      // 退卡并持卡(退到门口)
            sprintf(szErrBuf + strlen(szErrBuf), "设备复位: 参数(退卡并持卡): ");
        } else
        {
            nAct = CARD_EJECT;          // 完全弹出
            sprintf(szErrBuf + strlen(szErrBuf), "设备复位: 参数(完全弹出): ");
        }
    } else
    if (enMediaAct == MEDIA_RETRACT)    // 吞卡
    {
        nAct = CARD_RETRACT;
        sprintf(szErrBuf + strlen(szErrBuf), "设备复位: 参数(吞卡): ");
    } else
    {
        nAct = CARD_NOACTION;
        sprintf(szErrBuf + strlen(szErrBuf), "设备复位: 参数(无动作): ");
    }

    // 调用硬件处理
    nRet = m_pDevImpl.nResetDevice(nAct);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "%s ->nSoftResetDevice(%d) Fail, ErrCode: %d, Return: %s",
            szErrBuf, enMediaAct, nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
        return ConvertCode_Impl2IDC(nRet);
    }

    return IDC_SUCCESS;
}

// 取设备状态
int CDevIDX_DMTM1M::GetStatus(STDEVIDCSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    char cDeviceStatus[][24] = {  "DEV_ONLINE",
                                "DEV_OFFLINE",
                                "DEV_POWEROFF",
                                "DEV_NODEVICE",
                                "DEV_HWERROR",
                                "DEV_USERERROR",
                                "DEV_BUSY"
    };

    char cMediaStatus[][24] = {  "MEDIA_PRESENT",
                               "MEDIA_NOTPRESENT",
                               "MEDIA_JAMMED",
                               "MEDIA_NOTSUPP",
                               "MEDIA_UNKNOWN",
                               "MEDIA_ENTERING",
    };

    INT nRet = IMP_SUCCESS;

    stStatus.Clear();

    // 设备未Open
    if (m_pDevImpl.IsDeviceOpen() != TRUE)
    {
        stStatus.wDevice = DEVICE_STAT_OFFLINE;
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;
        return ERR_IDC_DEV_NOTOPEN;
    }

    // 获取设备状态
    nRet = m_pDevImpl.nGetDevStat();
    switch(nRet)
    {
        case IMP_ERR_DEV_0000H:     // 成功
        {
            stStatus.wDevice = DEVICE_STAT_ONLINE;
            break;
        }
        case IMP_ERR_DEV_8101H:     // 设备未打开
        {
            stStatus.wDevice = DEVICE_STAT_OFFLINE;
            break;
        }
        default:                    // 故障
        {
            stStatus.wDevice = DEVICE_STAT_HWERROR;
            break;
        }
    }

    // 取卡状态
    nRet = m_pDevImpl.nGetCardStat();
    switch(nRet)
    {
        case IMP_ERR_DEV_0000H:     // 成功
        {
            stStatus.wMedia = MEDIA_STAT_PRESENT;
            break;
        }
        case IMP_ERR_DEV_8D01H:     // 无卡
        {
            stStatus.wDevice = DEVICE_STAT_ONLINE;
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
            break;
        }
        default:                    // 故障
        {
            stStatus.wMedia = MEDIA_STAT_UNKNOWN;
            break;
        }
    }

    if (stStatus.Diff(m_stStatusOLD) != 0)
    {
        Log(ThisModule, __LINE__,
            "转换后设备状态: GetStatus()　Result: %d|%s -> %d|%s, %d|%s -> %d|%s",
            m_stStatusOLD.wDevice, cDeviceStatus[m_stStatusOLD.wDevice],
            stStatus.wDevice, cDeviceStatus[stStatus.wDevice],
            m_stStatusOLD.wMedia, cMediaStatus[m_stStatusOLD.wMedia],
            stStatus.wMedia, cMediaStatus[stStatus.wMedia]);
        m_stStatusOLD.Copy(stStatus);
    }

    // 强制退证(卡)按钮按下

    return IDC_SUCCESS;
}

// 介质控制
int CDevIDX_DMTM1M::MediaControl(MEDIA_ACTION enMediaAct, unsigned long ulParam)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    if(enMediaAct == MEDIA_EJECT)       // 介质退出
    {
        return nEjectCard((DWORD)ulParam);
    } else
    if(enMediaAct == MEDIA_RETRACT)     // 介质回收
    {
        Log(ThisModule, __LINE__,"介质控制: 回收: 硬件不支持.");
        return ERR_IDC_UNSUP_CMD;
    } else
    if (enMediaAct == MEDIA_ACCEPT_IC)  // IC卡进卡: 作为身份证进卡
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

// 介质读写(读身份证+扫描)
int CDevIDX_DMTM1M::MediaReadWrite(MEDIA_RW_MODE enRWMode, STMEDIARW &stMediaData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    char cImgHeadFile[256 * 2] = { 0x00 };  // 头像图像路径名
    char szIDCardNo[256] = { 0x00 };

    if (enRWMode == MEDIA_READ) // 读
    {
        if ((stMediaData.dwRWType & RW_TRACK1)  == RW_TRACK1)   // 读卡
        {
            if (strlen(m_szHeadImgName) > 0)   // 指定证件头像图像名
            {
                sprintf(cImgHeadFile, "%s/%s", m_szHeadImgSavePath, m_szHeadImgName);
            } else
            {
                sprintf(cImgHeadFile, "%s", m_szHeadImgSavePath);
            }


            // 背面图像路径check
            if (FileAccess::create_directory_by_path(cImgHeadFile, true) != true)
            {
                Log(ThisModule, __LINE__, "读卡: 头像保存路径[%s]建立失败, Return: %d.",
                   cImgHeadFile, WFS_ERR_UNSUPP_DATA);
                return WFS_ERR_UNSUPP_DATA;
            }

            // 获取卡类型
            INT nCardType = 0;
            if (m_debugMode)
            {
                nCardType = 1;
            } else
            {
                nRet = m_pDevImpl.nGetIDCardType(nCardType);
                if (nRet != IMP_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "读卡: 获取卡类型: ->nGetIDCardType() Fail, ErrCode: %d, Return: %s.",
                        nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                    return ConvertCode_Impl2IDC(nRet);
                }
            }
            Log(ThisModule, __LINE__, "读卡: ->bGetIDCardType() 获取卡类型 succ. Is %s. debugMode: %d",
                nCardType == 1 ? "国内证件" :
                 (nCardType == 2 ? "国外证件" :
                  (nCardType == 3 ? "港澳台证件" : "未知证件类型")), (INT)m_debugMode);

            // 读卡数据
            if (nCardType == 1)  // 国内证件
            {
                STIDCARDINFO stInfo;
                nRet = m_pDevImpl.nGetIDCardInfo(nCardType, &stInfo, (LPSTR)cImgHeadFile, strlen(m_szHeadImgName) > 0, m_debugMode);
                if (nRet != IMP_SUCCESS)
                {
                    stMediaData.stData[0].wResult = RW_RESULT_MISS;
                    Log(ThisModule, __LINE__,
                        "读卡: 国内证件读: ->nGetIDCardInfo() Fail, ErrCode: %d, Return: %s.",
                        nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                    return ConvertCode_Impl2IDC(nRet);
                }

                // 组织证件数据写入应答
                stMediaData.stData[0].wResult =
                        IDCardDataToTrack(&stInfo, stMediaData.stData[0].szData, stMediaData.stData[0].wSize, nCardType);
                stMediaData.stData[0].wSize = strlen(stMediaData.stData[0].szData);

                // 扫描图像路径(INI指定/证件号码_front/back.)
                sprintf(szIDCardNo, "%s", QString(stInfo.szNumber).trimmed().toStdString().c_str());
            } else
            if (nCardType == 2)  // 国外证件
            {
                STIDFOREIGNINFO stInfo;
                nRet = m_pDevImpl.nGetIDCardInfo(nCardType, &stInfo, (LPSTR)cImgHeadFile);
                if (nRet != IMP_SUCCESS)
                {
                    stMediaData.stData[0].wResult = RW_RESULT_MISS;
                    Log(ThisModule, __LINE__,
                        "读卡: 国外证件读: ->nGetIDCardInfo() Fail, ErrCode: %d, Return: %s.",
                        nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                    return ConvertCode_Impl2IDC(nRet);
                }

                // 组织证件数据写入应答
                stMediaData.stData[0].wResult =
                        IDCardDataToTrack(&stInfo, stMediaData.stData[0].szData, stMediaData.stData[0].wSize, nCardType);
                stMediaData.stData[0].wSize = strlen(stMediaData.stData[0].szData);

                // 扫描图像路径(INI指定/证件号码_front/back.)
                sprintf(szIDCardNo, "%s", QString(stInfo.szIDCardNO).trimmed().toStdString().c_str());
            } else
            if (nCardType == 3)  // 港澳台证件
            {
                STIDGATINFO stInfo;
                nRet = m_pDevImpl.nGetIDCardInfo(nCardType, &stInfo, (LPSTR)cImgHeadFile);
                if (nRet != IMP_SUCCESS)
                {
                    stMediaData.stData[0].wResult = RW_RESULT_MISS;
                    Log(ThisModule, __LINE__,
                        "读卡: 港澳台证件读: ->nGetIDCardInfo() Fail, ErrCode: %d, Return: %s.",
                        nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                    return ConvertCode_Impl2IDC(nRet);
                }

                // 组织证件数据写入应答
                stMediaData.stData[0].wResult =
                        IDCardDataToTrack(&stInfo, stMediaData.stData[0].szData, stMediaData.stData[0].wSize, nCardType);
                stMediaData.stData[0].wSize = strlen(stMediaData.stData[0].szData);

                // 扫描图像路径(INI指定/证件号码_front/back.jpg)
                sprintf(szIDCardNo, "%s", QString(stInfo.szNumber).trimmed().toStdString().c_str());
            } else
            {
                Log(ThisModule, __LINE__, "读卡: ->bGetIDCardType() 无效卡类型<%d>, Return: %s.",
                    nCardType, ConvertDevErrCodeToStr(ERR_IDC_MED_INV));
                return ERR_IDC_MED_INV;
            }
        }                                                                                               // 40-00-00-00(FT#0012)

        if ((stMediaData.dwRWType & RW_TRACK2)  == RW_TRACK2 ||
            (stMediaData.dwRWType & RW_TRACK3) == RW_TRACK3)     // 扫描图像
        {                                                                                                       // 40-00-00-00(FT#0012)
            Log(ThisModule, __LINE__, "读卡: 证件扫描: 硬件不支持, 不处理.");                                   // 40-00-00-00(FT#0012)
        }
    } else
    {
        Log(ThisModule, __LINE__, "介质读写: 不支持的入参[MEDIA_RW_MODE = %d], Return: %s",
            enRWMode, ConvertDevErrCodeToStr(ERR_IDC_PARAM_ERR));
        return ERR_IDC_PARAM_ERR;
    }

    return IDC_SUCCESS;
}

// 设置数据
int CDevIDX_DMTM1M::SetData(unsigned short usType, void *vData)
{
    THISMODULE(__FUNCTION__);

    switch(usType)
    {
        case SET_DEBUG_MODE: // debug mode
        {
            if (vData != nullptr)
            {
                m_debugMode = false;
                memcpy(&m_debugMode, vData, sizeof(BOOL));
            }
            break;
        }
        case SET_IMAGE_PAR: // 设置图像参数
        {
            if (vData != nullptr)
            {
                LPSTIMAGEPARAM lpImgPar = (LPSTIMAGEPARAM)vData;

                m_wBankNo = lpImgPar->wBankNo;
                // 证件头像存放位置
                memset(m_szHeadImgSavePath, 0x00, sizeof(m_szHeadImgSavePath));
                memcpy(m_szHeadImgSavePath, lpImgPar->szHeadImgSavePath, strlen(lpImgPar->szHeadImgSavePath));
                // 扫描图像存放位置
                memset(m_szScanImgSavePath, 0x00, sizeof(m_szScanImgSavePath));
                memcpy(m_szScanImgSavePath, lpImgPar->szScanImgSavePath, strlen(lpImgPar->szScanImgSavePath));
                // 头像名(空不使用)
                memset(m_szHeadImgName, 0x00, sizeof(m_szHeadImgName));
                memcpy(m_szHeadImgName, lpImgPar->szHeadImgName, strlen(lpImgPar->szHeadImgName));
                // 证件正面图像名(空不使用)
                memset(m_szScanImgFrontName, 0x00, sizeof(m_szScanImgFrontName));
                memcpy(m_szScanImgFrontName, lpImgPar->szScanImgFrontName, strlen(lpImgPar->szScanImgFrontName));
                // 证件背面图像名(空不使用)
                memset(m_szScanImgBackName, 0x00, sizeof(m_szScanImgBackName));
                memcpy(m_szScanImgBackName, lpImgPar->szScanImgBackName, strlen(lpImgPar->szScanImgBackName));
                // 证件扫描图像保存类型
                m_wScanImgSaveType = lpImgPar->wScanImgSaveType;
                if (m_wScanImgSaveType == SAVE_IMG_BMP || m_wScanImgSaveType == SAVE_IMG_BMP_X)
                {
                    m_wScanImgSaveType = SAVE_IMG_BMP;
                } else
                if (m_wScanImgSaveType == SAVE_IMG_JPG || m_wScanImgSaveType == SAVE_IMG_JPG_X)
                {
                    m_wScanImgSaveType = SAVE_IMG_JPG;
                } else
                {
                    m_wScanImgSaveType = SAVE_IMG_JPG;
                }
                memset(m_szScanImgSaveTypeS, 0x00, sizeof(m_szScanImgSaveTypeS));
                switch(lpImgPar->wScanImgSaveType)
                {
                    case SAVE_IMG_BMP: sprintf(m_szScanImgSaveTypeS, "%s", SAVE_IMG_BMP_S); break;
                    case SAVE_IMG_JPG: sprintf(m_szScanImgSaveTypeS, "%s", SAVE_IMG_JPG_S); break;
                    case SAVE_IMG_BMP_X: sprintf(m_szScanImgSaveTypeS, "%s", SAVE_IMG_BMP_SX); break;
                    case SAVE_IMG_JPG_X: sprintf(m_szScanImgSaveTypeS, "%s", SAVE_IMG_JPG_SX); break;
                    default: sprintf(m_szScanImgSaveTypeS, "%s", SAVE_IMG_JPG_S); break;
                }
                // 证件扫描图像保存图片缩放比例
                m_fScanImgSaveZoomSc = lpImgPar->fScanImgSaveZoomSc;
                // 证件扫描图像是否以人像信息面为正面
                m_wScanImageFrontIsInfor = lpImgPar->wScanImageFrontIsInfor;
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
        case SET_LIB_VER:           // 设置Lib版本
        {
            break;
        }
        case SET_DEV_RECON:         // 设置断线重连标记为T
        {
            return m_pDevImpl.SetReConFlag(TRUE);
        }
        default:
                break;
    }

    return IDC_SUCCESS;
}

// 获取数据
int CDevIDX_DMTM1M::GetData(unsigned short usType, void *vData)
{
    //THISMODULE(__FUNCTION__);

    switch (usType)
    {
    case SET_DEBUG_MODE:
    {
        if (vData != nullptr)
        {
            memcpy(vData, &m_debugMode, sizeof(BOOL));
        }
        break;
    }
    default:;
    }

    return IDC_SUCCESS;
}

// 获取版本
int CDevIDX_DMTM1M::GetVersion(unsigned short usType, char* szVer, int nSize)
{
    THISMODULE(__FUNCTION__);

    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_FW:        // 固件版本号
                m_pDevImpl.nGetFWVersion(szVer);
                break;
            default:
                break;
        }
    }

    return IDC_SUCCESS;
}

// 退卡处理
// dwParam: 0:退卡到出口, 非0:完全弹出
INT CDevIDX_DMTM1M::nEjectCard(DWORD dwParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    if (dwParam == 0)  // 退卡到出口
    {
        nRet = m_pDevImpl.nEjectIdCard(0);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "退卡到出口: ->nBackAndHoldIdCard() Fail, ErrCode: %d, Return: %s",
                nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
            return ConvertCode_Impl2IDC(nRet);
        }
    } else  // 完全弹出
    {
        nRet = m_pDevImpl.nEjectIdCard(1);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "退卡完全弹出: ->nBackIdCard() Fail, ErrCode: %d, Return: %s",
                nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
            return ConvertCode_Impl2IDC(nRet);
        }
    }

    return IDC_SUCCESS;
}

// 等待放卡处理(循环检查是否有卡进入入口)
INT CDevIDX_DMTM1M::AcceptMedia(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    m_bCancelReadCard = FALSE;

    INT nRet = IMP_SUCCESS;

    ULONG ulTimeCount = 0;
    QTime qtTimeCurr1, qtTimeCurr2;
    qtTimeCurr1 = QTime::currentTime();     // 取吸卡执行前时间

    // 循环检测是否放卡
    while(1)
    {
        QCoreApplication::processEvents();
        if (m_bCancelReadCard == TRUE)      // 取消读卡
        {
            m_bCancelReadCard = FALSE;
            return ERR_IDC_USER_CANCEL;
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

        // 检查是否有卡
        nRet = m_pDevImpl.nGetCardStat();
        if (nRet == IMP_SUCCESS)    // 已放卡
        {
            break;
        } else
        if (nRet == IMP_ERR_DEV_8D01H)  // 未放卡
        {
            continue;
        } else
        {
            Log(ThisModule, __LINE__, "进卡: ->nGetCardStat() Fail, ErrCode: %d, Return: %s.",
                nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
            return ConvertCode_Impl2IDC(nRet);
        }
    }

    return IDC_SUCCESS;
}

// 分类型证件数据组合
INT CDevIDX_DMTM1M::IDCardDataToTrack(LPVOID lpVoidData, LPSTR lpDestData, INT nDetSize, WORD wCardType)
{
    EN_IDINFOMODE enIDMode = IDINFO_00;     // 缺省

    if (m_wBankNo == 1)     // 长沙银行
    {
        enIDMode = IDINFO_01;
    } else
    if (m_wBankNo == 2)     // 陕西信合
    {
        enIDMode = IDINFO_02;
    }

    if (wCardType == 1)     // 国内二代身份证
    {
        return GetIDCardStr(*(LPSTIDCARDINFO)lpVoidData, enIDMode, lpDestData, nDetSize);
    } else
    if (wCardType == 2)     // 国外身份证
    {
        return GetIDForeignStr(*(LPSTIDFOREIGNINFO)lpVoidData, enIDMode, lpDestData, nDetSize);
    } else
    if (wCardType == 3)     // 港澳台通行证
    {
        return GetIDGATStr(*(LPSTIDGATINFO)lpVoidData, enIDMode, lpDestData, nDetSize);
    }

    return TRUE;
}

// DMTM1M错误码转换为DevIDC错误码
INT CDevIDX_DMTM1M::ConvertCode_Impl2IDC(INT nRetCode)
{
#define DMTM1M_CASE_CODE_IMP2DEV(IMP, DEV) \
    case IMP: return DEV;

    switch(nRetCode)
    {
        // Impl处理返回
        DMTM1M_CASE_CODE_IMP2DEV(IMP_SUCCESS, IDC_SUCCESS);                    // 成功
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_LOAD_LIB, ERR_IDC_LIBRARY);           // 动态库加载失败
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_PARAM_INVALID, ERR_IDC_PARAM_ERR);    // 参数无效
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_UNKNOWN, ERR_IDC_OTHER);              // 未知错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_NOTOPEN, ERR_IDC_DEV_NOTOPEN);        // 设备未Open
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_OTHER, ERR_IDC_OTHER);                // 其他错误
        // Device返回
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8101H, ERR_IDC_DEV_NOTOPEN);       // 设备未打开
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8102H, ERR_IDC_COMM_ERR);          // 传输错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8103H, ERR_IDC_OTHER);             // 句柄错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8201H, ERR_IDC_OTHER);             // XOR 错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8202H, ERR_IDC_OTHER);             // SUM 错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8203H, ERR_IDC_OTHER);             // 指令号错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8204H, ERR_IDC_PARAM_ERR);         // 参数错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8205H, ERR_IDC_COMM_ERR);          // 包头错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8206H, ERR_IDC_COMM_ERR);          // 包长错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8D01H, ERR_IDC_MED_NOTFOUND);      // 无卡
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8D02H, ERR_IDC_COMM_ERR);          // apdu 指令交互失败
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8E01H, ERR_IDC_OTHER);             // 获取SAMID 失败
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8E02H, ERR_IDC_OTHER);             // 读取身份证信息失败
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8F01H, ERR_IDC_OTHER);             // XOR 错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8F02H, ERR_IDC_OTHER);             // SUM 错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8F03H, ERR_IDC_OTHER);             // 指令号错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8F04H, ERR_IDC_PARAM_ERR);         // 参数错误
        DMTM1M_CASE_CODE_IMP2DEV(IMP_ERR_DEV_8F05H, ERR_IDC_COMM_ERR);          // 传输超时
        default :
            return IDC_SUCCESS;
    }
}

//////////////////////////////////////////////////////////////////////////






