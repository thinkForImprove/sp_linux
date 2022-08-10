/***************************************************************
* 文件名称: DevImpl_DMTM1M.h
* 文件描述: 身份证模块底层指令, 提供控制接口 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年3月25日
* 文件版本: 1.0.0.1
****************************************************************/

#ifndef DEVIMPL_DMTM1M_H
#define DEVIMPL_DMTM1M_H

#include <string>

#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "libDMTM1M.h"
#include "../ComFile/ComFile.h"

/*************************************************************
// 返回值/错误码　宏定义
*************************************************************/
// Impl处理返回
#define IMP_SUCCESS                     0           // 成功
#define IMP_ERR_LOAD_LIB                -1          // 动态库加载失败
#define IMP_ERR_PARAM_INVALID           -2          // 参数无效
#define IMP_ERR_UNKNOWN                 -3          // 未知错误
#define IMP_ERR_NOTOPEN                 -4          // 设备未Open
#define IMP_ERR_OTHER                   -5          // 其他错误
// Device返回
#define	IMP_ERR_DEV_0000H               0x0000      // 成功
#define	IMP_ERR_DEV_8101H               0x8101      // 设备未打开
#define	IMP_ERR_DEV_8102H               0x8102      // 传输错误
#define	IMP_ERR_DEV_8103H               0x8103      // 句柄错误
#define	IMP_ERR_DEV_8201H               0x8201      // XOR 错误
#define	IMP_ERR_DEV_8202H               0x8202      // SUM 错误
#define	IMP_ERR_DEV_8203H               0x8203      // 指令号错误
#define	IMP_ERR_DEV_8204H               0x8204      // 参数错误
#define	IMP_ERR_DEV_8205H               0x8205      // 包头错误
#define	IMP_ERR_DEV_8206H               0x8206      // 包长错误
#define	IMP_ERR_DEV_8D01H               0x8D01      // 无卡
#define	IMP_ERR_DEV_8D02H               0x8D02      // apdu 指令交互失败
#define	IMP_ERR_DEV_8E01H               0x8E01      // 获取SAMID 失败
#define	IMP_ERR_DEV_8E02H               0x8E02      // 读取身份证信息失败
#define	IMP_ERR_DEV_8F01H               0x8F01      // XOR 错误
#define	IMP_ERR_DEV_8F02H               0x8F02      // SUM 错误
#define	IMP_ERR_DEV_8F03H               0x8F03      // 指令号错误
#define	IMP_ERR_DEV_8F04H               0x8F04      // 参数错误
#define	IMP_ERR_DEV_8F05H               0x8F05      // 传输超时
#define	IMP_ERR_DEV_8F06H               0x8F06      // 扩展域参数错误


/*************************************************************************
// 宏定义
*************************************************************************/
#define DLL_DEVLIB_NAME  "libdmtreader.so"      // 缺省动态库名
#define LOG_NAME         "DevImpl_DMTM1M.log"   // 缺省日志名

#define SAVE_IMG_MAX_SIZE   1024 * 1024 * 16    // DMTM1M生成图像数据最大16M

#define SAVE_IMG_BMP        0
#define SAVE_IMG_JPG        1
#define SAVE_IMG_BMP_X      2
#define SAVE_IMG_JPG_X      3

#define SAVE_IMG_BMP_S      "BMP"
#define SAVE_IMG_JPG_S      "JPG"
#define SAVE_IMG_BMP_SX     "bmp"
#define SAVE_IMG_JPG_SX     "jpg"


// 卡动作
#define CARD_EJECT          0       // 退卡
#define CARD_RETRACT        1       // 吞卡
#define CARD_EJECTMENT      2       // 退卡并保持
#define CARD_NOACTION       3       // 无动作

// 证件类型
#define ID_CHINA            0       // 国内证件
#define ID_FOREIGN          1       // 国外证件
#define ID_GAT              2       // 港澳台证件

// 加载动态库接口
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

/*************************************************************************
// 动态库输出函数接口　定义
*************************************************************************/

// 1. 连接读卡器
// 入参: reader 读卡器名称
// 回参: 无
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_connectReader)(char *reader);

// 2. 断开读卡器连接
// 入参: 无
// 回参: 无
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_disconnect)();

// 3. 获取卡片ATR(找卡)
// 入参: 无
// 回参: atr ATR数据
//      atrsize ATR数据长度
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_getAtr)(unsigned char *atr, int *atrsize);

// 4. 获取读卡器固件版本
// 入参: 无
// 回参: ver 固件版本
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_getVersion)(char *ver);

// 5. APDU指令传输(实现读卡器和芯片APDU指令传输)
// 入参: cmd 指令数据
//      cmdsize 指令数据长度
// 回参: rsp 响应数据
//      rspsize 响应数据长度
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_transceive)(unsigned char *cmd, int cmdsize, unsigned char *rsp, int *rspsize);

// 6.
// 入参:
// 回参:
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_firmwareUpdate)();

// 7. 获取SAMID
// 入参:
// 回参: samid SAMID
// 返回值:
typedef int (*pDMT_getSAMID)(unsigned char *samid);

// 8. 读取身份证数据
// 入参:
// 回参: baseinf 基本信息,空间应不小于256字节
//      basesize 基本信息长度
//      photo 照片数据,空间应不小于1024字节
//      photosize 照片数据长度；
//      fpimg 指纹数据,空间应不小于1024字节
//      fpsiz 指纹数据长度；
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_readIDCardMsg)(unsigned char *baseinf, int *basesize, unsigned char *photo,
                                  int *photosize, unsigned char *fpimg, int *fpsize);

// 9.
// 入参:
// 回参:
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_buzzer)(int time_ms);

// 10.
// 入参:
// 回参:
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_getChipSN)(unsigned char *outbuf);

// 11.
// 入参:
// 回参:
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_getBoardSN)(char *snbuf);

// 12.
// 入参:
// 回参:
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_mutualAuth)();

// 13.
// 入参:
// 回参:
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_samCommand)(unsigned char *cmd, int cmdsize, unsigned char *rsp, int *rspsize);

// 14.
// 入参:
// 回参:
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_samCardCommand)(unsigned char *cmd, int cmdsize, unsigned char *rsp, int *rspsize);

// 15.
// 入参:
// 回参:
// 返回值: 0:成功, 非0:失败
typedef int (*pDMT_reset)();


/**************************************************************************
* 命令编辑、发送接收等处理                                                    *
***************************************************************************/
class CDevImpl_DMTM1M : public CLogManage
{
public:
    CDevImpl_DMTM1M();
    CDevImpl_DMTM1M(LPSTR lpLog);
    virtual ~CDevImpl_DMTM1M();

public: //
    INT OpenDevice();                                           // 打开设备(无入参)
    INT OpenDevice(WORD wType);                                 // 打开设备(有入参)
    INT CloseDevice();                                          // 关闭设备
    BOOL IsDeviceOpen();                                        // 设备是否Open成功

public: // 接口函数封装
    // 1. 设备复位
    INT nResetDevice(INT nMode);
    // 2. 取设备状态
    INT nGetDevStat();
    // 3. 取卡状态
    INT nGetCardStat();
    // 4. 退卡
    INT nEjectIdCard(INT nMode = 0);
    // 5. 取ATR
    INT nGetAtr(LPSTR lpAtr, INT &nSize);
    // 6. 取证件类型
    INT nGetIDCardType(INT &nType);
    // 7. 取身份数据
    INT nGetIDCardInfo(INT nIDType, LPVOID lpVoid, LPCSTR lpcSaveImage, BOOL headImgNameIncluded = false, BOOL debugMode = false);
    // 8. 取固件版本
    INT nGetFWVersion(LPSTR lpFwVer);


private:
    BOOL            m_bDevOpenOk;                               //
    CHAR            m_szErrStr[1024];

private:
    void Init();

public:
    void SetLibPath(LPCSTR lpPath);                             // 设置动态库路径(DeviceOpen前有效)
    void SetLibVer(WORD wVer);                                  // 设置动态库版本(DeviceOpen前有效)
    INT  SetReConFlag(BOOL bFlag);                              // 设置断线重连标记
    LPSTR ConvertCode_IMPL2Str(INT nErrCode);

private: // 接口加载
    BOOL bLoadLibrary();
    void vUnLoadLibrary();
    BOOL bLoadLibIntf();
    void vInitLibFunc();

private: // 接口加载
    char        m_szLoadDllPath[MAX_PATH];
    WORD        m_wLoadDllVer;
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;
    INT         m_nDevErrCode;
    INT         m_nStatErrCode;
    BOOL        m_bReCon;                                           // 是否断线重连状态
    INT         m_nRetErrOLD[8];                                    // 处理错误值保存(0:库加载/1:设备连接/
                                                                    //  2:设备状态/3介质状态/4)
    DMT_M1_M    m_clDmt;

private: // 动态库接口定义
    pDMT_connectReader             DMT_connectReader;               // 1. 连接读卡器
    pDMT_disconnect                DMT_disconnect;                  // 2. 断开读卡器连接
    pDMT_getAtr                    DMT_getAtr;                      // 3. 获取卡片ATR(找卡)
    pDMT_getVersion                DMT_getVersion;                  // 4. 获取读卡器固件版本
    pDMT_transceive                DMT_transceive;                  // 5. APDU指令传输(实现读卡器和芯片APDU指令传输)
    pDMT_firmwareUpdate            DMT_firmwareUpdate;              // 6.
    pDMT_getSAMID                  DMT_getSAMID;                    // 7. 获取SAMID
    pDMT_readIDCardMsg             DMT_readIDCardMsg;               // 8. 读取身份证数据
    pDMT_buzzer                    DMT_buzzer;                      // 9.
    pDMT_getChipSN                 DMT_getChipSN;                   // 10.
    pDMT_getBoardSN                DMT_getBoardSN;                  // 11.
    pDMT_mutualAuth                DMT_mutualAuth;                  // 12.
    pDMT_samCommand                DMT_samCommand;                  // 13.
    pDMT_samCardCommand            DMT_samCardCommand;              // 14.
    pDMT_reset                     DMT_reset;                       // 15. 读卡器复位

};

#endif // DEVIMPL_DMTM1M_H
