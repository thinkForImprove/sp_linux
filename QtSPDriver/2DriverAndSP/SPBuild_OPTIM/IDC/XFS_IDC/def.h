#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

#include "IDevIDC.h"

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义

//----------宏定义----------------------------------------------------

static const BYTE byXFSVRTU[17] = {"IDC00020100"};          // XFS_IDC 版本号
static const BYTE byDevIDCVRTU[17] = {"DevIDC020100"};      // DevIDC 版本号
static const BYTE byDevCRMVRTU[17] = {"DevCRM0210100"};      // DevCRM 版本号

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define SET_DEV_PREIC           51              // 进卡检查模式
#define SET_DEV_RETAINCNT       52              // 设置硬件回收计数
#define SET_INCARD_PARAM        53              // 设置进卡参数
#define SET_DEV_AUXPARAM        54              // 设置设备辅助参数
#define SET_DEV_WOBBLE_OPEN     55              // 设置开启抖动进卡支持
#define SET_DEV_WOBBLE_CLOSE    56              // 设置关闭抖动进卡支持
#define GET_DEV_FRAUDDETE       57              // 防逗卡保护是否生效中
#define SET_SND_NOTICE_2DEV     58              // 发送通知消息到DevXXX

#define SET_CLEAR_SLOT          101             // 清除卡槽信息
#define CARDNO_ISEXIST          102             // 检查卡号是否已存在
#define SLOTNO_ISEXIST          103             // 检查暂存仓是否有卡/被占用


// GetVersion()使用
#define GET_VER_DEVRPR          1               // DevRPR版本号
#define GET_VER_FW              2               // 固件版本号

//-------------------------------读卡器模块(IDC)相关声明-------------------------------------
// 设备类型
#define XFS_EC2G                0               // EC2G
#define XFS_CRT350N             1               // CRT-350N

#define IDEV_EC2G_STR           "EC2G"          //
#define IDEV_CRT350N_STR        "CRT-350N"      //

// INI配置变量结构体
typedef
struct st_idc_ini_config
{
    CHAR                szDevDllNameIDC[256];               // DevIDC动态库名
    CHAR                szDevDllNameSIU[256];               // DevSIU动态库名
    WORD                wDeviceType;                        // 设备类型
    STDEVICEOPENMODE    stDevOpenMode;                      // 设备打开模式
    CHAR                szSDKPath[256];                     // 设备SDK库路径
    // INI配置Open相关
    WORD                wOpenFailRet;                       // Open失败时返回值
    WORD                wOpenResetCardAction;               // Open后复位卡动作
    // INI配置复位相关
    WORD                wResetCardAction;                   // Reset时卡动作
    WORD                wResetFailReturn;                   // Reset失败时返回标准
    // INI配置回收相关
    WORD                wRetainSupp;                        // 是否支持回收功能
    WORD                wRetainCardCount;                   // 吞卡计数
    WORD                wRetainThreshold;                   // 回收将满报警阀值
    WORD                wRetainFull;                        // 回收满阀值
    // INI配置读卡器相关
    WORD                wCanWriteTrack;                     // 是否支持写磁
    WORD                wFluxSensorSupp;                    // 磁通感应器是否可用
    WORD                wRWAccFollowingEject;               // 退卡到出口后是否支持重新吸入读写
    WORD                wNeedWobble;                        // 是否需要支持抖动功能
    DWORD               dwInCardTimeOut;                    // 进卡超时时间
    WORD                wPostRemovedAftEjectFixed;          // 退卡时无卡是否报MediaRemoved事件
    WORD                wAcceptWhenCardFull;                // 回收盒满是否支持进卡
    WORD                wFluxInActive;                      // 卡时是否有磁道
    WORD                wSupportPredictIC;                  // 进卡检查模式
    WORD                wTamperSupp;                        // 防盗钩功能是否支持
    WORD                wAfterInCardOpen;                   // 吸卡时后出口进卡处理
    // INI配置异物检知相关
    WORD                wSkimmingSupp;                      // 是否支持异物检知
    WORD                wSkimmingMonitorInterval;           // 检测间隔时间
    WORD                wSkimmingErrorDuration;             // 判故障持续故障时间
    WORD                wSkimmingNormalDuration;            // 判正常持续正常时间
    // INI欺诈检测相关配置
    WORD                wTeaseCardProtectSupp;              // 防逗卡功能是否支持
    WORD                wTeaseInCardCount;                  // 防逗卡保护生效的进卡次数上限
    INT                 wTeaseCardProtectDate;              // 防逗卡保护生效后持续时间
    // INI配置测试模式相关
    WORD                wTestModeIsSup;                     // 是否启用测试模式
    INT                 nTestInsertComplete;                // 读卡数据完成后返回先返Insert事件,再返Complete
    // 其他
    INT                 nDevAuxParam[64];                   // 设备辅助参数

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
