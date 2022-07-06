#pragma once
#include "XFSPTR.H"
#include "ISPBaseClass.h"

#if defined(SPBASEPTR_LIBRARY)
#define SPBASEPTRSHARED_EXPORT Q_DECL_EXPORT
#else
#define SPBASEPTRSHARED_EXPORT Q_DECL_IMPORT
#endif

// 命令执行和结果数据返回
struct ICmdFunc
{
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName) = 0;
    virtual HRESULT OnClose() = 0;
    virtual HRESULT OnStatus() = 0;
    virtual HRESULT OnWaitTaken() = 0;
    virtual HRESULT OnCancelAsyncRequest() = 0;
    virtual HRESULT OnUpdateDevPDL() = 0;

    // PTR类型接口
    // INFOR
    virtual HRESULT GetStatus(LPWFSPTRSTATUS &lpStatus) = 0;
    virtual HRESULT GetCapabilities(LPWFSPTRCAPS &lpCaps) = 0;
    virtual HRESULT GetFormList(LPSTR &lpszFormList) = 0;
    virtual HRESULT GetMediaList(LPSTR &lpszMediaList) = 0;
    virtual HRESULT GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader) = 0;
    virtual HRESULT GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia) = 0;
    virtual HRESULT GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList) = 0;
    // EXECUTE
    virtual HRESULT MediaControl(const LPDWORD lpdwMeidaControl) = 0;
    virtual HRESULT PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut) = 0;
    virtual HRESULT ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut) = 0;
    virtual HRESULT RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut) = 0;
    virtual HRESULT MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt) = 0;
    virtual HRESULT ResetCount(const LPUSHORT lpusBinNum) = 0;
    virtual HRESULT ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut) = 0;
    virtual HRESULT Reset(const LPWFSPTRRESET lpReset) = 0;
    virtual HRESULT RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut) = 0;
    virtual HRESULT DispensePaper(const LPWORD lpPaperSource) = 0;
};
//////////////////////////////////////////////////////////////////////////
struct ISPBasePTR
{
    // 释放接口
    virtual void Release() = 0;
    // 注册回调接口
    virtual void RegisterICmdFunc(ICmdFunc *pCmdFunc) = 0;
    // 开始运行
    virtual bool StartRun() = 0;
    // 获取SPBase数据
    virtual void GetSPBaseData(SPBASEDATA &stData) = 0;
    // 发送事件
    virtual bool FireEvent(UINT uMsgID, DWORD dwEventID, LPVOID lpData) = 0;
    // 发送状态改变事件
    virtual bool FireStatusChanged(DWORD wStatus) = 0;
    // 发送故障状态事件
    virtual bool FireHWErrorStatus(DWORD dwAction, LPCSTR lpDescription = nullptr) = 0;
};

extern "C" SPBASEPTRSHARED_EXPORT long CreateISPBasePTR(LPCSTR lpDevType, ISPBasePTR *&p);
//////////////////////////////////////////////////////////////////////////
