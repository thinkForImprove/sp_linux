/***************************************************************
* 文件名称：DevDPR_P3018D.cpp
* 文件描述：HL-2260D打印模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年8月23日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevDPR_P3018D.h"

#include "data_convertor.h"
#include <QTextCodec>

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>

static const char *ThisModule = "DevDPR_P3018D.cpp";

//////////////////////////////////////////////////////////////////////////

CDevDPR_P3018D::CDevDPR_P3018D() : m_devP3018D(LOG_NAME_DEVDPR)
{
    SetLogFile(LOG_NAME_DEVDPR, ThisModule);  // 设置日志文件名和错误发生的文件
    m_usDPIx = 192;                           // X方向DPI
    m_usDPIy = 208;                           // y方向DPI
    m_nGetStatErrOLD = PTR_SUCCESS;           // 取状态接口上一次错误码
    m_nGetOpenErrOLD = PTR_SUCCESS;
    m_handle = nullptr;
    m_usSleep = 0;                            // 睡眠时间

    for (int i = 0; i < 3; i++)
    {
        m_szJson = new char[JSON_DATA_SIZE];
        if (m_szJson == nullptr)
        {
            m_szJson = new char[JSON_DATA_SIZE];
        } else
            break;
    }
}

CDevDPR_P3018D::~CDevDPR_P3018D()
{
    Close();
}

// 打开与设备的连接
int CDevDPR_P3018D::Open(const char* lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    int len = 0;
    //CJsonObject cJsonPrtIn;

    // 建立DPR连接
    if (m_devP3018D.IsDeviceOpen())
    {
        m_devP3018D.CloseDevice();
    }

    if (!m_devP3018D.OpenDevice())
    {
        if (m_nGetOpenErrOLD != nRet)
        {
            Log(ThisModule, __LINE__, "建立设备连接: ->DeviceOpen(%s) Fail, ErrCode=%d, Return %s.",
                lpMode, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            m_nGetOpenErrOLD = nRet;
        }
        return ConvertErrorCode(nRet);
    }
    m_nGetOpenErrOLD = nRet;

    m_stdOpenMode.append(lpMode == nullptr ? "" : lpMode);

    nRet = m_devP3018D.GetPrinterList(m_szJson, &len);
    if (nRet == SS_ERR_SHORTAGEMEMORY)
    {
        if (len < JSON_DATA_SIZE)
            len = JSON_DATA_SIZE;

        if (m_szJson != nullptr)
        {
            delete[] m_szJson;
            m_szJson = nullptr;
            m_szJson = new char[len];

            nRet = m_devP3018D.GetPrinterList(m_szJson, &len);
            if (nRet != SS_OK)
            {
                Log(ThisModule, __LINE__, "建立设备连接: ->GetPrinterList Fail, malloc memory len is: %d, ErrCode=%d, Return %s.",
                     len, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }
    } else if (nRet != 0 || len == 0)
    {
        Log(ThisModule, __LINE__, "建立设备连接: ->GetPrinterList Fail, malloc memory len is: %d, ErrCode=%d, Return %s.",
             len, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

//    // 加载/解析JSON
//    if (cJsonPrtIn.Parse(m_szJson) != true)
//    {
//        Log(ThisModule, __LINE__, "打印字串: ->JSON.Parse(%s) Fail, Return %d.",
//            m_szJson, ERR_PTR_JSON_ERR);
//        return ERR_PTR_JSON_ERR;
//    }

//    JSON_GET(cJsonPrtIn, JSON_KEY_PRINTNAME, m_stdDevName);
//    if (m_stdDevName.empty())
//    {
//        Log(ThisModule, __LINE__, "GetPrinterList: ->JSON.Get(%s) = %s is empty, ret: %d",
//            JSON_KEY_PRINTNAME, m_stdDevName.c_str(), ERR_PTR_JSON_ERR);
//        return ERR_PTR_JSON_ERR;
//    }

    //TODO 先写死
    m_stdDevName = "Pantum-P3018D-series";
    // 获取设备句柄
    len = 0;
    if (m_handle == nullptr)
    {
        nRet = m_devP3018D.Open((LPSTR)m_stdDevName.c_str(), m_handle, &len);
        if (nRet == SS_ERR_SHORTAGEMEMORY && m_handle == nullptr && len != 0)
        {
            //m_handle = malloc(len);
            m_handle = (void*)(new char[len]);
            if (m_handle != nullptr)
            {
                nRet = m_devP3018D.Open((LPSTR)m_stdDevName.c_str(), m_handle, &len);
                if (nRet == SS_ERR_SHORTAGEMEMORY && len != 0)
                {
                    //m_handle = malloc(len);
                    m_handle = (void*)(new char[len]);
                    if (m_handle != nullptr)
                    {
                        nRet = m_devP3018D.Open((LPSTR)m_stdDevName.c_str(), m_handle, &len);
                        if (nRet != SS_OK)
                        {
                            Log(ThisModule, __LINE__, "建立设备连接: ->m_devP3018D.Open Thrid Fail, malloc memory len is: %d, ErrCode=%d, Return %s.",
                                 len, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                            return ConvertErrorCode(nRet);
                        }
                    }
                } else if (nRet == SS_OK && m_handle != nullptr){
                    return PTR_SUCCESS;
                } else {
                    Log(ThisModule, __LINE__, "建立设备连接: ->m_devP3018D.Open Second Fail, malloc memory len is: %d, ErrCode=%d, Return %s.",
                         len, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                    return ConvertErrorCode(nRet);
                }
            }
        } else if (nRet == SS_OK && m_handle != nullptr) {
            return PTR_SUCCESS;
        } else {
            Log(ThisModule, __LINE__, "建立设备连接: ->m_devP3018D.Open Fail, malloc memory len is: %d, ErrCode=%d, Return %s.",
                 len, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    }
    return PTR_SUCCESS;
}

// 关闭与设备的连接
int CDevDPR_P3018D::Close()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if (m_handle != nullptr)
        m_devP3018D.Close(m_handle);

    m_devP3018D.CloseDevice();
    Log(ThisModule, __LINE__, "关闭设备连接: ->DeviceClose() Succ, Return %s.",
        ConvertErrCodeToStr(PTR_SUCCESS));

    if (m_handle != nullptr)
    {
        delete[] (LPSTR)m_handle;
        m_handle = nullptr;
    }

    if (m_szJson != nullptr)
    {
        delete m_szJson;
        m_szJson = nullptr;
    }

    return PTR_SUCCESS;
}

// 设备复位
int CDevDPR_P3018D::Reset()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    if (m_handle != nullptr)
    {
        nRet = m_devP3018D.WarmUp(m_handle);
        if (nRet == SS_ERR_PREHEATED || nRet == IMP_SUCCESS)
            return PTR_SUCCESS;
        else
        {
            Log(ThisModule, __LINE__, "设备复位: ->WarmUp() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    }

    return PTR_SUCCESS;
}

// 取设备状态
int CDevDPR_P3018D::GetStatus(DEVPTRSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;
    LPBYTE   ucDevStat = new BYTE[5];
    int    nDevStat = 0;
    DEVPTRSTATUS dev;

    // 设置设备初始状态
    dev.wDevice = DEV_STAT_HWERROR;
    dev.wMedia = MEDIA_STAT_UNKNOWN;
    for (INT i = 0; i < 16; i ++)
    {
        if (i == 0)
            dev.wPaper[WFS_PTR_SUPPLYUPPER] = WFS_PTR_PAPERFULL;
        else
            dev.wPaper[i] = PAPER_STAT_NOTSUPP;
    }

    dev.wInk = INK_STAT_FULL;
    dev.wToner = TONER_STAT_FULL;
    for (INT i = 0; i < 16; i ++)
    {
        dev.stRetract[i].wBin = RETRACT_STAT_MISSING;
    }

    if (!m_devP3018D.IsDeviceOpen())
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (m_nGetStatErrOLD != ERR_PTR_NOT_OPEN)
        {
            Log(ThisModule, __LINE__, "读取设备状态: ->IsDeviceOpen() Is FALSE, Device Not Open, Return %s.",
                ConvertErrCodeToStr(ERR_PTR_NOT_OPEN));
            m_nGetStatErrOLD = ERR_PTR_NOT_OPEN;
        }

        dev.wDevice = DEV_STAT_OFFLINE;
        return ERR_PTR_NOT_OPEN;
    }

    // 取设备状态
    DEVDPRSTATUS stDevStat;
    if (m_handle != nullptr && m_szJson != nullptr)
    {
        memset(m_szJson, 0x00, JSON_DATA_SIZE);
        nRet = m_devP3018D.GetDeviceInfo(m_handle, m_szJson);
        if (nRet != SS_OK)
        {
            Log(ThisModule, __LINE__, "读取设备信息: ->GetDeviceInfo() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            m_nGetStatErrOLD = ConvertErrorCode(nRet);
        }

        if (nRet == SS_ERR_HWERROR) // -3硬件故障(未连接或无法连接)
        {
            dev.wDevice = DEV_STAT_OFFLINE;
            return ConvertErrorCode(nRet);
        }
    }

    // 解析获取的设备状态
    GetDeviceInfo(m_szJson, dev);
    stDevStat = dev.stDPRStatus;
    // Device状态
    if (m_handle != nullptr)
    {
        nRet = m_devP3018D.GetState(m_handle, ucDevStat);
        if (nRet != SS_OK)
        {
            Log(ThisModule, __LINE__, "读取设备状态: ->GetState() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            m_nGetStatErrOLD = ConvertErrorCode(nRet);
        }

        nDevStat = SwitchDevStat(ucDevStat);

        if (ucDevStat != nullptr)
        {
            delete[] ucDevStat;
            ucDevStat = nullptr;
        }

        if (nRet == SS_ERR_HWERROR ||
            nDevStat == 2) // -3硬件故障(未连接或无法连接)
        {
            dev.wDevice = DEV_STAT_OFFLINE;
            dev.wMedia = MEDIA_STAT_UNKNOWN;
            return ConvertErrorCode(nRet);
        }
    }

    switch(nDevStat)
    {
        case 0:
        {
            dev.wDevice = DEV_STAT_ONLINE;
            dev.wMedia = MEDIA_STAT_NOTPRESENT;
        }
            break;
        case 1:
        {
            dev.wDevice = DEV_STAT_HWERROR;
            dev.wMedia = MEDIA_STAT_UNKNOWN;
        }
            break;
        case 2:
        {
            dev.wDevice = DEV_STAT_OFFLINE;
            dev.wMedia = MEDIA_STAT_UNKNOWN;
        }
            break;
        case 3:
        {
            dev.wPaper[WFS_PTR_SUPPLYUPPER] = WFS_PTR_PAPEROUT;
            dev.wDevice = DEV_STAT_ONLINE;
            dev.wMedia = MEDIA_STAT_NOTPRESENT;
        }
            break;
        case 5:
        {
            dev.wPaper[WFS_PTR_SUPPLYUPPER] = WFS_PTR_PAPERJAMMED;
            dev.wDevice = DEV_STAT_HWERROR;
            dev.wMedia = MEDIA_STAT_JAMMED;
        }
            break;
        case 6:
        {
            dev.wDevice = DEV_STAT_ONLINE;
            dev.stDPRStatus.wLife = LIFE_STAT_OUT;
            dev.wInk = INK_STAT_OUT;
            dev.wMedia = MEDIA_STAT_NOTPRESENT;
        }
            break;
        case 8:
        {
            dev.wDevice = DEV_STAT_BUSY;
            dev.wMedia = MEDIA_STAT_PRESENT;
        }
            break;
        case 9:
        {
            dev.wDevice = DEV_STAT_HWERROR;
            dev.wMedia = MEDIA_STAT_UNKNOWN;
        }
            break;
        case 10:
        {
            dev.wPaper[WFS_PTR_SUPPLYUPPER] = WFS_PTR_PAPERUNKNOWN;
            dev.wDevice = DEV_STAT_HWERROR;
            dev.wMedia = MEDIA_STAT_NOTPRESENT;
        }
            break;
    case 11:
        {
            dev.wPaper[WFS_PTR_SUPPLYUPPER] = WFS_PTR_PAPERUNKNOWN;
            dev.wDevice = DEV_STAT_HWERROR;
            dev.wMedia = MEDIA_STAT_NOTPRESENT;
        }
            break;
    case 12:
        {
            dev.wPaper[WFS_PTR_SUPPLYUPPER] = WFS_PTR_PAPERUNKNOWN;
            dev.wDevice = DEV_STAT_HWERROR;
            dev.wMedia = MEDIA_STAT_NOTPRESENT;
        }
            break;
    default:
        dev.wDevice = DEV_STAT_OFFLINE;
        break;
    }

    if (dev.wInk > 30)
        dev.wInk = INK_STAT_FULL;
    else if (dev.wInk > 0 && dev.wInk <= 30)
        dev.wInk = INK_STAT_LOW;
    else if (dev.wInk <= 5)
        dev.wInk = INK_STAT_OUT;
    else
        dev.wInk = INK_STAT_UNKNOWN;

    if (dev.wToner > 30)
        dev.wToner = TONER_STAT_FULL;
    else if (dev.wToner > 0 && dev.wToner <= 30)
        dev.wToner = TONER_STAT_LOW;
    else if (dev.wToner <= 5)
        dev.wToner = TONER_STAT_OUT;
    else
        dev.wToner = TONER_STAT_UNKNOWN;

    if (dev.stDPRStatus.wLessToner == 1)
    {
        dev.wInk = INK_STAT_LOW;
        dev.stDPRStatus.wLife = LIFE_STAT_LOW;
    }

    // 状态出现变化时,打印LOG
    //DiffDevStat(stDevStat, m_stScanStatus);

    memcpy(&stStatus, &dev, sizeof(DEVPTRSTATUS));

    return PTR_SUCCESS;
}

// 设置数据
int CDevDPR_P3018D::SetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case DTYPE_LIB_PATH:    // 设置Lib路径
        {
            m_devP3018D.SetLibPath((LPCSTR)vInitPar);
            break;
        }
        case DTYPE_DPIx:
        {
            m_usDPIx = *((USHORT*)vInitPar);
            break;
        }
        case DTYPE_DPIy:
        {
            m_usDPIy = *((USHORT*)vInitPar);
            break;
        }
        case DTYPE_SLEEP:
        {
            m_usSleep = *((USHORT*)vInitPar);
            break;
        }
        default:
            break;
    }

    int nRet = IMP_SUCCESS;
    if (wDataType == DTYPE_SLEEP)
    {
        // 设置睡眠时间
        if (CheckSleepTimeisVaild(m_usSleep) && m_handle != nullptr)
        {
            nRet = m_devP3018D.SetSleepTime(m_handle, m_usSleep);
            if (nRet != SS_OK)
            {
                Log(ThisModule, __LINE__, "SetData(设置睡眠时间): ->SetSleepTime() Fail, sleep=%ud, ErrCode=%d, Return %s.",
                    m_usSleep, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }
    } else if (wDataType == DTYPE_DELCURJOB) {
        // 删除当前打印作业
        if (m_handle != nullptr)
        {
            nRet = m_devP3018D.DeleteCurrentJob(m_handle);
            if (nRet != SS_OK)
            {
                Log(ThisModule, __LINE__, "SetData(删除当前打印作业): ->DeleteCurrentJob() Fail, ErrCode=%d, Return %s.",
                    nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        } else {
            Log(ThisModule, __LINE__, "SetData(删除当前打印作业): ->DeleteCurrentJob() Fail, 打印机句柄为空");
            return ERR_PTR_DRVHND_REMOVE;
        }
    }

    return PTR_SUCCESS;
}

// 获取数据
int CDevDPR_P3018D::GetData(void *vInitPar, WORD wDataType)
{
    int nRet = IMP_SUCCESS;
    switch(wDataType)
    {
        case DTYPE_ERRINFO:
        {
            // 获取设备状态描述信息
            memset(m_szErrStr, 0x00, 1024);
            if (m_handle != nullptr)
            {
                nRet = m_devP3018D.GetErrmsg(m_handle, m_szErrStr);
                if (nRet != SS_OK)
                {
                    Log(ThisModule, __LINE__, "GetData(获取设备状态描述信息): ->GetErrmsg() Fail, ErrCode=%d, Return %s.",
                        nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                    return ConvertErrorCode(nRet);
                }
            } else {
                Log(ThisModule, __LINE__, "GetData(获取设备状态描述信息): ->GetErrmsg() Fail, 打印机句柄为空");
                return ERR_PTR_DRVHND_REMOVE;
            }

            vInitPar = (void*)m_szErrStr;
            break;
        }
        case DTYPE_JOBNUM:
        {
            // 获取已打印页数
            DWORD num = 0;
            if (m_handle != nullptr)
            {
                nRet = m_devP3018D.GetPageNum(m_handle, &num);
                if (nRet != SS_OK)
                {
                    Log(ThisModule, __LINE__, "GetData(获取已打印页数): ->GetPageNum() Fail, num=%ud, ErrCode=%d, Return %s.",
                        num, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                    return ConvertErrorCode(nRet);
                }
            } else {
                Log(ThisModule, __LINE__, "GetData(获取已打印页数): ->GetPageNum() Fail, 打印机句柄为空");
                return ERR_PTR_DRVHND_REMOVE;
            }

            vInitPar = (void*)num;
            break;
        }
        default:
            break;
    }

    return PTR_SUCCESS;
}

// 获取版本号(1DevCSR版本/2固件版本/3设备软件版本/4其他)
void CDevDPR_P3018D::GetVersion(char* szVer, long lSize, ushort usType)
{
    CHAR    szVersion[128] = { 0x00 };
    int nRet = SS_OK;

    if (usType == 1)
    {
        //memcpy(szVersion, byDevVRTU, strlen((char*)byDevVRTU));
    } else if (usType == 2)
    {
        nRet = m_devP3018D.GetVersion(m_handle, szVersion);
        if (nRet != SS_OK)
        {
            Log(ThisModule, __LINE__, "GetVersion: 获取固件版本号失败, ErrCode=%d, Return %s.",
                 nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        }
    } else if (usType == 3)
    {
        return;
    } else if (usType == 4)
    {
        // 产品序列号
        nRet = m_devP3018D.GetSerialNumber(m_handle, szVersion);
        if (nRet != SS_OK)
        {
            Log(ThisModule, __LINE__, "GetVersion: 获取产品序列号失败, ErrCode=%d, Return %s.",
                 nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        }
    }

    memcpy(szVer, szVersion, strlen((char*)szVersion) > lSize ? lSize : strlen((char*)szVersion));
}

INT CDevDPR_P3018D::ConvertErrorCode(INT nRet)
{
    switch(nRet)
    {
        case SS_ERR_FAILED:                                         // -1:调用API失败
            return ERR_PTR_OTHER;
        case SS_ERR_SHORTAGEMEMORY:                                 // -2 内存大小不足
            return ERR_PTR_SHORTAGEMEMORY;
        case SS_ERR_HWERROR:                                        // -3:硬件出错(未连接或无法连接)->
            return ERR_PTR_HWERR;
        case SS_ERR_PARAMERR:                                       // -4:参数错误
            return ERR_PTR_PARAM_ERR;
        case SS_ERR_NOMATCHSDK:                                     // -5:打印机与 SDK 不匹配
            return ERR_PTR_DRVHND_ERR;
        case SS_ERR_UNSUPPCMD:                                      // -6:该打印机不支持此接口
            return ERR_PTR_UNSUP_CMD;
//        case SS_ERR_DEVICECLOSED:                                   // -6:设备已关闭->设备没有打开
//            return ERR_PTR_NOT_OPEN;
        case SS_ERR_PREHEATED:                                      // -7:打印机已预热->命令返回正确
            return ERR_PTR_CAN_RESUME;
        case SS_ERR_POCKETFULL:                                     // -8:票箱已满->回收箱满
            return ERR_PTR_RETRACTFULL;
        case SS_ERR_NOINKPRESENT:                                   // -9:墨盒不在位->INK或色带故障
            return ERR_PTR_TONER;
        case SS_ERR_PARSEJSON:                                      // -10:解析Json参数错误
            return ERR_PTR_JSON_ERR;
        case SS_ERR_LOADALGDLL:                                     // -20:加载算法库失败
        case SS_ERR_INITALG:                                        // -21:算法初始化接口失败
            return ERR_PTR_CHRONIC;
        case SS_ERR_SETBRIGHT:                                      // -22:设置图像亮度失败
        case SS_ERR_POCKETOPEN:                                     // -23:票箱开
        case SS_ERR_OTHER:                                          // -99999:其他错误->其它错误
            return ERR_PTR_OTHER;
        case SS_ERR_TIMEOUT:                                        // -24:操作超时->超时
            return ERR_PTR_TIMEOUT;
        case SS_ERR_CANCELED:                                       // -25:操作被取消->命令取消
            return ERR_PTR_CANCEL;
        case SS_ERR_CHK_JAM:                                        // -12010:卡票->卡纸
            return ERR_PTR_JAMMED;
        case SS_ERR_CHK_NOPAPERINPRINTPOSITION:                     // -12011:打印位置没有票据
        case SS_ERR_CHK_NOMEDIAPRESENT:                             // -12020:通道无票->指定位置无介质
            return ERR_PTR_NOMEDIA;
        case SS_ERR_CHK_NOIMAGE:                                    // -12021:无可用图像数据->软件故障
            return ERR_PTR_CHRONIC;
        case SS_ERR_CHK_MEDIAPRESENT:                               // -12022:通道有票->指定位置有介质
            return ERR_PTR_HAVEMEDIA;
        case SS_ERR_CHK_UNKNOWTYPE:                                 // -12030:票据类型未知
        case SS_ERR_CHK_IMPERFECT:                                  // -12031:票据缺角
        case SS_ERR_CHK_FORGED:                                     // -12032:伪票->介质异常
            return ERR_PTR_PAPER_ERR;
        case SS_ERR_CHK_NOOCRAREA:                                  // -12034:未设置OCR区域
            return ERR_PTR_CHRONIC;
        case SS_ERR_CHK_INCOMPLETE:                                 // -12040:票据要素不全
        case SS_ERR_CHK_TYPEERROR:                                  // -12050:票据类型不符
        case SS_ERR_CHK_DIR:                                        // -12070:票据正反面放置错误
        case SS_ERR_CHK_LONGLENGTH:                                 // -12080:票据超长->介质异常
            return ERR_PTR_PAPER_ERR;
        case SS_ERR_CHK_READRFID:                                   // -12090:读RFID失败->数据识别失败
            return ERR_PTR_DATA_DISCERN;
        case SS_ERR_CHK_SAVEIMAGE:                                  // -12601:图像保存失败
            return ERR_PTR_OTHER;
        case SS_ERR_CHK_EJECT_JAM:                                  // -12503:退票失败，卡票->卡纸
            return ERR_PTR_JAMMED;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_PATAMETER:             // -501:鉴伪参数错误
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_CONFIGFILE:            // -502:鉴伪配置文件格式错误
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_CONFIGDATA:            // -503:鉴伪配置文件数据错误
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_TYPENUMMANY:           // -504:鉴伪存单类型数量太多
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_LOADLIBRARY:           // -505:鉴伪加载动态库失败
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_DECRYPT:               // -506:鉴伪解密错误，交互验证不通过
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_DESKEWCROP:            // -507:鉴伪纠偏裁剪错误
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_WIDTHHEIGHT:           // -508:鉴伪票据图像宽度高度不符
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_BILLTYPE:              // -509:鉴伪票据类型错误
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_BASEPOINT:             // -510:鉴伪定位基准点失败
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_FRONTREAR:             // -511:鉴伪正反面放反
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_MALLOCBUFFER:          // -512:鉴伪内存申请错误
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NONETEMPLATE:          // -513:鉴伪没有模板数据
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NULLDATA:              // -514:鉴伪没有图像数据
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NONEFEATURE:           // -515:鉴伪没有特征数据
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_WATERMARK:             // -516:鉴伪水印错误
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_ANGLEBIG:              // -517:鉴伪倾斜角度大
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_FINDRECT:              // -518:鉴伪查找区域失败
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NONEFUNC:              // -600:鉴伪缺少功能
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_OHTER:                 // -700:鉴伪其它错误->其它错误
        case SCANNER_ERROR_OCR_ERROR_PARAS:                           // -801 :OCR参数错误
        case SCANNER_ERROR_OCR_ERROR_LOAD_CONFIG_FILE:                // -802 :OCR加载主动态库配置文件错误
        case SCANNER_ERROR_OCR_ERROR_LOAD_CONFIG_INFO:                // -803 :OCR主配置文件配置文件信息错误
        case SCANNER_ERROR_OCR_ERROR_LOAD_DLL:                        // -804 :OCR加载识别动态库失败
        case SCANNER_ERROR_OCR_ERROR_GET_DLL_PROC:                    // -805 :OCR获取导出识别函数失败
        case SCANNER_ERROR_OCR_ERROR_DLL_INIT :                       // -806 :OCR识别动态库初始化函数返回值错误
        case SCANNER_ERROR_OCR_ERROR_RECOG:                           // -807 :OCR识别失败
        case SCANNER_ERROR_OCR_ERROR_OHTER:                           // -900 :OCR其它错误
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_PATAMETER:               // -1001:支票鉴伪参数错误
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_ANGLEBING:               // -1002:支票鉴伪倾斜角度大
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_QUALITY:                 // -1003:支票鉴伪质量检测不合格
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_TABLESIZE:               // -1004:支票鉴伪表格大小不符合要求
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_BILLTYPE:                // -1005:支票鉴伪票据类型错误
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_IMAGEFLIP:               // -1006:支票鉴伪图像方向颠倒
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_IMAGEDARK:               // -1007:支票鉴伪红外发射过暗
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_INCOMPLETE:              // -1008:支票鉴伪票面扫描不完整
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_LOCATION:                // -1009:支票鉴伪定位失败
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_FRONTREAR:               // -1010:支票鉴伪图像正面与反面放反
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_BRTIRTR:                 // -1030:支票鉴伪红外透射亮度异常
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_DECRYPT:                 // -1101:支票鉴伪解密错误
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_DESKEWCROP:              // -1102:支票鉴伪纠偏错误
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_DESKEWCROPNOBASEIMAGE:   // -1103:支票鉴伪无纠偏基准图像
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_CHEQUETYPE:              // -1200:支票鉴伪票据类型错误
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_OHTER:                   // -1300:支票鉴伪其他错误
            return ERR_PTR_OTHER;
        case IMP_ERR_LOAD_LIB:                                      // 动态库加载失败->其它错误
            return ERR_PTR_OTHER;
        case IMP_ERR_PARAM_INVALID:                                 // 参数无效->参数错误
            return ERR_PTR_PARAM_ERR;
        case IMP_ERR_UNKNOWN:                                       // 未知错误->其它错误
            return ERR_PTR_OTHER;
        default:                                                    // 其它错误
            return ERR_PTR_OTHER;
    }
}

CHAR* CDevDPR_P3018D::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(nRet)
    {
        case PTR_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "操作成功");
            return m_szErrStr;
        case ERR_PTR_CANCEL:
            sprintf(m_szErrStr, "%d|%s", nRet, "命令取消");
            return m_szErrStr;
        case ERR_PTR_PARAM_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "参数错误");
            return m_szErrStr;
        case ERR_PTR_COMM_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "通讯错误");
            return m_szErrStr;
        case ERR_PTR_NO_PAPER:
            sprintf(m_szErrStr, "%d|%s", nRet, "打印机缺纸");
            return m_szErrStr;
        case ERR_PTR_JAMMED:
            sprintf(m_szErrStr, "%d|%s", nRet, "堵纸等机械故障");
            return m_szErrStr;
        case ERR_PTR_NOT_OPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备没有打开");
            return m_szErrStr;
        case ERR_PTR_HEADER:
            sprintf(m_szErrStr, "%d|%s", nRet, "打印头故障");
            return m_szErrStr;
        case ERR_PTR_CUTTER:
            sprintf(m_szErrStr, "%d|%s", nRet, "切刀故障");
            return m_szErrStr;
        case ERR_PTR_TONER:
            sprintf(m_szErrStr, "%d|%s", nRet, "INK或色带故障");
            return m_szErrStr;
        case ERR_PTR_STACKER_FULL:
            sprintf(m_szErrStr, "%d|%s", nRet, "用户没有取走");
            return m_szErrStr;
        case ERR_PTR_NO_RESUME:
            sprintf(m_szErrStr, "%d|%s", nRet, "不可恢复的错误");
            return m_szErrStr;
        case ERR_PTR_CAN_RESUME:
            sprintf(m_szErrStr, "%d|%s", nRet, "可恢复的错误");
            return m_szErrStr;
        case ERR_PTR_FORMAT_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "打印字串格式错误");
            return m_szErrStr;
        case ERR_PTR_CHRONIC:
            sprintf(m_szErrStr, "%d|%s", nRet, "慢性故障");
            return m_szErrStr;
        case ERR_PTR_HWERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "硬件故障");
            return m_szErrStr;
        case ERR_PTR_IMAGE_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "图片相关错误");
            return m_szErrStr;
        case ERR_PTR_NO_DEVICE:
            sprintf(m_szErrStr, "%d|%s", nRet, "指定名的设备不存在");
            return m_szErrStr;
        case ERR_PTR_UNSUP_CMD:
            sprintf(m_szErrStr, "%d|%s", nRet, "不支持的指令");
            return m_szErrStr;
        case ERR_PTR_DATA_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "收发数据错误");
            return m_szErrStr;
        case ERR_PTR_TIMEOUT:
            sprintf(m_szErrStr, "%d|%s", nRet, "超时");
            return m_szErrStr;
        case ERR_PTR_DRVHND_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "驱动错误");
            return m_szErrStr;
        case ERR_PTR_DRVHND_REMOVE:
            sprintf(m_szErrStr, "%d|%s", nRet, "驱动丢失");
            return m_szErrStr;
        case ERR_PTR_USB_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "USB/COM/连接错误");
            return m_szErrStr;
        case ERR_PTR_DEVBUSY:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备忙");
            return m_szErrStr;
        case ERR_PTR_OTHER:
            sprintf(m_szErrStr, "%d|%s", nRet, "其它错误");
            return m_szErrStr;
        case ERR_PTR_DEVUNKNOWN:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备未知");
            return m_szErrStr;
        case ERR_PTR_NOMEDIA:
            sprintf(m_szErrStr, "%d|%s", nRet, "指定位置无介质");
            return m_szErrStr;
        case ERR_PTR_HAVEMEDIA:
            sprintf(m_szErrStr, "%d|%s", nRet, "指定位置有介质");
            return m_szErrStr;
        case ERR_PTR_PAPER_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "介质异常");
            return m_szErrStr;
        case ERR_PTR_JSON_ERR:
            sprintf(m_szErrStr, "%d|%s", nRet, "JSON错误");
            return m_szErrStr;
        case ERR_PTR_SCAN_FAIL:
            sprintf(m_szErrStr, "%d|%s", nRet, "扫描失败");
            return m_szErrStr;
        case ERR_PTR_DATA_DISCERN:
            sprintf(m_szErrStr, "%d|%s", nRet, "数据识别失败");
            return m_szErrStr;
        case ERR_PTR_NO_MEDIA:
            sprintf(m_szErrStr, "%d|%s", nRet, "通道无纸");
            return m_szErrStr;
        case ERR_PTR_RETRACTFULL:
            sprintf(m_szErrStr, "%d|%s", nRet, "回收箱满");
            return m_szErrStr;
    case ERR_PTR_SHORTAGEMEMORY:
            sprintf(m_szErrStr, "%d|%s", nRet, "内存大小不足");
            return m_szErrStr;
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "其他错误/未知错误");
            return m_szErrStr;
    }
}

void CDevDPR_P3018D::DiffDevStat(SCANNERSTATUS stStat, SCANNERSTATUS stStatOLD)
{
    THISMODULE(__FUNCTION__);

    INT nStat[50] = {
            stStat.iStatus,                     // 状态位:0-12
            stStat.iHaveImageData,              // 0-无图像数据，1-有需要上传的图像数据
            stStat.iError,                      // 0-正常，1-出错(通用错误,具体错误根据其他参数判定)
            stStat.iJam,                        // 0-正常，1-塞纸
            stStat.iVoltageError,               // 0-正常，1-电压异常
            stStat.iHardwareError,              // 0-正常，1-硬件错误
            stStat.iFPGAError,                  // 0-正常，1-FPGA错误
            stStat.iPocketOpen,                 // 0-出纸兜关，1-出纸兜开(左/右票箱打开时状态,iError==1)
            stStat.iLeftPocketFull,             // 0-左票箱未满，1-左票箱满
            stStat.iPushGearError,              // 0-出票兜凸轮归位，1-出票兜凸轮未归位
            stStat.iPushBoardError,             // 0-出票兜推纸板归位，1-出票兜推纸板未归位
            stStat.iPickFail,                   // 0-纠偏成功，1-纠偏失败
            stStat.iPaperEnter,                 // 0-纸张进入正常，1-纸张进入异常
            stStat.iPaperLength,                // 0-纸张长度正常，1-纸张长度超长/过短
            stStat.iPocketFullSensorInit,		// 0-正常，1-票箱满传感器初始化异常
            stStat.iPaperEnterRepeat,			// 0-正常，1-纸张二次进入状态
            stStat.iRightPocketFull,			// 0-右票箱未满，1-右票箱满
            stStat.iLeftPocketHavePaper,		// 0-左票箱无票，1-左票箱有票
            stStat.iRightPocketHavePaper,		// 0-右票箱无票，1-右票箱有票
            stStat.iLeftPocketOpen,             // 0-左票箱关，1-左票箱开(打开时,iError==1,iPocketOpen=1)
            stStat.iRightPocketOpen,			// 0-右票箱关，1-右票箱开(打开时,iError==1,iPocketOpen=1)
            stStat.iInkPresent,                 // 0-墨盒不在位，1-墨盒在位
    };
    INT nStatOLD[50] = {
            stStatOLD.iStatus,                      // 状态位:0-12
            stStatOLD.iHaveImageData,               // 0-无图像数据，1-有需要上传的图像数据
            stStatOLD.iError,                       // 0-正常，1-出错(通用错误,具体错误根据其他参数判定)
            stStatOLD.iJam,                         // 0-正常，1-塞纸
            stStatOLD.iVoltageError,                // 0-正常，1-电压异常
            stStatOLD.iHardwareError,               // 0-正常，1-硬件错误
            stStatOLD.iFPGAError,                   // 0-正常，1-FPGA错误
            stStatOLD.iPocketOpen,                  // 0-出纸兜关，1-出纸兜开(左/右票箱打开时状态,iError==1)
            stStatOLD.iLeftPocketFull,              // 0-左票箱未满，1-左票箱满
            stStatOLD.iPushGearError,               // 0-出票兜凸轮归位，1-出票兜凸轮未归位
            stStatOLD.iPushBoardError,              // 0-出票兜推纸板归位，1-出票兜推纸板未归位
            stStatOLD.iPickFail,                    // 0-纠偏成功，1-纠偏失败
            stStatOLD.iPaperEnter,                  // 0-纸张进入正常，1-纸张进入异常
            stStatOLD.iPaperLength,                 // 0-纸张长度正常，1-纸张长度超长/过短
            stStatOLD.iPocketFullSensorInit,		// 0-正常，1-票箱满传感器初始化异常
            stStatOLD.iPaperEnterRepeat,			// 0-正常，1-纸张二次进入状态
            stStatOLD.iRightPocketFull,             // 0-右票箱未满，1-右票箱满
            stStatOLD.iLeftPocketHavePaper,         // 0-左票箱无票，1-左票箱有票
            stStatOLD.iRightPocketHavePaper,		// 0-右票箱无票，1-右票箱有票
            stStatOLD.iLeftPocketOpen,              // 0-左票箱关，1-左票箱开(打开时,iError==1,iPocketOpen=1)
            stStatOLD.iRightPocketOpen,             // 0-右票箱关，1-右票箱开(打开时,iError==1,iPocketOpen=1)
            stStatOLD.iInkPresent,                  // 0-墨盒不在位，1-墨盒在位
        };

    CHAR szData[1024] = { 0x00 };
    for (INT i = 0; i < 23; i ++)
    {
        sprintf(szData + strlen(szData), "%d->%d%s|", nStatOLD[i], nStat[i],
                (nStatOLD[i] != nStat[i] ? "*" : " "));
        if ((i + 1) % 10 == 0)
        {
            sprintf(szData + strlen(szData), "\n");
        }
    }

    if (memcmp(&stStat, &stStatOLD, sizeof(SCANNERSTATUS)) != 0)
    {
        Log(ThisModule, __LINE__, "读取设备状态: 变化比较: \n%s", szData);
    }
}

void CDevDPR_P3018D::GetDeviceInfo(LPSTR json, DEVPTRSTATUS &dev)
{
#define JSON_GET_VOID(JSON, KEY, VALUE) \
    if (JSON.Get(KEY, VALUE) != true) \
    {\
        Log(ThisModule, __LINE__, "打印字串: ->JSON.Get(%s) Fail, JSON=[%s],Return %d.", \
            KEY, VALUE.c_str(), ERR_PTR_JSON_ERR); \
        return; \
    }

#define CHECK_JSON(DATA1, DATA2) \
    if (DATA2.empty()) \
    { \
    Log(ThisModule, __LINE__, "打印字串: ->JSON.Get(%s) = %s is empty, Return %d.", DATA1, DATA2.c_str(), ERR_PTR_JSON_ERR); \
    return; \
    }


    if (json == nullptr)
        return;

    DEVDPRSTATUS status;
    CJsonObject cJsonPrtIn;

    std::string strToner = "";                   // 碳粉剩余百分比
    std::string strLife = "";                    // 硒鼓剩余百分比
    std::string strInk = "";                     // 黑色墨剩余百分比
    std::string strTotal = "";                   // 已打印总张数
    std::string strA4Print = "";                 // 已打印 A4 张数
    std::string strA5Print = "";                 // 已打印 A5 张数
    std::string strLessPaper = "";               // 打印机即将缺纸,1表示即将缺纸，0表示纸张足够
    std::string strLessToner = "";               // 硒鼓或粉盒即将耗尽,1表示即将耗尽，0表示硒鼓或粉盒正常

    // 加载/解析JSON
    if (cJsonPrtIn.Parse(json) != true)
    {
        Log(ThisModule, __LINE__, "打印字串: ->JSON.Parse(%s) Fail, Return %d.",
            json, ERR_PTR_JSON_ERR);
        return;
    }

    // toner
    JSON_GET_VOID(cJsonPrtIn, JSON_KEY_TONER, strToner);
    CHECK_JSON(JSON_KEY_TONER, strToner);
    dev.wToner = StringToWORD(strToner);

    // life
    JSON_GET_VOID(cJsonPrtIn, JSON_KEY_LIFE, strLife);
    CHECK_JSON(JSON_KEY_LIFE, strLife);
    status.wLife = StringToWORD(strLife);

    // black
    JSON_GET_VOID(cJsonPrtIn, JSON_KEY_BLACK, strInk);
    CHECK_JSON(JSON_KEY_BLACK, strInk);
    dev.wInk = StringToWORD(strInk);

    // total
    JSON_GET_VOID(cJsonPrtIn, JSON_KEY_TOTAL_PRINT, strTotal);
    CHECK_JSON(JSON_KEY_TOTAL_PRINT, strTotal);
    status.dwTotal = StringToDWORD(strTotal);

    // a4
    JSON_GET_VOID(cJsonPrtIn, JSON_KEY_A4_PRINT, strA4Print);
    CHECK_JSON(JSON_KEY_A4_PRINT, strA4Print);
    status.dwA4Print = StringToDWORD(strA4Print);

    // a5
    JSON_GET_VOID(cJsonPrtIn, JSON_KEY_A5_PRINT, strA5Print);
    CHECK_JSON(JSON_KEY_A5_PRINT, strA5Print);
    status.dwA5Print = StringToDWORD(strA5Print);

    // lessPaper
    JSON_GET_VOID(cJsonPrtIn, JSON_KEY_LESS_PAPER, strLessPaper);
    CHECK_JSON(JSON_KEY_LESS_PAPER, strLessPaper);
    dev.wPaper[WFS_PTR_SUPPLYUPPER] = StringToWORD(strLessPaper);

    // lessToner
    JSON_GET_VOID(cJsonPrtIn, JSON_KEY_LESS_TONER, strLessToner);
    CHECK_JSON(JSON_KEY_LESS_TONER, strLessToner);
    status.wLessToner = StringToWORD(strLessToner);

    dev.stDPRStatus.Clear();
    dev.stDPRStatus = status;
}

WORD CDevDPR_P3018D::StringToWORD(string str)
{
    WORD x = 0;
    stringstream ss;
    ss << str;
    ss >> x;
    return x;
}

DWORD CDevDPR_P3018D::StringToDWORD(string str)
{
    DWORD x = 0;
    stringstream ss;
    ss << str;
    ss >> x;
    return x;
}

BYTE CDevDPR_P3018D::SwitchDevStat(LPBYTE stat)
{
    BYTE btStatus= 0;
    if (stat == nullptr)
        return -1;

    btStatus = stat[0];

    return btStatus;
}

USHORT CDevDPR_P3018D::CheckSleepTimeisVaild(USHORT time)
{
    if (time == 1 || time == 5 || time == 15 ||
        time == 30 || time == 60)
        return TRUE;

    return FALSE;
}

// 错误码及含义
CHAR* CDevDPR_P3018D::ConvertImplErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nRet)
    {
        case SS_OK:                                                 // 0:操作成功
            sprintf(m_szErrStr, "%d|%s", nRet, "操作成功");
            return m_szErrStr;
        case SS_ERR_FAILED:                                         // -1:执行失败:
            sprintf(m_szErrStr, "%d|%s", nRet, "执行失败");
            return m_szErrStr;
        case SS_ERR_SHORTAGEMEMORY:                                 // 2:内存大小不足
            sprintf(m_szErrStr, "%d|%s", nRet, "内存大小不足");
            return m_szErrStr;
        case SS_ERR_HWERROR:                                        // -3:硬件出错
            sprintf(m_szErrStr, "%d|%s", nRet, "硬件出错");
            return m_szErrStr;
        case SS_ERR_PARAMERR:                                       //  -4:参数错误
            sprintf(m_szErrStr, "%d|%s", nRet, "参数错误");
            return m_szErrStr;
        case SS_ERR_NOMATCHSDK:                                     // -5:打印机与 SDK 不匹配
            sprintf(m_szErrStr, "%d|%s", nRet, "打印机与 SDK 不匹配");
            return m_szErrStr;
        case SS_ERR_UNSUPPCMD:                                      // -6:该打印机不支持此接口
            sprintf(m_szErrStr, "%d|%s", nRet, "该打印机不支持此接口");
            return m_szErrStr;
        case SS_ERR_PREHEATED:                                       // -7:打印机已预热
            sprintf(m_szErrStr, "%d|%s", nRet, "打印机已预热");
            return m_szErrStr;
/***********************************以下视情况删除*****************************************************/

        case SS_ERR_POCKETFULL:                                     // -8:票箱已满
            sprintf(m_szErrStr, "%d|%s", nRet, "票箱已满");
            return m_szErrStr;
        case SS_ERR_NOINKPRESENT:                                   // -9:墨盒不在位
            sprintf(m_szErrStr, "%d|%s", nRet, "墨盒不在位");
            return m_szErrStr;
        case SS_ERR_PARSEJSON:                                      // -10:解析Json参数错误
            sprintf(m_szErrStr, "%d|%s", nRet, "解析Json参数错误");
            return m_szErrStr;
        case SS_ERR_LOADALGDLL:                                     // -20:加载算法库失败
            sprintf(m_szErrStr, "%d|%s", nRet, "加载算法库失败");
            return m_szErrStr;
        case SS_ERR_INITALG:                                        // -21:算法初始化接口失败
            sprintf(m_szErrStr, "%d|%s", nRet, "算法初始化接口失败");
            return m_szErrStr;
        case SS_ERR_SETBRIGHT:                                      // -22:设置图像亮度失败
            sprintf(m_szErrStr, "%d|%s", nRet, "设置图像亮度失败");
            return m_szErrStr;
        case SS_ERR_POCKETOPEN:                                     // -23:票箱开
            sprintf(m_szErrStr, "%d|%s", nRet, "票箱开");
            return m_szErrStr;
        case SS_ERR_CANCELED:                                       // -25:操作被取消
            sprintf(m_szErrStr, "%d|%s", nRet, "操作被取消");
            return m_szErrStr;
        case SS_ERR_OTHER:                                          // -99999:其他错误
            sprintf(m_szErrStr, "%d|%s", nRet, "其他错误");
            return m_szErrStr;
        case SS_ERR_CHK_JAM:                                        // -12010:卡票
            sprintf(m_szErrStr, "%d|%s", nRet, "卡票");
            return m_szErrStr;
        case SS_ERR_CHK_NOPAPERINPRINTPOSITION:                     // -12011:打印位置没有票据
            sprintf(m_szErrStr, "%d|%s", nRet, "打印位置没有票据");
            return m_szErrStr;
        case SS_ERR_CHK_NOMEDIAPRESENT:                             // -12020:通道无票
            sprintf(m_szErrStr, "%d|%s", nRet, "通道无票");
            return m_szErrStr;
        case SS_ERR_CHK_NOIMAGE:                                    // -12021:无可用图像数据
            sprintf(m_szErrStr, "%d|%s", nRet, "无可用图像数据");
            return m_szErrStr;
        case SS_ERR_CHK_MEDIAPRESENT:                               // -12022:通道有票
            sprintf(m_szErrStr, "%d|%s", nRet, "通道有票");
            return m_szErrStr;
        case SS_ERR_CHK_UNKNOWTYPE:                                 // -12030:票据类型未知
            sprintf(m_szErrStr, "%d|%s", nRet, "票据类型未知");
            return m_szErrStr;
        case SS_ERR_CHK_IMPERFECT:                                  // -12031:票据缺角
            sprintf(m_szErrStr, "%d|%s", nRet, "票据缺角");
            return m_szErrStr;
        case SS_ERR_CHK_FORGED:                                     // -12032:伪票
            sprintf(m_szErrStr, "%d|%s", nRet, "伪票");
            return m_szErrStr;
        case SS_ERR_CHK_TAMPER:                                     // -12033:篡改票
            sprintf(m_szErrStr, "%d|%s", nRet, "篡改票");
            return m_szErrStr;
        case SS_ERR_CHK_NOOCRAREA:                                  // -12034:未设置OCR区域
            sprintf(m_szErrStr, "%d|%s", nRet, "未设置OCR区域");
            return m_szErrStr;
        case SS_ERR_CHK_INCOMPLETE:                                 // -12040:票据要素不全
            sprintf(m_szErrStr, "%d|%s", nRet, "票据要素不全");
            return m_szErrStr;
        case SS_ERR_CHK_TYPEERROR:                                  // -12050:票据类型不符
            sprintf(m_szErrStr, "%d|%s", nRet, "票据类型不符");
            return m_szErrStr;
        case SS_ERR_CHK_DIR:                                        // -12070:票据正反面放置错误
            sprintf(m_szErrStr, "%d|%s", nRet, "票据正反面放置错误");
            return m_szErrStr;
        case SS_ERR_CHK_LONGLENGTH:                                 // -12080:票据超长
            sprintf(m_szErrStr, "%d|%s", nRet, "票据超长");
            return m_szErrStr;
        case SS_ERR_CHK_READRFID:                                   // -12090:票据超长
            sprintf(m_szErrStr, "%d|%s", nRet, "票据超长");
            return m_szErrStr;
        case SS_ERR_CHK_EJECT_JAM:                                  // -12503:退票失败,卡票
            sprintf(m_szErrStr, "%d|%s", nRet, "退票失败,卡票");
            return m_szErrStr;
        case SS_ERR_CHK_SAVEIMAGE:                                  // -12601:图像保存失败
            sprintf(m_szErrStr, "%d|%s", nRet, "图像保存失败");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_PATAMETER:             // 501:鉴伪参数错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪参数错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_CONFIGFILE:            // 502:鉴伪配置文件格式错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪配置文件格式错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_CONFIGDATA:            // 503:鉴伪配置文件数据错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪配置文件数据错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_TYPENUMMANY:           // 504:鉴伪存单类型数量太多
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪存单类型数量太多");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_LOADLIBRARY:           // 505:鉴伪加载动态库失败
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪加载动态库失败");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_DECRYPT:               // 506:鉴伪解密错误，交互验证不通过
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪解密错误,交互验证不通过");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_DESKEWCROP:            // 507:鉴伪纠偏裁剪错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪纠偏裁剪错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_WIDTHHEIGHT:           // 508:鉴伪票据图像宽度高度不符
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪票据图像宽度高度不符");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_BILLTYPE:              // 509:鉴伪票据类型错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪票据类型错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_BASEPOINT:             // 510:鉴伪定位基准点失败
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪定位基准点失败");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_FRONTREAR:             // 511:鉴伪正反面放反
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪正反面放反");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_MALLOCBUFFER:          // 512:鉴伪内存申请错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪内存申请错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NONETEMPLATE:          // 513:鉴伪没有模板数据
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪没有模板数据");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NULLDATA:              // 514:鉴伪没有图像数据
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪没有图像数据");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NONEFEATURE:           // 515:鉴伪没有特征数据
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪没有特征数据");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_WATERMARK:             // 516:鉴伪水印错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪水印错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_ANGLEBIG:              // 517:鉴伪倾斜角度大
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪倾斜角度大");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_FINDRECT:              // 518:鉴伪查找区域失败
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪查找区域失败");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NONEFUNC:              // 600:鉴伪缺少功能
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪缺少功能");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_OHTER:                 // 700:鉴伪其它错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪其它错误");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_PARAS:                         // -801:OCR参数错误
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR参数错误");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_LOAD_CONFIG_FILE:              // -802:OCR加载主动态库配置文件错误
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR加载主动态库配置文件错误");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_LOAD_CONFIG_INFO:              // -803:OCR主配置文件配置文件信息错误
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR主配置文件配置文件信息错误");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_LOAD_DLL:                      // -804:OCR加载识别动态库失败
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR加载识别动态库失败");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_GET_DLL_PROC:                  // -805:OCR获取导出识别函数失败
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR获取导出识别函数失败");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_DLL_INIT:                      // -806:OCR识别动态库初始化函数返回值错误
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR识别动态库初始化函数返回值错误");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_RECOG:                         // -807:OCR识别失败
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR识别失败");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_OHTER:                         // -900:OCR其它错误
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR其它错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_PATAMETER:             // -1001:支票鉴伪参数错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪参数错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_ANGLEBING:             // -1002:支票鉴伪倾斜角度大
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪倾斜角度大");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_QUALITY:               // -1003:支票鉴伪质量检测不合格
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪质量检测不合格");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_TABLESIZE:             // -1004:支票鉴伪表格大小不符合要求
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪表格大小不符合要求");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_BILLTYPE:              // -1005:支票鉴伪票据类型错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪票据类型错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_IMAGEFLIP:             // -1006:支票鉴伪图像方向颠倒
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪图像方向颠倒");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_IMAGEDARK:             // -1007:支票鉴伪红外发射过暗
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪红外发射过暗");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_INCOMPLETE:            // -1008:支票鉴伪票面扫描不完整
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪票面扫描不完整");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_LOCATION:              // -1009:支票鉴伪定位失败
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪定位失败");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_FRONTREAR:             // -1010:支票鉴伪图像正面与反面放反
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪图像正面与反面放反");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_BRTIRTR:               // -1030:支票鉴伪红外透射亮度异常
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪红外透射亮度异常");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_DECRYPT:               // -1101:支票鉴伪解密错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪解密错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_DESKEWCROP:            // -1102:支票鉴伪纠偏错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪纠偏错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_DESKEWCROPNOBASEIMAGE: // -1103:支票鉴伪无纠偏基准图像
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪无纠偏基准图像");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_CHEQUETYPE:            // -1200:支票鉴伪票据类型错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪票据类型错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_OHTER:                 // -1300:支票鉴伪其他错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪其他错误");
            return m_szErrStr;
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义值");
            return m_szErrStr;
    }
}
//////////////////////////////////////////////////////////////////////////






