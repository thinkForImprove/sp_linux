
#pragma once
#include "IDevMSR.h"
#include "ISPBaseMSR.h"
#include "IDCXfsHelper.h"
#include "QtTypeInclude.h"

//////////////////////////////////////////////////////////////////////////
#define HEADIMAGE_SAVE_DEF  ""
#define SCANIMAGE_SAVE_DEF  ""

#define DEVTYPE_CHG(a)  a == 0 ? IDEV_TYPE_WBT2172 : (a == 1 ? IDEV_TYPE_WBCS10 :"")

//////////////////////////////////////////////////////////////////////////

//等待用户取卡标志
enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取卡
};

//////////////////////////////////////////////////////////////////////////

// ini获取
typedef
struct st_msr_ini_config
{
    CHAR                 szDevDllName[256];
    STMSRDEVINITPARAM    stMsrInitParamInfo;
    WORD                 wDeviceType;
    WORD                 wOpenResetSupp;
    WORD                 wResetFailReturn;

    st_msr_ini_config()
    {
        clear();
    }
    void clear()
    {
        memset(this, 0x00, sizeof(st_msr_ini_config));
    }
}ST_MSR_INI_CONFIG, *LPST_MSR_INI_CONFIG;



//////////////////////////////////////////////////////////////////////////
class CXFS_MSR : public ICmdFunc, public CLogManage
{
public:
    CXFS_MSR();
    virtual ~CXFS_MSR();
public:
    // 开始运行SP
    long StartRun();

public:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // IDC类型接口
    // 查询命令
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps);
    // 执行命令
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut);
    virtual HRESULT Reset(LPWORD lpResetIn);

protected:
    //  ------处理流程----------------------------
    // 状态初始化
    void InitStatus();
    // 能力值初始化
    void InitCaps();
    // 读卡子处理1
    HRESULT ReadRawDataExecute(DWORD dwReadOption, DWORD dwTimeOut);
    // 读卡子处理2
    long ReadTrackData(DWORD dwReadOption, DWORD dwTimeOut);
    // 读卡子处理３
    void SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData);
    // 复位子处理
    HRESULT ResetExecute();
    // 状态更新子处理
    long UpdateStatus();
    // Device状态转换为WFS标准
    WORD wConvertDeviceStatus(WORD wDevIdxStat);
    // Media状态转换为WFS标准
    WORD wConvertMediaStatus(WORD wDevIdxStat);
    // DevMSR返回值转换为WFS标准
    HRESULT hErrCodeChg(LONG lDevCode);
    // DevMSR返回值转换为明文解释
    LPCSTR GetErrorStr(int nCode);
    // Taken子处理
    long WaitItemTaken();

    // ------事件处理----------------------------
    // 硬件故障事件
    void FireHWEvent(DWORD dwHWAct, char *pErr);
    // 状态变化事件
    void FireStatusChanged(WORD wStatus);
    // 进卡事件
    void FireCardInserted();
    // 退卡/移走卡事件
    void FireMediaRemoved();
    // 复位时检测到卡事件
    void FireMediaDetected(WORD ResetOut);
    // 出现无效磁道事件
    void FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData);

    // ------INI处理----------------------------
    // 读INI
    int InitConfig();

private:
    CQtDLLLoader<IDevMSR>             m_pDev;
    CQtDLLLoader<ISPBaseMSR>          m_pBase;

    ST_MSR_INI_CONFIG                 m_stMsrIniConfig;
    CXfsRegValue                      m_cXfsReg;
    std::string                       m_strLogicalName;
    std::string                       m_strSPName;

    //返回数据
    CWfsIDCStatus                     m_Status, m_OldStatus;
    //WFSIDCCAPS                        m_Caps;
    CWfsIDCCap                        m_Caps;
    CWFSIDCCardDataPtrArray           m_CardDatas;
    CExtraInforHelper                 m_cExtra;
    CSimpleMutex                     *m_pMutexGetStatus;

};


//////////////////////////////////////////////////////////////////////////
