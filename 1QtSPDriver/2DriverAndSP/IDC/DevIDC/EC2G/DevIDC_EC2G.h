#pragma once

#include "IDevIDC.h"
#include "cardreader_error.h"
#include "IAllDevPort.h"
#include "QtTypeInclude.h"

//////////////////////////////////////////////////////////////////////////
#define TIMEOUT_ACTION      (60*1000)
#define TIMEOUT_NO_ACTION   (10*1000)
#define TIMEOUT_WRITEDATA   (3*1000)
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
class CDevIDC_EC2G :
    public CLogManage,
    public IDevIDC
{
public:
    CDevIDC_EC2G(LPCSTR lpDevType);
    virtual ~CDevIDC_EC2G();

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
    virtual int ICCChipIO(ICCardProtocol eProFlag, char *pInOutData, unsigned int &nInOutLen, DWORD dwBuffSz);
    virtual int SetRFIDCardReaderLED(LedType eFlagLedType, LedAction eFlagLedAct);
    virtual int SetRFIDCardReaderBeep(unsigned long ulTime = 5);

private:
    int InitConfig();
    LONG ReadConfig();
    int  TransparentCardSetting(bool bEnableDetect);
    void GetErrorDesc(int  nErrorCode, char *pszDesc);
    int CardReaderError(const char *pCode, const char *pszCallFunc);

    long StopCmd();
    //发送命令串，并接收ACK
    int SendCmd(const char *pszCmd, int nLen, const char *pszCallFunc);

    //功能：读取读卡器的返回数据  nTimeout超时(毫秒)
    int GetResponse(char *pszReponse, int nLen, int nTimeout, const char *pszCallFunc);
    int GeneralErrorCodeByCustom(int iRet);
    DWORD GetReportLenFromLength(int nInLen, int &nOutSendLen);
    bool InFraudProtect();
    bool TamperSensorDetect();
    int SetICControlInfomationMode();
    int SetWobble(WobbleAction nNeedWobble);
    int SetTimeOutMonitor(MONITOR_TYPE MonitorType, int nTimeOut);

    ////
    int UpdateCardStatus();
    int ConvertErrorCode(long iRet);
    long OpenDevice();
private:
    IDC_IDCSTAUTS                   m_nCardStatus;
    int                             m_nLastError;
    BOOL                            m_bCardInGatePosition;     //卡在读卡器口位置，用于重进卡判断
    BOOL                            m_bReTakeIn;               //重进卡控制,通过注册表配置是否启用
    int                             m_nEjectedCard;             //退卡标志位
    int                             m_nTakeCard;                //退卡后是否取走了卡,用于判断是否调用重进卡,只在NOTAKEN调用重进卡
    BOOL                            m_bCardTooShortError;      //进卡后卡短错误，用于判断退卡命令
    DWORD                           m_bTamperSensorStatus;     //防盗嘴状态

    DWORD                           m_dwLastFraudDetectedTime;
    BOOL                            m_bFraudDetected;
    BOOL                            m_bICCActived;
    USHORT                          m_OutPutReportByteLength;
    //待配置
private:
    IOMCIDCPARAM                    m_stIDC;
    CINIFileReader                  m_cINI;
    bool                            m_bOpen;
    string                          m_strMode;
private:
    CSimpleMutex                    m_MutexAction;
    CQtDLLLoader<IAllDevPort>       m_pDev;
    CAutoEvent                      m_SemCancel;
};

