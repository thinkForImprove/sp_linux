/***************************************************************
* 文件名称：DevImpl_P3018D.h
* 文件描述：HL-2260D打印模块底层指令，提供控制接口
*         用于设备SDK接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年8月23日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEVIMPL_P3018D_H
#define DEVIMPL_P3018D_H


#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "IAllDevPort.h"

#define DLL_DEVLIB_NAME  "libdevice_pantum_pt3018.so.2.4.0"     // 缺省动态库名
#define LOG_NAME         "DevImpl_P3018D.log"                   // 缺省日志名

/*************************************************************
// 无分类　宏定义
*************************************************************/
#define LOG_NAME         "DevImpl_P3018D.log"   // 缺省日志名

//
#define FUNC_POINTER_ERROR_RETURN(LPFUNC, LPFUNC2) \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

// 加载动态库接口
#define FUNC_LOADLIBRARY(FTYPE, LPFUNC, LPFUNC2) \
    LPFUNC = (FTYPE)m_LoadLibrary.resolve(LPFUNC2); \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }



/*************************************************************
// P3018D错误码(函数返回状态码)
*************************************************************/
#define SS_OK                                                   0       // 执行成功
#define SS_ERR_FAILED                                           -1      // 执行失败
#define SS_ERR_SHORTAGEMEMORY                                   -2      // 内存大小不足
#define SS_ERR_HWERROR                                          -3      // 硬件出错
#define SS_ERR_PARAMERR                                         -4      // 参数错误
#define SS_ERR_NOMATCHSDK                                       -5      // 打印机与 SDK 不匹配
#define SS_ERR_UNSUPPCMD                                        -6      // 该打印机不支持此接口
#define SS_ERR_PREHEATED                                        -7      // 打印机已预热
/*************************************************************/


#define SS_ERR_POCKETFULL                                       -8      // 票箱已满
#define SS_ERR_NOINKPRESENT                                     -9      // 墨盒不在位
#define SS_ERR_PARSEJSON                                        -10     // 解析Json参数错误
#define SS_ERR_LOADALGDLL                                       -20     // 加载算法库失败
#define SS_ERR_INITALG                                          -21     // 算法初始化接口失败
#define SS_ERR_SETBRIGHT                                        -22     // 设置图像亮度失败
#define SS_ERR_POCKETOPEN                                       -23     // 票箱开
#define SS_ERR_TIMEOUT                                          -24     // 超时
#define SS_ERR_CANCELED                                         -25      // 操作被取消
#define SS_ERR_OTHER/*INITALG*/                                 -99999  // 其他错误

/*************************** 打印机状态　************************/
#define PTR_STATUS_SUCCESS                                   0       // 正常
#define PTR_STATUS_ERROR                                     1       // 不正常
#define PTR_STATUS_OFFLINE                                   2       // 脱机
#define PTR_STATUS_LOWPAPER                                  3       // 少纸
#define PTR_STATUS_JAM                                       5       // 卡纸或不匹配
#define PTR_STATUS_LOWTONER                                  6       // 硒鼓或粉盒耗尽
#define PTR_STATUS_BUZY                                      8       // 正忙
#define PTR_STATUS_FRONTCOVEROPEN                            9       // 打印机盖打开
#define PTR_STATUS_NOMATCHPAPER                              10      // 纸型不匹配
#define PTR_STATUS_PAPERSOURCENOMATCHACTUAL                  11      // 纸张来源与实际进纸不匹配
#define PTR_STATUS_TRAYNOINSTALL                             12      // 纸盒未安装
/**************************************************************/

#define SS_ERR_CHK_JAM                                          -12010  // 卡票
#define SS_ERR_CHK_NOPAPERINPRINTPOSITION                       -12011  // 打印位置没有票据
#define SS_ERR_CHK_NOMEDIAPRESENT                               -12020  // 通道无票
#define SS_ERR_CHK_NOIMAGE                                      -12021  // 无可用图像数据
#define SS_ERR_CHK_MEDIAPRESENT                                 -12022  // 通道有票
#define SS_ERR_CHK_UNKNOWTYPE                                   -12030  // 票据类型未知
#define SS_ERR_CHK_IMPERFECT                                    -12031  // 票据缺角
#define SS_ERR_CHK_FORGED                                       -12032  // 伪票
#define SS_ERR_CHK_TAMPER                                       -12033  // 篡改票
#define SS_ERR_CHK_NOOCRAREA                                    -12034  // 未设置OCR区域
#define SS_ERR_CHK_INCOMPLETE                                   -12040  // 票据要素不全
#define SS_ERR_CHK_TYPEERROR                                    -12050  // 票据类型不符
#define SS_ERR_CHK_DIR                                          -12070  // 票据正反面放置错误
#define SS_ERR_CHK_LONGLENGTH                                   -12080  // 票据超长
#define SS_ERR_CHK_READRFID                                     -12090  // 读RFID失败
#define SS_ERR_CHK_EJECT_JAM                                    -12503  // 退票失败，卡票
#define SS_ERR_CHK_SAVEIMAGE                                    -12601  // 图像保存失败
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_PATAMETER               -501    // 鉴伪参数错误
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_CONFIGFILE              -502    // 鉴伪配置文件格式错误
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_CONFIGDATA              -503    // 鉴伪配置文件数据错误
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_TYPENUMMANY             -504    // 鉴伪存单类型数量太多
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_LOADLIBRARY             -505    // 鉴伪加载动态库失败
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_DECRYPT                 -506    // 鉴伪解密错误，交互验证不通过
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_DESKEWCROP              -507    // 鉴伪纠偏裁剪错误
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_WIDTHHEIGHT             -508    // 鉴伪票据图像宽度高度不符
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_BILLTYPE                -509    // 鉴伪票据类型错误
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_BASEPOINT               -510    // 鉴伪定位基准点失败
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_FRONTREAR               -511    // 鉴伪正反面放反
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_MALLOCBUFFER            -512    // 鉴伪内存申请错误
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_NONETEMPLATE            -513    // 鉴伪没有模板数据
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_NULLDATA                -514    // 鉴伪没有图像数据
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_NONEFEATURE             -515    // 鉴伪没有特征数据
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_WATERMARK               -516    // 鉴伪水印错误
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_ANGLEBIG                -517    // 鉴伪倾斜角度大
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_FINDRECT                -518    // 鉴伪查找区域失败
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_NONEFUNC                -600    // 鉴伪缺少功能
#define SCANNER_ERROR_CD_IDENTIFY_ERROR_OHTER                   -700    // 鉴伪其它错误
#define SCANNER_ERROR_OCR_ERROR_PARAS                           -801    // OCR参数错误
#define SCANNER_ERROR_OCR_ERROR_LOAD_CONFIG_FILE                -802    // OCR加载主动态库配置文件错误
#define SCANNER_ERROR_OCR_ERROR_LOAD_CONFIG_INFO                -803    // OCR主配置文件配置文件信息错误
#define SCANNER_ERROR_OCR_ERROR_LOAD_DLL                        -804    // OCR加载识别动态库失败
#define SCANNER_ERROR_OCR_ERROR_GET_DLL_PROC                    -805    // OCR获取导出识别函数失败
#define SCANNER_ERROR_OCR_ERROR_DLL_INIT                        -806    // OCR识别动态库初始化函数返回值错误
#define SCANNER_ERROR_OCR_ERROR_RECOG                           -807    // OCR识别失败
#define SCANNER_ERROR_OCR_ERROR_OHTER                           -900    // OCR其它错误
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_PATAMETER               -1001   // 支票鉴伪参数错误
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_ANGLEBING               -1002   // 支票鉴伪倾斜角度大
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_QUALITY                 -1003   // 支票鉴伪质量检测不合格
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_TABLESIZE               -1004   // 支票鉴伪表格大小不符合要求
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_BILLTYPE                -1005   // 支票鉴伪票据类型错误
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_IMAGEFLIP               -1006   // 支票鉴伪图像方向颠倒
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_IMAGEDARK               -1007   // 支票鉴伪红外发射过暗
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_INCOMPLETE              -1008   // 支票鉴伪票面扫描不完整
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_LOCATION                -1009   // 支票鉴伪定位失败
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_FRONTREAR               -1010   // 支票鉴伪图像正面与反面放反
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_BRTIRTR                 -1030   // 支票鉴伪红外透射亮度异常
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_DECRYPT                 -1101   // 支票鉴伪解密错误
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_DESKEWCROP              -1102   // 支票鉴伪纠偏错误
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_DESKEWCROPNOBASEIMAGE   -1103   // 支票鉴伪无纠偏基准图像
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_CHEQUETYPE              -1200   // 支票鉴伪票据类型错误
#define SCANNER_ERROR_ZP_IDENTIFY_ERROR_OHTER                   -1300   // 支票鉴伪其他错误

/*************************************************************
// 返回值/错误码　宏定义
// <0 : Impl处理返回
// 0~100: 硬件设备返回
*************************************************************/
// <0: Impl处理返回
#define IMP_SUCCESS             SS_OK       // 成功
#define IMP_ERR_LOAD_LIB        1           // 动态库加载失败
#define IMP_ERR_PARAM_INVALID   2           // 参数无效
#define IMP_ERR_UNKNOWN         3           // 未知错误


/*************************************************************
// 结构体定义
*************************************************************/
// 设备获取状态返回结构体(硬件SDK指定定义)
typedef struct SCANNERSTATUS
{
    int iStatus;                    // 状态位:0-12
    int iHaveImageData;             // 0-无图像数据，1-有需要上传的图像数据
    int iError;                     // 0-正常，1-出错(通用错误,具体错误根据其他参数判定)
    int iJam;                       // 0-正常，1-塞纸
    int iVoltageError;              // 0-正常，1-电压异常
    int iHardwareError;             // 0-正常，1-硬件错误
    int iFPGAError;                 // 0-正常，1-FPGA错误
    int iPocketOpen;				// 0-出纸兜关，1-出纸兜开(左/右票箱打开时状态,iError==1)
    int iLeftPocketFull;			// 0-左票箱未满，1-左票箱满
    int iPushGearError;             // 0-出票兜凸轮归位，1-出票兜凸轮未归位
    int iPushBoardError;			// 0-出票兜推纸板归位，1-出票兜推纸板未归位
    int iPickFail;                  // 0-纠偏成功，1-纠偏失败
    int iPaperEnter;				// 0-纸张进入正常，1-纸张进入异常
    int iPaperLength;               // 0-纸张长度正常，1-纸张长度超长/过短
    int iPocketFullSensorInit;		// 0-正常，1-票箱满传感器初始化异常
    int iPaperEnterRepeat;			// 0-正常，1-纸张二次进入状态
    int iRightPocketFull;			// 0-右票箱未满，1-右票箱满
    int iLeftPocketHavePaper;		// 0-左票箱无票，1-左票箱有票
    int iRightPocketHavePaper;		// 0-右票箱无票，1-右票箱有票
    int iLeftPocketOpen;			// 0-左票箱关，1-左票箱开(打开时,iError==1,iPocketOpen=1)
    int iRightPocketOpen;			// 0-右票箱关，1-右票箱开(打开时,iError==1,iPocketOpen=1)
    int iInkPresent;				// 0-墨盒不在位，1-墨盒在位
int iInkNearEmpty;                  // 0-墨盒墨未尽, 1-墨盒墨将尽
} ScannerStatus, *PScannerStatus, SCANNERSTATUS, *LPSCANNERSTATUS;
// iStatus状态如下：
#define  SCANNER_STATUS_IDLE			0			// 空闲状态(可作为通道有无票判定)
#define  SCANNER_STATUS_READY			1			// 就绪状态
#define  SCANNER_STATUS_SCAN			2			// 扫描状态
#define  SCANNER_STATUS_SENSORCORRECT	3			// 传感器校正状态
#define  SCANNER_STATUS_CISCORRECT		4			// CIS校正状态
#define  SCANNER_STATUS_FEED			5			// 走纸状态
#define  SCANNER_STATUS_READ			6			// 读数据状态
#define  SCANNER_STATUS_WAITACTION		7			// 等待动作状态
#define  SCANNER_STATUS_ERROR			8			// 错误状态
#define  SCANNER_STATUS_PUSH			9			// 压箱工作状态
#define  SCANNER_STATUS_PAPEROUT        10			// 票在出纸口
#define  SCANNER_STATUS_RFID			11			// 射频等待状态
#define  SCANNER_STATUS_PRINT			12			// 打印状态


/*************************************************************
// 动态库输出函数　定义
*************************************************************/
// 1. 获取设备句柄
typedef int (*m_SS_Device_Printer_Open)(char *printername, void* printer_handle, int *nNeeded);
// 2. 关闭设备
typedef void (*m_SS_Device_Printer_Close)(void* printer_handle);
// 3. 获取设备已打印页数
typedef int (*m_SS_Device_Printer_GetPageNum)(void* printer_handle , unsigned int *page);
// 4. 获取设备版本号
typedef int (*m_SS_Device_Printer_GetVersion)(void* printer_handle , char *version);
// 5. 获取设备序列号
typedef int (*m_SS_Device_Printer_GetSerialNumber)(void* printer_handle , char *serial);
// 6. 获取设备状态
typedef int (*m_SS_Device_Printer_GetState)(void* printer_handle , unsigned char *state);
// 7. 获取设备信息
typedef int (*m_SS_Device_Printer_GetDeviceInfo)(void* printer_handle ,char *json);
// 8. 获取设备状态描述信息
typedef int (*m_SS_Device_Printer_GetErrmsg)(void* printer_handle , char *errinfo);
// 9. 获取支持的打印机列表
typedef int (*m_SS_Device_Printer_GetPrinterList)(char* json, int* size);
// 10. 删除当前打印作业
typedef int (*m_SS_Device_Printer_DeleteCurrentJob)(void* printer_handle);
// 11. 预热打印机
typedef int (*m_SS_Device_Printer_WarmUp)(void* printer_handle);
// 12. 设置打印机休眠时间
typedef int (*m_SS_Device_Printer_SetSleepTime)(void* printer_handle,int time);

/*************************************************************
// 封装类: 命令编辑、发送接收等处理。
*************************************************************/
class CDevImpl_P3018D : public CLogManage
{
public:
    CDevImpl_P3018D();
    CDevImpl_P3018D(LPSTR lpLog);
    virtual ~CDevImpl_P3018D();

public:
    BOOL    OpenDevice();
    BOOL    OpenDevice(WORD wType);
    BOOL    CloseDevice();
    BOOL    IsDeviceOpen();
    CHAR*   ConvertErrCodeToStr(INT nRet);
    void    SetLibPath(LPCSTR lpPath);

public: // 接口函数封装
    // 1. 获取设备句柄
    int Open(char *printername, void* printer_handle, int *nNeeded);
    // 2. 关闭设备
    void Close(void* printer_handle);
    // 3. 获取设备已打印页数
    int GetPageNum(void* printer_handle , unsigned int *page);
    // 4. 获取设备版本号
    int GetVersion(void* printer_handle , char *version);
    // 5. 获取设备序列号
    int GetSerialNumber(void* printer_handle , char *serial);
    // 6. 获取设备状态
    int GetState(void* printer_handle , unsigned char *state);
    // 7. 获取设备信息
    int GetDeviceInfo(void* printer_handle ,char *json);
    // 8. 获取设备状态描述信息
    int GetErrmsg(void* printer_handle , char *errinfo);
    // 9. 获取支持的打印机列表
    int GetPrinterList(char* json, int* size);
    // 10. 删除当前打印作业
    int DeleteCurrentJob(void* printer_handle);
    // 11. 预热打印机
    int WarmUp(void* printer_handle);
    // 12. 设置打印机休眠时间
    int SetSleepTime(void* printer_handle,int time);
private:
    BOOL                            m_bDevOpenOk;

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

private: // 动态库接口定义    

    m_SS_Device_Printer_Open                Device_Printer_Open;            // 1. 获取设备句柄
    m_SS_Device_Printer_Close               Device_Printer_Close;           // 2. 关闭设备
    m_SS_Device_Printer_GetPageNum          Device_Printer_GetPageNum;      // 3. 获取设备已打印页数
    m_SS_Device_Printer_GetVersion          Device_Printer_GetVersion;      // 4. 获取设备版本号
    m_SS_Device_Printer_GetSerialNumber     Device_Printer_GetSerialNumber; // 5. 获取设备序列号
    m_SS_Device_Printer_GetState            Device_Printer_GetState;        // 6. 获取设备状态
    m_SS_Device_Printer_GetDeviceInfo       Device_Printer_GetDeviceInfo;   // 7. 获取设备信息
    m_SS_Device_Printer_GetErrmsg           Device_Printer_GetErrmsg;       // 8. 获取设备状态描述信息
    m_SS_Device_Printer_GetPrinterList      Device_Printer_GetPrinterList;  // 9. 获取支持的打印机列表
    m_SS_Device_Printer_DeleteCurrentJob    Device_Printer_DeleteCurrentJob;// 10. 删除当前打印作业
    m_SS_Device_Printer_WarmUp              Device_Printer_WarmUp;          // 11. 预热打印机
    m_SS_Device_Printer_SetSleepTime        Device_Printer_SetSleepTime;    // 12. 设置打印机休眠时间

private:
    CSimpleMutex                    m_MutexAction;
    CHAR                            m_szErrStr[1024];
};


#endif // DEVIMPL_P3018D_H
