#include "DevVdm.h"

static const char *ThisFile = "DevVdm.cpp";
CSimpleMutex m_cMutex;
//////////////////////////////////////////////////////////////////////////////
extern "C" DEVVDM_EXPORT long CreateIDevVDM(LPCSTR lpDevType, IDevVDM *&pDev)
{
    pDev = new CDevVDM(lpDevType);
    return (pDev != nullptr) ? 0 : -1;
}


CDevVDM::CDevVDM(LPCSTR lpDevType)
{
    SetLogFile(LOGFILE, ThisFile, lpDevType);

    memset(&m_stDevVdmStatus, 0, sizeof(m_stDevVdmStatus));
}

CDevVDM::~CDevVDM()
{

}

void CDevVDM::Release()
{
    return;
}

long CDevVDM::Open()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_stDevVdmStatus.wDevice = DEVICE_ONLINE;
    memcpy(m_stDevVdmStatus.szErrCode, "000", 3);

    //读取配置文件
    ReadConfig();

    m_vdmPageProcess.setProgram(m_strProgram.c_str());
    if(!m_strArgument.empty()){
        QStringList argument;
        CAutoSplitByStep autoSplitByStep(m_strArgument.c_str(), " ");
        int iCount = autoSplitByStep.Count();
        for(int i = 0; i < iCount; i++){
            argument.append(autoSplitByStep.At(i));
        }
        m_vdmPageProcess.setArguments(argument);
    }

    Log(ThisModule, __LINE__, "Program:%s argument:%s", m_strProgram.c_str(), m_strArgument.c_str());

    return ERR_VDM_SUCCESS;
}

long CDevVDM::Close()
{
    return ERR_VDM_SUCCESS;
}

long CDevVDM::Reset()
{
    return ERR_VDM_SUCCESS;
}

long CDevVDM::GetDevInfo(char *pInfo)
{
    return ERR_VDM_SUCCESS;
}

long CDevVDM::GetStatus(DEVVDMSTATUS &stStatus)
{
    AutoMutex(m_cMutex);
    memcpy(&stStatus, &m_stDevVdmStatus, sizeof(m_stDevVdmStatus));
    return ERR_VDM_SUCCESS;
}

long CDevVDM::EnterVDMReq()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //关闭页面进程
    if(m_vdmPageProcess.state() != QProcess::NotRunning){
        m_vdmPageProcess.kill();
    }
    return ERR_VDM_SUCCESS;
}

long CDevVDM::EnterVDMAck()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //启动页面进程
    if(!m_strProgram.empty()){
        m_vdmPageProcess.start();
        Log(ThisModule, __LINE__, "页面进程启动:PID(%ld)", m_vdmPageProcess.processId());
    }

    return ERR_VDM_SUCCESS;
}

long CDevVDM::ExitVDMReq()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return ERR_VDM_SUCCESS;
}

long CDevVDM::ExitVDMAck()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_vdmPageProcess.kill();

    //等待进程结束
    m_vdmPageProcess.waitForFinished(3000);
    return ERR_VDM_SUCCESS;
}

void CDevVDM::ReadConfig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    QString strIniFilePath = "/VDMConfig.ini";

    strIniFilePath.prepend(SPETCPATH);

    CINIFileReader iniFile(strIniFilePath.toStdString());

    CINIReader iniReader = iniFile.GetReaderSection("VDMInfo");

    m_strProgram = (LPCSTR)iniReader.GetValue("Program", "");
    m_strArgument = (LPCSTR)iniReader.GetValue("ProgramArgument", "");

    return;
}
