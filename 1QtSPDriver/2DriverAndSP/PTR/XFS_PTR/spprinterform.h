#ifndef SPPRINTERFORM_H
#define SPPRINTERFORM_H

#include "spbaseprinter.h"
#include "MultiString.h"
#include "stdio.h"
#include "string.h"
#include <assert.h>

#include <string>
#include <vector>
#include <set>

#define RGB(r, g, b)    ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

using namespace std;

typedef vector<string>      STRLIST;
typedef STRLIST::iterator   SLIT;

extern SIZE g_sizeTwipsPerRowCol;

STRLIST Split(LPCSTR s, LPCSTR SepStr);

class KeyRule
{
    struct ATTR
    {
        int     nOffset;
        BOOL    bString;
        int     nLen;
        BOOL    bVerifyOR;
        BOOL    bRequired;
        STRLIST Verify;
    };

public:
    KeyRule(LPCSTR szName, BOOL bRequired = TRUE, ITEMTYPE type = ITEM_NONE);
    ~KeyRule();
public:
    int GetAttrCount() const;
    KeyRule &AddAttr(int nOffset, int nLen = 0, LPCSTR szVerify = NULL, BOOL bRequired = TRUE);
public:
    typedef vector<ATTR>        ATTRLIST;
    typedef ATTRLIST::iterator  ALIT;

    ITEMTYPE        m_ItemType;
    BOOL            m_bRequired;
    string          m_Name;
    ATTRLIST        m_Attrs;
};

class KeyRules
{
public:
    typedef vector<KeyRule *> RULELIST;
    typedef RULELIST::iterator RLIT;

    KeyRules();
    ~KeyRules();
public:
    KeyRule &AddRule(LPCSTR szName, BOOL bRequired = TRUE, ITEMTYPE type = ITEM_NONE);
    DWORD size() const;
public:
    RULELIST m_Rules;
};

typedef struct _tagSPPRINTERITEMSIZE
{
    WORD    cx;
    WORD    cy;
} SPPRINTERITEMSIZE, LPSPPRINTERITEMSIZE;

template <class T>
class  CSPPrinterItem : public T
{
public:
    CSPPrinterItem(ISPPrinterItem *pUnitItem)
    {
        m_Size.cx       = m_Size.cy     = 0;
        m_Position.cx   = m_Position.cy = m_Position.cz = 0;
        m_pUnitItem     = pUnitItem ? pUnitItem : this;
    }

    virtual ~CSPPrinterItem()
    {
    }
public:
    void Release()
    {
        delete this;
    }

    virtual ITEMTYPE GetItemType() const = 0;

    LPCSTR GetName() const
    {
        return m_Name.empty() ? NULL : m_Name.c_str();
    }

    int MulDiv(int number, int numberator, int denominator) const
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

    SIZE GetSize() const
    {
        RECT rc;
        GetMulDiv(rc);

        SIZE sizeTwips;
        sizeTwips.cx = MulDiv(m_Size.cx, rc.left, rc.right);
        sizeTwips.cy = MulDiv(m_Size.cy, rc.top, rc.bottom);
        return sizeTwips;
    }

    SIZE GetOrigSize() const
    {
        // modify by sutx, 20080911
        // return m_Size;
        SIZE size;
        size.cx = m_Size.cx;
        size.cy = m_Size.cy;
        return size;
    }

    void GetMulDiv(RECT &rc) const
    {
        SIZE s;
        FORMBASE base = FORM_MM;
        if (ITEM_FORM == m_pUnitItem->GetItemType())
        {
            base = ((ISPPrinterForm *)m_pUnitItem)->GetOrigUNIT(&s);
        }
        else
        {
            base = ((ISPPrinterMedia *)m_pUnitItem)->GetOrigUNIT(&s);
        }
        switch (base)
        {
        case FORM_MM:
            {
                rc.left = rc.top = 567;
                rc.right = s.cx * 10;
                rc.bottom = s.cy * 10;
            }
            break;
        case FORM_INCH:
            {
                rc.left = rc.top = 1440;
                rc.right = s.cx;
                rc.bottom = s.cy;
            }
            break;
        default: // FORM_ROWCOL
            {
                rc.left = g_sizeTwipsPerRowCol.cx;
                rc.top = g_sizeTwipsPerRowCol.cy;
                rc.right = s.cx;
                rc.bottom = s.cy;
            }
            break;
        }
    }

    SIZE3 GetPosition() const
    {
        RECT rc;
        GetMulDiv(rc);

        SIZE3 sizeTwips;
        sizeTwips.cx = MulDiv(m_Position.cx, rc.left, rc.right);
        sizeTwips.cy = MulDiv(m_Position.cy, rc.top, rc.bottom);
        sizeTwips.cz = MulDiv(m_Position.cz, rc.top, rc.bottom);
        return sizeTwips;
    }

    SIZE3 GetOrigPosition() const
    {
        return m_Position;
    }

    ISPPrinterField *FindField(const char *lpszName)
    {
        ISPPrinterForm *pForm = (ISPPrinterForm *)m_pUnitItem;
        for (DWORD i = 0; i < pForm->GetSubItemCount(); i++)
        {
            ISPPrinterItem *pItem = pForm->GetSubItem(i);
            ISPPrinterSubform *pSubform = NULL;
            if (ITEM_SUBFORM == pItem->GetItemType())
            {
                pSubform = (ISPPrinterSubform *)pItem;
            }
            for (DWORD j = 0; !pSubform || j < pSubform->GetSubItemCount(); j++)
            {
                if (NULL != pSubform)
                {
                    pItem = pSubform->GetSubItem(j);
                }
                if (ITEM_FIELD == pItem->GetItemType() && 0 == strcmp(pItem->GetName(), lpszName))
                {
                    return (ISPPrinterField *)pItem;
                }
                if (!pSubform)
                {
                    break;
                }
            }
        }
        return NULL;
    }

    virtual KeyRules *GetKeyRules() = 0;

    virtual BOOL Load(char *&p, const char *lpszName, int &nLine);
    BOOL LoadAttrs(char *&p, int &nLine, KeyRule *pRule, char *pResult[], int nResultCount);
protected:
    string  m_Name;
    SPPRINTERITEMSIZE   m_Size;
    SIZE3   m_Position;
    ISPPrinterItem *m_pUnitItem;
};

typedef vector<ISPPrinterItem *> IL;
typedef IL::iterator            ILIT;

template <class T>
class CSPPrinterContainerItem : public CSPPrinterItem<T>
{
public:
    CSPPrinterContainerItem(ISPPrinterItem *pUnitItem) : CSPPrinterItem<T>(pUnitItem)
    {
    }
    virtual ~CSPPrinterContainerItem()
    {
        Clear();
    }
public:
    DWORD GetSubItemCount() const
    {
        return m_Children.size();
    }

    ISPPrinterItem *GetSubItem(DWORD index) const
    {
        assert(0 <= index && index < GetSubItemCount());
        return m_Children[index];
    }
protected:
    void Clear()
    {
        ILIT it;
        for (it = m_Children.begin(); it != m_Children.end(); it++)
        {
            (*it)->Release();
        }
        m_Children.clear();
    }

    IL m_Children;
};

class CSPPrinterSubform : public CSPPrinterContainerItem<ISPPrinterSubform>
{
public:
    CSPPrinterSubform(ISPPrinterItem *pUnitItem) : CSPPrinterContainerItem<ISPPrinterSubform>(pUnitItem)
    {
    }

    ~CSPPrinterSubform()
    {
    }
public:
    virtual ITEMTYPE GetItemType() const
    {
        return ITEM_SUBFORM;
    }

    KeyRules *GetKeyRules();
};

template <class T>
class  CSPPrinterPrintable : public CSPPrinterItem<T>
{
public:
    CSPPrinterPrintable(ISPPrinterItem *pUnitItem) : CSPPrinterItem<T>(pUnitItem)
    {
        m_bBack     = false;
        m_Overflow  = OF_TERMINATE;
        m_Style     = 0;
        m_HAlign    = LEFT;
        m_VAlign    = BOTTOM;
        m_ColorR    = m_ColorG = m_ColorB = 0;
        m_LangID    = 0;
        m_Class     = CLASS_OPTIONAL;
        m_SolidColor = 0;
    }
    virtual ~CSPPrinterPrintable()
    {
    }
public:
    static COLORREF ToRGBColor(WORD SolidColor, COLORREF DefColor)
    {
        switch (SolidColor)
        {
        case 0: return RGB(0, 0, 0);
        case 1: return RGB(255, 255, 255);
        case 2: return RGB(192, 192, 192);
        case 3: return RGB(255, 0, 0);
        case 4: return RGB(0, 0, 255);
        case 5: return RGB(0, 255, 0);
        case 6: return RGB(255, 255, 0);
        default: return DefColor;
        }
    }

    LPCSTR GetHead() const
    {
        return m_Header.empty() ? NULL : m_Header.c_str();
    }

    LPCSTR GetFooter() const
    {
        return m_Footer.empty() ? NULL : m_Footer.c_str();
    }

    BOOL IsFrontSide() const
    {
        return !m_bBack;
    }

    OVERFLOW GetOverflow() const
    {
        return m_Overflow;
    }

    DWORD GetStyle() const
    {
        return m_Style;
    }

    HORIZONTAL GetHorizAlign() const
    {
        return m_HAlign;
    }

    VERTICAL GetVertAlign() const
    {
        return m_VAlign;
    }

    COLORREF GetColor() const
    {
        return ToRGBColor(m_SolidColor, RGB(m_ColorR, m_ColorG, m_ColorB));
    }

    WORD GetLangID() const
    {
        return m_LangID;
    }

    CLASS GetClass() const
    {
        return m_Class;
    }

    void LoadKeyRules(KeyRules *pRules);
protected:
    string      m_Header;
    string      m_Footer;
    BOOL        m_bBack;
    OVERFLOW    m_Overflow;
    DWORD       m_Style;
    HORIZONTAL  m_HAlign;
    VERTICAL    m_VAlign;
    BYTE        m_ColorR, m_ColorG, m_ColorB;
    WORD        m_SolidColor;
    WORD        m_LangID;
    CLASS       m_Class;
};

class CSPPrinterField : public CSPPrinterPrintable<ISPPrinterField>
{
public:
    CSPPrinterField(ISPPrinterItem *pUnitItem) : CSPPrinterPrintable<ISPPrinterField>(pUnitItem)
    {
        m_FieldType     = FT_TEXT;
        m_Scaling       = SCALING_BESTFIT;
        m_BarcodePos    = BCP_NONE;
        m_Access        = ACCESS_WRITE;
        m_Case          = CASE_NOCHANGE;
        m_FontSize      = 0;
        m_CPI           = 0;
        m_LPI           = 0;
        m_RepeatCount   = 0;
        m_RepeatOffset.cx = m_RepeatOffset.cy = 0;
        m_pFollows      = NULL;
    }
    ~CSPPrinterField()
    {
    }
public:
    ISPPrinterField *GetFollows()
    {
        if (m_Follows.empty())
        {
            return NULL;
        }
        if (!m_pFollows)
        {
            m_pFollows = FindField(m_Follows.c_str());
        }
        return m_pFollows;
    }

    // SIZE GetRepeat(DWORD *pCount);
    FIELDTYPE GetFieldType() const
    {
        return m_FieldType;
    }

    SCALING GetScaling() const
    {
        return m_Scaling;
    }

    BARCODEPOS GetBarcodePos() const
    {
        return m_BarcodePos;
    }

    ACCESS GetAccess() const
    {
        return m_Access;
    }

    CASE GetCASE() const
    {
        return m_Case;
    }

    LPCSTR GetFontName() const
    {
        return m_FontName.empty() ? NULL : m_FontName.c_str();
    }

    DWORD GetFontSize() const
    {
        return m_FontSize;
    }

    DWORD GetCPI() const
    {
        return m_CPI;
    }

    DWORD GetLPI() const
    {
        return m_LPI;
    }

    SIZE GetRepeatOffset() const
    {
        RECT rc;
        GetMulDiv(rc);
        SIZE TwipsOffset;
        TwipsOffset.cx = MulDiv(m_RepeatOffset.cx, rc.left, rc.right);
        TwipsOffset.cy = MulDiv(m_RepeatOffset.cy, rc.top, rc.bottom);
        return TwipsOffset;
    }

    DWORD GetRepeatCount() const
    {
        return m_RepeatCount /*<= 0 ? 1 : m_RepeatCount*/;
    }

    LPCSTR GetFormat() const
    {
        return m_Format.empty() ? NULL : m_Format.c_str();
    }

    LPCSTR GetInitValue() const
    {
        return m_InitValue.empty() ? NULL : m_InitValue.c_str();
    }

    virtual ITEMTYPE GetItemType() const
    {
        return ITEM_FIELD;
    }

    KeyRules *GetKeyRules();
protected:
    FIELDTYPE   m_FieldType;
    SCALING     m_Scaling;
    BARCODEPOS  m_BarcodePos;
    ACCESS      m_Access;
    CASE        m_Case;
    string      m_FontName;
    DWORD       m_FontSize;
    DWORD       m_CPI;
    DWORD       m_LPI;
    DWORD       m_RepeatCount;
    SIZE        m_RepeatOffset;
    string      m_Format;
    string      m_InitValue;
    string      m_Follows;

    ISPPrinterField *m_pFollows;
};

class CSPPrinterFrame : public CSPPrinterPrintable<ISPPrinterFrame>
{
public:
    CSPPrinterFrame(ISPPrinterItem *pUnitItem) : CSPPrinterPrintable<ISPPrinterFrame>(pUnitItem)
    {
        m_Class             = CLASS_STATIC;
        m_RepeatCount.cx    = m_RepeatCount.cy  = 1;
        m_RepeatOffset.cx   = m_RepeatOffset.cy = 0;
        m_FrameType         = RECTANGLE;
        m_FSolidColor       = 0;
        m_FColorR           = m_FColorG = m_FColorB = 255;
        m_FillStyle         = FILL_NONE;
        m_SubstSign         = '?';
        m_pFrames           = NULL;
        m_pTitle            = NULL;
    }
    ~CSPPrinterFrame()
    {
    }
public:
    DWORD GetRepeatX(DWORD *pOffset) const
    {
        *pOffset    = m_RepeatOffset.cx;
        return m_RepeatCount.cx;
    }

    BOOL GetRepeatY(DWORD *pOffset) const
    {
        *pOffset    = m_RepeatOffset.cy;
        return m_RepeatCount.cy;
    }

    FRAMETYPE GetFrameType() const
    {
        return m_FrameType;
    }

    COLORREF GetFillColor() const
    {
        return ToRGBColor(m_FSolidColor, RGB(m_FColorR, m_FColorG, m_FColorB));
    }

    FILLSTYLE GetFillStyle() const
    {
        return m_FillStyle;
    }

    char GetSubstSign() const
    {
        return m_SubstSign;
    }

    virtual ITEMTYPE GetItemType() const
    {
        return ITEM_FRAME;
    }

    ISPPrinterField *GetFrame()
    {
        if (m_Frames.empty())
        {
            return NULL;
        }
        if (!m_pFrames)
        {
            m_pFrames = FindField(m_Frames.c_str());
        }
        return m_pFrames;
    }

    ISPPrinterField *GetTitle()
    {
        if (m_Title.empty())
        {
            return NULL;
        }
        if (!m_pTitle)
        {
            m_pTitle = FindField(m_Title.c_str());
        }
        return m_pTitle;
    }
    KeyRules *GetKeyRules();
protected:
    SIZE        m_RepeatCount;
    SIZE        m_RepeatOffset;
    FRAMETYPE   m_FrameType;
    WORD        m_FSolidColor;
    COLORREF    m_FColorR, m_FColorG, m_FColorB;
    FILLSTYLE   m_FillStyle;
    string      m_Title;
    string      m_Frames;
    char        m_SubstSign;
    ISPPrinterField *m_pFrames;
    ISPPrinterField *m_pTitle;
};

class CSPPrinterForm : public CSPPrinterContainerItem<ISPPrinterForm>
{
public:
    CSPPrinterForm() : CSPPrinterContainerItem<ISPPrinterForm>(NULL)
    {
        m_bLoadSucc     = false;
        m_Align         = TOPLEFT;
        m_BaseSize.cx   = m_BaseSize.cy = 1;
        m_Base          = FORM_MM;
        m_Orientation   = PORTRAIT;
        m_VerMajor      = 1;
        m_VerMinor      = 0;
        m_LangID        = 0;
        m_Skew          = 0;
    }
    virtual ~CSPPrinterForm()
    {
    }
public:
    BOOL IsLoadSucc() const
    {
        return m_bLoadSucc;
    }

    FORMALIGN GetAlign() const
    {
        return m_Align;
    }

    FORMBASE GetOrigUNIT(SIZE *pSize) const
    {
        *pSize = m_BaseSize;
        return m_Base;
    }

    FORMORIENTATION GetOrientation() const
    {
        return m_Orientation;
    }

    WORD GetVersion() const
    {
        return MAKEWORD(m_VerMajor, m_VerMinor);
    }

    WORD GetLangID() const
    {
        return m_LangID;
    }

    LPCSTR GetCopyright() const
    {
        return m_Copyright.empty() ? NULL : m_Copyright.c_str();
    }

    LPCSTR GetTitle() const
    {
        return m_Title.empty() ? NULL : m_Title.c_str();
    }

    LPCSTR GetComment() const
    {
        return m_Comment.empty() ? NULL : m_Comment.c_str();
    }

    LPCSTR GetUserPrompt() const
    {
        return m_Prompt.empty() ? NULL : m_Prompt.c_str();
    }

    virtual ITEMTYPE GetItemType() const
    {
        return ITEM_FORM;
    }

    static LPCSTR GetKey()
    {
        return "XFSFORM";
    }

    virtual BOOL Load(char *&p, const char *lpszName, int &nLine);

    KeyRules *GetKeyRules();

protected:
    BOOL        m_bLoadSucc;
    FORMALIGN   m_Align;
    SIZE        m_BaseSize;
    FORMBASE    m_Base;
    FORMORIENTATION m_Orientation;
    BYTE        m_VerMajor, m_VerMinor;
    WORD        m_LangID;
    DWORD       m_Skew;
    string      m_Title;
    string      m_Copyright;
    string      m_Comment;
    string      m_Prompt;
    string      m_Date, m_Author;
};

class CSPPrinterMedia : public CSPPrinterItem<ISPPrinterMedia>
{
public:
    CSPPrinterMedia() : CSPPrinterItem<ISPPrinterMedia>(NULL)
    {
        m_bLoadSucc             = false;
        m_MediaType             = MT_GENERIC;
        m_PaperSource           = PS_ANY;
        m_Base                  = FORM_MM;
        m_BaseSize.cx           = m_BaseSize.cy             = 1;
        m_PrintArea.left        = m_PrintArea.top           = 0;
        m_PrintArea.right       = m_PrintArea.bottom        = 0;
        m_RestrictedArea.left   = m_RestrictedArea.top      = 0;
        m_RestrictedArea.right  = m_RestrictedArea.bottom   = 0;
        m_PassbookFold          = PF_HORIZONTAL;
        m_Staggering            = 0;
        m_PageCount             = 0;
        m_LineCount             = 0;
    }
    ~CSPPrinterMedia()
    {
    }
public:
    BOOL IsLoadSucc() const
    {
        return m_bLoadSucc;
    }

    MEDIATYPE GetMediaType() const
    {
        return m_MediaType;
    }

    PAPERSOURCE GetPaperSource() const
    {
        return m_PaperSource;
    }

    FORMBASE GetOrigUNIT(SIZE *pSize) const
    {
        *pSize = m_BaseSize;
        return m_Base;
    }

    void GetPrintArea(RECT &rc) const
    {
        RECT md;
        GetMulDiv(md);

        rc.left = MulDiv(m_PrintArea.left, md.left, md.right);
        rc.right = MulDiv(m_PrintArea.left + m_PrintArea.right, md.left, md.right);
        rc.top = MulDiv(m_PrintArea.top, md.top, md.bottom);
        rc.bottom = MulDiv(m_PrintArea.top + m_PrintArea.bottom, md.top, md.bottom);
    }

    void GetOrigPrintArea(RECT &rc) const
    {
        rc = m_PrintArea;
        rc.right += rc.left;
        rc.bottom += rc.top;
    }

    void GetRestrictedArea(RECT &rc) const
    {
        RECT md;
        GetMulDiv(md);

        rc.left = MulDiv(m_RestrictedArea.left, md.left, md.right);
        rc.right = MulDiv(m_RestrictedArea.left + m_RestrictedArea.right, md.left, md.right);
        rc.top = MulDiv(m_RestrictedArea.top, md.top, md.bottom);
        rc.bottom = MulDiv(m_RestrictedArea.top + m_RestrictedArea.bottom, md.top, md.bottom);
    }

    void GetOrigRestrictedArea(RECT &rc) const
    {
        rc = m_RestrictedArea;
        rc.right += rc.left;
        rc.bottom += rc.top;
    }

    PASSBOOKFOLD GetPassbookFold() const
    {
        return m_PassbookFold;
    }

    DWORD GetStaggering() const
    {
        return m_Staggering;
    }

    DWORD GetPageCount() const
    {
        return m_PageCount;
    }

    DWORD GetLineCount() const
    {
        return m_LineCount;
    }

    static LPCSTR GetKey()
    {
        return "XFSMEDIA";
    }

    virtual ITEMTYPE GetItemType() const
    {
        return ITEM_MEDIA;
    }

    virtual BOOL Load(char *&p, const char *lpszName, int &nLine);

    KeyRules *GetKeyRules();
    void UpdatePrintAreaFromSize();
protected:
    BOOL            m_bLoadSucc;
    MEDIATYPE       m_MediaType;
    PAPERSOURCE     m_PaperSource;
    FORMBASE        m_Base;
    SIZE            m_BaseSize;
    RECT            m_PrintArea;
    RECT            m_RestrictedArea;
    PASSBOOKFOLD    m_PassbookFold;
    DWORD           m_Staggering;
    DWORD           m_PageCount;
    DWORD           m_LineCount;
};

class CSPPrinterList : public ISPPrinterList
{
    typedef vector<ISPPrinterItem *>    LIST;
    typedef LIST::iterator              IT;
public:
    CSPPrinterList()
    {
    }
    ~CSPPrinterList()
    {
        Clear();
    }
public:
    // 2015.8.5 const char
    char *strfind(char *str, const char *substr, int &nLine)
    {
        char *p = strstr(str, substr);
        while (*str && str != p)
        {
            if ('\n' == (*str))
            {
                nLine++;
            }
            str++;
        }
        return p;
    }

    BOOL Load(LPCSTR szFileName, BOOL bForm)
    {
        // modified by huanghc 20140611 for：使用rt形式读取的字节数可能会比实际文件中的内容少
        FILE *fp = fopen(szFileName, "rb");
        if (!fp)
        {
            return FALSE;
        }
        fseek(fp, 0L, SEEK_END);// 2015.8.5
        long nLen = ftell(fp);
        char *pBuf = new char[nLen + 1];
        memset(pBuf, 0, (nLen + 1));
        fseek(fp, 0, SEEK_SET);
        fread(pBuf, 1, nLen, fp);
        fclose(fp);

        char *p = pBuf;
        int nLine = 0;
        while (1)
        {
            p = strfind(p, bForm ? CSPPrinterForm::GetKey() : CSPPrinterMedia::GetKey(), nLine);
            if (!p)
            {
                break;
            }
            if (bForm)
            {
                CSPPrinterForm *pItem = new CSPPrinterForm;
                //              if(!pItem->Load(p, NULL, nLine))
                //              {
                //                  delete pItem;
                //                  p++;
                //                  continue;
                //              }
                pItem->Load(p, NULL, nLine);
                m_List.push_back(pItem);
            }
            else
            {
                CSPPrinterMedia *pItem = new CSPPrinterMedia;
                //              if(!pItem->Load(p, NULL, nLine))
                //              {
                //                  delete pItem;
                //                  p++;
                //                  continue;
                //              }
                pItem->Load(p, NULL, nLine);
                pItem->UpdatePrintAreaFromSize();
                m_List.push_back(pItem);
            }
        }

        delete [] pBuf;
        pBuf = NULL;

        return TRUE;
    }

    virtual DWORD GetCount() const
    {
        return m_List.size();
    }

    virtual ISPPrinterItem *GetAt(DWORD index) const
    {
        return m_List[index];
    }

    ISPPrinterItem *Find(LPCSTR szName)
    {
        IT it;
        for (it = m_List.begin(); it != m_List.end(); it++)
        {
            if (0 == strcmp((*it)->GetName(), szName))
            {
                return (*it);
            }
        }

        return NULL;
    }

    void Clear()
    {
        IT it;
        for (it = m_List.begin(); it != m_List.end(); it++)
        {
            (*it)->Release();
        }

        m_List.clear();
    }

    CMultiString GetNames()
    {
        CMultiString names;
        IT it;
        set<string> setNames;
        for (it = m_List.begin(); it != m_List.end(); it++)
        {
            if (setNames.find((*it)->GetName()) == setNames.end())
            {
                names.Add((*it)->GetName());
                setNames.insert((*it)->GetName());
            }
        }
        return names;
    }

    LIST m_List;
};

typedef CSPPrinterList CSPPrinterFormList;
typedef CSPPrinterList CSPPrinterMediaList;

#endif // SPPRINTERFORM_H
