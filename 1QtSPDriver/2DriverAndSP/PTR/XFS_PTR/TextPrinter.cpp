#include "TextPrinter.h"
#include <QTextCodec>
#include <unistd.h>
#include <QImage>

/***************************宏定义****************************************/
// 事件日志
#define ThisFile                    "SPPrinter"

// 错误描述
#define IDS_ERR_ON_EXECUTE_ERR              "调用[%s]失败[%s]"
#define IDS_ERR_FORM_INVALID                "FORM无效[%s]"
#define IDS_ERR_MEDIA_INVALID               "MEDIA无效[%s]"
#define IDS_ERR_FORM_NOT_FOUND              "FORM[%s]没有找到"
#define IDS_ERR_MEDIA_NOT_FOUND             "MEDIA[%s]没有找到"
#define IDS_ERR_FILED_NOT_FOUND             "FIELD[%s]没有找到"
#define IDS_ERR_MEDIA_OVERFLOW              "打印FORM[%s]到MEDIA[%s]上溢出"
#define IDS_ERR_MEDIA_OVERFLOW_PRINTAREA    "打印FORM[%s]到MEDIA[%s]上溢出打印区域"
#define IDS_ERR_FIELD_ERROR                 "字段域[%s]错误"
#define IDS_ERR_START_FORM                  "调用StartForm[%s]失败[%s]"
#define IDS_ERR_REQUIRED_FIELD              "打印FORM[%s]的字段[%s]没有提供数据"
#define IDS_ERR_PRINT_FIELD                 "打印FORM[%s]字段[%s]失败[%s]"
#define IDS_ERR_NO_CHARSET                  "系统缺少中文字符集"


#define IDS_ERR_GetTwipsPerRowCol   "GetTwipsPerRowCol()结果非法[%d, %d]"
#define IDS_ERR_FIELD_OVERFLOW      "FORM[%s]的字段[%s溢出]"
#define IDS_ERR_FIELD_OVERLFLOW_NUM "FORM[%s]的字段数目溢出[%d]"
#define IDS_ERR_FIELD_INVALID_PATH  "图片形字段域路径非法(最大长度1024)[%s]"

// 功能：转换整数为字串
LPCSTR IntToStr(int i)
{
    static char buf[20];
    sprintf(buf, "\"%d\"", i);
    return buf;
}

// 功能: 矩形偏移
void OffsetRect(LPRECT rc, int dx, int dy)
{
    rc->left  += dx;
    rc->right += dx;
    rc->top += dy;
    rc->bottom += dy;
}

// 功能：返回命令相对应的函数名
LPCSTR CmdToFuncName(DWORD dwCommand)
{
    switch (dwCommand)
    {
    case WFS_CMD_PTR_CONTROL_MEDIA:
        return "ControlMedia";
    case WFS_CMD_PTR_PRINT_FORM:
        return "PrintForm";
    case WFS_CMD_PTR_READ_FORM:
        return "ReadForm";
    case WFS_CMD_PTR_RAW_DATA:
        return "SendRawData";
    case WFS_CMD_PTR_MEDIA_EXTENTS:
        return "GetMediaExtents";
    case WFS_CMD_PTR_RESET_COUNT:
        return "ResetRetractBinCount";
    case WFS_CMD_PTR_READ_IMAGE:
        return "ReadImage";
    case WFS_CMD_PTR_RESET:
        return "Reset";
    case WFS_CMD_PTR_RETRACT_MEDIA:
        return "RetractMedia";
    case WFS_CMD_PTR_DISPENSE_PAPER:
        return "DispensePaper";
    default:
        return IntToStr(dwCommand);
    }
}

// 返回信息命令对应的函数名
LPCSTR CategoryToFuncName(DWORD dwCategory)
{
    switch (dwCategory)
    {
    case WFS_INF_PTR_STATUS:
        return "GetStatus";
    case WFS_INF_PTR_CAPABILITIES:
        return "Capabilities";
    case WFS_INF_PTR_FORM_LIST:
        return "LoadForms";
    case WFS_INF_PTR_MEDIA_LIST:
        return "LoadMedias";
    case WFS_INF_PTR_QUERY_FORM:
        return "FindForm";
    case WFS_INF_PTR_QUERY_MEDIA:
        return "FindMedia";
    case WFS_INF_PTR_QUERY_FIELD:
        return "FindField";
    default:
        return IntToStr(dwCategory);
    }
}

int MulDiv(int number, int numberator, int denominator)
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

CTextPrinter::CTextPrinter()
{
    SetLogFile(LOGFILE, ThisFile, "PTR");
    m_pData = nullptr;
    m_DevType = DEV_SNBC_BKC310;
}

CTextPrinter::~CTextPrinter()
{
    if (nullptr != m_pData)
    {
        delete m_pData;
        m_pData = nullptr;
    }
}

void CTextPrinter::InitPTRData(LPCSTR lpLogicalName)
{
    if (m_pData == nullptr)
    {
        m_pData = new CSPPtrData(lpLogicalName);
    }
}

// 转换处理结果为事件日志的错误码
int CTextPrinter::Result2ErrorCode(HRESULT hResult)
{
    switch (hResult)
    {
    case WFS_SUCCESS:
    case WFS_ERR_CANCELED:
    case WFS_ERR_TIMEOUT:
        return 0;
    default:
        return (-1);
    }
}

SIZE CTextPrinter::GetTwipsPerRowCol()
{
    SIZE s;
    s.cx = 85;
    s.cy = 170;
    return s;
}

// 功能：在多字串中查找对应名字的字段值
LPCSTR CTextPrinter::FindString(CMultiString &ms, LPCSTR lpszName, int index)
{
    char szName[1024] = {0};
    char szTmpBuf[1024] = {0};

    sprintf(szName, "%s[%d]", lpszName, index);
    const char *p = NULL;
    for (int i = 0; i < ms.GetCount(); i++)
    {
        memset(szTmpBuf, 0, sizeof(szTmpBuf));
        if (0 == index)
        {
            // 得到要比较的FORM中定义的字段值长度,用来下一步比较,不能以传进来的字段.
            // 否则会引起字段名包含比较时情况不能区分.如字段名"TransferTo"与"Transfer"
            strcpy(szTmpBuf, ms.GetAt(i));
            int j = 0;
            for (; j <= (int)strlen(szTmpBuf); j++)
            {
                if ('=' == szTmpBuf[j])
                {
                    break;
                }
            }

            //  比较名称的长度必须是较长的那一个，
            // 否则可能发生一个名称包含另外一个名称而出错
            if (0 == strncmp(ms.GetAt(i), lpszName, (j > (int)strlen(lpszName) ? j : strlen(lpszName))))
            {
                p = ms.GetAt(i);
                break;
            }
        }

        if (0 == strncmp(ms.GetAt(i), szName, strlen(szName)))
        {
            p = ms.GetAt(i);
            break;
        }
    }

    if (!p)
    {
        return NULL;
    }

    static char c = 0;
    p = strchr(p, '=');
    if (!p)
    {
        return &c;
    }
    p++;
    while (*p && isspace((BYTE)*p))
    {
        p++;
    }

    return p;
}

// 功能：打印字段内容或FRAME
HRESULT CTextPrinter::PrintFieldOrFrame(PrintContext &pc, ISPPrinterItem *pItem, const SIZE offset, CMultiString &Fields)
{
    const char *const ThisModule = "PrintFieldOrFrame";
    HRESULT hRes = WFS_SUCCESS;

    RECT rc;
    SIZE sizeItem = pItem->GetSize();
    SIZE3 posItem = pItem->GetPosition();
    rc.left = offset.cx + posItem.cx;
    rc.top = offset.cy + posItem.cy;
    rc.right = rc.left + sizeItem.cx;// 功能: 矩形偏移
    rc.bottom = rc.top + sizeItem.cy;

    // ADD BY SUTX,20090624打印FORM"%s"的字段"%s"没有提供数据
    // 如果域定义超过Form大小，则报错
    do
    {
        SIZE sizeForm = pc.pForm->GetOrigSize();
        SIZE sizeItem = pItem->GetOrigSize();
        SIZE3 sizePosItem = pItem->GetOrigPosition();
        if (sizePosItem.cx + sizeItem.cx > sizeForm.cx ||
            sizePosItem.cy + sizeItem.cy > sizeForm.cy)
        {
            Log(ThisModule, -1, IDS_ERR_FORM_INVALID, pc.pForm->GetName());
            return WFS_ERR_PTR_FORMINVALID;
        }
    } while (0);


    if (ITEM_FIELD == pItem->GetItemType())
    {
        pc.pField = (ISPPrinterField *)pItem;
        pc.pFrame = NULL;
        if (pc.pField->GetAccess() & ACCESS_WRITE)
        {
            SIZE sizeOffset = pc.pField->GetRepeatOffset();

            QTextCodec *codec = QTextCodec::codecForLocale();
            QTextCodec *codec1 = QTextCodec::codecForName("gb18030");
            if (NULL == codec1) codec1 = QTextCodec::codecForName("gb2312");
            if (NULL == codec1) codec1 = QTextCodec::codecForName("gbk");
            if (NULL == codec1)
            {
                Log(ThisModule, -1, IDS_ERR_NO_CHARSET);
                return WFS_ERR_PTR_CHARSETDATA;
            }

            for (int index = 0; index <= (int)pc.pField->GetRepeatCount(); index++)
            {
                LPCSTR lpszText = FindString(Fields, pc.pField->GetName(), index);
                if (!lpszText)
                {
                    lpszText = pc.pField->GetInitValue();
                    if (NULL != lpszText)
                    {
                        // INITIALVALUE字段配置的是""，那么这个域的值也是空的不打印
                        if (0 == strcmp(lpszText, "\"\x0D") || 0 == strcmp(lpszText, "\""))
                        {
                            lpszText = NULL;
                        }
                    }
                }

                if (lpszText)
                {
                    switch (pc.pField->GetFieldType())
                    {
                    case FT_TEXT: // (default)
                        {
                            // 转码
                            QString strText = QString::fromUtf8((char *)lpszText);
                            QTextCodec::setCodecForLocale(codec1);
                            QByteArray tmpData = strText.toLocal8Bit();
                            char *pTempCode = tmpData.data();
                            int nTempLen = tmpData.size();
                            char *pBuf = new char[nTempLen + 2];
                            memset(pBuf, '\0', nTempLen + 2);
                            memcpy(pBuf, pTempCode, nTempLen);
                            QTextCodec::setCodecForLocale(codec);
                            hRes = DrawText(&pc, pBuf, &rc);

                            delete [] pBuf;

                        }
                        break;
                    case FT_BARCODE:
                        {
                            // 转码
                            QString strText = QString::fromUtf8((char *)lpszText);
                            QTextCodec::setCodecForLocale(codec1);
                            QByteArray tmpData = strText.toLocal8Bit();
                            char *pTempCode = tmpData.data();
                            int nTempLen = tmpData.size();
                            char *pBuf = new char[nTempLen + 2];
                            memset(pBuf, '\0', nTempLen + 2);
                            memcpy(pBuf, pTempCode, nTempLen);
                            QTextCodec::setCodecForLocale(codec);
                            hRes = DrawBarcode(&pc, pBuf, &rc, pc.pField->GetBarcodePos());
                            delete [] pBuf;
                        }
                        break;
                    case FT_GRAPHIC:
                        {
                            QTextCodec::setCodecForLocale(codec1);
                            QString strText = QString::fromLocal8Bit((char *)lpszText);
                            QByteArray tmpData = strText.toUtf8();
                            char *pTempCode = tmpData.data();
                            int nTempLen = tmpData.size();
                            char *pBuf = new char[nTempLen + 2];
                            memset(pBuf, '\0', nTempLen + 2);
                            memcpy(pBuf, pTempCode, nTempLen);
                            QTextCodec::setCodecForLocale(codec);

                            if (access(pBuf, F_OK) == 0)
                            {
                                hRes = DrawGraph(&pc, pBuf, &rc, pc.pField->GetScaling());
                                delete [] pBuf;
                            }
                            else
                            {
                                delete [] pBuf;
                                Log(ThisModule, -1, IDS_ERR_FIELD_ERROR, lpszText);
                                FireFieldError(pc.pForm->GetName(), pc.pField->GetName(), WFS_PTR_FIELDREQUIRED);
                                hRes = WFS_ERR_PTR_FIELDERROR;
                            }
                        }
                        break;
                    case FT_MICR:
                    case FT_OCR:
                    case FT_MSF:
                        break;
                    case FT_PAGEMARK:
                        break;
                    }
                }
                else if (CLASS_REQUIRED == pc.pField->GetClass())
                {
                    Log(ThisModule, -1, IDS_ERR_REQUIRED_FIELD, pc.pForm->GetName(), pc.pField->GetName());
                    FireFieldError(pc.pForm->GetName(), pc.pField->GetName(), WFS_PTR_FIELDREQUIRED);
                    hRes = WFS_ERR_PTR_FIELDERROR;
                }
                if (WFS_SUCCESS != hRes)
                {
                    Log(ThisModule, Result2ErrorCode(hRes), IDS_ERR_PRINT_FIELD, \
                        pc.pForm->GetName(), pc.pField->GetName(), "");
                    break;
                }
                OffsetRect(&rc, sizeOffset.cx, sizeOffset.cy);
            }
        }
    }
    else
    {
        pc.pFrame = (ISPPrinterFrame *)pItem;
        pc.pField = NULL;
        SIZE RepeatOffset, RepeatCount;
        RepeatCount.cx = pc.pFrame->GetRepeatX((DWORD *)&RepeatOffset.cx);
        RepeatCount.cy = pc.pFrame->GetRepeatY((DWORD *)&RepeatOffset.cy);
        for (int y = 0; y < RepeatCount.cy; y++)
        {
            RECT OldRect = rc;
            for (int x = 0; x < RepeatCount.cx; x++)
            {
                DrawFrame(&pc, &rc, pc.pFrame->GetFrameType());
            }
            rc = OldRect;
            OffsetRect(&rc, 0, RepeatOffset.cy);
        }
    }

    return hRes;
}


//////////////////////

int CTextPrinter::ComparePrintItem(const void *elem1, const void *elem2)
{
    PRINT_ITEM *pItem1 = *(PRINT_ITEM **)elem1;
    PRINT_ITEM *pItem2 = *(PRINT_ITEM **)elem2;
    if (pItem1->y < pItem2->y)
    {
        return -1;
    }
    else if (pItem1->y > pItem2->y)
    {
        return 1;
    }
    else
    {}

    if (pItem1->x < pItem2->x)
    {
        return -1;
    }
    else if (pItem1->x > pItem2->x)
    {
        return 1;
    }
    else
    {}

    return 0;
}

HRESULT CTextPrinter::StartForm(PrintContext *pContext)
{
    //    if (m_pPrinter->IsJournalPrinter())
    //    {
    //        DWORD dwControl = pContext->pPrintData->media_control;
    //        if (WFS_PTR_CTRLFLUSH == dwControl || 0 == dwControl)
    //        {
    //            return CXFS_PTR::StartForm(pContext);
    //        }
    //        else
    //        {
    //            Log(ThisModule, -1, IDS_ERR_JPR_UNSUPP_COMMAND, dwControl);
    //            return WFS_ERR_UNSUPP_DATA;
    //        }
    //    }
    pContext->pUserData = new PRINT_ITEMS;
    return WFS_SUCCESS;
}

HRESULT CTextPrinter::EndForm(PrintContext *pContext)
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
            /*if (('\n' == szPrintData[(dwDataSize > 0) ? dwDataSize - 1 : 0]) &&
                    (dwDataSize < MAX_PRINTDATA_LEN))
            {
                szPrintData[dwDataSize] = '\n';
            }*/
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

            if (m_DevType == DEV_SNBC_BKC310 || m_DevType == DEV_SNBC_BTNH80)
            {
                s.m_nCurCol = 0;
                s.m_nCurRow += pItem->nHeight;
                nRet = PrintImageOrg(pItem->strImagePath, pItem->x, pItem->y);
            } else
            {
                s.m_nCurCol += pItem->nWidth;
                // 打印格式数据(空格和字符)，定位图片文件位置
                nRet = PrintString(strFormat.GetData(), strFormat.GetLen());
                nRet = PrintImage(pItem->strImagePath, pItem->nDstImgWidth, pItem->nDstImgHeight);
            }

            //nRet = PrintImage(pItem->strImagePath, pItem->nDstImgWidth, pItem->nDstImgHeight);
            bAppendNewLine = TRUE;
        }
        else // 字符串打印
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
    //    if (0 > nRet)
    //    {
    //        return nRet;
    //    }

    //    if (0 <= nRet && bAppendNewLine)
    //    {
    //        nRet = PrintString("\n", 1);
    //    }

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


HRESULT CTextPrinter::DrawText(PrintContext *pContext, LPCSTR szText, LPRECT pRect)
{
    const char *const ThisModule = "DrawText";

    PRINT_ITEMS &PrintItems = *(PRINT_ITEMS *)pContext->pUserData;

    // 得到行列的TWIPS
    SIZE sTwips = GetTwipsPerRowCol();
    if (0 == sTwips.cx || 0 == sTwips.cy)
    {
        Log(ThisModule, -1, IDS_ERR_GetTwipsPerRowCol, sTwips.cx, sTwips.cy);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 得到打印坐标
    RECT rc = *pRect;
    ISPPrinterField *pFollow = pContext->pField->GetFollows();
    if (pFollow)
    {
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;
        rc.left = pFollow->GetPosition().cx + pFollow->GetSize().cx;
        rc.right = rc.left + w;
        rc.top = pFollow->GetPosition().cy + pFollow->GetSize().cy;
        rc.bottom = rc.top + h;
    }

    // 修改坐标单位为行列
    rc.left /= sTwips.cx;
    rc.right /= sTwips.cx;
    rc.top /= sTwips.cy;
    rc.bottom /= sTwips.cy;

    // 修改文本为大写或小写
    char Text[1024] = {'\0'};
    memset(Text, '\0', sizeof(Text));
    strncpy(Text, szText, sizeof(Text) - 1);
    Text[sizeof(Text) - 1] = 0;
    char *pText = Text;
    switch (pContext->pField->GetCASE())
    {
    case CASE_UPPER:
        strupr(Text);
        break;
    case CASE_LOWER:
        strlwr(Text);
        break;
    default:
        break;
    }

    //  格式化输出
    char FormatResult[2096];
    LPCSTR pFormat = pContext->pField->GetFormat();
    if (pFormat && pFormat[0])
    {
        sprintf(FormatResult, pFormat, pText);
        pText = FormatResult;
    }

    // 根据对齐方式、溢出处理方式等计算实际位置和输出的字串
    tagOVERFLOW o = pContext->pField->GetOverflow();
    int nLen = strlen(pText);
    int nWidthMultiplier = 1;       // 文本所占宽度
    int nHeight = 1;                // 文本所占高度
    DWORD dwStyle = pContext->pField->GetStyle();
    if (NeedFormatString()) // 根据风格调整文本的宽度和高度
    {
        if (dwStyle & FS_DOUBLE)
        {
            nWidthMultiplier *= 2;
        }
        if (dwStyle & FS_TRIPLE)
        {
            nWidthMultiplier *= 3;
        }
        if (dwStyle & FS_QUADRUPLE)
        {
            nWidthMultiplier *= 4;
        }
        if (dwStyle & FS_DOUBLEHIGH)
        {
            nHeight *= 2;
        }
        if (dwStyle & FS_TRIPLEHIGH)
        {
            nHeight *= 3;
        }
        if (dwStyle & FS_QUADRUPLEHIGH)
        {
            nHeight *= 4;
        }
    }
    if (nLen > rc.right - rc.left)  // 处理水平方向位置
    {
        switch (o)
        {
        case OF_TERMINATE:
            {
                Log(ThisModule, -1, IDS_ERR_FIELD_OVERFLOW, \
                    pContext->pForm->GetName(), pContext->pField->GetName());
                FireFieldError(pContext->pForm->GetName(), pContext->pField->GetName(), \
                               WFS_PTR_FIELDOVERFLOW);
                return WFS_ERR_PTR_FIELDERROR;
            }
        case OF_TRUNCATE:
        case OF_BESTFIT:
        case OF_WORDWRAP:
            {
                nLen = (rc.right - rc.left) / nWidthMultiplier;
                nLen = SubStrLen(pText, nLen);
                pText[nLen] = 0;
                if (OF_TRUNCATE == o)
                {
                    FireFieldWarning(pContext->pForm->GetName(), pContext->pField->GetName(), \
                                     WFS_PTR_FIELDOVERFLOW);
                }
            }
            break;
        default:
            {}
        }
    }
    if (0 == nLen)
    {
        return WFS_SUCCESS;
    }

    SIZE sPos = {rc.left, rc.top};
    //  SIZE sPos = {rc.left, rc.bottom};
    switch (pContext->pField->GetHorizAlign())
    {
    case HCENTER:
        sPos.cx = rc.left + (rc.right - rc.left - nLen) / 2;
        break;
    case RIGHT:
        sPos.cx = rc.right - nLen;
        break;
    case LEFT:
    default:
        break;
    }
    if (0 > sPos.cx)
    {
        //todo: delete sPos.cx characters of pText
        sPos.cx = 0;
    }
    // todo: 暂不处理 pContext->pField->GetVertAlign();
    SIZE sMediaSize = pContext->pMedia->GetSize();
    sMediaSize.cx /= sTwips.cx;
    sMediaSize.cy /= sTwips.cy;

    // 处理溢出
    BOOL bOverflow = FALSE;
    if (0 != sMediaSize.cx)
    {
        if (0 > sPos.cx ||
            sPos.cx >= sMediaSize.cx ||
            sPos.cx + nLen > sMediaSize.cx)
        {
            bOverflow = TRUE;
        }
    }
    if (0 != sMediaSize.cy)
    {
        if (0 > sPos.cy ||
            sPos.cy >= sMediaSize.cy ||
            sPos.cy + 1 > sMediaSize.cy)
        {
            bOverflow = TRUE;
        }
    }
    if (bOverflow)
    {
        if (OF_TERMINATE == pContext->pField->GetOverflow())
        {
            Log(ThisModule, -1, IDS_ERR_FIELD_OVERFLOW, \
                pContext->pForm->GetName(), pContext->pField->GetName());
            FireFieldError(pContext->pForm->GetName(), pContext->pField->GetName(), \
                           WFS_PTR_FIELDOVERFLOW);
            return WFS_ERR_PTR_FIELDERROR;
        }
    }

    if (0 > sPos.cy)
    {
        sPos.cy = 0;
    }
    if (MAX_ITEM <= PrintItems.nItemNum)
    {
        Log(ThisModule, -1, IDS_ERR_FIELD_OVERLFLOW_NUM, \
            pContext->pForm->GetName(), PrintItems.nItemNum);
        return WFS_ERR_INTERNAL_ERROR;
    }
    PRINT_ITEM *&pItem = PrintItems.pItems[PrintItems.nItemNum++];
    int nAllocLen = sizeof(PRINT_ITEM) + nLen;
    if (FALSE != NeedFormatString())
    {
        nAllocLen += MAX_FORMAT_LEN;
    }
    pItem = (PRINT_ITEM *)new char[nAllocLen];
    pItem->x = sPos.cx;
    pItem->y = sPos.cy;
    pItem->nWidth = nLen * nWidthMultiplier;
    pItem->nHeight = nHeight;
    pItem->nFieldType = FT_TEXT;
    memset(pItem->strImagePath, '\0', sizeof(pItem->strImagePath));
    char *p = pItem->Text;
    LPCSTR pFont = pContext->pField->GetFontName();
    int nFontSize = pContext->pField->GetFontSize();
    COLORREF cColor = pContext->pField->GetColor();
    int nCPI = pContext->pField->GetCPI();
    int nLPI = pContext->pField->GetLPI();
    if (FALSE != NeedFormatString())
    {
        if (NULL != pFont && 0 != pFont[0]) // 字库名
        {
            *p++ = '\x01';
            strcpy(p, pFont);
            p += strlen(p) + 1;
        }
        if (0 < nFontSize)                  // 字体大小
        {
            sprintf(p, "\x02%c", nFontSize);
            p += 2;
        }
        if (0 != cColor)                    // 字体颜色
        {
            sprintf(p, "\x03%c%c%c", GetRValue(cColor), GetGValue(cColor), GetBValue(cColor));
            p += 4;
        }
        if (FS_NORMAL != dwStyle)           //风格
        {
            *p++ = '\x04';
            *(DWORD *)(p) = dwStyle;
            p += 4;
        }
        if (0 != nCPI || 0 != nLPI)         //行间距和字间距
        {
            sprintf(p, "\x05%c%c", nCPI, nLPI);
            p += 3;
        }
    }
    memcpy(p, pText, nLen);
    p += nLen;
    if (FALSE != NeedFormatString())    // 取消格式
    {
        if (NULL != pFont && 0 != pFont[0]) // 字库名
        {
            *p++ = '\x01';
            *p++ = '\x00';
        }
        if (0 < nFontSize)                  // 字体大小
        {
            sprintf(p, "\x02%c", 0x00);
            p += 2;
        }
        if (0 != cColor)                    // 字体颜色
        {
            sprintf(p, "\x03%c%c%c", 0x00, 0x00, 0x00);
            p += 4;
        }
        if (FS_NORMAL != dwStyle)           // 风格
        {
            *p++ = '\x04';
            *(DWORD *)(p) = 0x00;
            p += 4;
        }
        if (0 != nCPI || 0 != nLPI)         // 行间距和字间距
        {
            sprintf(p, "\x05%c%c", 0x00, 0x00);
            p += 3;
        }
    }
    pItem->nTextLen = p - pItem->Text;

    return WFS_SUCCESS;
}

HRESULT CTextPrinter::DrawGraph(PrintContext *pContext, LPCSTR szImagePath, LPRECT pRect, SCALING ScaleMode)
{
    const char *ThisModule = "DrawGraph";
    PRINT_ITEMS &PrintItems = *(PRINT_ITEMS *)pContext->pUserData;
    if (MAX_ITEM <= PrintItems.nItemNum)
    {
        Log(ThisModule, -1, IDS_ERR_FIELD_OVERLFLOW_NUM, \
            pContext->pForm->GetName(), PrintItems.nItemNum);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 得到行列的TWIPS
    SIZE sTwips = GetTwipsPerRowCol();
    if (0 == sTwips.cx || 0 == sTwips.cy)
    {
        Log(ThisModule, -1, IDS_ERR_GetTwipsPerRowCol, \
            sTwips.cx, sTwips.cy);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 得到打印坐标
    RECT rc = *pRect;
    ISPPrinterField *pFollow = pContext->pField->GetFollows();
    if (pFollow)
    {
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;
        rc.left = pFollow->GetPosition().cx + pFollow->GetSize().cx;
        rc.right = rc.left + w;
        rc.top = pFollow->GetPosition().cy + pFollow->GetSize().cy;
        rc.bottom = rc.top + h;
    }

    // 修改坐标单位为行列
    rc.left /= sTwips.cx;
    rc.right /= sTwips.cx;
    rc.top /= sTwips.cy;
    rc.bottom /= sTwips.cy;

    // 分配新的打印ITEM保存数据
    PRINT_ITEM *&pItem = PrintItems.pItems[PrintItems.nItemNum++];
    int nAllocLen = sizeof(PRINT_ITEM);
    pItem = (PRINT_ITEM *)new char[nAllocLen];
    pItem->x = rc.left;
    pItem->y = (m_DevType == DEV_HOTS ? rc.bottom : rc.top);
    pItem->nWidth = rc.right - rc.left;
    pItem->nHeight = rc.bottom - rc.top;
    pItem->nFieldType = FT_GRAPHIC;
    pItem->nTextLen = 0;

    // 获取LPI和CPI值，若为0使用默认值
    int nCPI = pContext->pField->GetCPI();
    if (0 == nCPI)
    {
        nCPI = 8;
    } // 203dpi
    int nLPI = pContext->pField->GetLPI();
    if (0 == nLPI)
    {
        nLPI = 3;
    }   // ks80默认32点行

    QString strTemp(szImagePath);
    // strTemp = strTemp.replace("\\\\", "/");

    QImage image(strTemp);
    if (image.isNull())
    {
        Log(ThisModule, -1, IDS_ERR_FIELD_INVALID_PATH, \
            pContext->pField->GetName());
        return WFS_ERR_PTR_FIELDERROR;
    }
    else
    {
        memset(pItem->strImagePath, '\0', sizeof(pItem->strImagePath));
        QByteArray tmpData = strTemp.toUtf8();
        if (tmpData.size() >= sizeof(pItem->strImagePath))
        {
            Log(ThisModule, -1, IDS_ERR_FIELD_INVALID_PATH, \
                pContext->pField->GetName());
            return WFS_ERR_PTR_FIELDERROR;
        }
        else
        {
            memcpy(pItem->strImagePath, tmpData.data(), tmpData.size());
        }
    }

    // 把form中表示图片宽度的字符数转换成点
    int nFormWidth  = (pItem->nWidth * 1440) / (nCPI * 20);
    int nFormHeight = (pItem->nHeight * 1440) / (nLPI * 20);

    switch (ScaleMode)
    {
    case SCALING_BESTFIT:        // form中指定的高宽
        {
            pItem->nDstImgWidth = nFormWidth;
            pItem->nDstImgHeight = nFormHeight;
        }
        break;
    case SCALING_ASIS:           // 保持图片本身大小
        {
            pItem->nDstImgWidth = 0;
            pItem->nDstImgHeight = 0;
        }
        break;
    case SCALING_MAINTAINASPECT: // 保持图片原高宽比
        {
            // 图片自适应算法
            //1)设容器宽为W，高为H，则宽高比例为W/H=A；
            //2)设被加载图片宽为W'，高为H',则宽高比例为W'/H'=A'
            //3)设修正后的被加载图片宽为W''，高为H''。
            if (image.width() *nFormHeight > nFormWidth * image.height())  // A' > A     W''=W， H''=W/(W'/H')
            {
                pItem->nDstImgWidth = nFormWidth;
                pItem->nDstImgHeight = (nFormWidth * image.height()) / image.width();
            }
            else if (image.width() *nFormHeight < nFormWidth * image.height()) // A'  < A    H''=H， W''=H(W’/H')
            {
                pItem->nDstImgWidth = (nFormHeight * image.width()) / image.height();
                pItem->nDstImgHeight = nFormHeight;
            }
            else      // A' = A     H''=H   W''=W
            {
                pItem->nDstImgWidth = 0;
                pItem->nDstImgHeight = 0;
            }
        }
        break;
    }

    return WFS_SUCCESS;
}

HRESULT CTextPrinter::DrawBarcode(PrintContext *pContext, LPCSTR szCode, LPRECT pRect, BARCODEPOS BarcodeMode)
{
    const char *ThisModule = "DrawBarcode";
    if (FALSE == NeedFormatString())    // 只有格式化字串才打印BARCODE
    {
        return WFS_SUCCESS;
    }
    PRINT_ITEMS &PrintItems = *(PRINT_ITEMS *)pContext->pUserData;
    if (MAX_ITEM <= PrintItems.nItemNum)
    {
        Log(ThisModule, -1, IDS_ERR_FIELD_OVERLFLOW_NUM, \
            pContext->pForm->GetName(), PrintItems.nItemNum);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 得到行列的TWIPS
    SIZE sTwips = GetTwipsPerRowCol();
    if (0 == sTwips.cx || 0 ==  sTwips.cy)
    {
        Log(ThisModule, -1, IDS_ERR_GetTwipsPerRowCol, \
            sTwips.cx, sTwips.cy);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 得到打印坐标
    RECT rc = *pRect;
    ISPPrinterField *pFollow = pContext->pField->GetFollows();
    if (pFollow)
    {
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;
        rc.left = pFollow->GetPosition().cx + pFollow->GetSize().cx;
        rc.right = rc.left + w;
        rc.top = pFollow->GetPosition().cy + pFollow->GetSize().cy;
        rc.bottom = rc.top + h;
    }

    // 修改坐标单位为行列
    rc.left /= sTwips.cx;
    rc.right /= sTwips.cx;
    rc.top /= sTwips.cy;
    rc.bottom /= sTwips.cy;

    // 分配新的打印ITEM保存数据
    PRINT_ITEM *&pItem = PrintItems.pItems[PrintItems.nItemNum++];
    int nLen = strlen(szCode);
    int nAllocLen = sizeof(PRINT_ITEM) + nLen + MAX_FORMAT_LEN;
    pItem = (PRINT_ITEM *)new char[nAllocLen];
    pItem->x = rc.left;
    pItem->y = rc.top;
    pItem->nWidth = rc.right - rc.left;
    pItem->nHeight = rc.bottom - rc.top;
    char *p = pItem->Text;
    if (0 < nLen)
    {
        *p++ = '\x06';
        *p++ = 0;   //宽度
        *p++ = 0;   //高度
        *p++ = 6;   //类型CODEBAR
        *p++ = (char)BarcodeMode;   //位置
        *p++ = nLen;//后跟数据长度
        strcpy(p, szCode);//数据
        p += nLen;
    }
    pItem->nTextLen = p - pItem->Text;
    return WFS_SUCCESS;
}

HRESULT CTextPrinter::DrawFrame(PrintContext *pContext, LPRECT pRect, FRAMETYPE Type)
{
    Q_UNUSED(pContext);
    Q_UNUSED(pRect);
    Q_UNUSED(Type);
    return WFS_SUCCESS;
}

void CTextPrinter::SetDeviceType(DEVICETYPE Type)
{
    m_DevType = Type;
}



