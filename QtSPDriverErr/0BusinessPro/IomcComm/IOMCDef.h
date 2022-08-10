#pragma once
#include "QtTypeInclude.h"

#define DEF_WRITE_TIMEOUT                         (3*1000)
#define DEF_SINGLE_READ_TIMEOUT                   (100)
#define DEF_READ_BUFF_SIZE                        (1024)
#define DEF_WRITE_BUFF_SIZE                       (1024)
#define DEF_TIMEOUT                               (3*1000)
#define RES_COMMON_LEN                            90         // response common part length
#define RES_DATA_OFFSET                           (2 + 6 + 90)
//命令ID定义


#pragma pack(push, 1)
typedef struct _FIRSTPART
{
    WORD wMLen;
    BYTE bySA;
    BYTE byUA;
    BYTE bySeq;
    BYTE byFlg;
    BYTE byReserved[2];
}FIRSTPART, *LPFIRSTPART;

typedef struct _CMDRESPKG
{
    WORD wID;
    WORD wLen;
    WORD wCode;
}CMDRESPKG, *LPCMDRESPKG;

typedef struct _RESPCOMMPART
{
    WORD wID;
    WORD wLen;
    BYTE byErrorCode[2];

    WORD wIDMC;
    WORD wLenMC;
    BYTE byMaintenanceCode[4];

    WORD wIDMS1;
    WORD wLenMS1;
    BYTE byMechaStatus1[4];

    WORD wIDMS2;
    WORD wLenMS2;
    BYTE byMechaStatus2[4];

    WORD wIDMS3;
    WORD wLenMS3;
    BYTE byMechaStatus3[8];

    WORD wIDCEC;
    WORD wLenCEC;
    BYTE byCommErrorCode[16];

    WORD wIDCP;
    WORD wLenCP;
    BYTE byCheckPosition[4];
    BYTE byResidualInfo[4];
    BYTE byTiltOpenInfo[8];
    BYTE byTiltCloseInfo[8];
    _RESPCOMMPART() {}
}RESPCOMMPART, *LPRESPCOMMPART;

typedef struct _IOMCCMDCOMMON
{
    FIRSTPART stFirstPart;
    WORD      wLen;
    CMDRESPKG stCmdPkg;
}IOMCCMDCOMMON, *LPIOMCCMDCOMMON;

typedef struct _IOMCRESPVALIDCOMMON
{
    WORD            wLen;
    CMDRESPKG       stRespPkg;
    RESPCOMMPART    stRespCommPart;
}IOMCRESPVALIDCOMMON, *LPIOMCRESPVALIDCOMMON;

typedef struct _IOMCRESPCOMMON
{
    FIRSTPART       stFirstPart;
    IOMCRESPVALIDCOMMON  stRespValidComm;           //应答中有效数据通用部分
}IOMCRESPCOMMON, *LPIOMCRESPCOMMON;


//----------------------------- 命令数据结构体 --------------------------------

//----------------------------- 应答数据结构体 --------------------------------
typedef struct _PARTVERINFO
{
    WORD wId;
    WORD wLen;
    BYTE byType[16];
    BYTE byVer[8];
    BYTE byData[6];
    BYTE bySum[4];
    _PARTVERINFO() {}
}PARTVERINFO, *LPPARTVERINFO;

typedef struct _VERSIONINFO
{
    PARTVERINFO stPartVerInfo[3];        //控制部，提升部，FPGA部
    _VERSIONINFO() {}
}VERSIONINFO, *LPVERSIONINFO;

typedef struct _WARNINGINFO
{
    BYTE byEC[2];
    BYTE byMTC[4];
    BYTE byCheckPosition[4];
    BYTE byResidualInfo[4];
    BYTE byTiltOpenInfo[8];
    BYTE byTiltCloseInfo[8];
    _WARNINGINFO() {}
}WARNINGINFO, *LPWARNINGINFO;

typedef struct _SENSORSTATUS
{
    WORD wId;
    WORD wLen;
    WARNINGINFO stWarning1;
    WARNINGINFO stWarning2;
    WARNINGINFO stWarning3;
    WARNINGINFO stWarning4;

    WORD wIdSI;
    WORD wLenSI;
    BYTE bySensorInfo[4];
    BYTE byMCSensorInfo[2];
    BYTE byAddedSensorInfo[8];
    _SENSORSTATUS() {}
}SENSORSTATUS, *LPSENSORSTATUS;

typedef struct _DATAREAD
{
    WORD wId;
    WORD wLen;
    BYTE byConfigData[2];
    BYTE byBoardVersion[2];

    WORD wIdDSS;
    WORD wLenDSS;
    BYTE byDipSwitchStatus;
    BYTE byReserved;

    WORD wIdDT;
    WORD wLenDT;
    BYTE byDateTime[8];         //年上位/年下位/月/日/星期/时/分/秒

    WORD wIdFOOT;
    WORD wLenFOOT;
    BYTE byFlickerOnOffTime[16];
    WORD wIdLOOT;
    WORD wLenLOOT;
    BYTE byLMCOnOffTime[4];
    WORD wIdSLOOT;
    WORD wLenSLOOT;
    BYTE bySPLLedOnOffTime[4];
    WORD wIdOOT;
    WORD wLenOOT;
    BYTE byOnOffTime[4];

    WORD wIdSSRI;
    WORD wLenSSRI;
    BYTE bySeismicSensorReadInterval[2];
    WORD wIdFSDT;
    WORD wLenFSDT;
    BYTE byFlickerStartDelayTime[2];
    WORD wIdES;
    WORD wLenES;
    BYTE byEarphoneSettings[2];
    WORD wIdAPT;
    WORD wLenAPT;
    BYTE byAlarmPoweronTime[8];

    WORD wIdAFE;
    WORD wLenAFE;
    BYTE byAlarmFlagEnable[2];
    WORD wIdAPCE;
    WORD wLenAPCE;
    BYTE byAlarmPowerConnEnable[2];
    WORD wIdRPCE;
    WORD wLenRPCE;
    BYTE byRemotePowerConnEnable[2];
    WORD wIdFC;
    WORD wLenFC;
    BYTE byFanControl[2];

    WORD wIdPO;
    WORD wLenPO;
    BYTE byPowerOff[4];
    WORD wIdVS;
    WORD wLenVS;
    BYTE byVolumeSettings[4];
    WORD wIdDSLI;
    WORD wLenDSLI;
    BYTE byDeviceSPLLANInfo[4];
    WORD wIdPAF;
    WORD wLenPAF;
    BYTE byPowerAbnormalFlag[2];

    WORD wIdRAF;
    WORD wLenRAF;
    BYTE byRTCAbnormalFlag[2];
    WORD wIdSLS;
    WORD wLenSLS;
    BYTE bySPLLedStatus[4];
    WORD wIdUPS;
    WORD wLenUPS;
    BYTE byUnitPowerStatus[6];
    WORD wIdBPI;
    WORD wLenBPI;
    BYTE byBasilarPlateInfo[8];
    _DATAREAD() {}
}DATAREAD, *LPDATAREAD;

typedef struct _LOGDATA
{
    WORD wId;
    WORD wLen;
    BYTE byLogData[1];
    _LOGDATA() {}
}LOGDATA, *LPLOGDATA;

typedef struct _ENVSENSORDATA
{
    WORD wId;
    WORD wLen;
    BYTE byEnvSensorData[2];
    _ENVSENSORDATA() {}
}ENVSENSORDATA, *LPENVSENSORDATA;

typedef struct _FLASHROMREAD
{
    WORD wId;
    WORD wLen;
    BYTE bySectorNo;
    BYTE byOffsetNo;
    BYTE byReadData[4096];
    _FLASHROMREAD() {}
}FLASHROMREAD, *LPFLASHROMREAD;

typedef struct _FHSPLSTATUS
{
    WORD wId;
    WORD wLen;
    BYTE byFHSPLStatus;
    BYTE byReserved;
    _FHSPLSTATUS() {}
}FHSPLSTATUS, *LPFHSPLSTATUS;

typedef struct _TIMERINFO
{
    WORD wId;
    WORD wLen;
    BYTE byTimerEnable;
    BYTE byTimerData[6];
    BYTE byReserved;
    _TIMERINFO() {}
}TIMERINFO, *LPTIMERINFO;

typedef struct _POWERINFO
{
    WORD wId;
    WORD wLen;
    BYTE byRemotePowerSettings;
    BYTE byReserved;

    WORD wIdAPS;
    WORD wLenAPS;
    BYTE byAlarmPowerSettings;
    BYTE byReservedAPS;

    WORD wIdAPCST;
    WORD wLenAPCST;
    BYTE byAlarmPowerConnSetTime[8];

    WORD wIdAF;
    WORD wLenAF;
    BYTE byAlarmFlag;
    BYTE byReservedAF;

    WORD wIdPCCF;
    WORD wLenPCCF;
    BYTE byPowerConnCauseFlag;
    BYTE byReservedPCCF;
    _POWERINFO() {}
}POWERINFO, *LPPOWERINFO;

typedef struct _TIMEOUTFLAGINFO
{
    WORD wId;
    WORD wLen;
    BYTE byTimeoutFlag;
    BYTE byReserved;
    _TIMEOUTFLAGINFO() {}
}TIMEOUTFLAGINFO, *LPTIMEOUTFLAGINFO;

typedef struct _I2CABNORMINFO
{
    WORD wId;
    WORD wLen;
    BYTE byErrorRegister[4];
    _I2CABNORMINFO() {}
}I2CABNORMINFO, *LPI2CABNORMINFO;

//////////////////////////////////////////////////////////////////////////
struct VERtag
{
    uchar    NAME[8];
    uchar    byV[2];
    uchar    byR[2];
    uchar    byT[2];
    uchar    byN[2];
};


struct IOMC_VERtag
{
    ushort    wType ;
    VERtag    VerDt[2] ;
};

struct DRV_VERtag
{
    ushort    wType ;
    VERtag    VerDt[4] ;
};

struct SVERSION
{
    ushort    wID;
    ushort    wLNG;
    char      cKatashiki[16] ;
    char      cVER[8] ;
    uchar     byDate[6];
    uint      dwSum;
};

struct IOMCVrtInfT                          // IOMC
{
    ushort        wLEN ;                   // LEN      (2Byte)
    ushort        wRESP_ID ;               // RESP ID  (2Byte)
    ushort        wRESP_LNG ;              //      LNG (2Byte)
    ushort        wRESP_RES ;              //      RES (2Byte)
    uchar         byDATA[RES_COMMON_LEN] ; // commom part STD40-00-00-00
    SVERSION      VRT[2] ;                 //
} ;

struct STR_IOMC
{
    WORD        wLEN;                      // LEN      (2Byte)
    WORD        wCNTL_ID;                  // CNTL ID  (2Byte)
    WORD        wCNTL_LNG;                 //      LNG (2Byte)
    WORD        wCNTL_CMD;                 //      CMD (2Byte)
    WORD        wDATA_ID;                  // DATA ID  (2Byte)
    WORD        wDATA_LNG;                 //      LNG (2Byte)
    BYTE        byDATA[64];                //
};

struct IOMCASYNCRespInfT
{
    BYTE        byOutDataSz[4];
    WORD        wLEN;                    // LEN      (2Byte)
    WORD        wRESP_ID;                // RESP ID  (2Byte)
    WORD        wRESP_LNG;               //      LNG (2Byte)
    WORD        wRESP_RES;               //      RES (2Byte)
    BYTE        byDATA[RES_COMMON_LEN];  // common part STD40-00-00-00
    BYTE        byRSPD[200];             // RSPD  STD40-00-00-00
};

typedef struct strIOMCRespInfT
{
    WORD        wLEN;                      // LEN      (2Byte)
    WORD        wRESP_ID;                  // RESP ID  (2Byte)
    WORD        wRESP_LNG;                 //      LNG (2Byte)
    WORD        wRESP_RES;                 //      RES (2Byte)
    BYTE        byDATA[RES_COMMON_LEN];    //
    BYTE        byRSPD[256];
} IOMCRespInfT, *LPIOMCRespInfT;

typedef struct _IOMC_Flicker_data
{
    BYTE Flk[15];   // Flicker 1~ 15
    BYTE Reserve;   // Reserve area
} STIOMCFLICKERDATA, *LPSTIOMCFLICKERDATA;


typedef struct _IOMCCOMMONOUTPUT{
    WORD wLen;
    WORD wCNTLId;
    WORD WCNTLLen;
    WORD wCNTLCmd;
    WORD wOutput1Id;
    WORD wOutput1Len;
    BYTE byOutput1Data[32];
    WORD wOutput2Id;
    WORD wOutput2Len;
    BYTE byOutput2Data[8];
}IOMCCOMMONOUTPUT, *LPIOMCCOMMONOUTPUT;

struct STR_IOMC_SENSE_INF_OUT               // IOMC Info sense Structure
{
    WORD        wLEN;                       // LEN      (2Byte)
    WORD        wRESP_ID;                   // RESP ID  (2Byte)
    WORD        wRESP_LNG;                  //      LNG (2Byte)
    WORD        wRESP_RES;                  //      RES (2Byte)
    BYTE        byDATA[RES_COMMON_LEN];     // Common part
    // Warning infomation
    WORD        wID_WARNING;
    WORD        wLNG_WARNING;
    BYTE        byWARNING1[30];         // MechaStatus(4Byte)
    BYTE        byWARNING2[30];         // MechaStatus(4Byte)
    BYTE        byWARNING3[30];         // MechaStatus(4Byte)
    BYTE        byWARNING4[30];         // MechaStatus(4Byte)
    // Sekkayku sense
    WORD        wID_SEKKYAKU;
    WORD        wLNG_SEKKYAKU;
    BYTE        bySEKKYAKU_SENSE_INF[4];    // Basic sense status   (4byte)
    // MC hikidashi sense
    WORD        wID_MC_HIKIDASHI;
    WORD        wLNG_MC_HIKIDASHI;
    BYTE        byMC_HIKIDASHI_SENSE_INF[2];// Reserve   (2byte)
    // Fuka sense
    WORD        wID_FUKA_SENSE;
    WORD        wLNG_FUKA_SENSE;
    BYTE        byFUKA_SENSE_INF[8];       // Sub sense status(8byte)

};  // Sensor Info


typedef struct strIOMCBoardVersionRespInfT      // BoardVersion IOMC response structure
{
    WORD        wLEN;                           // LEN      (2Byte)
    WORD        wRESP_ID;                       // RESP ID  (2Byte)
    WORD        wRESP_LNG;                      //      LNG (2Byte)
    WORD        wRESP_RES;                      //      RES (2Byte)
    BYTE        byCOMMON[RES_COMMON_LEN];       // Common part
    WORD        wRSPD_ID;                       // RSPD ID  (2Byte)
    WORD        wRSPD_LNG;                      //      LNG (2Byte)
    WORD        wCONFIGURATION;                 // Configuration  data(2Byte)
    BYTE        byBOARDVERSION;                 // BoardVersion data(1Byte)
    BYTE        bySYSTEMVERSION;                // SystemVersion data(1Byte)
    WORD        wDIPRSPD_ID;                    // RSPD ID  (2Byte)
    WORD        wDIPRSPD_LNG;                   //      LNG (2Byte)
    WORD        DIPswitch;                      // DIPswitch  data(2Byte)
    BYTE        byReserve;
    WORD        wDATERSPD_ID;                   // RSPD ID  (2Byte)
    WORD        wDATERSPD_LNG;                  //      LNG (2Byte)
    BYTE        byDAte[8];                      // year month day (8Byte)
} IOMCBoardVerRespInfT, *LPIOMCBoardVerRespInfT;

typedef struct _iomc_response_data {	// IOMCレスポンスデータ用構造体（出力）
    WORD	wID;			// 応答ID
    WORD	wLng;			// 自分自身を含む実行結果データの長さ
    BYTE	byDATA[5120];		// 実行結果データ
} IOMCRESPDATA, *LPIOMCRESPDATA;

typedef struct _iomc_response_info {	// IOMCレスポンス用構造体（出力）
    WORD	wLen;			// 電文本体の長さ
    WORD	wRespID;		// 応答ID
    WORD	wRespLng;		// 応答LNG
    WORD	wRespRes;		// 応答結果
    BYTE	byCommDATA[90];		// 共通部
    IOMCRESPDATA  iomcRespData[10];	// 応答データ
} IOMCRESPINFO, *LPIOMCRESPINFO;

typedef struct _iomc_sense_info {	// IOMCセンサ状態情報
    WORD	wLen;			// 電文本体の長さ
    WORD	wRespID;		// 応答ID
    WORD	wRespLng;		// 応答LNG
    WORD	wRespRes;		// 応答結果
    BYTE	byCommDATA[90];		// 共通部(90バイト)
    BYTE	byWarningInfo[124];	// ワーニング情報(124バイト)
    BYTE	bySekkyakuInfo[8];	// 接客センス情報(8バイト)
    BYTE	byMCPullOutInfo[6];	// MC引出しセンス情報(6バイト)
    BYTE	byAppendInfo[12];	// 付加センス情報(12バイト)
} IOMCSNSINFO, *LPIOMCSNSINFO;

//40-00-00-00(FT#0001) add start
//灯闪烁-循环模式ON/OFF时间参数设置结构体
typedef struct _IOMCWRITECYCLE {               // IOMC parameters for set cycle of flicker & lamp
    ushort      wLEN;
    ushort      wCNTL_ID;
    ushort      wCNTL_LNG;
    ushort      wCNTL_CMD;

    ushort      wDATA_ID_FLK;                  // DATA ID  (2uchar)
    ushort      wDATA_LNG_FLK;                 // LNG (2uchar)
    uchar       FLK_Cycle1[2];
    uchar	FLK_Cycle2[2];
    uchar	FLK_Cycle3[2];
    uchar	FLK_Cycle4[2];
    uchar	FLK_Cycle5[2];
    uchar	FLK_Cycle6[2];
    uchar	FLK_Cycle7[2];
    uchar	FLK_Cycle8[2];
}IOMCWRITECYCLE, *LPIOMCWRITECYCLE;
//40-00-00-00(FT#0001) add end
//////////////////////////////////////////////////////////////////////////
#pragma pack(pop)
