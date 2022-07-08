#ifndef FMTIDCRESULTDATA_H
#define FMTIDCRESULTDATA_H
#include "XfsSPIHelper.h"

class CFmtIDCResultData
{
public:
    CFmtIDCResultData();
    ~CFmtIDCResultData();
    HRESULT Fmt_WFSIDCSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSIDCCAPS(LPVOID lpData, LPWFSRESULT &lpResult);

private:
    CQtDLLLoader<IWFMShareMenory>       m_pIWFM;
};

#endif // FMTIDCRESULTDATA_H
