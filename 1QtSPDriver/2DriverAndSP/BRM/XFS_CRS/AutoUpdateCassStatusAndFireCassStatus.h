//AutoUpdateCassStatusAndFireCassStatus.h

#ifndef AUTO_UPDATE_CASS_DATA_AND_FIRE_CASS_STATUS_H
#define AUTO_UPDATE_CASS_DATA_AND_FIRE_CASS_STATUS_H

#include "BRMCASHUNITINFOR.h"
#include "ICashUnitManager.h"
#include "BRMCONFIGPARAM.h"
#include "CashUnitDefine.h"
#include "ILogWrite.h"

#include <map>
using namespace std;


class XFS_CRSImp;

typedef map<ICashUnit *, CASHUNIT_STATUS> CU2STATUS;
typedef CU2STATUS::const_iterator CU2STATUSIT;

//自动更新钞箱状态并FIRE钞箱事件的类
//该类在析构会自动从SP适配层得到最新的钞箱状态，并设置到钞箱管理模块中
class CAutoUpdateCassStatusAndFireCassStatus //: public CLogManage
{
public:
    CAutoUpdateCassStatusAndFireCassStatus(XFS_CRSImp *pSP);
    ~CAutoUpdateCassStatusAndFireCassStatus();

private:
    XFS_CRSImp          *m_pSP;         //SP指针
    static CASHUNIT_STATUS m_CassStatus[ADP_MAX_CU_SIZE]; //保存最后的钞箱状态，为了在钞箱MISSING与正常切换时发出声音
    static CU2STATUS m_mapCU2Status;    //保存原来的钞箱状态，为CIMFireCashUnitThreshold
};


#endif //AUTO_UPDATE_CASS_DATA_AND_FIRE_CASS_STATUS_H
