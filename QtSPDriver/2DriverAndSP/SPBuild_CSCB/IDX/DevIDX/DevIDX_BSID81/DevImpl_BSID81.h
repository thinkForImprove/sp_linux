//#pragma once
//#define LINUX
#ifndef DEVIMPL_BSID81_H
#define DEVIMPL_BSID81_H

#include <string>

#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "libID81_SO.h"


#define DLL_DEVLIB_NAME  "libID81_SO.so"        // 缺省动态库名
#define LOG_NAME         "DevImpl_BSID81.log"   // 缺省日志名

#define SAVE_IMG_MAX_SIZE   1024 * 1024 * 16    // BSID81生成图像数据最大16M

#define SAVE_IMG_BMP    0
#define SAVE_IMG_JPG    1

#define SAVE_IMG_BMP_S  "BMP"
#define SAVE_IMG_JPG_S  "JPG"

typedef unsigned BSDEVICEID;     // 定义设备ID类型

// 卡动作
#define CARD_EJECT        0  // 退卡
#define CARD_RETRACT      1  // 吞卡
#define CARD_EJECTMENT    2  // 退卡并保持
#define CARD_NOACTION     3  // 无动作

// 证件类型
#define ID_CHINA        0   // 国内证件
#define ID_FOREIGN      1   // 国外证件
#define ID_GAT          2   // 港澳台证件

#define FUNC_POINTER_ERROR_RETURN(LPFUNC, LPFUNC2) \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

/*
命令编辑、发送接收等处理。
*/

class CDevImpl_BSID81 : public CLogManage
{
public:
    CDevImpl_BSID81();
    CDevImpl_BSID81(LPSTR lpLog);
    virtual ~CDevImpl_BSID81();
public:
    BOOL OpenDevice();
    BOOL OpenDevice(WORD wType);
    BOOL CloseDevice();
    BOOL IsDeviceOpen();
    void SetDevOpenIsF();
    INT GetErrCode(BOOL bIsStat = FALSE);
    LPSTR GetErrorStr(LONG lErrCode);

public: // 接口函数封装
    // 1. 枚举扫描设备
    BOOL bEnumScannerDevice();
    // 2. 打开设备
    BOOL bOpenConnection();
    // 3. 关闭设备
    BOOL bCloseConnection();
    // 4. 检测是否放入卡
    BOOL bCheckHaveIdCard(UINT unChkTime = 500);
    // 5. 检测是否被取走
    /*BOOL bTakeOutIdCard();*/
    // 6. 启动扫描
    BOOL bStartScanIdCard();
    // 7. 读取当前图像数据块到内存
    BOOL bSavePicToMemory(LPSTR lpFrontImgBuf, LPSTR lpRearImgBuf, LPINT lpFrontLen, LPINT lpRearLen);
    // 8.1 保存图像数据块到文件
    BOOL bSavePicToFile(LPSTR lpImgBuf, UINT uBufLen, LPSTR lpFileName, INT Format);
    // 8.2 保存图像数据块到文件(指定缩放比例)
    BOOL bSavePicToFileII(LPSTR lpImgBuf, UINT uBufLen, LPSTR lpFileName, INT Format, FLOAT fZoomScale);
    // 9. 吞卡
    BOOL bRetainIdCard();
    // 10. 退卡
    BOOL bBackIdCard();
    // 11. 出卡并持卡
    BOOL bBackAndHoldIdCard();
    // 12.1 获取二代证信息(国内)
    BOOL bGetID2Info(IDInfo &stIDCard, LPSTR lpHeadImageName);
    // 12.2 获取二代证信息(国内,带指纹)
    BOOL bGetID2InfoEx(IDInfoEx &stIDCard, LPSTR lpHeadImageName);
    // 12.3 获取二代证信息(国外)
    BOOL bGetIDInfoForeign(IDInfoForeign &stIDCard, LPSTR lpHeadImageName);
    // 12.4 获取二代证信息(港澳台通行证)
    BOOL bGetIDInfoGAT(IDInfoGAT &stIDCard, LPSTR lpHeadImageName);
    // 12.5 获取二代证信息(卡类型)
    BOOL bGetIDCardType(LPINT lpCardType);
    // 15. 获取固件版本信息
    BOOL bGetFWVersion(LPSTR lpFwVer);
    // 16. 获取软件版本信息
    BOOL bGetSWVersion(LPSTR lpSwVer);
    // 17. 获取设备状态
    BOOL bGetDevStatus(DEVSTATUS &devStatus);
    // 18. 复位设备
    BOOL bResetDevice();
    // 19. 软复位
    BOOL bSoftResetDevice(INT nMode);
    // 29. 退卡到可识别位置
    BOOL bBackIdCardToRerec();

private:
    BSDEVICEID      m_bsDeviceId;
    BOOL            m_bDevOpenOk;
    CHAR            m_szErrStr[1024];
    //ScannerInfoRec  ScannerInfo[8];
    DEVSTATUS       m_stOldDevStatus;
    //DEVSTATUS       m_stNewDevStatus;

private:
    void Init();

private: // 接口加载
    BOOL bLoadLibrary();
    void vUnLoadLibrary();
    BOOL bLoadLibIntf();

private: // 接口加载
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;
    INT         m_nDevErrCode;
    INT         m_nStatErrCode;


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
