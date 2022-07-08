#include "DevIDX_BSID81.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>
#include <QSettings>
#include <sys/stat.h>

#include <dirent.h>

// IDX 版本号
BYTE    byDevVRTU[17] = {"DevIDX00000100"};

static const char *ThisFile = "DevIDX_BSID81.cpp";

//////////////////////////////////////////////////////////////////////////

CDevIDX_BSID81::CDevIDX_BSID81() : m_pDevBSID81(LOG_NAME_DEVIDX)
{
    SetLogFile(LOG_NAME_DEVIDX, ThisFile);  // 设置日志文件名和错误发生的文件

    memset(&m_stOldStatus, 0x00, sizeof(DEVIDXSTATUS));
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
    m_wEjectMode = 0;                                                   // 退卡动作模式(0保留在门口/1完全弹出)
    m_wScanImageFrontIsInfor = 1;                                       // 证件扫描图像是否以人像信息面为正面
    m_bIsCanceled = false;
    m_wBankNo = 0;
}

CDevIDX_BSID81::~CDevIDX_BSID81()
{
    m_pDevBSID81.CloseDevice();
}

void CDevIDX_BSID81::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    m_pDevBSID81.CloseDevice();
    return;
}

int CDevIDX_BSID81::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_pDevBSID81.CloseDevice();

    // 建立Idx连接
    if (m_pDevBSID81.IsDeviceOpen() != TRUE)
    {
        if (m_pDevBSID81.OpenDevice() != TRUE)
        {
            return ERR_IDX_NOT_OPEN;
        }
    }

    return ERR_IDX_SUCCESS;
}

int CDevIDX_BSID81::Reset(const int nActFlag)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nAct = 0;

    // Reset卡动作转换为设备支持
    if (nActFlag == CARDACTION_NOACTION)    // 无动作
    {
        nAct = CARD_NOACTION;
    } else
    if (nActFlag == CARDACTION_EJECT)    // 退卡
    {
        if (m_wEjectMode == 0)
        {
            nAct = CARD_EJECTMENT; // 退卡并持卡(退到门口)
        } else
        {
            nAct = CARD_EJECT;  // 完全弹出
        }
    } else
    if (nActFlag == CARDACTION_RETAIN)    // 吞卡
    {
        nAct = CARD_RETRACT;
    } else
    {
        nAct = CARD_NOACTION;
    }

    if (m_pDevBSID81.bSoftResetDevice(nAct) != TRUE)
    {
        return lErrCodeChgToIdx(m_pDevBSID81.GetErrCode());
        //return ERR_IDX_HWERR;
    }

    return ERR_IDX_SUCCESS;
}

void CDevIDX_BSID81::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_pDevBSID81.CloseDevice();

    return;
}

/************************************************************
** 功能：取设备状态
** 输入：无
** 输出：stStatus : 设备状态结构体
** 返回：无
************************************************************/
int CDevIDX_BSID81::GetStatus(DEVIDXSTATUS &stStatus)
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

    stStatus.clear();

    // 设备未Open
    if (m_pDevBSID81.IsDeviceOpen() != TRUE)
    {
        return ERR_IDX_NOT_OPEN;
    }

    // 获取设备状态
    if (m_pDevBSID81.bGetDevStatus(m_stDevStatus) != TRUE)
    {
        return lErrCodeChgToIdx(m_pDevBSID81.GetErrCode(TRUE));
        // return ERR_IDX_HWERR;
    }

    // ----------------Check 状态----------------
    // Device POWEROFF 断电
    if (m_stDevStatus.iStatusPowerOff == 1)     // 掉电
    {
        stStatus.wDevice = DEV_POWEROFF;     // 设备故障
        stStatus.wMedia = MEDIA_UNKNOWN;    // 卡的状态未知
    } else
    // Device HWERR
    if (m_stDevStatus.iStatusCoverOpen == 1 ||      // 上盖打开
        m_stDevStatus.iStatusLowVoltage == 1 ||     // 低电压错误
        m_stDevStatus.iStatusProcess == 1)          // 过程出错
    {
        stStatus.wDevice = DEV_HWERROR;     // 设备故障
        stStatus.wMedia = MEDIA_UNKNOWN;    // 卡的状态未知
    } else
    // Device Busy
    if (m_stDevStatus.iStatusProcess == 2)  // 执行过程中
    {
        stStatus.wDevice = DEV_BUSY;        // 设备忙
    } else
    {
        stStatus.wDevice = DEV_ONLINE;      // 设备正常
    }

    // Media JAM
    if (m_stDevStatus.iStatusCardJam == 1)  // JAM
    {
        stStatus.wDevice = DEV_HWERROR;
        stStatus.wMedia = MEDIA_JAMMED;
    }

    // Media status
    if (stStatus.wDevice == DEV_BUSY || stStatus.wDevice == DEV_ONLINE)
    {
        if (m_stDevStatus.iStatusMiddleSensorHaveCard == 0 &&     // 中间传感器无卡
            m_stDevStatus.iStatusScanSensorHaveCard == 0 &&       // 扫描传感器无卡
            m_stDevStatus.iStatusInputSensorHaveCard == 0)        // 入口传感器无卡
        {
            stStatus.wMedia = MEDIA_NOTPRESENT;
        } else
        {
            if (m_stDevStatus.iStatusMiddleSensorHaveCard == 1 ||    // 中间传感器有卡
                m_stDevStatus.iStatusScanSensorHaveCard == 1)        // 扫描传感器有卡
            {
                stStatus.wMedia = MEDIA_PRESENT;    // 介质在通道内
            }

            if (m_stDevStatus.iStatusInputSensorHaveCard == 1)        // 入口传感器有卡
            {
                stStatus.wMedia = MEDIA_ENTERING;    // 介质位于的入卡口位置
            }
        }
    }

    if (memcmp(&stStatus, &m_stOldStatus, sizeof(DEVIDXSTATUS)) != 0)
    {
        Log(ThisModule, __LINE__,
            "转换后设备状态: GetStatus()　Result: %d|%s -> %d|%s, %d|%s -> %d|%s",
            m_stOldStatus.wDevice, cDeviceStatus[m_stOldStatus.wDevice],
            stStatus.wDevice, cDeviceStatus[stStatus.wDevice],
            m_stOldStatus.wMedia, cMediaStatus[m_stOldStatus.wMedia],
            stStatus.wMedia, cMediaStatus[stStatus.wMedia]);
        memcpy(&m_stOldStatus, &stStatus, sizeof(DEVIDXSTATUS));
    }

    // 强制退证(卡)按钮按下

    return ERR_IDX_SUCCESS;
}

// 取卡位置
int CDevIDX_BSID81::GetCardPosition(CardPosition &enCardPos)
{
    //enCardPos = CARDPOS_UNKNOWN; // 未知状态

    // 设备未Open
    if (m_pDevBSID81.IsDeviceOpen() != TRUE)
    {
        return ERR_IDX_NOT_OPEN; // 未知状态;
    }

    // 获取设备状态
    if (m_pDevBSID81.bGetDevStatus(m_stDevStatus) != TRUE)
    {
        return lErrCodeChgToIdx(m_pDevBSID81.GetErrCode(TRUE));
        //return ERR_IDX_COMM_ERR;
    }

    if (m_stDevStatus.iStatusCardJam == 1)  // JAM
    {
        return ERR_IDX_JAMMED;
    }

    // ----------------Check 状态----------------
    if (m_stDevStatus.iStatusMiddleSensorHaveCard == 0 &&     // 中间传感器无卡
        m_stDevStatus.iStatusScanSensorHaveCard == 0 &&       // 扫描传感器无卡
        m_stDevStatus.iStatusInputSensorHaveCard == 0)        // 入口传感器无卡
    {
        enCardPos = CARDPOS_NOCARD; // 无卡
    } else
    {
        if (m_stDevStatus.iStatusMiddleSensorHaveCard == 1)    // 中间传感器有卡
        {
            enCardPos = CARDPOS_INTERNAL; // 内部
        } else
        if (m_stDevStatus.iStatusScanSensorHaveCard == 1)        // 扫描传感器有卡
        {
            enCardPos = CARDPOS_SCANNING; // 扫描位置
        } else
        if (m_stDevStatus.iStatusInputSensorHaveCard == 1)        // 入口传感器有卡
        {
            enCardPos = CARDPOS_ENTERING; // 门口
        }
    }

    return ERR_IDX_SUCCESS;
}

// 吞卡
int CDevIDX_BSID81::RetainCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if (m_pDevBSID81.bRetainIdCard() != TRUE)
    {
        return lErrCodeChgToIdx(m_pDevBSID81.GetErrCode());
        //return ERR_IDX_HWERR;
    }

    return ERR_IDX_SUCCESS;
}

// 退卡
int CDevIDX_BSID81::EjectCard()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    // 检查后部扫描位置是否有卡存在，有则移动卡到可识别位置。
    // 目的：当卡处于扫描器位置时，BackIdCardToRerec无法退卡
    // 该部分不进行错误处理
    DEVSTATUS stStat;
    if (m_pDevBSID81.bGetDevStatus(stStat) == TRUE)
    {
        if (stStat.iStatusScanSensorHaveCard == 1)  // 扫描位置有卡,退卡到可识别位置(失败后执行3次不报错)
        {
            if (m_pDevBSID81.bBackIdCardToRerec() != TRUE)
            {
                if (m_pDevBSID81.bBackIdCardToRerec() != TRUE)
                {
                    if (m_pDevBSID81.bBackIdCardToRerec() != TRUE)
                    {
                        Log(ThisModule, __LINE__, "退卡: EjectCard()->bCheckHaveIdCard() 扫描位置有卡, 退卡到可识别位置 fail. Not Return Error. ");
                    }
                }
            }
        }
    }

    if (m_wEjectMode == 0)  // 退卡到卡口
    {
        if (m_pDevBSID81.bBackAndHoldIdCard() != TRUE)
        {
            return lErrCodeChgToIdx(m_pDevBSID81.GetErrCode());
            //return ERR_IDX_HWERR;
        }
    } else  // 完全弹出
    {
        if (m_pDevBSID81.bBackIdCard() != TRUE)
        {
            return lErrCodeChgToIdx(m_pDevBSID81.GetErrCode());
            //return ERR_IDX_HWERR;
        }
    }

    return ERR_IDX_SUCCESS;
}

// 进卡
int CDevIDX_BSID81::AcceptCard(unsigned long ulTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    m_bIsCanceled = false;

    // 检查后部扫描位置是否有卡存在，有则移动卡到可识别位置。
    // 目的：当卡处于扫描器位置时，CheckHaveIdCard检测不到卡存在
    // 该部分不进行错误处理
    DEVSTATUS stStat;
    if (m_pDevBSID81.bGetDevStatus(stStat) == TRUE)
    {
        if (stStat.iStatusScanSensorHaveCard == 1)  // 扫描位置有卡,退卡到可识别位置(失败后执行3次不报错)
        {
            if (m_pDevBSID81.bBackIdCardToRerec() != TRUE)
            {
                if (m_pDevBSID81.bBackIdCardToRerec() != TRUE)
                {
                    if (m_pDevBSID81.bBackIdCardToRerec() != TRUE)
                    {
                        Log(ThisModule, __LINE__, "进卡: AcceptCard()->bCheckHaveIdCard() 扫描位置有卡, 退卡到可识别位置 fail. Not Return Error. ");
                    }
                }
            }
        }
    }

    // 检测是否放入卡
    //WORD wTimeCount = 0;  // 40-00-00-00(FT#0011)
    ULONG wTimeCount = 0;
    while(1)
    {
        QCoreApplication::processEvents();
        if (m_bIsCanceled == TRUE)
        {
            return ERR_IDX_USER_CANCEL;
        }
        if (ulTimeOut > 0)
        {
            if (wTimeCount >= ulTimeOut)
            {
                Log(ThisModule, __LINE__, "进卡: AcceptCard()->bCheckHaveIdCard() 检查放入卡 fail. Read TimeOut. ");
                return ERR_IDX_INSERT_TIMEOUT;     // 超时
            }
        }

        if (m_pDevBSID81.bCheckHaveIdCard(0) != TRUE)
        {
            // 40-00-00-00(FT#0011) ADD START // 吸卡失败后进行状态检查
            DEVIDXSTATUS stStatus;
            int nStatRet = GetStatus(stStatus);
            if (nStatRet != ERR_IDX_SUCCESS)
            {
                Log(ThisModule, __LINE__, "进卡: AcceptCard()->GetStatus() 检查放入卡 fail,获取状态失败，Return: %d.", nStatRet);
                return nStatRet;
            } else
            {
                if (stStatus.wMedia == MEDIA_JAMMED)
                {
                    Log(ThisModule, __LINE__, "进卡: AcceptCard()->GetStatus() 检查放入卡 fail,获取状态:Media=[%d|JAM]，Return: %d.",
                        stStatus.wMedia, ERR_IDX_JAMMED);
                    return ERR_IDX_JAMMED;
                }
            }
            // 40-00-00-00(FT#0011) ADD END

            if (ulTimeOut > 0)
            {
                wTimeCount = wTimeCount + 1000;
            }
            continue;
        } else
        {
            break;
        }
    }

    return ERR_IDX_SUCCESS;
}

// 读卡
int CDevIDX_BSID81::ReadTracks(STTRACK_INFO &stTrackInfo, unsigned int unMode)// 40-00-00-00(FT#0012)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    char cImgScanFront[256 * 2] = { 0x00 };
    char cImgScanBack[256 * 2] = { 0x00 };
    char cImgHeadFile[256 * 2] = { 0x00 };
    char szIDCardNo[256] = { 0x00 };


    INT nInPar = RT_TRACK1 | RT_TRACK2 | RT_TRACK3 | RT_CHIP |RT_FRONTIMAGE | RT_BACKIMAGE; // 40-00-00-00(FT#0012)
    if ((unMode & nInPar) == 0)                                                             // 40-00-00-00(FT#0012)
    {                                                                                       // 40-00-00-00(FT#0012)
        Log(ThisModule, __LINE__, "读卡: 入参[%d] 无效. Return: %d. ");                                // 40-00-00-00(FT#0012)
        return ERR_IDX_PARAM_ERR;                                                           // 40-00-00-00(FT#0012)
    }                                                                                       // 40-00-00-00(FT#0012)

    if ((unMode & RT_TRACK1)  == RT_TRACK1 || ( unMode & RT_CHIP) == RT_CHIP)   // 读数据    // 40-00-00-00(FT#0012)
    {                                                                                       // 40-00-00-00(FT#0012)
        if (strlen(m_szHeadImgName) > 0)   // 指定证件正面图像名
        {
            sprintf(cImgHeadFile, "%s/%s", m_szHeadImgSavePath, m_szHeadImgName);
        } else
        {
            sprintf(cImgHeadFile, "%s", m_szHeadImgSavePath);
        }

        // 获取卡类型
        int nCardType = 0;
        if (m_pDevBSID81.bGetIDCardType(&nCardType) != TRUE)
        {
            Log(ThisModule, __LINE__, "读卡: ReadTracks()->bGetIDCardType() 获取卡类型 fail. Read Error. ");
            return ERR_IDX_READERROR;     // 读卡错误
        }
        Log(ThisModule, __LINE__, "读卡: ReadTracks()->bGetIDCardType() 获取卡类型 succ. Is %s.",
            nCardType == ID_CHINA ? "国内证件" : (nCardType == ID_FOREIGN? "国外证件" : "港澳台证件"));

        // 读卡数据
        if (nCardType == ID_CHINA)  // 国内证件
        {
            IDInfoEx stInfoData;
            memset(&stInfoData, 0x00, sizeof(IDInfoEx));
            if (m_pDevBSID81.bGetID2InfoEx(stInfoData, (LPSTR)cImgHeadFile/*m_szHeadImgSavePath*/) != TRUE)
            {
                Log(ThisModule, __LINE__, "读卡: ReadTracks()->bGetID2InfoEx() 国内证件读 fail. Read Error. ");
                return ERR_IDX_READERROR;     // 读卡错误
            }

            // 组织证件数据写入应答
            stTrackInfo.TrackData[0].bTrackOK =
                    vIDCardDataToTrack(&stInfoData, stTrackInfo.TrackData[0].szTrack, nCardType);

            // 扫描图像路径(INI指定/证件号码_front/back.)
            sprintf(szIDCardNo, "%s", QString(stInfoData.number).trimmed().toStdString().c_str());// 40-00-00-00(FT#0012)
            /* // 40-00-00-00(FT#0012) DELETE START
            if (strlen(m_szScanImgFrontName) > 0)   // 指定证件正面图像名
            {
                sprintf(cImgScanFront, "%s/%s", m_szScanImgSavePath, m_szScanImgFrontName);
            } else
            {
                sprintf(cImgScanFront, "%s/%s_front.%s",
                        m_szScanImgSavePath, QString(stInfoData.number).trimmed().toStdString().c_str(), m_szScanImgSaveTypeS);
            }
            if (strlen(m_szScanImgBackName) > 0)   // 指定证件背面图像名
            {
                sprintf(cImgScanBack, "%s/%s", m_szScanImgSavePath, m_szScanImgBackName);
            } else
            {
                sprintf(cImgScanBack, "%s/%s_back.%s",
                        m_szScanImgSavePath, QString(stInfoData.number).trimmed().toStdString().c_str(), m_szScanImgSaveTypeS);
            } // 40-00-00-00(FT#0012) DELETE END */
        } else
        if (nCardType == ID_FOREIGN)  // 国外证件
        {
            IDInfoForeign stInfoData;
            memset(&stInfoData, 0x00, sizeof(IDInfoEx));
            if (m_pDevBSID81.bGetIDInfoForeign(stInfoData, (LPSTR)cImgHeadFile/*m_szHeadImgSavePath*/) != TRUE)
            {
                Log(ThisModule, __LINE__, "读卡: ReadTracks()->bGetIDInfoForeign() 国外证件读 fail. Read Error. ");
                return ERR_IDX_READERROR;     // 读卡错误
            }

            // 组织证件数据写入应答
            stTrackInfo.TrackData[0].bTrackOK =
                    vIDCardDataToTrack(&stInfoData, stTrackInfo.TrackData[0].szTrack, nCardType);

            // 扫描图像路径(INI指定/证件号码_front/back.)
            sprintf(szIDCardNo, "%s", QString(stInfoData.IDCardNO).trimmed().toStdString().c_str());// 40-00-00-00(FT#0012)
            /* // 40-00-00-00(FT#0012) DELETE START
            if (strlen(m_szScanImgFrontName) > 0)   // 指定证件正面图像名
            {
                sprintf(cImgScanFront, "%s/%s", m_szScanImgSavePath, m_szScanImgFrontName);
            } else
            {
                sprintf(cImgScanFront, "%s/%s_front.%s",
                        m_szScanImgSavePath, QString(stInfoData.IDCardNO).trimmed().toStdString().c_str(), m_szScanImgSaveTypeS);
            }
            if (strlen(m_szScanImgBackName) > 0)   // 指定证件背面图像名
            {
                sprintf(cImgScanBack, "%s/%s", m_szScanImgSavePath, m_szScanImgBackName);
            } else
            {
                sprintf(cImgScanBack, "%s/%s_back.%s",
                        m_szScanImgSavePath, QString(stInfoData.IDCardNO).trimmed().toStdString().c_str(), m_szScanImgSaveTypeS);
            } // 40-00-00-00(FT#0012) DELETE END */
        } else
        if (nCardType == ID_GAT)  // 港澳台证件
        {
            IDInfoGAT stInfoData;
            memset(&stInfoData, 0x00, sizeof(IDInfoEx));
            if (m_pDevBSID81.bGetIDInfoGAT(stInfoData, (LPSTR)cImgHeadFile/*m_szHeadImgSavePath*/) != TRUE)
            {
                Log(ThisModule, __LINE__, "读卡: ReadTracks()->bGetIDInfoGAT() 港澳台证件读 fail. Read Error. ");
                return ERR_IDX_READERROR;     // 读卡错误
            }

            // 组织证件数据写入应答
            stTrackInfo.TrackData[0].bTrackOK =
                    vIDCardDataToTrack(&stInfoData, stTrackInfo.TrackData[0].szTrack, nCardType);

            // 扫描图像路径(INI指定/证件号码_front/back.jpg)
            sprintf(szIDCardNo, "%s", QString(stInfoData.number).trimmed().toStdString().c_str());// 40-00-00-00(FT#0012)
            /* // 40-00-00-00(FT#0012) DELETE START
            if (strlen(m_szScanImgFrontName) > 0)   // 指定证件正面图像名
            {
                sprintf(cImgScanFront, "%s/%s", m_szScanImgSavePath, m_szScanImgFrontName);
            } else
            {
                sprintf(cImgScanFront, "%s/%s_front.%s",
                        m_szScanImgSavePath, QString(stInfoData.number).trimmed().toStdString().c_str(), m_szScanImgSaveTypeS);
            }
            if (strlen(m_szScanImgBackName) > 0)   // 指定证件背面图像名
            {
                sprintf(cImgScanBack, "%s/%s", m_szScanImgSavePath, m_szScanImgBackName);
            } else
            {
                sprintf(cImgScanBack, "%s/%s_back.%s",
                        m_szScanImgSavePath, QString(stInfoData.number).trimmed().toStdString().c_str(), m_szScanImgSaveTypeS);
            } // 40-00-00-00(FT#0012) DELETE END */
        } else
        {
            Log(ThisModule, __LINE__, "读卡: ReadTracks()->bGetIDCardType() 无效卡类型<%d>. ", nCardType);
            return ERR_IDX_INVALIDCARD;
        }
    }                                                                                               // 40-00-00-00(FT#0012)

    if ((unMode & RT_TRACK2)  == RT_TRACK2 || (unMode & RT_TRACK3) == RT_TRACK3 ||                          // 40-00-00-00(FT#0012)
        (unMode & RT_FRONTIMAGE)  == RT_FRONTIMAGE || (unMode & RT_BACKIMAGE) == RT_BACKIMAGE)   // 扫描图像 // 40-00-00-00(FT#0012)
    {                                                                                                       // 40-00-00-00(FT#0012)
        // 扫描图像路径(INI指定/证件号码_front/back.)
        // 40-00-00-00(FT#0012) ADD START
        if (strlen(m_szScanImgFrontName) > 0)   // 指定证件正面图像名
        {
            sprintf(cImgScanFront, "%s/%s", m_szScanImgSavePath, m_szScanImgFrontName);
        } else
        {
            sprintf(cImgScanFront, "%s/%s_front.%s",
                        m_szScanImgSavePath, szIDCardNo, m_szScanImgSaveTypeS);
        }
        if (strlen(m_szScanImgBackName) > 0)   // 指定证件背面图像名
        {
            sprintf(cImgScanBack, "%s/%s", m_szScanImgSavePath, m_szScanImgBackName);
        } else
        {
            sprintf(cImgScanBack, "%s/%s_back.%s",
                    m_szScanImgSavePath, szIDCardNo, m_szScanImgSaveTypeS);
        }
        // 40-00-00-00(FT#0012) ADD END

        // 启动扫描
        if (m_pDevBSID81.bStartScanIdCard() != TRUE)
        {
            Log(ThisModule, __LINE__, "读卡: ReadTracks()->bStartScanIdCard() 启动证件扫描 fail. Read Error. ");
            return ERR_IDX_READERROR;     // 读卡错误
        }

        memset(m_szScanImgFrontBuff, 0x00, sizeof(m_szScanImgFrontBuff));
        memset(m_szScanImgBackBuff, 0x00, sizeof(m_szScanImgBackBuff));
        m_nScanImgFrontBuffLen = 0;
        m_nScanImgBackBuffLen = 0;

        // 读取当前图像数据块到内存(Front:人像面/Back:名称面)
        if (m_pDevBSID81.bSavePicToMemory(m_szScanImgFrontBuff, m_szScanImgBackBuff, &m_nScanImgFrontBuffLen, &m_nScanImgBackBuffLen) != TRUE)
        {
            Log(ThisModule, __LINE__, "读卡: ReadTracks()->bSavePicToMemory() 扫描证件正反图像数据块到内存 fail. Read Error. ");
            return ERR_IDX_READERROR;     // 读卡错误
        }
        if ((unMode & RT_TRACK2)  == RT_TRACK2 || (unMode & RT_FRONTIMAGE)  == RT_FRONTIMAGE)   // 扫描正面图像// 40-00-00-00(FT#0012)
        {                                                                                                   // 40-00-00-00(FT#0012)
            if (m_wScanImageFrontIsInfor == 1)    // 证件扫描图像以人像信息面为正面(Front)
            {
                // 保存图像数据块到文件(指定缩放比例)_正面扫描:人像面
                if (m_pDevBSID81.bSavePicToFileII(m_szScanImgFrontBuff, m_nScanImgFrontBuffLen, cImgScanFront,
                                                  m_wScanImgSaveType, m_fScanImgSaveZoomSc) != TRUE)
                {
                    Log(ThisModule, __LINE__, "读卡: ReadTracks()->bSavePicToFileII() 保存正面扫描图像数据块到文件 fail. Read Error. ");
                    return ERR_IDX_READERROR;     // 读卡错误
                }
            } else
            {
                // 保存图像数据块到文件(指定缩放比例)_正面扫描
                if (m_pDevBSID81.bSavePicToFileII(m_szScanImgBackBuff, m_nScanImgBackBuffLen, cImgScanFront,
                                                  m_wScanImgSaveType, m_fScanImgSaveZoomSc) != TRUE)
                {
                    Log(ThisModule, __LINE__, "读卡: ReadTracks()->bSavePicToFileII() 保存正面扫描图像数据块到文件 fail. Read Error. ");
                    return ERR_IDX_READERROR;     // 读卡错误
                }
            }
            // 组织证件数据写入应答
            stTrackInfo.TrackData[1].bTrackOK = TRUE;
            memcpy(stTrackInfo.TrackData[1].szTrack, cImgScanFront, strlen(cImgScanFront));
        }                                                                                                   // 40-00-00-00(FT#0012)

        if ((unMode & RT_TRACK3) == RT_TRACK3 || (unMode & RT_BACKIMAGE) == RT_BACKIMAGE)   // 扫描背面图像   // 40-00-00-00(FT#0012)
        {                                                                                                   // 40-00-00-00(FT#0012)
            if (m_wScanImageFrontIsInfor == 1)    // 证件扫描图像以人像信息面为正面(Front),名称面为反面(Back)
            {
                // 保存图像数据块到文件(指定缩放比例)_背面扫描:名称面
                if (m_pDevBSID81.bSavePicToFileII(m_szScanImgBackBuff, m_nScanImgBackBuffLen, cImgScanBack,
                                                  m_wScanImgSaveType, m_fScanImgSaveZoomSc) != TRUE)
                {
                    Log(ThisModule, __LINE__, "读卡: ReadTracks()->bSavePicToFileII() 保存背面扫描图像数据块到文件 fail. Read Error. ");
                    return ERR_IDX_READERROR;     // 读卡错误
                }
            } else
            {
                // 保存图像数据块到文件(指定缩放比例)_背面扫描
                if (m_pDevBSID81.bSavePicToFileII(m_szScanImgFrontBuff, m_nScanImgFrontBuffLen, cImgScanBack,
                                                  m_wScanImgSaveType, m_fScanImgSaveZoomSc) != TRUE)
                {
                    Log(ThisModule, __LINE__, "读卡: ReadTracks()->bSavePicToFileII() 保存背面扫描图像数据块到文件 fail. Read Error. ");
                    return ERR_IDX_READERROR;     // 读卡错误
                }
            }
            // 组织证件数据写入应答
            stTrackInfo.TrackData[2].bTrackOK = TRUE;
            memcpy(stTrackInfo.TrackData[2].szTrack, cImgScanBack, strlen(cImgScanBack));
        }                                                                                                   // 40-00-00-00(FT#0012)
    }                                                                                                       // 40-00-00-00(FT#0012)

    return ERR_IDX_SUCCESS;
}

// 命令取消
int CDevIDX_BSID81::Cancel()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    m_bIsCanceled = true;

    return ERR_IDX_SUCCESS;
}

// 设置数据
int CDevIDX_BSID81::SetData(void *vData, WORD wDataType)
{
    THISMODULE(__FUNCTION__);

    switch(wDataType)
    {
        case DATATYPE_INIT: // 0/初始化数据
        {
            LPSTIDXDEVINITPARAM lpInit = (LPSTIDXDEVINITPARAM)vData;

            m_wBankNo = lpInit->wBankNo;
            // 证件头像存放位置
            memset(m_szHeadImgSavePath, 0x00, sizeof(m_szHeadImgSavePath));
            memcpy(m_szHeadImgSavePath, lpInit->szHeadImgSavePath, strlen(lpInit->szHeadImgSavePath));
            // 扫描图像存放位置
            memset(m_szScanImgSavePath, 0x00, sizeof(m_szScanImgSavePath));
            memcpy(m_szScanImgSavePath, lpInit->szScanImgSavePath, strlen(lpInit->szScanImgSavePath));
            // 头像名(空不使用)
            memset(m_szHeadImgName, 0x00, sizeof(m_szHeadImgName));
            memcpy(m_szHeadImgName, lpInit->szHeadImgName, strlen(lpInit->szHeadImgName));
            // 证件正面图像名(空不使用)
            memset(m_szScanImgFrontName, 0x00, sizeof(m_szScanImgFrontName));
            memcpy(m_szScanImgFrontName, lpInit->szScanImgFrontName, strlen(lpInit->szScanImgFrontName));
            // 证件背面图像名(空不使用)
            memset(m_szScanImgBackName, 0x00, sizeof(m_szScanImgBackName));
            memcpy(m_szScanImgBackName, lpInit->szScanImgBackName, strlen(lpInit->szScanImgBackName));
            // 证件扫描图像保存类型
            m_wScanImgSaveType = lpInit->wScanImgSaveType;
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
            switch(lpInit->wScanImgSaveType)
            {
                case SAVE_IMG_BMP: sprintf(m_szScanImgSaveTypeS, "%s", SAVE_IMG_BMP_S); break;
                case SAVE_IMG_JPG: sprintf(m_szScanImgSaveTypeS, "%s", SAVE_IMG_JPG_S); break;
                case SAVE_IMG_BMP_X: sprintf(m_szScanImgSaveTypeS, "%s", SAVE_IMG_BMP_SX); break;
                case SAVE_IMG_JPG_X: sprintf(m_szScanImgSaveTypeS, "%s", SAVE_IMG_JPG_SX); break;
                default: sprintf(m_szScanImgSaveTypeS, "%s", SAVE_IMG_JPG_S); break;
            }
            // 证件扫描图像保存图片缩放比例
            m_fScanImgSaveZoomSc = lpInit->fScanImgSaveZoomSc;
            // 退卡动作模式(0保留在门口/1完全弹出)
            m_wEjectMode = lpInit->wEjectMode;
            // 证件扫描图像是否以人像信息面为正面
            m_wScanImageFrontIsInfor = lpInit->wScanImageFrontIsInfor;
            break;
        }
        case DATATYPE_ResetDevice:
        {
            if (m_pDevBSID81.bResetDevice() != TRUE)
            {
                Log(ThisModule, __LINE__, "设置数据: SetData()->bResetDevice() 设备复位 fail. Read Error. ");
                return lErrCodeChgToIdx(m_pDevBSID81.GetErrCode());
            }
            break;
        }
        case DATATYPE_SetDevIsNotOpen:
        {
            m_pDevBSID81.SetDevOpenIsF();
            break;
        }
        case DTYPE_LIB_PATH:    // 设置Lib路径
        {
            m_pDevBSID81.SetLibPath((LPCSTR)vData);
            break;
        }
        case DTYPE_LIB_VER:    // 设置Lib版本
        {
            m_pDevBSID81.SetLibVer(*(WORD*)vData);
            break;
        }
        default:
                break;
    }

    return ERR_IDX_SUCCESS;
}

// 获取数据
int CDevIDX_BSID81::GetData(void *vData, WORD wDataType)
{
    switch(wDataType)
    {
        default:
            break;
    }

    return ERR_IDX_SUCCESS;
}

// 获取版本号(1DevCam版本/2固件版本/3其他)
void CDevIDX_BSID81::GetVersion(LPSTR szVer, long lSize, ushort usType)
{
    if (usType == 1)
    {
        memcpy(szVer, byDevVRTU, strlen((char*)byDevVRTU) > lSize ? lSize : strlen((char*)byDevVRTU));
    }
    if (usType == 2)
    {
        m_pDevBSID81.bGetFWVersion(szVer);
    }
    if (usType == 3)
    {
        m_pDevBSID81.bGetSWVersion(szVer);
    }
}

// 分类型证件数据组合
BOOL CDevIDX_BSID81::vIDCardDataToTrack(LPVOID lpVoidData, LPSTR lpDestData, WORD wCardType)
{
    if (m_wBankNo == 1)   // 长沙银行
    {
        return vIDCardDataToTrack_CSBC(lpVoidData, lpDestData, wCardType);
    } else
    if (m_wBankNo == 2)   // 陕西信合                                                // 40-00-00-00(FT#0010)
    {                                                                               // 40-00-00-00(FT#0010)
        return vIDCardDataToTrack_SXXH(lpVoidData, lpDestData, wCardType);          // 40-00-00-00(FT#0010)
    }                                                                               // 40-00-00-00(FT#0010)

    if (wCardType == ID_CHINA)  // 国内证件
    {
        IDInfoEx *stInfoData = (IDInfoEx *)lpVoidData;
        sprintf(lpDestData,
                "Name=%s|Sex=%s|Nation=%s|Born=%s|IDCardNo=%s|Address=%s|GrantDept=%s|UserLifeBegin=%s|UserLifeEnd=%s|Finger1=",
                QString(stInfoData->name).trimmed().toStdString().c_str(),           //姓名
                QString(stInfoData->sex).trimmed().toStdString().c_str(),            //性别
                QString(stInfoData->nation).trimmed().toStdString().c_str(),         //民族
                QString(stInfoData->birthday).trimmed().toStdString().c_str(),       //出生日期
                QString(stInfoData->number).trimmed().toStdString().c_str(),         //身份证号码
                QString(stInfoData->address).trimmed().toStdString().c_str(),        //住址信息
                QString(stInfoData->department).trimmed().toStdString().c_str(),     //签发机关
                QString(stInfoData->timeLimit).left(QString(stInfoData->timeLimit).indexOf("-")).trimmed().toStdString().c_str(),   // 有效日期开始
                QString(stInfoData->timeLimit).right(   // 有效日期结束
                    QString(stInfoData->timeLimit).length() - QString(stInfoData->timeLimit).indexOf("-") - 1).trimmed().toStdString().c_str()
               );
        for (int i = 0; i < stInfoData->iFingerDataLen / 2; i ++)
        {
            sprintf(lpDestData + strlen(lpDestData), "%02x", (BYTE)(stInfoData->FingerData[i]));
        }
        sprintf(lpDestData + strlen(lpDestData), "|Finger2=");
        for (int i = (stInfoData->iFingerDataLen / 2); i < stInfoData->iFingerDataLen; i ++)
        {
            sprintf(lpDestData + strlen(lpDestData), "%02x", (BYTE)(stInfoData->FingerData[i]));
        }
        sprintf(lpDestData + strlen(lpDestData), "|PhotoFileName=%s|",
                QString(stInfoData->Image).toStdString().c_str());//头像信息文件
    } else
    if (wCardType == ID_FOREIGN)  // 国外证件
    {
        IDInfoForeign *stInfoData = (IDInfoForeign *)lpVoidData;
        sprintf(lpDestData,
                "FgnNameEN=%s|FgnName=%s|FgnSex=%s|FgnNation=%s|FgnBirthDay=%s|"
                "FgnCardId=%s|FgnIssAuth=%s|FgnStartDate=%s|FgnEndDate=%s|"
                "FgnCardVar=%s|FgnCardSign=%s|Finger1=%s|Finger2=%s|PhotoFileName=%s|",
                QString(stInfoData->NameENG).trimmed().toStdString().c_str(),        // 英文姓名
                QString(stInfoData->NameCHN).trimmed().toStdString().c_str(),        // 中文姓名
                QString(stInfoData->Sex).trimmed().toStdString().c_str(),            // 性别
                QString(stInfoData->Nation).trimmed().toStdString().c_str(),         // 国籍
                QString(stInfoData->Born).trimmed().toStdString().c_str(),           // 出生日期
                QString(stInfoData->IDCardNO).trimmed().toStdString().c_str(),       // 证件号码
                QString(stInfoData->Department).trimmed().toStdString().c_str(),     // 签发机关
                QString(stInfoData->TimeLimitBegin).trimmed().toStdString().c_str(), // 证件签发日期开始
                QString(stInfoData->TimeLimitEnd).trimmed().toStdString().c_str(),   // 证件签发日期结束
                QString(stInfoData->IDVersion).trimmed().toStdString().c_str(),      // 证件版本
                QString(stInfoData->IDType).trimmed().toStdString().c_str(),         // 证件类型
                //stInfoData->Reserve,      //保留信息
                "", "",                                                              // 指纹信息数据1,2
                QString(stInfoData->Image).trimmed().toStdString().c_str()          // 头像
               );
    } else
    if (wCardType == ID_GAT)  // 港澳台证件
    {
        IDInfoGAT *stInfoData = (IDInfoGAT *)lpVoidData;
        sprintf(lpDestData,
                "Name=%s|Sex=%s|Born=%s|IDCardNo=%s|Address=%s|GrantDept=%s|UserLifeBegin=%s|"
                "UserLifeEnd=20%%s|PermitNo=%s|IssueCount=%s|FgnCardSign=J|Finger1=",
                QString(stInfoData->name).trimmed().toStdString().c_str(),           // 姓名
                QString(stInfoData->sex).trimmed().toStdString().c_str(),            // 性别
                QString(stInfoData->birthday).trimmed().toStdString().c_str(),       // 出生日期
                QString(stInfoData->number).trimmed().toStdString().c_str(),         // 身份证号码
                QString(stInfoData->address).trimmed().toStdString().c_str(),        // 住址信息
                QString(stInfoData->department).trimmed().toStdString().c_str(),     // 签发机关
                QString(stInfoData->timeLimit).left(QString(stInfoData->timeLimit).indexOf("-")).trimmed().toStdString().c_str(),   // 有效日期开始
                QString(stInfoData->timeLimit).right(                               // 有效日期结束
                    QString(stInfoData->timeLimit).length() - QString(stInfoData->timeLimit).indexOf("-") - 1).trimmed().toStdString().c_str(),
                QString(stInfoData->passport).trimmed().toStdString().c_str(),       // 通行证号码
                QString(stInfoData->issue).trimmed().toStdString().c_str(),          // 签发次数
                "J"                                                                  // 证件类型标识
               );
        for (int i = 0; i < stInfoData->iFingerDataLen / 2; i ++)
        {
            sprintf(lpDestData + strlen(lpDestData), "%02x", (BYTE)(stInfoData->FingerData[i]));
        }
        sprintf(lpDestData + strlen(lpDestData), "|Finger2=");
        for (int i = (stInfoData->iFingerDataLen / 2); i < stInfoData->iFingerDataLen; i ++)
        {
            sprintf(lpDestData + strlen(lpDestData), "%02x", (BYTE)(stInfoData->FingerData[i]));
        }
        sprintf(lpDestData + strlen(lpDestData), "|PhotoFileName=%s|",
                QString(stInfoData->Image).toStdString().c_str());//头像信息文件
    } else
    {
        return FALSE;
    }

    return TRUE;
}

// 分类型证件数据组合
BOOL CDevIDX_BSID81::vIDCardDataToTrack_CSBC(LPVOID lpVoidData, LPSTR lpDestData, WORD wCardType)
{
    if (wCardType == ID_CHINA)  // 国内证件
    {
        IDInfoEx *stInfoData = (IDInfoEx *)lpVoidData;
        sprintf(lpDestData,
                "IDType=0|Name=%s|Sex=%s|Nation=%s|Born=%s|Address=%s|IDCardNo=%s|GrantDept=%s|UserLifeBegin=%s|UserLifeEnd=%s|PhotoFileName=%s|",
                QString(stInfoData->name).trimmed().toStdString().c_str(),           //姓名
                QString(stInfoData->sex).trimmed().toStdString().c_str(),            //性别
                QString(stInfoData->nation).trimmed().toStdString().c_str(),         //民族
                QString(stInfoData->birthday).trimmed().toStdString().c_str(),       //出生日期
                QString(stInfoData->address).trimmed().toStdString().c_str(),        //住址信息
                QString(stInfoData->number).trimmed().toStdString().c_str(),         //身份证号码
                QString(stInfoData->department).trimmed().toStdString().c_str(),     //签发机关
                QString(stInfoData->timeLimit).left(QString(stInfoData->timeLimit).indexOf("-")).trimmed().toStdString().c_str(),   // 有效日期开始
                QString(stInfoData->timeLimit).right(   // 有效日期结束
                    QString(stInfoData->timeLimit).length() - QString(stInfoData->timeLimit).indexOf("-") - 1).trimmed().toStdString().c_str(),
               QString(stInfoData->Image).toStdString().c_str());//头像信息文件
    } else
    if (wCardType == ID_FOREIGN)  // 国外证件
    {
        IDInfoForeign *stInfoData = (IDInfoForeign *)lpVoidData;
        sprintf(lpDestData,
                "IDType=1|EnglishName=%s|Sex=%s|IDCardNO=%s|Nationality=%s|ChineseName=%s|"
                "UserLiftBegin=%s|UserLiftEnd=%s|Born=%s|VersionNumber=%s|"
                "OrganizationCode=%s|ReserveItem=%s|PhotoFileName=%s|Finger2=%s|PhotoFileName=%s",
                QString(stInfoData->NameENG).trimmed().toStdString().c_str(),        // 英文姓名
                QString(stInfoData->Sex).trimmed().toStdString().c_str(),            // 性别
                QString(stInfoData->IDCardNO).trimmed().toStdString().c_str(),       // 证件号码
                QString(stInfoData->Nation).trimmed().toStdString().c_str(),         // 国籍
                QString(stInfoData->NameCHN).trimmed().toStdString().c_str(),        // 中文姓名
                QString(stInfoData->TimeLimitBegin).trimmed().toStdString().c_str(), // 证件签发日期开始
                QString(stInfoData->TimeLimitEnd).trimmed().toStdString().c_str(),   // 证件签发日期结束
                QString(stInfoData->Born).trimmed().toStdString().c_str(),           // 出生日期
                QString(stInfoData->IDVersion).trimmed().toStdString().c_str(),      // 证件版本
                QString(stInfoData->Department).trimmed().toStdString().c_str(),     // 签发机关
                QString(stInfoData->Reserve).trimmed().toStdString().c_str(),        // 保留信息
                //QString(stInfoData->IDType).trimmed().toStdString().c_str(),         // 证件类型
                QString(stInfoData->Image).trimmed().toStdString().c_str()           // 头像
               );
    } else
    if (wCardType == ID_GAT)  // 港澳台证件
    {
        IDInfoGAT *stInfoData = (IDInfoGAT *)lpVoidData;
        sprintf(lpDestData,
                "IDType=2|Name=%s|Sex=%s|Born=%s|Address=%s|IDCardNo=%s|GrantDept=%s|UserLifeBegin=%s|"
                "UserLifeEnd=20%%s|PassNO=%s|IssueNumber=%s|FingerDataOne=",
                QString(stInfoData->name).trimmed().toStdString().c_str(),           // 姓名
                QString(stInfoData->sex).trimmed().toStdString().c_str(),            // 性别
                QString(stInfoData->birthday).trimmed().toStdString().c_str(),       // 出生日期
                QString(stInfoData->address).trimmed().toStdString().c_str(),        // 住址信息
                QString(stInfoData->number).trimmed().toStdString().c_str(),         // 身份证号码
                QString(stInfoData->department).trimmed().toStdString().c_str(),     // 签发机关
                QString(stInfoData->timeLimit).left(QString(stInfoData->timeLimit).indexOf("-")).trimmed().toStdString().c_str(),   // 有效日期开始
                QString(stInfoData->timeLimit).right(                               // 有效日期结束
                    QString(stInfoData->timeLimit).length() - QString(stInfoData->timeLimit).indexOf("-") - 1).trimmed().toStdString().c_str(),
                QString(stInfoData->passport).trimmed().toStdString().c_str(),       // 通行证号码
                QString(stInfoData->issue).trimmed().toStdString().c_str()           // 签发次数
               );
        for (int i = 0; i < stInfoData->iFingerDataLen / 2; i ++)
        {
            sprintf(lpDestData + strlen(lpDestData), "%02x", (BYTE)(stInfoData->FingerData[i]));
        }
        sprintf(lpDestData + strlen(lpDestData), "|FingerDataTwo=");
        for (int i = (stInfoData->iFingerDataLen / 2); i < stInfoData->iFingerDataLen; i ++)
        {
            sprintf(lpDestData + strlen(lpDestData), "%02x", (BYTE)(stInfoData->FingerData[i]));
        }
        sprintf(lpDestData + strlen(lpDestData), "|ICSerialNumber=|CardSerialNumber=");
    } else
    {
        return FALSE;
    }

    return TRUE;
}

// 分类型证件数据组合(陕西信合) // 40-00-00-00(FT#0010) ADD
BOOL CDevIDX_BSID81::vIDCardDataToTrack_SXXH(LPVOID lpVoidData, LPSTR lpDestData, WORD wCardType)
{
    if (wCardType == ID_CHINA)  // 国内证件
    {
        IDInfoEx *stInfoData = (IDInfoEx *)lpVoidData;
        sprintf(lpDestData,
                "IDType=0|Name=%s|Sex=%s|Nation=%s|Born=%s|Address=%s|IDCardNo=%s|GrantDept=%s|UserLifeBegin=%s|UserLifeEnd=%s|PhotoFileName=%s"
                "FingerDataOne=",
                QString(stInfoData->name).trimmed().toStdString().c_str(),           //姓名
                QString(stInfoData->sex).trimmed().toStdString().c_str(),            //性别
                QString(stInfoData->nation).trimmed().toStdString().c_str(),         //民族
                QString(stInfoData->birthday).trimmed().toStdString().c_str(),       //出生日期
                QString(stInfoData->address).trimmed().toStdString().c_str(),        //住址信息
                QString(stInfoData->number).trimmed().toStdString().c_str(),         //身份证号码
                QString(stInfoData->department).trimmed().toStdString().c_str(),     //签发机关
                QString(stInfoData->timeLimit).left(QString(stInfoData->timeLimit).indexOf("-")).trimmed().toStdString().c_str(),   // 有效日期开始
                QString(stInfoData->timeLimit).right(   // 有效日期结束
                    QString(stInfoData->timeLimit).length() - QString(stInfoData->timeLimit).indexOf("-") - 1).trimmed().toStdString().c_str(),
               QString(stInfoData->Image).toStdString().c_str());//头像信息文件
        for (int i = 0; i < stInfoData->iFingerDataLen / 2; i ++)
        {
            sprintf(lpDestData + strlen(lpDestData), "%02x", (BYTE)(stInfoData->FingerData[i]));
        }
        sprintf(lpDestData + strlen(lpDestData), "|FingerDataTwo=");
        for (int i = (stInfoData->iFingerDataLen / 2); i < stInfoData->iFingerDataLen; i ++)
        {
            sprintf(lpDestData + strlen(lpDestData), "%02x", (BYTE)(stInfoData->FingerData[i]));
        }
    } else
    if (wCardType == ID_FOREIGN)  // 国外证件
    {
        IDInfoForeign *stInfoData = (IDInfoForeign *)lpVoidData;
        sprintf(lpDestData,
                "IDType=1|EnglishName=%s|Sex=%s|IDCardNO=%s|Nationality=%s|ChineseName=%s|"
                "UserLiftBegin=%s|UserLiftEnd=%s|Born=%s|VersionNumber=%s|"
                "OrganizationCode=%s|ReserveItem=%s|PhotoFileName=%s|Finger2=%s|PhotoFileName=%s",
                QString(stInfoData->NameENG).trimmed().toStdString().c_str(),        // 英文姓名
                QString(stInfoData->Sex).trimmed().toStdString().c_str(),            // 性别
                QString(stInfoData->IDCardNO).trimmed().toStdString().c_str(),       // 证件号码
                QString(stInfoData->Nation).trimmed().toStdString().c_str(),         // 国籍
                QString(stInfoData->NameCHN).trimmed().toStdString().c_str(),        // 中文姓名
                QString(stInfoData->TimeLimitBegin).trimmed().toStdString().c_str(), // 证件签发日期开始
                QString(stInfoData->TimeLimitEnd).trimmed().toStdString().c_str(),   // 证件签发日期结束
                QString(stInfoData->Born).trimmed().toStdString().c_str(),           // 出生日期
                QString(stInfoData->IDVersion).trimmed().toStdString().c_str(),      // 证件版本
                QString(stInfoData->Department).trimmed().toStdString().c_str(),     // 签发机关
                QString(stInfoData->Reserve).trimmed().toStdString().c_str(),        // 保留信息
                //QString(stInfoData->IDType).trimmed().toStdString().c_str(),         // 证件类型
                QString(stInfoData->Image).trimmed().toStdString().c_str()           // 头像
               );
    } else
    if (wCardType == ID_GAT)  // 港澳台证件
    {
        IDInfoGAT *stInfoData = (IDInfoGAT *)lpVoidData;
        sprintf(lpDestData,
                "IDType=2|Name=%s|Sex=%s|Born=%s|Address=%s|IDCardNo=%s|GrantDept=%s|UserLifeBegin=%s|"
                "UserLifeEnd=20%%s|PassNO=%s|IssueNumber=%s|FingerDataOne=",
                QString(stInfoData->name).trimmed().toStdString().c_str(),           // 姓名
                QString(stInfoData->sex).trimmed().toStdString().c_str(),            // 性别
                QString(stInfoData->birthday).trimmed().toStdString().c_str(),       // 出生日期
                QString(stInfoData->address).trimmed().toStdString().c_str(),        // 住址信息
                QString(stInfoData->number).trimmed().toStdString().c_str(),         // 身份证号码
                QString(stInfoData->department).trimmed().toStdString().c_str(),     // 签发机关
                QString(stInfoData->timeLimit).left(QString(stInfoData->timeLimit).indexOf("-")).trimmed().toStdString().c_str(),   // 有效日期开始
                QString(stInfoData->timeLimit).right(                               // 有效日期结束
                    QString(stInfoData->timeLimit).length() - QString(stInfoData->timeLimit).indexOf("-") - 1).trimmed().toStdString().c_str(),
                QString(stInfoData->passport).trimmed().toStdString().c_str(),       // 通行证号码
                QString(stInfoData->issue).trimmed().toStdString().c_str()           // 签发次数
               );
    } else
    {
        return FALSE;
    }

    return TRUE;
}

// BSID81错误码转换为DevIDX错误码
LONG CDevIDX_BSID81::lErrCodeChgToIdx(LONG lDevCode)
{
    switch(lDevCode)
    {
        case IDDIGITALCOPIER_NO_ERROR:                      // 正常
            return ERR_IDX_SUCCESS;                             // 操作成功
        case IDDIGITALCOPIER_NO_DEVICE:                     // 无设备
            return ERR_IDX_NO_DEVICE;
        case IDDIGITALCOPIER_PORT_ERROR:                    // 端口错误
            return ERR_IDX_COMM_ERR;
        case IDDIGITALCOPIER_TABPAR_NONE:                   // 参数文件错误
            return ERR_IDX_PARAM_ERR;
        case IDDIGITALCOPIER_HAVE_NOT_INIT:                 // 未初始化
            return ERR_IDX_OTHER;
        case IDDIGITALCOPIER_INVALID_ARGUMENT:              // 无效参数
            return ERR_IDX_PARAM_ERR;
        case IDDIGITALCOPIER_TIMEOUT_ERROR:                 // 超时错误
            return ERR_IDX_OTHER;
        case IDDIGITALCOPIER_STATUS_COVER_OPENED:           // 上盖打开
            return ERR_IDX_HWERR;
        case IDDIGITALCOPIER_STATUS_PASSAGE_JAM:            // 塞卡
            return ERR_IDX_JAMMED;
        case IDDIGITALCOPIER_OUT_OF_MEMORY:                 // 内存溢出
            return ERR_IDX_OTHER;
        case IDDIGITALCOPIER_NO_ID_DATA:                    // 没有二代证数据
            return ERR_IDX_INVALIDCARD;
        case IDDIGITALCOPIER_NO_IMAGE_DATA:                 // 没有图像数据
            return ERR_IDX_INVALIDCARD;
        case IDDIGITALCOPIER_IMAGE_PROCESS_ERROR:           // 图像处理错误
            return ERR_IDX_READERROR;
        case IDDIGITALCOPIER_IMAGE_JUDGE_DIRECTION_ERROR:   // 判断图像方向错误
            return ERR_IDX_READERROR;
        case IDDIGITALCOPIER_CLOSE_FAILED:                  // 关闭端口失败
            return ERR_IDX_COMM_ERR;
        case IDDIGITALCOPIER_IDDATA_PROCESS_ERROR:          // 身份证电子信息处理错误
            return ERR_IDX_READERROR;
        case IDDIGITALCOPIER_SENSORVALIDATE:                // 传感器校验错误
            return ERR_IDX_HWERR;
        case IDDIGITALCOPIER_VOLTAGE_LOW:                   // 电压低
            return ERR_IDX_HWERR;
        case IDDIGITALCOPIER_CIS_CORRECTION_ERROR:          // 校正错误
            return ERR_IDX_HWERR;
        case IDDIGITALCOPIER_NO_CARD:                       // 无卡
            return ERR_IDX_INSERT_TIMEOUT;
        case IDDIGITALCOPIER_FIRMWARE_ERROR:                // 未知错误
            return ERR_IDX_OTHER;
        case IDDIGITALCOPIER_SAVE_IMAGE_ERROR:              // 保存位图错误
            return ERR_IDX_READERROR;
        case IDDIGITALCOPIER_POWER_OFF:                     // 掉电错误
            return ERR_IDX_HWERR;
        case IDDIGITALCOPIER_INPUT_BOOT:                    // BOOT错误
            return ERR_IDX_HWERR;
        case IDDIGITALCOPIER_BUTTON_UP:                     // 按键抬起
            return ERR_IDX_OTHER;
        case IDDIGITALCOPIER_RECOGNISE_FAILED:              // 识别错误
            return ERR_IDX_HWERR;
        case IDDIGITALCOPIER_SCAN_ERROR:                    // 扫描错误
            return ERR_IDX_READERROR;
        case IDDIGITALCOPIER_FEED_ERROR:                    // 走卡错误
            return ERR_IDX_HWERR;
        case IDDIGITALCOPIER_MAX_CODE:                      // 最大错误码
            return ERR_IDX_OTHER;
        default :
            return ERR_IDX_SUCCESS;
    }
}

//////////////////////////////////////////////////////////////////////////






