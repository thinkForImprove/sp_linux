#include "DevPPR_PR9.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>

static const char *ThisModule = "DevCPR_PR9.cpp";

//////////////////////////////////////////////////////////////////////////

CDevPPR_PR9::CDevPPR_PR9() : m_devPR9(LOG_NAME_DEVCPR)
{
    SetLogFile(LOG_NAME_DEVCPR, ThisModule);  // 设置日志文件名和错误发生的文件
}

CDevPPR_PR9::~CDevPPR_PR9()
{
    Close();
}

// 释放接口
void CDevPPR_PR9::Release()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return;
}


// 打开与设备的连接
int CDevPPR_PR9::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // 建立CPR连接
    if (m_devPR9.IsDeviceOpen() != TRUE)
    {
        if ((nRet = m_devPR9.DeviceOpen((LPSTR)lpMode)) != IMP_SUCCESS)
        {
            Log(ThisModule, __LINE__, "建立CPR连接: ->DeviceOpen(%s) Fail, ErrCode=%d, Return %s.",
                lpMode, nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
            return ConvertErrorCode(nRet);
        }
    }

    m_stdOpenMode.append(lpMode);

    return PTR_SUCCESS;
}

// 关闭与设备的连接
int CDevPPR_PR9::Close()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);


    return PTR_SUCCESS;
}

// 设备初始化
int CDevPPR_PR9::Init()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);    

    return PTR_SUCCESS;
}


// 设备复位
int CDevPPR_PR9::Reset()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    if ((nRet = m_devPR9.nResetDev()) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "CPR设备初始化: ->nResetDev() Fail, ErrCode=%d, Return %s.",
            nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    if ((nRet = m_devPR9.DeviceOpen((LPSTR)m_stdOpenMode.c_str())) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "CPR设备初始化后重新建立连接: ->DeviceOpen(%s) Fail, ErrCode=%d, Return %s.",
            m_stdOpenMode.c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

    return PTR_SUCCESS;
}

// 取设备状态
int CDevPPR_PR9::GetStatus(DEVPTRSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;
    INT    nDevStat[12];



    return PTR_SUCCESS;
}

// 打印字串(无指定打印坐标)
int CDevPPR_PR9::PrintData(const char *pStr, unsigned long ulDataLen)
{

    return PTR_SUCCESS;
}

int CDevPPR_PR9::PrintDataOrg(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);


    return PTR_SUCCESS;
}

// 图片打印(无指定打印坐标)
int CDevPPR_PR9::PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;


    return PTR_SUCCESS;
}

// 指定坐标打印图片
int CDevPPR_PR9::PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;


    return PTR_SUCCESS;
}

// 扫描信息获取
int CDevPPR_PR9::ReadForm(DEVPTRREADFORMIN stScanIn, DEVPTRREADFORMOUT &stScanOut)
{


    return PTR_SUCCESS;
}

// 扫描信息获取
int CDevPPR_PR9::ReadImage(DEVPTRREADIMAGEIN stScanIn, DEVPTRREADIMAGEOUT &stScanOut)
{


    return PTR_SUCCESS;
}

// 介质控制
int CDevPPR_PR9::MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);



    return PTR_SUCCESS;
}

// 设置数据
int CDevPPR_PR9::SetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case 0:
        {
            break;
        }
        default:
            break;
    }

    return PTR_SUCCESS;
}

// 获取数据
int CDevPPR_PR9::GetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case 0:
            break;
        default:
            break;
    }

    return PTR_SUCCESS;
}

// 获取版本号(1DevCPR版本/2固件版本/3设备软件版本/4其他)
void CDevPPR_PR9::GetVersion(char* szVer, long lSize, ushort usType)
{
    CHAR    szVersion[128] = { 0x00 };

    if (usType == 1)
    {
        //memcpy(szVersion, byDevVRTU, strlen((char*)byDevVRTU));
    } else
    if (usType == 2)
    {
        if (m_devPR9.IsDeviceOpen() == TRUE)
        {
            if (m_devPR9.nGetFirmwareVersion(szVersion, lSize) != IMP_SUCCESS)
            {
                return;
            }
        }return;
    } else
    if (usType == 3)
    {
        return;
    } else
    if (usType == 4)
    {
        return;
    }

    memcpy(szVer, szVersion, strlen((char*)szVersion) > lSize ? lSize : strlen((char*)szVersion));
}

INT CDevPPR_PR9::ConvertErrorCode(INT nRet)
{
    switch(nRet)
    {
        // > 100: Impl处理返回
        case IMP_SUCCESS:                   // 成功->操作成功
            return PTR_SUCCESS;
        default:
            return ERR_PPR_OTHER;
    }
}

CHAR* CDevPPR_PR9::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(nRet)
    {
        case PTR_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "操作成功");
            return m_szErrStr;

        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "其他错误/未知错误");
            return m_szErrStr;
    }
}

//////////////////////////////////////////////////////////////////////////






