#include "DevSIU_ACR500.h"

static const char *ThisFile = "DevSIU_ACR500.cpp";
static bool g_bExitThread = false;
CQtSimpleMutexEx            g_cSysMutex("SIU_ACR500");

void *ControlACRLampFlash(void *pDev){
    CDevSIU_ACR500 *pDevACR500 = (CDevSIU_ACR500 *)pDev;
    while(true){
        for(int i = 0; i < ACR500_LED_NUM; i++){
            LPACRLEDFLASH lpLamp = &(pDevACR500->m_stAcrLedFlash[i]);
            if(lpLamp->ulOnTime > 0){                   //检查是否配置为闪烁
                ULONG curTime = CQtTime::GetSysTick();
                if(lpLamp->ulNextCheckTime == 0 || (ULONG)curTime > (ULONG)lpLamp->ulNextCheckTime){
                    //获取灯设置状态
                    if(lpLamp->ulNextCheckTime == 0){
                        lpLamp->bOnFlag = true;
                    } else {
                        lpLamp->bOnFlag = !lpLamp->bOnFlag;
                    }
                    //更新下次检测时间
                    lpLamp->ulNextCheckTime = (ULONG)curTime;
                    lpLamp->ulNextCheckTime += (ULONG)(lpLamp->bOnFlag ? lpLamp->ulOnTime : lpLamp->ulOffTime);

                    //设置灯亮灭
                    int iLightStatus = lpLamp->bOnFlag ? GUIDLIGHT_CONTINUOUS : GUIDLIGHT_OFF;
                    if(!pDevACR500->SetGuidLightOnOff(i + 1, iLightStatus)){
                        pDevACR500->Log("ControlACRLampFlash", __LINE__, "ACR500控制灯命令失败[LedIndex:%d, cmd:%s]",
                            i + 1, lpLamp->bOnFlag ? "on" : "off");
                    }
                }
            }
            CQtTime::Sleep(5);
        }

        //线程是否退出
        if(g_bExitThread){
            break;
        }
        CQtTime::Sleep(10);
    }

    return (void *)0;
}


CDevSIU_ACR500::CDevSIU_ACR500()
{
    SetLogFile("ACR500.log", ThisFile, "SIU_ACR500");
    memset(&m_stAcrLedFlash, 0, sizeof(m_stAcrLedFlash));
    m_bFlashSoftwareControl = true;
    m_ulTid = 0;
}

CDevSIU_ACR500::~CDevSIU_ACR500()
{
    Close();
}

bool CDevSIU_ACR500::Open(const char *pCommParam, bool bFlashSoftwareControl/* = true*/)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_bFlashSoftwareControl = bFlashSoftwareControl;
    if(pCommParam == nullptr || strlen(pCommParam) == 0){
        return false;
    }

    if(nullptr == m_pDev){
        if(m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "SIU", "DevSIU_ACR500") != 0){
            Log(ThisModule, __LINE__, "AllDevPort.dll load fail[%s]", m_pDev.LastError().toStdString().c_str());
            return false;
        }
    }

    //读取配置项
    ReadConfig();

    AutoMutex(g_cSysMutex);
    if(m_pDev->Open(pCommParam) != 0){
        return false;
    }
    if(m_bFlashSoftwareControl){
        g_bExitThread = false;
        long lRet = pthread_create(&m_ulTid, NULL, ControlACRLampFlash, this);
        if(lRet != 0){
            Log(ThisModule, __LINE__, "ControlACRLampFlash thread start fail.");
        }
    }

    return true;
}

bool CDevSIU_ACR500::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(m_ulTid != 0){
        g_bExitThread = true;
        pthread_join(m_ulTid, nullptr);
        m_ulTid = 0;
    }

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
    memset(&m_stAcrLedFlash, 0, sizeof(m_stAcrLedFlash));
    BYTE byLightStatus[ACR500_LED_NUM] = {0};
    if(GetGuidLightStatus(byLightStatus)){
        for(int i = 0; i < ACR500_LED_NUM; i++){
            if(byLightStatus[i] == 1){
                SetGuidLight(i + 1, GUIDLIGHT_OFF);
            }
        }
    } else {
        for(int i = 0; i < ACR500_LED_NUM; i++){
            if(!SetGuidLight(i + 1, GUIDLIGHT_OFF)){
                return false;
            }
        }
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

    if(iLightIndex < 1 || iLightIndex > ACR500_LED_NUM){
        return false;
    }

    if(m_bFlashSoftwareControl){
        memset(&m_stAcrLedFlash[iLightIndex - 1], 0, sizeof(ACRLEDFLASH));
        CQtTime::Sleep(50);                //确保闪烁线程结束控制
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
    {
        if(m_bFlashSoftwareControl){
            int iOnTime = 0;
            int iOffTime = 0;
            if(iLightStatus == GUIDLIGHT_SLOW_FLASH){
                iOnTime = m_iACRFlashSlowOnTime;
                iOffTime = m_iACRFlashSlowOffTime;
            } else if(iLightStatus == GUIDLIGHT_MEDIUM_FLASH){
                iOnTime = m_iACRFlashMediumOnTime;
                iOffTime = m_iACRFlashMediumOffTime;
            } else {
                iOnTime = m_iACRFlashQuickOnTime;
                iOffTime = m_iACRFlashQuickOffTime;
            }

            m_stAcrLedFlash[iLightIndex - 1].ulOnTime = iOnTime;
            m_stAcrLedFlash[iLightIndex - 1].ulOffTime = iOffTime;
            return true;
        } else {
            if(iLightStatus == GUIDLIGHT_SLOW_FLASH){
                byLightStatus = 4;
            } else if(iLightStatus == GUIDLIGHT_MEDIUM_FLASH){
                byLightStatus = 5;
            } else {
                byLightStatus = 6;
            }
        }
    }
        break;
    default:
        return true;
    }

    BYTE byReply[25] = {0};
    DWORD dwInOutLen = sizeof(byReply);
    BYTE byCmd[6] = {0x02, 0x40, 0x40, 0x00, 0x03, 0x00};
    byCmd[2] = 0x40 + (iLightIndex - 1);
    byCmd[3] = byLightStatus;
    byCmd[5] = CalcBCC(byCmd + 1, sizeof(byCmd) - 2);  //BCC从第二byte开始计算

    //进程间互斥
    AutoMutex(g_cSysMutex);
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

    BYTE byReply[25] = {0};
    DWORD dwInOutLen = sizeof(byReply);
    BYTE byCmd[6] = {0x02, 0x20, 0x20, 0x20, 0x03, 0x00};
    byCmd[5] = CalcBCC(byCmd + 1, sizeof(byCmd) - 2);  //BCC从第二byte开始计算

    //进程间互斥
    AutoMutex(g_cSysMutex);
    long lRet = SendAndReadCmd(byCmd, sizeof(byCmd), byReply, dwInOutLen);
    if(lRet != 0){
        return false;
    }

    if(byReply[1] != ACR500_ACK){
        Log(ThisModule, __LINE__, "获取状态命令执行失败[%02X]", byReply[1]);
        return false;
    }

    for(int i = 0; i < 7; i++){
        byLightStatus[i] = byReply[4 + i];      //0:关 1:开 2:闪烁
    }

    return true;
}

bool CDevSIU_ACR500::SetGuidLightOnOff(int iLightIndex, int iLightStatus)
{
    THISMODULE(__FUNCTION__);
    if(!IsDevReady()){
        return false;
    }

    if(iLightIndex < 1 || iLightIndex > ACR500_LED_NUM){
        return false;
    }

    BYTE byLightStatus = 0;
    switch(iLightStatus){
    case GUIDLIGHT_OFF:
        byLightStatus = 0;
        break;
    case GUIDLIGHT_CONTINUOUS:
        byLightStatus = 1;
        break;
    default:
        return false;
    }

    BYTE byReply[25] = {0};
    DWORD dwInOutLen = sizeof(byReply);
    BYTE byCmd[6] = {0x02, 0x40, 0x40, 0x00, 0x03, 0x00};
    byCmd[2] = 0x40 + (iLightIndex - 1);
    byCmd[3] = byLightStatus;
    byCmd[5] = CalcBCC(byCmd + 1, sizeof(byCmd) - 2);  //BCC从第二byte开始计算

    //进程间互斥
    AutoMutex(g_cSysMutex);
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

void CDevSIU_ACR500::ReadConfig()
{
    THISMODULE(__FUNCTION__);
    QString strINIFile("SIUConfig.ini");
#ifdef QT_WIN32
    strINIFile.prepend("C:/CFES/ETC/");
#else
    strINIFile.prepend("/usr/local/CFES/ETC/");
#endif
    CINIFileReader cIni;
    if (!cIni.LoadINIFile(strINIFile.toLocal8Bit().data()))
    {
        Log(ThisModule, __LINE__, "加载配置文件失败:%s", strINIFile.toLocal8Bit().data());
        return;
    }
    CINIReader cINIReader = cIni.GetReaderSection("SIUInfo");
    m_iACRFlashSlowOnTime = (int)cINIReader.GetValue("ACRFlashSlowOn", 1000);
    m_iACRFlashSlowOffTime = (int)cINIReader.GetValue("ACRFlashSlowOff", 200);
    m_iACRFlashMediumOnTime = (int)cINIReader.GetValue("ACRFlashMediumOn", 600);
    m_iACRFlashMediumOffTime =  (int)cINIReader.GetValue("ACRFlashMediumOff", 200);
    m_iACRFlashQuickOnTime = (int)cINIReader.GetValue("ACRFlashQuickOn", 300);
    m_iACRFlashQuickOffTime = (int)cINIReader.GetValue("ACRFlashQuickOff", 200);
}

BYTE CDevSIU_ACR500::CalcBCC(LPBYTE lpData, int iSize)
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
    AutoMutex(m_cMutex);

    if(lpSendData == nullptr || iSendSize == 0 ||
       lpRecvData == nullptr || dwInOutData == 0){
        return ERR_DEVPORT_PARAM;
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
