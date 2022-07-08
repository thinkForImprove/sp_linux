/***************************************************************
* 文件名称：DevCPR_BT8500M.cpp
* 文件描述：BT-8500M票据发放模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevCPR_BT8500M.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>
#include <QTextCodec>

static const char *ThisModule = "DevCPR_BT8500M.cpp";

//////////////////////////////////////////////////////////////////////////

CDevCPR_BT8500M::CDevCPR_BT8500M() : m_devBT8500M(LOG_NAME_DEVCPR)
{
    SetLogFile(LOG_NAME_DEVCPR, ThisModule);  // 设置日志文件名和错误发生的文件

    memset(&m_stDevStatOLD, 0x00, sizeof(DEVICESTATUS));

    m_nGetStatErrOLD = PTR_SUCCESS;
    m_nGetOpenErrOLD = PTR_SUCCESS;
}

CDevCPR_BT8500M::~CDevCPR_BT8500M()
{
    Close();
}

// 打开与设备的连接
int CDevCPR_BT8500M::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // 建立CPR连接
    if (m_devBT8500M.IsDeviceOpen() == TRUE)
    {
        m_devBT8500M.DeviceClose(FALSE);
    }

    if ((nRet = m_devBT8500M.DeviceOpen((LPSTR)lpMode)) != IMP_SUCCESS)
    {
        if (m_nGetOpenErrOLD != nRet)
        {
            Log(ThisModule, __LINE__, "建立CPR连接: ->DeviceOpen(%s) Fail, ErrCode=%d, Return %s.",
                        lpMode, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            m_nGetOpenErrOLD = nRet;
        }
        return ConvertErrorCode(nRet);
    }
    m_nGetOpenErrOLD = nRet;

    m_stdOpenMode.append(lpMode == nullptr ? "" : lpMode);

    return PTR_SUCCESS;
}

// 关闭与设备的连接
int CDevCPR_BT8500M::Close()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_devBT8500M.DeviceClose();
    Log(ThisModule, __LINE__, "关闭CPR连接: ->DeviceClose() Succ, Return %s.",
        ConvertErrCodeToStr(PTR_SUCCESS));

    return PTR_SUCCESS;
}

// 设备复位
int CDevCPR_BT8500M::Reset()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    if ((nRet = m_devBT8500M.nResetDevEx(1)) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "CPR设备初始化: ->nResetDevEx(%d) Fail, ErrCode=%d, Return %s.",
            1, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    /*if ((nRet = m_devBT8500M.DeviceOpen((LPSTR)m_stdOpenMode.c_str())) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "CPR设备初始化后重新建立连接: ->DeviceOpen(%s) Fail, ErrCode=%d, Return %s.",
            m_stdOpenMode.c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }*/

    return PTR_SUCCESS;
}

// 设备复位
int CDevCPR_BT8500M::ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    if (enMediaAct == MEDIA_CTR_STACK)  // 硬复位
    {
        if ((nRet = m_devBT8500M.nResetDevEx(0)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "CPR设备初始化: 硬复位: ->nResetDevEx(%d) Fail, ErrCode=%d, Return %s.",
                0, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
        m_devBT8500M.DeviceClose();
        if ((nRet = m_devBT8500M.DeviceOpen((LPSTR)m_stdOpenMode.c_str())) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "CPR设备初始化后重新建立连接: ->DeviceOpen(%s) Fail, ErrCode=%d, Return %s.",
                m_stdOpenMode.c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (enMediaAct == MEDIA_CTR_ALARM)  // 软复位
    {
        if ((nRet = m_devBT8500M.nResetDevEx(1)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "CPR设备初始化: 软复位: ->nResetDevEx(%d) Fail, ErrCode=%d, Return %s.",
                1, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    {
        Log(ThisModule, __LINE__, "设备复位Ex: Input[%d] Invalid, Return %s.",
            enMediaAct, ERR_PTR_PARAM_ERR);
        return ERR_PTR_PARAM_ERR;
    }

    return PTR_SUCCESS;
}

// 取设备状态
int CDevCPR_BT8500M::GetStatus(DEVPTRSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;
    INT    nDevStat[12];

    stStatus.Clear();

    // 设置设备初始状态
    stStatus.wDevice = DEV_STAT_HWERROR;
    stStatus.wMedia = MEDIA_STAT_UNKNOWN;
    for (INT i = 0; i < 16; i ++)
        stStatus.wPaper[i] = PAPER_STAT_UNKNOWN;
    stStatus.wToner = TONER_STAT_UNKNOWN;
    stStatus.wInk = INK_STAT_NOTSUPP;
    for (INT i = 0; i < 16; i ++)
    {
        stStatus.stRetract[i].wBin = RETRACT_STAT_UNKNOWN;
    }

    if (m_devBT8500M.IsDeviceOpen() != TRUE)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (m_nGetStatErrOLD != ERR_PTR_NOT_OPEN)
        {
            Log(ThisModule, __LINE__, "读取CPR设备状态: ->IsDeviceOpen() Is FALSE, Device Not Open, Return %s.",
                ConvertErrCodeToStr(ERR_PTR_NOT_OPEN));
            m_nGetStatErrOLD = ERR_PTR_NOT_OPEN;
        }

        stStatus.wDevice = DEV_STAT_OFFLINE;
        return ERR_PTR_NOT_OPEN;
    }

    // 取设备状态
    DEVICESTATUS stDevStat;
    if ((nRet = m_devBT8500M.nGetDevStatus(stDevStat)) != IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (m_nGetStatErrOLD != ConvertErrorCode(nRet))
        {
            Log(ThisModule, __LINE__, "读取CPR设备状态: ->nGetDevStatus() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            m_nGetStatErrOLD = ConvertErrorCode(nRet);
        }

        if (nRet == PRINTER_ERROR_OPEN_PORT ||      // 打开端口失败
            nRet == PRINTER_ERROR_COMMUNICATION)    // 端口通讯失败
        {
            stStatus.wDevice = DEV_STAT_OFFLINE;
        }

        return ConvertErrorCode(nRet);
    }

    // 解析获取的设备状态

    // Device状态
    if (//stDevStat.bError != 0 ||                    // 错误状态
        stDevStat.bUnitVolTooHigh != 0 ||           // 整机电压偏高
        stDevStat.bUnitVolTooLow != 0 ||            // 整机电压偏低
        stDevStat.bFeederOpen != 0 ||               // 分纸模块上盖打开
        stDevStat.bPrintHeadOpen != 0 ||            // 打印模块微动开关错
        stDevStat.bChannelError != 0 ||             // 通道抬起压下错误
        stDevStat.bScanUnitOpen != 0 ||             // 打印后扫描模块上盖打开
        stDevStat.bFeedingError != 0 ||             // 分纸错误
        stDevStat.bCommunicationError != 0 ||       // 扫描模块通讯错
        stDevStat.bPaperLengthError != 0 ||         // 票据长度错
        stDevStat.bDoubleDocError != 0 ||           // 重张错
        stDevStat.bCorrectionError != 0 ||          // 纠偏错
        stDevStat.bBeltLackError != 0 ||            // 缺碳带错
        stDevStat.bTempVolError != 0 ||             // 温度电压异常错
        stDevStat.bBeltRecycleError != 0 ||         // 碳带回收错
        stDevStat.bPrinterHeadError != 0 ||         // 打印头抬起压下错误
        stDevStat.bGateError != 0 ||                // 闸门错
        stDevStat.bPaperIncompleteError != 0 ||     // 缺角错
        stDevStat.bOCRCoverOpen != 0 ||             // 打印前扫描模块上盖打开
        stDevStat.bSensorCalibrateError != 0 ||     // 传感器校验是否失败
        stDevStat.bOutputBoxPressureError != 0 ||   // 回收压板错误
        stDevStat.bStampError != 0 ||               // 印章错
        stDevStat.bScanUnitError != 0)              // 扫描模块状态错
    {
        stStatus.wDevice = DEV_STAT_HWERROR;
    } else
    {
        if ((stDevStat.bError != 0 &&               // 错误状态
            (stDevStat.bChannelHasPaper != 0 ||     // 通道有纸不算HWErr
             stDevStat.bPaperJamError != 0)) ||     // 塞纸错误不进行Device设置
            stDevStat.bError != 1)
        {
            stStatus.wDevice = DEV_STAT_ONLINE;
        }
    }

    if (stStatus.wDevice == DEV_STAT_HWERROR ||
        stStatus.wDevice == DEV_STAT_ONLINE)
    {
        // MEDIA状态
        if (stDevStat.bPaperJamError != 0)          // 塞纸错误
        {
            stStatus.wMedia = MEDIA_STAT_JAMMED;
            stStatus.wDevice = DEV_STAT_HWERROR;
        } else
        if (stDevStat.bWaitForScan != 0 ||          // 等待启动打印前扫描
            stDevStat.bWaitForPrinting != 0 ||      // 等待启动打印
            stDevStat.bWaitForPaperOut != 0 ||      // 等待出票
            stDevStat.bChannelHasPaper != 0)        // 通道有纸
        {
            stStatus.wMedia = MEDIA_STAT_PRESENT;
        } else
        {
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;// 通道内无介质
        }

        if (stDevStat.bStackerHavePaper == 1 &&   // 出票箱/出票口有纸 && 等待出票结束    // 40-00-00-00(FT#0017)
            stDevStat.bWaitForPaperOut == 0)                                         // 40-00-00-00(FT#0017)
        {
            stStatus.wMedia = MEDIA_STAT_ENTERING;
        }
    }

    if (stStatus.wDevice == DEV_STAT_ONLINE)
    {
        if (stDevStat.bPrinting != 0)               // 正在打印
        {
            stStatus.wDevice = DEV_STAT_BUSY;
        }

        // 票箱纸状态
        for (INT i = 0; i < 16; i ++)               // 初始不支持
            stStatus.wPaper[i] = PAPER_STAT_NOTSUPP;
        if (stDevStat.bBox1Exist == 1)              // 票箱1是否存在
        {
            if (stDevStat.bBox1HavePaper == 1)      // 票箱1是否有票
            {
                stStatus.wPaper[0] = PAPER_STAT_FULL;
            } else
            {
                stStatus.wPaper[0] = PAPER_STAT_OUT;
            }
        }
        if (stDevStat.bBox2Exist == 1)              // 票箱2是否存在
        {
            if (stDevStat.bBox2HavePaper == 1)      // 票箱2是否有票
            {
                stStatus.wPaper[1] = PAPER_STAT_FULL;
            } else
            {
                stStatus.wPaper[1] = PAPER_STAT_OUT;
            }
        }
        if (stDevStat.bBox3Exist == 1)              // 票箱3是否存在
        {
            if (stDevStat.bBox3HavePaper == 1)      // 票箱3是否有票
            {
                stStatus.wPaper[2] = PAPER_STAT_FULL;
            } else
            {
                stStatus.wPaper[2] = PAPER_STAT_OUT;
            }
        }
        if (stDevStat.bBox4Exist == 1)              // 票箱4是否存在
        {
            if (stDevStat.bBox4HavePaper == 1)      // 票箱4是否有票
            {
                stStatus.wPaper[3] = PAPER_STAT_FULL;
            } else
            {
                stStatus.wPaper[3] = PAPER_STAT_OUT;
            }
        }

        // 碳带状态
        if (stDevStat.bBeltOverError != 0)          // 碳带将尽错
        {
            stStatus.wToner = TONER_STAT_LOW;
        } else
        {
            stStatus.wToner = TONER_STAT_FULL;
        }

        // 票箱状态
        if (stDevStat.bRetrackError != 0)           // 废票箱(回收箱)未移除
        {
            stStatus.stRetract[0].wBin = RETRACT_STAT_OK;
        } else
        {
            stStatus.stRetract[0].wBin = RETRACT_STAT_MISSING;
        }
    }

    // 状态出现变化时,打印LOG
    DiffDevStat(stDevStat, m_stDevStatOLD);

    memcpy(&m_stDevStatOLD, &stDevStat, sizeof(DEVICESTATUS));

    return PTR_SUCCESS;
}

// 打印字串(无指定打印坐标)
int CDevCPR_BT8500M::PrintData(const char *pStr, unsigned long ulDataLen)
{
    #define JSON_GET(JSON, KEY, VALUE) \
        if (JSON.Get(KEY, VALUE) != true) \
        {\
            Log(ThisModule, __LINE__, "打印字串: ->JSON.Get(%s) Fail, Return %d.", \
                KEY, ERR_PTR_JSON_ERR); \
            return ERR_PTR_JSON_ERR; \
        }

    #define SET_KEYNAME(KEY, CNT, DEST) \
        memset(DEST, 0x00, sizeof(DEST)); \
        sprintf(DEST, "%s%d", KEY, CNT);

    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;
    CJsonObject cJsonPrtIn;
    CJsonObject cJsonSndData, cJsonType, cJsonTmp, cJsonFields;
    std::string stdTmp = "", stdTmp2 = "";
    CHAR szKeyName[64] = { 0x00 };
    WORD wCount = 0, wValue = 0;
    BOOL bIsPrint = FALSE;

    // 入参Check
    if (pStr == nullptr || strlen(pStr) < 1)
    {
        Log(ThisModule, __LINE__, "打印字串: Input[%s] IsNull, Return %s.",
            pStr, ERR_PTR_PARAM_ERR);
        return ERR_PTR_PARAM_ERR;
    }

    // 加载/解析JSON
    if (cJsonPrtIn.Parse(pStr) != true)
    {
        Log(ThisModule, __LINE__, "打印字串: ->JSON.Parse(%s) Fail, Return %s.",
            pStr, ERR_PTR_JSON_ERR);
        return ERR_PTR_JSON_ERR;
    }

    // 取打印数据
    cJsonSndData.Clear();
    JSON_GET(cJsonPrtIn, JSON_KEY_MEDIA_WIDTH, wValue);     // 取 票据宽
    cJsonSndData.Add(BT_JSON_PRINTWIDTH, wValue);
    JSON_GET(cJsonPrtIn, JSON_KEY_MEDIA_HEIGHT, wValue);    // 取 票据高
    cJsonSndData.Add(BT_JSON_PRINTHEIGHT, wValue);

    for(INT a = 0; a < 4; a ++)
    {
        if (a == 0) // 取文本
        {
            if (cJsonPrtIn.Get(JSON_KEY_IDEN_TYPE_TEXT, cJsonType) != true)
            {
                continue;
            }
        } else
        if (a == 1) // 取图片
        {
            if (cJsonPrtIn.Get(JSON_KEY_IDEN_TYPE_PIC, cJsonType) != true)
            {
                continue;
            }
        } else
        if (a == 2) // 取条码
        {
            if (cJsonPrtIn.Get(JSON_KEY_IDEN_TYPE_BAR, cJsonType) != true)
            {
                continue;
            }
        } else
        if (a == 3) // 取磁码
        {
            if (cJsonPrtIn.Get(JSON_KEY_IDEN_TYPE_MICR, cJsonType) != true)
            {
                continue;
            }
        }

        JSON_GET(cJsonType, JSON_KEY_IDEN_CNT, wCount);    // 取 项数
        if (wCount < 1)
        {
            Log(ThisModule, __LINE__, "打印字串: ->JSON.Get(%s) = %d < 1, Return %s.",
                JSON_KEY_IDEN_CNT, wCount, ERR_PTR_JSON_ERR);
            return ERR_PTR_JSON_ERR;
        }

        cJsonFields.Clear();
        for (INT i = 1; i <= wCount; i ++)   // 循环
        {
            cJsonTmp.Clear();
            //-- ALL --
            // 起始坐标X(单位:MM)
            wValue = 0;
            SET_KEYNAME(JSON_KEY_START_X, i, szKeyName);
            JSON_GET(cJsonType, szKeyName, wValue);
            cJsonTmp.Add("x", wValue);
            // 起始坐标Y(单位:MM)
            wValue = 0;
            SET_KEYNAME(JSON_KEY_START_Y, i, szKeyName);
            JSON_GET(cJsonType, szKeyName, wValue);
            cJsonTmp.Add("y", wValue);
            // 项名
            stdTmp.clear();
            SET_KEYNAME(JSON_KEY_IDEN_NAME, i, szKeyName);
            JSON_GET(cJsonType, szKeyName, stdTmp);
            // 字符集转换(GBK->UTF8)
            QTextCodec *codec = QTextCodec::codecForLocale();       // 取当前字符集
            QTextCodec *qtUTF8 = QTextCodec::codecForName("UTF-8"); // UTF8字符集
            QTextCodec *qtGBK = QTextCodec::codecForName("GB18030");// GBK字符集
            QTextCodec::setCodecForLocale(qtGBK);                   // 设置当前为GBK字符集
            QString qsText = QString::fromLocal8Bit((LPSTR)stdTmp.c_str()); // 导入GBK数据
            QTextCodec::setCodecForLocale(qtUTF8);                  // 设置当前为UTF8字符集
            QByteArray tmpData = qsText.toUtf8();                   // 转换为UTF8格式数据
            QTextCodec::setCodecForLocale(codec);                   // 还原当前字符集
            stdTmp.clear();
            stdTmp.append(tmpData.data());

            if (a == 0) // 文本
            {
                cJsonTmp.Add("content", stdTmp);
                // 可用宽(单位:0.1MM->PX)
                wValue = 0;
                SET_KEYNAME(JSON_KEY_AREA_WIDTH, i, szKeyName);
                JSON_GET(cJsonType, szKeyName, wValue);
                cJsonTmp.Add("width", MM2PX(wValue, m_usDPIx));
                // 可用高(单位:0.1MM->PX)
                wValue = 0;
                SET_KEYNAME(JSON_KEY_AREA_HEIGHT, i, szKeyName);
                JSON_GET(cJsonType, szKeyName, wValue);
                cJsonTmp.Add("height", MM2PX(wValue, m_usDPIx));
                // 字体
                stdTmp.clear();
                SET_KEYNAME(JSON_KEY_TEXT_FONT, i, szKeyName);
                JSON_GET(cJsonType, szKeyName, stdTmp);
                if (m_stFontList.GetFontPath((LPSTR)stdTmp.c_str()) != nullptr)
                {
                    cJsonTmp.Add("font", m_stFontList.GetFontPath((LPSTR)stdTmp.c_str()));
                } else
                {
                    cJsonTmp.Add("font", stdTmp);
                }
                // 加入节点下
                cJsonFields.Add(cJsonTmp);
            } else
            if (a == 1) // 图片
            {
                cJsonTmp.Add("path", stdTmp);
                // Zoom
                float fZoom = 1.0;
                SET_KEYNAME(JSON_KEY_PIC_ZOOM, i, szKeyName);
                JSON_GET(cJsonType, szKeyName, fZoom);
                cJsonTmp.Add("zoom", fZoom);
                // 加入节点下
                cJsonFields.Add(cJsonTmp);
            } else
            if (a == 2) // 条码
            {
                cJsonTmp.Add("Barcode", stdTmp);
                // 加入节点下
                cJsonSndData["Magcode"].Add(cJsonTmp);
            } else
            if (a == 3) // 磁码
            {
                cJsonTmp.Add("content", stdTmp);
                // 加入节点下
                cJsonSndData["Magcode"].Add(cJsonTmp);
            }
        }

        if (cJsonFields.IsNull() != true && cJsonFields.IsEmpty() != true)
        {
            if (a == 0)
            {
                cJsonSndData.Add("Text", cJsonFields);
            } else
            if (a == 1)
            {
                cJsonSndData.Add("Picture", cJsonFields);
            }
        }
    }

    if (cJsonSndData.ToString().length() > 0)
    {
        if ((nRet = m_devBT8500M.nPrintAndScan((LPSTR)cJsonSndData.ToString().c_str(), 1, 0)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打印字串: ->nPrintAndScan(JSON, 1, 0) Fail, ErrCode=%d, Return %s.",
                /*cJsonSndData.ToString().c_str(), */nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    {
        Log(ThisModule, __LINE__, "打印字串: ->Prt JSON[%s] Is NULL, Not Prints.",
            cJsonSndData.ToString().c_str());
    }

    return PTR_SUCCESS;
}

// ReadImage获取
int CDevCPR_BT8500M::ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut)
{
    #define JSON_GET(JSON, KEY, VALUE) \
        if (JSON.Get(KEY, VALUE) != true) \
        {\
            Log(ThisModule, __LINE__, "扫描信息获取: 取JSON数据: ->JSON.Get(%s) Fail, Return %d.", \
                KEY, ERR_PTR_JSON_ERR); \
            return ERR_PTR_JSON_ERR; \
        }

    #define SET_KEYNAME(KEY, CNT, DEST) \
        memset(DEST, 0x00, sizeof(DEST)); \
        sprintf(DEST, "%s%d", KEY, CNT);

    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;
    CJsonObject cJsonScanIn, cJsonScanOut;
    CJsonObject cJsonSndData, cJsonRcvData, cJsonTmpData, cJsonFields;
    std::string stdImageFront = "";
    std::string stdImageBack = "";
    std::string stdTmp = "";
    CHAR szKeyName[64] = { 0x00 };
    BOOL bIsSacnImage = FALSE;
    INT i = 0;

    // CHECK
    if (stImageIn.lpData == nullptr)
    {
        Log(ThisModule, __LINE__, "扫描信息获取: CKECK: stImageIn.lpData Is NULL, Return %s.",
            stImageIn.lpData, ERR_PTR_PARAM_ERR);
        return ERR_PTR_PARAM_ERR;
    }

    // 加载/解析JSON
    if (cJsonScanIn.Parse(stImageIn.lpData) != true)
    {
        Log(ThisModule, __LINE__, "扫描信息获取: 加载/解析JSON: ->JSON.Parse(%s) Fail, Return %s.",
            stImageIn.lpData, ERR_PTR_JSON_ERR);
        return ERR_PTR_JSON_ERR;
    }

    stImageOut.Clear();
    cJsonScanOut.Clear();
    stImageOut.wInMode = stImageIn.wInMode;
    stImageOut.wResult = READIMAGE_RET_MISSING; // 返回处理结果初始MISSING
    stImageOut.lpData = nullptr;

    // 扫描图像
    if (stImageIn.wInMode == IMAGE_MODE_FRONT ||
        stImageIn.wInMode == IMAGE_MODE_BACK ||
        stImageIn.wInMode == IMAGE_MODE_FRONT + IMAGE_MODE_BACK)
    {
        if ((stImageIn.wInMode & IMAGE_MODE_FRONT) == IMAGE_MODE_FRONT)
        {
            JSON_GET(cJsonScanIn, JSON_KEY_IMAGE_FRONT_PATH, stdImageFront);
        }
        if ((stImageIn.wInMode &IMAGE_MODE_BACK) == IMAGE_MODE_BACK)
        {
            JSON_GET(cJsonScanIn, JSON_KEY_IMAGE_BACK_PATH, stdImageBack);
        }

        if ((nRet = m_devBT8500M.nGetCheckImage(
                                    (stdImageFront.empty() == true ? nullptr : (LPSTR)stdImageFront.c_str()),
                                    (stdImageBack.empty() == true ? nullptr : (LPSTR)stdImageBack.c_str()))) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "扫描信息获取: 图像扫描: ->nGetCheckImage() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
        stImageOut.wResult = READIMAGE_RET_OK;
    }

    // 获取票据号并鉴伪
    if (stImageIn.wInMode == IMAGE_MODE_CODELINE)
    {
        cJsonSndData.Clear();
        WORD wCount = 0, wValue = 0, wUseJson = 0;

        JSON_GET(cJsonScanIn, JSON_KEY_USE_AREA, wUseJson);    // 取 JSON使用方式

        if (wUseJson == 0)  // 设备动态库默认方式
        {
            cJsonSndData.Clear();
            cJsonSndData.Add(BT_JSON_USEAREA, 0);
        } else
        {
            JSON_GET(cJsonScanIn, JSON_KEY_IDEN_CNT, wCount);    // 取 项数
            if (wCount < 1)
            {
                Log(ThisModule, __LINE__, "扫描信息获取: 取JSON项数: ->JSON.Get(%s) = %d < 1, Return %s.",
                    JSON_KEY_IDEN_CNT, wCount, ERR_PTR_JSON_ERR);
                return ERR_PTR_JSON_ERR;
            }

            cJsonSndData.Clear();
            cJsonSndData.Add(BT_JSON_USEAREA, 1);
            JSON_GET(cJsonScanIn, JSON_KEY_MEDIA_WIDTH, wValue);    // 取 票据宽
            cJsonSndData.Add(BT_JSON_CHECKWIDTH, wValue);
            JSON_GET(cJsonScanIn, JSON_KEY_MEDIA_HEIGHT, wValue);    // 取 票据高
            cJsonSndData.Add(BT_JSON_CHECKHEIGHT, wValue);

            cJsonFields.Clear();
            for (INT i = 1; i <= wCount; i ++)   // 循环
            {
                cJsonTmpData.Clear();
                // 项名
                stdTmp.clear();
                SET_KEYNAME(JSON_KEY_IDEN_NAME, i, szKeyName);
                JSON_GET(cJsonScanIn, szKeyName, stdTmp);
                cJsonTmpData.Add(BT_JSON_FIELDNAME, stdTmp);
                // 起始坐标X(单位:MM)
                wValue = 0;
                SET_KEYNAME(JSON_KEY_START_X, i, szKeyName);
                JSON_GET(cJsonScanIn, szKeyName, wValue);
                cJsonTmpData.Add(BT_JSON_STARTX, wValue);
                // 起始坐标Y(单位:MM)
                wValue = 0;
                SET_KEYNAME(JSON_KEY_START_Y, i, szKeyName);
                JSON_GET(cJsonScanIn, szKeyName, wValue);
                cJsonTmpData.Add(BT_JSON_STARTY, wValue);
                // 可用宽(单位:MM)
                wValue = 0;
                SET_KEYNAME(JSON_KEY_AREA_WIDTH, i, szKeyName);
                JSON_GET(cJsonScanIn, szKeyName, wValue);
                cJsonTmpData.Add(BT_JSON_AREAWIDTH, wValue);
                // 可用高(单位:MM)
                wValue = 0;
                SET_KEYNAME(JSON_KEY_AREA_HEIGHT, i, szKeyName);
                JSON_GET(cJsonScanIn, szKeyName, wValue);
                cJsonTmpData.Add(BT_JSON_AREAHEIGHT, wValue);
                // 加入节点下
                cJsonFields.Add(cJsonTmpData);
            }

            cJsonSndData.Add("Fields", cJsonFields);
        }

        // 设置票面OCR识别区域
        if ((nRet = m_devBT8500M.nSetCheckOCRArea((LPSTR)cJsonSndData.ToString().c_str())) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "扫描信息获取: 设置票面OCR识别区域: ->nSetCheckOCRArea(%s) Fail, ErrCode=%d, Return %s.",
                cJsonSndData.ToString().c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }

        // 取票据鉴伪和OCR识别结果
        CHAR szRcvBuffer[1024 * 10] = { 0x00 };
        if ((nRet = m_devBT8500M.nGetCheckInfo(szRcvBuffer, 1024 * 10)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "扫描信息获取: 取票据鉴伪和OCR识别结果: ->nGetCheckInfo() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        } else
        {
            Log(ThisModule, __LINE__, "扫描信息获取: 取票据鉴伪和OCR识别结果: ->nGetCheckInfo() succ, RcvData = %s.", szRcvBuffer);
        }

        // 解析结果
        if (cJsonRcvData.Parse(szRcvBuffer) != true)// 加载/解析JSON
        {
            Log(ThisModule, __LINE__, "扫描信息获取: ->JSON.Parse(%s) Fail, Return %s.",
                szRcvBuffer, ERR_PTR_JSON_ERR);
            return ERR_PTR_JSON_ERR;
        }

        wValue = 0;
        JSON_GET(cJsonRcvData, BT_JSON_ISTRUE, wValue);
        wValue == 1 ? stImageOut.wResult = READIMAGE_RET_OK : stImageOut.wResult = READIMAGE_RET_MISSING;

        JSON_GET(cJsonRcvData, BT_JSON_FIELS, cJsonTmpData);

        wCount = 0;
        if (cJsonTmpData.IsArray() != true || (wCount = cJsonTmpData.GetArraySize()) == 0)
        {
            Log(ThisModule, __LINE__, "扫描信息获取: ->GetArraySize(RcvJson:%s) Fail, Return %s.",
                cJsonRcvData.ToString().c_str(), nRet, ERR_PTR_JSON_ERR);
            return ERR_PTR_JSON_ERR;
        }

        for (i = 0; i < wCount; i ++)
        {
            SET_KEYNAME(JSON_KEY_IDEN_NAME, i + 1, szKeyName);
            cJsonScanOut.Add(szKeyName, cJsonTmpData[i](BT_JSON_FIELDNAME));
            SET_KEYNAME(JSON_KEY_IDEN_VALUE, i + 1, szKeyName);
            cJsonScanOut.Add(szKeyName, cJsonTmpData[i](BT_JSON_INFO));
        }
        cJsonScanOut.Add(JSON_KEY_IDEN_CNT, i);
    }

    // 获取票据号
    if ((stImageIn.wInMode & IMAGE_MODE_UPPER) == IMAGE_MODE_UPPER ||
        (stImageIn.wInMode & IMAGE_MODE_LOWER) == IMAGE_MODE_LOWER ||
        (stImageIn.wInMode & IMAGE_MODE_EXTERNAL) == IMAGE_MODE_EXTERNAL ||
        (stImageIn.wInMode & IMAGE_MODE_AUX) == IMAGE_MODE_AUX)
    {
        cJsonSndData.Clear();
        WORD wCount = 0, wValue = 0, wUseJson = 0;
        std::string stdNoteNo = "";     // 返回票号标记字符串

        JSON_GET(cJsonScanIn, JSON_KEY_USE_AREA, wUseJson);    // 取 JSON使用方式

        if (wUseJson == 0)  // 设备动态库默认方式
        {
            cJsonSndData.Clear();
            cJsonSndData.Add(BT_JSON_USEAREA, 0);
        } else
        {
            JSON_GET(cJsonScanIn, JSON_KEY_IDEN_CNT, wCount);    // 取 项数
            if (wCount < 1)
            {
                Log(ThisModule, __LINE__, "扫描信息获取: ->JSON.Get(%s) = %d < 1, Return %s.",
                    JSON_KEY_IDEN_CNT, wCount, ERR_PTR_JSON_ERR);
                return ERR_PTR_JSON_ERR;
            }

            cJsonSndData.Clear();
            cJsonSndData.Add(BT_JSON_USEAREA, 1);
            JSON_GET(cJsonScanIn, JSON_KEY_MEDIA_WIDTH, wValue);    // 取 票据宽
            cJsonSndData.Add(BT_JSON_CHECKWIDTH, wValue);
            JSON_GET(cJsonScanIn, JSON_KEY_MEDIA_HEIGHT, wValue);    // 取 票据高
            cJsonSndData.Add(BT_JSON_CHECKHEIGHT, wValue);
            // 项名
            stdTmp.clear();
            SET_KEYNAME(JSON_KEY_IDEN_NAME, 1, szKeyName);
            JSON_GET(cJsonScanIn, szKeyName, stdTmp);
            cJsonSndData.Add(BT_JSON_FIELDNAME, stdTmp);
            stdNoteNo = stdTmp;
            // 起始坐标X(单位:MM)
            wValue = 0;
            SET_KEYNAME(JSON_KEY_START_X, 1, szKeyName);
            JSON_GET(cJsonScanIn, szKeyName, wValue);
            cJsonSndData.Add(BT_JSON_STARTX, wValue);
            // 起始坐标Y(单位:MM)
            wValue = 0;
            SET_KEYNAME(JSON_KEY_START_Y, 1, szKeyName);
            JSON_GET(cJsonScanIn, szKeyName, wValue);
            cJsonSndData.Add(BT_JSON_STARTY, wValue);
            // 可用宽(单位:MM)
            wValue = 0;
            SET_KEYNAME(JSON_KEY_AREA_WIDTH, 1, szKeyName);
            JSON_GET(cJsonScanIn, szKeyName, wValue);
            cJsonSndData.Add(BT_JSON_AREAWIDTH, wValue);
            // 可用高(单位:MM)
            wValue = 0;
            SET_KEYNAME(JSON_KEY_AREA_HEIGHT, 1, szKeyName);
            JSON_GET(cJsonScanIn, szKeyName, wValue);
            cJsonSndData.Add(BT_JSON_AREAHEIGHT, wValue);
        }

        // 设置票据号
        if ((nRet = m_devBT8500M.nSetCheckNumOCRArea((LPSTR)cJsonSndData.ToString().c_str())) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "扫描信息获取: 设置票据号获取范围: ->nSetCheckNumOCRArea(%s) Fail, ErrCode=%d, Return %s.",
                cJsonSndData.ToString().c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }

        // 取票号识别结果
        CHAR szRcvBuffer[1024 * 10] = { 0x00 };
        if ((nRet = m_devBT8500M.nGetCheckNumFromArea(szRcvBuffer, 1024 * 10, nullptr)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "扫描信息获取: 取票号识别结果: ->nGetCheckNumFromArea() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        } else
        {
            Log(ThisModule, __LINE__, "扫描信息获取: 取票号识别结果: ->nGetCheckNumFromArea() succ, RcvData = %s.", szRcvBuffer);
        }

        // 解析结果
        if (cJsonRcvData.Parse(szRcvBuffer) != true)// 加载/解析JSON
        {
            Log(ThisModule, __LINE__, "扫描信息获取: 解析票号识别结果:->JSON.Parse(%s) Fail, Return %s.",
                szRcvBuffer, ERR_PTR_JSON_ERR);
            return ERR_PTR_JSON_ERR;
        }

        JSON_GET(cJsonRcvData, BT_JSON_FIELS, cJsonTmpData);

        wCount = 0;
        if (cJsonTmpData.IsArray() != true || (wCount = cJsonTmpData.GetArraySize()) == 0)
        {
            Log(ThisModule, __LINE__, "扫描信息获取: ->GetArraySize(RcvJson:%s) Fail, Return %s.",
                cJsonRcvData.ToString().c_str(), nRet, ERR_PTR_JSON_ERR);
            return ERR_PTR_JSON_ERR;
        }

        for (INT i = 0; i < wCount; i ++)
        {
            if (cJsonTmpData[i](BT_JSON_FIELDNAME).compare(BT_JSON_CHECKNO) == 0)   // 取票号
            {
                cJsonScanOut.Add(JSON_KEY_IDEN_NAME, stdNoteNo);//cJsonTmpData[i](BT_JSON_FIELDNAME));
                cJsonScanOut.Add(JSON_KEY_IDEN_VALUE, cJsonTmpData[i](BT_JSON_INFO));
                stImageOut.wResult = READIMAGE_RET_OK;
                break;
            }
        }
    }

    // 获取票据RFID
    if (stImageIn.wInMode == IMAGE_MODE_RFID)
    {
        WORD wCount = 0, wValue = 0;
        CHAR szRfidInfo[1024] = { 0x00 };

        if ((nRet = m_devBT8500M.nGetRfidInfo(szRfidInfo)) != 1)
        {
            Log(ThisModule, __LINE__, "扫描信息获取: ->nGetRfidInfo(%s) Fail, ErrCode=%d, Return %s.",
                cJsonSndData.ToString().c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
        cJsonScanOut.Add(JSON_KEY_RFID_DATA, szRfidInfo);
        stImageOut.wResult = READIMAGE_RET_OK;
    }

    INT nLen = cJsonScanOut.ToString().length();
    if (nLen > 0)
    {
        stImageOut.lpData = new CHAR[nLen + 1];
        memset(stImageOut.lpData, 0x00, nLen + 1);
        memcpy(stImageOut.lpData, (LPSTR)cJsonScanOut.ToString().c_str(), nLen);
    }

    return PTR_SUCCESS;
}

// 介质控制
int CDevCPR_BT8500M::MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;

    // MEDIA_CTR_EJECT: 票据退出(通道内退出)
    // MEDIA_CTR_CUT: 票据退出(打印位置退纸)
    // MEDIA_CTR_SKIP: 票据退出(打开闸门取票)
    // MEDIA_CTR_PERFORATE: 票据回收(通道内回收)
    // MEDIA_CTR_RETRACT: 票据回收(出口回收)
    // MEDIA_CTR_EXPEL: 开门
    // MEDIA_CTR_PARK: 关门

    if (enMediaAct == MEDIA_CTR_EJECT)  // 票据退出(通道内退出)
    {
        if ((nRet = m_devBT8500M.nMoveCheck(1, 0)) != IMP_SUCCESS)
        {
            if (nRet == PRINTER_ERROR_PAPERLACK)    // 缺纸(通道内无纸)
            {
                Log(ThisModule, __LINE__, "介质控制: 票据出: ->nMoveCheck(%d, %d) Fail, ErrCode=%d, Return %s.",
                    1, 0, nRet, ConvertErrCodeToStr(ERR_PTR_NO_MEDIA));
                return ERR_PTR_NO_MEDIA;
            } else
            {
                Log(ThisModule, __LINE__, "介质控制: 票据出: ->nMoveCheck(%d, %d) Fail, ErrCode=%d, Return %s.",
                    1, 0, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }
    } else
    if (enMediaAct == MEDIA_CTR_CUT)  // 票据退出(打印位置退纸)
    {
        if ((nRet = m_devBT8500M.nSetCheckOut(1, 0)) != IMP_SUCCESS)
        {
            if (nRet == PRINTER_ERROR_PAPERLACK)    // 缺纸(通道内无纸)
            {
                Log(ThisModule, __LINE__, "介质控制: 票据出: ->nSetCheckOut(%d, %d) Fail, ErrCode=%d, Return %s.",
                    1, 0, nRet, ConvertErrCodeToStr(ERR_PTR_NO_MEDIA));
                return ERR_PTR_NO_MEDIA;
            } else
            {
                Log(ThisModule, __LINE__, "介质控制: 票据出: ->nSetCheckOut(%d, %d) Fail, ErrCode=%d, Return %s.",
                    1, 0, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }
    } else
    if (enMediaAct == MEDIA_CTR_SKIP)  // 票据退出(打开闸门取票)
    {
        if ((nRet = m_devBT8500M.nTakeCheck(usParam < 1 ? 5 : usParam)) != IMP_SUCCESS)
        {
            if (nRet == PRINTER_ERROR_PAPERLACK)    // 缺纸(通道内无纸)
            {
                Log(ThisModule, __LINE__, "介质控制: 票据出: ->nTakeCheck(%d) Fail, ErrCode=%d, Return %s.",
                    usParam < 1 ? 5 : usParam, nRet, ConvertErrCodeToStr(ERR_PTR_NO_MEDIA));
                return ERR_PTR_NO_MEDIA;
            } else
            {
                Log(ThisModule, __LINE__, "介质控制: 票据出: ->nTakeCheck(%d) Fail, ErrCode=%d, Return %s.",
                    usParam < 1 ? 5 : usParam, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }
    } else
    if (enMediaAct == MEDIA_CTR_PERFORATE)  // 票据回收(通道内回收)
    {
        if ((nRet = m_devBT8500M.nMoveCheck(0, 0)) != IMP_SUCCESS)
        {
            if (nRet == PRINTER_ERROR_PAPERLACK)    // 缺纸(通道内无纸)
            {
                Log(ThisModule, __LINE__, "介质控制: 票据回收: ->nMoveCheck(%d, %d) Fail, ErrCode=%d, Return %s.",
                    0, 0, nRet, ConvertErrCodeToStr(ERR_PTR_NO_MEDIA));
                return ERR_PTR_NO_MEDIA;
            } else
            {
                Log(ThisModule, __LINE__, "介质控制: 票据回收: ->nMoveCheck(%d, %d) Fail, ErrCode=%d, Return %s.",
                    0, 0, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }
    } else
    if (enMediaAct == MEDIA_CTR_RETRACT)  // 票据回收(出口回收)
    {
        if ((nRet = m_devBT8500M.nRetractCheck()) != IMP_SUCCESS)
        {
            if (nRet == PRINTER_ERROR_PAPERLACK)    // 缺纸(出口无纸)
            {
                Log(ThisModule, __LINE__, "介质控制: 票据回收: ->nRetractCheck() Fail, ErrCode=%d, Return %s.",
                    nRet, ConvertErrCodeToStr(ERR_PTR_NO_MEDIA));
                return ERR_PTR_NO_MEDIA;
            } else
            {
                Log(ThisModule, __LINE__, "介质控制: 票据回收: ->nRetractCheck() Fail, ErrCode=%d, Return %s.",
                    nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
                return ConvertErrorCode(nRet);
            }
        }
    } else
    if (enMediaAct == MEDIA_CTR_EXPEL)  // 开门
    {
        if ((nRet = m_devBT8500M.nOutPaperDoorControl(0)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "介质控制: 闸门开: ->OutPaperDoorControl(0) Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (enMediaAct == MEDIA_CTR_PARK)  // 关门
    {
        if ((nRet = m_devBT8500M.nOutPaperDoorControl(1)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "介质控制: 闸门关: ->OutPaperDoorControl(1) Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (enMediaAct == MEDIA_CTR_ATPBACKWARD)  // 票据移出票箱(出票)
    {
        if ((nRet = m_devBT8500M.nFeedCheck(usParam, 1)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "介质控制: 分纸: ->nFeedCheck(%d, 0) Fail, ErrCode=%d, Return %s.",
               usParam, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    {
        Log(ThisModule, __LINE__, "介质控制: 无效入参[%d], Return %s.",
           enMediaAct, ERR_PTR_PARAM_ERR);
        return ERR_PTR_PARAM_ERR;
    }

    return PTR_SUCCESS;
}

// 设置数据
int CDevCPR_BT8500M::SetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case DTYPE_LIB_PATH:    // 设置Lib路径
        {
            m_devBT8500M.SetLibPath((LPCSTR)vInitPar);
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
        case DTYPE_FONT: // 设置支持的打印字体
        {
            m_stFontList.ImportFontData((LPSTR)vInitPar);
        }
        default:
            break;
    }

    return PTR_SUCCESS;
}

// 获取数据
int CDevCPR_BT8500M::GetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case 0:
            break;
        default:
            break;
    }

    return PTR_SUCCESS;
}

// 获取版本号(1DevCPR版本/2固件版本/3设备软件版本/4其他)
void CDevCPR_BT8500M::GetVersion(char* szVer, long lSize, ushort usType)
{
    CHAR    szVersion[128] = { 0x00 };

    if (usType == 1)
    {
        //memcpy(szVersion, byDevVRTU, strlen((char*)byDevVRTU));
    } else
    if (usType == 2)
    {
        if (m_devBT8500M.IsDeviceOpen() == TRUE)
        {
            if (m_devBT8500M.nGetFirmwareVersion(szVersion, lSize) != IMP_SUCCESS)
            {
                return;
            }
        }
    } else
    if (usType == 3)
    {
        return;
    } else
    if (usType == 4)
    {
        return;
    }

    memcpy(szVer, szVersion, strlen((char*)szVersion) > lSize ? lSize : strlen((char*)szVersion));
}

INT CDevCPR_BT8500M::ConvertErrorCode(INT nRet)
{
    switch(nRet)
    {
        case PRINTER_SUCCESS:                       // 正常->操作成功
            return PTR_SUCCESS;
        case PRINTER_ERROR_OPEN_PORT:               // 打开端口失败->USB/COM/连接错误
        case PRINTER_ERROR_COMMUNICATION:           // 端口通讯失败->通讯错误
            return ERR_PTR_NOT_OPEN;
        case PRINTER_ERROR_PAPERLACK:               // 缺纸->打印机缺纸
            return ERR_PTR_NO_PAPER;
        case PRINTER_ERROR_BELTOVER:                // 碳带将近->
        case PRINTER_ERROR_BELTLACK:                // 缺色带->INK或色带故障
            return ERR_PTR_TONER;
        case PRINTER_ERROR_FEEDER_OPEN:             // 分纸模块微动开关错->硬件故障
            return ERR_PTR_HWERR;
        case PRINTER_ERROR_PAPERINCOMPLETE:         // 票据缺角->
        case PRINTER_ERROR_PAPERSIZE:               // 票据长度错误->
        case PRINTER_ERROR_DOUBLE_DOC:              // 重张->介质异常
            return ERR_PTR_PAPER_ERR;
        case PRINTER_ERROR_FEEDER:                  // 分纸错->
        case PRINTER_ERROR_GATE:                    // 闸门错->
        case PRINTER_ERROR_PAPER_CORRECTION :       // 设备纠偏错->
        case PRINTER_ERROR_CHANNEL_ERROR:           // 通道抬起压下错误->
        case PRINTER_ERROR_SCAN_UNIT_OPEN:          // 扫描鉴伪模块上盖打开->
        case PRINTER_ERROR_OCR_COVER_OPEN:          // 票号识别模块上盖打开->
        case PRINTER_ERROR_OUTPUTBOX_OPEN:          // 出票箱上盖抬起->
        case PRINTER_ERROR_CALIBRATE:               // 扫描模块校正失败->
        case PRINTER_ERROR_SCANVOLHIGH:             // 扫描模块电压偏高->
        case PRINTER_ERROR_SCANVOLLOW:              // 扫描模块电压偏低->
        case PRINTER_ERROR_BELTRECYCLE:             // 碳带回收错->
        case PRINTER_ERROR_HEADDOWN:                // 打印头压下错误->
        case PRINTER_ERROR_PRINTMICROSWITCH:        // 打印模块微动开关错->
        case PRINTER_ERROR_STOPPING:                // 打印机暂停中->
        case PRINTER_ERROR_NOT_WAIT_PAPEROUT:       // 设备不在出纸等待状态->
        case PRINTER_ERROR_SENSOR_CALIBRATE:        // 传感器校验失败->
        case PRINTER_ERROR_OUTPUTBOX_PRESSURE:      // 发票箱压板错误->
        case PRINTER_ERROR_STAMP:                   // 印章错误->
        case PRINTER_ERROR_SCAN_UNIT:               // 扫描模块错误->
        case PRINTER_ERROR_OUTOFMEMORY:             // 内存不足->
        case PRINTER_ERROR_IMAGE_DESKEW:            // 裁剪纠偏错误->
        case PRINTER_ERROR_INIFILE:                 // 读取配置文件错误->
        case PRINTER_ERROR_TEMPVOL:                 // 温度电压异常->
            return ERR_PTR_HWERR;                                               // 硬件故障
        case PRINTER_ERROR_TIME_OUT:                // 取票超时->超时
            return ERR_PTR_TIMEOUT;
        case PRINTER_ERROR_PAPERJAM:                // 卡纸错->堵纸等机械故障
            return ERR_PTR_JAMMED;
        case PRINTER_ERROR_JPEG_COMPRESS:           // JPEG压缩错误->
        case PRINTER_ERROR_DOWNLOADIMAGE:           // 下载位图失败->图片相关错误
            return ERR_PTR_IMAGE_ERROR;
        case PRINTER_ERROR_FILE_R_W:                // 文件读写错误->
        case PRINTER_ERROR_RFID:                    // 获取射频信息失败->收发数据错误
            return ERR_PTR_DATA_ERR;
        case PRINTER_ERROR_NOT_IDLE:                // 设备非空闲状态->设备忙
            return ERR_PTR_DEVBUSY;
        case PRINTER_ERROR_UNKNOWN:                 // 未知错误->其它错误
            return ERR_PTR_OTHER;
        case PRINTER_ERROR_PARAM:                   // 接口传入参数错误->参数错误
            return ERR_PTR_PARAM_ERR;
        case IMP_ERR_LOAD_LIB:                      // 动态库加载失败
        case PRINTER_ERROR_LOADLIB:                 // 加载动态库失败->
        case PRINTER_ERROR_NOT_SUPPORT:             // 不支持的操作
        case PRINTER_ERROR_FLOW:                    // 调用流程错
        case PRINTER_ERROR_NOT_SET_PAPERCOUNT:      // 票箱张数事先未设置
        case PRINTER_ERROR_INIFILE_W:               // 写配置文件失败
        case PRINTER_ERROR_GBKTOUTF8:               // 打印条码时转码错误
        case PRINTER_ERROR_NOT_EMPTY_RETRACTBOX_COUNT:// 未清空回收箱计数
        case PRINTER_ERROR_CHECKNO_COMPARE:         // 票号比对失败
        case PRINTER_ERROR_BATCHFAKE:               // 批量打印中有假票，该错误码只会通过RegistFun回调函数参数返回
        case PRINTER_ERROR_IMAGEDATA:               // 图像数据错误
        case PRINTER_ERROR_CHANNELHAVEPAPER:        // 通道有纸
        case PRINTER_ERROR_GET_QR_CODE:             // 二维码读取失败
            return ERR_PTR_OTHER;                                               // 其它错误
        case IMP_ERR_PARAM_INVALID:                 // 参数无效->参数错误
            return ERR_PTR_PARAM_ERR;
        case PRINTER_ERROR_GET_CHECK_NO:            // 票号识别错误
        case PRINTER_ERROR_IDENTIFY:                // 鉴伪失败->数据识别失败
            return ERR_PTR_DATA_DISCERN;
        case PRINTER_ERROR_GET_IMAGE:               // 图像获取失败->扫描失败
            return ERR_PTR_SCAN_FAIL;
        case IMP_ERR_UNKNOWN:                       // 未知错误->其它错误
            return ERR_PTR_OTHER;
        default:
            return ERR_PTR_OTHER;
    }
}

CHAR* CDevCPR_BT8500M::ConvertErrCodeToStr(INT nRet)
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
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "其他错误/未知错误");
            return m_szErrStr;
    }
}

void CDevCPR_BT8500M::DiffDevStat(DEVICESTATUS stStat, DEVICESTATUS stStatOLD)
{
    THISMODULE(__FUNCTION__);

    INT nStat[100] = {
        stStat.iSize,                      // 结构体大小，便于扩展
        stStat.bError,                     // 错误状态
        stStat.bIdle,                      // 空闲
        stStat.bWaitForScan,               // 等待启动扫描
        stStat.bPrinting,                  // 正在打印
        stStat.bScanning_OCR,              // 打印前扫描过程
        stStat.bScanning_Identify,         // 打印后扫描过程
        stStat.bWaitForPrinting,           // 等待启动打印
        stStat.bWaitForPaperOut,           // 等待出票
        stStat.bDeviceInPause,             // 暂停状态
        stStat.bPaperJamError,             // 塞纸错误
        stStat.bUnitVolTooHigh,            // 整机电压偏高
        stStat.bUnitVolTooLow,             // 整机电压偏低
        stStat.bFeederOpen,                // 分纸模块上盖打开
        stStat.bPrintHeadOpen,             // 打印模块微动开关错
        stStat.bChannelError,              // 通道抬起压下错误
        stStat.bScanUnitOpen,              // 打印后扫描模块上盖打开
        stStat.bFeedingError,              // 分纸错误
        stStat.bCommunicationError,        // 扫描模块通讯错
        stStat.bPaperLengthError,          // 票据长度错
        stStat.bDoubleDocError,            // 重张错
        stStat.bCorrectionError,           // 纠偏错
        stStat.bBeltLackError,             // 缺碳带错
        stStat.bTempVolError,              // 温度电压异常错
        stStat.bBeltRecycleError,          // 碳带回收错
        stStat.bBeltOverError,             // 碳带将尽错
        stStat.bPrinterHeadError,          // 打印头抬起压下错误
        stStat.bGateError,                 // 闸门错
        stStat.bPaperIncompleteError,      // 缺角错
        stStat.bOutputBoxOpen,             // 出票箱是否打开
        stStat.bRetrackError,              // 废票箱是否取出
        stStat.bOCRCoverOpen,              // 打印前扫描模块上盖打开
        stStat.bSensorCalibrateError,      // 传感器校验是否失败
        stStat.bOutputBoxPressureError,    // 回收压板错误
        stStat.bStampError,                // 印章错
        stStat.bScanUnitError,             // 扫描模块状态错
        stStat.bBox1HavePaper,             // 票箱1是否有票
        stStat.bBox2HavePaper,             // 票箱2是否有票
        stStat.bBox3HavePaper,             // 票箱3是否有票
        stStat.bBox4HavePaper,             // 票箱4是否有票
        stStat.bBox1Exist,                 // 票箱1是否存在
        stStat.bBox2Exist,                 // 票箱2是否存在
        stStat.bBox3Exist,                 // 票箱3是否存在
        stStat.bBox4Exist,                 // 票箱4是否存在
        stStat.bChannelHasPaper,           // 通道有纸
        stStat.bStackerHavePaper,          // 出票箱是否有纸
        stStat.bGateOpen,                  // 闸门是否打开
        stStat.bHaveData,                  // 有数据
        stStat.bLastBulk,                  // 最后一块数据
        (INT)stStat.lDataLength,           // 图像数据的长度
        stStat.bRetrackHavePaper,          // 回收箱是否有纸
    };
    INT nStatOLD[100] = {
            stStatOLD.iSize,                      // 结构体大小，便于扩展
            stStatOLD.bError,                     // 错误状态
            stStatOLD.bIdle,                      // 空闲
            stStatOLD.bWaitForScan,               // 等待启动扫描
            stStatOLD.bPrinting,                  // 正在打印
            stStatOLD.bScanning_OCR,              // 打印前扫描过程
            stStatOLD.bScanning_Identify,         // 打印后扫描过程
            stStatOLD.bWaitForPrinting,           // 等待启动打印
            stStatOLD.bWaitForPaperOut,           // 等待出票
            stStatOLD.bDeviceInPause,             // 暂停状态
            stStatOLD.bPaperJamError,             // 塞纸错误
            stStatOLD.bUnitVolTooHigh,            // 整机电压偏高
            stStatOLD.bUnitVolTooLow,             // 整机电压偏低
            stStatOLD.bFeederOpen,                // 分纸模块上盖打开
            stStatOLD.bPrintHeadOpen,             // 打印模块微动开关错
            stStatOLD.bChannelError,              // 通道抬起压下错误
            stStatOLD.bScanUnitOpen,              // 打印后扫描模块上盖打开
            stStatOLD.bFeedingError,              // 分纸错误
            stStatOLD.bCommunicationError,        // 扫描模块通讯错
            stStatOLD.bPaperLengthError,          // 票据长度错
            stStatOLD.bDoubleDocError,            // 重张错
            stStatOLD.bCorrectionError,           // 纠偏错
            stStatOLD.bBeltLackError,             // 缺碳带错
            stStatOLD.bTempVolError,              // 温度电压异常错
            stStatOLD.bBeltRecycleError,          // 碳带回收错
            stStatOLD.bBeltOverError,             // 碳带将尽错
            stStatOLD.bPrinterHeadError,          // 打印头抬起压下错误
            stStatOLD.bGateError,                 // 闸门错
            stStatOLD.bPaperIncompleteError,      // 缺角错
            stStatOLD.bOutputBoxOpen,             // 出票箱是否打开
            stStatOLD.bRetrackError,              // 废票箱是否取出
            stStatOLD.bOCRCoverOpen,              // 打印前扫描模块上盖打开
            stStatOLD.bSensorCalibrateError,      // 传感器校验是否失败
            stStatOLD.bOutputBoxPressureError,    // 回收压板错误
            stStatOLD.bStampError,                // 印章错
            stStatOLD.bScanUnitError,             // 扫描模块状态错
            stStatOLD.bBox1HavePaper,             // 票箱1是否有票
            stStatOLD.bBox2HavePaper,             // 票箱2是否有票
            stStatOLD.bBox3HavePaper,             // 票箱3是否有票
            stStatOLD.bBox4HavePaper,             // 票箱4是否有票
            stStatOLD.bBox1Exist,                 // 票箱1是否存在
            stStatOLD.bBox2Exist,                 // 票箱2是否存在
            stStatOLD.bBox3Exist,                 // 票箱3是否存在
            stStatOLD.bBox4Exist,                 // 票箱4是否存在
            stStatOLD.bChannelHasPaper,           // 通道有纸
            stStatOLD.bStackerHavePaper,          // 出票箱是否有纸
            stStatOLD.bGateOpen,                  // 闸门是否打开
            stStatOLD.bHaveData,                  // 有数据
            stStatOLD.bLastBulk,                  // 最后一块数据
            (INT)stStatOLD.lDataLength,           // 图像数据的长度
            stStatOLD.bRetrackHavePaper,          // 回收箱是否有纸
        };

    CHAR szData[1024] = { 0x00 };
    for (INT i = 0; i < 51; i ++)
    {
        sprintf(szData + strlen(szData), "%d->%d%s|", nStatOLD[i], nStat[i],
                (nStatOLD[i] != nStat[i] ? "*" : " "));
        if ((i + 1) % 10 == 0)
        {
            sprintf(szData + strlen(szData), "\n");
        }
    }

    if (memcmp(&stStat, &stStatOLD, sizeof(DEVICESTATUS)) != 0)
    {
        Log(ThisModule, __LINE__, "读取CPR设备状态: 变化比较: \n%s", szData);
    }
}

//////////////////////////////////////////////////////////////////////////






