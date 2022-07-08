#ifndef DEVIDC_CRT_H
#define DEVIDC_CRT_H

#include "IDevIDC.h"
#include "IAllDevPort.h"
#include "QtTypeInclude.h"
#include "cardreader_error.h"
#include <bits/ios_base.h>
#include "USBDrive.h"                           //30-00-00-00(FS#0014)
///////////////////////////////////////////////////////////////////////////
//WIN和LINUX不一样的定义，32位和64位不一样字节大小的
#ifdef QT_WIN32
typedef unsigned long           DWORD;
#define SPETCPATH  "C:/CFES/ETC"
#define LOGPATH    "D:/LOG"
#else
typedef unsigned int           DWORD;
#define SPETCPATH  "/usr/local/CFES/ETC/"
#define LOGPATH    "/usr/local/LOG"
#endif
//////////////////////////////////////////////////////////////////////////
#define TIMEOUT_ACTION                                          (60*1000)
#define TIMEOUT_NO_ACTION                                       (10*1000)
#define TIMEOUT_WAIT_ACTION                                     (5*1000)
#define TIMEOUT_WRITEDATA                                       (3*1000)
#define CRT_PACK_MAX_LEN										(64) // ReportID + Data = 64
#define CRT_PACK_MAX_PAD_LEN									(63) //  64 - ReportID(1byte) = 63
#define CRT_PACK_MAX_CMP_LEN									(58) //  64 - ReportID(1byte) - LEN(2byte) - "Cxx"(3byte) = 58
#define CRT_REPORT_ID											(0x04)  // one packet
#define CRT_MULTI_REPORT_ID										(0xFF)  // Multi packet
const DWORD USB_READ_LENTH = 64;
//////////////////////////////////////////////////////////////////////////
#define VERTIFYISOPEN()\
    {\
        if(m_pDev == nullptr)\
        {\
            Log(ThisModule, __LINE__, "m_pDev == null");\
            return ERR_DEVPORT_NOTOPEN;\
        }\
        if(!m_pDev->IsOpened())\
        {\
            Log(ThisModule, __LINE__, "Not Open Dev");\
            return ERR_DEVPORT_NOTOPEN;\
        }\
    }

//////////////////////////////////////////////////////////////////////////
/// \brief The CDevIDC_CRT class
///
class CDevIDC_CRT :
    public CLogManage,
    public IDevIDC
{
public:
    CDevIDC_CRT(LPCSTR lpDevType);
    virtual ~CDevIDC_CRT();

public:
    virtual void Release();
    virtual INT Open(const char *pMode);
    virtual INT Init(CardAction eActFlag, WobbleAction nNeedWobble = WOBBLEACTION_NOACTION);
    virtual void Close();
    virtual INT EatCard();
    virtual INT EjectCard(); 
    virtual INT AcceptCard(ULONG ulTimeOut, bool Magnetic = true);
    virtual INT CancelReadCard();
    virtual INT WriteTracks(const STTRACK_INFO &stTrackInfo);
    virtual INT ReadTracks(STTRACK_INFO &stTrackInfo);
    virtual INT DetectCard(IDC_IDCSTAUTS &IDCstatus);
    virtual INT GetFWVersion(char pFWVersion[MAX_LEN_FWVERSION], unsigned int &uLen);
    virtual INT SetRecycleCount(LPCSTR pszCount);

    /*--------------------- 以下IC卡操作接口-------------------------*/
    virtual INT ICCPress();
    virtual INT ICCRelease();
    virtual INT ICCActive(char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen);
    virtual INT ICCReset(ICCardReset eResetFlag, char pATRInfo[MAX_LEN_ATRINFO], unsigned int &uATRLen);
    virtual INT ICCMove();
    virtual INT ICCDeActivation();

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
    virtual INT ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen, DWORD dwBuffSz);
    virtual INT SetRFIDCardReaderLED(LedType eFlagLedType, LedAction eFlagLedAct);
    virtual INT SetRFIDCardReaderBeep(unsigned long ulTime = 5);
    virtual BOOL GetSkimmingCheckStatus();                  //30-00-00-00(FS#0014)
public:                                                       //30-00-00-00(FS#0014)
    BOOL bSkimmingError() const;                              //30-00-00-00(FS#0014)
    void setBSkimmingError(const BOOL &bSkimmingError);       //30-00-00-00(FS#0014)

    IOMCIDCPARAM stIDC() const;                               //30-00-00-00(FS#0014)
    CUSBDrive &cUSBDrive();                                   //30-00-00-00(FS#0014)
private:
    INT InitConfig();
    LONG ReadConfig();
    void GetErrorDesc(int  nErrorCode, char *pszDesc);
    INT CardReaderError(const char *pCode, const char *pszCallFunc);

    LONG StopCmd();
    //发送命令串，并接收ACK
    INT SendCmd(const char *pszCmd, const char *lpData, int nLen, const char *pszCallFunc);
    INT SendSinglePacket(const char* pszCmd, const char *lpData, int nLen, const char *pszCallFunc);
    INT SendMultiPacket(const char *pszCmd, const char *lpData, int nLen, const char *pszCallFunc);

    //功能：读取读卡器的返回数据  nTimeout超时(毫秒)
    INT GetResponse(char *pszResponse, int nLen, int nTimeout, const char *pszCallFunc);

    // 收发命令
    INT SendReadCmd(LPCSTR pszCmd, LPCSTR lpData, LPSTR pszResponse, int &nInOutReadLen, int nTimeout, LPCSTR pszCallFunc);

    DWORD GetReportLenFromLength(int nInLen, int &nOutSendLen);
    bool InFraudProtect();
    bool TamperSensorDetect();

    INT SetWobble(WobbleAction nNeedWobble);
    INT SetTimeOutMonitor(MONITOR_TYPE MonitorType, int nTimeOut);

    INT UpdateCardStatus();
    INT ConvertErrorCode(long iRet);
    LONG OpenDevice();

    INT GetBit(BYTE reply, WORD wposition);

    //禁止进卡
    int ForbiddenAcceptCard();
    int HWErrToStdErr(int iCardError);
    int GetCardStatusC12();

    INT SetESUMode(int iMode);              //30-00-00-00(FT#0064)
private:
    INT                             m_nLastError;
    BOOL                            m_bCardInGatePosition;     //卡在读卡器口位置，用于重进卡判断
    BOOL                            m_bReTakeIn;               //重进卡控制,通过注册表配置是否启用
    INT                             m_nEjectedCard;            //退卡标志位
    INT                             m_nTakeCard;               //退卡后是否取走了卡,用于判断是否调用重进卡,只在NOTAKEN调用重进卡
    DWORD                           m_bTamperSensorStatus;     //防盗嘴状态
    DWORD                           m_dwLastFraudDetectedTime;
    BOOL                            m_bFraudDetected;
    BOOL                            m_bICCActived;
    BOOL                            m_bICCRelease;              //IC卡是否释放触点
    BOOL                            m_bICCJam;                  //IC卡是否堵塞
    BOOL                            m_bOpen;                    //读卡器开启标志   //30-00-00-00(FT#0019)
    BOOL                            m_bReOpen;                  //多次重连开启标志  //30-00-00-00(FT#0019)
private:
    IDC_IDCSTAUTS                   m_nCardStatus;
    string                          m_strMode;

private:
    IOMCIDCPARAM                    m_stIDC;
    CINIFileReader                  m_cINI;
    CQtDLLLoader<IAllDevPort>       m_pDev;
    CSimpleMutex                    m_MutexAction;
    CAutoEvent                      m_SemCancel;

    QByteArray                      m_initParam;                //30-00-00-00(FT#0030)
    BOOL                            m_bSkimmingError;           //30-00-00-00(FS#0014)
    CUSBDrive                       m_cUSBDrive;                //30-00-00-00(FS#0014)
    pthread_t                       m_skimmingTid;              //30-00-00-00(FS#0014)
};
#endif // DEVIDC_CRT_H
