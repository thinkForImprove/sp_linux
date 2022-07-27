#pragma once
#include "IDevIDC.h"
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
class DevFIDC_CJ201 :
    public CLogManage,
    public IDevIDC
{
public:
    DevFIDC_CJ201(LPCSTR lpDevType);
    virtual ~DevFIDC_CJ201();

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
    virtual int GetFWVersion(char pFWVersion[10][MAX_LEN_FWVERSION], unsigned int &uLen);
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
    int SendAndRecv(char cCmd, char *lpRecvData, int &nLenOut, LPCSTR lpModel);
    int SendAndRecv(char cCmd, const char *lpSendData, int nDataLen, char *lpRecvData, int &nLenOut, LPCSTR ThisModule);
    int ConvertErrorCode(long iRet);
    int ActiveCard(char *szATR, unsigned int &nATRLen);
    int AnalyseCmdResult(const char *lpResData, int nDataLen, char *lpATRData, unsigned int &nATRLen);

    int OpenLed();
    int CloseLed();
    int CloseField();
    int MoveCard();
    bool IsFormatCorrect(const char *lpResData, int nDataLen);
    int SendAndRecvData(LPCSTR lpSendData, DWORD dwSendLen, LPSTR lpReadData, DWORD &dwReadLen, DWORD dwTimeOut);
private:
    IDC_IDCSTAUTS m_nCardStatus;
    int m_nLastError;

    //待配置
private:
    bool                            m_bOpen;
    string                          m_strMode;

private:
    CSimpleMutex                    m_MutexAction;
    CQtDLLLoader<IAllDevPort>       m_pDev;
    CAutoEvent                      m_SemCancel;
};

