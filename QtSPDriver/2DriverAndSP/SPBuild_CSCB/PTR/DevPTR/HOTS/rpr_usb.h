#ifndef RPR_USB_H
#define RPR_USB_H
#include "devptr_rpr.h"
#include <QLibrary>
#include <unistd.h>

#define OK 1
#define SPR_RSV_BUFF_SIZE       16      // リサ＂—フ＂ハ＂ッファサイス＂
#define SPR_DRV_BUFF_SIZE       64      // ト＂ライハ＂构造体ハ＂ッファサイス＂
#define SPR_SNS_BUFF_SIZE       14      // センサハ＂ッファサイス＂
#define SPR_SND_BUFF_SIZE       4096    // 送信发送字节数
#define SPR_RCV_BUFF_SIZE       4096    // 受信发送字节数
#define SPR_OPN_SBUF_SIZE       16      // オ—フ°ン送信ハ＂ッファサイス＂
#define SPR_OPN_RBUF_SIZE       16      // オ—フ°ン受信ハ＂ッファサイス＂
#define SPR_CNL_BUFF_SIZE       32      // キャンセル送信ハ＂ッファサイス＂
#define SPR_SNSCHG_BUFF_SIZE    32      // センサマスクハ＂ッファサイス＂

#define OPENTIMER_DEF           30      // オ—フ°ンタイムアウト值
#define SENDTIMER_DEF           10      // 送信タイムアウト值
#define RECVTIMER_DEF           2400    // 受信タイムアウト值


//#define WM_USER  0x0400
#define AP_MSG  WM_USER + 100
#define DVMSG_HSPRT_SNS         AP_MSG + 19510  //ＳＰＲ センサ
#define DVMSG_HSPRT_RCV         AP_MSG + 19511  //ＳＰＲ 受信
#define DVMSG_HSPRT_SNDE        AP_MSG + 19512  //ＳＰＲ 送信终了
#define DVMSG_HSPRT_CNLE        AP_MSG + 19513  //ＳＰＲ キャンセル终了
#define DVMSG_HSPRT_LOOPE       AP_MSG + 19514  //ＳＰＲ ル－プ终了
#define DVMSG_HSPRT_JBKAN       AP_MSG + 19515  //ＳＰＲ 准备完オン
#define DVMSG_HSPRT_EPDWN       AP_MSG + 19516  //ＳＰＲ ＥＰダウン
#define DVMSG_HSPRT_LPDWN       AP_MSG + 19517  //ＳＰＲ ＬＰダウン
#define DVMSG_HSPRT_TMOUT       AP_MSG + 19518  //ＳＰＲ タイムアウト

// USBト＂ライハ＂终了チェック用コ—ルハ＂ック种别
#define     DRV_RETURN_DRVKIDO      0       // ト＂ライハ＂起动
#define     DRV_RETURN_ONSENSER     1       // センサ变化通知
#define     DRV_RETURN_ONSNDEND     2       // 送信终了通知
#define     DRV_RETURN_ONJNBKAN     3       // ト＂ライハ＂オ—フ°ン终了通知

#define     DRV_VENDOR_VRT              0x03    // ファ—ムVRT取得
#define     DRV_VENDOR_RESET            0x07    // フ°リンタリセット
#define     DRV_VENDOR_TRACEABILITY     0x11    // トレ-サビリティ
#define     DRV_VENDOR_MAINT_COUNTER    0x12    // メンテナンスカウンタ
#define     DRV_VENDOR_USB_ERROR        0x13    // USBエラ
#define     DRV_VENDOR_ERROR_LOG        0x14    // エラLog
#define     DRV_VENDOR_ROLL_REPLENISH   0x00    // ロ-ル装填コマンド
#define     DRV_VENDOR_ROLL_SAVE        0x01    // ロ-ル退避コマンド
#define     DRV_VENDOR_SETFUNCTION      0x08    //
#define     DRV_VENDOR_DIPSET_REQ       0x15    // DIP
#define     DRV_VENDOR_HEADOPEN_DOT     0x16    // ヘッドオ-プンドット数
//only for HOTS
#define     DRV_VENDOR_EXTRA_STATUS     0x17    // 扩展状态

#define Syoki_st        1
#define SnsEnd_W        3
#define RCV_W1          4
#define OpenEnd_W       20

const   long    USB_RtnCd_OK1          = 0x00000000 ;   // 正常终了
const   long    USB_RtnCd_OK2          = 0x33000000 ;   // 正常终了
const   long    USB_RtnCd_MSK_ERRLVL   = 0xFF000000 ;   // エラ—レヘ＂ルマスク值
const   long    USB_RtnCd_MSK_WinAK1   = 0x20000000 ;   // Win32API/AK1マスク值
const   long    USB_RtnCd_CMD_TIMEOUT  = 0xF5A00000 ;   // コマント＂タイムアウト
const   long    USB_RtnCd_MSK_LPDWN    = 0x20000000 ;   // LPタ＂ウン

const long  DVEP_RET_NoDt       = 7;    //no data

#define HIWORD(I) (( WORD ) ( (( DWORD )(I) >> 16) & 0xFFFF ) )
#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIBYTE(I) (( BYTE ) ( (( WORD )(I) >> 8) & 0xFF ) )
#define LOBYTE(l) ((BYTE)((WORD)(l) & 0xff))

class RPR_USB
{
public:
    RPR_USB();
    long    N_Open_Usb();
    long    N_Reset_Usb(BOOL bInit);
    long    N_Close_Usb();
    long    N_CutPaper_Usb(LPBYTE lpData, LONG ulDataSz);
    long    N_SendData_Usb(LPBYTE lpData, LONG ulDataSz);
    BYTE    N_GetStatus_Usb(enSPRSTATUS enSPRStatus);
    LPBYTE  N_GetStatus_Usb();
    UINT    N_GetDevHandle_Usb();
    ULONG   N_GetErrCode_Usb();

    void    N_OnJnbKan(WPARAM wParam, LPARAM lParam);
    void    N_OnSensor(WPARAM wParam, LPARAM lParam);
    void    N_OnSndEnd(WPARAM wParam, LPARAM lParam);

private:
    long    lUsbDllLoad();
    long    lUsbDllCall(unsigned int uifncNo, unsigned short usParam, BYTE i);
    long    lUsbDllCall(unsigned int uifncNo, unsigned short usParam, LPBYTE lpData, LONG ulDataSz);
    long    lUsbDllCall_all(unsigned int uifncNo, unsigned short usParam, BYTE i, LPBYTE lpData, LONG ulDataSz, WORD wParamValue = 0, WORD wLength = 0);
    long    lUsbDrvSyuryoCheck(long lDrvRtn, BYTE byProcType);

private:
    LPBYTE  Buff_hnd;
    LPBYTE  lpStrDrvDll;
    LPBYTE  lpbySensLP_Buf;
    LPBYTE  lpbyVrtLP_Buf;
    LPBYTE  lpbySndLP_Buf;
    LPBYTE  lpbyOpnSLP_Buf;
    LPBYTE  lpbyOpnRLP_Buf;
    LPBYTE  lpbyCnlLP_Buf;
    LPBYTE  lpbyVrtDrv_Buf;
    LPBYTE  lpbyVendor_Buf;
    LPBYTE  lpbyPrevSensLP_Buf;

    unsigned int    uiSndLP_sz;
    STR_DRV strDrvInf_SPR;

    unsigned int    uiOpenTimer;
    unsigned int    uiSendTimer;

    BOOL    bEPDwn;
    BOOL    bLPDwn;
    BOOL    bTMOut;

    BYTE    status;                // 用来判断回调函数的状态
    long    lOpenEndRet;           // Open回调函数返回值
    long    lRcvEndRet;

    BOOL    bSnsData;

    ULONG   ulUsbErrCode;          // 硬件设备返回错误码
};

#endif // RPR_USB_H
