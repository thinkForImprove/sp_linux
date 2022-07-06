#pragma once

#include "TextPrinter.h"
#include "QtTypeDef.h"
#include "IDevPTR.h"
#include "ISPBasePTR.h"
#include "PTRData.h"
#include "PTRForm.h"

//#include "offlinejournalprinter.h"

#define IDS_ERR_ON_EXECUTE_ERR              "调用[%s]失败[%s]"
#define IDS_ERR_FORM_INVALID                "FORM无效[%s]"
#define IDS_ERR_MEDIA_INVALID               "MEDIA无效[%s]"
#define IDS_ERR_FORM_NOT_FOUND              "FORM[%s]没有找到"
#define IDS_ERR_MEDIA_NOT_FOUND             "MEDIA[%s]没有找到"
#define IDS_ERR_FILED_NOT_FOUND             "FIELD[%s]没有找到"
#define IDS_ERR_MEDIA_OVERFLOW              "打印FORM[%s]到MEDIA[%s]上溢出"
#define IDS_ERR_MEDIA_OVERFLOW_PRINTAREA    "打印FORM[%s]到MEDIA[%s]上溢出打印区域"
#define IDS_ERR_FIELD_ERROR                 "字段域[%s]错误"
#define IDS_ERR_START_FORM                  "调用StartForm[%s]失败"
#define IDS_ERR_REQUIRED_FIELD              "打印FORM[%s]的字段[%s]没有提供数据"
#define IDS_ERR_PRINT_FIELD                 "打印FORM[%s]字段[%s]失败[%s]"
#define IDS_ERR_NO_CHARSET                  "系统缺少中文字符集"

//#define DEVTYPE2STR(n)      (n == 0 ? "HOTS" : (n == 1 || n == 2 ? "SNBC" : "SNBC"))                  // 30-00-00-00(FT#0052)
#define DEVTYPE2STR(n)      (n == 0 ? "HOTS" : (n == 1 ? "BK-C310" : (n == 2 ? "BT-NH80" : "BK-C310"))) // 30-00-00-00(FT#0052)

enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取纸
};

class CXFS_PTR : public ICmdFunc, public CTextPrinter
{

public:
    CXFS_PTR();
    virtual~ CXFS_PTR();

public:
    // 开始运行SP
    long StartRun();

    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // PTR类型接口
    // INFOR
    virtual HRESULT GetStatus(LPWFSPTRSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSPTRCAPS &lpCaps);
    virtual HRESULT GetFormList(LPSTR &lpszFormList);
    virtual HRESULT GetMediaList(LPSTR &lpszMediaList);
    virtual HRESULT GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader);
    virtual HRESULT GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia) ;
    virtual HRESULT GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList);
    // EXECUTE
    virtual HRESULT MediaControl(const LPDWORD lpdwMeidaControl);
    virtual HRESULT PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut);
    virtual HRESULT ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut);
    virtual HRESULT RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut);
    virtual HRESULT MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt);
    virtual HRESULT ResetCount(const LPUSHORT lpusBinNum);
    virtual HRESULT ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut);
    virtual HRESULT Reset(const LPWFSPTRRESET lpReset);
    virtual HRESULT RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut);
    virtual HRESULT DispensePaper(const LPWORD lpPaperSource);

public:
    // Fire消息
    void FireHWEvent(DWORD dwHWAct, char *pErr);
    void FireStatusChanged(WORD wStatus);
    void FireNoMedia(LPCSTR szPrompt);
    void FireMediaInserted();
    void FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure);
    void FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure);
    void FireRetractBinThreshold(USHORT BinNumber, WORD wStatus);
    void FireMediaTaken();
    void FirePaperThreshold(WORD wSrc, WORD wStatus);
    void FireTonerThreshold(WORD wStatus);
    void FireInkThreshold(WORD wStatus);
    void FireLampThreshold(WORD wStatus);
    void FireSRVMediaInserted();
    void FireMediaDetected(WORD wPos, USHORT BinNumber);

public:
    HRESULT StartOpen();// 30-00-00-00(FT#0032)

    void SetStatus(int nPrinter, int nPaper, int nTone);

private:
    long InnerPrintForm(LPWFSPTRPRINTFORM pInData);
    long InnerReadForm(LPWFSPTRREADFORM pInData);

    BOOL NeedFormatString() const;
protected:
    // 重载CSPBaseClass的方法
    HRESULT OnInit();
    HRESULT OnExit();

    virtual HRESULT ControlMedia(DWORD dwControl);
    virtual HRESULT SendRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData);
    virtual HRESULT Reset(DWORD dwMediaControl, USHORT usBinIndex);

    //以下是子类必须实现的函数,EndForm调用它
    virtual HRESULT PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint = FALSE);
    virtual HRESULT AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut);
    /* 图片打印
     *  nDstWidth  期望宽度
     *  nDstHeight 期望高度
     */
    virtual HRESULT PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight);    
    virtual HRESULT PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY);
    virtual long PrintData(const char *pBuffer, DWORD dwSize);

protected:
    bool LoadDevDll(LPCSTR ThisModule);
    void InitConifig();
    long InitStatus();
    long UpdateStatus();
    //去掉不可打印字符
    void RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData);

    WORD ConvertMediaStatus(OutletStatus eOutletStatus);
    WORD ConvertMediaStatus2(PaperStatus ePaperStatus);
    WORD ConvertPaperStatus(PaperStatus ePaperStatus);
    WORD ConvertTonerStatus(TonerStatus eTonerStatus);
    long ConvertErrCode(long lRes);
    void UpdateDeviceStatus(int iRet);
    bool IsJournalPrinter();

protected:
    BOOL                    m_bNeedKeepJammedStatus;
    bool                    m_bNeedReset;
    BOOL                    m_bPaperCutted;
    BOOL                    m_bReset;
    stPTRConfig             m_sConfig;
    WAIT_TAKEN_FLAG         m_WaitTaken;
private:
    CQtDLLLoader<IDevPTR>          m_pPrinter;
    CQtDLLLoader<ISPBasePTR>       m_pBase;
    CXfsRegValue                   m_cXfsReg;
    std::string                    m_strLogicalName;
    std::string                    m_strSPName;
    STPRINTFORMAT                  m_stPrintFormat;
    CSimpleMutex                  *m_pMutexGetStatus;
protected:
    CExtraInforHelper              m_cExtra;
    CWfsPtrStatus                  m_sStatus;
    CWfsPtrCaps                    m_sCaps;

private:
    DWORD   dwTakeTimeSize;

};

