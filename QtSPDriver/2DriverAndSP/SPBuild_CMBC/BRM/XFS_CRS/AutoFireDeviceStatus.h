//AutoFireDeviceStatus.h
#ifndef AUTO_FIRE_DEVICE_STATUS_H
#define AUTO_FIRE_DEVICE_STATUS_H

#include "IBRMAdapter.h"
#include "ILogWrite.h"
class XFS_CRSImp;

//自动更新设备状态并FIRE设备事件的类（包括ITEM TAKEN和ITEM INSERT事件）
//该类在析构会自动从SP适配层得到最新的设备状态
//该类支持嵌套调用
class CAutoFireDeviceStatus //: public CLogManage
{
public:
    CAutoFireDeviceStatus(XFS_CRSImp *pSP);
    ~CAutoFireDeviceStatus();

    //忽略一次ITEM插入取出事件（是设备操作造成ITEM变化）
    static void IgnoreItemTakenInsertedOnce(BOOL bIgnoreItemInserted = TRUE, BOOL bIgnoreItemTaken = TRUE)
    {
        m_bIgnoreItemInserted = bIgnoreItemInserted;
        m_bIgnoreItemTaken = bIgnoreItemTaken;
    }
private:
    XFS_CRSImp          *m_pSP;         //SP指针

    //以下数据是保存最后的状态
    static ADPSTATUSINFOR   m_ADPStatus;    //保存最后的设备状态，为FIRE状态改变服务
    static BOOL             m_bIgnoreItemInserted;
    static BOOL             m_bIgnoreItemTaken;
};


#endif //AUTO_FIRE_DEVICE_STATUS_H
