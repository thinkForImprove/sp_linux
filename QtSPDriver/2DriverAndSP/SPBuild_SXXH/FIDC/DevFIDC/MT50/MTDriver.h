/*************************************************************************
 * 明泰非接动态库libCnSysReader.so调用dlopen显式加载时会报异常，所以采用隐式加载方式，
 * 该文件已废弃．
 * ***********************************************************************/
#pragma once
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include <QLibrary>
#include "SimpleMutex.h"
#include "QtTypeInclude.h"

//////////////////////////////////////////////////////////////////////////
//USB驱动函数定义
typedef int (*FPOpenUsbDevice)(int vid,int pid,int proid);//open device
typedef int (*FPCloseDevice)();//close
typedef int (*FPGetDeviceVersion)(char *device_version,int *rlen);
typedef int (*FPPiccPowerOff)();
typedef int (*FPPiccApdu)(unsigned char *sCmd, int nCmdLen, unsigned char *sResp, int *nRespLen);
typedef int (*FPPiccPowerOn)(int nMode,unsigned char *sSnr,unsigned char *sAtr, int *nAtrLen);
typedef int (*FPDeviceBeep)(int nMsec,int nMsec_end,int nTime);
typedef int (*FPPiccStatus)();
//////////////////////////////////////////////////////////////////////////
//API 错误码
//非接触卡操作返回返回码
#define UNT_CARD_NOT_SUPPORT         0x3001
#define UNT_CARD_NOT_ACTIVATED       0x3004
#define UNT_CARD_ACTIVATE_FAIL       0x3005
#define UNT_CARD_NOT_ACK             0x3006
#define UNT_CARD_DATA_ERROR          0x3007
#define UNT_CARD_HALT_FAIL           0x3008
#define UNT_CARD_MORE_ONE            0x3009

//软件返回错误
#define LIUSB_IN_OUT_ERROR           0x00A1
#define LIBUSB_PARAM_ERROR           0x00A2
#define LIBUSB_ACCESS_DENY           0x00A3
#define LIBUSB_DEVICE_OFFLINE        0x00A4
#define LIBUSB_NO_DEVICE             0x00A5
#define LIBUSB_DEVICE_BUSY           0x00A6
#define LIBUSB_ACT_TIMEOUT           0x00A7
#define LIBUSB_OVERFLOW              0x00A8
#define LIBUSB_PIPE_ERROR            0x00A9
#define LIBUSB_SYS_CALL_ABORT        0x00AA
#define LIBUSB_NO_MEMORY             0x00AB
#define LIBUSB_PLATFORM_ERROR        0x00AC
#define COMM_TIMEOUT                 0x00B1
#define INVALID_COMM_HANDLE          0x00B2
#define OPEN_PORT_ERROR              0x00B3
#define PORT_ALREADY_OPEN            0x00B4
#define GET_PORT_STATUS_FAIL         0x00B5
#define SET_PORT_STATUS_FAIL         0x00B6
#define READ_FROM_READER_ERROR       0x00B7
#define WRITE_TO_READER_ERROR        0x00B8
#define SET_PORT_BPS_ERROR           0x00B9
#define STX_ERROR                    0x00C1
#define ETX_ERROR                    0x00C2
#define BCC_ERROR                    0x00C3
#define CMD_DATA_MORE_MAX_LEN        0x00C4
#define DATA_ERROR                   0x00C5
#define ROTOCOL_TYPE_ERROR           0x00C6
#define DEVICE_TYPE_ERROR            0x00C7
#define ERROR_USB_CLASS_TYPE         0x00C8
#define DEVICE_COMM_OR_CLOSE         0x00C9
#define DEVICE_COMM_BUSY             0x00CA
#define RECEIVE_DATA_LEN_ERROR       0x00CB
#define CALL_LIBWLT_ERROR            0x00D7
#define WLT_ERROR                    0x00D8
#define OPEN_FOLDER_FAIL             0x00D9
#define FILE_NOT_EXIST               0x00DA
#define TRACK_CARD_DATA              0x00DB
#define UNKNOWN_CARD_TYPE            0x00DC
#define NO_CARD                      0x00DD
#define CARD_NO_POWER_ON             0x00DE
#define SWIPE_CARD_TIMEOUT           0x00E0
#define SWIPE_TRACK_CARD_FAIL        0x00E1
#define SWIPE_TRACK_CARD_NO_OPEN     0x00E2
#define SEND_APDU_ERROR              0x00E3
//////////////////////////////////////////////////////////////////
class CMTDriver: public CLogManage
{
public:
    CMTDriver();
    ~CMTDriver();
public:
    // 加载USB底层通讯库
    long DriverDllLoad();
    // 卸载USB底层驱动库
    void DriverDllRelease();


protected:
    // 错误描述
    LPCSTR GetErrDesc(ULONG ulErrCode);
public:
    char            m_szDesc[256];
    char            m_szDevName[128];
    UINT            m_ulUSBHandle;
    QLibrary        m_mtDll;

    FPOpenUsbDevice         OpenUsbDevice;
    FPCloseDevice           CloseDevice;
    FPGetDeviceVersion      GetDeviceVersion;
    FPPiccPowerOff          PiccPowerOff;
    FPPiccApdu              PiccApdu;
    FPPiccPowerOn           PiccPowerOn;
    FPPiccStatus            PiccStatus;
    FPDeviceBeep            DeviceBeep;

    CQtSimpleMutex          m_cSysMutex;
};

