/***************************************************************
* 文件名称：XFS_MSR.h
* 文件描述：刷折器模块命令处理接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#pragma once
#include "IDevIDC.h"
#include "ISPBaseIDC.h"
#include "IDCXfsHelper.h"
#include "QtTypeInclude.h"
#include "def.h"

#include <unistd.h>

/*************************************************************************
// 结构体 声明
*************************************************************************/
// ini获取
typedef
struct st_msr_ini_config
{
    CHAR                 szDevDllName[256];
    STMSRDEVINITPARAM    stMsrInitParamInfo;
    WORD                 wDeviceType;
    WORD                 wOpenResetSupp;
    WORD                 wResetFailReturn;
    INT                  nOpenFailRet;          // Open失败时返回值
    CHAR                 szSDKPath[256];        // 设备SDK库路径

    st_msr_ini_config()
    {
        clear();
    }
    void clear()
    {
        memset(this, 0x00, sizeof(st_msr_ini_config));
    }
} STINICONFIG, *LPSTINICONFIG;


/*************************************************************************
// 主处理类
*************************************************************************/
class CXFS_MSR : public ICmdFunc, public CLogManage, public ConvertVarIDC
{
public:
    CXFS_MSR();
    virtual ~CXFS_MSR();
public:
    // 开始运行SP
    long StartRun();

public:
    //----------------------------基本接口----------------------------
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

private:    // XFS_MSR_DEC.cpp 命令子处理
    HRESULT InnerOpen(BOOL bReConn = FALSE);                            // Open设备及初始化相关子处理
    int InitConfig();                                                   // 读INI
    void InitStatus();                                                  // 状态初始化
    void InitCaps();                                                    // 能力值初始化
    HRESULT InnerReadRawData(DWORD dwReadOption, DWORD dwTimeOut);      // 读卡子处理1
    void SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData);// 读卡数据结构体变量初始化
    HRESULT InnerReset();                                               // 复位子处理
    long UpdateStatus();                                                // 状态更新子处理
    void UpdateExtra();                                                 // 更新扩展数据

private:    // XFS_MSR_FIRE.cpp 消息事件处理
    void FireHWEvent(DWORD dwHWAct, char *pErr);                        // 硬件故障事件
    void FireStatusChanged(WORD wStatus);                               // 状态变化事件
    void FireCardInserted();                                            // 进卡事件
    void FireMediaRemoved();                                            // 退卡/移走卡事件
    void FireMediaDetected(WORD ResetOut);                              // 复位时检测到卡事件
    void FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData);// 出现无效磁道事件

private:
    CQtDLLLoader<IDevIDC>           m_pDev;
    CQtDLLLoader<ISPBaseIDC>        m_pBase;
    STINICONFIG                  m_stConfig;
    CXfsRegValue                    m_cXfsReg;
    std::string                     m_strLogicalName;
    std::string                     m_strSPName;
    CWfsIDCStatus                   m_stStatus, m_stStatusOLD;          // 返回数据
    //WFSIDCCAPS                      m_Caps;
    CWfsIDCCap                      m_Caps;
    CWFSIDCCardDataPtrArray         m_CardDatas;
    CExtraInforHelper               m_cExtra;
    CSimpleMutex                    *m_pMutexGetStatus;
    INT                             m_nReConErr;                        // 断线重连错误码记录
};

