#ifndef SPPTRDATA_H
#define SPPTRDATA_H

//#include "spprinterform.h"
//#include "XfsDesc.h"
#include "PTRForm.h"
#include <string.h>
#include "XFSPTR.H"
#include "MultiString.h"
#include "XfsRegValue.h"
#include "ILogWrite.h"

#define FORMFILEVALUENAME           "ptrform"               // Form值名
#define MEDIAFILEVALUENAME          "ptrmedia"              // Form值名fwControl

#ifdef Q_OS_WIN32
#define FORMFILEDEFAULT             "C:/CFES/FORM/PTR/printerform.wfm"
#define MEDIAFILEDEFAULT            "C:/CFES/FORM/PTR/printermedia.wfm"
#else
#define FORMFILEDEFAULT             "/usr/local/CFES/DATA/FORM/PTR/receiptform.wfm"
#define MEDIAFILEDEFAULT            "/usr/local/CFES/DATA/FORM/PTR/printermedia.wfm"
#endif


#define MAX_IMAGE_COUNT              3

#define MAX_FIELD_COUNT              1024

// 该类根据Cen标准PTR Status命令参数生成,所有PTR系模块通用,勿随意增减修改
// 如需修改,请继承该类,在子类中进行修改
// 以下初始设置只对应了凭条和流水两种，其他类型可在对应模块中进行第二次初始化指定
class CWfsPtrStatus : public WFSPTRSTATUS
{
public:
    CWfsPtrStatus()
    {
        this->fwDevice          = WFS_PTR_DEVONLINE;
        this->fwMedia           = WFS_PTR_MEDIANOTPRESENT;
        memset(this->fwPaper, 0, sizeof(this->fwPaper));    // 0 for WFS_PTR_PAPERFULL
        this->fwToner           = WFS_PTR_TONERFULL;
        this->fwInk             = WFS_PTR_INKFULL;
        this->fwLamp            = WFS_PTR_LAMPOK;
        this->lppRetractBins    = nullptr;
        this->usMediaOnStacker  = 0;
        this->lpszExtra         = nullptr;
        memset((char *)this->wGuidLights, 0, sizeof(this->wGuidLights));
        this->wDevicePosition = 0;
        this->usPowerSaveRecoveryTime = 0;
    }

    ~CWfsPtrStatus()
    {
    }

    int Diff(CWfsPtrStatus clDest)
    {
        if (this->fwDevice != clDest.fwDevice ||
            this->fwMedia != clDest.fwMedia ||
            this->fwPaper[0] != clDest.fwPaper[0]  ||
            this->fwToner != clDest.fwToner ||
            this->fwInk != clDest.fwInk ||
            this->fwLamp != clDest.fwLamp)
        {
            return 1;
        }
        return 0;
    }

    int Copy(CWfsPtrStatus *clDest)
    {
        clDest->fwDevice = this->fwDevice;
        clDest->fwMedia = this->fwMedia;
        clDest->fwPaper[0] = this->fwPaper[0];
        clDest->fwToner = this->fwToner;
        clDest->fwInk = this->fwInk;
        clDest->fwLamp = this->fwLamp;
        return 0;
    }
};

// 该类根据Cen标准PTR Capabilities命令参数生成,所有PTR系模块通用,勿随意增减修改
// 如需修改,请继承该类,在子类中进行修改
// 以下初始设置只对应了凭条和流水两种，其他类型可在对应模块中进行第二次初始化指定
class CWfsPtrCaps : public WFSPTRCAPS
{
public:
    CWfsPtrCaps()
    {
        this->wClass                    = WFS_SERVICE_CLASS_PTR;                                    // 逻辑服务类
        this->fwType                    = WFS_PTR_TYPERECEIPT;                                      // 设备类型(缺省凭条,各模块自行初始化设定)
        this->bCompound                 = FALSE;
        this->wResolution               = WFS_PTR_RESMED;                                           // 打印清晰度:中等
        this->fwReadForm                = 0;                                                        // 是否支持读介质:不支持
        this->fwWriteForm               = WFS_PTR_WRITETEXT | WFS_PTR_WRITEGRAPHICS;                // 缺省支持文本+图片打印
        this->fwExtents                 = 0;                                                        // 是否支持测量介质:不支持
        this->fwControl                 = WFS_PTR_CTRLCUT | WFS_PTR_CTRLEJECT | WFS_PTR_CTRLFLUSH;
        this->usMaxMediaOnStacker       = 0;
        this->bAcceptMedia              = FALSE;
        this->bMultiPage                = FALSE;
        this->fwPaperSources            = WFS_PTR_PAPERUPPER;
        this->bMediaTaken               = TRUE;
        this->usRetractBins             = 0;
        this->lpusMaxRetract            = nullptr;
        this->fwImageType               = 0;
        this->fwFrontImageColorFormat   = 0;
        this->fwBackImageColorFormat    = 0;
        this->fwCodelineFormat          = 0;
        this->fwImageSource             = 0;
        this->fwCharSupport             = WFS_PTR_ASCII;
        this->bDispensePaper            = FALSE;
        this->lpszExtra                 = nullptr;
        memset(this->dwGuidLights, 0, sizeof(this->dwGuidLights));
        this->lpszPrinter               = nullptr;
        this->bMediaPresented           = FALSE;
        this->usAutoRetractPeriod       = 0;
        this->bRetractToTransport       = FALSE;
        this->bPowerSaveControl         = FALSE;
    }

    ~CWfsPtrCaps()
    {
    }
};

class CInputRawData : public WFSPTRRAWDATAIN
{
public:
    CInputRawData()
    {
        m_nAllocLen = 0;
        this->ulSize  = 0;
        this->lpbData  = NULL;
    }

    ~CInputRawData()
    {
        if (NULL != this->lpbData)
        {
            delete [] this->lpbData;
            this->lpbData = NULL;
        }
    }

    inline LPBYTE GetData() const
    {
        return this->lpbData;
    }

    inline DWORD GetLen() const
    {
        return this->ulSize;
    }

    BOOL SetData(DWORD dwLen, LPBYTE pData)
    {
        if (dwLen >= m_nAllocLen)
        {
            if (NULL != this->lpbData)
            {
                delete [] this->lpbData;
                this->lpbData = NULL;
            }
            m_nAllocLen = dwLen + 1;
            this->lpbData = new BYTE[m_nAllocLen];
            memset(this->lpbData, 0,  m_nAllocLen);
        }
        if (0 < dwLen)
        {
            memcpy(this->lpbData, pData, dwLen);
        }
        this->ulSize = dwLen;

        return TRUE;
    }

    operator LPWFSPTRRAWDATAIN()
    {
        return this;
    }
protected:
    DWORD   m_nAllocLen;        // 分配长度
};

class CSPReadFormOut : public WFSPTRREADFORMOUT
{
public:
    CSPReadFormOut()
    {
        this->lpszFields            = nullptr;
        this->lpszUNICODEFields     = nullptr;
    }

    ~CSPReadFormOut()
    {
    }

    BOOL Add(LPCSTR szName, LPCSTR szValue)
    {
        char buf[1024];
        sprintf(buf, "%s=%s", szName, szValue);
        BOOL bRet = m_Fields.Add(buf);
        this->lpszFields = (LPSTR)(LPCSTR)m_Fields;
        return bRet;
    }

    void Clear()
    {
        m_Fields     = nullptr;
        this->lpszFields = nullptr;
    }

    operator LPWFSPTRREADFORMOUT()
    {
        return this;
    }
protected:
    CMultiString m_Fields;
};
class CWfsPtrImage : public WFSPTRIMAGE
{
public:
    CWfsPtrImage()
    {
        this->lpbData = NULL;
    }

    const CWfsPtrImage &operator=(const WFSPTRIMAGE &i)
    {
        Clear();
        Assign(i.wImageSource, i.wStatus, i.ulDataLength, i.lpbData);
        return *this;
    }

    void Assign(WORD wSrc, WORD wStatus, ULONG Size, LPBYTE lpData)
    {
        Clear();

        this->wImageSource = wSrc;
        this->wStatus       = wStatus;
        this->ulDataLength = Size;
        this->lpbData        = NULL;

        if (0 < this->ulDataLength)
        {
            this->lpbData = new BYTE[this->ulDataLength + 1];
            memcpy(this->lpbData, lpData, this->ulDataLength);
            this->lpbData[this->ulDataLength] = 0;
        }
    }

    CWfsPtrImage(const WFSPTRIMAGE &i)
    {
        this->lpbData = NULL;
        *this = i;
    }

    ~CWfsPtrImage()
    {
        Clear();
    }

    void Clear()
    {
        if (NULL != this->lpbData)
        {
            delete [] this->lpbData;
            this->lpbData = NULL;
        }
    }
};

class CSPReadImageOut
{
public:
    CSPReadImageOut()
    {
        Clear();
    }

    CSPReadImageOut(const CSPReadImageOut &io)
    {
        Clear();
        m_nCount = io.m_nCount;
        int i = 0;
        while (io.m_pImages[i])
        {
            m_Images[i] = io.m_Images[i];
            m_pImages[i] = m_Images + i;
        }
    }

    DWORD GetCount() const
    {
        return m_nCount;
    }

    BOOL Add(WORD wSrc, WORD wStatus, ULONG Size, LPBYTE lpData)
    {
        CWfsPtrImage *pi = Find(wSrc);
        if (pi)
        {
            pi->Assign(wSrc, wStatus, Size, lpData);
            return TRUE;
        }
        if (m_nCount >= MAX_IMAGE_COUNT)
        {
            return FALSE;
        }
        m_Images[m_nCount].Assign(wSrc, wStatus, Size, lpData);
        m_pImages[m_nCount] = m_Images + m_nCount;
        m_nCount++;
        return TRUE;
    }

    CWfsPtrImage *Find(WORD wSrc) const
    {
        int i = 0;
        while (m_pImages[i] && m_pImages[i]->wImageSource != wSrc)
        {
            i++;
        }
        return m_pImages[i];
    }

    ~CSPReadImageOut()
    {
        Clear();
    }

    void Clear()
    {
        m_nCount = 0;
        memset(m_pImages, 0, sizeof(m_pImages));
    }

    inline operator LPWFSPTRIMAGE *() const
    {
        return (LPWFSPTRIMAGE *)m_pImages;
    }
protected:
    int             m_nCount;
    CWfsPtrImage    *m_pImages[MAX_IMAGE_COUNT + 1];
    CWfsPtrImage     m_Images[MAX_IMAGE_COUNT];
};
class CWfsFormHeader : public WFSFRMHEADER
{
public:
    CWfsFormHeader()
    {
        this->lpszFormName   = NULL;
        this->lpszUserPrompt = NULL;
        this->lpszFields      = NULL;
    }

    ~CWfsFormHeader()
    {
    }

    void ExtractFromForm(ISPPrinterForm *pForm)
    {
        this->lpszFormName     = (LPSTR)pForm->GetName();
        SIZE size;
        this->wBase         = pForm->GetOrigUNIT(&size);
        this->wUnitX        = size.cx;
        this->wUnitY        = size.cy;
        size      = pForm->GetOrigSize();
        this->wWidth            = size.cx;
        this->wHeight       = size.cy;
        this->wAlignment        = pForm->GetAlign();
        this->wOrientation  = pForm->GetOrientation();
        SIZE3 pos           = pForm->GetOrigPosition();
        this->wOffsetX      = pos.cx;
        this->wOffsetY      = pos.cy;
        this->wVersionMajor = LOBYTE(pForm->GetVersion());
        this->wVersionMinor = HIBYTE(pForm->GetVersion());
        this->lpszUserPrompt    = (LPSTR)pForm->GetUserPrompt();
        this->fwCharSupport = WFS_PTR_ASCII;

        m_Fields = NULL;
        for (int i = 0; i < (int)(pForm->GetSubItemCount()); i++)
        {
            m_Fields.Add(pForm->GetSubItem(i)->GetName());
        }
        this->lpszFields        = (LPSTR)(LPCSTR)m_Fields;
    }
protected:
    CMultiString    m_Fields;
};
class CWfsMedia : public WFSFRMMEDIA
{
public:
    void ExtractFromMedia(CSPPrinterMedia *pMedia)
    {
        this->fwMediaType       = pMedia->GetMediaType();
        SIZE size;
        this->wBase              = pMedia->GetOrigUNIT(&size);
        this->wUnitX            = size.cx;
        this->wUnitY            = size.cy;
        size                   = pMedia->GetOrigSize();
        this->wSizeWidth        = size.cx;
        this->wSizeHeight       = size.cy;
        this->wPageCount        = pMedia->GetPageCount();
        this->wLineCount        = pMedia->GetLineCount();
        RECT rc;
        pMedia->GetOrigPrintArea(rc);
        this->wPrintAreaX       = rc.left;
        this->wPrintAreaY       = rc.top;
        this->wPrintAreaWidth   = rc.right - rc.left;
        this->wPrintAreaHeight = rc.bottom - rc.top;
        pMedia->GetOrigRestrictedArea(rc);
        this->wRestrictedAreaX = rc.left;
        this->wRestrictedAreaY = rc.top;
        this->wRestrictedAreaWidth   = rc.right - rc.left;
        this->wRestrictedAreaHeight = rc.bottom - rc.top;
        this->wStagger      = pMedia->GetStaggering();
        this->wFoldType     = pMedia->GetPassbookFold();


        // modify by lvcb 2013.03.06
        int iPSources   = pMedia->GetPaperSource();
        switch (iPSources)
        {
        case 0:
        case 1:
        case 2:
            this->wPaperSources = iPSources;
            break;
        case 3:
            this->wPaperSources = 4;
            break;
        case 4:
            this->wPaperSources = 8;
            break;
        case 5:
            this->wPaperSources = 16;
            break;
        case 6:
            this->wPaperSources = 32;
            break;
        case 7:
            this->wPaperSources = 64;
            break;
        default:
            break;
        }
    }
};
class CWfsField : public WFSFRMFIELD
{
public:
    void ExtractFromField(CSPPrinterField *pField)
    {
        this->lpszFieldName            = (LPSTR)pField->GetName();
        this->wIndexCount           = pField->GetRepeatCount();
        this->fwType                  = pField->GetFieldType();
        this->fwClass           = pField->GetClass();
        this->fwAccess                = pField->GetAccess();
        this->fwOverflow              = pField->GetOverflow();
        this->lpszInitialValue         = (LPSTR)pField->GetInitValue();
        this->lpszUNICODEInitialValue = NULL;
        this->lpszUNICODEFormat        = NULL;
        this->lpszFormat                = (LPSTR)pField->GetFormat();
    }
};
class CWfsFields
{
    typedef vector<CWfsField *> FIELDLIST;
    typedef FIELDLIST::iterator FLIT;
public:
    CWfsFields()
    {
        Clear();
    }

    DWORD GetCount() const
    {
        return m_nCount;
    }

    BOOL ExtractFromForm(ISPPrinterForm *pForm, LPCSTR lpszField)
    {
        Clear();
        ExtractFromContainer(pForm, lpszField);
        if (0 == m_nCount)
        {
            return FALSE;
        }
        return TRUE;
    }

    ~CWfsFields()
    {
        Clear();
        ClearFields();
    }

    void Clear()
    {
        m_nCount = 0;
        memset(m_pFields, 0, sizeof(m_pFields));
        ClearFields();
    }

    inline operator LPWFSFRMFIELD *() const
    {
        return (LPWFSFRMFIELD *)m_pFields;
    }
protected:
    void ExtractFromContainer(ISPPrinterContainerItem *p, LPCSTR lpszField)
    {
        for (int i = 0; i < (int)(p->GetSubItemCount()); i++)
        {
            ISPPrinterItem *pItem = p->GetSubItem(i);
            if (ITEM_FIELD == pItem->GetItemType())
            {
                if (!lpszField || 0 == strcmp(pItem->GetName(), lpszField))
                {
                    CWfsField *pField = GetNewField();
                    pField->ExtractFromField((CSPPrinterField *)pItem);
                }
            }
            else if (ITEM_SUBFORM == pItem->GetItemType())
            {
                ExtractFromContainer((ISPPrinterContainerItem *)pItem, lpszField);
            }
        }
    }

    CWfsField *GetNewField()
    {
        if (m_nCount >= (int)m_Fields.size())
        {
            m_Fields.push_back(new CWfsField);
        }
        m_pFields[m_nCount++] = m_Fields.back();
        return m_pFields[m_nCount - 1];
    }

    void ClearFields()
    {
        FLIT it;
        for (it = m_Fields.begin(); it != m_Fields.end(); it++)
        {
            delete (*it);
        }
        m_Fields.clear();
    }

    int                 m_nCount;
    CWfsField           *m_pFields[MAX_FIELD_COUNT + 1];
    FIELDLIST            m_Fields;
};

class CSPPtrData : public CLogManage
{
public:
    CSPPtrData(LPCSTR lpLogicalName);
    virtual~CSPPtrData();
public:
    // 装入MEDIA文件，如果是第一次装入或MEDIA文件修改过
    BOOL LoadMedias();
    // 装入FORM文件，如果是第一次装入或FORM文件修改过
    BOOL LoadForms();
    // 查找指定的MEDIA
    inline CSPPrinterMedia *FindMedia(LPCSTR lpszMediaName)
    {
        return (CSPPrinterMedia *)m_MediaList.Find(lpszMediaName);
    }
    // 查找指定的FORM
    inline CSPPrinterForm *FindForm(LPCSTR lpszFormName)
    {
        return (CSPPrinterForm *)m_FormList.Find(lpszFormName);
    }
    // 查找指定域
    HRESULT FindField(LPCSTR lpszForm, LPCSTR lpszField);

    inline LPCSTR GetNames(BOOL bForm = TRUE)
    {
        if (FALSE != bForm)
        {
            m_FormNames = m_FormList.GetNames();
            return m_FormNames;
        }
        else
        {
            m_MediaNames = m_MediaList.GetNames();
            return m_MediaNames;
        }
    }


    CSPPrinterFormList *GetFormList()
    {
        return &m_FormList;
    }

    CSPPrinterMediaList *GetMediaList()
    {
        return &m_MediaList;
    }

    void SetTwipsPerRowCol(SIZE *pSize)
    {
        if (g_sizeTwipsPerRowCol.cx != pSize->cx ||
            g_sizeTwipsPerRowCol.cy != pSize->cy)
        {
            g_sizeTwipsPerRowCol = *pSize;
        }
    }

public:
    CInputRawData       m_InputRawData;
    CSPReadFormOut      m_ReadFormOut;
    CSPReadImageOut     m_ReadImageOut;
    CWfsPtrStatus       m_Status;
    CWfsPtrCaps         m_Caps;
    CMultiString        m_FormNames;
    CMultiString        m_MediaNames;
    CWfsFormHeader      m_LastForm;
    CWfsMedia           m_LastMedia;
    CWfsFields          m_LastFields;
    WFSPTRMEDIAEXT      m_LastExtents;
    USHORT              m_ActualBinNumber;

    //CXfsDesc          m_pXfsDesc;
protected:
    //CSPBasePrinter*   m_pBasePrinter;       // 调用具体的实现方法

    time_t  m_FormFileLastChangeTime;
    time_t  m_MediaFileLastChangeTime;

    CXfsRegValue         m_cXfsReg;
    string               m_strFormName;
    string               m_strMediaName;
    string               m_strFormKey;
    string               m_strMediaKey;

    CSPPrinterFormList  m_FormList;
    CSPPrinterMediaList m_MediaList;
};

#endif // SPPTRDATA_H
