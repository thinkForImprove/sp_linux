/********************************************************************************
** Form generated from reading UI file 'DevSIUTest.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEVSIUTEST_H
#define UI_DEVSIUTEST_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CDevSIUTest
{
public:
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QPushButton *pbt_Reset;
    QPushButton *pbt_Close;
    QPushButton *pbt_SetAuxiliaries;
    QPushButton *pbt_SetIndicators;
    QPushButton *pbt_Open;
    QPushButton *pbt_GetDevInfo;
    QPushButton *pbt_SetDoors;
    QPushButton *pbt_GetStatus;
    QPushButton *pbt_SetGuidLights;
    QPushButton *pbt_UpdateDevPDL;
    QPushButton *pbt_GetFirmWareVer;
    QPushButton *pbt_SetGuidLights_Close;
    QListWidget *listWidget;
    QPushButton *pbtClearLog;

    void setupUi(QWidget *CDevSIUTest)
    {
        if (CDevSIUTest->objectName().isEmpty())
            CDevSIUTest->setObjectName(QStringLiteral("CDevSIUTest"));
        CDevSIUTest->resize(647, 525);
        gridLayoutWidget = new QWidget(CDevSIUTest);
        gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(20, 0, 601, 131));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        pbt_Reset = new QPushButton(gridLayoutWidget);
        pbt_Reset->setObjectName(QStringLiteral("pbt_Reset"));

        gridLayout->addWidget(pbt_Reset, 0, 2, 1, 1);

        pbt_Close = new QPushButton(gridLayoutWidget);
        pbt_Close->setObjectName(QStringLiteral("pbt_Close"));

        gridLayout->addWidget(pbt_Close, 0, 1, 1, 1);

        pbt_SetAuxiliaries = new QPushButton(gridLayoutWidget);
        pbt_SetAuxiliaries->setObjectName(QStringLiteral("pbt_SetAuxiliaries"));

        gridLayout->addWidget(pbt_SetAuxiliaries, 1, 3, 1, 1);

        pbt_SetIndicators = new QPushButton(gridLayoutWidget);
        pbt_SetIndicators->setObjectName(QStringLiteral("pbt_SetIndicators"));

        gridLayout->addWidget(pbt_SetIndicators, 1, 2, 1, 1);

        pbt_Open = new QPushButton(gridLayoutWidget);
        pbt_Open->setObjectName(QStringLiteral("pbt_Open"));
        pbt_Open->setMaximumSize(QSize(649, 16777215));

        gridLayout->addWidget(pbt_Open, 0, 0, 1, 1);

        pbt_GetDevInfo = new QPushButton(gridLayoutWidget);
        pbt_GetDevInfo->setObjectName(QStringLiteral("pbt_GetDevInfo"));

        gridLayout->addWidget(pbt_GetDevInfo, 0, 3, 1, 1);

        pbt_SetDoors = new QPushButton(gridLayoutWidget);
        pbt_SetDoors->setObjectName(QStringLiteral("pbt_SetDoors"));

        gridLayout->addWidget(pbt_SetDoors, 1, 1, 1, 1);

        pbt_GetStatus = new QPushButton(gridLayoutWidget);
        pbt_GetStatus->setObjectName(QStringLiteral("pbt_GetStatus"));

        gridLayout->addWidget(pbt_GetStatus, 1, 0, 1, 1);

        pbt_SetGuidLights = new QPushButton(gridLayoutWidget);
        pbt_SetGuidLights->setObjectName(QStringLiteral("pbt_SetGuidLights"));

        gridLayout->addWidget(pbt_SetGuidLights, 2, 0, 1, 1);

        pbt_UpdateDevPDL = new QPushButton(gridLayoutWidget);
        pbt_UpdateDevPDL->setObjectName(QStringLiteral("pbt_UpdateDevPDL"));

        gridLayout->addWidget(pbt_UpdateDevPDL, 2, 3, 1, 1);

        pbt_GetFirmWareVer = new QPushButton(gridLayoutWidget);
        pbt_GetFirmWareVer->setObjectName(QStringLiteral("pbt_GetFirmWareVer"));

        gridLayout->addWidget(pbt_GetFirmWareVer, 2, 2, 1, 1);

        pbt_SetGuidLights_Close = new QPushButton(gridLayoutWidget);
        pbt_SetGuidLights_Close->setObjectName(QStringLiteral("pbt_SetGuidLights_Close"));

        gridLayout->addWidget(pbt_SetGuidLights_Close, 2, 1, 1, 1);

        listWidget = new QListWidget(CDevSIUTest);
        listWidget->setObjectName(QStringLiteral("listWidget"));
        listWidget->setGeometry(QRect(0, 160, 650, 360));
        pbtClearLog = new QPushButton(CDevSIUTest);
        pbtClearLog->setObjectName(QStringLiteral("pbtClearLog"));
        pbtClearLog->setGeometry(QRect(540, 130, 80, 26));

        retranslateUi(CDevSIUTest);

        QMetaObject::connectSlotsByName(CDevSIUTest);
    } // setupUi

    void retranslateUi(QWidget *CDevSIUTest)
    {
        CDevSIUTest->setWindowTitle(QApplication::translate("CDevSIUTest", "CDevSIUTest", Q_NULLPTR));
        pbt_Reset->setText(QApplication::translate("CDevSIUTest", "Reset", Q_NULLPTR));
        pbt_Close->setText(QApplication::translate("CDevSIUTest", "Close", Q_NULLPTR));
        pbt_SetAuxiliaries->setText(QApplication::translate("CDevSIUTest", "SetAuxiliaries", Q_NULLPTR));
        pbt_SetIndicators->setText(QApplication::translate("CDevSIUTest", "SetIndicators", Q_NULLPTR));
        pbt_Open->setText(QApplication::translate("CDevSIUTest", "Open", Q_NULLPTR));
        pbt_GetDevInfo->setText(QApplication::translate("CDevSIUTest", "GetDevInfo", Q_NULLPTR));
        pbt_SetDoors->setText(QApplication::translate("CDevSIUTest", "SetDoors", Q_NULLPTR));
        pbt_GetStatus->setText(QApplication::translate("CDevSIUTest", "GetStatus", Q_NULLPTR));
        pbt_SetGuidLights->setText(QApplication::translate("CDevSIUTest", "OpenGuidLights", Q_NULLPTR));
        pbt_UpdateDevPDL->setText(QApplication::translate("CDevSIUTest", "UpdateDevPDL", Q_NULLPTR));
        pbt_GetFirmWareVer->setText(QApplication::translate("CDevSIUTest", "GetFirmWareVer", Q_NULLPTR));
        pbt_SetGuidLights_Close->setText(QApplication::translate("CDevSIUTest", "CloseGuidLights", Q_NULLPTR));
        pbtClearLog->setText(QApplication::translate("CDevSIUTest", "ClearLog", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CDevSIUTest: public Ui_CDevSIUTest {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVSIUTEST_H
