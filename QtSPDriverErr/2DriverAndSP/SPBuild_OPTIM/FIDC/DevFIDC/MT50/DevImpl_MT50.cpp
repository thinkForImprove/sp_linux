/***************************************************************
* 文件名称：DevImpl_MT50.cpp
* 文件描述：明泰非接模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月18日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_MT50.h"
#include "QtTypeDef.h"
#include "QtDLLLoader.h"

static const char *ThisFile = "DevImpl_MT50.cpp";

#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        if (m_nRetErrOLD[4] != IMP_ERR_NOTOPEN) \
        { \
            Log(ThisModule, __LINE__, "检查设备OPEN标记: OpenFlag == FALSE, Device Not Open, return fail.Return: %s.", \
            ConvertCode_Impl2Str(IMP_ERR_NOTOPEN)); \
        } \
        m_nRetErrOLD[4] = IMP_ERR_NOTOPEN; \
        return IMP_ERR_NOTOPEN; \
    }

CDevImpl_MT50::CDevImpl_MT50()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_MT50::CDevImpl_MT50(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_MT50::Init()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bDevOpenOk = FALSE;
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    memset(m_szFWVer, 0x00, sizeof(m_szFWVer));

    m_vDlHandle = nullptr;

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("FIDC/MT50/");
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

CDevImpl_MT50::~CDevImpl_MT50()
{
    vDlUnLoadLibrary();
}

BOOL CDevImpl_MT50::bLoadLibrary()
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

void CDevImpl_MT50::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
        vInitLibFunc();
    }
}

BOOL CDevImpl_MT50::bLoadLibIntf()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = FALSE;

    // 1. 打开USB设备
    LOAD_LIBINFO_FUNC(pOpenUsbDevice, MT_OpenUsbDevice, "open_usb_device");

    // 2. 打开串口设备
    LOAD_LIBINFO_FUNC(pOpenSerialDevice, MT_OpenSerialDevice, "open_serial_device");

    // 3. 获取设备版本
    LOAD_LIBINFO_FUNC(pGetDeviceVersion, MT_GetDeviceVersion, "get_device_version");

    // 4. 获取动态库版本
    LOAD_LIBINFO_FUNC(pGetSoVersion, MT_GetSoVersion, "get_so_version");

    // 5. 设备蜂鸣
    LOAD_LIBINFO_FUNC(pDeviceBeep, MT_DeviceBeep, "device_beep");

    // 6. 读设备序列号
    LOAD_LIBINFO_FUNC(pReadDeviceSNR, MT_ReadDeviceSNR, "read_device_snr");

    // 7. 写设备序列号
    LOAD_LIBINFO_FUNC(pWriteDeviceSNR, MT_WriteDeviceSNR, "write_device_snr");

    // 8. 关闭打开的设备
    LOAD_LIBINFO_FUNC(pCloseDevice, MT_CloseDevice, "close_device");

    // 9. 非接CPU卡状态
    LOAD_LIBINFO_FUNC(pPiccStatus, MT_PiccStatus, "picc_status");

    // 10. 非接CPU卡上电
    LOAD_LIBINFO_FUNC(pPiccPowerOn, MT_PiccPowerOn, "picc_poweron");

    // 11. 非接CPU卡下电
    LOAD_LIBINFO_FUNC(pPiccPowerOff, MT_PiccPowerOff, "picc_poweroff");

    // 12. 非接CPU卡发送APDU
    LOAD_LIBINFO_FUNC(pPiccApdu, MT_PiccApdu, "picc_apdu");

    // 13. 接触CPU卡状态
    LOAD_LIBINFO_FUNC(pIccStatus, MT_IccStatus, "icc_status");

    // 14. 指定卡座寻卡上电热复位
    LOAD_LIBINFO_FUNC(pIccReset, MT_IccReset, "icc_reset");

    // 15. 指定卡座寻卡上电热复位(指定波特率)
    LOAD_LIBINFO_FUNC(pIccResetBuad, MT_IccResetBuad, "icc_reset_buad");

    // 16. 指定卡座寻卡上电冷复位
    LOAD_LIBINFO_FUNC(pIccPowerOn, MT_IccPowerOn, "icc_poweron");

    // 17. 指定卡座寻卡上电冷复位(指定波特率)
    LOAD_LIBINFO_FUNC(pIccPowerOnBuad, MT_IccPowerOnBuad, "icc_poweron_buad");

    // 18. 接触CPU卡下电
    LOAD_LIBINFO_FUNC(pIccPowerOff, MT_IccPowerOff, "icc_poweroff");

    // 19. 接触CPU卡发送APDU
    LOAD_LIBINFO_FUNC(pIccApdu, MT_IccApdu, "icc_apdu");

    // 20. 接触式存储卡类型设置
    LOAD_LIBINFO_FUNC(pContactSelect, MT_ContactSelect, "contact_select");

    // 21. 接触式存储卡类型识别
    LOAD_LIBINFO_FUNC(pContactVerify, MT_ContactVerify, "contact_verify");

    // 22. 检测是否4442卡
    LOAD_LIBINFO_FUNC(pSle4442Is42, MT_Sle4442Is42, "sle4442_is42");

    // 23. 获取卡片指定地址的数据
    LOAD_LIBINFO_FUNC(pSle4442Read, MT_Sle4442Read, "sle4442_read");

    // 24. 更改卡片指定地址的数据
    LOAD_LIBINFO_FUNC(pSle4442Write, MT_Sle4442Write, "sle4442_write");

    // 25. 读密码
    LOAD_LIBINFO_FUNC(pSle4442PwdRead, MT_Sle4442PwdRead, "sle4442_pwd_read");

    // 26. 校验密码是否正确
    LOAD_LIBINFO_FUNC(pSle4442PwdCheck, MT_Sle4442PwdCheck, "sle4442_pwd_check");

    // 27. 修改密码
    LOAD_LIBINFO_FUNC(pSle4442PwdModify, MT_Sle4442PwdModify, "sle4442_pwd_modify");

    // 28. 获取卡片保护位数据
    LOAD_LIBINFO_FUNC(pSle4442ProbitRead, MT_Sle4442ProbitRead, "sle4442_probit_read");

    // 29. 对卡片指定地址的数据进行写保护
    LOAD_LIBINFO_FUNC(pSle4442ProbitWrite, MT_Sle4442ProbitWrite, "sle4442_probit_write");

    // 30. 获取密码校验剩余错误次数
    LOAD_LIBINFO_FUNC(pSle4442ErrcountRead, MT_Sle4442ErrcountRead, "sle4442_errcount_read");

    // 31. 检测是否4428卡
    LOAD_LIBINFO_FUNC(pSle4428Is28, MT_Sle4428Is28, "sle4428_is28");

    // 32. 获取卡片指定地址的数据
    LOAD_LIBINFO_FUNC(pSle4428Read, MT_Sle4428Read, "sle4428_read");

    // 33. 更改卡片指定地址的数据
    LOAD_LIBINFO_FUNC(pSle4428Write, MT_Sle4428Write, "sle4428_write");

    // 34. 读密码
    LOAD_LIBINFO_FUNC(pSle4428PwdRead, MT_Sle4428PwdRead, "sle4428_pwd_read");

    // 35. 校验密码是否正确
    LOAD_LIBINFO_FUNC(pSle4428PwdCheck, MT_Sle4428PwdCheck, "sle4428_pwd_check");

    // 36. 修改密码
    LOAD_LIBINFO_FUNC(pSle4428PwdModify, MT_Sle4428PwdModify, "sle4428_pwd_modify");

    // 37. 获取卡片保护位数据
    LOAD_LIBINFO_FUNC(pSle4428ProbitRead, MT_Sle4428ProbitRead, "sle4428_probit_read");

    // 38. 对卡片指定地址的数据进行写保护
    LOAD_LIBINFO_FUNC(pSle4428ProbitWrite, MT_Sle4428ProbitWrite, "sle4428_probit_write");

    // 39. 获取密码校验剩余错误次数
    LOAD_LIBINFO_FUNC(pSle4428ErrcountRead, MT_Sle4428ErrcountRead, "sle4428_errcount_read");

    // 40. 射频复位
    LOAD_LIBINFO_FUNC(pRFReset, MT_RFReset, "rf_reset");

    // 41. 激活非接触式存储卡
    LOAD_LIBINFO_FUNC(pRFCard, MT_RFCard, "rf_card");

    // 42. 非接触式存储卡认证扇区
    LOAD_LIBINFO_FUNC(pRFAuthenticationKey, MT_RFAuthenticationKey, "rf_authentication_key");

    // 43. 读取数据
    LOAD_LIBINFO_FUNC(pRFRead, MT_RFRead, "rf_read");

    // 44. 写入数据
    LOAD_LIBINFO_FUNC(pRFWrite, MT_RFWrite, "rf_write");

    // 45. 初始化块值
    LOAD_LIBINFO_FUNC(pRFInitval, MT_RFInitval, "rf_initval");

    // 46. 读块值
    LOAD_LIBINFO_FUNC(pRFReadVal, MT_RFReadVal, "rf_readval");

    // 47. 加值
    LOAD_LIBINFO_FUNC(pRFIncrement, MT_RFIncrement, "rf_increment");

    // 48. 减值
    LOAD_LIBINFO_FUNC(pRFDecrement, MT_RFDecrement, "rf_decrement");

    // 49.
    LOAD_LIBINFO_FUNC(pRFTransfer, MT_RFTransfer, "rf_transfer");

    // 50.
    LOAD_LIBINFO_FUNC(pRFRestore, MT_RFRestore, "rf_restore");

    // 51. 将卡片状态设置为halt
    LOAD_LIBINFO_FUNC(pRFTerminal, MT_RFTerminal, "rf_terminal");

    // 52. 设置磁条卡模式
    LOAD_LIBINFO_FUNC(pSetMagneticMode, MT_SetMagneticMode, "set_magnetic_mode");

    // 53. 读取磁条卡数据
    LOAD_LIBINFO_FUNC(pMagneticRead, MT_MagneticRead, "magnetic_read");

    // 54. 获取身份证安全模块ID
    LOAD_LIBINFO_FUNC(pIDCardModuleId, MT_IDCardModuleId, "idcard_moduleid");

    // 55. 获取身份证UID
    LOAD_LIBINFO_FUNC(pIDCardUid, MT_IDCardUid, "idcard_uid");

    // 56. 读取身份证原始数据
    LOAD_LIBINFO_FUNC(pIDCardReadBase, MT_IDCardReadBase, "idcard_read_base");

    // 57. 解析身份证文字信息
    LOAD_LIBINFO_FUNC(pParseIDCardText, MT_ParseIDCardText, "parse_idcard_text");

    // 58. 读取磁条卡数据
    LOAD_LIBINFO_FUNC(pParseIDCardPhoto, MT_ParseIDCardPhoto, "parse_idcard_photo");

    // 59.
    LOAD_LIBINFO_FUNC(pMyStrUpr, MT_MyStrUpr, "mystrupr");

    // 60.
    LOAD_LIBINFO_FUNC(pMyStrLwr, MT_MyStrLwr, "mystrlwr");

    // 61.
    LOAD_LIBINFO_FUNC(pSplitPath, MT_SplitPath, "_splitpath");

    // 62.
    LOAD_LIBINFO_FUNC(pMakePath, MT_MakePath, "_makepath");

    // 63.
    LOAD_LIBINFO_FUNC(pBmpGenerate, MT_BmpGenerate, "bmp_generate");

    // 64.
    LOAD_LIBINFO_FUNC(pImgGenerate, MT_ImgGenerate, "img_generate");

    // 65. 字符编码转换
    LOAD_LIBINFO_FUNC(pCodeConvert, MT_CodeConvert, "code_convert");

    // 66. 将16进制数转换为ASCII字符
    LOAD_LIBINFO_FUNC(pHex2Asc, MT_Hex2Asc, "hex_asc");

    // 67. 将ASCII字符转换为16进制
    LOAD_LIBINFO_FUNC(pAsc2Hex, MT_Asc2Hex, "asc_hex");

    // 68. 将Base64字符转换为16进制数
    LOAD_LIBINFO_FUNC(pBase642Hex, MT_Base642Hex, "base64_hex");

    // 69. 将16进制数转换为Base64
    LOAD_LIBINFO_FUNC(pHex2Base64, MT_Hex2Base64, "hex_base64");

    // 70. 解析二代证数据为RGB数据
    LOAD_LIBINFO_FUNC(pWlt2RGB, MT_Wlt2RGB, "wlt_2_rgb");

    // 71. 获取照片文件的base64码
    LOAD_LIBINFO_FUNC(pGetBase64Data, MT_GetBase64Data, "get_base64_data");

    // 72.
    LOAD_LIBINFO_FUNC(pGetErrmsg, MT_GetErrmsg, "get_errmsg");

    // 73. 开启日志
    LOAD_LIBINFO_FUNC(pEnabledLog, MT_EnabledLog, "enabled_log");

    // 74. 关闭日志
    LOAD_LIBINFO_FUNC(pDisenabledLog, MT_DisenabledLog, "disenabled_log");

    return TRUE;
}

void CDevImpl_MT50::vInitLibFunc()
{
    MT_OpenUsbDevice = nullptr;           // 1. 打开USB设备
    MT_OpenSerialDevice = nullptr;        // 2. 打开串口设备
    MT_GetDeviceVersion = nullptr;        // 3. 获取设备版本
    MT_GetSoVersion = nullptr;            // 4. 获取动态库版本
    MT_DeviceBeep = nullptr;              // 5. 设备蜂鸣
    MT_ReadDeviceSNR = nullptr;           // 6. 读设备序列号
    MT_WriteDeviceSNR = nullptr;          // 7. 写设备序列号
    MT_CloseDevice = nullptr;             // 8. 关闭打开的设备
    MT_PiccStatus = nullptr;              // 9. 非接CPU卡状态
    MT_PiccPowerOn = nullptr;             // 10. 非接CPU卡上电
    MT_PiccPowerOff = nullptr;            // 11. 非接CPU卡下电
    MT_PiccApdu = nullptr;                // 12. 非接CPU卡发送APDU
    MT_IccStatus = nullptr;               // 13. 接触CPU卡状态
    MT_IccReset = nullptr;                // 14. 指定卡座寻卡上电热复位
    MT_IccResetBuad = nullptr;            // 15. 指定卡座寻卡上电热复位(指定波特率)
    MT_IccPowerOn = nullptr;              // 16. 指定卡座寻卡上电冷复位
    MT_IccPowerOnBuad = nullptr;          // 17. 指定卡座寻卡上电冷复位(指定波特率)
    MT_IccPowerOff = nullptr;             // 18. 接触CPU卡下电
    MT_IccApdu = nullptr;                 // 19. 接触CPU卡发送APDU
    MT_ContactSelect = nullptr;           // 20. 接触式存储卡类型设置
    MT_ContactVerify = nullptr;           // 21. 接触式存储卡类型识别
    MT_Sle4442Is42 = nullptr;             // 22. 检测是否4442卡
    MT_Sle4442Read = nullptr;             // 23. 获取卡片指定地址的数据
    MT_Sle4442Write = nullptr;            // 24. 更改卡片指定地址的数据
    MT_Sle4442PwdRead = nullptr;          // 25. 读密码
    MT_Sle4442PwdCheck = nullptr;         // 26. 校验密码是否正确
    MT_Sle4442PwdModify = nullptr;        // 27. 修改密码
    MT_Sle4442ProbitRead = nullptr;       // 28. 获取卡片保护位数据
    MT_Sle4442ProbitWrite = nullptr;      // 29. 对卡片指定地址的数据进行写保护
    MT_Sle4442ErrcountRead = nullptr;     // 30. 获取密码校验剩余错误次数
    MT_Sle4428Is28 = nullptr;             // 31. 检测是否4428卡
    MT_Sle4428Read = nullptr;             // 32. 获取卡片指定地址的数据
    MT_Sle4428Write = nullptr;            // 33. 更改卡片指定地址的数据
    MT_Sle4428PwdRead = nullptr;          // 34. 读密码
    MT_Sle4428PwdCheck = nullptr;         // 35. 校验密码是否正确
    MT_Sle4428PwdModify = nullptr;        // 36. 修改密码
    MT_Sle4428ProbitRead = nullptr;       // 37. 获取卡片保护位数据
    MT_Sle4428ProbitWrite = nullptr;      // 38. 对卡片指定地址的数据进行写保护
    MT_Sle4428ErrcountRead = nullptr;     // 39. 获取密码校验剩余错误次数
    MT_RFReset = nullptr;                 // 40. 射频复位
    MT_RFCard = nullptr;                  // 41. 激活非接触式存储卡
    MT_RFAuthenticationKey = nullptr;     // 42. 非接触式存储卡认证扇区
    MT_RFRead = nullptr;                  // 43. 读取数据
    MT_RFWrite = nullptr;                 // 44. 写入数据
    MT_RFInitval = nullptr;               // 45. 初始化块值
    MT_RFReadVal = nullptr;               // 46. 读块值
    MT_RFIncrement = nullptr;             // 47. 加值
    MT_RFDecrement = nullptr;             // 48. 减值
    MT_RFTransfer = nullptr;              // 49.
    MT_RFRestore = nullptr;               // 50.
    MT_RFTerminal = nullptr;              // 51. 将卡片状态设置为halt
    MT_SetMagneticMode = nullptr;         // 52. 设置磁条卡模式
    MT_MagneticRead = nullptr;            // 53. 读取磁条卡数据
    MT_IDCardModuleId = nullptr;          // 54. 获取身份证安全模块ID
    MT_IDCardUid = nullptr;               // 55. 获取身份证UID
    MT_IDCardReadBase = nullptr;          // 56. 读取身份证原始数据
    MT_ParseIDCardText = nullptr;         // 57. 解析身份证文字信息
    MT_ParseIDCardPhoto = nullptr;        // 58. 读取磁条卡数据
    MT_MyStrUpr = nullptr;                // 59.
    MT_MyStrLwr = nullptr;                // 60.
    MT_SplitPath = nullptr;               // 61.
    MT_MakePath = nullptr;                // 62.
    MT_BmpGenerate = nullptr;             // 63.
    MT_ImgGenerate = nullptr;             // 64.
    MT_CodeConvert = nullptr;             // 65. 字符编码转换
    MT_Hex2Asc = nullptr;                 // 66. 将16进制数转换为ASCII字符
    MT_Asc2Hex = nullptr;                 // 67. 将ASCII字符转换为16进制
    MT_Base642Hex = nullptr;              // 68. 将Base64字符转换为16进制数
    MT_Hex2Base64 = nullptr;              // 69. 将16进制数转换为Base64
    MT_Wlt2RGB = nullptr;                 // 70. 解析二代证数据为RGB数据
    MT_GetBase64Data = nullptr;           // 71. 获取照片文件的base64码
    MT_GetErrmsg = nullptr;               // 72.
    MT_EnabledLog = nullptr;              // 73. 开启日志
    MT_DisenabledLog = nullptr;           // 74. 关闭日志
}

BOOL CDevImpl_MT50::bDlLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = TRUE;

    if (m_vDlHandle == nullptr)
    {
        m_vDlHandle = dlopen(m_szLoadDllPath, RTLD_NOW | RTLD_DEEPBIND);
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

void CDevImpl_MT50::vDlUnLoadLibrary()
{
    if (m_vDlHandle != nullptr)
    {
        dlclose(m_vDlHandle);
        m_vDlHandle = nullptr;
        m_bLoadIntfFail = TRUE;
        vInitLibFunc();
    }
}

BOOL CDevImpl_MT50::bDlLoadLibIntf()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    m_bLoadIntfFail = FALSE;

    // 1. 打开USB设备
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenUsbDevice, MT_OpenUsbDevice, "open_usb_device");

    // 2. 打开串口设备
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pOpenSerialDevice, MT_OpenSerialDevice, "open_serial_device");

    // 3. 获取设备版本
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pGetDeviceVersion, MT_GetDeviceVersion, "get_device_version");

    // 4. 获取动态库版本
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pGetSoVersion, MT_GetSoVersion, "get_so_version");

    // 5. 设备蜂鸣
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pDeviceBeep, MT_DeviceBeep, "device_beep");

    // 6. 读设备序列号
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pReadDeviceSNR, MT_ReadDeviceSNR, "read_device_snr");

    // 7. 写设备序列号
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pWriteDeviceSNR, MT_WriteDeviceSNR, "write_device_snr");

    // 8. 关闭打开的设备
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCloseDevice, MT_CloseDevice, "close_device");

    // 9. 非接CPU卡状态
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPiccStatus, MT_PiccStatus, "picc_status");

    // 10. 非接CPU卡上电
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPiccPowerOn, MT_PiccPowerOn, "picc_poweron");

    // 11. 非接CPU卡下电
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPiccPowerOff, MT_PiccPowerOff, "picc_poweroff");

    // 12. 非接CPU卡发送APDU
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pPiccApdu, MT_PiccApdu, "picc_apdu");

    // 13. 接触CPU卡状态
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pIccStatus, MT_IccStatus, "icc_status");

    // 14. 指定卡座寻卡上电热复位
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pIccReset, MT_IccReset, "icc_reset");

    // 15. 指定卡座寻卡上电热复位(指定波特率)
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pIccResetBuad, MT_IccResetBuad, "icc_reset_buad");

    // 16. 指定卡座寻卡上电冷复位
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pIccPowerOn, MT_IccPowerOn, "icc_poweron");

    // 17. 指定卡座寻卡上电冷复位(指定波特率)
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pIccPowerOnBuad, MT_IccPowerOnBuad, "icc_poweron_buad");

    // 18. 接触CPU卡下电
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pIccPowerOff, MT_IccPowerOff, "icc_poweroff");

    // 19. 接触CPU卡发送APDU
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pIccApdu, MT_IccApdu, "icc_apdu");

    // 20. 接触式存储卡类型设置
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pContactSelect, MT_ContactSelect, "contact_select");

    // 21. 接触式存储卡类型识别
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pContactVerify, MT_ContactVerify, "contact_verify");

    // 22. 检测是否4442卡
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4442Is42, MT_Sle4442Is42, "sle4442_is42");

    // 23. 获取卡片指定地址的数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4442Read, MT_Sle4442Read, "sle4442_read");

    // 24. 更改卡片指定地址的数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4442Write, MT_Sle4442Write, "sle4442_write");

    // 25. 读密码
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4442PwdRead, MT_Sle4442PwdRead, "sle4442_pwd_read");

    // 26. 校验密码是否正确
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4442PwdCheck, MT_Sle4442PwdCheck, "sle4442_pwd_check");

    // 27. 修改密码
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4442PwdModify, MT_Sle4442PwdModify, "sle4442_pwd_modify");

    // 28. 获取卡片保护位数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4442ProbitRead, MT_Sle4442ProbitRead, "sle4442_probit_read");

    // 29. 对卡片指定地址的数据进行写保护
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4442ProbitWrite, MT_Sle4442ProbitWrite, "sle4442_probit_write");

    // 30. 获取密码校验剩余错误次数
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4442ErrcountRead, MT_Sle4442ErrcountRead, "sle4442_errcount_read");

    // 31. 检测是否4428卡
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4428Is28, MT_Sle4428Is28, "sle4428_is28");

    // 32. 获取卡片指定地址的数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4428Read, MT_Sle4428Read, "sle4428_read");

    // 33. 更改卡片指定地址的数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4428Write, MT_Sle4428Write, "sle4428_write");

    // 34. 读密码
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4428PwdRead, MT_Sle4428PwdRead, "sle4428_pwd_read");

    // 35. 校验密码是否正确
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4428PwdCheck, MT_Sle4428PwdCheck, "sle4428_pwd_check");

    // 36. 修改密码
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4428PwdModify, MT_Sle4428PwdModify, "sle4428_pwd_modify");

    // 37. 获取卡片保护位数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4428ProbitRead, MT_Sle4428ProbitRead, "sle4428_probit_read");

    // 38. 对卡片指定地址的数据进行写保护
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4428ProbitWrite, MT_Sle4428ProbitWrite, "sle4428_probit_write");

    // 39. 获取密码校验剩余错误次数
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSle4428ErrcountRead, MT_Sle4428ErrcountRead, "sle4428_errcount_read");

    // 40. 射频复位
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFReset, MT_RFReset, "rf_reset");

    // 41. 激活非接触式存储卡
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFCard, MT_RFCard, "rf_card");

    // 42. 非接触式存储卡认证扇区
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFAuthenticationKey, MT_RFAuthenticationKey, "rf_authentication_key");

    // 43. 读取数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFRead, MT_RFRead, "rf_read");

    // 44. 写入数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFWrite, MT_RFWrite, "rf_write");

    // 45. 初始化块值
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFInitval, MT_RFInitval, "rf_initval");

    // 46. 读块值
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFReadVal, MT_RFReadVal, "rf_readval");

    // 47. 加值
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFIncrement, MT_RFIncrement, "rf_increment");

    // 48. 减值
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFDecrement, MT_RFDecrement, "rf_decrement");

    // 49.
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFTransfer, MT_RFTransfer, "rf_transfer");

    // 50.
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFRestore, MT_RFRestore, "rf_restore");

    // 51. 将卡片状态设置为halt
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pRFTerminal, MT_RFTerminal, "rf_terminal");

    // 52. 设置磁条卡模式
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSetMagneticMode, MT_SetMagneticMode, "set_magnetic_mode");

    // 53. 读取磁条卡数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pMagneticRead, MT_MagneticRead, "magnetic_read");

    // 54. 获取身份证安全模块ID
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pIDCardModuleId, MT_IDCardModuleId, "idcard_moduleid");

    // 55. 获取身份证UID
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pIDCardUid, MT_IDCardUid, "idcard_uid");

    // 56. 读取身份证原始数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pIDCardReadBase, MT_IDCardReadBase, "idcard_read_base");

    // 57. 解析身份证文字信息
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pParseIDCardText, MT_ParseIDCardText, "parse_idcard_text");

    // 58. 读取磁条卡数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pParseIDCardPhoto, MT_ParseIDCardPhoto, "parse_idcard_photo");

    // 59.
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pMyStrUpr, MT_MyStrUpr, "mystrupr");

    // 60.
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pMyStrLwr, MT_MyStrLwr, "mystrlwr");

    // 61.
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pSplitPath, MT_SplitPath, "_splitpath");

    // 62.
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pMakePath, MT_MakePath, "_makepath");

    // 63.
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pBmpGenerate, MT_BmpGenerate, "bmp_generate");

    // 64.
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pImgGenerate, MT_ImgGenerate, "img_generate");

    // 65. 字符编码转换
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pCodeConvert, MT_CodeConvert, "code_convert");

    // 66. 将16进制数转换为ASCII字符
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pHex2Asc, MT_Hex2Asc, "hex_asc");

    // 67. 将ASCII字符转换为16进制
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pAsc2Hex, MT_Asc2Hex, "asc_hex");

    // 68. 将Base64字符转换为16进制数
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pBase642Hex, MT_Base642Hex, "base64_hex");

    // 69. 将16进制数转换为Base64
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pHex2Base64, MT_Hex2Base64, "hex_base64");

    // 70. 解析二代证数据为RGB数据
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pWlt2RGB, MT_Wlt2RGB, "wlt_2_rgb");

    // 71. 获取照片文件的base64码
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pGetBase64Data, MT_GetBase64Data, "get_base64_data");

    // 72.
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pGetErrmsg, MT_GetErrmsg, "get_errmsg");

    // 73. 开启日志
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pEnabledLog, MT_EnabledLog, "enabled_log");

    // 74. 关闭日志
    LOAD_LIBINFO_FUNC_DL(m_vDlHandle, pDisenabledLog, MT_DisenabledLog, "disenabled_log");

    return TRUE;
}

//------------------------------------------------------------------------------------------------------
// 打开指定设备(USB方式)
INT CDevImpl_MT50::OpenDeviceUSB(INT nVid, INT nPid, INT nProtocol)
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
                    ConvertCode_Impl2Str(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        }
        m_nRetErrOLD[0] = IMP_SUCCESS;
    }
    m_nRetErrOLD[0] = IMP_SUCCESS;

    //-----------------------打开设备-----------------------
    nRet = MT_OpenUsbDevice(nVid, nPid, nProtocol);
    if (nRet != IMP_SUCCESS)
    {
        if (m_nRetErrOLD[1] != IMP_ERR_NOTOPEN)
        {
            Log(ThisModule, __LINE__, "打开设备: ->MT_OpenUsbDevice(%d, %d, %d) Fail. ErrCode: %d, Return: %s",
                nVid, nPid, nProtocol, nRet, ConvertCode_Impl2Str(IMP_ERR_NOTOPEN));
            m_nRetErrOLD[1] = IMP_ERR_NOTOPEN;
        }
        return IMP_ERR_NOTOPEN;
    } else
    {
        Log(ThisModule, __LINE__, "打开设备: ->MT_OpenUsbDevice(%d, %d, %d) Succ, Device Count",
            nVid, nPid, nProtocol);
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
INT CDevImpl_MT50::CloseDevice()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    // 关闭设备
    MT_CloseDevice();

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
BOOL CDevImpl_MT50::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

//------封装动态库接口------------------------------------------------------------------------------------
// 获取读卡器上卡状态
INT CDevImpl_MT50::GetCardStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = MT_PiccStatus();
    //if (nRet <= IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[3])
        {
            Log(ThisModule, __LINE__, "获取读卡器上卡状态: MT_PiccStatus() fail. Return: %s",
                ConvertCode_Impl2Str(nRet));
            m_nRetErrOLD[3] = nRet;
        }
        return nRet;
    }

    return nRet;
}

// 获取读卡器版本信息
INT CDevImpl_MT50::GetFWVersion(LPSTR lpVerStr, WORD wVerSize)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    CHAR szBuffer[128] = { 0x00 };
    INT nBuffLen = 0;

    nRet = MT_GetDeviceVersion(szBuffer, &nBuffLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取读卡器版本信息: MT_GetDeviceVersion() fail. Return: %s",
            ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    memcpy(lpVerStr, szBuffer, strlen(szBuffer) > wVerSize ? wVerSize : strlen(szBuffer));

    return IMP_SUCCESS;
}

// 读卡器卡片下电
INT CDevImpl_MT50::SetReaderPowerOff()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = MT_PiccPowerOff();
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡器卡片下电: MT_PiccPowerOff() fail. Return: %s",
            ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 读卡器卡片上电
INT CDevImpl_MT50::SetReaderPowerOn(INT nMode, LPUCHAR lpData, INT &nDataLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    UCHAR ucSNR[4] = { 0x00 };
    nRet = MT_PiccPowerOn(nMode, ucSNR, lpData, &nDataLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡器卡片上电: MT_PiccPowerOn(%d, %d, %d, %d) fail. Return: %s",
            ucSNR, lpData, &nDataLen, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 读卡器发送APDU指令
INT CDevImpl_MT50::SendReaderAPDU(LPUCHAR lpSndData, INT nSndLen,
                                  LPUCHAR lpRcvData, INT &nRcvLen)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    nRet = MT_PiccApdu(lpSndData, nSndLen, lpRcvData, &nRcvLen);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "读卡器发送APDU指令: MT_PiccApdu(%s, %d, %s, %d) fail. Return: %s",
            lpSndData, nSndLen, lpRcvData, nRcvLen, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 设置读卡器鸣响
// nMsec: 一次鸣响持续时间(单位:毫秒)
// nInterval: 多次鸣响间隔时间(单位:毫秒)
// nCount: 鸣响次数
INT CDevImpl_MT50::SetReaderBeep(INT nMsec, INT nInterval, INT nCount)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);

    INT nM = (nMsec / 100 > 1 ? nMsec / 100 : 1);
    INT nI = (nInterval / 100 > 1 ? nInterval / 100 : 1);

    nRet = MT_DeviceBeep(nM, nI, nCount);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "设置读卡器鸣响: MT_DeviceBeep(%d|(%d/100), %d|(%d/100), %d) fail. Return: %s",
            nM, nMsec, nI, nInterval, nCount, ConvertCode_Impl2Str(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

//----------------------------------对外参数设置接口----------------------------------
// 设置断线重连标记
INT CDevImpl_MT50::SetReConFlag(BOOL bFlag)
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
INT CDevImpl_MT50::SetLibPath(LPCSTR lpPath)
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

LPSTR CDevImpl_MT50::ConvertCode_Impl2Str(INT nErrCode)
{
#define MT50_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStr, "%d|%s", CODE, STR); \
        return m_szErrStr;

    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nErrCode)
    {
        MT50_CASE_CODE_STR(IMP_SUCCESS, nErrCode, "成功");
        MT50_CASE_CODE_STR(IMP_ERR_LOAD_LIB, nErrCode, "动态库加载失败");
        MT50_CASE_CODE_STR(IMP_ERR_PARAM_INVALID, nErrCode, "参数无效");
        MT50_CASE_CODE_STR(IMP_ERR_UNKNOWN, nErrCode, "未知错误");
        MT50_CASE_CODE_STR(IMP_ERR_NOTOPEN, nErrCode, "设备未Open");
        MT50_CASE_CODE_STR(IMP_ERR_NODEVICE, nErrCode, "未找到有效设备");
        // <0: Device返回
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0001H, nErrCode, "在操作区域内无卡");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0002H, nErrCode, "卡片CRC错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0003H, nErrCode, "数值溢出");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0004H, nErrCode, "验证不成功");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0005H, nErrCode, "卡片奇偶校验错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0006H, nErrCode, "与M1卡卡片通讯错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0008H, nErrCode, "防冲突过程中读系列号错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0009H, nErrCode, "波特率修改失败");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_000AH, nErrCode, "卡片没有通过验证");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_000BH, nErrCode, "从卡片接收到的位数错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_000CH, nErrCode, "从卡片接收到的字节数错误（仅仅读函数有效）");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0016H, nErrCode, "调用request函数出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0017H, nErrCode, "调用select函数出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0018H, nErrCode, "调用anticoll函数出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0019H, nErrCode, "调用read函数出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_001AH, nErrCode, "调用write函数出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_001BH, nErrCode, "调用增值函数出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_001CH, nErrCode, "调用减值函数出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_001DH, nErrCode, "调用重载函数出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_001EH, nErrCode, "调用loadkey函数出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_002AH, nErrCode, "命令错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_002BH, nErrCode, "PC同reader之间通讯错误，比如BCC错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_002CH, nErrCode, "PC同reader之间通讯命令码错误(设备不支持这条指令)");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0041H, nErrCode, "4442卡错误计数等于0（锁卡）");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0042H, nErrCode, "4442卡密码校验失败");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0043H, nErrCode, "4442卡读失败");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0044H, nErrCode, "4442卡写失败");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0045H, nErrCode, "4442卡读写地址越界");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0060H, nErrCode, "接收出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0061H, nErrCode, "输入偏移地址与长度超出读写范围");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0062H, nErrCode, "不是102卡");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0063H, nErrCode, "密码校验错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0070H, nErrCode, "接收参数出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0071H, nErrCode, "校验码出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0072H, nErrCode, "命令执行失败");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0073H, nErrCode, "命令执行超时");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0074H, nErrCode, "错误通信模式");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0075H, nErrCode, "无磁条卡");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_008AH, nErrCode, "校验和错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_008BH, nErrCode, "卡型错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_008CH, nErrCode, "拔卡错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_008DH, nErrCode, "通用错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_008EH, nErrCode, "命令头错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_008FH, nErrCode, "数据长度错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_0099H, nErrCode, "FLASH读写错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_1001H, nErrCode, "不支持接触用户卡");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_1002H, nErrCode, "接触用户卡未插到位");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_1003H, nErrCode, "接触用户卡已上电");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_1004H, nErrCode, "接触用户卡未上电");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_1005H, nErrCode, "接触用户卡上电失败");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_1006H, nErrCode, "操作接触用户卡数据无回应");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_1007H, nErrCode, "操作接触用户卡数据出现错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_2001H, nErrCode, "不支持PSAM卡");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_2002H, nErrCode, "PSAM卡未插到位");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_2003H, nErrCode, "PSAM卡已上电");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_2004H, nErrCode, "PSAM卡未上电");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_2005H, nErrCode, "PSAM卡上电失败");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_2006H, nErrCode, "操作PSAM卡数据无回应");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_2007H, nErrCode, "操作PSAM卡数据出现错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_3001H, nErrCode, "不支持非接触用户卡");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_3004H, nErrCode, "非接触用户卡未激活");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_3005H, nErrCode, "非接触用户卡激活失败");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_3006H, nErrCode, "操作非接触用户卡无回应（等待超时）");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_3007H, nErrCode, "操作非接触用户卡数据出现错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_3008H, nErrCode, "非接触用户卡halt失败");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_3009H, nErrCode, "有多张卡在感应区");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6001H, nErrCode, "不支持逻辑操作");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6020H, nErrCode, "卡片类型不对（卡状态6A82）");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6021H, nErrCode, "余额不足（卡状态9401）");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6022H, nErrCode, "卡片功能不支持（卡状态6A81）");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6023H, nErrCode, "扣款失败（卡状态9302）");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6030H, nErrCode, "卡片未启用");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6031H, nErrCode, "卡片不在有效期");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6032H, nErrCode, "交易明细无此记录");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6033H, nErrCode, "交易明细记录未处理完成");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6040H, nErrCode, "需要做防拔处理");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6041H, nErrCode, "防拔处理中出错, 非原来卡");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_6042H, nErrCode, "交易中断, 没有资金损失");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00A1H, nErrCode, "libusb返回输入输出错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00A2H, nErrCode, "libusb参数错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00A3H, nErrCode, "libusb拒绝访问，可能权限不足，可用管理员权限再试");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00A4H, nErrCode, "libusb找不到设备(设备可能已经掉线)");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00A5H, nErrCode, "libusb未找到设备");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00A6H, nErrCode, "libusb资源忙，可能设备已经被占用");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00A7H, nErrCode, "libusb操作超时");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00A8H, nErrCode, "libusb溢出错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00A9H, nErrCode, "libusb管道错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00AAH, nErrCode, "libusb系统调用中断(可能是由于信号)");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00ABH, nErrCode, "libusb内存不足");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00ACH, nErrCode, "libusb在这个平台上不支持的操作(linux)");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00B1H, nErrCode, "通讯超时");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00B2H, nErrCode, "无效的通讯句柄");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00B3H, nErrCode, "打开串口错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00B4H, nErrCode, "串口已经打开");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00B5H, nErrCode, "获取通讯端口状态错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00B6H, nErrCode, "设置通讯端口状态错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00B7H, nErrCode, "从读写器读取数据出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00B8H, nErrCode, "向读写器写入数据出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00B9H, nErrCode, "设置串口通讯波特率错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00C1H, nErrCode, "STX错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00C2H, nErrCode, "ETX错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00C3H, nErrCode, "BCC错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00C4H, nErrCode, "命令的数据长度大于最大长度");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00C5H, nErrCode, "数据值错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00C6H, nErrCode, "错误的协议类型");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00C7H, nErrCode, "错误的设备类型");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00C8H, nErrCode, "错误的USB通讯设备类");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00C9H, nErrCode, "设备正在通讯中或者是设备已经关闭");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00CAH, nErrCode, "设备通讯忙，函数正在操作可能还没返回");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00CBH, nErrCode, "接收到的设备返回的数据长度不对");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00D1H, nErrCode, "获取身份证");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00D2H, nErrCode, "身份证读卡");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00D3H, nErrCode, "身份证校验");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00D4H, nErrCode, "内存分配失");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00D5H, nErrCode, "调用ICONV");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00D6H, nErrCode, "调用iconv");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00D7H, nErrCode, "调用libwlt.so库出错");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00D8H, nErrCode, "传入的WLT错误");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00D9H, nErrCode, "打开文件失");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00DAH, nErrCode, "文件不存在");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00DBH, nErrCode, "磁条卡数据");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00DCH, nErrCode, "未识别卡类");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00DDH, nErrCode, "无卡");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00DEH, nErrCode, "有卡未上电");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00DFH, nErrCode, "卡无应答");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00E0H, nErrCode, "刷卡命令超时");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00E1H, nErrCode, "磁条卡刷卡失败");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00E2H, nErrCode, "磁条卡刷卡模式未开启");
        MT50_CASE_CODE_STR(IMP_ERR_DEV_00E3H, nErrCode, "发送APDU错误");
        default :
            sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStr;
}

