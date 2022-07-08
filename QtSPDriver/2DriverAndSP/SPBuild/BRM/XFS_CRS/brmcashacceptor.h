//test#5
#ifndef BRMCASHACCEPTOR_H
#define BRMCASHACCEPTOR_H

#include "XFSCIM.H"
#include "XFSCDM.H"
#include <string>
#include "SPConfigFile.h"
#include "MultiString.h"
#include "ILogWrite.h"

//所有相关的配置函数
typedef struct _tag_brm_cashacceptor_param : public CLogManage
{
    //get limit count
    int GetCashAcceptorParam() const
    {
        return iCashAcceptorParam;
    }

    //从配置文件装入参数，先看ETCDIR,再看工作目录
    //lpszFileName: 不带路径的文件名
    int LoadCashAcceptorParam(LPCSTR lpszFileName);

    //释放使用过程的分配的内存
    void Clear();

    //Check digit
    int CheckDigit(DWORD dwCashAcceptor);

    //构造函数，清空数据
    _tag_brm_cashacceptor_param()
    {
        iCashAcceptorParam = 0;
        SetLogFile(LOGFILE, "_tag_brm_cashacceptor_param", "BRM");
    }

     //析构函数，释放使用过程的分配的内存
     virtual ~_tag_brm_cashacceptor_param()
     {
         Clear();
     }
 private:
    int iCashAcceptorParam;
    CSPConfigFile m_BrmCashAcceptorFile;        //具体读写配置文件的对象
}BRMCASHACCEPTORPARAM, *LPBRMCASHACCEPTORPARAM;

#endif // BRMCASHACCEPTOR_H
