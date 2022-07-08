#pragma once
#include "QtTypeDef.h"
#include "UsbDLLDef.h"
#include "ILogWrite.h"
#include <QLibrary>
#include "SimpleMutex.h"

#define IOMC_RESP_DATA_MAX		10
typedef struct _iomc_response_data {	// IOMCレスポンスデータ用構造体（出力）
    WORD	wID;						// 応答ID
    WORD	wLng;						// 自分自身を含む実行結果データの長さ
    BYTE	byDATA[5120];				// 実行結果データ
} IOMCRESPDATA, *LPIOMCRESPDATA;

typedef struct _iomc_response_info {					// IOMCレスポンス用構造体（出力）
    WORD			wLen;								// 電文本体の長さ
    WORD			wRespID;							// 応答ID
    WORD			wRespLng;							// 応答LNG
    WORD			wRespRes;							// 応答結果
    BYTE			byCommDATA[90];						// 共通部
    IOMCRESPDATA	iomcRespData[IOMC_RESP_DATA_MAX];	// 応答データ
} IOMCRESPINFO, *LPIOMCRESPINFO;

typedef struct _iomc_sense_info {		// IOMCセンサ状態情報
    WORD	wLen;						// 電文本体の長さ
    WORD	wRespID;					// 応答ID
    WORD	wRespLng;					// 応答LNG
    WORD	wRespRes;					// 応答結果
    BYTE	byCommDATA[90];				// 共通部(90バイト)
    BYTE	byWarningInfo[124];			// ワーニング情報(124バイト)
    BYTE	bySekkyakuInfo[8];			// 接客センス情報(8バイト)
    BYTE	byMCPullOutInfo[6];			// MC引出しセンス情報(6バイト)
    BYTE	byAppendInfo[12];			// 付加センス情報(12バイト)
} IOMCSNSINFO, *LPIOMCSNSINFO;
//////////////////////////////////////////////////////////////////////////
//USB驱动函数定义
typedef unsigned int(*HT_FnATMUSB)(unsigned int uifncNo, PSTR_DRV pstrDrvDllPtr);   // 收发USB数据
typedef unsigned int(*HT_InfATMUSB)(unsigned int uifncNo, PSTR_DRV pstrDrvDllPtr);  // 打开关闭USB驱动连接，查询Sensor状态
//////////////////////////////////////////////////////////////////////////
class CUSBDrive: public CLogManage
{
public:
    CUSBDrive(LPCSTR lpDevType);
    ~CUSBDrive();
public:
    // 加载USB底层通讯库
    long USBDllLoad();
    // 卸载USB底层驱动库
    void USBDllRelease();
    // 打开连接
    long USBOpen(const char *pDevName);
    // 关闭连接
    long USBClose();
    // 是否已打开连接
    bool IsOpen();
    // USB命令调用
    long USBDrvCall(WORD wCmd, PSTR_DRV pParam);
    // 设置USB库接口
    void SetFnATMUSB(HT_FnATMUSB pFnATMUSB, HT_InfATMUSB pInfATMUSB);
    // 获取接口指针
    HT_FnATMUSB GetFnATMUSB();
    HT_InfATMUSB GetInfATMUSB();
protected:
    // 错误描述
    LPCSTR GetErrDesc(ULONG ulErrCode);
private:
    char            m_szDesc[256];
    char            m_szDevName[128];
    UINT            m_ulUSBHandle;
    QLibrary        m_cUsbDll;
    HT_FnATMUSB     m_pFnATMUSB;
    HT_InfATMUSB    m_pInfATMUSB;
    CQtSimpleMutexEx m_cSysMutex;
};

