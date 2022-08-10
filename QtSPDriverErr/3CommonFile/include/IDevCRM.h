
/***************************************************************
* 文件名称：IDevCRM.h
* 文件描述：声明退卡模块底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1
****************************************************************/
#if !defined(SPBuild_OPTIM)     // 非优化工程

#pragma once

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

#else   // 优化工程

#pragma once

#include "XFSIDC.H"
#include "IDevDEF.h"
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


//***************************************************************************
// 使用 IDevDEF.h 通用定义部分
// 1. SetData/GetData 数据类型 (0~50为共通使用, 50以上为各模块自行定义)
// 2. GetVersion 数据类型
// 3. 设备打开方式结构体变量, 适用于 SET_DEV_OPENMODE 参数传递
//***************************************************************************

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
    virtual int Release()
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：打开与设备的连接
    ** 输入：pMode: 自定义OPEN参数字串,
            串口： 格式为："COMX: BaudRate,Parity,DataBits,StopBits"(例如："COM2:115200,N,8,1")
            USB:  格式自定义(USB:VID,PID)
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Open(const char *pMode)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：设备初始化
    ** 输入: eActFlag : 卡动作
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Init(CRMInitAction eActFlag)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：关闭与设备的连接
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Close()
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：设备复位
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Reset()
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：读取设备信息
    ** 输入：无
    ** 输出：pInfo　设备信息
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetDevInfo(char *pInfo)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：读取暂存仓信息
    ** 输入：无
    ** 输出：pInfo　设备信息
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetCardSlotInfo(STCRMSLOTINFO &stInfo)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：读取设备状态
    ** 输入：无
    ** 输出：stStatus 设备状态
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetStatus(STCRMSTATUS &stStatus)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：指定卡号退卡
    ** 输入：szCardNo 入参　卡号
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CMEjectCard(const char *szCardNo)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：指定暂存仓编号退卡
    ** 输入：nSlotNo 入参　卡号
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CMEjectCard(const int nSlotNo)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：指定卡号和暂存仓存卡
    ** 输入：szCardNo 入参　卡号
    **      nSlotNo  入参  暂存仓编号(1-5)
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CMRetainCard(const char *szCardNo, const int nSlotNo)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：移动暂存仓
    ** 输入：nMode 入参 （0暂存仓回到初始位置;1指定第1个空置暂存仓对准卡口;
    **                  2指定暂存仓编号对准卡口)
    **      nSlotNo 入参 暂存仓编号(1~5),nMode=2时有效
    ** 输出：nSlotNo 回餐 nMode=1时,返回空置暂存仓编号
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CMCardSlotMove(const int nMode, int &nSlotNo)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：设备复位
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CMReset()
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：设置数据
    ** 输入：vData 入参
    **      wDataType 入参 设置类型(参考宏定义)
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int SetData(unsigned short usType, void *vData = nullptr)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：获取数据
    ** 输入：wDataType 入参 获取类型(参考宏定义)
    ** 输出：vData 回参 数据
    **
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetData(unsigned short usType, void *vData)
    {
        return ERR_CRM_UNSUP_CMD;
    }

    /************************************************************
    ** 功能：获取版本号
    ** 输入：wType 入参 获取类型
    ** 输出：szVer 回参 数据
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetVersion(unsigned short usType, char* szVer, int nSize)
    {
        return ERR_CRM_UNSUP_CMD;
    }
};

extern "C" DEVCRM_EXPORT long CreateIDevCRM(LPCSTR lpDevType, IDevCRM *&pDev);

//****************************************************************************
// IDevCRM定义 设备相关变量转换 类定义(统一转换)
//****************************************************************************
class ConvertVarCRM
{
private:
    CHAR m_szCRMErrStr[1024];
public:
    // 错误码转换为WFS格式
    LONG CRM_ConvertDevErrCode2WFS(INT nRet)
    {
        switch (nRet)
        {
            case CRM_SUCCESS:                   return WFS_SUCCESS;
            case ERR_CRM_INSERT_TIMEOUT:        return WFS_ERR_TIMEOUT;             // CRM:进卡超时
            case ERR_CRM_USER_CANCEL:           return WFS_ERR_CANCELED;            // CRM:用户取消
            case ERR_CRM_COMM_ERR:              return WFS_ERR_CONNECTION_LOST;     // CRM:通讯错误
            case ERR_CRM_JAMMED:                return WFS_ERR_IDC_MEDIAJAM;        // CRM:堵卡
            case ERR_CRM_OFFLINE:               return WFS_ERR_CONNECTION_LOST;     // CRM:脱机
            case ERR_CRM_NOT_OPEN:              return WFS_ERR_HARDWARE_ERROR;      // CRM:没有打开
            case ERR_CRM_SLOT_FULL:             return WFS_ERR_IDC_RETAINBINFULL;   // CRM:卡箱满
            case ERR_CRM_HWERR:                 return WFS_ERR_HARDWARE_ERROR;      // CRM:硬件故障
            case ERR_CRM_STATUS_ERR:            return WFS_ERR_HARDWARE_ERROR;      // CRM:状态出错
            case ERR_CRM_SLOT_ISEXIST:          return WFS_ERR_CM_MEDIANOTEXIST;    // CRM:指定卡箱被占用
            case ERR_CRM_SLOT_NOTEXIST:         return WFS_ERR_IDC_CMNOMEDIA;       // CRM:指定卡箱没有被占用
            case ERR_CRM_UNSUP_CMD:             return WFS_ERR_UNSUPP_COMMAND;      // CRM:不支持的指令
            case ERR_CRM_PARAM_ERR:             return WFS_ERR_INVALID_DATA;        // CRM:参数错误
            case ERR_CRM_READTIMEOUT:           return WFS_ERR_HARDWARE_ERROR;      // CRM:读数据超时
            case ERR_CRM_WRITETIMEOUT:          return WFS_ERR_HARDWARE_ERROR;      // CRM:写数据超时
            case ERR_CRM_READERROR:             return WFS_ERR_HARDWARE_ERROR;      // CRM:读数据错
            case ERR_CRM_WRITEERROR:            return WFS_ERR_HARDWARE_ERROR;      // CRM:写数据错
            case ERR_CRM_CARD_NOTFOUND:         return WFS_ERR_IDC_CMNOMEDIA;       // CRM:指定卡不存在
            case ERR_CRM_CARD_ISEXIST:          return WFS_ERR_CM_MEDIANOTEXIST;    // CRM:指定卡已存在
            case ERR_CRM_LOAD_LIB:              return WFS_ERR_INTERNAL_ERROR;      // CRM:动态库错误
            case ERR_CRM_OTHER:                 return WFS_ERR_HARDWARE_ERROR;      // CRM:其他错误/未知错误
            default:                            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    CHAR* CRM_ConvertDevErrCodeToStr(INT nRet)
    {
        #define CRM_CONV_CODE_STR(RET, STR) \
            sprintf(m_szCRMErrStr, "%d|%s", RET, STR); \
            return m_szCRMErrStr;

        memset(m_szCRMErrStr, 0x00, sizeof(m_szCRMErrStr));

        switch(nRet)
        {
            case CRM_SUCCESS:               CRM_CONV_CODE_STR(nRet, "CRM:操作成功");
            case ERR_CRM_INSERT_TIMEOUT:    CRM_CONV_CODE_STR(nRet, "CRM:进卡超时");
            case ERR_CRM_USER_CANCEL:       CRM_CONV_CODE_STR(nRet, "CRM:用户取消");
            case ERR_CRM_COMM_ERR:          CRM_CONV_CODE_STR(nRet, "CRM:通讯错误");
            case ERR_CRM_JAMMED:            CRM_CONV_CODE_STR(nRet, "CRM:堵卡");
            case ERR_CRM_OFFLINE:           CRM_CONV_CODE_STR(nRet, "CRM:脱机");
            case ERR_CRM_NOT_OPEN:          CRM_CONV_CODE_STR(nRet, "CRM:没有打开");
            case ERR_CRM_SLOT_FULL:         CRM_CONV_CODE_STR(nRet, "CRM:卡箱满");
            case ERR_CRM_HWERR:             CRM_CONV_CODE_STR(nRet, "CRM:硬件故障");
            case ERR_CRM_STATUS_ERR:        CRM_CONV_CODE_STR(nRet, "CRM:状态出错");
            case ERR_CRM_SLOT_ISEXIST:      CRM_CONV_CODE_STR(nRet, "CRM:指定卡箱被占用");
            case ERR_CRM_SLOT_NOTEXIST:     CRM_CONV_CODE_STR(nRet, "CRM:指定卡箱没有被占用");
            case ERR_CRM_UNSUP_CMD:         CRM_CONV_CODE_STR(nRet, "CRM:不支持的指令");
            case ERR_CRM_PARAM_ERR:         CRM_CONV_CODE_STR(nRet, "CRM:参数错误");
            case ERR_CRM_READTIMEOUT:       CRM_CONV_CODE_STR(nRet, "CRM:读数据超时");
            case ERR_CRM_WRITETIMEOUT:      CRM_CONV_CODE_STR(nRet, "CRM:写数据超时");
            case ERR_CRM_READERROR:         CRM_CONV_CODE_STR(nRet, "CRM:读数据错");
            case ERR_CRM_WRITEERROR:        CRM_CONV_CODE_STR(nRet, "CRM:写数据错");
            case ERR_CRM_CARD_NOTFOUND:     CRM_CONV_CODE_STR(nRet, "CRM:指定卡不存在");
            case ERR_CRM_CARD_ISEXIST:      CRM_CONV_CODE_STR(nRet, "CRM:指定卡已存在");
            case ERR_CRM_LOAD_LIB:          CRM_CONV_CODE_STR(nRet, "CRM:动态库错误");
            case ERR_CRM_OTHER:             CRM_CONV_CODE_STR(nRet, "CRM:其他错误/未知错误");
            default:                        CRM_CONV_CODE_STR(nRet, "未定义错误");
        }
    }
};

#endif // SPBuild_OPTIM
