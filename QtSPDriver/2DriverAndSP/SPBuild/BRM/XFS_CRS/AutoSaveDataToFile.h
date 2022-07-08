//AutoSaveDataToFile.h

#ifndef AUTO_SAVE_DATA_TO_FILE_H
#define AUTO_SAVE_DATA_TO_FILE_H

#include "BRMPRESENTINFOR.h"
#include "BRMCASHININFOR.h"

class XFS_CRSImp;

//自动保存数据到文件的辅助类
class CAutoSaveDataToFile
{
public:
    CAutoSaveDataToFile(XFS_CRSImp *pSP);
    virtual ~CAutoSaveDataToFile();

private:
    XFS_CRSImp          *m_pSP;         //SP指针

    //以下数据是构造时保存的老数据
    BRMPRESENTINFOR     m_PresentInfor; //PRESENT状态
    BRMCASHININFOR      m_CashInInfor;  //CASH IN STATUS
};

#endif //AUTO_SAVE_DATA_TO_FILE_H

