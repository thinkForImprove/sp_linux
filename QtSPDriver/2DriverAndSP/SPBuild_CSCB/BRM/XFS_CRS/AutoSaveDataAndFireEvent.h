// AutoSaveDataAndFireEvent.h: interface for the CAutoSaveDataAndFireEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOSAVEDATAANDFIREEVENT_H__4DD722C7_C1C6_4726_8B9D_75E40CA64723__INCLUDED_)
#define AFX_AUTOSAVEDATAANDFIREEVENT_H__4DD722C7_C1C6_4726_8B9D_75E40CA64723__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AutoFireDeviceStatus.h"
#include "AutoUpdateCassStatusAndFireCassStatus.h"
#include "AutoSaveDataToFile.h"

class XFS_CRSImp;

//自动更新数据到SP、保存到文件并FIRE事件的辅助类
class CAutoSaveDataAndFireEvent
{
public:
    CAutoSaveDataAndFireEvent(XFS_CRSImp *pSP);
    virtual ~CAutoSaveDataAndFireEvent();

    //内部数据成员
private: //注意： 以下三个数据成员的顺序不能随意改变
    CAutoSaveDataToFile m_AutoSaveDataToFile;       //自动保存数据到文件
    CAutoUpdateCassStatusAndFireCassStatus m_AutoUpdateCassStatusAndFireCassStatus; //自动更新钞箱状态并FIRE状态改变
    CAutoFireDeviceStatus m_AutoFireDeviceStatus;   //自动更新设备状态并FIRE设备状态改变
};

#endif // !defined(AFX_AUTOSAVEDATAANDFIREEVENT_H__4DD722C7_C1C6_4726_8B9D_75E40CA64723__INCLUDED_)
