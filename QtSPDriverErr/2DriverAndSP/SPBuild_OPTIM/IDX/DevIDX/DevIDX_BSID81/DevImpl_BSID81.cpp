/***************************************************************
* 文件名称: DevImpl_BSID81.cpp
* 文件描述: 身份证模块底层指令, 提供控制接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年3月25日
* 文件版本: 1.0.0.1
****************************************************************/

#include "DevImpl_BSID81.h"
#include "data_convertor.h"

static const char *ThisFile = "DevImpl_BSID81.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        if (m_nRetErrOLD[4] != IMP_ERR_NOTOPEN) \
        { \
            Log(ThisModule, __LINE__, "检查设备OPEN标记: OpenFlag == FALSE, Device Not Open, return fail.Return: %s.", \
            ConvertCode_IMPL2Str(IMP_ERR_NOTOPEN)); \
        } \
        m_nRetErrOLD[4] = IMP_ERR_NOTOPEN; \
        return IMP_ERR_NOTOPEN; \
    }

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
    m_wLoadDllVer = 0;
    m_bLoadIntfFail = TRUE;

    memset(&m_stOldDevStatus, 0x00, sizeof(DEVSTATUS));

    vInitLibFunc();
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

    if (m_LoadLibrary.load() != true)
    {
        if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> fail, ReturnCode: %s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
        }
        return FALSE;
    } else
    {
        Log(ThisModule, __LINE__, "加载动态库<%s> succ. ", m_szLoadDllPath);
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> fail, ReturnCode: %s.",
                    m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> succ. ", m_szLoadDllPath);
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
            Log(ThisModule, __LINE__, "卸载动态库<%s> fail, ReturnCode: %s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().data());
        }
        m_bLoadIntfFail = TRUE;
        vInitLibFunc();
    }
}

BOOL CDevImpl_BSID81::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1. 枚举扫描设备
    LOAD_LIBINFO_FUNC(mEnumScannerDevice, EnumScannerDevice, "EnumScannerDevice");

    // 2. 打开设备
    LOAD_LIBINFO_FUNC(mOpenConnection, OpenConnection, "OpenConnection");

    // 3. 关闭设备
    LOAD_LIBINFO_FUNC(mCloseConnection, CloseConnection, "CloseConnection");

    // 4. 检测是否放入卡
    LOAD_LIBINFO_FUNC(mCheckHaveIdCard, CheckHaveIdCard, "CheckHaveIdCard");

    // 5. 检测是否被取走
    LOAD_LIBINFO_FUNC(mTakeOutIdCard, TakeOutIdCard, "TakeOutIdCard");

    // 6. 启动扫描
    LOAD_LIBINFO_FUNC(mStartScanIdCard, StartScanIdCard, "StartScanIdCard");

    // 7. 读取当前图像数据块到内存
    LOAD_LIBINFO_FUNC(mSavePicToMemory, SavePicToMemory, "SavePicToMemory");

    // 8.1 保存图像数据块到文件
    LOAD_LIBINFO_FUNC(mSavePicToFile, SavePicToFile, "SavePicToFile");

    // 8.2 保存图像数据块到文件
    LOAD_LIBINFO_FUNC(mSavePicToFileII, SavePicToFileII, "SavePicToFileII");

    // 9. 吞卡
    LOAD_LIBINFO_FUNC(mRetainIdCard, RetainIdCard, "RetainIdCard");

    // 10. 退卡
    LOAD_LIBINFO_FUNC(mBackIdCard, BackIdCard, "BackIdCard");

    // 11. 出卡并持卡
    LOAD_LIBINFO_FUNC(mBackAndHoldIdCard, BackAndHoldIdCard, "BackAndHoldIdCard");

    // 12.1 获取二代证信息
    LOAD_LIBINFO_FUNC(mGetID2Info, GetID2Info, "GetID2Info");

    // 12.2 获取二代证信息(带指纹)
    LOAD_LIBINFO_FUNC(mGetID2InfoEx, GetID2InfoEx, "GetID2InfoEx");

    // 12.3 获取二代证信息
    LOAD_LIBINFO_FUNC(mGetIDInfoForeign, GetIDInfoForeign, "GetIDInfoForeign");

    // 12.4 获取二代证信息
    LOAD_LIBINFO_FUNC(mGetIDInfoGAT, GetIDInfoGAT, "GetIDInfoGAT");

    // 12.5 获取二代证信息
    LOAD_LIBINFO_FUNC(mGetIDCardType, GetIDCardType, "GetIDCardType");

    // 12.6 获取二代证信息
    if (m_wLoadDllVer == 0)
    {
        LOAD_LIBINFO_FUNC(mGetAllTypeIdInfo, GetAllTypeIdInfo, "GetAllTypeIdInfo");
    } else
    {
        LOAD_LIBINFO_FUNC(mGetAllTypeIdInfo, GetAllTypeIdInfo, "GetIDCardType");
    }

    // 13. 获取最近一次的错误码
    LOAD_LIBINFO_FUNC(mGetLastErrorCode, GetLastErrorCode, "GetLastErrorCode");

    // 14. 获取最近一次的错误描述
    LOAD_LIBINFO_FUNC(mGetLastErrorStr, GetLastErrorStr, "GetLastErrorStr");

    // 15. 获取固件版本信息
    LOAD_LIBINFO_FUNC(mGetFWVersion, GetFWVersion, "GetFWVersion");

    // 16. 获取软件版本信息
    LOAD_LIBINFO_FUNC(mGetSWVersion, GetSWVersion, "GetSWVersion");

    // 17. 获取设备状态
    LOAD_LIBINFO_FUNC(mGetDevStatus, GetDevStatus, "GetDevStatus");

    // 18. 复位设备
    LOAD_LIBINFO_FUNC(mResetDevice, ResetDevice, "ResetDevice");

    // 19. 软复位
    LOAD_LIBINFO_FUNC(mSoftResetDevice, SoftResetDevice, "SoftResetDevice");

    // 20. 固件升级
    LOAD_LIBINFO_FUNC(mUpdateOnLine, UpdateOnLine, "UpdateOnLine");

    // 21. CIS校验
    LOAD_LIBINFO_FUNC(mCISCalibrate, CISCalibrate, "CISCalibrate");

    // 22. 传感器校验
    LOAD_LIBINFO_FUNC(mSensorCalibrate, SensorCalibrate, "SensorCalibrate");

    // 23. 设置按键强制退卡使能
    LOAD_LIBINFO_FUNC(mSetButtonEnable, SetButtonEnable, "SetButtonEnable");

    // 24. 设置自动进卡使能
    LOAD_LIBINFO_FUNC(mSetAutoFeedEnable, SetAutoFeedEnable, "SetAutoFeedEnable");

    // 25. 设置上电、复位初始化吸卡模式
    LOAD_LIBINFO_FUNC(mSetInitFeedMode, SetInitFeedMode, "SetInitFeedMode");

    // 26. 设置二代证芯片图像存储格式
    LOAD_LIBINFO_FUNC(mSetHeadFileFormat, SetHeadFileFormat, "SetHeadFileFormat");

    // 27. 通过图像获取通行证信息
    LOAD_LIBINFO_FUNC(mGetID2InfoFromImage, GetID2InfoFromImage, "GetID2InfoFromImage");

    // 28. 从图像获取二代证信息
    LOAD_LIBINFO_FUNC(mGetPassportInfoFromImage, GetPassportInfoFromImage, "GetPassportInfoFromImage");

    // 29. 退卡到识别位置
    LOAD_LIBINFO_FUNC(mBackIdCardToRerec, BackIdCardToRerec, "BackIdCardToRerec");

    return TRUE;
}

void CDevImpl_BSID81::vInitLibFunc()
{
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
}

//------------------------------------------------------------------------------------------------------
INT CDevImpl_BSID81::OpenDevice()
{
    return OpenDevice(0);
}

INT CDevImpl_BSID81::OpenDevice(WORD wType)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    // 如果已打开，则关闭
    if (m_bDevOpenOk == TRUE)
    {
        CloseDevice();
    }

    m_bDevOpenOk = FALSE;

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__,
                    "打开设备: 加载动态库: bLoadLibrary() Failed, Return: %s.",
                    ConvertCode_IMPL2Str(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        }
        m_nRetErrOLD[0] = IMP_SUCCESS;
    }
    m_nRetErrOLD[0] = IMP_SUCCESS;

    // 枚举扫描设备
    nRet = nEnumScannerDevice();
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[4])
        {
            Log(ThisModule, __LINE__,
                "枚举扫描设备: ->nEnumScannerDevice() fail, ErrCode: %d, Return: %s.",
                nRet, ConvertCode_IMPL2Str(nRet));
            m_nRetErrOLD[4] = nRet;
        }
        return nRet;
    }

    // 打开设备
    nRet = nOpenConnection();
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[1])
        {
            Log(ThisModule, __LINE__,
                "打开设备: ->nOpenConnection() fail, ErrCode: %d, Return: %s.",
                nRet, ConvertCode_IMPL2Str(nRet));
            m_nRetErrOLD[1] = nRet;
        }
        return nRet;
    }

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 End......");
    }
    m_bReCon = FALSE; // 是否断线重连状态: 初始F

    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IMP_SUCCESS;
}

INT CDevImpl_BSID81::CloseDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 关闭设备
    if (m_bDevOpenOk == TRUE)
    {
        nCloseConnection();
        m_bsDeviceId = 0;
    }

    //if (m_bReCon == FALSE)  // 非断线重连时关闭需释放动态库
    {
        // 释放动态库
        vUnLoadLibrary();
    }

    // 设备Open标记=F
    m_bDevOpenOk = FALSE;

    if (m_bReCon == FALSE)
    {
        Log(ThisModule, __LINE__, "设备关闭,释放动态库: Close Device(), unLoad Library.");
    }

    return IMP_SUCCESS;
}

BOOL CDevImpl_BSID81::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// 1. 枚举扫描设备
INT CDevImpl_BSID81::nEnumScannerDevice()
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    ScannerInfoRec  ScannerInfo[8] = {0};
    unsigned int iDevNumber = 0;

    // 枚举扫描设备
    nRet = EnumScannerDevice(ScannerInfo, &iDevNumber);
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[4])
        {
            Log(ThisModule, __LINE__,
                "枚举扫描设备: ->EnumScannerDevice() fail, ErrCode: %d, Return: %s.",
                nRet, ConvertCode_IMPL2Str(nRet));
            m_nRetErrOLD[4] = nRet;
        }
        return nRet;
    }

    m_bsDeviceId = ScannerInfo[0].DeviceID;
    //if (nRet != m_nRetErrOLD[4])
    {
        Log(ThisModule, __LINE__,
            "枚举扫描设备: ->EnumScannerDevice() succ. DevNumber = %d, ScannerInfo = %d|%d|%d|%d|%d|%d|%d|%d, "
            "Set m_bsDeviceId = %d",
            iDevNumber, ScannerInfo[0].DeviceID, ScannerInfo[1].DeviceID, ScannerInfo[2].DeviceID, ScannerInfo[3].DeviceID,
            ScannerInfo[4].DeviceID, ScannerInfo[5].DeviceID, ScannerInfo[6].DeviceID, ScannerInfo[7].DeviceID,
            m_bsDeviceId);
        m_nRetErrOLD[4] = nRet;
    }

    return IMP_SUCCESS;
}

// 2. 打开设备
INT CDevImpl_BSID81::nOpenConnection()
{
    THISMODULE(__FUNCTION__);

    // 打开设备
    INT nRet = IMP_SUCCESS;
    nRet = OpenConnection(m_bsDeviceId);
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[1])
        {
            Log(ThisModule, __LINE__,
                "打开设备: OpenConnection(%d) fail, ErrCode: %d, Return: %s.",
                m_bsDeviceId, nRet, ConvertCode_IMPL2Str(nRet));
            m_nRetErrOLD[1] = nRet;
        }
        return nRet;
    }

    //if (nRet != m_nRetErrOLD[1])
    {
        Log(ThisModule, __LINE__, "打开设备:->OpenConnection(%d) succ. ", m_bsDeviceId);
        m_nRetErrOLD[1] = nRet;
    }


    return IMP_SUCCESS;
}

// 3. 关闭设备
INT CDevImpl_BSID81::nCloseConnection()
{
    THISMODULE(__FUNCTION__);

    if (CloseConnection == nullptr)
    {
        Log(ThisModule, __LINE__,
            "关闭设备: ->CloseConnection(%d) fail. CloseConnection is NULL, Not LoadLibrary", m_bsDeviceId);
        return IMP_SUCCESS;
    }

    INT nRet = IMP_SUCCESS;
    nRet = CloseConnection(m_bsDeviceId);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "关闭设备: ->CloseConnection(%d) fail, ErrCode: %d, Return: %s.",
            m_bsDeviceId, nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 4. 检测是否放入卡
INT CDevImpl_BSID81::nCheckHaveIdCard(UINT unChkTime)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = CheckHaveIdCard(m_bsDeviceId, unChkTime);
    if (nRet != IMP_SUCCESS)
    {
        //Log(ThisModule, __LINE__,
        // "检测是否放入卡: ->CheckHaveIdCard(%d, %d) fail, ErrCode: %d, Return: %s.",
        // m_bsDeviceId, unChkTime, nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 6. 启动扫描
INT CDevImpl_BSID81::nStartScanIdCard()
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = StartScanIdCard(m_bsDeviceId);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "启动扫描: ->StartScanIdCard(%d) fail, ErrCode: %d, Return: %s.",
            m_bsDeviceId, nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 7. 读取当前图像数据块到内存
INT CDevImpl_BSID81::nSavePicToMemory(LPSTR lpFrontImgBuf, LPSTR lpRearImgBuf, LPINT lpFrontLen, LPINT lpRearLen)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = SavePicToMemory(m_bsDeviceId, lpFrontImgBuf, lpRearImgBuf, lpFrontLen, lpRearLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "读取当前图像数据块到内存: )->SavePicToMemory(%d) fail. Result: %d|%d, ReturnCode:%s",
            m_bsDeviceId, *lpFrontLen, *lpRearLen, nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 8.1 保存图像数据块到文件
INT CDevImpl_BSID81::nSavePicToFile(LPSTR lpImgBuf, UINT uBufLen, LPSTR lpFileName, INT Format)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = SavePicToFile(m_bsDeviceId, lpImgBuf, uBufLen, lpFileName, Format);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "保存图像数据块到文件: ->SavePicToFile(%d, Data, %d, %s, %d|%s) fail, ErrCode: %d, Return: %s.",
            m_bsDeviceId, uBufLen, lpFileName, Format, (Format == SAVE_IMG_BMP ? "BMP" : "JPG"),
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 8.2 保存图像数据块到文件(指定缩放比例)
INT CDevImpl_BSID81::nSavePicToFileII(LPSTR lpImgBuf, UINT uBufLen, LPSTR lpFileName, INT Format, FLOAT fZoomScale)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = SavePicToFileII(m_bsDeviceId, lpImgBuf, uBufLen, lpFileName, Format, fZoomScale);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "保存图像数据块到文件(指定缩放比例): ->SavePicToFileII(%d, Data, %d, %s, %d|%s, %.2f) fail, ErrCode: %d, Return: %s.",
            m_bsDeviceId, uBufLen, lpFileName, Format,
            (Format == SAVE_IMG_BMP ? "BMP" : "JPG"), fZoomScale,
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 9. 吞卡
INT CDevImpl_BSID81::nRetainIdCard()
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = RetainIdCard(m_bsDeviceId);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "吞卡: ->RetainIdCard() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 10. 退卡
INT CDevImpl_BSID81::nBackIdCard()
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = BackIdCard(m_bsDeviceId);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "退卡: ->BackIdCard() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 11. 出卡并持卡
INT CDevImpl_BSID81::nBackAndHoldIdCard()
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = BackAndHoldIdCard(m_bsDeviceId);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "出卡并持卡: ->BackAndHoldIdCard() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 12.1 获取二代证信息(国内)
INT CDevImpl_BSID81::nGetID2Info(IDInfo &stIDCard, LPSTR lpHeadImageName)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = GetID2Info(m_bsDeviceId, &stIDCard, lpHeadImageName);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取二代证信息(国内): ->GetID2Info() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 12.2 获取二代证信息(国内,带指纹)
INT CDevImpl_BSID81::nGetID2InfoEx(IDInfoEx &stIDCard, LPSTR lpHeadImageName)
{
    THISMODULE(__FUNCTION__);

    IDInfoEx_GBK stIDCard_GBK;

    INT nRet = IMP_SUCCESS;

    if (m_wLoadDllVer == 0) // V2.01
    {
        nRet = GetID2InfoEx(m_bsDeviceId, &stIDCard, lpHeadImageName);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "获取二代证信息(国内,带指纹): V2.0(UTF8): ->GetID2InfoEx() fail, ErrCode: %d, Return: %s.",
                nRet, ConvertCode_IMPL2Str(nRet));
            return nRet;
        }
    } else
    {
        nRet = GetID2InfoEx(m_bsDeviceId, (IDInfoEx*)&stIDCard_GBK, lpHeadImageName);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "获取二代证信息(国内,带指纹): V3.0(GBK): ->GetID2InfoEx_GBK() fail, ErrCode: %d, Return: %s.",
                nRet, ConvertCode_IMPL2Str(nRet));
            return nRet;
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

    return IMP_SUCCESS;
}

// 12.3 获取二代证信息(国外)
INT CDevImpl_BSID81::nGetIDInfoForeign(IDInfoForeign &stIDCard, LPSTR lpHeadImageName)
{
    THISMODULE(__FUNCTION__);

    IDInfoForeign_GBK stIDCard_GBK;

    INT nRet = IMP_SUCCESS;

    if (m_wLoadDllVer == 0) // V2.01
    {
        nRet = GetIDInfoForeign(m_bsDeviceId, &stIDCard, lpHeadImageName);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "获取二代证信息(国外): V2.0(UTF8): ->GetIDInfoForeign() fail, ErrCode: %d, Return: %s.",
                nRet, ConvertCode_IMPL2Str(nRet));
            return nRet;
        }
    } else
    {
        nRet = GetIDInfoForeign(m_bsDeviceId, (IDInfoForeign*)&stIDCard_GBK, lpHeadImageName);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "获取二代证信息(国外): V3.0(GBK): ->GetIDInfoForeign() fail, ErrCode: %d, Return: %s.", nRet, ConvertCode_IMPL2Str(nRet));
            return nRet;
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

    return IMP_SUCCESS;
}
// 12.4 获取二代证信息(港澳台通行证)
INT CDevImpl_BSID81::nGetIDInfoGAT(IDInfoGAT &stIDCard, LPSTR lpHeadImageName)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    IDInfoGAT_GBK stIDCard_GBK;

    if (m_wLoadDllVer == 0) // V2.01
    {
        nRet = GetIDInfoGAT(m_bsDeviceId, &stIDCard, lpHeadImageName);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "获取二代证信息(港澳台通行证): V2.0(UTF8): ->GetIDInfoGAT() fail, ErrCode: %d, Return: %s.",
                nRet, ConvertCode_IMPL2Str(nRet));
            return nRet;
        }
    } else
    {
        nRet = GetIDInfoGAT(m_bsDeviceId, (IDInfoGAT*)&stIDCard_GBK, lpHeadImageName);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "获取二代证信息(港澳台通行证): V3.0(GBK): ->GetIDInfoGAT() fail, ErrCode: %d, Return: %s.",
                nRet, ConvertCode_IMPL2Str(nRet));
            return nRet;
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

    return IMP_SUCCESS;
}

// 12.5 获取二代证信息(卡类型)
INT CDevImpl_BSID81::nGetIDCardType(LPINT lpCardType)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = GetIDCardType(m_bsDeviceId, lpCardType);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取二代证信息(卡类型): ->bGetIDCardType(%d) fail, ErrCode: %d, Return: %s.",
            m_bsDeviceId, nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 15. 获取固件版本信息
INT CDevImpl_BSID81::nGetFWVersion(LPSTR lpFwVer)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = GetFWVersion(m_bsDeviceId, lpFwVer);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取固件版本信息: ->GetFWVersion(%d) fail, ErrCode: %d, Return: %s.",
            m_bsDeviceId, nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 16. 获取软件版本信息
INT CDevImpl_BSID81::nGetSWVersion(LPSTR lpSwVer)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = GetSWVersion(lpSwVer);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "获取软件版本信息: ->GetSWVersion() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 17. 获取设备状态
INT CDevImpl_BSID81::nGetDevStatus(DEVSTATUS &devStatus)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = GetDevStatus(m_bsDeviceId, &devStatus);
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[2])
        {
            Log(ThisModule, __LINE__,
                "获取设备状态: ->GetDevStatus() fail, ErrCode: %d, Return: %s.",
                nRet, ConvertCode_IMPL2Str(nRet));
            m_nRetErrOLD[2] = nRet;
        }
        return nRet;
    }

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

    return IMP_SUCCESS;
}

// 18. 复位设备
INT CDevImpl_BSID81::nResetDevice()
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    nRet = ResetDevice(m_bsDeviceId);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "硬复位设备: ->ResetDevice() fail, ErrCode: %d, Return: %s.",
            nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    if (OpenDevice() != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "硬复位设备后重新连接: ->OpenDevice() fail. ");
        return nRet;
    }

    return IMP_SUCCESS;
}

// 19. 软复位设备
INT CDevImpl_BSID81::nSoftResetDevice(int iMode)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    if (iMode < CARD_EJECT || iMode > CARD_NOACTION)
    {
        Log(ThisModule, __LINE__,
            "软复位设备: input mode invalid fail. mode<%d> != <%d|%d|%d|%d>",
            iMode, CARD_EJECT, CARD_RETRACT, CARD_EJECTMENT, CARD_NOACTION);
        return nRet;
    }

    nRet = SoftResetDevice(m_bsDeviceId, iMode);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "软复位设备: ->SoftResetDevice(%d, %d) fail, ErrCode: %d, Return: %s.",
            m_bsDeviceId, iMode, nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 29. 退卡到可识别位置
INT CDevImpl_BSID81::nBackIdCardToRerec()
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;
    nRet = BackIdCardToRerec(m_bsDeviceId);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "退卡到可识别位置: ->BackIdCardToRerec(%d) fail, ErrCode: %d, Return: %s.",
            m_bsDeviceId, nRet, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
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

// 设置断线重连标记
INT CDevImpl_BSID81::SetReConFlag(BOOL bFlag)
{
    THISMODULE(__FUNCTION__);

    if (m_bReCon == FALSE && bFlag == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 Start......");
    }
    m_bReCon = bFlag;

    return IMP_SUCCESS;
}

LPSTR CDevImpl_BSID81::ConvertCode_IMPL2Str(INT nErrCode)
{
#define BSID81_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStr, "%d|%s", CODE, STR); \
        return m_szErrStr;

    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(nErrCode)
    {
        //Impl处理返回
        BSID81_CASE_CODE_STR(IMP_SUCCESS, nErrCode, "成功");
        BSID81_CASE_CODE_STR(IMP_ERR_LOAD_LIB, nErrCode, "动态库加载失败");
        BSID81_CASE_CODE_STR(IMP_ERR_PARAM_INVALID, nErrCode, "参数无效");
        BSID81_CASE_CODE_STR(IMP_ERR_UNKNOWN, nErrCode, "未知错误");
        BSID81_CASE_CODE_STR(IMP_ERR_NOTOPEN, nErrCode, "设备未Open");
        //Device返回, nErrCode, "");
        //BSID81_CASE_CODE_STR(IMP_ERR_DEV_00H, nErrCode, "正常");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_01H, nErrCode, "无设备");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_02H, nErrCode, "端口错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_03H, nErrCode, "参数文件错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_04H, nErrCode, "未初始化");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_05H, nErrCode, "无效参数");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_06H, nErrCode, "超时错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_07H, nErrCode, "上盖打开");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_08H, nErrCode, "塞卡");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_09H, nErrCode, "内存溢出");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_0AH, nErrCode, "没有二代证数据");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_0BH, nErrCode, "没有图像数据");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_0CH, nErrCode, "图像处理错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_0DH, nErrCode, "判断图像方向错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_0EH, nErrCode, "关闭端口失败");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_0FH, nErrCode, "身份证电子信息处理错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_10H, nErrCode, "传感器校验错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_11H, nErrCode, "电压低");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_12H, nErrCode, "校正错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_13H, nErrCode, "无卡");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_14H, nErrCode, "未知错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_15H, nErrCode, "保存位图错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_16H, nErrCode, "掉电错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_17H, nErrCode, "BOOT错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_18H, nErrCode, "按键抬起");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_19H, nErrCode, "识别错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_1AH, nErrCode, "扫描错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_1BH, nErrCode, "走卡错误");
        BSID81_CASE_CODE_STR(IMP_ERR_DEV_1CH, nErrCode, "最大错误码");
        default :
            sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStr;
}


//---------------------------------------------
