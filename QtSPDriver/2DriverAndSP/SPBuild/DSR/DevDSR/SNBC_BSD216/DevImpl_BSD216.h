/***************************************************************
* 文件名称：DevImpl_PRM.h
* 文件描述：PRM存折打印模块底层指令，提供控制接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVIMPL_BSD216_H
#define DEVIMPL_BSD216_H

#include <string>
#include <unistd.h>
#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "IDevPTR.h"
#include "../XFS_DSR/def.h"

/*************************************************************************
// 返回值/错误码　宏定义
// <0 : Impl处理返回
// 0~100: 硬件设备返回
*************************************************************************/
// >100: Impl处理返回
#define IMP_SUCCESS                     0               // 成功
#define IMP_ERR_LOAD_LIB                101             // 动态库加载失败
#define IMP_ERR_PARAM_INVALID           102             // 参数无效
#define IMP_ERR_UNKNOWN                 103             // 未知错误
#define IMP_ERR_NOTOPEN                 104             // 设备未Open

// >0: Device异常返回
#define SNBC_ERROR_NONE                   0       // 无错
#define SNBC_FILE_ERROR                   1       // 文件错误(Termb.Lic文件错误)
#define SNBC_COMMUNICATION_ERROR          2       // 通信错
#define SNBC_SET_SCAN_MODE_ERROR          3       // 设置扫描仪失败
#define SNBC_COVER_ERROR                  4       // 上盖错
#define SNBC_PAPER_ERROR                  5       // 塞纸错
#define SNBC_NO_PAPER_ERROR               6       // 无纸
#define SNBC_DOUBLE_PAPER_ERROR           7       // 重张错
#define SNBC_STATUS_ERROR                 8       // 错误状态
#define SNBC_ERROR_UNKNOWN                9       // 设备出错，非塞纸、上盖打开、重张错
#define SNBC_LOAD_DLL_ERROR               10      // 加载dll失败
#define SNBC_PARA_ERROR                   11      // 参数错
#define SNBC_MEMORY_ERROR                 12      // 内存不足
#define SNBC_HARDWARE_ERROR               13      // 硬件错误
#define SNBC_DELETECISDATA_ERROR          14      // 删除CIS多余数据错误
#define SNBC_MIRROR_ERROR                 15      // 镜像处理错误
#define SNBC_NO_IMAGE_ERROR               16      // 无图像错误
#define SNBC_INIT_ERROR                   17      // 初始化错误
#define SNBC_PROCESS_IMAGE_ERROR          18	  // 图像处理错

/*************************************************************************
// 设备返回设备状态值　宏定义
*************************************************************************/

/*************************************************************************
// 无分类　宏定义
*************************************************************************/

#define LOG_NAME            "DevImpl_BSD216.log"           // 缺省日志名
#define DLL_DEVLIB_NAME     "libScanAPI.so.1.0.3.0"            // 缺省动态库名

// 加载动态库接口
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

#ifndef MIN
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#endif

// 图像信息结构体
typedef struct ImageDataAndInfo_T{
    unsigned char*  PicBuffer;
    long  lPicSize;
    int  iPicType;
} ImageDataAndInfo,*PImageDataAndInfo;

// 设备状态
typedef struct DeviceStatus{
    BOOL bIdle;                 //是否空闲状态
    BOOL bWaitting;             //是否等待状态
    BOOL bScaning;              //是否扫描状态
    BOOL bHaveData;             //是否有图像数据
    BOOL bError;                //是否有错误状态
    BOOL bHardWareError;        //是否硬件错误
    BOOL bPaperError;           //是否塞纸错误
    BOOL bCoverError;           //是否上盖打开错误
    BOOL bDoublePaper;          //是否重张错误
    BOOL bFrontSensorHavePaper; //进纸传感器是否有纸
    BOOL bRearSensorHavePaper;  //出纸传感器是否有纸
    BYTE  TicketBoxVoltage;
    //int bHavePaper;//通过前后传感器都没有纸的情况计算得到，暂时使用
} DEVICESTATUS;


/*************************************************************************
// 打印机SDK接口使用 宏定义
*************************************************************************/
// Init控制
#define INIT_NOACTION                   1               // 无动作
#define INIT_EJECT                      2               // 退折
#define INIT_RETRACT                    0               // 吞折


/*************************************************************************
// 动态库输出函数接口　定义
*************************************************************************/

typedef int (*pScan_Open)();
typedef int (*pScan_Init)(char* cFilePath);
typedef int (*pScan_Start)();
typedef int (*pScan_GetStatus)(DEVICESTATUS* m_devicestatus);
typedef int (*pScan_SetPaperOut)(int iDirection,short length);
typedef int (*pScan_GetDecodeLen)(int* DecodeLenFront, int* DecodeLenRear);
typedef int (*pScan_GetImages)(PImageDataAndInfo PicFront,PImageDataAndInfo PicRear);
typedef int (*pScan_GetDecode)(char* cResultFront,int* iDecodeTypeFront,int* iLenFront,char* cResultRear,int* iDecodeTypeRear,int* iLenRear);
typedef int (*pScan_GetFWVersion)(char* cVersion);
typedef int (*pScan_GetSerialNumber)(char* cSerialNum);
typedef int (*pScan_UpdateFirmware)(char* cFilePath);
typedef int (*pScan_Close)();
typedef int (*pScan_Reset)(int type);

/*************************************************************************
// 命令编辑、发送接收等处理
*************************************************************************/
class CDevImpl_BSD216 : public CLogManage
{
public:
    CDevImpl_BSD216();
    CDevImpl_BSD216(LPSTR lpLog);
    CDevImpl_BSD216(LPSTR lpLog, LPSTR lpDevStr);
    virtual ~CDevImpl_BSD216();

public: // 动态库接口封装
    INT DeviceOpen();
    INT DeviceClose();                                          // 2. 关闭设备
    INT Reset(INT nAction, INT nTimeOut = 5/*秒*/);             // 3. 复位设备    
    INT MediaEject();                                           // 10. 退纸(介质退出)
    INT GetDevStatus(DEVICESTATUS *ds);                                         // 11. 获取设备状态
    INT GetFWVersion(LPSTR pBuff, DWORD dwSize);                // 13. 获取固件版本
    INT ImplScanImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut);

private:
    INT     ConvertErrorCode(INT nRet);                         // 转换为Impl返回码/错误码
    CHAR*   ConvertErrCodeToStr(INT nRet);

public: // 对外参数设置接口
    INT SetReConFlag(BOOL bFlag);
    void SetLibPath(LPCSTR lpPath);                             // 设置动态库路径(DeviceOpen前有效)   

private:
    CSimpleMutex    m_MutexAction;                              // 执行互斥
    BOOL            m_bDevOpenOk;                               // 设备Open标记
    CHAR            m_szErrStr[1024];
    CHAR            m_szDevStataOLD;                            // 上一次获取设备状态保留
    INT             m_nRetErrOLD[12];                           // 处理错误值保存(0:库加载/1:设备连接/2:设备状态/3介质状态/4介质吸入/5Open后Reset)
    BOOL            m_bReCon;                                   // 是否断线重连状态
    CHAR            m_szOpenMode[64];                           // 打开模式(缺省USB)
    INT             m_nBaudRate;                                // 波特率(缺省9600)

    CINIFileReader  m_cINI;

private:
    void Init();

private: // 接口加载
    BOOL    bLoadLibrary();
    void    vUnLoadLibrary();
    BOOL    bLoadLibIntf();

private: // 接口加载
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;
    INT         m_nDevErrCode;

private: // 动态库接口定义+初始化
    pScan_Open				 Scan_Open = nullptr;
    pScan_Init               Scan_Init = nullptr;
    pScan_Start              Scan_Start = nullptr;
    pScan_GetStatus          Scan_GetStatus = nullptr;
    pScan_SetPaperOut        Scan_SetPaperOut = nullptr;
    pScan_GetDecodeLen       Scan_GetDecodeLen = nullptr;
    pScan_GetImages          Scan_GetImages = nullptr;
    pScan_GetDecode          Scan_GetDecode = nullptr;
    pScan_GetFWVersion       Scan_GetFWVersion = nullptr;
    pScan_GetSerialNumber    Scan_GetSerialNumber = nullptr;
    pScan_UpdateFirmware     Scan_UpdateFirmware = nullptr;
    pScan_Close              Scan_Close = nullptr;
    pScan_Reset              Scan_Reset = nullptr;

};

#endif // DEVIMPL_PRM_H
