﻿#include "StatusConvertor.h"
#include "IBRMAdapter.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStatusConvertor::CStatusConvertor()
{
    //键值为-1，当转换失败时采用的值

//--------Device-----------
	//CDM Device
	{
		STATUS_MAP &sm = m_mapData[0][SCT_DEVICE];
		sm[ADP_DEVICE_ONLINE] = WFS_CDM_DEVONLINE;
		sm[ADP_DEVICE_OFFLINE] = WFS_CDM_DEVOFFLINE;
		sm[ADP_DEVICE_POWER_OFF] = WFS_CDM_DEVPOWEROFF;
		sm[ADP_DEVICE_NODEVICE] = WFS_CDM_DEVNODEVICE;
		sm[ADP_DEVICE_HWERROR] = WFS_CDM_DEVHWERROR;
		sm[ADP_DEVICE_USERERROR] = WFS_CDM_DEVUSERERROR;
        sm[ADP_DEVICE_BUSY] = WFS_CDM_DEVBUSY;
		sm[-1] = WFS_CDM_DEVHWERROR;
	}
	//CIM Device
	{
		STATUS_MAP &sm = m_mapData[1][SCT_DEVICE];
		sm[ADP_DEVICE_ONLINE] = WFS_CIM_DEVONLINE;
		sm[ADP_DEVICE_OFFLINE] = WFS_CIM_DEVOFFLINE;
		sm[ADP_DEVICE_POWER_OFF] = WFS_CIM_DEVPOWEROFF;
		sm[ADP_DEVICE_NODEVICE] = WFS_CIM_DEVNODEVICE;
		sm[ADP_DEVICE_HWERROR] = WFS_CIM_DEVHWERROR;
        sm[ADP_DEVICE_BUSY] = WFS_CIM_DEVBUSY;
		sm[ADP_DEVICE_USERERROR] = WFS_CIM_DEVUSERERROR;
		sm[-1] = WFS_CIM_DEVHWERROR;
	}

    //--------SafeDoor-----------
    //CDM SafeDoor
    {
        STATUS_MAP &sm = m_mapData[0][SCT_SAFEDOOR];
        sm[ADP_SAFEDOOR_CLOSED] = WFS_CDM_DOORCLOSED;
        sm[ADP_SAFEDOOR_OPEN] = WFS_CDM_DOOROPEN;
        sm[ADP_SAFEDOOR_UNKNOWN] = WFS_CDM_DOORUNKNOWN;
        sm[ADP_SAFEDOOR_NOTSUPPORTED] = WFS_CDM_DOORNOTSUPPORTED;
        sm[-1] = WFS_CDM_DOORUNKNOWN;
    }
    //CIM SafeDoor
    {
        STATUS_MAP &sm = m_mapData[1][SCT_SAFEDOOR];
        sm[ADP_SAFEDOOR_CLOSED] = WFS_CIM_DOORCLOSED;
        sm[ADP_SAFEDOOR_OPEN] = WFS_CIM_DOOROPEN;
        sm[ADP_SAFEDOOR_UNKNOWN] = WFS_CIM_DOORUNKNOWN;
        sm[ADP_SAFEDOOR_NOTSUPPORTED] = WFS_CIM_DOORNOTSUPPORTED;
        sm[-1] = WFS_CIM_DOORUNKNOWN;
    }

    //--------Shutter-----------
    //CDM OutShutter
    {
        STATUS_MAP &sm = m_mapData[0][SCT_OUTSHUTTER];
        sm[ADP_OUTSHUTTER_CLOSED] = WFS_CDM_SHTCLOSED;
        sm[ADP_OUTSHUTTER_OPEN] = WFS_CDM_SHTOPEN;
        sm[ADP_OUTSHUTTER_UNKNOWN] = WFS_CDM_SHTUNKNOWN;
        sm[ADP_OUTSHUTTER_NOTSUPPORTED] = WFS_CDM_SHTNOTSUPPORTED;
        sm[ADP_OUTSHUTTER_JAMMED] = WFS_CDM_SHTJAMMED;
        sm[-1] = WFS_CDM_SHTUNKNOWN;
    }
    //CIM OutShutter
    {
        STATUS_MAP &sm = m_mapData[1][SCT_OUTSHUTTER];
        sm[ADP_OUTSHUTTER_CLOSED] = WFS_CIM_SHTCLOSED;
        sm[ADP_OUTSHUTTER_OPEN] = WFS_CIM_SHTOPEN;
        sm[ADP_OUTSHUTTER_UNKNOWN] = WFS_CIM_SHTUNKNOWN;
        sm[ADP_OUTSHUTTER_NOTSUPPORTED] = WFS_CIM_SHTNOTSUPPORTED;
        sm[ADP_OUTSHUTTER_JAMMED] = WFS_CIM_SHTJAMMED;
        sm[-1] = WFS_CIM_SHTUNKNOWN;
    }

    //CDM InShutter
    {
        STATUS_MAP &sm = m_mapData[0][SCT_INSHUTTER];
        sm[ADP_INSHUTTER_CLOSED] = WFS_CDM_SHTCLOSED;
        sm[ADP_INSHUTTER_OPEN] = WFS_CDM_SHTOPEN;
        sm[ADP_INSHUTTER_UNKNOWN] = WFS_CDM_SHTUNKNOWN;
        sm[ADP_INSHUTTER_NOTSUPPORTED] = WFS_CDM_SHTNOTSUPPORTED;
        sm[ADP_INSHUTTER_JAMMED] = WFS_CDM_SHTJAMMED;
        sm[-1] = WFS_CDM_SHTUNKNOWN;
    }
    //CIM InShutter
    {
        STATUS_MAP &sm = m_mapData[1][SCT_INSHUTTER];
        sm[ADP_INSHUTTER_CLOSED] = WFS_CIM_SHTCLOSED;
        sm[ADP_INSHUTTER_OPEN] = WFS_CIM_SHTOPEN;
        sm[ADP_INSHUTTER_UNKNOWN] = WFS_CIM_SHTUNKNOWN;
        sm[ADP_INSHUTTER_NOTSUPPORTED] = WFS_CIM_SHTNOTSUPPORTED;
        sm[ADP_INSHUTTER_JAMMED] = WFS_CIM_SHTJAMMED;
        sm[-1] = WFS_CIM_SHTUNKNOWN;
    }

    //--------Output-----------
    //CDM Outpos
    {
        STATUS_MAP &sm = m_mapData[0][SCT_OUTPOS];
        sm[ADP_OUTPOS_EMPTY] = WFS_CDM_PSEMPTY;
        sm[ADP_OUTPOS_NOTEMPTY] = WFS_CDM_PSNOTEMPTY;
        sm[ADP_OUTPOS_UNKNOWN] = WFS_CDM_PSUNKNOWN;
        sm[ADP_OUTPOS_NOTSUPPORTED] = WFS_CDM_PSNOTSUPPORTED;
        sm[-1] = WFS_CDM_PSUNKNOWN;
    }
    //CIM Outpos
    {
        STATUS_MAP &sm = m_mapData[1][SCT_OUTPOS];
        sm[ADP_OUTPOS_EMPTY] = WFS_CIM_PSEMPTY;
        sm[ADP_OUTPOS_NOTEMPTY] = WFS_CIM_PSNOTEMPTY;
        sm[ADP_OUTPOS_UNKNOWN] = WFS_CIM_PSUNKNOWN;
        sm[ADP_OUTPOS_NOTSUPPORTED] = WFS_CIM_PSNOTSUPPORTED;
        sm[-1] = WFS_CIM_PSUNKNOWN;
    }

    //--------Input-----------
    //CDM Inpos
    {
        STATUS_MAP &sm = m_mapData[0][SCT_INPOS];
        sm[ADP_INPOS_NOTSUPPORTED] = WFS_CDM_PSNOTSUPPORTED;
        sm[-1] = WFS_CDM_PSUNKNOWN;
    }
    //CIM Inpos
    {
        STATUS_MAP &sm = m_mapData[1][SCT_INPOS];
        sm[ADP_INPOS_EMPTY] = WFS_CIM_PSEMPTY;
        sm[ADP_INPOS_NOTEMPTY] = WFS_CIM_PSNOTEMPTY;
        sm[ADP_INPOS_UNKNOWN] = WFS_CIM_PSUNKNOWN;
        sm[ADP_INPOS_NOTSUPPORTED] = WFS_CIM_PSNOTSUPPORTED;
        sm[-1] = WFS_CIM_PSUNKNOWN;
    }

    //--------Stacker-----------
    //CDM Stacker
    {
        STATUS_MAP &sm = m_mapData[0][SCT_STACKER];
        sm[ADP_STACKER_EMPTY] = WFS_CDM_ISEMPTY;
        sm[ADP_STACKER_NOTEMPTY] = WFS_CDM_ISNOTEMPTY;
        sm[ADP_STACKER_FULL] = WFS_CDM_ISNOTEMPTY;                  //30-00-00-00(FT#0029)
        //      sm[ADP_CASHUNIT_] = WFS_CDM_ISNOTEMPTYCUST;
        //      sm[ADP_CASHUNIT_] = WFS_CDM_ISNOTEMPTYUNK;
        sm[ADP_STACKER_UNKNOWN] = WFS_CDM_ISUNKNOWN;
        sm[ADP_STACKER_NOTSUPPORTED] = WFS_CDM_ISNOTSUPPORTED;
        sm[-1] = WFS_CDM_ISUNKNOWN;
    }
    //CIM Stacker
    {
        STATUS_MAP &sm = m_mapData[1][SCT_STACKER];
        sm[ADP_STACKER_EMPTY] = WFS_CIM_ISEMPTY;
        sm[ADP_STACKER_NOTEMPTY] = WFS_CIM_ISNOTEMPTY;
        sm[ADP_STACKER_FULL] = WFS_CIM_ISFULL;                          //30-00-00-00(FT#0029)
        //      sm[ADP_CASHUNIT_] = WFS_CIM_ISNOTEMPTYCUST;
        //      sm[ADP_CASHUNIT_] = WFS_CIM_ISNOTEMPTYUNK;
        sm[ADP_STACKER_UNKNOWN] = WFS_CIM_ISUNKNOWN;
        sm[ADP_STACKER_NOTSUPPORTED] = WFS_CIM_ISNOTSUPPORTED;
        sm[-1] = WFS_CIM_ISUNKNOWN;
    }

    //--------Transport-----------
    //CDM Transport
    {
        STATUS_MAP &sm = m_mapData[0][SCT_TRANSPORT];
        sm[ADP_TRANSPORT_EMPTY] = WFS_CDM_TPSTATEMPTY;
        sm[ADP_TRANSPORT_NOTEMPTY] = WFS_CDM_TPSTATNOTEMPTY;
        sm[ADP_TRANSPORT_UNKNOWN] = WFS_CDM_TPSTATNOTEMPTY_UNK;
        sm[ADP_TRANSPORT_NOTSUPPORTED] = WFS_CDM_TPSTATNOTSUPPORTED;
        sm[-1] = WFS_CDM_TPSTATNOTEMPTY_UNK;
    }
    //CIM Transport
    {
        STATUS_MAP &sm = m_mapData[1][SCT_TRANSPORT];
        sm[ADP_TRANSPORT_EMPTY] = WFS_CIM_TPSTATEMPTY;
        sm[ADP_TRANSPORT_NOTEMPTY] = WFS_CIM_TPSTATNOTEMPTY;
        sm[ADP_TRANSPORT_UNKNOWN] = WFS_CIM_TPSTATNOTEMPTY_UNK;
        sm[ADP_TRANSPORT_NOTSUPPORTED] = WFS_CIM_TPSTATNOTSUPPORTED;
        sm[-1] = WFS_CIM_TPSTATNOTEMPTY_UNK;
    }

    //--------NoteReader-----------
    //CDM NotReader
    {
        STATUS_MAP &sm = m_mapData[0][SCT_NOTE_READER];
        sm[-1] = -1;
    }
    //CIM NotReader
    {
        STATUS_MAP &sm = m_mapData[1][SCT_NOTE_READER];
        sm[ADP_BR_OK] = WFS_CIM_BNROK;
        sm[ADP_BR_INOP] = WFS_CIM_BNRINOP;
        sm[ADP_BR_UNKNOWN] = WFS_CIM_BNRUNKNOWN;
        sm[ADP_BR_NOTSUPPORTED] = WFS_CIM_BNRNOTSUPPORTED;
        sm[-1] = WFS_CIM_BNRUNKNOWN;
    }

    //--------CassetteStatus-----------
    //CDM CassetteStatus
    {
        STATUS_MAP &sm = m_mapData[0][SCT_CASS_STATUS];
        sm[ADP_CASHUNIT_OK] = WFS_CDM_STATCUOK;
        sm[ADP_CASHUNIT_FULL] = WFS_CDM_STATCUFULL;
        sm[ADP_CASHUNIT_HIGH] = WFS_CDM_STATCUHIGH;
        sm[ADP_CASHUNIT_LOW] = WFS_CDM_STATCULOW;
        sm[ADP_CASHUNIT_EMPTY] = WFS_CDM_STATCUEMPTY;
        sm[ADP_CASHUNIT_INOP] = WFS_CDM_STATCUINOP;
        sm[ADP_CASHUNIT_MISSING] = WFS_CDM_STATCUMISSING;
        //      sm[ADP_CASHUNIT_NOVAL] = WFS_CDM_STATCUNOVAL;
        //      sm[ADP_CASHUNIT_NOREF] = WFS_CDM_STATCUNOREF;
        sm[ADP_CASHUNIT_MANIP] = WFS_CDM_STATCUMANIP;
        sm[-1] = WFS_CDM_STATCUINOP;
    }
    //CIM CassetteStatus
    {
        STATUS_MAP &sm = m_mapData[1][SCT_CASS_STATUS];
        sm[ADP_CASHUNIT_OK] = WFS_CIM_STATCUOK;
        sm[ADP_CASHUNIT_FULL] = WFS_CIM_STATCUFULL;
        sm[ADP_CASHUNIT_HIGH] = WFS_CIM_STATCUHIGH;
        sm[ADP_CASHUNIT_LOW] = WFS_CIM_STATCULOW;
        sm[ADP_CASHUNIT_EMPTY] = WFS_CIM_STATCUEMPTY;
        sm[ADP_CASHUNIT_INOP] = WFS_CIM_STATCUINOP;
        sm[ADP_CASHUNIT_MISSING] = WFS_CIM_STATCUMISSING;
        //      sm[ADP_CASHUNIT_NOVAL] = WFS_CIM_STATCUNOVAL;
        //      sm[ADP_CASHUNIT_NOREF] = WFS_CIM_STATCUNOREF;
        sm[ADP_CASHUNIT_MANIP] = WFS_CIM_STATCUMANIP;
        sm[-1] = WFS_CDM_STATCUINOP;
    }
}

CStatusConvertor::~CStatusConvertor()
{

}
