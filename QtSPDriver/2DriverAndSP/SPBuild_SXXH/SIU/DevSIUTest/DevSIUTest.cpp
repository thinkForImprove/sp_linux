#include "DevSIUTest.h"
#include "ui_DevSIUTest.h"

#include "qstringlistmodel.h"
#include "qstandarditemmodel.h"


CDevSIUTest::CDevSIUTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CDevSIUTest)
{
    ui->setupUi(this);
    //ui->listWidget->addItem("123\n456");
}

CDevSIUTest::~CDevSIUTest()
{
    delete ui;
}

void CDevSIUTest::on_pbt_Open_clicked()
{
    THISMODULE(__FUNCTION__);
    if (m_pDev == nullptr)
    {
        m_pDev.Load("DevSIU_IOMC.dll", "CreateIDevSIU", "SIU");
    }
    if (m_pDev == nullptr)
        return;

    long lRet = m_pDev->Open("");
    AddString("%s lRet = %d", ThisModule, lRet);
}

void CDevSIUTest::on_pbt_Close_clicked()
{
    THISMODULE(__FUNCTION__);
    long lRet = m_pDev->Close();
    AddString("%s lRet = %d", ThisModule, lRet);
}

void CDevSIUTest::on_pbt_Reset_clicked()
{
    THISMODULE(__FUNCTION__);
    long lRet = m_pDev->Reset();
    AddString("%s lRet = %d", ThisModule, lRet);
}

void CDevSIUTest::on_pbt_GetDevInfo_clicked()
{
    THISMODULE(__FUNCTION__);
    QByteArray strBuff(256, 0);
    long lRet = m_pDev->GetDevInfo(strBuff.data());
    AddString("%s lRet = %d\n[%s]", ThisModule, lRet, strBuff.constData());
}

void CDevSIUTest::on_pbt_GetStatus_clicked()
{
    THISMODULE(__FUNCTION__);

    QString strLog;
    QString strSensors;
    QString strDoors;
    QString strLights;
    QString strAuxils;
    QString strIndias;
    DEVSIUSTATUS stStatus;
    long lRet = m_pDev->GetStatus(stStatus);
    if (lRet == 0)
    {
        for (int i = 0; i < DEFSIZE; i++)
        {
            strSensors += strLog.sprintf("[%d]", stStatus.wSensors[i]);
            strDoors += strLog.sprintf("[%d]", stStatus.wDoors[i]);
            strLights += strLog.sprintf("[%d]", stStatus.wGuidLights[i]);
            strAuxils += strLog.sprintf("[%d]", stStatus.wAuxiliaries[i]);
            strIndias += strLog.sprintf("[%d]", stStatus.wIndicators[i]);
        }

        strLog.clear();;
        strLog.push_back("strSensors=");
        strLog += strSensors;
        strLog.push_back("\nstrDoors=");
        strLog += strDoors;
        strLog.push_back("\nstrLights=");
        strLog += strLights;
        strLog.push_back("\nstrAuxils=");
        strLog += strAuxils;
        strLog.push_back("\nstrIndias=");
        strLog += strIndias;
    }

    AddString("%s lRet = %d\n%s", ThisModule, lRet, strLog.toLocal8Bit().constData());

}

void CDevSIUTest::on_pbt_SetDoors_clicked()
{

}

void CDevSIUTest::on_pbt_SetIndicators_clicked()
{

}

void CDevSIUTest::on_pbt_SetAuxiliaries_clicked()
{

}

void CDevSIUTest::on_pbt_SetGuidLights_clicked()
{
    THISMODULE(__FUNCTION__);
    WORD wGuidLights[DEFSIZE];
    memset(wGuidLights, 0x00, sizeof(wGuidLights));
    wGuidLights[0] = GUIDLIGHT_QUICK_FLASH;
    wGuidLights[1] = GUIDLIGHT_QUICK_FLASH;
    wGuidLights[2] = GUIDLIGHT_QUICK_FLASH;
    wGuidLights[4] = GUIDLIGHT_QUICK_FLASH;
    for (int i = 0; i < DEFSIZE; i++)
        wGuidLights[i] = GUIDLIGHT_QUICK_FLASH;
    long lRet = m_pDev->SetGuidLights(wGuidLights);
    AddString("%s lRet = %d", ThisModule, lRet);
}


void CDevSIUTest::on_pbt_SetGuidLights_Close_clicked()
{
    THISMODULE(__FUNCTION__);
    WORD wGuidLights[DEFSIZE];
    memset(wGuidLights, 0x00, sizeof(wGuidLights));
    wGuidLights[0] = GUIDLIGHT_OFF;
    wGuidLights[1] = GUIDLIGHT_OFF;
    wGuidLights[2] = GUIDLIGHT_OFF;
    wGuidLights[4] = GUIDLIGHT_OFF;
    for (int i = 0; i < DEFSIZE; i++)
        wGuidLights[i] = GUIDLIGHT_OFF;
    long lRet = m_pDev->SetGuidLights(wGuidLights);
    AddString("%s lRet = %d", ThisModule, lRet);
}

void CDevSIUTest::AddString(LPCSTR lpFmt, ...)
{
    //  格式化日志
    char szBuff[8192] = { 0 };
    va_list vl;
    va_start(vl, lpFmt);
    vsprintf(szBuff, lpFmt, vl);
    va_end(vl);

    ui->listWidget->addItem(szBuff);
}


void CDevSIUTest::on_pbt_GetFirmWareVer_clicked()
{
    THISMODULE(__FUNCTION__);
    QByteArray strBuff(256, 0);
    long lRet = m_pDev->GetFirmWareVer(strBuff.data());
    AddString("%s lRet = %d\n[%s]", ThisModule, lRet, strBuff.constData());
}

void CDevSIUTest::on_pbt_UpdateDevPDL_clicked()
{

}

void CDevSIUTest::on_pbtClearLog_clicked()
{
    int nCount = ui->listWidget->count();
    for (int i = nCount - 1; i >= 0; i--)
    {
        ui->listWidget->takeItem(i);
    }
}
