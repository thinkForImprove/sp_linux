/***************************************************************
* 文件名称：XFS_CPR_DEC.cpp
* 文件描述：票据发放模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_CPR.h"


//-----------------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_CPR::StartOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = PTR_SUCCESS;

    if (bReConn == FALSE)   // 非重连状态,避免重复调用
    {
        // Open前下传初始参数
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pPrinter->SetData(m_stConfig.szSDKPath, DTYPE_LIB_PATH);
        }

        // BT-8500M设备下传初始参数
        if (m_stConfig.nDriverType == IXFSCPR_TYPE_BT8500M)
        {
            m_pPrinter->SetData((LPSTR)m_stConfig.stCfg_BT8500M.stdFontBuffer.c_str(), DTYPE_FONT);  // 设置支持的打印字体
            m_pPrinter->SetData(&m_stConfig.stCfg_BT8500M.usDPIx, DTYPE_DPIx);
            m_pPrinter->SetData(&m_stConfig.stCfg_BT8500M.usDPIy, DTYPE_DPIy);
            m_pPrinter->SetData(&m_stConfig.usBank, DTYPE_BANKNO);
        }
    }

    // 打开连接
    nRet = m_pPrinter->Open(nullptr);
    if (nRet != PTR_SUCCESS)
    {
        if (bReConn == FALSE)   // 非重连状态
        {
            Log(ThisModule, __LINE__, "打开设备连接失败．ReturnCode:%d.", ConvertErrCode(nRet));
        } else
        {
            if (m_nReConRet != nRet)
            {
                Log(ThisModule, __LINE__, "断线重连:打开设备连接失败．ReturnCode:%d.", ConvertErrCode(nRet));
                m_nReConRet = nRet;
            }
        }

        return ConvertErrCode(nRet);
    }
    m_nReConRet = nRet;

    // 设置不需要Reset
    //m_bNeedReset = false;


    // 设备初始化
    /*nRet = OnInit();
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "设备初始化失败．ReturnCode:%d.", nRet);
        return ConvertErrCode(nRet);
    }*/

    // 更新扩展状态
    CHAR szFWVer[128] = { 0x00 };
    m_pPrinter->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

    m_cExtra.AddExtra("VRTCount", "3");
    m_cExtra.AddExtra("VRT[00]_XFSCPR", (char*)byXFSVRTU);
    m_cExtra.AddExtra("VRT[01]_DevCPR", (char*)byDevVRTU);
    m_cExtra.AddExtra("FirmwareVersion", szFWVer);
    m_cExtra.AddExtra("LastErrorCode", "00-000");
    m_cExtra.AddExtra("LastErrorDetail", "");

    // 更新一次状态
    OnStatus();

    if (bReConn == FALSE)   // 非重连状态
    {
        Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, __LINE__, "断线重连:打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());
    }

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//-----------------------------------重载函数-------------------------------
HRESULT CXFS_CPR::EndForm(PrintContext *pContext)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    #define JSON_ADD(JSON, KEY, CNT, STR, VAL) \
        memset(STR, 0x00, sizeof(STR)); \
        sprintf(STR, "%s%d", KEY, CNT); \
        JSON.Add(STR, VAL);

    // FORM行列值转换为MM(1MM单位),有小数位四舍五入
    #define ROWCOL2MM(ROL, MM) \
        pContext->pForm->GetOrigUNIT(nullptr) == FORM_ROWCOLUMN ? \
            (int)((ROL * MM + 5) / 10) : ROL

    // FORM设置CPI转MM(0.1MM单位)
    #define CPI2MM(CPI) CPI < 1 ? (int)(254 / 18) : (int)(254 / CPI)

    CJsonObject cJsonData;
    CJsonObject cJson_Text, cJson_Pic, cJson_Bar, cJson_Micr;
    WORD wTextCnt, wPicCnt, wBarCnt, wMicrCnt;
    CHAR szIdenKey[32];
    char szPrintData[MAX_PRINTDATA_LEN] = { 0x00 };
    SIZE stSize = GetPerRowColTwips2MM();

    cJsonData.Clear();

    PRINT_ITEMS *pItems = (PRINT_ITEMS *)pContext->pUserData;
    if (pContext->bCancel)  // 收到取消命令,不处理返回成功
    {
        if (pItems)
        {
            delete pItems;
            pItems = NULL;
        }
        pContext->pUserData = NULL;
        return WFS_SUCCESS;
    }

    // 排序打印ITEM
    qsort(pItems->pItems, pItems->nItemNum, sizeof(PRINT_ITEM *), ComparePrintItem);

    cJson_Text.Clear();
    cJson_Pic.Clear();
    cJson_Bar.Clear();
    cJson_Micr.Clear();
    wTextCnt = 0;
    wPicCnt = 0;
    wBarCnt = 0;
    wMicrCnt = 0;

    cJsonData.Add(JSON_KEY_MEDIA_WIDTH, ROWCOL2MM(pContext->pMedia->GetOrigSize().cx, stSize.cx));    // 介质宽(单位:MM)
    cJsonData.Add(JSON_KEY_MEDIA_HEIGHT, ROWCOL2MM(pContext->pMedia->GetOrigSize().cy, stSize.cy));   // 介质高(单位:MM)

    for (int i = 0; i < pItems->nItemNum; i++)
    {
        PRINT_STRING strFormat; // 记录格式化数据
        PRINT_ITEM *pItem = pItems->pItems[i];
        // 图片打印
        if (pItem->nFieldType == FT_GRAPHIC)
        {
            wPicCnt ++;
            JSON_ADD(cJson_Pic, JSON_KEY_IDEN_NAME, wPicCnt, szIdenKey, pItem->strImagePath);
            JSON_ADD(cJson_Pic, JSON_KEY_START_X, wPicCnt, szIdenKey, ROWCOL2MM(pItem->x, stSize.cx));
            JSON_ADD(cJson_Pic, JSON_KEY_START_Y, wPicCnt, szIdenKey, ROWCOL2MM(pItem->y, stSize.cy));
            JSON_ADD(cJson_Pic, JSON_KEY_PIC_ZOOM, wPicCnt, szIdenKey, m_stConfig.stCfg_BT8500M.fPictureZoom);
        } else
        if (pItem->nFieldType == FT_MICR)   // 磁码打印
        {
            wMicrCnt ++;
            memset(szPrintData, 0x00, sizeof(szPrintData));
            memcpy(szPrintData, pItem->Text, pItem->nTextLen);
            JSON_ADD(cJson_Micr, JSON_KEY_IDEN_NAME, wMicrCnt, szIdenKey, szPrintData);
            JSON_ADD(cJson_Micr, JSON_KEY_START_X, wMicrCnt, szIdenKey, ROWCOL2MM(pItem->x, stSize.cx));
            JSON_ADD(cJson_Micr, JSON_KEY_START_Y, wMicrCnt, szIdenKey, ROWCOL2MM(pItem->y, stSize.cy));
        } else // 字符串打印
        {
            wTextCnt ++;
            memset(szPrintData, 0x00, sizeof(szPrintData));
            memcpy(szPrintData, pItem->Text, pItem->nTextLen);
            JSON_ADD(cJson_Text, JSON_KEY_IDEN_NAME, wTextCnt, szIdenKey, szPrintData);
            JSON_ADD(cJson_Text, JSON_KEY_START_X, wTextCnt, szIdenKey, ROWCOL2MM(pItem->x, stSize.cx));
            JSON_ADD(cJson_Text, JSON_KEY_START_Y, wTextCnt, szIdenKey, ROWCOL2MM(pItem->y, stSize.cy));
            JSON_ADD(cJson_Text, JSON_KEY_AREA_WIDTH, wTextCnt, szIdenKey, CPI2MM(pItem->dwCPI));//ROWCOL2MM(pItem->nWidth, stSize.cx));
            JSON_ADD(cJson_Text, JSON_KEY_AREA_HEIGHT, wTextCnt, szIdenKey, CPI2MM(pItem->dwLPI));//ROWCOL2MM(pItem->nHeight, stSize.cy));
            JSON_ADD(cJson_Text, JSON_KEY_TEXT_FONT, wTextCnt, szIdenKey, pItem->strFontName);
        }
    }

    if (wTextCnt > 0)
    {
        cJson_Text.Add(JSON_KEY_IDEN_CNT, wTextCnt);
        cJsonData.Add(JSON_KEY_IDEN_TYPE_TEXT, cJson_Text);
    }
    if (wPicCnt > 0)
    {
        cJson_Pic.Add(JSON_KEY_IDEN_CNT, wPicCnt);
        cJsonData.Add(JSON_KEY_IDEN_TYPE_PIC, cJson_Pic);
    }
    if (wBarCnt > 0)
    {
        cJson_Bar.Add(JSON_KEY_IDEN_CNT, wBarCnt);
        cJsonData.Add(JSON_KEY_IDEN_TYPE_BAR, cJson_Bar);
    }
    if (wMicrCnt > 0)
    {
        cJson_Micr.Add(JSON_KEY_IDEN_CNT, wMicrCnt);
        cJsonData.Add(JSON_KEY_IDEN_TYPE_MICR, cJson_Micr);
    }

    // 删除自定义数据
    pContext->pUserData = nullptr;
    if (pItems)
    {
        delete pItems;
    }

    INT nRet = m_pPrinter->PrintData((LPSTR)cJsonData.ToString().c_str(),
                                     cJsonData.ToString().length());
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "->PrintData(%s, %d) Fail, ErrCode = %d, Return: %d.",
            (LPSTR)cJsonData.ToString().c_str(), cJsonData.ToString().length(),
            nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    // 控制MEDIA
    DWORD dwMediaControl = pContext->pPrintData->dwMediaControl;
    if (dwMediaControl != 0)
    {
        return InnerControlMedia(dwMediaControl);
    }

    return WFS_SUCCESS;
}

inline int MulDiv(int number, int numberator, int denominator)
{
    long long ret = number;
    ret *= numberator;
    //ret += numberator;
    if (0 == denominator)
    {
        ret = (-1);
    }
    else
    {
        ret /= denominator;
    }
    return (int) ret;
}



//-----------------------------------------------------------------------------------
//--------------------------------------功能处理---------------------------------------
// 读INI
void CXFS_CPR::InitConifig()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    #define NOTE_NO_PAR_MAP(NO, PAR, TYPE) \
        memset(szBuffer, 0x00, sizeof(szBuffer)); \
        strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, NO, NOTE_GETJSON)); \
        m_stConfig.m_Map_NoteNo_GetList.insert(map<USHORT, std::string>::value_type(TYPE, szBuffer)); \
        memset(szBuffer, 0x00, sizeof(szBuffer)); \
        strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, PAR, NOTE_GETJSON)); \
        m_stConfig.m_Map_NotePar_GetList.insert(map<USHORT, std::string>::value_type(TYPE, szBuffer)); \

    CHAR    szIniAppName[256];
    CHAR    szKeyName[256];
    CHAR    szBuffer[65536];
    INT     nCount = 0;

    // 底层设备控制动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 设备类型(0/)
    m_stConfig.nDriverType = m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (DWORD)0);

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.nOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stConfig.nOpenFailRet != 0 && m_stConfig.nOpenFailRet != 1)
    {
        m_stConfig.nOpenFailRet = 0;
    }

    // ----------------------------------------设备分类相关信息获取----------------------------------------
    // 根据设备类型获取相关参数
    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.nDriverType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    // 获取BT-8500M设备INI设置
    if (m_stConfig.nDriverType == IXFSCPR_TYPE_BT8500M)
    {
        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.nDriverType);

        // 鉴伪时是否使用Form或者INI设置值指定的值设置票面识别范围,0不使用/1使用,缺省1
        m_stConfig.stCfg_BT8500M.usUseDistAreaSupp = m_cXfsReg.GetValue(szIniAppName, "UseDistAreaSupp", (INT)1);
        if (m_stConfig.stCfg_BT8500M.usUseDistAreaSupp != 0 && m_stConfig.stCfg_BT8500M.usUseDistAreaSupp != 1)
        {
            m_stConfig.stCfg_BT8500M.usUseDistAreaSupp = 1;
        }

        // 图片缩放,缺省1.0
        memset(szBuffer, 0x00, sizeof(szBuffer));
        strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, "PictureZoom", "1.0"));
        m_stConfig.stCfg_BT8500M.fPictureZoom = atof(szBuffer);

        // X方向DPI
        m_stConfig.stCfg_BT8500M.usDPIx = m_cXfsReg.GetValue(szIniAppName, "DPIx", (INT)200);
        // Y方向DPI
        m_stConfig.stCfg_BT8500M.usDPIy = m_cXfsReg.GetValue(szIniAppName, "DPIy", (INT)200);

        // 复位方式:0软复位/1硬复位,缺省0
        m_stConfig.usResetMode = m_cXfsReg.GetValue(szIniAppName, "ResetMode", (DWORD)0);

        // --------获取支持的字体设置--------

        // 复位方式:0软复位/1硬复位,缺省0
        m_stConfig.usResetMode = m_cXfsReg.GetValue(szIniAppName, "ResetMode", (DWORD)0);

        // 字体支持数目
        nCount = 0;
        CHAR szFontName[32] = { 0x00 };
        CHAR szFontPath[256] = { 0x00 };
        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "DEVICE_SET_%d_FONT", m_stConfig.nDriverType);
        for (INT i = 1; i < 33; i ++)
        {
            // 取字体名称
            memset(szKeyName, 0x00, sizeof(szKeyName));
            memset(szFontName, 0x00, sizeof(szFontName));
            sprintf(szKeyName, "FontName%d", i);
            strcpy(szFontName, m_cXfsReg.GetValue(szIniAppName, szKeyName, ""));
            if (strlen(szFontName) < 1)
            {
                continue;
            }

            // 取字体路径
            memset(szKeyName, 0x00, sizeof(szKeyName));
            memset(szFontPath, 0x00, sizeof(szFontPath));
            sprintf(szKeyName, "FontPath%d", i);
            strcpy(szFontPath, m_cXfsReg.GetValue(szIniAppName, szKeyName, ""));
            if (strlen(szFontPath) < 1)
            {
                continue;
            }

            if (m_stConfig.stCfg_BT8500M.stdFontBuffer.length() > 0)
            {
                m_stConfig.stCfg_BT8500M.stdFontBuffer.append("|");
            }
            m_stConfig.stCfg_BT8500M.stdFontBuffer.append(szFontName);
            m_stConfig.stCfg_BT8500M.stdFontBuffer.append(",");
            m_stConfig.stCfg_BT8500M.stdFontBuffer.append(szFontPath);
        }
    }

    // ----------------------------------------票据箱相关信息获取----------------------------------------
    // 票据箱数目
    m_stNoteBoxList.usBoxCount = m_cXfsReg.GetValue("NOTEBOX_CONFIG", "NoteBoxCount", (DWORD)0);

    // 读指定票据箱相关参数
    for (INT i = 0; i < m_stNoteBoxList.usBoxCount; i ++)
    {
        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "NOTEBOX_%d", i);

        // 票据箱有效标记
        m_stNoteBoxList.stNoteBox[i].bIsHave = TRUE;
        // 票据箱是否支持计数
        m_stNoteBoxList.stNoteBox[i].bIsTally = (m_cXfsReg.GetValue(szIniAppName, "TallySup", (INT)1) == 1 ? TRUE : FALSE);
        // 票据箱索引
        m_stNoteBoxList.stNoteBox[i].usBoxNo = m_cXfsReg.GetValue(szIniAppName, "BoxNo", (DWORD)0);
        // 票据箱类型
        m_stNoteBoxList.stNoteBox[i].usBoxType = m_cXfsReg.GetValue(szIniAppName, "BoxType", (DWORD)0);
        // 票据类型
        m_stNoteBoxList.stNoteBox[i].usNoteType = m_cXfsReg.GetValue(szIniAppName, "NoteType", (DWORD)0);
        // 票据张数
        m_stNoteBoxList.stNoteBox[i].usNoteCount = m_cXfsReg.GetValue(szIniAppName, "NoteCount", (DWORD)0);
        // 报警阀值(HIGH回收箱/LOW存储箱)
        m_stNoteBoxList.stNoteBox[i].usThreshold = m_cXfsReg.GetValue(szIniAppName, "Threshold", (DWORD)0);
        // FULL报警阀值(回收箱使用)
        m_stNoteBoxList.stNoteBox[i].usFullThreshold = m_cXfsReg.GetValue(szIniAppName, "FullThreshold", (DWORD)0);
    }

    // ----------------------------------------银行分类相关信息获取----------------------------------------
    // 根据银行获取特殊设置
    m_stConfig.usBank = m_cXfsReg.GetValue("BANK_CONFIG", "BankNo", (DWORD)0);

    // 设置指定银行INI AppName
    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "BANK_SET_DEF");//, m_stConfig.usBank);

    // 取银行别码
    strcpy(m_stConfig.szBankCode, m_cXfsReg.GetValue(szIniAppName, "BankCode", "000"));

    // 取缺省票号/票面信息获取JSON
    NOTE_NO_PAR_MAP("NoteNo1_PTCD", "NotePar1_PTCD", NOTE_TYPE_PTCD);       // 普通存单-票号信息点
    NOTE_NO_PAR_MAP("NoteNo2_XPCD", "NotePar2_XPCD", NOTE_TYPE_XPCD);       // 芯片存单-票号信息点
    NOTE_NO_PAR_MAP("NoteNo3_DECD", "NotePar3_DECD", NOTE_TYPE_DECD);       // 大额存单-票号信息点
    NOTE_NO_PAR_MAP("NoteNo4_GZPZ", "NotePar4_GZPZ", NOTE_TYPE_GZPZ);       // 国债凭证-票号信息点
    NOTE_NO_PAR_MAP("NoteNo5_JSYWWTS", "NotePar5_JSYWWTS", NOTE_TYPE_JSYWWTS);// 结算业务委托书-票号信息点
    NOTE_NO_PAR_MAP("NoteNo6_XJZP", "NotePar6_XJZP", NOTE_TYPE_XJZP);       // 现金支票-票号信息点
    NOTE_NO_PAR_MAP("NoteNo7_ZZZP", "NotePar7_ZZZP", NOTE_TYPE_ZZZP);       // 转账支票-票号信息点
    NOTE_NO_PAR_MAP("NoteNo8_QFZZP", "NotePar8_QFZZP", NOTE_TYPE_QFJZP);    // 清分机支票-票号信息点
    NOTE_NO_PAR_MAP("NoteNo9_YHHP", "NotePar9_YHHP", NOTE_TYPE_YHHP);       // 银行汇票-票号信息点
    NOTE_NO_PAR_MAP("NoteNo10_YHCDHP", "NotePar10_YHCDHP", NOTE_TYPE_YHCDHP);// 银行承兑汇票-票号信息点
    NOTE_NO_PAR_MAP("NoteNo11_SYCDHP", "NotePar11_SYCDHP", NOTE_TYPE_SYCDHP);// 商业承兑汇票-票号信息点
    NOTE_NO_PAR_MAP("NoteNo12_FQFJBP", "NotePar12_FQFJBP", NOTE_TYPE_FQFJBP);// 非清分机本票-票号信息点
    NOTE_NO_PAR_MAP("NoteNo13_QFJBP", "NotePar13_QFJBP", NOTE_TYPE_QFJBP);   // 清分机本票-票号信息点


    // ----------------------------------------其他相关信息获取----------------------------------------
    // 读左右上下边距设置,单位:毫米,缺省0
    memset(&m_stMargin, 0, sizeof(RECT));
    m_stMargin.left = m_cXfsReg.GetValue("RECT_CFG", "Left", (DWORD)0);
    m_stMargin.right = m_cXfsReg.GetValue("RECT_CFG", "Right", (DWORD)0);
    m_stMargin.top = m_cXfsReg.GetValue("RECT_CFG", "Top", (DWORD)0);
    m_stMargin.bottom = m_cXfsReg.GetValue("RECT_CFG", "Bottom", (DWORD)0);

    m_stConfig.nVerifyField = m_cXfsReg.GetValue("CONFIG", "verify_field", (DWORD)0);
    if (m_stConfig.nVerifyField < 0 || m_stConfig.nVerifyField > 2)
        m_stConfig.nVerifyField = 0;

    return;
}

// 初始化状态类变量
HRESULT CXFS_CPR::InitStatus()
{
    memset(&m_stStatus, 0x00, sizeof(WFSPTRSTATUS));
    m_stStatus.fwDevice      = WFS_PTR_DEVNODEVICE;
    for (INT i = 0; i < WFS_PTR_SUPPLYSIZE; i ++)
    {
        m_stStatus.fwPaper[i]    = WFS_PTR_PAPERUNKNOWN;
    }
    m_stStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
    m_stStatus.fwToner       = WFS_PTR_TONERUNKNOWN;
    m_stStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
    m_stStatus.fwInk         = WFS_PTR_INKUNKNOWN;
    m_stStatus.lppRetractBins = nullptr;

    return WFS_SUCCESS;
}

// 初始化能力值类变量
HRESULT CXFS_CPR::InitCaps()
{
    memset(&m_sCaps, 0x00, sizeof(WFSPTRCAPS));
    m_sCaps.fwType = WFS_PTR_TYPEDOCUMENT;
    m_sCaps.lpusMaxRetract = nullptr;       // 每个回收箱可容纳媒介数目(INI获取)(结构体数组指针)
    m_sCaps.usRetractBins = 0;              // 回收箱个数(INI获取)

    // 回收箱统计: 状态 + 计数
    INT nRetractBins = 0;
    if ((nRetractBins = m_stNoteBoxList.nIsRetractBox()) > 0)
    {
        m_sCaps.usRetractBins = nRetractBins;

        if (m_sCaps.lpusMaxRetract != nullptr)
        {
            delete [] m_sCaps.lpusMaxRetract;
            m_sCaps.lpusMaxRetract = nullptr;
        }
        m_sCaps.lpusMaxRetract = new USHORT[nRetractBins];
        memset(m_sCaps.lpusMaxRetract, 0x00, sizeof(USHORT) * nRetractBins);
        INT nBoxSum = 0;
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (m_stNoteBoxList.stNoteBox[i].bIsHave == TRUE && m_stNoteBoxList.stNoteBox[i].usBoxType == BOX_RETRACT)
            {
                m_sCaps.lpusMaxRetract[nBoxSum] = m_stNoteBoxList.stNoteBox[i].usFullThreshold;
                nBoxSum ++;
            }
        }
    }

    return WFS_SUCCESS;
}

// 状态获取
void CXFS_CPR::UpdateDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题

    INT nRet = PTR_SUCCESS;
    DEVPTRSTATUS stDevStatus;
    BOOL    bNeedFirePrinterStatus  = FALSE;    // 需要上报打印状态变化事件
    BOOL    bNeedFirePaperStatus[16];           // 需要上报票箱状态变化事件
    BOOL    bNeedFireTonerStatus    = FALSE;    // 需要上报碳带状态变化事件
    BOOL    bNeedFireInkStatus      = FALSE;    // 需要上报墨盒状态变化事件
    BOOL    bNeedFireRetractStatus[16];         // 需要上报回收箱状态变化事件
    BOOL    bNeedFireHWError        = FALSE;    // 需要上报硬件错误事件
    BOOL    bNeedFirePaperTaken     = FALSE;    // 需要上报介质拿走事件
    memset(bNeedFirePaperStatus, FALSE, sizeof(bNeedFirePaperStatus));
    memset(bNeedFireRetractStatus, FALSE, sizeof(bNeedFireRetractStatus));


    WFSPTRSTATUS sLastStatus = m_stStatus;

    nRet = m_pPrinter->GetStatus(stDevStatus);
    if (nRet != PTR_SUCCESS)
    {
        if (nRet == ERR_PTR_NOT_OPEN)
        {
            m_stStatus.fwDevice = WFS_PTR_DEVOFFLINE;
        } else
        if (nRet == ERR_PTR_NO_DEVICE)
        {
            m_stStatus.fwDevice = WFS_PTR_DEVNODEVICE;
        } else
        {
            m_stStatus.fwDevice = WFS_PTR_DEVHWERROR;
        }
        for (INT i = 0; i < WFS_PTR_SUPPLYSIZE; i ++)
        {
            m_stStatus.fwPaper[i]    = WFS_PTR_PAPERUNKNOWN;
        }
        m_stStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
        m_stStatus.fwToner       = WFS_PTR_TONERUNKNOWN;
        m_stStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
        m_stStatus.fwInk         = WFS_PTR_INKUNKNOWN;
        m_stStatus.lppRetractBins = nullptr;
    } else
    {
        m_stStatus.fwDevice = ConvertDeviceStatus(stDevStatus.wDevice);
        m_stStatus.fwMedia = ConvertMediaStatus(stDevStatus.wMedia);
        for (INT i = 0; i < 16; i ++)
        {
            m_stStatus.fwPaper[i] = ConvertPaperStatus(stDevStatus.wPaper[i]);

            if (m_stStatus.fwPaper[i] == WFS_PTR_PAPERFULL) // 如果满,检查计数
            {
                INT nCount = m_stNoteBoxList.nGetBoxStat(i + 1);
                if (nCount == BOX_HIGHLOW)
                {
                    m_stStatus.fwPaper[i] = WFS_PTR_PAPERLOW;
                } else
                if (nCount == BOX_EMPTY)
                {
                    m_stStatus.fwPaper[i] = WFS_PTR_PAPEROUT;
                }
            }
        }
        m_stStatus.fwToner = ConvertTonerStatus(stDevStatus.wToner);
        m_stStatus.fwInk = ConvertInkStatus(stDevStatus.wInk);
        memcpy(&m_stWFSRetractBinOLD, &m_stWFSRetractBin, sizeof(WFSPTRRETRACTBINS));
        for (INT i = 0; i < 16; i++)
        {
            m_stWFSRetractBin[i].wRetractBin = ConvertRetractStatus(stDevStatus.stRetract[i].wBin);
            m_stWFSRetractBin[i].usRetractCount = stDevStatus.stRetract[i].usCount;
        }

        // Media原状态为出口有票＆当前状态为通道内无票＆n_WaitTaken为准执行Taken,设置Taken事件上报标记
        if ((m_stStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT) &&
            (sLastStatus.fwMedia == WFS_PTR_MEDIAENTERING) && m_WaitTaken == WTF_TAKEN)
        {
            bNeedFirePaperTaken = TRUE;
            m_WaitTaken = WTF_NONE;
        }

        // 票箱变化事件
        for (INT i = 0; i < 16; i ++)
        {
            if (m_stStatus.fwPaper[i] != sLastStatus.fwPaper[i])
            {
                if (m_stStatus.fwPaper[i] == WFS_PTR_PAPERLOW ||
                    m_stStatus.fwPaper[i] == WFS_PTR_PAPEROUT ||
                    m_stStatus.fwPaper[i] == WFS_PTR_PAPERFULL)
                {
                    bNeedFirePaperStatus[i] = TRUE;
                }
            }
        }

        // 碳带变化事件
        if (m_stStatus.fwToner != sLastStatus.fwToner)
        {
            if (m_stStatus.fwToner == WFS_PTR_TONERLOW ||
                m_stStatus.fwToner == WFS_PTR_TONEROUT ||
                m_stStatus.fwToner == WFS_PTR_TONERFULL)
            {
                bNeedFireTonerStatus = TRUE;
            }
        }

        // 墨盒变化事件
        if (m_stStatus.fwInk != sLastStatus.fwInk)
        {
            if (m_stStatus.fwToner == WFS_PTR_INKLOW ||
                m_stStatus.fwToner == WFS_PTR_INKOUT ||
                m_stStatus.fwToner == WFS_PTR_INKFULL)
            {
                bNeedFireInkStatus = TRUE;
            }
        }

        // 回收变化事件
        /*for (INT i = 0; i < 16; i ++)
        {
            if (m_stWFSRetractBin[i].wRetractBin == WFS_PTR_RETRACTBINOK)
            {
                    bNeedFireRetractStatus[i] = TRUE;
            }
        }*/
    }

    // Device状态有变化&当前Device状态为HWERR,设置HWERR事件上报标记
    if (m_stStatus.fwDevice != sLastStatus.fwDevice)
    {
        bNeedFirePrinterStatus = TRUE;
        if (m_stStatus.fwDevice == WFS_PTR_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }


    //--------事件上报处理--------

    if (bNeedFireHWError == TRUE)       // 上报Device HWERR事件
    {
        FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
    }

    if (bNeedFirePrinterStatus == TRUE)  // 上报状态变化事件
    {
        FireStatusChanged(m_stStatus.fwDevice);
    }

    for (INT i = 0; i < 16; i ++)       // 上报票箱状态变化
    {
        if (bNeedFirePaperStatus[i] == TRUE && ConvertPaperCode(i + 1) != 0)
        {
            FirePaperThreshold(ConvertPaperCode(i + 1), m_stStatus.fwPaper[i]);
        }
    }

    if (bNeedFireTonerStatus == TRUE)   // 上报碳带状态变化
    {
        FireTonerThreshold(m_stStatus.fwToner);
    }

    if (bNeedFireInkStatus == TRUE)     // 上报墨盒状态变化
    {
        FireInkThreshold(m_stStatus.fwInk);
    }

    if (bNeedFirePaperTaken == TRUE)    // 上报Taken事件
    {
        FireMediaTaken();
        Log(ThisModule, __LINE__, "介质被取走(Taken)");
    }

    /*for (INT i = 0; i < 16; i ++)       // 上报回收变化事件
    {

        if (bNeedFireRetractStatus[i] == TRUE)
        {
            FireRetractBinThreshold(i + 1,
                                    (m_stWFSRetractBin[i].wRetractBin == WFS_PTR_RETRACTNOTSUPP) ?
                                        WFS_PTR_RETRACTBININSERTED : WFS_PTR_RETRACTBINREMOVED);
        }
    }*/

    return;
}

// 介质控制处理
HRESULT CXFS_CPR::InnerControlMedia(DWORD dwControl)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;

    // Check: 参数互斥
    if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT &&
        (dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT) // 不支持支持参数: 退票+回收
    {
        Log(ThisModule, __LINE__, "入参dwControl=%d, 包含冲突入参. Return: %d.", dwControl, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 分别处理
    if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT ||     // 支持参数: 出票
        (dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT || // 支持参数: 回收
        (dwControl & WFS_PTR_CTRLPARK) == WFS_PTR_CTRLPARK ||       // 支持参数: 关门
        (dwControl & WFS_PTR_CTRLEXPEL) == WFS_PTR_CTRLEXPEL)       // 支持参数: 开门
    {
         MEDIA_ACTION enInput;
         USHORT usParam = 0;
         if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)      // 出票
         {
             enInput = MEDIA_CTR_EJECT;
             usParam = 5;
         } else
         if ((dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT)  // 回收
         {
             if (m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT)
             {
                 enInput = MEDIA_CTR_PERFORATE; // 通道内回收
             } else
             {
                 enInput = MEDIA_CTR_RETRACT;   // 出口回收
             }
         } else
         if ((dwControl & WFS_PTR_CTRLPARK) == WFS_PTR_CTRLPARK)        // 关门
         {
             enInput = MEDIA_CTR_PARK;
         } else
         if ((dwControl & WFS_PTR_CTRLEXPEL) == WFS_PTR_CTRLEXPEL)      // 开门
         {
             enInput = MEDIA_CTR_EXPEL;
         }

         nRet = m_pPrinter->MeidaControl(enInput, usParam);         
         UpdateDeviceStatus();
         if (nRet != PTR_SUCCESS)
         {
             if (nRet == ERR_PTR_JAMMED)
             {
                 m_bNeedKeepJammedStatus = TRUE;
             }

             Log(ThisModule, __LINE__, "m_pPrinter->MeidaControl(%d) Fail, ErrCode = %d, ReturnCode: %d",
                 enInput, nRet, ConvertErrCode(nRet));
             return ConvertErrCode(nRet);
         }

         // 命令成功后处理
         if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)      // 退票成功,设置Taken标记
         {
             m_WaitTaken = WTF_TAKEN;
         } else
         if ((dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT)  // 回收
         {
             // 统计: 票据计数: +1,写入INI记录
             SetRetractBoxCount(1, 1, TRUE);
         }
    } else
    {
        // 无效入参
        Log(ThisModule, __LINE__, "接收ControlMedia参数[%d]无效, ReturnCode: %d.",
            dwControl, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    return WFS_SUCCESS;
}

// 格式化打印处理
HRESULT CXFS_CPR::InnerPrintForm(LPWFSPTRPRINTFORM pInData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    CJsonObject cJsonPrtData;

    CSPPtrData *pData = (CSPPtrData *)m_pData;
    PrintContext pc;
    memset(&pc, 0, sizeof(pc));

    pc.pPrintData = pInData;

    pc.pForm = pData->FindForm(pInData->lpszFormName);
    if (!pc.pForm)
    {
        Log(ThisModule, __LINE__, "FORM<%s>未找到, Return: %d",
            pInData->lpszFormName, WFS_ERR_PTR_FORMNOTFOUND);
        return WFS_ERR_PTR_FORMNOTFOUND;
    }
    if (!pc.pForm->IsLoadSucc())
    {
        Log(ThisModule, __LINE__, "指定FORM名<%>无效, Return: %d",
            pInData->lpszFormName, WFS_ERR_PTR_FORMINVALID);
        return WFS_ERR_PTR_FORMINVALID;
    }
    pc.pMedia = pData->FindMedia(pInData->lpszMediaName);
    if (!pc.pMedia)
    {
        Log(ThisModule, __LINE__, "MEDIA<%s>未找到, Return: %d",
            pInData->lpszMediaName, WFS_ERR_PTR_MEDIANOTFOUND);
        return WFS_ERR_PTR_MEDIANOTFOUND;
    }
    if (!pc.pMedia->IsLoadSucc())
    {
        Log(ThisModule, __LINE__, "指定MEDIA名<%>无效, Return: %d",
            pInData->lpszMediaName, WFS_ERR_PTR_MEDIAINVALID);
        return WFS_ERR_PTR_MEDIAINVALID;
    }

    // 检查是否Media可打印区域超过其自身大小
    do
    {
        RECT rectMedia = {0, 0, 0, 0};
        pc.pMedia->GetPrintArea(rectMedia);
        SIZE sizeMedia = pc.pMedia->GetSize();
        if (rectMedia.right > sizeMedia.cx ||
            rectMedia.bottom > sizeMedia.cy)
        {
            Log(ThisModule, __LINE__, "指定MEDIA<%>无效,出现越界, Return: %d",
                pc.pMedia->GetName(), WFS_ERR_PTR_MEDIAINVALID);
            return WFS_ERR_PTR_MEDIAINVALID;
        }
    } while (0);

    RECT rcMD;
    ((CSPPrinterForm *)pc.pForm)->GetMulDiv(rcMD);

    if (pInData->wOffsetX == WFS_PTR_OFFSETUSEFORMDEFN)
    {
        pInData->wOffsetX = pc.pForm->GetPosition().cx;
    }
    if (pInData->wOffsetY == WFS_PTR_OFFSETUSEFORMDEFN)
    {
        pInData->wOffsetY = pc.pForm->GetPosition().cy;
    }
    SIZE offset;
    offset.cx = MulDiv(pInData->wOffsetX, rcMD.left, rcMD.right);
    offset.cy = MulDiv(pInData->wOffsetY, rcMD.top, rcMD.bottom);

    SIZE sizeForm = pc.pForm->GetSize();
    SIZE sizeMedia = pc.pMedia->GetSize();
    if ((sizeMedia.cx > 0 && offset.cx + sizeForm.cx > sizeMedia.cx) ||
        (sizeMedia.cy > 0 && offset.cy + sizeForm.cy > sizeMedia.cy))
    {
        Log(ThisModule, __LINE__, "FORM<%s>超出MEDIA<%s>.SIZE属性指定范围(包含OffsetX,OffsetY入参), Return: %d",
            pInData->lpszFormName, pInData->lpszMediaName, WFS_ERR_PTR_MEDIAOVERFLOW);
        return WFS_ERR_PTR_MEDIAOVERFLOW;
    }
    dwMediaWidth = sizeMedia.cx;
    dwMediaHeight = sizeMedia.cy;

    RECT rc;
    pc.pMedia->GetPrintArea(rc);
    if (//ffset.cx < rc.left ||
        (rc.right - rc.left > 0 && sizeForm.cx + offset.cx > rc.right - rc.left) ||
        //offset.cy < rc.top ||
        (rc.bottom - rc.top > 0 && sizeForm.cy + offset.cy > rc.bottom - rc.top))
    {
        Log(ThisModule, __LINE__, "FORM<%s>超出MEDIA<%s>.PRINTAREA属性指定范围(包含OffsetX,OffsetY入参), Return: %d",
            pInData->lpszFormName, pInData->lpszMediaName, WFS_ERR_PTR_MEDIAOVERFLOW);
        return WFS_ERR_PTR_MEDIAOVERFLOW;
    }
    FORMALIGN FormAlign = pc.pForm->GetAlign();
    if (pInData->wAlignment != WFS_PTR_ALNUSEFORMDEFN &&
        pInData->wAlignment != WFS_PTR_ALNTOPLEFT &&
        pInData->wAlignment != WFS_PTR_ALNTOPRIGHT &&
        pInData->wAlignment != WFS_PTR_ALNBOTTOMLEFT &&
        pInData->wAlignment != WFS_PTR_ALNBOTTOMRIGHT )
    {
        Log(ThisModule, __LINE__, "入参Alignment[%d]无效, Return: %d",
            pInData->wAlignment, WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }
    if (pInData->wAlignment != WFS_PTR_ALNUSEFORMDEFN)
    {
        FormAlign = (FORMALIGN)(pInData->wAlignment - WFS_PTR_ALNTOPLEFT + TOPLEFT);
    }
    switch (FormAlign)
    {
        case TOPLEFT:
            break;  //(default)
        case TOPRIGHT:
            if (sizeMedia.cx > 0)
            {
                offset.cx = sizeMedia.cx - sizeForm.cx - offset.cx;
            }
            break;
        case BOTTOMLEFT:
            if (sizeMedia.cy > 0)
            {
                offset.cy = sizeMedia.cy - sizeForm.cy - offset.cy;
            }
            break;
        case BOTTOMRIGHT:
            if (sizeMedia.cx > 0)
            {
                offset.cx = sizeMedia.cx - sizeForm.cx - offset.cx;
            }
            if (sizeMedia.cy > 0)
            {
                offset.cy = sizeMedia.cy - sizeForm.cy - offset.cy;
            }
            break;
        default:
            return WFS_ERR_INVALID_DATA;
    }

    CMultiString Fields = pInData->lpszFields;

    //功能：打印字段内容或FRAME
    if (m_stConfig.nVerifyField > 0) // 需要进行Field检查
    {
        do
        {
            for (int i = 0; i < Fields.GetCount(); i++)
            {
                LPCSTR lpField = Fields.GetAt(i);
                if (lpField == nullptr)
                {
                    continue;
                }
                char szFieldName[1024] = {0};
                for (int j = 0; j < (int)strlen(lpField) && j < 1023; j++)
                {
                    if (lpField[j] != '=' && lpField[j] != '\0')
                    {
                        szFieldName[j] = lpField[j];
                    }
                }
                if (strcmp(szFieldName, "") == 0)
                {
                    continue;
                }
                DWORD iChild = 0;
                for (; iChild < pc.pForm->GetSubItemCount(); iChild++)
                {
                    if (strcmp(pc.pForm->GetSubItem(iChild)->GetName(), szFieldName) == 0)
                    {
                        break;
                    }
                }
                if (iChild == pc.pForm->GetSubItemCount())
                {
                    Log(ThisModule, __LINE__, "FIELD<%>未找到, Return: %d",
                        szFieldName, WFS_ERR_PTR_FIELDNOTFOUND);
                    if (m_stConfig.nVerifyField == 1)
                    {
                        return WFS_ERR_PTR_FIELDNOTFOUND;
                    } else
                    {
                        FireFieldWarning(pInData->lpszFormName, szFieldName, WFS_PTR_FIELDNOTFOUND);
                    }
                }
            }

        } while(0);
    }

    HRESULT hRet = StartForm(&pc);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, Result2ErrorCode(hRet), "StartForm<%s>失败, Return: %d",
            pInData->lpszFormName, hRet);
        return hRet;
    }

    for (DWORD iChild = 0; iChild < pc.pForm->GetSubItemCount() && hRet == WFS_SUCCESS; iChild++)
    {
        ISPPrinterItem *pItem = pc.pForm->GetSubItem(iChild);
        SIZE3 SubOffset;
        SubOffset.cx = SubOffset.cy = SubOffset.cz = 0;
        pc.pSubform = NULL;
        if (pItem->GetItemType() == ITEM_SUBFORM)
        {
            pc.pSubform = (ISPPrinterSubform *)pItem;
            SubOffset = pc.pSubform->GetPosition();
        }
        for (DWORD iField = 0; (!pc.pSubform || iField < pc.pSubform->GetSubItemCount()) && hRet == WFS_SUCCESS; iField++)
        {
            if (pc.pSubform)
            {
                pItem = pc.pSubform->GetSubItem(iField);
            }
            SIZE OffsetAll = offset;
            OffsetAll.cx += SubOffset.cx;
            OffsetAll.cy += SubOffset.cy;
            hRet = PrintFieldOrFrame(pc, pItem, OffsetAll, Fields);
            if (!pc.pSubform)
            {
                break;
            }
        }
    }

    if (WFS_SUCCESS != hRet)
    {
        pc.bCancel = TRUE;
    }

    HRESULT hRetOld = hRet;
    hRet = EndForm(&pc);
    return  WFS_SUCCESS != hRetOld ? hRetOld : hRet;
}

HRESULT CXFS_CPR::InnerReadForm(LPWFSPTRREADFORM pInData)
{
    THISMODULE(__FUNCTION__);
    CSPPtrData *pData = static_cast<CSPPtrData *>(m_pData);

    HRESULT hRet = WFS_SUCCESS;
    ReadContext rc;
    ZeroMemory(&rc, sizeof(rc));
    rc.pReadData = pInData;

    if (pInData->lpszFormName != nullptr && strlen(pInData->lpszFormName) > 0)
    {
        rc.pForm = pData->FindForm(pInData->lpszFormName);
        if (!rc.pForm)
        {
            Log(ThisModule, __LINE__, "FORM<%s>未找到, Return: %d",
                pInData->lpszFormName, WFS_ERR_PTR_FORMNOTFOUND);
            return WFS_ERR_PTR_FORMNOTFOUND;
        }
        if (!rc.pForm->IsLoadSucc())
        {
            Log(ThisModule, __LINE__, "指定FORM名<%>无效, Return: %d",
                pInData->lpszFormName, WFS_ERR_PTR_FORMINVALID);
            return WFS_ERR_PTR_FORMINVALID;
        }
    }
    if (pInData->lpszMediaName != nullptr && strlen(pInData->lpszMediaName) > 0)
    {
        rc.pMedia = pData->FindMedia(pInData->lpszMediaName);
        if (!rc.pMedia)
        {
            Log(ThisModule, __LINE__, "MEDIA<%s>未找到, Return: %d",
                pInData->lpszFormName, WFS_ERR_PTR_MEDIANOTFOUND);
            return WFS_ERR_PTR_MEDIANOTFOUND;
        }
        if (!rc.pMedia->IsLoadSucc())
        {
            Log(ThisModule, __LINE__, "指定MEDIA名<%>无效, Return: %d",
                pInData->lpszFormName, WFS_ERR_PTR_MEDIAINVALID);
            return WFS_ERR_PTR_MEDIAINVALID;
        }
    }

    CMultiString Fields = pInData->lpszFieldNames;

    //  如果参数为空，即未指定任何磁道，则要读所有在Form文件中的磁道
    if (Fields.GetCount() == 0)
    {
        for (DWORD iChild = 0; iChild < rc.pForm->GetSubItemCount() && WFS_SUCCESS == hRet; iChild++)
        {
            ISPPrinterItem *pItem = rc.pForm->GetSubItem(iChild);
            Fields.Add(pItem->GetName());
        }
    }

    hRet = StartReadForm(&rc);
    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, Result2ErrorCode(hRet), "StartReadForm(%s) Fail, Return: %d",
            pInData->lpszFormName, hRet);
        return hRet;
    }
    for (DWORD iChild = 0; iChild < rc.pForm->GetSubItemCount() && WFS_SUCCESS == hRet; iChild++)
    {
        ISPPrinterItem *pItem = rc.pForm->GetSubItem(iChild);

        //  如果Form文件中的该字段，不在用户的执行命令时的参数中，则不读
        bool bUserRead = false;
        for (int i1 = 0; i1 < Fields.GetCount(); ++i1)
        {
            if (strcmp(Fields.GetAt(i1), pItem->GetName()) == 0)
            {
                bUserRead = true;
                break;
            }
        }
        if (!bUserRead)
            continue;

        // pItem->GetName();
        rc.pSubform = nullptr;
        if (pItem->GetItemType() == ITEM_SUBFORM)
        {
            rc.pSubform = (ISPPrinterSubform *)pItem;
        }
        for (DWORD iField = 0; (!rc.pSubform || iField < rc.pSubform->GetSubItemCount()) && hRet == WFS_SUCCESS; iField++)
        {
            if (rc.pSubform)
            {
                pItem = rc.pSubform->GetSubItem(iField);
            }
            hRet = ReadFieldOrFrame(rc, pItem, Fields);
            if (!rc.pSubform || hRet == WFS_SUCCESS)
            {
                break;
            }
        }
    }

    if (hRet != WFS_SUCCESS)
        rc.bCancel = TRUE;

    HRESULT hRetOld = hRet;
    hRet = EndReadForm(&rc);
    return hRetOld != WFS_SUCCESS ? hRetOld : hRet;
}

HRESULT CXFS_CPR::InnerReadImage(LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LPWFSPTRIMAGEREQUEST lpIn = nullptr;
    LPWFSPTRIMAGE lpOut = nullptr;
    DEVPTRREADIMAGEIN stScanImageIn;
    DEVPTRREADIMAGEOUT stScanImageOut;
    CJsonObject cJsonData;
    WORD    wNoteType = 0;
    HRESULT hRet = WFS_SUCCESS;

    INT nRet = 0;

    // Check: 入参
    if (lpImgRequest == nullptr)
    {
        Log(ThisModule, __LINE__, "入参 Is NULL, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    lpIn = (LPWFSPTRIMAGEREQUEST)lpImgRequest;
    if (lpImgRequest->fwImageSource == 0)
    {
        Log(ThisModule, __LINE__, "入参 lpImgRequest->fwImageSource == 0, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    INT nImageSource[4] = { WFS_PTR_IMAGEUPPER, WFS_PTR_IMAGEEXTERNAL, WFS_PTR_IMAGELOWER, WFS_PTR_IMAGEAUX };
    BOOL bIsHave = FALSE;
    for (INT i = 0; i < 4; i ++)
    {
        if(((lpIn->fwImageSource & nImageSource[i]) == nImageSource[i]) && (bIsHave == FALSE))
        {
            bIsHave = TRUE;
            continue;
        }
        if(((lpIn->fwImageSource & nImageSource[i]) == nImageSource[i]) && (bIsHave == TRUE))
        {
            Log(ThisModule, __LINE__, "入参fwImageSource=%d, 包含冲突入参. Return: %d.",
                lpIn->fwImageSource, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    // --------检查入参并组织下发到DevCPR的入参--------
    stScanImageIn.Clear();
    cJsonData.Clear();
    if ((lpIn->fwImageSource & WFS_PTR_CODELINE) == WFS_PTR_CODELINE ||             // 获取票面信息并鉴伪
        (lpIn->fwImageSource & WFS_PTR_IMAGEUPPER) == WFS_PTR_IMAGEUPPER ||         // 从1号票箱获取票号等信息
        (lpIn->fwImageSource & WFS_PTR_IMAGEEXTERNAL) == WFS_PTR_IMAGEEXTERNAL ||   // 从2号票箱获取票号等信息
        (lpIn->fwImageSource & WFS_PTR_IMAGELOWER) == WFS_PTR_IMAGELOWER ||         // 从3号票箱获取票号等信息
        (lpIn->fwImageSource & WFS_PTR_IMAGEAUX) == WFS_PTR_IMAGEAUX)               // 从4号票箱获取票号等信息
    {
        // 不检查银行别码
        if (strlen(m_stConfig.szBankCode) == 0 || MCMP_IS0(m_stConfig.szBankCode, "000"))
        {
            // 根据入参指定票号识别Form转换为JSON或使用INI缺省JSON
            if (lpIn->lpszBackImageFile == nullptr || strlen(lpIn->lpszBackImageFile) < 1)  // 要识别的票号Form名
            {
                cJsonData.Clear();
                if (m_stConfig.usBank == BANK_NO_JIANGNAN) // 江南银行
                {
                    cJsonData.Parse(NOTE_GETJSON2);
                } else
                {
                    GetDefFieldToJSON(nullptr, cJsonData);
                }
                Log(ThisModule, __LINE__, "入参lpszBackImageFile=%s, 未设置票号/票面识别Form(采用INI设置自动识别), INI Set= %s.",
                    lpIn->lpszFrontImageFile, cJsonData.ToString().c_str());
            } else
            {
                // 取Form中Field信息写入JSON
                if ((hRet = GetFormFieldToJSON(lpIn->lpszBackImageFile, cJsonData)) != WFS_SUCCESS)
                {
                    return hRet;
                }
            }
        } else
        {
            // 根据入参验证银行别码并获取票据类型
            if (lpIn->lpszFrontImageFile == nullptr || strlen(lpIn->lpszFrontImageFile) < 1)    // 要识别的票据信息
            {
                Log(ThisModule, __LINE__, "入参lpszFrontImageFile=%s, 未设置要识别的票据信息. Return: %d.",
                    lpIn->lpszFrontImageFile, WFS_ERR_UNSUPP_DATA);
                return WFS_ERR_UNSUPP_DATA;
            } else // 取入参票据类型
            {
                if (strlen(lpIn->lpszFrontImageFile) < 9 ||
                    memcmp(lpIn->lpszFrontImageFile, "Type=", 5) != 0)
                {
                    Log(ThisModule, __LINE__, "入参lpszFrontImageFile=<%s>, 指定要识别的票据信息格式错误. Return: %d.",
                        lpIn->lpszFrontImageFile, WFS_ERR_UNSUPP_DATA);
                    return WFS_ERR_UNSUPP_DATA;
                }
                if (m_stConfig.usBank == BANK_NO_CSBC) // 长沙银行
                {
                    if (memcmp(lpIn->lpszFrontImageFile + 5, m_stConfig.szBankCode, strlen(m_stConfig.szBankCode)) != 0)
                    {
                        Log(ThisModule, __LINE__, "入参lpszFrontImageFile=<%s>, 指定要识别的票据银行别码错误. Return: %d.",
                            lpIn->lpszFrontImageFile, WFS_ERR_UNSUPP_DATA);
                        return WFS_ERR_UNSUPP_DATA;
                    }
                }
                // 根据银行编号获取票据类型
                if ((wNoteType = NoteTypeConvert(lpIn->lpszFrontImageFile + 8, m_stConfig.usBank)) == 0)
                {
                    Log(ThisModule, __LINE__, "入参lpszFrontImageFile=%s, 指定要识别的票据类型无效. Return: %d.",
                        lpIn->lpszFrontImageFile, WFS_ERR_UNSUPP_DATA);
                    return WFS_ERR_UNSUPP_DATA;
                }
            }

            // 根据入参指定票号识别Form转换为JSON或使用INI缺省JSON
            if (lpIn->lpszBackImageFile == nullptr || strlen(lpIn->lpszBackImageFile) < 1)  // 要识别的票号Form名
            {
                cJsonData.Clear();
                if ((lpIn->fwImageSource & WFS_PTR_CODELINE) == WFS_PTR_CODELINE)    // 获取票面信息并鉴伪
                {
                    GetDefFieldToJSON((LPSTR)(m_stConfig.m_Map_NotePar_GetList.at(wNoteType).c_str()), cJsonData);
                } else
                {
                    GetDefFieldToJSON((LPSTR)(m_stConfig.m_Map_NoteNo_GetList.at(wNoteType).c_str()), cJsonData);
                }
                Log(ThisModule, __LINE__, "入参lpszBackImageFile=%s, 未设置票号/票面识别Form(采用INI设置自动识别), INI Set= %s.",
                    lpIn->lpszFrontImageFile, cJsonData.ToString().c_str());
            } else
            {
                // 取Form中Field信息写入JSON
                if ((hRet = GetFormFieldToJSON(lpIn->lpszBackImageFile, cJsonData)) != WFS_SUCCESS)
                {
                    return hRet;
                }
            }
        }

        if ((lpIn->fwImageSource & WFS_PTR_CODELINE) == WFS_PTR_CODELINE)   // 获取票据号并鉴伪
        {
            if (m_stConfig.usBank == BANK_NO_JIANGNAN) // 江南银行
            {
                stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_UPPER);
            } else
            {
                stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_CODELINE);
            }
        } else
        {
            // 从1号票箱获取票号等信息
            WORD wNoteNo = 0;
            if ((lpIn->fwImageSource & WFS_PTR_IMAGEUPPER) == WFS_PTR_IMAGEUPPER)
            {
                wNoteNo = 1;
                stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_UPPER);
            }
            // 从2号票箱获取票号等信息
            if ((lpIn->fwImageSource & WFS_PTR_IMAGELOWER) == WFS_PTR_IMAGELOWER)
            {
                wNoteNo = 2;
                stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_LOWER);
            }
            // 从3号票箱获取票号等信息
            if ((lpIn->fwImageSource & WFS_PTR_IMAGEEXTERNAL) == WFS_PTR_IMAGEEXTERNAL)
            {
                wNoteNo = 3;
                stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_EXTERNAL);
            }
            // 从4号票箱获取票号等信息
            if ((lpIn->fwImageSource & WFS_PTR_IMAGEAUX) == WFS_PTR_IMAGEAUX)
            {
                wNoteNo = 4;
                stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_AUX);
            }

            nRet = m_stNoteBoxList.nBoxIsHave(wNoteNo);
            if (nRet == BOX_NOHAVE)
            {
                Log(ThisModule, __LINE__, "入参fwImageSource[%d]指定票据箱[%d]不存在或非存储箱. Return: %d.",
                    lpIn->fwImageSource, wNoteNo, WFS_ERR_UNSUPP_DATA);
                return WFS_ERR_UNSUPP_DATA;
            }
            if (wNoteType != m_stNoteBoxList.nGetNoteTypeIsBox(wNoteNo))
            {
                Log(ThisModule, __LINE__, "入参fwImageSource[%d]指定票据箱[%d]与lpszFrontImageFile[%s]指定票据类型不符合. Return: %d.",
                    lpIn->fwImageSource, wNoteNo, lpIn->lpszFrontImageFile, WFS_ERR_UNSUPP_DATA);
                return WFS_ERR_UNSUPP_DATA;
            }
        }
    } else  // 只扫描图像
    if ((lpIn->fwImageSource & WFS_PTR_IMAGEFRONT) == WFS_PTR_IMAGEFRONT ||   // 扫描正面图像
        (lpIn->fwImageSource & WFS_PTR_IMAGEBACK) == WFS_PTR_IMAGEBACK)       // 扫描背面图像
    {
        if ((lpIn->fwImageSource & WFS_PTR_IMAGEFRONT) == WFS_PTR_IMAGEFRONT)   // 扫描正面图像
        {
            if (lpIn->lpszFrontImageFile == nullptr || strlen(lpIn->lpszFrontImageFile) < 1)
            {
                Log(ThisModule, __LINE__, "入参lpszFrontImageFile支持IMAGEFRONT[扫描正面图像], 未设置图像路径参数. Return: %d.", WFS_ERR_UNSUPP_DATA);
                return WFS_ERR_UNSUPP_DATA;
            }

            // 正面图像路径check
            if (FileAccess::create_directory_by_path(lpIn->lpszFrontImageFile, true) != true)
            {
                Log(ThisModule, __LINE__, "入参lpszFrontImageFile支持IMAGEFRONT[扫描正面图像], 指定图像路径[%s]建立失败. Return: %d.",
                   lpIn->lpszFrontImageFile, WFS_ERR_UNSUPP_DATA);
                return WFS_ERR_UNSUPP_DATA;
            }

            stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_FRONT);
            memcpy(stScanImageIn.szImageFrontPath, lpIn->lpszFrontImageFile, strlen(lpIn->lpszFrontImageFile));
            cJsonData.Add(JSON_KEY_IMAGE_FRONT_PATH, lpIn->lpszFrontImageFile);
        }

        if ((lpIn->fwImageSource & WFS_PTR_IMAGEBACK) == WFS_PTR_IMAGEBACK) // 扫描背面图像
        {
            if (lpIn->lpszBackImageFile == nullptr || strlen(lpIn->lpszBackImageFile) < 1)
            {
                Log(ThisModule, __LINE__, "入参lpszFrontImageFile支持IMAGEBACK[扫描背面图像], 未设置图像保存路径参数. Return: %d.", WFS_ERR_UNSUPP_DATA);
                return WFS_ERR_UNSUPP_DATA;
            }

            // 背面图像路径check
            if (FileAccess::create_directory_by_path(lpIn->lpszBackImageFile, true) != true)
            {
                Log(ThisModule, __LINE__, "入参lpszFrontImageFile支持IMAGEFRONT[扫描背面图像], 指定图像保存路径[%s]建立失败. Return: %d.",
                   lpIn->lpszBackImageFile, WFS_ERR_UNSUPP_DATA);
                return WFS_ERR_UNSUPP_DATA;
            }

            stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_BACK);
            memcpy(stScanImageIn.szImageBackPath, lpIn->lpszBackImageFile, strlen(lpIn->lpszBackImageFile));
            cJsonData.Add(JSON_KEY_IMAGE_BACK_PATH, lpIn->lpszBackImageFile);
        }
    }

    // 获取RFID数据
    if ((lpIn->fwImageSource & WFS_PTR_RFID) == WFS_PTR_RFID)
    {
        stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_RFID);
    }

    // Check 入参处理后是否包含有效数据
    if (stScanImageIn.wInMode == 0)
    {
        Log(ThisModule, __LINE__, "入参 lpImgRequest->fwImageSource == %d, 无效入参, Return: %d.",
            lpImgRequest->fwImageSource, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    nRet = PTR_SUCCESS;
    stScanImageOut.Clear();
    INT nLen = cJsonData.ToString().length();
    stScanImageIn.lpData = new CHAR[nLen + 1];
    memset(stScanImageIn.lpData, 0x00, nLen + 1);
    memcpy(stScanImageIn.lpData, (LPSTR)cJsonData.ToString().c_str(), nLen);
    nRet = m_pPrinter->ReadImage(stScanImageIn, stScanImageOut);
    stScanImageIn.Clear();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "->ScanImage() Fail, ErrCode = %d, Return: %d.",
            nRet, ConvertErrCode(nRet));
        //return ConvertErrCode(nRet);
    }

    // 记录应答Data到Log
    Log(ThisModule, __LINE__, "->ScanImage() succ, wInMode = %d, wResult = %d, lpData=%s.",
        stScanImageOut.wInMode, stScanImageOut.wResult,
        stScanImageOut.lpData == nullptr ? "(null)" : stScanImageOut.lpData);

    // 组织应答
    lpOut = new WFSPTRIMAGE();
    lpOut->wImageSource = lpIn->fwImageSource;
    lpOut->ulDataLength = 0;
    lpOut->lpbData = nullptr;
    if (stScanImageOut.wResult == READIMAGE_RET_OK)
    {
        lpOut->wStatus = WFS_PTR_DATAOK;
    } else
    if (stScanImageOut.wResult == READIMAGE_RET_MISSING)
    {
        lpOut->wStatus = WFS_PTR_DATASRCMISSING;
    } else
    if (stScanImageOut.wResult == READIMAGE_RET_NOTSUPP)
    {
        lpOut->wStatus = WFS_PTR_DATASRCNOTSUPP;
    }

    if (stScanImageOut.wResult == READIMAGE_RET_OK ||
        stScanImageOut.wResult == READIMAGE_RET_MISSING)
    {
        std::string stdData;

        if ((stScanImageOut.wInMode & IMAGE_MODE_CODELINE) == IMAGE_MODE_CODELINE &&
            stScanImageOut.lpData != nullptr)
        {
            cJsonData.Clear();
            if (cJsonData.Parse(stScanImageOut.lpData) != true)
            {
                Log(ThisModule, __LINE__, "->JSON Parse(%s) Fail, Return: %d.", stScanImageOut.lpData, WFS_ERR_INTERNAL_ERROR);
                return ConvertErrCode(nRet);
            }

            INT nCount = 0;
            std::string stdTmp;
            std::string stdCardNo, stdAccount, stdAmount;
            CHAR szKeyName[24];
            cJsonData.Get(JSON_KEY_IDEN_CNT, nCount);

            for (INT i = 1; i <= nCount; i ++)
            {
                stdTmp.clear();
                if (stdData.length() > 0)   // 非第一分割数据时,追加"|"标记
                {
                    stdData.append("|");
                }

                // 取Key
                memset(szKeyName, 0x00, sizeof(szKeyName));
                sprintf(szKeyName, "%s%d", JSON_KEY_IDEN_NAME, i);
                if (cJsonData.Get(szKeyName, stdTmp) != true)
                {
                    Log(ThisModule, __LINE__, "->JSON[%s] Get(%s) Fail, Return: %d.",
                        cJsonData.ToString().c_str(),szKeyName, WFS_ERR_INTERNAL_ERROR);
                    return WFS_ERR_INTERNAL_ERROR;
                }

                if (m_stConfig.usBank == BANK_NO_CSBC)  // 长沙银行
                {
                    stdData.append(stdTmp);
                    stdData.append("=");
                }

                // 取Value
                stdTmp.clear();
                memset(szKeyName, 0x00, sizeof(szKeyName));
                sprintf(szKeyName, "%s%d", JSON_KEY_IDEN_VALUE, i);
                if (cJsonData.Get(szKeyName, stdTmp) != true)
                {
                    Log(ThisModule, __LINE__, "->JSON[%s] Get(%s) Fail, Return: %d.",
                        cJsonData.ToString().c_str(), szKeyName, WFS_ERR_INTERNAL_ERROR);
                    return WFS_ERR_INTERNAL_ERROR;
                }
                stdData.append(stdTmp);
            }
        } else
        if (((stScanImageOut.wInMode & IMAGE_MODE_UPPER) == IMAGE_MODE_UPPER ||
             (stScanImageOut.wInMode & IMAGE_MODE_LOWER) == IMAGE_MODE_LOWER ||
             (stScanImageOut.wInMode & IMAGE_MODE_EXTERNAL) == IMAGE_MODE_EXTERNAL ||
             (stScanImageOut.wInMode & IMAGE_MODE_AUX) == IMAGE_MODE_AUX) &&
             stScanImageOut.lpData != nullptr)
        {
            cJsonData.Clear();
            if (cJsonData.Parse(stScanImageOut.lpData) != true)
            {
                Log(ThisModule, __LINE__, "->JSON Parse(%s) Fail, Return: %d.", stScanImageOut.lpData, WFS_ERR_INTERNAL_ERROR);
                return ConvertErrCode(nRet);
            }

            std::string stdTmp;
            stdTmp.clear();
            if (cJsonData.Get(JSON_KEY_IDEN_NAME, stdTmp) != true)
            {
                Log(ThisModule, __LINE__, "->JSON[%s] Get(%s) Fail, Return: %d.",
                    cJsonData.ToString().c_str(), JSON_KEY_IDEN_NAME, WFS_ERR_INTERNAL_ERROR);
                return WFS_ERR_INTERNAL_ERROR;
            }

            if (m_stConfig.usBank == BANK_NO_CSBC)  // 长沙银行
            {
                stdData.append(stdTmp);
                stdData.append("=");
            }

            stdTmp.clear();
            if (cJsonData.Get(JSON_KEY_IDEN_VALUE, stdTmp) != true)
            {
                Log(ThisModule, __LINE__, "->JSON[%s] Get(%s) Fail, Return: %d.",
                    cJsonData.ToString().c_str(), JSON_KEY_IDEN_VALUE, WFS_ERR_INTERNAL_ERROR);
                return WFS_ERR_INTERNAL_ERROR;
            }
            stdData.append(stdTmp);
        } else
        if ((stScanImageOut.wInMode & IMAGE_MODE_RFID) == IMAGE_MODE_RFID &&
             stScanImageOut.lpData != nullptr)
        {
            cJsonData.Clear();
            if (cJsonData.Parse(stScanImageOut.lpData) != true)
            {
                Log(ThisModule, __LINE__, "->JSON Parse(%s) Fail, Return: %d.", stScanImageOut.lpData, WFS_ERR_INTERNAL_ERROR);
                return ConvertErrCode(nRet);
            }

            cJsonData.Get(JSON_KEY_RFID_DATA, stdData);
            if (cJsonData.Get(JSON_KEY_RFID_DATA, stdData) != true)
            {
                Log(ThisModule, __LINE__, "->JSON[%s] Get(%s) Fail, Return: %d.",
                    cJsonData.ToString().c_str(), JSON_KEY_RFID_DATA, WFS_ERR_INTERNAL_ERROR);
                return WFS_ERR_INTERNAL_ERROR;
            }
        }

        // 有应答数据
        if (stdData.length() > 0)
        {
            lpOut->ulDataLength = stdData.length();
            lpOut->lpbData = new BYTE[stdData.length() + 1];
            memset(lpOut->lpbData, 0x00, stdData.length() + 1);
            memcpy(lpOut->lpbData, (LPBYTE)stdData.c_str(), stdData.length());
        }
        Log(ThisModule, __LINE__, "->返回应答数据 wImageSource = %d, wStatus = %d, ulDataLength = %d, lpbData=%s.",
            lpOut->wImageSource, lpOut->wStatus, lpOut->ulDataLength,
            (lpOut->lpbData == nullptr ? "(null)" : (LPSTR)lpOut->lpbData));
    }

    lppImage = new LPWFSPTRIMAGE[2];
    //*lppImage = lpOut;
    lppImage[0] = lpOut;
    lppImage[1] = nullptr;

    stScanImageOut.Clear();

    return WFS_SUCCESS;
}

HRESULT CXFS_CPR::InnerRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// 设置出票箱计数并记录到INI
void CXFS_CPR::SetStorageBoxCount(USHORT usBoxNo, USHORT usCnt, BOOL bIsAdd)
{
    THISMODULE(__FUNCTION__);

    INT nRet = 0;
    INT nCount = 0;
    CHAR szKeyName[MAX_PATH] = { 0x00 };
    nCount = m_stNoteBoxList.nSetStorageCount(usBoxNo, usCnt, bIsAdd);

    // 取顺序索引
    nRet = m_stNoteBoxList.nGetBoxOrder(usBoxNo);
    if (nRet == BOX_ISHAVE)
    {
        Log(ThisModule, __LINE__, "设置出票箱计数并记录到INI->取发票箱[%d]顺序索引未找到,不更新INI.", usBoxNo);
        return;
    }

    // 设置INI中指定票据箱计数
    sprintf(szKeyName, "NOTEBOX_%d", nRet);
    m_cXfsReg.SetValue(szKeyName, "NoteCount", std::to_string(nCount).c_str());

    return;
}

// 设置回收箱计数并记录
// usBoxNo: 箱号, usCnt: 计数, bIsAdd:T增加/F减少
void CXFS_CPR::SetRetractBoxCount(USHORT usBoxNo, USHORT usCnt, BOOL bIsAdd)
{
    THISMODULE(__FUNCTION__);

    INT nRet = 0;
    INT nCount = 0;
    CHAR szKeyName[MAX_PATH] = { 0x00 };
    nCount = m_stNoteBoxList.nSetRetractCount(usBoxNo, usCnt, bIsAdd);

    // 取顺序索引
    nRet = m_stNoteBoxList.nGetBoxOrder(usBoxNo, BOX_RETRACT);
    if (nRet == BOX_NOHAVE)
    {
        Log(ThisModule, __LINE__, "设置回收箱计数并记录到INI->取回收箱[%d]顺序索引未找到,不更新INI.", usBoxNo);
        return;
    }

    // 设置INI中指定票据箱计数
    sprintf(szKeyName, "NOTEBOX_%d", nRet);
    m_cXfsReg.SetValue(szKeyName, "NoteCount", std::to_string(nCount).c_str());

    return;
}

//-----------------------------------------------------------------------------------
//------------------------------------格式转换WFS-------------------------------------
// 票箱号转换为WFS格式
WORD CXFS_CPR::ConvertPaperCode(INT nCode)
{
    switch (nCode)
    {
        case 1: return WFS_PTR_PAPERUPPER;      // 票箱1
        case 2: return WFS_PTR_PAPERLOWER;      // 票箱2
        case 3: return WFS_PTR_PAPEREXTERNAL;   // 票箱3
        case 4: return WFS_PTR_PAPERAUX;        // 票箱4
        default: return 0;
    }
}

// 指定票据类型转换为DevCPR定义
WORD CXFS_CPR::NoteTypeConvert(LPSTR lpNoteType, WORD wBank)
{
    #define IF_CMCP(X, Y, Z) \
        if (memcmp(X, Y, strlen(X)) == 0 && memcmp(X, Y, strlen(Y)) == 0) \
        return Z

    if (wBank == BANK_NO_CSBC ||    // 长沙银行
        wBank == BANK_NO_PSBC)      // 邮储
    {
        IF_CMCP(lpNoteType, "1", NOTE_TYPE_PTCD);       // 普通存单
        else
        IF_CMCP(lpNoteType, "2", NOTE_TYPE_XPCD);       // 芯片存单
        else
        IF_CMCP(lpNoteType, "3", NOTE_TYPE_DECD);       // 大额存单
        else
        IF_CMCP(lpNoteType, "4", NOTE_TYPE_GZPZ);       // 国债凭证
        else
        IF_CMCP(lpNoteType, "5", NOTE_TYPE_JSYWWTS);    // 结算业务委托书
        else
        IF_CMCP(lpNoteType, "6", NOTE_TYPE_XJZP);       // 现金支票
        else
        IF_CMCP(lpNoteType, "7", NOTE_TYPE_ZZZP);       // 转账支票
        else
        IF_CMCP(lpNoteType, "8", NOTE_TYPE_QFJZP);      // 清分机支票
        else
        IF_CMCP(lpNoteType, "9", NOTE_TYPE_YHHP);       // 银行汇票
        else
        IF_CMCP(lpNoteType, "A", NOTE_TYPE_YHCDHP);     // 银行承兑汇票
        else
        IF_CMCP(lpNoteType, "B", NOTE_TYPE_SYCDHP);     // 商业承兑汇票
        else
        IF_CMCP(lpNoteType, "C", NOTE_TYPE_FQFJBP);     // 非清分机本票
        else
        IF_CMCP(lpNoteType, "D", NOTE_TYPE_QFJBP);      // 清分机本票
        else
        return NOTE_TYPE_INV;                           // 未知
    } else
    {
        return 0;
    }
}

// 指定票据类型是否对应票箱存在
WORD CXFS_CPR::NoteTypeIsHave(LPSTR lpNoteType, WORD wBox)
{
    // 票箱指定票据类型: 1普通存单/2芯片存单/3大额存单/4国债凭证/5结算业务委托书/6现金支票/7转账支票/
    //                 8清分机支票/9银行汇票/10银行承兑汇票/11商业承兑汇票/12非清分机本票/13清分机本票

    WORD wNoteTypeTmp = 0;

}

// 票据箱指定票据类型转换为设备接口专用类型
WORD CXFS_CPR::BoxNoteType2DevSDK(USHORT usNote, USHORT usDevMode)
{
    // 票箱指定票据类型: 1普通存单/2芯片存单/3大额存单/4国债凭证/5结算业务委托书/6现金支票/7转账支票/
    //                 8清分机支票/9银行汇票/10银行承兑汇票/11商业承兑汇票/12非清分机本票/13清分机本票

    if (usDevMode == IXFSCPR_TYPE_BT8500M)
    {
        switch(usNote)
        {
            case NOTE_TYPE_PTCD:            // 普通存单
            case NOTE_TYPE_XPCD:            // 芯片存单
            case NOTE_TYPE_DECD:            // 大额存单
                return 1;   // 分纸类型:存单
            case NOTE_TYPE_GZPZ:            // 国债凭证
                return 2;   // 分纸类型:国债
            case NOTE_TYPE_JSYWWTS:         // 结算业务委托书
                return 3;   // 分纸类型:业务委托书
            case NOTE_TYPE_XJZP:            // 现金支票
            case NOTE_TYPE_ZZZP:            // 转账支票
            case NOTE_TYPE_QFJZP:           // 清分机支票
            case NOTE_TYPE_YHHP:            // 银行汇票
            case NOTE_TYPE_YHCDHP:          // 银行承兑汇票
            case NOTE_TYPE_SYCDHP:          // 商业承兑汇票
            case NOTE_TYPE_FQFJBP:          // 非清分机本票
            case NOTE_TYPE_QFJBP:           // 清分机本票
                 return 0;   // 分纸类型:支票
            default: return 1;  // 存单
        }
    }

    return 1;
}

HRESULT CXFS_CPR::GetFormFieldToJSON(LPSTR lpForm, CJsonObject &cJson)
{
    THISMODULE(__FUNCTION__);

    // FORM行列值转换为MM(1MM单位),有小数位四舍五入
    #define ROWCOL2MM(ROL, MM) \
        rc.pForm->GetOrigUNIT(nullptr) == FORM_ROWCOLUMN ? \
            (int)((ROL * MM + 5) / 10) : ROL

    SIZE stSize = GetPerRowColTwips2MM();
    // 设置边界
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    // 加载Form文件
    m_pData->LoadForms();
    // 加载Media文件
    m_pData->LoadMedias();

    CSPPtrData *pData = static_cast<CSPPtrData *>(m_pData);

    ReadContext rc;
    ZeroMemory(&rc, sizeof(rc));

    rc.pForm = pData->FindForm(lpForm);
    if (!rc.pForm)
    {
        Log(ThisModule, __LINE__, "FORM<%s>未找到, Return: %d",
            lpForm, WFS_ERR_PTR_FORMNOTFOUND);
        return WFS_ERR_PTR_FORMNOTFOUND;
    }
    if (!rc.pForm->IsLoadSucc())
    {
        Log(ThisModule, __LINE__, "指定FORM名<%>无效, Return: %d",
            lpForm, WFS_ERR_PTR_FORMINVALID);
        return WFS_ERR_PTR_FORMINVALID;
    }
    if (rc.pForm->GetSubItemCount() < 1) // 无Field
    {
        Log(ThisModule, __LINE__, "Field列表为空, Return: %d", WFS_ERR_PTR_FORMINVALID);
        return WFS_ERR_PTR_FIELDNOTFOUND;
    }

    // 循环取Field
    cJson.Clear();
    cJson.Add(JSON_KEY_USE_AREA, 1);
    cJson.Add(JSON_KEY_MEDIA_WIDTH, ROWCOL2MM(rc.pForm->GetOrigSize().cx, stSize.cx));    // 介质宽(单位:MM)
    cJson.Add(JSON_KEY_MEDIA_HEIGHT, ROWCOL2MM(rc.pForm->GetOrigSize().cy, stSize.cy));   // 介质高(单位:MM)
    ISPPrinterItem *pItem = nullptr;
    CHAR szIdenKey[32];
    DWORD iChild = 0;
    for (iChild = 0; iChild < rc.pForm->GetSubItemCount(); iChild++)
    {
        pItem = rc.pForm->GetSubItem(iChild);
        if (pItem == nullptr)
        {
            Log(ThisModule, __LINE__, "From[%s]->GetSubItem(%d) Fail", lpForm, iChild);
            return WFS_ERR_PTR_FIELDERROR;
        }
        // 项名
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_IDEN_NAME, iChild + 1);
        cJson.Add(szIdenKey, std::string(pItem->GetName()));
        // 起始坐标X(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_START_X, iChild + 1);
        cJson.Add(szIdenKey, ROWCOL2MM(pItem->GetOrigPosition().cx, stSize.cx));
        // 起始坐标Y(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_START_Y, iChild + 1);
        cJson.Add(szIdenKey, ROWCOL2MM(pItem->GetOrigPosition().cy, stSize.cy));
        // 可用宽(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_AREA_WIDTH, iChild + 1);
        cJson.Add(szIdenKey, ROWCOL2MM(pItem->GetOrigSize().cx, stSize.cx));
        // 可用高(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_AREA_HEIGHT, iChild + 1);
        cJson.Add(szIdenKey, ROWCOL2MM(pItem->GetOrigSize().cy, stSize.cy));
    }
    cJson.Add(JSON_KEY_IDEN_CNT, iChild);   // 项数目

    return WFS_SUCCESS;
}

HRESULT CXFS_CPR::GetDefFieldToJSON(LPSTR lpStr, CJsonObject &cJson)
{
    THISMODULE(__FUNCTION__);

    cJson.Clear();

    if (lpStr == nullptr || strlen(lpStr) < 1)
    {
        cJson.Parse(NOTE_GETJSON);
        return WFS_SUCCESS;
    }

    // 解析lpStr
    CHAR szAppList[64][CONST_VALUE_260];
    CHAR szKeyList[12][CONST_VALUE_260];
    CHAR szIdenKey[32];
    INT  nAppCnt = 0, nKeyCnt = 0;
    INT  nIdenCnt = 0;

    memset(szAppList, 0x00, sizeof(szAppList));
    nAppCnt = DataConvertor::split_string(lpStr, '|', szAppList, 64);
    if (nAppCnt == 0)
    {
        cJson.Add(NOTE_GETJSON);
        return WFS_SUCCESS;
    }

    // 取第一个分割串处理
    memset(szKeyList, 0x00, sizeof(szKeyList));
    nKeyCnt = DataConvertor::split_string(szAppList[0], ',', szKeyList, 64);
    if (nKeyCnt < 3)
    {
        cJson.Add(NOTE_GETJSON);
        return WFS_SUCCESS;
    }
    cJson.Add(JSON_KEY_USE_AREA, atoi(szKeyList[0]));
    cJson.Add(JSON_KEY_MEDIA_WIDTH, atoi(szKeyList[1]));    // 介质宽(单位:MM)
    cJson.Add(JSON_KEY_MEDIA_HEIGHT, atoi(szKeyList[2]));   // 介质高(单位:MM)

    for (INT i = 1; i < nAppCnt; i ++)
    {
        memset(szKeyList, 0x00, sizeof(szKeyList));
        if (DataConvertor::split_string(szAppList[i], ',', szKeyList, 64) < 5)
        {
            continue;
        }
        // 项名
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_IDEN_NAME, nIdenCnt + 1);
        cJson.Add(szIdenKey, std::string(szKeyList[0]));
        // 起始坐标X(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_START_X, nIdenCnt + 1);
        cJson.Add(szIdenKey, atoi(szKeyList[1]));
        // 起始坐标Y(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_START_Y, nIdenCnt + 1);
        cJson.Add(szIdenKey, atoi(szKeyList[2]));
        // 可用宽(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_AREA_WIDTH, nIdenCnt + 1);
        cJson.Add(szIdenKey, atoi(szKeyList[3]));
        // 可用高(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_AREA_HEIGHT, nIdenCnt + 1);
        cJson.Add(szIdenKey, atoi(szKeyList[4]));

        nIdenCnt ++;
    }
    cJson.Add(JSON_KEY_IDEN_CNT, nIdenCnt);   // 项数目

    return WFS_SUCCESS;
}



