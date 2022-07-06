#include "AutoSaveDataAndFireEvent.h"
#include "XFS_CRS.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////
// CAutoSaveDataAndFireEvent
//////////////////////////////////////////////////////////////////////
CAutoSaveDataAndFireEvent::CAutoSaveDataAndFireEvent(XFS_CRSImp *pSP) :
    m_AutoSaveDataToFile(pSP),
    m_AutoUpdateCassStatusAndFireCassStatus(pSP),
    m_AutoFireDeviceStatus(pSP)
{
    assert(pSP != NULL);
}

CAutoSaveDataAndFireEvent::~CAutoSaveDataAndFireEvent()
{
}
