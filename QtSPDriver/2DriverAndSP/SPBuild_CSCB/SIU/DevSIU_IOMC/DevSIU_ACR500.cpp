#include "DevSIU_ACR500.h"

static const char *ThisFile = "DevSIU_ACR500.cpp";

CDevSIU_ACR500::CDevSIU_ACR500():m_cSysMutex("SIU_ACR500")
{
    SetLogFile("ACR500.log", ThisFile, "SIU_ACR500");
}

CDevSIU_ACR500::~CDevSIU_ACR500()
{
    Close();
}

bool CDevSIU_ACR500::Open(const char *pCommParam)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(pCommParam == nullptr || strlen(pCommParam) == 0){
        return false;
    }

    if(nullptr == m_pDev){
        if(m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "SIU", "DevSIU_ACR500") != 0){
            Log(ThisModule, __LINE__, "AllDevPort.dll load fail[%s]", m_pDev.LastError().toStdString().c_str());
            return false;
        }
    }

    AutoMutex(m_cSysMutex);
    if(m_pDev->Open(pCommParam, false) != 0){
        return false;
    }

    return true;
}

bool CDevSIU_ACR500::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(m_pDev != nullptr){
        if(m_pDev->IsOpened()){
            Reset();
            m_pDev->Close();
        }
    }

    return true;
}

bool CDevSIU_ACR500::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(!IsDevReady()){
        return false;
    }

    //关闭所有灯
    for(int i = 0; i < 7; i++){
        SetGuidLight(i + 1, GUIDLIGHT_OFF);
    }

    return true;
}

bool CDevSIU_ACR500::SetGuidLight(int iLightIndex, int iLightStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if(!IsDevReady()){
        return false;
    }

    if(iLightIndex < 1 || iLightIndex > 7){
        return false;
    }

    LOGDEVACTION();
    BYTE byLightStatus = 0;
    switch(iLightStatus){
    case GUIDLIGHT_OFF:
        byLightStatus = 0;
        break;
    case GUIDLIGHT_CONTINUOUS:
        byLightStatus = 1;
        break;
    case GUIDLIGHT_SLOW_FLASH:
    case GUIDLIGHT_MEDIUM_FLASH:
    case GUIDLIGHT_QUICK_FLASH:
        byLightStatus = 4;
        break;
    default:
        return true;
    }

    BYTE byReply[25] = {0};
    DWORD dwInOutLen = sizeof(byReply);
    BYTE byCmd[6] = {0x02, 0x40, 0x40, 0x00, 0x03, 0x00};
    byCmd[2] = 0x40 + (iLightIndex - 1);
    byCmd[3] = byLightStatus;
    byCmd[5] = byCalcBCC(byCmd + 1, sizeof(byCmd) - 2);  //BCC从第二byte开始计算

    //进程间互斥
    AutoMutex(m_cSysMutex);
    long lRet = SendAndReadCmd(byCmd, sizeof(byCmd), byReply, dwInOutLen);
    if(lRet != 0){
        return false;
    }

    if(byReply[1] != ACR500_ACK){
        Log(ThisModule, __LINE__, "设置灯命令执行失败[%02X]", byReply[1]);
        return false;
    }

    return true;
}

bool CDevSIU_ACR500::GetGuidLightStatus(unsigned char byLightStatus[])
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();


    return true;
}

BYTE CDevSIU_ACR500::byCalcBCC(LPBYTE lpData, int iSize)
{
    BYTE byBcc = 0x00;
    for(int i = 0; i < iSize; i++){
        byBcc ^= lpData[i];
    }

    return byBcc;
}

bool CDevSIU_ACR500::IsDevReady()
{
    if(m_pDev == nullptr ||
       m_pDev->IsOpened() == false){
        return false;
    }

    return true;
}

long CDevSIU_ACR500::SendAndReadCmd(LPBYTE lpSendData, int iSendSize, LPBYTE lpRecvData, DWORD &dwInOutData)
{
    THISMODULE(__FUNCTION__);
    if(lpSendData == nullptr || iSendSize == 0 ||
       lpRecvData == nullptr || dwInOutData == 0){
        return false;
    }

    long lRet = 0;
    lRet = m_pDev->Send((LPSTR)lpSendData, iSendSize, ACR500_TIMEOUT);
    if(lRet != iSendSize){
        Log(ThisModule, __LINE__, "数据发送失败");
        return lRet;
    }

    int iRecvDataLen = 0;
    ULONG ulStartTime = CQtTime::GetSysTick();
    while(true){
        DWORD iBuffLeftLen = dwInOutData - iRecvDataLen;
        lRet = m_pDev->Read((LPSTR)(lpRecvData + iRecvDataLen), iBuffLeftLen, 1000);
        if(lRet < 0){
            Log(ThisModule, __LINE__, "接收数据时发生错误:%d", lRet);
            break;
        }

        iRecvDataLen += iBuffLeftLen;
        if(iRecvDataLen >= ACR500_REPLY_LEN){
            break;
        }

        //判断超时
        if(CQtTime::GetSysTick() - ulStartTime >= ACR500_TIMEOUT){
            Log(ThisModule, __LINE__, "接收数据超时");
            lRet = ERR_DEVPORT_RTIMEOUT;
            break;
        }
    }

    dwInOutData = iRecvDataLen;
    return lRet;
}
