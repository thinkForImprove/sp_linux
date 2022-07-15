#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

#include "IDevBCR.h"

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义

//----------宏定义----------------------------------------------------

static const BYTE byXFSVRTU[17] = {"BCR00020100"};          // XFS_BCR 版本号
static const BYTE byDevBCRVRTU[17] = {"DevBCR020100"};      // DevBCR 版本号

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define SET_DEV_PREIC           51              // 进卡检查模式

//-------------------------------读卡器模块(IDC)相关声明-------------------------------------
// 设备类型
#define XFS_NT0861              2               // 牛图(NT0861)

#define IDEV_NT0861_STR         "NT0861"        // 牛图(NT0861)

// INI配置变量结构体
typedef
struct st_idc_ini_config
{
    CHAR                szDevDllNameBCR[256];               // DevBCR动态库名
    WORD                wDeviceType;                        // 设备类型
    STDEVICEOPENMODE    stDevOpenMode;                      // 设备打开模式
    CHAR                szSDKPath[256];                     // 设备SDK库路径
    WORD                wDistSymModeSup;                    // 设备是否支持识别条码类型
    // INI配置Open相关
    WORD                wOpenFailRet;                       // Open失败时返回值
    // INI配置复位相关
    WORD                wResetFailReturn;                   // Reset失败时返回标准
    // INI配置通用相关
    WORD                wReadBcrRetDataMode;                // 扫码读码返回数据格式
    // INI配置测试模式相关
    WORD                wTestModeIsSup;                     // 是否启用测试模式
    INT                 nTestInsertComplete;                //
    // 其他

    st_idc_ini_config()
    {
        Clear();
    }

    void Clear()
    {
        memset(this, 0x00, sizeof(st_idc_ini_config));
    }
} STINICONFIG, LPSTINICONFIG;


//-------------------------------退卡模块(CRM)相关声明-------------------------------------
// CRM设备类型
#define DEV_CRM_CRT730B         0       // 退卡模块:CRT-730B

// CRM设备类型(DevCRM.cpp区分不同类型使用)
#define IDEV_CRM_CRT730B       "CRT-730B"     // 退卡模块:CRT-730B

// INI中指定[int型]设备类型 转换为 STR型
#define CRM_DEVTYPE2STR(n) \
    (n == DEV_CRM_CRT730B ? IDEV_CRM_CRT730B : "")

// INI配置变量结构体
typedef
struct st_crm_ini_config
{
    CHAR        szDevDllNameCRM[256];       // 退卡模块动态库名
    BOOL        bCRMDeviceSup;              // 是否支持启动退卡模块,缺省F
    WORD        wCRMDeviceType;             // 退卡模块设备类型,缺省0(CRT-730B)
    CHAR        szCRMDeviceConList[24];     // 退卡模块连接字符串
    WORD        wDeviceInitAction;          // 设备初始化动作,缺省3
    WORD        wEjectCardPos;              // 退卡后卡位置，缺省0
    WORD        wEnterCardRetainSup;        // 卡在入口是否支持CMRetain,缺省0
    CHAR        szPlaceHolder[2];           // 占位符
    WORD        wEmptyAllCard0;             // 是否支持CMEmptyCard入参为0时回收所有卡
    CHAR        szSlotHaveCard[3];          // 卡槽有卡状态标记(仅限2位),缺省01
    CHAR        szSlotNoHaveCard[3];        // 卡槽无卡状态标记(仅限2位),缺省00
    WORD        wOpenFailRet;               // Open失败时返回值
    WORD        wCMEjectMode;               // 退卡模式(CMEject命令)

    st_crm_ini_config()
    {
        Clear();
    }

    void Clear()
    {
        memset(this, 0x00, sizeof(st_crm_ini_config));
        bCRMDeviceSup = FALSE;
        wDeviceInitAction = 3;
    }
} STCRMINICONFIG, LPSTCRMINICONFIG;




#endif // DEF_H
