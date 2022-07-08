/***************************************************************
* 文件名称：XFS_JPR_DEC.cpp
* 文件描述：流水打印模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月14日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_JPR.h"

//-----------------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_JPR::StartOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    CHAR szFWVersion[64] = { 0x00 };
    LONG lFWVerSize = 0;

    if (m_stConfig.nDeviceType != DEV_BTT080AII)
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定了不支持的设备类型[%d], Return :%d.",
            m_stConfig.nDeviceType, WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 加载DevJPR动态库
    if (!LoadDevDll(ThisModule))
    {
        return WFS_ERR_HARDWARE_ERROR;
    }

    // Open前下传初始参数(非断线重连)
    if (bReConn == FALSE)
    {
        // 设置SDK路径
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pPrinter->SetData(m_stConfig.szSDKPath, DTYPE_LIB_PATH);
        }

        // 设置设备打开模式
        m_pPrinter->SetData(&(m_stConfig.stDevOpenMode), SET_DEV_OPENMODE);
    }

    // 打开设备
    INT nRet = m_pPrinter->Open(DEVTYPE2STR(m_stConfig.nDeviceType));
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "Open[%s] fail, ErrCode = %d, Return: %d",
            DEVTYPE2STR(m_stConfig.nDeviceType), nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    nRet = OnInit();

    if (bReConn != TRUE)
    {
        SetInit();      // 非断线重连时初始功能设置
    }

    m_pPrinter->GetVersion(szFWVersion, 64, GET_VER_FW);

    m_cExtra.AddExtra("VRTCount", "3");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byXFSVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", (char*)byDevVRTU);
    m_cExtra.AddExtra("VRTDetail[02]", szFWVersion);

    // 更新一次状态
    OnStatus();

    Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());

    return WFS_SUCCESS;
}

// Open设备后相关功能初始设置
HRESULT CXFS_JPR::SetInit()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//-----------------------------------接口-------------------------------
inline int MulDiv(int number, int numberator, int denominator)
{
    long long ret = number;
    ret *= numberator;
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
//-----------------------------------子处理接口-------------------------------
// PrintForm子处理
HRESULT CXFS_JPR::InnerPrintForm(LPWFSPTRPRINTFORM pInData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

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
        Log(ThisModule, __LINE__, "FORM<%s>超出MEDIA<%s>.SIZE属性指定范围(包含OffsetX,OffsetY入参), Return: %d",
            pInData->lpszFormName, pInData->lpszMediaName, WFS_ERR_PTR_MEDIAOVERFLOW);
        return WFS_ERR_PTR_MEDIAOVERFLOW;
    }
    RECT rc;
    pc.pMedia->GetPrintArea(rc);
    if (offset.cx < rc.left ||
        (rc.right - rc.left > 0 && sizeForm.cx + offset.cx > rc.right - rc.left) ||
        offset.cy < rc.top ||
        (rc.bottom - rc.top > 0 && sizeForm.cy + offset.cy > rc.bottom - rc.top))
    {
        Log(ThisModule, __LINE__, "FORM<%s>超出MEDIA<%s>.PRINTAREA属性指定范围(包含OffsetX,OffsetY入参), Return: %d",
            pInData->lpszFormName, pInData->lpszMediaName, WFS_ERR_PTR_MEDIAOVERFLOW);
        return WFS_ERR_PTR_MEDIAOVERFLOW;
    }
    FORMALIGN FormAlign = pc.pForm->GetAlign();
    if (WFS_PTR_ALNUSEFORMDEFN != pInData->wAlignment &&
        WFS_PTR_ALNTOPLEFT != pInData->wAlignment &&
        WFS_PTR_ALNTOPRIGHT != pInData->wAlignment &&
        WFS_PTR_ALNBOTTOMLEFT != pInData->wAlignment &&
        WFS_PTR_ALNBOTTOMRIGHT != pInData->wAlignment)
    {
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
    if (m_stConfig.nVerifyField > 0)
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
        pc.pSubform = nullptr;
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

    if (hRet != WFS_SUCCESS)
    {
        pc.bCancel = TRUE;
    }

    //HRESULT hResOld = hRes;
    hRet = EndForm(&pc);

    if (hRet != WFS_SUCCESS || pInData->dwMediaControl == 0)
    {
        UpdateDeviceStatus();
        if (m_sStatus.fwMedia == WFS_PTR_MEDIAJAMMED)
        {
            Log(ThisModule, __LINE__, "打印机卡纸");
            return WFS_PTR_MEDIAJAMMED;
        }
    }

    return hRet;
}

// RawData无格式打印命令子处理
HRESULT CXFS_JPR::InnerRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;

    if (nSize == 0)
    {
        return WFS_SUCCESS;
    }

    // 设置行高
    m_pPrinter->SetData(&m_stConfig.stPrtModeRaw.wRowHeight, SET_DEV_ROWHEIGHT);

    if (m_stConfig.nRawDataInPar == 0)                       // RawData入参模式:0/UTF8
    {                                                       //
        // 转码(UTF8->GBK)
        QTextCodec *codec = QTextCodec::codecForLocale();
        QTextCodec *codec1 = QTextCodec::codecForName("gb18030");
        if (nullptr == codec1) codec1 = QTextCodec::codecForName("gb2312");
        if (nullptr == codec1) codec1 = QTextCodec::codecForName("gbk");
        if (nullptr == codec1)
        {
            Log(ThisModule, __LINE__, "转码失败,获取当前编码集失败, Return: %d", WFS_ERR_PTR_CHARSETDATA);
            return WFS_ERR_PTR_CHARSETDATA;
        }
        QString strText = QString::fromUtf8((char *)pData, nSize);
        QTextCodec::setCodecForLocale(codec1);
        QByteArray tmpData = strText.toLocal8Bit();
        char *pTempCode = tmpData.data();
        int nTempSize = tmpData.size();
        ULONG ulDataSize = 0;
        int nBufferSize = nTempSize + 2 + (nTempSize / m_stConfig.nLineSize) * 2;
        BYTE *pBuf = new BYTE[nBufferSize];
        memset(pBuf, 0, nBufferSize);
        RemoveUnPrintableChar(nTempSize, (LPBYTE)pTempCode, ulDataSize, pBuf);
        QTextCodec::setCodecForLocale(codec);

        // 去除不可打印字符
        if ('\n' != pBuf[ulDataSize - 1])
        {
            pBuf[ulDataSize++] = '\n';
        }
        pBuf[ulDataSize] = 0;

        //hRes = PrintString((char *)pBuf, ulDataSize, FALSE);
        hRet = PrintString2((char *)pBuf, ulDataSize, FALSE);
        delete [] pBuf;
        pBuf = nullptr;
    } else                                                  // RawData入参模式:非0/GBK
    {
        //hRes = PrintString((char *)pData, nSize, FALSE);
        hRet = PrintString2((char *)pData, nSize, FALSE);
    }

    if (hRet != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印失败, Return: %d", hRet);
        return hRet;
    }

    if (bExpectResp)
    {
        m_pData->m_InputRawData.SetData(0, nullptr);
    }

    return WFS_SUCCESS;
}

// 设备复位命令子处理
HRESULT CXFS_JPR::InnerReset(DWORD dwMediaControl, USHORT usBinIndex)
{
    Q_UNUSED(usBinIndex);
    THISMODULE(__FUNCTION__);

    INT nRet = m_pPrinter->Init();
    UpdateDeviceStatus();
    if (nRet != PTR_SUCCESS)
    {
        if (nRet == ERR_PTR_NO_PAPER)// 无纸
        {
            m_bNeedKeepJammedStatus = FALSE;
            Log(ThisModule, __LINE__, "复位成功, 设备无纸");
            return WFS_SUCCESS;
        }

        Log(ThisModule, __LINE__, "复位失败, ErrCode: %d, Return: %d", nRet, WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;

    }
    m_bReset = TRUE;

    m_bNeedKeepJammedStatus = FALSE;
    Log(ThisModule, __LINE__, "复位成功");
    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//-----------------------------------重载接口-------------------------------
//
HRESULT CXFS_JPR::OnInit()
{
    THISMODULE(__FUNCTION__);

    long nRet = m_pPrinter->Init();
    UpdateDeviceStatus();
    if (nRet < 0)
    {
        return ConvertErrCode(nRet);
    }

    return ConvertErrCode(nRet);
}

//
HRESULT CXFS_JPR::OnExit()
{
    if (nullptr != m_pPrinter)
    {
        m_pPrinter->Close();
    }

    return WFS_SUCCESS;
}

// PrintForm最终处理
HRESULT CXFS_JPR::EndForm(PrintContext *pContext)
{
    if (m_stConfig.wPrintMode == 0)  // 批量打印方式
    {
        if (m_stConfig.nDeviceType == DEV_BTT080AII)
        {
            return EndForm_BTT080AII(pContext);
        }
    } else  // 分Field打印方式
    {
        return EndForm_Print(pContext);
    }
}


//-----------------------------------------------------------------------------------
//--------------------------------------功能处理接口-----------------------------------
// 加载DevXXX动态库
bool CXFS_JPR::LoadDevDll(LPCSTR ThisModule)
{
    if (m_pPrinter == nullptr)
    {
        if (m_pPrinter.Load(m_stConfig.szDevDllName, "CreateIDevPTR", DEVTYPE2STR(m_stConfig.nDeviceType)) != 0)
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s, DriverType=%d|%s, ERR:%s",
                m_stConfig.szDevDllName, m_stConfig.nDeviceType,
                DEVTYPE2STR(m_stConfig.nDeviceType), m_pPrinter.LastError().toUtf8().constData());
            return false;
        } else
        {
            Log(ThisModule, __LINE__, "加载库: DriverDllName=%s, DriverType=%d|%s, Succ.",
                m_stConfig.szDevDllName, m_stConfig.nDeviceType, DEVTYPE2STR(m_stConfig.nDeviceType));
        }
    }
    return (m_pPrinter != nullptr);
}

// 加载INI设置
void CXFS_JPR::InitConifig()
{
    THISMODULE(__FUNCTION__);

    CHAR    szIniAppName[MAX_PATH];

    // DevJPR动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 设备类型
    m_stConfig.nDeviceType = m_cXfsReg.GetValue("DriverType", (DWORD)DEV_BTT080AII);

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.nOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stConfig.nOpenFailRet != 0 && m_stConfig.nOpenFailRet != 1)
    {
        m_stConfig.nOpenFailRet = 0;
    }

    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.nDeviceType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    if (m_stConfig.nDeviceType == DEV_BTT080AII)    // 新北洋BT-T080AII打印机 参数获取
    {
        // 设备打开模式(0默认模式/1指定内部ID,缺省0)
        m_stConfig.stDevOpenMode.nOpenMode = (DWORD)m_cXfsReg.GetValue(szIniAppName, "OpenMode", (DWORD)0);
        if (m_stConfig.stDevOpenMode.nOpenMode > 1 || m_stConfig.stDevOpenMode.nOpenMode < 0)
        {
            m_stConfig.stDevOpenMode.nOpenMode = 0;
        }
        // 设备内部ID,缺省0
        m_stConfig.stDevOpenMode.nOpenParam = (DWORD)m_cXfsReg.GetValue(szIniAppName, "DeviceID", (DWORD)0);
        if (m_stConfig.stDevOpenMode.nOpenParam < 0)
        {
            m_stConfig.stDevOpenMode.nOpenParam = 0;
        }
    }

    // 是否对传入的打印数据进行Field检查(缺省0)
    m_stConfig.nVerifyField = m_cXfsReg.GetValue("CONFIG", "verify_field", (DWORD)0);
    if (m_stConfig.nVerifyField < 0 || m_stConfig.nVerifyField > 2)
    {
        m_stConfig.nVerifyField = 0;
    }

    m_stConfig.nPageSize = m_cXfsReg.GetValue("CONFIG", "split_size", 2976);
    if (m_stConfig.nPageSize < 49)
    {
        m_stConfig.nPageSize = 50;
    }
    if (m_stConfig.nPageSize > 2796)
    {
        m_stConfig.nPageSize = 2976;
    }

    // 一页所占占行数
    m_stConfig.nPageLine = m_cXfsReg.GetValue("CONFIG", "page_lines", 30);
    if (m_stConfig.nPageLine < 1)
    {
        m_stConfig.nPageLine = 1;
    }
    if (m_stConfig.nPageLine > 44)
    {
        m_stConfig.nPageLine = 44;
    }

    // 一行字符数
    m_stConfig.nLineSize = m_cXfsReg.GetValue("CONFIG", "line_size", 50);
    if (m_stConfig.nLineSize < 1 || m_stConfig.nLineSize > 50)
    {
        m_stConfig.nLineSize = 50;
    }

    m_stConfig.bEnableSplit = m_cXfsReg.GetValue("CONFIG", "enabled_split", 1) != 0;

    // RawData入参模式: 0/UTF8;1GBK,缺省0
    m_stConfig.nRawDataInPar = m_cXfsReg.GetValue("RawData", "InParMode", (DWORD)0);
    if (m_stConfig.nRawDataInPar != 0 && m_stConfig.nRawDataInPar != 1)
    {
        m_stConfig.nRawDataInPar = 0;
    }

    //------------------------------打印相关配置------------------------------
    // 打印方式(针对打印机模式设定, 0批量打印/1分Field打印,缺省0)
    // 批量打印方式: 所有打印数据组成一个串单次打印,不支持设置单个打印项的属性
    // 分Field打印: 按Field指定的打印项分别下发到打印机,支持设置单个打印项的属性
    m_stConfig.wPrintMode = m_cXfsReg.GetValue("PRINT_CONFIG", "PrintMode", (DWORD)0);
    if (m_stConfig.wPrintMode != 0 && m_stConfig.wPrintMode != 1)
    {
        m_stConfig.wPrintMode = 0;
    }

    // PrintForm左边距,单位: 0.1MM, 缺省0
    m_stConfig.stPrtModeForm.wLeft = m_cXfsReg.GetValue("PRINT_CONFIG", "LeftMargin", (DWORD)0);


    //-----------------用于Form标准相关定义----------------
    // 介质上边界留白高度单位(0:行列值,1:毫米,2:0.1毫米,缺省0)
    // Field CPI(字符宽)单位(0:Cen标准,1:毫米,2:0.1毫米,缺省0)
    m_stConfig.nFieldCPIMode = m_cXfsReg.GetValue("FORM_CONFIG", "FieldCPIMode", (DWORD)0);
    if (m_stConfig.nFieldCPIMode < 0 || m_stConfig.nFieldCPIMode > 3)
    {
        m_stConfig.nFieldCPIMode = 0;
    }

    // Field LPI(行高)单位(0:Cen标准,1:毫米,2:0.1毫米,缺省0)
    m_stConfig.nFieldLPIMode = m_cXfsReg.GetValue("FORM_CONFIG", "FieldLPIMode", (DWORD)0);
    if (m_stConfig.nFieldLPIMode < 0 || m_stConfig.nFieldLPIMode > 3)
    {
        m_stConfig.nFieldLPIMode = 0;
    }


    //------------------------------RawData命令配置------------------------------
    // RawData入参模式: 0/UTF8;1GBK,缺省0
    m_stConfig.nRawDataInPar = m_cXfsReg.GetValue("RawData", "InParMode", (DWORD)0);
    if (m_stConfig.nRawDataInPar != 0 && m_stConfig.nRawDataInPar != 1)
    {
        m_stConfig.nRawDataInPar = 0;
    }

    // 左边距,单位: 0.1MM, 缺省0
    m_stConfig.stPrtModeRaw.wLeft = m_cXfsReg.GetValue("RawData", "LeftMargin", (DWORD)0);

    // 行高,单位: 0.1MM, 缺省30, 设置<30时按缺省值
    m_stConfig.stPrtModeRaw.wRowHeight = m_cXfsReg.GetValue("RawData", "RowHeight", (DWORD)30);
    if (m_stConfig.stPrtModeRaw.wRowHeight < 30)
    {
        m_stConfig.stPrtModeRaw.wRowHeight = 30;
    }


    //------------------------------条形码格式配置------------------------------
    // 条码类型关键字列表指定: 通过Field.FONT指定的字符串确定条码类型关键字,不设置则无效,采用BarType缺省指定
    // UPC-A条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[0], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_UPCA", ""));
    // UPC-C条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[1], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_UPCC", ""));
    // JAN13/EAN13条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[2], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_JAN13", ""));
    // JAN8/EAN8条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[3], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_JAN8", ""));
    // CODE39条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[4], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_CODE39", ""));
    // INTERLEAVED 2 OF 5条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[5], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_INTE", ""));
    // CODEBAR条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[6], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_CODEBAR", ""));
    // CODE93条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[7], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_CODE93", ""));
    // CODE128条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[8], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_CODE128", ""));
    // PDF417条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[9], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_PDF417", ""));
    // QRCODE条码关键字,缺省空
    strcpy(m_stConfig.szBarFontList[10], m_cXfsReg.GetValue("BARCODE_CONFIG", "FieldFont_QRCODE", ""));

    // 指定条码类型(缺省0)
    m_stConfig.wPrtBarcodeMode[0] = m_cXfsReg.GetValue("BARCODE_CONFIG", "BarType", (DWORD)0);
    if (m_stConfig.wPrtBarcodeMode[0] < 0 || m_stConfig.wPrtBarcodeMode[0] > 8)
    {
        m_stConfig.wPrtBarcodeMode[0] = 0;
    }

    // 基本元素宽度点数(1~6,缺省3)
    m_stConfig.wPrtBarcodeMode[1] = m_cXfsReg.GetValue("BARCODE_CONFIG", "BarBasicWidth", (DWORD)3);
    if (m_stConfig.wPrtBarcodeMode[1] < 1 || m_stConfig.wPrtBarcodeMode[1] > 6)
    {
        m_stConfig.wPrtBarcodeMode[1] = 3;
    }

    // 条码高度(1~255,单位:点,缺省48)，注:1MM=8个点
    m_stConfig.wPrtBarcodeMode[2] = m_cXfsReg.GetValue("BARCODE_CONFIG", "BarHeight", (DWORD)48);
    if (m_stConfig.wPrtBarcodeMode[2] < 1 || m_stConfig.wPrtBarcodeMode[2] > 255)
    {
        m_stConfig.wPrtBarcodeMode[2] = 48;
    }

    // 指定HRI字符的字体类型(0:标准ASCII, 1压缩ASCII, 缺省0)
    m_stConfig.wPrtBarcodeMode[3] = m_cXfsReg.GetValue("BARCODE_CONFIG", "HRIFontType", (DWORD)0);
    if (m_stConfig.wPrtBarcodeMode[3] < 0 || m_stConfig.wPrtBarcodeMode[3] > 1)
    {
        m_stConfig.wPrtBarcodeMode[3] = 0;
    }

    // 指定HRI字符的位置(0:不打印, 1:只在条码上方打印, 2:只在条码下方打印, 3:条码上下方都打印, 缺省2)
    m_stConfig.wPrtBarcodeMode[4] = m_cXfsReg.GetValue("BARCODE_CONFIG", "HRIFontPos", (DWORD)2);
    if (m_stConfig.wPrtBarcodeMode[4] < 0 || m_stConfig.wPrtBarcodeMode[4] > 3)
    {
        m_stConfig.wPrtBarcodeMode[4] = 2;
    }

    //------------------------------PDF417码格式配置------------------------------
    // 基本元素宽度点数(1~7,缺省3)
    m_stConfig.wPrtPDF417Mode[0] = m_cXfsReg.GetValue("PDF417_CONFIG", "BasicWidth", (DWORD)3);
    if (m_stConfig.wPrtPDF417Mode[0] < 1 || m_stConfig.wPrtPDF417Mode[0] > 7)
    {
        m_stConfig.wPrtPDF417Mode[0] = 3;
    }

    // 元素高度点数(2~25,缺省15)
    m_stConfig.wPrtPDF417Mode[1] = m_cXfsReg.GetValue("PDF417_CONFIG", "Height", (DWORD)15);
    if (m_stConfig.wPrtPDF417Mode[1] < 2 || m_stConfig.wPrtPDF417Mode[1] > 25)
    {
        m_stConfig.wPrtPDF417Mode[1] = 15;
    }

    // 条码的行数(3~90,缺省60)
    m_stConfig.wPrtPDF417Mode[2] = m_cXfsReg.GetValue("PDF417_CONFIG", "Lines", (DWORD)60);
    if (m_stConfig.wPrtPDF417Mode[2] < 3 || m_stConfig.wPrtPDF417Mode[2] > 90)
    {
        m_stConfig.wPrtPDF417Mode[2] = 60;
    }

    // 条码的列数(1~30,缺省15)
    m_stConfig.wPrtPDF417Mode[3] = m_cXfsReg.GetValue("PDF417_CONFIG", "Columns", (DWORD)15);
    if (m_stConfig.wPrtPDF417Mode[3] < 1 || m_stConfig.wPrtPDF417Mode[3] > 30)
    {
        m_stConfig.wPrtPDF417Mode[3] = 15;
    }

    // 条码的外观比高度(1~10,缺省5)
    m_stConfig.wPrtPDF417Mode[4] = m_cXfsReg.GetValue("PDF417_CONFIG", "ScaleHeight", (DWORD)5);
    if (m_stConfig.wPrtPDF417Mode[4] < 1 || m_stConfig.wPrtPDF417Mode[4] > 10)
    {
        m_stConfig.wPrtPDF417Mode[4] = 5;
    }

    // 条码的外观比宽度(1~100,缺省50)
    m_stConfig.wPrtPDF417Mode[5] = m_cXfsReg.GetValue("PDF417_CONFIG", "ScaleWidth", (DWORD)50);
    if (m_stConfig.wPrtPDF417Mode[5] < 1 || m_stConfig.wPrtPDF417Mode[5] > 100)
    {
        m_stConfig.wPrtPDF417Mode[5] = 50;
    }

    // 纠错级别(0~8,缺省5)
    m_stConfig.wPrtPDF417Mode[6] = m_cXfsReg.GetValue("PDF417_CONFIG", "ScaleWidth", (DWORD)5);
    if (m_stConfig.wPrtPDF417Mode[6] < 1 || m_stConfig.wPrtPDF417Mode[6] > 8)
    {
        m_stConfig.wPrtPDF417Mode[6] = 5;
    }


    //------------------------------二维码格式配置------------------------------
    // 基本元素宽度点数(1~10,缺省3)
    m_stConfig.wPrtQrcodeMode[0] = m_cXfsReg.GetValue("QRCODE_CONFIG", "QRBasicWidth", (DWORD)3);
    if (m_stConfig.wPrtQrcodeMode[0] < 1 || m_stConfig.wPrtQrcodeMode[0] > 10)
    {
        m_stConfig.wPrtQrcodeMode[0] = 3;
    }

    // 符号类型(1:原始类型, 2:增强类型,缺省2)
    m_stConfig.wPrtQrcodeMode[1] = m_cXfsReg.GetValue("QRCODE_CONFIG", "QRSymbolType", (DWORD)2);
    if (m_stConfig.wPrtQrcodeMode[1] < 1 || m_stConfig.wPrtQrcodeMode[1] > 2)
    {
        m_stConfig.wPrtQrcodeMode[1] = 2;
    }

    // 语言模式(0:汉字, 1:日文,缺省0)
    m_stConfig.wPrtQrcodeMode[2] = m_cXfsReg.GetValue("QRCODE_CONFIG", "QRLanguageMode", (DWORD)0);
    if (m_stConfig.wPrtQrcodeMode[2] < 0 || m_stConfig.wPrtQrcodeMode[2] > 1)
    {
        m_stConfig.wPrtQrcodeMode[2] = 0;
    }

    // 纠错级别(0:L级别[7%], 1:M级别[15%], 2:Q级别[25%], 3:H级别,[30%] 缺省1)
    m_stConfig.wPrtQrcodeMode[3] = m_cXfsReg.GetValue("QRCODE_CONFIG", "QRErrorCorrect", (DWORD)1);
    if (m_stConfig.wPrtQrcodeMode[3] < 0 || m_stConfig.wPrtQrcodeMode[3] > 3)
    {
        m_stConfig.wPrtQrcodeMode[3] = 1;
    }
}

// 初始化Cen标准状态类
HRESULT CXFS_JPR::InitStatus()
{
    memset(&m_sStatus, 0x00, sizeof(WFSPTRSTATUS));
    m_sStatus.fwDevice      = WFS_PTR_DEVNODEVICE;
    m_sStatus.fwPaper[0]    = WFS_PTR_PAPERUNKNOWN;
    m_sStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
    m_sStatus.fwToner       = WFS_PTR_TONERFULL;
    m_sStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
    m_sStatus.fwInk         = WFS_PTR_INKNOTSUPP;

    return WFS_SUCCESS;
}

// 初始化Cen标准能力值类
HRESULT CXFS_JPR::InitCaps()
{
    m_sCaps.fwType = WFS_PTR_TYPEJOURNAL;          // 设备类型
    m_sCaps.fwWriteForm = WFS_PTR_WRITETEXT | WFS_PTR_WRITEGRAPHICS;
    m_sCaps.fwControl = 0;
    m_sCaps.bMediaTaken = FALSE;

    return WFS_SUCCESS;
}

//
HRESULT CXFS_JPR::PrintData(const char *pBuffer, DWORD dwSize)
{
    THISMODULE(__FUNCTION__);

    int nRet = m_pPrinter->PrintData(pBuffer, dwSize);
    UpdateDeviceStatus();
    if (nRet)
    {
        Log(ThisModule, __LINE__, "PrintData fail, ErrCode:%d", nRet);
    }
    return ConvertErrCode(nRet);
}

// 调用DevXXX动态库的图片打印接口1
HRESULT CXFS_JPR::PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight)
{
    THISMODULE(__FUNCTION__);

    INT nRet = m_pPrinter->PrintImage(szImagePath, nDstWidth, nDstHeight);
    UpdateDeviceStatus();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印图片错误: %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

// 调用DevXXX动态库的图片打印接口2
HRESULT CXFS_JPR::PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY)
{
    THISMODULE(__FUNCTION__);

    INT nRet = m_pPrinter->PrintImageOrg(szImagePath, ulOrgX, ulOrgY);
    UpdateDeviceStatus();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "打印图片错误: %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

// 打印前数据处理
HRESULT CXFS_JPR::AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut)
{
    THISMODULE(__FUNCTION__);

    if (dwSize == 0)
    {
        return WFS_SUCCESS;
    }

    int iCurPos = 0, iLenToEnter = 0, iEnterNum = 0, iLastSize = 0;
    const char *pp = pBuffer;
    int PAGESIZE = m_stConfig.nPageSize;
    int PAGELINE = m_stConfig.nPageLine;
    // FORM打印时根据数据量判断是否需要换行
    bool bNeedCR = true;
    if (dwSize < (DWORD)PAGESIZE && FALSE != bIsFromPrint)
    {
        bNeedCR = false;
    }

    if (m_stConfig.bEnableSplit)
    {
        while (dwSize > 0)
        {
            iLastSize = dwSize > (DWORD)PAGESIZE ? PAGESIZE : dwSize;
            while (iCurPos < iLastSize)
            {
                if (pp[ iCurPos ] & 0x80)
                {
                    iCurPos++;
                }
                else
                {
                    if (pp[ iCurPos ] == '\n')
                    {
                        if (iEnterNum < PAGELINE)
                        {
                            ++iEnterNum;
                            iLenToEnter = iCurPos + 1;
                        }
                        else
                        {
                            iCurPos++;
                            break;
                        }
                    }
                }
                iCurPos++;
            }

            if ((iCurPos >= PAGESIZE) && ('\n' != pp[ iCurPos - 1 ]) && (0 != iLenToEnter))
            {
                iCurPos = iLenToEnter;
            }

            char cLastChar = ((char *)pp)[ iCurPos - 1 ];
            char cBackup;
            bool bChanged = false;
            // FORM打印时根据数据量判断是否需要换行
            if (cLastChar != '\n' && bNeedCR)
            {
                cBackup = ((char *)pp)[ iCurPos ];
                ((char *)pp)[ iCurPos ] = '\n';
                iCurPos++;
                bChanged = true;
            }

            if ((dwSizeOut + iCurPos) < MAX_PRINTDATA_LEN)
            {
                memcpy(pBuffOut + dwSizeOut, pp, iCurPos);
                dwSizeOut += iCurPos;
            }
            else
            {
                Log(ThisModule, __LINE__, "PrintData(%u) is more than MAXLEN", dwSizeOut + iCurPos);
            }

            if (bChanged)
            {
                ((char *)pp)[ iCurPos ] = cBackup;
                --iCurPos;
            }
            bChanged = false;

            pp = pp + iCurPos;
            dwSize -= iCurPos;
            iCurPos = 0;
            iEnterNum = 0;
            iLenToEnter = 0;
        }
    } else
    {
        memcpy(pBuffOut + dwSizeOut, pp, iCurPos);
        dwSizeOut += iCurPos;        
    }

    return WFS_SUCCESS;
}

// 打印前数据处理
HRESULT CXFS_JPR::PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint)
{
    THISMODULE(__FUNCTION__);

    // 无数据直接返回成功
    if (dwSize == 0)
    {
        return WFS_SUCCESS;
    }

    int iCurPos = 0, iLenToEnter = 0, iEnterNum = 0, iLastSize = 0;
    const char *pp = pBuffer;
    int PAGESIZE = m_stConfig.nPageSize;
    int PAGELINE = m_stConfig.nPageLine;
    // FORM打印时根据数据量判断是否需要换行
    bool bNeedCR = true;
    if (dwSize < (DWORD)PAGESIZE && FALSE != bIsFromPrint)
    {
        bNeedCR = false;
    }

    int nRet = PTR_SUCCESS;
    if (m_stConfig.bEnableSplit) // 对传入数据根据INI指定一行数目进行换行处理
    {
        while (dwSize > 0)
        {
            iLastSize = dwSize > (DWORD)PAGESIZE ? PAGESIZE : dwSize;
            while (iCurPos < iLastSize)
            {
                if (pp[ iCurPos ] & 0x80)
                {
                    iCurPos++;
                } else
                {
                    if (pp[ iCurPos ] == '\n')
                    {
                        if (iEnterNum < PAGELINE)
                        {
                            ++iEnterNum;
                            iLenToEnter = iCurPos + 1;
                        }
                        else
                        {
                            iCurPos++;
                            break;
                        }
                    }
                }
                iCurPos++;
            }

            if ((iCurPos >= PAGESIZE) && (pp[ iCurPos - 1 ] != '\n') && (iLenToEnter != 0))
            {
                iCurPos = iLenToEnter;
            }

            char cLastChar = ((char *)pp)[ iCurPos - 1 ];
            char cBackup;
            bool bChanged = false;
            // FORM打印时根据数据量判断是否需要换行
            if (cLastChar!= '\n'  && bNeedCR)
            {
                cBackup = ((char *)pp)[ iCurPos ];
                ((char *)pp)[ iCurPos ] = '\n';
                iCurPos++;
                bChanged = true;
            }

            if (bIsFromPrint == false)
            {
                if (dwSize > 0 && cLastChar == '\n')
                {
                    ((char *)pp)[ iCurPos - 1 ] = '\0';
                }
            }

            nRet = m_pPrinter->PrintData(const_cast<char *>(pp), iCurPos);

            if (bChanged)
            {
                ((char *)pp)[ iCurPos ] = cBackup;
                --iCurPos;
            }
            bChanged = false;

            if (nRet != PTR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "打印失败, ErrCode: %d", nRet);
                break;
            }
            pp = pp + iCurPos;
            dwSize -= iCurPos;
            iCurPos = 0;
            iEnterNum = 0;
            iLenToEnter = 0;
        }
    } else
    {
        nRet = m_pPrinter->PrintData(const_cast<char *>(pBuffer), dwSize);
    }
    UpdateDeviceStatus();
    return ConvertErrCode(nRet);
}

// 打印前数据处理
HRESULT CXFS_JPR::PrintString2(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint)
{
    THISMODULE(__FUNCTION__);

    if (dwSize == 0)
    {
        return WFS_SUCCESS;
    }

    INT nRet = PTR_SUCCESS;
    INT nPrtStartPos = 0;   // 打印起始位置
    INT nCurPos = 0;        // 处理数据的当前位置
    INT nPrtDataSize = 0;   // 打印字符数
    BOOL bIsRunPrt = FALSE; // 是否打印标记
    INT nLeftChrCnt = ((m_stConfig.stPrtModeRaw.wLeft / 30) +
                       ((m_stConfig.stPrtModeRaw.wLeft % 30) > 1 ? 1 : 0));    // 左边距转换为字符数
    INT nLineCount = m_stConfig.nLineSize - nLeftChrCnt;   // 一行可容纳字符数
    INT nLeftMargin = m_stConfig.stPrtModeRaw.wLeft;
    LPCSTR lpcData = pBuffer;

    // 新北洋BT-NH80M打印机: 物理左边距=1MM
    //nLeftMargin = (nLeftMargin - 10 < 0 ? 0 : nLeftMargin - 10);

    // 开始循环处理
    while (nCurPos < dwSize)
    {
        if (nCurPos + 1 >= dwSize)
        {
            bIsRunPrt = TRUE;
            nCurPos ++;
        } else
        {
            if (lpcData[nCurPos] & 0x80)
            {
                nCurPos++;
                nPrtDataSize ++;
            } else
            {
                if (lpcData[nCurPos] == '\n')    // 出现换行符
                {
                    nCurPos ++;
                    bIsRunPrt = TRUE;
                } else
                {
                    nCurPos++;
                    nPrtDataSize ++;
                    if (m_stConfig.bEnableSplit == TRUE)     // 按INI指定一行字符数进行换行处理
                    {
                        if (nPrtDataSize >= nLineCount)    // 处理字符数 >= 一行可容纳字符数
                        {
                            bIsRunPrt = TRUE;
                        }
                    }
                }
            }
        }

        if (bIsRunPrt == TRUE)  // 执行打印
        {
            if (nPrtDataSize == 0)  // 没有打印数据时直接换行
            {
                m_pPrinter->SetData(nullptr, SET_DEV_LINEFEED);
            } else
            {
                nRet = m_pPrinter->PrintDataOrg(std::string(lpcData + nPrtStartPos).substr(0, nPrtDataSize).c_str(),
                                                nPrtDataSize, nLeftMargin, 0);
                if (nRet != PTR_SUCCESS)
                {
                    Log(ThisModule, __LINE__, "打印数据: fail, ErrCode: %d, Return: %d",
                        nRet, ConvertErrCode(nRet));
                    return ConvertErrCode(nRet);
                }
            }

            bIsRunPrt = FALSE;
            nPrtStartPos = nCurPos;
            nPrtDataSize = 0;
        }
    }

    UpdateDeviceStatus();

    return WFS_SUCCESS;
}

void CXFS_JPR::RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData)
{
    ulOutSize = 0;
    int PAGELINE = m_stConfig.nLineSize;                                 // 一行字符数
    int nLineCnt = 0;                                                   // 一行字符数计数
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
            if ((nLineCnt % PAGELINE == 0 && nLineCnt != 0) ||
               (nLineCnt % PAGELINE + 1) == PAGELINE)
            {
                pOutData[ulOutSize++] = '\n';
                nLineCnt = 0;
            }
            pOutData[ulOutSize++] = pInData[i];
            pOutData[ulOutSize++] = pInData[i + 1];
            i++;
            nLineCnt = nLineCnt + 2;
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
            if ((nLineCnt % PAGELINE == 0 && nLineCnt != 0) ||
               (nLineCnt % PAGELINE + 1) == PAGELINE)
            {
                pOutData[ulOutSize++] = '\n';
                nLineCnt = 0;
            }
            pOutData[ulOutSize++] = pInData[i];
            pOutData[ulOutSize++] = pInData[i + 1];
            i++;
            nLineCnt = nLineCnt + 2;
        }
        else if (pInData[i] >= 0x80)
        {
            i++;
        }
        else if ((pInData[i] == 0x0A)
                 || ((pInData[i] > 0x1F) && (pInData[i] < 0x7F)))
        {
            if (nLineCnt % PAGELINE == 0 && nLineCnt != 0)
            {
                pOutData[ulOutSize++] = '\n';
                nLineCnt = 0;
            }
            if (pInData[i] == '\n')
            {
                nLineCnt = 0;
            }
            pOutData[ulOutSize++] = pInData[i];
            nLineCnt = nLineCnt + 1;
        }
    }
}

// 设备状态实时更新
void CXFS_JPR::UpdateDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题
    //int     nPrinterStatus  = WFS_PTR_DEVHWERROR;

    INT     nRet = PTR_SUCCESS;

    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFirePaperStatus    = FALSE;
    BOOL    bNeedFireTonerStatus    = FALSE;
    BOOL    bNeedFireHWError        = FALSE;

    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log

    DEVPTRSTATUS stDevStatus;
    nRet = m_pPrinter->GetStatus(stDevStatus);

    m_sStatus.fwInk                  = WFS_PTR_INKNOTSUPP;
    m_sStatus.fwLamp                 = WFS_PTR_LAMPNOTSUPP;
    m_sStatus.lppRetractBins         = nullptr;
    m_sStatus.usMediaOnStacker       = 0;
    m_sStatus.lpszExtra              = nullptr;


    WFSPTRSTATUS sLastStatus = m_sStatus;

    //----------------------设备状态处理----------------------
    switch (nRet)
    {
        // 打印机出纸口有纸设备状态为ONLINE
        case PTR_SUCCESS:
        case ERR_PTR_PARAM_ERR:
        case ERR_PTR_UNSUP_CMD:
            m_sStatus.fwDevice = WFS_PTR_DEVONLINE;
            break;
        case ERR_PTR_COMM_ERR:
        case ERR_PTR_NOT_OPEN:
        case ERR_PTR_NO_DEVICE:
            m_sStatus.fwDevice = WFS_PTR_DEVOFFLINE;
            break;
        default:
            m_sStatus.fwDevice = WFS_PTR_DEVHWERROR;
            break;
    }

    if (m_sStatus.fwDevice == WFS_PTR_DEVOFFLINE)
    {
        m_sStatus.fwMedia = WFS_PTR_MEDIAUNKNOWN;
        m_sStatus.fwPaper[0] = WFS_PTR_PAPERUNKNOWN;
    }

    // Device == ONLINE && 有命令在执行中,设置Device = BUSY
    if (m_sStatus.fwDevice == WFS_PTR_DEVONLINE && m_bCmdRuning == TRUE)    // 命令执行中
    {
        m_sStatus.fwDevice == WFS_PTR_DEVBUSY;
    }

    //----------------------Media状态处理----------------------
    if (m_sStatus.fwMedia != WFS_PTR_MEDIAENTERING)
    {
        m_sStatus.fwMedia = ConvertPaper2MediaStatus(stDevStatus.wPaper[0]);
    }

    if (m_sStatus.fwMedia == WFS_PTR_MEDIAJAMMED)
    {
        m_sStatus.fwDevice = WFS_PTR_DEVHWERROR;
    }

    //----------------------纸状态处理----------------------
    int nPaperStatus = ConvertPaperStatus(stDevStatus.wPaper[0]);

    if (m_sStatus.fwPaper[0] != (WORD)nPaperStatus)
    {
        m_sStatus.fwPaper[0] = (WORD)nPaperStatus;
    }
    for (int i = 1; i < WFS_PTR_SUPPLYSIZE; i++)
    {
        m_sStatus.fwPaper[i] = WFS_PTR_PAPERNOTSUPP;
    }

    //----------------------状态检查----------------------
    // Device状态变化检查
    if (m_sStatus.fwDevice != sLastStatus.fwDevice)
    {
        bNeedFireStatusChanged = TRUE;
        if (m_sStatus.fwDevice == WFS_PTR_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }

    // 纸状态变化检查,只有当纸状态变为少或空时才Fire状态
    if (sLastStatus.fwPaper[0] !=  m_sStatus.fwPaper[0])
    {
        if (m_sStatus.fwPaper[0] == WFS_PTR_PAPERLOW ||
            m_sStatus.fwPaper[0] == WFS_PTR_PAPEROUT ||
            m_sStatus.fwPaper[0] == WFS_PTR_PAPERFULL)
        {
            bNeedFirePaperStatus = TRUE;
        }
    }

    //----------------------事件上报----------------------
    if (bNeedFireHWError)
    {
        FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
        sprintf(szFireBuffer + strlen(szFireBuffer), "HWEvent:%d,%d|",
                WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
    }

    if (bNeedFireStatusChanged)
    {
        FireStatusChanged(m_sStatus.fwDevice);
        sprintf(szFireBuffer + strlen(szFireBuffer), "StatusChange:%d|",  m_sStatus.fwDevice);
    }

    if (bNeedFirePaperStatus)
    {
        FirePaperThreshold(WFS_PTR_PAPERUPPER, m_sStatus.fwPaper[0]);
        sprintf(szFireBuffer + strlen(szFireBuffer), "PaperThreshold:%d|",  m_sStatus.fwPaper[0]);
    }

    if (bNeedFireTonerStatus)
    {
        FireTonerThreshold(m_sStatus.fwToner);
        sprintf(szFireBuffer + strlen(szFireBuffer), "TonerThreshold:%d|",  m_sStatus.fwToner);
    }

    // 比较两次状态记录LOG
    if (memcmp(&sLastStatus, &m_sStatus, sizeof(WFSPTRSTATUS)) != 0)
    {
        Log(ThisModule, __LINE__, "状态结果比较: Device:%d->%d%s|Media:%d->%d%s|Paper[0]:%d->%d%s|"
                            "Ink:%d->%d%s|Toner:%d->%d%s|Lamp:%d->%d%s|; 事件上报记录: %s.",
            sLastStatus.fwDevice, m_sStatus.fwDevice, (sLastStatus.fwDevice != m_sStatus.fwDevice ? " *" : ""),
            sLastStatus.fwMedia, m_sStatus.fwMedia, (sLastStatus.fwMedia != m_sStatus.fwMedia ? " *" : ""),
            sLastStatus.fwPaper[0], m_sStatus.fwPaper[0], (sLastStatus.fwPaper[0] != m_sStatus.fwPaper[0] ? " *" : ""),
            sLastStatus.fwInk, m_sStatus.fwInk, (sLastStatus.fwInk != m_sStatus.fwInk ? " *" : ""),
            sLastStatus.fwToner, m_sStatus.fwToner, (sLastStatus.fwToner != m_sStatus.fwToner ? " *" : ""),
            sLastStatus.fwLamp, m_sStatus.fwLamp, (sLastStatus.fwLamp != m_sStatus.fwLamp ? " *" : ""),
            szFireBuffer);
    }

    return;
}

// BT-T080AII 打印数据下发
HRESULT CXFS_JPR::EndForm_BTT080AII(PrintContext *pContext)
{
    THISMODULE(__FUNCTION__);

    PRINT_ITEMS *pItems = (PRINT_ITEMS *)pContext->pUserData;
    if (pContext->bCancel)
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

    // 形成打印字串strupr
    int nCurColHeight = 1;
    PRINT_STRING s; // 记录当前行列位置
    BOOL bAppendNewLine = TRUE;
    int nRet = 0;
    char szPrintData[MAX_PRINTDATA_LEN] = {0};
    DWORD dwDataSize = 0;
    for (int i = 0; i < pItems->nItemNum && nRet >= 0; i++)
    {
        PRINT_STRING strFormat; // 记录格式化数据
        PRINT_ITEM *pItem = pItems->pItems[i];
        while (pItem->y > s.m_nCurRow)
        {
            s.m_nCurRow += nCurColHeight;
            nCurColHeight = 1;
            s.m_nCurCol = 0;
            strFormat.Append('\n');
        }
        while (pItem->x > s.m_nCurCol)
        {
            s.m_nCurCol++;
            strFormat.Append(' ');
        }

        // 图片打印
        if (pItem->nFieldType == FT_GRAPHIC)
        {
            if (dwDataSize > 1)
            {
                nRet = PrintData(szPrintData, dwDataSize);
                if (0 > nRet)
                {
                    return nRet;
                }

                memset(szPrintData, 0x00, sizeof(szPrintData));
                dwDataSize = 0;
            }

            s.m_nCurCol = 0;
            nRet = PrintImageOrg(pItem->strImagePath, pItem->x, pItem->y + pItem->nHeight);

            s.m_nCurRow += pItem->nHeight;// 图片上边与行Y坐标齐平

            bAppendNewLine = TRUE;
        } else
        if (pItem->nFieldType == FT_BARCODE)    // 条码打印
        {
            if (dwDataSize > 1) // 有文本先打印
            {
                nRet = PrintData(szPrintData, dwDataSize);
                if (0 > nRet)
                {
                    return nRet;
                }

                memset(szPrintData, 0x00, sizeof(szPrintData));
                dwDataSize = 0;
            }

            DEVPRINTFORMIN stPrintIn;
            DEVPRINTFORMOUT stPrintOut;

            // 下发打印数据
            stPrintIn.Clear();
            MCPY_LEN(stPrintIn.szData, pItem->Text, pItem->nTextLen);
            stPrintIn.dwDataSize = pItem->nTextLen;
            stPrintIn.ulX = pItem->x * 15;
            stPrintIn.ulY = pItem->y * 30;
            stPrintIn.wInMode = PRINTFORM_BAR;
            INT nBarMode = GetBarFontMode(pItem->strFontName);
            if (nBarMode == BAR_PDF417)
            {
                stPrintIn.wInMode = PRINTFORM_PDF417;
                stPrintIn.lOtherParam[1] = m_stConfig.wPrtPDF417Mode[0];
                stPrintIn.lOtherParam[2] = m_stConfig.wPrtPDF417Mode[1];
                stPrintIn.lOtherParam[3] = m_stConfig.wPrtPDF417Mode[2];
                stPrintIn.lOtherParam[4] = m_stConfig.wPrtPDF417Mode[3];
                stPrintIn.lOtherParam[5] = m_stConfig.wPrtPDF417Mode[4];
                stPrintIn.lOtherParam[6] = m_stConfig.wPrtPDF417Mode[5];
                stPrintIn.lOtherParam[7] = m_stConfig.wPrtPDF417Mode[6];
            } else
            if (nBarMode == BAR_QRCODE)
            {
                stPrintIn.wInMode = PRINTFORM_QR;
                stPrintIn.lOtherParam[1] = m_stConfig.wPrtQrcodeMode[0];
                stPrintIn.lOtherParam[2] = m_stConfig.wPrtQrcodeMode[1];
                stPrintIn.lOtherParam[3] = m_stConfig.wPrtQrcodeMode[2];
                stPrintIn.lOtherParam[4] = m_stConfig.wPrtQrcodeMode[3];
            } else
            {
                stPrintIn.lOtherParam[1] = (nBarMode < 0 ? m_stConfig.wPrtBarcodeMode[0] : nBarMode);
                stPrintIn.lOtherParam[2] = m_stConfig.wPrtBarcodeMode[1];
                stPrintIn.lOtherParam[3] = m_stConfig.wPrtBarcodeMode[2];
                stPrintIn.lOtherParam[4] = m_stConfig.wPrtBarcodeMode[3];
                stPrintIn.lOtherParam[5] = m_stConfig.wPrtBarcodeMode[4];
            }

            if (m_stConfig.nDeviceType == DEV_BTT080AII)  // MB2打印机
            {
                stPrintIn.lOtherParam[0] = 1;   // 打印换行

                nRet = m_pPrinter->PrintForm(stPrintIn, stPrintOut);
                if (nRet != PTR_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "下发条码打印数据: EndForm_MB2() fail, ErrCode:%d, Return: %d.", nRet, nRet);
                }
            }
            s.m_nCurCol = 0;
            s.m_nCurRow += pItem->nHeight;// 图片上边与行Y坐标齐平

            bAppendNewLine = TRUE;
        } else // 字符串打印
        {
            PRINT_STRING strText;
            strText.Append(strFormat.GetData(), strFormat.GetLen());
            strText.Append(pItem->Text, pItem->nTextLen);
            s.m_nCurCol += pItem->nWidth;
            if (nCurColHeight < pItem->nHeight)
            {
                nCurColHeight = pItem->nHeight;
            }

            if (strText.GetLen() > 0)
            {
                nRet = AddPrintString(strText.GetData(), strText.GetLen(), TRUE, szPrintData, dwDataSize);
                if ('\n' == strText.GetData()[strText.GetLen() - 1])
                {
                    bAppendNewLine = FALSE;
                }
                else
                {
                    bAppendNewLine = TRUE;
                }
            }
        }
    }

    // 删除自定义数据
    pContext->pUserData = NULL;
    if (pItems)
    {
        delete pItems;
    }

    if ((szPrintData[(dwDataSize > 0) ? dwDataSize - 1 : 0] == '\n') &&
        (dwDataSize < MAX_PRINTDATA_LEN))
    {
        szPrintData[dwDataSize] = '\n';
    }

    nRet = PrintData(szPrintData, dwDataSize);

    if (0 > nRet)
    {
        return nRet;
    }

    return WFS_SUCCESS;
}

// 打印数据下发(分Field方式打印)
HRESULT CXFS_JPR::EndForm_Print(PrintContext *pContext)
{
    THISMODULE(__FUNCTION__);

    // FORM行列值转换为MM(0.1MM单位),有小数位四舍五入
    #define ROWCOL2MM(ROL, MM) \
        pContext->pForm->GetOrigUNIT(nullptr) == FORM_ROWCOLUMN ? \
            (int)(ROL * MM) : ROL * 10

    // FORM设置LPI转MM(0.1MM单位)
    #define LPI2MM(LPI) (LPI < 1 ? (int)(254 / 30) : (int)(254 / LPI))

    CSPPtrData *pData = (CSPPtrData *)m_pData;

    INT nRet = WFS_SUCCESS;


    SIZE stSize = GetPerRowColTwips2MM();
    PRINT_ITEMS *pItems = (PRINT_ITEMS *)pContext->pUserData;
    if (pContext->bCancel)
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

    DEVPRINTFORMIN stPrintIn;                       // 打印接口入参
    DEVPRINTFORMOUT stPrintOut;                     // 打印接口回参
    char szPrintData[MAX_PRINTDATA_LEN] = { 0x00 }; // 存放打印数据
    DWORD dwDataSize = 0;                           // 打印数据长度
    DWORD dwX = 0, dwY = 0;                         // 转换后的XY坐标(单位: 0.1MM)
    WORD  dwCurrLine = 0;                           // 当前打印行
    WORD  wFeedLine = 0;                            // 是否换行
    DWORD dwLPI = 0;
    DWORD dwLPI_Last = LINE_HEIGHT_DEF;
    INT nLeftMargin = m_stConfig.stPrtModeForm.wLeft;// 左边距

    // 循环获取打印数据并下发到打印机
    for (int i = 0; i < pItems->nItemNum && nRet >= 0; i++)
    {
        PRINT_ITEM *pItem = pItems->pItems[i];
        nRet = WFS_SUCCESS;

        // 行列转换为0.1MM值
        dwX = ROWCOL2MM(pItem->x, stSize.cx);
        dwY = ROWCOL2MM(pItem->y, stSize.cy);
        dwLPI = (m_stConfig.nFieldLPIMode == 0 ? LPI2MM(pItem->dwLPI) :
                 (m_stConfig.nFieldLPIMode == 1 ? pItem->dwLPI * 10 :
                  (m_stConfig.nFieldLPIMode == 2 ? pItem->dwLPI : pItem->dwLPI)));

        // 检查行列是否需要换行,记录将打印的行列
        if (pItem->y > dwCurrLine)
        {
            // 下发换行打印
            stPrintIn.Clear();
            stPrintIn.wInMode = PRINTFORM_FEEDLINE;

            if (pItem->y - dwCurrLine > 1)
            {
                stPrintIn.ulY = pItem->y - dwCurrLine - 1;
                stPrintIn.lOtherParam[0] = dwLPI_Last;
                nRet = m_pPrinter->PrintForm(stPrintIn, stPrintOut);
                if (nRet != PTR_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "下发换行打印: PrintForm() fail, ErrCode:%d, Return: %d.", nRet, ConvertErrCode(nRet));
                }
            }

            stPrintIn.ulY = 1;
            stPrintIn.lOtherParam[0] = dwLPI;
            nRet = m_pPrinter->PrintForm(stPrintIn, stPrintOut);
            if (nRet != PTR_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "下发换行打印: PrintForm() fail, ErrCode:%d, Return: %d.", nRet, ConvertErrCode(nRet));
            }
            dwCurrLine = pItem->y;
            dwLPI_Last = dwLPI;
        }

        // 最后一组打印,需要换行
        if (i + 1 == pItems->nItemNum)
        {
            wFeedLine = 1;
        }

        // 打印前X坐标增加左边距
        dwX += nLeftMargin;

        // 文本打印
        if (pItem->nFieldType == FT_TEXT)
        {
            if (pItem->nTextLen > 0)    // 有打印数据
            {
                // 设置打印参数
                DEVPTRFONTPAR stPrtFontPar;
                stPrtFontPar.Clear();
                stPrtFontPar.dwFontType = pItem->dwStyle;   // 字体属性
                if (m_stConfig.nDeviceType == DEV_BTT080AII)
                {
                    if (MCMP_IS0(pItem->strFontName, "宋体"))
                    {
                        stPrtFontPar.dwMarkPar[0] = 3;
                    }
                }
                m_pPrinter->SetData(&stPrtFontPar, DTYPE_SET_PRTFONT);

                // 下发打印数据
                stPrintIn.Clear();
                stPrintIn.wInMode = PRINTFORM_TEXT;
                stPrintIn.ulX = dwX;
                stPrintIn.ulY = dwY;
                MCPY_LEN(stPrintIn.szData, pItem->Text, pItem->nTextLen);
                stPrintIn.dwDataSize = pItem->nTextLen;
                stPrintIn.dwTimeOut = 0;
                stPrintIn.lOtherParam[0] = wFeedLine;

                nRet = m_pPrinter->PrintForm(stPrintIn, stPrintOut);
                if (nRet != PTR_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "下发文本打印数据: PrintForm() fail, ErrCode:%d, Return: %d.", nRet, ConvertErrCode(nRet));
                }
            }
        } else
        // 图片打印
        if (pItem->nFieldType == FT_GRAPHIC)
        {
            stPrintIn.Clear();
            stPrintIn.wInMode = PRINTFORM_PIC;
            stPrintIn.ulX = dwX;
            stPrintIn.ulY = dwY;
            MCPY_LEN(stPrintIn.szData, pItem->strImagePath, strlen(pItem->strImagePath));
            stPrintIn.dwDataSize = strlen(pItem->strImagePath);
            stPrintIn.dwTimeOut = 0;
            stPrintIn.lOtherParam[0] = wFeedLine;

            nRet = m_pPrinter->PrintForm(stPrintIn, stPrintOut);
            if (nRet != PTR_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "下发图片打印数据: PrintForm() fail, ErrCode:%d, Return: %d.", nRet, ConvertErrCode(nRet));
            }

        } else
        // 条码打印
        if (pItem->nFieldType == FT_BARCODE)
        {
            // 下发打印数据
            stPrintIn.Clear();
            MCPY_LEN(stPrintIn.szData, pItem->Text, pItem->nTextLen);
            stPrintIn.dwDataSize = pItem->nTextLen;
            stPrintIn.ulX = dwX;
            stPrintIn.ulY = dwY;
            stPrintIn.wInMode = PRINTFORM_BAR;
            stPrintIn.lOtherParam[0] = wFeedLine;
            INT nBarMode = GetBarFontMode(pItem->strFontName);
            if (nBarMode == BAR_PDF417)
            {
                stPrintIn.wInMode = PRINTFORM_PDF417;
                stPrintIn.lOtherParam[1] = m_stConfig.wPrtPDF417Mode[0];
                stPrintIn.lOtherParam[2] = m_stConfig.wPrtPDF417Mode[1];
                stPrintIn.lOtherParam[3] = m_stConfig.wPrtPDF417Mode[2];
                stPrintIn.lOtherParam[4] = m_stConfig.wPrtPDF417Mode[3];
                stPrintIn.lOtherParam[5] = m_stConfig.wPrtPDF417Mode[4];
                stPrintIn.lOtherParam[6] = m_stConfig.wPrtPDF417Mode[5];
                stPrintIn.lOtherParam[7] = m_stConfig.wPrtPDF417Mode[6];
            } else
            if (nBarMode == BAR_QRCODE)
            {
                stPrintIn.wInMode = PRINTFORM_QR;
                stPrintIn.lOtherParam[1] = m_stConfig.wPrtQrcodeMode[0];
                stPrintIn.lOtherParam[2] = m_stConfig.wPrtQrcodeMode[1];
                stPrintIn.lOtherParam[3] = m_stConfig.wPrtQrcodeMode[2];
                stPrintIn.lOtherParam[4] = m_stConfig.wPrtQrcodeMode[3];
            } else
            {
                stPrintIn.lOtherParam[1] = (nBarMode < 0 ? m_stConfig.wPrtBarcodeMode[0] : nBarMode);
                stPrintIn.lOtherParam[2] = m_stConfig.wPrtBarcodeMode[1];
                stPrintIn.lOtherParam[3] = m_stConfig.wPrtBarcodeMode[2];
                stPrintIn.lOtherParam[4] = m_stConfig.wPrtBarcodeMode[3];
                stPrintIn.lOtherParam[5] = m_stConfig.wPrtBarcodeMode[4];
            }

            nRet = m_pPrinter->PrintForm(stPrintIn, stPrintOut);
            if (nRet != PTR_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "下发条码打印数据: PrintForm() fail, ErrCode:%d, Return: %d.", nRet, ConvertErrCode(nRet));
            }
        } else
        {
            Log(ThisModule, __LINE__, "不支持的Fieled[%s]打印类型[%d],跳过.",
                pItem->strFieldName, pItem->nFieldType);
        }
    }

    // 删除自定义数据
    if (pItems)
    {
        delete pItems;
        pItems = nullptr;
    }
    pContext->pUserData = nullptr;

    return ConvertErrCode(nRet);
}

INT CXFS_JPR::GetBarFontMode(LPSTR lpData)
{
    int n = sizeof(m_stConfig.szBarFontList);
    for (INT i = 0; i < 24; i ++)
    {
        if (strlen(m_stConfig.szBarFontList[i]) > 0 &&
            MCMP_IS0(m_stConfig.szBarFontList[i], lpData))
        {
            return i;
        }
    }

    return -1;
}

