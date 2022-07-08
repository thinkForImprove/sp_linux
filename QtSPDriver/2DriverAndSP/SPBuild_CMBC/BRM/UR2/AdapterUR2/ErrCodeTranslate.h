// ErrCodeTranslate.h: interface for the ErrCodeTranslate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ERRCODETRANSLATE_H__E670DB82_EA6E_4283_AAEE_62AD85FD405C__INCLUDED_)
#define AFX_ERRCODETRANSLATE_H__E670DB82_EA6E_4283_AAEE_62AD85FD405C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IBRMAdapter.h"

#pragma warning (disable: 4786)

#include <string>
#include <set>
using namespace std;

class CBRMAdapter;
typedef struct _adp_cass_info ADP_CASS_INFO;

class ErrCodeTranslate
{
public:
    ErrCodeTranslate(const ADP_CASS_INFO *pCassInfo);
    virtual ~ErrCodeTranslate();
    //根据错误码获取钞箱错误原因
    BOOL GetCUErr(const char *szErrCode,
                  BOOL bCassEnabled[ADP_MAX_CU_SIZE],
                  ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) const;

    //根据错误码获取开门状态是否为阻塞
    BOOL IsOpenShutterJammed(const char *szErrCode) const;
    //根据错误码获取关门状态是否为阻塞
    BOOL IsCloseShutterJammed(const char *szErrCode) const;

    //获取验钞拒钞原因
    BOOL GetRejectCauseOfCashCount(const char *szErrCode, ADP_ERR_REJECT &usErrReject) const;
    //验钞是否因为门口钞票过多导致无法进行验钞动作
    BOOL IsTooManyItemInCSOfCashCount(const char *szErrCode) const;

    //获取机芯设备是否不在正确位置上导致出错
    BOOL IsDeviceNotInPosition(const char *szErrCode) const;

    //私有成员函数
private:
    //钞箱是否是空
    BOOL GetCUErrEmpty(const string &strErrCode, BOOL bCassEnabled[ADP_MAX_CU_SIZE], ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) const;
    //钞箱是否是满
    BOOL GetCUErrFull(const string &strErrCode, BOOL bCassEnabled[ADP_MAX_CU_SIZE], ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) const;
    //是否为致命错误
    BOOL GetCUErrFatal(const string &strErrCode, BOOL bCassEnabled[ADP_MAX_CU_SIZE], ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) const;
    //是否为可重试错误
    BOOL GetCUErrRetryable(const string &strErrCode, BOOL bCassEnabled[ADP_MAX_CU_SIZE], ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) const;

private:
    set<string> m_setErrCodeFatal;
    const ADP_CASS_INFO *m_pCassInfo;
};

#endif // !defined(AFX_ERRCODETRANSLATE_H__E670DB82_EA6E_4283_AAEE_62AD85FD405C__INCLUDED_)
