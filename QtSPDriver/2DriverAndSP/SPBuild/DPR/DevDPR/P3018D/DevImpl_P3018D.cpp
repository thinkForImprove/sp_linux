/***************************************************************
* 文件名称：DevImpl_P3018D.cpp
* 文件描述：HL-2260D打印模块底层控制指令接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年8月23日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_P3018D.h"

static const char *ThisFile = "DevImpl_P3018D.cpp";

CDevImpl_P3018D::CDevImpl_P3018D()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_P3018D::CDevImpl_P3018D(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);  // 设置日志文件名和错误发生的文件
    Init();
}

// 参数初始化
void CDevImpl_P3018D::Init()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_MutexAction);

    m_bDevOpenOk = FALSE;
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
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
    Device_Printer_Open = nullptr;            // 1. 获取设备句柄
    Device_Printer_Close = nullptr;           // 2. 关闭设备
    Device_Printer_GetPageNum = nullptr;      // 3. 获取设备已打印页数
    Device_Printer_GetVersion = nullptr;      // 4. 获取设备版本号
    Device_Printer_GetSerialNumber = nullptr; // 5. 获取设备序列号
    Device_Printer_GetState = nullptr;        // 6. 获取设备状态
    Device_Printer_GetDeviceInfo = nullptr;   // 7. 获取设备信息
    Device_Printer_GetErrmsg = nullptr;       // 8. 获取设备状态描述信息
    Device_Printer_GetPrinterList = nullptr;  // 9. 获取支持的打印机列表
    Device_Printer_DeleteCurrentJob = nullptr;// 10. 删除当前打印作业
    Device_Printer_WarmUp = nullptr;          // 11. 预热打印机
    Device_Printer_SetSleepTime = nullptr;    // 12. 设置打印机休眠时间

//    //加载设备动态库
//    if (!bLoadLibrary())
//    {
//        Log(ThisFile, 1, "加载动态库: bLoadLibrary() fail. ");
//    }
}

CDevImpl_P3018D::~CDevImpl_P3018D()
{

}

BOOL CDevImpl_P3018D::bLoadLibrary()
{
    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            Log(ThisFile, 1, "加载动态库<%s> fail. ReturnCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            return FALSE;
        }
    }
    return TRUE;
}

void CDevImpl_P3018D::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

BOOL CDevImpl_P3018D::bLoadLibIntf()
{
    m_bLoadIntfFail = FALSE;

    // 1
    Device_Printer_Open = (m_SS_Device_Printer_Open)m_LoadLibrary.resolve("Device_Printer_Open");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_Open, "m_SS_Device_Printer_Open");
    // 2
    Device_Printer_Close = (m_SS_Device_Printer_Close)m_LoadLibrary.resolve("Device_Printer_Close");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_Close, "m_SS_Device_Printer_Close");
    // 3
    Device_Printer_GetPageNum = (m_SS_Device_Printer_GetPageNum)m_LoadLibrary.resolve("Device_Printer_GetPageNum");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_GetPageNum, "m_SS_Device_Printer_GetPageNum");
    // 4
    Device_Printer_GetVersion = (m_SS_Device_Printer_GetVersion)m_LoadLibrary.resolve("Device_Printer_GetVersion");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_GetVersion, "m_SS_Device_Printer_GetVersion");
    // 5
    Device_Printer_GetSerialNumber = (m_SS_Device_Printer_GetSerialNumber)m_LoadLibrary.resolve("Device_Printer_GetSerialNumber");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_GetSerialNumber, "m_SS_Device_Printer_GetSerialNumber");
    // 6
    Device_Printer_GetState = (m_SS_Device_Printer_GetState)m_LoadLibrary.resolve("Device_Printer_GetState");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_GetState, "m_SS_Device_Printer_GetState");
    // 7
    Device_Printer_GetDeviceInfo = (m_SS_Device_Printer_GetDeviceInfo)m_LoadLibrary.resolve("Device_Printer_GetDeviceInfo");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_GetDeviceInfo, "m_SS_Device_Printer_GetDeviceInfo");
    // 8
    Device_Printer_GetErrmsg = (m_SS_Device_Printer_GetErrmsg)m_LoadLibrary.resolve("Device_Printer_GetErrmsg");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_GetErrmsg, "m_SS_Device_Printer_GetErrmsg");
    // 9
    Device_Printer_GetPrinterList = (m_SS_Device_Printer_GetPrinterList)m_LoadLibrary.resolve("Device_Printer_GetPrinterList");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_GetPrinterList, "m_SS_Device_Printer_GetPrinterList");
    // 10
    Device_Printer_DeleteCurrentJob = (m_SS_Device_Printer_DeleteCurrentJob)m_LoadLibrary.resolve("Device_Printer_DeleteCurrentJob");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_DeleteCurrentJob, "m_SS_Device_Printer_DeleteCurrentJob");
    // 11
    Device_Printer_WarmUp = (m_SS_Device_Printer_WarmUp)m_LoadLibrary.resolve("Device_Printer_WarmUp");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_WarmUp, "m_SS_Device_Printer_WarmUp");
    // 12
    Device_Printer_SetSleepTime = (m_SS_Device_Printer_SetSleepTime)m_LoadLibrary.resolve("Device_Printer_SetSleepTime");
    FUNC_POINTER_ERROR_RETURN(Device_Printer_SetSleepTime, "m_SS_Device_Printer_SetSleepTime");

    return TRUE;
}


BOOL CDevImpl_P3018D::OpenDevice()
{
    return OpenDevice(0);
}

BOOL CDevImpl_P3018D::OpenDevice(WORD wType)
{
    //如果已打开，则关闭
    CloseDevice();

    m_bDevOpenOk = FALSE;

    if (bLoadLibrary() != TRUE)
    {
        Log(ThisFile, 1, "加载动态库: OpenDevice()->LoadCloudWalkDll() fail. ");
        return FALSE;
    }

    m_bDevOpenOk = TRUE;
    return TRUE;
}

BOOL CDevImpl_P3018D::CloseDevice()
{
    vUnLoadLibrary();
    m_bDevOpenOk = FALSE;

    Log(ThisFile, 1, "设备关闭,释放动态库: Close Device, unLoad libdevice_pantum_pt3018.so.2.4.0.");

    return TRUE;
}

BOOL CDevImpl_P3018D::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

// 设置动态库路径(DeviceOpen前有效)
void CDevImpl_P3018D::SetLibPath(LPCSTR lpPath)
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

//---------------------------------------------
// 接口对外
// 1. 获取设备句柄
int CDevImpl_P3018D::Open(char *printername, void* printer_handle, int *nNeeded)
{
    return Device_Printer_Open(printername, printer_handle, nNeeded);
}
// 2. 关闭设备
void CDevImpl_P3018D::Close(void *printer_handle)
{
    Device_Printer_Close(printer_handle);
}
// 3. 获取设备已打印页数
int CDevImpl_P3018D::GetPageNum(void *printer_handle, unsigned int *page)
{
    return Device_Printer_GetPageNum(printer_handle, page);
}

int CDevImpl_P3018D::GetVersion(void *printer_handle, char *version)
{
    return Device_Printer_GetVersion(printer_handle, version);
}

int CDevImpl_P3018D::GetSerialNumber(void *printer_handle, char *serial)
{
    return Device_Printer_GetSerialNumber(printer_handle, serial);
}

int CDevImpl_P3018D::GetState(void *printer_handle, unsigned char *state)
{
    return Device_Printer_GetState(printer_handle, state);
}

int CDevImpl_P3018D::GetDeviceInfo(void *printer_handle, char *json)
{
    return Device_Printer_GetDeviceInfo(printer_handle, json);
}

int CDevImpl_P3018D::GetErrmsg(void *printer_handle, char *errinfo)
{
    return Device_Printer_GetErrmsg(printer_handle, errinfo);
}

int CDevImpl_P3018D::GetPrinterList(char *json, int *size)
{
    return Device_Printer_GetPrinterList(json, size);
}

int CDevImpl_P3018D::DeleteCurrentJob(void *printer_handle)
{
    return Device_Printer_DeleteCurrentJob(printer_handle);
}

int CDevImpl_P3018D::WarmUp(void *printer_handle)
{
    return Device_Printer_WarmUp(printer_handle);
}

int CDevImpl_P3018D::SetSleepTime(void *printer_handle, int time)
{
    return Device_Printer_SetSleepTime(printer_handle, time);
}

