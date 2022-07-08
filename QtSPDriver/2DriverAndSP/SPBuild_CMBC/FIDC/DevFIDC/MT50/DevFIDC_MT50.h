#ifndef DEVFIDC_MT50_H
#define DEVFIDC_MT50_H

#include "IDevIDC.h"
#include "QtTypeInclude.h"
//////////////////////////////////////////////////////////////////////////
//SDK API 错误码
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
#define CARD_NO_RESPONSE             0x00DF
#define SWIPE_CARD_TIMEOUT           0x00E0
#define SWIPE_TRACK_CARD_FAIL        0x00E1
#define SWIPE_TRACK_CARD_NO_OPEN     0x00E2
#define SEND_APDU_ERROR              0x00E3
//////////////////////////////////////////////////////////////////////////
//平安叠卡检测
#define  CXFS_FIDC_MULTICARD                (-0x3009)
//////////////////////////////////////////////////////////////////////////
class CDevFIDC_MT50 :
    public CLogManage,
    public IDevIDC
{
public:
    CDevFIDC_MT50(LPCSTR lpDevType);
    virtual ~CDevFIDC_MT50();

public:
    virtual void Release();
    virtual int Open(const char *pMode);
    virtual int Init(CardAction eActFlag, WobbleAction nNeedWobble = WOBBLEACTION_NOACTION);
    virtual void Close();
    virtual int EatCard();
    virtual int EjectCard();
    virtual int AcceptCard(ULONG ulTimeOut, bool Magnetic = true);
    virtual int CancelReadCard();
    virtual int WriteTracks(const STTRACK_INFO &stTrackInfo);
    virtual int ReadTracks(STTRACK_INFO &stTrackInfo);
    virtual int DetectCard(IDC_IDCSTAUTS &IDCstatus);
    virtual int GetFWVersion(char pFWVersion[MAX_LEN_FWVERSION], unsigned int &uLen);
    virtual int SetRecycleCount(LPCSTR pszCount);

    /*--------------------- 以下IC卡操作接口-------------------------*/
    virtual int ICCPress();
    virtual int ICCRelease();
    virtual int ICCActive(char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen);
    virtual int ICCReset(ICCardReset eResetFlag, char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen);
    virtual int ICCMove();
    virtual int ICCDeActivation();

    /************************************************************
    ** 功能：数据接收与发送
    ** 输入：eProFlag:表示读卡器在主机与IC卡之间接收与发送数据所用的协议
    ** 输出：pInOutData:输入数据缓存
             nInOutLen：输入数据有效长度
    ** 输出：pInOutData:输出数据缓存
             nInOutLen：输出数据有效长度
             T0协议时InLen范围为：4~261，OutLen范围为：2~258；
             T1协议时InLen范围为：4~360，OutLen范围为：2~320；
    ** 返回：见返回错误码定义
    ************************************************************/
    int ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen, DWORD dwBuffSz);
    int SetRFIDCardReaderLED(LedType eFlagLedType, LedAction eFlagLedAct);
    int SetRFIDCardReaderBeep(unsigned long ulTime = 5);
    virtual BOOL GetSkimmingCheckStatus();                  //30-00-00-00(FS#0014)

private:
    //INI 配置项目
    int             m_nVid;
    int             m_nPid;
    int             m_nProid;
    int             m_nDetectCardMode;

    bool            m_bOpen;
    string          m_strMode;
    CSimpleMutex    m_MutexAction;
    CAutoEvent      m_SemCancel;
    CINIFileReader  m_cINI;

    int             m_iLastError;
    IDC_IDCSTAUTS   m_eCardStatus;
    DEVIDCSTATUS    m_stDevIdcStatus;
    bool            m_bIccActive;

private:
    int ReadIniFile();
    // 更新错误码
    void UpdateErrorCode(long lErrCode = -1, BYTE byErrType = 0);
    int  ConvertErrorCode(int nRetCode);
};



#endif // DEVFIDC_MT50_H
