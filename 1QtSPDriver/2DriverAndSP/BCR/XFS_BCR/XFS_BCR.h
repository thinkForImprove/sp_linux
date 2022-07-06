#pragma once
#include "IDevBCR.h"
#include "ISPBaseBCR.h"
#include "QtTypeInclude.h"

//////////////////////////////////////////////////////////////////////////
class CAutoSetScannerStatus
{
public:
    CAutoSetScannerStatus(bool *pbExecuting) : m_pbExecuting(pbExecuting) { if (m_pbExecuting != nullptr) *m_pbExecuting = true; }
    ~CAutoSetScannerStatus() { if (m_pbExecuting != nullptr) *m_pbExecuting = false; }
private:
    bool *m_pbExecuting;
};
//////////////////////////////////////////////////////////////////////////
class CWFSBCRREADOUTPUTHelper
{
public:
    CWFSBCRREADOUTPUTHelper() : m_pszOutput(nullptr), m_uSize(0) { }
    ~CWFSBCRREADOUTPUTHelper() { Release(); }
    bool NewBuff(UINT uSize)
    {
        m_uSize = uSize;
        m_ppOutput = new LPWFSBCRREADOUTPUT[m_uSize + 1];
        if (m_ppOutput == nullptr)
            return false;
        memset(m_ppOutput, 0x00, sizeof(LPWFSBCRREADOUTPUT) * (m_uSize + 1));

        m_pszOutput = new WFSBCRREADOUTPUT[m_uSize];
        if (m_pszOutput == nullptr)
            return false;
        memset(m_pszOutput, 0x00, sizeof(WFSBCRREADOUTPUT) * m_uSize);
        for (UINT i = 0; i < m_uSize; i++)
        {
            m_ppOutput[i] = &m_pszOutput[i];
        }

        for (UINT i = 0; i < m_uSize; i++)
        {
            m_pszOutput[i].lpxBarcodeData = new WFSBCRXDATA;
            if (m_pszOutput[i].lpxBarcodeData == nullptr)
                return false;

            m_pszOutput[i].lpxBarcodeData->usLength = 0;
            m_pszOutput[i].lpxBarcodeData->lpbData = new BYTE[4096];
            if (m_pszOutput[i].lpxBarcodeData->lpbData == nullptr)
                return false;

            m_pszOutput[i].lpxBarcodeData->lpbData[0] = 0x00;
        }
        return true;
    }
    void Release()
    {
        if (m_pszOutput != nullptr)
        {
            for (UINT i = 0; i < m_uSize; i++)
            {
                if (m_pszOutput[i].lpxBarcodeData != nullptr)
                {
                    if (m_pszOutput[i].lpxBarcodeData->lpbData != nullptr)
                    {
                        delete[]m_pszOutput[i].lpxBarcodeData->lpbData;
                        m_pszOutput[i].lpxBarcodeData->lpbData = nullptr;
                    }

                    delete m_pszOutput[i].lpxBarcodeData;
                    m_pszOutput[i].lpxBarcodeData = nullptr;
                }
            }
            delete[]m_pszOutput;
            m_pszOutput = nullptr;
        }
    }
    LPWFSBCRREADOUTPUT GetBuff(UINT uIndex)
    {
        if (uIndex < m_uSize)
            return &m_pszOutput[uIndex];
        else
            return nullptr;
    }
    LPWFSBCRREADOUTPUT *GetData() { return m_ppOutput; }
    UINT GetSize() { return m_uSize; }
private:
    LPWFSBCRREADOUTPUT m_pszOutput;
    LPWFSBCRREADOUTPUT *m_ppOutput;
    UINT m_uSize;
};


//////////////////////////////////////////////////////////////////////////
class CXFS_BCR : public ICmdFunc, public CLogManage
{
public:
    CXFS_BCR();
    virtual ~CXFS_BCR();
public:
    // 开始运行SP
    long StartRun();

protected:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // BCR类型接口
    virtual HRESULT GetStatus(LPWFSBCRSTATUS &lpstStatus);
    virtual HRESULT GetCapabilities(LPWFSBCRCAPS &lpstCaps);
    virtual HRESULT ReadBCR(const WFSBCRREADINPUT &stReadInput, LPWFSBCRREADOUTPUT *&lppReadOutput, DWORD dwTimeOut);
    virtual HRESULT Reset();
    virtual HRESULT SetGuidLight(const WFSBCRSETGUIDLIGHT &stLight);
    virtual HRESULT PowerSaveControl(const WFSBCRPOWERSAVECONTROL &stPowerCtrl);

protected:
    // 更新状态
    bool UpdateStatus(const DEVBCRSTATUS &stStatus);
    // 更新扩展状态，strErrCode错误码的后三位，因前六位是固化的
    void UpdateExtra(string strErrCode, string strDevVer = "");
    // 设备状态是否正常
    bool IsDevStatusOK();
private:
    bool                                    m_bScanerOn;
    char                                    m_szLogType[MAX_PATH];
    CQtDLLLoader<ISPBaseBCR>                m_pBase;
    CQtDLLLoader<IDevBCR>                   m_pDev;
    WFSBCRSTATUS                            m_stStatus;
    WFSBCRCAPS                              m_stCaps;
    CAutoEvent                              m_cCancelEvent;
    CXfsRegValue                            m_cXfsReg;
    std::string                             m_strLogicalName;
    std::string                             m_strSPName;
    CExtraInforHelper                       m_cExtra;
    CWFSBCRREADOUTPUTHelper                 m_cReadOutData;
    CSimpleMutex                           *m_pMutexGetStatus;
};
