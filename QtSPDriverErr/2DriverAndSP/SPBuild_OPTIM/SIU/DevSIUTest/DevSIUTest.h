#ifndef DEVSIUTEST_H
#define DEVSIUTEST_H

#include <QWidget>
#include "QtTypeDef.h"
#include "AutoQtHelpClass.h"
#include "QtDLLLoader.h"
#include "IDevSIU.h"


#define THISMODULE(X)       const char* ThisModule = X


namespace Ui
{
class CDevSIUTest;
}

class CDevSIUTest : public QWidget
{
    Q_OBJECT

public:
    explicit CDevSIUTest(QWidget *parent = nullptr);
    ~CDevSIUTest();

private slots:
    void on_pbt_Open_clicked();

    void on_pbt_Close_clicked();

    void on_pbt_Reset_clicked();

    void on_pbt_GetDevInfo_clicked();

    void on_pbt_GetStatus_clicked();

    void on_pbt_SetDoors_clicked();

    void on_pbt_SetIndicators_clicked();

    void on_pbt_SetAuxiliaries_clicked();

    void on_pbt_SetGuidLights_clicked();

    void on_pbt_GetFirmWareVer_clicked();

    void on_pbt_UpdateDevPDL_clicked();

    void on_pbt_SetGuidLights_Close_clicked();

    void on_pbtClearLog_clicked();

    void on_flicker_on_clicked();

    void on_flicker_off_clicked();

    void on_skim_on_clicked();

    void on_skim_off_clicked();

private:
    Ui::CDevSIUTest *ui;
private:
    CQtDLLLoader<IDevSIU> m_pDev;
protected:
    void AddString(LPCSTR lpFmt, ...);
};

#endif // DEVSIUTEST_H
