#ifndef SPPTRDATA_H
#define SPPTRDATA_H

#include "spprinterform.h"
//#include "XfsDesc.h"
#include <string.h>

#define FORMFILEVALUENAME           "ptrform"               // Form值名
#define FORMFILEDEFAULT             "/etc/ndt/form/ptr_form/printerform.wfm"        // FORM缺省值
#define MEDIAFILEVALUENAME          "ptrmedia"              // Form值名fwControl
#define MEDIAFILEDEFAULT            "/etc/ndt/form/ptr_form/printermedia.wfm"       // FORM缺省值

#define MAX_IMAGE_COUNT              3

#define MAX_FIELD_COUNT              1024

class CWfsPtrStatus : public WFSPTRSTATUS
{
public:
    CWfsPtrStatus()
    {
        this->fwDevice  = WFS_PTR_DEVONLINE;
        this->fwMedia       = WFS_PTR_MEDIANOTPRESENT;
        memset(this->fwPaper, 0, sizeof(this->fwPaper));    // 0 for WFS_PTR_PAPERFULL
        this->fwToner       = WFS_PTR_TONERFULL;
        this->fwInk     = WFS_PTR_INKFULL;
        this->fwLamp        = WFS_PTR_LAMPOK;
        this->lppRetractBins        = nullptr;
        this->usMediaOnStacker  = 0;
        this->lpszExtra = nullptr;
    }

    ~CWfsPtrStatus()
    {
    }
};
class CWfsPtrCaps : public WFSPTRCAPS
{
public:
    CWfsPtrCaps()
    {
        this->wClass                = WFS_SERVICE_CLASS_PTR;
        this->fwType                = WFS_PTR_TYPERECEIPT;
        this->bCompound             = FALSE;
        this->wResolution           = WFS_PTR_RESMED;
        this->fwReadForm             = 0;
        this->fwWriteForm           = WFS_PTR_WRITETEXT;
        this->extents               = 0;
        this->control               = WFS_PTR_CTRLCUT;
        this->max_media_on_stacker  = 0;
        this->accept_media          = FALSE;
        this->multi_page            = FALSE;
        this->paper_sources         = WFS_PTR_PAPERUPPER;
        this->media_taken           = FALSE;
        this->retract_bins          = 0;
        this->max_retract           = NULL;
        this->image_type            = 0;
        this->front_image_color_format  = 0;
        this->back_image_color_format   = 0;
        this->codeline_format       = 0;
        this->image_source          = 0;
        this->char_support          = WFS_PTR_UTF8;
        this->dispense_paper        = FALSE;
        this->extra                 = NULL;
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
        this->size  = 0;
        this->data  = NULL;
    }

    ~CInputRawData()
    {
        if (NULL != this->data)
        {
            delete [] this->data;
            this->data = NULL;
        }
    }

    inline LPBYTE GetData() const
    {
        return this->data;
    }

    inline DWORD GetLen() const
    {
        return this->size;
    }

    BOOL SetData(DWORD dwLen, LPBYTE pData)
    {
        if (dwLen >= m_nAllocLen)
        {
            if (NULL != this->data)
            {
                delete [] this->data;
                this->data = NULL;
            }
            m_nAllocLen = dwLen + 1;
            this->data = new BYTE[m_nAllocLen];
            memset(this->data, 0,  m_nAllocLen);
        }
        if (0 < dwLen)
        {
            memcpy(this->data, pData, dwLen);
        }
        this->size = dwLen;

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
        this->fields            = NULL;
        this->unicode_fields    = NULL;
    }

    ~CSPReadFormOut()
    {
    }

    BOOL Add(LPCSTR szName, LPCSTR szValue)
    {
        char buf[1024];
        sprintf(buf, "%s=%s", szName, szValue);
        BOOL bRet = m_Fields.Add(buf);
        this->fields = (LPSTR)(LPCSTR)m_Fields;
        return bRet;
    }

    void Clear()
    {
        m_Fields     = NULL;
        this->fields = NULL;
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
        this->data = NULL;
    }

    const CWfsPtrImage &operator=(const WFSPTRIMAGE &i)
    {
        Clear();
        Assign(i.image_source, i.status, i.data_length, i.data);
        return *this;
    }

    void Assign(WORD wSrc, WORD wStatus, ULONG Size, LPBYTE lpData)
    {
        Clear();

        this->image_source = wSrc;
        this->status       = wStatus;
        this->data_length = Size;
        this->data        = NULL;

        if (0 < this->data_length)
        {
            this->data = new BYTE[this->data_length + 1];
            memcpy(this->data, lpData, this->data_length);
            this->data[this->data_length] = 0;
        }
    }

    CWfsPtrImage(const WFSPTRIMAGE &i)
    {
        this->data = NULL;
        *this = i;
    }

    ~CWfsPtrImage()
    {
        Clear();
    }

    void Clear()
    {
        if (NULL != this->data)
        {
            delete [] this->data;
            this->data = NULL;
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
        while (m_pImages[i] && m_pImages[i]->image_source != wSrc)
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
        this->form_name   = NULL;
        this->user_prompt = NULL;
        this->fields      = NULL;
    }

    ~CWfsFormHeader()
    {
    }

    void ExtractFromForm(ISPPrinterForm *pForm)
    {
        this->form_name     = (LPSTR)pForm->GetName();
        SIZE size;
        this->base          = pForm->GetOrigUNIT(&size);
        this->unit_x        = size.cx;
        this->unit_y        = size.cy;
        size      = pForm->GetOrigSize();
        this->width         = size.cx;
        this->height        = size.cy;
        this->alignment     = pForm->GetAlign();
        this->orientation   = pForm->GetOrientation();
        SIZE3 pos           = pForm->GetOrigPosition();
        this->offset_x      = pos.cx;
        this->offset_y      = pos.cy;
        this->version_major = LOBYTE(pForm->GetVersion());
        this->version_minor = HIBYTE(pForm->GetVersion());
        this->user_prompt   = (LPSTR)pForm->GetUserPrompt();
        this->char_support  = WFS_PTR_UTF8;

        m_Fields = NULL;
        for (int i = 0; i < (int)(pForm->GetSubItemCount()); i++)
        {
            m_Fields.Add(pForm->GetSubItem(i)->GetName());
        }
        this->fields        = (LPSTR)(LPCSTR)m_Fields;
    }
protected:
    CMultiString    m_Fields;
};
class CWfsMedia : public WFSFRMMEDIA
{
public:
    void ExtractFromMedia(CSPPrinterMedia *pMedia)
    {
        this->media_type        = pMedia->GetMediaType();
        SIZE size;
        this->base              = pMedia->GetOrigUNIT(&size);
        this->unit_x            = size.cx;
        this->unit_y            = size.cy;
        size                   = pMedia->GetOrigSize();
        this->size_width        = size.cx;
        this->size_height       = size.cy;
        this->page_count        = pMedia->GetPageCount();
        this->line_count        = pMedia->GetLineCount();
        RECT rc;
        pMedia->GetOrigPrintArea(rc);
        this->print_area_x      = rc.left;
        this->print_area_y      = rc.top;
        this->print_area_width  = rc.right - rc.left;
        this->print_area_height = rc.bottom - rc.top;
        pMedia->GetOrigRestrictedArea(rc);
        this->restricted_area_x = rc.left;
        this->restricted_area_y = rc.top;
        this->restricted_area_width  = rc.right - rc.left;
        this->restricted_area_height = rc.bottom - rc.top;
        this->stagger       = pMedia->GetStaggering();
        this->fold_type     = pMedia->GetPassbookFold();


        // modify by lvcb 2013.03.06
        int iPSources   = pMedia->GetPaperSource();
        switch (iPSources)
        {
        case 0:
        case 1:
        case 2:
            this->paper_sources = iPSources;
            break;
        case 3:
            this->paper_sources = 4;
            break;
        case 4:
            this->paper_sources = 8;
            break;
        case 5:
            this->paper_sources = 16;
            break;
        case 6:
            this->paper_sources = 32;
            break;
        case 7:
            this->paper_sources = 64;
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
        this->field_name            = (LPSTR)pField->GetName();
        this->index_count           = pField->GetRepeatCount();
        this->type                  = pField->GetFieldType();
        this->field_class           = pField->GetClass();
        this->access                = pField->GetAccess();
        this->overflow              = pField->GetOverflow();
        this->initial_value         = (LPSTR)pField->GetInitValue();
        this->unicode_initial_value = NULL;
        this->unicode_format        = NULL;
        this->format                = (LPSTR)pField->GetFormat();
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

class CSPPtrData
{
public:
    CSPPtrData(CSPBasePrinter *pBasePrinter);
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

    CXfsDesc            m_pXfsDesc;
protected:
    CSPBasePrinter     *m_pBasePrinter;       // 调用具体的实现方法

    time_t  m_FormFileLastChangeTime;
    time_t  m_MediaFileLastChangeTime;

    CSPPrinterFormList  m_FormList;
    CSPPrinterMediaList m_MediaList;
};

#endif // SPPTRDATA_H
