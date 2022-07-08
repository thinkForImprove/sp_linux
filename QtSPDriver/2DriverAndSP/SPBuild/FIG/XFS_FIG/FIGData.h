#ifndef SPPTRDATA_H
#define SPPTRDATA_H

//#include "spprinterform.h"
//#include "XfsDesc.h"
#include <string.h>
#include "XFSPTR.H"
#include "MultiString.h"
#include "XfsRegValue.h"
#include "ILogWrite.h"

#define FORMFILEVALUENAME           "figform"               // Form值名
#define MEDIAFILEVALUENAME          "figmedia"              // Form值名fwControl

#ifdef Q_OS_WIN32
#define FORMFILEDEFAULT             "C:/CFES/FORM/FIG/ReceiptForm.wfm"
#define MEDIAFILEDEFAULT            "C:/CFES/FORM/FIG/FingerprintMedia.wfm"
#else
#define FORMFILEDEFAULT             "/usr/local/CFES/DATA/FORM/FIG/ReceiptForm.wfm"
#define MEDIAFILEDEFAULT            "/usr/local/CFES/DATA/FORM/FIG/FingerprintMedia.wfm"
#endif

#define  FINGERFEATUREDIRECTORY     "/usr/local/CFES/DATA/FORM/FIG"
#define  FINGERFEATUREPATH   "/usr/local/CFES/DATA/FORM/FIG/temp.txt"

#define MAX_IMAGE_COUNT              3
#define MAX_FEATURE_SIZE             512
#define MAX_FIELD_COUNT              1024

typedef enum
{
    FINGER_PRESENT = 0,         // 按下
    FINGER_NOTPRESENT,          // 移开
    FINGER_UNKNOWN,             // 未知
    FINGER_NOTSUPP,             // 不支持
} FINGERPRESSEDMODE;

typedef enum
{
    COLLECT_FRONT = 0,          // 采集指纹
    COLLECT_BACK,               // 比对
    COLLECT_CODELINE,           // 采集模板
    COLLECT_UNKNOWN,            // 未知
} LASTSUCCESSFINGERMODE;

class CWfsFigStatus : public WFSPTRSTATUS
{
public:
    CWfsFigStatus()
    {
        this->fwDevice          = WFS_PTR_DEVONLINE;
        this->fwMedia           = WFS_PTR_MEDIANOTPRESENT;
        memset(this->fwPaper, 0, sizeof(this->fwPaper));
        this->fwToner           = WFS_PTR_TONERNOTSUPP;
        this->fwInk             = WFS_PTR_INKNOTSUPP;
        this->fwLamp            = WFS_PTR_LAMPNOTSUPP;
        this->lppRetractBins    = nullptr;
        this->usMediaOnStacker  = 0;
        this->lpszExtra         = nullptr;
    }

    ~CWfsFigStatus()
    {
    }
};
class CWfsFigCaps : public WFSPTRCAPS
{
public:
    CWfsFigCaps()
    {
        BOOL bJournal = FALSE;
        this->wClass                = WFS_SERVICE_CLASS_PTR;                                    // 逻辑服务类
        this->fwType                = WFS_PTR_TYPESCANNER;                                      // 设备类型
        this->bCompound             = FALSE;
        this->wResolution           = WFS_PTR_RESMED;                                           // 打印清晰度:中等
        this->fwReadForm            = WFS_PTR_READIMAGE;                                        // 是否支持读介质:具有成像功能 raw: 0
        this->fwWriteForm           = 0;
        this->fwExtents               = 0;                                                      // 是否支持测量介质:不支持
        this->fwControl             = WFS_PTR_CTRLFLUSH;
        this->usMaxMediaOnStacker   = 0;
        this->bAcceptMedia          = FALSE;
        this->bMultiPage            = FALSE;
        this->fwPaperSources         = 0;
        this->bMediaTaken           = FALSE;
        this->usRetractBins          = 0;
        this->lpusMaxRetract           = nullptr;
        this->fwImageType           = WFS_PTR_IMAGEBMP;
        this->fwFrontImageColorFormat   = 0;
        this->fwBackImageColorFormat    = 0;
        this->fwCodelineFormat       = WFS_PTR_CODELINECMC7;
        this->fwImageSource          = WFS_PTR_IMAGEFRONT | WFS_PTR_IMAGEBACK | WFS_PTR_CODELINE;
        this->fwCharSupport          = WFS_PTR_ASCII;
        this->bDispensePaper        = FALSE;
        this->lpszExtra             = nullptr;
    }

    ~CWfsFigCaps()
    {
    }
};

class CWFSPTRIMAGEHelper
{
public:
    CWFSPTRIMAGEHelper() : m_pszOutput(nullptr), m_uSize(0) { }
    ~CWFSPTRIMAGEHelper() { Release(); }
    bool NewBuff(UINT uSize)
    {
        m_uSize = uSize;
        m_ppOutput = new LPWFSPTRIMAGE[m_uSize + 1];
        if (m_ppOutput == nullptr)
            return false;
        memset(m_ppOutput, 0x00, sizeof(LPWFSPTRIMAGE) * (m_uSize + 1));

        m_pszOutput = new WFSPTRIMAGE[m_uSize];
        if (m_pszOutput == nullptr)
            return false;
        memset(m_pszOutput, 0x00, sizeof(WFSPTRIMAGE) * m_uSize);
        for (UINT i = 0; i < m_uSize; i++)
        {
            m_ppOutput[i] = &m_pszOutput[i];
        }

        for (UINT i = 0; i < m_uSize; i++)
        {
            m_pszOutput[i].ulDataLength = 0;
            m_pszOutput[i].lpbData = new BYTE[MAX_FEATURE_SIZE];
            if (m_pszOutput[i].lpbData == nullptr)
                return false;

            m_pszOutput[i].lpbData[0] = 0x00;
        }
        return true;
    }
    void Release()
    {
        if (m_pszOutput != nullptr)
        {
            for (UINT i = 0; i < m_uSize; i++)
            {
                if (m_pszOutput[i].lpbData != nullptr)
                {
                    delete m_pszOutput[i].lpbData;
                    m_pszOutput[i].lpbData = nullptr;
                }
            }
            delete[]m_pszOutput;
            m_pszOutput = nullptr;
        }
    }
    LPWFSPTRIMAGE GetBuff(UINT uIndex)
    {
        if (uIndex < m_uSize)
            return &m_pszOutput[uIndex];
        else
            return nullptr;
    }
    LPWFSPTRIMAGE *GetData() { return m_ppOutput; }
    UINT GetSize() { return m_uSize; }
private:
    LPWFSPTRIMAGE m_pszOutput;
    LPWFSPTRIMAGE *m_ppOutput;
    UINT m_uSize;
};

class CWfsPtrImage : public WFSPTRIMAGE
{
public:
    CWfsPtrImage()
    {
        this->lpbData = nullptr;
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
        this->lpbData        = nullptr;

        if (0 < this->ulDataLength)
        {
            this->lpbData = new BYTE[this->ulDataLength + 1];
            memcpy(this->lpbData, lpData, this->ulDataLength);
            this->lpbData[this->ulDataLength] = 0;
        }
    }

    CWfsPtrImage(const WFSPTRIMAGE &i)
    {
        this->lpbData = nullptr;
        *this = i;
    }

    ~CWfsPtrImage()
    {
        Clear();
    }

    void Clear()
    {
        if (nullptr != this->lpbData)
        {
            //delete [] this->lpbData;
            this->lpbData = nullptr;
        }
    }
};

class CSPReadImageOut
{
public:
    CSPReadImageOut() : m_nCount(0)
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
    DWORD             m_nCount;
    CWfsPtrImage    *m_pImages[MAX_IMAGE_COUNT + 1];
    CWfsPtrImage     m_Images[MAX_IMAGE_COUNT];
};

#endif // SPPTRDATA_H
