/********************************************************************************
** Form generated from reading UI file 'DevPINTest.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEVPINTEST_H
#define UI_DEVPINTEST_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CDevPINTest
{
public:
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QPushButton *bt_InitEpp;
    QPushButton *bt_Close;
    QPushButton *bt_Reset;
    QPushButton *bt_CancelInput;
    QPushButton *bt_GetStatus;
    QPushButton *bt_SetKeyVal;
    QPushButton *bt_PinInput;
    QPushButton *bt_GetKeyPress;
    QPushButton *bt_Open;
    QPushButton *bt_TextInput;
    QPushButton *bt_GetPinBlock;
    QPushButton *bt_GetFWVer;
    QPushButton *bt_ImportKey;
    QPushButton *bt_DataCrypt;
    QPushButton *bt_SM4ImportKey;
    QPushButton *bt_SM4DataCrypt;
    QPushButton *bt_SM4MacData;
    QPushButton *bt_SM4GetPinBlock;

    void setupUi(QWidget *CDevPINTest)
    {
        if (CDevPINTest->objectName().isEmpty())
            CDevPINTest->setObjectName(QStringLiteral("CDevPINTest"));
        CDevPINTest->resize(513, 488);
        layoutWidget = new QWidget(CDevPINTest);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(70, 30, 371, 421));
        gridLayout = new QGridLayout(layoutWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        bt_InitEpp = new QPushButton(layoutWidget);
        bt_InitEpp->setObjectName(QStringLiteral("bt_InitEpp"));

        gridLayout->addWidget(bt_InitEpp, 2, 0, 1, 1);

        bt_Close = new QPushButton(layoutWidget);
        bt_Close->setObjectName(QStringLiteral("bt_Close"));

        gridLayout->addWidget(bt_Close, 0, 1, 1, 1);

        bt_Reset = new QPushButton(layoutWidget);
        bt_Reset->setObjectName(QStringLiteral("bt_Reset"));

        gridLayout->addWidget(bt_Reset, 4, 0, 1, 1);

        bt_CancelInput = new QPushButton(layoutWidget);
        bt_CancelInput->setObjectName(QStringLiteral("bt_CancelInput"));

        gridLayout->addWidget(bt_CancelInput, 10, 0, 1, 1);

        bt_GetStatus = new QPushButton(layoutWidget);
        bt_GetStatus->setObjectName(QStringLiteral("bt_GetStatus"));

        gridLayout->addWidget(bt_GetStatus, 7, 0, 1, 1);

        bt_SetKeyVal = new QPushButton(layoutWidget);
        bt_SetKeyVal->setObjectName(QStringLiteral("bt_SetKeyVal"));

        gridLayout->addWidget(bt_SetKeyVal, 5, 0, 1, 1);

        bt_PinInput = new QPushButton(layoutWidget);
        bt_PinInput->setObjectName(QStringLiteral("bt_PinInput"));

        gridLayout->addWidget(bt_PinInput, 11, 0, 1, 1);

        bt_GetKeyPress = new QPushButton(layoutWidget);
        bt_GetKeyPress->setObjectName(QStringLiteral("bt_GetKeyPress"));

        gridLayout->addWidget(bt_GetKeyPress, 9, 0, 1, 1);

        bt_Open = new QPushButton(layoutWidget);
        bt_Open->setObjectName(QStringLiteral("bt_Open"));

        gridLayout->addWidget(bt_Open, 0, 0, 1, 1);

        bt_TextInput = new QPushButton(layoutWidget);
        bt_TextInput->setObjectName(QStringLiteral("bt_TextInput"));

        gridLayout->addWidget(bt_TextInput, 8, 0, 1, 1);

        bt_GetPinBlock = new QPushButton(layoutWidget);
        bt_GetPinBlock->setObjectName(QStringLiteral("bt_GetPinBlock"));

        gridLayout->addWidget(bt_GetPinBlock, 12, 0, 1, 1);

        bt_GetFWVer = new QPushButton(layoutWidget);
        bt_GetFWVer->setObjectName(QStringLiteral("bt_GetFWVer"));

        gridLayout->addWidget(bt_GetFWVer, 6, 0, 1, 1);

        bt_ImportKey = new QPushButton(layoutWidget);
        bt_ImportKey->setObjectName(QStringLiteral("bt_ImportKey"));

        gridLayout->addWidget(bt_ImportKey, 2, 1, 1, 1);

        bt_DataCrypt = new QPushButton(layoutWidget);
        bt_DataCrypt->setObjectName(QStringLiteral("bt_DataCrypt"));

        gridLayout->addWidget(bt_DataCrypt, 4, 1, 1, 1);

        bt_SM4ImportKey = new QPushButton(layoutWidget);
        bt_SM4ImportKey->setObjectName(QStringLiteral("bt_SM4ImportKey"));

        gridLayout->addWidget(bt_SM4ImportKey, 5, 1, 1, 1);

        bt_SM4DataCrypt = new QPushButton(layoutWidget);
        bt_SM4DataCrypt->setObjectName(QStringLiteral("bt_SM4DataCrypt"));

        gridLayout->addWidget(bt_SM4DataCrypt, 6, 1, 1, 1);

        bt_SM4MacData = new QPushButton(layoutWidget);
        bt_SM4MacData->setObjectName(QStringLiteral("bt_SM4MacData"));

        gridLayout->addWidget(bt_SM4MacData, 7, 1, 1, 1);

        bt_SM4GetPinBlock = new QPushButton(layoutWidget);
        bt_SM4GetPinBlock->setObjectName(QStringLiteral("bt_SM4GetPinBlock"));

        gridLayout->addWidget(bt_SM4GetPinBlock, 8, 1, 1, 1);


        retranslateUi(CDevPINTest);

        QMetaObject::connectSlotsByName(CDevPINTest);
    } // setupUi

    void retranslateUi(QWidget *CDevPINTest)
    {
        CDevPINTest->setWindowTitle(QApplication::translate("CDevPINTest", "CDevPINTest", Q_NULLPTR));
        bt_InitEpp->setText(QApplication::translate("CDevPINTest", "InitEpp", Q_NULLPTR));
        bt_Close->setText(QApplication::translate("CDevPINTest", "Close", Q_NULLPTR));
        bt_Reset->setText(QApplication::translate("CDevPINTest", "Reset", Q_NULLPTR));
        bt_CancelInput->setText(QApplication::translate("CDevPINTest", "CancelInput", Q_NULLPTR));
        bt_GetStatus->setText(QApplication::translate("CDevPINTest", "GetStatus", Q_NULLPTR));
        bt_SetKeyVal->setText(QApplication::translate("CDevPINTest", "SetKeyVal", Q_NULLPTR));
        bt_PinInput->setText(QApplication::translate("CDevPINTest", "PinInput", Q_NULLPTR));
        bt_GetKeyPress->setText(QApplication::translate("CDevPINTest", "GetKeyPress", Q_NULLPTR));
        bt_Open->setText(QApplication::translate("CDevPINTest", "Open", Q_NULLPTR));
        bt_TextInput->setText(QApplication::translate("CDevPINTest", "TextInput", Q_NULLPTR));
        bt_GetPinBlock->setText(QApplication::translate("CDevPINTest", "GetPinBlock", Q_NULLPTR));
        bt_GetFWVer->setText(QApplication::translate("CDevPINTest", "GetFWVer", Q_NULLPTR));
        bt_ImportKey->setText(QApplication::translate("CDevPINTest", "ImportKey", Q_NULLPTR));
        bt_DataCrypt->setText(QApplication::translate("CDevPINTest", "DataCrypt", Q_NULLPTR));
        bt_SM4ImportKey->setText(QApplication::translate("CDevPINTest", "SM4ImportKey", Q_NULLPTR));
        bt_SM4DataCrypt->setText(QApplication::translate("CDevPINTest", "SM4DataCrypt", Q_NULLPTR));
        bt_SM4MacData->setText(QApplication::translate("CDevPINTest", "SM4Macdata", Q_NULLPTR));
        bt_SM4GetPinBlock->setText(QApplication::translate("CDevPINTest", "SM4GetPinBlock", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CDevPINTest: public Ui_CDevPINTest {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVPINTEST_H
