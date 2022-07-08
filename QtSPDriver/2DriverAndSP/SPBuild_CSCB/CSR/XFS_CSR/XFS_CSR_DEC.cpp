/***************************************************************
* 文件名称：XFS_CSR_DEC.cpp
* 文件描述：票据受理模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_CSR.h"
#include "data_convertor.h"

//-----------------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_CSR::StartOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = 0;

    if (bReConn == FALSE)   // 非重连状态,避免重复调用
    {
        // Open前下传初始参数
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pPrinter->SetData(m_stConfig.szSDKPath, DTYPE_LIB_PATH);
        }
        // RSC-D400M设备下传初始参数
        if (m_stConfig.nDriverType == IXFSCSR_TYPE_RSCD400M)
        {
            m_pPrinter->SetData((LPSTR)m_stConfig.stCfg_RSCD400M.stdFontBuffer.c_str(), DTYPE_FONT);  // 设置支持的打印字体
            m_pPrinter->SetData(&m_stConfig.stCfg_RSCD400M.usDPIx, DTYPE_DPIx);
            m_pPrinter->SetData(&m_stConfig.stCfg_RSCD400M.usDPIy, DTYPE_DPIy);
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
    CHAR szDevVer[128] = { 0x00 };
    m_pPrinter->GetVersion(szDevVer, sizeof(szDevVer) - 1, 1);

    CHAR szFWVer[128] = { 0x00 };
    m_pPrinter->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

    m_cExtra.AddExtra("VRTCount", "1");
    //m_cExtra.AddExtra("VRT[00]_XFSCSR", (char*)byVRTU);
    //m_cExtra.AddExtra("VRT[01]_DevCSR", szDevVer);
    m_cExtra.AddExtra("FirmwareVersion", szFWVer);
    m_cExtra.AddExtra("LastErrorCode", "00-000");
    m_cExtra.AddExtra("LastErrorDetail", "");

    // 更新一次状态
    OnStatus();

    if (bReConn == FALSE)   // 非重连状态
    {
        Log(ThisModule, 1, "打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, 1, "断线重连:打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());
    }

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//-----------------------------------重载函数-------------------------------
//
HRESULT CXFS_CSR::SendRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

HRESULT CXFS_CSR::Reset(DWORD dwMediaControl, USHORT usBinIndex)
{
    Q_UNUSED(usBinIndex);
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    Log(ThisModule, 1, IDS_INFO_RESET_DEVICE);

    int nRet = m_pPrinter->Reset();
    UpdateDeviceStatus();
    if (PTR_SUCCESS != nRet)
    {
        //if (m_stStatus.fwPaper[0] == WFS_ERR_PTR_PAPEROUT)   // 无纸
        if (ERR_PTR_NO_PAPER == nRet)
        {
            m_bNeedKeepJammedStatus = FALSE;
            Log(ThisModule, 1, IDS_INFO_RESET_INFO, nRet, dwMediaControl);
            return WFS_SUCCESS;
        }

        Log(ThisModule, __LINE__, IDS_ERR_REESET_ERROR, nRet);
        return WFS_ERR_HARDWARE_ERROR;

    }
    m_bReset = TRUE;

    m_bNeedKeepJammedStatus = FALSE;
    Log(ThisModule, 1, IDS_INFO_RESET_SUCCESS, nRet, dwMediaControl);
    return WFS_SUCCESS;
}

/* 图片打印
     *  nDstWidth  期望宽度
     *  nDstHeight 期望高度
     */

HRESULT CXFS_CSR::PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    //int nRet = m_pPrinter->PrintImage(szImagePath, nDstWidth, nDstHeight);

    /*int nRet = m_pPrinter->PrintDataOrg(dwMediaWidth, dwMediaHeight, 0, 0, "", 1, 0,
                                        szImagePath, strlen(szImagePath), 2);
    UpdateDeviceStatus();
    if (PTR_SUCCESS != nRet)
    {
        Log(ThisModule, __LINE__, "打印图片错误: %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }*/

    return WFS_SUCCESS;
}

HRESULT CXFS_CSR::PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    /*int nRet = m_pPrinter->PrintImageOrg(szImagePath, ulOrgX, ulOrgY);
    UpdateDeviceStatus();
    if (PTR_SUCCESS != nRet)
    {
        Log(ThisModule, __LINE__, "打印图片错误: %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }*/

    return WFS_SUCCESS;
}

long CXFS_CSR::PrintData(const char *pBuffer, DWORD dwSize)
{
    const char *ThisModule = "PrintData";
    //int iRet = m_pPrinter->PrintData(pBuffer, dwSize);
    /*int iRet = m_pPrinter->PrintDataOrg(dwMediaWidth, dwMediaHeight, 0, 0, "新宋体", 40, 40,
                                        pBuffer, dwSize, 1);
    UpdateDeviceStatus();
    if (iRet)
    {
        Log(ThisModule, __LINE__, "PrintData fail, ErrCode:%d", iRet);
    }
    return ConvertErrCode(iRet);*/
    return WFS_SUCCESS;
}

long CXFS_CSR::PrintMICR(const char *pBuffer, DWORD dwSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    /*int iRet = m_pPrinter->WriteRFID(pBuffer);
    UpdateDeviceStatus();
    if (iRet)
    {
        Log(ThisModule, __LINE__, "Write MICR fail, ErrCode:%d", iRet);
    }
    return ConvertErrCode(iRet);*/
}

HRESULT CXFS_CSR::PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    return WFS_SUCCESS;
}

HRESULT CXFS_CSR::AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    return WFS_SUCCESS;
}

HRESULT CXFS_CSR::EndForm(PrintContext *pContext)
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
            JSON_ADD(cJson_Pic, JSON_KEY_PIC_ZOOM, wPicCnt, szIdenKey, m_stConfig.stCfg_RSCD400M.fPictureZoom);
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
    pContext->pUserData = NULL;
    if (pItems)
    {
        delete pItems;
    }

    Log(ThisModule, __LINE__, "->PrintData() 打印数据JSON: %s.", (LPSTR)cJsonData.ToString().c_str());
    INT nRet = m_pPrinter->PrintData((LPSTR)cJsonData.ToString().c_str(),
                                     cJsonData.ToString().length());
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "->PrintData(%d) Fail, ErrCode = %d, Return: %d.",
            (LPSTR)cJsonData.ToString().c_str(), cJsonData.ToString().length(),
            nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    // 控制MEDIA
    DWORD dwMediaControl = pContext->pPrintData->dwMediaControl;
    if (dwMediaControl != 0)
    {
        return ControlMedia(dwMediaControl);
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
void CXFS_CSR::InitConifig()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    #define NOTE_NO_PAR_MAP(PAR, TYPE) \
        memset(szBuffer, 0x00, sizeof(szBuffer)); \
        strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, PAR, NOTE_GETJSON)); \
        m_stConfig.m_Map_NotePar_GetList.insert(map<USHORT, std::string>::value_type(TYPE, szBuffer)); \

    CHAR    szIniAppName[256];
    CHAR    szKeyName[256];
    CHAR    szBuffer[65536];
    INT     nCount = 0;

    // 底层设备控制动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 设备类型(0/RSC-D400M)
    m_stConfig.nDriverType = m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (INT)0);

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.nOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (INT)0);
    if (m_stConfig.nOpenFailRet != 0 && m_stConfig.nOpenFailRet != 1)
    {
        m_stConfig.nOpenFailRet = 0;
    }

    // ----------------------------------------回收箱相关信息获取----------------------------------------
    // 回收箱数目
    m_stNoteBoxList.usBoxCount = m_cXfsReg.GetValue("NOTEBOX_CONFIG", "NoteBoxCount", (INT)0);

    // 读指定票据箱相关参数
    for (INT i = 0; i < m_stNoteBoxList.usBoxCount; i ++)
    {
        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "NOTEBOX_%d", i);

        // 票据箱有效标记
        m_stNoteBoxList.stNoteBox[i].bIsHave = TRUE;
        // 票据箱索引
        m_stNoteBoxList.stNoteBox[i].usBoxNo = m_cXfsReg.GetValue(szIniAppName, "BoxNo", (INT)0);
        // 票据张数
        m_stNoteBoxList.stNoteBox[i].usNoteCount = m_cXfsReg.GetValue(szIniAppName, "NoteCount", (INT)0);
        // HIGH报警阀值
        m_stNoteBoxList.stNoteBox[i].usHighThreshold = m_cXfsReg.GetValue(szIniAppName, "HighThreshold", (INT)99);
        // FULL报警阀值
        m_stNoteBoxList.stNoteBox[i].usFullThreshold = m_cXfsReg.GetValue(szIniAppName, "FullThreshold", (INT)100);
    }

    // ----------------------------------------设备分类相关信息获取----------------------------------------
    // 根据设备类型获取相关参数
    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.nDriverType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    // 底层设备控制动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 获取RSC-D400M设备INI设置
    if (m_stConfig.nDriverType == IXFSCSR_TYPE_RSCD400M)
    {
        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.nDriverType);

        // 左票箱指定编号
        m_stConfig.stCfg_RSCD400M.usLeftNoteBoxNo = m_cXfsReg.GetValue(szIniAppName, "LeftNoteBoxNo", (INT)1);
        // 右票箱指定编号
        m_stConfig.stCfg_RSCD400M.usRightNoteBoxNo = m_cXfsReg.GetValue(szIniAppName, "RightNoteBoxNo", (INT)2);

        // 鉴伪时是否使用Form或者INI设置值指定的值设置票面识别范围,0不使用/1使用,缺省1
        m_stConfig.stCfg_RSCD400M.usUseDistAreaSupp = m_cXfsReg.GetValue(szIniAppName, "UseDistAreaSupp", (INT)1);
        if (m_stConfig.stCfg_RSCD400M.usUseDistAreaSupp != 0 && m_stConfig.stCfg_RSCD400M.usUseDistAreaSupp != 1)
        {
            m_stConfig.stCfg_RSCD400M.usUseDistAreaSupp = 1;
        }

        // 图片缩放,缺省1.0
        memset(szBuffer, 0x00, sizeof(szBuffer));
        strcpy(szBuffer, m_cXfsReg.GetValue(szIniAppName, "PictureZoom", "1.0"));
        m_stConfig.stCfg_RSCD400M.fPictureZoom = atof(szBuffer);

        // X方向DPI
        m_stConfig.stCfg_RSCD400M.usDPIx = m_cXfsReg.GetValue(szIniAppName, "DPIx", (INT)192);
        // Y方向DPI
        m_stConfig.stCfg_RSCD400M.usDPIy = m_cXfsReg.GetValue(szIniAppName, "DPIy", (INT)208);

        // --------获取支持的字体设置--------
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

            if (m_stConfig.stCfg_RSCD400M.stdFontBuffer.length() > 0)
            {
                m_stConfig.stCfg_RSCD400M.stdFontBuffer.append("|");
            }
            m_stConfig.stCfg_RSCD400M.stdFontBuffer.append(szFontName);
            m_stConfig.stCfg_RSCD400M.stdFontBuffer.append(",");
            m_stConfig.stCfg_RSCD400M.stdFontBuffer.append(szFontPath);
        }
    }

    // ----------------------------------------银行分类相关信息获取----------------------------------------
    // 根据银行获取特殊设置
    m_stConfig.usBank = m_cXfsReg.GetValue("BANK_CONFIG", "BankNo", (INT)0);

    // 设置指定银行INI AppName
    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "BANK_SET_%d", m_stConfig.usBank);

    // 取银行别码
    strcpy(m_stConfig.szBankCode, m_cXfsReg.GetValue(szIniAppName, "BankCode", "000"));

    // 取缺省票号/票面信息获取JSON
    NOTE_NO_PAR_MAP("NotePar1_PTCD", NOTE_TYPE_PTCD);       // 普通存单-票号信息点
    NOTE_NO_PAR_MAP("NotePar2_XPCD", NOTE_TYPE_XPCD);       // 芯片存单-票号信息点
    NOTE_NO_PAR_MAP("NotePar3_DECD", NOTE_TYPE_DECD);       // 大额存单-票号信息点
    NOTE_NO_PAR_MAP("NotePar4_GZPZ", NOTE_TYPE_GZPZ);       // 国债凭证-票号信息点
    NOTE_NO_PAR_MAP("NotePar5_JSYWWTS", NOTE_TYPE_JSYWWTS);// 结算业务委托书-票号信息点
    NOTE_NO_PAR_MAP("NotePar6_XJZP", NOTE_TYPE_XJZP);       // 现金支票-票号信息点
    NOTE_NO_PAR_MAP("NotePar7_ZZZP", NOTE_TYPE_ZZZP);       // 转账支票-票号信息点
    NOTE_NO_PAR_MAP("NotePar8_QFZZP", NOTE_TYPE_QFJZP);    // 清分机支票-票号信息点
    NOTE_NO_PAR_MAP("NotePar9_YHHP", NOTE_TYPE_YHHP);       // 银行汇票-票号信息点
    NOTE_NO_PAR_MAP("NotePar10_YHCDHP", NOTE_TYPE_YHCDHP);// 银行承兑汇票-票号信息点
    NOTE_NO_PAR_MAP("NotePar11_SYCDHP", NOTE_TYPE_SYCDHP);// 商业承兑汇票-票号信息点
    NOTE_NO_PAR_MAP("NotePar12_FQFJBP", NOTE_TYPE_FQFJBP);// 非清分机本票-票号信息点
    NOTE_NO_PAR_MAP("NotePar13_QFJBP", NOTE_TYPE_QFJBP);   // 清分机本票-票号信息点

    // ----------------------------------------其他相关信息获取----------------------------------------
    // 读左右上下边距设置,单位:毫米,缺省0
    memset(&m_stMargin, 0, sizeof(RECT));
    m_stMargin.left = m_cXfsReg.GetValue("RECT_CFG", "Left", (INT)0);
    m_stMargin.right = m_cXfsReg.GetValue("RECT_CFG", "Right", (INT)0);
    m_stMargin.top = m_cXfsReg.GetValue("RECT_CFG", "Top", (INT)0);
    m_stMargin.bottom = m_cXfsReg.GetValue("RECT_CFG", "Bottom", (INT)0);


    m_stConfig.nVerifyField = m_cXfsReg.GetValue("CONFIG", "verify_field", (DWORD)0);
    if (m_stConfig.nVerifyField < 0 || m_stConfig.nVerifyField > 2)
        m_stConfig.nVerifyField = 0;

}

// 初始化状态类变量
long CXFS_CSR::InitStatus()
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
}

// 初始化能力值类变量
long CXFS_CSR::InitCaps()
{
    memset(&m_sCaps, 0x00, sizeof(WFSPTRCAPS));
    m_sCaps.fwType = WFS_PTR_TYPESCANNER;

    INT nBoxCount = m_stNoteBoxList.GetBoxCount();
    m_sCaps.lpusMaxRetract = nullptr;               // 每个回收箱可容纳媒介数目(INI获取)(结构体数组指针)
    m_sCaps.usRetractBins = nBoxCount;              // 回收箱个数(INI获取)
    if (nBoxCount > 0)
    {
        m_sCaps.lpusMaxRetract = new USHORT[nBoxCount + 1];
        memset(m_sCaps.lpusMaxRetract, 0x00, sizeof(USHORT) * (nBoxCount + 1));
        INT nBoxSum = 0;
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (m_stNoteBoxList.GetBoxNo(i) != BOX_NOHAVE)
            {
                m_sCaps.lpusMaxRetract[nBoxSum] = m_stNoteBoxList.stNoteBox[i].usFullThreshold;
                nBoxSum ++;
            }
        }
    }
}

// 状态获取
void CXFS_CSR::UpdateDeviceStatus()
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
    BOOL    bNeedFireMediaTaken     = FALSE;    // 需要上报介质拿走事件
    BOOL    bNeedFireMediaInsert    = FALSE;    // 需要上报介质放入事件
    memset(bNeedFirePaperStatus, FALSE, sizeof(bNeedFirePaperStatus));
    memset(bNeedFireRetractStatus, FALSE, sizeof(bNeedFireRetractStatus));


    WFSPTRSTATUS stLastStatus = m_stStatus;

    nRet = m_pPrinter->GetStatus(stDevStatus);
    if (nRet != PTR_SUCCESS && nRet != ERR_PTR_CANCEL)
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
        }
        m_stStatus.fwToner = ConvertTonerStatus(stDevStatus.wToner);
        m_stStatus.fwInk = ConvertInkStatus(stDevStatus.wInk);

        // 回收箱状态设定
        memset(m_stWFSRetractBin, 0x00, sizeof(WFSPTRRETRACTBINS) * 16);
        // 取INI记录数目及阀值状态
        for (INT i = 0; i < 16; i++)
        {
            if (m_stNoteBoxList.GetBoxNo(i) != BOX_NOHAVE)
            {
                INT nStat = m_stNoteBoxList.GetBoxStat(m_stNoteBoxList.GetBoxNo(i));
                m_stWFSRetractBin[i].wRetractBin = (nStat == BOX_FULL ? WFS_PTR_RETRACTBINFULL :
                                                   (nStat == BOX_HIGHLOW ? WFS_PTR_RETRACTBINHIGH :
                                                    WFS_PTR_RETRACTBINOK));
                m_stWFSRetractBin[i].usRetractCount = m_stNoteBoxList.GetNoteCount(m_stNoteBoxList.GetBoxNo(i));
                m_stNoteBoxList.SetRetraceFull(m_stNoteBoxList.GetBoxNo(i), FALSE);

                // RSC-D400M设备设置,返回状态缺省:0为左回收箱,1为右回收箱
                if (m_stConfig.nDriverType == IXFSCSR_TYPE_RSCD400M)
                {
                    // 当回收箱计数为OK或High,物理状态为Full时,以物理状态为准
                    if (m_stNoteBoxList.GetBoxNo(i) == m_stConfig.stCfg_RSCD400M.usLeftNoteBoxNo)  // 左回收箱
                    {
                        if (nStat != BOX_FULL && stDevStatus.stRetract[0].wBin == RETRACT_STAT_FULL)
                        {
                            m_stWFSRetractBin[i].wRetractBin = ConvertRetractStatus(stDevStatus.stRetract[0].wBin);
                            m_stNoteBoxList.SetRetraceFull(m_stNoteBoxList.GetBoxNo(i), TRUE);
                        }
                    } else
                    if (m_stNoteBoxList.GetBoxNo(i) == m_stConfig.stCfg_RSCD400M.usRightNoteBoxNo) // 右回收箱
                    {
                        if (nStat != BOX_FULL && stDevStatus.stRetract[1].wBin == RETRACT_STAT_FULL)
                        {
                            m_stWFSRetractBin[i].wRetractBin = ConvertRetractStatus(stDevStatus.stRetract[1].wBin);
                            m_stNoteBoxList.SetRetraceFull(m_stNoteBoxList.GetBoxNo(i), TRUE);
                        }
                    }
                }
            }
        }

        //--------事件检查处理--------
        // Media原状态为出口+通道内无票＆n_WaitTaken为准执行Taken,设置Taken事件上报标记
        if ((m_stStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT) &&
            (stLastStatus.fwMedia == WFS_PTR_MEDIAENTERING) && m_WaitTaken == WTF_TAKEN)
        {
            bNeedFireMediaTaken = TRUE;
            m_WaitTaken = WTF_NONE;
        }

        // Media原状态为出口无票||当前状态为通道内无票->当前状态为出口||通道内有票,设置MediaInsert事件上报标记
        if (stLastStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT &&
            (m_stStatus.fwMedia == WFS_PTR_MEDIAENTERING || m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT))
        {
            bNeedFireMediaInsert = TRUE;
        }

        // 票箱变化事件
        /*for (INT i = 0; i < 16; i ++)
        {
            if (m_stStatus.fwPaper[i] != stLastStatus.fwPaper[i])
            {
                if (m_stStatus.fwPaper[i] == WFS_PTR_PAPERLOW ||
                    m_stStatus.fwPaper[i] == WFS_PTR_PAPEROUT ||
                    m_stStatus.fwPaper[i] == WFS_PTR_PAPERFULL)
                {
                    bNeedFirePaperStatus[i] = TRUE;
                }
            }
        }*/

        // 碳带变化事件
        if (m_stStatus.fwToner != stLastStatus.fwToner)
        {
            if (m_stStatus.fwToner == WFS_PTR_TONERLOW ||
                m_stStatus.fwToner == WFS_PTR_TONEROUT ||
                m_stStatus.fwToner == WFS_PTR_TONERFULL)
            {
                bNeedFireTonerStatus = TRUE;
            }
        }

        // 墨盒变化事件
        if (m_stStatus.fwInk != stLastStatus.fwInk)
        {
            if (m_stStatus.fwToner == WFS_PTR_INKLOW ||
                m_stStatus.fwToner == WFS_PTR_INKOUT ||
                m_stStatus.fwToner == WFS_PTR_INKFULL)
            {
                bNeedFireInkStatus = TRUE;
            }
        }

        // 回收变化事件
        for (INT i = 0; i < 16; i ++)
        {
            if (m_stWFSRetractBin[i].wRetractBin != m_stWFSRetractBinOLD[i].wRetractBin)
            {
                if (m_stWFSRetractBin[i].wRetractBin == WFS_PTR_RETRACTBINOK ||
                    m_stWFSRetractBin[i].wRetractBin == WFS_PTR_RETRACTBINFULL ||
                    m_stWFSRetractBin[i].wRetractBin == WFS_PTR_RETRACTBINHIGH)
                {
                    bNeedFireRetractStatus[i] = TRUE;
                }
            }
        }
        memcpy(m_stWFSRetractBinOLD, m_stWFSRetractBin, sizeof(WFSPTRRETRACTBINS) * 16);
    }

    // Device状态有变化&当前Device状态为HWERR,设置HWERR事件上报标记
    if (m_stStatus.fwDevice != stLastStatus.fwDevice)
    {
        bNeedFirePrinterStatus = TRUE;
        if (m_stStatus.fwDevice == WFS_PTR_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }


    //--------事件上报处理--------

    if (bNeedFireHWError == TRUE)           // 上报Device HWERR事件
    {
        FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
    }

    if (bNeedFirePrinterStatus == TRUE)     // 上报状态变化事件
    {
        FireStatusChanged(m_stStatus.fwDevice);
    }

    /*for (INT i = 0; i < 16; i ++)           // 上报票箱状态变化
    {
        if (bNeedFirePaperStatus[i] == TRUE && ConvertPaperCode(i + 1) != 0)
        {
            FirePaperThreshold(ConvertPaperCode(i + 1), m_stStatus.fwPaper[i]);
        }
    }*/

    if (bNeedFireTonerStatus == TRUE)       // 上报碳带状态变化
    {
        FireTonerThreshold(m_stStatus.fwToner);
    }

    if (bNeedFireInkStatus == TRUE)         // 上报墨盒状态变化
    {
        FireInkThreshold(m_stStatus.fwInk);
    }

    if (bNeedFireMediaTaken == TRUE)        // 上报Taken事件
    {
        FireMediaTaken();
        Log(ThisModule, 1, IDS_INFO_PAPER_TAKEN);
    }

    if (bNeedFireMediaInsert == TRUE && m_bInsertEventRep == TRUE)       // 上报Insert事件
    {
        FireMediaInserted();
    }

    for (INT i = 0; i < 16; i ++)           // 上报回收箱状态变化
    {
        if (bNeedFireRetractStatus[i] == TRUE)
        {
            if (m_stConfig.nDriverType == IXFSCSR_TYPE_RSCD400M)
            {
                FireRetractBinThreshold(m_stNoteBoxList.GetBoxNo(i), m_stWFSRetractBin[i].wRetractBin);
            } else
            {
                FireRetractBinThreshold(i, m_stWFSRetractBin[i].wRetractBin);
            }
        }
    }

    return;
}

// 介质控制处理
HRESULT CXFS_CSR::ControlMedia(DWORD dwControl)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;

    if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)     // 支持参数: 退票
    {
         MEDIA_ACTION enInput;
         if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)      // 退票
         {
             enInput = MEDIA_CTR_EJECT;
         }

         nRet = m_pPrinter->MeidaControl(enInput);
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

         m_WaitTaken = WTF_TAKEN;
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
HRESULT CXFS_CSR::InnerPrintForm(LPWFSPTRPRINTFORM pInData)
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
        Log(ThisModule, __LINE__, IDS_ERR_FORM_NOT_FOUND, pInData->lpszFormName);
        return WFS_ERR_PTR_FORMNOTFOUND;
    }
    if (!pc.pForm->IsLoadSucc())
    {
        Log(ThisModule, __LINE__, IDS_ERR_FORM_INVALID, pInData->lpszFormName);
        return WFS_ERR_PTR_FORMINVALID;
    }
    pc.pMedia = pData->FindMedia(pInData->lpszMediaName);
    if (!pc.pMedia)
    {
        Log(ThisModule, __LINE__, IDS_ERR_MEDIA_NOT_FOUND, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIANOTFOUND;
    }
    if (!pc.pMedia->IsLoadSucc())
    {
        Log(ThisModule, __LINE__, IDS_ERR_MEDIA_INVALID, pInData->lpszMediaName);
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
            Log(ThisModule, __LINE__, IDS_ERR_MEDIA_INVALID, pc.pMedia->GetName());
            return WFS_ERR_PTR_MEDIAINVALID;
        }
    } while (0);

    RECT rcMD;
    ((CSPPrinterForm *)pc.pForm)->GetMulDiv(rcMD);

    if (WFS_PTR_OFFSETUSEFORMDEFN == pInData->wOffsetX)
    {
        pInData->wOffsetX = pc.pForm->GetPosition().cx;
    }
    if (WFS_PTR_OFFSETUSEFORMDEFN == pInData->wOffsetY)
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
        Log(ThisModule, __LINE__, IDS_ERR_MEDIA_OVERFLOW, pInData->lpszFormName, pInData->lpszMediaName);
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
        Log(ThisModule, __LINE__, IDS_ERR_MEDIA_OVERFLOW_PRINTAREA, pInData->lpszFormName, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIAOVERFLOW;
    }
    FORMALIGN FormAlign = pc.pForm->GetAlign();
    if (WFS_PTR_ALNUSEFORMDEFN != pInData->wAlignment &&
        WFS_PTR_ALNTOPLEFT != pInData->wAlignment &&
        WFS_PTR_ALNTOPRIGHT != pInData->wAlignment &&
        WFS_PTR_ALNBOTTOMLEFT != pInData->wAlignment &&
        WFS_PTR_ALNBOTTOMRIGHT != pInData->wAlignment)
    {
        Log(ThisModule, __LINE__, IDS_ERR_FORM_ATTRI_INV, pInData->lpszFormName, "Alignment");
        return WFS_ERR_INVALID_DATA;
    }
    if (WFS_PTR_ALNUSEFORMDEFN != pInData->wAlignment)
    {
        FormAlign = (FORMALIGN)(pInData->wAlignment - WFS_PTR_ALNTOPLEFT + TOPLEFT);
    }
    switch (FormAlign)
    {
    case TOPLEFT:
        break;  //(default)
    case TOPRIGHT:
        if (0 < sizeMedia.cx)
        {
            offset.cx = sizeMedia.cx - sizeForm.cx - offset.cx;
        }
        break;
    case BOTTOMLEFT:
        if (0 < sizeMedia.cy)
        {
            offset.cy = sizeMedia.cy - sizeForm.cy - offset.cy;
        }
        break;
    case BOTTOMRIGHT:
        if (0 <  sizeMedia.cx)
        {
            offset.cx = sizeMedia.cx - sizeForm.cx - offset.cx;
        }
        if (0 < sizeMedia.cy)
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
                if (NULL == lpField)
                {
                    continue;
                }
                char szFieldName[1024] = {0};
                for (int j = 0; j < (int)strlen(lpField) && j < 1023; j++)
                {
                    if ('=' != lpField[j]  && '\0' != lpField[j])
                    {
                        szFieldName[j] = lpField[j];
                    }
                }
                if (0 == strcmp(szFieldName, ""))
                {
                    continue;
                }
                DWORD iChild = 0;
                for (; iChild < pc.pForm->GetSubItemCount(); iChild++)
                {
                    if (0 == strcmp(pc.pForm->GetSubItem(iChild)->GetName(), szFieldName))
                    {
                        break;
                    }
                }
                if (iChild == pc.pForm->GetSubItemCount())
                {
                    Log(ThisModule, __LINE__, IDS_ERR_FILED_NOT_FOUND, szFieldName);
                    if (m_stConfig.nVerifyField == 1)
                    {
                        return WFS_ERR_PTR_FIELDNOTFOUND;
                    } else
                    {
                        FireFieldWarning(pInData->lpszFormName, szFieldName, WFS_PTR_FIELDNOTFOUND);
                    }
                }
            }

        } while (0);
    }

    HRESULT hRes = StartForm(&pc);
    if (WFS_SUCCESS != hRes)
    {
        Log(ThisModule, Result2ErrorCode(hRes), IDS_ERR_START_FORM, pInData->lpszFormName);
        return hRes;
    }

    for (DWORD iChild = 0; iChild < pc.pForm->GetSubItemCount() && WFS_SUCCESS == hRes; iChild++)
    {
        ISPPrinterItem *pItem = pc.pForm->GetSubItem(iChild);
        SIZE3 SubOffset;
        SubOffset.cx = SubOffset.cy = SubOffset.cz = 0;
        pc.pSubform = NULL;
        if (ITEM_SUBFORM == pItem->GetItemType())
        {
            pc.pSubform = (ISPPrinterSubform *)pItem;
            SubOffset = pc.pSubform->GetPosition();
        }
        for (DWORD iField = 0; (!pc.pSubform || iField < pc.pSubform->GetSubItemCount()) && WFS_SUCCESS == hRes; iField++)
        {
            if (pc.pSubform)
            {
                pItem = pc.pSubform->GetSubItem(iField);
            }
            SIZE OffsetAll = offset;
            OffsetAll.cx += SubOffset.cx;
            OffsetAll.cy += SubOffset.cy;
            hRes = PrintFieldOrFrame(pc, pItem, OffsetAll, Fields);
            if (!pc.pSubform)
            {
                break;
            }
        }
    }

    if (WFS_SUCCESS != hRes)
    {
        pc.bCancel = TRUE;
    }

    HRESULT hResOld = hRes;
    hRes = EndForm(&pc);

    return  WFS_SUCCESS != hResOld ? hResOld : hRes;
}

HRESULT CXFS_CSR::InnerReadImage(LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LPWFSPTRIMAGEREQUEST lpIn = nullptr;
    LPWFSPTRIMAGE lpOut = nullptr;
    DEVPTRREADIMAGEIN stScanImageIn;
    DEVPTRREADIMAGEOUT stScanImageOut;
    CJsonObject cJsonData;
    WORD    wNoteType = 0;
    HRESULT hRes = WFS_SUCCESS;

    INT nRet = 0;

    lpIn = (LPWFSPTRIMAGEREQUEST)lpImgRequest;


    // --------检查入参并组织下发到DevCPR的入参--------
    stScanImageIn.Clear();
    cJsonData.Clear();
    if ((lpIn->fwImageSource & WFS_PTR_CODELINE) == WFS_PTR_CODELINE)             // 获取票面信息并鉴伪
    {
        // 根据入参验证银行别码并获取票据类型
        // lpszBackImageFile 未指定form时,需要lpszFrontImageFile的票据类型设置获取范围
        if (lpIn->lpszBackImageFile == nullptr || strlen(lpIn->lpszBackImageFile) < 1)
        {
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
        }

        // 鉴伪时是否使用Form或者INI设置值指定的值设置票面识别范围,0不使用/1使用,缺省1
        if (m_stConfig.stCfg_RSCD400M.usUseDistAreaSupp == 1)
        {
            // 根据入参指定票据识别Form转换为JSON或使用INI缺省JSON
            if (lpIn->lpszBackImageFile == nullptr || strlen(lpIn->lpszBackImageFile) < 1)  // 要识别的票号Form名
            {
                cJsonData.Clear();
                GetDefFieldToJSON((LPSTR)(m_stConfig.m_Map_NotePar_GetList.at(wNoteType).c_str()), cJsonData);
            } else
            {
                // 取Form中Field信息写入JSON
                if ((hRes = GetFormFieldToJSON(lpIn->lpszBackImageFile, cJsonData)) != WFS_SUCCESS)
                {
                    return hRes;
                }
            }
        } else
        {
            cJsonData.Parse(NOTE_GETJSON);
        }

        stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_CODELINE);
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

        if ((stScanImageOut.wInMode & IMAGE_MODE_CODELINE) == IMAGE_MODE_CODELINE &&    // 取票号鉴伪
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
                stdData.append(stdTmp);
                stdData.append("=");

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
        if ((stScanImageOut.wInMode & IMAGE_MODE_RFID) == IMAGE_MODE_RFID && stScanImageOut.lpData != nullptr)
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


HRESULT CXFS_CSR::InnerReadForm(LPWFSPTRREADFORM pInData)
{
    THISMODULE(__FUNCTION__);
    return WFS_SUCCESS;
}

HRESULT CXFS_CSR::MediaRetract(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;
    WORD wRetractBox = 0;

    // 获取RSC-D400M设备INI设置(0右票箱/1左票箱)
    if (m_stConfig.nDriverType == IXFSCSR_TYPE_RSCD400M)
    {
        if (*lpusBinNum == m_stConfig.stCfg_RSCD400M.usLeftNoteBoxNo)      // 左票箱
        {
            wRetractBox = 1;
        } else
        if (*lpusBinNum == m_stConfig.stCfg_RSCD400M.usRightNoteBoxNo)     // 右票箱
        {
            wRetractBox = 0;
        } else
        {
            Log(ThisModule, __LINE__, "->入参(lpusBinNum = %d) 无效, Return: %d.",
                MEDIA_CTR_RETRACT, wRetractBox, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    } else
    {
        Log(ThisModule, __LINE__, "->INI指定设备类型[%d]不支持, Return: %d.",
            m_stConfig.nDriverType, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    nRet = m_pPrinter->MeidaControl(MEDIA_CTR_RETRACT, wRetractBox);
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "->MeidaControl(%d, %d) Fail, ErrCode = %d, Return: %d.",
            MEDIA_CTR_RETRACT, wRetractBox, nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    // 统计: 票据计数: +1,写入INI记录
    SetRetractBoxCount(*lpusBinNum, 1, TRUE);

    // 组织返回值
    lpusBinNumOut = new USHORT();
    *lpusBinNumOut = wRetractBox;

    return WFS_SUCCESS;
}

HRESULT CXFS_CSR::MediaInsertWait(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    INT nRet = 0;

    m_bCancelFlag = FALSE;

    // 下发允许入票命令
    nRet = m_pPrinter->MeidaControl(MEDIA_CTR_TURNMEDIA);
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "下发允许入票命令: ->MeidaControl(%d) Fail, ErrCode = %d, Return: %d.",
            MEDIA_CTR_RETRACT, nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    // 循环检查是否入票超时(间隔500毫米)
    DWORD dwTimeCount = 0;
    while(true)
    {
        // 是否被取消
        if (m_bCancelFlag == TRUE)
        {
            // 下发停止入票命令
            m_pPrinter->MeidaControl(MEDIA_CTR_STAMP);
            Log(ThisModule, __LINE__, "命令取消:下发停止入票命令: ->MeidaControl(%d) Fail, Return: %d.",
                 MEDIA_CTR_STAMP, WFS_ERR_CANCELED);
            return WFS_ERR_CANCELED;
        }

        UpdateDeviceStatus();   // 取当前最新状态
        if (m_stStatus.fwDevice != WFS_PTR_DEVONLINE && m_stStatus.fwDevice != WFS_PTR_DEVBUSY) \
        {
            Log(ThisModule, __LINE__, "Device != ONLINE|BUSY, Return: %d.", WFS_ERR_HARDWARE_ERROR); \
            return WFS_ERR_HARDWARE_ERROR;
        } else
        if (m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT || m_stStatus.fwMedia == WFS_PTR_MEDIAENTERING) \
        {
            break;
        } else
        {
            if (dwTimeOut > 0)
            {
                if (dwTimeCount >= dwTimeOut)
                {
                    m_pPrinter->MeidaControl(MEDIA_CTR_STAMP);
                    Log(ThisModule, __LINE__, "已超时:下发停止入票命令: ->MeidaControl(%d) Fail, Return: %d.",
                         MEDIA_CTR_STAMP, WFS_ERR_TIMEOUT);
                    return WFS_ERR_TIMEOUT;
                } else
                {
                    usleep(1000 * 500); // 休眠500毫秒
                    dwTimeCount = dwTimeCount + 500;
                    continue;
                }
            }
            usleep(1000 * 500); // 休眠500毫秒
        }
    }

    //m_pPrinter->MeidaControl(MEDIA_CTR_STAMP);

    return WFS_SUCCESS;
}

void CXFS_CSR::RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData)
{
    ulOutSize = 0;
    /*
     * 去除字符串中非ASCII码字符(0x00~0x7F)，去除非汉字字符(低字节A1~FE,高字节B0~F7)
     * 增加支持中文全角字符打印（低字节A0~FF,高字节A1~A3）
     */
    for (ULONG i = 0; i < ulInSize; i++)
    {
        if ((pInData[i] >= 0xB0) && (pInData[i] <= 0xF7)
            && ((pInData[i + 1] >= 0xA1) && (pInData[i + 1] <= 0xFE))
           )
        {
            pOutData[ulOutSize++] = pInData[i];
            pOutData[ulOutSize++] = pInData[i + 1];
            i++;
        }
        else if ((pInData[i] >= 0xA1) && (pInData[i] <= 0xA3)
                 && ((pInData[i + 1] >= 0xA1) && (pInData[i + 1] <= 0xFF))
                )
        {
            if (0xA1 == pInData[i])
            {
                if (0xA0 == pInData[i + 1] || 0xA1 == pInData[i + 1] || 0xFF == pInData[i + 1])
                {
                    i++;
                    continue;
                }
            }
            else if (0xA2 == pInData[i])
            {
                if (0xA0 == pInData[i + 1] || 0xAB == pInData[i + 1] || 0xAC == pInData[i + 1] ||
                    0xAD == pInData[i + 1] || 0xAE == pInData[i + 1] || 0xAF == pInData[i + 1] ||
                    0xB0 == pInData[i + 1] || 0xEF == pInData[i + 1] || 0xF0 == pInData[i + 1] ||
                    0xFD == pInData[i + 1] || 0xFE == pInData[i + 1] || 0xFF == pInData[i + 1])
                {
                    i++;
                    continue;
                }
            }
            else if (0xA3 == pInData[i])
            {
                if (0xA0 == pInData[i + 1] || 0xFF == pInData[i + 1])
                {
                    i++;
                    continue;
                }
            }
            pOutData[ulOutSize++] = pInData[i];
            pOutData[ulOutSize++] = pInData[i + 1];
            i++;
        }
        else if (pInData[i] >= 0x80)
        {
            i++;
        }
        else if ((pInData[i] == 0x0A)
                 || ((pInData[i] > 0x1F) && (pInData[i] < 0x7F)))
        {
            pOutData[ulOutSize++] = pInData[i];
        }
    }
}

// 设置回收箱计数并记录
void CXFS_CSR::SetRetractBoxCount(USHORT usBoxNo, USHORT usCnt, BOOL bIsAdd)
{
    THISMODULE(__FUNCTION__);

    INT nRet = 0;
    INT nCount = 0;
    CHAR szKeyName[MAX_PATH] = { 0x00 };
    nCount = m_stNoteBoxList.SetNoteCount(usBoxNo, usCnt, bIsAdd);

    // 取顺序索引
    nRet = m_stNoteBoxList.GetBoxOrder(usBoxNo);
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
// 错误码转换为WFS格式
INT CXFS_CSR::ConvertErrCode(INT nRet)
{
    switch (nRet)
    {
        case PTR_SUCCESS:           return WFS_SUCCESS;
        case ERR_PTR_PARAM_ERR:     return WFS_ERR_UNSUPP_DATA;
        case ERR_PTR_COMM_ERR:      return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_NO_PAPER:      return WFS_ERR_PTR_PAPEROUT;
        case ERR_PTR_JAMMED:        return WFS_ERR_PTR_PAPERJAMMED;
        case ERR_PTR_NOT_OPEN:      return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_HEADER:        return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_CUTTER:        return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_TONER:         return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_STACKER_FULL:  return WFS_ERR_PTR_STACKERFULL;
        case ERR_PTR_NO_RESUME:     return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_CAN_RESUME:    return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_FORMAT_ERROR:  return WFS_ERR_UNSUPP_DATA;
        case ERR_PTR_CHRONIC:       return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_HWERR:         return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_IMAGE_ERROR:   return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_UNSUP_CMD:     return WFS_ERR_UNSUPP_COMMAND;
        case ERR_PTR_NO_DEVICE:     return WFS_ERR_HARDWARE_ERROR;
        default:                    return WFS_ERR_HARDWARE_ERROR;
    }
}

// 设备状态转换为WFS格式
WORD CXFS_CSR::ConvertDeviceStatus(WORD wDevStat)
{
    switch (wDevStat)
    {
        case DEV_STAT_ONLINE     /* 设备正常 */     : return WFS_PTR_DEVONLINE;
        case DEV_STAT_OFFLINE    /* 设备脱机 */     : return WFS_PTR_DEVOFFLINE;
        case DEV_STAT_POWEROFF   /* 设备断电 */     : return WFS_PTR_DEVPOWEROFF;
        case DEV_STAT_NODEVICE   /* 设备不存在 */    : return WFS_PTR_DEVNODEVICE;
        case DEV_STAT_HWERROR    /* 设备故障 */     : return WFS_PTR_DEVHWERROR;
        case DEV_STAT_USERERROR  /*  */             : return WFS_PTR_DEVUSERERROR;
        case DEV_STAT_BUSY       /* 设备读写中 */    : return WFS_PTR_DEVBUSY;
        defaule: return WFS_PTR_DEVOFFLINE;
    }
}

// Media状态转换为WFS格式
WORD CXFS_CSR::ConvertMediaStatus(WORD wMediaStat)
{
    switch (wMediaStat)
    {
        case MEDIA_STAT_PRESENT   /* 通道内有票 */               : return WFS_PTR_MEDIAPRESENT;
        case MEDIA_STAT_NOTPRESENT/* 通道内无票 */               : return WFS_PTR_MEDIANOTPRESENT;
        case MEDIA_STAT_JAMMED    /* 通道内有票且票被夹住 */       : return WFS_PTR_MEDIAJAMMED;
        case MEDIA_STAT_NOTSUPP   /* 不支持检测通道内是否有票 */    : return WFS_PTR_MEDIANOTPRESENT;
        case MEDIA_STAT_UNKNOWN   /* 通道内票状态未知 */           : return WFS_PTR_MEDIAUNKNOWN;
        case MEDIA_STAT_ENTERING  /* 票在出票口 */                : return WFS_PTR_MEDIAENTERING;
        default: return WFS_PTR_MEDIAUNKNOWN;
    }
}

// Paper状态转换为WFS格式
WORD CXFS_CSR::ConvertPaperStatus(WORD wPaperStat)
{
    switch (wPaperStat)
    {
        case PAPER_STAT_FULL      /* 票据满 */          : return WFS_PTR_PAPERFULL;
        case PAPER_STAT_LOW       /* 票据少 */          : return WFS_PTR_PAPERLOW;
        case PAPER_STAT_OUT       /* 票据无 */          : return WFS_PTR_PAPEROUT;
        case PAPER_STAT_NOTSUPP   /* 设备不支持该能力 */  : return WFS_PTR_PAPERNOTSUPP;
        case PAPER_STAT_UNKNOWN   /* 不能确定当前状态 */  : return WFS_PTR_PAPERUNKNOWN;
        case PAPER_STAT_JAMMED    /* 票据被卡住 */       : return WFS_PTR_PAPERJAMMED;
        defaule: return WFS_PTR_PAPERUNKNOWN;
    }
}

// Toner状态转换为WFS格式
WORD CXFS_CSR::ConvertTonerStatus(WORD wTonerStat)
{
    switch (wTonerStat)
    {
        case TONER_STAT_FULL      /* 碳带状态满或正常 */    : return WFS_PTR_TONERFULL;
        case TONER_STAT_LOW       /* 碳带少 */            : return WFS_PTR_TONERLOW;
        case TONER_STAT_OUT       /* 碳带无 */            : return WFS_PTR_TONEROUT;
        case TONER_STAT_NOTSUPP   /* 设备不支持该能力 */    : return WFS_PTR_TONERNOTSUPP;
        case TONER_STAT_UNKNOWN   /* 不能确定当前状态 */    : return WFS_PTR_TONERUNKNOWN;
        default: return WFS_PTR_TONERUNKNOWN;
    }
}

// Ink状态转换为WFS格式
WORD CXFS_CSR::ConvertInkStatus(WORD wInkStat)
{
    switch (wInkStat)
    {
        case INK_STAT_FULL    /* 墨盒状态满或正常 */  : return WFS_PTR_INKFULL;
        case INK_STAT_LOW     /* 墨盒少 */           : return WFS_PTR_INKLOW;
        case INK_STAT_OUT     /* 墨盒无 */           : return WFS_PTR_INKOUT;
        case INK_STAT_NOTSUPP /* 设备不支持该能力 */  : return WFS_PTR_INKNOTSUPP;
        case INK_STAT_UNKNOWN /* 不能确定当前状态 */  : return WFS_PTR_INKUNKNOWN;
        default: return WFS_PTR_INKUNKNOWN;
    }
}

// Retract状态转换为WFS格式
WORD CXFS_CSR::ConvertRetractStatus(WORD wRetractStat)
{
    switch (wRetractStat)
    {
        case RETRACT_STAT_OK/* 回收箱正常 */   : return WFS_PTR_RETRACTBINOK;
        case RETRACT_STAT_FULL/* 回收箱满 */   : return WFS_PTR_RETRACTBINFULL;
        case RETRACT_STAT_HIGH/* 回收箱将满 */   : return WFS_PTR_RETRACTBINHIGH;
        default: return WFS_PTR_RETRACTBINOK;
    }
}

// 票箱号转换为WFS格式
WORD CXFS_CSR::ConvertPaperCode(INT nCode)
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
WORD CXFS_CSR::NoteTypeConvert(LPSTR lpNoteType, WORD wBank)
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
WORD CXFS_CSR::NoteTypeIsHave(LPSTR lpNoteType, WORD wBox)
{
    // 票箱指定票据类型: 1普通存单/2芯片存单/3大额存单/4国债凭证/5结算业务委托书/6现金支票/7转账支票/
    //                 8清分机支票/9银行汇票/10银行承兑汇票/11商业承兑汇票/12非清分机本票/13清分机本票

    WORD wNoteTypeTmp = 0;

}

HRESULT CXFS_CSR::GetFormFieldToJSON(LPSTR lpForm, CJsonObject &cJson)
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
        Log(ThisModule, __LINE__, IDS_ERR_FORM_NOT_FOUND, lpForm);
        return WFS_ERR_PTR_FORMNOTFOUND;
    }
    if (!rc.pForm->IsLoadSucc())
    {
        Log(ThisModule, __LINE__, IDS_ERR_FORM_INVALID, lpForm);
        return WFS_ERR_PTR_FORMINVALID;
    }
    if (rc.pForm->GetSubItemCount() < 1) // 无Field
    {
        Log(ThisModule, __LINE__, IDS_ERR_FIELD_EMPTY, lpForm);
        return WFS_ERR_PTR_FORMINVALID;
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
            Log(ThisModule, __LINE__, "From[%s]->GetSubItem(%d) Fail", lpForm);
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

HRESULT CXFS_CSR::GetDefFieldToJSON(LPSTR lpStr, CJsonObject &cJson)
{
    THISMODULE(__FUNCTION__);

    cJson.Clear();

    if (lpStr == nullptr || strlen(lpStr) < 1)
    {
        cJson.Add(NOTE_GETJSON);
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

