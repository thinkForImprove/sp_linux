/***************************************************************
* 文件名称: DevImpl_BSID81.h
* 文件描述: 身份证模块底层指令, 提供控制接口 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年3月25日
* 文件版本: 1.0.0.1
****************************************************************/

#ifndef DEVIMPL_BSID81_H
#define DEVIMPL_BSID81_H

#include <string>

#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "libID81_SO.h"

/*************************************************************
// 返回值/错误码　宏定义
*************************************************************/
// Impl处理返回
#define IMP_SUCCESS                     0           // 成功
#define IMP_ERR_LOAD_LIB                -1          // 动态库加载失败
#define IMP_ERR_PARAM_INVALID           -2          // 参数无效
#define IMP_ERR_UNKNOWN                 -3          // 未知错误
#define IMP_ERR_NOTOPEN                 -4          // 设备未Open
// Device返回
#define	IMP_ERR_DEV_00H                 0x00        // 正常
#define	IMP_ERR_DEV_01H                 0x01        // 无设备
#define	IMP_ERR_DEV_02H                 0x02        // 端口错误
#define	IMP_ERR_DEV_03H                 0x03        // 参数文件错误
#define	IMP_ERR_DEV_04H                 0x04        // 未初始化
#define	IMP_ERR_DEV_05H                 0x05        // 无效参数
#define	IMP_ERR_DEV_06H                 0x06        // 超时错误
#define	IMP_ERR_DEV_07H                 0x07        // 上盖打开
#define	IMP_ERR_DEV_08H                 0x08        // 塞卡
#define	IMP_ERR_DEV_09H                 0x09        // 内存溢出
#define	IMP_ERR_DEV_0AH                 0x0A        // 没有二代证数据
#define	IMP_ERR_DEV_0BH                 0x0B        // 没有图像数据
#define	IMP_ERR_DEV_0CH                 0x0C        // 图像处理错误
#define	IMP_ERR_DEV_0DH                 0x0D        // 判断图像方向错误
#define IMP_ERR_DEV_0EH                 0x0E        // 关闭端口失败
#define	IMP_ERR_DEV_0FH                 0x0F        // 身份证电子信息处理错误
#define IMP_ERR_DEV_10H                 0x10        // 传感器校验错误
#define IMP_ERR_DEV_11H                 0x11        // 电压低
#define IMP_ERR_DEV_12H                 0x12        // 校正错误
#define IMP_ERR_DEV_13H                 0x13        // 无卡
#define IMP_ERR_DEV_14H                 0x14        // 未知错误
#define IMP_ERR_DEV_15H                 0x15        // 保存位图错误
#define	IMP_ERR_DEV_16H                 0x16        // 掉电错误
#define	IMP_ERR_DEV_17H                 0x17        // BOOT错误
#define	IMP_ERR_DEV_18H                 0x18        // 按键抬起
#define	IMP_ERR_DEV_19H                 0x19        // 识别错误
#define IMP_ERR_DEV_1AH                 0x1A        // 扫描错误
#define IMP_ERR_DEV_1BH                 0x1B        // 走卡错误
#define	IMP_ERR_DEV_1CH                 0x1C        // 最大错误码


/*************************************************************************
// 宏定义
*************************************************************************/
#define DLL_DEVLIB_NAME  "libID81_SO.so"        // 缺省动态库名
#define LOG_NAME         "DevImpl_BSID81.log"   // 缺省日志名

#define SAVE_IMG_MAX_SIZE   1024 * 1024 * 16    // BSID81生成图像数据最大16M

#define SAVE_IMG_BMP        0
#define SAVE_IMG_JPG        1
#define SAVE_IMG_BMP_X      2
#define SAVE_IMG_JPG_X      3

#define SAVE_IMG_BMP_S      "BMP"
#define SAVE_IMG_JPG_S      "JPG"
#define SAVE_IMG_BMP_SX     "bmp"
#define SAVE_IMG_JPG_SX     "jpg"

typedef unsigned BSDEVICEID;        // 定义设备ID类型

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

/**************************************************************************
* 命令编辑、发送接收等处理                                                    *
***************************************************************************/
class CDevImpl_BSID81 : public CLogManage
{
public:
    CDevImpl_BSID81();
    CDevImpl_BSID81(LPSTR lpLog);
    virtual ~CDevImpl_BSID81();
public:
    INT OpenDevice();
    INT OpenDevice(WORD wType);
    INT CloseDevice();
    BOOL IsDeviceOpen();

public: // 接口函数封装
    // 1. 枚举扫描设备
    INT nEnumScannerDevice();
    // 2. 打开设备
    INT nOpenConnection();
    // 3. 关闭设备
    INT nCloseConnection();
    // 4. 检测是否放入卡
    INT nCheckHaveIdCard(UINT unChkTime = 500);
    // 5. 检测是否被取走
    /*INT nTakeOutIdCard();*/
    // 6. 启动扫描
    INT nStartScanIdCard();
    // 7. 读取当前图像数据块到内存
    INT nSavePicToMemory(LPSTR lpFrontImgBuf, LPSTR lpRearImgBuf, LPINT lpFrontLen, LPINT lpRearLen);
    // 8.1 保存图像数据块到文件
    INT nSavePicToFile(LPSTR lpImgBuf, UINT uBufLen, LPSTR lpFileName, INT Format);
    // 8.2 保存图像数据块到文件(指定缩放比例)
    INT nSavePicToFileII(LPSTR lpImgBuf, UINT uBufLen, LPSTR lpFileName, INT Format, FLOAT fZoomScale);
    // 9. 吞卡
    INT nRetainIdCard();
    // 10. 退卡
    INT nBackIdCard();
    // 11. 出卡并持卡
    INT nBackAndHoldIdCard();
    // 12.1 获取二代证信息(国内)
    INT nGetID2Info(IDInfo &stIDCard, LPSTR lpHeadImageName);
    // 12.2 获取二代证信息(国内,带指纹)
    INT nGetID2InfoEx(IDInfoEx &stIDCard, LPSTR lpHeadImageName);
    // 12.3 获取二代证信息(国外)
    INT nGetIDInfoForeign(IDInfoForeign &stIDCard, LPSTR lpHeadImageName);
    // 12.4 获取二代证信息(港澳台通行证)
    INT nGetIDInfoGAT(IDInfoGAT &stIDCard, LPSTR lpHeadImageName);
    // 12.5 获取二代证信息(卡类型)
    INT nGetIDCardType(LPINT lpCardType);
    // 15. 获取固件版本信息
    INT nGetFWVersion(LPSTR lpFwVer);
    // 16. 获取软件版本信息
    INT nGetSWVersion(LPSTR lpSwVer);
    // 17. 获取设备状态
    INT nGetDevStatus(DEVSTATUS &devStatus);
    // 18. 复位设备
    INT nResetDevice();
    // 19. 软复位
    INT nSoftResetDevice(INT nMode);
    // 29. 退卡到可识别位置
    INT nBackIdCardToRerec();

private:
    BSDEVICEID      m_bsDeviceId;
    BOOL            m_bDevOpenOk;
    CHAR            m_szErrStr[1024];
    DEVSTATUS       m_stOldDevStatus;

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
                                                                    //  2:设备状态/3介质状态/4枚举设备)

private: // 动态库接口定义
    mEnumScannerDevice          EnumScannerDevice;          // 1. 枚举扫描设备
    mOpenConnection             OpenConnection;             // 2. 打开设备
    mCloseConnection            CloseConnection;            // 3. 关闭设备
    mCheckHaveIdCard            CheckHaveIdCard;            // 4. 检测是否放入卡
    mTakeOutIdCard              TakeOutIdCard;              // 5. 检测是否被取走
    mStartScanIdCard            StartScanIdCard;            // 6. 启动扫描
    mSavePicToMemory            SavePicToMemory;            // 7. 读取当前图像数据块到内存
    mSavePicToFile              SavePicToFile;              // 8.1 保存图像数据块到文件
    mSavePicToFileII            SavePicToFileII;            // 8.2 保存图像数据块到文件(指定缩放比例)
    mRetainIdCard               RetainIdCard;               // 9. 吞卡
    mBackIdCard                 BackIdCard;                 // 10. 退卡
    mBackAndHoldIdCard          BackAndHoldIdCard;          // 11. 出卡并持卡
    mGetID2Info                 GetID2Info;                 // 12.1 获取二代证信息(国内)
    mGetID2InfoEx               GetID2InfoEx;               // 12.2 获取二代证信息(国内,带指纹)
    mGetIDInfoForeign           GetIDInfoForeign;           // 12.3 获取二代证信息(国外)
    mGetIDInfoGAT               GetIDInfoGAT;               // 12.4 获取二代证信息(港澳台通行证)
    mGetIDCardType              GetIDCardType;              // 12.5 获取二代证信息
    mGetAllTypeIdInfo           GetAllTypeIdInfo;           // 12.6 获取二代证信息
    mGetLastErrorCode           GetLastErrorCode;           // 13. 获取最近一次的错误码
    mGetLastErrorStr            GetLastErrorStr;            // 14. 获取最近一次的错误描述
    mGetFWVersion               GetFWVersion;               // 15. 获取固件版本信息
    mGetSWVersion               GetSWVersion;               // 16. 获取软件版本信息
    mGetDevStatus               GetDevStatus;               // 17. 获取设备状态
    mResetDevice                ResetDevice;                // 18. 复位设备
    mSoftResetDevice            SoftResetDevice;            // 19. 软复位
    mUpdateOnLine               UpdateOnLine;               // 20. 固件升级
    mCISCalibrate               CISCalibrate;               // 21. CIS校验
    mSensorCalibrate            SensorCalibrate;            // 22. 传感器校验
    mSetButtonEnable            SetButtonEnable;            // 23. 设置按键强制退卡使能
    mSetAutoFeedEnable          SetAutoFeedEnable;          // 24. 设置自动进卡使能
    mSetInitFeedMode            SetInitFeedMode;            // 25. 设置上电、复位初始化吸卡模式
    mSetHeadFileFormat          SetHeadFileFormat;          // 26. 设置二代证芯片图像存储格式
    mGetID2InfoFromImage        GetID2InfoFromImage;        // 27. 通过图像获取通行证信息
    mGetPassportInfoFromImage   GetPassportInfoFromImage;   // 28. 从图像获取二代证信息
    mBackIdCardToRerec          BackIdCardToRerec;          // 29. 退卡到识别位置
};


#endif // DEVIMPL_BSID81_H
