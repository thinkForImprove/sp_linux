#include "AutoFireDeviceStatus.h"
#include "XFS_CRS.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////
// CAutoFireDeviceStatus
//////////////////////////////////////////////////////////////////////
//为日志服务的宏定义
#define THISFILE                "AutoFireDeviceStatus"                      //定义为记录日志

ADPSTATUSINFOR CAutoFireDeviceStatus::m_ADPStatus = {(DEVICE_STATUS) - 1};  //设备状态，为FIRE状态改变服务
BOOL CAutoFireDeviceStatus::m_bIgnoreItemInserted = FALSE;
BOOL CAutoFireDeviceStatus::m_bIgnoreItemTaken = FALSE;

static inline const char *DevType2Desc(DEVICE_STATUS stDevice)
{
    switch (stDevice)
    {
    case ADP_DEVICE_ONLINE:    return "ADP_DEVICE_ONLINE ";
    case ADP_DEVICE_OFFLINE:   return "ADP_DEVICE_OFFLINE";
    case ADP_DEVICE_POWER_OFF: return "ADP_DEVICE_POWER_OFF";
    case ADP_DEVICE_NODEVICE:  return "ADP_DEVICE_NODEVICE";
    case ADP_DEVICE_HWERROR:   return "ADP_DEVICE_HWERROR";
    case ADP_DEVICE_USERERROR: return "ADP_DEVICE_USERERROR";
    case ADP_DEVICE_BUSY:      return "ADP_DEVICE_BUSY";
    default:                   return "UNKN";
    }
}

CAutoFireDeviceStatus::CAutoFireDeviceStatus(XFS_CRSImp *pSP)
{
    assert(pSP != NULL);

    m_pSP = pSP;
    if (m_ADPStatus.stDevice == (DEVICE_STATUS) - 1)
    {
        //m_pSP->GetUpdatedStatus(m_ADPStatus);
    }
    //SetLogFile(LOGFILE, ThisFile, "BRM");
}

CAutoFireDeviceStatus::~CAutoFireDeviceStatus()
{
    const char *ThisModule = "Destructor";
    assert(m_pSP->m_pAdapter);

    //Update ADP Device Status
    ADPSTATUSINFOR ADPStatus;
    m_pSP->GetUpdatedStatus(ADPStatus);

    if (m_ADPStatus.stStacker == ADP_STACKER_NOTEMPTY &&
        ADPStatus.stStacker == ADP_STACKER_EMPTY &&
        ADPStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY)
    {
        m_pSP->SetFlagOfCSItemFromTS(TRUE);
    }
    else if (ADPStatus.stOutPutPosition == ADP_OUTPOS_EMPTY)
    {
        m_pSP->SetFlagOfCSItemFromTS(FALSE);
    }

    //Fire Device Status Changed
    WORD wDispense = m_pSP->ComputeDispenseState(ADPStatus);
    WORD wAcceptor = m_pSP->ComputeAcceptorState(ADPStatus);
    WORD wOldDispense = m_pSP->ComputeDispenseState(m_ADPStatus);
    WORD wOldAcceptor = m_pSP->ComputeAcceptorState(m_ADPStatus);
    if (memcmp(&m_ADPStatus, &ADPStatus, sizeof(m_ADPStatus)) != 0 ||
        wDispense != wOldDispense ||
        wAcceptor != wOldAcceptor)
    {
        m_pSP->FireStatusChanged(
        m_pSP->m_StatusConvertor.ADP2XFS(ADPStatus.stDevice, SCT_DEVICE, TRUE));
        if (ADPStatus.stDevice != ADP_DEVICE_ONLINE)
        {
            m_pSP->FireDeviceHardWare();
            m_pSP->Log(ThisModule, -1, "机芯状态变为%s", DevType2Desc(ADPStatus.stDevice));
        }
    }

    if (ADPStatus.stSafeDoor != m_ADPStatus.stSafeDoor)
    {
        ADPCDMCAPS CDMCaps;
        ADPCIMCAPS CIMCaps;
        m_pSP->m_pAdapter->GetCDMCapabilities(&CDMCaps);
        m_pSP->m_pAdapter->GetCIMCapabilities(&CIMCaps);
        if (ADPStatus.stSafeDoor == ADP_SAFEDOOR_OPEN)
        {
            m_pSP->CDMFireSafeDoorOpen();
            m_pSP->Log(ThisModule, 0,
                       "m_pSP->CDMFireSafeDoorOpen()");
            m_pSP->CIMFireSafeDoorOpen();
            m_pSP->Log(ThisModule, 0,
                       "m_pSP->CIMFireSafeDoorOpen()");
        }
        else if (ADPStatus.stSafeDoor == ADP_SAFEDOOR_CLOSED)
        {
            m_pSP->CDMFireSafeDoorClose();
            m_pSP->Log(ThisModule, 0,
                       "m_pSP->CDMFireSafeDoorClose()");
            m_pSP->CIMFireSafeDoorClose();
            m_pSP->Log(ThisModule, 0,
                       "m_pSP->CIMFireSafeDoorClose()");
        }
    }

    //Fire item taken event
//    if (!m_bIgnoreItemTaken &&
//        m_ADPStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY &&
//        ADPStatus.stOutPutPosition == ADP_OUTPOS_EMPTY &&
//        (m_pSP->m_eWaitTaken != WTF_NONE || m_ADPStatus.stOutShutter != ADP_OUTSHUTTER_CLOSED) &&
//        ADPStatus.stShutterPosition == ADP_SHUTPOS_EMPTY)
        if (!m_bIgnoreItemTaken &&
            (m_ADPStatus.stShutterPosition ==  ADP_SHUTPOS_NOTEMPTY || m_ADPStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY)&&
            !(ADPStatus.stShutterPosition ==  ADP_SHUTPOS_NOTEMPTY || ADPStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY) &&
            (m_pSP->m_eWaitTaken != WTF_NONE || ADPStatus.stOutShutter != ADP_OUTSHUTTER_CLOSED))
    {
        ADPCDMCAPS CDMCaps;
        ADPCIMCAPS CIMCaps;
        m_pSP->m_pAdapter->GetCDMCapabilities(&CDMCaps);
        m_pSP->m_pAdapter->GetCIMCapabilities(&CIMCaps);
        if(m_pSP->m_bCdmCmd){
            if(m_pSP->m_eWaitTaken == WTF_CDM && m_pSP->m_Param.GetTakenEventAfterCloseShutterFlag()){

            } else {
                m_pSP->CDMFireItemTaken(CDMCaps.fwPositions);
            }
        } else {
            m_pSP->CIMFireItemTaken();
        }

        m_pSP->Log(ThisModule, 0,
                   "CDM&CIM OUTPOS FireItemTaken()");
    }

    if (m_pSP->m_Param.GetCashUnitType() == 2) //URT
    {
        if (!m_bIgnoreItemTaken &&
            m_ADPStatus.stInPutPosition == ADP_INPOS_NOTEMPTY &&
            ADPStatus.stInPutPosition == ADP_INPOS_EMPTY &&
            (m_pSP->m_eWaitTaken != WTF_NONE || m_ADPStatus.stInShutter != ADP_INSHUTTER_CLOSED) &&
            ADPStatus.stShutterPosition == ADP_SHUTPOS_EMPTY)
        {
            ADPCIMCAPS CIMCaps;
            m_pSP->m_pAdapter->GetCIMCapabilities(&CIMCaps);
            m_pSP->CIMFireItemTaken();
            m_pSP->Log(ThisModule, 0,
                       "CIM INPOS FireItemTaken()");
        }
    }

    //Fire item inserted event
//    if (!m_bIgnoreItemInserted &&
//        m_ADPStatus.stInPutPosition == ADP_INPOS_EMPTY &&
//        ADPStatus.stInPutPosition == ADP_INPOS_NOTEMPTY &&
//        ADPStatus.stShutterPosition == ADP_SHUTPOS_EMPTY &&
//        ADPStatus.stInShutter == ADP_INSHUTTER_OPEN)
    if (!m_bIgnoreItemInserted &&
       !(m_ADPStatus.stInPutPosition == ADP_INPOS_NOTEMPTY && m_ADPStatus.stShutterPosition == ADP_SHUTPOS_EMPTY) &&
       (ADPStatus.stInPutPosition == ADP_INPOS_NOTEMPTY && ADPStatus.stShutterPosition == ADP_SHUTPOS_EMPTY) &&
       ADPStatus.stInShutter == ADP_INSHUTTER_OPEN)
    {
        m_pSP->CIMFireItemsInserted();
        m_pSP->Log(ThisModule, 0,
                   "CIM INPOS CIMFireItemsInserted()");
    }

    m_bIgnoreItemInserted = FALSE;
    m_bIgnoreItemTaken = FALSE;

    //保存最后的状态
    m_ADPStatus = ADPStatus;
}
