/***************************************************************
* 文件名称：XFS_PPR_DEC.cpp
* 文件描述：存折打印模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年11月6日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_PPR.h"
#include "file_access.h"
#include "data_convertor.h"

//-----------------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_PPR::StartOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    CHAR szFWVersion[64] = { 0x00 };
    LONG lFWVerSize = 0;

    if (m_stConfig.nDeviceType != DEV_MB2 &&
        m_stConfig.nDeviceType != DEV_PRM)
    {
        Log(ThisModule, __LINE__, "Open fail, INI指定了不支持的设备类型[%d], Return :%d.",
            m_stConfig.nDeviceType, WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 加载DevPPR动态库
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

        if (m_stConfig.nDeviceType == DEV_PRM)
        {
            m_pPrinter->SetData(&(m_stConfig.stConfig_PRM), SET_DEV_PARAM);
        }
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
HRESULT CXFS_PPR::SetInit()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// 等待介质放入
HRESULT CXFS_PPR::MediaInsertWait(DWORD &dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    INT nMediaWait = MEDINS_OK;             // 等待放折结果

    m_pPrinter->SetData(&(m_stConfig.stConfig_Beep), SET_DEV_BEEP);     // 设备鸣响设置

    // 南天MB2/中航PRM采用通用处理模式标准
    //if (m_stConfig.nDeviceType == DEV_MB2 ||
    //    m_stConfig.nDeviceType == DEV_PRM)
    //{
        nMediaWait = MediaInsertWait_Pub(dwTimeOut);
    //}
    if (nMediaWait != MEDINS_OK)
    {
        // 取消
        if (nMediaWait == MEDINS_CANCEL)
        {
            return WFS_ERR_CANCELED;
        }
        // 介质JAM
        if (nMediaWait == MEDINS_JAM)
        {
            return WFS_ERR_PTR_MEDIAJAMMED;
        }
        // 放折超时
        if (nMediaWait == MEDINS_TIMEOUT)
        {
            return WFS_ERR_TIMEOUT;
        }
        // 断线
        if (nMediaWait == MEDINS_OFFLINE)
        {
            m_wNeedFireEvent[0] = 1;
            return WFS_ERR_HARDWARE_ERROR;
        }
        // 故障
        if (nMediaWait == MEDINS_HWERR)
        {            
            m_wNeedFireEvent[0] = 1;
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    // 介质放入,上报Insert事件
    Log(ThisModule, __LINE__, "介质插入,上报MediaInsert.");
    FireMediaInserted();

    return WFS_SUCCESS;
}

// 等待介质放入
// 返回: 参考[介质放入等待处理 宏定义]
INT CXFS_PPR::MediaInsertWait_Pub(DWORD &dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    BOOL bMediaExist = FALSE;       // 介质是否就位
    BOOL bMediaJAM = FALSE;         // 介质JAM
    INT nRetMediaInsert = PTR_SUCCESS;

    DWORD dwTimeOutTmp = dwTimeOut; // 超时毫秒数
    qint64 dwLastTime = QDateTime::currentMSecsSinceEpoch();  // 超时计数:起始时间
    qint64 dwLocalTime = QDateTime::currentMSecsSinceEpoch(); // 超时计数:当前时间

    while (bMediaExist != TRUE && m_bCmdCanceled != TRUE && bMediaJAM != TRUE)
    {
        dwLocalTime = QDateTime::currentMSecsSinceEpoch(); // 取当前时间
        if (dwLocalTime - dwLastTime >= dwTimeOutTmp)
        {
            Log(ThisModule, __LINE__, "等待介质插入超时.");
            return MEDINS_TIMEOUT;   // 超时
        }

        //m_pPrinter->MeidaControl(MEDIA_CTR_PARTIALCUT, 500);    // 介质吸入,超时500毫秒
        UpdateDeviceStatus();       // 更新当前设备状态
        if (m_stStatus.fwDevice != WFS_PTR_DEVONLINE)   // 设备处于非ONLINE状态
        {
            switch(m_stStatus.fwDevice)
            {
                case WFS_PTR_DEVOFFLINE:
                case WFS_PTR_DEVNODEVICE:
                case WFS_PTR_DEVPOWEROFF:
                {
                    Log(ThisModule, __LINE__, "介质插入等待中设备断线.");
                    return MEDINS_OFFLINE;
                }
                case WFS_PTR_DEVHWERROR:
                case WFS_PTR_DEVUSERERROR:
                {
                    if (m_stStatus.fwMedia != WFS_PTR_MEDIAJAMMED)
                    {
                        Log(ThisModule, __LINE__, "介质插入等待中设备故障.");
                        return MEDINS_HWERR;
                    }
                }
            }
        }

        switch(m_stStatus.fwMedia)    // 检查Media状态
        {
            case WFS_PTR_MEDIANOTPRESENT:   // 入口及打印位置无介质(休眠500毫秒)
            case WFS_PTR_MEDIAUNKNOWN:      // 介质状态未知
            case WFS_PTR_MEDIANOTSUPP:      // 不支持获取介质状态
            case WFS_PTR_MEDIARETRACTED:    // 介质被复位回收
                //usleep(1000 * 500);
                //dwTimeOutCount = dwTimeOutCount + 500;
                //dwTimeOut = (dwTimeOut - 500 < 0 ? 0 : dwTimeOut - 500);
                break;
            case WFS_PTR_MEDIAENTERING:     // 介质在出口,介质吸入
                nRetMediaInsert = m_pPrinter->MeidaControl(MEDIA_CTR_PARTIALCUT, 500);    // 介质吸入,超时500毫秒
                if (nRetMediaInsert != PTR_SUCCESS)
                {                    
                    UpdateDeviceStatus();
                    if (m_stStatus.fwMedia == WFS_PTR_MEDIAJAMMED)
                    {
                        Log(ThisModule, __LINE__, "介质JAM.");
                        bMediaJAM = TRUE;
                        break;
                    } else
                    if (m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT)
                    {
                        bMediaExist = TRUE;         // 设置介质就位
                        break;
                    }
                    Log(ThisModule, __LINE__, "介质在出口,介质吸入失败, MeidaControl(%d) RetCode : %d.",
                         MEDIA_CTR_PARTIALCUT, nRetMediaInsert);
                    return MEDINS_HWERR;
                }
                break;
            case WFS_PTR_MEDIAPRESENT:      // 介质就位
            //case WFS_PTR_MEDIAENTERING:     // 介质在出口
                bMediaExist = TRUE;         // 设置介质就位
                break;
            case WFS_PTR_MEDIAJAMMED:       // 介质JAM
                bMediaJAM = TRUE;           // 设置介质JAM
                break;
        }
    }

    // 放入取消
    if (m_bCmdCanceled == TRUE)
    {
        m_bCmdCanceled = FALSE;
        Log(ThisModule, __LINE__, "介质插入等待已取消.");
        return MEDINS_CANCEL;
    }

    // 介质JAM
    if (bMediaJAM == TRUE)
    {
        Log(ThisModule, __LINE__, "介质JAM.");
        return MEDINS_JAM;
    }

    return MEDINS_OK;
}

// 执行自动复位
INT CXFS_PPR::RunAutoReset(LONG lErrCode)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;

    for (INT i = 0; i < m_stConfig.nAutoResetErrListCnt; i ++)
    {
        if (m_stConfig.nAutoResetErrList[i] == lErrCode)
        {
            nRet = m_pPrinter->Reset();
            if (nRet != PTR_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "错误码[%d], 执行自动复位: ->Reset() Fail, ErrCode = %d, 不做错误处理.",
                   lErrCode, nRet);
            }
            Log(ThisModule, __LINE__,  "错误码[%d], 执行自动复位: ->Reset() Succ.", lErrCode);
        }
    }

    return 0;
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
HRESULT CXFS_PPR::InnerPrintForm(LPWFSPTRPRINTFORM pInData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    CSPPtrData *pData = (CSPPtrData *)m_pData;
    PrintContext pc;
    memset(&pc, 0, sizeof(pc));
    pc.dwTimeOut = dwTimeOut;

    // MediaControl入参检查
    if (pInData->dwMediaControl != 0)
    {
        DWORD dwCtrEj_Ret = (WFS_PTR_CTRLEJECT | WFS_PTR_CTRLRETRACT);
        if ((pInData->dwMediaControl & dwCtrEj_Ret) == dwCtrEj_Ret)
        {
            Log(ThisModule, __LINE__, "接收ControlMedia参数[%d]无效,入参冲突(不能同时退出和回收), ReturnCode: %d.",
                pInData->dwMediaControl, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

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

    if (hRet != WFS_SUCCESS)
    {
        pc.bCancel = TRUE;
    }

    hRet = EndForm(&pc);
    if (hRet != WFS_SUCCESS)
    {
        UpdateDeviceStatus();
        if (m_stStatus.fwMedia == WFS_PTR_MEDIAJAMMED) // 介质JAM
        {
            Log(ThisModule, __LINE__,
                "打印数据: EndForm() fail, ErrCode:%d, GetStatus() Medis Is JAM, Return: %d.",
                hRet, WFS_ERR_PTR_MEDIAJAMMED);
            return WFS_ERR_PTR_MEDIAJAMMED;
        } else
        {
            Log(ThisModule, __LINE__,
                "打印数据: EndForm() fail, ErrCode:%d, Return: %d.", hRet, hRet);
            return hRet;
        }
    }

    return hRet;
}

HRESULT CXFS_PPR::InnerReadForm(LPWFSPTRREADFORM pInData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    CSPPtrData *pData = static_cast<CSPPtrData *>(m_pData);

    HRESULT hRet = WFS_SUCCESS;
    ReadContext rc;
    ZeroMemory(&rc, sizeof(rc));
    rc.pReadData = pInData;
    rc.dwTimeOut = dwTimeOut;
    BOOL bForm = FALSE;

    // MediaControl入参检查
    if (pInData->dwMediaControl != 0)
    {
        DWORD dwCtrEj_Ret = (WFS_PTR_CTRLEJECT | WFS_PTR_CTRLRETRACT);
        if ((pInData->dwMediaControl & dwCtrEj_Ret) == dwCtrEj_Ret)
        {
            Log(ThisModule, __LINE__, "接收ControlMedia参数[%d]无效,入参冲突(不能同时退出和回收), ReturnCode: %d.",
                pInData->dwMediaControl, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

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

    if (Fields.GetCount() == 0 && rc.pForm->GetSubItemCount() == 0)
    {
        Log(ThisModule, __LINE__, "入参无效: FieldNames and Form[%s] is FieldCount = 0, Return: %d.",
            pInData->lpszFormName, WFS_ERR_INVALID_DATA);
        return WFS_ERR_INVALID_DATA;
    }

    hRet = EndReadForm(&rc);
    if (hRet != WFS_SUCCESS)
    {
        if (m_stStatus.fwMedia == WFS_PTR_MEDIAJAMMED) // 介质JAM
        {
            Log(ThisModule, __LINE__,
                "读磁数据: EndReadForm() fail, ErrCode:%d, GetStatus() Medis Is JAM, Return: %d.",
                hRet, WFS_ERR_PTR_MEDIAJAMMED);
            return WFS_ERR_PTR_MEDIAJAMMED;
        } else
        {
            Log(ThisModule, __LINE__,
                "读磁数据: EndReadForm() fail, ErrCode:%d, Return: %d.", hRet, hRet);
            return hRet;
        }
    }

    return hRet;
}

HRESULT CXFS_PPR::InnerReadImage(LPWFSPTRIMAGEREQUEST lpInData, LPWFSPTRIMAGE *&lppOutData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    LPWFSPTRIMAGEREQUEST lpIn = nullptr;
    LPWFSPTRIMAGE lpOut = nullptr;
    READIMAGEIN stScanImageIn;
    READIMAGEOUT stScanImageOut;
    HRESULT hRes = WFS_SUCCESS;

    INT nRet = 0;

    // Check: 入参
    if (lpInData == nullptr)
    {
        Log(ThisModule, __LINE__, "入参无效: Is NULL, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    lpIn = (LPWFSPTRIMAGEREQUEST)lpInData;
    if (lpInData->fwImageSource == 0)
    {
        Log(ThisModule, __LINE__, "入参无效: lpImgRequest->fwImageSource == 0, Return: %d.",
            WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    INT nImageSource = WFS_PTR_IMAGEFRONT | WFS_PTR_IMAGEBACK | WFS_PTR_CODELINE;
    if ((lpInData->fwImageSource & nImageSource) == 0)
    {
        Log(ThisModule, __LINE__, "入参无效: lpImgRequest->fwImageSource == %d, Return: %d.",
            lpInData->fwImageSource, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    // --------检查入参并组织下发到DevCPR的入参--------
    stScanImageIn.Clear();
    if ((lpIn->fwImageSource & WFS_PTR_IMAGEFRONT) == WFS_PTR_IMAGEFRONT)   // 扫描正面图像
    {
        if (lpIn->lpszFrontImageFile == nullptr || strlen(lpIn->lpszFrontImageFile) < 1)
        {
            Log(ThisModule, __LINE__,
                "入参lpszFrontImageFile支持IMAGEFRONT[扫描正面图像], 未设置图像路径参数. Return: %d.",
                WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        // 正面图像路径check
        if (FileAccess::create_directory_by_path(lpIn->lpszFrontImageFile, true) != true)
        {
            Log(ThisModule, __LINE__,
                "入参lpszFrontImageFile支持IMAGEFRONT[扫描正面图像], 指定图像路径[%s]建立失败. Return: %d.",
               lpIn->lpszFrontImageFile, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        // 正面图像格式
        switch(lpIn->wFrontImageType)
        {
            case WFS_PTR_IMAGETIF:  stScanImageIn.wImageType[0] = IMAGE_TYPE_TIF; break;
            case WFS_PTR_IMAGEWMF:  stScanImageIn.wImageType[0] = IMAGE_TYPE_WMF; break;
            case WFS_PTR_IMAGEBMP:  stScanImageIn.wImageType[0] = IMAGE_TYPE_BMP; break;
            case WFS_PTR_IMAGEJPG:  stScanImageIn.wImageType[0] = IMAGE_TYPE_JPG; break;
            default: stScanImageIn.wImageType[0] = IMAGE_TYPE_BMP; break;
        }

        // 正面图像色彩格式
        switch(lpIn->wFrontImageColorFormat)
        {
            case WFS_PTR_IMAGECOLORBINARY: stScanImageIn.wColorFormat[0] = IMAGE_COLOR_BIN; break;
            case WFS_PTR_IMAGECOLORGRAYSCALE: stScanImageIn.wColorFormat[0] = IMAGE_COLOR_GARY; break;
            case WFS_PTR_IMAGECOLORFULL: stScanImageIn.wColorFormat[0] = IMAGE_COLOR_FULL; break;
            default: stScanImageIn.wColorFormat[0] = IMAGE_COLOR_FULL; break;
        }

        stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_FRONT);
        memcpy(stScanImageIn.szImageFrontPath, lpIn->lpszFrontImageFile, strlen(lpIn->lpszFrontImageFile));
    }

    if ((lpIn->fwImageSource & WFS_PTR_IMAGEBACK) == WFS_PTR_IMAGEBACK) // 扫描背面图像
    {
        if (lpIn->lpszBackImageFile == nullptr || strlen(lpIn->lpszBackImageFile) < 1)
        {
            Log(ThisModule, __LINE__,
                "入参lpszFrontImageFile支持IMAGEBACK[扫描背面图像], 未设置图像保存路径参数. Return: %d.",
                WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        // 背面图像路径check
        if (FileAccess::create_directory_by_path(lpIn->lpszBackImageFile, true) != true)
        {
            Log(ThisModule, __LINE__,
                "入参lpszFrontImageFile支持IMAGEFRONT[扫描背面图像], 指定图像保存路径[%s]建立失败. Return: %d.",
               lpIn->lpszBackImageFile, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        // 正面图像格式
        switch(lpIn->wBackImageType)
        {
            case WFS_PTR_IMAGETIF:  stScanImageIn.wImageType[1] = IMAGE_TYPE_TIF; break;
            case WFS_PTR_IMAGEWMF:  stScanImageIn.wImageType[1] = IMAGE_TYPE_WMF; break;
            case WFS_PTR_IMAGEBMP:  stScanImageIn.wImageType[1] = IMAGE_TYPE_BMP; break;
            case WFS_PTR_IMAGEJPG:  stScanImageIn.wImageType[1] = IMAGE_TYPE_JPG; break;
            default: stScanImageIn.wImageType[1] = IMAGE_TYPE_BMP; break;
        }

        // 正面图像色彩格式
        switch(lpIn->wBackImageColorFormat)
        {
            case WFS_PTR_IMAGECOLORBINARY: stScanImageIn.wColorFormat[1] = IMAGE_COLOR_BIN; break;
            case WFS_PTR_IMAGECOLORGRAYSCALE: stScanImageIn.wColorFormat[1] = IMAGE_COLOR_GARY; break;
            case WFS_PTR_IMAGECOLORFULL: stScanImageIn.wColorFormat[1] = IMAGE_COLOR_FULL; break;
            default: stScanImageIn.wColorFormat[1] = IMAGE_COLOR_FULL; break;
        }

        stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_BACK);
        memcpy(stScanImageIn.szImageBackPath, lpIn->lpszBackImageFile, strlen(lpIn->lpszBackImageFile));
    }

    if ((lpIn->fwImageSource & WFS_PTR_CODELINE) == WFS_PTR_CODELINE) // 代码行
    {
        stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_CODELINE);

        switch(lpIn->wCodelineFormat)
        {
            case WFS_PTR_CODELINECMC7: stScanImageIn.wCodeLine = IMAGE_CODE_CMC7; break;
            case WFS_PTR_CODELINEE13B: stScanImageIn.wCodeLine = IMAGE_CODE_E13B; break;
            case WFS_PTR_CODELINEOCR: stScanImageIn.wCodeLine = IMAGE_CODE_OCR; break;
            default: stScanImageIn.wCodeLine = IMAGE_CODE_CMC7; break;
        }
    }

    if (m_stConfig.nDeviceType == DEV_MB2)    // 南天MB2打印机 参数设置
    {
        stScanImageIn.lOtherParam[0] = m_stConfig.stConfig_MB2.wScanDPI;        // 扫描分辨率控制
        stScanImageIn.lOtherParam[1] = m_stConfig.stConfig_MB2.wCisColorMode;   // 扫描光调色模式
        stScanImageIn.lOtherParam[2] = m_stConfig.stConfig_MB2.wGrayMode;       // 扫描灰度模式模式
        stScanImageIn.lOtherParam[3] = m_stConfig.stConfig_MB2.wBrightness;     // 扫描亮度
        stScanImageIn.lOtherParam[4] = m_stConfig.stConfig_MB2.wThresholdLevel; // 扫描黑白包容度
        stScanImageIn.lOtherParam[5] = m_stConfig.stConfig_MB2.wScanDirection;  // 扫描图像的方向
    }

    // 等待插折(按不同打印机分支处理)
    nRet = MediaInsertWait(dwTimeOut);
    if (nRet != WFS_SUCCESS)
    {
        return nRet;
    }

    nRet = PTR_SUCCESS;
    stScanImageOut.Clear();
    nRet = m_pPrinter->ReadImage(stScanImageIn, stScanImageOut);
    stScanImageIn.Clear();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "图像扫描: ->ScanImage() Fail, ErrCode = %d, Return: %d.",
            nRet, ConvertErrCode(nRet));
        //return ConvertErrCode(nRet);
    }

    // 记录应答Data到Log
    Log(ThisModule, __LINE__, "图像扫描: ->ScanImage() succ, wInMode = %d, wResult = %d, lpData=%s.",
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
        if ((stScanImageOut.wInMode & IMAGE_MODE_CODELINE) == IMAGE_MODE_CODELINE &&
            stScanImageOut.lpData != nullptr)
        {
            lpOut->ulDataLength = stScanImageOut.ulDataSize;
            lpOut->lpbData = new BYTE[stScanImageOut.ulDataSize + 1];
            memset(lpOut->lpbData, 0x00, stScanImageOut.ulDataSize + 1);
            memcpy(lpOut->lpbData, stScanImageOut.lpData, stScanImageOut.ulDataSize);
        }
    }

    lppOutData = new LPWFSPTRIMAGE[2];
    lppOutData[0] = lpOut;
    lppOutData[1] = nullptr;

    stScanImageOut.Clear();

    return WFS_SUCCESS;
}

// RawData无格式打印命令子处理
HRESULT CXFS_PPR::InnerReadRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRet = WFS_SUCCESS;
    INT nRet = PTR_SUCCESS;

    if (nSize == 0)
    {
        return WFS_SUCCESS;
    }

    // 等待插折
    hRet = MediaInsertWait(dwTimeOut);          // 等待插折处理
    if (hRet != WFS_SUCCESS)
    {
        return hRet;
    }

    m_pData->m_InputRawData.SetData(0, nullptr);

    if (m_stConfig.nRawDataInPar == 0)                          // RawData入参模式:0/UTF8
    {                                                           //
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
        int nTempSize = tmpData.size();     // 转换后GBK数据长度
        BYTE *pBuf = nullptr;
        ULONG ulDataSize = 0;
        if (m_stConfig.nLineSize > 0)
        {
            int nBufferSize = nTempSize + 2 + (nTempSize / m_stConfig.nLineSize) * 2;
            pBuf = new BYTE[nBufferSize];
            memset(pBuf, 0, nBufferSize);
            RemoveUnPrintableChar(nTempSize, (LPBYTE)pTempCode, ulDataSize, pBuf);
            // 去除不可打印字符
            if (pBuf[ulDataSize - 1] != '\n')
            {
                pBuf[ulDataSize++] = '\n';
            }
            pBuf[ulDataSize] = 0;
        } else
        {
            ulDataSize = nTempSize;
            pBuf = new BYTE[ulDataSize + 1];
            memset(pBuf, 0, (ulDataSize + 1));
            memcpy(pBuf, pTempCode, ulDataSize);
        }
        QTextCodec::setCodecForLocale(codec);

        nRet = m_pPrinter->PrintData((char *)pBuf, ulDataSize);
        if (nRet != PTR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "RawData: ->PrintData(%s, %d) Fail, ErrCode = %d, Return: %d.",
                pBuf, ulDataSize, nRet, ConvertErrCode(nRet));
        }

        if (bExpectResp)
        {
            m_pData->m_InputRawData.SetData(ulDataSize, pBuf);
        }
        delete [] pBuf;
        pBuf = nullptr;
    } else                                                  // RawData入参模式:非0/GBK
    {
        nRet = m_pPrinter->PrintData((char *)pData, nSize);
        if (nRet != PTR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "RawData: ->PrintData(%s, %d) Fail, ErrCode = %d, Return: %d.",
                pData, nSize, nRet, ConvertErrCode(nRet));
        }

        if (bExpectResp)
        {
            m_pData->m_InputRawData.SetData(nSize, pData);
        }
    }

    return ConvertErrCode(nRet);
}

// 设备复位命令子处理
HRESULT CXFS_PPR::InnerReset(DWORD dwMediaControl, USHORT usBinIndex)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = PTR_SUCCESS;
    HRESULT hRet = WFS_SUCCESS;
    BOOL bRunReset = TRUE;

    // CHECK
    // WFS_PTR_CTRLEJECT 退出媒介
    // WFS_PTR_CTRLRETRACT 根据usRetractBinNumber 规定将媒介回收入回收盒
    // WFS_PTR_CTRLEXPEL 从出口抛出媒介
    WORD wCheckCmd = WFS_PTR_CTRLEJECT | WFS_PTR_CTRLRETRACT | WFS_PTR_CTRLEXPEL;

    if (dwMediaControl != 0)    // 参数检查
    {
        if ((dwMediaControl & wCheckCmd) == 0)
        {
            Log(ThisModule, __LINE__, "入参:MediaControl[%d] 无效, Return: %d.",
                dwMediaControl, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        if ((dwMediaControl & WFS_PTR_CTRLEXPEL) == WFS_PTR_CTRLEXPEL)
        {
            Log(ThisModule, __LINE__, "入参:MediaControl[%d] 不支持, Return: %d.", WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    // MB2打印机每次执行Reset会将介质向外推出一部分,避免介质完全推出出口，在介质位于出口时,缺省不执行
    if (m_stConfig.nDeviceType == DEV_MB2)
    {
        if(m_stStatus.fwMedia == WFS_PTR_MEDIAENTERING &&
           m_stStatus.fwDevice == WFS_PTR_DEVONLINE)
        {
            Log(ThisModule, __LINE__, "复位(MB2): 介质在出口, 状态ONLINE, 不执行Reset, 直接返回Success.");
            if ((dwMediaControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)
            {
                m_WaitTaken = WTF_TAKEN;
            }
            bRunReset = FALSE;
        }
    }

    // 执行复位
    if (bRunReset == TRUE)
    {
        nRet = m_pPrinter->Reset();
        if (nRet != PTR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "复位: ->Reset() Fail, ErrCode = %d, Return: %d.",
               nRet, ConvertErrCode(nRet));
            return ConvertErrCode(nRet);
        }
    }
    m_bCmdCanceled = FALSE;

    // 复位后参数动作处理
    if (dwMediaControl != 0)
    {
        hRet = InnerMediaControl(dwMediaControl);
        if (hRet != WFS_SUCCESS && hRet != WFS_ERR_PTR_NOMEDIAPRESENT)
        {
            Log(ThisModule, __LINE__, "复位后参数动作处理: ->InnerMediaControl(%d) Fail, Return: %d.",
               dwMediaControl, hRet);
            return hRet;
        }
    }

    return WFS_SUCCESS;
}

// MediaControl介质控制子处理
HRESULT CXFS_PPR::InnerMediaControl(DWORD dwControl)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;

    MEDIA_ACTION emMediaAct = MEDIA_CTR_NOTACTION;  // 无动作

    DWORD dwCtrEj_Ret = (WFS_PTR_CTRLEJECT | WFS_PTR_CTRLRETRACT);

    // 无效入参(参数冲突)
    if ((dwControl & dwCtrEj_Ret) == dwCtrEj_Ret)
    {
        Log(ThisModule, __LINE__, "接收ControlMedia参数[%d]无效,入参冲突(不能同时退出和回收), ReturnCode: %d.",
            dwControl, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 回收前检查: 回收盒状态
    if ((dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT)
    {
        if (m_stRetractCfg.nRetractSup == 0)    // 不支持回收
        {
            Log(ThisModule, __LINE__, "设备(INI)设置不支持回收, ReturnCode: %d.",
                WFS_ERR_UNSUPP_COMMAND);
            return WFS_ERR_UNSUPP_COMMAND;
        }
        if (m_stRetractCfg.dwRetractCnt >= m_stRetractCfg.dwRetractVol) // 回收满
        {
            Log(ThisModule, __LINE__, "回收已满(支持回收数:%d,已回收数:%d), ReturnCode: %d.",
                m_stRetractCfg.dwRetractVol, m_stRetractCfg.dwRetractCnt,
                WFS_ERR_PTR_RETRACTBINFULL);
            return WFS_ERR_PTR_RETRACTBINFULL;
        }
    }

    if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT ||
        (dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT ||
        (dwControl & WFS_PTR_CTRLFLUSH) == WFS_PTR_CTRLFLUSH )
    {
        // 介质检查(退出/回收)
        if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT ||
            (dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT)
        {
            UpdateDeviceStatus();       // 更新当前设备状态
            if (m_stStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT)    // 没有介质
            {
                Log(ThisModule, __LINE__, "介质控制: 设备内部没有介质, ReturnCode: %d.", WFS_ERR_PTR_NOMEDIAPRESENT);
                return WFS_ERR_PTR_NOMEDIAPRESENT;
            } else
            if (m_stStatus.fwMedia == WFS_PTR_MEDIAJAMMED)        // 介质JAM
            {
                Log(ThisModule, __LINE__, "介质控制: 介质处于JAM状态, ReturnCode: %d.", WFS_ERR_PTR_MEDIAJAMMED);
                return WFS_ERR_PTR_MEDIAJAMMED;
            }

            if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT &&      // 退出介质+已在出口
                 m_stStatus.fwMedia == WFS_PTR_MEDIAENTERING)
            {
                Log(ThisModule, __LINE__, "介质控制: 介质在出口, ReturnCode: %d.", WFS_SUCCESS);
                m_WaitTaken = WTF_TAKEN;
                return WFS_SUCCESS;
            }
        }

        // 下发命令组织入参
        if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)      // 退出介质
        {
            emMediaAct = MEDIA_CTR_EJECT;
        } else
        if((dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT)    // 回收介质
        {
            emMediaAct = MEDIA_CTR_RETRACT;
        } else
        if((dwControl & WFS_PTR_CTRLFLUSH) == WFS_PTR_CTRLFLUSH)
        {
            emMediaAct = MEDIA_CTR_FLUSH;
        }

        // 下发命令
        nRet = m_pPrinter->MeidaControl(emMediaAct);
        if (nRet != PTR_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "介质控制: MeidaControl(%d) Fail, ErrCode: %d, ReturnCode: %d.",
                emMediaAct, nRet, ConvertErrCode(nRet));
            return ConvertErrCode(nRet);
        }

        if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)      // 退出介质
        {
            m_WaitTaken = WTF_TAKEN;
        }
        if ((dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT)    // 回收介质
        {
            // 回收计数
            m_stRetractCfg.dwRetractCnt = m_stRetractCfg.dwRetractCnt + 1;
            m_cXfsReg.SetValue("RETRACT_CFG", "RetractCount",
                               std::to_string(m_stRetractCfg.dwRetractCnt).c_str());
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

//-----------------------------------------------------------------------------------
//-----------------------------------重载接口-------------------------------
//
HRESULT CXFS_PPR::OnInit()
{
    THISMODULE(__FUNCTION__);

    /*long nRet = m_pPrinter->Init();
    UpdateDeviceStatus();
    if (nRet < 0)
    {
        return ConvertErrCode(nRet);
    }

    return ConvertErrCode(nRet);*/
    return WFS_SUCCESS;
}

//
HRESULT CXFS_PPR::OnExit()
{
    if (nullptr != m_pPrinter)
    {
        m_pPrinter->Close();
    }

    return WFS_SUCCESS;
}

// PrintForm最终处理
HRESULT CXFS_PPR::EndForm(PrintContext *pContext)
{
    THISMODULE(__FUNCTION__);

    // FORM行列值转换为MM(0.1MM单位),有小数位四舍五入
    #define ROWCOL2MM(ROL, MM) \
        pContext->pForm->GetOrigUNIT(nullptr) == FORM_ROWCOLUMN ? \
            (int)(ROL * MM) : ROL * 10

    // FORM设置CPI/LPI转MM(0.1MM单位)(英寸->0.1MM)
    #define LPI2MM(LPI) (LPI < 1 ? (int)(30) : (int)(254 / LPI))
    #define CPI2MM(CPI) (CPI < 1 ? (int)(15) : (int)(254 / CPI))

    CSPPtrData *pData = (CSPPtrData *)m_pData;

    DWORD dwStagger = 0; // 介质交错位置(上留白)
    HRESULT hRet = WFS_SUCCESS;

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

    // 等待插折
    DWORD dwTimeOut = pContext->dwTimeOut;      // 超时时间
    hRet = MediaInsertWait(dwTimeOut);          // 等待插折处理
    if (hRet != WFS_SUCCESS)
    {
        if (pItems) // 返回非SUCCESS,清空Items
        {
            delete pItems;
            pItems = nullptr;
        }
        return hRet;
    }

    // 形成打印字串strupr
    WORD wStagger = pContext->pMedia->GetStaggering(); // 介质交错位置
    wStagger = (m_stConfig.nStaggerMode == 0 ? ROWCOL2MM(wStagger, stSize.cy) * 10 :
                 (m_stConfig.nStaggerMode == 1 ? wStagger * 10 : wStagger));
    RECT stMediaRestArea;
    pContext->pMedia->GetOrigRestrictedArea(stMediaRestArea);

    char szPrintData[MAX_PRINTDATA_LEN] = {0};
    //DWORD dwDataSize = 0;
    DWORD dwX = 0;
    DWORD dwY = 0, dwY_Tmp = 0;
    DWORD dwLPI = 0;
    DWORD dwCPI = 0;
    DWORD dwStyle = 0;
    //WORD  wRowIdx = 0;
    WORD  wRowCnt = 0;
    DEVPRINTFORMIN stPrintIn;
    DEVPRINTFORMOUT stPrintOut;

    for (int i = 0; i < pItems->nItemNum && hRet >= 0; i++)
    {
        PRINT_ITEM *pItem = pItems->pItems[i];
        hRet = WFS_SUCCESS;

        // 行列转换为0.1MM值
        dwX = ROWCOL2MM(pItem->x, stSize.cx);
        dwY = ROWCOL2MM(pItem->y, stSize.cy);
        dwLPI = (m_stConfig.nFieldLPIMode == 0 ? LPI2MM(pItem->dwLPI) :
                 (m_stConfig.nFieldLPIMode == 1 ? pItem->dwLPI * 10 :
                  (m_stConfig.nFieldLPIMode == 2 ? pItem->dwLPI : pItem->dwLPI)));
        dwCPI = (m_stConfig.nFieldCPIMode == 0 ? CPI2MM(pItem->dwCPI) :
                 (m_stConfig.nFieldCPIMode == 1 ? pItem->dwCPI * 10 :
                  (m_stConfig.nFieldCPIMode == 2 ? pItem->dwCPI * 10 : pItem->dwCPI)));
        dwStyle = pItem->dwStyle;

        //wRowIdx = (m_stConfig.nFieldIdxStart == 0 ? pItem->nFieldIdx + 1 : pItem->nFieldIdx);  // Field下标
        wRowCnt = pItem->y;

        // 介质折叠区处理
        if (stMediaRestArea.top > 0 && /*wRowIdx*/wRowCnt > stMediaRestArea.top) // top = RestrictedAreaY;
        {
            //wRowIdx = wRowIdx + (stMediaRestArea.bottom - stMediaRestArea.top);   // bottom = RestrictHeight 折叠占行数
            wRowCnt = wRowCnt + (stMediaRestArea.bottom - stMediaRestArea.top);
            dwY_Tmp = pItem->y + (stMediaRestArea.bottom - stMediaRestArea.top);
            dwY = ROWCOL2MM(dwY_Tmp, stSize.cy);
        }

        // dwY为当前行顶线坐标,根据Field LPI指定行高进行处理(按单位0.1MM)
        // dwY + 介质上留白 + 缺省行高与指定行高的差距 * 当前行数 + 缺省行高
        //dwY = (dwY + wStagger + wRowCnt * (dwLPI - stSize.cy >= 0 ? dwLPI - stSize.cy : 0) + stSize.cy);
        dwY = (dwY + wStagger + wRowCnt * (dwLPI - stSize.cy >= 0 ? dwLPI - stSize.cy : 0) + stSize.cy);

        // 文本打印
        if (pItem->nFieldType == FT_TEXT)
        {
            if (pItem->nTextLen > 0)    // 有打印数据
            {
                // 下发打印数据
                MCPY_LEN(szPrintData, pItem->Text, pItem->nTextLen);
                stPrintIn.Clear();
                stPrintIn.wInMode = PRINTFORM_TEXT;
                stPrintIn.ulX = dwX;
                stPrintIn.ulY = dwY;
                stPrintIn.lpData = szPrintData;
                stPrintIn.dwDataSize = pItem->nTextLen;
                stPrintIn.dwTimeOut = dwTimeOut;

                stPrintIn.lOtherParam[0] = dwCPI;
                stPrintIn.lOtherParam[1] = dwLPI;
                stPrintIn.lOtherParam[2] = dwStyle;

                if (m_stConfig.nDeviceType == DEV_MB2)  // MB2打印机
                {
                    hRet = EndForm_MB2(stPrintIn, stPrintOut);
                    if (hRet != WFS_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "下发打印数据: EndForm_MB2() fail, ErrCode:%d, Return: %d.", hRet, hRet);
                        break;
                    }
                } else
                if (m_stConfig.nDeviceType == DEV_PRM)  // PRM打印机
                {
                    hRet = EndForm_PRM(stPrintIn, stPrintOut);
                    if (hRet != WFS_SUCCESS)
                    {
                        Log(ThisModule, __LINE__,
                            "下发打印数据: EndForm_PRM() fail, ErrCode:%d, Return: %d.", hRet, hRet);
                        break;
                    }
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
            stPrintIn.lpData = pItem->strImagePath;
            stPrintIn.dwDataSize = strlen(pItem->strImagePath);
            stPrintIn.dwTimeOut = dwTimeOut;

            if (m_stConfig.nDeviceType == DEV_MB2)  // MB2打印机
            {
                hRet = EndForm_MB2(stPrintIn, stPrintOut);
                if (hRet != WFS_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "下发图片打印数据: EndForm_MB2() fail, ErrCode:%d, Return: %d.", hRet, hRet);
                    break;
                }
            } else
            if (m_stConfig.nDeviceType == DEV_PRM)  // PRM打印机
            {
                hRet = EndForm_PRM(stPrintIn, stPrintOut);
                if (hRet != WFS_SUCCESS)
                {
                    Log(ThisModule, __LINE__,
                        "下发图片打印数据: EndForm_PRM() fail, ErrCode:%d, Return: %d.", hRet, hRet);
                    break;
                }
            }
        } else
        {
            Log(ThisModule, __LINE__, "不支持的Fieled[%s]打印类型[%d],跳过.",
                pItem->strFieldName, pItem->nFieldType);
        }

        // 支持打印取消
        if (m_stConfig.nPrintCalcelSup == 1)
        {
            if (m_bCmdCanceled == TRUE)
            {
                m_bCmdCanceled = FALSE;
                Log(ThisModule, __LINE__, "PrintForm打印已取消.");
                hRet = WFS_ERR_CANCELED;
                break;
            }
        }
    }

    // 打印成功后,执行MediaControl
    if (hRet == WFS_SUCCESS)
    {
        DWORD dwMediaControl = pContext->pPrintData->dwMediaControl;
        if (dwMediaControl != 0)
        {
            hRet = InnerMediaControl(dwMediaControl);
        }
    }

    // 删除自定义数据
    if (pItems)
    {
        delete pItems;
        pItems = NULL;
    }
    pContext->pUserData = NULL;

    return hRet;
}

// ReadForm最终处理
HRESULT CXFS_PPR::EndReadForm(ReadContext *pContext)
{
    //if (m_stConfig.nDeviceType == DEV_MB2 ||    // MB2打印机
    //    m_stConfig.nDeviceType == DEV_PRM)      // PRM打印机
    //{
        return EndReadForm_Pub(pContext);
    //}

    return WFS_SUCCESS;
}


//-----------------------------------------------------------------------------------
//--------------------------------------功能处理接口-----------------------------------
// 加载DevXXX动态库
bool CXFS_PPR::LoadDevDll(LPCSTR ThisModule)
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
void CXFS_PPR::InitConifig()
{
    THISMODULE(__FUNCTION__);

    CHAR    szIniAppName[MAX_PATH];
    CHAR    szBuffer[MAX_PATH];
    INT     nTmp;

    // DevPPR动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 设备类型
    m_stConfig.nDeviceType = m_cXfsReg.GetValue("DriverType", (DWORD)DEV_MB2);

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

    if (m_stConfig.nDeviceType == DEV_MB2)    // 南天MB2打印机 参数获取
    {
        // 设备打开模式(USB/ttyS*,缺省USB)
        strcpy(m_stConfig.stDevOpenMode.szOpenParam, m_cXfsReg.GetValue(szIniAppName, "OpenMode", "USB"));
        if (strlen(m_stConfig.stDevOpenMode.szOpenParam) < 1)
        {
            MSET_0(m_stConfig.stDevOpenMode.szOpenParam)
        }
        // 波特率,缺省9600
        m_stConfig.stDevOpenMode.nOpenParam = (DWORD)m_cXfsReg.GetValue(szIniAppName, "BandRate", (DWORD)9600);
        if (m_stConfig.stDevOpenMode.nOpenParam < 0)
        {
            m_stConfig.stDevOpenMode.nOpenParam = 9600;
        }
        // 打印质量(0:草稿/1:高速草稿/2:NLQ1/3:NLQ2/4:LQ2,缺省0)
        m_stConfig.stConfig_MB2.wPrintQuality = (DWORD)m_cXfsReg.GetValue(szIniAppName, "PrintQuality", (DWORD)0);
        if (m_stConfig.stConfig_MB2.wPrintQuality < 0 || m_stConfig.stConfig_MB2.wPrintQuality > 4)
        {
            m_stConfig.stConfig_MB2.wPrintQuality = 0;
        }
        // 指定磁道2类型，正确适用(0/2/3),缺省0
        m_stConfig.stConfig_MB2.wTrack2Type = (DWORD)m_cXfsReg.GetValue(szIniAppName, "Track2Type", (DWORD)0);
        if (m_stConfig.stConfig_MB2.wTrack2Type != 0 &&
            m_stConfig.stConfig_MB2.wTrack2Type != 2 &&
            m_stConfig.stConfig_MB2.wTrack2Type != 3)
        {
            m_stConfig.stConfig_MB2.wTrack2Type = 0;
        }
        // 指定磁道3类型，正确适用(1/4/6),缺省1
        m_stConfig.stConfig_MB2.wTrack3Type = (DWORD)m_cXfsReg.GetValue(szIniAppName, "Track3Type", (DWORD)1);
        if (m_stConfig.stConfig_MB2.wTrack3Type != 1 &&
            m_stConfig.stConfig_MB2.wTrack3Type != 4 &&
            m_stConfig.stConfig_MB2.wTrack3Type != 6)
        {
            m_stConfig.stConfig_MB2.wTrack3Type = 1;
        }
        // 扫描分辨率控制(200/300/600,缺省300)
        m_stConfig.stConfig_MB2.wScanDPI = (DWORD)m_cXfsReg.GetValue(szIniAppName, "ScanDPI", (DWORD)300);
        if (m_stConfig.stConfig_MB2.wScanDPI != 200 &&
            m_stConfig.stConfig_MB2.wScanDPI != 300 &&
            m_stConfig.stConfig_MB2.wScanDPI != 600)
        {
            m_stConfig.stConfig_MB2.wScanDPI = 300;
        }
        // 扫描光调色模式(0:Red, 1:Green, 2:Red+Green+Blue, 3:Blue, 4:真彩色(RGB), 缺省2)
        m_stConfig.stConfig_MB2.wCisColorMode = (DWORD)m_cXfsReg.GetValue(szIniAppName, "CisColorMode", (DWORD)2);
        if (m_stConfig.stConfig_MB2.wCisColorMode < 0 ||
            m_stConfig.stConfig_MB2.wCisColorMode > 4)
        {
            m_stConfig.stConfig_MB2.wCisColorMode = 2;
        }
        // 扫描灰度模式模式(0:黑白, 1:16色, 2:256色/24为真彩色, 3:不支持/不做扫描, 缺省2)
        m_stConfig.stConfig_MB2.wGrayMode = (DWORD)m_cXfsReg.GetValue(szIniAppName, "GrayMode", (DWORD)2);
        if (m_stConfig.stConfig_MB2.wGrayMode < 0 ||
            m_stConfig.stConfig_MB2.wGrayMode > 3)
        {
            m_stConfig.stConfig_MB2.wGrayMode = 2;
        }
        // 扫描亮度(1~255,缺省100)
        m_stConfig.stConfig_MB2.wBrightness = (DWORD)m_cXfsReg.GetValue(szIniAppName, "Brightness", (DWORD)100);
        if (m_stConfig.stConfig_MB2.wBrightness < 0 ||
            m_stConfig.stConfig_MB2.wBrightness > 255)
        {
            m_stConfig.stConfig_MB2.wBrightness = 100;
        }
        // 扫描黑白包容度(1~255,缺省100)
        m_stConfig.stConfig_MB2.wThresholdLevel = (DWORD)m_cXfsReg.GetValue(szIniAppName, "ThresholdLevel", (DWORD)100);
        if (m_stConfig.stConfig_MB2.wThresholdLevel < 0 ||
            m_stConfig.stConfig_MB2.wThresholdLevel > 255)
        {
            m_stConfig.stConfig_MB2.wThresholdLevel = 100;
        }
        // 扫描图像的方向(0:镜像, 1:正像, 缺省1)
        m_stConfig.stConfig_MB2.wScanDirection = (DWORD)m_cXfsReg.GetValue(szIniAppName, "ScanDirection", (DWORD)1);
        if (m_stConfig.stConfig_MB2.wScanDirection < 0 ||
            m_stConfig.stConfig_MB2.wScanDirection > 1)
        {
            m_stConfig.stConfig_MB2.wScanDirection = 1;
        }
    } else
    if (m_stConfig.nDeviceType == DEV_PRM)    // 中航PRM打印机 参数获取
    {
        // 设备打开模式(USB/ttyS*,缺省USB)
        strcpy(m_stConfig.stDevOpenMode.szOpenParam, m_cXfsReg.GetValue(szIniAppName, "OpenMode", "USB"));
        if (strlen(m_stConfig.stDevOpenMode.szOpenParam) < 1)
        {
            MSET_0(m_stConfig.stDevOpenMode.szOpenParam)
        }
        // SDK打印命令返回模式(0:设备打印完成后返回, 1:不等待设备打印完返回, 缺省0)
        m_stConfig.stConfig_PRM.wPrintDataMode = (DWORD)m_cXfsReg.GetValue(szIniAppName, "SDK_PrintDataMode", (DWORD)0);
        if (m_stConfig.stConfig_PRM.wPrintDataMode < 0 ||
            m_stConfig.stConfig_PRM.wPrintDataMode > 1)
        {
            m_stConfig.stConfig_PRM.wPrintDataMode = 1;
        }
        // SDK接口等待时间(缺省5, 单位:秒)
        m_stConfig.stConfig_PRM.wFuncWaitTime = (DWORD)m_cXfsReg.GetValue(szIniAppName, "SDK_FuncWaitTime", (DWORD)5);
        if (m_stConfig.stConfig_PRM.wFuncWaitTime < 0)
        {
            m_stConfig.stConfig_PRM.wFuncWaitTime = 5;
        }
    }

    //-----------------用于共通相关定义----------------
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
    m_stConfig.nLineSize = m_cXfsReg.GetValue("CONFIG", "line_size", (DWORD)0);

    // 是否根据line_size对打印数据进行换行处理,0否/1是,缺省1
    m_stConfig.bEnableSplit = m_cXfsReg.GetValue("CONFIG", "enabled_split", 1) != 0;


    //-----------------用于Form标准相关定义----------------
    // 介质上边界留白高度单位(0:行列值,1:毫米,2:0.1毫米,缺省0)
    m_stConfig.nStaggerMode = m_cXfsReg.GetValue("FORM_CONFIG", "StaggerMode", (DWORD)0);
    if (m_stConfig.nStaggerMode < 0 || m_stConfig.nStaggerMode > 2)
    {
        m_stConfig.nStaggerMode = 0;
    }

    // Field下标起始值(0/1,缺省0)
    m_stConfig.nFieldIdxStart = m_cXfsReg.GetValue("FORM_CONFIG", "FieldIdxStart", (DWORD)0);
    if (m_stConfig.nFieldIdxStart < 0 || m_stConfig.nFieldIdxStart > 1)
    {
        m_stConfig.nFieldIdxStart = 0;
    }

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

    // 磁道1关键字(最大60位,缺省Track1)
    strcpy(m_stConfig.szReadTrackName[0], m_cXfsReg.GetValue("FORM_CONFIG", "Track1Name", "Track1"));

    // 磁道2关键字(最大60位,缺省Track2)
    strcpy(m_stConfig.szReadTrackName[1], m_cXfsReg.GetValue("FORM_CONFIG", "Track2Name", "Track2"));

    // 磁道3关键字(最大60位,缺省Track3)
    strcpy(m_stConfig.szReadTrackName[2], m_cXfsReg.GetValue("FORM_CONFIG", "Track3Name", "Track3"));

    //-----------------用于PrintForm/ReadForm相关定义----------------
    // PrintForm打印中是否支持取消(0:不支持, 1支持, 缺省0)
    m_stConfig.nPrintCalcelSup = m_cXfsReg.GetValue("PRINT_READ_CFG", "PrintCancelSup", (DWORD)0);
    if (m_stConfig.nPrintCalcelSup != 0 && m_stConfig.nPrintCalcelSup != 1)
    {
        m_stConfig.nPrintCalcelSup = 0;
    }

    //-----------------用于ReadRawData相关定义----------------
    // RawData入参模式: 0/UTF8;1GBK,缺省0
    m_stConfig.nRawDataInPar = m_cXfsReg.GetValue("RawData", "InParMode", (DWORD)0);
    if (m_stConfig.nRawDataInPar != 0 && m_stConfig.nRawDataInPar != 1)
    {
        m_stConfig.nRawDataInPar = 0;
    }

    //-----------------用于RESET相关定义----------------
    // 需要自动复位的错误码列表[数字,以","分隔,最多64个],缺省空
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue("RESET_CFG", "AutoReset_ErrList", ""));

    m_stConfig.nAutoResetErrListCnt = 0;
    MSET_0(m_stConfig.nAutoResetErrList);

    nTmp = DataConvertor::split_string(szBuffer, ',', nullptr, 0);
    if (nTmp > 0)
    {
        char szErrList[nTmp][CONST_VALUE_260];
        DataConvertor::split_string(szBuffer, ',', szErrList, nTmp);

        for (INT i = 0; i < nTmp; i ++)
        {
            if (atoi(szErrList[i]) != 0)
            {
                m_stConfig.nAutoResetErrList[m_stConfig.nAutoResetErrListCnt ++] = atoi(szErrList[i]);
            }
        }
    }

    //-----------------用于Event相关定义----------------
    // 是否上报WFS_SYSE_HARDWARE_ERROR事件,0不上报,1上报,缺省1
    m_stConfig.nReportHWErrEvent = m_cXfsReg.GetValue("EVENT_CFG", "ReportHWErrEvent", (DWORD)1);
    if (m_stConfig.nReportHWErrEvent != 0 && m_stConfig.nReportHWErrEvent != 1)
    {
        m_stConfig.nReportHWErrEvent = 1;
    }

    //-----------------用于回收相关定义----------------
    // 是否支持回收(0不支持/1支持,缺省0)
    m_stRetractCfg.nRetractSup = m_cXfsReg.GetValue("RETRACT_CFG", "RetractSup", (DWORD)0);
    if (m_stRetractCfg.nRetractSup != 0 && m_stRetractCfg.nRetractSup != 1)
    {
        m_stRetractCfg.nRetractSup = 0;
    }

    // 回收满数目,缺省20
    m_stRetractCfg.dwRetractVol = m_cXfsReg.GetValue("RETRACT_CFG", "RetractVolume", (DWORD)20);

    // 回收将满阀值,缺省15
    m_stRetractCfg.dwFullThreshold = m_cXfsReg.GetValue("RETRACT_CFG", "FullThreshold", (DWORD)15);

    // 回收计数,缺省0
    m_stRetractCfg.dwRetractCnt = m_cXfsReg.GetValue("RETRACT_CFG", "RetractCount", (DWORD)0);


    //-----------------设备鸣响设置定义----------------
    // 是否支持鸣响(0:不支持, 1:支持, 缺省1)
    m_stConfig.stConfig_Beep.wSupp = m_cXfsReg.GetValue("BEEP_CFG", "BeepSup", (DWORD)1);
    if (m_stConfig.stConfig_Beep.wSupp != 0 && m_stConfig.stConfig_Beep.wSupp != 1)
    {
        m_stConfig.stConfig_Beep.wSupp = 1;
    }

    // 鸣响频率/鸣响间隔(缺省1000,单位:毫秒)
    m_stConfig.stConfig_Beep.wInterval = m_cXfsReg.GetValue("BEEP_CFG", "BeepInterval", (DWORD)1000);
    if (m_stConfig.stConfig_Beep.wInterval < 0 || m_stConfig.stConfig_Beep.wInterval > 65536)
    {
        m_stConfig.stConfig_Beep.wInterval = 1000;
    }

    // 每次鸣响的次数(缺省1)
    m_stConfig.stConfig_Beep.wCount = m_cXfsReg.GetValue("BEEP_CFG", "BeepCount", (DWORD)1);
    if (m_stConfig.stConfig_Beep.wCount < 0 || m_stConfig.stConfig_Beep.wCount > 65536)
    {
        m_stConfig.stConfig_Beep.wCount = 1;
    }

    return;
}

// 初始化Cen标准状态类
HRESULT CXFS_PPR::InitStatus()
{
    memset(&m_stStatus, 0x00, sizeof(WFSPTRSTATUS));
    m_stStatus.fwDevice      = WFS_PTR_DEVNODEVICE;
    m_stStatus.fwPaper[0]    = WFS_PTR_PAPERUNKNOWN;
    m_stStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
    m_stStatus.fwToner       = WFS_PTR_TONERFULL;
    m_stStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
    m_stStatus.fwInk         = WFS_PTR_INKNOTSUPP;

    // 回收设置
    if (m_stRetractCfg.nRetractSup == 0)    // 不支持回收
    {
        m_stStatus.lppRetractBins = nullptr;
    } else
    {
        // 申请回收箱计数buffer
        m_stStatus.lppRetractBins = new LPWFSPTRRETRACTBINS[m_stRetractCfg.wRetBoxCount + 1];
        memset(m_stStatus.lppRetractBins, 0x00, sizeof(LPWFSPTRRETRACTBINS) * (m_stRetractCfg.wRetBoxCount + 1));
        m_stStatus.lppRetractBins[0] = new WFSPTRRETRACTBINS;
        m_stStatus.lppRetractBins[1] = nullptr;
    }

    m_stStatus.Copy(&m_stStatusOLD);

    return WFS_SUCCESS;
}

// 初始化Cen标准能力值类
HRESULT CXFS_PPR::InitCaps()
{
    m_sCaps.fwType = WFS_PTR_TYPEPASSBOOK;          // 设备类型
    m_sCaps.fwReadForm = WFS_PTR_READMSF;           // 具有读磁条功能
    m_sCaps.fwWriteForm = WFS_PTR_WRITETEXT | WFS_PTR_WRITEGRAPHICS;
    m_sCaps.fwControl = WFS_PTR_CTRLEJECT | WFS_PTR_CTRLFLUSH;
    m_sCaps.bMediaTaken = TRUE;
    m_sCaps.fwImageType = WFS_PTR_IMAGEBMP;

    // 回收设置
    if (m_stRetractCfg.nRetractSup == 0)    // 不支持回收
    {
        m_sCaps.usRetractBins = 0;              // 回收箱个数
        m_sCaps.lpusMaxRetract = nullptr;       // 每个回收箱可容纳媒介数目
    } else
    {
        m_sCaps.fwControl = (m_sCaps.fwControl | WFS_PTR_CTRLRETRACT);
        m_sCaps.usRetractBins = m_stRetractCfg.wRetBoxCount;              // 回收箱个数
        m_sCaps.lpusMaxRetract = new USHORT[m_stRetractCfg.wRetBoxCount];
        memset(m_sCaps.lpusMaxRetract, 0x00, sizeof(USHORT) * 1);
        m_sCaps.lpusMaxRetract[0] = m_stRetractCfg.dwRetractVol;
    }

    return WFS_SUCCESS;
}

void CXFS_PPR::RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData)
{
    ulOutSize = 0;
    int PAGELINE = m_stConfig.nLineSize;                                // 一行字符数
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
void CXFS_PPR::UpdateDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题
    //int     nPrinterStatus  = WFS_PTR_DEVHWERROR;

    INT     nRet = PTR_SUCCESS;

    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFirePaperStatus    = FALSE;
    BOOL    bNeedFireTonerStatus    = FALSE;
    BOOL    bNeedFireHWError        = FALSE;
    BOOL    bNeedFirePaperTaken     = FALSE;
    BOOL    bNeedFireMediaInserted  = FALSE;

    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log

    // 特定情形事件上报
    if (m_wNeedFireEvent[0] == 1)
    {
        if (m_stConfig.nReportHWErrEvent == 0)   // INI设定不上报WFS_SYSE_HARDWARE_ERROR事件
        {
            Log(ThisModule, __LINE__, "产生特定情形事件上报: HWEvent:%d,%d,INI设定不上报|",
                    WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
            m_wNeedFireEvent[0] = 0;
        } else
        {
            FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
            sprintf(szFireBuffer + strlen(szFireBuffer), "特定情形事件上报: HWEvent:%d,%d|",
                    WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
            m_wNeedFireEvent[0] = 0;
            m_stStatusOLD.fwDevice = WFS_PTR_DEVHWERROR;
        }
    }

    DEVPTRSTATUS stDevStatus;
    nRet = m_pPrinter->GetStatus(stDevStatus);

    m_stStatus.fwInk                  = WFS_PTR_INKNOTSUPP;
    m_stStatus.fwLamp                 = WFS_PTR_LAMPNOTSUPP;
    m_stStatus.lppRetractBins         = nullptr;
    m_stStatus.usMediaOnStacker       = 0;
    m_stStatus.lpszExtra              = nullptr;


    //WFSPTRSTATUS m_stStatusOLD;
    //memcpy(&m_stStatusOLD, &m_stStatus, sizeof(WFSPTRSTATUS));

    //----------------------设备状态处理----------------------
    switch (nRet)
    {
        // 打印机出纸口有纸设备状态为ONLINE
        case PTR_SUCCESS:
        case ERR_PTR_PARAM_ERR:
        case ERR_PTR_UNSUP_CMD:
            m_stStatus.fwDevice = WFS_PTR_DEVONLINE;
            break;
        case ERR_PTR_COMM_ERR:
        case ERR_PTR_NOT_OPEN:
        case ERR_PTR_NO_DEVICE:
            m_stStatus.fwDevice = WFS_PTR_DEVOFFLINE;
            break;
        default:
            m_stStatus.fwDevice = WFS_PTR_DEVHWERROR;
            break;
    }

    if (m_stStatus.fwDevice == WFS_PTR_DEVOFFLINE)
    {
        m_stStatus.fwMedia = WFS_PTR_MEDIAUNKNOWN;
        m_stStatus.fwPaper[0] = WFS_PTR_PAPERUNKNOWN;
    }

    // Device == ONLINE && 有命令在执行中,设置Device = BUSY
    if (m_stStatus.fwDevice == WFS_PTR_DEVONLINE && m_bCmdRuning == TRUE)    // 命令执行中
    {
        m_stStatus.fwDevice == WFS_PTR_DEVBUSY;
    }

    //----------------------Media状态处理----------------------
    m_stStatus.fwMedia = ConvertMediaStatus(stDevStatus.wMedia);
    if (m_stStatus.fwMedia == WFS_PTR_MEDIAJAMMED)
    {
        m_stStatus.fwDevice = WFS_PTR_DEVHWERROR;
    }

    //----------------------纸状态处理----------------------
    int nPaperStatus = ConvertPaperStatus(stDevStatus.wPaper[0]);

    if (m_stStatus.fwPaper[0] != (WORD)nPaperStatus)
    {
        m_stStatus.fwPaper[0] = (WORD)nPaperStatus;
    }
    for (int i = 1; i < WFS_PTR_SUPPLYSIZE; i++)
    {
        m_stStatus.fwPaper[i] = WFS_PTR_PAPERNOTSUPP;
    }

    //----------------------状态检查----------------------
    // Device状态变化检查
    if (m_stStatus.fwDevice != m_stStatusOLD.fwDevice)
    {
        bNeedFireStatusChanged = TRUE;
        if (m_stStatus.fwDevice == WFS_PTR_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }

    // Media状态变化检查
    if (m_WaitTaken == WTF_TAKEN)   // Taken事件检查
    {
        if (m_stStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT &&
            (m_stStatusOLD.fwMedia == WFS_PTR_MEDIAPRESENT || m_stStatusOLD.fwMedia == WFS_PTR_MEDIAENTERING))
        {
            bNeedFirePaperTaken = TRUE;
        }
    }

    // MediaInsert事件检查
    /*if (m_stStatusOLD.fwMedia == WFS_PTR_MEDIANOTPRESENT &&
        (m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT || m_stStatus.fwMedia == WFS_PTR_MEDIAENTERING))
    {
        bNeedFireMediaInserted = TRUE;
    }*/

    // 纸状态变化检查,只有当纸状态变为少或空时才Fire状态
    if (m_stStatusOLD.fwPaper[0] !=  m_stStatus.fwPaper[0])
    {
        if (m_stStatus.fwPaper[0] == WFS_PTR_PAPERLOW ||
            m_stStatus.fwPaper[0] == WFS_PTR_PAPEROUT ||
            m_stStatus.fwPaper[0] == WFS_PTR_PAPERFULL)
        {
            bNeedFirePaperStatus = TRUE;
        }
    }


    //----------------------事件上报----------------------
    if (bNeedFireHWError)
    {
        if (m_stConfig.nReportHWErrEvent == 0)   // INI设定不上报WFS_SYSE_HARDWARE_ERROR事件
        {
           Log(ThisModule, __LINE__, "产生HWEvent:%d,%d,INI设置不上报|",
               WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
        } else
        {
            FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
            sprintf(szFireBuffer + strlen(szFireBuffer), "HWEvent:%d,%d|",
                    WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
        }
    }

    if (bNeedFireStatusChanged)
    {
        FireStatusChanged(m_stStatus.fwDevice);
        sprintf(szFireBuffer + strlen(szFireBuffer), "StatusChange:%d|",  m_stStatus.fwDevice);
    }

    if (bNeedFirePaperStatus)
    {
        FirePaperThreshold(WFS_PTR_PAPERUPPER, m_stStatus.fwPaper[0]);
        sprintf(szFireBuffer + strlen(szFireBuffer), "PaperThreshold:%d|",  m_stStatus.fwPaper[0]);
    }

    if (bNeedFireTonerStatus)
    {
        FireTonerThreshold(m_stStatus.fwToner);
        sprintf(szFireBuffer + strlen(szFireBuffer), "TonerThreshold:%d|",  m_stStatus.fwToner);
    }

    if (bNeedFirePaperTaken)
    {
        FireMediaTaken();
        Log(ThisModule, __LINE__, "介质被取走(Taken)");
        sprintf(szFireBuffer + strlen(szFireBuffer), "PaperTaken|");
        m_WaitTaken = WTF_NONE;
    }

    if (bNeedFireMediaInserted == TRUE && m_bCmdRuning == TRUE)
    {
        FireMediaInserted();
        Log(ThisModule, __LINE__, "介质插入");
        sprintf(szFireBuffer + strlen(szFireBuffer), "MediaInsert|");
    }

    // 回收盒状态变化检查
    if (m_stRetractCfg.dwRetractCnt >= m_stRetractCfg.dwRetractVol)  // 已满
    {
        if (m_stRetractCfg.dwRetractCnt != m_dwRetractCntEvent)
        {
            FireRetractBinThreshold(m_stRetractCfg.wRetBoxCount, WFS_PTR_RETRACTBINFULL);
            m_dwRetractCntEvent = m_stRetractCfg.dwRetractVol;
            m_stRetractCfg.wRetractStat = 2;
            sprintf(szFireBuffer + strlen(szFireBuffer), "RetractBinThreshold(%d, FULL)|",
                    m_dwRetractCntEvent);
        }
    } else
    if (m_stRetractCfg.dwRetractCnt >= m_stRetractCfg.dwFullThreshold)  // 将满
    {
        if (m_stRetractCfg.dwRetractCnt != m_dwRetractCntEvent)
        {
            FireRetractBinThreshold(m_stRetractCfg.wRetBoxCount, WFS_PTR_RETRACTBINHIGH);
            m_dwRetractCntEvent = m_stRetractCfg.dwRetractCnt;
            m_stRetractCfg.wRetractStat = 1;
            sprintf(szFireBuffer + strlen(szFireBuffer), "RetractBinThreshold(%d, HIGH)|",
                    m_dwRetractCntEvent);
        }
    } else  // 正常
    {
        if (m_stRetractCfg.wRetractStat > 0)
        {
            FireRetractBinThreshold(m_stRetractCfg.wRetBoxCount, WFS_PTR_RETRACTBINOK);
            m_dwRetractCntEvent = m_stRetractCfg.dwRetractCnt;
            m_stRetractCfg.wRetractStat = 0;
            sprintf(szFireBuffer + strlen(szFireBuffer), "RetractBinThreshold(%d, OK)|",
                    m_dwRetractCntEvent);
        }
    }

    // 比较两次状态记录LOG
    //if (memcmp(&m_stStatusOLD, &m_stStatus, sizeof(WFSPTRSTATUS)) != 0)
    if (m_stStatus.Diff(m_stStatusOLD) == 1)
    {
        Log(ThisModule, __LINE__, "状态结果比较: Device:%d->%d%s|Media:%d->%d%s|Paper[0]:%d->%d%s|"
                            "Ink:%d->%d%s|Toner:%d->%d%s|Lamp:%d->%d%s|; 事件上报记录: %s.",
            m_stStatusOLD.fwDevice, m_stStatus.fwDevice, (m_stStatusOLD.fwDevice != m_stStatus.fwDevice ? " *" : ""),
            m_stStatusOLD.fwMedia, m_stStatus.fwMedia, (m_stStatusOLD.fwMedia != m_stStatus.fwMedia ? " *" : ""),
            m_stStatusOLD.fwPaper[0], m_stStatus.fwPaper[0], (m_stStatusOLD.fwPaper[0] != m_stStatus.fwPaper[0] ? " *" : ""),
            m_stStatusOLD.fwInk, m_stStatus.fwInk, (m_stStatusOLD.fwInk != m_stStatus.fwInk ? " *" : ""),
            m_stStatusOLD.fwToner, m_stStatus.fwToner, (m_stStatusOLD.fwToner != m_stStatus.fwToner ? " *" : ""),
            m_stStatusOLD.fwLamp, m_stStatus.fwLamp, (m_stStatusOLD.fwLamp != m_stStatus.fwLamp ? " *" : ""),
            szFireBuffer);
        m_stStatus.Copy(&m_stStatusOLD);
    }

    return;
}

// MB2打印数据下发
HRESULT CXFS_PPR::EndForm_MB2(DEVPRINTFORMIN stIn, DEVPRINTFORMOUT &stOut)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;
    DEVPTRFONTPAR stPrtFontPar;     // 打印参数

    // 根据设备类型选择打印参数设置
    stPrtFontPar.Clear();
    stPrtFontPar.dwMarkPar[0] = m_stConfig.stConfig_MB2.wPrintQuality;  // 打印质量
    stPrtFontPar.dwMarkPar[1] = stIn.lOtherParam[0];                    // 打印字距
    stPrtFontPar.dwMarkPar[2] = stIn.lOtherParam[2];                    // 其他
    m_pPrinter->SetData(&stPrtFontPar, DTYPE_SET_PRTFONT);

    /*nRet = m_pPrinter->PrintDataOrg(szPrintData, pItem->nTextLen, dwX, dwY);
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "下发打印数据: PrintDataOrg(%s, %d, %d, %d) fail, ErrCode:%d, Return: %d.",
            pItem->Text, pItem->nTextLen, dwX, dwY, nRet, ConvertErrCode(nRet));
    }*/

    nRet = m_pPrinter->PrintForm(stIn, stOut);
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "下发打印数据: PrintForm(PrintInPar[%d, %d, %d, %s, %d,%d]) fail, ErrCode:%d, Return: %d.",
            stIn.wInMode, stIn.ulX, stIn.ulY, stIn.lpData,
            stIn.dwDataSize, stIn.dwTimeOut, nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    return WFS_SUCCESS;
}

// PRM打印数据下发
HRESULT CXFS_PPR::EndForm_PRM(DEVPRINTFORMIN stIn, DEVPRINTFORMOUT &stOut)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;
    DEVPTRFONTPAR stPrtFontPar;     // 打印参数

    // 根据设备类型选择打印参数设置
    stPrtFontPar.Clear();
    stPrtFontPar.dwMarkPar[0] = stIn.lOtherParam[0];                    // CPI
    stPrtFontPar.dwMarkPar[1] = stIn.lOtherParam[1];                    // LPI
    stPrtFontPar.dwMarkPar[2] = stIn.lOtherParam[2];                    // 字体属性
    m_pPrinter->SetData(&stPrtFontPar, DTYPE_SET_PRTFONT);

    nRet = m_pPrinter->PrintForm(stIn, stOut);
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "下发打印数据: PrintForm(PrintInPar[%d, %d, %d, %s, %d,%d]) fail, ErrCode:%d, Return: %d.",
            stIn.wInMode, stIn.ulX, stIn.ulY, stIn.lpData,
            stIn.dwDataSize, stIn.dwTimeOut, nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    return WFS_SUCCESS;
}

// MB2读数据下发
HRESULT CXFS_PPR::EndReadForm_Pub(ReadContext *pContext)
{
    THISMODULE(__FUNCTION__);

    CSPPtrData *pData = (CSPPtrData *)m_pData;

    INT nRet = PTR_SUCCESS;
    HRESULT hRet = WFS_SUCCESS;
    BOOL bIsTrack1 = FALSE, bIsTrack2 = FALSE, bIsTrack3 = FALSE;
    BOOL bRetTrack1 = FALSE, bRetTrack2 = FALSE, bRetTrack3 = FALSE;
    WORD wReadTrackSize = 0;
    DEVPTRREADFORMIN stReadIn;
    DEVPTRREADFORMOUT stReadOut;

    CMultiString clFields = pContext->pReadData->lpszFieldNames;

    pData->m_ReadFormOut.Clear();

    //  如果参数为空，即未指定任何磁道，则要读所有在Form文件中的磁道
    if (clFields.GetCount() == 0)
    {
        for (INT i = 0; pContext->pForm->GetSubItemCount() > i; i ++)
        {
            clFields.Add(pContext->pForm->GetSubItem(i)->GetName());
        }
    }

    // 判断读磁道值并组织下发入参
    for(INT i = 0; i < clFields.GetCount(); i++)
    {
        if (MCMP_IS0(m_stConfig.szReadTrackName[0], clFields.GetAt(i))) // 读磁道1
        {
            bIsTrack1 = TRUE;
            wReadTrackSize ++;
        } else
        if (MCMP_IS0(m_stConfig.szReadTrackName[1], clFields.GetAt(i))) // 读磁道2
        {
            bIsTrack2 = TRUE;
            wReadTrackSize ++;
        } else
        if (MCMP_IS0(m_stConfig.szReadTrackName[2], clFields.GetAt(i))) // 读磁道3
        {
            bIsTrack3 = TRUE;
            wReadTrackSize ++;
        }
    }

    // 无有效入参
    if (bIsTrack1 == FALSE && bIsTrack2 == FALSE && bIsTrack3 == FALSE)
    {
        Log(ThisModule, __LINE__, "入参无效: FieldNames and Form[%s] is invalid = 0, Return: %d.",
            pContext->pReadData->lpszFormName, WFS_ERR_INVALID_DATA);

        pContext->pUserData = NULL;

        return WFS_ERR_INVALID_DATA;
    }

    // 等待插折
    DWORD dwTimeOut = pContext->dwTimeOut;
    hRet = MediaInsertWait(dwTimeOut);
    if (hRet != WFS_SUCCESS)
    {
        pContext->pUserData = NULL;
        return hRet;
    }

    // 读磁道1
    if (bIsTrack1 == TRUE)
    {
        pData->m_ReadFormOut.Add(m_stConfig.szReadTrackName[0], "");
        bRetTrack1 = TRUE;
    }

    // 读磁道2
    if (bIsTrack2 == TRUE)
    {
        stReadIn.Clear();
        stReadOut.Clear();
        stReadIn.wInMode = READFORM_TRACK2;
        stReadIn.dwTimeOut = dwTimeOut;
        stReadIn.lOtherParam[0] = m_stConfig.stConfig_MB2.wTrack2Type;
        nRet = m_pPrinter->ReadForm(stReadIn, stReadOut);
        if (nRet != PTR_SUCCESS)
        {
            pData->m_ReadFormOut.Clear();
            Log(ThisModule, __LINE__,
                "读磁道2数据: ReadForm(%d) fail, ErrCode:%d, Return: %d.",
                stReadIn.wInMode, nRet, ConvertErrCode(nRet));
            return ConvertErrCode(nRet);
        }
        if (strlen(stReadOut.szRetData) > 0)
        {
            pData->m_ReadFormOut.Add(m_stConfig.szReadTrackName[1], stReadOut.szRetData);
            bRetTrack2 = TRUE;
        } else
        {
            Log(ThisModule, __LINE__,
                "读磁道2数据: ReadForm(%d) succ, RetData < 1|读到空数据.", stReadIn.wInMode);
        }
    }

    // 读磁道3
    if (bIsTrack3 == TRUE)
    {
        stReadIn.Clear();
        stReadOut.Clear();
        stReadIn.wInMode = READFORM_TRACK3;
        stReadIn.dwTimeOut = dwTimeOut;
        stReadIn.lOtherParam[1] = m_stConfig.stConfig_MB2.wTrack3Type;
        nRet = m_pPrinter->ReadForm(stReadIn, stReadOut);
        if (nRet != PTR_SUCCESS)
        {
            pData->m_ReadFormOut.Clear();
            Log(ThisModule, __LINE__,
                    "读磁道3数据: ReadForm(%d) fail, ErrCode:%d, Return: %d.",
                    stReadIn.wInMode, nRet, ConvertErrCode(nRet));
            return ConvertErrCode(nRet);
        }
        if (strlen(stReadOut.szRetData) > 0)
        {
            pData->m_ReadFormOut.Add(m_stConfig.szReadTrackName[2], stReadOut.szRetData);
            bRetTrack3 = TRUE;
        } else
        {
            Log(ThisModule, __LINE__,
                "读磁道3数据: ReadForm(%d) succ, RetData < 1|读到空数据.", stReadIn.wInMode);
        }
    }

    // 读数据结果检查
    if (wReadTrackSize == 3)    // 需要读3个磁道
    {
        if (bRetTrack1 == FALSE || bRetTrack2 == FALSE || bRetTrack3 == FALSE)
        {
            Log(ThisModule, __LINE__,
                "读数据结果检查: 需要读3个磁道,读结果=%d|%d|%d, Return: %d.",
                bRetTrack1, bRetTrack2, bRetTrack3, WFS_ERR_PTR_MEDIAINVALID);
            pData->m_ReadFormOut.Clear();
            return WFS_ERR_PTR_MEDIAINVALID;
        }
    } else
    if (wReadTrackSize == 2)    // 需要读2个磁道
    {
        if ((bRetTrack1 == FALSE && bRetTrack2 == FALSE) ||
            (bRetTrack1 == FALSE && bRetTrack3 == FALSE) ||
            (bRetTrack2 == FALSE && bRetTrack3 == FALSE))
        {
            Log(ThisModule, __LINE__,
                "读数据结果检查: 需要读2个磁道,读结果=%d|%d|%d, Return: %d.",
                bRetTrack1, bRetTrack2, bRetTrack3, WFS_ERR_PTR_MEDIAINVALID);
            pData->m_ReadFormOut.Clear();
            return WFS_ERR_PTR_MEDIAINVALID;
        }
    } else
    {
        if (bRetTrack1 == FALSE && bRetTrack2 == FALSE && bRetTrack3 == FALSE)
        {
            Log(ThisModule, __LINE__,
                "读数据结果检查: 需要读1个磁道,读结果=%d|%d|%d, Return: %d.",
                bRetTrack1, bRetTrack2, bRetTrack3, WFS_ERR_PTR_MEDIAINVALID);
            pData->m_ReadFormOut.Clear();
            return WFS_ERR_PTR_MEDIAINVALID;
        }
    }

    // 读折成功后,执行MediaControl
    if (nRet == PTR_SUCCESS)
    {
        DWORD dwMediaControl = pContext->pReadData->dwMediaControl;
        if (dwMediaControl != 0)
        {
            hRet = InnerMediaControl(dwMediaControl);
        }
    }

    // 删除自定义数据
    /*if (pItems)
    {
        delete pItems;
        pItems = NULL;
    }*/
    pContext->pUserData = NULL;

    if (hRet != WFS_SUCCESS)
    {
        return hRet;
    }

    return WFS_SUCCESS;
}

