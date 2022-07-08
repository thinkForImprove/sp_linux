#pragma once
/***************************************************************
* 文件名称：IDevCRD.h
* 文件描述：声明发卡模块底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年6月30日
* 文件版本：1.0.0.1
****************************************************************/
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "IAllDevPort.h"
#include "QtDLLLoader.h"
#include <string.h>
//////////////////////////////////////////////////////////////////////////
#if defined(IDEVCRD_LIBRARY)
#define DEVCRD_EXPORT     Q_DECL_EXPORT
#else
#define DEVCRD_EXPORT     Q_DECL_IMPORT
#endif
//////////////////////////////////////////////////////////////////////////

//成功
#define CRD_SUCCESS             (0)     // 操作成功
//警告
#define ERR_CRD_INSERT_TIMEOUT  (1)     // 进卡超时
#define ERR_CRD_USER_CANCEL     (2)     // 用户取消
//错误
#define ERR_CRD_COMM_ERR        (-1)    // 通讯错误
#define ERR_CRD_JAMMED          (-2)    // 堵卡
#define ERR_CRD_OFFLINE         (-3)    // 脱机
#define ERR_CRD_NOT_OPEN        (-4)    // 没有打开
#define ERR_CRD_UNIT_FULL       (-5)    // 箱满
#define ERR_CRD_UNIT_EMPTY      (-6)    // 箱空
#define ERR_CRD_UNIT_NOTFOUND   (-7)    // 箱不存在
#define ERR_CRD_HWERR           (-8)    // 硬件故障
#define ERR_CRD_STATUS_ERR      (-9)    // 状态出错
#define ERR_CRD_UNSUP_CMD       (-10)   // 不支持的指令
#define ERR_CRD_PARAM_ERR       (-11)   // 参数错误
#define ERR_CRD_READTIMEOUT     (-12)   // 读数据超时
#define ERR_CRD_WRITETIMEOUT    (-13)   // 写数据超时
#define ERR_CRD_READERROR       (-14)   // 读数据错
#define ERR_CRD_WRITEERROR      (-15)   // 写数据错
#define ERR_CRD_LOAD_LIB        (-16)   // 动态库错误
#define ERR_CRD_DATA_INVALID    (-17)   // 无效数据
#define ERR_CRD_SCAN            (-18)   // 扫描错
#define ERR_CRD_OTHER           (-100)  // 其他错误/未知错误


//-----------------------------------------------------------------------
// CRD 版本号
static const BYTE    byDevVRTUd[17] = {"DevCRD00000100"};

// CRD设备类型
#define CRD_DEV_CRT591H         0       // 发卡模块:CRT-730B

// CRD设备类型(DevCRD.cpp区分不同类型使用)
#define ICRD_TYPE_CRT591H       "0"     // 发卡模块:CRT-730B

// SetData/GetData 数据类型
#define DATATYPE_INIT           0       // 初始化数据

// 缺省卡箱数
#define CARDBOX_COUNT           16

//
#define MCMP_IS0(a, b) (memcmp(a, b, strlen(a)) == 0 && memcmp(a, b, strlen(b)) == 0)
#define MSET(a) memset(a, 0x00, sizeof(a))

// -----------------------------------枚举定义-----------------------------------

//　Status.Device: 设备状态
enum DEVICE_STATUS
{
    DEVICE_STAT_ONLINE                  = 0,    // 设备正常
    DEVICE_STAT_OFFLINE                 = 1,    // 设备脱机
    DEVICE_STAT_POWEROFF                = 2,    // 设备断电
    DEVICE_STAT_NODEVICE                = 3,    // 设备不存在
    DEVICE_STAT_HWERROR                 = 4,    // 设备故障
    DEVICE_STAT_USERERROR               = 5,    // 设备存在,但有人阻止设备操作
    DEVICE_STAT_BUSY                    = 6,    // 设备读写中
    DEVICE_STAT_FRAUDAT                 = 7,    // 设备存在,但有检测到欺诈企图
    DEVICE_STAT_POTENTIAL               = 8,    // 设备检测到欺诈企图但可继续使用,应用决定是否脱机
};

//　status.Dispensr: 总单元状态,例如:发卡箱
enum DISPENSR_STATUS
{
    DISP_STAT_OK                        = 0,    // 所有单元正常
    DISP_STAT_STATE                     = 1,    // 1个/多个单元处于低/空/不工作状态,但仍有一个单元在工作
    DISP_STAT_STOP                      = 2,    // 所有单元故障/空/停止工作
    DISP_STAT_UNKNOWN                   = 3,    // 无法确定单元状态
};

//　status.Transport: 传送模块状态
enum TRANSPORT_STATUS
{
    TRANS_STAT_OK                       = 0,    // 正常
    TRANS_STAT_INOP                     = 1,    // 传输不工作(故障/堵塞)
    TRANS_STAT_UNKNOWN                  = 2,    // 无法确定状态
    TRANS_STAT_NOTSUPP                  = 3,    // 不支持状态报告
};

//　status.Media: 介质状态
enum MEDIA_STATUS
{
    MEDIA_STAT_PRESENT                  = 0,    // 通道内有介质
    MEDIA_STAT_NOTPRESENT               = 1,    // 通道内无介质
    MEDIA_STAT_JAMMED                   = 2,    // 通道内有介质且被夹住
    MEDIA_STAT_NOTSUPP                  = 3,    // 不支持检测介质状态
    MEDIA_STAT_UNKNOWN                  = 4,    // 介质状态未知
    MEDIA_STAT_ENTERING                 = 5,    // 介质在出口
    MEDIA_STAT_LATCHED                  = 6,    // 介质被锁定(IDC专用)
    MEDIA_STAT_EXITING                  = 5,    // 介质在出口插槽(CRD专用)
};

//　status.Shutter: 门状态
enum SHUTTER_STATUS
{
    SHUTTER_STAT_CLOSED                 = 0,    // 关闭
    SHUTTER_STAT_OPEN                   = 1,    // 打开
    SHUTTER_STAT_JAMMED                 = 2,    // 卡住
    SHUTTER_STAT_UNKNOWN                = 3,    // 未知
    SHUTTER_STAT_NOTSUPP                = 4,    // 不支持状态查询
};

//　status.DevicePosition: 指定设备位置状态
enum DEVICE_POSITION_STATUS
{
    DEVPOS_STAT_INPOS                   = 0,    // 设备处于正常工作位置
    DEVPOS_STAT_NOTINPOS                = 1,    // 设备不在正常工作位置
    DEVPOS_STAT_UNKNOWN                 = 2,    // 未知
    DEVPOS_STAT_NOTSUPP                 = 3,    // 不支持状态查询
};

//　status.AntiFraudModule: 反欺诈模块状态
enum ANTIFRAUD_STATUS
{
    ANFRAUD_STAT_OK                     = 0,    // 正常
    ANFRAUD_STAT_INOP                   = 1,    // 不可用
    ANFRAUD_STAT_DETECTED               = 2,    // 检测到外部设备
    ANFRAUD_STAT_UNKNOWN                = 3,    // 未知
    ANFRAUD_STAT_NOTSUPP                = 4,    // 不支持状态查询
};

//　UnitInfo.usStatus: 单个单元状态,例如:发卡箱
enum UnitInfo_STATUS
{
    UNITINFO_STAT_OK                    = 0,    // 正常
    UNITINFO_STAT_LOW                   = 1,    // 介质少
    UNITINFO_STAT_EMPTY                 = 2,    // 空
    UNITINFO_STAT_INOP                  = 3,    // 故障
    UNITINFO_STAT_MISSING               = 4,    // 不存在
    UNITINFO_STAT_HIGH                  = 5,    // 回收单元将满
    UNITINFO_STAT_FULL                  = 6,    // 回收单元满
    UNITINFO_STAT_UNKNOWN               = 7,    // 无法确定状态
};

//　发卡移动状态
enum CARDMOVE_STATUS
{
    CAREMOVE_STAT_ALL_EMPTY             = 0,    // 读卡器+发卡器通道及出口无卡
    CAREMOVE_STAT_IDRW_SIDE             = 1,    // 卡在读卡器内
    CAREMOVE_STAT_IDRW_ENTR             = 2,    // 卡在读卡器插卡口
    CAREMOVE_STAT_IDRW_EMPTY            = 3,    // 读卡器插卡口无卡
    CAREMOVE_STAT_DISP_SIDE             = 4,    // 卡在发卡器插卡口
    CAREMOVE_STAT_NOTSUPP               = 5,    // 不支持状态查询
};

// Unit类型
enum Unit_TYPE
{
    UNIT_NOTSUPP                        = 0,    // 不支持
    UNIT_STORAGE                        = 1,    // 存储箱
    UNIT_RETRACT                        = 2,    // 回收箱
};

// 复位模式
enum MODE_RESET
{
    RESET_NOACTION                      = 0,    // 无动作
    RESET_EJECT                         = 1,    // 退卡
    RESET_RETAIN                        = 2,    // 回收
};

// -----------------------------------结构体定义-----------------------------------

// 发卡模块(CRD)卡箱单元信息
typedef
struct ST_CRD_Unit_Info
{
    WORD    wUnitCnt;                       // 可用单元数目
    WORD    wUnitStat[CARDBOX_COUNT];       // 单元状态
    USHORT  usUnitType[CARDBOX_COUNT];      // 单元类型(存储箱/回收箱/不支持)

    ST_CRD_Unit_Info()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(ST_CRD_Unit_Info));
        wUnitCnt = 0;
        for (int i = 0; i < CARDBOX_COUNT; i ++)
        {
            wUnitStat[i] = UNITINFO_STAT_UNKNOWN;
            usUnitType[i] = UNIT_NOTSUPP;
        }
    }
} STCRDUNITINFO, *LPSTCRDUNITINFO;


// 发卡模块(CRD)状态
typedef
struct ST_CRD_Device_Status
{
    WORD wDevice;               // 设备状态(参考enum DEVICE_STATUS)
    WORD wDispensr;             // 单元状态(参考enum DISPENSR_STATUS)
    WORD wTransport;            // 传输状态(参考enum TRANSPORT_STATUS)
    WORD wMedia;                // 介质状态(参考enum MEDIA_STATUS)
    WORD wShutter;              // 门状态(参考enum SHUTTER_STATUS)
    WORD wDevicePos;            // 设备位置状态(参考enum DEVICE_POSITION_STATUS)
    WORD wAntiFraudMod;         // 反欺诈模块状态(参考enum ANTIFRAUD_STATUS)
    WORD wCardMoveStat;         // 发卡移动状态(参考enum CARDMOVE_STATUS)
    char szErrCode[8];          // 8位的错误码
    STCRDUNITINFO stUnitInfo;   // 卡箱信息
    ST_CRD_Device_Status()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(ST_CRD_Device_Status));
        wDevice = DEVICE_STAT_OFFLINE;
        wDispensr = DISP_STAT_UNKNOWN;
        wTransport = TRANS_STAT_UNKNOWN;
        wMedia = MEDIA_STAT_UNKNOWN;
        wShutter = SHUTTER_STAT_UNKNOWN;
        wAntiFraudMod = ANFRAUD_STAT_NOTSUPP;
        wCardMoveStat = CAREMOVE_STAT_ALL_EMPTY;
        stUnitInfo.Clear();
    }
    int GetUnitStatus(USHORT usBoxNo)   // 指定卡箱序列号[1~N]返回状态
    {
        usBoxNo = (usBoxNo < 1 ? 1 : usBoxNo);
        return stUnitInfo.wUnitStat[usBoxNo - 1];
    }
    int GetUnitType(USHORT usBoxNo)   // 指定卡箱序列号[1~N]返回类型
    {
        usBoxNo = (usBoxNo < 1 ? 1 : usBoxNo);
        return stUnitInfo.usUnitType[usBoxNo - 1];
    }
} STCRDDEVSTATUS, *LPSTCRDDEVSTATUS;


//-----------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////
//接口类定义
struct  IDevCRD
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
    ** 输入：pMode: 自定义OPEN参数字串
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Open(const char *pMode) = 0;

    /************************************************************
    ** 功能：设备初始化
    ** 输入: emActFlag : 卡动作
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Init(int emActFlag) = 0;

    /************************************************************
    ** 功能：关闭与设备的连接
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Close() = 0;

    /************************************************************
    ** 功能：设备复位
    ** 输入：nMode 复位模式(各模块不同可自行定义)
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
#if defined(SET_BANK_SXXH) | defined(SET_BANK_JIANGNAN)
	virtual int Reset(int nMode = 0, int nParam = 0) = 0;
#else
    virtual int Reset() = 0;
#endif

    /************************************************************
    ** 功能：读取设备状态及卡箱信息
    ** 输入：无
    ** 输出：stStat　设备状态信息
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetDevStat(STCRDDEVSTATUS &stStat) = 0;

    /************************************************************
    ** 功能：读取设备信息
    ** 输入：无
    ** 输出：stInfo　设备信息
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetUnitInfo(STCRDUNITINFO &stInfo) = 0;
	
	/************************************************************
    ** 功能：发卡(所有介质从卡箱出来的动作接口)
    ** 输入：nUnitNo　单元编号
    **      nMode 辅助参数,不同含义,根据需要定义在各模块内
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
#if defined(SET_BANK_SXXH) | defined(SET_BANK_JIANGNAN)
    virtual int DispenseCard(const int nUnitNo, const int nMode = 0) = 0;
#else
    virtual int DispenseCard(const int nUnitNo) = 0;
#endif

    /************************************************************
    ** 功能：弹卡(所有介质在通道和出口的移动可以用该接口)
    ** 输入：nMode 辅助参数,不同含义,根据需要定义在各模块内
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int EjectCard(const int nMode = 0) = 0;

    /************************************************************
    ** 功能：回收卡(所有介质回收用该接口)
    ** 输入：nMode 辅助参数,不同含义,根据需要定义在各模块内
    ** 输出：nUnitNo　单元编号
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int RetainCard(const int nMode = 0) = 0;
	
    /************************************************************
    ** 功能：设置数据
    ** 输入：vData 入参
    **      wDataType 入参 设置类型(参考宏定义)
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
#if defined(SET_BANK_SXXH) | defined(SET_BANK_JIANGNAN)
    virtual int SetData(void *vData, int nSize, WORD wDataType = 0) = 0;
#else
    virtual int SetData(void *vData, WORD wDataType = 0) = 0;
#endif


    /************************************************************
    ** 功能：获取数据
    ** 输入：wDataType 入参 获取类型(参考宏定义)
    ** 输出：vData 回参 数据
    **
    ** 返回：见返回错误码定义
    ************************************************************/
#if defined(SET_BANK_SXXH) | defined(SET_BANK_JIANGNAN)
    virtual int GetData(void *vData, int *nSize, WORD wDataType = 0) = 0;
#else
    virtual int GetData(void *vData, WORD wDataType = 0) = 0;
#endif

    /************************************************************
    ** 功能：获取版本号
    ** 输入：wType 入参 获取类型(1DevCRD版本/2固件版本/3设备软件版本/4其他)
    ** 输出：szVer 回参 数据
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual void GetVersion(char* szVer, long lSize, WORD wType) = 0;
};

extern "C" DEVCRD_EXPORT long CreateIDevCRD(LPCSTR lpDevType, IDevCRD *&pDev);
