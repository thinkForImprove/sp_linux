#ifndef DevCOR_BCR106_H
#define DevCOR_BCR106_H
#include "IDevCOR.h"
#include "IAllDevPort.h"
#include "ILogWrite.h"
#include "AutoQtHelpClass.h"
#include "QtDLLLoader.h"
#include "SimpleMutex.h"
#include "INIFileReader.h"
#include "MultiString.h"

#define SEP_STR_COMMA           ","

#define TIMEOUT_ACTION     (200 * 1000)
#define TIMEOUT_NO_ACTION  (1000)
#define TIMEOUT_WRITEDATA  (1000)
#define MAX_CMD_BUFFER_LEN (2048)
//设备活动状态
typedef struct tag_DEV_ACTIV_STATUS
{
    BOOL bSingulRun;      // TURE：分离器运行中
    BOOL bEscalRun;       // TURE: 自动传送带运行中
    BOOL bMoneyIn;        // TURE：接收硬币过程中
    BOOL bMoneyOut;       // TURE：支付硬币过程中
    BOOL bCoinInErr;      // TURE：接收硬币阻塞或出错
    BOOL bCoinOutErr;     // TURE：支付硬币阻塞或出错
    BOOL bInitialising;   // TURE：初始化中，当为此值时，设备不响应任何指令，直至此参数转为FALSE。
    BOOL bEntryFlapOpen;  // TURE: 投币口已打开
    BOOL bContReject;     // TURE: 持续拒绝
    BOOL bConfChange;     // TURE：配置变更
    BOOL bRejeDivert;     // TURE: 拒绝移动
} ST_DEV_ACTIV_STATUS;
typedef struct tag_COIN_POSITION
{
    USHORT usCN005A;
    USHORT usCN010B;
    USHORT usCN010C;
    USHORT usCN010D;
    USHORT usCN050B;
    USHORT usCN050C;
    USHORT usCN050D;
    USHORT usCN100B;
    USHORT usCN100C;
} ST_COIN_POSITION;
class CDevCOR_BCR106 : public IDevCOR, public CLogManage
{
public:
    CDevCOR_BCR106(LPCSTR lpDevType);
    virtual ~CDevCOR_BCR106() override;
    void Release() override;
    long Open(LPCSTR lpMode) override;
    long Close() override;
    long GetFWVersion(char pszFWVersion[MAX_LEN_FWVERSION]) override;
    long GetDeviceCaps(void *pCaps) override;
    long GetStatus(void *pStatus = nullptr) override;
    long Reset(ulong ulAction, void *pData = nullptr) override;
    long Cancel(ulong ulReasion, void *pData) override;
    long GetDeviceCode() override;
    long GetCoinCylinderList(ST_COINCYLINDER_INFO pCoinCylinderInfo[MAX_COINCYLINDER_NUM]) override;
    long SetCoinCylinderInfo(ST_COINCYLINDER_INFO pCoinCylinderInfo) override;
    long OpenClap() override;
    long CloseClap() override;
    long SetSupportCoinTypes(USHORT usCoinEnable) override;
    long CashIn(DWORD dwTimeOut, ULONG RecvItems[MAX_COINCYLINDER_NUM], ST_RETRACTBIN_COUNT &stRetract, DEV_CUERROR eCUError[MAX_COINCYLINDER_NUM],
                OnCashIn fnCallback = nullptr) override;
    long Dispense(const ULONG RequestItems[MAX_COINCYLINDER_NUM], ULONG DispensedItems[MAX_COINCYLINDER_NUM],
                  DEV_CUERROR arywCUError[MAX_COINCYLINDER_NUM]) override;
    long Count(int iCylinderNo, ULONG DispensedItems[MAX_COINCYLINDER_NUM], ULONG CountedItems[MAX_COINCYLINDER_NUM],
               DEV_CUERROR arywCUError[MAX_COINCYLINDER_NUM]) override;
    long GetErrorString(long lErrorCode, char szErrorString[MAX_LEN_BUFFER]) override;
    long UpdateDevicePDL() override;
    long CoinIn(USHORT usCoinEnable) override;
    long CoinInEnd() override;
    long Payout(ULONG ulCoinValue) override;
    long GetPayoutValue(ULONG &ulCoinValue) override;
    long GetMoneyCount(MONEY_COUNTERS_TYPE iCounters, ULONG &ulMoneyValue) override;
    long ClearMoneyCounter() override;
    long GetDataStorageAvai(int &iMemoryType, int &iReadBlocks, int &iReadBlockSize, int &iWriteBlocks, int &iWriteBlockSize) override;
    long WriteData(int iBlockNo, BYTE *szData, int iLen) override;
    long ReadData(int iBlockNo, BYTE szData[MAX_DATA_LENGTH], int &iLen) override;
    long GetDevInfo(ST_DEV_INFO &stDevInfo) override;
    long Rollback(DEV_CUERROR arywCUError[MAX_COINCYLINDER_NUM]) override;

    void SetDeviceConfig(void *pConfig) override;
    // IDevBase interface
public:
    long IsNeedDevicePDL() override;
    long GetLastError() override;

private:
    CQtDLLLoader<IAllDevPort> m_pDev;
    //    string                          m_strMode;
    ST_COIN_POSITION m_stCoinPos;
    USHORT m_usCoinTypeSupported;
    BYTE CalcSimpleCheckSum(BYTE pszCmd[MAX_CMD_BUFFER_LEN], DWORD nCmdLen);
    long Excute(LPCSTR pszCallFunc, BYTE bCmdIndex, BYTE *pszCmdData, DWORD dwCmdDataLen, BYTE *pszResponse, DWORD &dwInOutRespLen,
                DWORD iTimeOut = TIMEOUT_NO_ACTION);
    long ConvertErrorCode(long lRet);
    long Poll();
    long GetTimeClock(time_t &tClock);
    long SetTimeClock(time_t tClock);
    long SetCashValueInDev(ULONG ulCashValue);
    long GetCashValueInDev(ULONG &ulCashValue);
    long SelfCheck(BYTE &ucErrorCode, BYTE &ucExInfo);
    long GetActiveStatus(ST_DEV_ACTIV_STATUS &stDevStatus);
    long GetErr(BYTE &iDeviceNo, BYTE &bErrorCode);
    long GetClapStatus(USHORT &usInhibit);
    long GetCoinPosition(ST_COIN_POSITION &stCoinPosition);
    long GetCoinCylinderInfo(int iCylinderNo, ST_COINCYLINDER_INFO &CoinCylinderInfo);
    long PurgeHopper(BYTE bCylinderNO, BYTE bCount);
    long DispensePart(int iCylinderNo, ULONG RequestItems, ULONG &DispensedItems, DEV_CUERROR &arywCUError);
    long FWLoadStart();
    long FWLoadSendData(BYTE uclock, BYTE ucLine, const BYTE *szData, BYTE ucDataLen);
    long FWLoadEnd();

    int SplitMultipleItems(LPCSTR pValue, CMultiString &ms, std::string separator = SEP_STR_COMMA);
    int MapHopperNo2Index(int iHopperNo);

private:
    CAutoEvent m_SemCancel;
    USHORT m_lastStatusError;
    BOOL m_bIsOpen;
    char m_pszMode[32];
    int32_t m_iHopperNo[MAX_COINCYLINDER_NUM];
};
#endif  // CORDEVICE_H
