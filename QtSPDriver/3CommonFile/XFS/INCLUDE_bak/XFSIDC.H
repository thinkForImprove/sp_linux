/******************************************************************************
 *                                                                             *
 * xfsidc.h    XFS - Identification card unit (IDC) definitions                *
 *                                                                             *
 *             Version 3.02  (09/05/03)                                        *
 *                                                                             *
 ******************************************************************************/

#ifndef __INC_XFSIDC__H
#define __INC_XFSIDC__H

#ifdef __cplusplus
extern "C"
{
#endif

#include "XFSAPI.H"

/*   be aware of alignment   */
#pragma pack(push, 1)

/* values of WFSIDCCAPS.wClass */

/* XFS_COMMON.H 中定义
#ifdef QT_WIN_LINUX_XFS

    #define WFS_SERVICE_CLASS_IDC         (2)
    #define WFS_SERVICE_CLASS_VERSION_IDC 0x0303
#else

    #define WFS_SERVICE_CLASS_IDC         (3)
    #define WFS_SERVICE_CLASS_VERSION_IDC 0x0001
#endif
*/

//增加广电关于退卡模块的扩展接口
#define CARD_REJECT_GD_MODE

#define WFS_SERVICE_CLASS_NAME_IDC "IDC"
#define IDC_SERVICE_OFFSET         (WFS_SERVICE_CLASS_IDC * 100)

    /* IDC Info Commands */

#define WFS_INF_IDC_STATUS       (IDC_SERVICE_OFFSET + 1)
#define WFS_INF_IDC_CAPABILITIES (IDC_SERVICE_OFFSET + 2)
#define WFS_INF_IDC_FORM_LIST    (IDC_SERVICE_OFFSET + 3)
#define WFS_INF_IDC_QUERY_FORM   (IDC_SERVICE_OFFSET + 4)
#define WFS_INF_IDC_QUERY_IFM_IDENTIFIER    (IDC_SERVICE_OFFSET + 5) 
#define WFS_INF_IDC_MCR_CAPABILITIES	  	(IDC_SERVICE_OFFSET + 51)
#define	WFS_INF_IDC_MCR_STATUS				(IDC_SERVICE_OFFSET + 52)
#define WFS_INF_IDC_SLOTS_INFO	            (IDC_SERVICE_OFFSET + 53)

    /* IDC Execute Commands */

#define WFS_CMD_IDC_READ_TRACK     (IDC_SERVICE_OFFSET + 1)
#define WFS_CMD_IDC_WRITE_TRACK    (IDC_SERVICE_OFFSET + 2)
#define WFS_CMD_IDC_EJECT_CARD     (IDC_SERVICE_OFFSET + 3)
#define WFS_CMD_IDC_RETAIN_CARD    (IDC_SERVICE_OFFSET + 4)
#define WFS_CMD_IDC_RESET_COUNT    (IDC_SERVICE_OFFSET + 5)
#define WFS_CMD_IDC_SETKEY         (IDC_SERVICE_OFFSET + 6)
#define WFS_CMD_IDC_READ_RAW_DATA  (IDC_SERVICE_OFFSET + 7)
#define WFS_CMD_IDC_WRITE_RAW_DATA (IDC_SERVICE_OFFSET + 8)
#define WFS_CMD_IDC_CHIP_IO        (IDC_SERVICE_OFFSET + 9)
#define WFS_CMD_IDC_RESET          (IDC_SERVICE_OFFSET + 10)
#define WFS_CMD_IDC_CHIP_POWER     (IDC_SERVICE_OFFSET + 11)
#define WFS_CMD_IDC_PARSE_DATA     (IDC_SERVICE_OFFSET + 12)
#define WFS_CMD_IDC_SET_GUIDANCE_LIGHT      (IDC_SERVICE_OFFSET + 13) 
#define WFS_CMD_IDC_POWER_SAVE_CONTROL      (IDC_SERVICE_OFFSET + 14) 
#define WFS_CMD_IDC_RETRACT_CARD	            (IDC_SERVICE_OFFSET + 91)
#define WFS_CMD_IDC_HANDBACK_CARD           (IDC_SERVICE_OFFSET + 92)
#define WFS_CMD_IDC_CLEAR_RETRACT_CARD      (IDC_SERVICE_OFFSET + 93)
#define	WFS_CMD_IDC_RETAIN_SLOT_START 		(IDC_SERVICE_OFFSET + 51)
#define WFS_CMD_IDC_RETAIN_SLOT_END 		(IDC_SERVICE_OFFSET + 52)
#define WFS_CMD_IDC_EJECT_SLOT_START 		(IDC_SERVICE_OFFSET + 53)
#define	WFS_CMD_IDC_EJECT_SLOT_END 			(IDC_SERVICE_OFFSET + 54)
#define WFS_CMD_IDC_MCR_RESET 				(IDC_SERVICE_OFFSET + 58)
#define	WFS_CMD_IDC_RETAIN_SLOT_START_EX 	(IDC_SERVICE_OFFSET + 96)
#define WFS_CMD_IDC_MCR_RETAIN_CARD			(IDC_SERVICE_OFFSET + 97)
#define WFS_CMD_IDC_OPEN_SHUTTER			(IDC_SERVICE_OFFSET + 98)
#define WFS_CMD_IDC_CLOSE_SHUTTER           (IDC_SERVICE_OFFSET + 99)

//退卡模块
#ifdef CARD_REJECT_GD_MODE
    #define WFS_CMD_IDC_CMEJECT_CARD   (IDC_SERVICE_OFFSET + 13)
    #define WFS_CMD_IDC_CMSTATUS       (IDC_SERVICE_OFFSET + 14)
    #define WFS_CMD_IDC_SETCARDDATA    (IDC_SERVICE_OFFSET + 16)
    #define WFS_CMD_IDC_CMRETAIN_CARD  (IDC_SERVICE_OFFSET + 17)
    #define WFS_CMD_IDC_CMREDUCE_COUNT (IDC_SERVICE_OFFSET + 19)
    #define WFS_CMD_IDC_CMSET_COUNT    (IDC_SERVICE_OFFSET + 21)
    #define WFS_CMD_CMEMPTY_CARD       (IDC_SERVICE_OFFSET + 22)
    #define WFS_CMD_IDC_GETCARDINFO    (IDC_SERVICE_OFFSET + 23)
    #define WFS_CMD_IDC_CMRESET        (IDC_SERVICE_OFFSET + 24)
    //针对读卡器的扩展
    #define WFS_CMD_IDC_REDUCE_COUNT     (IDC_SERVICE_OFFSET + 50)
    #define WFS_CMD_IDC_SET_COUNT        (IDC_SERVICE_OFFSET + 51)
    #define WFS_CMD_IDC_INTAKE_CARD_BACK (IDC_SERVICE_OFFSET + 60)
#endif

    /* IDC Messages */

#define WFS_EXEE_IDC_INVALIDTRACKDATA   (IDC_SERVICE_OFFSET + 1)
#define WFS_EXEE_IDC_MEDIAINSERTED      (IDC_SERVICE_OFFSET + 3)
#define WFS_SRVE_IDC_MEDIAREMOVED       (IDC_SERVICE_OFFSET + 4)
#define WFS_SRVE_IDC_CARDACTION         (IDC_SERVICE_OFFSET + 5)
#define WFS_USRE_IDC_RETAINBINTHRESHOLD (IDC_SERVICE_OFFSET + 6)
#define WFS_EXEE_IDC_INVALIDMEDIA       (IDC_SERVICE_OFFSET + 7)
#define WFS_EXEE_IDC_MEDIARETAINED      (IDC_SERVICE_OFFSET + 8)
#define WFS_SRVE_IDC_MEDIADETECTED      (IDC_SERVICE_OFFSET + 9)
#define WFS_SRVE_IDC_RETAINBININSERTED  (IDC_SERVICE_OFFSET + 10) 
#define WFS_SRVE_IDC_RETAINBINREMOVED   (IDC_SERVICE_OFFSET + 11) 
#define WFS_EXEE_IDC_INSERTCARD         (IDC_SERVICE_OFFSET + 12) 
#define WFS_SRVE_IDC_DEVICEPOSITION     (IDC_SERVICE_OFFSET + 13) 
#define WFS_SRVE_IDC_POWER_SAVE_CHANGE  (IDC_SERVICE_OFFSET + 14) 

/* values of WFSIDCSTATUS.fwDevice */
#define WFS_IDC_DEVONLINE    WFS_STAT_DEVONLINE
#define WFS_IDC_DEVOFFLINE   WFS_STAT_DEVOFFLINE
#define WFS_IDC_DEVPOWEROFF  WFS_STAT_DEVPOWEROFF
#define WFS_IDC_DEVNODEVICE  WFS_STAT_DEVNODEVICE
#define WFS_IDC_DEVHWERROR   WFS_STAT_DEVHWERROR
#define WFS_IDC_DEVUSERERROR WFS_STAT_DEVUSERERROR
#define WFS_IDC_DEVBUSY      WFS_STAT_DEVBUSY

    /* values of WFSIDCSTATUS.fwMedia, WFSIDCRETAINCARD.fwPosition,  */
    /* WFSIDCCARDACT.fwPosition  */

#define WFS_IDC_MEDIAPRESENT    (1)
#define WFS_IDC_MEDIANOTPRESENT (2)
#define WFS_IDC_MEDIAJAMMED     (3)
#define WFS_IDC_MEDIANOTSUPP    (4)
#define WFS_IDC_MEDIAUNKNOWN    (5)
#define WFS_IDC_MEDIAENTERING   (6)
#define WFS_IDC_MEDIALATCHED    (7)

    /* values of WFSIDCSTATUS.fwRetainBin */

#define WFS_IDC_RETAINBINOK   (1)
#define WFS_IDC_RETAINNOTSUPP (2)
#define WFS_IDC_RETAINBINFULL (3)
#define WFS_IDC_RETAINBINHIGH (4)
#define WFS_IDC_RETAINBINMISSING            (5)

    /* values of WFSIDCSTATUS.fwSecurity */

#define WFS_IDC_SECNOTSUPP  (1)
#define WFS_IDC_SECNOTREADY (2)
#define WFS_IDC_SECOPEN     (3)

    /* values of WFSIDCSTATUS.fwChipPower */

#define WFS_IDC_CHIPONLINE     (0)
#define WFS_IDC_CHIPPOWEREDOFF (1)
#define WFS_IDC_CHIPBUSY       (2)
#define WFS_IDC_CHIPNODEVICE   (3)
#define WFS_IDC_CHIPHWERROR    (4)
#define WFS_IDC_CHIPNOCARD     (5)
#define WFS_IDC_CHIPNOTSUPP    (6)
#define WFS_IDC_CHIPUNKNOWN    (7)

/* Size and max index of dwGuidLights array */ 
#define     WFS_IDC_GUIDLIGHTS_SIZE             (32) 
#define     WFS_IDC_GUIDLIGHTS_MAX              (WFS_IDC_GUIDLIGHTS_SIZE - 1) 
 
/* Indices of WFSIDCSTATUS.dwGuidLights [...] 
              WFSIDCCAPS.dwGuidLights [...] 
*/ 
#define     WFS_IDC_GUIDANCE_CARDUNIT           (0) 
 
/* Values of WFSIDCSTATUS.dwGuidLights [...] 
             WFSIDCCAPS.dwGuidLights [...] 
*/ 
#define     WFS_IDC_GUIDANCE_NOT_AVAILABLE      (0x00000000) 
#define     WFS_IDC_GUIDANCE_OFF                (0x00000001) 
#define     WFS_IDC_GUIDANCE_ON                 (0x00000002) 
#define     WFS_IDC_GUIDANCE_SLOW_FLASH         (0x00000004) 
#define     WFS_IDC_GUIDANCE_MEDIUM_FLASH       (0x00000008) 
#define     WFS_IDC_GUIDANCE_QUICK_FLASH        (0x00000010) 
#define     WFS_IDC_GUIDANCE_CONTINUOUS         (0x00000080) 
#define     WFS_IDC_GUIDANCE_RED                (0x00000100) 
#define     WFS_IDC_GUIDANCE_GREEN              (0x00000200) 
#define     WFS_IDC_GUIDANCE_YELLOW             (0x00000400) 
#define     WFS_IDC_GUIDANCE_BLUE               (0x00000800) 
#define     WFS_IDC_GUIDANCE_CYAN               (0x00001000) 
#define     WFS_IDC_GUIDANCE_MAGENTA            (0x00002000) 
#define     WFS_IDC_GUIDANCE_WHITE              (0x00004000) 
 
/* values of WFSIDCSTATUS.wChipModule */ 
 
#define     WFS_IDC_CHIPMODOK                   (1) 
#define     WFS_IDC_CHIPMODINOP                 (2) 
#define     WFS_IDC_CHIPMODUNKNOWN              (3) 
#define     WFS_IDC_CHIPMODNOTSUPP              (4) 
 
/* values of WFSIDCSTATUS.wMagReadModule and 
             WFSIDCSTATUS.wMagWriteModule  */ 
 
#define     WFS_IDC_MAGMODOK                    (1) 
#define     WFS_IDC_MAGMODINOP                  (2) 
#define     WFS_IDC_MAGMODUNKNOWN               (3) 
#define     WFS_IDC_MAGMODNOTSUPP               (4) 
 
/* values of WFSIDCSTATUS.wFrontImageModule and 
             WFSIDCSTATUS.wBackImageModule */ 
 
#define     WFS_IDC_IMGMODOK                    (1) 
#define     WFS_IDC_IMGMODINOP                  (2) 
#define     WFS_IDC_IMGMODUNKNOWN               (3) 
#define     WFS_IDC_IMGMODNOTSUPP               (4) 
 
/* values of WFSIDCSTATUS.wDevicePosition 
             WFSIDCDEVICEPOSITION.wPosition */ 
 
#define     WFS_IDC_DEVICEINPOSITION            (0) 
#define     WFS_IDC_DEVICENOTINPOSITION         (1) 
#define     WFS_IDC_DEVICEPOSUNKNOWN            (2) 
#define     WFS_IDC_DEVICEPOSNOTSUPP            (3) 
    /* values of WFSIDCCAPS.fwType */

#define WFS_IDC_TYPEMOTOR       (1)
#define WFS_IDC_TYPESWIPE       (2)
#define WFS_IDC_TYPEDIP         (3)
#define WFS_IDC_TYPECONTACTLESS (4)
#define WFS_IDC_TYPELATCHEDDIP  (5)
#define WFS_IDC_TYPEPERMANENT   (6)

    /* values of WFSIDCCAPS.fwReadTracks, WFSIDCCAPS.fwWriteTracks,
                 WFSIDCCARDDATA.wDataSource */

#define WFS_IDC_NOTSUPP 0x0000
#define WFS_IDC_TRACK1  0x0001
#define WFS_IDC_TRACK2  0x0002
#define WFS_IDC_TRACK3  0x0004
#define WFS_IDC_FRONT_TRACK_1       0x0080 

    /* further values of WFSIDCCARDDATA.wDataSource */

#define WFS_IDC_CHIP         0x0008
#define WFS_IDC_SECURITY     0x0010
#define WFS_IDC_FLUXINACTIVE 0x0020
#define WFS_IDC_TRACK_WM     0x8000
#define WFS_IDC_MEMORY_CHIP  0x0040
#define WFS_IDC_FRONTIMAGE   0x0100
#define WFS_IDC_BACKIMAGE    0x0200

    /* values of WFSIDCCAPS.fwChipProtocols */

#define WFS_IDC_CHIPT0  0x0001
#define WFS_IDC_CHIPT1  0x0002
#ifdef _PISA_STD_
#define WFS_IDC_CHIP_PROTOCOL_NOT_REQUIRED  0x0004 
#else
#define WFS_IDC_CHIPT2  0x0004
#define WFS_IDC_CHIPT3  0x0008
#define WFS_IDC_CHIPT4  0x0010
#define WFS_IDC_CHIPT5  0x0020
#define WFS_IDC_CHIPT6  0x0040
#define WFS_IDC_CHIPT7  0x0080
#define WFS_IDC_CHIPT8  0x0100
#define WFS_IDC_CHIPT9  0x0200
#define WFS_IDC_CHIPT10 0x0400
#define WFS_IDC_CHIPT11 0x0800
#define WFS_IDC_CHIPT12 0x1000
#define WFS_IDC_CHIPT13 0x2000
#define WFS_IDC_CHIPT14 0x4000
#define WFS_IDC_CHIPT15 0x8000
#endif

    /* values of WFSIDCCAPS.fwSecType */

#define WFS_IDC_SECNOTSUPP (1)
#define WFS_IDC_SECMMBOX   (2)
#define WFS_IDC_SECCIM86   (3)

/* values of WFSIDCCAPS.wEjectModuleEmptyType*/

#define	    WFS_EM_EMPTYSTORAGE     (1)
#define	    WFS_EM_EMPTYCAPTUREBIN  (2)
#define	    WFS_EM_EMPTYALL         (3)
    /* values of WFSIDCCAPS.fwPowerOnOption, WFSIDCCAPS.fwPowerOffOption,  */

#define WFS_IDC_NOACTION        (1)
#define WFS_IDC_EJECT           (2)
#define WFS_IDC_RETAIN          (3)
#define WFS_IDC_EJECTTHENRETAIN (4)
#define WFS_IDC_READPOSITION    (5)

    /* values of WFSIDCCAPS.fwWriteMode; WFSIDCWRITETRACK.fwWriteMethod, WFSIDCCARDDATA.fwWriteMethod */

#define WFS_IDC_UNKNOWN 0x0001
#define WFS_IDC_LOCO    0x0002
#define WFS_IDC_HICO    0x0004
#define WFS_IDC_AUTO    0x0008

    /* values of WFSIDCCAPS.fwChipPower */

#define WFS_IDC_CHIPPOWERCOLD 0x0002
#define WFS_IDC_CHIPPOWERWARM 0x0004
#define WFS_IDC_CHIPPOWEROFF  0x0008

#define     WFS_IDC_SAM1	                        0x8000
#define     WFS_IDC_SAM2	                        0x4000
#define     WFS_IDC_SAM3	                        0x2000
#define     WFS_IDC_SAM4	                        0x1000
#define     WFS_IDC_SAM5	                        0x0800
#define     WFS_IDC_SAM6	                        0x0400
#define     WFS_IDC_SAM7	                        0x0200
#define     WFS_IDC_SAM8	                        0x0100

/* values of WFSIDCCAPS.wDIPMode */ 

#define     WFS_IDC_DIP_UNKNOWN                 0x0001 
#define     WFS_IDC_DIP_EXIT                    0x0002 
#define     WFS_IDC_DIP_ENTRY                   0x0004 
#define     WFS_IDC_DIP_ENTRY_EXIT              0x0008 

/* values of WFSIDCCAPS. lpwMemoryChipProtocols */ 

#define     WFS_IDC_MEM_SIEMENS4442             0x0001 
#define     WFS_IDC_MEM_GPM896                  0x0002
#define     WFS_IDC_MEM_1608                    0x0004
#define     WFS_IDC_MEM_102                     0x0008 
    /* values of WFSIDCFORM.fwAction */

#define WFS_IDC_ACTIONREAD  0x0001
#define WFS_IDC_ACTIONWRITE 0x0002

    /* values of WFSIDCTRACKEVENT.fwStatus, WFSIDCCARDDATA.wStatus */

#define WFS_IDC_DATAOK         (0)
#define WFS_IDC_DATAMISSING    (1)
#define WFS_IDC_DATAINVALID    (2)
#define WFS_IDC_DATATOOLONG    (3)
#define WFS_IDC_DATATOOSHORT   (4)
#define WFS_IDC_DATASRCNOTSUPP (5)
#define WFS_IDC_DATASRCMISSING (6)

    /* values WFSIDCCARDACT.wAction */

#define WFS_IDC_CARDRETAINED     (1)
#define WFS_IDC_CARDEJECTED      (2)
#define WFS_IDC_CARDREADPOSITION (3)
#define WFS_IDC_CARDJAMMED       (4)

    /* values of WFSIDCCARDDATA.lpbData if security is read */

#define WFS_IDC_SEC_READLEVEL1   '1'
#define WFS_IDC_SEC_READLEVEL2   '2'
#define WFS_IDC_SEC_READLEVEL3   '3'
#define WFS_IDC_SEC_READLEVEL4   '4'
#define WFS_IDC_SEC_READLEVEL5   '5'
#define WFS_IDC_SEC_BADREADLEVEL '6'
#define WFS_IDC_SEC_NODATA       '7'
#define WFS_IDC_SEC_DATAINVAL    '8'
#define WFS_IDC_SEC_HWERROR      '9'
#define WFS_IDC_SEC_NOINIT       'A'

/* values of WFSIDCIFMIDENTIFIER.wIFMAuthority */ 

#define     WFS_IDC_IFMEMV                      (1) 
#define     WFS_IDC_IFMEUROPAY                  (2) 
#define     WFS_IDC_IFMVISA                     (3) 
#define     WFS_IDC_IFMGIECB                    (4) 

/* values WFSIDCCAPS.wEjectPosition, WFSIDCEJECTCARD.wEjectPosition */ 

#define     WFS_IDC_EXITPOSITION                (0x0001) 
#define     WFS_IDC_TRANSPORTPOSITION           (0x0002) 

/* values if LPWFSIDCCLEARRETRACTCARD.wNum */
#define WFS_IDC_CLEARALL                        (0)
#define WFS_IDC_CLEARONE                        (1) 

/* value of WFSIDCMCRCAPS.wType */
#define	WFS_IDC_TYPE							(10)

/* values of WFSIDCMCRSTATUS.wSlots */
#define	WFS_IDC_SLOTESMPTY						(0)
#define	WFS_IDC_SLOTSOK							(1)
#define	WFS_IDC_SLOTSFULL						(2)

/*values of WFSIDCSLOTUNITINFO.wStatus */
#define	WFS_IDC_SLOTUNIT_NOTPRESENT				(0)
#define	WFS_IDC_SLOTUNIT_PRESENT				(1)

/* values of WFSIDCEJECTSLOTOUT.wType */
#define WFS_IDC_TYPE_SLOTID						(0)
#define WFS_IDC_TYPE_CARDDATA	                (1)
    /* WOSA/XFS IDC Errors */

#define WFS_ERR_IDC_MEDIAJAM         (-(IDC_SERVICE_OFFSET + 0))
#define WFS_ERR_IDC_NOMEDIA          (-(IDC_SERVICE_OFFSET + 1))
#define WFS_ERR_IDC_MEDIARETAINED    (-(IDC_SERVICE_OFFSET + 2))
#define WFS_ERR_IDC_RETAINBINFULL    (-(IDC_SERVICE_OFFSET + 3))
#define WFS_ERR_IDC_INVALIDDATA      (-(IDC_SERVICE_OFFSET + 4))
#define WFS_ERR_IDC_INVALIDMEDIA     (-(IDC_SERVICE_OFFSET + 5))
#define WFS_ERR_IDC_FORMNOTFOUND     (-(IDC_SERVICE_OFFSET + 6))
#define WFS_ERR_IDC_FORMINVALID      (-(IDC_SERVICE_OFFSET + 7))
#define WFS_ERR_IDC_DATASYNTAX       (-(IDC_SERVICE_OFFSET + 8))
#define WFS_ERR_IDC_SHUTTERFAIL      (-(IDC_SERVICE_OFFSET + 9))
#define WFS_ERR_IDC_SECURITYFAIL     (-(IDC_SERVICE_OFFSET + 10))
#define WFS_ERR_IDC_PROTOCOLNOTSUPP  (-(IDC_SERVICE_OFFSET + 11))
#define WFS_ERR_IDC_ATRNOTOBTAINED   (-(IDC_SERVICE_OFFSET + 12))
#define WFS_ERR_IDC_INVALIDKEY       (-(IDC_SERVICE_OFFSET + 13))
#define WFS_ERR_IDC_WRITE_METHOD     (-(IDC_SERVICE_OFFSET + 14))
#define WFS_ERR_IDC_CHIPPOWERNOTSUPP (-(IDC_SERVICE_OFFSET + 15))
#define WFS_ERR_IDC_CARDTOOSHORT     (-(IDC_SERVICE_OFFSET + 16))
#define WFS_ERR_IDC_CARDTOOLONG      (-(IDC_SERVICE_OFFSET + 17))
#define WFS_ERR_IDC_INVALID_PORT                (-(IDC_SERVICE_OFFSET + 18)) 
#define WFS_ERR_IDC_POWERSAVETOOSHORT           (-(IDC_SERVICE_OFFSET + 19)) 
#define WFS_ERR_IDC_POWERSAVEMEDIAPRESENT       (-(IDC_SERVICE_OFFSET + 20)) 
#define WFS_ERR_IDC_CARDPRESENT                 (-(IDC_SERVICE_OFFSET + 21))
#define WFS_ERR_IDC_POSITIONINVALID             (-(IDC_SERVICE_OFFSET + 22)) 
#define WFS_ERR_IDC_NORETRACTBIN	            (-(IDC_SERVICE_OFFSET + 21))
#define WFS_ERR_IDC_RETRACTBINFULL              (-(IDC_SERVICE_OFFSET + 22))
#define WFS_ERR_IDC_CARD_NOTFOUND               (-(IDC_SERVICE_OFFSET + 23))
#define WFS_ERR_IDC_MEDIAEXIST					(-(IDC_SERVICE_OFFSET + 52))
#define WFS_ERR_IDC_MEDIARETRACTJAM				(-(IDC_SERVICE_OFFSET + 53))
#define WFS_ERR_IDC_SLOTFULL					(-(IDC_SERVICE_OFFSET + 54))
#define WFS_ERR_IDC_SLOTNOTEXIST				(-(IDC_SERVICE_OFFSET + 55))
#define WFS_ERR_IDC_SHUTTERBLOCKED				(-(IDC_SERVICE_OFFSET + 56)) 
#define WFS_ERR_IDC_SHUTTERCLOSEFAILED			(-(IDC_SERVICE_OFFSET + 57)) 
#define WFS_ERR_IDC_SHUTTEROPENFAILED			(-(IDC_SERVICE_OFFSET + 58)) 
#define WFS_ERR_IDC_CODENOTMATCH				(-(IDC_SERVICE_OFFSET + 59))
#define WFS_ERR_IDC_CAPTUREIMPURITY             (-(IDC_SERVICE_OFFSET + 60))
//退卡模块
#ifdef CARD_REJECT_GD_MODE
    #define WFS_ERR_IDC_CMNOMEDIA    (-(IDC_SERVICE_OFFSET + 55))       // 暂存仓无卡
    #define WFS_ERR_CM_MEDIAJAMMED   (-(IDC_SERVICE_OFFSET + 56))
    #define WFS_ERR_CM_ILLHELTH      (-(IDC_SERVICE_OFFSET + 57))       // CRM不正常
    #define WFS_ERR_CM_MEDIAFULL     (-(IDC_SERVICE_OFFSET + 58))       //
    #define WFS_ERR_CM_MEDIAINDOOR   (-(IDC_SERVICE_OFFSET + 59))
    #define WFS_ERR_CM_MEDIANOTEXIST (-(IDC_SERVICE_OFFSET + 61))       // 暂存仓有卡
    #define WFS_ERR_CM_CMFULL        (-(IDC_SERVICE_OFFSET + 62))       // 暂存仓满
#endif

    /*=================================================================*/
    /* IDC Info Command Structures and variables */
    /*=================================================================*/

    typedef struct _wfs_idc_status
    {
        WORD fwDevice;
        WORD fwMedia;
        WORD fwRetainBin;
        WORD fwSecurity;
        USHORT usCards;
        WORD fwChipPower;
        LPSTR lpszExtra;
        DWORD                    dwGuidLights[WFS_IDC_GUIDLIGHTS_SIZE];
        WORD                     wChipModule;
        WORD                     wMagReadModule;
        WORD                     wMagWriteModule;
        WORD                     wFrontImageModule;
        WORD                     wBackImageModule;
        WORD                     wDevicePosition;
        USHORT                   usPowerSaveRecoveryTime;
    } WFSIDCSTATUS, *LPWFSIDCSTATUS;

    typedef struct _wfs_idc_caps
    {
        WORD wClass;
        WORD fwType;
        BOOL bCompound;
        WORD fwReadTracks;
        WORD fwWriteTracks;
        WORD fwChipProtocols;
        USHORT usCards;
        WORD fwSecType;
        WORD fwPowerOnOption;
        WORD fwPowerOffOption;
        BOOL bFluxSensorProgrammable;
        BOOL bReadWriteAccessFollowingEject;
        WORD fwWriteMode;
        WORD fwChipPower;
        LPSTR lpszExtra;
        WORD                     wDIPMode;
        LPWORD                   lpwMemoryChipProtocols;
        DWORD                    dwGuidLights[WFS_IDC_GUIDLIGHTS_SIZE];
        WORD                     wEjectPosition;
        BOOL                     bPowerSaveControl;
    } WFSIDCCAPS, *LPWFSIDCCAPS;

    typedef struct _wfs_idc_form
    {
        LPSTR lpszFormName;
        CHAR cFieldSeparatorTrack1;
        CHAR cFieldSeparatorTrack2;
        CHAR cFieldSeparatorTrack3;
        WORD fwAction;
        LPSTR lpszTracks;
        BOOL bSecure;
        LPSTR lpszTrack1Fields;
        LPSTR lpszTrack2Fields;
        LPSTR lpszTrack3Fields;
    } WFSIDCFORM, *LPWFSIDCFORM;

typedef struct _wfs_idc_ifm_identifier
{
	WORD                wIFMAuthority;
	LPSTR               lpszIFMIdentifier;
} WFSLFSIFMIDENTIFIER, *LPWFSIDCIFMIDENTIFIER;
    /*=================================================================*/
    /* IDC Execute Command Structures */
    /*=================================================================*/

    typedef struct _wfs_idc_write_track
    {
        LPSTR lpstrFormName;
        LPSTR lpstrTrackData;
        WORD fwWriteMethod;
    } WFSIDCWRITETRACK, *LPWFSIDCWRITETRACK;

    typedef struct _wfs_idc_retain_card
    {
        USHORT usCount;
        WORD fwPosition;
    } WFSIDCRETAINCARD, *LPWFSIDCRETAINCARD;

    typedef struct _wfs_idc_setkey
    {
        USHORT usKeyLen;
        LPBYTE lpbKeyValue;
    } WFSIDCSETKEY, *LPWFSIDCSETKEY;

    typedef struct _wfs_idc_card_data
    {
        WORD wDataSource;
        WORD wStatus;
        ULONG ulDataLength;
        LPBYTE lpbData;
        WORD fwWriteMethod;
    } WFSIDCCARDDATA, *LPWFSIDCCARDDATA;

    typedef struct _wfs_idc_chip_io
    {
        WORD wChipProtocol;
        ULONG ulChipDataLength;
        LPBYTE lpbChipData;
    } WFSIDCCHIPIO, *LPWFSIDCCHIPIO;

    typedef struct _wfs_idc_chip_power_out
    {
        ULONG ulChipDataLength;
        LPBYTE lpbChipData;
    } WFSIDCCHIPPOWEROUT, *LPWFSIDCCHIPPOWEROUT;

    typedef struct _wfs_idc_parse_data
    {
        LPSTR lpstrFormName;
        LPWFSIDCCARDDATA *lppCardData;
    } WFSIDCPARSEDATA, *LPWFSIDCPARSEDATA;
typedef struct _wfs_idc_set_guidlight 
{ 
	WORD                     wGuidLight; 
	DWORD                    dwCommand; 
} WFSIDCSETGUIDLIGHT, *LPWFSIDCSETGUIDLIGHT; 

typedef struct _wfs_idc_eject_card 
{ 
    WORD                     wEjectPosition;  
} WFSIDCEJECTCARD, *LPWFSIDCEJECTCARD; 

typedef struct _wfs_idc_power_save_control 
{ 
	USHORT                   usMaxPowerSaveRecoveryTime; 
} WFSIDCPOWERSAVECONTROL, *LPWFSIDCPOWERSAVECONTROL; 

    /*=================================================================*/
    /* IDC Message Structures */
    /*=================================================================*/

    typedef struct _wfs_idc_track_event
    {
        WORD fwStatus;
        LPSTR lpstrTrack;
        LPSTR lpstrData;
    } WFSIDCTRACKEVENT, *LPWFSIDCTRACKEVENT;

    typedef struct _wfs_idc_card_act
    {
        WORD wAction;
        WORD wPosition;
    } WFSIDCCARDACT, *LPWFSIDCCARDACT;

typedef struct _wfs_idc_device_position 
{ 
    WORD                     wPosition; 
} WFSIDCDEVICEPOSITION, *LPWFSIDCDEVICEPOSITION; 

typedef struct _wfs_idc_power_save_change 
{ 
    USHORT                   usPowerSaveRecoveryTime; 
} WFSIDCPOWERSAVECHANGE, *LPWFSIDCPOWERSAVECHANGE; 

typedef struct _wfs_idc_retract_card
{
	LPSTR                 lpszCardNo;
	LPSTR                 lpszRetractTime;
} WFSIDCRETRACTCARD, * LPWFSIDCRETRACTCARD;

typedef struct _wfs_idc_handback_card
{
	LPSTR                 lpszCardNo;
} WFSIDCHANDBACKCARD, * LPWFSIDCHANDBACKCARD;

typedef struct _wfs_idc_clear_retract_card
{
    WORD                  wNum;
} WFSIDCCLEARRETRACTCARD, * LPWFSIDCCLEARRETRACTCARD;
    
typedef struct _wfs_idc_MCR_status
{
	WORD             wDevice;
	WORD             wSlots;
	WORD             wRetainBin;
	USHORT           usUseSlots;
	WORD			 wShutter;
	LPSTR            lpstrExtra;
} WFSIDCMCRSTATUS, * LPWFSIDCMCRSTATUS;

typedef struct _wfs_idc_MCR_caps
{
	WORD             wClass;
	WORD             wType;
	USHORT           usBigSlots;
	USHORT           usNormalSlots;
	LPSTR            lpstrExtra;
} WFSIDCMCRCAPS, * LPWFSIDCMCRCAPS;

typedef struct _wfs_idc_slotsunitinfo
{
	USHORT		    usNumber;
	WORD		    wStatus;
	USHORT	     	usIsRetained;
	LPSTR		    lpszCard_Info;
	LPSTR		    lpszReatin_Card_Info;
	LPSTR		    lpszRestore_Time;
	LPSTR		    lpszRetain_Time;
	LPSTR		    lpszPosition;
	USHORT		    usSlotSize;
} WFSIDCSLOTUNITINFO, *LPWFSIDCSLOTUNITINFO;

typedef struct _wfs_idc_slots_info
{
	USHORT                 usCount;
        LPWFSIDCSLOTUNITINFO  *lppList;
} WFSIDCSLOTSINFO, *LPWFSIDCSLOTSINFO;

typedef struct _wfs_idc_retainslot_strart_in
{
	LPSTR             lpszData;
} WFSIDCRETAINSLOTSTARTIN, *LPWFSIDCRETAINSLOTSTARTIN;

typedef struct _wfs_idc_retainslot_strart_out
{
	LPSTR            lpszData;
} WFSIDCRETAINSLOTSTARTOUT, *LPWFSIDCRETAINSLOTSTARTOUT;


typedef struct _wfs_idc_retainslot
{
	LPSTR             lpszCard_Info;
	LPSTR             lpszRetain_Time;
} WFSIDCRETAINSLOT, *LPWFSIDCRETAINSLOT;

typedef struct _wfs_idc_ejectslot
{
	WORD	          wType;
	LPSTR         	  lpCmd_Data;
} WFSIDCEJECTSLOT, *LPWFSIDCEJECTSLOT;

typedef struct _wfs_idc_ejectslotout
{ 
	WORD	          wType;
	LPSTR    	      lpCmd_Data;
} WFSIDCEJECTSLOTOUT, *LPWFSIDCEJECTSLOTOUT; 

typedef struct _wfs_idc_retainslot_out
{
	LPSTR            lpszCardInfo;
	LPSTR            lpszRetainTime;
	LPSTR            lpszPosition;
	WORD             wStatus;
} WFSIDCRETAINSLOTOUT, *LPWFSIDCRETAINSLOTOUT;
/*   restore alignment   */
#pragma pack(pop)

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __INC_XFSIDC__H */
