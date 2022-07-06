/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPushButton *BUTTON_INITEJECT;
    QPushButton *BUTTON_INITEAT;
    QPushButton *BUTTON_INITHOLD;
    QPushButton *BUTTON_DETECTCARD;
    QPushButton *BUTTON_RETRACTCARD;
    QPushButton *BUTTON_EJECTCARD;
    QPushButton *BUTTON_ACCEPTCARD;
    QPushButton *BUTTON_READTRACK;
    QPushButton *BUTTON_CANCELACCEPT;
    QPushButton *BUTTON_WRITECARD;
    QPushButton *BUTTON_ICCDEACTIVE;
    QPushButton *BUTTON_ICCPRESS;
    QPushButton *BUTTON_ICCACCEPT;
    QPushButton *BUTTON_ICCACTIVE;
    QPushButton *BUTTON_ICCRELEASE;
    QPushButton *BUTTON_ICCRESETWARM;
    QPushButton *BUTTON_ICCCHIPIOT0;
    QPushButton *BUTTON_ICCRESETCOLD;
    QPushButton *BUTTON_ICCCHIPIOT1;
    QPushButton *BUTTON_GETFWVERSION;
    QPushButton *BUTTON_OPEN;
    QPushButton *BUTTON_CLOSE;
    QPlainTextEdit *IDC_EDIT_LOG;
    QPushButton *BUTTON_OPENLED;
    QPushButton *BUTTON_CLOSELED;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->setEnabled(true);
        MainWindow->resize(473, 531);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        BUTTON_INITEJECT = new QPushButton(centralWidget);
        BUTTON_INITEJECT->setObjectName(QStringLiteral("BUTTON_INITEJECT"));
        BUTTON_INITEJECT->setGeometry(QRect(20, 40, 81, 23));
        BUTTON_INITEAT = new QPushButton(centralWidget);
        BUTTON_INITEAT->setObjectName(QStringLiteral("BUTTON_INITEAT"));
        BUTTON_INITEAT->setGeometry(QRect(130, 40, 81, 23));
        BUTTON_INITHOLD = new QPushButton(centralWidget);
        BUTTON_INITHOLD->setObjectName(QStringLiteral("BUTTON_INITHOLD"));
        BUTTON_INITHOLD->setGeometry(QRect(240, 40, 81, 23));
        BUTTON_DETECTCARD = new QPushButton(centralWidget);
        BUTTON_DETECTCARD->setObjectName(QStringLiteral("BUTTON_DETECTCARD"));
        BUTTON_DETECTCARD->setGeometry(QRect(350, 40, 81, 23));
        BUTTON_RETRACTCARD = new QPushButton(centralWidget);
        BUTTON_RETRACTCARD->setObjectName(QStringLiteral("BUTTON_RETRACTCARD"));
        BUTTON_RETRACTCARD->setGeometry(QRect(350, 70, 81, 23));
        BUTTON_EJECTCARD = new QPushButton(centralWidget);
        BUTTON_EJECTCARD->setObjectName(QStringLiteral("BUTTON_EJECTCARD"));
        BUTTON_EJECTCARD->setGeometry(QRect(240, 70, 81, 23));
        BUTTON_ACCEPTCARD = new QPushButton(centralWidget);
        BUTTON_ACCEPTCARD->setObjectName(QStringLiteral("BUTTON_ACCEPTCARD"));
        BUTTON_ACCEPTCARD->setGeometry(QRect(20, 70, 81, 23));
        BUTTON_READTRACK = new QPushButton(centralWidget);
        BUTTON_READTRACK->setObjectName(QStringLiteral("BUTTON_READTRACK"));
        BUTTON_READTRACK->setGeometry(QRect(130, 70, 81, 23));
        BUTTON_CANCELACCEPT = new QPushButton(centralWidget);
        BUTTON_CANCELACCEPT->setObjectName(QStringLiteral("BUTTON_CANCELACCEPT"));
        BUTTON_CANCELACCEPT->setGeometry(QRect(20, 100, 81, 23));
        BUTTON_WRITECARD = new QPushButton(centralWidget);
        BUTTON_WRITECARD->setObjectName(QStringLiteral("BUTTON_WRITECARD"));
        BUTTON_WRITECARD->setGeometry(QRect(130, 100, 81, 23));
        BUTTON_ICCDEACTIVE = new QPushButton(centralWidget);
        BUTTON_ICCDEACTIVE->setObjectName(QStringLiteral("BUTTON_ICCDEACTIVE"));
        BUTTON_ICCDEACTIVE->setGeometry(QRect(350, 130, 81, 23));
        BUTTON_ICCPRESS = new QPushButton(centralWidget);
        BUTTON_ICCPRESS->setObjectName(QStringLiteral("BUTTON_ICCPRESS"));
        BUTTON_ICCPRESS->setGeometry(QRect(130, 130, 81, 23));
        BUTTON_ICCACCEPT = new QPushButton(centralWidget);
        BUTTON_ICCACCEPT->setObjectName(QStringLiteral("BUTTON_ICCACCEPT"));
        BUTTON_ICCACCEPT->setGeometry(QRect(20, 130, 81, 23));
        BUTTON_ICCACTIVE = new QPushButton(centralWidget);
        BUTTON_ICCACTIVE->setObjectName(QStringLiteral("BUTTON_ICCACTIVE"));
        BUTTON_ICCACTIVE->setGeometry(QRect(240, 130, 81, 23));
        BUTTON_ICCRELEASE = new QPushButton(centralWidget);
        BUTTON_ICCRELEASE->setObjectName(QStringLiteral("BUTTON_ICCRELEASE"));
        BUTTON_ICCRELEASE->setGeometry(QRect(240, 160, 81, 23));
        BUTTON_ICCRESETWARM = new QPushButton(centralWidget);
        BUTTON_ICCRESETWARM->setObjectName(QStringLiteral("BUTTON_ICCRESETWARM"));
        BUTTON_ICCRESETWARM->setGeometry(QRect(130, 160, 81, 23));
        BUTTON_ICCCHIPIOT0 = new QPushButton(centralWidget);
        BUTTON_ICCCHIPIOT0->setObjectName(QStringLiteral("BUTTON_ICCCHIPIOT0"));
        BUTTON_ICCCHIPIOT0->setGeometry(QRect(350, 160, 81, 23));
        BUTTON_ICCRESETCOLD = new QPushButton(centralWidget);
        BUTTON_ICCRESETCOLD->setObjectName(QStringLiteral("BUTTON_ICCRESETCOLD"));
        BUTTON_ICCRESETCOLD->setGeometry(QRect(20, 160, 81, 23));
        BUTTON_ICCCHIPIOT1 = new QPushButton(centralWidget);
        BUTTON_ICCCHIPIOT1->setObjectName(QStringLiteral("BUTTON_ICCCHIPIOT1"));
        BUTTON_ICCCHIPIOT1->setGeometry(QRect(20, 190, 81, 23));
        BUTTON_GETFWVERSION = new QPushButton(centralWidget);
        BUTTON_GETFWVERSION->setObjectName(QStringLiteral("BUTTON_GETFWVERSION"));
        BUTTON_GETFWVERSION->setGeometry(QRect(240, 10, 81, 23));
        BUTTON_OPEN = new QPushButton(centralWidget);
        BUTTON_OPEN->setObjectName(QStringLiteral("BUTTON_OPEN"));
        BUTTON_OPEN->setGeometry(QRect(20, 10, 81, 23));
        BUTTON_CLOSE = new QPushButton(centralWidget);
        BUTTON_CLOSE->setObjectName(QStringLiteral("BUTTON_CLOSE"));
        BUTTON_CLOSE->setGeometry(QRect(130, 10, 81, 23));
        IDC_EDIT_LOG = new QPlainTextEdit(centralWidget);
        IDC_EDIT_LOG->setObjectName(QStringLiteral("IDC_EDIT_LOG"));
        IDC_EDIT_LOG->setEnabled(false);
        IDC_EDIT_LOG->setGeometry(QRect(20, 230, 431, 231));
        QFont font;
        font.setPointSize(10);
        IDC_EDIT_LOG->setFont(font);
        IDC_EDIT_LOG->setReadOnly(true);
        BUTTON_OPENLED = new QPushButton(centralWidget);
        BUTTON_OPENLED->setObjectName(QStringLiteral("BUTTON_OPENLED"));
        BUTTON_OPENLED->setGeometry(QRect(130, 190, 81, 23));
        BUTTON_CLOSELED = new QPushButton(centralWidget);
        BUTTON_CLOSELED->setObjectName(QStringLiteral("BUTTON_CLOSELED"));
        BUTTON_CLOSELED->setGeometry(QRect(240, 190, 81, 23));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 473, 23));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
        BUTTON_INITEJECT->setText(QApplication::translate("MainWindow", "InitEject", Q_NULLPTR));
        BUTTON_INITEAT->setText(QApplication::translate("MainWindow", "InitEat", Q_NULLPTR));
        BUTTON_INITHOLD->setText(QApplication::translate("MainWindow", "InitHold", Q_NULLPTR));
        BUTTON_DETECTCARD->setText(QApplication::translate("MainWindow", "DetectCard", Q_NULLPTR));
        BUTTON_RETRACTCARD->setText(QApplication::translate("MainWindow", "RetractCard", Q_NULLPTR));
        BUTTON_EJECTCARD->setText(QApplication::translate("MainWindow", "EjectCard", Q_NULLPTR));
        BUTTON_ACCEPTCARD->setText(QApplication::translate("MainWindow", "AcceptCard", Q_NULLPTR));
        BUTTON_READTRACK->setText(QApplication::translate("MainWindow", "ReadTrack", Q_NULLPTR));
        BUTTON_CANCELACCEPT->setText(QApplication::translate("MainWindow", "CancelAccept", Q_NULLPTR));
        BUTTON_WRITECARD->setText(QApplication::translate("MainWindow", "WriteCard", Q_NULLPTR));
        BUTTON_ICCDEACTIVE->setText(QApplication::translate("MainWindow", "ICCDeActive", Q_NULLPTR));
        BUTTON_ICCPRESS->setText(QApplication::translate("MainWindow", "ICCPress", Q_NULLPTR));
        BUTTON_ICCACCEPT->setText(QApplication::translate("MainWindow", "ICCAccept", Q_NULLPTR));
        BUTTON_ICCACTIVE->setText(QApplication::translate("MainWindow", "ICCActive", Q_NULLPTR));
        BUTTON_ICCRELEASE->setText(QApplication::translate("MainWindow", "ICCRelease", Q_NULLPTR));
        BUTTON_ICCRESETWARM->setText(QApplication::translate("MainWindow", "ICCResetWarm", Q_NULLPTR));
        BUTTON_ICCCHIPIOT0->setText(QApplication::translate("MainWindow", "ICCChipIOT0", Q_NULLPTR));
        BUTTON_ICCRESETCOLD->setText(QApplication::translate("MainWindow", "ICCResetCold", Q_NULLPTR));
        BUTTON_ICCCHIPIOT1->setText(QApplication::translate("MainWindow", "ICCChipIOT1", Q_NULLPTR));
        BUTTON_GETFWVERSION->setText(QApplication::translate("MainWindow", "GetFWVer", Q_NULLPTR));
        BUTTON_OPEN->setText(QApplication::translate("MainWindow", "Open", Q_NULLPTR));
        BUTTON_CLOSE->setText(QApplication::translate("MainWindow", "Close", Q_NULLPTR));
        BUTTON_OPENLED->setText(QApplication::translate("MainWindow", "OpenLed", Q_NULLPTR));
        BUTTON_CLOSELED->setText(QApplication::translate("MainWindow", "CloseLed", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
