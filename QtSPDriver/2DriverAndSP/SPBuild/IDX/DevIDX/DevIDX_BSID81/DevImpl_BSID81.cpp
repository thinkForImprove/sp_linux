#include "DevImpl_BSID81.h"
#include "data_convertor.h"

static const char *ThisFile = "DevImpl_BSID81.cpp";

CDevImpl_BSID81::CDevImpl_BSID81()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_BSID81::CDevImpl_BSID81(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_BSID81::Init()
{
    m_bsDeviceId = 0;

    m_bDevOpenOk = FALSE;

    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("IDX/BSID81/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    //m_LoadLibrary.setFileName(QString(m_szLoadDllPath));//strDllName);
    m_wLoadDllVer = 0;
    m_bLoadIntfFail = TRUE;

    //memset(ScannerInfo, 0x00, sizeof(ScannerInfo));
    memset(&m_stOldDevStatus, 0x00, sizeof(DEVSTATUS));
    //memset(&m_stNewDevStatus, 0x00, sizeof(DEVSTATUS));

    m_nDevErrCode =  m_nStatErrCode = IDDIGITALCOPIER_NO_ERROR;

    EnumScannerDevice = nullptr;          // 1. 枚举扫描设备
    OpenConnection = nullptr;             // 2. 打开设备
    CloseConnection = nullptr;            // 3. 关闭设备
    CheckHaveIdCard = nullptr;            // 4. 检测是否放入卡
    TakeOutIdCard = nullptr;              // 5. 检测是否被取走
    StartScanIdCard = nullptr;            // 6. 启动扫描
    SavePicToMemory = nullptr;            // 7. 读取当前图像数据块到内存
    SavePicToFile = nullptr;              // 8.1 保存图像数据块到文件
    SavePicToFileII = nullptr;            // 8.2 保存图像数据块到文件
    RetainIdCard = nullptr;               // 9. 吞卡
    BackIdCard = nullptr;                 // 10. 退卡
    BackAndHoldIdCard = nullptr;          // 11. 出卡并持卡
    GetID2Info = nullptr;                 // 12.1 获取二代证信息
    GetID2InfoEx = nullptr;               // 12.2 获取二代证信息(带指纹)
    GetIDInfoForeign = nullptr;           // 12.3 获取二代证信息
    GetIDInfoGAT = nullptr;               // 12.4 获取二代证信息
    GetIDCardType = nullptr;              // 12.5 获取二代证信息
    GetAllTypeIdInfo = nullptr;           // 12.6 获取二代证信息
    GetLastErrorCode = nullptr;           // 13. 获取最近一次的错误码
    GetLastErrorStr = nullptr;            // 14. 获取最近一次的错误描述
    GetFWVersion = nullptr;               // 15. 获取固件版本信息
    GetSWVersion = nullptr;               // 16. 获取软件版本信息
    GetDevStatus = nullptr;               // 17. 获取设备状态
    ResetDevice = nullptr;                // 18. 复位设备
    SoftResetDevice = nullptr;            // 19. 软复位
    UpdateOnLine = nullptr;               // 20. 固件升级
    CISCalibrate = nullptr;               // 21. CIS校验
    SensorCalibrate = nullptr;            // 22. 传感器校验
    SetButtonEnable = nullptr;            // 23. 设置按键强制退卡使能
    SetAutoFeedEnable = nullptr;          // 24. 设置自动进卡使能
    SetInitFeedMode = nullptr;            // 25. 设置上电、复位初始化吸卡模式
    SetHeadFileFormat = nullptr;          // 26. 设置二代证芯片图像存储格式
    GetID2InfoFromImage = nullptr;        // 27. 通过图像获取通行证信息
    GetPassportInfoFromImage = nullptr;   // 28. 从图像获取二代证信息
    BackIdCardToRerec = nullptr;          // 29. 退卡到识别位置

    // 接口加载
    //bLoadLibrary();
}

CDevImpl_BSID81::~CDevImpl_BSID81()
{
    vUnLoadLibrary();
}


BOOL CDevImpl_BSID81::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);

    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> fail. ReturnCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
    }
    return TRUE;
}

void CDevImpl_BSID81::vUnLoadLibrary()
{
    THISMODULE(__FUNCTION__);

    if (m_LoadLibrary.isLoaded())
    {
        if (m_LoadLibrary.unload() != TRUE)
        {
            Log(ThisModule, __LINE__, "卸载动态库<%s> fail. ReturnCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().data());
        }
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_BSID81::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1. 枚举扫描设备
    EnumScannerDevice = (mEnumScannerDevice)m_LoadLibrary.resolve("EnumScannerDevice");
    FUNC_POINTER_ERROR_RETURN(EnumScannerDevice, "EnumScannerDevice");

    // 2. 打开设备
    OpenConnection = (mOpenConnection)m_LoadLibrary.resolve("OpenConnection");
    FUNC_POINTER_ERROR_RETURN(OpenConnection, "OpenConnection");

    // 3. 关闭设备
    CloseConnection = (mCloseConnection)m_LoadLibrary.resolve("CloseConnection");
    FUNC_POINTER_ERROR_RETURN(CloseConnection, "CloseConnection");

    // 4. 检测是否放入卡
    CheckHaveIdCard = (mCheckHaveIdCard)m_LoadLibrary.resolve("CheckHaveIdCard");
    FUNC_POINTER_ERROR_RETURN(CheckHaveIdCard, "CheckHaveIdCard");

    // 5. 检测是否被取走
    TakeOutIdCard = (mTakeOutIdCard)m_LoadLibrary.resolve("TakeOutIdCard");
    FUNC_POINTER_ERROR_RETURN(TakeOutIdCard, "TakeOutIdCard");

    // 6. 启动扫描
    StartScanIdCard = (mStartScanIdCard)m_LoadLibrary.resolve("StartScanIdCard");
    FUNC_POINTER_ERROR_RETURN(StartScanIdCard, "StartScanIdCard");

    // 7. 读取当前图像数据块到内存
    SavePicToMemory = (mSavePicToMemory)m_LoadLibrary.resolve("SavePicToMemory");
    FUNC_POINTER_ERROR_RETURN(SavePicToMemory, "SavePicToMemory");

    // 8.1 保存图像数据块到文件
    SavePicToFile = (mSavePicToFile)m_LoadLibrary.resolve("SavePicToFile");
    FUNC_POINTER_ERROR_RETURN(SavePicToFile, "SavePicToFile");

    // 8.2 保存图像数据块到文件
    SavePicToFileII = (mSavePicToFileII)m_LoadLibrary.resolve("SavePicToFileII");
    FUNC_POINTER_ERROR_RETURN(SavePicToFileII, "SavePicToFileII");

    // 9. 吞卡
    RetainIdCard = (mRetainIdCard)m_LoadLibrary.resolve("RetainIdCard");
    FUNC_POINTER_ERROR_RETURN(RetainIdCard, "RetainIdCard");

    // 10. 退卡
    BackIdCard = (mBackIdCard)m_LoadLibrary.resolve("BackIdCard");
    FUNC_POINTER_ERROR_RETURN(BackIdCard, "BackIdCard");

    // 11. 出卡并持卡
    BackAndHoldIdCard = (mBackAndHoldIdCard)m_LoadLibrary.resolve("BackAndHoldIdCard");
    FUNC_POINTER_ERROR_RETURN(BackAndHoldIdCard, "BackAndHoldIdCard");

    // 12.1 获取二代证信息
    GetID2Info = (mGetID2Info)m_LoadLibrary.resolve("GetID2Info");
    FUNC_POINTER_ERROR_RETURN(GetID2Info, "GetID2Info");

    // 12.2 获取二代证信息(带指纹)
    GetID2InfoEx = (mGetID2InfoEx)m_LoadLibrary.resolve("GetID2InfoEx");
    FUNC_POINTER_ERROR_RETURN(GetID2InfoEx, "GetID2InfoEx");

    // 12.3 获取二代证信息
    GetIDInfoForeign = (mGetIDInfoForeign)m_LoadLibrary.resolve("GetIDInfoForeign");
    FUNC_POINTER_ERROR_RETURN(GetIDInfoForeign, "GetIDInfoForeign");

    // 12.4 获取二代证信息
    GetIDInfoGAT = (mGetIDInfoGAT)m_LoadLibrary.resolve("GetIDInfoGAT");
    FUNC_POINTER_ERROR_RETURN(GetIDInfoGAT, "GetIDInfoGAT");

    // 12.5 获取二代证信息
    GetIDCardType = (mGetIDCardType)m_LoadLibrary.resolve("GetIDCardType");
    FUNC_POINTER_ERROR_RETURN(GetIDCardType, "GetIDCardType");

    // 12.6 获取二代证信息
    if (m_wLoadDllVer == 0)
    {
        GetAllTypeIdInfo = (mGetAllTypeIdInfo)m_LoadLibrary.resolve("GetAllTypeIdInfo");
        FUNC_POINTER_ERROR_RETURN(GetAllTypeIdInfo, "GetAllTypeIdInfo");
    } else
    {
        GetAllTypeIdInfo = (mGetAllTypeIdInfo)m_LoadLibrary.resolve("GetIDCardType");
        FUNC_POINTER_ERROR_RETURN(GetAllTypeIdInfo, "GetIDCardType");
    }

    // 13. 获取最近一次的错误码
    GetLastErrorCode = (mGetLastErrorCode)m_LoadLibrary.resolve("GetLastErrorCode");
    FUNC_POINTER_ERROR_RETURN(GetLastErrorCode, "GetLastErrorCode");

    // 14. 获取最近一次的错误描述
    GetLastErrorStr = (mGetLastErrorStr)m_LoadLibrary.resolve("GetLastErrorStr");
    FUNC_POINTER_ERROR_RETURN(GetLastErrorStr, "GetLastErrorStr");

    // 15. 获取固件版本信息
    GetFWVersion = (mGetFWVersion)m_LoadLibrary.resolve("GetFWVersion");
    FUNC_POINTER_ERROR_RETURN(GetFWVersion, "GetFWVersion");

    // 16. 获取软件版本信息
    GetSWVersion = (mGetSWVersion)m_LoadLibrary.resolve("GetSWVersion");
    FUNC_POINTER_ERROR_RETURN(GetSWVersion, "GetSWVersion");

    // 17. 获取设备状态
    GetDevStatus = (mGetDevStatus)m_LoadLibrary.resolve("GetDevStatus");
    FUNC_POINTER_ERROR_RETURN(GetDevStatus, "GetDevStatus");

    // 18. 复位设备
    ResetDevice = (mResetDevice)m_LoadLibrary.resolve("ResetDevice");
    FUNC_POINTER_ERROR_RETURN(ResetDevice, "ResetDevice");

    // 19. 软复位
    SoftResetDevice = (mSoftResetDevice)m_LoadLibrary.resolve("SoftResetDevice");
    FUNC_POINTER_ERROR_RETURN(SoftResetDevice, "SoftResetDevice");

    // 20. 固件升级
    UpdateOnLine = (mUpdateOnLine)m_LoadLibrary.resolve("UpdateOnLine");
    FUNC_POINTER_ERROR_RETURN(UpdateOnLine, "UpdateOnLine");

    // 21. CIS校验
    CISCalibrate = (mCISCalibrate)m_LoadLibrary.resolve("CISCalibrate");
    FUNC_POINTER_ERROR_RETURN(CISCalibrate, "CISCalibrate");

    // 22. 传感器校验
    SensorCalibrate = (mSensorCalibrate)m_LoadLibrary.resolve("SensorCalibrate");
    FUNC_POINTER_ERROR_RETURN(SensorCalibrate, "SensorCalibrate");

    // 23. 设置按键强制退卡使能
    SetButtonEnable = (mSetButtonEnable)m_LoadLibrary.resolve("SetButtonEnable");
    FUNC_POINTER_ERROR_RETURN(SetButtonEnable, "SetButtonEnable");

    // 24. 设置自动进卡使能
    SetAutoFeedEnable = (mSetAutoFeedEnable)m_LoadLibrary.resolve("SetAutoFeedEnable");
    FUNC_POINTER_ERROR_RETURN(SetAutoFeedEnable, "SetAutoFeedEnable");

    // 25. 设置上电、复位初始化吸卡模式
    SetInitFeedMode = (mSetInitFeedMode)m_LoadLibrary.resolve("SetInitFeedMode");
    FUNC_POINTER_ERROR_RETURN(SetInitFeedMode, "SetInitFeedMode");

    // 26. 设置二代证芯片图像存储格式
    SetHeadFileFormat = (mSetHeadFileFormat)m_LoadLibrary.resolve("SetHeadFileFormat");
    FUNC_POINTER_ERROR_RETURN(SetHeadFileFormat, "SetHeadFileFormat");

    // 27. 通过图像获取通行证信息
    GetID2InfoFromImage = (mGetID2InfoFromImage)m_LoadLibrary.resolve("GetID2InfoFromImage");
    FUNC_POINTER_ERROR_RETURN(GetID2InfoFromImage, "GetID2InfoFromImage");

    // 28. 从图像获取二代证信息
    GetPassportInfoFromImage = (mGetPassportInfoFromImage)m_LoadLibrary.resolve("GetPassportInfoFromImage");
    FUNC_POINTER_ERROR_RETURN(GetPassportInfoFromImage, "GetPassportInfoFromImage");

    // 29. 退卡到识别位置
    BackIdCardToRerec = (mBackIdCardToRerec)m_LoadLibrary.resolve("BackIdCardToRerec");
    FUNC_POINTER_ERROR_RETURN(BackIdCardToRerec, "BackIdCardToRerec");

    return TRUE;
}

//------------------------------------------------------------------------------------------------------
BOOL CDevImpl_BSID81::OpenDevice()
{
    return OpenDevice(0);
}

BOOL CDevImpl_BSID81::OpenDevice(WORD wType)
{
    THISMODULE(__FUNCTION__);

    //如果已打开，则关闭
    CloseDevice();

    //m_bDevOpenOk = FALSE;

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            Log(ThisModule, __LINE__, "加载动态库: OpenDevice()->bLoadLibrary() fail. ");
            return FALSE;
        }
    }

    // 加载设备动态库
    /*if (bLoadLibrary() != TRUE)
    {
        Log(ThisModule, __LINE__, "加载动态库: OpenDevice()->bLoadLibrary() fail. ");
        return FALSE;
    }*/

    // 枚举扫描设备
    if (bEnumScannerDevice() != TRUE)
    {
        Log(ThisModule, __LINE__, "枚举扫描设备: OpenDevice()->bEnumScannerDevice() fail. ");
        return FALSE;
    }

    // 打开设备
    if (bOpenConnection() != TRUE)
    {
        Log(ThisModule, __LINE__, "打开设备: OpenDevice()->bOpenConnection() fail. ");
        return FALSE;
    }

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;
    return TRUE;
}

BOOL CDevImpl_BSID81::CloseDevice()
{
    THISMODULE(__FUNCTION__);

    // 关闭设备
    if (m_bDevOpenOk == TRUE)
    {
        bCloseConnection();
    }

    // 释放动态库
    vUnLoadLibrary();

    // 设备Open标记=F
    m_bDevOpenOk = FALSE;

    Log(ThisModule, __LINE__, "设备关闭,释放动态库: Close Device(%d), unLoad Library.", m_bsDeviceId);

    return TRUE;
}

BOOL CDevImpl_BSID81::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

void CDevImpl_BSID81::SetDevOpenIsF()
{
    m_bDevOpenOk = FALSE;
}

INT CDevImpl_BSID81::GetErrCode(BOOL bIsStat)
{
    if (bIsStat == TRUE)
        return m_nStatErrCode;
    else
        return m_nDevErrCode;
}

// 1. 枚举扫描设备
BOOL CDevImpl_BSID81::bEnumScannerDevice()
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    ScannerInfoRec  ScannerInfo[8] = {0};
    unsigned int iDevNumber = 0;
    // 枚举扫描设备
    m_nDevErrCode = EnumScannerDevice(ScannerInfo, &iDevNumber);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "枚举扫描设备: bEnumScannerDevice()->EnumScannerDevice() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    m_bsDeviceId = ScannerInfo[0].DeviceID;
    Log(ThisModule, __LINE__,
        "枚举扫描设备: bEnumScannerDevice()->EnumScannerDevice() succ. DevNumber = %d, ScannerInfo = %d|%d|%d|%d|%d|%d|%d|%d, "
        "Set m_bsDeviceId = %d",
        iDevNumber, ScannerInfo[0].DeviceID, ScannerInfo[1].DeviceID, ScannerInfo[2].DeviceID, ScannerInfo[3].DeviceID,
            ScannerInfo[4].DeviceID, ScannerInfo[5].DeviceID, ScannerInfo[6].DeviceID, ScannerInfo[7].DeviceID,
            m_bsDeviceId);

    return TRUE;
}

// 2. 打开设备
BOOL CDevImpl_BSID81::bOpenConnection()
{
    THISMODULE(__FUNCTION__);

    // 打开设备
    m_nDevErrCode = 0;
    m_nDevErrCode = OpenConnection(m_bsDeviceId);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "打开设备: bOpenConnection()->OpenConnection(%d) fail. ReturnCode:%s", m_bsDeviceId, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }
    Log(ThisModule, __LINE__, "打开设备: bOpenConnection()->OpenConnection(%d) succ. ", m_bsDeviceId);

    return TRUE;
}

// 3. 关闭设备
BOOL CDevImpl_BSID81::bCloseConnection()
{
    THISMODULE(__FUNCTION__);

    if (CloseConnection == nullptr)
    {
        Log(ThisModule, __LINE__, "关闭设备: bCloseConnectio()->CloseConnection(%d) fail. CloseConnection is NULL, Not LoadLibrary", m_bsDeviceId);
        return FALSE;
    }

    m_nDevErrCode = 0;
    m_nDevErrCode = CloseConnection(m_bsDeviceId);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "关闭设备: bCloseConnectio()->CloseConnection(%d) fail. ReturnCode:%s", m_bsDeviceId, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 4. 检测是否放入卡
BOOL CDevImpl_BSID81::bCheckHaveIdCard(UINT unChkTime)
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = CheckHaveIdCard(m_bsDeviceId, unChkTime);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        //Log(ThisModule, __LINE__, "检测是否放入卡: bCheckHaveIdCard()->CheckHaveIdCard(%d, %d) fail. ReturnCode:%s",
        //    m_bsDeviceId, unChkTime, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 6. 启动扫描
BOOL CDevImpl_BSID81::bStartScanIdCard()
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = StartScanIdCard(m_bsDeviceId);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "启动扫描: bStartScanIdCard()->StartScanIdCard(%d) fail. ReturnCode:%s",
            m_bsDeviceId, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 7. 读取当前图像数据块到内存
BOOL CDevImpl_BSID81::bSavePicToMemory(LPSTR lpFrontImgBuf, LPSTR lpRearImgBuf, LPINT lpFrontLen, LPINT lpRearLen)
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = SavePicToMemory(m_bsDeviceId, lpFrontImgBuf, lpRearImgBuf, lpFrontLen, lpRearLen);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "读取当前图像数据块到内存: bSavePicToMemory()->SavePicToMemory(%d) fail. Result: %d|%d, ReturnCode:%s",
            m_bsDeviceId, *lpFrontLen, *lpRearLen, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 8.1 保存图像数据块到文件
BOOL CDevImpl_BSID81::bSavePicToFile(LPSTR lpImgBuf, UINT uBufLen, LPSTR lpFileName, INT Format)
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = SavePicToFile(m_bsDeviceId, lpImgBuf, uBufLen, lpFileName, Format);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "保存图像数据块到文件: bSavePicToFile()->SavePicToFile(%d, Data, %d, %s, %d|%s) fail. ReturnCode:%s",
            m_bsDeviceId, uBufLen, lpFileName, Format, (Format == SAVE_IMG_BMP ? "BMP" : "JPG"),
            GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 8.2 保存图像数据块到文件(指定缩放比例)
BOOL CDevImpl_BSID81::bSavePicToFileII(LPSTR lpImgBuf, UINT uBufLen, LPSTR lpFileName, INT Format, FLOAT fZoomScale)
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = SavePicToFileII(m_bsDeviceId, lpImgBuf, uBufLen, lpFileName, Format, fZoomScale);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "保存图像数据块到文件(指定缩放比例): bSavePicToFileII()->SavePicToFileII(%d, Data, %d, %s, %d|%s, %.2f) fail. ReturnCode:%s",
            m_bsDeviceId, uBufLen, lpFileName, Format, (Format == SAVE_IMG_BMP ? "BMP" : "JPG"), fZoomScale,
            GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 9. 吞卡
BOOL CDevImpl_BSID81::bRetainIdCard()
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = RetainIdCard(m_bsDeviceId);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "吞卡: bRetainIdCard()->RetainIdCard() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 10. 退卡
BOOL CDevImpl_BSID81::bBackIdCard()
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = BackIdCard(m_bsDeviceId);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "退卡: bBackIdCard()->BackIdCard() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 11. 出卡并持卡
BOOL CDevImpl_BSID81::bBackAndHoldIdCard()
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = BackAndHoldIdCard(m_bsDeviceId);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "出卡并持卡: bBackAndHoldIdCard()->BackAndHoldIdCard() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 12.1 获取二代证信息(国内)
BOOL CDevImpl_BSID81::bGetID2Info(IDInfo &stIDCard, LPSTR lpHeadImageName)
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = GetID2Info(m_bsDeviceId, &stIDCard, lpHeadImageName);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "获取二代证信息(国内): bGetID2Info()->GetID2Info() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 12.2 获取二代证信息(国内,带指纹)
BOOL CDevImpl_BSID81::bGetID2InfoEx(IDInfoEx &stIDCard, LPSTR lpHeadImageName)
{
    THISMODULE(__FUNCTION__);

    IDInfoEx_GBK stIDCard_GBK;

    m_nDevErrCode = 0;

    if (m_wLoadDllVer == 0) // V2.01
    {
        m_nDevErrCode = GetID2InfoEx(m_bsDeviceId, &stIDCard, lpHeadImageName);
        if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
        {
            Log(ThisModule, __LINE__, "获取二代证信息(国内,带指纹): V2.0(UTF8): bGetID2InfoEx()->GetID2InfoEx() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
            return FALSE;
        }
    } else
    {
        m_nDevErrCode = GetID2InfoEx(m_bsDeviceId, (IDInfoEx*)&stIDCard_GBK, lpHeadImageName);
        if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
        {
            Log(ThisModule, __LINE__, "获取二代证信息(国内,带指纹): V3.0(GBK): bGetID2InfoEx()->GetID2InfoEx_GBK() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
            return FALSE;
        }

        //--------GBK转换为UTF8(有中文做转换)--------
        // 姓名32->72
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.name, stIDCard.name, sizeof(stIDCard.name));
        // 性别4->4
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.sex, stIDCard.sex, sizeof(stIDCard.sex));
        // 民族12->20
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.nation, stIDCard.nation, sizeof(stIDCard.nation));
        // 出生日期20->20
        memcpy(stIDCard.birthday, stIDCard_GBK.birthday, strlen(stIDCard_GBK.birthday));
        // 住址信息72->200
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.address, stIDCard.address, sizeof(stIDCard.address));
        // 身份证号码40->40
        memcpy(stIDCard.number, stIDCard_GBK.number, strlen(stIDCard_GBK.number));
        // 签发机关32->120
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.department, stIDCard.department, sizeof(stIDCard.department));
        // 有效日期36->36
        memcpy(stIDCard.timeLimit, stIDCard_GBK.timeLimit, strlen(stIDCard_GBK.timeLimit));
        // 头像信息文件256->256
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.Image, stIDCard.Image, sizeof(stIDCard.Image));
        // 指纹信息数据1024->1024
        memcpy(stIDCard.FingerData, stIDCard_GBK.FingerData, stIDCard_GBK.iFingerDataLen);
        // 指纹信息数据长度
        stIDCard.iFingerDataLen = stIDCard_GBK.iFingerDataLen;
    }

    return TRUE;
}

// 12.3 获取二代证信息(国外)
BOOL CDevImpl_BSID81::bGetIDInfoForeign(IDInfoForeign &stIDCard, LPSTR lpHeadImageName)
{
    THISMODULE(__FUNCTION__);

    IDInfoForeign_GBK stIDCard_GBK;

    m_nDevErrCode = 0;    

    if (m_wLoadDllVer == 0) // V2.01
    {
        m_nDevErrCode = GetIDInfoForeign(m_bsDeviceId, &stIDCard, lpHeadImageName);
        if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
        {
            Log(ThisModule, __LINE__, "获取二代证信息(国外): V2.0(UTF8): bGetIDInfoForeign()->GetIDInfoForeign() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
            return FALSE;
        }
    } else
    {
        m_nDevErrCode = GetIDInfoForeign(m_bsDeviceId, (IDInfoForeign*)&stIDCard_GBK, lpHeadImageName);
        if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
        {
            Log(ThisModule, __LINE__, "获取二代证信息(国外): V3.0(GBK): bGetIDInfoForeign()->GetIDInfoForeign() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
            return FALSE;
        }

        //--------GBK转换为UTF8(有中文做转换)--------
        // 英文姓名120->120
        memcpy(stIDCard.NameENG, stIDCard_GBK.NameENG, sizeof(stIDCard_GBK.NameENG));
        // 性别4->4
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.Sex, stIDCard.Sex, sizeof(stIDCard.Sex));
        // 证件号码30->30
        memcpy(stIDCard.IDCardNO, stIDCard_GBK.IDCardNO, sizeof(stIDCard_GBK.IDCardNO));
        // 国籍6->40
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.Nation, stIDCard.Nation, sizeof(stIDCard.Nation));
        // 中文姓名72->120
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.NameCHN, stIDCard.NameCHN, sizeof(stIDCard.NameCHN));
        // 证件签发日期开始16->16
        memcpy(stIDCard.TimeLimitBegin, stIDCard_GBK.TimeLimitBegin, sizeof(stIDCard_GBK.TimeLimitBegin));
        // 证件签发日期结束16->16
        memcpy(stIDCard.TimeLimitEnd, stIDCard_GBK.TimeLimitEnd, sizeof(stIDCard_GBK.TimeLimitEnd));
        // 出生日期36->36
        memcpy(stIDCard.Born, stIDCard_GBK.Born, strlen(stIDCard_GBK.Born));
        // 证件版本4->8
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.IDVersion, stIDCard.IDVersion, sizeof(stIDCard.IDVersion));
        // 签发机关8->40
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.Department, stIDCard.Department, sizeof(stIDCard.Department));
        // 证件类型2->4
        memcpy(stIDCard.IDType, stIDCard_GBK.IDType, strlen(stIDCard_GBK.IDType));
        // 保留信息6->12
        memcpy(stIDCard.Reserve, stIDCard_GBK.Reserve, strlen(stIDCard_GBK.Reserve));
        // 头像1024->1024
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.Image, stIDCard.Image, sizeof(stIDCard.Image));
    }

    return TRUE;
}
// 12.4 获取二代证信息(港澳台通行证)
BOOL CDevImpl_BSID81::bGetIDInfoGAT(IDInfoGAT &stIDCard, LPSTR lpHeadImageName)
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;    

    IDInfoGAT_GBK stIDCard_GBK;

    m_nDevErrCode = 0;

    if (m_wLoadDllVer == 0) // V2.01
    {
        m_nDevErrCode = GetIDInfoGAT(m_bsDeviceId, &stIDCard, lpHeadImageName);
        if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
        {
            Log(ThisModule, __LINE__, "获取二代证信息(港澳台通行证): V2.0(UTF8): bGetIDInfoGAT()->GetIDInfoGAT() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
            return FALSE;
        }
    } else
    {
        m_nDevErrCode = GetIDInfoGAT(m_bsDeviceId, (IDInfoGAT*)&stIDCard_GBK, lpHeadImageName);
        if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
        {
            Log(ThisModule, __LINE__, "获取二代证信息(港澳台通行证): V3.0(GBK): bGetIDInfoGAT()->GetIDInfoGAT() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
            return FALSE;
        }

        //--------GBK转换为UTF8(有中文做转换)--------
        // 姓名32->72
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.name, stIDCard.name, sizeof(stIDCard.name));
        // 性别4->4
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.sex, stIDCard.sex, sizeof(stIDCard.sex));
        // 民族12->20
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.nation, stIDCard.nation, sizeof(stIDCard.nation));
        // 出生日期20->20
        memcpy(stIDCard.birthday, stIDCard_GBK.birthday, strlen(stIDCard_GBK.birthday));
        // 住址信息72->200
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.address, stIDCard.address, sizeof(stIDCard.address));
        // 身份证号码40->40
        memcpy(stIDCard.number, stIDCard_GBK.number, strlen(stIDCard_GBK.number));
        // 签发机关32->120
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.department, stIDCard.department, sizeof(stIDCard.department));
        // 有效日期36->36
        memcpy(stIDCard.timeLimit, stIDCard_GBK.timeLimit, strlen(stIDCard_GBK.timeLimit));
        // 通行证号码20->20
        memcpy(stIDCard.passport, stIDCard_GBK.passport, strlen(stIDCard_GBK.passport));
        // 签发次数6->6
        memcpy(stIDCard.issue, stIDCard_GBK.issue, strlen(stIDCard_GBK.issue));
        // 头像信息文件256->256
        DataConvertor::string_ascii_to_utf8(stIDCard_GBK.Image, stIDCard.Image, sizeof(stIDCard.Image));
        // 指纹信息数据1024->1024
        memcpy(stIDCard.FingerData, stIDCard_GBK.FingerData, stIDCard_GBK.iFingerDataLen);
        // 指纹信息数据长度
        stIDCard.iFingerDataLen = stIDCard_GBK.iFingerDataLen;
    }

    return TRUE;
}

// 12.5 获取二代证信息(卡类型)
BOOL CDevImpl_BSID81::bGetIDCardType(LPINT lpCardType)
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = GetIDCardType(m_bsDeviceId, lpCardType);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "获取二代证信息(卡类型): bGetIDCardType()->bGetIDCardType(%d) fail. ReturnCode:%s",
            m_bsDeviceId, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 15. 获取固件版本信息
BOOL CDevImpl_BSID81::bGetFWVersion(LPSTR lpFwVer)
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = GetFWVersion(m_bsDeviceId, lpFwVer);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "获取固件版本信息: bGetFWVersion()->GetFWVersion(%d) fail. ReturnCode:%s",
            m_bsDeviceId, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 16. 获取软件版本信息
BOOL CDevImpl_BSID81::bGetSWVersion(LPSTR lpSwVer)
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = GetSWVersion(lpSwVer);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "获取软件版本信息: bGetSWVersion()->GetSWVersion() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 17. 获取设备状态
BOOL CDevImpl_BSID81::bGetDevStatus(DEVSTATUS &devStatus)
{
    THISMODULE(__FUNCTION__);

    m_nStatErrCode = 0;
    //memset(&m_stNewDevStatus, 0x00, sizeof(DEVSTATUS));
    m_nStatErrCode = GetDevStatus(m_bsDeviceId, &devStatus);//&m_stNewDevStatus);
    if (m_nStatErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "获取设备状态: bGetDevStatus()->GetDevStatus() fail. ReturnCode:%s", GetErrorStr(m_nStatErrCode));
        return FALSE;
    }

    //if (memcmp(&m_stNewDevStatus, &m_stOldDevStatus, sizeof(DEVSTATUS)) != 0)
    if (memcmp(&devStatus, &m_stOldDevStatus, sizeof(DEVSTATUS)) != 0)
    {
        Log(ThisModule, __LINE__,
            "获取设备状态: bGetDevStatus()->GetDevStatus() Result: %d:%s|%d:%s|%d:%s|%d:%s|%d:%s|%d:%s|%d:%s|%d:%s|%d:%s",
            devStatus.iStatusProcess,
                (devStatus.iStatusProcess == 0 ? "过程空闲" : (devStatus.iStatusProcess == 1 ? "过程出错" : "过程执行中")),
            devStatus.iStatusCoverOpen,
                (devStatus.iStatusCoverOpen == 0 ? "上盖未打开" : "上盖打开"),
            devStatus.iStatusPowerOff,
                (devStatus.iStatusPowerOff == 0 ? "未掉电" : "掉电"),
            devStatus.iStatusLowVoltage,
                (devStatus.iStatusLowVoltage == 0 ? "未出现低电压" : "低电压错误"),
            devStatus.iStatusInputSensorHaveCard,
                (devStatus.iStatusInputSensorHaveCard == 0 ? "入口传感器无卡" : "入口传感器有卡"),
            devStatus.iStatusCardJam,
                (devStatus.iStatusCardJam == 0 ? "未塞卡JAM" : "塞卡JAM"),
            devStatus.iStatusBoot,
                (devStatus.iStatusBoot == 0 ? "未进入BOOT状态" : "进入BOOT状态"),
            devStatus.iStatusMiddleSensorHaveCard,
                (devStatus.iStatusMiddleSensorHaveCard == 0 ? "中间传感器无卡" : "中间传感器有卡"),
            devStatus.iStatusScanSensorHaveCard,
                (devStatus.iStatusScanSensorHaveCard == 0 ? "扫描传感器无卡" : "扫描传感器有卡"));
    }
    memcpy(&m_stOldDevStatus, &devStatus, sizeof(DEVSTATUS));

    return TRUE;
}

// 18. 复位设备
BOOL CDevImpl_BSID81::bResetDevice()
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;

    m_nDevErrCode = ResetDevice(m_bsDeviceId);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "硬复位设备: bResetDevice()->ResetDevice() fail. ReturnCode:%s", GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    if (OpenDevice() != TRUE)
    {
        Log(ThisModule, __LINE__, "硬复位设备后重新连接: bResetDevice()->OpenDevice() fail. ");
        return FALSE;
    }

    return TRUE;
}

// 19. 软复位设备
BOOL CDevImpl_BSID81::bSoftResetDevice(int iMode)
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;

    if (iMode < CARD_EJECT || iMode > CARD_NOACTION)
    {
        Log(ThisModule, __LINE__, "软复位设备: bSoftResetDevice() input mode invalid fail. mode<%d> != <%d|%d|%d|%d>",
            iMode, CARD_EJECT, CARD_RETRACT, CARD_EJECTMENT, CARD_NOACTION);
        return FALSE;
    }

    m_nDevErrCode = SoftResetDevice(m_bsDeviceId, iMode);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "软复位设备: bSoftResetDevice()->SoftResetDevice(%d, %d) fail. ReturnCode:%s",
            m_bsDeviceId, iMode, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

// 29. 退卡到可识别位置
BOOL CDevImpl_BSID81::bBackIdCardToRerec()
{
    THISMODULE(__FUNCTION__);

    m_nDevErrCode = 0;
    m_nDevErrCode = BackIdCardToRerec(m_bsDeviceId);
    if (m_nDevErrCode != IDDIGITALCOPIER_NO_ERROR)
    {
        Log(ThisModule, __LINE__, "退卡到可识别位置: bBackIdCardToRerec()->BackIdCardToRerec(%d) fail. ReturnCode:%s",
            m_bsDeviceId, GetErrorStr(m_nDevErrCode));
        return FALSE;
    }

    return TRUE;
}

LPSTR CDevImpl_BSID81::GetErrorStr(LONG lErrCode)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(lErrCode)
    {
        case IDDIGITALCOPIER_NO_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "正常");
            break;
        case IDDIGITALCOPIER_NO_DEVICE:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "无设备");
            break;
        case IDDIGITALCOPIER_PORT_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "端口错误");
            break;
        case IDDIGITALCOPIER_TABPAR_NONE:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "参数文件错误");
            break;
        case IDDIGITALCOPIER_HAVE_NOT_INIT:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "未初始化");
            break;
        case IDDIGITALCOPIER_INVALID_ARGUMENT:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "无效参数");
            break;
        case IDDIGITALCOPIER_TIMEOUT_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "超时错误");
            break;
        case IDDIGITALCOPIER_STATUS_COVER_OPENED:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "上盖打开");
            break;
        case IDDIGITALCOPIER_STATUS_PASSAGE_JAM:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "塞卡");
            break;
        case IDDIGITALCOPIER_OUT_OF_MEMORY:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "内存溢出");
            break;
        case IDDIGITALCOPIER_NO_ID_DATA:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "没有二代证数据");
            break;
        case IDDIGITALCOPIER_NO_IMAGE_DATA:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "没有图像数据");
            break;
        case IDDIGITALCOPIER_IMAGE_PROCESS_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "图像处理错误");
            break;
        case IDDIGITALCOPIER_IMAGE_JUDGE_DIRECTION_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "判断图像方向错误");
            break;
        case IDDIGITALCOPIER_CLOSE_FAILED:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "关闭端口失败");
            break;
        case IDDIGITALCOPIER_IDDATA_PROCESS_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "身份证电子信息处理错误");
            break;
        case IDDIGITALCOPIER_SENSORVALIDATE:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "传感器校验错误");
            break;
        case IDDIGITALCOPIER_VOLTAGE_LOW:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "电压低");
            break;
        case IDDIGITALCOPIER_CIS_CORRECTION_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "校正错误");
            break;
        case IDDIGITALCOPIER_NO_CARD:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "无卡");
            break;
        case IDDIGITALCOPIER_FIRMWARE_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "未知错误");
            break;
        case IDDIGITALCOPIER_SAVE_IMAGE_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "保存位图错误");
            break;
        case IDDIGITALCOPIER_POWER_OFF:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "掉电错误");
            break;
        case IDDIGITALCOPIER_INPUT_BOOT:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "BOOT错误");
            break;
        case IDDIGITALCOPIER_BUTTON_UP:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "按键抬起");
            break;
        case IDDIGITALCOPIER_RECOGNISE_FAILED:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "识别错误");
            break;
        case IDDIGITALCOPIER_SCAN_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "扫描错误");
            break;
        case IDDIGITALCOPIER_FEED_ERROR:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "走卡错误");
            break;
        case IDDIGITALCOPIER_MAX_CODE:
            sprintf(m_szErrStr, "%d|%s", lErrCode, "最大错误码");
            break;
        default :
            sprintf(m_szErrStr,  "%d|%s", lErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStr;
}

// 设置动态库路径(DeviceOpen前有效)
void CDevImpl_BSID81::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径
    if (lpPath == nullptr || strlen(lpPath) < 1 ||
        (strlen(lpPath) == 1 && (char*)lpPath[0] == "/"))
    {
        Log(ThisModule, __LINE__, "入参[%s]无效,不设置动态库路径.", lpPath);
        return;
    }

    if (lpPath[0] == '/')   // 绝对路径
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s", lpPath);
    } else
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s%s", LINUXPATHLIB, lpPath);
    }

    Log(ThisModule, __LINE__, "设置动态库路径=<%s>.", m_szLoadDllPath);
}

// 设置动态库版本(DeviceOpen前有效)
void CDevImpl_BSID81::SetLibVer(WORD wVer)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径
    if (wVer < 0 || wVer > 1)
    {
        Log(ThisModule, __LINE__, "入参[%d]无效,不设置动态库路径.", wVer);
        return;
    }

    m_wLoadDllVer = wVer;

    Log(ThisModule, __LINE__, "设置动态库版本=<%d|%s>.", wVer,
        (wVer == 0 ? "BS-ID81M_so_Linux_UOS_FT_V2.01" : "IDCard_SDK_Linux_FeiTeng_Kylin_V3.0.7.1"));
}


//---------------------------------------------
