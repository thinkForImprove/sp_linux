/***************************************************************
* 文件名称：DevImpl_CRT603CZ7.cpp
* 文件描述：创自非接模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月18日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_CRT603CZ7.h"
#include "QtTypeDef.h"
#include "QtDLLLoader.h"

static const char *ThisFile = "DevImpl_CRT603CZ7.cpp";

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

CDevImpl_CRT603CZ7::CDevImpl_CRT603CZ7()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_CRT603CZ7::CDevImpl_CRT603CZ7(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_CRT603CZ7::Init()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_wReaderCnt = 0;                                      // 连接的读卡器数目
    memset(m_szReaderList, 0x00, sizeof(m_szReaderList));  // 连接的读卡器列表
    m_bDevOpenOk = FALSE;
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    memset(m_szFWVer, 0x00, sizeof(m_szFWVer));

    m_vDlHandle = nullptr;

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("FIDC/CRT603CZ7/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_bLoadIntfFail = TRUE;

    m_bReCon = FALSE;                       // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    // 动态库接口函数初始化
    vInitLibFunc();
}

CDevImpl_CRT603CZ7::~CDevImpl_CRT603CZ7()
{
    //vUnLoadLibrary();
    vDlUnLoadLibrary();
}

BOOL CDevImpl_CRT603CZ7::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库<%s> fail. ReturnCode:%s.",
                    m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> fail. ReturnCode:%s.",
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

void CDevImpl_CRT603CZ7::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
        vInitLibFunc(); // 动态库接口函数初始化
    }
}

BOOL CDevImpl_CRT603CZ7::bLoadLibIntf()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = FALSE;

    // 1.打开CRT智能读卡器
    LOAD_LIBINFO_FUNC(pCRT_OpenConnect, CRT_OpenConnect, "CRT_OpenConnect");

    // 2.关闭CRT智能读卡器
    LOAD_LIBINFO_FUNC(pCRT_CloseConnect, CRT_CloseConnect, "CRT_CloseConnect");

    // 3.获取智能读卡器列表名字
    LOAD_LIBINFO_FUNC(pCRT_GetReaderListName, CRT_GetReaderListName, "CRT_GetReaderListName");

    // 4.设置当前工作的读卡器序列号
    LOAD_LIBINFO_FUNC(pCRT_SetReaderName, CRT_SetReaderName, "CRT_SetReaderName");

    // 5.获取读卡器上卡状态
    LOAD_LIBINFO_FUNC(pCRT_GetCardStatus, CRT_GetCardStatus, "CRT_GetCardStatus");

    // 6.弹卡下电, 读卡器断开连接
    LOAD_LIBINFO_FUNC(pCRT_EjectCard, CRT_EjectCard, "CRT_EjectCard");

    // 7.读卡器卡片上电
    LOAD_LIBINFO_FUNC(pCRT_ReaderConnect, CRT_ReaderConnect, "CRT_ReaderConnect");

    // 8.读卡器发送APDU指令
    LOAD_LIBINFO_FUNC(pCRT_SendApdu, CRT_SendApdu, "CRT_SendApdu");

    // 9.读卡器发送扩展控制命令
    LOAD_LIBINFO_FUNC(pCRT_SendControlCMD, CRT_SendControlCMD, "CRT_SendControlCMD");

    // 10.读卡器热复位
    LOAD_LIBINFO_FUNC(pCRT_HotReset, CRT_HotReset, "CRT_HotReset");

    // 11.设置读卡器操作模式
    LOAD_LIBINFO_FUNC(pCRT_SetReaderType, CRT_SetReaderType, "CRT_SetReaderType");

    // 12.获取读卡器操作模式
    LOAD_LIBINFO_FUNC(pCRT_GetReaderType, CRT_GetReaderType, "CRT_GetReaderType");

    // 13.获取读卡器版本信息
    LOAD_LIBINFO_FUNC(pCRT_GetVersionInfo, CRT_GetVersionInfo, "CRT_GetVersionInfo");

    // 14.读卡器自动蜂鸣
    LOAD_LIBINFO_FUNC(pCRT_AutoBeel, CRT_AutoBeel, "CRT_AutoBeel");

    // 15.读卡器蜂鸣设置
    LOAD_LIBINFO_FUNC(pCRT_Beel, CRT_Beel, "CRT_Beel");

    // 16.设置读卡器指示灯模式
    LOAD_LIBINFO_FUNC(pCRT_SetLightMode, CRT_SetLightMode, "CRT_SetLightMode");

    // 17.获取读卡器指示灯模式
    LOAD_LIBINFO_FUNC(pCRT_GetLightMode, CRT_GetLightMode, "CRT_GetLightMode");

    // 18.设置读卡器指示灯状态
    LOAD_LIBINFO_FUNC(pCRT_SetLightStatus, CRT_SetLightStatus, "CRT_SetLightStatus");

    // 19.获取读卡器指示灯状态
    LOAD_LIBINFO_FUNC(pCRT_GetLightStatus, CRT_GetLightStatus, "CRT_GetLightStatus");

    // 20.设置读卡器读取TYPEB卡能力
    LOAD_LIBINFO_FUNC(pCRT_BanTypeBCap, CRT_BanTypeBCap, "CRT_BanTypeBCap");

    // 21.Mifare卡下载密码
    LOAD_LIBINFO_FUNC(pCRT_LoadMifareKey, CRT_LoadMifareKey, "CRT_LoadMifareKey");

    // 22.Mifare卡校验密码
    LOAD_LIBINFO_FUNC(pCRT_CheckMifareKey, CRT_CheckMifareKey, "CRT_CheckMifareKey");

    // 23.非CPU卡读数据操作
    LOAD_LIBINFO_FUNC(pCRT_Read, CRT_Read, "CRT_Read");

    // 24.非CPU卡写数据操作
    LOAD_LIBINFO_FUNC(pCRT_Write, CRT_Write, "CRT_Write");

    // 25.获取卡片UID信息
    LOAD_LIBINFO_FUNC(pCRT_GetCardUID, CRT_GetCardUID, "CRT_GetCardUID");

    // 26.SAM卡切换并激活卡座
    LOAD_LIBINFO_FUNC(pCRT_SAMSlotActivation, CRT_SAMSlotActivation, "CRT_SAMSlotActivation");

    // 27.读所有磁道操作
    LOAD_LIBINFO_FUNC(pCRT_ReadMagAllTracks, CRT_ReadMagAllTracks, "CRT_ReadMagAllTracks");

    // 28.获取RF卡类型
    LOAD_LIBINFO_FUNC(pCRT_GetRFCardType, CRT_GetRFCardType, "CRT_GetRFCardType");

    // 29.读二代证信息
    LOAD_LIBINFO_FUNC(pCRT_ReadIDCardInifo, CRT_ReadIDCardInifo, "CRT_ReadIDCardInifo");

    // 30.M1卡值操作
    LOAD_LIBINFO_FUNC(pCRT_M1ValueProcess, CRT_M1ValueProcess, "CRT_M1ValueProcess");

    // 31.M1卡查询余额
    LOAD_LIBINFO_FUNC(pCRT_M1InquireBalance, CRT_M1InquireBalance, "CRT_M1InquireBalance");

    // 32.M1卡备份钱包
    LOAD_LIBINFO_FUNC(pCRT_M1BackBlock, CRT_M1BackBlock, "CRT_M1BackBlock");

    // 33.获取二代证指纹信息
    LOAD_LIBINFO_FUNC(pCRT_GetIDFinger, CRT_GetIDFinger, "CRT_GetIDFinger");

    // 34.获取二代证DN号
    LOAD_LIBINFO_FUNC(pCRT_GetIDDNNums, CRT_GetIDDNNums, "CRT_GetIDDNNums");

    // 35.获取身份证盒子SAMID
    LOAD_LIBINFO_FUNC(pCRT_GetSAMID, CRT_GetSAMID, "CRT_GetSAMID");

    // 36.读卡器开关射频场
    LOAD_LIBINFO_FUNC(pCRT_SwitchRF, CRT_SwitchRF, "CRT_SwitchRF");

    // 37.获取最后一次错误描叙
    LOAD_LIBINFO_FUNC(pCRT_GetLastError, CRT_GetLastError, "CRT_GetLastError");

    // 38. 设置身份证头像保存路径
    LOAD_LIBINFO_FUNC(pCRT_SetHeadPath, CRT_SetHeadPath, "CRT_SetHeadPath");

    return TRUE;
}

void CDevImpl_CRT603CZ7::vInitLibFunc()
{
    // 动态库接口函数初始化
    CRT_OpenConnect         = nullptr;      // 1. 打开CRT智能读卡器
    CRT_CloseConnect        = nullptr;      // 2. 关闭CRT智能读卡器
    CRT_GetReaderListName   = nullptr;      // 3. 获取智能读卡器列表名字
    CRT_SetReaderName       = nullptr;      // 4. 设置当前工作的读卡器序列号
    CRT_GetCardStatus       = nullptr;      // 5. 获取读卡器上卡状态
    CRT_EjectCard           = nullptr;      // 6. 弹卡下电,读卡器断开连接
    CRT_ReaderConnect       = nullptr;      // 7. 读卡器卡片上电
    CRT_SendApdu            = nullptr;      // 8. 读卡器发送APDU指令
    CRT_SendControlCMD      = nullptr;      // 9. 读卡器发送扩展控制命令
    CRT_HotReset            = nullptr;      // 10. 读卡器热复位
    CRT_SetReaderType       = nullptr;      // 11. 设置读卡器操作模式
    CRT_GetReaderType       = nullptr;      // 12. 获取读卡器操作模式
    CRT_GetVersionInfo      = nullptr;      // 13. 获取读卡器版本信息
    CRT_AutoBeel            = nullptr;      // 14. 读卡器自动蜂鸣
    CRT_Beel                = nullptr;      // 15. 读卡器蜂鸣设置
    CRT_SetLightMode        = nullptr;      // 16. 设置读卡器指示灯模式
    CRT_GetLightMode        = nullptr;      // 17. 获取读卡器指示灯模式
    CRT_SetLightStatus      = nullptr;      // 18. 设置读卡器指示灯状态
    CRT_GetLightStatus      = nullptr;      // 19. 获取读卡器指示灯状态
    CRT_BanTypeBCap         = nullptr;      // 20. 设置读卡器读取TYPE B卡能力
    CRT_LoadMifareKey       = nullptr;      // 21. Mifare卡下载密码
    CRT_CheckMifareKey      = nullptr;      // 22. Mifare卡校验密码
    CRT_Read                = nullptr;      // 23. 非CPU卡读数据操作
    CRT_Write               = nullptr;      // 24. 非CPU卡写数据操作
    CRT_GetCardUID          = nullptr;      // 25. 获取卡片UID信息
    CRT_SAMSlotActivation   = nullptr;      // 26. SAM卡切换并激活卡座
    CRT_ReadMagAllTracks    = nullptr;      // 27. 读所有磁道操作
    CRT_GetRFCardType       = nullptr;      // 28. 获取RF卡类型
    CRT_ReadIDCardInifo     = nullptr;      // 29. 读二代证信息
    CRT_M1ValueProcess      = nullptr;      // 30. M1卡值操作
    CRT_M1InquireBalance    = nullptr;      // 31. M1卡查询余额
    CRT_M1BackBlock         = nullptr;      // 32. M1卡备份钱包
    CRT_GetIDFinger         = nullptr;      // 33. 获取二代证指纹信息
    CRT_GetIDDNNums         = nullptr;      // 34. 获取二代证DN号
    CRT_GetSAMID            = nullptr;      // 35. 获取身份证盒子SAM ID
    CRT_SwitchRF            = nullptr;      // 36. 读卡器开关射频场
    CRT_GetLastError        = nullptr;      // 37. 获取最后一次错误描叙
    CRT_SetHeadPath         = nullptr;      // 38. 设置身份证头像保存路径
}

BOOL CDevImpl_CRT603CZ7::bDlLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = TRUE;

    if (m_vDlHandle == nullptr)
    {
        m_vDlHandle = dlopen(m_szLoadDllPath, RTLD_LAZY);
        if (m_vDlHandle == nullptr)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "dlxxx方式加载动态库<%s> fail.", m_szLoadDllPath);
            }
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "dlxxx方式加载动态库<%s> succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bDlLoadLibIntf() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> fail. ", m_szLoadDllPath);
            }
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

void CDevImpl_CRT603CZ7::vDlUnLoadLibrary()
{
    if (m_vDlHandle != nullptr)
    {
        dlclose(m_vDlHandle);
        m_vDlHandle = nullptr;
        m_bLoadIntfFail = TRUE;
        vInitLibFunc(); // 动态库接口函数初始化
    }
}

BOOL CDevImpl_CRT603CZ7::bDlLoadLibIntf()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = FALSE;

    // 1.打开CRT智能读卡器
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_OpenConnect, CRT_OpenConnect, "CRT_OpenConnect");

    // 2.关闭CRT智能读卡器
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_CloseConnect, CRT_CloseConnect, "CRT_CloseConnect");

    // 3.获取智能读卡器列表名字
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetReaderListName, CRT_GetReaderListName, "CRT_GetReaderListName");

    // 4.设置当前工作的读卡器序列号
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_SetReaderName, CRT_SetReaderName, "CRT_SetReaderName");

    // 5.获取读卡器上卡状态
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetCardStatus, CRT_GetCardStatus, "CRT_GetCardStatus");

    // 6.弹卡下电, 读卡器断开连接
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_EjectCard, CRT_EjectCard, "CRT_EjectCard");

    // 7.读卡器卡片上电
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_ReaderConnect, CRT_ReaderConnect, "CRT_ReaderConnect");

    // 8.读卡器发送APDU指令
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_SendApdu, CRT_SendApdu, "CRT_SendApdu");

    // 9.读卡器发送扩展控制命令
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_SendControlCMD, CRT_SendControlCMD, "CRT_SendControlCMD");

    // 10.读卡器热复位
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_HotReset, CRT_HotReset, "CRT_HotReset");

    // 11.设置读卡器操作模式
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_SetReaderType, CRT_SetReaderType, "CRT_SetReaderType");

    // 12.获取读卡器操作模式
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetReaderType, CRT_GetReaderType, "CRT_GetReaderType");

    // 13.获取读卡器版本信息
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetVersionInfo, CRT_GetVersionInfo, "CRT_GetVersionInfo");

    // 14.读卡器自动蜂鸣
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_AutoBeel, CRT_AutoBeel, "CRT_AutoBeel");

    // 15.读卡器蜂鸣设置
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_Beel, CRT_Beel, "CRT_Beel");

    // 16.设置读卡器指示灯模式
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_SetLightMode, CRT_SetLightMode, "CRT_SetLightMode");

    // 17.获取读卡器指示灯模式
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetLightMode, CRT_GetLightMode, "CRT_GetLightMode");

    // 18.设置读卡器指示灯状态
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_SetLightStatus, CRT_SetLightStatus, "CRT_SetLightStatus");

    // 19.获取读卡器指示灯状态
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetLightStatus, CRT_GetLightStatus, "CRT_GetLightStatus");

    // 20.设置读卡器读取TYPEB卡能力
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_BanTypeBCap, CRT_BanTypeBCap, "CRT_BanTypeBCap");

    // 21.Mifare卡下载密码
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_LoadMifareKey, CRT_LoadMifareKey, "CRT_LoadMifareKey");

    // 22.Mifare卡校验密码
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_CheckMifareKey, CRT_CheckMifareKey, "CRT_CheckMifareKey");

    // 23.非CPU卡读数据操作
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_Read, CRT_Read, "CRT_Read");

    // 24.非CPU卡写数据操作
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_Write, CRT_Write, "CRT_Write");

    // 25.获取卡片UID信息
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetCardUID, CRT_GetCardUID, "CRT_GetCardUID");

    // 26.SAM卡切换并激活卡座
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_SAMSlotActivation, CRT_SAMSlotActivation, "CRT_SAMSlotActivation");

    // 27.读所有磁道操作
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_ReadMagAllTracks, CRT_ReadMagAllTracks, "CRT_ReadMagAllTracks");

    // 28.获取RF卡类型
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetRFCardType, CRT_GetRFCardType, "CRT_GetRFCardType");

    // 29.读二代证信息
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_ReadIDCardInifo, CRT_ReadIDCardInifo, "CRT_ReadIDCardInifo");

    // 30.M1卡值操作
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_M1ValueProcess, CRT_M1ValueProcess, "CRT_M1ValueProcess");

    // 31.M1卡查询余额
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_M1InquireBalance, CRT_M1InquireBalance, "CRT_M1InquireBalance");

    // 32.M1卡备份钱包
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_M1BackBlock, CRT_M1BackBlock, "CRT_M1BackBlock");

    // 33.获取二代证指纹信息
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetIDFinger, CRT_GetIDFinger, "CRT_GetIDFinger");

    // 34.获取二代证DN号
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetIDDNNums, CRT_GetIDDNNums, "CRT_GetIDDNNums");

    // 35.获取身份证盒子SAMID
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetSAMID, CRT_GetSAMID, "CRT_GetSAMID");

    // 36.读卡器开关射频场
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_SwitchRF, CRT_SwitchRF, "CRT_SwitchRF");

    // 37.获取最后一次错误描叙
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_GetLastError, CRT_GetLastError, "CRT_GetLastError");

    // 38. 设置身份证头像保存路径
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCRT_SetHeadPath, CRT_SetHeadPath, "CRT_SetHeadPath");

    return TRUE;
}

//------------------------------------------------------------------------------------------------------
// 打开指定设备
INT CDevImpl_CRT603CZ7::OpenDevice()
{
    return OpenDevice(nullptr);
}

// 打开指定设备(指定参数)
INT CDevImpl_CRT603CZ7::OpenDevice(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

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
        //if (bLoadLibrary() != TRUE)
        if (bDlLoadLibrary() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "打开设备: 加载动态库: bLoadLibrary() Failed, Return: %s.",
                    ConvertCode_IMPL2Str(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        }
        m_nRetErrOLD[0] = IMP_SUCCESS;
    }
    m_nRetErrOLD[0] = IMP_SUCCESS;

    //-----------------------打开设备-----------------------
    // 1. 打开CRT智能读卡器
    nRet = CRT_OpenConnect((INT*)&m_wReaderCnt);
    if (nRet != IMP_SUCCESS)
    {
        if (m_nRetErrOLD[1] != IMP_ERR_NOTOPEN)
        {
            Log(ThisModule, __LINE__, "打开设备: ->CRT_OpenConnect(%d) Fail. ErrCode: %d, Return: %s",
                m_wReaderCnt, nRet, ConvertCode_IMPL2Str(IMP_ERR_NOTOPEN));
            m_nRetErrOLD[1] = IMP_ERR_NOTOPEN;
        }
        return IMP_ERR_NOTOPEN;
    } else
    {
        if (m_wReaderCnt < 1)
        {
            if (m_nRetErrOLD[1] != IMP_ERR_NODEVICE)
            {
                Log(ThisModule, __LINE__, "打开设备: ->CRT_OpenConnect(%d) Succ, Device Count[%d] < 1,"
                                          "没有连接设备, Return: %s",
                    m_wReaderCnt, nRet, ConvertCode_IMPL2Str(IMP_ERR_NODEVICE));
                m_nRetErrOLD[1] = IMP_ERR_NODEVICE;
            }
            return IMP_ERR_NODEVICE;
        }

        Log(ThisModule, __LINE__, "打开设备: ->CRT_OpenConnect() Succ, Device Count",
            m_wReaderCnt);
    }

    // 获取智能读卡器列表名字
    CHAR szDevList[32 * 129] = { 0x00 };
    for (INT i = 0; i < m_wReaderCnt; i ++)
    {
        nRet = CRT_GetReaderListName(i, m_szReaderList[i]);
        if (nRet != IMP_SUCCESS)
        {
            sprintf(szDevList + strlen(szDevList), "%d: Errcode: %d|", i, nRet);
        } else
        {
            sprintf(szDevList + strlen(szDevList), "%d: %s|", i, m_szReaderList[i]);
        }
    }

    if (m_nRetErrOLD[6] != 1)
    {
        Log(ThisModule, __LINE__, "获取智能读卡器列表: %s", szDevList);
        m_nRetErrOLD[6] = 1;
    }

    // 设置当前工作的读卡器序列号
    nRet = CRT_SetReaderName(0);
    if (nRet != IMP_SUCCESS)
    {
        if (m_nRetErrOLD[5] != nRet)
        {
            Log(ThisModule, __LINE__,
                "打开设备: 设置当前工作的读卡器序列号: ->CRT_SetReaderName(%d) Fail. ErrCode: %d, Return: %d",
                0, nRet, IMP_ERR_NOTOPEN);
            m_nRetErrOLD[5] = nRet;
        }
        return IMP_ERR_NOTOPEN;

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

// 关闭设备
INT CDevImpl_CRT603CZ7::CloseDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 关闭设备
    CRT_CloseConnect();

    if (m_bReCon == FALSE)  // 非断线重连时关闭需释放动态库
    {
        // 释放动态库
        //vUnLoadLibrary();
        vDlUnLoadLibrary();
    }

    // 设备Open标记=F
    m_bDevOpenOk = FALSE;

    if (m_bReCon == FALSE)
    {
        Log(ThisModule, __LINE__, "设备关闭,释放动态库: Close Device(), unLoad Library.");
    }

    return IMP_SUCCESS;
}

// 设备是否Open成功
BOOL CDevImpl_CRT603CZ7::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

//------封装动态库接口------------------------------------------------------------------------------------
// 5. 获取读卡器上卡状态
INT CDevImpl_CRT603CZ7::GetCardStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_GetCardStatus();
    if (nRet <= IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[3])
        {
            Log(ThisModule, __LINE__, "获取读卡器上卡状态: CRT_GetCardStatus() fail. Return: %s",
                ConvertCode_IMPL2Str(nRet));
            m_nRetErrOLD[3] = nRet;
        }
        return nRet;
    }

    return nRet;
}

// 6. 弹卡下电,读卡器断开连接
INT CDevImpl_CRT603CZ7::SetEjectCard()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_EjectCard();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "弹卡下电,读卡器断开连接: CRT_EjectCard() fail. Return: %s",
            ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 7. 读卡器卡片上电
INT CDevImpl_CRT603CZ7::SetReaderPowerOn(LPUCHAR lpData, INT &nDataLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    //BYTE byAtrData[1024] =  {0};
    nRet = CRT_ReaderConnect(lpData, &nDataLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡器卡片上电: CRT_ReaderConnect(%s, %d) fail. Return: %s",
            lpData, nDataLen, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 8. 读卡器发送APDU指令
INT CDevImpl_CRT603CZ7::SendReaderAPDU(LPUCHAR lpSndData, INT nSndLen,
                                       LPUCHAR lpRcvData, INT &nRcvLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_SendApdu(lpSndData, nSndLen, lpRcvData, &nRcvLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "弹卡下电,读卡器断开连接: CRT_SendApdu(%s, %d, %s, %d) fail. Return: %s",
            lpSndData, nSndLen, lpRcvData, nRcvLen, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 10. 读卡器热复位
INT CDevImpl_CRT603CZ7::ReaderReset()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_HotReset();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡器热复位: CRT_HotReset() fail. Return: %s",
            ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 11. 设置读卡器操作模式
INT CDevImpl_CRT603CZ7::SetReaderMode(EN_CARDER_MODE enMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_SetReaderType(enMode);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置读卡器操作模式: CRT_SetReaderType(%d) fail. Return: %s",
            enMode, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 12. 获取读卡器操作模式
INT CDevImpl_CRT603CZ7::GetReaderMode()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_GetReaderType();
    if (nRet < IMP_SUCCESS) // <0失败
    {
        Log(ThisModule, __LINE__, "获取读卡器操作模式: CRT_GetReaderType() fail. Return: %s",
            ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return nRet;
}

// 13. 获取读卡器版本信息
INT CDevImpl_CRT603CZ7::GetReaderVersion(LPSTR lpVerStr, WORD wVerSize, EN_VER_TYPE enVerType)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    CHAR szVerBuffer[256] = { 0x00 };

    nRet = CRT_GetVersionInfo(enVerType, szVerBuffer);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取版本信息: CRT_GetVersionInfo(%d, %s) fail. Return: %s",
            enVerType, szVerBuffer, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    memcpy(lpVerStr, szVerBuffer, (strlen(szVerBuffer) > wVerSize ? wVerSize : strlen(szVerBuffer)));

    return IMP_SUCCESS;
}

// 14. 读卡器自动蜂鸣设置
INT CDevImpl_CRT603CZ7::SetBeepAuto(BOOL bIsAuto)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_AutoBeel(bIsAuto);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡器自动蜂鸣设置: CRT_AutoBeel(%d) fail. Return: %s",
            bIsAuto, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 15. 读卡器蜂鸣设置
INT CDevImpl_CRT603CZ7::SetBeepCont(INT nMesc)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    // 入参: 0.25秒的倍数, 比如2, 蜂鸣时间=2*0.25,即0.5秒)(0-20默认为1)
    INT nMulti = (nMesc / 250) <= 0 ? 1 : (nMesc / 250);

    nRet = CRT_Beel(nMulti);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡器蜂鸣设置: CRT_Beel(%d|nMesc(%d)/250) fail. Return: %s",
            nMulti, nMesc, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 16. 设置读卡器指示灯模式
INT CDevImpl_CRT603CZ7::SetLightAuto(INT nMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_SetLightMode(nMode); //指示灯模式(0:自动模式, 1:手动模式)
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置读卡器指示灯模式: CRT_SetLightMode(%d) fail. Return: %s",
            nMode, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 17. 获取读卡器指示灯模式
INT CDevImpl_CRT603CZ7::GetLightAuto()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_GetLightMode();
    if (nRet < IMP_SUCCESS) // <0失败
    {
        Log(ThisModule, __LINE__, "获取读卡器指示灯模式: CRT_GetLightMode() fail. Return: %s",
            ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return nRet;
}

// 18. 设置读卡器指示灯状态
INT CDevImpl_CRT603CZ7::SetLightStat(INT nYellow, INT nBlue, INT nGreen, INT Red)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_SetLightStatus(nYellow, nBlue, nGreen, Red);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置读卡器指示灯状态: CRT_SetLightStatus(%d, %d, %d, %d) fail. Return: %s",
            nYellow, nBlue, nGreen, Red, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 19. 获取读卡器指示灯状态
INT CDevImpl_CRT603CZ7::GetLightStat(INT &nYellow, INT &nBlue, INT &nGreen, INT &Red)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_GetLightStatus(&nYellow, &nBlue, &nGreen, &Red);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取读卡器指示灯状态: CRT_GetLightStatus(%d, %d, %d, %d) fail. Return: %s",
            nYellow, nBlue, nGreen, Red, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 20. 设置关闭读卡器读取TYPE B卡能力
INT CDevImpl_CRT603CZ7::SetCloseTypeBCap(BOOL bIsClose)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_BanTypeBCap(bIsClose);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置关闭读卡器读取TYPE B卡能力: CRT_BanTypeBCap(%d) fail. Return: %s",
            bIsClose, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 28. 获取RF卡类型
INT CDevImpl_CRT603CZ7::GetCardRFType()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_GetRFCardType();
    if (nRet != CARDRF_NOHAVE && nRet != CARDRF_TYPEA &&
        nRet != CARDRF_TYPEB && nRet != CARDRF_IDCARD)
    {
        Log(ThisModule, __LINE__, "获取RF卡类型: CRT_GetRFCardType() fail. Return: %s",
            ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 29. 读二代证信息
INT CDevImpl_CRT603CZ7::GetIDCardInfo(LPSTCRTDEFIDINFO lpStInfo)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_ReadIDCardInifo(lpStInfo);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读二代证信息: CRT_ReadIDCardInifo() fail. Return: %s",
            ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 33. 获取二代证指纹信息
INT CDevImpl_CRT603CZ7::GetIDCardFinger(LPBYTE lpFinger, INT &nLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_GetIDFinger(lpFinger, &nLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取二代证指纹信息: CRT_GetIDFinger() fail. Return: %s",
            ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}
// 34. 获取二代证DN号
INT CDevImpl_CRT603CZ7::GetIDCardDN(LPSTR lpDN)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_GetIDDNNums(lpDN);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取二代证DN号: CRT_GetIDDNNums() fail. Return: %s",
            ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 35. 获取身份证盒子SAM ID
INT CDevImpl_CRT603CZ7::GetIDCardSAMID(LPSTR lpSAMID)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_GetSAMID(lpSAMID);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取身份证盒子SAM ID: CRT_GetSAMID() fail. Return: %s",
            ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 36. 读卡器开关射频场
INT CDevImpl_CRT603CZ7::SetSwitchRF(INT nMode)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_SwitchRF(nMode);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡器开关射频场: CRT_SwitchRF(%d) fail. Return: %s",
            nMode, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 37. 获取最后一次错误描叙
CHAR* CDevImpl_CRT603CZ7::GetLastError()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    //CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    return CRT_GetLastError();
}

// 38. 设置身份证头像保存路径
INT CDevImpl_CRT603CZ7::SetIDCardHeadImgPath(LPSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = CRT_SetHeadPath(lpPath);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置身份证头像保存路径: CRT_SetHeadPath(%s) fail. Return: %s",
            lpPath, ConvertCode_IMPL2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}


//----------------------------------对外参数设置接口----------------------------------
// 设置断线重连标记
INT CDevImpl_CRT603CZ7::SetReConFlag(BOOL bFlag)
{
    THISMODULE(__FUNCTION__);

    if (m_bReCon == FALSE && bFlag == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 Start......");
    }
    m_bReCon = bFlag;

    return IMP_SUCCESS;
}

// 设置动态库路径(DeviceOpen前有效)
INT CDevImpl_CRT603CZ7::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径
    if (lpPath == nullptr || strlen(lpPath) < 1 ||
        (strlen(lpPath) == 1 && (char*)lpPath[0] == "/"))
    {
        Log(ThisModule, __LINE__, "入参[%s]无效,不设置动态库路径.");
        return IMP_SUCCESS;
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

    return IMP_SUCCESS;
}

LPSTR CDevImpl_CRT603CZ7::ConvertCode_IMPL2Str(INT nErrCode)
{
#define CRT603_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStr, "%d|%s", CODE, STR); \
        return m_szErrStr;

    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nErrCode)
    {
        CRT603_CASE_CODE_STR(IMP_SUCCESS, nErrCode, "成功");
        CRT603_CASE_CODE_STR(IMP_ERR_LOAD_LIB, nErrCode, "动态库加载失败");
        CRT603_CASE_CODE_STR(IMP_ERR_PARAM_INVALID, nErrCode, "参数无效");
        CRT603_CASE_CODE_STR(IMP_ERR_UNKNOWN, nErrCode, "未知错误");
        CRT603_CASE_CODE_STR(IMP_ERR_NOTOPEN, nErrCode, "设备未Open");
        CRT603_CASE_CODE_STR(IMP_ERR_NODEVICE, nErrCode, "未找到有效设备");
        // <0: Device返回
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F1, nErrCode, "An internal consistency check failed(内部一致性检查失败)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F2, nErrCode, "The action was cancelled by an SCardCancel request(该行动因一项取消请求而取消)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F3, nErrCode, "The supplied handle was invalid(提供的句柄无效)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F4, nErrCode, "One or more of the supplied parameters could not be properly interpreted(无法正确解释提供的一个或多个参数)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F5, nErrCode, "Registry startup information is missing or invalid(注册表启动信息丢失或无效)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F6, nErrCode, "Not enough memory available to complete this command(内存不足，无法完成此命令)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F7, nErrCode, "An internal consistency timer has expired(内部一致性计时器已过期)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F8, nErrCode, "The specified reader name is not recognized(无法识别指定的读取器名称)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F9, nErrCode, "The specified reader name is not recognized(无法识别指定的读取器名称)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F10, nErrCode, "The user-specified timeout value has expired(用户指定的超时值已过期)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F11, nErrCode, "The operation requires a Smart Card, but no Smart Card is currently in the device(该操作需要智能卡，但设备中当前没有智能卡)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F12, nErrCode, "The operation requires a Smart Card, but no Smart Card is currently in the device(该操作需要智能卡，但设备中当前没有智能卡)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F13, nErrCode, "The specified smart card name is not recognized(无法识别指定的智能卡名称)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F14, nErrCode, "The system could not dispose of the media in the requested manner(系统无法按请求的方式处置介质)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F15, nErrCode, "The requested protocols are incompatible with the protocol currently in use with the smart card(请求的协议与智能卡当前使用的协议不兼容)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F16, nErrCode, "The reader or smart card is not ready to accept commands(读卡器或智能卡尚未准备好接受命令)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F17, nErrCode, "One or more of the supplied parameters values could not be properly interpreted(无法正确解释提供的一个或多个参数值)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F18, nErrCode, "The action was cancelled by the system, presumably to log off or shut down(该操作被系统取消，可能是为了注销或关闭)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F19, nErrCode, "An internal communications error has been detected(检测到内部通信错误)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F20, nErrCode, "An internal error has been detected, but the source is unknown(检测到内部错误，但来源未知)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F21, nErrCode, "An ATR obtained from the registry is not a valid ATR string(从注册表获取的ATR不是有效的ATR字符串)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F22, nErrCode, "An attempt was made to end a non-existent transaction(试图结束一项不存在的交易)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F23, nErrCode, "The specified reader is not currently available for use(指定的读卡器当前不可用)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F24, nErrCode, "The operation has been aborted to allow the server application to exit(操作已中止，以允许服务器应用程序退出)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F25, nErrCode, "The PCI Receive buffer was too small(PCI接收缓冲区太小)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F26, nErrCode, "The reader driver does not meet minimal requirements for support(读卡器驱动程序不满足最低支持要求)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F27, nErrCode, "The reader driver did not produce a unique reader name(读卡器驱动程序没有生成唯一的读卡器名称)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F28, nErrCode, "The smart card does not meet minimal requirements for support(智能卡不满足最低支持要求)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F29, nErrCode, "The Smart card resource manager is not running(智能卡资源管理器未运行)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F30, nErrCode, "The Smart card resource manager has shut down(智能卡资源管理器已关闭)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F31, nErrCode, "An unexpected card error has occurred(发生了意外的卡错误)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F32, nErrCode, "No Primary Provider can be found for the smart card(找不到智能卡的主要提供商)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F33, nErrCode, "The requested order of object creation is not supported(不支持请求的对象创建顺序)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F34, nErrCode, "This smart card does not support the requested feature(此智能卡不支持请求的功能)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F35, nErrCode, "The identified directory does not exist in the smart card(智能卡中不存在标识的目录)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F36, nErrCode, "The identified file does not exist in the smart card(智能卡中不存在识别的文件)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F37, nErrCode, "The supplied path does not represent a smart card directory(提供的路径不代表智能卡目录)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F38, nErrCode, "The supplied path does not represent a smart card file(提供的路径不代表智能卡文件)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F39, nErrCode, "Access is denied to this file(拒绝访问此文件)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F40, nErrCode, "The smartcard does not have enough memory to store the information(智能卡内存不足，无法存储信息)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F41, nErrCode, "There was an error trying to set the smart card file object pointer(试图设置智能卡文件对象指针时出错)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F42, nErrCode, "The supplied PIN is incorrect(提供的PIN不正确)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F43, nErrCode, "An unrecognized error code was returned from a layered component(分层组件返回了无法识别的错误代码)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F44, nErrCode, "The requested certificate does not exist(请求的证书不存在)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F45, nErrCode, "The requested certificate could not be obtained(无法获取请求的证书)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F46, nErrCode, "Cannot find a smart card reader(找不到智能卡读卡器)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F47, nErrCode, "A communications error with the smart card has been detected, Retry the operation(检测到智能卡存在通信错误, 请重试该操作)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F48, nErrCode, "The requested key container does not exist on the smart card(智能卡上不存在请求的密钥容器)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F49, nErrCode, "The Smart card resource manager is too busy to complete this operation(智能卡资源管理器太忙，无法完成此操作)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F101, nErrCode, "The Smart card resource manager is too busy to complete this operation(智能卡资源管理器太忙，无法完成此操作)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F102, nErrCode, "The smart card is not responding to a reset(智能卡对重置没有响应)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F103, nErrCode, "Power has been removed from the smart card, so that further communication is not possible(智能卡已断电，因此无法进行进一步通信)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F104, nErrCode, "The smart card has been reset, so any shared state information is invalid(智能卡已重置，因此任何共享状态信息都无效)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F105, nErrCode, "The smart card has been removed, so that further communication is not possible(智能卡已被移除，因此无法进行进一步通信)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F106, nErrCode, "Access was denied because of a security violation(由于安全违规，访问被拒绝)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F107, nErrCode, "The card cannot be accessed because the wrong PIN was presented(无法访问该卡，因为提供了错误的PIN)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F108, nErrCode, "The card cannot be accessed because the maximum number of PIN entry attempts has been reached(无法访问该卡，因为已达到尝试输入PIN码的最大次数)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F109, nErrCode, "The end of the smart card file has been reached(已到达智能卡文件的末尾)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F110, nErrCode, "The action was cancelled by the user(该操作已被用户取消)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F111, nErrCode, "No PIN was presented to the smart card(智能卡上未显示PIN)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F112, nErrCode, "The requested item could not be found in the cache(在缓存中找不到请求的项)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F113, nErrCode, "The requested cache item is too old and was deleted from the cache(请求的缓存项太旧，已从缓存中删除)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F114, nErrCode, "The new cache item exceeds the maximum per-item size defined for the cache(新缓存项超过了为缓存定义的最大每项大小)");
        CRT603_CASE_CODE_STR(IMP_ERR_DEV_F99, nErrCode, "Function returned unknown error code(函数返回未知错误代码)");
        default :
            sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStr;
}

