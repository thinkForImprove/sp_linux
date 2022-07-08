#ifndef EPPTEST_H
#define EPPTEST_H

#include <QWidget>
#include "QtTypeDef.h"
#include "AutoQtHelpClass.h"
#include "QtDLLLoader.h"
#include "IDevPIN.h"

namespace Ui
{
class CDevPINTest;
}

class CDevPINTest : public QWidget
{
    Q_OBJECT

public:
    explicit CDevPINTest(QWidget *parent = nullptr);
    ~CDevPINTest();

private slots:
    void on_bt_Open_clicked();

    void on_bt_Close_clicked();

    void on_bt_InitEpp_clicked();

    void on_bt_Reset_clicked();

    void on_bt_SetKeyVal_clicked();

    void on_bt_GetFWVer_clicked();

    void on_bt_GetStatus_clicked();

    void on_bt_TextInput_clicked();

    void on_bt_GetKeyPress_clicked();

    void on_bt_CancelInput_clicked();

    void on_bt_PinInput_clicked();

    void on_bt_GetPinBlock_clicked();

    void on_bt_ImportKey_clicked();

    void on_bt_DataCrypt_clicked();

    void on_bt_SM4ImportKey_clicked();

    void on_bt_SM4DataCrypt_clicked();

    void on_bt_SM4MacData_clicked();

    void on_bt_SM4GetPinBlock_clicked();


private:
    Ui::CDevPINTest *ui;
private:
    CQtDLLLoader<IDevPIN> m_pDev;
};

#endif // EPPTEST_H
