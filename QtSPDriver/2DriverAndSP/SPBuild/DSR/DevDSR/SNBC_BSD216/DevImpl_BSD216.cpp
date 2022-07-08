/***************************************************************
* 文件名称：DevImpl_BSD216.cpp
* 文件描述：PRM存折打印模块底层指令，提供控制接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevImpl_BSD216.h"
#include "../XFS_DSR/def.h"
#include <qthread.h>

#define ThisFile (basename(__FILE__))


#define CHK_DEV_OPEN_FLAG(OFLAG) \
    if (OFLAG != TRUE) \
    { \
        if (m_nRetErrOLD[7] != IMP_ERR_NOTOPEN) \
        { \
            Log(ThisModule, __LINE__, "检查设备OPEN标记: OpenFlag == FALSE, Device Not Open, return fail.Return: %s.", \
            ConvertErrCodeToStr(IMP_ERR_NOTOPEN)); \
        } \
        m_nRetErrOLD[7] = IMP_ERR_NOTOPEN; \
        return IMP_ERR_NOTOPEN; \
    }

//----------------------------------构造/析构/初始化----------------------------------
CDevImpl_BSD216::CDevImpl_BSD216()
{
    SetLogFile(LOG_NAME, ThisFile);  // 设置日志文件名和错误发生的源文件名
    Init();
}

CDevImpl_BSD216::CDevImpl_BSD216(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);
    Init();
}

CDevImpl_BSD216::CDevImpl_BSD216(LPSTR lpLog, LPSTR lpDevStr)
{
    SetLogFile(lpLog, ThisFile, lpDevStr);
    Init();
}

CDevImpl_BSD216::~CDevImpl_BSD216()
{
    DeviceClose();
}

// 参数初始化
void CDevImpl_BSD216::Init()
{
    THISMODULE(__FUNCTION__);

    m_bDevOpenOk = FALSE;

    m_szDevStataOLD = 0x00;                                 // 上一次获取设备状态保留
    m_bReCon = FALSE;                                       // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));
    MCPY_NOLEN(m_szOpenMode, "USB");                        // 打开模式(缺省USB)
    m_nBaudRate = 9600;                                     // 波特率(缺省9600)

    // 设定动态库路径
    QString strDllName(QString::fromLocal8Bit(DLL_DEVLIB_NAME));
    strDllName.prepend("DSR/BSD216/");
    #ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
    #else
        strDllName.prepend(LINUXPATHLIB);
    #endif

    memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    sprintf(m_szLoadDllPath, "%s", strDllName.toStdString().c_str());

    m_bLoadIntfFail = TRUE;
}

//----------------------------------SDK接口加载----------------------------------
// 加载动态库
BOOL CDevImpl_BSD216::bLoadLibrary()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(m_MutexAction);

    m_LoadLibrary.setFileName(m_szLoadDllPath);
    m_bLoadIntfFail = TRUE;

    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库<%s> Fail. ErrCode:%s.",
                    m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        } else
        {
            Log(ThisModule, __LINE__, "加载动态库<%s> Succ. ", m_szLoadDllPath);
        }
    }

    if (m_bLoadIntfFail)
    {
        if (bLoadLibIntf() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "加载动态库函数接口<%s> Fail. ErrCode:%s.",
                m_szLoadDllPath, m_LoadLibrary.errorString().toStdString().c_str());
            }
            return FALSE;
        }
        {
            Log(ThisModule, __LINE__, "加载动态库函数接口<%s> Succ. ", m_szLoadDllPath);
        }
    }

    return TRUE;
}

// 释放动态库
void CDevImpl_BSD216::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfFail = TRUE;
    }
}

// 加载动态库接口函数
BOOL CDevImpl_BSD216::bLoadLibIntf()
{
    LOAD_LIBINFO_FUNC(pScan_Open, Scan_Open, "Scan_Open");
    LOAD_LIBINFO_FUNC(pScan_Init, Scan_Init, "Scan_Init");
    LOAD_LIBINFO_FUNC(pScan_Start, Scan_Start, "Scan_Start");
    LOAD_LIBINFO_FUNC(pScan_GetStatus, Scan_GetStatus, "Scan_GetStatus");
    LOAD_LIBINFO_FUNC(pScan_SetPaperOut, Scan_SetPaperOut, "Scan_SetPaperOut");
    LOAD_LIBINFO_FUNC(pScan_GetDecodeLen, Scan_GetDecodeLen, "Scan_GetDecodeLen");
    LOAD_LIBINFO_FUNC(pScan_GetImages, Scan_GetImages, "Scan_GetImages");
    LOAD_LIBINFO_FUNC(pScan_GetDecode, Scan_GetDecode, "Scan_GetDecode");
    LOAD_LIBINFO_FUNC(pScan_GetFWVersion, Scan_GetFWVersion, "Scan_GetFWVersion");
    LOAD_LIBINFO_FUNC(pScan_GetSerialNumber, Scan_GetSerialNumber, "Scan_GetSerialNumber");
    LOAD_LIBINFO_FUNC(pScan_UpdateFirmware, Scan_UpdateFirmware, "Scan_UpdateFirmware");
    LOAD_LIBINFO_FUNC(pScan_Close, Scan_Close, "Scan_Close");
    LOAD_LIBINFO_FUNC(pScan_Reset, Scan_Reset, "Scan_Reset");

    m_bLoadIntfFail = FALSE;

    return TRUE;
}

//----------------------------------SDK封装接口方法----------------------------------
// 打开设备
INT CDevImpl_BSD216::DeviceOpen()
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    DeviceClose();

    m_bDevOpenOk = FALSE;

    // so 未加载 or 加载失败
    if (m_bLoadIntfFail == TRUE)
    {
        if (bLoadLibrary() != TRUE)
        {
            if (m_nRetErrOLD[0] != IMP_ERR_LOAD_LIB)
            {
                Log(ThisModule, __LINE__, "打开设备: 加载动态库: bLoadLibrary Failed, Return: %s.", ConvertErrCodeToStr(IMP_ERR_LOAD_LIB));
            }
            m_nRetErrOLD[0] = IMP_ERR_LOAD_LIB;
            return IMP_ERR_LOAD_LIB;
        }
        m_nRetErrOLD[0] = IMP_SUCCESS;
    }

    m_nRetErrOLD[0] = IMP_SUCCESS;

    // 设备连接
    nRet = Scan_Open();
    if(SNBC_ERROR_NONE != nRet)
    {
        if (nRet != m_nRetErrOLD[1])
        {
            Log(ThisModule, __LINE__,
                "打开设备: Scan_Open Failed, ErrCode: %d, Return: %s.", nRet, ConvertErrCodeToStr(nRet));
            m_nRetErrOLD[1] = nRet;
        }
        return nRet;
    } else
    {
        Log(ThisModule, __LINE__, "打开设备: Scan_Open Success.");
    }

    QString strINIFile("DSR/BSD216/configure.ini");

    #ifdef Q_OS_WIN
        strINIFile.prepend(WINPATH);
    #else
        strINIFile.prepend(LINUXPATHLIB);
    #endif

    nRet = Scan_Init(strINIFile.toLatin1().data());
    if (nRet != IMP_SUCCESS)
    {
        if (nRet != m_nRetErrOLD[5])
        {
            m_nRetErrOLD[5] = nRet;
            Log(ThisModule, __LINE__,
                "打开设备: Scan_Init: Scan_Init Failed, ErrCode: %d, Return: %s.", nRet, ConvertErrCodeToStr(nRet));
        }
        return nRet;
    } else
    {
        Log(ThisModule, __LINE__,
            "打开设备: Reset: Reset(%d) Success.", INIT_NOACTION);
    }

    if (m_bReCon == TRUE)
    {
        Log(ThisModule, __LINE__, "设备重连 End......");
    }

    m_bReCon = FALSE;   // 是否断线重连状态: 初始F
    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));
    m_bDevOpenOk = TRUE;

    return IMP_SUCCESS;
}

// 关闭设备
INT CDevImpl_BSD216::DeviceClose()
{
    THISMODULE(__FUNCTION__);

    if (m_bDevOpenOk == TRUE)
    {
        if (Scan_Close)
        {
            Scan_Close();
        }
        m_bDevOpenOk = FALSE;
    }

    if (m_bReCon != TRUE)   // 断线重连状态时不释放动态库
    {
        vUnLoadLibrary();
        Log(ThisModule, __LINE__, "关闭设备: Close Device, and unLoad SDK Dll.");
    }    

    return IMP_SUCCESS;
}

// 复位设备
INT CDevImpl_BSD216::Reset(INT nAction, INT nTimeOut)
{
    THISMODULE(__FUNCTION__);

    AutoMutex(m_MutexAction);
//Log(ThisModule, __LINE__, "on Rest thread id = %d", QThread::currentThread());
    int nRet = Scan_Reset(0);
    if (nRet != SNBC_ERROR_NONE)
    {
        if (nRet != m_nRetErrOLD[5])
        {
            Log(ThisModule, __LINE__, "设备复位: Scan_Init(%d, %d) fail. Return: %s.",
                nAction, nTimeOut, ConvertErrCodeToStr(nRet));
        }

        return nRet;
    }

    usleep(4 * 1000 * 1000);

    DeviceOpen();

    return IMP_SUCCESS;
}

// 退纸(介质退出)
INT CDevImpl_BSD216::MediaEject()
{
    THISMODULE(__FUNCTION__);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = Scan_SetPaperOut(1, 0);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "退纸: Scan_SetPaperOut() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    return IMP_SUCCESS;
}

// 获取设备状态
INT CDevImpl_BSD216::GetDevStatus(DEVICESTATUS *ds)
{
    THISMODULE(__FUNCTION__);

    AutoMutex(m_MutexAction);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    INT nRet = Scan_GetStatus(ds);
    //Log(ThisModule, __LINE__, "GetDevStatus thread id = %d, %d", QThread::currentThread(), nRet);
    if (nRet != m_nRetErrOLD[2])    // 比较两次状态记录LOG
    {
        Log(ThisModule, __LINE__, "设备状态变化: %d->%d.", m_nRetErrOLD[2], nRet);
        m_nRetErrOLD[2] = nRet;
    }

    if (nRet != IMP_SUCCESS)
    {
        // 该接口调用频繁,记录本次错误码与上次比较,不同则记录Log,用于避免多次写log造成文本冗余
        if (nRet != m_nRetErrOLD[2])
        {
            Log(ThisModule, __LINE__, "获取设备状态: Scan_GetStatus() fail. Return: %s.", ConvertErrCodeToStr(nRet));
            m_nRetErrOLD[2] = nRet;
        }
        return nRet;
    }

    return nRet;
}

// 获取固件版本
INT CDevImpl_BSD216::GetFWVersion(LPSTR pBuff, DWORD dwSize)
{
    THISMODULE(__FUNCTION__);

    CHK_DEV_OPEN_FLAG(m_bDevOpenOk);    // 检查设备OPEN标记

    CHAR szVersion[256] = { 0x00 };

    if (pBuff == NULL || dwSize == 0)
    {
        Log(ThisModule, __LINE__, "获取固件版本: Input Buffer fail. Return: %s.", ConvertErrCodeToStr(IMP_ERR_PARAM_INVALID));
        return IMP_ERR_PARAM_INVALID;
    }

    int nRet = Scan_GetFWVersion(szVersion);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "获取固件版本: PR2_GetFWID() fail. Return: %s.", ConvertErrCodeToStr(nRet));
        return nRet;
    }

    memcpy(pBuff, szVersion, (strlen(szVersion) > dwSize ? dwSize : strlen(szVersion)));

    return IMP_SUCCESS;
}

INT CDevImpl_BSD216::ImplScanImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut)
{
    int nRet = SNBC_ERROR_NONE;

REDO:
    nRet = Scan_Start();
    if (nRet != SNBC_ERROR_NONE)
    {
        if (nRet == SNBC_NO_PAPER_ERROR)
        {
            usleep(300 * 1000);
            goto REDO;
        }
        return IMP_ERR_UNKNOWN;
    }
    else {
        ImageDataAndInfo PicFront, PicRear;
        memset(&PicFront, 0, sizeof(ImageDataAndInfo));
        memset(&PicRear, 0, sizeof(ImageDataAndInfo));
        nRet = Scan_GetImages(&PicFront, &PicRear);
        if (nRet != SNBC_ERROR_NONE)
        {
            return IMP_ERR_UNKNOWN;
        }
        else
        {
            if (stImageIn.wInMode & IMAGE_MODE_FRONT)
            {
                if(PicFront.lPicSize > 0)
                {
                    FILE *fp = NULL;
                    fp = fopen(stImageIn.szImageFrontPath, "wb");
                    if (fp)
                    {
                        fwrite((char *)PicFront.PicBuffer, 1, PicFront.lPicSize, fp);
                        fclose(fp);
                    }
                }
            }

            if (stImageIn.wInMode & IMAGE_MODE_BACK)
            {
                if(PicRear.lPicSize > 0)
                {
                    FILE *fp = NULL;
                    fp = fopen(stImageIn.szImageBackPath, "wb");
                    if (fp)
                    {
                        fwrite((char *)PicRear.PicBuffer, 1, PicRear.lPicSize, fp);
                        fclose(fp);
                    }

                }
            }
        }

        if (PicFront.PicBuffer) {
            free(PicFront.PicBuffer);
        }
        if (PicRear.PicBuffer) {
            free(PicRear.PicBuffer);
        }

        if (stImageIn.wInMode & IMAGE_MODE_CODELINE)
        {
            int nDecodeLen_F = 0, nDecodeLen_R = 0;
            int nDecodeType_F = 0, nDecodeType_R = 0;
            char szDecResult_F[2048], szDecResult_R[2048];
            memset(szDecResult_F, 0, sizeof(szDecResult_F));
            memset(szDecResult_R, 0, sizeof(szDecResult_R));

            nRet = Scan_GetImages(&PicFront, &PicRear);
            if (Scan_GetDecodeLen(&nDecodeLen_F, &nDecodeLen_R))
            {
                if (Scan_GetDecode(szDecResult_F, &nDecodeType_F, &nDecodeLen_F, szDecResult_R, &nDecodeType_R, &nDecodeLen_R))
                {
                    if (nDecodeLen_F)
                    {
                        szDecResult_F[nDecodeLen_F] = 0;
                        stImageOut.ulDataSize = nDecodeLen_F;
                        stImageOut.lpData = new CHAR[nDecodeLen_F + 1];
                        memcpy(stImageOut.lpData, szDecResult_F, nDecodeLen_F);
                        stImageOut.wInMode |= IMAGE_MODE_CODELINE;
                    }
                    else if (nDecodeLen_R)
                    {
                        szDecResult_R[nDecodeLen_R] = 0;
                        stImageOut.ulDataSize = nDecodeLen_R;
                        stImageOut.lpData = new CHAR[nDecodeLen_R + 1];
                        memcpy(stImageOut.lpData, szDecResult_R, nDecodeLen_R);
                    }
                }
            }
        }

        nRet = Scan_SetPaperOut(1, 0);
        return IMP_SUCCESS;
    }
}

//----------------------------------对外参数设置接口----------------------------------
INT CDevImpl_BSD216::SetReConFlag(BOOL bFlag)
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
void CDevImpl_BSD216::SetLibPath(LPCSTR lpPath)
{
    THISMODULE(__FUNCTION__);    

    // 设定动态库路径
    if (lpPath == nullptr || strlen(lpPath) < 1 ||
        (strlen(lpPath) == 1 && (char*)lpPath[0] == "/"))
    {
        Log(ThisModule, __LINE__, "入参[%s]无效,不设置动态库路径.");
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

//----------------------------------内部使用方法----------------------------------
INT CDevImpl_BSD216::ConvertErrorCode(INT nRet)
{
    return nRet;
}

CHAR* CDevImpl_BSD216::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nRet)
    {
        case SNBC_ERROR_NONE:
            sprintf(m_szErrStr, "%d|%s", nRet, "成功/状态正常");
            return m_szErrStr;
        case SNBC_FILE_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "错误");
            return m_szErrStr;
        case SNBC_COMMUNICATION_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "通信错");
            return m_szErrStr;
        case SNBC_SET_SCAN_MODE_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "设置扫描仪失败");
            return m_szErrStr;
        case SNBC_COVER_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "上盖错");
            return m_szErrStr;
        case SNBC_PAPER_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "塞纸错");
            return m_szErrStr;
        case SNBC_NO_PAPER_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "无纸");
            return m_szErrStr;
        case SNBC_DOUBLE_PAPER_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "重张错");
            return m_szErrStr;
        case SNBC_STATUS_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "错误状态");
            return m_szErrStr;
        case SNBC_ERROR_UNKNOWN:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备出错，非塞纸、上盖打开、重张错");
            return m_szErrStr;
        case SNBC_LOAD_DLL_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "加载dll失败");
            return m_szErrStr;
        case SNBC_PARA_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "参数错");
            return m_szErrStr;
        case SNBC_MEMORY_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "内存不足");
            return m_szErrStr;
        case SNBC_HARDWARE_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "硬件错误");
            return m_szErrStr;
        case SNBC_DELETECISDATA_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "删除CIS多余数据错误");
            return m_szErrStr;
        case SNBC_MIRROR_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "镜像处理错误");
            return m_szErrStr;
        case SNBC_NO_IMAGE_ERROR:
            sprintf(m_szErrStr, "%d|%s", nRet, "无图像错误");
            return m_szErrStr;
        case IMP_ERR_LOAD_LIB:
            sprintf(m_szErrStr, "%d|%s", nRet, "动态库加载失败");
            return m_szErrStr;
        case IMP_ERR_PARAM_INVALID:
            sprintf(m_szErrStr, "%d|%s", nRet, "参数无效");
            return m_szErrStr;
        case IMP_ERR_UNKNOWN:
            sprintf(m_szErrStr, "%d|%s", nRet, "未知错误");
            return m_szErrStr;
        case IMP_ERR_NOTOPEN:
            sprintf(m_szErrStr, "%d|%s", nRet, "设备未Open");
            return m_szErrStr;

        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义错误");
            return m_szErrStr;
    }
}

