/******************************************************************************
*                                                                             *
* xfscim310.h      XFS - Cash Acceptor (CIM) definitions                         *
*                                                                             *
*               Version 3.10 (29/11/2007)                                     *
*                                                                             *
******************************************************************************/
#ifndef __INC_XFSCIM310__H
#define __INC_XFSCIM310__H
//#ifdef __cplusplus
//extern "C" {
//#endif

#include "XFSCIM.H"
/* be aware of alignment */
#pragma pack (push, 1)

/* CIM Info Commands */
#define     WFS_INF_CIM_GET_ITEM_INFO           (CIM_SERVICE_OFFSET + 10)
#define     WFS_INF_CIM_POSITION_CAPABILITIES   (CIM_SERVICE_OFFSET + 11)


/* CIM Execute Commands */
#define     WFS_CMD_CIM_SET_GUIDANCE_LIGHT      (CIM_SERVICE_OFFSET + 17)
#define     WFS_CMD_CIM_CONFIGURE_NOTE_READER   (CIM_SERVICE_OFFSET + 18)
#define     WFS_CMD_CIM_COMPARE_P6_SIGNATURE    (CIM_SERVICE_OFFSET + 19)
#define     WFS_CMD_CIM_POWER_SAVE_CONTROL      (CIM_SERVICE_OFFSET + 20)


/* CIM Messages */
#define     WFS_EXEE_CIM_INFO_AVAILABLE         (CIM_SERVICE_OFFSET + 16)
#define     WFS_EXEE_CIM_INSERTITEMS            (CIM_SERVICE_OFFSET + 17)
#define     WFS_SRVE_CIM_DEVICEPOSITION         (CIM_SERVICE_OFFSET + 18)
#define     WFS_SRVE_CIM_POWER_SAVE_CHANGE      (CIM_SERVICE_OFFSET + 19)

/* Size and max index of dwGuidLights array */
#define     WFS_CIM_GUIDLIGHTS_SIZE             (32)
#define     WFS_CIM_GUIDLIGHTS_MAX              (WFS_CIM_GUIDLIGHTS_SIZE - 1)

/* Indices of WFSCIMSTATUS.dwGuidLights [...]
              WFSCIMCAPS.dwGuidLights [...]
*/

#define     WFS_CIM_GUIDANCE_POSINNULL          (0)
#define     WFS_CIM_GUIDANCE_POSINLEFT          (1)
#define     WFS_CIM_GUIDANCE_POSINRIGHT         (2)
#define     WFS_CIM_GUIDANCE_POSINCENTER        (3)
#define     WFS_CIM_GUIDANCE_POSINTOP           (4)
#define     WFS_CIM_GUIDANCE_POSINBOTTOM        (5)
#define     WFS_CIM_GUIDANCE_POSINFRONT         (6)
#define     WFS_CIM_GUIDANCE_POSINREAR          (7)
#define     WFS_CIM_GUIDANCE_POSOUTLEFT         (8)
#define     WFS_CIM_GUIDANCE_POSOUTRIGHT        (9)
#define     WFS_CIM_GUIDANCE_POSOUTCENTER       (10)
#define     WFS_CIM_GUIDANCE_POSOUTTOP          (11)
#define     WFS_CIM_GUIDANCE_POSOUTBOTTOM       (12)
#define     WFS_CIM_GUIDANCE_POSOUTFRONT        (13)
#define     WFS_CIM_GUIDANCE_POSOUTREAR         (14)
#define     WFS_CIM_GUIDANCE_POSOUTNULL         (15)

/* Values of WFSCIMSTATUS.dwGuidLights [...]
             WFSCIMCAPS.dwGuidLights [...]
*/

#define     WFS_CIM_GUIDANCE_NOT_AVAILABLE      (0x00000000)
#define     WFS_CIM_GUIDANCE_OFF                (0x00000001)
#define     WFS_CIM_GUIDANCE_SLOW_FLASH         (0x00000004)
#define     WFS_CIM_GUIDANCE_MEDIUM_FLASH       (0x00000008)
#define     WFS_CIM_GUIDANCE_QUICK_FLASH        (0x00000010)
#define     WFS_CIM_GUIDANCE_CONTINUOUS         (0x00000080)
#define     WFS_CIM_GUIDANCE_RED                (0x00000100)
#define     WFS_CIM_GUIDANCE_GREEN              (0x00000200)
#define     WFS_CIM_GUIDANCE_YELLOW             (0x00000400)
#define     WFS_CIM_GUIDANCE_BLUE               (0x00000800)
#define     WFS_CIM_GUIDANCE_CYAN               (0x00001000)
#define     WFS_CIM_GUIDANCE_MAGENTA            (0x00002000)
#define     WFS_CIM_GUIDANCE_WHITE              (0x00004000)

/* values of WFSCIMSTATUS.wDevicePosition
             WFSCIMDEVICEPOSITION.wPosition */

#define     WFS_CIM_DEVICEINPOSITION            (0)
#define     WFS_CIM_DEVICENOTINPOSITION         (1)
#define     WFS_CIM_DEVICEPOSUNKNOWN            (2)
#define     WFS_CIM_DEVICEPOSNOTSUPP            (3)

/* values of WFSCIMINPOS.fwPositionStatus */
#define     WFS_CIM_PSFOREIGNITEMS              (4)

/* values of WFSCIMCAPS.fwRetractTransportActions */
/* values of WFSCIMCAPS.fwRetractStackerActions */
#define     WFS_CIM_REJECT                      (0x0008)

/* values of WFSCIMCASHIN.fwType */
#define     WFS_CIM_TYPEREJECT                  (5)
#define     WFS_CIM_TYPECDMSPECIFIC             (6)

/* values of WFSCIMCASHINSTATUS.wStatus */
#define     WFS_CIM_CIRESET                     (5)

/* values of WFSCIMCAPS.fwRetractAreas */
/* values of WFSCIMRETRACT.usRetractArea */
#define     WFS_CIM_RA_REJECT                   (0x0020)

/* values of WFSCIMP6INFO.usLevel */
/* values of WFSCIMP6SIGNATURE.usLevel */
#define     WFS_CIM_LEVEL_4                     (4)

/* values for WFSCIMGETITEMINFO.wItemInfoType */
#define     WFS_CIM_ITEM_SERIALNUMBER           (0x00000001)
#define     WFS_CIM_ITEM_SIGNATURE              (0x00000002)

/* values of lpusReason in WFS_EXEE_CIM_INPUTREFUSE */
#define     WFS_CIM_FOREIGN_ITEMS_DETECTED      (7)
#define     WFS_CIM_INVALIDBUNCH                (8)
#define     WFS_CIM_COUNTERFEIT                 (9)

/* values of lpusReason in WFS_EXEE_CIM_NOTESERROR */
#define     WFS_CIM_OTHERNOTEERROR              (6)
#define     WFS_CIM_SHORTNOTEDETECTED           (7)

/* Values of fwUsage in WFS_INF_CIM_POSITION_CAPABILITIES */

#define     WFS_CIM_POSIN                       (0x0001)
#define     WFS_CIM_POSREFUSE                   (0x0002)
#define     WFS_CIM_POSROLLBACK                 (0x0004)

/* values of WFSCIMPOSITIONINFO.wAdditionalBunches */

#define     WFS_CIM_ADDBUNCHNONE                (1)
#define     WFS_CIM_ADDBUNCHONEMORE             (2)
#define     WFS_CIM_ADDBUNCHUNKNOWN             (3)

/* values of WFSCIMPOSITIONINFO.usBunchesRemaining */

#define     WFS_CIM_NUMBERUNKNOWN               (255)

/* WOSA/XFS CIM Errors */
#define WFS_ERR_CIM_INVALID_PORT                (-(CIM_SERVICE_OFFSET + 36))
#define WFS_ERR_CIM_FOREIGN_ITEMS_DETECTED      (-(CIM_SERVICE_OFFSET + 37))
#define WFS_ERR_CIM_LOADFAILED                  (-(CIM_SERVICE_OFFSET + 38))
#define WFS_ERR_CIM_CASHUNITNOTEMPTY            (-(CIM_SERVICE_OFFSET + 39))
#define WFS_ERR_CIM_INVALIDREFSIG               (-(CIM_SERVICE_OFFSET + 40))
#define WFS_ERR_CIM_INVALIDTRNSIG               (-(CIM_SERVICE_OFFSET + 41))
#define WFS_ERR_CIM_POWERSAVETOOSHORT           (-(CIM_SERVICE_OFFSET + 42))
#define WFS_ERR_CIM_POWERSAVEMEDIAPRESENT       (-(CIM_SERVICE_OFFSET + 43))

/*=================================================================*/
/* CIM Info Command Structures */
/*=================================================================*/
typedef struct _wfs_cim_P6_signature
{
    USHORT             usNoteId;
    ULONG              ulLength;
    DWORD              dwOrientation;
    LPVOID             lpSignature;
} WFSCIMP6SIGNATURE, *LPWFSCIMP6SIGNATURE;

typedef struct _wfs_cim_item_info
{
    USHORT                   usNoteID;
    LPWSTR                   lpszSerialNumber;
    LPWFSCIMP6SIGNATURE      lpP6Signature;
} WFSCIMITEMINFO, *LPWFSCIMITEMINFO;

typedef struct _wfs_cim_item_info_summary
{
    USHORT                   usLevel;
    USHORT                   usNumOfItems;
} WFSCIMITEMINFOSUMMARY, *LPWFSCIMITEMINFOSUMMARY;

typedef struct _wfs_cim_get_item_info
{
    USHORT                   usLevel;
    USHORT                   usIndex;
    DWORD                    dwItemInfoType;
} WFSCIMGETITEMINFO, *LPWFSCIMGETITEMINFO;

typedef struct _wfs_cim_pos_caps
{
    WORD                     fwPosition;
    WORD                     fwUsage;
    BOOL                     bShutterControl;
    BOOL                     bItemsTakenSensor;
    BOOL                     bItemsInsertedSensor;
    WORD                     fwRetractAreas;
    LPSTR                    lpszExtra;
} WFSCIMPOSCAPS, *LPWFSCIMPOSCAPS;

typedef struct _wfs_cim_pos_capabilities
{
    LPWFSCIMPOSCAPS          *lppPosCapabilities;
} WFSCIMPOSCAPABILITIES, *LPWFSCIMPOSCAPABILITIES;

/*=================================================================*/
/* CIM Execute Command Structures */
/*=================================================================*/

typedef struct _wfs_cim_set_guidlight
{
    WORD                     wGuidLight;
    DWORD                    dwCommand;
} WFSCIMSETGUIDLIGHT, *LPWFSCIMSETGUIDLIGHT;

typedef struct _wfs_cim_configure_note_reader
{
    BOOL                     bLoadAlways;
} WFSCIMCONFIGURENOTEREADER, *LPWFSCIMCONFIGURENOTEREADER;

typedef struct _wfs_cim_configure_note_reader_out
{
    BOOL                     bRebootNecessary;
} WFSCIMCONFIGURENOTEREADEROUT, *LPWFSCIMCONFIGURENOTEREADEROUT;

typedef struct _wfs_cim_P6_compare_signature
{
    LPWFSCIMP6SIGNATURE       *lppP6ReferenceSignatures;
    LPWFSCIMP6SIGNATURE       *lppP6Signatures;
} WFSCIMP6COMPARESIGNATURE, *LPWFSCIMP6COMPARESIGNATURE;

typedef struct _wfs_cim_P6_signatures_index
{
    USHORT                   usIndex;
    USHORT                   usConfidenceLevel;
    ULONG                    ulLength;
    LPVOID                   lpComparisonData;
} WFSCIMP6SIGNATURESINDEX, *LPWFSCIMP6SIGNATURESINDEX;

typedef struct _wfs_cim_P6_compare_result
{
    USHORT                    usCount;
    LPWFSCIMP6SIGNATURESINDEX *lppP6SignaturesIndex;
} WFSCIMP6COMPARERESULT, *LPWFSCIMP6COMPARERESULT;

typedef struct _wfs_cim_power_save_control
{
    USHORT                   usMaxPowerSaveRecoveryTime;
} WFSCIMPOWERSAVECONTROL, *LPWFSCIMPOWERSAVECONTROL;

/*=================================================================*/
/* CIM Message Structures */
/*=================================================================*/

typedef struct _wfs_cim_position_info
{
    WORD                     wPosition;
    WORD                     wAdditionalBunches;
    USHORT                   usBunchesRemaining;
} WFSCIMPOSITIONINFO, *LPWFSCIMPOSITIONINFO;

typedef struct _wfs_cim_device_position
{
    WORD                     wPosition;
} WFSCIMDEVICEPOSITION, *LPWFSCIMDEVICEPOSITION;

typedef struct _wfs_cim_power_save_change
{
    USHORT                   usPowerSaveRecoveryTime;
} WFSCIMPOWERSAVECHANGE, *LPWFSCIMPOWERSAVECHANGE;


/* restore alignment */
#pragma pack (pop)

//#ifdef __cplusplus
//}       /*extern "C"*/
//#endif
#endif /* __INC_XFSCIM__H */
