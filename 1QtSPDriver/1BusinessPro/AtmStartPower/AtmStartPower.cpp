#include "AtmStartPower.h"
#include "ui_AtmStartPower.h"

static LPCSTR ThisFile = "AtmStartPower.cpp";
//////////////////////////////////////////////////////////////////////////
BYTE Hex2BCD(BYTE byData)
{
    return ((byData / 10) << 4) + (byData % 10);
}
BYTE BCD2Hex(BYTE byData)
{
    return byData - (byData >> 4) * 6;
}

//////////////////////////////////////////////////////////////////////////
CAtmStartPower::CAtmStartPower(QWidget *parent) : QWidget(parent), ui(new Ui::CAtmStart), m_cDev("IOMC"), m_cDevPW("IOMC_PW")
{
    SetLogFile(LOGFILE, ThisFile, "IOMC");
    // 托盘图标
    m_sysTray.setIcon(QIcon(":/Image/res/iconStop.png"));
    m_sysTray.setToolTip("AtmStartPower");

    //托盘菜单项
    QMenu *qMenu = new QMenu(this);
    QAction *qAction = new QAction("Exit AtmStartPower", qMenu);
    qMenu->addAction(qAction);
    m_sysTray.setContextMenu(qMenu);

    //托盘菜单响应
    connect(qAction, SIGNAL(triggered()), this, SLOT(OnExit()));

    //显示托盘
    m_sysTray.show();

    // 启动线程
    ThreadStart();

    ui->setupUi(this);
}

CAtmStartPower::~CAtmStartPower()
{
    delete ui;
}

void CAtmStartPower::OnExit()
{
    ThreadStop();
    this->close();
}
//////////////////////////////////////////////////////////////////////////
void CAtmStartPower::Run()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    // 打开电源，并监测心跳，防止断电
    // 打开电源成功，则修改图标为绿色
    QIcon cIconGo(":/Image/res/iconGo.png");
    QIcon cIconGoGo(":/Image/res/iconGoGo.png");
    QIcon cIconStop(":/Image/res/iconStop.png");
    QIcon *pIcon = &cIconStop;
    do
    {
        // 初始化
        if (0 != ReadConfig())
            break;
        if (0 != Open())
            break;
        if (0 != GetFWVer())
            break;
        if (0 != Reset())
            break;
        if (0 != ClearPowOffFlag())
            break;
        if (0 != DataWrite(POWER_CUTDOWN))
            break;
        if (0 != BackLightOn())
            break;
        if (0 != StopPowerScan())
            break;
        if (0 != DataWrite(DATETIME_NOW))
            break;
        if (0 != SetPowerScanTime())
            break;
        if (0 != PowerOnOff(false))
            break;
        if (0 != PowerOnOff(true))
            break;
        if (0 != PowerOnAutoControl())
            break;

        // 初始化成功，显示绿色图标
        m_sysTray.setIcon(cIconGo);
        // 开始监测心跳和电源状态
        bool bIconGo = false;
        uint uTryTime = 0;
        while (!m_bQuitRun)
        {
            do
            {
                pIcon = &cIconStop;
                if (0 != PowerOnScanRefresh())
                {
                    if (++uTryTime <= 3)
                    {
                        Close();
                        Open();
                    }
                    break;
                }
                if (0 != CheckScanTimeoutFlag())
                    break;

                uTryTime = 0;
                bIconGo = !bIconGo;
                pIcon = bIconGo ? &cIconGo : &cIconGoGo;
            } while (false);
            // 更换图标
            m_sysTray.setIcon(*pIcon);
            QtSleep(10*1000);
        }

        // 退出时，关闭电源
        PowerOnOff(false);
    } while (false);
    // 退出
    Close();
    return;
}

//////////////////////////////////////////////////////////////////////////

long CAtmStartPower::ReadConfig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    QByteArray strFile("AtmStartConfig.ini");
#ifdef QT_WIN32
    strFile.prepend(ETC_WINPATH);
#else
    strFile.prepend(ETC_LINUXPATH);
#endif
    if (!m_cINI.LoadINIFile(strFile.constData()))
    {
        Log(ThisModule, __LINE__, "加载配置文件失败：%s", strFile.constData());
        return -1;
    }

    CINIReader cIR = m_cINI.GetReaderSection("POWER");
    m_stPW.bAutoPowerOn = (BOOL)cIR.GetValue("AutoPowerOn", "0");
    m_stPW.bStartScanSupport = (BOOL)cIR.GetValue("StartScanSupport", "1");
    m_stPW.bAliveScanSupport = (BOOL)cIR.GetValue("AliveScanSupport", "1");
    m_stPW.bShutdownScanSupport = (BOOL)cIR.GetValue("ShutdownScanSupport", "1");
    m_stPW.bPowerOnAllDevTpye = (BOOL)cIR.GetValue("PowerOnAllDevTpye", "1");

    cIR = m_cINI.GetReaderSection("POWERTIME");
    m_stPW.wPowerOffSleepTime = (WORD)cIR.GetValue("PowerOffSleepTime", "2");
    m_stPW.wPowerONSleepTime = (WORD)cIR.GetValue("PowerONSleepTime", "200");
    m_stPW.wStartScanTime = (WORD)cIR.GetValue("StartScanTime", "60");
    m_stPW.wAliveScan1Time = (WORD)cIR.GetValue("AliveScan1Time", "60");
    m_stPW.wAliveScan2Time = (WORD)cIR.GetValue("AliveScan2Time", "60");
    m_stPW.wShutdownScanTime = (WORD)cIR.GetValue("ShutdownScanTime", "30");
    m_stPW.wForcePowerOffTime = (WORD)cIR.GetValue("ForcePowerOffTime", "30");
    return 0;
}

long CAtmStartPower::Open()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    // 加载USB库，并打开连接
    long lRet = m_cDev.USBDllLoad();
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "加载USB库失败: lRet=%d", lRet);
        return -1;
    }

    lRet = m_cDev.USBOpen(IOMC_DEVICE_NAME);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "打开连接失败: lRet=%d", lRet);
        return -1;
    }

    m_cDevPW.SetFnATMUSB(m_cDev.GetFnATMUSB(), m_cDev.GetInfATMUSB());
    // 打开电源管理
    lRet = m_cDevPW.USBOpen(PWRSPLY_DEVICE_NAME);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "打开电源管理失败: lRet=%d", lRet);
        return -1;
    }
    return 0;
}

long CAtmStartPower::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_cDev.IsOpen() && !m_cDevPW.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return -1;
    }

    long lRet = 0;
    if (m_cDev.IsOpen())
    {
        lRet = m_cDev.USBClose();
        if (lRet != 0)
            Log(ThisModule, __LINE__, "关闭连接失败: lRet=%d", lRet);
    }

    if (m_cDevPW.IsOpen())
    {
        lRet = m_cDevPW.USBClose();
        if (lRet != 0)
            Log(ThisModule, __LINE__, "关闭连接失败: lRet=%d", lRet);
    }
    return 0;
}

long CAtmStartPower::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x0800;                       // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;    // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;  // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_RESET;       // CMD

    return SendAndReadCmd(strIomc, stRespInf, ThisModule);
}

long CAtmStartPower::DataWrite(WORD wParam)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x0800;                       // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;    // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;  // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_DATA_WRITE;  // CMD

    WORD wLen = 0;
    if (wParam & DATETIME_NOW)
    {
        QDateTime qDT = QDateTime::currentDateTime();
        QDate qDate = qDT.date();
        QTime qTime = qDT.time();
        wLen = 12;
        strIomc.wDATA_ID = 0x4D07;
        strIomc.wDATA_LNG = 0x0A00;
        strIomc.byDATA[0] = Hex2BCD(LOBYTE((WORD)qDate.year() / 100));
        strIomc.byDATA[1] = Hex2BCD(LOBYTE((WORD)qDate.year() % 100));
        strIomc.byDATA[2] = Hex2BCD(LOBYTE((WORD)qDate.month()));
        strIomc.byDATA[3] = Hex2BCD(LOBYTE((WORD)qDate.day()));
        strIomc.byDATA[4] = Hex2BCD(LOBYTE((WORD)(qDate.dayOfWeek() - 1)));  // 取值：0~6
        strIomc.byDATA[5] = Hex2BCD(LOBYTE((WORD)qTime.hour()));
        strIomc.byDATA[6] = Hex2BCD(LOBYTE((WORD)qTime.minute()));
        strIomc.byDATA[7] = Hex2BCD(LOBYTE((WORD)qTime.second()));
    }
    else if (wParam & POWER_CUTDOWN)
    {
        wLen = 8;
        strIomc.wDATA_ID = 0x6007;
        strIomc.wDATA_LNG = 0x0600;
        strIomc.byDATA[0] = 0x00;
        strIomc.byDATA[1] = 0x00;
        strIomc.byDATA[2] = 0x00;
        strIomc.byDATA[3] = 0x00;
    }
    else
    {
        Log(ThisModule, __LINE__, "不支持参数：%02X", wParam);
        return -1;
    }

    wLen += HIBYTE(strIomc.wLEN);
    strIomc.wLEN = wLen << 8;
    return SendAndReadCmd(strIomc, stRespInf, ThisModule);
}

long CAtmStartPower::DataRead()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x0800;                       // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;    // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;  // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_RESET;       // CMD

    return SendAndReadCmd(strIomc, stRespInf, ThisModule);
}

long CAtmStartPower::BackLightOn()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x1400;                       // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;    // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;  // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_BKLIGHT_ON;  // CMD

    strIomc.wDATA_ID = 0x4B07;
    strIomc.wDATA_LNG = 0x0400;
    strIomc.byDATA[0] = 0x01;  // ACT　2byte
    strIomc.byDATA[1] = 0x08;  //
    strIomc.byDATA[2] = 0x07;  // ID　2byte(SPL)
    strIomc.byDATA[3] = 0x4C;  //
    strIomc.byDATA[4] = 0x00;
    strIomc.byDATA[5] = 0x04;  // LNG　2byte
    strIomc.byDATA[6] = 0x01;  // ACT　2byte
    strIomc.byDATA[7] = 0x08;  //

    return SendAndReadCmd(strIomc, stRespInf, ThisModule);
}

long CAtmStartPower::StopPowerScan()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x0800;                         // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;      // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;    // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_STOP_PW_SCAN;  // CMD

    return SendAndReadCmd(strIomc, stRespInf, ThisModule);
}

long CAtmStartPower::PowerOnOff(bool bPowerOn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = 0;
    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x1000;                       // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;    // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;  // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_PW_CONTROL;  // CMD

    strIomc.wDATA_ID = 0x5707;
    strIomc.wDATA_LNG = 0x0600;
    if (!bPowerOn)
    {
        strIomc.byDATA[0] = 0x0F;
        strIomc.byDATA[1] = 0xFF;
        strIomc.byDATA[2] = 0x00;
        strIomc.byDATA[3] = 0x00;
        lRet = SendAndReadCmd(strIomc, stRespInf, ThisModule);
        if (lRet != 0)
        {
            Log(ThisModule, __LINE__, "执行六个通道下电失败");
            return lRet;
        }

        Log(ThisModule, 1, "执行六个通道下电命令成功，延时：%d 秒，确保下电真正执行", m_stPW.wPowerOffSleepTime);
        QtSleep(m_stPW.wPowerOffSleepTime * 1000);
    }
    else
    {
        if (!m_stPW.bPowerOnAllDevTpye)
        {
            strIomc.byDATA[0] = 0x05;
            strIomc.byDATA[1] = 0x55;
            strIomc.byDATA[2] = 0x00;
            strIomc.byDATA[3] = 0x00;
            lRet = SendAndReadCmd(strIomc, stRespInf, ThisModule);
            if (lRet != 0)
            {
                Log(ThisModule, __LINE__, "执行六个通道一起上电失败");
                return lRet;
            }

            Log(ThisModule, 1, "执行六个通道一起上电命令成功");
        }
        else
        {
            // 打开电源，六个通道分别打开
            uchar szPwOn[8] = {0x04, 0x01, 0x40, 0x10, 0x04, 0x01};
            for (int i = 0; i < 6; i++)
            {
                memset(strIomc.byDATA, 0x00, 4);
                if (i == 0 || i == 1)
                    strIomc.byDATA[0] = szPwOn[i];
                else
                    strIomc.byDATA[1] = szPwOn[i];

                lRet = SendAndReadCmd(strIomc, stRespInf, ThisModule);
                if (lRet != 0)
                {
                    Log(ThisModule, __LINE__, "执行：通道[%d]上电失败", i + 1);
                    return lRet;
                }

                Log(ThisModule, 1, "执行：通道[%d]上电成功，延时：%d 毫秒", i + 1, m_stPW.wPowerONSleepTime);
                QtSleep(m_stPW.wPowerONSleepTime);
            }
        }
    }

    return lRet;
}

long CAtmStartPower::SetPowerScanTime()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // cmd param
    strIomc.wLEN = 0x1400;                          // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;       // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;     // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_SCAN_TIME_SET;  // CMD

    strIomc.wDATA_ID = 0x0107;
    strIomc.wDATA_LNG = 0x0A00;
    strIomc.byDATA[0] = 0x00;  // 打开监控
    if (m_stPW.bStartScanSupport)
        strIomc.byDATA[0] |= START_SCAN_TIME;
    if (m_stPW.bAliveScanSupport)
    {
        strIomc.byDATA[0] |= ALIVE_SCAN_TIME;
        strIomc.byDATA[0] |= FORCE_POWEROFF_TIME;
    }
    if (m_stPW.bShutdownScanSupport)
        strIomc.byDATA[0] |= SHUTDOWN_SCAN_TIME;

    // 监控时间设定
    strIomc.byDATA[1] = LOBYTE(m_stPW.wStartScanTime);
    strIomc.byDATA[2] = LOBYTE(m_stPW.wAliveScan1Time);
    strIomc.byDATA[3] = LOBYTE(m_stPW.wAliveScan2Time);
    strIomc.byDATA[4] = LOBYTE(m_stPW.wShutdownScanTime);
    strIomc.byDATA[5] = 0x00;
    strIomc.byDATA[6] = LOBYTE(m_stPW.wForcePowerOffTime);
    strIomc.byDATA[7] = 0x00;

    return SendAndReadCmd(strIomc, stRespInf, ThisModule);
}

long CAtmStartPower::PowerOnAutoControl()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x1400;                        // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;     // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;   // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_AUTOPOWERON;  // CMD

    strIomc.wDATA_ID = 0x5F07;
    strIomc.wDATA_LNG = 0x0A00;
    strIomc.byDATA[0] = 0x0F;
    strIomc.byDATA[1] = 0xFF;
    if (m_stPW.bAutoPowerOn)  // IOMC自动上电
    {
        strIomc.byDATA[0] = 0x05;
        strIomc.byDATA[1] = 0x55;
    }
    strIomc.byDATA[4] = 0x05;
    strIomc.byDATA[5] = 0x01;
    strIomc.byDATA[6] = 0x00;
    strIomc.byDATA[7] = 0x00;

    return SendAndReadCmd(strIomc, stRespInf, ThisModule);
}

long CAtmStartPower::PowerOnScanRefresh()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x0800;                         // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;      // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;    // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_SCAN_REFRESH;  // CMD

    return SendAndReadCmd(strIomc, stRespInf, ThisModule);
}

long CAtmStartPower::CheckScanTimeoutFlag()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x0800;                         // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;      // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;    // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_PW_SCAN_FLAG;  // CMD

    return SendAndReadCmd(strIomc, stRespInf, ThisModule);
}

long CAtmStartPower::ClearPowOffFlag()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    STR_IOMC strIomc;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stRespInf, 0x00, sizeof(stRespInf));

    // Sense Status
    strIomc.wLEN = 0x0800;                          // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;       // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;     // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_PW_CLEAR_FLAG;  // CMD

    return SendAndReadCmd(strIomc, stRespInf, ThisModule);
}

long CAtmStartPower::GetFWVer()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    DRV_VERtag stDrvVer;
    IOMC_VERtag stIomcVer;
    long lRet = GetDrvVer(stDrvVer);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "读取驱动版本失败：lRet = %d", lRet);
        return lRet;
    }
    lRet = GetIOMCVer(stIomcVer);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "读取固件版本失败：lRet = %d", lRet);
        return lRet;
    }

    char szBuff[64] = {0};
    char szFwVer[256] = {0};
    for (ushort i = 0; i < stDrvVer.wType; i++)
    {
        memcpy(szBuff, &stDrvVer.VerDt[i], sizeof(VERtag));
        sprintf(szFwVer + strlen(szFwVer), "DrvVer[%d]=%s|", i, szBuff);
    }
    for (ushort k = 0; k < stIomcVer.wType; k++)
    {
        memcpy(szBuff, &stIomcVer.VerDt[k], sizeof(VERtag));
        sprintf(szFwVer + strlen(szFwVer), "IomcVer[%d]=%s|", k, szBuff);
    }

    Log(ThisModule, 1, "IOMC版本信息：%s", szFwVer);
    return 0;
}

long CAtmStartPower::GetDrvVer(DRV_VERtag &stDrvVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return -1;
    }

    // 获取版本信息
    STR_DRV strDrv;
    BYTE byVRTBuff[64] = {0};
    memset(&strDrv, 0x00, sizeof(STR_DRV));

    // USB driver parameter
    strDrv.usParam = USB_PRM_VRT;               // Param: usually
    strDrv.pvDataOutBuffPtr = byVRTBuff;        // Output data pointer
    strDrv.uiDataOutReqSz = sizeof(byVRTBuff);  // Output data area size
    long lRet = m_cDev.USBDrvCall(USB_DRV_INF_INFGET, &strDrv);
    if (lRet != 0)
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d", lRet);
        return -2;
    }

    memset(&stDrvVer, 0x00, sizeof(DRV_VERtag));
    VERtag *lpVer = (VERtag *)&byVRTBuff[0];
    while (lpVer->NAME[0] != NULL)
    {
        memcpy(stDrvVer.VerDt[stDrvVer.wType++].NAME, lpVer, sizeof(VERtag));
        if (stDrvVer.wType >= sizeof(stDrvVer.VerDt) / sizeof(stDrvVer.VerDt[0]))
            break;
        lpVer++;
    }
    return 0;
}

long CAtmStartPower::GetIOMCVer(IOMC_VERtag &stIOMCVer)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 获取版本信息
    STR_IOMC strIomc;
    IOMCVrtInfT stVerInfo;
    IOMCRespInfT stRespInf;
    memset(&strIomc, 0x00, sizeof(STR_IOMC));
    memset(&stVerInfo, 0x00, sizeof(IOMCVrtInfT));
    memset(&stRespInf, 0x00, sizeof(IOMCRespInfT));

    // Sense Status
    strIomc.wLEN = 0x0800;                          // LEN
    strIomc.wCNTL_ID = EN_IOMC_CMDID_CMDCODE;       // CNTL ID  : Command
    strIomc.wCNTL_LNG = EN_IOMC_PKTLNG_COMMAND;     // LNG
    strIomc.wCNTL_CMD = EN_IOMC_CMD_VERSION_SENSE;  // CMD

    long lRet = SendAndReadCmd(strIomc, stRespInf, ThisModule);
    if (lRet != 0)
        return lRet;

    memset(&stIOMCVer, 0x00, sizeof(IOMC_VERtag));
    memcpy(&stVerInfo, &stRespInf, sizeof(IOMCVrtInfT));
    ushort uVerNum = sizeof(stVerInfo.VRT) / sizeof(stVerInfo.VRT[0]);
    for (ushort i = 0; i < uVerNum; i++)
    {
        memcpy(stIOMCVer.VerDt[i].NAME, &stVerInfo.VRT[i].cKatashiki[8], 7);
        memcpy(stIOMCVer.VerDt[i].byV, &stVerInfo.VRT[i].cVER[2], 6);
        stIOMCVer.wType++;
    }

    return 0;
}

void CAtmStartPower::QtSleep(ulong ulTimeOut)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ulTimeOut));
}

long CAtmStartPower::SendAndReadCmd(const STR_IOMC &strIomc, IOMCRespInfT &stRespInf, const char *ThisModule)
{
    // THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    if (!m_cDev.IsOpen())
    {
        Log(ThisModule, __LINE__, "没有打开连接");
        return -1;
    }

    STR_DRV strDrv;
    STR_IOMC stData;
    memset(&strDrv, 0x00, sizeof(STR_DRV));
    memcpy(&stData, &strIomc, sizeof(STR_IOMC));

    // USB driver parameter
    strDrv.usParam = USB_PRM_USUALLY;              // Param: usually
    strDrv.uiDataInBuffSz = HIBYTE(stData.wLEN);   // Input data size
    strDrv.pvDataInBuffPtr = &stData;              // Input data pointer
    strDrv.pvDataOutBuffPtr = &stRespInf;          // Output data pointer
    strDrv.uiDataOutReqSz = sizeof(IOMCRespInfT);  // Output data area size
    strDrv.uiTimer = EN_IOMCTOUT_DATA_READ;        // Timeout

    CUSBDrive *pDev = &m_cDev;
    switch (stData.wCNTL_CMD)
    {
    case EN_IOMC_CMD_AUTOPOWERON:
    case EN_IOMC_CMD_PW_CONTROL:
        pDev = &m_cDevPW;
        break;
    default:
        break;
    }

    long lRet = pDev->USBDrvCall(USB_DRV_FN_DATASENDRCV, &strDrv);
    if (lRet != 0 || stRespInf.wRESP_RES != LOBYTE(strIomc.wCNTL_CMD))
    {
        Log(ThisModule, __LINE__, "执行命令失败：lRet=%d[%02X]", lRet, stRespInf.wRESP_RES);
        return -2;
    }
    return 0;
}
