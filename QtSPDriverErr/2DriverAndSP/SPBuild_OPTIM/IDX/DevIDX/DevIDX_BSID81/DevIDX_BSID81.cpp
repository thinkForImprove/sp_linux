/***************************************************************
* 文件名称: DevIDX_BSID81.cpp
* 文件描述: 身份证模块功能处理接口封装
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2023年3月25日
* 文件版本: 1.0.0.1
****************************************************************/

#include "DevIDX_BSID81.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>
#include <QSettings>
#include <sys/stat.h>

#include <dirent.h>

static const char *ThisFile = "DevIDX_BSID81.cpp";

/****************************************************************************
*     对外接口调用处理                                                         *
****************************************************************************/
CDevIDX_BSID81::CDevIDX_BSID81() : m_pDevImpl(LOG_NAME_DEV)
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

CDevIDX_BSID81::~CDevIDX_BSID81()
{
    m_pDevImpl.CloseDevice();
}

// 释放接口
int CDevIDX_BSID81::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    m_pDevImpl.CloseDevice();
    return IDC_SUCCESS;
}

// 打开与设备的连接
int CDevIDX_BSID81::Open(LPCSTR lpMode)
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
int CDevIDX_BSID81::Close()
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_pDevImpl.CloseDevice();
    return IDC_SUCCESS;
}

// 取消
int CDevIDX_BSID81::Cancel(unsigned short usMode)
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
int CDevIDX_BSID81::Reset(MEDIA_ACTION enMediaAct, unsigned short usParam)
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

    nRet = m_pDevImpl.nSoftResetDevice(nAct);
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
int CDevIDX_BSID81::GetStatus(STDEVIDCSTATUS &stStatus)
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
        stStatus.wChipPower = CHIP_STAT_UNKNOWN;
        return ERR_IDC_DEV_NOTOPEN;
    }

    // 获取设备状态
    nRet = m_pDevImpl.nGetDevStatus(m_stDevStatus);
    if (nRet != IMP_SUCCESS)
    {
        return ConvertCode_Impl2IDC(nRet);
    }

    // ----------------Check 状态----------------
    // Device POWEROFF 断电
    if (m_stDevStatus.iStatusPowerOff == 1)     // 掉电
    {
        stStatus.wDevice = DEVICE_STAT_POWEROFF;            // 设备故障
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;               // 卡的状态未知
    } else
    // Device HWERR
    if (m_stDevStatus.iStatusCoverOpen == 1 ||  // 上盖打开
        m_stDevStatus.iStatusLowVoltage == 1 || // 低电压错误
        m_stDevStatus.iStatusProcess == 1)      // 过程出错
    {
        stStatus.wDevice = DEVICE_STAT_HWERROR;             // 设备故障
        stStatus.wMedia = MEDIA_STAT_UNKNOWN;               // 卡的状态未知
    } else
    // Device Busy
    if (m_stDevStatus.iStatusProcess == 2)      // 执行过程中
    {
        stStatus.wDevice = DEVICE_STAT_BUSY;                // 设备忙
    } else
    {
        stStatus.wDevice = DEVICE_STAT_ONLINE;              // 设备正常
    }

    // Media JAM
    if (m_stDevStatus.iStatusCardJam == 1)      // JAM
    {
        stStatus.wDevice = DEVICE_STAT_HWERROR;
        stStatus.wMedia = MEDIA_STAT_JAMMED;
    }

    // Media status
    if (stStatus.wDevice == DEVICE_STAT_BUSY || stStatus.wDevice == DEVICE_STAT_ONLINE)
    {
        stStatus.wOtherCode[0] = 0;  // 其他错误:[0]:该模块作为扫描位置有无卡标记
        if (m_stDevStatus.iStatusMiddleSensorHaveCard == 0 &&// 中间传感器无卡
            m_stDevStatus.iStatusScanSensorHaveCard == 0 && // 扫描传感器无卡
            m_stDevStatus.iStatusInputSensorHaveCard == 0)  // 入口传感器无卡
        {
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
        } else
        {
            if (m_stDevStatus.iStatusMiddleSensorHaveCard == 1 ||// 中间传感器有卡
                m_stDevStatus.iStatusScanSensorHaveCard == 1)   // 扫描传感器有卡
            {
                stStatus.wMedia = MEDIA_STAT_PRESENT;    // 介质在通道内
            }

            if (m_stDevStatus.iStatusScanSensorHaveCard == 1)   // 扫描传感器有卡
            {
                stStatus.wOtherCode[0] = 1;
            }

            if (m_stDevStatus.iStatusInputSensorHaveCard == 1)// 入口传感器有卡
            {
                stStatus.wMedia = MEDIA_STAT_ENTERING;    // 介质位于的入卡口位置
            }
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
int CDevIDX_BSID81::MediaControl(MEDIA_ACTION enMediaAct, unsigned long ulParam)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IDC_SUCCESS;
    if(enMediaAct == MEDIA_EJECT)       // 介质退出
    {
        return nEjectCard((DWORD)ulParam);
    } else
    if(enMediaAct == MEDIA_RETRACT)     // 介质回收
    {
        nRet = m_pDevImpl.nRetainIdCard();
        if(nRet != IDC_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "介质控制: 回收: ->nRetainIdCard() Fail, ErrCode: %d, Return: %s",
                nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
            return ConvertCode_Impl2IDC(nRet);
        }
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

// 介质读写
int CDevIDX_BSID81::MediaReadWrite(MEDIA_RW_MODE enRWMode, STMEDIARW &stMediaData)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    char cImgScanFront[256 * 2] = { 0x00 }; // 正面扫描图像路径名
    char cImgScanBack[256 * 2] = { 0x00 };  // 背面扫描图像路径名
    char cImgHeadFile[256 * 2] = { 0x00 };  // 头像图像路径名
    char szIDCardNo[256] = { 0x00 };

    if (enRWMode == MEDIA_READ) // 读
    {
        if ((stMediaData.dwRWType & RW_TRACK1)  == RW_TRACK1)   // 正面扫描
        {
            if (strlen(m_szHeadImgName) > 0)   // 指定证件头像图像名
            {
                sprintf(cImgHeadFile, "%s/%s", m_szHeadImgSavePath, m_szHeadImgName);
            } else
            {
                sprintf(cImgHeadFile, "%s", m_szHeadImgSavePath);
            }

            // 获取卡类型
            INT nCardType = 0;
            nRet = m_pDevImpl.nGetIDCardType(&nCardType);
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "读卡: 获取卡类型: ->nGetIDCardType() Fail, ErrCode: %d, Return: %s.",
                    nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                return ConvertCode_Impl2IDC(nRet);
            }
            Log(ThisModule, __LINE__, "读卡: ->bGetIDCardType() 获取卡类型 succ. Is %s.",
                nCardType == ID_CHINA ? "国内证件" : (nCardType == ID_FOREIGN? "国外证件" : "港澳台证件"));

            // 读卡数据
            if (nCardType == ID_CHINA)  // 国内证件
            {
                IDInfoEx stInfoData;
                memset(&stInfoData, 0x00, sizeof(IDInfoEx));
                nRet = m_pDevImpl.nGetID2InfoEx(stInfoData, (LPSTR)cImgHeadFile);
                if (nRet != IMP_SUCCESS)
                {
                    stMediaData.stData[0].wResult = RW_RESULT_MISS;
                    Log(ThisModule, __LINE__,
                        "读卡:  国内证件读: ->nGetID2InfoEx() Fail, ErrCode: %d, Return: %s.",
                        nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                    return ConvertCode_Impl2IDC(nRet);
                }

                // 组织证件数据写入应答
                stMediaData.stData[0].wResult =
                        vIDCardDataToTrack(&stInfoData, stMediaData.stData[0].szData, nCardType);
                stMediaData.stData[0].wSize = strlen(stMediaData.stData[0].szData);

                // 扫描图像路径(INI指定/证件号码_front/back.)
                sprintf(szIDCardNo, "%s", QString(stInfoData.number).trimmed().toStdString().c_str());
            } else
            if (nCardType == ID_FOREIGN)  // 国外证件
            {
                IDInfoForeign stInfoData;
                memset(&stInfoData, 0x00, sizeof(IDInfoEx));
                nRet = m_pDevImpl.nGetIDInfoForeign(stInfoData, (LPSTR)cImgHeadFile);
                if (nRet != IMP_SUCCESS)
                {
                    stMediaData.stData[0].wResult = RW_RESULT_MISS;
                    Log(ThisModule, __LINE__,
                        "读卡:  国外证件读: ->nGetIDInfoForeign() Fail, ErrCode: %d, Return: %s.",
                        nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                    return ConvertCode_Impl2IDC(nRet);
                }

                // 组织证件数据写入应答
                stMediaData.stData[0].wResult =
                        vIDCardDataToTrack(&stInfoData, stMediaData.stData[0].szData, nCardType);
                stMediaData.stData[0].wSize = strlen(stMediaData.stData[0].szData);

                // 扫描图像路径(INI指定/证件号码_front/back.)
                sprintf(szIDCardNo, "%s", QString(stInfoData.IDCardNO).trimmed().toStdString().c_str());
            } else
            if (nCardType == ID_GAT)  // 港澳台证件
            {
                IDInfoGAT stInfoData;
                memset(&stInfoData, 0x00, sizeof(IDInfoEx));
                nRet = m_pDevImpl.nGetIDInfoGAT(stInfoData, (LPSTR)cImgHeadFile);
                if (nRet != IMP_SUCCESS)
                {
                    stMediaData.stData[0].wResult = RW_RESULT_MISS;
                    Log(ThisModule, __LINE__,
                        "读卡:  港澳台证件读: ->nGetIDInfoGAT() Fail, ErrCode: %d, Return: %s.",
                        nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                    return ConvertCode_Impl2IDC(nRet);
                }

                // 组织证件数据写入应答
                stMediaData.stData[0].wResult =
                        vIDCardDataToTrack(&stInfoData, stMediaData.stData[0].szData, nCardType);
                stMediaData.stData[0].wSize = strlen(stMediaData.stData[0].szData);

                // 扫描图像路径(INI指定/证件号码_front/back.jpg)
                sprintf(szIDCardNo, "%s", QString(stInfoData.number).trimmed().toStdString().c_str());
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
            // 扫描图像路径(INI指定/证件号码_front/back.)
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

            // 启动扫描
            nRet = m_pDevImpl.nStartScanIdCard();
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "读卡: 启动证件扫描: ->nStartScanIdCard() Fail, ErrCode: %d, Return: %s.",
                    nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                return ConvertCode_Impl2IDC(nRet);
            }

            memset(m_szScanImgFrontBuff, 0x00, sizeof(m_szScanImgFrontBuff));
            memset(m_szScanImgBackBuff, 0x00, sizeof(m_szScanImgBackBuff));
            m_nScanImgFrontBuffLen = 0;
            m_nScanImgBackBuffLen = 0;

            // 读取当前图像数据块到内存(Front:人像面/Back:名称面)
            nRet = m_pDevImpl.nSavePicToMemory(m_szScanImgFrontBuff, m_szScanImgBackBuff,
                                               &m_nScanImgFrontBuffLen, &m_nScanImgBackBuffLen);
            if (nRet != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "读卡: 扫描证件正反图像数据块到内存: nSavePicToMemory() Fail, ErrCode: %d, Return: %s.",
                    nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                return ConvertCode_Impl2IDC(nRet);
            }

            if ((stMediaData.dwRWType & RW_TRACK2)  == RW_TRACK2)   // 扫描正面图像
            {                                                                                                   // 40-00-00-00(FT#0012)
                if (m_wScanImageFrontIsInfor == 1)    // 证件扫描图像以人像信息面为正面(Front)
                {
                    // 保存图像数据块到文件(指定缩放比例)_正面扫描:人像面
                    nRet = m_pDevImpl.nSavePicToFileII(m_szScanImgFrontBuff, m_nScanImgFrontBuffLen,
                                                       cImgScanFront, m_wScanImgSaveType, m_fScanImgSaveZoomSc);
                    if (nRet != IMP_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "读卡: 保存正面扫描图像(人像面)数据块到文件: bSavePicToFileII(%s, %d, %f), ErrCode: %d, Return: %s.",
                            cImgScanFront, m_wScanImgSaveType, m_fScanImgSaveZoomSc,
                            nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                        stMediaData.stData[1].wResult = RW_RESULT_MISS;
                        return ConvertCode_Impl2IDC(nRet);
                    }
                } else
                {
                    // 保存图像数据块到文件(指定缩放比例)_正面扫描
                    nRet = m_pDevImpl.nSavePicToFileII(m_szScanImgBackBuff, m_nScanImgBackBuffLen,
                                                       cImgScanFront, m_wScanImgSaveType, m_fScanImgSaveZoomSc);
                    if (nRet != IMP_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "读卡: 保存正面扫描图像(名称面)数据块到文件: bSavePicToFileII(%s, %d, %f), ErrCode: %d, Return: %s.",
                            cImgScanFront, m_wScanImgSaveType, m_fScanImgSaveZoomSc,
                            nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                        stMediaData.stData[1].wResult = RW_RESULT_MISS;
                        return ConvertCode_Impl2IDC(nRet);
                    }
                }

                // 组织证件数据写入应答
                stMediaData.stData[1].wResult = RW_RESULT_SUCC;
                memcpy(stMediaData.stData[1].szData, cImgScanFront, strlen(cImgScanFront));
                stMediaData.stData[1].wSize = strlen(cImgScanFront);
            }                                                                                                   // 40-00-00-00(FT#0012)

            if ((stMediaData.dwRWType & RW_TRACK3) == RW_TRACK3)   // 扫描背面图像
            {                                                                                                   // 40-00-00-00(FT#0012)
                if (m_wScanImageFrontIsInfor == 1)    // 证件扫描图像以人像信息面为正面(Front),名称面为反面(Back)
                {
                    // 保存图像数据块到文件(指定缩放比例)_背面扫描:名称面
                    nRet = m_pDevImpl.nSavePicToFileII(m_szScanImgBackBuff, m_nScanImgBackBuffLen,
                                                       cImgScanBack, m_wScanImgSaveType, m_fScanImgSaveZoomSc);
                    if (nRet != IMP_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "读卡: 保存背面扫描图像(名称面)数据块到文件: bSavePicToFileII(%s, %d, %f), ErrCode: %d, Return: %s.",
                            cImgScanBack, m_wScanImgSaveType, m_fScanImgSaveZoomSc,
                            nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                        stMediaData.stData[2].wResult = RW_RESULT_MISS;
                        return ConvertCode_Impl2IDC(nRet);
                    }
                } else
                {
                    // 保存图像数据块到文件(指定缩放比例)_背面扫描
                    nRet = m_pDevImpl.nSavePicToFileII(m_szScanImgFrontBuff, m_nScanImgFrontBuffLen,
                                                       cImgScanBack, m_wScanImgSaveType, m_fScanImgSaveZoomSc);
                    if (nRet != IMP_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "读卡: 保存背面扫描图像(人像面)数据块到文件: bSavePicToFileII(%s, %d, %f), ErrCode: %d, Return: %s.",
                            cImgScanBack, m_wScanImgSaveType, m_fScanImgSaveZoomSc,
                            nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                        stMediaData.stData[2].wResult = RW_RESULT_MISS;
                        return ConvertCode_Impl2IDC(nRet);
                    }
                }

                // 组织证件数据写入应答
                stMediaData.stData[2].wResult = RW_RESULT_SUCC;
                memcpy(stMediaData.stData[2].szData, cImgScanFront, strlen(cImgScanFront));
                stMediaData.stData[2].wSize = strlen(cImgScanFront);
            }                                                                                                   // 40-00-00-00(FT#0012)
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
int CDevIDX_BSID81::SetData(unsigned short usType, void *vData)
{
    THISMODULE(__FUNCTION__);

    switch(usType)
    {
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
        case SET_LIB_VER:    // 设置Lib版本
        {
            if (vData != nullptr)
            {
                m_pDevImpl.SetLibVer(*(WORD*)vData);
            }
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
int CDevIDX_BSID81::GetData(unsigned short usType, void *vData)
{
    //THISMODULE(__FUNCTION__);

    return IDC_SUCCESS;
}

// 获取版本
int CDevIDX_BSID81::GetVersion(unsigned short usType, char* szVer, int nSize)
{
    THISMODULE(__FUNCTION__);

    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_FW:        // 固件版本号
                m_pDevImpl.nGetFWVersion(szVer);
                break;
            case GET_VER_SOFT:      // 软件版本号
                m_pDevImpl.nGetSWVersion(szVer);
                break;
            default:
                break;
        }
    }

    return IDC_SUCCESS;
}

// 退卡处理
// dwParam: 0:退卡到出口, 非0:完全弹出
INT CDevIDX_BSID81::nEjectCard(DWORD dwParam)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // 检查后部扫描位置是否有卡存在，有则移动卡到可识别位置。
    // 目的：当卡处于扫描器位置时，BackIdCardToRerec无法退卡
    // 该部分不进行错误处理
    STDEVIDCSTATUS stDevStat;
    if (GetStatus(stDevStat) == IMP_SUCCESS)
    {
        if (stDevStat.wOtherCode[0] == 1)  // 扫描位置有卡,退卡到可识别位置(失败后执行3次不报错)
        {
            if ((nRet = m_pDevImpl.nBackIdCardToRerec()) != IMP_SUCCESS)
            {
                if ((nRet = m_pDevImpl.nBackIdCardToRerec()) != IMP_SUCCESS)
                {
                    if ((nRet = m_pDevImpl.nBackIdCardToRerec()) != IMP_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "退卡: ->bCheckHaveIdCard() 扫描位置有卡, 退卡到可识别位置 fail, ErrCode: %s, Not Return Error.",
                            ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                    }
                }
            }
        }
    }

    if (dwParam == 0)  // 退卡到出口
    {
        nRet = m_pDevImpl.nBackAndHoldIdCard();
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "退卡到出口: ->nBackAndHoldIdCard() Fail, ErrCode: %d, Return: %s",
                nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
            return ConvertCode_Impl2IDC(nRet);
        }
    } else  // 完全弹出
    {
        nRet = m_pDevImpl.nBackIdCard();
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

// 等待放卡处理
INT CDevIDX_BSID81::AcceptMedia(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    m_bCancelReadCard = FALSE;

    INT nRet = IMP_SUCCESS;

    // 检查后部扫描位置是否有卡存在，有则移动卡到可识别位置。
    // 目的：当卡处于扫描器位置时，CheckHaveIdCard检测不到卡存在
    // 该部分不进行错误处理
    STDEVIDCSTATUS stDevStat;
    if (GetStatus(stDevStat) == IMP_SUCCESS)
    {
        if (stDevStat.wOtherCode[0] == 1)  // 扫描位置有卡,退卡到可识别位置(失败后执行3次不报错)
        {
            if ((nRet = m_pDevImpl.nBackIdCardToRerec()) != IMP_SUCCESS)
            {
                if ((nRet = m_pDevImpl.nBackIdCardToRerec()) != IMP_SUCCESS)
                {
                    if ((nRet = m_pDevImpl.nBackIdCardToRerec()) != IMP_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "进卡: ->bCheckHaveIdCard() 扫描位置有卡, 退卡到可识别位置 fail, ErrCode: %s, Not Return Error.",
                            ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                    }
                }
            }
        }
    }

    ULONG ulTimeCount = 0;
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

        // 执行吸卡
        nRet = m_pDevImpl.nCheckHaveIdCard(0);        
        INT nRetStat = GetStatus(stDevStat);        // 执行吸卡后取状态
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "进卡: ->nCheckHaveIdCard(0) Fail, ErrCode: %d.", nRet);

            // 吸卡失败后进行状态检查
            if (nRetStat != IMP_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "进卡失败: ->GetStatus() 检查放入卡, 获取状态失败, ErrCode: %d, Return: %s",
                    nRet, ConvertDevErrCodeToStr(ConvertCode_Impl2IDC(nRet)));
                return ConvertCode_Impl2IDC(nRet);
            } else
            {
                if (stDevStat.wMedia == MEDIA_STAT_JAMMED)
                {
                    Log(ThisModule, __LINE__,
                        "进卡失败: ->GetStatus() 检查放入卡, 获取状态:Media=[%d|JAM]，Return: %s.",
                        stDevStat.wMedia, ConvertDevErrCodeToStr(ERR_IDC_MED_JAMMED));
                    return ERR_IDC_MED_JAMMED;
                }
            }
        } else
        {
            // 吸卡成功后进行状态检查
            if (stDevStat.wMedia == MEDIA_STAT_PRESENT)
            {
                break;
            }
        }

        continue;
    }

    return IDC_SUCCESS;
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
INT CDevIDX_BSID81::ConvertCode_Impl2IDC(INT nRetCode)
{
#define BSID81_CASE_CODE_IMP2DEV(IMP, DEV) \
    case IMP: return DEV;

    switch(nRetCode)
    {
        // Impl处理返回
        BSID81_CASE_CODE_IMP2DEV(IMP_SUCCESS, IDC_SUCCESS);                    // 成功
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_LOAD_LIB, ERR_IDC_LIBRARY);           // 动态库加载失败
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_PARAM_INVALID, ERR_IDC_PARAM_ERR);    // 参数无效
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_UNKNOWN, ERR_IDC_OTHER);              // 未知错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_NOTOPEN, ERR_IDC_DEV_NOTOPEN);        // 设备未Open
        // Device返回
        //BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_00H, IDC_SUCCESS);              // 正常
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_01H, ERR_IDC_DEV_NOTFOUND);       // 无设备
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_02H, ERR_IDC_COMM_ERR);           // 端口错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_03H, ERR_IDC_PARAM_ERR);          // 参数文件错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_04H, ERR_IDC_OTHER);              // 未初始化
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_05H, ERR_IDC_PARAM_ERR);          // 无效参数
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_06H, ERR_IDC_OTHER);              // 超时错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_07H, ERR_IDC_DEV_HWERR);          // 上盖打开
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_08H, ERR_IDC_MED_JAMMED);         // 塞卡
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_09H, ERR_IDC_OTHER);              // 内存溢出
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_0AH, ERR_IDC_READ_ERR);           // 没有二代证数据
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_0BH, ERR_IDC_READ_ERR);           // 没有图像数据
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_0CH, ERR_IDC_READ_ERR);           // 图像处理错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_0DH, ERR_IDC_READ_ERR);           // 判断图像方向错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_0EH, ERR_IDC_COMM_ERR);           // 关闭端口失败
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_0FH, ERR_IDC_READ_ERR);           // 身份证电子信息处理错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_10H, ERR_IDC_DEV_HWERR);          // 传感器校验错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_11H, ERR_IDC_DEV_HWERR);          // 电压低
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_12H, ERR_IDC_DEV_HWERR);          // 校正错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_13H, ERR_IDC_INSERT_TIMEOUT);     // 无卡
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_14H, ERR_IDC_OTHER);              // 未知错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_15H, ERR_IDC_READ_ERR);           // 保存位图错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_16H, ERR_IDC_DEV_HWERR);          // 掉电错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_17H, ERR_IDC_DEV_HWERR);          // BOOT错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_18H, ERR_IDC_OTHER);              // 按键抬起
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_19H, ERR_IDC_DEV_HWERR);          // 识别错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_1AH, ERR_IDC_READ_ERR);           // 扫描错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_1BH, ERR_IDC_DEV_HWERR);          // 走卡错误
        BSID81_CASE_CODE_IMP2DEV(IMP_ERR_DEV_1CH, ERR_IDC_OTHER);              // 最大错误码
        default :
            return IDC_SUCCESS;
    }







}

//////////////////////////////////////////////////////////////////////////






