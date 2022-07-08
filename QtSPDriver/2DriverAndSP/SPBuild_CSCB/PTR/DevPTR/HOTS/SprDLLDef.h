//--------------------------------------------------------------------
// [社外秘]
// COPYRIGHT (c) HITACHI-OMRON TERMINAL SOLUTIONS,CORP.
// 2004,2016.All rights reserved.
//--------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
// 名称     = SPRDLL (SPR Driver Dll)
// ????名   = SPRDLLDEF.H
// 日付     = 2004/07/17(CREATE)
// HISTORY  = SPRUSB,15-00-00-00,2004/07/17 : new make
// HISTORY  = SPRUSB,15-01-00-00,2005/08/05 : add for JPR function
// HISTORY  = SPRUSB,52-00-00-00,2016/01/05 : Linux
// HISTORY  = SPRUSB,53-32-00-00,2016/01/05 : Linux Product version
////////////////////////////////////////////////////////////////////////////////
#ifndef __sprdlldef_h__
#define __sprdlldef_h__

#ifndef _STRUCTDEF_SPR
#define _STRUCTDEF_SPR

//------------------------------------------------------------------------------
// Driver Function Struct
//------------------------------------------------------------------------------
typedef struct _STR_DRV
{
    unsigned int    uiDrvHnd;                                   // 52-00-00-00 CHG
    unsigned short  usParam;                                    // 52-00-00-00 CHG
    unsigned short  usExParam;                                  // 52-00-00-00 CHG
    void           *pvDataInBuffPtr;                            // 52-00-00-00 CHG
    unsigned int    uiDataInBuffSz;                             // 52-00-00-00 CHG
    void           *pvDataOutBuffPtr;                           // 52-00-00-00 CHG
    unsigned int    uiDataOutReqSz;                             // 52-00-00-00 CHG
    unsigned int    uiDataOutBuffSz;                            // 52-00-00-00 CHG
    unsigned int    uiTimer;                                    // 52-00-00-00 CHG
    void           *pvReserve;                                  // 52-00-00-00 ADD
    unsigned int    uiWndMsg ;                                  // 53-32-00-00 ADD
    void           *pvCallBackPtr;                              // 53-32-00-00 ADD
} __attribute__((packed)) STR_DRV, *PSTR_DRV ;                  // 52-00-00-00 CHG


#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
// Func Cmnd
#define     SPR_DRV_FN_DATASEND             0       // Send
#define     SPR_DRV_FN_DATARCV              1       // Receive
//#define   SPR_DRV_FN_DATASENDRCV          2       // Send & Receive   // reservation for a USB driver
#define     SPR_DRV_FN_USBRESET             3       // (Not-support)

// Inf Cmnd
#define     SPR_DRV_INF_OPEN                4       // OPEN
#define     SPR_DRV_INF_CLOSE               5       // CLOSE
#define     SPR_DRV_INF_INFGET              6       // Info Get
#define     SPR_DRV_INF_SENS                7       // Sensor
#define     SPR_DRV_INF_INFSET              8       // (Not-support)    // 53-32-00-00 ADD
#define     SPR_DRV_END             0xFFFFFFFF      // Dummy

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
// Param
//#define   SPR_PRM_PDL             0   // PDL                          // reservation for a USB driver
//#define   SPR_PRM_USUALLY         1   //                              // reservation for a USB driver
//#define   SPR_PRM_CANCEL          2   // CANCEL                       // reservation for a USB driver
//#define   SPR_PRM_STOP            3   // STOP                         // reservation for a USB driver
//#define   SPR_PRM_NO_RSP          4   // No Resp                      // reservation for a USB driver
//#define   SPR_PRM_STS_CANCEL      0x10// Status Cancel                // reservation for a USB driver
#define     SPR_PRM_SEND_COMMAND    5   // Cmnd Send
#define     SPR_PRM_SEND_VENDOR_IN  6   // Vendor Req(IN)
#define     SPR_PRM_SEND_VENDOR_OUT 7   // Vendor Req(OUT)

#define     SPR_PRM_RESET           0   // (Not-support)

#define     SPR_PRM_OPEN            0   // OPEN

#define     SPR_PRM_CLOSE           0   // CLOS

#define     SPR_PRM_VRT             0   // Version
#define     SPR_PRM_SENS            1   // Sendor
#define     SPR_PRM_TRACE           2   // (Not-support)                // 53-32-00-00 ADD
#define     SPR_PRM_CRTRACE         3   // (Not-support)                // 53-32-00-00 ADD

#define     SPR_PRM_PROTOCOL        0   // (Not-support)                // 53-32-00-00 ADD
#define     SPR_PRM_MOVEMENT        1   // (Not-support)                // 53-32-00-00 ADD


#define     SPR_PRM_START           0   // Start
//#define   SPR_PRM_MASK_START      1   // Mask Start                   // reservation for a USB driver
//#define   SPR_PRM_CHANGE          2   // Change                       // reservation for a USB driver
//#define   SPR_PRM_MASK_CHANGE     3   // Mask Change                  // reservation for a USB driver
#define     SPR_PRM_END             4   // End

//------------------------------------------------------------------------------
// Error Code
//------------------------------------------------------------------------------
#define     ERR_DRV_SPR_SUCCESS                     0x00000000  // #1
#define     ERR_DRV_SPR_CANCEL_END                  0x35200000  // #2
#define     ERR_DRV_SPR_CANCEL_NOPST                0x35200001  //
#define     ERR_DRV_SPR_CANCEL_CROSS_END            0x35210000  // #3
#define     ERR_DRV_SPR_CANCEL_NOTHING              0x35220000  // #4
#define     ERR_DRV_SPR_FUNC                        0xF5000000  // #5
#define     ERR_DRV_SPR_PRM                         0xF5010000  // #6
#define     ERR_DRV_SPR_DRVHND_DIFFER               0xF5020000  // #7
#define     ERR_DRV_SPR_DRV_REMOVE                  0xF5200000  // #8
#define     ERR_DRV_SPR_BLD                         0xF5210000  // #9
#define     ERR_DRV_SPR_INDATA                      0xF5500000  // #10
#define     ERR_DRV_SPR_OUTDATA                     0xF5510000  // #11
#define     ERR_DRV_SPR_INOUTDATA                   0xF5520000  // #12
#define     ERR_DRV_SPR_ENTRY_DEVICE_OVER           0xF5530000  // #13
#define     ERR_DRV_SPR_ENTRY_THREAD_OVER           0xF5540000  // #14
#define     ERR_DRV_SPR_BCC                         0xF5550000  // #15
#define     ERR_DRV_SPR_INDATA_BUFFSZ               0xF5600000  // #16
#define     ERR_DRV_SPR_OUTDATA_BUFFSZ              0xF5610000  // #17
#define     ERR_DRV_SPR_INOUTDATA_BUFFSZ            0xF5620000  // #18
#define     ERR_DRV_SPR_LINE_TIMEOUT                0xF5900000  // #19
#define     ERR_DRV_SPR_COMMAND_TIMEOUT             0xF5A00000  // #20
#define     ERR_DRV_SPR_CLOSE                       0xF5F00000  // #21
#define     ERR_DRV_SPR_OPEN_BUSY                   0xF5F10000  // #22
#define     ERR_DRV_SPR_SEND_BUSY                   0xF5F20000  // #23
#define     ERR_DRV_SPR_RCV_BUSY                    0xF5F30000  // #24
#define     ERR_DRV_SPR_EP_DOWN                     0xF5F40000  // #25
#define     ERR_DRV_SPR_MEMORY                      0xF5F50000  // #26
#define     ERR_DRV_SPR_HANDLE                      0xF5F60000  // #27
#define     ERR_DRV_SPR_REG                         0xF5F70000  // #28
#define     ERR_DRV_SPR_FILE                        0xF5F70100  //              // 53-32-00-00 ADD
#define     ERR_DRV_SPR_DRVCALL                     0xF5F80000  // #29
#define     ERR_DRV_SPR_THREAD                      0xF5F90000  // #30
#define     ERR_DRV_SPR_POSTMSG                     0xF5FA0000  // #31
#define     ERR_DRV_SPR_TRACE                       0xF5FB0000  // #32
#define     ERR_DRV_SPR_CRYPT                       0xF5FC0000  //              // 53-32-00-00 ADD
#define     ERR_DRV_SPR_SUM                         0xF5FD0000  //              // 53-32-00-00 ADD

//------------------------------------------------------------------------------
// extern
//------------------------------------------------------------------------------
extern "C"
{
    unsigned int FnSPRUSB(unsigned int, PSTR_DRV) ;              // Func         // 52-00-00-00 CHG
    unsigned int InfSPRUSB(unsigned int, PSTR_DRV) ;             // Inf          // 52-00-00-00 CHG
} ;

#endif
