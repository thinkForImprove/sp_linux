/***************************************************************
* 文件名称：DevImpl_RSCD400M.h
* 文件描述：RSC-D403M票据受理模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月27日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEVIMPL_RSCD400M_H
#define DEVIMPL_RSCD400M_H


#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "IAllDevPort.h"

#define DLL_DEVLIB_PATH  "CSR/RSCD400M/"           // 指定动态库中间路径位置
#define DLL_DEVLIB_NAME  "libRSC-D400M_API.so"    // 缺省动态库名
#define LOG_NAME         "DevImpl_RSCD400M.log"   // 缺省日志名

/*************************************************************
// 无分类　宏定义
*************************************************************/
#define LOG_NAME         "DevImpl_RSCD400M.log"   // 缺省日志名

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

// 票据类型
#define NOTE_TYPE_ZP    0   // 支票
#define NOTE_TYPE_CD    1   // 存单
#define NOTE_TYPE_GZ    2   // 国债

// JSON
#define BT_JSON_USEAREA         "UseJsonArea"       // 是否使用JSON串内的数据(0不使用/1使用)
#define BT_JSON_CHECKWIDTH      "CheckWidth"        // 票据宽(单位:MM)
#define BT_JSON_CHECKHEIGHT     "CheckHeight"       // 票据高(单位:MM)
#define BT_JSON_PRINTWIDTH      "Billwidth"         // 票据宽(单位:MM)
#define BT_JSON_PRINTHEIGHT     "Billheight"        // 票据高(单位:MM)
#define BT_JSON_FIELS           "Fields"            // 节点Key名
#define BT_JSON_FIELDNAME       "FieldName"         // 项名
#define BT_JSON_STARTX          "StartX"            // 起始坐标X(单位:MM)
#define BT_JSON_STARTY          "StartY"            // 起始坐标Y(单位:MM)
#define BT_JSON_AREAWIDTH       "AreaWidth"         // 可用宽(单位:MM)
#define BT_JSON_AREAHEIGHT      "AreaHeight"        // 可用高(单位:MM)
#define BT_JSON_ISTRUE          "IsTrue"            // 鉴伪结果
#define BT_JSON_INFO            "Info"              // OCR信息

#define BT_JSON_OCR_RESULLT     "OCR_Result"        // OCR识别结果
#define BT_JSON_ACCOUNT         "Account"           // 账号识别
#define BT_JSON_AMOUNT          "Amount"            // 金额识别
#define BT_JSON_AMOUNT2         "AmountInFigures"   // 金额识别: 小写
#define BT_JSON_CHECKNO         "CheckNo"           // 票号识别结果
#define BT_JSON_ISTRUE          "isTrue"            // 票据真伪
#define BT_JSON_TYPE            "Type"              // 票据类型
#define BT_JSON_QRDATA          "QrCodeData"        // QR码识别结果

/*************************************************************
// RSC-D403M错误码(函数返回状态码)
*************************************************************/
#define SS_OK                                                   0       // 操作成功
#define SS_ERR_NOTSUPPORTED                                     -1      // 设备不支持此操作
#define SS_ERR_TIMEOUT                                          -2      // 操作超时
#define SS_ERR_HWERROR                                          -3      // 硬件出错
#define SS_ERR_INVALIDFILEPATH                                  -4      // 无效的文件路径
#define SS_ERR_INVALIDPARAMETER                                 -5      // 无效的参数
#define SS_ERR_DEVICECLOSED                                     -6      // 设备已关闭
#define SS_ERR_CANCELED                                         -7      // 操作被取消
#define SS_ERR_POCKETFULL                                       -8      // 票箱已满
#define SS_ERR_NOINKPRESENT                                     -9      // 墨盒不在位
#define SS_ERR_PARSEJSON                                        -10     // 解析Json参数错误
#define SS_ERR_LOADALGDLL                                       -20     // 加载算法库失败
#define SS_ERR_INITALG                                          -21     // 算法初始化接口失败
#define SS_ERR_SETBRIGHT                                        -22     // 设置图像亮度失败
#define SS_ERR_POCKETOPEN                                       -23     // 票箱开
#define SS_ERR_OTHER/*INITALG*/                                 -99999  // 其他错误
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
// 1. 打开设备
typedef int (*m_SS_CHK_Open)();
// 2. 关闭设备
typedef int (*m_SS_CHK_Close)();
// 3. 打开入票口
typedef int (*m_SS_CHK_Insert)();
// 4. 关闭入票口
typedef int (*m_SS_CHK_CancelInsert)();
// 6. 票据压箱
typedef int (*m_SS_CHK_Accept)();
// 7. 退出票据
typedef int (*m_SS_CHK_Eject)();
// 8. 设备复位(无动作)
typedef int (*m_SS_CHK_Reset)();
// 9. 设备复位(有动作)
typedef int (*m_SS_CHK_ResetEx)(int nAction);
// 10. 查询设备状态
typedef int (*m_SS_CHK_GetStatus)(LPSCANNERSTATUS lpStatus);
// 11. 获取固件版本号
typedef int (*m_SS_CHK_GetFWVer)(LPSTR lpVer);
// 12. 走纸到读取RFID位置
typedef int (*m_SS_CHK_MoveToRFID)();
// 13. 走纸到读取RFID位置(指定距离)
typedef int (*m_SS_CHK_MoveToRFIDLen)(int nLen);
// 14. 选择票箱
typedef int (*m_SS_CHK_SetBox)(DWORD dwBoxNo);
// 15. 单行打印
typedef int (*m_SS_CHK_Print)(LPSTR lpPrtData, int nX, int nY, LPSTR lpFont, int nWidth, int nHeight);
// 16. 多行打印
typedef int (*m_SS_CHK_PrintJson)(LPSTR lpPrtData);
// 17. 扫描并保存图像
typedef int (*m_SS_CHK_ScanAndGetImage)(LPSTR lpFrontImageName, LPSTR lpBackImageName,
                                        int *nType, int nTimeOut, int *nDirection);
// 18. 设置票面OCR识别区域
typedef int (*m_SS_CHK_SetCheckOCRArea)(LPSTR lpArea);
// 19. 获取票据鉴伪及OCR信息
typedef int (*m_SS_CHK_GetCheckAndOcrResult)(LPSTR lpResult, int nRetSize);
// 20. 设置鉴伪输入参数
typedef int (*m_SS_CHK_SetIdentifyInput)(LPSTR lpIdentifyInput);
// 21. 扫描并保存图像II
typedef int (*m_SS_CHK_ScanAndGetImageII)(LPSTR lpArea);
// 22. 获取票据背面OCR信息
typedef int (*m_SS_CHK_GetCheckAndOcrResultII)(LPSTR lpResult, int nRetSize);


/*************************************************************
// 封装类: 命令编辑、发送接收等处理。
*************************************************************/
class CDevImpl_RSCD400M : public CLogManage
{
public:
    CDevImpl_RSCD400M();
    CDevImpl_RSCD400M(LPSTR lpLog);
    virtual ~CDevImpl_RSCD400M();

public:
    INT     DeviceOpen(LPSTR lpMode);                       // 打开设备
    INT     DeviceClose(BOOL bUnLoad = TRUE);               // 关闭设备
    BOOL    IsDeviceOpen();                                 //
    CHAR*   ConvertErrCodeToStr(INT nRet);
    void    SetLibPath(LPCSTR lpPath);                      // 设置动态库路径(DeviceOpen前有效)

public: // 接口函数封装
    // 1. 打开设备
    INT nCHKOpen();
    // 2. 关闭设备
    INT nCHKClose();
    // 3. 打开入票口
    INT nCHKInsert();
    // 4. 关闭入票口
    INT nCHKCancelInsert();
    // 6. 票据压箱
    INT nCHKAccept();
    // 7. 退出票据
    INT nCHKEject();
    // 8. 设备复位(无动作)
    INT nCHKReset();
    // 9. 设备复位(有动作)
    INT nCHKResetEx(int nAction);
    // 10. 查询设备状态
    INT nCHKGetStatus(LPSCANNERSTATUS lpStatus);
    // 11. 获取固件版本号
    INT nCHKGetFWVer(LPSTR lpVer);
    // 12. 退票到读取RFID位置
    INT nCHKMoveToRFID();
    // 13. 退票到读取RFID位置(指定距离)
    INT nCHKMoveToRFIDLen(INT nLen);
    // 14. 选择票箱
    INT nCHKSetBox(DWORD dwBoxNo);
    // 15. 单行打印
    INT nCHKPrint(LPSTR lpPrtData, int nX, int nY, LPSTR lpFont, int nWidth, int nHeight);
    // 16. 多行打印
    INT nCHKPrintJson(LPSTR lpPrtData);
    // 17. 扫描并保存图像
    INT nCHKScanAndGetImage(LPSTR lpFrontImageName, LPSTR lpBackImageName, int *nType, int nTimeOut, int *nDirection);
    // 18. 设置票面OCR识别区域
    INT nCHKSetCheckOCRArea(LPSTR lpArea);
    // 19. 获取票据鉴伪及OCR信息
    INT nCHKGetCheckAndOcrResult(LPSTR lpResult, INT nRetSize);
    // 20. 设置鉴伪输入参数
    INT nCHK_SetIdentifyInput(LPSTR lpIdentifyInput);
    // 21. 扫描并保存图像II
    INT nCHK_ScanAndGetImageII(LPSTR lpArea);
    // 22. 获取票据背面OCR信息
    INT nCHK_GetCheckAndOcrResultII(LPSTR lpIdentifyResult, INT nRetSize);

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
    m_SS_CHK_Open                   SS_CHK_Open;                    // 1. 打开设备
    m_SS_CHK_Close                  SS_CHK_Close;                   // 2. 关闭设备
    m_SS_CHK_Insert                 SS_CHK_Insert;                  // 3. 打开入票口
    m_SS_CHK_CancelInsert           SS_CHK_CancelInsert;            // 4. 关闭入票口
    m_SS_CHK_Accept                 SS_CHK_Accept;                  // 6. 票据压箱
    m_SS_CHK_Eject                  SS_CHK_Eject;                   // 7. 退出票据
    m_SS_CHK_Reset                  SS_CHK_Reset;                   // 8. 设备复位(无动作)
    m_SS_CHK_ResetEx                SS_CHK_ResetEx;                 // 9. 设备复位(有动作)
    m_SS_CHK_GetStatus              SS_CHK_GetStatus;               // 10. 查询设备状态
    m_SS_CHK_GetFWVer               SS_CHK_GetFWVer;                // 11. 获取固件版本号
    m_SS_CHK_MoveToRFID             SS_CHK_MoveToRFID;              // 12. 走纸到读取RFID位置
    m_SS_CHK_MoveToRFIDLen          SS_CHK_MoveToRFIDLen;           // 13. 走纸到读取RFID位置(指定距离)
    m_SS_CHK_SetBox                 SS_CHK_SetBox;                  // 14. 选择票箱
    m_SS_CHK_Print                  SS_CHK_Print;                   // 15. 单行打印
    m_SS_CHK_PrintJson              SS_CHK_PrintJson;               // 16. 多行打印
    m_SS_CHK_ScanAndGetImage        SS_CHK_ScanAndGetImage;         // 17. 扫描并保存图像
    m_SS_CHK_SetCheckOCRArea        SS_CHK_SetCheckOCRArea;         // 18. 设置票面OCR识别区域
    m_SS_CHK_GetCheckAndOcrResult   SS_CHK_GetCheckAndOcrResult;    // 19. 获取票据鉴伪及OCR信息
    m_SS_CHK_SetIdentifyInput       SS_CHK_SetIdentifyInput;        // 20. 设置鉴伪输入参数
    m_SS_CHK_ScanAndGetImageII      SS_CHK_ScanAndGetImageII;       // 21. 扫描并保存图像II
    m_SS_CHK_GetCheckAndOcrResultII SS_CHK_GetCheckAndOcrResultII;  // 22. 获取票据背面OCR信息

private:
    CSimpleMutex                    m_MutexAction;
    CHAR                            m_szErrStr[1024];
    INT                             m_nGetStatErrOLD;       // 取状态接口上一次错误码
    INT                             m_nGetOpenErrOLD;
};


#endif // DEVIMPL_RSCD400M_H
