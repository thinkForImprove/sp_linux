/***************************************************************
* 文件名称：XFS_UKEY.h
* 文件描述：UKEN发放模块命令处理接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年8月24日
* 文件版本：1.0.0.1
****************************************************************/

#pragma once

#include "IDevIDC.h"
#include "IDevCRD.h"
#include "ISPBaseIDC.h"
#include "QtTypeInclude.h"
#include "IDCXfsHelper.h"
#include "XFS_UKEY_DEC.h"
#include "def.h"
//#include "IDCForm.h"

#include <unistd.h>
#include <queue>

// SP 版本号
const BYTE byVRTU[24] = {"HWUKEYSTE00000100"};


//-------------------------命令处理类 定义-------------------------
class CXFS_UKEY : public CXFS_CMDFUNC, public CLogManage
{
public:
    CXFS_UKEY();
    virtual ~CXFS_UKEY();
public:
    // 开始运行SP
    long StartRun();
public:
    // --------基本接口--------
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);                                   // Open设备及初始化相关
    virtual HRESULT OnClose();                                                      // 关闭设备
    virtual HRESULT OnStatus();                                                     // 实时状态更新
    virtual HRESULT OnWaitTaken();                                                  // Taken事件处理
    virtual HRESULT OnCancelAsyncRequest();                                         // 命令取消
    virtual HRESULT OnUpdateDevPDL();                                               // 固件升级

    // --------IDC类型接口--------
    // 查询命令
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus);                            // 状态
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps);                          // 能力值
    // 执行命令
    virtual HRESULT EjectCard(DWORD dwTimeOut);                                     // 退UKEY
    virtual HRESULT RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData);               // 吞UKEY
    virtual HRESULT ResetCount();                                                   // 清除吞卡计数
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut);    // 读UKEY编号    
    virtual HRESULT Reset(LPWORD lpResetIn);                                        // 复位

    // --------CRD类型接口--------
    // 查询命令
    virtual HRESULT CRD_GetStatus(LPWFSCRDSTATUS &lpStatus);
    virtual HRESULT CRD_GetCapabilities(LPWFSCRDCAPS &lpCaps);
    virtual HRESULT CRD_GetCardUnitInfo(LPWFSCRDCUINFO &lpCardUnit);
    // 执行命令
    virtual HRESULT CRD_DispenseCard(const LPWFSCRDDISPENSE lpDispense);                // 发卡
    virtual HRESULT CRD_EjecdCard();                                                    // 退卡
    virtual HRESULT CRD_RetainCard(const LPWFSCRDRETAINCARD lpRetainCard);              // 回收卡
    virtual HRESULT CRD_Reset(const LPWFSCRDRESET lpResset);                            // 复位
    virtual HRESULT CRD_SetCardUnitInfo(const LPWFSCRDCUINFO lpCuInfo);                 // 设置卡箱信息

protected:
    HRESULT StartOpen(BOOL bReConn = FALSE);                                            // Open设备及初始化相关子处理
    HRESULT InitOpen();                                                                 // Open设备初始化相关子处理
    int    InitConfig();                                                                // 读INI配置
    void   InitIDCStatus();                                                             // 初始化 IDC Status应答类
    void   InitIDCCaps();                                                               // 初始化 IDC Caps应答类
    void   InitCRDStatus();                                                             // 初始化 CRD Status应答类
    void   InitCRDCaps();                                                               // 初始化 CRD Caps应答类

protected:  // 功能处理
    INT     UpdateDevStatus();                                                          // 状态更新
    HRESULT InnerGetCardUnitInfo(LPWFSCRDCUINFO &lpCardUnit);                           // 取UnitInfo子处理(Info)
    HRESULT InnerSetCardUnitInfo(LPWFSCRDCUINFO lpCuInfo);                              // 设置UnitInfo子处理(Info)
    HRESULT InnerDispenseCard(USHORT usBoxNo, BOOL bPresent);                           // 发卡子处理(CMD)
    HRESULT InnerEjecdCard();                                                           // 弹卡子处理(CMD)
    HRESULT InnerRetainCard(USHORT usBoxNo);                                            // 吞卡子处理(CMD)
    HRESULT InnerReset(USHORT usAction, USHORT usUnitNum);                              // 复位子处理(CMD)
    HRESULT InnerReadRawData(DWORD dwOption, DWORD dwTimeOut);                          // 读UKEY编号(CMD)

protected:
    int  SetDispBoxCfg(USHORT usBoxNum, ULONG ulData, USHORT usType);                   // 设置INI中卡箱单元信息
    int  SetDispBoxCfg(USHORT usBoxNum, LPSTR lpData, USHORT usType);                   // 设置INI中卡箱单元信息
    void CardUnitInfoPack(LPWFSCRDCARDUNIT lpUnit, USHORT usBoxNum);                    // 根据单元索引号赋值信息结构体
    void SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData);            // CWFSIDCCardDataPtrArray类处理

protected:  // (IDC)事件上报相关函数
    void IDC_FireHWEvent(DWORD dwHWAct, char *pErr);
    void IDC_FireStatusChanged(USHORT usStatus);
    void IDC_FireCardInserted();
    void IDC_FireMediaRemoved();
    void IDC_FireMediaRetained();
    void IDC_FireMediaDetected(WORD ResetOut);
    void IDC_FireRetainBinThreshold(USHORT usReBin);
    void IDC_FireInvalidTrackData(USHORT usStatus, LPSTR pTrackName, LPSTR pTrackData);

protected:  // (CRD)事件上报相关函数
    void CRD_FireMediaRemoved();                                                        // 上报 EVENT: WFS_SRVE_CRD_MEDIAREMOVED
    void CRD_FireMediaDetected(LPWFSCRDMEDIADETECTED lpMediaDet);                       // 上报 EVENT: WFS_SRVE_CRD_MEDIADETECTED
    void CRD_FireUnitTheshold(LPWFSCRDCARDUNIT lpCardUnit);                             // 上报 EVENT: WFS_USRE_CRD_CARDUNITTHRESHOLD
    void CRD_FireUnitInfoChanged(LPWFSCRDCARDUNIT lpCardUnit);                          // 上报 EVENT: WFS_SRVE_CRD_CARDUNITINFOCHANGED
    void CRD_FireUnitError(LPWFSCRDCUERROR lpCardUnitError);                            // 上报 EVENT: WFS_EXEE_CRD_CARDUNITERROR
    void CRD_FireDEvicePosition(LPWFSCRDDEVICEPOSITION lpDevicePosition);               // 上报 EVENT: WFS_SRVE_CRD_DEVICEPOSITION

protected: // (CRD)事件上报相关函数封装
    void    CRD_FireUnitThresHold_Pack();                                               // EVENT封装: WFS_USRE_CRD_CARDUNITTHRESHOLD
    void    CRD_FireUnitInfoChanged_Pack();                                             // EVENT封装: WFS_SRVE_CRD_CARDUNITINFOCHANGED
    void    CRD_FireUnitError_Pack(USHORT usBoxNo, LONG lCode);                         // EVENT封装: WFS_EXEE_CRD_CARDUNITERROR

protected: // 类型格式转换
    INT ConvertErrCode(INT nRet);                                                       // IDevCRD错误码转换为WFS格式
    LONG ConvertDeviceStatus(USHORT usDevStat, CONVERTYPE enTYPE = CONVERT_CRD);        // 设备状态转换为WFS格式
    LONG ConvertCRDDispensrStatus(USHORT usStat);                                       // 总单元状态转换为WFS格式
    LONG ConvertCRDTransportStatus(USHORT usStat);                                      // 传送模块状态转换为WFS格式
    LONG ConvertMediaStatus(USHORT usStat, CONVERTYPE enTYPE = CONVERT_CRD);            // 介质状态转换为WFS格式
    LONG ConvertCRDShutterStatus(USHORT usStat);                                        // 门状态转换为WFS格式
    LONG ConvertCRDDevicePosStatus(USHORT usStat);                                      // 指定设备位置转换为WFS格式
    LONG ConvertCRDAntiFraudStatus(USHORT usStat);                                      // 反欺诈模块状态转换为WFS格式
    LONG ConvertDBoxStat2WFS(INT nStat, CONVERTYPE enTYPE = CONVERT_CRD);               // DispBox单元状态转换为WFS格式
    LONG ConvertCRDUnitStat2DBoxStat(USHORT usStat);                                    // CRD定义Unit单元状态转换为DISPBOX结构体状态值转换为WFS格式
    LONG ConvertWfsCRD2IDC(INT nErrCode);                                               // WFS CRD 错误码 转换为 WFS IDC 错误码

private:
    CQtDLLLoader<IDevCRD>           m_pDev;
    CQtDLLLoader<ISPBaseIDC>        m_pBase;
    CXfsRegValue                    m_cXfsReg;
    std::string                     m_strLogicalName;
    std::string                     m_strSPName;
    CSimpleMutex                    *m_pMutexGetStatus;

private:
    STINICONFIG                     m_stConfig;                                         // INI配置保存结构体变量
    CWfsIDCStatus                   m_stIDCStatus;                                      // IDC Status类变量
    CWfsIDCStatus                   m_stIDCStatusOLD;                                   // IDC Status类变量上一次
    CWfsIDCCap                      m_stIDCCaps;                                        // IDC Caps类变量
    CWfsCRDStatus                   m_stCRDStatus;                                      // CRD Status类变量
    CWfsCRDStatus                   m_stCRDStatusOLD;                                   // CRD Status类变量上一次
    CWfsCRDCaps                     m_stCRDCaps;                                        // CRD Caps类变量
    CExtraInforHelper               m_cExtra;                                           // 扩展信息类变量(适用IDC/CRD)
    STDISPBOXLIST                   m_stUKEYBoxList;                                    // UKEY箱信息列表
    STDISPBOXLIST                   m_stUKEYBoxListOld;                                 // UKEY箱信息列表上一次记录
    WAIT_TAKEN_FLAG                 m_enWaitTaken;                                      // 模块Take标记(适用IDC/CRD)
    CWFSIDCCardDataPtrArray         m_clCardData;                                       // 封装的WFSIDCCARDDATA类变量
    INT                             m_nReConRet;                                        // 断线重连记录返回值

    /*BOOL                              m_bNeedRepair;
    DWORD                             m_dwTamperSensorStatus;
    DWORD                             m_nResetFailedTimes;
    IDC_IDCSTAUTS                     m_Cardstatus;
    WAIT_TAKEN_FLAG                   m_WaitTaken;
    BOOL                              m_bJamm;
    BOOL                              m_bICCActived;
    //

    //返回数据
    CWfsIDCStatus                     m_Status;
    WFSIDCCAPS                        m_Caps;
    CWFSIDCCardDataPtrArray           m_CardDatas;
    WFSIDCRETAINCARD                  m_CardRetain;
    CWFSChipIO                        m_ChipIO;
    CMultiMultiString                 m_TrackData;              //最后的磁道数据
    CWFSChipPower                     m_ChipPowerRes;       // chip power返回的ATR数据

    string                            m_strDevName;
    string                            m_strPort;                //30-00-00-00(FT#0019)



private:    // 发卡模块(CRD)相关变量
    STCRDINICONFIG                    m_stCRDINIConfig;     // CRD模块INI
    CQtDLLLoader<IDevCRD>             m_pCRDDev;            // CRD模块连接
    CWfsCRDStatus                     m_CRDStatus;          // CRD模块当前状态(XFS)
    CWfsCRDStatus                     m_CRDStatusOld;       // CRD模块上一次状态(XFS)
    CWfsCRDCaps                       m_CRDCaps;            // CRD模块能力值
    STDISPBOXLIST                     m_stCRDBoxList;       // CRD模块卡箱信息列表
    STDISPBOXLIST                     m_stCRDBoxListOld;    // CRD模块卡箱信息列表上一次记录
    CExtraInforHelper                 m_cCRDExtra;          // CRD模块扩展信息类变量
    WAIT_TAKEN_FLAG                   m_CRDWaitTaken;       // CRD模块Take标记

protected:  // 发卡模块(CRD)相关函数
    void    InitCRDStatus();
    void    InitCRDCaps();
    int     InitCRDConfig();                            // 发卡模块INI配置参数获取
    void    UpdateDevCRDStatus(LPSTCRDDEVSTATUS lpStat = nullptr);// 取CRD状态
    bool    LoadCRDDevDll(LPCSTR ThisModule);           // 加载CRD模块处理动态库
    long    CRDErr2XFSErrCode(INT nCRDErrCode);         // CRD ErrCode 转换为 XFS ErrCode
    long    IDCErr2CRDXFSErrCode(long lCode);           // IDC ErrCode 转换为 CRD XFS ErrCode

*/
};





//////////////////////////////////////////////////////////////////////////
