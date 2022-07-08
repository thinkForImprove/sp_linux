#ifndef DEVFIDC_TMZ_H
#define DEVFIDC_TMZ_H

#include "IDevIDC.h"
#include "QtTypeInclude.h"
#include "TMZDriver.h"
//////////////////////////////////////////////////////////////////////////
//SDK API 错误码
#define INVALID_HANDLE               0x0061
#define DEVICE_DISCONNECT            0x0062
#define FUNC_OR_PARAM_NOT_SUPP       0x1001
#define CMD_EXEC_ERR                 0x1002
#define WRITE_EPPROM_FAIL            0x2001
#define READ_EPPROM_FAIL             0x2002
#define CARD_TYPE_NOT_SUPP           0x3001
#define NO_CARD                      0x3002
#define CARD_POWER_ON                0x3003
#define CARD_POWER_OFF               0x3004
#define CARD_POWER_ON_FAIL           0x3005
#define HANDLE_CARD_DATA_TIMEOUT     0x3006
#define HANDLE_CARD_DATA_ERR         0x3007
#define CARD_HALT_FAIL               0x3008
#define MULTI_CARD                   0x3009
//////////////////////////////////////////////////////////////////////////
//平安叠卡检测
#define  CXFS_FIDC_MULTICARD                (-0x3009)
//////////////////////////////////////////////////////////////////////////
class CDevFIDC_TMZ : public CLogManage, public IDevIDC
{
public:
    CDevFIDC_TMZ(LPCSTR lpDevType);
    virtual ~CDevFIDC_TMZ();

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

    TMZDriver       m_TMZDriver;
    long            m_lDevHdl;
private:
    int ReadIniFile();
    // 更新错误码
    void UpdateErrorCode(long lErrCode = -1, BYTE byErrType = 0);
    int  ConvertErrorCode(int nRetCode);
};



#endif // DEVFIDC_MT50_H
