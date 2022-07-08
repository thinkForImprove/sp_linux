//BRMCONFIGPARAM.h
#ifndef BRM_CONFIG_PARAM_H
#define BRM_CONFIG_PARAM_H

#include "XFSCIM.H"
#include "XFSCDM.H"
#include <string>
using namespace std;
#include "SPConfigFile.h"
#include "MultiString.h"
#include "ILogWrite.h"

//GetCfgNoteTypeCantDepositMode返回码判断宏
#define GCNTCDM_DEPOSIT_ALLOWED(m)      ((m) == 0)  //允许进该币种
#define GCNTCDM_DEPOSIT_NOT_ALLOWED(m)  ((m) >= 1)  //不允许进该币种
#define GCNTCDM_GET_OLD_STATUS(m)       ((m) < 2)//给应用原状态？

// 所有相关的配置参数
typedef struct _tag_brm_config_param : public CLogManage
{
    LPWFSCDMCURRENCYEXP *GetCurrencyExp() const
    {
        return lppCurrencyExp;
    }

    // 设备验钞模块对钞票币种的识别配置
    LPWFSCIMNOTETYPELIST GetNoteTypeList() const
    {
        return lpNoteTypeList;
    }

    //得到加钞后是否自动复位
    BOOL IsExchangeReset() const
    {
        return bExchangeReset;
    }

    //空闲时TS中有钞票机芯状态是否置故障
    BOOL ErrWhenStackNotEmpty() const
    {
        return bErrStackNotEmpty;
    }

    //得到回收张数的计数方式,0--按张数；1---按回收次数
    int GetRetractCountMode() const
    {
        return bRetractCountMode;
    }

    //得到自动更新状态的时间间隔, 缺省为0，不自动更新
    DWORD GetUpdateInterval() const
    {
        return dwUpdateInterval;
    }

    //得到自动修复的时间间隔，缺省为0，不自动修复
    DWORD GetAutoRecoverInterval() const
    {
        return dwAutoRecoverInterval;
    }

    //是否OEPN时复位
    DWORD GetResetOnOpen() const
    {
        return bResetOnOpen;
    }

    //下载固件时是否需要异步执行
    DWORD GetAsynDownLoadFW() const
    {
        return bAsynDownLoadFW;
    }

    DWORD GetMixEqualEmptyMode() const
    {
        return dwMixEqualEmptyMode;
    }
    //得到自动修复次数，缺省为1，只修复一次
    DWORD GetAutoRecoverNumber() const
    {
        return dwAutoRecoverNumber;
    }
    //得到适配器DLL名
    string GetAdapterDLLName() const
    {
        return sAdapterDLLName;
    }
    //得到FL DLL名
    string GetFlDLLName() const
    {
        return sFlDLLName;
    }

    //END_EXCHANGE设置钞箱当前张数为ulCount=0时，其他ulCount是否自动设置为0
    BOOL IsEndExchangeAutoClearCount() const
    {
        return bEndExchangeAutoClearCount;
    }

    //CIM模块进钞(或循环)箱全满时fwAcceptor状态是否为ACCCUSTOP
    BOOL IsCashInBoxFullAccptStop() const
    {
        return bCashInBoxFullAccptStop;
    }

    //当CASH_IN_STATUS的wState为WFS_CIM_CIROLLBACK或WFS_CIM_CIACTIVE时RESET是否设置为WFS_CIM_CIRETRACT
    BOOL IsSetCIRetractAfterReset() const
    {
        return bSetCIRetractAfterReset;
    }

    BOOL IsBeepWhenBoxInsertedOrTaken() const
    {
        return bBeepWhenBoxInsertedOrTaken;
    }

    //当STACKER、通道、门口不为空或SHUTTER打开时是否停止存取款。
    //0：不停止；1：停止并且Dispenser和Acceptor状态设置为STOP；其他：停止但不影响状态
    int GetStopWhenStackerNotEmpty() const
    {
        return dwStopWhenStackerNotEmpty;
    }
    int GetStopWhenTransportNotEmpty() const
    {
        return dwStopWhenTransportNotEmpty;
    }
    int GetStopWhenPositionNotEmpty() const
    {
        return dwStopWhenPositionNotEmpty;
    }
    int GetStopWhenShutterAbnormal() const
    {
        return dwStopWhenShutterAbnormal;
    }
    int GetStopWhenSafeDoorOpen() const
    {
        return dwStopWhenSafeDoorOpen;
    }

    DWORD GetCashUnitType() const
    {
        return dwCashUnitType;
    }

    DWORD GetCassOperWhenSafeDoorOpen() const
    {
        return dwCassClosedWhenSafeDoorOpen;
    }

    DWORD GetCassOperWhenStatusChangeWaitTime() const
    {
        return dwCassOperWhenStatusChangeWaitTime;
    }
    //当回收位置为REJECT是否将其按Retract箱处理回收
    BOOL IsGenerRejectPosbyRetractCass() const
    {
        return bGenerRejectPosbyRetractCass;
    }

    //当进钞事务激活时是否CDM设备状态设置为BUSY。0 不设置；1 设置
    BOOL IsCDMStatusBusyWhenCashInActive() const
    {
        return bCDMStatusBusyWhenCashInActive;
    }

    //显式控制门时CashIn检查门是否关闭：1，检查；0，不检查
    BOOL IsCheckDoorCloseBeforeCashIn() const
    {
        return bCheckDoorAbnormalBeforeCashIn;
    }

    //CIM配置的币种与实际设备可处理的币种矛盾时的处理方式：
    //0，给应用OK，允许进；1，给应用的为OK，不许进；其他，给应用NO OK，不许进
    DWORD GetCfgNoteTypeCantDepositMode() const
    {
        return dwCfgNoteTypeCantDepositMode;
    }

    //取钞后报JAMMED前重试关门次数: -1，不限定次数
    DWORD GetCloseShutterCountBeforeJammed() const
    {
        return dwCloseShutterCountBeforeJammed;
    }

    //RollBack时是否检查钞口和暂存区为空：1：检查；0：不检查
    BOOL IsCheckCSTSEmptyWhenRollBack() const
    {
        return bCheckCSTSEmptyWhenRollBack;
    }

    //Retract前是否检查钞口为空：1：检查；0：不检查
    BOOL IsCheckCSEmptyBeforeRetract() const
    {
        return bCheckCSEmptyBeforeRetract;
    }

    //Reject前是否检查钞口为空：1：检查；0：不检查
    BOOL IsCheckTSEmptyBeforeReject() const
    {
        return bCheckTSEmptyBeforeReject;
    }

    //在钞票处理过程中是否获取序列号(0:不获取; 1:获取)
    BOOL GetInfoAvailableAfterCashOut() const
    {
        return bGetInfoAvailableAfterCashOut;
    }
    BOOL GetInfoAvailableAfterRetractCount() const
    {
        return bGetInfoAvailableAfterRetractCount;
    }
    BOOL GetInfoAvailableAfterCashIn() const
    {
        return bGetInfoAvailableAfterCashIn;
    }
    BOOL GetInfoAvailableAfterCashInEnd() const
    {
        return bGetInfoAvailableAfterCashInEnd;
    }

    //是否支持非法索引的回收箱索引：1，支持，如果应用传的是非法索引，则由SP决定回收位置；
    //      0，不支持，如果传0则报错
    BOOL IsSupportIllegalRetractIndex() const
    {
        return bIsSupportIllegalRetractIndex;
    }

    //是否支持状态改变侦听: 1,支持状态侦听；0，不支持状态侦听
    BOOL IsSupportStatusChangeListener() const
    {
        return bIsSupportStatusChangeListener;
    }

    //如何使用CashInStart中UseRecycleUnits标示。0，正常模式，根据应用的标志来决定是否使用循环箱；
    //       1，忽略标志，总是使用循环箱；2或其他，忽略标志，总是禁用循环箱
    DWORD GetModeOfUseRecycleUnitsOfCashInStart() const
    {
        return dwModeOfUseRecycleUnitsOfCashInStart;
    }

    //Item taken wait time
    DWORD GetRollbackWaitItemtakenTime() const
    {
        return dwRollbackWaitItemtakenTime;
    }

    BOOL GetEPDownAutoReset() const
    {
        return bEPDownAutoReset;
    }

    ULONG GetSubDispsenseCount() const
    {
        return ulSubDispsenseCount;
    }

    //从配置文件装入参数，先看ETCDIR，再看工作目录
    //lpszFileName: 不带路径的文件名
    int LoadParam(LPCSTR lpszFileName);

    //保存钞票类型列表
    void SaveNoteTypeList();

    //释放使用过程的分配的内存
    void Clear();

    //构造函数，清空数据
    _tag_brm_config_param()
    {
        lppCurrencyExp = NULL;
        lpNoteTypeList = NULL;
        bExchangeReset = FALSE;
        dwUpdateInterval = 0;
        dwAutoRecoverInterval = 0;
        dwAutoRecoverNumber = 1;
        bErrStackNotEmpty = FALSE;
        bRetractCountMode = FALSE;
        bEndExchangeAutoClearCount = FALSE;
        bCashInBoxFullAccptStop = TRUE;
        bSetCIRetractAfterReset = FALSE;
        bBeepWhenBoxInsertedOrTaken = FALSE;
        dwStopWhenStackerNotEmpty = 0;
        dwStopWhenTransportNotEmpty = 0;
        dwStopWhenPositionNotEmpty = 0;
        dwStopWhenShutterAbnormal = 0;
        dwStopWhenSafeDoorOpen = 0;
        bCDMStatusBusyWhenCashInActive = FALSE;
        bCheckDoorAbnormalBeforeCashIn = FALSE;
        dwCfgNoteTypeCantDepositMode = 0;
        dwCloseShutterCountBeforeJammed = 0;
        bGetInfoAvailableAfterCashOut = FALSE;
        bGetInfoAvailableAfterRetractCount = FALSE;
        bGetInfoAvailableAfterCashIn = FALSE;
        bGetInfoAvailableAfterCashInEnd = FALSE;
        bCheckCSTSEmptyWhenRollBack = TRUE;
        bCheckCSEmptyBeforeRetract  = TRUE;
        bCheckTSEmptyBeforeReject  = TRUE;
        bIsSupportIllegalRetractIndex = FALSE;
        bIsSupportStatusChangeListener = TRUE;
        dwModeOfUseRecycleUnitsOfCashInStart = 0;
        dwCashUnitType = 0;
        dwCassClosedWhenSafeDoorOpen = 0;
        dwCassOperWhenStatusChangeWaitTime = 0;
        bGenerRejectPosbyRetractCass = FALSE;
        bEPDownAutoReset = FALSE;                   //40-00-00-00(FT#0015)
        ulSubDispsenseCount = 100;
        SetLogFile(LOGFILE, "_tag_brm_config_param", "BRM");
    }

    //析构函数，释放使用过程的分配的内存
    virtual ~_tag_brm_config_param()
    {
        Clear();
    }
    //内部成员函数
private:
    //读由多个项目组成的配置项，如“1=USD,2”
    //返回值：读取的项数
    int ReadConfigMultipleItem(LPCSTR pKeyName, LPCSTR pValueName, CMultiString &ms);

    //内部数据
private:
    // 设备支持的所有钞票的信息["币种-指数"]
    LPWFSCDMCURRENCYEXP     *lppCurrencyExp;
    // 设备验钞模块对钞票币种的识别配置
    LPWFSCIMNOTETYPELIST    lpNoteTypeList;
    // 加钞后是否自动复位
    BOOL                    bExchangeReset;
    // 空闲时TS中有钞票机芯状态是否置故障
    BOOL                    bErrStackNotEmpty;
    // 回收张数的计数方式,0--按张数；1---按回收次数
    BOOL                    bRetractCountMode;
    //自动更新状态的时间间隔, 缺省为0，不自动更新
    DWORD                   dwUpdateInterval;
    //自动修复的时间间隔，缺省为0，不自动修复
    DWORD                   dwAutoRecoverInterval;
    //自动修复次数，缺省为1，只修复一次
    DWORD                   dwAutoRecoverNumber;
    //OPEN时是否复位
    BOOL                     bResetOnOpen;
    //下载固件时是否异步执行
    BOOL                     bAsynDownLoadFW;
    //END_EXCHANGE设置钞箱当前张数为ulCount=0时，其他ulCount是否自动设置为0（0：否  1：是）
    BOOL                    bEndExchangeAutoClearCount;

    //等空算法实现方式，0：采用标准方式，1：特殊方式
    DWORD                dwMixEqualEmptyMode;
    //CIM模块进钞(或循环)箱全满时fwAcceptor状态是否为ACCCUSTOP
    BOOL                    bCashInBoxFullAccptStop;

    //当CASH_IN_STATUS的wState为WFS_CIM_CIROLLBACK或WFS_CIM_CIACTIVE时RESET是否设置为WFS_CIM_CIRETRACT
    BOOL                    bSetCIRetractAfterReset;

    //钞箱插入或拔出发出BEEP
    BOOL                    bBeepWhenBoxInsertedOrTaken;

    //显式控制门时CashIn检查门是否异常：1，检查；0，不检查
    BOOL                    bCheckDoorAbnormalBeforeCashIn;

    //CIM配置的币种与实际设备可处理的币种矛盾时的处理方式：
    //0，给应用OK，允许进；1，给应用的为OK，不许进；其他，给应用NO OK，不许进
    DWORD                   dwCfgNoteTypeCantDepositMode;

    //取钞后报JAMMED前重试关门次数: -1，不限定次数
    DWORD                   dwCloseShutterCountBeforeJammed;

    //当STACKER、通道、门口异常、安全门打开或SHUTTER打开时是否停止存取款。
    //0：不停止；1：停止并且Dispenser和Acceptor状态设置为STOP；其他：停止但不影响状态
    DWORD dwStopWhenStackerNotEmpty;
    DWORD dwStopWhenTransportNotEmpty;
    DWORD dwStopWhenPositionNotEmpty;
    DWORD dwStopWhenShutterAbnormal;
    DWORD dwStopWhenSafeDoorOpen;

    DWORD dwRollbackWaitItemtakenTime; //test#24

    //当回收位置为REJECT是否将其按Retract箱处理回收
    BOOL  bGenerRejectPosbyRetractCass;

    //当进钞事务激活时是否CDM设备状态设置为BUSY。0 不设置；1 设置
    BOOL bCDMStatusBusyWhenCashInActive;

    //RollBack时是否检查钞口为空：1：检查；0：不检查
    BOOL bCheckCSTSEmptyWhenRollBack;

    //Retract前是否检查钞口为空：1：检查；0：不检查
    BOOL bCheckCSEmptyBeforeRetract;

    //Reject前是否检查暂存区为空：1：检查；0：不检查
    BOOL bCheckTSEmptyBeforeReject;

    string                  sAdapterDLLName;    //适配器DLL名
    string                  sFlDLLName;         //FL DLL名

    //在钞票处理过程中是否获取序列号(0:不获取; 1:获取)
    BOOL bGetInfoAvailableAfterCashOut;
    BOOL bGetInfoAvailableAfterRetractCount;
    BOOL bGetInfoAvailableAfterCashIn;
    BOOL bGetInfoAvailableAfterCashInEnd;

    //是否支持非法索引的回收箱索引：1，支持，如果应用传的是非法索引，则由SP决定回收位置；0，不支持，如果传0则报错
    BOOL bIsSupportIllegalRetractIndex;

    //是否支持状态改变侦听: 1,支持状态侦听；0，不支持状态侦听
    BOOL bIsSupportStatusChangeListener;

    //如何使用CashInStart中UseRecycleUnits标示。0，正常模式，根据应用的标志来决定是否使用循环箱；
    //       1，忽略标志，总是使用循环箱；2或其他，忽略标志，总是禁用循环箱
    DWORD dwModeOfUseRecycleUnitsOfCashInStart;

    DWORD dwCashUnitType;

    DWORD dwCassClosedWhenSafeDoorOpen;
    DWORD dwCassOperWhenStatusChangeWaitTime;

    BOOL bEPDownAutoReset;              //40-00-00-00(FT#0015)

    ULONG ulSubDispsenseCount;

    CSPConfigFile m_SPConfigFile;   //具体读写配置文件的对象
} BRMCONFIGPARAM, *LPBRMCONFIGPARAM;


#endif //BRM_CONFIG_PARAM_H
