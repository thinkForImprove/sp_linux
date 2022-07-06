#pragma once
/***************************************************************
* 文件名称：IDevCRM.h
* 文件描述：声明退卡模块底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include <string.h>
//////////////////////////////////////////////////////////////////////////
#if defined(IDEVCRM_LIBRARY)
#define DEVCRM_EXPORT     Q_DECL_EXPORT
#else
#define DEVCRM_EXPORT     Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////

//成功
#define CRM_SUCCESS             (0)     // 操作成功
//警告
#define ERR_CRM_INSERT_TIMEOUT  (1)     // 进卡超时
#define ERR_CRM_USER_CANCEL     (2)     // 用户取消
//错误
#define ERR_CRM_COMM_ERR        (-1)    // 通讯错误
#define ERR_CRM_JAMMED          (-2)    // 堵卡
#define ERR_CRM_OFFLINE         (-3)    // 脱机
#define ERR_CRM_NOT_OPEN        (-4)    // 没有打开
#define ERR_CRM_SLOT_FULL       (-5)    // 卡箱满
#define ERR_CRM_HWERR           (-6)    // 硬件故障
#define ERR_CRM_STATUS_ERR      (-7)    // 状态出错
#define ERR_CRM_SLOT_ISEXIST    (-8)    // 指定卡箱被占用
#define ERR_CRM_SLOT_NOTEXIST   (-9)    // 指定卡箱没有被占用
#define ERR_CRM_UNSUP_CMD       (-31)   // 不支持的指令
#define ERR_CRM_PARAM_ERR       (-32)   // 参数错误
#define ERR_CRM_READTIMEOUT     (-33)   // 读数据超时
#define ERR_CRM_WRITETIMEOUT    (-34)   // 写数据超时
#define ERR_CRM_READERROR       (-35)   // 读数据错
#define ERR_CRM_WRITEERROR      (-36)   // 写数据错
#define ERR_CRM_CARD_NOTFOUND   (-37)   // 指定卡不存在
#define ERR_CRM_CARD_ISEXIST    (-38)   // 指定卡已存在
#define ERR_CRM_LOAD_LIB        (-39)   // 动态库错误
#define ERR_CRM_OTHER           (-100)  // 其他错误/未知错误


//-----------------------------------------------------------------------
// CRM 版本号
static const BYTE    byDevVRTU[17] = {"DevCRM00000100"};

// CRM设备类型
#define CRM_DEV_CRT730B         0       // 退卡模块:CRT-730B

// CRM设备类型(DevCRM.cpp区分不同类型使用)
#define ICRM_TYPE_CRT730B       "0"     // 退卡模块:CRT-730B

// SetData/GetData 数据类型
#define DATATYPE_INIT           0       // 初始化数据
#define CARDNO_ISEXIST          1       // 检查卡号是否已存在
#define SLOTNO_ISEXIST          2       // 检查暂存仓是否有卡/被占用

// 退卡模块(CRM)状态
typedef
struct st_device_crm_status
{
    CHAR    szSlotCard[5][128+1];   // 暂存箱(卡槽)1/2/3/4/5卡号
    WORD    wStorageCount;          // 暂存卡数目
    WORD    wSensorStat[5];         // 传感器1/2/3/4/5状态(0无遮挡/1有遮挡)
    WORD    wVertPowerStat;         // 垂直电机状态(0无故障/1有故障)
    WORD    wHoriPowerStat;         // 水平电机状态(0无故障/1有故障)
    WORD    wDeviceStat;            // 设备状态(0/繁忙;1正常;2不在线;3硬件故障;4卡槽满;
                                    //         5卡槽空;6读卡器故障;7读卡器和模块都不正常;8可复位)
    st_device_crm_status()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_device_crm_status));
        memset(szSlotCard, 0x00, sizeof(szSlotCard));
        wDeviceStat = 1;
    }
}STCRMSTATUS, LPSTCRMSTATUS;

// 退卡模块(CRM)暂存仓卡信息
typedef
struct st_device_crm_slot_info
{
    BOOL    bSlotHave[5];           // 暂存箱(卡槽)1/2/3/4/5是否有卡
    CHAR    szSlotCard[5][128+1];   // 暂存箱(卡槽)1/2/3/4/5卡号
    CHAR    szStorageTime[5][14+1]; // 暂存卡时间(YYYYMMDDHHmmss)

    st_device_crm_slot_info()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_device_crm_slot_info));
    }
}STCRMSLOTINFO, LPSTCRMSLOTINFO;


//-----------------------------------------------------------------------
//定义卡动作
enum CRMInitAction
{
    CRMINIT_HOMING   = 0,   // 正常归位
    CRMINIT_EJECT    = 1,   // 强行退卡
    CRMINIT_STORAGE  = 2,   // 强行暂存
    CRMINIT_NOACTION = 4,   // 无动作
};

//////////////////////////////////////////////////////////////////////////
//接口类定义
struct  IDevCRM
{
    /************************************************************
    ** 功能：释放端口
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual void Release() = 0;

    /************************************************************
    ** 功能：打开与设备的连接
    ** 输入：pMode: 自定义OPEN参数字串,
            串口： 格式为："COMX: BaudRate,Parity,DataBits,StopBits"(例如："COM2:115200,N,8,1")
            USB:  格式自定义(USB:VID,PID)
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Open(const char *pMode) = 0;

    /************************************************************
    ** 功能：设备初始化
    ** 输入: eActFlag : 卡动作
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Init(CRMInitAction eActFlag) = 0;

    /************************************************************
    ** 功能：关闭与设备的连接
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Close() = 0;

    /************************************************************
    ** 功能：设备复位
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Reset() = 0;

    /************************************************************
    ** 功能：读取设备信息
    ** 输入：无
    ** 输出：pInfo　设备信息
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetDevInfo(char *pInfo) = 0;

    /************************************************************
    ** 功能：读取暂存仓信息
    ** 输入：无
    ** 输出：pInfo　设备信息
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetCardSlotInfo(STCRMSLOTINFO &stInfo) = 0;

    /************************************************************
    ** 功能：读取设备状态
    ** 输入：无
    ** 输出：stStatus 设备状态
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetStatus(STCRMSTATUS &stStatus) = 0;

    /************************************************************
    ** 功能：指定卡号退卡
    ** 输入：szCardNo 入参　卡号
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CMEjectCard(const char *szCardNo) = 0;

    /************************************************************
    ** 功能：指定暂存仓编号退卡
    ** 输入：nSlotNo 入参　卡号
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CMEjectCard(const int nSlotNo) = 0;

    /************************************************************
    ** 功能：指定卡号和暂存仓存卡
    ** 输入：szCardNo 入参　卡号
    **      nSlotNo  入参  暂存仓编号(1-5)
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CMRetainCard(const char *szCardNo, const int nSlotNo) = 0;

    /************************************************************
    ** 功能：移动暂存仓
    ** 输入：nMode 入参 （0暂存仓回到初始位置;1指定第1个空置暂存仓对准卡口;
    **                  2指定暂存仓编号对准卡口)
    **      nSlotNo 入参 暂存仓编号(1~5),nMode=2时有效
    ** 输出：nSlotNo 回餐 nMode=1时,返回空置暂存仓编号
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CMCardSlotMove(const int nMode, int &nSlotNo) = 0;

    /************************************************************
    ** 功能：设备复位
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CMReset() = 0;

    /************************************************************
    ** 功能：设置数据
    ** 输入：vData 入参
    **      wDataType 入参 设置类型(参考宏定义)
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int SetData(void *vData, WORD wDataType = 0) = 0;

    /************************************************************
    ** 功能：获取数据
    ** 输入：wDataType 入参 获取类型(参考宏定义)
    ** 输出：vData 回参 数据
    **
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetData(void *vData, WORD wDataType = 0) = 0;

    // (1DevCam版本/2固件版本/3设备软件版本/4其他)
    /************************************************************
    ** 功能：获取版本号
    ** 输入：wType 入参 获取类型(1DevCRM版本/2固件版本/3设备软件版本/4其他)
    ** 输出：szVer 回参 数据
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual void GetVersion(char* szVer, long lSize, WORD wType) = 0;
};

extern "C" DEVCRM_EXPORT long CreateIDevCRM(LPCSTR lpDevType, IDevCRM *&pDev);
