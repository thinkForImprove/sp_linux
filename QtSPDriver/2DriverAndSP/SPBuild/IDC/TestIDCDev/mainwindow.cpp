#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "AutoQtHelpClass.h"

const char *ThisFile = "mainwindow";
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    SetLogFile(LOGFILE, ThisFile, "TESTIDC");
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::WriteLog(QString strlog, bool bEnter)
{

    QString strLog;
    if (bEnter)
    {
        SYSTEMTIME st;
        CQtTime::GetLocalTime(st);
        strLog.sprintf("%02d:%02d:%02d.%03d: %s", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, \
                       strlog.toStdString().c_str());
    }
    else
    {
        strLog = strlog;
    }
    ui->IDC_EDIT_LOG->appendPlainText(strLog);
}

void MainWindow::WriteLog(QString strModule, int iRet)
{
    QString strLog;
    SYSTEMTIME st;
    CQtTime::GetLocalTime(st);
    strLog.sprintf("%02d:%02d:%02d.%03d: %s Execute %s", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, \
                   strModule.toStdString().c_str(), iRet < 0 ? "Failed" : "Success");
    ui->IDC_EDIT_LOG->appendPlainText(strLog);
}


long MainWindow::OnOpenDLl()
{
    const char *ThisModule = "OnOpenDLl";
    AutoLogFuncBeginEnd();

    if (m_pIDCDrv == nullptr)
    {
        if (0 != m_pIDCDrv.Load("DevIDC_EC2G", "CreateIDevIDC", "IDC"))
            //if (0 != m_pIDCDrv.Load("DevIDC_CJ201", "CreateIDevIDC", "RFID"))
        {
            return -1;
        }
    }

    if (m_pIDCDrv != nullptr)
    {
        QString strPort;
#ifdef __unix
        //strPort.sprintf("%d,%d",0x04a4, 0x00bf);
        strPort.sprintf("HID:%d,%d", 0x2D9A, 0x0101);
#else
        strPort.sprintf("%d,%d", 0x2D9A, 0x0101);
#endif
        strPort = "USB:04A4,00BF";
        long iRet = m_pIDCDrv->Open(strPort.toStdString().c_str());
        if (iRet != 0)
        {
            Log(ThisModule, 1, "Open Failure");
            WriteLog("Open Failure");
        }
        else
        {
            Log(ThisModule, 1, "Open success");
            WriteLog("Open success");
        }
        return iRet;
    }
    return -1;
}

void MainWindow::on_BUTTON_OPEN_clicked()
{
    const char *ThisModule = "OPEN";
    AutoLogFuncBeginEnd();
    Log(ThisModule, 1, "Open start ");

    ui->IDC_EDIT_LOG->document()->setMaximumBlockCount(10);


    long lRes = OnOpenDLl();
    WriteLog(ThisModule, static_cast<int>(lRes));
}

void MainWindow::on_BUTTON_CLOSE_clicked()
{
    const char *ThisModule = "CLOSE";
    if (m_pIDCDrv != nullptr)
    {
        m_pIDCDrv->Close();
        WriteLog(ThisModule, static_cast<int>(0));
    }
}

void MainWindow::on_BUTTON_GETFWVERSION_clicked()
{
    const char *ThisModule = "GETFWVERSION";
    if (m_pIDCDrv != nullptr)
    {
        char szFWVer[10][256] = {0};
        unsigned int ulen = sizeof(szFWVer);
        int iRet = m_pIDCDrv->GetFWVersion(&szFWVer[0], ulen);
        WriteLog(ThisModule, static_cast<int>(iRet));
        //WriteLog("\n");
        WriteLog(szFWVer[0], false);
    }
}

void MainWindow::on_BUTTON_INITEJECT_clicked()
{
    const char *ThisModule = "INITEJECT";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->Init(CARDACTION_EJECT);
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_INITEAT_clicked()
{
    const char *ThisModule = "INITEAT";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->Init(CARDACTION_RETRACT);
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_INITHOLD_clicked()
{
    const char *ThisModule = "INITHOLD";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->Init(CARDACTION_NOACTION);
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_DETECTCARD_clicked()
{
    const char *ThisModule = "DETECTCARD";
    if (m_pIDCDrv != nullptr)
    {
        IDC_IDCSTAUTS IDCstatus;
        int iRet = m_pIDCDrv->DetectCard(IDCstatus);
        QString strIDCStatus;
        switch (IDCstatus)
        {
        case IDCSTAUTS_NOCARD:  strIDCStatus = "无卡"; break;
        case IDCSTAUTS_ENTERING:  strIDCStatus = "门口"; break;
        case IDCSTAUTS_INTERNAL:  strIDCStatus = "内部"; break;
        case IDCSTAUTS_ICC_PRESS:  strIDCStatus = "IC卡被压下"; break;
        case IDCSTAUTS_ICC_ACTIVE:  strIDCStatus = "IC卡被激活"; break;
        case IDCSTAUTS_ICC_POWEROFF:  strIDCStatus = "IC POWEROFF"; break;
        case IDCSTATUS_UNKNOWN:  strIDCStatus = "未知状态"; break;
        default:
            break;
        }
        WriteLog(ThisModule, static_cast<int>(iRet));
        QString strlog("卡状态:");
        strlog += strIDCStatus;
        WriteLog(strlog);
    }
}

void MainWindow::on_BUTTON_ACCEPTCARD_clicked()
{
    const char *ThisModule = "ACCEPTCARD";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->AcceptCard(10 * 1000);
        if (iRet == ERR_IDC_INSERT_TIMEOUT)
            WriteLog("Insert Card Timeout");
        else if (iRet == ERR_IDC_INSERT_TIMEOUT)
            WriteLog("Insert Card Cancel");
        else
            WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_READTRACK_clicked()
{
    const char *ThisModule = "READTRACK";
    if (m_pIDCDrv != nullptr)
    {
        STTRACK_INFO stTrackInfo;
        int iRet = m_pIDCDrv->ReadTracks(stTrackInfo);
        WriteLog(ThisModule, static_cast<int>(iRet));
        if (iRet != 0)
            return;
        QString strlog;
        for (int i = 0; i < 3; i++)
        {
            QString strInfo;
            strInfo.sprintf("Track%d:%s\n", i + 1, stTrackInfo.TrackData[i].szTrack);
            strlog += strInfo;
        }
        WriteLog(strlog, false);
    }
}

void MainWindow::on_BUTTON_EJECTCARD_clicked()
{
    const char *ThisModule = "EJECTCARD";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->EjectCard();
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_RETRACTCARD_clicked()
{
    const char *ThisModule = "RETRACTCARD";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->EatCard();
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_CANCELACCEPT_clicked()
{
    const char *ThisModule = "CANCELACCEPT";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->CancelReadCard();
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_WRITECARD_clicked()
{
    const char *ThisModule = "WRITECARD";
    if (m_pIDCDrv != nullptr)
    {
        STTRACK_INFO stTrackInfo;
        int iRet = m_pIDCDrv->WriteTracks(stTrackInfo);
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_ICCACCEPT_clicked()
{
    const char *ThisModule = "ICCACCEPT";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->AcceptCard(10 * 1000, false);
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_ICCPRESS_clicked()
{
    const char *ThisModule = "ICCPRESS";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->ICCPress();
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_ICCACTIVE_clicked()
{
    const char *ThisModule = "ICCACTIVE";
    if (m_pIDCDrv != nullptr)
    {
        char szATRInfo[MAX_LEN_ATRINFO];
        unsigned int uATRLen = MAX_LEN_ATRINFO;
        int iRet = m_pIDCDrv->ICCActive(szATRInfo, uATRLen);
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_ICCDEACTIVE_clicked()
{
    const char *ThisModule = "ICCDEACTIVE";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->ICCDeActivation();
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_ICCRESETCOLD_clicked()
{
    const char *ThisModule = "ICCRESETCOLD";
    if (m_pIDCDrv != nullptr)
    {
        char szATRInfo[MAX_LEN_ATRINFO];
        uint uATRLen = MAX_LEN_ATRINFO;
        int iRet = m_pIDCDrv->ICCReset(ICCARDRESET_COLD, szATRInfo, uATRLen);
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_ICCRESETWARM_clicked()
{
    const char *ThisModule = "";
    if (m_pIDCDrv != nullptr)
    {
        char szATRInfo[MAX_LEN_ATRINFO];
        uint uATRLen = MAX_LEN_ATRINFO;
        int iRet = m_pIDCDrv->ICCReset(ICCARDRESET_WARM, szATRInfo, uATRLen);
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_ICCRELEASE_clicked()
{
    const char *ThisModule = "ICCRELEASE";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->ICCRelease();
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_ICCCHIPIOT0_clicked()
{
    const char *ThisModule = "ICCCHIPIOT0";
    if (m_pIDCDrv != nullptr)
    {
        char szInOutData[256] = {(char)0x00, (char)0x84, (char)0x00, (char)0x00, (char)0x08};
        unsigned int nInOutLen = 5;
        int iRet = m_pIDCDrv->ICCChipIO(ICCARD_PROTOCOL_T0, szInOutData, nInOutLen, sizeof(szInOutData));
        WriteLog(ThisModule, static_cast<int>(iRet));
        QString strData;
        for (int i = 0; i < nInOutLen; i++)
        {
            char szBuff[32];
            sprintf(szBuff, " %02X", (unsigned char)szInOutData[i]);
            strData += szBuff;
        }
        WriteLog(strData, false);
    }
}

void MainWindow::on_BUTTON_ICCCHIPIOT1_clicked()
{
    const char *ThisModule = "ICCCHIPIOT1";
    if (m_pIDCDrv != nullptr)
    {
        char szInOutData[256] = {0};
        unsigned int nInOutLen = static_cast<unsigned int>(strlen(szInOutData));
        int iRet = m_pIDCDrv->ICCChipIO(ICCARD_PROTOCOL_T1, szInOutData, nInOutLen, sizeof(szInOutData));
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
    static bool bOpen = true;
    int iRet = m_pIDCDrv->SetRFIDCardReaderLED(LEDTYPE_BLUE, bOpen ? LEDACTION_OPEN : LEDACTION_CLOSE);
    bOpen = !bOpen;
}

void MainWindow::on_BUTTON_OPENLED_clicked()
{
    const char *ThisModule = "SetRFIDCardReaderLED";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->SetRFIDCardReaderLED(LEDTYPE_BLUE, LEDACTION_OPEN);
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}

void MainWindow::on_BUTTON_CLOSELED_clicked()
{
    const char *ThisModule = "SetRFIDCardReaderLED";
    if (m_pIDCDrv != nullptr)
    {
        int iRet = m_pIDCDrv->SetRFIDCardReaderLED(LEDTYPE_BLUE, LEDACTION_CLOSE);
        WriteLog(ThisModule, static_cast<int>(iRet));
    }
}
