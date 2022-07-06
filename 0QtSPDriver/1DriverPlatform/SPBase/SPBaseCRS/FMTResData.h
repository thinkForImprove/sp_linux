#pragma once
#include "XFSCDM.H"
#include "XFSCIM.H"
#include "XFSAPI.H"
#include "ILogWrite.h"
#include "XfsSPIHelper.h"
#include "ISPBaseClass.h"

//////////////////////////////////////////////////////////////////////////
class CFMTResData : public CLogManage
{
public:
    CFMTResData(LPCSTR lpLogType);
    ~CFMTResData();

    // 格式并保存结果数据
    HRESULT FmtCDMGetResultBuffer(DWORD dwCategory, LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT FmtCDMExeResultBuffer(DWORD dwCommand, LPVOID lpData, LPWFSRESULT &lpResult, DWORD dwSize = 0);
    HRESULT FmtCIMGetResultBuffer(DWORD dwCategory, LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT FmtCIMExeResultBuffer(DWORD dwCommand, LPVOID lpData, LPWFSRESULT &lpResult, DWORD dwSize = 0);

    // CDM数据
    HRESULT Fmt_WFSCDMDENOMINATION(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMCUINFO(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMCASHUNIT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMMEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMITEMTAKEN(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMCAPS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMMIXTYPE(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMPRESENTSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMCIMCOUNTSCHANGED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMCURRENCYEXP(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCDMCUERROR(LPVOID lpData, LPWFSRESULT &lpResult);

    // CIM数据
    HRESULT Fmt_WFSCIMNOTENUMBERLIST(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMCASHINFO(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMCASHIN(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMITEMTAKEN(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMINPUTREFUSE(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMMEDIADETECTED(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMCAPS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMBANKNOTETYPES(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMCASHINSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMCURRENCYEXP(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMCUERROR(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSCIMSUBCASHIN(LPVOID lpData, LPWFSRESULT &lpResult);

    HRESULT Fmt_NODATA(LPWFSRESULT &lpResult);
    //系统事件
public:
    bool LoadWFMDll();

private:
    char                          m_szLogType[MAX_PATH];
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};
