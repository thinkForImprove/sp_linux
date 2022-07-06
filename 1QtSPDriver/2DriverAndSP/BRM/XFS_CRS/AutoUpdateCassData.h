//AutoUpdateCassData.h

#ifndef AUTO_UPDATE_CASS_DATA_H
#define AUTO_UPDATE_CASS_DATA_H

#include "IBRMAdapter.h"

class XFS_CRSImp;

#include "vector"
using namespace std;

typedef pair<USHORT, ULONG>      NOTEID_COUNT;
typedef vector<NOTEID_COUNT>    NOTEID_COUNT_ARRAY;

//自动更新钞箱数据的辅助类
class CAutoUpdateCassData
{
public:
    CAutoUpdateCassData(XFS_CRSImp *pSP,
                        ULONG ulDispenseCounts[ADP_MAX_CU_SIZE],
                        ULONG ulRejectCounts[ADP_MAX_CU_SIZE],
                        ULONG ulRetractCount[ADP_MAX_CU_SIZE],
                        NOTEID_COUNT_ARRAY NoteIDCountArray[ADP_MAX_CU_SIZE],
                        BOOL bCDMOperation,
                        BOOL bSubstractRejectCountFromCount = FALSE);

    ~CAutoUpdateCassData();

private:
    void UpdateCountToCUManager();

private:
    XFS_CRSImp *m_pSP;              //指向SP对象
    BOOL        m_bCDMOperation;    //指示是CDM操作还是CIM操作
    BOOL        m_bSubstractRejectCountFromCount; //是否从钞箱的Count中减去RejectCount
    ULONG *m_pulDispenseCounts;     //各钞箱的出钞数
    ULONG *m_pulRejectCounts;       //各钞箱的拒钞数
    ULONG  *m_pulRetractCounts;     //各钞箱的回收数
    NOTEID_COUNT_ARRAY *m_pNoteIDCountArray; //钞币ID数组
};


#endif //AUTO_UPDATE_CASS_DATA_H
