// ErrCodeTranslate.cpp: implementation of the ErrCodeTranslate class.
//
//////////////////////////////////////////////////////////////////////
#include "ErrCodeTranslate.h"
#include "AdapterUR2.h"
//#include "log_lib.h"
#include "ILogWrite.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define VERIFYERRCODE(strCodeErr)\
    {\
        if (strCodeErr.length() != 8)\
        {\
            return FALSE;\
        }\
    }

ErrCodeTranslate::ErrCodeTranslate(const ADP_CASS_INFO *pCassInfo)
    : m_pCassInfo(pCassInfo)
{

}

ErrCodeTranslate::~ErrCodeTranslate()
{

}

//获取机芯设备是否不在正确位置上导致出错
BOOL ErrCodeTranslate::IsDeviceNotInPosition(const char *szErrCode) const
{
    assert(szErrCode != NULL);
    string strTemp = szErrCode;
    VERIFYERRCODE(strTemp)

    strTemp.replace(2, 6, 6, '*');
    if (strTemp.compare("80******") == 0)
    {
        return TRUE;
    }
    return FALSE;

}

BOOL ErrCodeTranslate::IsOpenShutterJammed(const char *szErrCode) const
{
    assert(szErrCode != NULL);
    string strTemp = szErrCode;
    VERIFYERRCODE(strTemp)

    if (strTemp.compare("05423801") == 0 //Outer shutter not moving to the proper position
       )
    {
        return TRUE;
    }

    strTemp.replace(6, 2, 2, '*');
    if (strTemp.compare("830536**") == 0  //OutShutter open time out
        || strTemp.compare("830526**") == 0 //InnerShutter open time out
        || strTemp.compare("054236**") == 0 //OutShutter open time out
        || strTemp.compare("054237**") == 0 //OutShutter close time out
        || strTemp.compare("054226**") == 0 //InnerShutter open time out
        || strTemp.compare("054227**") == 0 //InnerShutter close time out
        || strTemp.compare("054229**") == 0 //InnerShutter close time out
       )
    {
        return TRUE;
    }
    strTemp = szErrCode;
    strTemp.replace(7, 1, 1, '*');
    if (strTemp.compare("8305070*") == 0   //InnerShutter open err (Full open)
        || strTemp.compare("8305080*") == 0  //InnerShutter open err (Full open)
        || strTemp.compare("8305090*") == 0  //InnerShutter open err (Full open)
       )
    {
        return TRUE;
    }
    return FALSE;
}

BOOL ErrCodeTranslate::IsCloseShutterJammed(const char *szErrCode) const
{
    assert(szErrCode != NULL);
    string strTemp = szErrCode;
    VERIFYERRCODE(strTemp)

    if (strTemp.compare("05423500") == 0    //outshutter close err
        || strTemp.compare("05423911") == 0   //Phase of Outer shutter driving motor time-out
        || strTemp.compare("05423912") == 0   //Phase of Outer shutter driving motor time-out
        || strTemp.compare("05423913") == 0   //Phase of Outer shutter driving motor time-out
        || strTemp.compare("05423915") == 0   //Outer shutter not moving to the proper position
       )
    {
        return TRUE;
    }

    strTemp.replace(6, 2, 2, '*');
    if (strTemp.compare("830537**") == 0 // outshutter close time out
        || strTemp.compare("830511**") == 0 //outshutter close err
        || strTemp.compare("830512**") == 0 //out shutter detect hand
        || strTemp.compare("830524**") == 0 // inner shutter detect hand
        || strTemp.compare("830527**") == 0 // inner shutter close time out
        || strTemp.compare("830529**") == 0 // inner shutter close time out
        || strTemp.compare("830534**") == 0 //out shutter detect hand
        || strTemp.compare("830535**") == 0 //outshutter close err
        || strTemp.compare("054234**") == 0 //out shutter detect hand
        || strTemp.compare("054237**") == 0 // outshutter shutter close time out
       )
    {
        return TRUE;
    }

    strTemp = szErrCode;
    strTemp.replace(7, 1, 1, '*');
    if (strTemp.compare("8305130*") == 0       //InnerShutter close err
        || strTemp.compare("8305140*") == 0  // inner shutter detect hand
        || strTemp.compare("8305150*") == 0) //InnerShutter close err
    {
        return TRUE;
    }
    return FALSE;
}

BOOL ErrCodeTranslate::GetRejectCauseOfCashCount(const char *szErrCode, ADP_ERR_REJECT &usErrReject) const
{
    assert(szErrCode != NULL);
    string strTemp = szErrCode;
    VERIFYERRCODE(strTemp)

    if (strTemp.compare("84010000") == 0
        || strTemp.compare("84030000") == 0
        || strTemp.compare("840A0100") == 0
        || strTemp.compare("840A0200") == 0
        || strTemp.compare("840A0300") == 0
        || strTemp.compare("840A0400") == 0)//No note existed in CS todo
    {
        usErrReject = ADP_ERR_REJECT_NOBILL;
    }
    else if (strTemp.compare("8F010000") == 0
             || strTemp.compare("86020000") == 0
            )
    {
        //log_write(LOGFILE, "ErrCodeTranslate", "GetRejectCauseOfCashCount", LFS_ERR_CIM_NOITEMS,
        //  "存款时部分识别或无钞票被识别警告，警告码:%s", strTemp.c_str());
        usErrReject = ADP_ERR_REJECT_INVALIDBILL;
    }
    else if (strTemp.compare("91010000") == 0  //TS逻辑满
             || strTemp.compare("85991410") == 0
             || strTemp.compare("85991411") == 0
             || strTemp.compare("85010000") == 0
             || strTemp.compare("91000000") == 0    //TS物理满
             || strTemp.compare("91991410") == 0)   //总金额到达最大值

    {
        usErrReject = ADP_ERR_REJECT_STACKERFULL;
    }
    else // 86991220  RJSD满
    {
        usErrReject = ADP_ERR_REJECT_OTHER;
    }
    return TRUE;
}

BOOL ErrCodeTranslate::IsTooManyItemInCSOfCashCount(const char *szErrCode) const
{
    assert(szErrCode != NULL);
    string strTemp = szErrCode;
    VERIFYERRCODE(strTemp)
    strTemp.replace(5, 2, 2, '*');
    if (strTemp.compare("8E011**0") == 0)//CS太多钞票
    {
        return TRUE;
    }
    return FALSE;
}


BOOL ErrCodeTranslate::GetCUErr(const char *szErrCode, BOOL bCassEnabled[ADP_MAX_CU_SIZE],
                                ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) const
{
    assert(szErrCode != NULL);
    string strErrCode = szErrCode;
    VERIFYERRCODE(strErrCode)

    if (GetCUErrRetryable(strErrCode, bCassEnabled, arywCUERROR))
    {
        return TRUE;
    }
    else if (GetCUErrEmpty(strErrCode, bCassEnabled, arywCUERROR))
    {
        return TRUE;
    }
    else if (GetCUErrFull(strErrCode, bCassEnabled, arywCUERROR))
    {
        return TRUE;
    }
    else if (GetCUErrFatal(strErrCode, bCassEnabled, arywCUERROR))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL ErrCodeTranslate::GetCUErrEmpty(const string &strErrCode, BOOL bCassEnabled[ADP_MAX_CU_SIZE],
                                     ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) const
{
    assert(bCassEnabled != NULL);
    assert(arywCUERROR != NULL);
    string strTemp = strErrCode;
    strTemp.replace(7, 1, 1, '*');
    if (strTemp.compare("059B410*") == 0)//CASS empty 测试结果 文档没有此错误码的解释
    {
        int iIndex = atoi(strErrCode.substr(7, 1).c_str());
        iIndex--;
        if ((iIndex >= 0 && iIndex <= 4))
        {
            arywCUERROR[iIndex] = ADP_CUERROR_EMPTY;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL ErrCodeTranslate::GetCUErrFull(const string &strErrCode, BOOL bCassEnabled[ADP_MAX_CU_SIZE],
                                    ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) const
{
    assert(bCassEnabled != NULL);
    assert(arywCUERROR != NULL);

    string strTemp = strErrCode;
    strTemp.replace(6, 2, 2, '*');
    BYTE uctemp = 0x01;
    BOOL bCassFull = FALSE;
    if (strTemp.compare("059F10**") == 0)//CASS  FULL
    {
        string str1 = strErrCode.substr(6, 2);
        BYTE ucIndex = strtol(str1.c_str(), NULL, 16);
        for (int i = 0; i < ADP_MAX_CU_SIZE - 1; i++)
        {
            BYTE bbb = uctemp << i;
            if ((ucIndex & uctemp << i))
            {
                arywCUERROR[i] = ADP_CUERROR_FULL;
                bCassFull = TRUE;
            }
        }
    }
    return bCassFull;
}

BOOL ErrCodeTranslate::GetCUErrRetryable(const string &strErrCode, BOOL bCassEnabled[ADP_MAX_CU_SIZE],
                                         ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) const
{
    assert(bCassEnabled != NULL);
    assert(arywCUERROR != NULL);
    string strTemp = strErrCode;

    strTemp.replace(3, 1, 1, '*');
    strTemp.replace(6, 2, 2, '*');
    if (strTemp.compare("05A*15**") == 0
        || strTemp.compare("05A*19**") == 0
        || strTemp.compare("05A*55**") == 0
        || strTemp.compare("05A*59**") == 0)//Mis feeding

    {
        USHORT iDeno = atoi(strErrCode.substr(3, 1).c_str());
        iDeno -= 4;
        if ((iDeno >= 0 && iDeno <= 4))
        {
            arywCUERROR[iDeno] = ADP_CUERROR_RETRYABLE;
        }
        return TRUE;
    }

    strTemp = strErrCode;
    strTemp.replace(6, 1, 1, '*');
    if (strTemp.compare("05991A*A") == 0) //拒钞数达到最大

    {
        USHORT iDeno = atoi(strErrCode.substr(6, 1).c_str());
        iDeno -= 1;
        if ((iDeno >= 0 && iDeno <= 4))
        {
            arywCUERROR[iDeno] = ADP_CUERROR_RETRYABLE;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL ErrCodeTranslate::GetCUErrFatal(const string &strErrCode, BOOL bCassEnabled[ADP_MAX_CU_SIZE],
                                     ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE]) const
{
    assert(bCassEnabled != NULL);
    assert(arywCUERROR != NULL);
    string strTemp = strErrCode;

    strTemp.replace(3, 5, 5, '*');
    if (strTemp.compare("05A*****") == 0)
    {
        string s = strErrCode.substr(3, 1);
        USHORT usIndex = atoi(s.c_str());
        usIndex -= 4;
        if ((usIndex >= 0 && usIndex <= 4))
        {
            arywCUERROR[usIndex] = ADP_CUERROR_FATAL;
            return TRUE;
        }
    }

    return FALSE;
}
