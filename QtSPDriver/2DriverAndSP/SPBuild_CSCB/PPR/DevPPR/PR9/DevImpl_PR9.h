/***************************************************************
* 文件名称：DevImpl_CRT780B.h
* 文件描述：声明CRT780B退卡模块底层控制指令接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

/***************************************************************
* 文件名称：DevImpl_PR9.h
* 文件描述：新北洋BT-8500M票据模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月27日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEVIMPL_PR9_H
#define DEVIMPL_PR9_H


#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "IAllDevPort.h"

#define DLL_DEVLIB_NAME  "libBT-8500M_API.so"          // 缺省动态库名
#define LOG_NAME         "DevImpl_WBT2172.log"   // 缺省日志名


/*************************************************************
// BT-8500M错误码
*************************************************************/
#define PRINTER_SUCCESS                     0   // 正常
#define PRINTER_ERROR_OPEN_PORT             1   // 打开端口失败
#define PRINTER_ERROR_COMMUNICATION         2   // 端口通讯失败
#define PRINTER_ERROR_PAPERLACK             3   // 缺纸
#define PRINTER_ERROR_BELTLACK              4   // 缺色带
#define PRINTER_ERROR_FEEDER_OPEN           5   // 分纸模块微动开关错
#define PRINTER_ERROR_DOUBLE_DOC            6   // 重张
#define PRINTER_ERROR_BELTOVER              7   // 碳带将近
#define PRINTER_ERROR_TEMPVOL               8   // 温度电压异常
#define PRINTER_ERROR_PAPERINCOMPLETE       9   // 票据缺角
#define PRINTER_ERROR_PRINTMICROSWITCH      10  // 打印模块微动开关错
#define PRINTER_ERROR_PAPERSIZE             11  // 票据长度错误
#define PRINTER_ERROR_HEADDOWN              12  // 打印头压下错误
#define PRINTER_ERROR_TIME_OUT              13  // 取票超时
#define PRINTER_ERROR_PAPERJAM              14  // 卡纸错
#define PRINTER_ERROR_GATE                  15  // 闸门错
#define PRINTER_ERROR_BELTRECYCLE           16  // 碳带回收错
#define PRINTER_ERROR_OUTPUTBOX_OPEN        17  // 出票箱上盖抬起
#define PRINTER_ERROR_CALIBRATE             18  // 扫描模块校正失败
#define PRINTER_ERROR_SCANVOLHIGH           19  // 扫描模块电压偏高
#define PRINTER_ERROR_SCANVOLLOW            20  // 扫描模块电压偏低
#define PRINTER_ERROR_FEEDER                21  // 分纸错
#define PRINTER_ERROR_DOWNLOADIMAGE         22  // 下载位图失败
#define PRINTER_ERROR_STOPPING              23  // 打印机暂停中
#define PRINTER_ERROR_RFID                  24  // 获取射频信息失败
#define PRINTER_ERROR_NOT_IDLE              25  // 设备非空闲状态
#define PRINTER_ERROR_NOT_WAIT_PAPEROUT     26  // 设备不在出纸等待状态
#define PRINTER_ERROR_PAPER_CORRECTION      27  // 设备纠偏错
#define PRINTER_ERROR_CHANNEL_ERROR         28  // 通道抬起压下错误
#define PRINTER_ERROR_SCAN_UNIT_OPEN        29  // 扫描鉴伪模块上盖打开
#define PRINTER_ERROR_OCR_COVER_OPEN        30  // 票号识别模块上盖打开
#define PRINTER_ERROR_SENSOR_CALIBRATE      31  // 传感器校验失败
#define PRINTER_ERROR_OUTPUTBOX_PRESSURE    32  // 发票箱压板错误
#define PRINTER_ERROR_STAMP                 33  // 印章错误
#define PRINTER_ERROR_SCAN_UNIT             34  // 扫描模块错误
#define PRINTER_ERROR_UNKNOWN               35  // 未知错误
#define PRINTER_ERROR_FILE_R_W              101 // 文件读写错误
#define PRINTER_ERROR_INIFILE               102 // 读取配置文件错误
#define PRINTER_ERROR_OUTOFMEMORY           103 // 内存不足
#define PRINTER_ERROR_PARAM                 104 // 接口传入参数错误
#define PRINTER_ERROR_JPEG_COMPRESS         105 // JPEG压缩错误
#define PRINTER_ERROR_IMAGE_DESKEW          106 // 裁剪纠偏错误


/*************************************************************
// 返回值/错误码　宏定义
// <0 : Impl处理返回
// 0~100: 硬件设备返回
*************************************************************/
// <0: Impl处理返回
#define IMP_SUCCESS             0       // 成功
#define IMP_ERR_LOAD_LIB        -1      // 动态库加载失败
#define IMP_ERR_PARAM_INVALID   -2      // 参数无效
#define IMP_ERR_UNKNOWN         -3      // 未知错误
// >0: Device返回
#define IMP_ERR_OPEN_PORT             PRINTER_ERROR_OPEN_PORT
#define IMP_ERR_COMMUNICATION         PRINTER_ERROR_COMMUNICATION
#define IMP_ERR_PAPERLACK             PRINTER_ERROR_PAPERLACK
#define IMP_ERR_BELTLACK              PRINTER_ERROR_BELTLACK
#define IMP_ERR_FEEDER_OPEN           PRINTER_ERROR_FEEDER_OPEN
#define IMP_ERR_DOUBLE_DOC            PRINTER_ERROR_DOUBLE_DOC
#define IMP_ERR_BELTOVER              PRINTER_ERROR_BELTOVER
#define IMP_ERR_TEMPVOL               PRINTER_ERROR_TEMPVOL
#define IMP_ERR_PAPERINCOMPLETE       PRINTER_ERROR_PAPERINCOMPLETE
#define IMP_ERR_PRINTMICROSWITCH      PRINTER_ERROR_PRINTMICROSWITCH
#define IMP_ERR_PAPERSIZE             PRINTER_ERROR_PAPERSIZE
#define IMP_ERR_HEADDOWN              PRINTER_ERROR_HEADDOWN
#define IMP_ERR_TIME_OUT              PRINTER_ERROR_TIME_OUT
#define IMP_ERR_PAPERJAM              PRINTER_ERROR_PAPERJAM
#define IMP_ERR_GATE                  PRINTER_ERROR_GATE
#define IMP_ERR_BELTRECYCLE           PRINTER_ERROR_BELTRECYCLE
#define IMP_ERR_OUTPUTBOX_OPEN        PRINTER_ERROR_OUTPUTBOX_OPEN
#define IMP_ERR_CALIBRATE             PRINTER_ERROR_CALIBRATE
#define IMP_ERR_SCANVOLHIGH           PRINTER_ERROR_SCANVOLHIGH
#define IMP_ERR_SCANVOLLOW            PRINTER_ERROR_SCANVOLLOW
#define IMP_ERR_FEEDER                PRINTER_ERROR_FEEDER
#define IMP_ERR_DOWNLOADIMAGE         PRINTER_ERROR_DOWNLOADIMAGE
#define IMP_ERR_STOPPING              PRINTER_ERROR_STOPPING
#define IMP_ERR_RFID                  PRINTER_ERROR_RFID
#define IMP_ERR_NOT_IDLE              PRINTER_ERROR_NOT_IDLE
#define IMP_ERR_NOT_WAIT_PAPEROUT     PRINTER_ERROR_NOT_WAIT_PAPEROUT
#define IMP_ERR_PAPER_CORRECTION      PRINTER_ERROR_PAPER_CORRECTION
#define IMP_ERR_CHANNEL_ERROR         PRINTER_ERROR_CHANNEL_ERROR
#define IMP_ERR_SCAN_UNIT_OPEN        PRINTER_ERROR_SCAN_UNIT_OPEN
#define IMP_ERR_OCR_COVER_OPEN        PRINTER_ERROR_OCR_COVER_OPEN
#define IMP_ERR_SENSOR_CALIBRATE      PRINTER_ERROR_SENSOR_CALIBRATE
#define IMP_ERR_OUTPUTBOX_PRESSURE    PRINTER_ERROR_OUTPUTBOX_PRESSURE
#define IMP_ERR_STAMP                 PRINTER_ERROR_STAMP
#define IMP_ERR_SCAN_UNIT             PRINTER_ERROR_SCAN_UNIT
#define IMP_ERR_UNKNOWN               PRINTER_ERROR_UNKNOWN
#define IMP_ERR_FILE_R_W              PRINTER_ERROR_FILE_R_W
#define IMP_ERR_INIFILE               PRINTER_ERROR_INIFILE
#define IMP_ERR_OUTOFMEMORY           PRINTER_ERROR_OUTOFMEMORY
#define IMP_ERR_PARAM                 PRINTER_ERROR_PARAM
#define IMP_ERR_JPEG_COMPRESS         PRINTER_ERROR_JPEG_COMPRESS
#define IMP_ERR_IMAGE_DESKEW          PRINTER_ERROR_IMAGE_DESKEW


/*************************************************************
// 无分类　宏定义
*************************************************************/
#define LOG_NAME         "DevImpl_PR9.log"   // 缺省日志名

//
#define FUNC_POINTER_ERROR_RETURN(LPFUNC, LPFUNC2) \
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



/*************************************************************
// 结构体定义
*************************************************************/
// 设备获取状态返回结构体
typedef struct DeviceStatus
{
    int iSize;//结构体大小，便于扩展
    int bError;//错误状态
    int bIdle;//空闲
    int bWaitForScan;//等待启动扫描
    int bPrinting;//正在打印
    int bScanning_OCR;//OCR扫描过程
    int bScanning_Identify;//多光谱扫描过程
    int bWaitForPrinting;//等待OCR识别结果,可以启动打印
    int bWaitForPaperOut;//等待出票
    int bDeviceInPause;//暂停状态

    int bPaperJamError;//塞纸错误
    int bUnitVolTooHigh;//整机电压偏高
    int bUnitVolTooLow;//整机电压偏低
    int bFeederOpen;//分纸模块上盖打开
    int bPrintHeadOpen;//打印模块微动开关错
    int bChannelError;//通道抬起压下错误
    int bScanUnitOpen;//扫描模块上盖打开
    int bFeedingError;//分纸错误
    int bCommunicationError;//扫描模块通讯错
    int bPaperLengthError;//票据长度错
    int bDoubleDocError;//重张错
    int bCorrectionError;//纠偏错
    int bBeltLackError;//缺碳带错
    int bTempVolError;//温度电压异常错
    int bBeltRecycleError;//碳带回收错
    int bBeltOverError;//碳带将尽错
    int bPrinterHeadError;//打印头抬起压下错误
    int bGateError;//闸门错
    int bPaperIncompleteError;//缺角错
    int bOutputBoxOpen;//出票箱是否打开
    int bRetrackError;//废票箱是否取出

    int bOCRCoverOpen;//票号扫描位置上盖打开
    int bSensorCalibrateError;//传感器校验是否失败
    int bOutputBoxPressureError;//回收压板错误
    int bStampError;//印章错
    int bScanUnitError;//扫描模块状态错

    int bBox1HavePaper;//票箱1是否有票
    int bBox2HavePaper;//票箱2是否有票
    int bBox3HavePaper;//票箱3是否有票
    int bBox4HavePaper;//票箱4是否有票

    int bBox1Exist;//票箱1是否有票
    int bBox2Exist;//票箱2是否有票
    int bBox3Exist;//票箱3是否有票
    int bBox4Exist;//票箱4是否有票

    int bChannelHasPaper;//通道有纸
    int bStackerHavePaper;//出票箱是否有纸
    int bGateOpen;//闸门开

    int bHaveData;//有数据
    int bLastBulk;//最后一块数据
    unsigned long lDataLength;//图像数据的长度
    int bRetrackHavePaper;		//回收箱是否有纸
} DEVICESTATUS;

/*************************************************************
// 动态库输出函数　定义
*************************************************************/
// 1. 打开设备
typedef int (*mOpenDevice)();
// 2. 关闭设备
typedef int (*mCloseDevice)();
// 3. 启动分纸
typedef int (*mFeedCheck)(int iCheckBox,int iPaperType);
// 4. 获取票号
typedef int (*mGetCheckNum)(char* pchCheckNo,char* filename);
// 5. 启动打印和扫描
typedef int (*mPrintAndScan)(char* pPrintContent,int bScanEnabled,int iPrintNext);
// 6. 闸门开关控制
typedef int (*mOutPaperDoorControl)(int nCmd);
// 7. 鉴伪/保存图像
typedef int (*mGetIdentifyInfo)(int* iIdentifyResult,char* cIdentifyInfo,int bufferlen,char* cFrontImageFileName,  char* cRearImageFileName);
// 8. 查询设备状态
typedef int (*mGetDevStatus)(DEVICESTATUS* mdevicestatus);
// 9. 打开闸门取支票
typedef int (*mTakeCheck)(int iTimeout);
// 10. 出票或退票，退票时可选择是否盖章
typedef int (*mSetCheckOut)(int mode,int bStamp);
// 11. 设备复位
typedef int (*mResetDev)();
// 12. 获取固件版本号
typedef int (*mGetFirmwareVersion)(char *version, int bufSize);
// 13. 固件升级
typedef int (*mUpdateFirmware)(char *fileName);
// 14. 获取最后一个错误码
typedef int (*mGetLastErrorCode)();
// 15. 可将通道中的票据走出,回收时可选择是否盖章
typedef int (*mMoveCheck)(int mode,int bStamp);
// 16. 遗忘回收
typedef int (*mRetractCheck)();
// 17. 设置票号OCR识别区域
typedef int (*mSetCheckNumOCRArea)(char * pchArea);
// 18. 获取票号OCR识别结果
typedef int (*mGetCheckNumFromArea)(char * pchOCR, int nSize, char* filename);
// 19. 获取票据的正反面图像
typedef int (*mGetCheckImage)(char * pchFrontImageFile, char * pchBackImageFile);
// 20. 设置票面OCR识别区域
typedef int (*mSetCheckOCRArea)(char * pchArea);
// 21. 获取票据鉴伪及OCR信息
typedef int (*mGetCheckInfo)(char * pchInfo, int nSize);
// 22.
typedef int (*mGetCheckNumImage)(char* filename);
// 23.
typedef int (*mGetRfidInfo)(char* pchRfidInfo);
// 24.
typedef int (*mReadCommand)(int relength, unsigned char* buffer, int* length);
// 25.
typedef int (*mSendCommand)(char* buffer,int length);


/*************************************************************
// 封装类: 命令编辑、发送接收等处理。
*************************************************************/
class CDevImpl_PR9 : public CLogManage
{
public:
    CDevImpl_PR9();
    CDevImpl_PR9(LPSTR lpLog);
    virtual ~CDevImpl_PR9();

public:
    INT     DeviceOpen(LPSTR lpMode);                       // 打开设备
    INT     DeviceClose();                                  // 关闭设备
    BOOL    IsDeviceOpen();                                 //
    INT     ConvertErrorCode(INT nRet);                     // 转换为Impl返回码/错误码
    CHAR*   ConvertErrCodeToStr(long lRet);

public: // 接口函数封装
    INT     nFeedCheck(INT nCheckBox, INT nPaperType);                              // 3. 启动分纸
    INT     nGetCheckNum(LPSTR lpCheckNo, LPSTR lpFilename);                        // 4. 获取票号
    INT     nPrintAndScan(LPSTR lpPrINTContent, INT bScanEnabled, INT nPrINTNext);  // 5. 启动打印和扫描
    INT     nOutPaperDoorControl(INT nCmd);                                         // 6. 闸门开关控制
    INT     nGetIdentifyInfo(INT *iIdentifyResult, LPSTR lpIdentifyInfo, INT nBufferlen,
                             LPSTR lpFrontImageFileName,  LPSTR lpRearImageFileName);// 7. 鉴伪/保存图像
    INT     nGetDevStatus(DEVICESTATUS &stDevicestatus);                            // 8. 查询设备状态
    INT     nTakeCheck(INT nTimeout);                                               // 9. 打开闸门取支票
    INT     nSetCheckOut(INT nMode, INT nStamp);                                    // 10. 出票或退票，退票时可选择是否盖章
    INT     nResetDev();                                                            // 11. 设备复位
    INT     nGetFirmwareVersion(LPSTR lpVersion, INT nBufSize);                     // 12. 获取固件版本号
    INT     nUpdateFirmware(LPSTR lpFileName);                                      // 13. 固件升级
    INT     nGetLastErrorCode();                                                    // 14. 获取最后一个错误码
    INT     nMoveCheck(INT nMode, INT nStamp);                                      // 15. 可将通道中的票据走出,回收时可选择是否盖章
    INT     nRetractCheck();                                                        // 16. 遗忘回收
    INT     nSetCheckNumOCRArea(LPSTR  lpChArea);                                   // 17. 设置票号OCR识别区域
    INT     nGetCheckNumFromArea(LPSTR  lpOCR, INT nSize, LPSTR lpFilename);        // 18. 获取票号OCR识别结果
    INT     nGetCheckImage(LPSTR  lpFrontImageFile, LPSTR  lpBackImageFile);        // 19. 获取票据的正反面图像
    INT     nSetCheckOCRArea(LPSTR  lpArea);                                        // 20. 设置票面OCR识别区域
    INT     nGetCheckInfo(LPSTR  lpInfo, INT nSize);                                // 21. 获取票据鉴伪及OCR信息
    INT     nGetCheckNumImage(LPSTR lpFilename);                                    // 22.
    INT     nGetRfidInfo(LPSTR lpRfidInfo);                                         // 23.
    INT     nReadCommand(INT nRelength, LPBYTE lpBuffer, INT* nLength);             // 24.
    INT     nSendCommand(LPSTR lpBuffer, INT nLength);                              // 25.
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
    mOpenDevice             OpenDevice;             // 1. 打开设备
    mCloseDevice            CloseDevice;            // 2. 关闭设备
    mFeedCheck              FeedCheck;              // 3. 启动分纸
    mGetCheckNum            GetCheckNum;            // 4. 获取票号
    mPrintAndScan           PrintAndScan;           // 5. 启动打印和扫描
    mOutPaperDoorControl    OutPaperDoorControl;    // 6. 闸门开关控制
    mGetIdentifyInfo        GetIdentifyInfo;        // 7. 鉴伪/保存图像
    mGetDevStatus           GetDevStatus;           // 8. 查询设备状态
    mTakeCheck              TakeCheck;              // 9. 打开闸门取支票
    mSetCheckOut            SetCheckOut;            // 10. 出票或退票，退票时可选择是否盖章
    mResetDev               ResetDev;               // 11. 设备复位
    mGetFirmwareVersion     GetFirmwareVersion;     // 12. 获取固件版本号
    mUpdateFirmware         UpdateFirmware;         // 13. 固件升级
    mGetLastErrorCode       GetLastErrorCode;       // 14. 获取最后一个错误码
    mMoveCheck              MoveCheck;              // 15. 可将通道中的票据走出,回收时可选择是否盖章
    mRetractCheck           RetractCheck;           // 16. 遗忘回收
    mSetCheckNumOCRArea     SetCheckNumOCRArea;     // 17. 设置票号OCR识别区域
    mGetCheckNumFromArea    GetCheckNumFromArea;    // 18. 获取票号OCR识别结果
    mGetCheckImage          GetCheckImage;          // 19. 获取票据的正反面图像
    mSetCheckOCRArea        SetCheckOCRArea;        // 20. 设置票面OCR识别区域
    mGetCheckInfo           GetCheckInfo;           // 21. 获取票据鉴伪及OCR信息
    mGetCheckNumImage       GetCheckNumImage;       // 22.
    mGetRfidInfo            GetRfidInfo;            // 23.
    mReadCommand            ReadCommand;            // 24.
    mSendCommand            SendCommand;            // 25.

private:
    CSimpleMutex                    m_MutexAction;
    CHAR                            m_szErrStr[1024];
};


#endif // DEVIMPL_PR9_H
