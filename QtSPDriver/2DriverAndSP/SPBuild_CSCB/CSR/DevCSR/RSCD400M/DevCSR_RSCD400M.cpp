/***************************************************************
* 文件名称：DevCSR_RSCD400M.cpp
* 文件描述：RSC-D400M票据受理模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevCSR_RSCD400M.h"

#include "data_convertor.h"
#include <QTextCodec>

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>

static const char *ThisModule = "DevCSR_RSCD400M.cpp";

//////////////////////////////////////////////////////////////////////////

CDevCSR_RSCD400M::CDevCSR_RSCD400M() : m_devRSCD400M(LOG_NAME_DEVCSR)
{
    SetLogFile(LOG_NAME_DEVCSR, ThisModule);  // 设置日志文件名和错误发生的文件
    m_usDPIx = 192;                           // X方向DPI
    m_usDPIy = 208;                           // y方向DPI
    m_nGetStatErrOLD = PTR_SUCCESS;           // 取状态接口上一次错误码
    m_nGetOpenErrOLD = PTR_SUCCESS;
}

CDevCSR_RSCD400M::~CDevCSR_RSCD400M()
{
    Close();
}

// 打开与设备的连接
int CDevCSR_RSCD400M::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // 建立CSR连接
    if (m_devRSCD400M.IsDeviceOpen() == TRUE)
    {
        m_devRSCD400M.DeviceClose(FALSE);
    }

    if ((nRet = m_devRSCD400M.DeviceOpen((LPSTR)lpMode)) != IMP_SUCCESS)
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

    return PTR_SUCCESS;
}

// 关闭与设备的连接
int CDevCSR_RSCD400M::Close()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_devRSCD400M.DeviceClose();
    Log(ThisModule, __LINE__, "关闭设备连接: ->DeviceClose() Succ, Return %s.",
        ConvertErrCodeToStr(PTR_SUCCESS));

    return PTR_SUCCESS;
}

// 设备复位
int CDevCSR_RSCD400M::Reset()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    if ((nRet = m_devRSCD400M.nCHKReset()) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备复位: ->nCHKReset() Fail, ErrCode=%d, Return %s.",
            nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

// 设备复位
int CDevCSR_RSCD400M::ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    INT nAct = 0;

    if (enMediaAct == MEDIA_CTR_NOTACTION)      // 无动作
    {
        nAct = 2;
    } else
    if (enMediaAct == MEDIA_CTR_EJECT)          // 票据退出
    {
        nAct = 0;
    } else
    if (enMediaAct == MEDIA_CTR_PARK)           // 票据压箱
    {
        nAct = 1;
    } else
    {
        Log(ThisModule, __LINE__, "设备复位: 入参错误[enMediaAct=%d], Return %d.",
            enMediaAct, ERR_PTR_PARAM_ERR);
        return ERR_PTR_PARAM_ERR;
    }

    nRet = m_devRSCD400M.nCHKResetEx(nAct);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设备复位: ->nCHKResetEx(%d, %s) Fail, ErrCode=%d, Return %s.",
            nAct, (nAct == 2 ? "无动作" : (nAct == 1 ? "票据压箱" : "票据退出")),
            nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

// 取设备状态
int CDevCSR_RSCD400M::GetStatus(DEVPTRSTATUS &stStatus)
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

    if (m_devRSCD400M.IsDeviceOpen() != TRUE)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (m_nGetStatErrOLD != ERR_PTR_NOT_OPEN)
        {
            Log(ThisModule, __LINE__, "读取设备状态: ->IsDeviceOpen() Is FALSE, Device Not Open, Return %s.",
                ConvertErrCodeToStr(ERR_PTR_NOT_OPEN));
            m_nGetStatErrOLD = ERR_PTR_NOT_OPEN;
        }

        stStatus.wDevice = DEV_STAT_OFFLINE;
        return ERR_PTR_NOT_OPEN;
    }

    // 取设备状态
    SCANNERSTATUS stDevStat;
    if ((nRet = m_devRSCD400M.nCHKGetStatus(&stDevStat)) != IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (m_nGetStatErrOLD != ConvertErrorCode(nRet))
        {
            Log(ThisModule, __LINE__, "读取设备状态: ->nCHKGetStatus() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            m_nGetStatErrOLD = ConvertErrorCode(nRet);
        }

        if (nRet == SS_ERR_HWERROR) // -3硬件故障(未连接或无法连接)
        {
            stStatus.wDevice = DEV_STAT_OFFLINE;
        }

        return ConvertErrorCode(nRet);
    }

    // 解析获取的设备状态

    // Device状态
    if (stDevStat.iError == 1)                          // 出错
    {
        if (stDevStat.iVoltageError == 1 ||             // 电压异常
            stDevStat.iHardwareError == 1 ||            // 硬件错误
            stDevStat.iFPGAError == 1 ||                // FPGA错误
            stDevStat.iPushGearError == 1 ||            // 出票兜凸轮未归位
            stDevStat.iPushBoardError == 1 ||           // 出票兜推纸板未归位
            stDevStat.iPickFail == 1 ||                 // 纠偏失败
            stDevStat.iPaperEnter == 1 ||				// 纸张进入异常
            stDevStat.iPaperLength == 1 ||              // 纸张长度超长/过短
            stDevStat.iPocketFullSensorInit == 1)		// 票箱满传感器初始化异常
        {
            stStatus.wDevice = DEV_STAT_HWERROR;
        } else
        if (stDevStat.iPocketOpen == 1 ||               // 出纸兜开(左/右票箱打开时状态,iError==1)
            stDevStat.iLeftPocketOpen == 1 ||			// 左票箱开(打开时,iError==1,iPocketOpen=1)
            stDevStat.iRightPocketOpen == 1 ||          // 右票箱开(打开时,iError==1,iPocketOpen=1)
            stDevStat.iPaperEnterRepeat == 1)           // 纸张二次进入状态
        {
            stStatus.wDevice = DEV_STAT_HWERROR;
        }
    } else
    {
        stStatus.wDevice = DEV_STAT_ONLINE;
    }

    // MEDIA状态
    if (stDevStat.iJam == 1)   // 塞纸
    {
        stStatus.wMedia = MEDIA_STAT_JAMMED;
        stStatus.wDevice = DEV_STAT_HWERROR;
    } else
    {
        if (stDevStat.iStatus == SCANNER_STATUS_IDLE)       // 空闲状态(通道内无票)
        {
            stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
        } else
        if (stDevStat.iStatus == SCANNER_STATUS_PAPEROUT)   // 票在出纸口
        {
            stStatus.wMedia = MEDIA_STAT_ENTERING;
        } else
        if (stDevStat.iStatus == SCANNER_STATUS_READY ||        // 就绪状态
            stDevStat.iStatus == SCANNER_STATUS_FEED ||         // 走纸状态
            stDevStat.iStatus == SCANNER_STATUS_PRINT ||        // 打印状态
            stDevStat.iStatus == SCANNER_STATUS_SCAN  ||        // 扫描状态
            stDevStat.iStatus == SCANNER_STATUS_WAITACTION ||   // 等待动作状态
            stDevStat.iStatus == SCANNER_STATUS_READ)           // 读数据状态
        {
            stStatus.wMedia = MEDIA_STAT_PRESENT;    // 通道有纸
        }
    }

    // 回收箱状态(0右票箱/1左票箱)
    if (stDevStat.iRightPocketFull == 1)    // 右票箱满
    {
        stStatus.stRetract[1].wBin = RETRACT_STAT_FULL;
    } else
    {
        stStatus.stRetract[1].wBin = RETRACT_STAT_OK;
    }
    if (stDevStat.iLeftPocketFull == 1)    // 左票箱满
    {
        stStatus.stRetract[0].wBin = RETRACT_STAT_FULL;
    } else
    {
        stStatus.stRetract[0].wBin = RETRACT_STAT_OK;
    }

    // 墨盒状态
    if (stDevStat.iInkPresent == 0) // 墨盒不在位
    {
        stStatus.wInk = INK_STAT_UNKNOWN;
    } else
    {
        if (stDevStat.iInkNearEmpty == 1) // 墨盒墨将尽
        {
            stStatus.wInk = INK_STAT_LOW;
        } else
        {
            stStatus.wInk = INK_STAT_FULL;
        }
    }

    // 状态出现变化时,打印LOG
    DiffDevStat(stDevStat, m_stScanStatOLD);

    memcpy(&m_stScanStatOLD, &stDevStat, sizeof(SCANNERSTATUS));

    return PTR_SUCCESS;
}

// 打印字串(无指定打印坐标)
int CDevCSR_RSCD400M::PrintData(const char *pStr, unsigned long ulDataLen)
{
    #define JSON_GET(JSON, KEY, VALUE) \
        if (JSON.Get(KEY, VALUE) != true) \
        {\
            Log(ThisModule, __LINE__, "打印字串: ->JSON.Get(%s) Fail, JSON=[%s],Return %d.", \
                KEY, JSON.ToString().c_str(), ERR_PTR_JSON_ERR); \
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
    WORD wMediaWidth = 0, wMediaHeignt = 0, wStartX = 0, wStartY = 0, wStartY_Chg = 0;

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
    JSON_GET(cJsonPrtIn, JSON_KEY_MEDIA_WIDTH, wMediaWidth);        // 取 票据宽
    cJsonSndData.Add(BT_JSON_PRINTWIDTH, wMediaWidth);
    JSON_GET(cJsonPrtIn, JSON_KEY_MEDIA_HEIGHT, wMediaHeignt);      // 取 票据高
    cJsonSndData.Add(BT_JSON_PRINTHEIGHT, wMediaHeignt);

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
            wStartX = wStartY = wStartY_Chg = 0;
            //-- ALL --
            // 起始坐标X(单位:MM)
            SET_KEYNAME(JSON_KEY_START_X, i, szKeyName);
            JSON_GET(cJsonType, szKeyName, wStartX);
            // 起始坐标Y(单位:MM)
            SET_KEYNAME(JSON_KEY_START_Y, i, szKeyName);
            JSON_GET(cJsonType, szKeyName, wStartY);
            // 坐标Check+转换
            if (wStartX > wMediaWidth || wStartY > wMediaHeignt)
            {
                Log(ThisModule, __LINE__, "打印字串: 坐标越界[X=%d, Y=%d, Width=%d, Height=%d], JSON=[%s], Return %s.",
                    wStartX , wStartY, wMediaWidth, wMediaHeignt,
                    cJsonType.ToString().c_str(), ConvertErrCodeToStr(ERR_PTR_PARAM_ERR));
                return ERR_PTR_PARAM_ERR;
            }
            wStartY_Chg = wMediaHeignt - wStartY;
            Log(ThisModule, __LINE__, "打印字串: 设备坐标以左下角为基准点(X>4,Y>36),转换StartY=%d(MediaHeight[%d]-StartY[%d])",
                wStartY_Chg, wMediaHeignt, wStartY);
            cJsonTmp.Add("x", wStartX);
            cJsonTmp.Add("y", wStartY_Chg);

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
                cJsonTmp.Add("height", MM2PX(wValue, m_usDPIy));
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
                cJsonFields.Add(cJsonTmp);
            } else
            if (a == 3) // 磁码
            {
                cJsonTmp.Add("content", stdTmp);
                // 加入节点下
                cJsonFields.Add(cJsonTmp);
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
        if ((nRet = m_devRSCD400M.nCHKPrintJson((LPSTR)cJsonSndData.ToString().c_str())) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "打印字串: ->nCHKPrintJson(JSON, 1, 0) Fail, ErrCode=%d, Return %s.",
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
int CDevCSR_RSCD400M::ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    #define JSON_GET(JSON, KEY, VALUE) \
        if (JSON.Get(KEY, VALUE) != true) \
        {\
            Log(ThisModule, __LINE__, "ReadImage获取: ->JSON.Get(%s) Fail, Return %d.", \
                KEY, ERR_PTR_JSON_ERR); \
            return ERR_PTR_JSON_ERR; \
        }

    #define JSON_GET_NOR(JSON, KEY, VALUE) \
    if (JSON.Get(KEY, VALUE) != true) \
    {\
        Log(ThisModule, __LINE__, "ReadImage获取: ->JSON.Get(%s) Fail, Not Return.", \
            KEY); \
    }

    #define SET_KEYNAME(KEY, CNT, DEST) \
        memset(DEST, 0x00, sizeof(DEST)); \
        sprintf(DEST, "%s%d", KEY, CNT);


    INT    nRet = IMP_SUCCESS;
    CJsonObject cJsonScanIn, cJsonScanOut;
    CJsonObject cJsonSndData, cJsonRcvData, cJsonTmpData, cJsonFields;
    std::string stdImageFront = "";
    std::string stdImageBack = "";
    std::string stdTmp = "";
    CHAR szKeyName[64] = { 0x00 };
    BOOL bIsSacnImage = FALSE;
    INT nNoteType = 0;  // 票据类型
    INT nDirection = 0; // 票据放置方向


    // 加载/解析JSON
    if (cJsonScanIn.Parse(stImageIn.lpData) != true)
    {
        Log(ThisModule, __LINE__, "ReadImage获取: ->JSON.Parse(%s) Fail, Return %s.",
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
        if ((stImageIn.wInMode & IMAGE_MODE_BACK) == IMAGE_MODE_BACK)
        {
            JSON_GET(cJsonScanIn, JSON_KEY_IMAGE_BACK_PATH, stdImageBack);
        }
        if ((nRet = m_devRSCD400M.nCHKScanAndGetImage((stdImageFront.empty() == true ? nullptr : (LPSTR)stdImageFront.c_str()),
                                              (stdImageBack.empty() == true ? nullptr : (LPSTR)stdImageBack.c_str()),
                                               &nNoteType, 0/*stImageIn.wTimeOut*/, &nDirection)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "ReadImage获取: ->nCHKScanAndGetImage(%s, %s, %d, %d, %d) Fail, ErrCode=%d, Return %s.",
                stdImageFront.c_str(), stdImageBack.c_str(), nNoteType, stImageIn.wTimeOut, nDirection,
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
        stImageOut.wResult = READIMAGE_RET_OK;
    }

    // 获取票据票号并鉴伪
    if ((stImageIn.wInMode & IMAGE_MODE_CODELINE) == IMAGE_MODE_CODELINE)
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
                Log(ThisModule, __LINE__, "ReadImage获取: 取JSON项数: ->JSON.Get(%s) = %d < 1, Return %s.",
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
        if ((nRet = m_devRSCD400M.nCHKSetCheckOCRArea((LPSTR)cJsonSndData.ToString().c_str())) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "ReadImage获取: 设置票面OCR识别区域: ->nCHKSetCheckOCRArea(%s) Fail, ErrCode=%d, Return %s.",
                cJsonSndData.ToString().c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
        // 取票据鉴伪和OCR识别结果
        CHAR szRcvBuffer[1024 * 10] = { 0x00 };
        if ((nRet = m_devRSCD400M.nCHKGetCheckAndOcrResult(szRcvBuffer, 1024 * 10)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "ReadImage获取: 取票据鉴伪和OCR识别结果: ->nCHKGetCheckAndOcrResult() Fail, ErrCode=%d, Return %s.",
                nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        } else
        {
            Log(ThisModule, __LINE__, "ReadImage获取: 取票据鉴伪和OCR识别结果: ->nCHKGetCheckAndOcrResult() succ, RcvData = %s.", szRcvBuffer);
        }
        
        // 解析结果
        if (cJsonRcvData.Parse(szRcvBuffer) != true)// 加载/解析JSON
        {
            Log(ThisModule, __LINE__, "ReadImage获取: ->JSON.Parse(%s) Fail, Return %s.",
                szRcvBuffer, ERR_PTR_JSON_ERR);
            return ERR_PTR_JSON_ERR;
        }

        // 取票据类型
        USHORT usNoteType = 0;
        JSON_GET(cJsonRcvData, BT_JSON_TYPE, usNoteType);
        Log(ThisModule, __LINE__, "ReadImage获取: ->JSON.Parse(%s) = %d(票据类型).",
            BT_JSON_TYPE, usNoteType);

        if (usNoteType == 10)   // 结算业务委托书 不鉴伪
        {
            stImageOut.wResult = READIMAGE_RET_OK;
        } else
        {
            // 取鉴伪结果
            wValue = 0;
            JSON_GET(cJsonRcvData, BT_JSON_ISTRUE, wValue);
            wValue == 1 ? stImageOut.wResult = READIMAGE_RET_OK : stImageOut.wResult = READIMAGE_RET_MISSING;
        }

        // 取OCR结果JSON
        JSON_GET(cJsonRcvData, BT_JSON_OCR_RESULLT, cJsonTmpData);

        // 取票号填充应答
        std::string stdNoteNo = "";
        // 取票号
        stdNoteNo.clear();
        JSON_GET_NOR(cJsonTmpData, BT_JSON_CHECKNO, stdNoteNo); // JSON取票号
        stdNoteNo.length() < 1 ? stdNoteNo.append("NULL") : "";
        SET_KEYNAME(JSON_KEY_IDEN_NAME, 1, szKeyName);      // 项名1:
        cJsonScanOut.Add(szKeyName, BT_JSON_CHECKNO);       // 项名1: 写入JSON
        SET_KEYNAME(JSON_KEY_IDEN_VALUE, 1, szKeyName);     // 项值1:
        cJsonScanOut.Add(szKeyName, stdNoteNo);             // 项值1: 票号写入JSON
        // 取账号
        stdNoteNo.clear();
        JSON_GET_NOR(cJsonTmpData, BT_JSON_ACCOUNT, stdNoteNo); // JSON取账号
        stdNoteNo.length() < 1 ? stdNoteNo.append("NULL") : "";
        SET_KEYNAME(JSON_KEY_IDEN_NAME, 2, szKeyName);      // 项名2:
        cJsonScanOut.Add(szKeyName, BT_JSON_ACCOUNT);       // 项名2: 写入JSON
        SET_KEYNAME(JSON_KEY_IDEN_VALUE, 2, szKeyName);     // 项值2:
        cJsonScanOut.Add(szKeyName, stdNoteNo);             // 项值2: 账号写入JSON

        /*if (wValue >= 2 && wValue <= 9)     // 票据类型存单
        {
            // 取金额
            stdNoteNo.clear();
            JSON_GET(cJsonTmpData, BT_JSON_AMOUNT, stdNoteNo);  // JSON取金额
            stdNoteNo.length() < 1 ? stdNoteNo.append("NULL") : "";
            SET_KEYNAME(JSON_KEY_IDEN_NAME, 3, szKeyName);      // 项名1:
            cJsonScanOut.Add(szKeyName, BT_JSON_AMOUNT);        // 项名1: 写入JSON
            SET_KEYNAME(JSON_KEY_IDEN_VALUE, 3, szKeyName);     // 项值1:
            cJsonScanOut.Add(szKeyName, stdNoteNo);             // 项值1: 金额写入JSON
        } else
        {
            // 取金额
            stdNoteNo.clear();
            JSON_GET(cJsonTmpData, BT_JSON_AMOUNT2, stdNoteNo); // JSON取金额
            stdNoteNo.length() < 1 ? stdNoteNo.append("NULL") : "";
            SET_KEYNAME(JSON_KEY_IDEN_NAME, 3, szKeyName);      // 项名1:
            cJsonScanOut.Add(szKeyName, BT_JSON_AMOUNT);        // 项名1: 写入JSON
            SET_KEYNAME(JSON_KEY_IDEN_VALUE, 3, szKeyName);     // 项值1:
            cJsonScanOut.Add(szKeyName, stdNoteNo);             // 项值1: 金额写入JSON
        }*/
        cJsonScanOut.Add(JSON_KEY_IDEN_CNT, 2);                 // 项数目: 3
    }

    // 获取票据RFID
    if ((stImageIn.wInMode & IMAGE_MODE_RFID) == IMAGE_MODE_RFID)
    {
        stdTmp.clear();
        // 取QR码识别结果
        stImageOut.wResult = 2;
        stImageOut.lpData = nullptr;
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
int CDevCSR_RSCD400M::MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;

    if (enMediaAct == MEDIA_CTR_EJECT)  // 票据退出
    {
        if ((nRet = m_devRSCD400M.nCHKEject()) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "介质控制: 票据退出: ->nCHKEject(%d) Fail, ErrCode=%d, Return %s.",
                usParam, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (enMediaAct == MEDIA_CTR_RETRACT)  // 票据压箱
    {
        if ((nRet = m_devRSCD400M.nCHKAccept()) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "介质控制: 票据压箱: ->nCHKAccept() Fail, ErrCode=%d, Return %s.",
                m_JsonData.ToString().c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (enMediaAct == MEDIA_CTR_TURNMEDIA)  // 入票
    {
        if ((nRet = m_devRSCD400M.nCHKInsert()) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "介质控制: 入票: ->nCHKInsert() Fail, ErrCode=%d, Return %s.",
                m_JsonData.ToString().c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    } else
    if (enMediaAct == MEDIA_CTR_STAMP)  // 取消入票
    {
        if ((nRet = m_devRSCD400M.nCHKCancelInsert()) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "介质控制: 取消入票: ->nCHKCancelInsert() Fail, ErrCode=%d, Return %s.",
                m_JsonData.ToString().c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
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
int CDevCSR_RSCD400M::SetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case DTYPE_LIB_PATH:    // 设置Lib路径
        {
            m_devRSCD400M.SetLibPath((LPCSTR)vInitPar);
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
int CDevCSR_RSCD400M::GetData(void *vInitPar, WORD wDataType)
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

// 获取版本号(1DevCSR版本/2固件版本/3设备软件版本/4其他)
void CDevCSR_RSCD400M::GetVersion(char* szVer, long lSize, ushort usType)
{
    CHAR    szVersion[128] = { 0x00 };

    if (usType == 1)
    {
        //memcpy(szVersion, byDevVRTU, strlen((char*)byDevVRTU));
    } else
    if (usType == 2)
    {
        if (m_devRSCD400M.IsDeviceOpen() == TRUE)
        {
            if (m_devRSCD400M.nCHKGetFWVer(szVersion) != IMP_SUCCESS)
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

INT CDevCSR_RSCD400M::ConvertErrorCode(INT nRet)
{
    switch(nRet)
    {
        case SS_ERR_NOTSUPPORTED:                                   // -1:设备不支持此操作->不支持的指令
            return ERR_PTR_UNSUP_CMD;
        case SS_ERR_TIMEOUT:                                        // -2:操作超时->超时
            return ERR_PTR_TIMEOUT;
        case SS_ERR_HWERROR:                                        // -3:硬件出错(未连接或无法连接)->
            return ERR_PTR_NOT_OPEN;
        case SS_ERR_INVALIDFILEPATH:                                // -4:无效的文件路径->参数错误
        case SS_ERR_INVALIDPARAMETER:                               // -5:无效参数->参数错误
            return ERR_PTR_PARAM_ERR;
        case SS_ERR_DEVICECLOSED:                                   // -6:设备已关闭->设备没有打开
            return ERR_PTR_NOT_OPEN;
        case SS_ERR_CANCELED:                                       // -7:操作被取消->命令取消
            return ERR_PTR_CANCEL;
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

CHAR* CDevCSR_RSCD400M::ConvertErrCodeToStr(INT nRet)
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
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "其他错误/未知错误");
            return m_szErrStr;
    }
}

void CDevCSR_RSCD400M::DiffDevStat(SCANNERSTATUS stStat, SCANNERSTATUS stStatOLD)
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

//////////////////////////////////////////////////////////////////////////






