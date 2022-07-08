/***************************************************************
* 文件名称：XFS_PTR_DEC.cpp
* 文件描述：凭条打印模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_PTR.h"

//-----------------------------------------------------------------------------------
// Open设备及初始化相关子处理
HRESULT CXFS_PTR::StartOpen()
{
    char szDevRPRVer[64] = { 0x00 };
    char szFWVersion[64] = { 0x00 };
    long lFWVerSize = 0;

    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_sConfig.nDriverType != DEV_SNBC_BKC310 &&
        m_sConfig.nDriverType != DEV_SNBC_BTNH80)
    {
        Log(ThisModule, -1, "Open fail, INI指定了不支持的设备类型[%d], ErrCode:%d",
            m_sConfig.nDriverType, WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 加载DevRPR动态库
    if (!LoadDevDll(ThisModule))
    {
        return WFS_SUCCESS;//WFS_ERR_HARDWARE_ERROR;
    }

    // 打开设备
    int nRet = m_pPrinter->Open(DEVTYPE2STR(m_sConfig.nDriverType));
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, -1, "Open[%s] fail , ErrCode:%d", DEVTYPE2STR(m_sConfig.nDriverType), nRet);
        return WFS_SUCCESS;//WFS_ERR_HARDWARE_ERROR;
    }
    m_bNeedReset = false;

    /*if (m_sConfig.dwMarkHeader > 0)
    {
        nRet = m_pPrinter->ChkPaperMarkHeader(m_sConfig.dwMarkHeader);
        if (nRet < 0)
        {
            Log(ThisModule, -1, "ChkPaperMarkHeader fail, ErrCode:%d", nRet);
            return WFS_SUCCESS;//WFS_ERR_HARDWARE_ERROR;
        }
        Log(ThisModule, 1, "ChkPaperMarkHeader success");
    }*/

    nRet = OnInit();

    m_pPrinter->GetVersion(szDevRPRVer, 64, GET_VER_DEVRPR);
    m_pPrinter->GetVersion(szFWVersion, 64, GET_VER_FW);

    m_cExtra.AddExtra("VRTCount", "3");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", szDevRPRVer);
    m_cExtra.AddExtra("VRTDetail[02]", szFWVersion);

    // 更新一次状态
    OnStatus();

    Log(ThisModule, 1, "打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());

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
HRESULT CXFS_PTR::InnerPrintForm(LPWFSPTRPRINTFORM pInData)
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
        Log(ThisModule, -1, IDS_ERR_FORM_NOT_FOUND, pInData->lpszFormName);
        return WFS_ERR_PTR_FORMNOTFOUND;
    }
    if (!pc.pForm->IsLoadSucc())
    {
        Log(ThisModule, -1, IDS_ERR_FORM_INVALID, pInData->lpszFormName);
        return WFS_ERR_PTR_FORMINVALID;
    }
    pc.pMedia = pData->FindMedia(pInData->lpszMediaName);
    if (!pc.pMedia)
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_NOT_FOUND, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIANOTFOUND;
    }
    if (!pc.pMedia->IsLoadSucc())
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_INVALID, pInData->lpszMediaName);
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
            Log(ThisModule, -1, IDS_ERR_MEDIA_INVALID, pc.pMedia->GetName());
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
        Log(ThisModule, -1, IDS_ERR_MEDIA_OVERFLOW, pInData->lpszFormName, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIAOVERFLOW;
    }
    RECT rc;
    pc.pMedia->GetPrintArea(rc);
    if (offset.cx < rc.left ||
        (rc.right - rc.left > 0 && sizeForm.cx + offset.cx > rc.right - rc.left) ||
        offset.cy < rc.top ||
        (rc.bottom - rc.top > 0 && sizeForm.cy + offset.cy > rc.bottom - rc.top))
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_OVERFLOW_PRINTAREA, pInData->lpszFormName, pInData->lpszMediaName);
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
    if (m_sConfig.nVerifyField > 0)
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
                    Log(ThisModule, -1, IDS_ERR_FILED_NOT_FOUND, szFieldName);
                    if (m_sConfig.nVerifyField == 1)
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

    //HRESULT hResOld = hRes;
    hRes = EndForm(&pc);

    if (hRes != WFS_SUCCESS || pInData->dwMediaControl == 0)
    {
        UpdateDeviceStatus(WFS_SUCCESS);
        if (m_sStatus.fwPaper[0] == WFS_PTR_PAPERJAMMED)
        {
            Log(ThisModule, -1, IDS_ERR_PAPER_JAMMED);
            return WFS_ERR_PTR_PAPERJAMMED;
        }
    }

    return /*WFS_SUCCESS != hResOld ? hResOld : */hRes;
}

// ReadForm命令子处理
HRESULT CXFS_PTR::InnerReadForm(LPWFSPTRREADFORM pInData)
{
    return WFS_ERR_UNSUPP_COMMAND;  // 暂不支持
}

// MediaControl介质控制子处理
HRESULT CXFS_PTR::ControlMedia(DWORD dwControl)
{
    THISMODULE(__FUNCTION__);

    MEDIA_ACTION emMediaAct = MEDIA_CTR_ATPBACKWARD;    // 缺省黑标纸

    if (m_sStatus.fwDevice != WFS_PTR_DEVONLINE &&			//30-00-00-00（FT#0008）
        m_sStatus.fwDevice != WFS_PTR_DEVBUSY)				//30-00-00-00（FT#0008）
    {														//30-00-00-00（FT#0008）
        return WFS_ERR_HARDWARE_ERROR;						//30-00-00-00（FT#0008）
    }														//30-00-00-00（FT#0008）

    if (m_sStatus.fwPaper[0] == WFS_PTR_PAPEROUT)			//30-00-00-00（FT#0008）
    {														//30-00-00-00（FT#0008）
        return WFS_ERR_PTR_PAPEROUT;						//30-00-00-00（FT#0008）
    }
    if ((WFS_PTR_CTRLCUT & dwControl) || (WFS_PTR_CTRLEJECT &  dwControl)) // 切纸
    {

        if (m_sConfig.bDetectBlackStripe == 1)  // 黑标纸
        {
            emMediaAct = MEDIA_CTR_ATPFORWARD;
        } else
        if (m_sConfig.bDetectBlackStripe == 0)  // 连续纸
        {
            emMediaAct = MEDIA_CTR_ATPBACKWARD;
        }
        int nRet = m_pPrinter->MeidaControl(emMediaAct, m_sConfig.nFeed);
        usleep(500 * 1000); // 休止0.5秒，确保获取状态准确
        UpdateDeviceStatus(nRet);
        if (nRet != PTR_SUCCESS)
        {
            if (nRet == ERR_PTR_NO_PAPER)
            {
                Log(ThisModule, -1, IDS_ERR_NO_PAPER_WHENCUT);
                return WFS_ERR_PTR_PAPEROUT;
            }
            else if (ERR_PTR_JAMMED == nRet)
            {
                m_bNeedKeepJammedStatus = TRUE;
                Log(ThisModule, -1, IDS_ERR_PAPER_JAMMED);
                return WFS_ERR_PTR_PAPERJAMMED;
            }
            else
            {
                Log(ThisModule, -1, IDS_ERR_CUTPAPER_ERROR, \
                    m_sConfig.bDetectBlackStripe, m_sConfig.nFeed);
                return WFS_ERR_HARDWARE_ERROR;
            }
        } else
        {
            UpdateDeviceStatus(nRet);
            if (m_sStatus.fwPaper[0] == WFS_PTR_PAPEROUT)
            {
                Log(ThisModule, -1, IDS_ERR_NO_PAPER_WHENCUT);
                return WFS_ERR_PTR_NOMEDIAPRESENT;
            } else
            if (m_sStatus.fwPaper[0] == WFS_PTR_PAPERJAMMED)
            {
                Log(ThisModule, -1, IDS_ERR_PAPER_JAMMED);
                return WFS_ERR_PTR_PAPERJAMMED;
            }
        }

        Log(ThisModule, 0, IDS_INFO_CUTPAPER_SUCCESS);
        m_WaitTaken = WTF_TAKEN;

        if (m_sConfig.nDriverType == DEV_SNBC_BTNH80 && m_sConfig.nTakeSleep > 0 &&
            m_WaitTaken == WTF_TAKEN)
        {
            dwTakeTimeSize = time(NULL);
        }

        return WFS_SUCCESS;
    } else
    if (WFS_PTR_CTRLFLUSH == dwControl)
    {
        if (m_sStatus.fwDevice != WFS_PTR_DEVONLINE)
        {
            Log(ThisModule, -1, IDS_ERR_DEVIVE_STA,  m_sStatus.fwDevice);
            return WFS_ERR_HARDWARE_ERROR;
        } else
        if (m_sStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT)
        {
            return WFS_ERR_PTR_PAPEROUT;
        } else
        {
            return WFS_SUCCESS;
        }
    } else
    {
        Log(ThisModule, -1, IDS_ERR_RPR_UNSUPP_COMMAND, dwControl);
        return WFS_ERR_UNSUPP_DATA;
    }
}

// RawData无格式打印命令子处理
HRESULT CXFS_PTR::SendRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRes = WFS_SUCCESS;

    if (0 == nSize)
    {
        return WFS_SUCCESS;
    }

    if (m_sConfig.nRawDataInPar == 0)                       // 30-00-00-00(FT#0045) RawData入参模式:0/UTF8
    {                                                       // 30-00-00-00(FT#0045)
        // 转码(UTF8->GBK)
        QTextCodec *codec = QTextCodec::codecForLocale();
        QTextCodec *codec1 = QTextCodec::codecForName("gb18030");
        if (nullptr == codec1) codec1 = QTextCodec::codecForName("gb2312");
        if (nullptr == codec1) codec1 = QTextCodec::codecForName("gbk");
        if (nullptr == codec1)
        {
            Log(ThisModule, -1, IDS_ERR_PRINTSTRING_FAILD);
            return WFS_ERR_PTR_CHARSETDATA;
        }
        QString strText = QString::fromUtf8((char *)pData, nSize);
        QTextCodec::setCodecForLocale(codec1);
        QByteArray tmpData = strText.toLocal8Bit();
        char *pTempCode = tmpData.data();
        int nTempSize = tmpData.size();
        ULONG ulDataSize = 0;
        int nBufferSize = nTempSize + 2 + (nTempSize / m_sConfig.nLineSize) * 2;    // 30-00-00-00(FT#0008)
        BYTE *pBuf = new BYTE[/*nTempSize + 2*/nBufferSize];                        // 30-00-00-00(FT#0008)
        memset(pBuf, 0, /*nTempSize + 2*/nBufferSize);                              // 30-00-00-00(FT#0008)
        RemoveUnPrintableChar(nTempSize, (LPBYTE)pTempCode, ulDataSize, pBuf);
        QTextCodec::setCodecForLocale(codec);

        // 去除不可打印字符
        if ('\n' != pBuf[ulDataSize - 1])
        {
            pBuf[ulDataSize++] = '\n';
        }
        pBuf[ulDataSize] = 0;

        hRes = PrintString((char *)pBuf, ulDataSize, FALSE);
        delete [] pBuf;
        pBuf = nullptr;
    } else                                                  // 30-00-00-00(FT#0045)RawData入参模式:非0/GBK
    {                                                       // 30-00-00-00(FT#0045)
        hRes = PrintString((char *)pData, nSize, FALSE);    // 30-00-00-00(FT#0045)
    }                                                       // 30-00-00-00(FT#0045)

    if (WFS_SUCCESS != hRes)
    {
        Log(ThisModule, -1, IDS_ERR_PRINTSTRING_FAILD, hRes);
        return hRes;
    }

    if (bExpectResp)
    {
        //SetInputRawData(0, nullptr);
        m_pData->m_InputRawData.SetData(0, nullptr);
    }

    return WFS_SUCCESS;
}

// 设备复位命令子处理
HRESULT CXFS_PTR::ResetDevice(DWORD dwMediaControl, USHORT usBinIndex)
{
    Q_UNUSED(usBinIndex);
    THISMODULE(__FUNCTION__);
    Log(ThisModule, 1, IDS_INFO_RESET_DEVICE);

    int nRet = m_pPrinter->Init();
    UpdateDeviceStatus(nRet);
    if (nRet != PTR_SUCCESS)
    {
        //if (m_sStatus.fwPaper[0] == WFS_ERR_PTR_PAPEROUT)   // 无纸
        if (nRet == ERR_PTR_NO_PAPER)
        {
            m_bNeedKeepJammedStatus = FALSE;
            Log(ThisModule, 1, IDS_INFO_RESET_INFO, nRet, dwMediaControl);
            return WFS_SUCCESS;
        }

        Log(ThisModule, -1, IDS_ERR_REESET_ERROR, nRet);
        return WFS_ERR_HARDWARE_ERROR;

    }
    m_bReset = TRUE;

    m_bNeedKeepJammedStatus = FALSE;
    Log(ThisModule, 1, IDS_INFO_RESET_SUCCESS, nRet, dwMediaControl);
    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//-----------------------------------重载接口-------------------------------
//
HRESULT CXFS_PTR::OnInit()
{
    THISMODULE(__FUNCTION__);

    if (m_sConfig.type == PTR_TYPE_RECEIPT)
    {
        //m_pPrinter = new CReceiptPrinter;
    }

    long nRet = m_pPrinter->Init();
    UpdateDeviceStatus(nRet);
    if (nRet < 0)
    {
        return ConvertErrCode(nRet);
    }

    return ConvertErrCode(nRet);
}

//
HRESULT CXFS_PTR::OnExit()
{
    if (nullptr != m_pPrinter)
    {
        m_pPrinter->Close();
    }

    return WFS_SUCCESS;
}

// PrintForm最终处理
HRESULT CXFS_PTR::EndForm(PrintContext *pContext)
{
    if (m_sConfig.nDriverType == DEV_SNBC_BTNH80 ||
        m_sConfig.nDriverType == DEV_SNBC_BKC310)
    {
        return EndForm_SNBC(pContext);
    }
}


//-----------------------------------------------------------------------------------
//--------------------------------------功能处理接口-----------------------------------
// 加载DevXXX动态库
bool CXFS_PTR::LoadDevDll(LPCSTR ThisModule)
{
    if (m_pPrinter == nullptr)
    {
        if (m_pPrinter.Load(m_sConfig.szDevDllName, "CreateIDevPTR", DEVTYPE2STR(m_sConfig.nDriverType)) != 0)
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s, DriverType=%d|%s, ERR:%s",
                m_sConfig.szDevDllName, m_sConfig.nDriverType,
                DEVTYPE2STR(m_sConfig.nDriverType), m_pPrinter.LastError().toUtf8().constData());
            return false;
        } else
        {
            Log(ThisModule, __LINE__, "加载库: DriverDllName=%s, DriverType=%d|%s, Succ.",
                m_sConfig.szDevDllName, m_sConfig.nDriverType, DEVTYPE2STR(m_sConfig.nDriverType));
        }
    }
    return (m_pPrinter != nullptr);
}

// 加载INI设置
void CXFS_PTR::InitConifig()
{
    THISMODULE(__FUNCTION__);

    strcpy(m_sConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));
    m_sConfig.nDriverType = m_cXfsReg.GetValue("DriverType", DEV_SNBC_BKC310);


    m_sConfig.type  = (PTR_TYPE)m_cXfsReg.GetValue("CONFIG", "type", 1);
    // 纸类型
    m_sConfig.bDetectBlackStripe = m_cXfsReg.GetValue("CONFIG", "feedblcakdetect", 1) != 0;
    // 走纸距离
    m_sConfig.nFeed = (int)m_cXfsReg.GetValue("CONFIG", "feedsize", (DWORD)0);
    m_sConfig.dwMarkHeader = m_cXfsReg.GetValue("CONFIG", "MarkHeader", 318);

    m_sConfig.nVerifyField = m_cXfsReg.GetValue("CONFIG", "verify_field", (DWORD)0);
    if (m_sConfig.nVerifyField < 0 || m_sConfig.nVerifyField > 2)
        m_sConfig.nVerifyField = 0;

    m_sConfig.nPageSize = m_cXfsReg.GetValue("CONFIG", "split_size", 2976);
    m_sConfig.nPageLine = m_cXfsReg.GetValue("CONFIG", "page_lines", 30);
    m_sConfig.nLineSize = m_cXfsReg.GetValue("CONFIG", "line_size", 50);    // 30-00-00-00(FT#0008)
    m_sConfig.bEnableSplit = m_cXfsReg.GetValue("CONFIG", "enabled_split", 1) != 0;


    LPCSTR lpFont = m_cXfsReg.GetValue("CONFIG", "FontType", "");
    strcpy(m_stPrintFormat.szFontType, lpFont);
    Log(ThisModule, 1, "加载FontType%s", m_stPrintFormat.szFontType);
    m_stPrintFormat.uFontSize = m_cXfsReg.GetValue("CONFIG", "FontSize", (DWORD)0);
    m_stPrintFormat.ulStyle = m_cXfsReg.GetValue("CONFIG", "Style", (DWORD)0);
    m_stPrintFormat.uWPI = m_cXfsReg.GetValue("CONFIG", "WPI", (DWORD)0);
    m_stPrintFormat.uLPI = m_cXfsReg.GetValue("CONFIG", "LPI", (DWORD)0);
    m_stPrintFormat.uLineHeight = m_cXfsReg.GetValue("CONFIG", "LineHeight", (DWORD)0);
    Log(ThisModule, 1, "Style%d", m_stPrintFormat.ulStyle);

    if (49 > m_sConfig.nPageSize)
    {
        m_sConfig.nPageSize = 50;
    }
    if (2976 < m_sConfig.nPageSize)
    {
        m_sConfig.nPageSize = 2976;
    }

    if (1 > m_sConfig.nPageLine)
    {
        m_sConfig.nPageLine = 1;
    }
    if (44 < m_sConfig.nPageLine)
    {
        m_sConfig.nPageLine = 44;
    }

    if (m_sConfig.nLineSize < 1 || m_sConfig.nLineSize > 50)    // 30-00-00-00(FT#0008)
    {                                                           // 30-00-00-00(FT#0008)
        m_sConfig.nLineSize = 50;                               // 30-00-00-00(FT#0008)
    }                                                           // 30-00-00-00(FT#0008)

    m_sConfig.nTakeSleep = m_cXfsReg.GetValue("TakeCfg", "TakeSleep", (DWORD)3);

    // RawData入参模式: 0/UTF8;1GBK,缺省0                                             // 30-00-00-00(FT#0045)
    m_sConfig.nRawDataInPar = m_cXfsReg.GetValue("RawData", "InParMode", (INT)0);   // 30-00-00-00(FT#0045)
    if (m_sConfig.nRawDataInPar != 0 && m_sConfig.nRawDataInPar != 1)               // 30-00-00-00(FT#0045)
    {                                                                               // 30-00-00-00(FT#0045)
        m_sConfig.nRawDataInPar = 0;                                                // 30-00-00-00(FT#0045)
    }                                                                               // 30-00-00-00(FT#0045)
}

// 更新状态(暂时不使用)
long CXFS_PTR::UpdateStatus()
{
    return 0;
}

// 初始化Cen标准状态类
long CXFS_PTR::InitStatus()
{
    memset(&m_sStatus, 0x00, sizeof(WFSPTRSTATUS));
    m_sStatus.fwDevice      = WFS_PTR_DEVNODEVICE;
    m_sStatus.fwPaper[0]    = WFS_PTR_PAPERUNKNOWN;
    m_sStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
    m_sStatus.fwToner       = WFS_PTR_TONERFULL;
    m_sStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
    m_sStatus.fwInk         = WFS_PTR_INKNOTSUPP;
}

// DevXXX动态库错误码 转换为 Cen标准错误码
long CXFS_PTR::ConvertErrCode(long lRes)
{
    switch (lRes)
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

// 调用DevXXX动态库的文本打印接口
HRESULT CXFS_PTR::PrintData(const char *pBuffer, DWORD dwSize)
{
    THISMODULE(__FUNCTION__);

    int iRet = m_pPrinter->PrintData(pBuffer, dwSize);
    UpdateDeviceStatus(iRet);
    if (iRet)
    {
        Log(ThisModule, -1, "PrintData fail, ErrCode:%d", iRet);
    }
    return ConvertErrCode(iRet);
}

// 调用DevXXX动态库的图片打印接口1
HRESULT CXFS_PTR::PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight)
{
    THISMODULE(__FUNCTION__);

    int nRet = m_pPrinter->PrintImage(szImagePath, nDstWidth, nDstHeight);
    UpdateDeviceStatus(nRet);
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, nRet, "打印图片错误: %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

// 调用DevXXX动态库的图片打印接口2
HRESULT CXFS_PTR::PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY)
{
    THISMODULE(__FUNCTION__);

    int nRet = m_pPrinter->PrintImageOrg(szImagePath, ulOrgX, ulOrgY);
    UpdateDeviceStatus(nRet);
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, nRet, "打印图片错误: %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

// 打印前数据处理
HRESULT CXFS_PTR::AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut)
{
    THISMODULE(__FUNCTION__);

    if (0 == dwSize)
    {
        return WFS_SUCCESS;
    }

    int iCurPos = 0, iLenToEnter = 0, iEnterNum = 0, iLastSize = 0;
    const char *pp = pBuffer;
    int PAGESIZE = m_sConfig.nPageSize;
    int PAGELINE = m_sConfig.nPageLine;
    // FORM打印时根据数据量判断是否需要换行
    bool bNeedCR = true;
    if (dwSize < (DWORD)PAGESIZE && FALSE != bIsFromPrint)
    {
        bNeedCR = false;
    }

    int nRet = PTR_SUCCESS;
    if (m_sConfig.bEnableSplit)
    {
        while (0 < dwSize)
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
            if ('\n' != cLastChar  && bNeedCR)
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
                Log(ThisModule, -1, "PrintData(%u) is more than MAXLEN", dwSizeOut + iCurPos);
            }
            //nRet = m_pPrinter->PrintData(const_cast<char *>( pp ), iCurPos );

            if (bChanged)
            {
                ((char *)pp)[ iCurPos ] = cBackup;
                --iCurPos;
            }
            bChanged = false;

            if (nRet = PTR_SUCCESS)
            {
                Log(ThisModule, nRet, IDS_ERR_PRINT_FAILD, nRet);
                //缺纸时打印应返回WFS_ERR_PTR_PAPEROUT
                if (ERR_PTR_NO_PAPER == nRet)
                {
                    return WFS_ERR_PTR_PAPEROUT;
                }
                else
                {
                    return WFS_ERR_HARDWARE_ERROR;
                }
            }
            pp = pp + iCurPos;
            dwSize -= iCurPos;
            iCurPos = 0;
            iEnterNum = 0;
            iLenToEnter = 0;
        }
    }
    else
    {
        //nRet = m_pPrinter->PrintData(const_cast<char *>(pBuffer), dwSize);
        memcpy(pBuffOut + dwSizeOut, pp, iCurPos);
        dwSizeOut += iCurPos;
        if (nRet = PTR_SUCCESS)
        {
            Log(ThisModule, nRet, IDS_ERR_PRINT_FAILD, nRet);
            // 缺纸时打印应返回WFS_ERR_PTR_PAPEROUT
            if (ERR_PTR_NO_PAPER == nRet)
            {
                return WFS_ERR_PTR_PAPEROUT;
            }
            else
            {
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
    }

    return WFS_SUCCESS;
}

// 打印前数据处理
HRESULT CXFS_PTR::PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint)
{
    THISMODULE(__FUNCTION__);

    if (0 == dwSize)
    {
        return WFS_SUCCESS;
    }

    int iCurPos = 0, iLenToEnter = 0, iEnterNum = 0, iLastSize = 0;
    const char *pp = pBuffer;
    int PAGESIZE = m_sConfig.nPageSize;
    int PAGELINE = m_sConfig.nPageLine;
    // FORM打印时根据数据量判断是否需要换行
    bool bNeedCR = true;
    if (dwSize < (DWORD)PAGESIZE && FALSE != bIsFromPrint)
    {
        bNeedCR = false;
    }

    int nRet = PTR_SUCCESS;
    if (m_sConfig.bEnableSplit)
    {
        while (0 < dwSize)
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
            if ('\n' != cLastChar && bNeedCR)
            {
                cBackup = ((char *)pp)[ iCurPos ];
                ((char *)pp)[ iCurPos ] = '\n';
                iCurPos++;
                bChanged = true;
            }

            if (bIsFromPrint == false)                  // 30-00-00-00(FT#00008)
            {                                           // 30-00-00-00(FT#00008)
                if (dwSize > 0 && cLastChar == '\n')    // 30-00-00-00(FT#00008)
                {                                       // 30-00-00-00(FT#00008)
                    ((char *)pp)[ iCurPos - 1 ] = '\0'; // 30-00-00-00(FT#00008)
                }                                       // 30-00-00-00(FT#00008)
            }                                           // 30-00-00-00(FT#00008)

            nRet = m_pPrinter->PrintData(const_cast<char *>(pp), iCurPos);

            if (bChanged)
            {
                ((char *)pp)[ iCurPos ] = cBackup;
                --iCurPos;
            }
            bChanged = false;

            if (nRet = PTR_SUCCESS)
            {
                break;

                Log(ThisModule, nRet, IDS_ERR_PRINT_FAILD, nRet);
            }
            pp = pp + iCurPos;
            dwSize -= iCurPos;
            iCurPos = 0;
            iEnterNum = 0;
            iLenToEnter = 0;
        }
    }
    else
    {
        nRet = m_pPrinter->PrintData(const_cast<char *>(pBuffer), dwSize);
    }
    UpdateDeviceStatus(nRet);
    return ConvertErrCode(nRet);
}

void CXFS_PTR::RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData)
{
    ulOutSize = 0;
    int PAGELINE = m_sConfig.nLineSize;                                 // 一行字符数　30-00-00-00(FT#0008)
    int nLineCnt = 0;                                                   // 一行字符数计数　30-00-00-00(FT#0008)
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
            if ((nLineCnt % PAGELINE == 0 && nLineCnt != 0) ||  // 30-00-00-00(FT#0008)
               (nLineCnt % PAGELINE + 1) == PAGELINE)           // 30-00-00-00(FT#0008)
            {                                                   // 30-00-00-00(FT#0008)
                pOutData[ulOutSize++] = '\n';                   // 30-00-00-00(FT#0008)
                nLineCnt = 0;                                   // 30-00-00-00(FT#0008)
            }                                                   // 30-00-00-00(FT#0008)
            pOutData[ulOutSize++] = pInData[i];
            pOutData[ulOutSize++] = pInData[i + 1];
            i++;
            nLineCnt = nLineCnt + 2;                            // 30-00-00-00(FT#0008)
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
            if ((nLineCnt % PAGELINE == 0 && nLineCnt != 0) ||          // 30-00-00-00(FT#0008)
               (nLineCnt % PAGELINE + 1) == PAGELINE)                   // 30-00-00-00(FT#0008)
            {                                                           // 30-00-00-00(FT#0008)
                pOutData[ulOutSize++] = '\n';                           // 30-00-00-00(FT#0008)
                nLineCnt = 0;                                           // 30-00-00-00(FT#0008)
            }                                                           // 30-00-00-00(FT#0008)
            pOutData[ulOutSize++] = pInData[i];
            pOutData[ulOutSize++] = pInData[i + 1];
            i++;
            nLineCnt = nLineCnt + 2;                                    // 30-00-00-00(FT#0008)
        }
        else if (pInData[i] >= 0x80)
        {
            i++;
        }
        else if ((pInData[i] == 0x0A)
                 || ((pInData[i] > 0x1F) && (pInData[i] < 0x7F)))
        {
            if (nLineCnt % PAGELINE == 0 && nLineCnt != 0)              // 30-00-00-00(FT#0008)
            {                                                           // 30-00-00-00(FT#0008)
                pOutData[ulOutSize++] = '\n';                           // 30-00-00-00(FT#0008)
                nLineCnt = 0;                                           // 30-00-00-00(FT#0008)
            }                                                           // 30-00-00-00(FT#0008)
            if (pInData[i] == '\n')                                     // 30-00-00-00(FT#0008)
            {                                                           // 30-00-00-00(FT#0008)
                nLineCnt = 0;                                           // 30-00-00-00(FT#0008)
            }                                                           // 30-00-00-00(FT#0008)
            pOutData[ulOutSize++] = pInData[i];
            nLineCnt = nLineCnt + 1;                                    // 30-00-00-00(FT#0008)
        }
    }
}

// 设备状态实时更新
void CXFS_PTR::UpdateDeviceStatus(int nRet)
{
    const char* ThisModule = "UpdateDeviceStatus";
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题
    //int     nPrinterStatus  = WFS_PTR_DEVHWERROR;

    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFirePaperStatus    = FALSE;
    BOOL    bNeedFireTonerStatus    = FALSE;
    BOOL    bNeedFireHWError        = FALSE;
    BOOL    bNeedFirePaperTaken     = FALSE;

    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log

    DEVPTRSTATUS stDevStatus;
    nRet = m_pPrinter->GetStatus(stDevStatus);

    m_sStatus.fwInk                  = WFS_PTR_INKNOTSUPP;
    m_sStatus.fwLamp                 = WFS_PTR_LAMPNOTSUPP;
    m_sStatus.lppRetractBins         = nullptr;
    m_sStatus.usMediaOnStacker       = 0;
    m_sStatus.lpszExtra              = nullptr;


    WFSPTRSTATUS sLastStatus = m_sStatus;
    m_bNeedReset = false;
    if (!m_bNeedReset)
    {
        // 翻译设备状态
        switch (nRet)
        {
            // 打印机出纸口有纸设备状态为ONLINE
            case PTR_SUCCESS:
            case ERR_PTR_PARAM_ERR:
            case ERR_PTR_UNSUP_CMD:
                m_sStatus.fwDevice = WFS_PTR_DEVONLINE;
                //nPrinterStatus = WFS_PTR_DEVONLINE;
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
    }
    if (m_sStatus.fwDevice == WFS_PTR_DEVOFFLINE)
    {
        m_sStatus.fwMedia = WFS_PTR_MEDIAUNKNOWN;
        m_sStatus.fwPaper[0] = WFS_PTR_PAPERUNKNOWN;
    }

    if (m_sStatus.fwDevice != sLastStatus.fwDevice)
    {
        bNeedFireStatusChanged = TRUE;
        //if (nPrinterStatus == WFS_PTR_DEVHWERROR)
        if (m_sStatus.fwDevice == WFS_PTR_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }

    // **********************介质状态****************************
    if (m_sConfig.nDriverType == DEV_SNBC_BTNH80 && m_sConfig.nTakeSleep > 0 &&
         m_WaitTaken == WTF_TAKEN)
    {
        DWORD dwSize = time(NULL);
        if (dwSize - dwTakeTimeSize > m_sConfig.nTakeSleep)
        {
            bNeedFirePaperTaken = TRUE;
            m_WaitTaken = WTF_NONE;
        }
    } else
    {
        int nMediaStatus = ConvertMediaStatus(stDevStatus.wMedia);
        if (m_sStatus.fwMedia != nMediaStatus)
        {
            if ((m_sStatus.fwMedia == WFS_PTR_MEDIAENTERING) &&
                (nMediaStatus == WFS_PTR_MEDIANOTPRESENT || nMediaStatus == WFS_PTR_MEDIAPRESENT))
            {
                bNeedFirePaperTaken = TRUE;
                m_WaitTaken = WTF_NONE;
            }
            m_sStatus.fwMedia = nMediaStatus;
        }
    }

    if (m_sStatus.fwMedia != WFS_PTR_MEDIAENTERING)
    {
        m_sStatus.fwMedia = ConvertPaper2MediaStatus(stDevStatus.wPaper[0]);
    }

    if (m_sStatus.fwMedia == WFS_PTR_MEDIAJAMMED)   // 30-00-00-00(FT#0008)
    {                                               // 30-00-00-00(FT#0008)
        m_sStatus.fwDevice = WFS_PTR_DEVHWERROR;    // 30-00-00-00(FT#0008)
        if (m_sStatus.fwDevice != sLastStatus.fwDevice)     // 40-00-00-00(FT#0007)
        {                                                   // 40-00-00-00(FT#0007)
            bNeedFireStatusChanged = TRUE;                  // 40-00-00-00(FT#0007)
            if (m_sStatus.fwDevice == WFS_PTR_DEVHWERROR)   // 40-00-00-00(FT#0007)
            {                                               // 40-00-00-00(FT#0007)
                bNeedFireHWError = TRUE;                    // 40-00-00-00(FT#0007)
            }                                               // 40-00-00-00(FT#0007)
        }                                                   // 40-00-00-00(FT#0007)
    }                                               // 30-00-00-00(FT#0008)

    // **********************纸状态****************************
    int nPaperStatus = ConvertPaperStatus(stDevStatus.wPaper[0]);

    if (m_sStatus.fwPaper[0] != (WORD)nPaperStatus)
    {
        m_sStatus.fwPaper[0] = (WORD)nPaperStatus;
        //只有当纸状态变为少或空时才Fire状态
        //if (WFS_PTR_PAPERLOW == nPaperStatus || WFS_PTR_PAPEROUT == nPaperStatus)
        //{
        if (sLastStatus.fwPaper[0] !=  m_sStatus.fwPaper[0])// 40-00-00-00(FT#0007)
        {
            if (m_sStatus.fwPaper[0] == WFS_PTR_PAPERLOW || // 40-00-00-00(FT#0007)
                m_sStatus.fwPaper[0] == WFS_PTR_PAPEROUT || // 40-00-00-00(FT#0007)
                m_sStatus.fwPaper[0] == WFS_PTR_PAPERFULL)  // 40-00-00-00(FT#0007)
            {                                               // 40-00-00-00(FT#0007)
                bNeedFirePaperStatus = TRUE;
            }                                               // 40-00-00-00(FT#0007)
        }
        //}
    }
    for (int i = 1; i < WFS_PTR_SUPPLYSIZE; i++)
    {
        m_sStatus.fwPaper[i] = WFS_PTR_PAPERNOTSUPP;
    }

    // **********************色带状态****************************
    WORD nTonerStatus = ConvertTonerStatus(stDevStatus.wToner);
    if (m_sStatus.fwToner != (WORD)nTonerStatus)
    {
        //只有当Toner状态变为少或空时才Fire状态
        if (nTonerStatus == WFS_PTR_TONERLOW || nTonerStatus == WFS_PTR_TONEROUT)
        {
            bNeedFireTonerStatus = TRUE;
        }
        m_sStatus.fwToner = (WORD)nTonerStatus;
    }

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

    if (bNeedFirePaperTaken)
    {
        FireMediaTaken();
        Log(ThisModule, 1, IDS_INFO_PAPER_TAKEN);
        sprintf(szFireBuffer + strlen(szFireBuffer), "PaperTaken:|");
    }

    // 比较两次状态记录LOG
    if (memcmp(&sLastStatus, &m_sStatus, sizeof(WFSPTRSTATUS)) != 0)
    {
        Log(ThisModule, -1, "状态结果比较: Device:%d->%d%s|Media:%d->%d%s|Paper[0]:%d->%d%s|"
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

// BK-C310/BT-NH80M 打印数据下发
HRESULT CXFS_PTR::EndForm_SNBC(PrintContext *pContext)
{
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
        // 增加图像打印
        // 图片打印
        if (FT_GRAPHIC == pItem->nFieldType)
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

    if (('\n' == szPrintData[(dwDataSize > 0) ? dwDataSize - 1 : 0]) &&
        (dwDataSize < MAX_PRINTDATA_LEN))
    {
        szPrintData[dwDataSize] = '\n';
    }

    nRet = PrintData(szPrintData, dwDataSize);

    if (0 > nRet)
    {
        return nRet;
    }

    // 控制MEDIA
    int nResult = 0;
    DWORD dwMediaControl = pContext->pPrintData->dwMediaControl;
    if (0 != dwMediaControl)
    {
        nResult = ControlMedia(dwMediaControl);
    }

    return nResult;
}

// HOTS 打印数据下发
HRESULT CXFS_PTR::EndForm_HOTS(PrintContext *pContext)
{
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
        // 增加图像打印
        // 图片打印
        if (FT_GRAPHIC == pItem->nFieldType)
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

            s.m_nCurCol += pItem->nWidth;   // 空格占位:打印格式数据(空格和字符)，定位图片文件位置
            nRet = PrintString(strFormat.GetData(), strFormat.GetLen());
            nRet = PrintImage(pItem->strImagePath, pItem->nDstImgWidth, pItem->nDstImgHeight);

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
                //nRet = PrintString(strText.GetData(), strText.GetLen(), TRUE);
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

    if (('\n' == szPrintData[(dwDataSize > 0) ? dwDataSize - 1 : 0]) &&
        (dwDataSize < MAX_PRINTDATA_LEN))
    {
        szPrintData[dwDataSize] = '\n';
    }

    nRet = PrintData(szPrintData, dwDataSize);

    if (0 > nRet)
    {
        return nRet;
    }

    // 控制MEDIA
    int nResult = 0;
    DWORD dwMediaControl = pContext->pPrintData->dwMediaControl;
    if (0 != dwMediaControl)
    {
        nResult = ControlMedia(dwMediaControl);
    }

    return nResult;
}
