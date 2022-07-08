/***************************************************************
* 文件名称：DevImpl_RSCD400M.cpp
* 文件描述：RSC-D400M票据受理模块底层控制指令接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_RSCD400M.h"

static const char *ThisModule = "DevImpl_RSCD400M.cpp";

CDevImpl_RSCD400M::CDevImpl_RSCD400M()
{
    SetLogFile(LOG_NAME, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_RSCD400M::CDevImpl_RSCD400M(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisModule);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_RSCD400M::Init()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_bDevOpenOk = FALSE;
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    //strDllName.prepend("CSR/RSCD400M/");
    strDllName.prepend(DLL_DEVLIB_PATH);
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());
    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    // 动态库接口函数初始化
    SS_CHK_Open = nullptr;                    // 1. 打开设备
    SS_CHK_Close = nullptr;                   // 2. 关闭设备
    SS_CHK_Insert = nullptr;                  // 3. 打开入票口
    SS_CHK_CancelInsert = nullptr;            // 4. 关闭入票口
    SS_CHK_Accept = nullptr;                  // 6. 票据压箱
    SS_CHK_Eject = nullptr;                   // 7. 退出票据
    SS_CHK_Reset = nullptr;                   // 8. 设备复位(无动作)
    SS_CHK_ResetEx = nullptr;                 // 9. 设备复位(有动作)
    SS_CHK_GetStatus = nullptr;               // 10. 查询设备状态
    SS_CHK_GetFWVer = nullptr;                // 11. 获取固件版本号
    SS_CHK_MoveToRFID = nullptr;              // 12. 走纸到读取RFID位置
    SS_CHK_MoveToRFIDLen = nullptr;           // 13. 走纸到读取RFID位置(指定距离)
    SS_CHK_SetBox = nullptr;                  // 14. 选择票箱
    SS_CHK_Print = nullptr;                   // 15. 单行打印
    SS_CHK_PrintJson = nullptr;               // 16. 多行打印
    SS_CHK_ScanAndGetImage = nullptr;         // 17. 扫描并保存图像
    SS_CHK_SetCheckOCRArea = nullptr;         // 18. 设置票面OCR识别区域
    SS_CHK_GetCheckAndOcrResult = nullptr;    // 19. 获取票据鉴伪及OCR信息
    SS_CHK_SetIdentifyInput = nullptr;        // 20. 设置鉴伪输入参数
    SS_CHK_ScanAndGetImageII = nullptr;       // 21. 扫描并保存图像II
    SS_CHK_GetCheckAndOcrResultII = nullptr;  // 22. 获取票据背面OCR信息

    m_nGetStatErrOLD = SS_OK;
    m_nGetOpenErrOLD = SS_OK;

    // 加载设备动态库
    //if (bLoadLibrary() != TRUE)
    //{
    //    Log(ThisModule, 1, "加载动态库: bLoadLibrary() fail. ");
    //}
}

CDevImpl_RSCD400M::~CDevImpl_RSCD400M()
{

}


/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::DeviceOpen(LPSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;

    // so加载失败,重新加载
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            Log(ThisModule, __LINE__, "打开设备: 重新加载动态库: bLoadLibrary() Failed, Return: %s.",
                ConvertErrCodeToStr(IMP_ERR_LOAD_LIB));
            return IMP_ERR_LOAD_LIB;
        }
    }

    // 打开设备
    nRet = nCHKOpen();
    if (nRet != SS_OK)
    {
        if (m_nGetOpenErrOLD != nRet)
        {
            Log(ThisModule, __LINE__, "打开设备: ->nCHKOpen() Failed, ErrCode:%d, Return: %s.",
                nRet, ConvertErrCodeToStr(nRet));
            m_nGetOpenErrOLD = nRet;
        }
        return nRet;
    }
    m_nGetOpenErrOLD = nRet;

    m_bDevOpenOk = TRUE;

    Log(ThisModule, 1, "打开设备: OpenDevice() Success.");

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::DeviceClose(BOOL bUnLoad)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    if (m_bDevOpenOk == TRUE)
    {
        if (nCHKClose() != 0)
        {
            if (nCHKClose() != 0)
            {
                nCHKClose();
            }
        }
    }

    m_bDevOpenOk = FALSE;

    if (bUnLoad == TRUE)
    {
        vUnLoadLibrary();
    }

    return IMP_SUCCESS;
}

BOOL CDevImpl_RSCD400M::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

/************************************************************
 * 功能: 1. 打开设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKOpen()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_Open();
    if (nRet != SS_OK)
    {
        if (m_nGetOpenErrOLD != nRet)
        {
            Log(ThisModule, __LINE__, "打开设备: SS_CHK_Open() Failed, ErrCode:%d, Return: %s.",
                nRet, ConvertErrCodeToStr(nRet));
            m_nGetOpenErrOLD = nRet;
        }
        return nRet;
    }
    m_nGetOpenErrOLD = nRet;

    return SS_OK;
}

/************************************************************
 * 功能: 2. 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKClose()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_Close();
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "关闭设备: SS_CHK_Close() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 3. 打开入票口
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKInsert()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_Insert();
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "打开入票口: SS_CHK_Insert() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 4. 关闭入票口
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKCancelInsert()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_CancelInsert();
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "关闭入票口: SS_CHK_CancelInsert() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 6. 票据压箱
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKAccept()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_Accept();
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "票据压箱: SS_CHK_Accept() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 7. 退出票据
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKEject()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_Eject();
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "退出票据: SS_CHK_Eject() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 7. 设备复位(无动作)
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKReset()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_Reset();
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "设备复位(无动作: SS_CHK_Reset() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 9. 设备复位(有动作)
 * 参数: nAction 入参 0退票/1压箱/2无动作
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKResetEx(int nAction)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_ResetEx(nAction);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "设备复位(有动作: SS_CHK_ResetEx(%d) Failed, ErrCode:%d, Return: %s.",
            nAction, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 10. 查询设备状态
 * 参数: lpStatus 回参 设备状态
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKGetStatus(LPSCANNERSTATUS lpStatus)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_GetStatus(lpStatus);
    if (nRet != SS_OK)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nGetStatErrOLD)
        {
            m_nGetStatErrOLD = nRet;
            Log(ThisModule, __LINE__, "查询设备状态: SS_CHK_GetStatus() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        }
        return nRet;
    }
    m_nGetStatErrOLD = nRet;

    return SS_OK;
}

/************************************************************
 * 功能: 11. 获取固件版本号
 * 参数: lpVer 回参 版本号
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKGetFWVer(LPSTR lpVer)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_GetFWVer(lpVer);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "获取固件版本号: SS_CHK_GetFWVer() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 12. 退票到读取RFID位置
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKMoveToRFID()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_MoveToRFID();
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "退票到读取RFID位置: SS_CHK_MoveToRFID() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 13. 退票到读取RFID位置(指定距离)
 * 参数: nLen 入参 指定距离(单位:MM)
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKMoveToRFIDLen(INT nLen)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_MoveToRFIDLen(nLen);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "退票到读取RFID位置(指定距离: SS_CHK_MoveToRFIDLen(%d) Failed, ErrCode:%d, Return: %s.",
            nLen, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 14. 选择票箱
 * 参数: dwBoxNo 入参 0右票箱/1左票箱
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKSetBox(DWORD dwBoxNo)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;

    if (dwBoxNo != 0 && dwBoxNo != 1)
    {
        Log(ThisModule, __LINE__, "选择票箱: 入参[%d]无效(0右票箱/1左票箱), Return: %s.",
            ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    nRet = SS_CHK_SetBox(dwBoxNo);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "选择票箱: SS_CHK_SetBox() Failed, ErrCode:%d, Return: %s.",
            dwBoxNo, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 15. 单行打印
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKPrint(LPSTR lpPrtData, int nX, int nY, LPSTR lpFont, int nWidth, int nHeight)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_Print(lpPrtData, nX, nY, lpFont, nWidth, nHeight);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "单行打印: SS_CHK_Print(%s, %d, %d, %s, %d, %d) Failed, ErrCode:%d, Return: %s.",
            lpPrtData, nX, nY, lpFont, nWidth, nHeight, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    } else
    {
        Log(ThisModule, __LINE__, "单行打印: SS_CHK_Print(%s, %d, %d, %s, %d, %d) Succ, ErrCode:%d, Return: %s.",
            lpPrtData, nX, nY, lpFont, nWidth, nHeight, nRet, ConvertErrCodeToStr(nRet));
    }

    return SS_OK;
}

/************************************************************
 * 功能: 16. 多行打印
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKPrintJson(LPSTR lpPrtData)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    Log(ThisModule, __LINE__, "多行打印JSON: %s.", lpPrtData);
    nRet = SS_CHK_PrintJson(lpPrtData);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "多行打印: SS_CHK_PrintJson() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 17. 扫描并保存图像
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKScanAndGetImage(LPSTR lpFrontImageName, LPSTR lpBackImageName,
                                             int *nType, int nTimeOut, int *nDirection)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_ScanAndGetImage(lpFrontImageName, lpBackImageName, nType, nTimeOut, nDirection);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "扫描并保存图像: SS_CHK_ScanAndGetImage(%s, %s, %d, %d, %d) Failed, ErrCode:%d, Return: %s.",
            lpFrontImageName, lpBackImageName, *nType, nTimeOut, *nDirection, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 18. 设置票面OCR识别区域
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKSetCheckOCRArea(LPSTR lpArea)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    Log(ThisModule, __LINE__, "设置票面OCR识别区域JSON: %s.", lpArea);
    nRet = SS_CHK_SetCheckOCRArea(lpArea);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "设置票面OCR识别区域: SS_CHK_SetCheckOCRArea() Failed, ErrCode:%d, Return: %s.",
            nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 19. 获取票据鉴伪及OCR信息
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHKGetCheckAndOcrResult(LPSTR lpResult, INT nRetSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_GetCheckAndOcrResult(lpResult, nRetSize);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "获取票据鉴伪及OCR信息: SS_CHK_GetCheckAndOcrResult(%s, %d) Failed, ErrCode:%d, Return: %s.",
            lpResult, nRetSize, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

/************************************************************
 * 功能: 20. 设置鉴伪输入参数
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHK_SetIdentifyInput(LPSTR lpIdentifyInput)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_SetIdentifyInput(lpIdentifyInput);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "设置鉴伪输入参数: SS_CHK_SetIdentifyInput(%s) Failed, ErrCode:%d, Return: %s.",
            lpIdentifyInput, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    } else
    {
        Log(ThisModule, __LINE__, "设置鉴伪输入参数: SS_CHK_SetIdentifyInput(%s) Succ, ErrCode:%d, Return: %s.",
            lpIdentifyInput, nRet, ConvertErrCodeToStr(nRet));
    }

    return SS_OK;
}

/************************************************************
 * 功能: 21. 扫描并保存图像II
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHK_ScanAndGetImageII(LPSTR lpArea)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_ScanAndGetImageII(lpArea);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "扫描并保存图像II: SS_CHK_ScanAndGetImageII(%s) Failed, ErrCode:%d, Return: %s.",
            lpArea, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    } else
    {
        Log(ThisModule, __LINE__, "扫描并保存图像II: SS_CHK_ScanAndGetImageII(%s) Succ, ErrCode:%d, Return: %s.",
            lpArea, nRet, ConvertErrCodeToStr(nRet));
    }

    return SS_OK;
}

/************************************************************
 * 功能: 22. 获取票据背面OCR信息
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_RSCD400M::nCHK_GetCheckAndOcrResultII(LPSTR lpIdentifyResult, INT nRetSize)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    INT nRet = 0;
    nRet = SS_CHK_GetCheckAndOcrResultII(lpIdentifyResult, nRetSize);
    if (nRet != SS_OK)
    {
        Log(ThisModule, __LINE__, "获取票据背面OCR信息: SS_CHK_GetCheckAndOcrResultII(%s, %d) Failed, ErrCode:%d, Return: %s.",
            lpIdentifyResult, nRetSize, nRet, ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return SS_OK;
}

// 错误码及含义
CHAR* CDevImpl_RSCD400M::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nRet)
    {
        case SS_OK:                                                 // 0:操作成功
            sprintf(m_szErrStr, "%d|%s", nRet, "操作成功");
            return m_szErrStr;
        case SS_ERR_NOTSUPPORTED:                                   // -1:设备不支持此操作:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备不支持此操作");
            return m_szErrStr;
        case SS_ERR_TIMEOUT:                                        // 2:操作超时
            sprintf(m_szErrStr, "%d|%s", nRet, "操作超时");
            return m_szErrStr;
        case SS_ERR_HWERROR:                                        // -3:硬件出错
            sprintf(m_szErrStr, "%d|%s", nRet, "硬件出错");
            return m_szErrStr;
        case SS_ERR_INVALIDFILEPATH:                                //  -4:无效的文件路径
            sprintf(m_szErrStr, "%d|%s", nRet, "无效的文件路径");
            return m_szErrStr;
        case SS_ERR_INVALIDPARAMETER:                               // -5:无效的参数
            sprintf(m_szErrStr, "%d|%s", nRet, "无效的参数");
            return m_szErrStr;
        case SS_ERR_DEVICECLOSED:                                   // -6:设备已关闭
            sprintf(m_szErrStr, "%d|%s", nRet, "设备已关闭");
            return m_szErrStr;
        case SS_ERR_CANCELED:                                       // -7:操作被取消
            sprintf(m_szErrStr, "%d|%s", nRet, "操作被取消");
            return m_szErrStr;
        case SS_ERR_POCKETFULL:                                     // -8:票箱已满
            sprintf(m_szErrStr, "%d|%s", nRet, "票箱已满");
            return m_szErrStr;
        case SS_ERR_NOINKPRESENT:                                   // -9:墨盒不在位
            sprintf(m_szErrStr, "%d|%s", nRet, "墨盒不在位");
            return m_szErrStr;
        case SS_ERR_PARSEJSON:                                      // -10:解析Json参数错误
            sprintf(m_szErrStr, "%d|%s", nRet, "解析Json参数错误");
            return m_szErrStr;
        case SS_ERR_LOADALGDLL:                                     // -20:加载算法库失败
            sprintf(m_szErrStr, "%d|%s", nRet, "加载算法库失败");
            return m_szErrStr;
        case SS_ERR_INITALG:                                        // -21:算法初始化接口失败
            sprintf(m_szErrStr, "%d|%s", nRet, "算法初始化接口失败");
            return m_szErrStr;
        case SS_ERR_SETBRIGHT:                                      // -22:设置图像亮度失败
            sprintf(m_szErrStr, "%d|%s", nRet, "设置图像亮度失败");
            return m_szErrStr;
        case SS_ERR_POCKETOPEN:                                     // -23:票箱开
            sprintf(m_szErrStr, "%d|%s", nRet, "票箱开");
            return m_szErrStr;
        case SS_ERR_OTHER:                                          // -99999:其他错误
            sprintf(m_szErrStr, "%d|%s", nRet, "其他错误");
            return m_szErrStr;
        case SS_ERR_CHK_JAM:                                        // -12010:卡票
            sprintf(m_szErrStr, "%d|%s", nRet, "卡票");
            return m_szErrStr;
        case SS_ERR_CHK_NOPAPERINPRINTPOSITION:                     // -12011:打印位置没有票据
            sprintf(m_szErrStr, "%d|%s", nRet, "打印位置没有票据");
            return m_szErrStr;
        case SS_ERR_CHK_NOMEDIAPRESENT:                             // -12020:通道无票
            sprintf(m_szErrStr, "%d|%s", nRet, "通道无票");
            return m_szErrStr;
        case SS_ERR_CHK_NOIMAGE:                                    // -12021:无可用图像数据
            sprintf(m_szErrStr, "%d|%s", nRet, "无可用图像数据");
            return m_szErrStr;
        case SS_ERR_CHK_MEDIAPRESENT:                               // -12022:通道有票
            sprintf(m_szErrStr, "%d|%s", nRet, "通道有票");
            return m_szErrStr;
        case SS_ERR_CHK_UNKNOWTYPE:                                 // -12030:票据类型未知
            sprintf(m_szErrStr, "%d|%s", nRet, "票据类型未知");
            return m_szErrStr;
        case SS_ERR_CHK_IMPERFECT:                                  // -12031:票据缺角
            sprintf(m_szErrStr, "%d|%s", nRet, "票据缺角");
            return m_szErrStr;
        case SS_ERR_CHK_FORGED:                                     // -12032:伪票
            sprintf(m_szErrStr, "%d|%s", nRet, "伪票");
            return m_szErrStr;
        case SS_ERR_CHK_TAMPER:                                     // -12033:篡改票
            sprintf(m_szErrStr, "%d|%s", nRet, "篡改票");
            return m_szErrStr;
        case SS_ERR_CHK_NOOCRAREA:                                  // -12034:未设置OCR区域
            sprintf(m_szErrStr, "%d|%s", nRet, "未设置OCR区域");
            return m_szErrStr;
        case SS_ERR_CHK_INCOMPLETE:                                 // -12040:票据要素不全
            sprintf(m_szErrStr, "%d|%s", nRet, "票据要素不全");
            return m_szErrStr;
        case SS_ERR_CHK_TYPEERROR:                                  // -12050:票据类型不符
            sprintf(m_szErrStr, "%d|%s", nRet, "票据类型不符");
            return m_szErrStr;
        case SS_ERR_CHK_DIR:                                        // -12070:票据正反面放置错误
            sprintf(m_szErrStr, "%d|%s", nRet, "票据正反面放置错误");
            return m_szErrStr;
        case SS_ERR_CHK_LONGLENGTH:                                 // -12080:票据超长
            sprintf(m_szErrStr, "%d|%s", nRet, "票据超长");
            return m_szErrStr;
        case SS_ERR_CHK_READRFID:                                   // -12090:票据超长
            sprintf(m_szErrStr, "%d|%s", nRet, "票据超长");
            return m_szErrStr;
        case SS_ERR_CHK_EJECT_JAM:                                  // -12503:退票失败,卡票
            sprintf(m_szErrStr, "%d|%s", nRet, "退票失败,卡票");
            return m_szErrStr;
        case SS_ERR_CHK_SAVEIMAGE:                                  // -12601:图像保存失败
            sprintf(m_szErrStr, "%d|%s", nRet, "图像保存失败");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_PATAMETER:             // 501:鉴伪参数错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪参数错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_CONFIGFILE:            // 502:鉴伪配置文件格式错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪配置文件格式错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_CONFIGDATA:            // 503:鉴伪配置文件数据错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪配置文件数据错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_TYPENUMMANY:           // 504:鉴伪存单类型数量太多
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪存单类型数量太多");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_LOADLIBRARY:           // 505:鉴伪加载动态库失败
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪加载动态库失败");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_DECRYPT:               // 506:鉴伪解密错误，交互验证不通过
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪解密错误,交互验证不通过");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_DESKEWCROP:            // 507:鉴伪纠偏裁剪错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪纠偏裁剪错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_WIDTHHEIGHT:           // 508:鉴伪票据图像宽度高度不符
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪票据图像宽度高度不符");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_BILLTYPE:              // 509:鉴伪票据类型错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪票据类型错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_BASEPOINT:             // 510:鉴伪定位基准点失败
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪定位基准点失败");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_FRONTREAR:             // 511:鉴伪正反面放反
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪正反面放反");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_MALLOCBUFFER:          // 512:鉴伪内存申请错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪内存申请错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NONETEMPLATE:          // 513:鉴伪没有模板数据
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪没有模板数据");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NULLDATA:              // 514:鉴伪没有图像数据
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪没有图像数据");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NONEFEATURE:           // 515:鉴伪没有特征数据
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪没有特征数据");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_WATERMARK:             // 516:鉴伪水印错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪水印错误");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_ANGLEBIG:              // 517:鉴伪倾斜角度大
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪倾斜角度大");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_FINDRECT:              // 518:鉴伪查找区域失败
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪查找区域失败");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_NONEFUNC:              // 600:鉴伪缺少功能
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪缺少功能");
            return m_szErrStr;
        case SCANNER_ERROR_CD_IDENTIFY_ERROR_OHTER:                 // 700:鉴伪其它错误
            sprintf(m_szErrStr, "%d|%s", nRet, "鉴伪其它错误");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_PARAS:                         // -801:OCR参数错误
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR参数错误");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_LOAD_CONFIG_FILE:              // -802:OCR加载主动态库配置文件错误
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR加载主动态库配置文件错误");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_LOAD_CONFIG_INFO:              // -803:OCR主配置文件配置文件信息错误
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR主配置文件配置文件信息错误");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_LOAD_DLL:                      // -804:OCR加载识别动态库失败
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR加载识别动态库失败");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_GET_DLL_PROC:                  // -805:OCR获取导出识别函数失败
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR获取导出识别函数失败");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_DLL_INIT:                      // -806:OCR识别动态库初始化函数返回值错误
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR识别动态库初始化函数返回值错误");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_RECOG:                         // -807:OCR识别失败
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR识别失败");
            return m_szErrStr;
        case SCANNER_ERROR_OCR_ERROR_OHTER:                         // -900:OCR其它错误
            sprintf(m_szErrStr, "%d|%s", nRet, "OCR其它错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_PATAMETER:             // -1001:支票鉴伪参数错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪参数错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_ANGLEBING:             // -1002:支票鉴伪倾斜角度大
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪倾斜角度大");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_QUALITY:               // -1003:支票鉴伪质量检测不合格
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪质量检测不合格");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_TABLESIZE:             // -1004:支票鉴伪表格大小不符合要求
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪表格大小不符合要求");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_BILLTYPE:              // -1005:支票鉴伪票据类型错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪票据类型错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_IMAGEFLIP:             // -1006:支票鉴伪图像方向颠倒
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪图像方向颠倒");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_IMAGEDARK:             // -1007:支票鉴伪红外发射过暗
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪红外发射过暗");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_INCOMPLETE:            // -1008:支票鉴伪票面扫描不完整
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪票面扫描不完整");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_LOCATION:              // -1009:支票鉴伪定位失败
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪定位失败");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_FRONTREAR:             // -1010:支票鉴伪图像正面与反面放反
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪图像正面与反面放反");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_BRTIRTR:               // -1030:支票鉴伪红外透射亮度异常
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪红外透射亮度异常");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_DECRYPT:               // -1101:支票鉴伪解密错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪解密错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_DESKEWCROP:            // -1102:支票鉴伪纠偏错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪纠偏错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_DESKEWCROPNOBASEIMAGE: // -1103:支票鉴伪无纠偏基准图像
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪无纠偏基准图像");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_CHEQUETYPE:            // -1200:支票鉴伪票据类型错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪票据类型错误");
            return m_szErrStr;
        case SCANNER_ERROR_ZP_IDENTIFY_ERROR_OHTER:                 // -1300:支票鉴伪其他错误
            sprintf(m_szErrStr, "%d|%s", nRet, "支票鉴伪其他错误");
            return m_szErrStr;
        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义值");
            return m_szErrStr;
    }
}

// 设置动态库路径(DeviceOpen前有效)
void CDevImpl_RSCD400M::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    // 设定动态库路径
    if (lpPath == nullptr || strlen(lpPath) < 1 ||
        (strlen(lpPath) == 1 && (char*)lpPath[0] == "/"))
    {
        Log(ThisModule, 1, "入参[%s]无效,不设置动态库路径.");
        return;
    }

    if ((char*)lpPath[0] == "/")   // 绝对路径
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s", lpPath);
    } else
    {
        memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
        sprintf(m_szLoadDllPath, "%s%s", LINUXPATHLIB, lpPath);
    }

    Log(ThisModule, 1, "设置动态库路径=<%s>.", m_szLoadDllPath);
}

BOOL CDevImpl_RSCD400M::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);


    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            Log(ThisModule, 1, "加载动态库<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        } else
        {
            Log(ThisModule, 1, "加载动态库<%s> Succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            Log(ThisModule, 1, "加载动态库函数接口<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
        {
            Log(ThisModule, 1, "加载动态库函数接口<%s> Succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

void CDevImpl_RSCD400M::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_RSCD400M::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1. 打开设备
    FUNC_LOADLIBRARY(m_SS_CHK_Open, SS_CHK_Open, "SS_CHK_Open");
    // 2. 关闭设备
    FUNC_LOADLIBRARY(m_SS_CHK_Close, SS_CHK_Close, "SS_CHK_Close");
    // 3. 打开入票口
    FUNC_LOADLIBRARY(m_SS_CHK_Insert, SS_CHK_Insert, "SS_CHK_Insert");
    // 4. 关闭入票口
    FUNC_LOADLIBRARY(m_SS_CHK_CancelInsert, SS_CHK_CancelInsert, "SS_CHK_CancelInsert");
    // 6. 票据压箱
    FUNC_LOADLIBRARY(m_SS_CHK_Accept, SS_CHK_Accept, "SS_CHK_Accept");
    // 7. 退出票据
    FUNC_LOADLIBRARY(m_SS_CHK_Eject, SS_CHK_Eject, "SS_CHK_Eject");
    // 8. 设备复位(无动作)
    FUNC_LOADLIBRARY(m_SS_CHK_Reset, SS_CHK_Reset, "SS_CHK_Reset");
    // 9. 设备复位(有动作)
    FUNC_LOADLIBRARY(m_SS_CHK_ResetEx, SS_CHK_ResetEx, "SS_CHK_ResetEx");
    // 10. 查询设备状态
    FUNC_LOADLIBRARY(m_SS_CHK_GetStatus, SS_CHK_GetStatus, "SS_CHK_GetStatus");
    // 11. 获取固件版本号
    FUNC_LOADLIBRARY(m_SS_CHK_GetFWVer, SS_CHK_GetFWVer, "SS_CHK_GetFWVer");
    // 12. 走纸到读取RFID位置
    FUNC_LOADLIBRARY(m_SS_CHK_MoveToRFID, SS_CHK_MoveToRFID, "SS_CHK_MoveToRFID");
    // 13. 走纸到读取RFID位置(指定距离)
    FUNC_LOADLIBRARY(m_SS_CHK_MoveToRFIDLen, SS_CHK_MoveToRFIDLen, "SS_CHK_MoveToRFIDLen");
    // 14. 选择票箱
    FUNC_LOADLIBRARY(m_SS_CHK_SetBox, SS_CHK_SetBox, "SS_CHK_SetBox");
    // 15. 单行打印
    FUNC_LOADLIBRARY(m_SS_CHK_Print, SS_CHK_Print, "SS_CHK_Print");
    // 16. 多行打印
    FUNC_LOADLIBRARY(m_SS_CHK_PrintJson, SS_CHK_PrintJson, "SS_CHK_PrintJson");
    // 17. 扫描并保存图像
    FUNC_LOADLIBRARY(m_SS_CHK_ScanAndGetImage, SS_CHK_ScanAndGetImage, "SS_CHK_ScanAndGetImage");
    // 18. 设置票面OCR识别区域
    FUNC_LOADLIBRARY(m_SS_CHK_SetCheckOCRArea, SS_CHK_SetCheckOCRArea, "SS_CHK_SetCheckOCRArea");
    // 19. 获取票据鉴伪及OCR信息
    FUNC_LOADLIBRARY(m_SS_CHK_GetCheckAndOcrResult, SS_CHK_GetCheckAndOcrResult, "SS_CHK_GetCheckAndOcrResult");
    // 20. 设置鉴伪输入参数
    FUNC_LOADLIBRARY(m_SS_CHK_SetIdentifyInput, SS_CHK_SetIdentifyInput, "SS_CHK_SetIdentifyInput");
    // 21. 扫描并保存图像II
    FUNC_LOADLIBRARY(m_SS_CHK_ScanAndGetImageII, SS_CHK_ScanAndGetImageII, "SS_CHK_ScanAndGetImageII");
    // 22. 获取票据背面OCR信息
    FUNC_LOADLIBRARY(m_SS_CHK_GetCheckAndOcrResultII, SS_CHK_GetCheckAndOcrResultII, "SS_CHK_GetCheckAndOcrResultII");

    return TRUE;
}

//---------------------------------------------
