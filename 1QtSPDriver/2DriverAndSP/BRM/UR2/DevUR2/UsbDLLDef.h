//--------------------------------------------------------------------
// [ŽÐŠO”é]
// COPYRIGHT (c) HITACHI-OMRON TERMINAL SOLUTIONS,CORP.
// 2006,2016.All rights reserved.
//--------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
// NAME     = USBDLL (USB Driver Dll)
// File Name= USBDLLDEF.H
// DATE     = 2006/12/07(CREATE)
// HISTORY  = 01-00-00-00,2006/01/24 : For ZBV
// HISTORY  = 01-00-00-01,2006/12/07 : Add Command(Send,Reciv,InfGet)
// HISTORY  = 52-00-00-00,2015/04/23 : Linux ‘Î‰ž
// HISTORY  = 52-32-00-00,2015/10/01 : Linux Product version
// HISTORY  = 53-32-01-00,2016/01/15 : Linux Async version
////////////////////////////////////////////////////////////////////////////////
#ifndef __usbdlldef_h__
#define __usbdlldef_h__

#ifndef _STRUCTDEF
#define _STRUCTDEF

//------------------------------------------------------------------------------
// Driver Function Struct
//------------------------------------------------------------------------------
typedef struct _STR_DRV
{
    unsigned int    uiDrvHnd;                   // 52-00-00-00 CHG
    unsigned short  usParam;                    // 52-00-00-00 CHG
    unsigned short  usExParam;                  // 52-00-00-00 CHG
    void           *pvDataInBuffPtr;            // 52-00-00-00 CHG
    unsigned int    uiDataInBuffSz;             // 52-00-00-00 CHG
    void           *pvDataOutBuffPtr;           // 52-00-00-00 CHG
    unsigned int    uiDataOutReqSz;             // 52-00-00-00 CHG
    unsigned int    uiDataOutBuffSz;            // 52-00-00-00 CHG
    unsigned int    uiTimer;                    // 52-00-00-00 CHG
    void           *pvReserve;                  // 52-00-00-00 ADD
    //  HWND        hwWndHnd ;                      // 52-00-00-00 DEL
    //  UINT        uiWndMsg ;                      // 52-00-00-00 DEL
    unsigned int    uiWndMsg ;                  // 53-32-01-00 ADD
    void           *pvCallBackPtr;              // 53-32-01-00 ADD
} __attribute__((packed)) STR_DRV, *PSTR_DRV ;  // 52-00-00-00 CHG

#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
// Func Cmnd
#define     USB_DRV_FN_DATASEND             0       // Send
#define     USB_DRV_FN_DATARCV              1       // Reciv
#define     USB_DRV_FN_DATASENDRCV          2       // Send & Reciv
#define     USB_DRV_FN_USBRESET             3       // (Not-support)        // 52-32-00-00 ADD
// Inf Cmnd
#define     USB_DRV_INF_OPEN                4       // OPEN
#define     USB_DRV_INF_CLOSE               5       // CLOSE
#define     USB_DRV_INF_INFGET              6       // Info Get
#define     USB_DRV_INF_SENS                7       // Sensor
#define     USB_DRV_INF_INFSET              8       // (Not-support)        // 52-32-00-00 ADD

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
// Param
#define     USB_PRM_PDL                     0   // PDL
#define     USB_PRM_USUALLY                 1   // 
#define     USB_PRM_CANCEL                  2   // CANCEL                   // 52-32-00-00 ADD
#define     USB_PRM_STOP                    3   // STOP                     // 52-32-00-00 ADD
#define     USB_PRM_NO_RSP                  4   // NO RSP                   // 53-32-01-00 ADD
#define     USB_PRM_STS_CANCEL          0x10    // STATUS CANCEL            // 53-32-01-00 ADD
#define     USB_PRM_ZERO_NO_RETRY       0x11    // ZERO NO RETRY            // 53-32-01-00 ADD

//
#define     USB_PRM_OPEN                    0   // OPEN
//
#define     USB_PRM_CLOSE                   0   // CLOSE
//
#define     USB_PRM_VRT                     0   // Version
#define     USB_PRM_SENS                    1   // Sensor
#define     USB_PRM_TRACE                   2   // (Not-support)            // 53-32-01-00 ADD
#define     USB_PRM_CRTRACE                 3   // (Not-support)            // 53-32-01-00 ADD

#define     USB_PRM_PROTOCOL                0   // (Not-support)            // 53-32-01-00 ADD
#define     USB_PRM_MOVEMENT                1   // (Not-support)            // 53-32-01-00 ADD
//
#define     USB_PRM_START                   0   // Start                    // 52-32-00-00 ADD
#define     USB_PRM_MASK_START              1   // Mask Start               // 52-32-00-00 ADD
#define     USB_PRM_CHANGE                  2   // Change                   // 52-32-00-00 ADD
#define     USB_PRM_MASK_CHANGE             3   // Mask Change              // 52-32-00-00 ADD
#define     USB_PRM_END                     4   // End                      // 52-32-00-00 ADD
//
#define     USB_PRM_RESET                   0   // Reset                    // 52-32-00-00 ADD

//------------------------------------------------------------------------------
// Error Code
//------------------------------------------------------------------------------
#define     ERR_DRV_USB_SUCCESS                     0x00000000  // #1
#define     ERR_DRV_USB_CANCEL_END                  0x33200000  // #2
#define     ERR_DRV_USB_CANCEL_NOPST                0x33200001  // #3
#define     ERR_DRV_USB_ALREADY_COMPLETE            0x33200002  //          // 52-32-00-00 ADD
#define     ERR_DRV_USB_CANCEL_CROSS_END            0x33210000  // #4
#define     ERR_DRV_USB_CANCEL_NOTHING              0x33220000  // #5
#define     ERR_DRV_USB_FUNC                        0xF3000000  // #6
#define     ERR_DRV_USB_PRM                         0xF3010000  // #7
#define     ERR_DRV_USB_DRVHND_DIFFER               0xF3020000  // #8
#define     ERR_DRV_USB_DRV_REMOVE                  0xF3200000  // #9
#define     ERR_DRV_USB_BLD                         0xF3210000  // #10
#define     ERR_DRV_USB_INDATA                      0xF3500000  // #11
#define     ERR_DRV_USB_OUTDATA                     0xF3510000  // #12
#define     ERR_DRV_USB_INOUTDATA                   0xF3520000  // #13
#define     ERR_DRV_USB_ENTRY_DEVICE_OVER           0xF3530000  // #14
#define     ERR_DRV_USB_ENTRY_THREAD_OVER           0xF3540000  // #15
#define     ERR_DRV_USB_BCC                         0xF3550000  // #16
#define     ERR_DRV_USB_INDATA_BUFFSZ               0xF3600000  // #17
#define     ERR_DRV_USB_OUTDATA_BUFFSZ              0xF3610000  // #18
#define     ERR_DRV_USB_INOUTDATA_BUFFSZ            0xF3620000  // #19
#define     ERR_DRV_USB_LINE_TIMEOUT                0xF3900000  // #20
#define     ERR_DRV_USB_COMMAND_TIMEOUT             0xF3A00000  // #21
#define     ERR_DRV_USB_CLOSE                       0xF3F00000  // #22
#define     ERR_DRV_USB_OPEN_BUSY                   0xF3F10000  // #23
#define     ERR_DRV_USB_SEND_BUSY                   0xF3F20000  // #24
#define     ERR_DRV_USB_RCV_BUSY                    0xF3F30000  // #25
#define     ERR_DRV_USB_EP_DOWN                     0xF3F40000  // #26
#define     ERR_DRV_USB_MEMORY                      0xF3F50000  // #27
#define     ERR_DRV_USB_HANDLE                      0xF3F60000  // #28
#define     ERR_DRV_USB_REG                         0xF3F70000  // #29
#define     ERR_DRV_USB_FILE                        0xF3F70100  //          // 53-32-01-00 ADD
#define     ERR_DRV_USB_DRVCALL                     0xF3F80000  // #30
#define     ERR_DRV_USB_THREAD                      0xF3F90000  // #31
#define     ERR_DRV_USB_POSTMSG                     0xF3FA0000  // #32
#define     ERR_DRV_USB_TRACE                       0xF3FB0000  // #33
#define     ERR_DRV_USB_CRYPT                       0xF3FC0000  //          // 52-32-00-00 ADD
#define     ERR_DRV_USB_SUM                         0xF3FD0000  //          // 52-32-00-00 ADD

#define     ERR_DRV_USB_CMDSND_SETUP                0xF3280000  // #34
#define     ERR_DRV_USB_CMDSND_SETUP_CHK            0xF3290000  // #35
#define     ERR_DRV_USB_CMDRCV_SETUP                0xF32A0000  // #36
#define     ERR_DRV_USB_CMDRCV_SETUP_CHK            0xF32B0000  // #37
//------------------------------------------------------------------------------
// extern
//------------------------------------------------------------------------------
extern "C"
{
    unsigned int FnATMUSB(unsigned int, PSTR_DRV) ;      // Func // 52-00-00-00 CHG
    unsigned int InfATMUSB(unsigned int, PSTR_DRV) ;     // Inf  // 52-00-00-00 CHG
} ;

#endif
