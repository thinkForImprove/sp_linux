#include "DevCRD_CRT591H.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>

// CRD 版本号
BYTE    byDevVRTU[17] = {"DevCRD00000100"};

static const char *ThisModule = "DevCRD_CRT591H.cpp";

//////////////////////////////////////////////////////////////////////////

CDevCRD_CRT591H::CDevCRD_CRT591H() : m_devCRT591H(LOG_NAME_DEVCRD)
{
    SetLogFile(LOG_NAME_DEVCRD, ThisModule);  // 设置日志文件名和错误发生的文件
}

CDevCRD_CRT591H::~CDevCRD_CRT591H()
{
    Close();
}

void CDevCRD_CRT591H::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    return;
}

// 建立CRD连接
int CDevCRD_CRT591H::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    return ERR_CRD_UNSUP_CMD;
}

// CRD初始化
int CDevCRD_CRT591H::Init(EM_CRD_MEDIA_ACT eActFlag)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    return ERR_CRD_UNSUP_CMD;
}

// 关闭CRD连接
int CDevCRD_CRT591H::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_CRD_UNSUP_CMD;
}

// 设备复位
int CDevCRD_CRT591H::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_CRD_UNSUP_CMD;
}

// 读取设备状态
int CDevCRD_CRT591H::GetDevStat(STCRDDEVSTATUS &stStat)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_CRD_UNSUP_CMD;
}

// 读取设备信息
int CDevCRD_CRT591H::GetUnitInfo(STCRDUNITINFO &stInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_CRD_UNSUP_CMD;
}

// 发卡
int CDevCRD_CRT591H::DispenseCard(const int nUnitNo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return ERR_CRD_UNSUP_CMD;
}

// 设置数据
int CDevCRD_CRT591H::SetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case DATATYPE_INIT:
        {
            break;
        }
        default:
            break;
    }

    return CRD_SUCCESS;
}

// 获取数据
int CDevCRD_CRT591H::GetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case DATATYPE_INIT:
            break;
        default:
            break;
    }

    return CRD_SUCCESS;
}

// 获取版本号(1DevCRD版本/2固件版本/3设备软件版本/4其他)
void CDevCRD_CRT591H::GetVersion(char* szVer, long lSize, ushort usType)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return;
}

INT CDevCRD_CRT591H::ConvertErrorCode(long lRet)
{
    switch(lRet)
    {
        // > 100: Impl处理返回
        case IMP_SUCCESS:                   // 成功->操作成功
            return CRD_SUCCESS;
        default:
            return ERR_CRD_OTHER;
    }
}

CHAR* CDevCRD_CRT591H::ConvertErrCodeToStr(long lRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(lRet)
    {
        case CRD_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", lRet, "操作成功");
        default:
            sprintf(m_szErrStr, "%d|%s", lRet, "其他错误/未知错误");
            return m_szErrStr;
    }
}

//////////////////////////////////////////////////////////////////////////






