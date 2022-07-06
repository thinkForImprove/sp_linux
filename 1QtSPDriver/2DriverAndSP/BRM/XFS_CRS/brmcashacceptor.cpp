//test#5
#include "brmcashacceptor.h"
#include "ILogWrite.h"
#include <assert.h>
#define THISFILE            "BRMCASHACCEPTOR"        //记录日志的当前文件名

int _tag_brm_cashacceptor_param::LoadCashAcceptorParam(LPCSTR lpszFileName)
{
    const char *ThisModule = "LoadCashAcceptorParam";

    int iRet = m_BrmCashAcceptorFile.Load(lpszFileName);
/*    if (iRet < 0)
    {
        Log(ThisModule, -1,
            "m_BrmCashAcceptorFile.Load(%s) failed(iRet = %d)",
            lpszFileName, iRet);
        return iRet;
    } */ //test#5 //文件CashAcceptor.ini不存在时，OPEN报INTERNAL异常，所以注释掉

    iCashAcceptorParam = m_BrmCashAcceptorFile.GetInt("CFG", "OnceAceeptCashAmount", 0);
    return 0;
}

void _tag_brm_cashacceptor_param::Clear()
{
    iCashAcceptorParam = 0;
}


