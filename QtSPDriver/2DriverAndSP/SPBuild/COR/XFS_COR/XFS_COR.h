#ifndef XFS_COR_H
#define XFS_COR_H

#include "IDevCOR.h"
#include "ISPBaseCOR.h"
#include "QtTypeInclude.h"
#include "CORCONFIGPARAM.h"

//////////////////////////////////////////////////////////////////////////
class CXFS_COR : public ICmdFunc, public CLogManage
{
public:
    CXFS_COR();
    virtual ~CXFS_COR();
public:
    // 开始运行SP
    long StartRun();

protected:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    virtual void SetExeFlag(DWORD dwCmd, BOOL bInExe);

    // CDM类型接口
    // 查询命令
    virtual HRESULT CDMGetStatus(LPWFSCDMSTATUS &lpStatus);
    virtual HRESULT CDMGetCapabilities(LPWFSCDMCAPS &lpCaps);
    virtual HRESULT CDMGetCashUnitInfo(LPWFSCDMCUINFO &lpCUInfor);
    virtual HRESULT CDMGetMixType(LPWFSCDMMIXTYPE *&lppMixType);
    virtual HRESULT CDMGetMixTable(USHORT usMixNumber, LPWFSCDMMIXTABLE &lpMixTable);
    virtual HRESULT CDMGetPresentStatus(WORD wPos, LPWFSCDMPRESENTSTATUS &lpPresentStatus);
    virtual HRESULT CDMGetCurrencyEXP(LPWFSCDMCURRENCYEXP *&lppCurrencyExp);
    // 执行命令
    virtual HRESULT CDMDenominate(const LPWFSCDMDENOMINATE lpDenoData, LPWFSCDMDENOMINATION &pDenoInOut);
    virtual HRESULT CDMDispense(const LPWFSCDMDISPENSE lpDisData, LPWFSCDMDENOMINATION &lpDenoOut);
    virtual HRESULT CDMCount(LPWFSCDMPHYSICALCU lpPhysicalCU, LPWFSCDMCOUNT &lpCount);
    virtual HRESULT CDMPresent(WORD wPos);
    virtual HRESULT CDMReject();
    virtual HRESULT CDMRetract(const WFSCDMRETRACT &stData);
    virtual HRESULT CDMOpenShutter(WORD wPos);
    virtual HRESULT CDMCloseShutter(WORD wPos);
    virtual HRESULT CDMStartEXChange(const LPWFSCDMSTARTEX lpData, LPWFSCDMCUINFO &lpCUInfor);
    virtual HRESULT CDMEndEXChange(const LPWFSCDMCUINFO lpCUInfo);
    virtual HRESULT CDMOpenSafeDoor();
    virtual HRESULT CDMReset(const LPWFSCDMITEMPOSITION lpResetIn);
    virtual HRESULT CDMSetCashUnitInfo(const LPWFSCDMCUINFO lpCUInfo);
    virtual HRESULT CDMTestCashUnits(LPWFSCDMITEMPOSITION lpPosition, LPWFSCDMCUINFO &lpCUInfo);
    virtual HRESULT CDMSetGuidanceLight(LPWFSCDMSETGUIDLIGHT lpSetGuidLight);

    // CIM类型接口
    // 查询命令
    virtual HRESULT CIMGetStatus(LPWFSCIMSTATUS &lpStatus);
    virtual HRESULT CIMGetCapabilities(LPWFSCIMCAPS &lpCaps);
    virtual HRESULT CIMGetCashUnitInfo(LPWFSCIMCASHINFO &lpCUInfo);
    virtual HRESULT CIMGetBankNoteType(LPWFSCIMNOTETYPELIST &lpNoteList);
    virtual HRESULT CIMGetCashInStatus(LPWFSCIMCASHINSTATUS &lpCashInStatus);
    virtual HRESULT CIMGetCurrencyEXP(LPWFSCIMCURRENCYEXP *&lppCurrencyExp);
    // 执行命令
    virtual HRESULT CIMCashInStart(const LPWFSCIMCASHINSTART lpData);
    virtual HRESULT CIMCashIn(LPWFSCIMNOTENUMBERLIST &lpResult);
    virtual HRESULT CIMCashInEnd(LPWFSCIMCASHINFO &lpCUInfo);
    virtual HRESULT CIMCashInRollBack(LPWFSCIMCASHINFO &lpCUInfo);
    virtual HRESULT CIMRetract(const LPWFSCIMRETRACT lpData);
    virtual HRESULT CIMOpenShutter(WORD wPos);
    virtual HRESULT CIMCloseShutter(WORD wPos);
    virtual HRESULT CIMStartEXChange(const LPWFSCIMSTARTEX lpStartEx, LPWFSCIMCASHINFO &lpCUInfor);
    virtual HRESULT CIMEndEXChange(const LPWFSCIMCASHINFO lpCUInfo);
    virtual HRESULT CIMOpenSafeDoor();
    virtual HRESULT CIMReset(const LPWFSCIMITEMPOSITION lpResetIn);
    virtual HRESULT CIMConfigureCashInUnits(const LPWFSCIMCASHINTYPE *lppCashInType);
    virtual HRESULT CIMConfigureNoteTypes(const LPUSHORT lpusNoteIDs);
    virtual HRESULT CIMSetCashUnitInfo(const LPWFSCIMCASHINFO lpCUInfo);
    virtual HRESULT CIMSetGuidanceLight(const LPWFSCIMSETGUIDLIGHT lpSetGuidLight);
    virtual HRESULT CIMCashInLimit(const LPWFSCIMCASHINLIMIT lpCUInfo);

private:
    HRESULT DoDenomination(USHORT usMixNumber, LPWFSCDMDENOMINATION lpDenomination, LPULONG lpulAmount, LPULONG lpulValues);
    HRESULT EditCDMCashUnitInfo();
    HRESULT EditCIMCashUnitInfo();
    HRESULT UpdateCashUnitCount(LPULONG ulValues, int count, int type);
    HRESULT InitializeCashUnitInfo();
    inline BOOL CheckCashUnitThreshold(int index, ST_COINCYLINDER_INFO& info, int type);
    void CheckCashUnitThreshold(int type);
    void CheckCashUnitInfoChanged();
    void BackupCashUnitInfo();
    long UpdateStatus(const char *ThisModule);
    HRESULT CheckAndLog(BOOL bErrCondition, const char *ThisModule, HRESULT hRes, const char *pLogDesc);

    void CDMFireCashUnitThreshold(const LPWFSCDMCASHUNIT lpCU);
    void CDMFireCUInfoChanged(const LPWFSCDMCASHUNIT lpCU);
    void CDMFireCUError(WORD wFail, const LPWFSCDMCASHUNIT lpCU);

    void CIMFireCashUnitThreshold(const LPWFSCIMCASHIN lpCashIn);
    void CIMFireCUInfoChanged(const LPWFSCIMCASHIN lpCashIn);
    void CIMFireCUError(WORD wFail, const LPWFSCIMCASHIN lpCashIn);

    void EditStatusExtraInfo();
    inline void SetSPErrorInfo(int iErrCode);

private:
    char                            m_szLogType[MAX_PATH];
    CXfsRegValue                    m_cXfsReg;
    std::string                     m_strLogicalName;
    std::string                     m_strSPName;
    CExtraInforHelper               m_cExtra;
    std::string                     m_strConfigFile;        //配置文件名
    BOOL                            m_bCDMExchangeActive;   //CDM是否处于交换钱箱事务中
    BOOL                            m_bCIMExchangeActive;   //CIM是否处于交换钱箱事务中
    BOOL                            m_bCashInActive;        //是否处于存款事务中
    BOOL                            m_bCashOutActive;       //是否处于取款事务中，Dispense、Present置为TRUE、Reset、Retract、Reject、Exchange、取钞后或其他进钞动作置为FALSE
    BOOL                            m_bInCmdExecute;        //是否处于命令执行中，用于设置BUSY状态
    BOOL                            m_bOpened;              //是否正常打开设备
    BOOL                            m_bCUInfoInitialized;   //是否正常初始化钞箱信息(配置文件和硬币机不挥发数据记录一致)
    DWORD                           m_dwCurrentCommand;     //仅当m_bInCmdExecute为TRUE时表示当前正在处理的xfs命令码，否则无意义
    BOOL                            m_bCanceled;            //取消正在執行的命令

    // present status relative.
    WORD                            m_wPresentState;
    WFSCDMPRESENTSTATUS             m_CDMPresentStatus;
    WFSCDMDENOMINATION              m_CDMDenomination;
    ULONG                           m_ulValues[MAX_COINCYLINDER_NUM];       //钞箱别出币个数
    ST_COINCYLINDER_INFO            m_stCoinCylinderInfos[MAX_COINCYLINDER_NUM + 1];

    // Count relative info.
    WFSCDMCOUNT                     m_CDMCount;
    LPWFSCDMCOUNTEDPHYSCU           m_CDMCountedPhysCUPtrArray[MAX_COINCYLINDER_NUM];
    WFSCDMCOUNTEDPHYSCU             m_CDMCountedPhysCUs[MAX_COINCYLINDER_NUM];

    ST_COR_STATUS                   m_stCorStatus;         // Device Status info.
    WFSCDMSTATUS                    m_CDMStatus;
    WFSCIMSTATUS                    m_CIMStatus;
    WFSCDMCAPS                      m_CDMCaps;
    WFSCIMCAPS                      m_CIMCaps;
    LPWFSCDMOUTPOS                  m_CDMOutPosArray[4];
    WFSCDMOUTPOS                    m_CDMOutPos;
    LPWFSCIMINPOS                   m_CIMInPosArray[4];
    WFSCIMINPOS                     m_CIMInPos;

    // cash unit info relative.
    WFSCDMCUINFO                    m_CDMCUInfo;
    LPWFSCDMCASHUNIT                m_lpCDMCashUnitList[MAX_COINCYLINDER_NUM + 1];
    WFSCDMCASHUNIT                  m_CDMCashUnitArray[MAX_COINCYLINDER_NUM + 1];
    LPWFSCDMPHCU                    m_lpCDMPHCashUnitList[MAX_COINCYLINDER_NUM + 1];
    WFSCDMPHCU                      m_CDMPHCashUnitArray[MAX_COINCYLINDER_NUM + 1];
    char                            m_cCashUnitNameArray[MAX_COINCYLINDER_NUM + 1][32];
    char                            m_cPHPositionNameArray[MAX_COINCYLINDER_NUM + 1][32];
    WFSCIMCASHINFO                  m_CIMCashInfo;
    LPWFSCIMCASHIN                  m_CIMCashInPtrList[MAX_COINCYLINDER_NUM + 1];
    WFSCIMCASHIN                    m_CIMCashInArray[MAX_COINCYLINDER_NUM + 1];
    LPWFSCIMPHCU                    m_CIMPHCashUnitPtrList[MAX_COINCYLINDER_NUM + 1];
    WFSCIMPHCU                      m_CIMPHCashUnitArray[MAX_COINCYLINDER_NUM + 1];
    WFSCIMNOTENUMBERLIST            m_CIMNoteNumberListArray[MAX_COINCYLINDER_NUM + 1];
    LPWFSCIMNOTENUMBER              m_CIMNoteNumberPtrArrayArray[MAX_COINCYLINDER_NUM + 1][MAX_SUPP_COIN_TYPE_NUM];
    WFSCIMNOTENUMBER                m_CIMNoteNumberArrayArray[MAX_COINCYLINDER_NUM + 1][MAX_SUPP_COIN_TYPE_NUM];
    char                            m_cNullExtra[4];

    // 钞箱信息备份
    ULONG                           m_ulCountBK[MAX_COINCYLINDER_NUM + 1];
    COIN_UNIT_STATUS                m_CUStatus[MAX_COINCYLINDER_NUM + 1];

    // Cash in relative info.
    USHORT                          m_usCoinEnable;
    WFSCIMNOTENUMBERLIST            m_cashinCIMNoteNumberList;
    LPWFSCIMNOTENUMBER              m_cashinCIMNoteNumberPtrArray[MAX_SUPP_COIN_TYPE_NUM];
    WFSCIMNOTENUMBER                m_cashinCIMNoteNumberArray[MAX_SUPP_COIN_TYPE_NUM];
    ULONG                           m_ulCashInCount[MAX_COINCYLINDER_NUM];
    ST_RETRACTBIN_COUNT             m_stRetractBin;
    WORD                            m_wCashInStatus;
    WFSCIMCASHINSTATUS              m_CIMCashInStatus;
    WFSCIMNOTENUMBERLIST            m_cashinStatusNoteNumberList;
    LPWFSCIMNOTENUMBER              m_cashinStatusNoteNumberPtrArray[MAX_SUPP_COIN_TYPE_NUM];
    WFSCIMNOTENUMBER                m_cashinStatusNoteNumberArray[MAX_SUPP_COIN_TYPE_NUM];

    // cash-in-end command result.
    WFSCIMCASHINFO                  m_endCIMCashInfo;
    LPWFSCIMCASHIN                  m_endCIMCashInPtrList[MAX_COINCYLINDER_NUM + 1];
    WFSCIMCASHIN                    m_endCIMCashInArray[MAX_COINCYLINDER_NUM + 1];
    LPWFSCIMPHCU                    m_endCIMPHCashUnitPtrList[MAX_COINCYLINDER_NUM + 1];
    WFSCIMPHCU                      m_endCIMPHCashUnitArray[MAX_COINCYLINDER_NUM + 1];
    WFSCIMNOTENUMBERLIST            m_endCIMNoteNumberListArray[MAX_COINCYLINDER_NUM + 1];
    LPWFSCIMNOTENUMBER              m_endCIMNoteNumberPtrArrayArray[MAX_COINCYLINDER_NUM + 1][MAX_SUPP_COIN_TYPE_NUM];
    WFSCIMNOTENUMBER                m_endCIMNoteNumberArrayArray[MAX_COINCYLINDER_NUM + 1][MAX_SUPP_COIN_TYPE_NUM];

    // device info.
    char                            m_cFirmwareVer[MAX_LEN_FWVERSION];
    ST_DEV_INFO                     m_stDevInfo;
    char                            m_cErrorCode[16];
    char                            m_cErrorDetail[128];
    char                            m_cStatusExtraInfo[1024];

protected:
    CQtDLLLoader<IDevCOR>           m_pDev;
    CQtDLLLoader<ISPBaseCOR>        m_pBase;
    CORCONFIGPARAM                  m_Param;                //SP配置数据
};

#endif // XFS_COR_H
