#include "TestUR2Driver.h"
#include <sys/stat.h>
#include <unistd.h>
#include "AutoQtHelpClass.h"

//------------------------------辅助函数---------------------------------------

void CodeChange(const char *srcData, QString &strRetData, const char *strDestCode, const char *strSrcCode = "")
{
    QTextCodec *localCode = QTextCodec::codecForLocale();

    QTextCodec *srcCode;
    if (nullptr == strSrcCode)
    {
        srcCode = QTextCodec::codecForLocale();
    }
    else
    {
        srcCode = QTextCodec::codecForName(strSrcCode);
    }
    QTextCodec *destCode = QTextCodec::codecForName(strDestCode);
    QTextCodec::setCodecForLocale(destCode);
    QString strText = QString::fromLocal8Bit(srcData);

    QTextCodec::setCodecForLocale(srcCode);
    QByteArray strTextData = strText.toLocal8Bit();
    strRetData = strTextData.data();

    QTextCodec::setCodecForLocale(localCode);
}

char *UCharToASCII(UCHAR ucData)
{
    static char data[MAX_SIZE] = {0};
    data[0] = (char)ucData;
    return data;
}

QString BOOLArrayToChar(BOOL bArray[], int iNum)
{
    QString temp;
    for (int i = 0; i < iNum; i++)
    {
        if (bArray[i])
        {
            temp += "1";
        }
        else
        {
            temp += "0";
        }
    }
    return temp;
}

//------------------------------TestUR2Driver---------------------------------------

TestUR2Driver::TestUR2Driver(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UR2DriverTest)
{
    ui->setupUi(this);

    connect(ui->treeWidget_command, SIGNAL(itemClicked(QTreeWidgetItem *, int)), SLOT(show_widget(QTreeWidgetItem *)));
    connect(&m_ConfigData, SIGNAL(SetValue(QString, QString, QString)), SLOT(SetCtrlValue(QString, QString, QString)));
    connect(ui->textEdit_UserMemoryWrite_szUserMemoryData, SIGNAL(textChanged()), SLOT(TextEditLimit()));
    connect(ui->menu_clear, SIGNAL(aboutToShow()), SLOT(ClearInfo()));
    connect(ui->pushButton_UpdatePDL_SelectPDL, SIGNAL(clicked()), SLOT(SelectPDLFile()));
    connect(ui->pushButton_UpdatePDL_LocalFW, SIGNAL(clicked()), SLOT(GetLocalFW()));
    connect(ui->pushButton_UpdatePDL_FileFW, SIGNAL(clicked()), SLOT(GetFileFW()));

    connect(ui->checkBox_SetUnitInfo_EnableParam, SIGNAL(stateChanged(int)), SLOT(EnableSetUnitInfoPriority(int)));
    connect(ui->checkBox_SetUnitInfo_bArryAcceptDenoCode, SIGNAL(stateChanged(int)), SLOT(EnableSetUnitInfoDenoCode(int)));
    connect(ui->checkBox_Dispense_pArrDispDeno, SIGNAL(stateChanged(int)), SLOT(EnableDispenseArrDispDeno(int)));
    connect(ui->checkBox_Dispense_pArrDispRoom, SIGNAL(stateChanged(int)), SLOT(EnableDispenseArrDispRoom(int)));
    connect(ui->checkBox_Dispense_bEnableBVModeSetting, SIGNAL(stateChanged(int)), SLOT(EnableDispenseBVMode(int)));
    connect(ui->checkBox_CashCount_bArryAcceptDenoCode, SIGNAL(stateChanged(int)), SLOT(EnableCashCountAcceptDeno(int)));
    connect(ui->checkBox_CashCount_bEnableBVModeSetting, SIGNAL(stateChanged(int)), SLOT(EnableCashCountBVMode(int)));
    connect(ui->checkBox_StoreMoney_bEnableBVModeSetting, SIGNAL(stateChanged(int)), SLOT(EnableStoreMoneyBVMode(int)));
    connect(ui->checkBox_CashCountRetract_bEnableBVModeSetting, SIGNAL(stateChanged(int)), SLOT(EnableCashCountRetractBVMode(int)));
    connect(ui->checkBox_RetractESC_bEnableBVModeSetting, SIGNAL(stateChanged(int)), SLOT(EnableRetractESCBVMode(int)));
    connect(ui->checkBox_Rejcet_bEnableBVModeSetting, SIGNAL(stateChanged(int)), SLOT(EnableRejectBVMode(int)));

    //窗口布局
    QHBoxLayout *vLayout = new QHBoxLayout;
    vLayout->addWidget(ui->pushButton_exec, 250);
    vLayout->addStretch(505);

    QVBoxLayout *hLayout = new QVBoxLayout;
    hLayout->addWidget(ui->label_command_title, 30);
    hLayout->addWidget(ui->stackedWidget_command, 375);
    hLayout->addLayout(vLayout, 31);
    hLayout->addWidget(ui->listWidget_result, 155);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(ui->treeWidget_command, 0, 0);
    mainLayout->addLayout(hLayout, 0, 1);
    mainLayout->setRowStretch(0, 621);
    mainLayout->setColumnStretch(0, 230);
    mainLayout->setColumnStretch(1, 755);
    mainLayout->setAlignment(Qt::AlignHCenter);
    mainLayout->setAlignment(Qt::AlignVCenter);

    mainWidget = new QWidget();
    setCentralWidget(mainWidget);
    mainWidget->setLayout(mainLayout);

    //初始化树控件
    InitTreeWidget();
    ReadInitData();

    //表格设置
    ui->tableWidget_GetDevStatusInfo->setColumnWidth(0, 150);
    ui->tableWidget_GetDevStatusInfo->setColumnWidth(1, 220);
    ui->tableWidget_GetDevStatusInfo->setColumnWidth(2, 130);
    ui->tableWidget_GetDevStatusInfo->setColumnWidth(3, 220);
    ui->tableWidget_GetDevStatusInfo->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget_GetDevStatusInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //初始化成员变量
    m_iCommandType = -1;
    m_bGetLocalFWFlag = false;
    m_bGetFileFWFlag = false;
    m_bUpdate = false;
    m_iBVCount = 0;
    m_list_log = ui->listWidget_result;
    m_currency_path = getcwd(nullptr, 0);

    QDir dir;
    m_strLogDir.sprintf("%s/%s", qPrintable(dir.currentPath()), LOG_DIR);

    memset(&m_stDevStatusInfo, 0, sizeof(ST_DEV_STATUS_INFO));
    ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
}

TestUR2Driver::~TestUR2Driver()
{
    if (m_UR2Device != nullptr)
    {
        m_UR2Device->CloseURConn();
        m_UR2Device->CloseBVConn();
    }

    Release();

    delete ui;
    delete mainWidget;
}

void TestUR2Driver::Release()
{
    m_UR2Device.Release();
}

void TestUR2Driver::InitTreeWidget()
{
    ui->treeWidget_command->setColumnCount(1); //设置列数
    ui->treeWidget_command->setHeaderLabel("命令集"); //设置头的标题

    //初始化设置指令组
    QTreeWidgetItem *treeItem1   = new QTreeWidgetItem(ui->treeWidget_command, QStringList(QString("初始化设置指令组")));
    QTreeWidgetItem *treeItem1_1 = new QTreeWidgetItem(treeItem1, QStringList(QString("查询固件版本信息")));
    //QTreeWidgetItem *treeItem1_2 = new QTreeWidgetItem(treeItem1,QStringList(QString("设置校验级别")));
    QTreeWidgetItem *treeItem1_2 = new QTreeWidgetItem(treeItem1, QStringList(QString("查询BV序列号及配置表")));
    QTreeWidgetItem *treeItem1_3 = new QTreeWidgetItem(treeItem1, QStringList(QString("设置面额配置代码")));
    QTreeWidgetItem *treeItem1_4 = new QTreeWidgetItem(treeItem1, QStringList(QString("查询钞箱信息")));
    QTreeWidgetItem *treeItem1_5 = new QTreeWidgetItem(treeItem1, QStringList(QString("设置HCM钞箱配置信息")));
    QTreeWidgetItem *treeItem1_6 = new QTreeWidgetItem(treeItem1, QStringList(QString("获取HCM钞箱配置信息")));
    QTreeWidgetItem *treeItem1_7 = new QTreeWidgetItem(treeItem1, QStringList(QString("复位")));
    treeItem1->addChild(treeItem1_1); //添加子节点
    treeItem1->addChild(treeItem1_2);
    treeItem1->addChild(treeItem1_3);
    treeItem1->addChild(treeItem1_4);
    treeItem1->addChild(treeItem1_5);
    treeItem1->addChild(treeItem1_6);
    treeItem1->addChild(treeItem1_7);

    //查询类指令组
    QTreeWidgetItem *treeItem2   = new QTreeWidgetItem(ui->treeWidget_command, QStringList(QString("查询类指令组")));
    QTreeWidgetItem *treeItem2_1 = new QTreeWidgetItem(treeItem2, QStringList(QString("查询BV序列号及配置表")));
    QTreeWidgetItem *treeItem2_2 = new QTreeWidgetItem(treeItem2, QStringList(QString("查询BV模块信息")));
    QTreeWidgetItem *treeItem2_3 = new QTreeWidgetItem(treeItem2, QStringList(QString("获得设备状态信息")));
    QTreeWidgetItem *treeItem2_4 = new QTreeWidgetItem(treeItem2, QStringList(QString("获取冠字码相关信息")));
    QTreeWidgetItem *treeItem2_5 = new QTreeWidgetItem(treeItem2, QStringList(QString("获取钞票流向面额信息")));
    QTreeWidgetItem *treeItem2_6 = new QTreeWidgetItem(treeItem2, QStringList(QString("查询钞箱信息")));
    QTreeWidgetItem *treeItem2_7 = new QTreeWidgetItem(treeItem2, QStringList(QString("获取HCM钞箱配置信息")));
    QTreeWidgetItem *treeItem2_8 = new QTreeWidgetItem(treeItem2, QStringList(QString("读取所有感应器的亮灭状态")));
    treeItem2->addChild(treeItem2_1); //添加子节点
    treeItem2->addChild(treeItem2_2);
    treeItem2->addChild(treeItem2_3);
    treeItem2->addChild(treeItem2_4);
    treeItem2->addChild(treeItem2_5);
    treeItem2->addChild(treeItem2_6);
    treeItem2->addChild(treeItem2_7);
    treeItem2->addChild(treeItem2_8);

    //控制指令组
    QTreeWidgetItem *treeItem3    = new QTreeWidgetItem(ui->treeWidget_command, QStringList(QString("控制指令组")));
    QTreeWidgetItem *treeItem3_1  = new QTreeWidgetItem(treeItem3, QStringList(QString("点钞")));
    QTreeWidgetItem *treeItem3_2  = new QTreeWidgetItem(treeItem3, QStringList(QString("点钞回收")));
    QTreeWidgetItem *treeItem3_3  = new QTreeWidgetItem(treeItem3, QStringList(QString("存入钞票")));
    QTreeWidgetItem *treeItem3_4  = new QTreeWidgetItem(treeItem3, QStringList(QString("取消存款")));
    QTreeWidgetItem *treeItem3_5  = new QTreeWidgetItem(treeItem3, QStringList(QString("挖钞")));
    QTreeWidgetItem *treeItem3_6  = new QTreeWidgetItem(treeItem3, QStringList(QString("回收ESC中的钞票(取款废钞)")));
    QTreeWidgetItem *treeItem3_7  = new QTreeWidgetItem(treeItem3, QStringList(QString("回收ESC中的钞票")));
    QTreeWidgetItem *treeItem3_8  = new QTreeWidgetItem(treeItem3, QStringList(QString("关闭Shutter门")));
    QTreeWidgetItem *treeItem3_9  = new QTreeWidgetItem(treeItem3, QStringList(QString("打开Shutter门")));
    QTreeWidgetItem *treeItem3_10 = new QTreeWidgetItem(treeItem3, QStringList(QString("PC与ZERO BV通信开始")));
    QTreeWidgetItem *treeItem3_11 = new QTreeWidgetItem(treeItem3, QStringList(QString("PC与ZERO BV通信结束")));
    QTreeWidgetItem *treeItem3_12 = new QTreeWidgetItem(treeItem3, QStringList(QString("复位")));
    treeItem3->addChild(treeItem3_1); //添加子节点
    treeItem3->addChild(treeItem3_2);
    treeItem3->addChild(treeItem3_3);
    treeItem3->addChild(treeItem3_4);
    treeItem3->addChild(treeItem3_5);
    treeItem3->addChild(treeItem3_6);
    treeItem3->addChild(treeItem3_7);
    treeItem3->addChild(treeItem3_8);
    treeItem3->addChild(treeItem3_9);
    treeItem3->addChild(treeItem3_10);
    treeItem3->addChild(treeItem3_11);
    treeItem3->addChild(treeItem3_12);

    //维护指令组
    QTreeWidgetItem *treeItem4   = new QTreeWidgetItem(ui->treeWidget_command, QStringList(QString("维护指令组")));
    QTreeWidgetItem *treeItem4_1 = new QTreeWidgetItem(treeItem4, QStringList(QString("断开连接并重启")));
    QTreeWidgetItem *treeItem4_2 = new QTreeWidgetItem(treeItem4, QStringList(QString("开始存取款交易")));
    QTreeWidgetItem *treeItem4_3 = new QTreeWidgetItem(treeItem4, QStringList(QString("准备进入一下笔交易")));
    QTreeWidgetItem *treeItem4_4 = new QTreeWidgetItem(treeItem4, QStringList(QString("设置状态改变侦听接口")));
    QTreeWidgetItem *treeItem4_5 = new QTreeWidgetItem(treeItem4, QStringList(QString("写数据到用户内存区域")));
    QTreeWidgetItem *treeItem4_6 = new QTreeWidgetItem(treeItem4, QStringList(QString("读取用户内存的数据")));
    treeItem4->addChild(treeItem4_1); //添加子节点
    treeItem4->addChild(treeItem4_2);
    treeItem4->addChild(treeItem4_3);
    treeItem4->addChild(treeItem4_4);
    treeItem4->addChild(treeItem4_5);
    treeItem4->addChild(treeItem4_6);

    //其他指令组
    QTreeWidgetItem *treeItem5   = new QTreeWidgetItem(ui->treeWidget_command, QStringList(QString("其他指令组")));
    QTreeWidgetItem *treeItem5_1 = new QTreeWidgetItem(treeItem5, QStringList(QString("请求日志数据")));
    QTreeWidgetItem *treeItem5_2 = new QTreeWidgetItem(treeItem5, QStringList(QString("清除日志数据")));
    QTreeWidgetItem *treeItem5_3 = new QTreeWidgetItem(treeItem5, QStringList(QString("关闭连接")));
    QTreeWidgetItem *treeItem5_4 = new QTreeWidgetItem(treeItem5, QStringList(QString("更新固件")));
    treeItem5->addChild(treeItem5_1); //添加子节点
    treeItem5->addChild(treeItem5_2);
    treeItem5->addChild(treeItem5_3);
    treeItem5->addChild(treeItem5_4);
}

void TestUR2Driver::ReadInitData()
{
    if (FALSE == m_ConfigData.LoadConfigFile())
    {
        Log("读取配置文件失败");
    }
}

//---------------------------打开可选参数---------------------------------

void TestUR2Driver::EnableSetUnitInfoPriority(int iStatus)
{
    if (2 == iStatus)
    {
        char pTempCtrlName[MAX_SIZE];
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            sprintf(pTempCtrlName, "spinBox_SetUnitInfo_Cassette%d_usArryCassCashInPrioritySet", i + 1);
            QSpinBox *spinBox = findChild<QSpinBox *>(pTempCtrlName);
            spinBox->setEnabled(true);

            sprintf(pTempCtrlName, "spinBox_SetUnitInfo_Cassette%d_usArryCassCashOutPrioritySet", i + 1);
            spinBox = findChild<QSpinBox *>(pTempCtrlName);
            spinBox->setEnabled(true);
        }
    }
    else if (0 == iStatus)
    {
        char pCtrlName[MAX_SIZE];
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            sprintf(pCtrlName, "spinBox_SetUnitInfo_Cassette%d_usArryCassCashInPrioritySet", i + 1);
            QSpinBox *spinBox1 = findChild<QSpinBox *>(pCtrlName);
            spinBox1->setEnabled(false);

            sprintf(pCtrlName, "spinBox_SetUnitInfo_Cassette%d_usArryCassCashOutPrioritySet", i + 1);
            spinBox1 = findChild<QSpinBox *>(pCtrlName);
            spinBox1->setEnabled(false);
        }
    }
}

void TestUR2Driver::EnableDispenseArrDispDeno(int iStatus)
{
    if (2 == iStatus)
    {
        char pTempCtrlName[MAX_SIZE];
        for (int i = 0; i < MAX_DISP_DENO_NUM; i++)
        {
            sprintf(pTempCtrlName, "comboBox_Dispense_pArrDispDeno_iDenoCode%d", i + 1);
            QComboBox *comboBox = findChild<QComboBox *>(pTempCtrlName);
            comboBox->setEnabled(true);

            sprintf(pTempCtrlName, "lineEdit_Dispense_pArrDispDeno_iCashNumber%d", i + 1);
            QLineEdit *lineEdit = findChild<QLineEdit *>(pTempCtrlName);
            lineEdit->setEnabled(true);
        }
    }
    else if (0 == iStatus)
    {
        char pTempCtrlName[MAX_SIZE];
        for (int i = 0; i < MAX_DISP_DENO_NUM; i++)
        {
            sprintf(pTempCtrlName, "comboBox_Dispense_pArrDispDeno_iDenoCode%d", i + 1);
            QComboBox *comboBox = findChild<QComboBox *>(pTempCtrlName);
            comboBox->setEnabled(false);

            sprintf(pTempCtrlName, "lineEdit_Dispense_pArrDispDeno_iCashNumber%d", i + 1);
            QLineEdit *lineEdit = findChild<QLineEdit *>(pTempCtrlName);
            lineEdit->setEnabled(false);
        }
    }
}

void TestUR2Driver::EnableDispenseArrDispRoom(int iStatus)
{
    if (2 == iStatus)
    {
        char pTempCtrlName[MAX_SIZE];
        for (int i = 0; i < MAX_DISP_ROOM_NUM; i++)
        {
            sprintf(pTempCtrlName, "comboBox_Dispense_pArrDispRoom_iCassID%d", i + 1);
            QComboBox *comboBox = findChild<QComboBox *>(pTempCtrlName);
            comboBox->setEnabled(true);

            sprintf(pTempCtrlName, "lineEdit_Dispense_pArrDispRoom_iCashNumber%d", i + 1);
            QLineEdit *lineEdit = findChild<QLineEdit *>(pTempCtrlName);
            lineEdit->setEnabled(true);
        }
    }
    else if (0 == iStatus)
    {
        char pTempCtrlName[MAX_SIZE];
        for (int i = 0; i < MAX_DISP_ROOM_NUM; i++)
        {
            sprintf(pTempCtrlName, "comboBox_Dispense_pArrDispRoom_iCassID%d", i + 1);
            QComboBox *comboBox = findChild<QComboBox *>(pTempCtrlName);
            comboBox->setEnabled(false);

            sprintf(pTempCtrlName, "lineEdit_Dispense_pArrDispRoom_iCashNumber%d", i + 1);
            QLineEdit *lineEdit = findChild<QLineEdit *>(pTempCtrlName);
            lineEdit->setEnabled(false);
        }
    }
}

void TestUR2Driver::EnableCashCountAcceptDeno(int iStatus)
{
    if (2 == iStatus)
    {
        char pTempCtrlName[MAX_SIZE];
        for (int i = 0; i < MAX_DENO_LENGTH; i++)
        {
            sprintf(pTempCtrlName, "spinBox_CashCount_bArryAcceptDeno%d", i + 1);
            QSpinBox *spinBox = findChild<QSpinBox *>(pTempCtrlName);
            spinBox->setEnabled(true);
        }
    }
    else if (0 == iStatus)
    {
        char pCtrlName[MAX_SIZE];
        for (int i = 0; i < MAX_DENO_LENGTH; i++)
        {
            sprintf(pCtrlName, "spinBox_CashCount_bArryAcceptDeno%d", i + 1);
            QSpinBox *spinBox = findChild<QSpinBox *>(pCtrlName);
            spinBox->setEnabled(false);
        }
    }
}

void TestUR2Driver::EnableSetUnitInfoDenoCode(int iStatus)
{
    if (2 == iStatus)
    {
        char pTempCtrlName[MAX_SIZE];
        for (int i = 0; i < MAX_DENO_LENGTH; i++)
        {
            sprintf(pTempCtrlName, "spinBox_SetUnitInfo_bArryAcceptDenoCode%d", i + 1);
            QSpinBox *spinBox = findChild<QSpinBox *>(pTempCtrlName);
            spinBox->setEnabled(true);
        }
    }
    else if (0 == iStatus)
    {
        char pCtrlName[MAX_SIZE];
        for (int i = 0; i < MAX_DENO_LENGTH; i++)
        {
            sprintf(pCtrlName, "spinBox_SetUnitInfo_bArryAcceptDenoCode%d", i + 1);
            QSpinBox *spinBox = findChild<QSpinBox *>(pCtrlName);
            spinBox->setEnabled(false);
        }
    }
}

void TestUR2Driver::EnableCashCountBVMode(int iStatus)
{
    if (2 == iStatus)
    {
        ui->comboBox_CashCount_iFullImageMode->setEnabled(true);
        ui->comboBox_CashCount_iRejcetNoteNOSN->setEnabled(true);
    }
    else if (0 == iStatus)
    {
        ui->comboBox_CashCount_iFullImageMode->setEnabled(false);
        ui->comboBox_CashCount_iRejcetNoteNOSN->setEnabled(false);
    }
}

void TestUR2Driver::EnableCashCountRetractBVMode(int iStatus)
{
    if (2 == iStatus)
    {
        ui->comboBox_CashCountRetract_iFullImageMode->setEnabled(true);
        ui->comboBox_CashCountRetract_iRejcetNoteNOSN->setEnabled(true);
    }
    else if (0 == iStatus)
    {
        ui->comboBox_CashCountRetract_iFullImageMode->setEnabled(false);
        ui->comboBox_CashCountRetract_iRejcetNoteNOSN->setEnabled(false);
    }
}

void TestUR2Driver::EnableStoreMoneyBVMode(int iStatus)
{
    if (2 == iStatus)
    {
        ui->comboBox_StoreMoney_iFullImageMode->setEnabled(true);
        ui->comboBox_StoreMoney_iRejcetNoteNOSN->setEnabled(true);
    }
    else if (0 == iStatus)
    {
        ui->comboBox_StoreMoney_iFullImageMode->setEnabled(false);
        ui->comboBox_StoreMoney_iRejcetNoteNOSN->setEnabled(false);
    }
}

void TestUR2Driver::EnableDispenseBVMode(int iStatus)
{
    if (2 == iStatus)
    {
        ui->comboBox_Dispense_iFullImageMode->setEnabled(true);
        ui->comboBox_Dispense_iRejcetNoteNOSN->setEnabled(true);
    }
    else if (0 == iStatus)
    {
        ui->comboBox_Dispense_iFullImageMode->setEnabled(false);
        ui->comboBox_Dispense_iRejcetNoteNOSN->setEnabled(false);
    }
}

void TestUR2Driver::EnableRejectBVMode(int iStatus)
{
    if (2 == iStatus)
    {
        ui->comboBox_Rejcet_iFullImageMode->setEnabled(true);
        ui->comboBox_Rejcet_iRejcetNoteNOSN->setEnabled(true);
    }
    else if (0 == iStatus)
    {
        ui->comboBox_Rejcet_iFullImageMode->setEnabled(false);
        ui->comboBox_Rejcet_iRejcetNoteNOSN->setEnabled(false);
    }
}

void TestUR2Driver::EnableRetractESCBVMode(int iStatus)
{
    if (2 == iStatus)
    {
        ui->comboBox_RetractESC_iFullImageMode->setEnabled(true);
        ui->comboBox_RetractESC_iRejcetNoteNOSN->setEnabled(true);
    }
    else if (0 == iStatus)
    {
        ui->comboBox_RetractESC_iFullImageMode->setEnabled(false);
        ui->comboBox_RetractESC_iRejcetNoteNOSN->setEnabled(false);
    }
}

//////////////////////////////////////////////////////
//
//                 日志记录函数
//
//////////////////////////////////////////////////////
void  TestUR2Driver::Log(QString Log_content)
{

    //ui->textEdit_ShowInfo->append(Log_content);

    //记录到文件
    //形成文件名
    SYSTEMTIME stSystemTime;
    CQtTime::GetLocalTime(stSystemTime);

    char tmp_file[MAX_SIZE] = {0};

    QDir logDir;
    if (!logDir.exists(m_strLogDir))
    {
        bool bReturn = logDir.mkpath(m_strLogDir);
        if (!bReturn)
        {
            QMessageBox::warning(this, "创建文件夹", "日志文件夹创建失败！");
            return ;
        }
    }

    char tmp[MAX_SIZE] = {0};
    sprintf(tmp, "%04d_%02d_%02d_%02d.Log", stSystemTime.wYear, stSystemTime.wMonth, stSystemTime.wDay, stSystemTime.wHour);
    sprintf(tmp_file, "%s/%s", qPrintable(m_strLogDir), tmp);
    FILE *fp = fopen(tmp_file, "at");
    if (fp != nullptr)
    {
        Log_content += "\n";
        fputs(qPrintable(Log_content), fp);
        fclose(fp);
    }
    if (m_list_log->count() >= 10000)
    {
        m_list_log->clear();
    }

    m_list_log->addItem(Log_content);
    m_list_log->setCurrentRow(m_list_log->count() - 1);

    //记录到文件
    //形成文件名

    /*
    char tmp_dir[1024] = {0};
    char tmp_file[1024] = {0};
    sprintf(tmp_dir, "%s/HCMDriverTestLog", m_currency_path);
    mkdir(tmp_dir, S_IRUSR | S_IWUSR | S_IXUSR);
    char tmp[256] = {0};
    sprintf(tmp, "%04d_%02d_%02d_%02d.Log", stSystemTime.wYear, stSystemTime.wMonth, stSystemTime.wDay, stSystemTime.wHour);
    sprintf(tmp_file, "%s/%s", tmp_dir, tmp);
    FILE *fp = fopen(tmp_file, "at");
    if (fp != NULL)
    {
        Log_content += "\n";
        fputs(qPrintable(Log_content), fp);
        fclose(fp);
    }
    */
}

void TestUR2Driver::LogBeforeExecute()
{
    QString str;
    if (m_iCommandType == InvalidCommand)
    {
        QMessageBox::information(this, "error", "不能执行命令组!");
        return;
    }

    //记录时间及命令
    SYSTEMTIME stSystemTime;
    CQtTime::GetLocalTime(stSystemTime);
    str.sprintf("当前时间: %04d-%02d-%02d %02d:%02d:%02d.%03d",
                stSystemTime.wYear, stSystemTime.wMonth, stSystemTime.wDay,
                stSystemTime.wHour, stSystemTime.wMinute, stSystemTime.wSecond,
                stSystemTime.wMilliseconds);
    m_iStartTime = (stSystemTime.wHour * 60 * 60 * 1000) + (stSystemTime.wMinute * 60 * 1000) + (stSystemTime.wSecond * 1000) + stSystemTime.wMilliseconds;
    Log(str);
    str.clear();
    str.sprintf("执行命令: %s", qPrintable(m_strCurrencyCommand));
    Log(str);
}

void TestUR2Driver::LogAfterExecute(int nError, bool bparam)
{
    QString str;
    //记录时间及命令
    SYSTEMTIME stSystemTime;
    CQtTime::GetLocalTime(stSystemTime);
    str.sprintf("当前时间: %04d-%02d-%02d %02d:%02d:%02d.%03d",
                stSystemTime.wYear, stSystemTime.wMonth, stSystemTime.wDay,
                stSystemTime.wHour, stSystemTime.wMinute, stSystemTime.wSecond,
                stSystemTime.wMilliseconds);
    m_iEndTime = (stSystemTime.wHour * 60 * 60 * 1000) + (stSystemTime.wMinute * 60 * 1000) + (stSystemTime.wSecond * 1000) + stSystemTime.wMilliseconds;
    Log(str);
    str.clear();

    if (nError >= 0)
    {
        if (bparam)
        {
            str.sprintf("输出参数: ");
        }
        else
        {
            str.sprintf("输出参数：无");
        }
        Log(str);
    }

}

//记录一条输出，iTabNum指定前面加TAB个数
void TestUR2Driver::LogOutput(const char *pName, const QString Value, int iTabNum)
{
    QString sTab;
    while (iTabNum > 0)
    {
        sTab += TAB_STR;
        iTabNum--;
    }

    QString strTemp;
    strTemp.sprintf("%s%-30s = %s", qPrintable(sTab), pName, qPrintable(Value));
    Log(strTemp);
}

void TestUR2Driver::LogEnd(int nError)
{
    QString str;
    str.sprintf("命令执行时间： %d秒,%d毫秒", ((m_iEndTime - m_iStartTime) / 1000), ((m_iEndTime - m_iStartTime) % 1000));
    Log(str);
    str.clear();

    //命令执行成功不处理警告码查询
    if (nError == 0)
    {
        str.sprintf("返 回 值: %d[][]", nError);
    }
    else if (nError == -3)
    {
        str.sprintf("返 回 值: %d[][参数错误]", nError);
    }
    else
    {
        ST_ERR_DETAIL stErrDetail;
        m_UR2Device->GetLastErrDetail(stErrDetail);
        QString errString;

        CodeChange(m_UR2Device->GetErrDesc(stErrDetail.ucErrorCode), errString, "GB2312");
        str.sprintf("返 回 值: %d[%s][%s]", nError, stErrDetail.ucErrorCode, qPrintable(errString));
    }
    Log(str);
    Log("*************************************************************");
}

void TestUR2Driver::LogOutputStructName(const char *pStructName, int iTabNum) //记录结构名
{
    QString sTab, str_temp;
    while (iTabNum > 0)
    {
        sTab += TAB_STR;
        iTabNum--;
    }
    str_temp.sprintf("%s%s", qPrintable(sTab), pStructName);
    Log(str_temp);
}

//---------------------------特定结构记录-----------------------------------

//ST_DEV_STATUS_INFO
void TestUR2Driver::LogDevStatusInfo(const ST_DEV_STATUS_INFO &stDevStatusInfo)
{
    //数据相同则不更新列表
    static bool bisInit = true;
    if (!bisInit &&
        memcmp(&m_stDevStatusInfo, &stDevStatusInfo, sizeof(ST_DEV_STATUS_INFO)) == 0)
    {
        return;
    }

    bisInit = false;
    QTableWidgetItem *item[42]; // ST_DEV_STATUS_INFO需要输出的项目数
    for (int i = 0; i < 42 ; i++)
    {
        item[i] = new QTableWidgetItem();
    }

    item[0]->setText(stDevStatusInfo.bCashAtShutter ? "TRUE" : "FALSE");
    item[1]->setText(stDevStatusInfo.bCashAtCS ? "TRUE" : "FALSE");
    item[2]->setText(stDevStatusInfo.bCashAtCSErrPos ? "TRUE" : "FALSE");
    item[3]->setText(stDevStatusInfo.bESCFull ? "TRUE" : "FALSE");
    item[4]->setText(stDevStatusInfo.bESCNearFull ? "TRUE" : "FALSE");
    item[5]->setText(stDevStatusInfo.bESCEmpty ? "TRUE" : "FALSE");
    item[6]->setText(stDevStatusInfo.bURJBFull ? "TRUE" : "FALSE");
    item[7]->setText(stDevStatusInfo.bURJBEmpty ? "TRUE" : "FALSE");
    item[8]->setText(stDevStatusInfo.bNotesRJInCashCout ? "TRUE" : "FALSE");
    item[9]->setText(stDevStatusInfo.bHCMUPInPos ? "TRUE" : "FALSE");
    item[10]->setText(stDevStatusInfo.bHCMLOWInPos ? "TRUE" : "FALSE");
    item[11]->setText(stDevStatusInfo.bURJBOpen ? "TRUE" : "FALSE");
    item[12]->setText(stDevStatusInfo.bRearDoorOpen ? "TRUE" : "FALSE");
    item[13]->setText(stDevStatusInfo.bFrontDoorOpen ? "TRUE" : "FALSE");
    item[14]->setText(stDevStatusInfo.bESCOpen ? "TRUE" : "FALSE");
    item[15]->setText(stDevStatusInfo.bESCInPos ? "TRUE" : "FALSE");
    item[16]->setText(stDevStatusInfo.bESCRearEnd ? "TRUE" : "FALSE");
    item[17]->setText(stDevStatusInfo.bCSInPos ? "TRUE" : "FALSE");
    item[18]->setText(stDevStatusInfo.bBVFanErr ? "TRUE" : "FALSE");
    item[19]->setText(stDevStatusInfo.bBVOpen ? "TRUE" : "FALSE");
    item[20]->setText(EnumToString("HCM2SHUTTER_STATUS", stDevStatusInfo.iOutShutterStatus));

    for (int iCount = 0; iCount < MAX_CASSETTE_NUM - 1; iCount++)
    {
        char szCassStatus[100] = {0};
        sprintf(szCassStatus, "Cass[%d]Status", iCount);
        item[21 + iCount]->setText(EnumToString("CASSETTE_STATUS", stDevStatusInfo.CassStatus[iCount]));
    }

    for (int iCount = 0; iCount < MAX_CASSETTE_NUM - 1; iCount++)
    {
        char szCassInPos[100] = {0};
        sprintf(szCassInPos, "Cass[%d]InPos", iCount);
        item[26 + iCount]->setText((stDevStatusInfo.bCassInPos[iCount] ? "TRUE" : "FALSE"));
    }
    item[31]->setText(stDevStatusInfo.bForcedOpenShutter ? "TRUE" : "FALSE");
    item[32]->setText(stDevStatusInfo.bForcedRemovCashInCS ? "TRUE" : "FALSE");
    item[33]->setText(stDevStatusInfo.bCashLeftInCS ? "TRUE" : "FALSE");
    item[34]->setText(stDevStatusInfo.bCashExistInESC ? "TRUE" : "FALSE");
    item[35]->setText(stDevStatusInfo.bReqReadStatus ? "TRUE" : "FALSE");
    item[36]->setText(stDevStatusInfo.bReqGetOPLog ? "TRUE" : "FALSE");
    item[37]->setText(stDevStatusInfo.bReqReset ? "TRUE" : "FALSE");
    item[38]->setText(stDevStatusInfo.bBVWarning ? "TRUE" : "FALSE");
    item[39]->setText(stDevStatusInfo.bDuringEnergy ? "TRUE" : "FALSE");

    for (int i = 0; i < 42 / 2; i++)
    {
        ui->tableWidget_GetDevStatusInfo->setItem(i, 1, item[i * 2]);
        ui->tableWidget_GetDevStatusInfo->setItem(i, 3, item[i * 2 + 1]);
    }

    FindDevDifference(stDevStatusInfo);

    SaveDevStatus(stDevStatusInfo);
}

//ST_BV_INFO
void TestUR2Driver::LogBVInfo(const ST_BV_INFO &stBVInfo, int iTabNum)
{
    LogOutputStructName("BVInfo", iTabNum);

    LogOutput("BVSerialNumber", (stBVInfo.szBVSerialNumber), iTabNum + 1);
    LogOutput("Article6Support", (stBVInfo.bArticle6Support ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("BackTracingSupport", (stBVInfo.bBackTracingSupport ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("UnknownNotesNumberInDispSupport", (stBVInfo.bUnknownNotesNumberInDispSupport ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("SNImageReadFunctionSupport", (stBVInfo.bSNImageReadFunctionSupport ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("FullSNImageRecordSupport", (stBVInfo.bFullSNImageRecordSupport ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("UnknownNotesEstimationSupport", (stBVInfo.bUnknownNotesEstimationSupport ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("SNReadSNFunctionSupport", (stBVInfo.bSNReadSNFunctionSupport ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("ActiveVerificationLevelSupport", (stBVInfo.bActiveVerificationLevelSupport ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("UseUnfitLevelSupport", (stBVInfo.bUseUnfitLevelSupport ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("RejectSNSupport", (stBVInfo.bRejectSNSupport ? "TRUE" : "FALSE"), iTabNum + 1);
}

//ST_DENO_INFO
void TestUR2Driver::LogDenoInfo(const ST_DENO_INFO &stDenoInfo, int iNum, int iTabNum)
{
    if (-1 == iNum)
    {
        LogOutputStructName("DenoInfo", iTabNum);
    }
    else
    {
        QString temp;
        temp.sprintf("DenoInfo[%d]", iNum);
        LogOutputStructName(qPrintable(temp), iTabNum);
    }

    LogOutput("CurrencyCode",  EnumToString("CURRENCY_CODE", stDenoInfo.iCurrencyCode), iTabNum + 1);
    LogOutput("CashValue",  QString::number(stDenoInfo.iCashValue), iTabNum + 1);
    LogOutput("Version",  UCharToASCII(stDenoInfo.ucVersion), iTabNum + 1);
    LogOutput("IssuingBank",  UCharToASCII(stDenoInfo.ucIssuingBank), iTabNum + 1);
    LogOutput("NoteWidth",  QString::number(stDenoInfo.ucNoteWidth), iTabNum + 1);
    LogOutput("NoteLength",  QString::number(stDenoInfo.ucNoteLength), iTabNum + 1);
}

//ST_CASSETTE_INFO
void TestUR2Driver::LogCassetteInfo(const ST_CASSETTE_INFO &stCassetteInfo, int iNum, int iTabNum)
{
    if (-1 == iNum)
    {
        LogOutputStructName("CassetteInfo", iTabNum);
    }
    else
    {
        QString temp;
        temp.sprintf("Cassette[%d]Info", iNum);
        LogOutputStructName(qPrintable(temp), iTabNum);
    }

    LogOutput("CassNO",  EnumToString("CASSETTE_NUMBER", stCassetteInfo.iCassNO), iTabNum + 1);
    LogOutput("CassType",  EnumToString("CASSETTE_TYPE", stCassetteInfo.iCassType), iTabNum + 1);
    LogOutput("DenoCode",  EnumToString("DENOMINATION_CODE", stCassetteInfo.iDenoCode), iTabNum + 1);
    LogOutput("CassOper",  EnumToString("CASSETTE_OPERATION", stCassetteInfo.iCassOper), iTabNum + 1);
    LogOutput("CurrencyCode",  EnumToString("CURRENCY_CODE", stCassetteInfo.iCurrencyCode), iTabNum + 1);
    LogOutput("CashValue",  QString::number(stCassetteInfo.iCashValue), iTabNum + 1);
    LogOutput("Version",  UCharToASCII(stCassetteInfo.ucVersion), iTabNum + 1);
    LogOutput("IssuingBank",  UCharToASCII(stCassetteInfo.ucIssuingBank), iTabNum + 1);
    LogOutput("CassNoteHandInfo",  EnumToString("DESTINATION_REJCET", stCassetteInfo.iCassNoteHandInfo), iTabNum + 1);
}

//ST_OPERATIONAL_INFO
void TestUR2Driver::LogOperationalInfo(const ST_OPERATIONAL_INFO &stOperationalInfo, int iTabNum)
{
    LogOutputStructName("OperationalInfo", iTabNum);

    LogOutput("Article6Support", (stOperationalInfo.bArticle6Support ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("ActiveVerificationLevel", (stOperationalInfo.bActiveVerificationLevel ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("UseUnfitLevel", (stOperationalInfo.bRejectUnfitNotesStore ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("ReportUnacceptDeno", (stOperationalInfo.bReportUnacceptDeno ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("CashCountErrAsWarning", (stOperationalInfo.bCashCountErrAsWarning ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("UseCorrectionFunction", (stOperationalInfo.bUseCorrectionFunction ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("DispErrAsWarning", (stOperationalInfo.bDispErrAsWarning ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("ShutterCheckBeforeDispense", (stOperationalInfo.bShutterCheckBeforeDispense ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("UseSNImageReadFunction", (stOperationalInfo.bUseSNImageReadFunction ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("StorDiffSizeNotesInDeposit", (stOperationalInfo.bStorDiffSizeNotesInDeposit ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("UseSNReadFuncton", (stOperationalInfo.bUseSNReadFuncton ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("CassMemoryOperation", (stOperationalInfo.bCassMemoryOperation ? "TRUE" : "FALSE"), iTabNum + 1);
}

//ST_HW_CONFIG
void TestUR2Driver::LogHWConfig(const ST_HW_CONFIG &stHWConfig, int iTabNum)
{
    LogOutputStructName("HWConfig", iTabNum);

    LogOutput("ETType", (stHWConfig.bETType    ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("HaveLane1", (stHWConfig.bHaveLane1 ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("HaveLane2", (stHWConfig.bHaveLane2 ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("HaveLane3", (stHWConfig.bHaveLane3 ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("HaveLane4", (stHWConfig.bHaveLane4 ? "TRUE" : "FALSE"), iTabNum + 1);
    LogOutput("HaveLane5", (stHWConfig.bHaveLane5 ? "TRUE" : "FALSE"), iTabNum + 1);
}

//ST_BV_VERIFICATION_LEVEL
void TestUR2Driver::LogBVVerificationLevel(const ST_BV_VERIFICATION_LEVEL &stBVVerificationLevel,
                                           int iTabNum)
{
    LogOutput("UnfitLevel",  EnumToString("UNFIT_LEVEL", stBVVerificationLevel.iUnfitLevel), iTabNum + 1);
    LogOutput("VerifiationLevel",  EnumToString("VERIFICATION_LEVEL", stBVVerificationLevel.iVerifiationLevel), iTabNum + 1);
}

//ST_MEDIA_INFORMATION_INFO
void TestUR2Driver::LogMediaInformationInfo(const ST_MEDIA_INFORMATION_INFO &totalMediaInfo,
                                            int iNum, int iTabNum)
{
    if (-1 == iNum)
    {
        LogOutputStructName("MediaInfo", iTabNum);
    }
    else
    {
        QString temp;
        temp.sprintf("MediaInfo[%d]", iNum);
        LogOutputStructName(qPrintable(temp), iTabNum);
    }

    QString szULong;
    szULong.sprintf("%lu", totalMediaInfo.ulBVInternalCounter);
    LogOutput("ulBVInternalCounter", szULong, iTabNum + 1);
    LogOutput("iMediaInfoOrigin", EnumToString("CASSETTE_ROOM_ID", totalMediaInfo.iMediaInfoOrigin), iTabNum + 1);
    LogOutput("iMediaInfoDest", EnumToString("CASSETTE_ROOM_ID", totalMediaInfo.iMediaInfoDest), iTabNum + 1);
    LogOutput("iDENOMINATION_CODE", EnumToString("DENOMINATION_CODE", totalMediaInfo.iMediaInfoDnoCode), iTabNum + 1);
    LogOutput("iBVImage", EnumToString("BV_IMAGE_TYPE", totalMediaInfo.iBVImage), iTabNum + 1);
    LogOutput("iMediaInfoRejectCause", EnumToString("MEDIA_INFORMATION_REJECT_CAUSE", totalMediaInfo.iMediaInfoRejectCause), iTabNum + 1);
}

//ST_TOTAL_STACKE_NOTES_DENO_INFO
void TestUR2Driver::LogTotalStackeNotesDenoInfo(const ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo, const char *szType,
                                                int iTabNum)
{
    if (strcmp(szType, "stStackeNotesDenoInfo") == 0)
        LogOutputStructName("StackeNotesDenoInfo", iTabNum);
    else if (strcmp(szType, "stStackeUnfitNotesDenoInfo") == 0)
        LogOutputStructName("StackeUnfitNotesDenoInfo", iTabNum);
    else if (strcmp(szType, "stStackeRejectNotesDenoInfo") == 0)
        LogOutputStructName("StackeRejectNotesDenoInfo", iTabNum);
    else if (strcmp(szType, "stStackeRejectNotesDenoSourInfo") == 0)
        LogOutputStructName("StackeRejectNotesDenoSourInfo", iTabNum);
    else if (strcmp(szType, "stStackeRejectNotesDenoBVInfo") == 0)
        LogOutputStructName("StackeRejectNotesDenoBVInfo", iTabNum);
    else if (strcmp(szType, "stStackeRejectNotesDenoDestInfo") == 0)
        LogOutputStructName("StackeRejectNotesDenoDestInfo", iTabNum);
    else
        LogOutputStructName("Default StackeNotesDenoInfo", iTabNum);


    LogOutput("Count",  QString::number(stStackeNotesDenoInfo.ucCount), iTabNum + 1);
    for (int i = 0; i < stStackeNotesDenoInfo.ucCount ; i++)
    {
        LogOutput("DENOCode",  QString::number(stStackeNotesDenoInfo.stStackeNotesInfo[i].ucDENOCode), iTabNum + 1);
        LogOutput("Dest",  EnumToString("CASSETTE_ROOM_ID", stStackeNotesDenoInfo.stStackeNotesInfo[i].iDest), iTabNum + 1);
        LogOutput("NumberStack",  QString::number(stStackeNotesDenoInfo.stStackeNotesInfo[i].usNumberStack), iTabNum + 1);
    }
}

//---------------------------槽函数-----------------------------------

void TestUR2Driver::GetLocalFW()
{
    ui->textEdit_UpdatePDL_LocalFW->clear();

    if (m_UR2Device == nullptr)
    {
        Log("更新固件前请先连接机芯");
        Log("*************************************************************");
        m_bGetLocalFWFlag = false;
        return;
    }

    char szFWVersion[MAX_FW_DATA_LENGTH] = {0};
    USHORT usLen = 0;
    int iRet = m_UR2Device->GetFWVersion(szFWVersion, usLen,  true);

    if (iRet < 0)
    {
        Log("获取当前固件版本失败");
        Log("*************************************************************");
        m_bGetLocalFWFlag = false;
        return;
    }

    AnalysisFWVersion(szFWVersion);

    for (int i = 0; i < m_strLocalFWVersion.length(); i++)
    {
        ui->textEdit_UpdatePDL_LocalFW->append(m_strLocalFWVersion[i]);
    }

    m_bGetLocalFWFlag = true;
}

void TestUR2Driver::GetFileFW()
{

}

void TestUR2Driver::SelectPDLFile()
{

}

void TestUR2Driver::show_widget(QTreeWidgetItem *item)
{
    int row = 0;//对应子节点行号
    if (!item)
        return;
    QTreeWidgetItem *phItem = item->parent();//获取当前item的父item
    if (!phItem) //说明为根节点
    {
        m_strSerName = item->text(0);
        row = -1;
        m_iCommandType = -1;
    }
    while (phItem)
    {
        m_strSerName = phItem->text(0);
        row = phItem->indexOfChild(item);
        phItem = phItem->parent();
    }

    //根据根节点名字和行号显示不同的命令界面
    if (m_strSerName == "初始化设置指令组")
    {
        if (row == -1)
        {
            ui->label_command_title->setText("初始化设置指令组");
            ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
        }
        else
        {
            switch (row)
            {
            case 0:
                m_iCommandType = getFWVersion;
                m_strCurrencyCommand.sprintf("固件版本信息");
                ui->label_command_title->setText("固件版本信息");
                ui->stackedWidget_command->setCurrentWidget(ui->GetFWVersion);
                break;
            //            case 1:
            //                m_iCommandType = setVerificationLevel;
            //                m_strCurrencyCommand.sprintf("设置校验级别");
            //                ui->label_command_title->setText("设置校验级别");
            //                ui->stackedWidget_command->setCurrentWidget(ui->SetVerificationLevel);
            //                break;
            case 1:
                m_iCommandType = getBanknoteInfo;
                m_strCurrencyCommand.sprintf("查询BV序列号及配置表");
                ui->label_command_title->setText("查询BV序列号及配置表");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 2:
                m_iCommandType = setDenominationCode;
                m_strCurrencyCommand.sprintf("设置面额配置代码");
                ui->label_command_title->setText("设置面额配置代码");

                if (m_DenoInfo.size() > 0)
                {
                    ui->tableWidget_SetDenominationCode->setRowCount(m_DenoInfo.size());
                    for (uint i = 0; i < m_DenoInfo.size(); i++)
                    {
                        m_lineEdit[i] = new QLineEdit;
                        ui->tableWidget_SetDenominationCode->setCellWidget(i, 2, m_lineEdit[i]);
                        m_lineEdit[i]->setInputMask("D");
                    }

                    QTableWidgetItem *item[MAX_DENOMINATION_NUM * 2];
                    QString tempStr;

                    map<int, DenoInfo>::const_iterator it = m_DenoInfo.begin();
                    for (uint i = 0; i < m_DenoInfo.size() && it != m_DenoInfo.end(); i++, it++)
                    {
                        item[i * 2] = new QTableWidgetItem;
                        tempStr.sprintf("%d", it->second.iDenoValue);
                        item[i * 2]->setText(tempStr);
                        ui->tableWidget_SetDenominationCode->setItem(i, 0, item[i * 2]);

                        item[i * 2 + 1] = new QTableWidgetItem;
                        if (it->second.ucVersion == 'A')
                        {
                            tempStr = "第四版";
                        }
                        else
                        {
                            tempStr = "第五版";
                        }
                        item[i * 2 + 1]->setText(tempStr);
                        ui->tableWidget_SetDenominationCode->setItem(i, 1, item[i * 2 + 1]);

                        tempStr.sprintf("%d", i + 1);
                        m_lineEdit[i]->setText(tempStr);
                    }
                }

                ui->stackedWidget_command->setCurrentWidget(ui->SetDenominationCode);
                break;
            case 3:
                m_iCommandType = getCassetteInfo;
                m_strCurrencyCommand.sprintf("查询钞箱信息");
                ui->label_command_title->setText("查询钞箱信息");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 4:
                m_iCommandType = setUnitInfo;
                m_strCurrencyCommand.sprintf("设置HCM钞箱配置信息");
                ui->label_command_title->setText("设置HCM钞箱配置信息");
                ui->stackedWidget_command->setCurrentWidget(ui->SetUnitInfo);
                break;
            case 5:
                m_iCommandType = getUnitInfo;
                m_strCurrencyCommand.sprintf("获取HCM钞箱配置信息");
                ui->label_command_title->setText("获取HCM钞箱配置信息");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 6:
                m_iCommandType = initReset;
                m_strCurrencyCommand.sprintf("复位");
                ui->label_command_title->setText("复位");
                ui->stackedWidget_command->setCurrentWidget(ui->Reset);
                break;
            default:
                break;
            }
        }

    }

    else if (m_strSerName == "查询类指令组")
    {
        if (row == -1)
        {
            ui->label_command_title->setText("查询类指令组");
            ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
        }
        else
        {
            switch (row)
            {
            case 0:
                m_iCommandType = getBanknoteInfo;
                m_strCurrencyCommand.sprintf("查询BV序列号及配置表");
                ui->label_command_title->setText("查询BV序列号及配置表");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 1:
                m_iCommandType = queryBVStatusSense;
                m_strCurrencyCommand.sprintf("查询BV模块信息");
                ui->label_command_title->setText("查询BV模块信息");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 2:
                m_iCommandType = getDevStatusInfo;
                m_strCurrencyCommand.sprintf("获得设备状态信息");
                ui->label_command_title->setText("获得设备状态信息");
                ui->stackedWidget_command->setCurrentWidget(ui->GetDevStatusInfo);
                break;
            case 3:
                m_iCommandType = getNoteSeiralInfo;
                m_strCurrencyCommand.sprintf("获取冠字码相关信息");
                ui->label_command_title->setText("获取冠字码相关信息");

                if (m_iBVCount > 0)
                {
                    QString temp;
                    temp.sprintf("BV获取到的钞票总数为：%d", m_iBVCount);
                    ui->label_GetNoteSeiralInfo_SNNum->setText(temp);
                }
                else
                {
                    ui->label_GetNoteSeiralInfo_SNNum->setText("BV未检测到钞票");
                }

                ui->stackedWidget_command->setCurrentWidget(ui->GetNoteSeiralInfo);
                break;
            case 4:
                m_iCommandType = getMediaInformation;
                m_strCurrencyCommand.sprintf("获取钞票的流向面额信息");
                ui->label_command_title->setText("获取钞票的流向面额信息");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 5:
                m_iCommandType = getCassetteInfo;
                m_strCurrencyCommand.sprintf("查询钞箱信息");
                ui->label_command_title->setText("查询钞箱信息");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 6:
                m_iCommandType = getUnitInfo;
                m_strCurrencyCommand.sprintf("获取HCM钞箱配置信息");
                ui->label_command_title->setText("获取HCM钞箱配置信息");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 7:
                m_iCommandType = getAllSersorLight;
                m_strCurrencyCommand.sprintf("读取所有感应器的亮灭状态");
                ui->label_command_title->setText("读取所有感应器的亮灭状态");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            default:
                break;
            }
        }
    }

    else if (m_strSerName == "控制指令组")
    {
        if (row == -1)
        {
            ui->label_command_title->setText("控制指令组");
            ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
        }
        else
        {
            switch (row)
            {
            case 0:
                m_iCommandType = cashCount;
                m_strCurrencyCommand.sprintf("点钞");
                ui->label_command_title->setText("点钞");
                ui->stackedWidget_command->setCurrentWidget(ui->CashCount);
                break;
            case 1:
                m_iCommandType = cashCountRetract;
                m_strCurrencyCommand.sprintf("点钞回收");
                ui->label_command_title->setText("点钞回收");
                ui->stackedWidget_command->setCurrentWidget(ui->CashCountRetract);
                break;
            case 2:
                m_iCommandType = storeMoney;
                m_strCurrencyCommand.sprintf("存入钞票");
                ui->label_command_title->setText("存入钞票");
                ui->stackedWidget_command->setCurrentWidget(ui->StoreMoney);
                break;
            case 3:
                m_iCommandType = cashRollback;
                m_strCurrencyCommand.sprintf("取消存款");
                ui->label_command_title->setText("取消存款");
                ui->stackedWidget_command->setCurrentWidget(ui->CashRollback);
                break;
            case 4:
                m_iCommandType = dispense;
                m_strCurrencyCommand.sprintf("挖钞");
                ui->label_command_title->setText("挖钞");
                ui->stackedWidget_command->setCurrentWidget(ui->Dispense);
                break;
            case 5:
                m_iCommandType = rejcet;
                m_strCurrencyCommand.sprintf("出钞后回收取款拒钞");
                ui->label_command_title->setText("出钞后回收取款拒钞");
                ui->stackedWidget_command->setCurrentWidget(ui->Rejcet);
                break;
            case 6:
                m_iCommandType = retractESC;
                m_strCurrencyCommand.sprintf("回收ESC中的钞票");
                ui->label_command_title->setText("回收ESC中的钞票");
                ui->stackedWidget_command->setCurrentWidget(ui->RetractESC);
                break;
            case 7:
                m_iCommandType = closeShutter;
                m_strCurrencyCommand.sprintf("关闭Shutter门");
                ui->label_command_title->setText("关闭Shutter门");
                ui->stackedWidget_command->setCurrentWidget(ui->CloseShutter);
                break;
            case 8:
                m_iCommandType = openShutter;
                m_strCurrencyCommand.sprintf("打开Shutter门");
                ui->label_command_title->setText("打开Shutter门");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 9:
                m_iCommandType = bvCommStart;
                m_strCurrencyCommand.sprintf("PC与ZERO BV通信开始");
                ui->label_command_title->setText("PC与ZERO BV通信开始");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 10:
                m_iCommandType = bvCommEnd;
                m_strCurrencyCommand.sprintf("PC与ZERO BV通信结束");
                ui->label_command_title->setText("PC与ZERO BV通信结束");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 11:
                m_iCommandType = initReset;
                m_strCurrencyCommand.sprintf("复位");
                ui->label_command_title->setText("复位");
                ui->stackedWidget_command->setCurrentWidget(ui->Reset);
                break;
            default:
                break;
            }
        }
    }

    else if (m_strSerName == "维护指令组")
    {
        if (row == -1)
        {
            ui->label_command_title->setText("维护指令组");
            ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
        }
        else
        {
            switch (row)
            {
            case 0:
                m_iCommandType = reboot;
                m_strCurrencyCommand.sprintf("断开连接并重启");
                ui->label_command_title->setText("断开连接并重启");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 1:
                m_iCommandType = startTransaction;
                m_strCurrencyCommand.sprintf("开始存取款交易");
                ui->label_command_title->setText("开始存取款交易");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 2:
                m_iCommandType = preNextTransaction;
                m_strCurrencyCommand.sprintf("准备进入一下笔交易");
                ui->label_command_title->setText("准备进入一下笔交易");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 3:
                m_iCommandType = setStatusChangeListener;
                m_strCurrencyCommand.sprintf("设置状态改变侦听接口");
                ui->label_command_title->setText("设置状态改变侦听接口");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 4:
                m_iCommandType = userMemoryWrite;
                m_strCurrencyCommand.sprintf("写数据到用户内存区域");
                ui->label_command_title->setText("写数据到用户内存区域");
                ui->stackedWidget_command->setCurrentWidget(ui->UserMemoryWrite);
                break;
            case 5:
                m_iCommandType = userMemoryRead;
                m_strCurrencyCommand.sprintf("读取用户内存的数据");
                ui->label_command_title->setText("读取用户内存的数据");
                ui->stackedWidget_command->setCurrentWidget(ui->UserMemoryRead);
                break;
            default:
                break;
            }
        }
    }

    else if (m_strSerName == "其他指令组")
    {
        if (row == -1)
        {
            ui->label_command_title->setText("其他指令组");
            ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
        }
        else
        {
            switch (row)
            {
            case 0:
                m_iCommandType = getLogData;
                m_strCurrencyCommand.sprintf("请求日志数据");
                ui->label_command_title->setText("请求日志数据");
                ui->stackedWidget_command->setCurrentWidget(ui->GetLogData);
                break;
            case 1:
                m_iCommandType = eraseAllLogData;
                m_strCurrencyCommand.sprintf("清除日志数据");
                ui->label_command_title->setText("清除日志数据");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 2:
                m_iCommandType = closeUSBConn;
                m_strCurrencyCommand.sprintf("关闭连接");
                ui->label_command_title->setText("关闭连接");
                ui->stackedWidget_command->setCurrentWidget(ui->NoParameter);
                break;
            case 3:
                m_iCommandType = updatePDL;
                m_strCurrencyCommand.sprintf("更新固件");
                ui->label_command_title->setText("更新固件");
                ui->stackedWidget_command->setCurrentWidget(ui->UpdatePDL);
                break;
            default:
                break;
            }
        }
    }
}

void TestUR2Driver::on_action_connect_triggered()
{
    if (m_bUpdate)
    {
        QMessageBox::information(this, "提示", "机芯升级固件中...");
        return;
    }

    int ret = 0;
    ret = m_UR2Device.Load("libDevUR2.so", "CreateURDevice");
    if (ret != 0)
    {
        QMessageBox::information(this, "Error", "驱动加载失败!");
        return;
    }

    ret = m_UR2Device->OpenURConn();

    if (ERR_UR_SUCCESS != ret)
    {
        m_UR2Device.Release();
        QMessageBox::information(this, "error", "打开UR连接失败!");
        return;
    }

    ret = m_UR2Device->OpenBVConn();

    if (ERR_UR_SUCCESS != ret)
    {
        m_UR2Device.Release();
        QMessageBox::information(this, "error", "打开BV连接失败!");
        return;
    }
    //m_UR2Device->SetWaitCallback();

    Log("连接机芯成功!");
    Log("*************************************************************");
}

void TestUR2Driver::on_action_close_triggered()
{
    if (m_bUpdate)
    {
        QMessageBox::information(this, "提示", "机芯升级固件中...");
        return ;
    }

    if (m_UR2Device == nullptr)
    {
        QMessageBox::information(this, "提示", "机芯未连接，请先连接机芯!");
        return ;
    }

    int ret = m_UR2Device->CloseURConn();

    if (ERR_UR_SUCCESS != ret)
    {
        QMessageBox::information(this, "error", "关闭UR连接失败!");
        return ;
    }

    ret = m_UR2Device->CloseBVConn();

    if (ERR_UR_SUCCESS != ret)
    {
        QMessageBox::information(this, "error", "关闭BV连接失败!");
        return ;
    }

    Release();

    Log("断开连接成功!");
    Log("*************************************************************");
}

void TestUR2Driver::ClearInfo()
{
    m_list_log->clear();
}

void TestUR2Driver::on_pushButton_exec_clicked()
{
    if (m_bUpdate)
    {
        QMessageBox::information(this, "提示", "机芯升级固件中...");
        return;
    }

    if (m_UR2Device == nullptr)
    {
        QMessageBox::information(this, "错误", "执行命令前请连接机芯!");
        return ;
    }

    LogBeforeExecute();

    switch (m_iCommandType)
    {
    //初始化设置指令组
    case getFWVersion:
        OnGetFWVersion();
        break;
    case setVerificationLevel:
        OnSetVerificationLevel();
        break;
    case getBanknoteInfo:
        OnGetBanknoteInfo();
        break;
    case setDenominationCode:
        OnSetDenominationCode();
        break;
    case getCassetteInfo:
        OnGetCassetteInfo();
        break;
    case setUnitInfo:
        OnSetUnitInfo();
        break;
    case getUnitInfo:
        OnGetUnitInfo();
        break;
    case initReset:
        OnInitReset();
        break;
    //查询指令组
    case queryBVStatusSense:
        OnQueryBVStatusSense();
        break;
    case getDevStatusInfo:
        OnGetDevStatusInfo();
        break;
    case getNoteSeiralInfo:
        OnGetNoteSeiralInfo();
        break;
    case getMediaInformation:
        OnGetMediaInformation();
        break;
    case getAllSersorLight:
        OnGetAllSersorLight();
        break;
    //控制指令组
    case cashCount:
        OnCashCount();
        break;
    case cashCountRetract:
        OnCashCountRetract();
        break;
    case storeMoney:
        OnStoreMoney();
        break;
    case cashRollback:
        OnCashRollback();
        break;
    case dispense:
        OnDispense();
        break;
    case rejcet:
        OnReject();
        break;
    case retractESC:
        OnRetractESC();
        break;
    case closeShutter:
        OnCloseShutter();
        break;
    case openShutter:
        OnOpenShutter();
        break;
    case bvCommEnd:
        OnBVCommEnd();
        break;

    //维护指令组
    case reboot:
        OnReboot();
        break;
    case startTransaction:
        OnStartTransaction();
        break;
    case preNextTransaction:
        OnPreNextTransaction();
        break;
    case setStatusChangeListener:
        OnSetStatusChangeListener();
        break;
    case bvCommStart:
        OnBVCommStart();
        break;
    case userMemoryWrite:
        OnUserMemoryWrite();
        break;
    case userMemoryRead:
        OnUserMemoryRead();
        break;
    //其他指令组
    case getLogData:
        OnGetLogData();
        break;
    case eraseAllLogData:
        OnEraseAllLogData();
        break;
    case closeUSBConn:
        OnCloseUSBConn();
        break;
    case updatePDL:
        OnUpdatePDL();
        break;

    default:
        break;
    }

}

bool TestUR2Driver::SetCtrlValue(QString strType, QString strCtrlName, QString strValue)
{
    if ("comboBox" == strType)
    {
        // 在combobox中搜索字符串，返回Index值
        if (findChild<QComboBox *>(strCtrlName) != nullptr)
        {
            int iIndex = findChild<QComboBox *>(strCtrlName)->findText(strValue);
            if (-1 == iIndex)
            {
                return FALSE;
            }

            findChild<QComboBox *>(strCtrlName)->setCurrentIndex(iIndex);
        }
    }
    else if ("lineEdit" == strType)
    {
        if (findChild<QLineEdit *>(strCtrlName) != nullptr)
            findChild<QLineEdit *>(strCtrlName)->setText(strValue);
    }
    else if ("textEdit" == strType)
    {
        if (findChild<QTextEdit *>(strCtrlName) != nullptr)
            findChild<QTextEdit *>(strCtrlName)->setText(strValue);
    }
    else if ("checkBox" == strType)
    {
        if ("TRUE" == strValue)
        {
            if (findChild<QCheckBox *>(strCtrlName) != nullptr)
                findChild<QCheckBox *>(strCtrlName)->setChecked(TRUE);
        }
        else if ("FALSE" == strValue)
        {
            if (findChild<QCheckBox *>(strCtrlName) != nullptr)
                findChild<QCheckBox *>(strCtrlName)->setChecked(FALSE);
        }
        else
        {
            return FALSE;
        }
    }
    else if ("spinBox" == strType)
    {
        if (findChild<QSpinBox *>(strCtrlName) != nullptr)
            findChild<QSpinBox *>(strCtrlName)->setValue(strValue.toInt());
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

void TestUR2Driver::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton exitButton;
    exitButton = QMessageBox::question(this, "退出程序", QString("确认退出程序?"),
                                       QMessageBox::Yes | QMessageBox::No);
    if (exitButton == QMessageBox::No)
    {
        event->ignore();  //忽略退出信号，程序继续运行
    }
    else if (exitButton == QMessageBox::Yes)
    {
        event->accept();  //接受退出信号，程序退出
    }
}

//---------------------------初始化命令-----------------------------------

void TestUR2Driver::OnGetFWVersion()
{
    //输入参数
    bool bNeedInitial = ui->checkBox_GetFWVersion_bNeedInitial->isChecked();

    //输出参数
    char arryFWVersion[MAX_FW_DATA_LENGTH] = {0};
    USHORT usLen = 0;
    int iError = m_UR2Device->GetFWVersion(arryFWVersion, usLen, bNeedInitial);

    LogAfterExecute(iError, true);

    if (iError >= 0)
    {
        AnalysisFWVersion(arryFWVersion);
    }

    LogEnd(iError);
}

void TestUR2Driver::OnSetVerificationLevel()
{
    //输入参数
    SET_VERIFICATION_LEVEL stSetVerificationLevel;
    stSetVerificationLevel.bSetForCashCount =
    ui->checkBox_SetVerificationLevel_bSetForCashCount->isChecked() ? TRUE : FALSE;
    stSetVerificationLevel.bSetForStoreMoney =
    ui->checkBox_SetVerificationLevel_bSetForStoreMoney->isChecked() ? TRUE : FALSE;
    stSetVerificationLevel.bSetForDispense =
    ui->checkBox_SetVerificationLevel_bSetForDispense->isChecked() ? TRUE : FALSE;

    int iError = m_UR2Device->SetVerificationLevel(stSetVerificationLevel);

    LogAfterExecute(iError);

    LogEnd(iError);
}

void TestUR2Driver::OnGetBanknoteInfo()
{
    m_DenoInfo.clear();

    //输出参数
    ST_BV_INFO pBVInfo;
    ST_DENO_INFO pArryDenoInfo[MAX_DENOMINATION_NUM];

    memset(&pBVInfo, 0, sizeof(ST_BV_INFO));
    memset(pArryDenoInfo, 0, sizeof(ST_DENO_INFO) * MAX_DENOMINATION_NUM);

    int iError = m_UR2Device->GetBanknoteInfo(pBVInfo, pArryDenoInfo);

    LogAfterExecute(iError, true);

    int iIndex = 0;
    if (iError >= 0)
    {
        LogBVInfo(pBVInfo);
        for (int i = 0; i < MAX_DENOMINATION_NUM; i++)
        {
            if (0 != pArryDenoInfo[i].iCashValue)
            {
                LogDenoInfo(pArryDenoInfo[i], i);
                DenoInfo denoInfo;
                denoInfo.iDenoValue = pArryDenoInfo[i].iCashValue;
                denoInfo.ucVersion = pArryDenoInfo[i].ucVersion;
                m_DenoInfo[iIndex++] = denoInfo;
            }
        }
    }

    LogEnd(iError);
}

void TestUR2Driver::OnSetDenominationCode()
{
    if (m_DenoInfo.size() <= 0)
    {
        Log("设置面额代码前请先获取配置表");
        Log("*************************************************************");
        return;
    }

    for (uint i = 0; i < m_DenoInfo.size(); i++)
    {
        if (m_lineEdit[i]->text().isEmpty())
        {
            Log("参数设置错误，NoteID不能留空");
            Log("*************************************************************");
            return;
        }
    }

    //输入参数
    char DenoCode[MAX_DENOMINATION_NUM] = {0};

    for (uint i = 0; i < m_DenoInfo.size(); i++)
    {
        DenoCode[i] = m_lineEdit[i]->text().toInt();
    }

    int iError = m_UR2Device->SetDenominationCode(DenoCode);

    LogAfterExecute(iError, false);

    LogEnd(iError);
}

void TestUR2Driver::OnGetCassetteInfo()
{
    //输出参数
    ST_CASSETTE_INFO pArryCassInfo[MAX_CASSETTE_NUM];
    memset(pArryCassInfo, 0, sizeof(ST_CASSETTE_INFO) * MAX_CASSETTE_NUM);

    int iError = m_UR2Device->GetCassetteInfo(pArryCassInfo);

    LogAfterExecute(iError, true);

    if (iError >= 0)
    {
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            LogOutputStructName("ST_CASSETTE_INFO", 1);

            LogOutput("CassNO", EnumToString("CASSETTE_NUMBER", pArryCassInfo[i].iCassNO), 2);
            LogOutput("CassType", EnumToString("CASSETTE_TYPE", pArryCassInfo[i].iCassType), 2);
            LogOutput("CassOper", EnumToString("CASSETTE_OPERATION", pArryCassInfo[i].iCassOper), 2);
            LogOutput("CurrencyCode", EnumToString("CURRENCY_CODE", pArryCassInfo[i].iCurrencyCode), 2);
            LogOutput("CashValue", QString::number(pArryCassInfo[i].iCashValue), 2);
            LogOutput("Version", UCharToASCII(pArryCassInfo[i].ucVersion), 2);
            LogOutput("IssuingBank", UCharToASCII(pArryCassInfo[i].ucIssuingBank), 2);
        }
    }

    LogEnd(iError);
}

void TestUR2Driver::OnSetUnitInfo()
{
    //输入参数
    SYSTEMTIME stSystemTime;
    CQtTime::GetLocalTime(stSystemTime);
    int stTotalCashInURJB = ui->lineEdit_SetUnitInfo_iTotalCashInURJB->text().toInt();
    if (stTotalCashInURJB >= 0x8000)
    {
        QMessageBox::warning(this, "参数有误", "参数有误，stTotalCashInURJB超出范围！");
        return;
    }

    ST_OPERATIONAL_INFO stOperationalInfo;
    GetSTOperationalInfo(stOperationalInfo);

    ST_HW_CONFIG stHWConfig;
    stHWConfig.bHaveLane1 = ui->checkBox_SetUnitInfo_bHaveLane1->isChecked();
    stHWConfig.bHaveLane2 = ui->checkBox_SetUnitInfo_bHaveLane2->isChecked();
    stHWConfig.bHaveLane3 = ui->checkBox_SetUnitInfo_bHaveLane3->isChecked();
    stHWConfig.bHaveLane4 = ui->checkBox_SetUnitInfo_bHaveLane4->isChecked();
    stHWConfig.bHaveLane5 = ui->checkBox_SetUnitInfo_bHaveLane5->isChecked();

    char pTempCtrlName[MAX_SIZE] = {0};
    char usArryCassCashInPrioritySet[MAX_CASSETTE_NUM] = {0};
    char *pArryCassCashInPrioritySet = usArryCassCashInPrioritySet;
    char usArryCassCashOutPrioritySet[MAX_CASSETTE_NUM] = {0};
    char *pArryCassCashOutPrioritySet = usArryCassCashOutPrioritySet;
    if (ui->checkBox_SetUnitInfo_EnableParam->isChecked())
    {
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            sprintf(pTempCtrlName, "spinBox_SetUnitInfo_Cassette%d_usArryCassCashInPrioritySet", i + 1);
            usArryCassCashInPrioritySet[i] = findChild<QSpinBox *>(pTempCtrlName)->value();

            sprintf(pTempCtrlName, "spinBox_SetUnitInfo_Cassette%d_usArryCassCashOutPrioritySet", i + 1);
            usArryCassCashOutPrioritySet[i] = findChild<QSpinBox *>(pTempCtrlName)->value();
        }
    }
    else
    {
        pArryCassCashInPrioritySet = nullptr;
        pArryCassCashOutPrioritySet = nullptr;
    }

    ST_BV_VERIFICATION_LEVEL stCashCountLevel;
    stCashCountLevel.iUnfitLevel =
    (UNFIT_LEVEL)ui->comboBox_SetUnitInfo_stCashCountLevel_iUnfitLevel->currentIndex();
    stCashCountLevel.iVerifiationLevel =
    (VERIFICATION_LEVEL)ui->comboBox_SetUnitInfo_stCashCountLevel_iVerifiationLevel->currentIndex();

    ST_BV_VERIFICATION_LEVEL stStoreMoneyLevel;
    stStoreMoneyLevel.iUnfitLevel =
    (UNFIT_LEVEL)ui->comboBox_SetUnitInfo_stStoreMoneyLevel_iUnfitLevel->currentIndex();
    stStoreMoneyLevel.iVerifiationLevel =
    (VERIFICATION_LEVEL)ui->comboBox_SetUnitInfo_stStoreMoneyLevel_iVerifiationLevel->currentIndex();

    ST_BV_VERIFICATION_LEVEL stDispenseLevel;
    stDispenseLevel.iUnfitLevel =
    (UNFIT_LEVEL)ui->comboBox_SetUnitInfo_stDispenseLevel_iUnfitLevel->currentIndex();
    stDispenseLevel.iVerifiationLevel =
    (VERIFICATION_LEVEL)ui->comboBox_SetUnitInfo_stDispenseLevel_iVerifiationLevel->currentIndex();

    ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM];
    memset(stCassType, 0, sizeof(ST_CASSETTE_INFO) * MAX_CASSETTE_NUM);
    GetCassetteInfo(stCassType);

    BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM] = {0};
    BOOL *pArryAcceptDenoCode = bArryAcceptDenoCode;
    if (ui->checkBox_SetUnitInfo_bArryAcceptDenoCode->isChecked())
    {
        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)
        {
            bArryAcceptDenoCode[j] = 0;
        }
        for (int j = 0; j < MAX_DENO_LENGTH; j++)
        {
            sprintf(pTempCtrlName, "spinBox_SetUnitInfo_bArryAcceptDenoCode%d", j + 1);
            bArryAcceptDenoCode[j] = findChild<QSpinBox *>(pTempCtrlName)->value();
        }
    }
    else
    {
        pArryAcceptDenoCode = NULL;
    }

    int iError = m_UR2Device->SetUnitInfo(stTotalCashInURJB, /*stSystemTime,*/ stOperationalInfo,
                                          stHWConfig, stCassType, pArryAcceptDenoCode,
                                          stCashCountLevel, stStoreMoneyLevel, stDispenseLevel,
                                          pArryCassCashInPrioritySet, pArryCassCashOutPrioritySet);

    LogAfterExecute(iError, false);

    LogEnd(iError);
}

void TestUR2Driver::OnGetUnitInfo()
{
    //输出参数
    int iTotalCashInURJB = 0;
    BOOL bArryDenoCodeBySet[MAX_DENOMINATION_NUM] = {0};
    BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM] = {0};
    ST_OPERATIONAL_INFO stOperationalInfo;
    ST_HW_CONFIG stHWConfig;
    ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM];
    ST_BV_VERIFICATION_LEVEL stCashCountLevel;
    ST_BV_VERIFICATION_LEVEL stStoreMoneyLevel;
    ST_BV_VERIFICATION_LEVEL stDispenseLevel;

    memset(stCassType, 0, sizeof(ST_CASSETTE_INFO) * MAX_CASSETTE_NUM);

    int iError = m_UR2Device->GetUnitInfo(iTotalCashInURJB, bArryDenoCodeBySet, bArryAcceptDenoCode,
                                          stOperationalInfo, stHWConfig, stCassType, stCashCountLevel,
                                          stStoreMoneyLevel, stDispenseLevel);

    LogAfterExecute(iError, true);

    if (iError >= 0)
    {
        LogOutput("TotalCashInURJB", QString::number(iTotalCashInURJB));
        LogOutput("ArryDenoCodeBySet", BOOLArrayToChar(bArryDenoCodeBySet, MAX_DENOMINATION_NUM));
        LogOutput("ArryAcceptDenoCode", BOOLArrayToChar(bArryAcceptDenoCode, MAX_DENOMINATION_NUM));
        LogOperationalInfo(stOperationalInfo);
        LogHWConfig(stHWConfig);
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            LogCassetteInfo(stCassType[i]);
        }

        Log("CashCountLevel");
        LogBVVerificationLevel(stCashCountLevel);
        Log("StoreMoneyLevel");
        LogBVVerificationLevel(stStoreMoneyLevel);
        Log("DispenseLevel");
        LogBVVerificationLevel(stDispenseLevel);
    }

    LogEnd(iError);
}

void TestUR2Driver::OnInitReset()
{
    m_iBVCount = 0;
    // 输入参数
    BOOL bCancelEnergySaveMode = ui->checkBox_InitReset_bCancelEnergySaveMode->isChecked();
    BOOL bQuickRest = ui->checkBox_InitReset_bQuickRest->isChecked();

    // 输出参数
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};

    int iError = m_UR2Device->Reset(iTotalCashInURJB,
                                    iNumStackedToCS, iNumStackedToESC, pNumStackedToPerCass,
                                    iNumCSFed, iNumESCFed, pNumPerCassFed, pMaintenanceInfo,
                                    bCancelEnergySaveMode, bQuickRest);

    LogAfterExecute(iError, true);

    //if (iError >= 0)
    {
        LogOutput("TotalCashInURJB", QString::number(iTotalCashInURJB));
        LogOutput("NumStackedToCS", QString::number(iNumStackedToCS));
        LogOutput("NumStackedToESC", QString::number(iNumStackedToESC));

        QString tempName;
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumStackedToCass[%d]", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumStackedToPerCass[i]));
        }
        LogOutput("NumStackedToURJB", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogOutput("NumCSFed", QString::number(iNumCSFed));
        LogOutput("NumESCFed", QString::number(iNumESCFed));

        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumCassFed[%d]", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumPerCassFed[i]));
        }
        LogOutput("NumFedURJB", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        m_iBVCount = iNumStackedToCS + iNumStackedToESC;
    }

    LogEnd(iError);
}


//---------------------------查询命令-----------------------------------
void TestUR2Driver::OnQueryBVStatusSense()
{
    // 输出参数
    ST_BV_WARNING_INFO stBVWarningInfo[MAX_BV_WARNING_INFO_NUM];
    memset(stBVWarningInfo, 0, sizeof(ST_BV_WARNING_INFO) * MAX_BV_WARNING_INFO_NUM);

    int iError = m_UR2Device->QueryBVStatusSense(stBVWarningInfo);

    LogAfterExecute(iError, true);

    if (iError >= 0)
    {
        for (int i = 0; i < MAX_BV_WARNING_INFO_NUM ; i++)
        {
            QString tempString;
            tempString.sprintf("BVWarningInfo[%d]", i);
            Log(tempString);
            LogOutput("WaringCode",  QString::number(stBVWarningInfo[i].iWaringCode));
            LogOutput("RecoveryCode", QString::number(stBVWarningInfo[i].usRecoveryCode));
            QString temp;
            temp.sprintf("%s", stBVWarningInfo[i].ucPositionCode);
            LogOutput("PositionCode", temp);
        }
    }

    LogEnd(iError);
}

void TestUR2Driver::OnGetDevStatusInfo()
{
    // 输出参数
    ST_DEV_STATUS_INFO stDevStatusInfo;

    m_UR2Device->GetDevStatusInfo(stDevStatusInfo);

    LogAfterExecute(0, true);

    LogDevStatusInfo(stDevStatusInfo);

    LogEnd(0);
}

void TestUR2Driver::OnGetNoteSeiralInfo()
{
    // BV验钞数检测
    if (m_iBVCount <= 0)
    {
        QMessageBox::warning(this, "参数有误", "BV没有检测到钞票信息，无法获取冠字码!");
        return;
    }

    int iCount = 1;
    int iError = 0;

    // 输入参数
    DWORD dwIndex = 0;

    // 输出参数
    SNOTESERIALINFO lpNoteSerialInfo[m_iBVCount];
    memset(lpNoteSerialInfo, 0, sizeof(SNOTESERIALINFO) * m_iBVCount);

    SNOTESERIALINFOFULLIMAGE lpNoteSerialInfoFullImage[m_iBVCount * 2];
    memset(lpNoteSerialInfoFullImage, 0, sizeof(SNOTESERIALINFOFULLIMAGE) * m_iBVCount * 2);


    if (!ui->checkBox_GetNoteSeiralInfo_All->isChecked())
    {
        // 输入参数
        int iIndex = ui->lineEdit_GetNoteSeiralInfo_dwIndex->text().toInt();
        if (!ui->checkBox_GetNoteSeiralInfo_FullImage->isChecked())
        {
            if (iIndex < 0 || iIndex >= m_iBVCount)
            {
                QMessageBox::warning(this, "参数有误", "dwIndex小于0或者大于BV检测到的钞票总数!");
                return;
            }
        }
        else
        {
            if (iIndex < 0 || iIndex >= m_iBVCount * 2)
            {
                QMessageBox::warning(this, "参数有误", "dwIndex小于0或者大于BV检测到的钞票总数!");
                return;
            }
        }

        dwIndex = iIndex;
    }
    else
    {
        iCount = m_iBVCount;
    }

    //非全幅
    if (!ui->checkBox_GetNoteSeiralInfo_FullImage->isChecked())
    {
        for (int i = 0; i < iCount; i++)
        {
            if (ui->checkBox_GetNoteSeiralInfo_All->isChecked())
            {
                iError = m_UR2Device->GetNoteSeiralInfo(i, &lpNoteSerialInfo[i]);
            }
            else
            {
                iError = m_UR2Device->GetNoteSeiralInfo(dwIndex, &lpNoteSerialInfo[i]);
            }

            if (i == 0)
                LogAfterExecute(iError, true);

            QString tempString;
            tempString.sprintf("NoteSerialInfo[%d]", i);
            Log(tempString);

            if (iError >= 0)
            {
                QString tempStr;
                tempStr.sprintf("%lu", lpNoteSerialInfo[i].dwBVInternalIndex);
                LogOutput("BVInternalIndex", tempStr);
                tempStr.sprintf("%lu", lpNoteSerialInfo[i].dwImgDataLen);
                LogOutput("ImgDataLen", tempStr);
                LogOutput("SerialNumber", lpNoteSerialInfo[i].cSerialNumber);

                if (lpNoteSerialInfo[i].dwImgDataLen > 0)
                {
                    CreateSerialImage(lpNoteSerialInfo[i], i);
                }
                else
                {
                    CreateDefaultImage(i);
                }

                LogOutput("NotesEdition", EnumToString("eNoteEdition", lpNoteSerialInfo[i].NotesEdition));
            }
        }
    }
    else
    {
        for (int i = 0; i < iCount * 2; i++)
        {
            if (ui->checkBox_GetNoteSeiralInfo_All->isChecked())
            {
                iError = m_UR2Device->GetNoteSeiralInfoFullImage(i, &lpNoteSerialInfoFullImage[i]);
            }
            else
            {
                iError = m_UR2Device->GetNoteSeiralInfoFullImage(dwIndex, &lpNoteSerialInfoFullImage[i]);
            }

            if (i == 0)
                LogAfterExecute(iError, true);

            QString tempString;
            tempString.sprintf("NoteSerialInfoFullImage[%d]", i);
            Log(tempString);

            if (iError >= 0)
            {
                QString tempStr;
                tempStr.sprintf("%lu", lpNoteSerialInfoFullImage[i].dwBVInternalIndex);
                LogOutput("BVInternalIndex", tempStr);
                tempStr.sprintf("%lu", lpNoteSerialInfoFullImage[i].dwImgDataLen);
                LogOutput("ImgDataLen", tempStr);


                if (lpNoteSerialInfoFullImage[i].dwImgDataLen > 0)
                {
                    CreateSerialImage(lpNoteSerialInfoFullImage[i], i);
                }
                else
                {
                    CreateDefaultImage(i);
                }
            }
        }
    }
    LogEnd(iError);
}

void TestUR2Driver::OnGetMediaInformation()
{
    // 输入参数
    USHORT usNumNotes = 0;

    // 输出参数
    USHORT usNumTotalNotes = 0;
    ST_MEDIA_INFORMATION_INFO arryMediaInfo[MAX_MEDIA_INFO_NUM];
    ST_MEDIA_INFORMATION_INFO totalMediaInfo[MAX_MDEIA_DATALENGTH];

    USHORT usNumNotesTotal = 0;
    int iError = 0;
    int iIndex = 0;
    int i = 1;
    do
    {
        usNumNotes = 0;
        iError = m_UR2Device->GetMediaInformation(i, usNumNotes, usNumTotalNotes, arryMediaInfo);
        if (iError < 0)
        {
            LogAfterExecute(iError, true);
            LogEnd(iError);
            return ;
        }

        for (int j = 0; j < usNumNotes; j++)
        {
            totalMediaInfo[iIndex++] = arryMediaInfo[j];
        }


        usNumNotesTotal += usNumNotes;
        if (usNumNotesTotal == usNumTotalNotes)
            break;

        i++;
    } while (iError > 0);


    LogAfterExecute(iError, true);

    if (iError >= 0)
    {
        for (int i = 0; i < usNumTotalNotes; i++)
        {
            LogMediaInformationInfo(totalMediaInfo[i], i + 1);
        }
    }

    LogEnd(iError);
}

void TestUR2Driver::OnGetAllSersorLight()
{
    // 输入参数
    char szAllSersorLight[58 + 1] = {0};

    // 输出参数
    int nAllSensorLightDataLen = 0;
    int iError = m_UR2Device->ReadAllSensorLight(szAllSersorLight, nAllSensorLightDataLen);
    if (iError < 0)
    {
        LogAfterExecute(iError, true);
        LogEnd(iError);
        return ;
    }

    UCHAR ucData = 0;
    char szLightData[9] = {0};
    char szByteNum[10] = {0};
    for (int j = 0; j < nAllSensorLightDataLen; j++)
    {
        memset(szLightData, 0, sizeof(szLightData));
        ucData = szAllSersorLight[j];
        for (int i = 0; i < 8; i++)
        {
            if (ucData & (1 << i))
            {
                szLightData[7 - i] = '1';
            }
            else
            {
                szLightData[7 - i] = '0';
            }
        }
        sprintf(szByteNum, "Block %02d: ", j + 1);
        LogOutput(szByteNum, szLightData);
    }

    LogAfterExecute(iError, true);
    LogEnd(iError);
}

//---------------------------控制命令-----------------------------------

void TestUR2Driver::OnCashCount()
{
    m_iBVCount = 0;

    //输入参数
    VALIDATION_MODE iValidateMode =
    (VALIDATION_MODE)StringToEnum("VALIDATION_MODE", ui->comboBox_CashCount_iValidateMode->currentText());
    int iNumDepositLimit = ui->spinBox_CashCount_iNumDepositLimit->value();
    int iNumAmountLimit = ui->spinBox_CashCount_iNumAmountLimit->value();
    NOTE_POWER_INDEX iPowerIndex =
    (NOTE_POWER_INDEX)StringToEnum("NOTE_POWER_INDEX", ui->comboBox_CashCount_iPowerIndex->currentText());

    BOOL ArryAcceptDeno[MAX_DENOMINATION_NUM] = {0};
    BOOL *bArryAcceptDeno = ArryAcceptDeno;
    if (ui->checkBox_CashCount_bArryAcceptDenoCode->isChecked())
    {
        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)
        {
            ArryAcceptDeno[j] = FALSE;
        }
        char pTempCtrlName[MAX_SIZE];
        for (int j = 0; j < MAX_DENO_LENGTH; j++)
        {
            sprintf(pTempCtrlName, "spinBox_CashCount_bArryAcceptDeno%d", j + 1);
            ArryAcceptDeno[j] = (findChild<QSpinBox *>(pTempCtrlName)->value() == 0) ? FALSE : TRUE;
        }
    }
    else
    {
        bArryAcceptDeno = nullptr;
    }

    ST_BV_DEPENDENT_MODE stBVDependentMode;
    stBVDependentMode.bEnableBVModeSetting = ui->checkBox_CashCount_bEnableBVModeSetting->isChecked();
    stBVDependentMode.iFullImageMode =
    (RECORD_FULLIMAGE_MODE)StringToEnum("RECORD_FULLIMAGE_MODE", ui->comboBox_CashCount_iFullImageMode->currentText());
    stBVDependentMode.iRejcetNoteNOSN =
    (REJECT_NOTE_NOSN)StringToEnum("REJECT_NOTE_NOSN", ui->comboBox_CashCount_iRejcetNoteNOSN->currentText());

    //输出参数
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    int nRejectNotes = 0;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    USHORT usFedNoteCondition = 0;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    memset(&stStackeNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));

    int iError = m_UR2Device->CashCount(iValidateMode, iNumDepositLimit, iNumAmountLimit,
                                        iPowerIndex, bArryAcceptDeno,
                                        stBVDependentMode, iTotalCashInURJB, iNumStackedToCS,
                                        iNumStackedToESC, pNumStackedToPerCass, iNumCSFed,
                                        iNumESCFed, pNumPerCassFed, nRejectNotes,
                                        stStackeNotesDenoInfo, usFedNoteCondition, pMaintenanceInfo);

    LogAfterExecute(iError, true);

    //if (iError >= 0)
    {
        LogOutput("TotalCashInURJB", QString::number(iTotalCashInURJB));
        LogOutput("NumStackedToCS", QString::number(iNumStackedToCS));
        LogOutput("NumStackedToESC", QString::number(iNumStackedToESC));

        QString tempName;
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumStackedToCass[%d]", i + 1);
            LogOutput(qPrintable(tempName),  QString::number(pNumStackedToPerCass[i]));
        }
        LogOutput("NumStackedToURJB", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogOutput("NumCSFed", QString::number(iNumCSFed));
        LogOutput("NumESCFed", QString::number(iNumESCFed));

        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumCass[%d]Fed", i + 1);
            LogOutput(qPrintable(tempName),  QString::number(pNumPerCassFed[i]));
        }
        LogOutput("NumURJBFed", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogOutput("RejectNotes", QString::number(nRejectNotes));
        LogTotalStackeNotesDenoInfo(stStackeNotesDenoInfo, "stStackeNotesDenoInfo");
        LogOutput("FedNoteCondition", QString::number(usFedNoteCondition));

        //        for (int i = 0; i < MAX_DENOMINATION_NUM; i++)
        //        {
        //            m_iBVCount += pMaintenanceInfo[868 + i * 2] * 256 + pMaintenanceInfo[868 + i *2 + 1];
        //        }
        m_iBVCount = iNumStackedToCS + iNumStackedToESC;
    }

    LogEnd(iError);
}

void TestUR2Driver::OnCashCountRetract()
{
    m_iBVCount = 0;

    //输入参数
    VALIDATION_MODE iValidateMode =
    (VALIDATION_MODE)StringToEnum("VALIDATION_MODE", ui->comboBox_CashCountRetract_iValidateMode->currentText());

    ST_BV_DEPENDENT_MODE stBVDependentMode;
    stBVDependentMode.bEnableBVModeSetting = ui->checkBox_CashCountRetract_bEnableBVModeSetting->isChecked();
    stBVDependentMode.iFullImageMode =
    (RECORD_FULLIMAGE_MODE)StringToEnum("RECORD_FULLIMAGE_MODE", ui->comboBox_CashCountRetract_iFullImageMode->currentText());
    stBVDependentMode.iRejcetNoteNOSN =
    (REJECT_NOTE_NOSN)StringToEnum("REJECT_NOTE_NOSN", ui->comboBox_CashCountRetract_iRejcetNoteNOSN->currentText());
    BOOL bIgnoreESC = ui->checkBox_CashCountRetract_bIgnoreESC->isChecked();

    //输出参数
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    int nRejectNotes = 0;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    USHORT usFedNoteCondition = 0;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    memset(&stStackeNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));

    int iError = m_UR2Device->CashCountRetract(iValidateMode,
                                               stBVDependentMode,
                                               iTotalCashInURJB, iNumStackedToCS,
                                               iNumStackedToESC, pNumStackedToPerCass, iNumCSFed,
                                               iNumESCFed, pNumPerCassFed, nRejectNotes,
                                               stStackeNotesDenoInfo, usFedNoteCondition, pMaintenanceInfo,
                                               bIgnoreESC);

    LogAfterExecute(iError, true);

    //if (iError >= 0)
    {
        LogOutput("TotalCashInURJB", QString::number(iTotalCashInURJB));
        LogOutput("NumStackedToCS", QString::number(iNumStackedToCS));
        LogOutput("NumStackedToESC", QString::number(iNumStackedToESC));

        QString tempName;
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumStackedToCass[%d]", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumStackedToPerCass[i]));
        }
        LogOutput("NumStackedToURJB", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogOutput("NumCSFed", QString::number(iNumCSFed));
        LogOutput("NumESCFed", QString::number(iNumESCFed));

        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumCass[%d]Fed", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumPerCassFed[i]));
        }
        LogOutput("NumURJBFed", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogOutput("RejectNotes", QString::number(nRejectNotes));
        LogTotalStackeNotesDenoInfo(stStackeNotesDenoInfo, "stStackeNotesDenoInfo");
        LogOutput("FedNoteCondition", QString::number(usFedNoteCondition));

        m_iBVCount = iNumStackedToCS + iNumStackedToESC;
    }

    LogEnd(iError);
}

void TestUR2Driver::OnStoreMoney()
{
    m_iBVCount = 0;

    //输入参数
    VALIDATION_MODE iValidateMode =
    (VALIDATION_MODE)StringToEnum("VALIDATION_MODE", ui->comboBox_StoreMoney_iValidateMode->currentText());
    DESTINATION_REJCET iDestinationReject =
    (DESTINATION_REJCET)StringToEnum("DESTINATION_REJCET", ui->comboBox_StoreMoney_iDestinationReject->currentText());
    BYTE btProhibitedBox = 0;
    char pTempCtrlName[MAX_SIZE] = {0};
    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        sprintf(pTempCtrlName, "checkBox_StoreMoney_btProhibitedBox_Cassette%d", i + 1);
        btProhibitedBox += (findChild<QCheckBox *>(pTempCtrlName)->isChecked() ? 1 : 0) << i;
    }

    ST_BV_DEPENDENT_MODE stBVDependentMode;
    stBVDependentMode.bEnableBVModeSetting = ui->checkBox_StoreMoney_bEnableBVModeSetting->isChecked();
    stBVDependentMode.iFullImageMode =
    (RECORD_FULLIMAGE_MODE)StringToEnum("RECORD_FULLIMAGE_MODE", ui->comboBox_StoreMoney_iFullImageMode->currentText());
    stBVDependentMode.iRejcetNoteNOSN =
    (REJECT_NOTE_NOSN)StringToEnum("REJECT_NOTE_NOSN", ui->comboBox_StoreMoney_iRejcetNoteNOSN->currentText());

    //输出参数
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeUnfitNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoInfo;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    memset(&stStackeNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));
    memset(&stStackeUnfitNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));
    memset(&stStackeRejectNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));

    int iError = m_UR2Device->StoreMoney(iValidateMode, TRUE, iDestinationReject, btProhibitedBox,
                                         stBVDependentMode,
                                         iTotalCashInURJB, iNumStackedToCS,
                                         iNumStackedToESC, pNumStackedToPerCass, iNumCSFed,
                                         iNumESCFed, pNumPerCassFed, stStackeNotesDenoInfo,
                                         stStackeUnfitNotesDenoInfo, stStackeRejectNotesDenoInfo, pMaintenanceInfo);

    LogAfterExecute(iError, true);

    if (iError >= 0)
    {
        LogOutput("iTotalCashInURJB", QString::number(iTotalCashInURJB));
        LogOutput("iNumStackedToCS", QString::number(iNumStackedToCS));
        LogOutput("iNumStackedToESC", QString::number(iNumStackedToESC));

        QString tempName;
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumStackedToCass[%d]", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumStackedToPerCass[i]));
        }
        LogOutput("NumStackedToURJB", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogOutput("NumCSFed", QString::number(iNumCSFed));
        LogOutput("NumESCFed", QString::number(iNumESCFed));

        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumCass[%d]Fed", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumPerCassFed[i]));
        }
        LogOutput("NumURJBFed", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogTotalStackeNotesDenoInfo(stStackeNotesDenoInfo, "stStackeNotesDenoInfo");
        LogTotalStackeNotesDenoInfo(stStackeUnfitNotesDenoInfo, "stStackeUnfitNotesDenoInfo");
        LogTotalStackeNotesDenoInfo(stStackeRejectNotesDenoInfo, "stStackeRejectNotesDenoInfo");

        m_iBVCount = iNumCSFed + iNumESCFed;
    }

    LogEnd(iError);
}

void TestUR2Driver::OnCashRollback()
{
    m_iBVCount = 0;
    //输入参数
    VALIDATION_MODE iValidateMode =
    (VALIDATION_MODE)StringToEnum("VALIDATION_MODE", ui->comboBox_CashRollback_iValidateMode->currentText());
    //输出参数
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    memset(&stStackeNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));

    int iError = m_UR2Device->CashRollback(iValidateMode,
                                           iTotalCashInURJB, iNumStackedToCS,
                                           iNumStackedToESC, pNumStackedToPerCass, iNumCSFed,
                                           iNumESCFed, pNumPerCassFed, stStackeNotesDenoInfo,
                                           pMaintenanceInfo);

    LogAfterExecute(iError, true);

    //if (iError >= 0)
    {
        LogOutput("TotalCashInURJB", QString::number(iTotalCashInURJB));
        LogOutput("NumStackedToCS", QString::number(iNumStackedToCS));
        LogOutput("NumStackedToESC", QString::number(iNumStackedToESC));

        QString tempName;
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumStackedToCass[%d]", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumStackedToPerCass[i]));
        }
        LogOutput("NumStackedToURJB", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogOutput("iNumCSFed", QString::number(iNumCSFed));
        LogOutput("iNumESCFed", QString::number(iNumESCFed));

        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumCass[%d]Fed", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumPerCassFed[i]));
        }
        LogOutput("NumURJBFed", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogTotalStackeNotesDenoInfo(stStackeNotesDenoInfo, "stStackeNotesDenoInfo");

        m_iBVCount = iNumStackedToCS + iNumStackedToESC;
    }

    LogEnd(iError);
}

void TestUR2Driver::OnDispense()
{
    m_iBVCount = 0;

    //输入参数
    VALIDATION_MODE iValidateMode =
    (VALIDATION_MODE)StringToEnum("VALIDATION_MODE", ui->comboBox_Dispense_iValidateMode->currentText());
    BYTE btProhibitedBox = 0;
    char pTempCtrlName[MAX_SIZE] = {0};
    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        sprintf(pTempCtrlName, "checkBox_Dispense_btProhibitedBox_Cassette%d", i + 1);
        btProhibitedBox += (findChild<QCheckBox *>(pTempCtrlName)->isChecked() ? 1 : 0) << i;
    }

    int iIndex = 0;
    ST_DISP_DENO szArrDispDeno[MAX_DISP_DENO_NUM];
    memset(szArrDispDeno, 0, sizeof(ST_DISP_DENO) * MAX_DISP_DENO_NUM);
    ST_DISP_DENO *pArrDispDeno = szArrDispDeno;
    if (ui->checkBox_Dispense_pArrDispDeno->isChecked())
    {
        for (int i = 0; i < MAX_DISP_DENO_NUM; i++)
        {
            sprintf(pTempCtrlName, "lineEdit_Dispense_pArrDispDeno_iCashNumber%d", i + 1);
            QLineEdit *lineEdit = findChild<QLineEdit *>(pTempCtrlName);

            sprintf(pTempCtrlName, "comboBox_Dispense_pArrDispDeno_iDenoCode%d", i + 1);
            QComboBox *comboBox = findChild<QComboBox *>(pTempCtrlName);

            if (lineEdit->text().toInt() > 0 && comboBox->currentText() != "DENOMINATION_CODE_00")
            {
                pArrDispDeno[iIndex].iDenoCode = (DENOMINATION_CODE)StringToEnum("DENOMINATION_CODE", comboBox->currentText());
                pArrDispDeno[iIndex].iCashNumber = lineEdit->text().toInt();
                iIndex++;
            }
        }
    }
    else
    {
        pArrDispDeno = nullptr;
    }
    if (iIndex <= 0)
    {
        pArrDispDeno = nullptr;
    }

    iIndex = 0;
    ST_DISP_ROOM szArrDispRoom[MAX_DISP_ROOM_NUM];
    memset(szArrDispRoom, 0, sizeof(ST_DISP_ROOM) * MAX_DISP_ROOM_NUM);
    ST_DISP_ROOM *pArrDispRoom = szArrDispRoom;
    if (ui->checkBox_Dispense_pArrDispRoom->isChecked())
    {
        for (int i = 0; i < MAX_DISP_DENO_NUM; i++)
        {
            sprintf(pTempCtrlName, "comboBox_Dispense_pArrDispRoom_iCassID%d", i + 1);
            QComboBox *comboBox = findChild<QComboBox *>(pTempCtrlName);

            sprintf(pTempCtrlName, "lineEdit_Dispense_pArrDispRoom_iCashNumber%d", i + 1);
            QLineEdit *lineEdit = findChild<QLineEdit *>(pTempCtrlName);

            if (lineEdit->text().toInt() > 0 && comboBox->currentText() != "ID_ROOM_RESERVED")
            {
                pArrDispRoom[iIndex].iCassID = (CASSETTE_ROOM_ID)StringToEnum("CASSETTE_ROOM_ID", comboBox->currentText());
                pArrDispRoom[iIndex].iCashNumber = lineEdit->text().toInt();
                iIndex++;
            }
        }
    }
    else
    {
        pArrDispRoom = nullptr;
    }
    if (iIndex <= 0)
    {
        pArrDispRoom = nullptr;
    }

    ST_BV_DEPENDENT_MODE stBVDependentMode;
    stBVDependentMode.bEnableBVModeSetting = ui->checkBox_Dispense_bEnableBVModeSetting->isChecked();
    stBVDependentMode.iFullImageMode =
    (RECORD_FULLIMAGE_MODE)StringToEnum("RECORD_FULLIMAGE_MODE", ui->comboBox_Dispense_iFullImageMode->currentText());
    stBVDependentMode.iRejcetNoteNOSN =
    (REJECT_NOTE_NOSN)StringToEnum("REJECT_NOTE_NOSN", ui->comboBox_Dispense_iRejcetNoteNOSN->currentText());

    //输出参数
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    int pNumDispensedForPerCass[MAX_CASSETTE_NUM] = {0};
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoSourInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoBVInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoDestInfo;
    ST_DISP_MISSFEED_ROOM stDispMissfeedRoom;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    memset(&stStackeNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));
    memset(&stStackeRejectNotesDenoSourInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));
    memset(&stStackeRejectNotesDenoBVInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));
    memset(&stStackeRejectNotesDenoDestInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));

    int iError = m_UR2Device->Dispense(iValidateMode, btProhibitedBox, pArrDispDeno, pArrDispRoom,
                                       stBVDependentMode,
                                       iTotalCashInURJB, iNumStackedToCS, iNumStackedToESC,
                                       pNumStackedToPerCass, iNumCSFed, iNumESCFed, pNumPerCassFed,
                                       pNumDispensedForPerCass, stStackeNotesDenoInfo,
                                       stStackeRejectNotesDenoSourInfo, stStackeRejectNotesDenoBVInfo,
                                       stStackeRejectNotesDenoDestInfo, stDispMissfeedRoom,
                                       pMaintenanceInfo);

    LogAfterExecute(iError, true);

    //if (iError >= 0)
    {
        LogOutput("TotalCashInURJB", QString::number(iTotalCashInURJB));
        LogOutput("NumStackedToCS", QString::number(iNumStackedToCS));
        LogOutput("NumStackedToESC", QString::number(iNumStackedToESC));

        QString tempName;
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumStackedToCass[%d]", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumStackedToPerCass[i]));
        }
        LogOutput("NumStackedToURJB", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogOutput("NumCSFed", QString::number(iNumCSFed));
        LogOutput("NumESCFed", QString::number(iNumESCFed));

        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumCass[%d]Fed", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumPerCassFed[i]));
        }
        LogOutput("NumURJBFed", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumDispensedForCass[%d]", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumDispensedForPerCass[i]));
        }
        LogOutput("NumDispensedForURJB", QString::number(pNumDispensedForPerCass[MAX_CASSETTE_NUM - 1]));

        LogTotalStackeNotesDenoInfo(stStackeNotesDenoInfo, "stStackeNotesDenoInfo");
        LogTotalStackeNotesDenoInfo(stStackeRejectNotesDenoSourInfo, "stStackeRejectNotesDenoSourInfo");
        LogTotalStackeNotesDenoInfo(stStackeRejectNotesDenoBVInfo, "stStackeRejectNotesDenoBVInfo");
        LogTotalStackeNotesDenoInfo(stStackeRejectNotesDenoDestInfo, "stStackeRejectNotesDenoDestInfo");

        LogOutputStructName("DispMissfeedRoom");
        LogOutput("ArryRoom", BOOLArrayToChar(stDispMissfeedRoom.bArryRoom, MAX_CASSETTE_NUM));
        LogOutput("RoomContinuousRejects", stDispMissfeedRoom.bRoomContinuousRejects ? "TRUE" : "FALSE");
        LogOutput("RoomEmpty", stDispMissfeedRoom.bRoomEmpty ? "TRUE" : "FALSE");
        LogOutput("RoomMissFeed", stDispMissfeedRoom.bRoomMissFeed ? "TRUE" : "FALSE");
        LogOutput("RoomTooManyRejects", stDispMissfeedRoom.bRoomTooManyRejects ? "TRUE" : "FALSE");

        m_iBVCount = iNumStackedToCS + iNumStackedToESC;
    }

    LogEnd(iError);
}

void TestUR2Driver::OnReject()
{
    m_iBVCount = 0;
    //输入参数
    VALIDATION_MODE iValidateMode =
    (VALIDATION_MODE)StringToEnum("VALIDATION_MODE", ui->comboBox_Rejcet_iValidateMode->currentText());
    BYTE btProhibitedBox = 0;
    char pTempCtrlName[MAX_SIZE] = {0};
    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        sprintf(pTempCtrlName, "checkBox_Rejcet_btProhibitedBox_Cassette%d", i + 1);
        btProhibitedBox += (findChild<QCheckBox *>(pTempCtrlName)->isChecked() ? 1 : 0) << i;
    }

    DESTINATION_REJCET iDestinationReject =
    (DESTINATION_REJCET)StringToEnum("DESTINATION_REJCET", ui->comboBox_Rejcet_iDestinationReject->currentText());

    ST_BV_DEPENDENT_MODE stBVDependentMode;
    stBVDependentMode.bEnableBVModeSetting = ui->checkBox_Rejcet_bEnableBVModeSetting->isChecked();
    stBVDependentMode.iFullImageMode =
    (RECORD_FULLIMAGE_MODE)StringToEnum("RECORD_FULLIMAGE_MODE", ui->comboBox_Rejcet_iFullImageMode->currentText());
    stBVDependentMode.iRejcetNoteNOSN =
    (REJECT_NOTE_NOSN)StringToEnum("REJECT_NOTE_NOSN", ui->comboBox_Rejcet_iRejcetNoteNOSN->currentText());

    //输出参数
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeUnfitNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoInfo;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    memset(&stStackeNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));
    memset(&stStackeUnfitNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));
    memset(&stStackeRejectNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));

    int iError = m_UR2Device->RetractESCForDispenseRejcet(iValidateMode, btProhibitedBox, iDestinationReject,
                                                          stBVDependentMode, iTotalCashInURJB,
                                                          iNumStackedToCS, iNumStackedToESC, pNumStackedToPerCass, iNumCSFed,
                                                          iNumESCFed, pNumPerCassFed, stStackeNotesDenoInfo,
                                                          stStackeUnfitNotesDenoInfo, stStackeRejectNotesDenoInfo,
                                                          pMaintenanceInfo);

    LogAfterExecute(iError, true);

    //if (iError >= 0)
    {
        LogOutput("TotalCashInURJB", QString::number(iTotalCashInURJB));
        LogOutput("NumStackedToCS", QString::number(iNumStackedToCS));
        LogOutput("NumStackedToESC", QString::number(iNumStackedToESC));

        QString tempName;
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumStackedToCass[%d]", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumStackedToPerCass[i]));
        }
        LogOutput("NumStackedToURJB", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogOutput("NumCSFed", QString::number(iNumCSFed));
        LogOutput("NumESCFed", QString::number(iNumESCFed));

        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumCass[%d]Fed", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumPerCassFed[i]));
        }
        LogOutput("NumURJBFed", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogTotalStackeNotesDenoInfo(stStackeNotesDenoInfo, "stStackeNotesDenoInfo");
        LogTotalStackeNotesDenoInfo(stStackeUnfitNotesDenoInfo, "stStackeUnfitNotesDenoInfo");
        LogTotalStackeNotesDenoInfo(stStackeRejectNotesDenoInfo, "stStackeRejectNotesDenoInfo");

        m_iBVCount = iNumStackedToCS + iNumStackedToESC;
    }

    LogEnd(iError);
}

void TestUR2Driver::OnRetractESC()
{
    m_iBVCount = 0;
    //输入参数
    VALIDATION_MODE iValidateMode =
    (VALIDATION_MODE)StringToEnum("VALIDATION_MODE", ui->comboBox_RetractESC_iValidateMode->currentText());
    BYTE btProhibitedBox = 0;
    char pTempCtrlName[MAX_SIZE] = {0};
    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        sprintf(pTempCtrlName, "checkBox_RetractESC_btProhibitedBox_Cassette%d", i + 1);
        btProhibitedBox += (findChild<QCheckBox *>(pTempCtrlName)->isChecked() ? 1 : 0) << i;
    }
    DESTINATION_REJCET iDestinationReject =
    (DESTINATION_REJCET)StringToEnum("DESTINATION_REJCET", ui->comboBox_RetractESC_iDestinationReject->currentText());

    ST_BV_DEPENDENT_MODE stBVDependentMode;
    stBVDependentMode.bEnableBVModeSetting = ui->checkBox_RetractESC_bEnableBVModeSetting->isChecked();
    stBVDependentMode.iFullImageMode =
    (RECORD_FULLIMAGE_MODE)StringToEnum("RECORD_FULLIMAGE_MODE", ui->comboBox_RetractESC_iFullImageMode->currentText());
    stBVDependentMode.iRejcetNoteNOSN =
    (REJECT_NOTE_NOSN)StringToEnum("REJECT_NOTE_NOSN", ui->comboBox_RetractESC_iRejcetNoteNOSN->currentText());

    //输出参数
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeUnfitNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoInfo;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    memset(&stStackeNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));
    memset(&stStackeUnfitNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));
    memset(&stStackeRejectNotesDenoInfo, 0, sizeof(ST_TOTAL_STACKE_NOTES_DENO_INFO));

    int iError = m_UR2Device->RetractESC(iValidateMode, btProhibitedBox, iDestinationReject,
                                         stBVDependentMode, iTotalCashInURJB,
                                         iNumStackedToCS, iNumStackedToESC, pNumStackedToPerCass, iNumCSFed,
                                         iNumESCFed, pNumPerCassFed, stStackeNotesDenoInfo,
                                         stStackeUnfitNotesDenoInfo, stStackeRejectNotesDenoInfo,
                                         pMaintenanceInfo);

    LogAfterExecute(iError, true);

    //if (iError >= 0)
    {
        LogOutput("TotalCashInURJB", QString::number(iTotalCashInURJB));
        LogOutput("NumStackedToCS", QString::number(iNumStackedToCS));
        LogOutput("NumStackedToESC", QString::number(iNumStackedToESC));

        QString tempName;
        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumStackedToCass[%d]", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumStackedToPerCass[i]));
        }
        LogOutput("NumStackedToURJB", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogOutput("NumCSFed", QString::number(iNumCSFed));
        LogOutput("NumESCFed", QString::number(iNumESCFed));

        for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            tempName.sprintf("NumCass[%d]Fed", i + 1);
            LogOutput(qPrintable(tempName), QString::number(pNumPerCassFed[i]));
        }
        LogOutput("NumURJBFed", QString::number(pNumStackedToPerCass[MAX_CASSETTE_NUM - 1]));

        LogTotalStackeNotesDenoInfo(stStackeNotesDenoInfo, "stStackeNotesDenoInfo");
        LogTotalStackeNotesDenoInfo(stStackeUnfitNotesDenoInfo, "stStackeUnfitNotesDenoInfo");
        LogTotalStackeNotesDenoInfo(stStackeRejectNotesDenoInfo, "stStackeRejectNotesDenoInfo");

        m_iBVCount = iNumCSFed + iNumESCFed;
    }

    LogEnd(iError);
}

void TestUR2Driver::OnCloseShutter()
{
    // 输入参数
    BOOL bForcible = ui->checkBox_CloseShutter_bForcible->isChecked();

    // 输出参数
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};

    int iError = m_UR2Device->CloseShutter(pMaintenanceInfo, bForcible);

    LogAfterExecute(iError, true);

    LogEnd(iError);
}

void TestUR2Driver::OnOpenShutter()
{
    // 输出参数
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};

    int iError = m_UR2Device->OpenShutter(pMaintenanceInfo);

    LogAfterExecute(iError, true);

    LogEnd(iError);
}

//---------------------------维护指令-----------------------------------

void TestUR2Driver::OnReboot()
{
    int iError = m_UR2Device->Reboot();

    LogAfterExecute(iError, true);

    LogEnd(iError);
}

void TestUR2Driver::OnStartTransaction()
{
    int iError = m_UR2Device->StartTransaction();

    LogAfterExecute(iError, true);

    LogEnd(iError);
}

void TestUR2Driver::OnPreNextTransaction()
{
    int iError = m_UR2Device->PreNextTransaction();

    LogAfterExecute(iError, true);

    LogEnd(iError);
}

void TestUR2Driver::OnSetStatusChangeListener()
{
    QMessageBox::information(this, "HCM2驱动测试软件", "该命令暂不实现");
    Log("该命令暂不实现");
    LogEnd(0);
}

void TestUR2Driver::OnBVCommStart()
{
    int iError = m_UR2Device->BVCommStart();

    LogAfterExecute(iError, true);

    LogEnd(iError);
}

void TestUR2Driver::OnBVCommEnd()
{
    int iError = m_UR2Device->BVCommEnd();

    LogAfterExecute(iError, true);

    LogEnd(iError);
}

void TestUR2Driver::OnUserMemoryWrite()
{
    if (ui->lineEdit_UserMemoryWrite_usDataLen->text().toInt() <= 0)
    {
        Log("写往内存的数据为空");
        Log("*************************************************************");
        return;
    }

    // 输入参数
    USER_MEMORY_TARGET iUserMemoryTaget =
    (USER_MEMORY_TARGET)StringToEnum("USER_MEMORY_TARGET", ui->comboBox_UserMemoryWrite_iUserMemoryTaget->currentText());
    char szUserMemoryData[MAX_USER_MEMORY_DATA_LENGTH] = {0};
    sprintf(szUserMemoryData, "%s", qPrintable(ui->textEdit_UserMemoryWrite_szUserMemoryData->toPlainText()));
    USHORT usDataLen = ui->lineEdit_UserMemoryWrite_usDataLen->text().toInt();

    int iError = m_UR2Device->UserMemoryWrite(iUserMemoryTaget, szUserMemoryData, usDataLen);

    LogAfterExecute(iError, false);

    LogEnd(iError);
}

void TestUR2Driver::OnUserMemoryRead()
{
    // 输入参数
    USER_MEMORY_TARGET iUserMemoryTaget =
    (USER_MEMORY_TARGET)StringToEnum("USER_MEMORY_TARGET", ui->comboBox_UserMemoryRead_iUserMemoryTaget->currentText());

    // 输出参数
    ST_USER_MEMORY_READ stUserMemoryData[MAX_USER_MEMORY_DATA_ARRAY_NUM];
    memset(stUserMemoryData, 0, sizeof(ST_USER_MEMORY_READ) * MAX_USER_MEMORY_DATA_ARRAY_NUM);
    USHORT usDataArrayCount = 0;
    int iError = m_UR2Device->UserMemoryRead(iUserMemoryTaget, stUserMemoryData, usDataArrayCount);

    LogAfterExecute(iError, true);

    if (iError >= 0)
    {
        for (int i = 0; i < usDataArrayCount ; i++)
        {
            LogOutput("UserMemoryTaget", EnumToString("USER_MEMORY_TARGET", stUserMemoryData[i].iUserMemoryTaget));
            LogOutput("UserMemoryData", stUserMemoryData[i].szUserMemoryData);
        }
    }

    LogEnd(iError);
}

//---------------------------其他指令-----------------------------------

void TestUR2Driver::OnGetLogData()
{
    // 输入参数
    LOG_INFO_TYPE iLogType =
    (LOG_INFO_TYPE)StringToEnum("LOG_INFO_TYPE", ui->comboBox_GetLogData_iLogType->currentText());
    LOG_INFO_TERM iLogTerm =
    (LOG_INFO_TERM)StringToEnum("LOG_INFO_TERM", ui->comboBox_GetLogData_iLogTerm->currentText());;

    // 输出参数
    char pszLogData[MAX_LOG_DATA_LENGTH] = {0};
    DWORD iLogDataLen = 0;

    int iError = m_UR2Device->GetLogData(iLogType, iLogTerm, pszLogData, iLogDataLen);

    LogAfterExecute(iError, true);

    if (iError >= 0)
    {
        QString strLogName;

        SYSTEMTIME stSystemTime;
        CQtTime::GetLocalTime(stSystemTime);

        strLogName.sprintf("%s/%s_%04d%02d%02d.log", qPrintable(m_strLogDir),
                           qPrintable(EnumToString("LOG_INFO_TYPE", iLogType)), stSystemTime.wYear,
                           stSystemTime.wMonth, stSystemTime.wDay);

        FILE *pFile;
        pFile = fopen(qPrintable(strLogName), "wb");
        if (nullptr == pFile)
        {
            QString filePath;
            filePath += "创建日志失败:";
            filePath += strLogName;
            Log(filePath);
            return;
        }

        try
        {
            fwrite(pszLogData, sizeof(uchar), iLogDataLen, pFile);
        }
        catch (...)
        {
            Log("保存日志文件失败");
            fclose(pFile);
            return;
        }
        fclose(pFile);

        LogOutput("LogDataLen", QString::number(iLogDataLen));

        QString filePath;
        filePath += "创建日志成功:";
        filePath += strLogName;
        Log(filePath);
    }

    LogEnd(iError);
}

void TestUR2Driver::OnEraseAllLogData()
{
    int iError = m_UR2Device->EraseAllLogData();

    LogAfterExecute(iError, false);

    LogEnd(iError);
}

void TestUR2Driver::OnCloseUSBConn()
{
    int iError = m_UR2Device->CloseURConn();
    iError = m_UR2Device->CloseBVConn();
    LogAfterExecute(iError, false);

    LogEnd(iError);
}

void TestUR2Driver::OnUpdatePDL()
{
    /*
    if (!m_bGetLocalFWFlag)
    {
        Log("未获取本地固件版本，请获取后重试");
        Log("*************************************************************");
        return;
    }
    if (!m_bGetFileFWFlag)
    {
        Log("未获取到更新固件版本，请获取后重试");
        Log("*************************************************************");
        return;
    }

    map<USHORT, ProgramFileInfo> mUpdateMap;
    map<USHORT, ProgramFileInfo>::iterator it = m_ProFilesInfo.begin();
    for (; it != m_ProFilesInfo.end(); it++)
    {
        for (int i = 0; i < m_strLocalFWVersion.length(); i++)
        {
            if (m_strLocalFWVersion[i].contains(it->second.szFWType))
            {
                QString fileFWVersion(it->second.szFWVersion);
                if (m_strLocalFWVersion[i].right(fileFWVersion.length()) < fileFWVersion)
                {
                    mUpdateMap[it->second.uCTLID] = it->second;
                }
            }
        }
    }
    if (mUpdateMap.empty())
    {
        Log("没有可更新的版本");
        Log("*************************************************************");
        return ;
    }

    m_bUpdate = TRUE;

    //开始下载固件程序
    Log("开始更新固件");

    int iRet = m_UR2Device->ProgramDownloadStart();
    if (iRet < 0 )
    {
        Log("执行开始下载固件命令失败");
        m_bUpdate = false;
        return;
    }
    //BOOL bLastFile = FALSE;
    QString temp;
    uint i = 0;
    for (i = 1, it = mUpdateMap.begin(); it != mUpdateMap.end() && i <= mUpdateMap.size(); it++, i++)
    {
        //bLastFile = (i == mUpdateMap.size());
        FILE* fp = fopen((*it).second.szFilePath, "rb");
        if (fp == nullptr)
        {
            temp.sprintf("读取下载文件(%s)失败", (*it).second.szFilePath);
            m_bUpdate = false;
            Log(temp);
            m_UR2Device->ProgramDownloadEnd();
            return;
        }

        fseek(fp, 0L, SEEK_END);
        long nLen = ftell(fp);
        if (nLen == 0)
        {
            temp.sprintf("固件文件(%s)为空文件", (*it).second.szFilePath);
            m_UR2Device->ProgramDownloadEnd();
            m_bUpdate = false;
            return;
        }

        fseek(fp, 0, SEEK_SET);
        char szDataBuff[10240] = {0};
        BOOL bFirstData = TRUE;
        BOOL bLastData = FALSE;
        PDL_BLOCK_TYPE PDLBlockType = PDL_FIRST_BLOCK;
        long lDownLoadedLen = 0;
        while (!bLastData)
        {
            memset(szDataBuff, 0, sizeof(szDataBuff));
            int iReadLen = fread(szDataBuff, sizeof(BYTE), FW_BATCH_DATA_SIZE, fp);
            if(ferror(fp))
            {
                temp.sprintf("固件下载过程中读取文件(%s)失败", (*it).second.szFilePath);
                Log(temp);
                m_bUpdate = false;
                m_UR2Device->ProgramDownloadEnd();
                return;
            }
            if (feof(fp)) // 已到文件末尾
            {
                bLastData = TRUE;
            }
            if (bFirstData)
            {
                PDLBlockType = PDL_FIRST_BLOCK;
                bFirstData = FALSE;
            }
            else if (bLastData)
            {
                PDLBlockType = PDL_LAST_BLOCK;
            }
            else
            {
                PDLBlockType = PDL_MIDDLE_BLOCK;
            }

            iRet = m_UR2Device->ProgramDownloadSendData((*it).second.uCTLID, PDLBlockType,(*it).second.uLoadAdress,
                                                        lDownLoadedLen, szDataBuff, iReadLen);
            if (iRet < 0)
            {
                fclose(fp);
                m_bUpdate = false;
                m_UR2Device->ProgramDownloadEnd();
                Log("发送数据失败");
                return;
            }

            lDownLoadedLen += iReadLen;
        }

        Log("更新完成");
        fclose(fp);
    }

    iRet = m_UR2Device->ProgramDownloadEnd();
    if (iRet < 0 )
    {
        Log("执行结束固件更新命令失败");
        m_bUpdate = false;
        return;
    }

    LogAfterExecute(0, false);

    LogEnd(0);

    m_bUpdate = false;
    */
}

//---------------------------辅助函数-----------------------------------

int TestUR2Driver::CreateSerialImage(SNOTESERIALINFO &lpNoteSerialInfo, int iNumber)
{
    QDir dir;
    char tmp_dir[MAX_SIZE] = {0};
    sprintf(tmp_dir, "%s/NoteSeiral", qPrintable(dir.currentPath()));
    QDir SNInfoDir;
    if (!SNInfoDir.exists(tmp_dir))
    {
        bool bReturn = SNInfoDir.mkdir(tmp_dir);
        if (!bReturn)
        {
            QMessageBox::warning(this, "创建文件夹", "冠字码文件夹创建失败！");
            return -1;
        }
    }

    SYSTEMTIME stSystemTime;
    CQtTime::GetLocalTime(stSystemTime);

    QString SNInfoFile;
    SNInfoFile.sprintf("%s/%04d%02d%02d%02d%02d%02d_%03d_%04d_%s.bmp", tmp_dir, stSystemTime.wYear,
                       stSystemTime.wMonth, stSystemTime.wDay, stSystemTime.wHour,
                       stSystemTime.wMinute, stSystemTime.wSecond, iNumber,
                       lpNoteSerialInfo.dwBVInternalIndex,
                       lpNoteSerialInfo.cSerialNumber);

    SNInfoFile.replace("*", "$");//替换*为$
    FILE *pFile;
    pFile = fopen(qPrintable(SNInfoFile), "wb");
    if (nullptr == pFile)
    {
        QString filePath;
        filePath += "创建冠字码图片失败:";
        filePath += SNInfoFile;
        Log(filePath);
        return -1;
    }

    try
    {
        fwrite(lpNoteSerialInfo.cSerialImgData, sizeof(char), lpNoteSerialInfo.dwImgDataLen, pFile);
    }
    catch (...)
    {
        Log("保存冠字码图片失败");
        fclose(pFile);
        return -1;
    }
    fclose(pFile);

    LogOutput("SerialImgData", SNInfoFile);

    return 0;
}

int TestUR2Driver::CreateSerialImage(SNOTESERIALINFOFULLIMAGE &lpNoteSerialInfoFullImage, int iNumber)
{
    QDir dir;
    char tmp_dir[MAX_SIZE] = {0};
    sprintf(tmp_dir, "%s/NoteSeiral", qPrintable(dir.currentPath()));
    QDir SNInfoDir;
    if (!SNInfoDir.exists(tmp_dir))
    {
        bool bReturn = SNInfoDir.mkdir(tmp_dir);
        if (!bReturn)
        {
            QMessageBox::warning(this, "创建文件夹", "冠字码文件夹创建失败！");
            return -1;
        }
    }

    SYSTEMTIME stSystemTime;
    CQtTime::GetLocalTime(stSystemTime);

    QString SNInfoFile;
    SNInfoFile.sprintf("%s/%04d%02d%02d%02d%02d%02d_%03d_%04d_FullImage.bmp", tmp_dir, stSystemTime.wYear,
                       stSystemTime.wMonth, stSystemTime.wDay, stSystemTime.wHour,
                       stSystemTime.wMinute, stSystemTime.wSecond, iNumber,
                       lpNoteSerialInfoFullImage.dwBVInternalIndex);

    SNInfoFile.replace("*", "$");//替换*为$
    FILE *pFile;
    pFile = fopen(qPrintable(SNInfoFile), "wb");
    if (nullptr == pFile)
    {
        QString filePath;
        filePath += "创建冠字码图片失败:";
        filePath += SNInfoFile;
        Log(filePath);
        return -1;
    }

    try
    {
        fwrite(lpNoteSerialInfoFullImage.cFullImgData, sizeof(char), lpNoteSerialInfoFullImage.dwImgDataLen, pFile);
    }
    catch (...)
    {
        Log("保存冠字码图片失败");
        fclose(pFile);
        return -1;
    }
    fclose(pFile);

    LogOutput("SerialImgData", SNInfoFile);

    return 0;
}

int TestUR2Driver::CreateDefaultImage(int iNumber)
{
    QDir dir;
    char tmp_dir[MAX_SIZE] = {0};
    sprintf(tmp_dir, "%s/NoteSeiral", qPrintable(dir.currentPath()));
    QDir SNInfoDir;
    if (!SNInfoDir.exists(tmp_dir))
    {
        bool bReturn = SNInfoDir.mkdir(tmp_dir);
        if (!bReturn)
        {
            QMessageBox::warning(this, "创建文件夹", "冠字码文件夹创建失败！");
            return -1;
        }
    }

    SYSTEMTIME stSystemTime;
    CQtTime::GetLocalTime(stSystemTime);

    QString SNInfoFile;
    SNInfoFile.sprintf("%s/%04d%02d%02d%02d%02d%02d_%03d_$$$$$$$$$$.bmp", tmp_dir, stSystemTime.wYear,
                       stSystemTime.wMonth, stSystemTime.wDay, stSystemTime.wHour,
                       stSystemTime.wMinute, stSystemTime.wSecond, iNumber);

    if (!QFile::copy(DEFAULT_IMAGE, SNInfoFile))
    {
        QString temp;
        temp += "冠字码图片生成失败，没有找到默认图片:";
        temp += DEFAULT_IMAGE;
        QMessageBox::warning(this, "生成冠字码图片", temp);
        return -1;
    }

    LogOutput("SerialImgData", SNInfoFile);
    return 0;
}

void TestUR2Driver::AnalysisFWVersion(const char *pszFWVersion)
{
    LogOutputStructName("Firmware version");

    m_strLocalFWVersion.clear();

    char szBuf[28 + 1] = {0};
    QString str_temp;

    for (int i = 0; i < 8; i++)
    {
        str_temp.clear();
        memset(szBuf, 0, sizeof(szBuf));
        memcpy(szBuf, pszFWVersion + 28 * i, 28);
        str_temp = szBuf;
        QString OutputStr;
        OutputStr.sprintf("%s%s = %s ", TAB_STR, qPrintable(str_temp.mid(0, 16)), qPrintable(str_temp.mid(16, 8)));
        Log(OutputStr);

        if (str_temp.left(8) == "00000000" || str_temp.isEmpty())
        {
            continue;
        }

        OutputStr.remove(QChar(' '));
        m_strLocalFWVersion.append(OutputStr);
    }
}

void TestUR2Driver::SaveDevStatus(const ST_DEV_STATUS_INFO &stDevStatusInfo)
{
    memcpy(&m_stDevStatusInfo, &stDevStatusInfo, sizeof(ST_DEV_STATUS_INFO));
}

void TestUR2Driver::FindDevDifference(const ST_DEV_STATUS_INFO &stDevStatusInfo)
{
    QColor color(170, 170, 170);
    if (m_stDevStatusInfo.bCashAtShutter != stDevStatusInfo.bCashAtShutter)
    {
        ui->tableWidget_GetDevStatusInfo->item(0, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bCashAtCS != stDevStatusInfo.bCashAtCS)
    {
        ui->tableWidget_GetDevStatusInfo->item(0, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bCashAtCSErrPos != stDevStatusInfo.bCashAtCSErrPos)
    {
        ui->tableWidget_GetDevStatusInfo->item(1, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bESCFull != stDevStatusInfo.bESCFull)
    {
        ui->tableWidget_GetDevStatusInfo->item(1, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bESCNearFull != stDevStatusInfo.bESCNearFull)
    {
        ui->tableWidget_GetDevStatusInfo->item(2, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bESCEmpty != stDevStatusInfo.bESCEmpty)
    {
        ui->tableWidget_GetDevStatusInfo->item(2, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bURJBFull != stDevStatusInfo.bURJBFull)
    {
        ui->tableWidget_GetDevStatusInfo->item(3, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bURJBEmpty != stDevStatusInfo.bURJBEmpty)
    {
        ui->tableWidget_GetDevStatusInfo->item(3, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bNotesRJInCashCout != stDevStatusInfo.bNotesRJInCashCout)
    {
        ui->tableWidget_GetDevStatusInfo->item(4, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bHCMUPInPos != stDevStatusInfo.bHCMUPInPos)
    {
        ui->tableWidget_GetDevStatusInfo->item(4, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bHCMLOWInPos != stDevStatusInfo.bHCMLOWInPos)
    {
        ui->tableWidget_GetDevStatusInfo->item(5, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bURJBOpen != stDevStatusInfo.bURJBOpen)
    {
        ui->tableWidget_GetDevStatusInfo->item(5, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bRearDoorOpen != stDevStatusInfo.bRearDoorOpen)
    {
        ui->tableWidget_GetDevStatusInfo->item(6, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bFrontDoorOpen != stDevStatusInfo.bFrontDoorOpen)
    {
        ui->tableWidget_GetDevStatusInfo->item(6, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bESCOpen != stDevStatusInfo.bESCOpen)
    {
        ui->tableWidget_GetDevStatusInfo->item(7, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bESCInPos != stDevStatusInfo.bESCInPos)
    {
        ui->tableWidget_GetDevStatusInfo->item(7, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bESCRearEnd != stDevStatusInfo.bESCRearEnd)
    {
        ui->tableWidget_GetDevStatusInfo->item(8, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bCSInPos != stDevStatusInfo.bCSInPos)
    {
        ui->tableWidget_GetDevStatusInfo->item(8, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bBVFanErr != stDevStatusInfo.bBVFanErr)
    {
        ui->tableWidget_GetDevStatusInfo->item(9, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bBVOpen != stDevStatusInfo.bBVOpen)
    {
        ui->tableWidget_GetDevStatusInfo->item(9, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.iOutShutterStatus != stDevStatusInfo.iOutShutterStatus)
    {
        ui->tableWidget_GetDevStatusInfo->item(10, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.CassStatus[0] != stDevStatusInfo.CassStatus[0])
    {
        ui->tableWidget_GetDevStatusInfo->item(10, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.CassStatus[1] != stDevStatusInfo.CassStatus[1])
    {
        ui->tableWidget_GetDevStatusInfo->item(11, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.CassStatus[2] != stDevStatusInfo.CassStatus[2])
    {
        ui->tableWidget_GetDevStatusInfo->item(11, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.CassStatus[3] != stDevStatusInfo.CassStatus[3])
    {
        ui->tableWidget_GetDevStatusInfo->item(12, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.CassStatus[4] != stDevStatusInfo.CassStatus[4])
    {
        ui->tableWidget_GetDevStatusInfo->item(12, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bCassInPos[0] != stDevStatusInfo.bCassInPos[0])
    {
        ui->tableWidget_GetDevStatusInfo->item(13, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bCassInPos[1] != stDevStatusInfo.bCassInPos[1])
    {
        ui->tableWidget_GetDevStatusInfo->item(13, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bCassInPos[2] != stDevStatusInfo.bCassInPos[2])
    {
        ui->tableWidget_GetDevStatusInfo->item(14, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bCassInPos[3] != stDevStatusInfo.bCassInPos[3])
    {
        ui->tableWidget_GetDevStatusInfo->item(14, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bCassInPos[4] != stDevStatusInfo.bCassInPos[4])
    {
        ui->tableWidget_GetDevStatusInfo->item(15, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bForcedOpenShutter != stDevStatusInfo.bForcedOpenShutter)
    {
        ui->tableWidget_GetDevStatusInfo->item(15, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bForcedRemovCashInCS != stDevStatusInfo.bForcedRemovCashInCS)
    {
        ui->tableWidget_GetDevStatusInfo->item(16, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bCashLeftInCS != stDevStatusInfo.bCashLeftInCS)
    {
        ui->tableWidget_GetDevStatusInfo->item(16, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bCashExistInESC != stDevStatusInfo.bCashExistInESC)
    {
        ui->tableWidget_GetDevStatusInfo->item(17, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bReqReadStatus != stDevStatusInfo.bReqReadStatus)
    {
        ui->tableWidget_GetDevStatusInfo->item(17, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bReqGetOPLog != stDevStatusInfo.bReqGetOPLog)
    {
        ui->tableWidget_GetDevStatusInfo->item(18, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bReqReset != stDevStatusInfo.bReqReset)
    {
        ui->tableWidget_GetDevStatusInfo->item(18, 3)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bBVWarning != stDevStatusInfo.bBVWarning)
    {
        ui->tableWidget_GetDevStatusInfo->item(19, 1)->setBackgroundColor(color);
    }
    if (m_stDevStatusInfo.bDuringEnergy != stDevStatusInfo.bDuringEnergy)
    {
        ui->tableWidget_GetDevStatusInfo->item(19, 3)->setBackgroundColor(color);
    }
}

int TestUR2Driver::StringToEnum(QString strEnumName, QString strEnumValue)
{
    if (strEnumName.isEmpty())
    {
        return -1;
    }
    if (strEnumName == "CONNECT_TYPE")
    {
        if ("CONNECT_TYPE_ZERO"     == strEnumValue)    return CONNECT_TYPE_ZERO;
        if ("CONNECT_TYPE_UR"       == strEnumValue)    return CONNECT_TYPE_UR;
    }
    else if (strEnumName == "CASSETTE_TYPE")
    {
        if ("CASSETTE_TYPE_RB"      == strEnumValue)    return CASSETTE_TYPE_RB;
        if ("CASSETTE_TYPE_AB"      == strEnumValue)    return CASSETTE_TYPE_AB;
        //if ("CASSETTE_TYPE_DAB"     == strEnumValue)    return CASSETTE_TYPE_DAB;
        if ("CASSETTE_TYPE_UNKNOWN" == strEnumValue)    return CASSETTE_TYPE_UNKNOWN;
        if ("CASSETTE_TYPE_UNLOAD"  == strEnumValue)    return CASSETTE_TYPE_UNLOAD;
    }
    else if (strEnumName == "CASSETTE_NUMBER")
    {
        if ("CASSETTE_1" == strEnumValue)    return CASSETTE_1;
        if ("CASSETTE_2" == strEnumValue)    return CASSETTE_2;
        if ("CASSETTE_3" == strEnumValue)    return CASSETTE_3;
        if ("CASSETTE_4" == strEnumValue)    return CASSETTE_4;
        if ("CASSETTE_5" == strEnumValue)    return CASSETTE_5;
        if ("CASSETTE_6" == strEnumValue)    return CASSETTE_6;
        if ("RESERVED"   == strEnumValue)    return RESERVED;
    }
    else if (strEnumName == "CASSETTE_ERR")
    {
        if ("CASSETTE_ERR_RESERVED" == strEnumValue)    return CASSETTE_ERR_RESERVED;
        if ("CASSETTE_ERR_LANE5"    == strEnumValue)    return CASSETTE_ERR_LANE5;
        if ("CASSETTE_ERR_LANE4"    == strEnumValue)    return CASSETTE_ERR_LANE4;
        if ("CASSETTE_ERR_LANE3"    == strEnumValue)    return CASSETTE_ERR_LANE3;
        if ("CASSETTE_ERR_LANE2"    == strEnumValue)    return CASSETTE_ERR_LANE2;
        if ("CASSETTE_ERR_LANE1"    == strEnumValue)    return CASSETTE_ERR_LANE1;
    }
    else if (strEnumName == "CASSETTE_ROOM_ID")
    {
        if ("ID_CS"            == strEnumValue)    return ID_CS;
        if ("ID_ESC"           == strEnumValue)    return ID_ESC;
        if ("ID_URJB"          == strEnumValue)    return ID_URJB;
        if ("ID_ROOM_1"        == strEnumValue)    return ID_ROOM_1;
        if ("ID_ROOM_2"        == strEnumValue)    return ID_ROOM_2;
        if ("ID_ROOM_3"        == strEnumValue)    return ID_ROOM_3;
        if ("ID_ROOM_4"        == strEnumValue)    return ID_ROOM_4;
        if ("ID_ROOM_5"        == strEnumValue)    return ID_ROOM_5;
        if ("ID_ROOM_1B"       == strEnumValue)    return ID_ROOM_1B;
        if ("ID_ROOM_1C"       == strEnumValue)    return ID_ROOM_1C;
        if ("ID_ROOM_RESERVED" == strEnumValue)    return ID_ROOM_RESERVED;
    }
    else if (strEnumName == "FED_NOTE_CONDITION")
    {
        if ("CONDITION_SKEW"                   == strEnumValue)    return CONDITION_SKEW;
        if ("CONDITION_SHORT_NOTE"             == strEnumValue)    return CONDITION_SHORT_NOTE;
        if ("CONDITION_LONG_NOTE"              == strEnumValue)    return CONDITION_LONG_NOTE;
        if ("CONDITION_SHIFT"                  == strEnumValue)    return CONDITION_SHIFT;
        if ("CONDITION_NOTES_REMAIN"           == strEnumValue)    return CONDITION_NOTES_REMAIN;
        if ("CONDITION_MIS_FEED"               == strEnumValue)    return CONDITION_MIS_FEED;
        if ("CONDITION_MOTOR_LOST_CALIBRATION" == strEnumValue)    return CONDITION_MOTOR_LOST_CALIBRATION;
        if ("CONDITION_HALF_NOTE"              == strEnumValue)    return CONDITION_HALF_NOTE;
    }
    else if (strEnumName == "DESTINATION_REJCET")
    {
        if ("DESTINATION_REJCET_DEFAULT"       == strEnumValue)    return DESTINATION_REJCET_DEFAULT;
        if ("DESTINATION_REJCET_CS"            == strEnumValue)    return DESTINATION_REJCET_CS;
        if ("DESTINATION_REJCET_DISPENSE"      == strEnumValue)    return DESTINATION_REJCET_DISPENSE;
        if ("DESTINATION_REJCET_DEPOSIT"       == strEnumValue)    return DESTINATION_REJCET_DEPOSIT;
        if ("DESTINATION_REJECT_CASH1_COMBINE" == strEnumValue)    return DESTINATION_REJECT_CASH1_COMBINE;
    }
    else if (strEnumName == "VALIDATION_MODE")
    {
        if ("VALIDATION_MODE_REAL" == strEnumValue)    return VALIDATION_MODE_REAL;
        if ("VALIDATION_MODE_TEST" == strEnumValue)    return VALIDATION_MODE_TEST;
    }
    else if (strEnumName == "CURRENCY_CODE")
    {
        if ("CURRENCY_CODE_CNY"      == strEnumValue)    return CURRENCY_CODE_CNY;
        if ("CURRENCY_CODE_EUR"      == strEnumValue)    return CURRENCY_CODE_EUR;
        if ("CURRENCY_CODE_RESERVED" == strEnumValue)    return CURRENCY_CODE_RESERVED;
    }
    else if (strEnumName == "DENOMINATION_CODE")
    {
        if ("DENOMINATION_CODE_00"      == strEnumValue)    return DENOMINATION_CODE_00;
        if ("DENOMINATION_CODE_10_4TH"  == strEnumValue)    return DENOMINATION_CODE_10_4TH;
        if ("DENOMINATION_CODE_50_4TH"  == strEnumValue)    return DENOMINATION_CODE_50_4TH;
        if ("DENOMINATION_CODE_100_4TH" == strEnumValue)    return DENOMINATION_CODE_100_4TH;
        if ("DENOMINATION_CODE_100_5TH" == strEnumValue)    return DENOMINATION_CODE_100_5TH;
        if ("DENOMINATION_CODE_20_5TH"  == strEnumValue)    return DENOMINATION_CODE_20_5TH;
        if ("DENOMINATION_CODE_10_5TH"  == strEnumValue)    return DENOMINATION_CODE_10_5TH;
        if ("DENOMINATION_CODE_50_5TH"  == strEnumValue)    return DENOMINATION_CODE_50_5TH;
        if ("DENOMINATION_CODE_ALL"     == strEnumValue)    return DENOMINATION_CODE_ALL;
    }
    else if (strEnumName == "CASSETTE_OPERATION")
    {
        if ("RB_OPERATION_RECYCLE"   == strEnumValue)    return RB_OPERATION_RECYCLE;
        if ("RB_OPERATION_DEPOSITE"  == strEnumValue)    return RB_OPERATION_DEPOSITE;
        if ("RB_OPERATION_DISPENSE"  == strEnumValue)    return RB_OPERATION_DISPENSE;
        if ("RB_OPERATION_UNKNOWN"   == strEnumValue)    return RB_OPERATION_UNKNOWN;
        if ("AB_OPERATION_DEPREJRET" == strEnumValue)    return AB_OPERATION_DEPREJRET;
    }
    else if (strEnumName == "NOTE_POWER_INDEX")
    {
        if ("POWER_INDEX_0"   == strEnumValue)    return POWER_INDEX_0;
        if ("POWER_INDEX_1"   == strEnumValue)    return POWER_INDEX_1;
        if ("POWER_INDEX_2"   == strEnumValue)    return POWER_INDEX_2;
        if ("POWER_INDEX_3"   == strEnumValue)    return POWER_INDEX_3;
        if ("POWER_INDEX_4"   == strEnumValue)    return POWER_INDEX_4;
        if ("POWER_INDEX_5"   == strEnumValue)    return POWER_INDEX_5;
    }
    else if (strEnumName == "RECORD_FULLIMAGE_MODE")
    {

        if ("FULLIMAGE_ALL_NOTES"      == strEnumValue)    return FULLIMAGE_ALL_NOTES;
        if ("FULLIMAGE_REJECTE_NOTES"  == strEnumValue)    return FULLIMAGE_REJECTE_NOTES;
        if ("FULLIMAGE_NO_RECORDS"     == strEnumValue)    return FULLIMAGE_NO_RECORDS;
    }
    else if (strEnumName == "REJECT_NOTE_NOSN")
    {
        if ("ACTION_REJECT_NOTES"      == strEnumValue)    return ACTION_REJECT_NOTES;
        if ("ACTION_NO_REJECT_NOTES"   == strEnumValue)    return ACTION_NO_REJECT_NOTES;
    }
    else if (strEnumName == "USER_MEMORY_TARGET")
    {
        if ("USER_MEMORY_TARGET_RESERVED"  == strEnumValue)    return USER_MEMORY_TARGET_RESERVED;
        if ("USER_MEMORY_TARGET_HCM2"      == strEnumValue)    return USER_MEMORY_TARGET_HCM2;
        if ("USER_MEMORY_TARGET_CASS1"     == strEnumValue)    return USER_MEMORY_TARGET_CASS1;
        if ("USER_MEMORY_TARGET_CASS2"     == strEnumValue)    return USER_MEMORY_TARGET_CASS2;
        if ("USER_MEMORY_TARGET_CASS3"     == strEnumValue)    return USER_MEMORY_TARGET_CASS3;
        if ("USER_MEMORY_TARGET_CASS4"     == strEnumValue)    return USER_MEMORY_TARGET_CASS4;
        if ("USER_MEMORY_TARGET_CASS5"     == strEnumValue)    return USER_MEMORY_TARGET_CASS5;
        if ("USER_MEMORY_TARGET_ALLCASS"   == strEnumValue)    return USER_MEMORY_TARGET_ALLCASS;
    }
    else if (strEnumName == "LOG_INFO_TYPE")
    {
        if ("LOG_INFO_STATISTICAL_TOTAL"      == strEnumValue)    return LOG_INFO_STATISTICAL_TOTAL;
        if ("LOG_INFO_STATISTICAL_SPECIFIC"   == strEnumValue)    return LOG_INFO_STATISTICAL_SPECIFIC;
        if ("LOG_INFO_ERRCODE"                == strEnumValue)    return LOG_INFO_ERRCODE;
        if ("LOG_INFO_WARNINGCODE"            == strEnumValue)    return LOG_INFO_WARNINGCODE;
        if ("LOG_INFO_ERRANALYSIS_GENERAL"    == strEnumValue)    return LOG_INFO_ERRANALYSIS_GENERAL;
        if ("LOG_INFO_OPERATIONAL"            == strEnumValue)    return LOG_INFO_OPERATIONAL;
        if ("LOG_INFO_NEAREST_OPERATION"      == strEnumValue)    return LOG_INFO_NEAREST_OPERATION;
        if ("LOG_INFO_ERRANALYSIS_INDIVIDUAL" == strEnumValue)    return LOG_INFO_ERRANALYSIS_INDIVIDUAL;
        if ("LOG_INFO_SENSOR"                 == strEnumValue)    return LOG_INFO_SENSOR;
        if ("LOG_INFO_INTERNAL_COMMAND"       == strEnumValue)    return LOG_INFO_INTERNAL_COMMAND;
        if ("LOG_INFO_NOTES_HANDLING"         == strEnumValue)    return LOG_INFO_NOTES_HANDLING;
        if ("LOG_INFO_MOTOR_CONTROL"          == strEnumValue)    return LOG_INFO_MOTOR_CONTROL;
        if ("LOG_INFO_SENSORLEVEL_LATEST"     == strEnumValue)    return LOG_INFO_SENSORLEVEL_LATEST;
        if ("LOG_INFO_SENSORLEVEL_SPECIFIC"   == strEnumValue)    return LOG_INFO_SENSORLEVEL_SPECIFIC;
    }
    else if (strEnumName == "LOG_INFO_TERM")
    {
        if ("TERM_WHOLE_DATA"  == strEnumValue)    return TERM_WHOLE_DATA;
        if ("TERM_THIS_MONTH"  == strEnumValue)    return TERM_THIS_MONTH;
        if ("TERM_LAST_MONTH"  == strEnumValue)    return TERM_LAST_MONTH;
        if ("MONTH_BEFOR_LAST" == strEnumValue)    return MONTH_BEFOR_LAST;
    }

    return -1;
}

QString TestUR2Driver::EnumToString(QString strSrc, int iValue)
{
    QString strTemp = "Unknown_Enum_Value";
    if (strSrc.isEmpty())
    {
        return nullptr;
    }
    if (strSrc == "HCM2SHUTTER_STATUS")
    {
        switch (iValue)
        {
        case HCM2SHUTTER_STATUS_OPEN:
            strTemp.sprintf("HCM2SHUTTER_STATUS_OPEN");
            break;
        case HCM2SHUTTER_STATUS_CLOSED:
            strTemp.sprintf("HCM2SHUTTER_STATUS_CLOSED");
            break;
        case HCM2SHUTTER_STATUS_OTHERS:
            strTemp.sprintf("HCM2SHUTTER_STATUS_OTHERS");
            break;
        case HCM2SHUTTER_STATUS_UNKNOWN:
            strTemp.sprintf("HCM2SHUTTER_STATUS_UNKNOWN");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "CURRENCY_CODE")
    {
        switch (iValue)
        {
        case CURRENCY_CODE_CNY:
            strTemp.sprintf("CURRENCY_CODE_CNY");
            break;
        case CURRENCY_CODE_EUR:
            strTemp.sprintf("CURRENCY_CODE_EUR");
            break;
        case CURRENCY_CODE_RESERVED:
            strTemp.sprintf("CURRENCY_CODE_RESERVED");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "CASSETTE_STATUS")
    {
        switch (iValue)
        {
        case CASSETTE_STATUS_NORMAL:
            strTemp.sprintf("CASSETTE_STATUS_NORMAL");
            break;
        case CASSETTE_STATUS_EMPTY:
            strTemp.sprintf("CASSETTE_STATUS_EMPTY");
            break;
        case CASSETTE_STATUS_NEAREST_EMPTY:
            strTemp.sprintf("CASSETTE_STATUS_NEAREST_EMPTY");
            break;
        case CASSETTE_STATUS_NEAR_EMPTY:
            strTemp.sprintf("CASSETTE_STATUS_NEAR_EMPTY");
            break;
        case CASSETTE_STATUS_NEAR_FULL:
            strTemp.sprintf("CASSETTE_STATUS_NEAR_FULL");
            break;
        case CASSETTE_STATUS_FULL:
            strTemp.sprintf("CASSETTE_STATUS_FULL");
            break;
        case CASSETTE_STATUS_UNKNOWN:
            strTemp.sprintf("CASSETTE_STATUS_UNKNOWN");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "CASSETTE_NUMBER")
    {
        switch (iValue)
        {
        case CASSETTE_1:
            strTemp.sprintf("CASSETTE_1");
            break;
        case CASSETTE_2:
            strTemp.sprintf("CASSETTE_2");
            break;
        case CASSETTE_3:
            strTemp.sprintf("CASSETTE_3");
            break;
        case CASSETTE_4:
            strTemp.sprintf("CASSETTE_4");
            break;
        case CASSETTE_5:
            strTemp.sprintf("CASSETTE_5");
            break;
        case CASSETTE_6:
            strTemp.sprintf("CASSETTE_6");
            break;
        case RESERVED:
            strTemp.sprintf("RESERVED");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "CASSETTE_TYPE")
    {
        switch (iValue)
        {
        case CASSETTE_TYPE_UNLOAD:
            strTemp.sprintf("CASSETTE_TYPE_UNLOAD");
            break;
        case CASSETTE_TYPE_RB:
            strTemp.sprintf("CASSETTE_TYPE_RB");
            break;
        case CASSETTE_TYPE_AB:
            strTemp.sprintf("CASSETTE_TYPE_AB");
            break;
        //case CASSETTE_TYPE_DAB:
        //   strTemp.sprintf("CASSETTE_TYPE_DAB");
        //    break;
        case CASSETTE_TYPE_UNKNOWN:
            strTemp.sprintf("CASSETTE_TYPE_UNKNOWN");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "DENOMINATION_CODE")
    {
        switch (iValue)
        {
        case DENOMINATION_CODE_00:
            strTemp.sprintf("DENOMINATION_CODE_00");
            break;
        case DENOMINATION_CODE_10_4TH:
            strTemp.sprintf("DENOMINATION_CODE_10_4TH");
            break;
        case DENOMINATION_CODE_50_4TH:
            strTemp.sprintf("DENOMINATION_CODE_50_4TH");
            break;
        case DENOMINATION_CODE_100_4TH:
            strTemp.sprintf("DENOMINATION_CODE_100_4TH");
            break;
        case DENOMINATION_CODE_100_5TH:
            strTemp.sprintf("DENOMINATION_CODE_100_5TH");
            break;
        case DENOMINATION_CODE_20_5TH:
            strTemp.sprintf("DENOMINATION_CODE_20_5TH");
            break;
        case DENOMINATION_CODE_10_5TH:
            strTemp.sprintf("DENOMINATION_CODE_10_5TH");
            break;
        case DENOMINATION_CODE_50_5TH:
            strTemp.sprintf("DENOMINATION_CODE_50_5TH");
            break;
        case DENOMINATION_CODE_ALL:
            strTemp.sprintf("DENOMINATION_CODE_ALL");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "CASSETTE_OPERATION")
    {
        switch (iValue)
        {
        case RB_OPERATION_RECYCLE:
            strTemp.sprintf("RB_OPERATION_RECYCLE");
            break;
        case RB_OPERATION_DEPOSITE:
            strTemp.sprintf("RB_OPERATION_DEPOSITE");
            break;
        case RB_OPERATION_DISPENSE:
            strTemp.sprintf("RB_OPERATION_DISPENSE");
            break;
        case RB_OPERATION_UNKNOWN:
            strTemp.sprintf("RB_OPERATION_UNKNOWN");
            break;
        case AB_OPERATION_DEPREJRET:
            strTemp.sprintf("AB_OPERATION_DEPREJRET");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "DESTINATION_REJCET")
    {
        switch (iValue)
        {
        case DESTINATION_REJCET_DEFAULT:
            strTemp.sprintf("DESTINATION_REJCET_DEFAULT");
            break;
        case DESTINATION_REJCET_CS:
            strTemp.sprintf("DESTINATION_REJCET_CS");
            break;
        case DESTINATION_REJCET_DISPENSE:
            strTemp.sprintf("DESTINATION_REJCET_DISPENSE");
            break;
        case DESTINATION_REJCET_DEPOSIT:
            strTemp.sprintf("DESTINATION_REJCET_DEPOSIT");
            break;
        case DESTINATION_REJECT_CASH1_COMBINE:
            strTemp.sprintf("DESTINATION_REJECT_CASH1_COMBINE");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "UNFIT_LEVEL")
    {
        switch (iValue)
        {
        case UNFIT_LEVEL_DEFAULT:
            strTemp.sprintf("UNFIT_LEVEL_DEFAULT");
            break;
        case UNFIT_LEVEL_NORMAL:
            strTemp.sprintf("UNFIT_LEVEL_NORMAL");
            break;
        case UNFIT_LEVEL_SOFT:
            strTemp.sprintf("UNFIT_LEVEL_SOFT");
            break;
        case UNFIT_LEVEL_STRICT:
            strTemp.sprintf("UNFIT_LEVEL_STRICT");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "VERIFICATION_LEVEL")
    {
        switch (iValue)
        {
        case VERIFICATION_LEVEL_DEFAULT:
            strTemp.sprintf("VERIFICATION_LEVEL_DEFAULT");
            break;
        case VERIFICATION_LEVEL_NORMAL:
            strTemp.sprintf("VERIFICATION_LEVEL_NORMAL");
            break;
        case VERIFICATION_LEVEL_STRICT:
            strTemp.sprintf("VERIFICATION_LEVEL_STRICT");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "CASSETTE_ROOM_ID")
    {
        switch (iValue)
        {
        case ID_CS:
            strTemp.sprintf("ID_CS");
            break;
        case ID_ESC:
            strTemp.sprintf("ID_ESC");
            break;
        case ID_URJB:
            strTemp.sprintf("ID_URJB");
            break;
        case ID_ROOM_1:
            strTemp.sprintf("ID_ROOM_1");
            break;
        case ID_ROOM_2:
            strTemp.sprintf("ID_ROOM_2");
            break;
        case ID_ROOM_3:
            strTemp.sprintf("ID_ROOM_3");
            break;
        case ID_ROOM_4:
            strTemp.sprintf("ID_ROOM_4");
            break;
        case ID_ROOM_5:
            strTemp.sprintf("ID_ROOM_5");
            break;
        case ID_ROOM_1B:
            strTemp.sprintf("ID_ROOM_1B");
            break;
        case ID_ROOM_1C:
            strTemp.sprintf("ID_ROOM_1C");
            break;
        case ID_ROOM_RESERVED:
            strTemp.sprintf("ID_ROOM_RESERVED");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "BV_IMAGE_TYPE")
    {
        switch (iValue)
        {
        case BV_IMAGE_NOEXIST:
            strTemp.sprintf("BV_IMAGE_NOEXIST");
            break;
        case BV_IMAGE_SNIMAGE:
            strTemp.sprintf("BV_IMAGE_SNIMAGE");
            break;
        case BV_IMAGE_FULLIMAGE:
            strTemp.sprintf("BV_IMAGE_FULLIMAGE");
            break;
        case BV_IMAGE_BOTH:
            strTemp.sprintf("BV_IMAGE_BOTH");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "MEDIA_INFORMATION_REJECT_CAUSE")
    {
        switch (iValue)
        {
        case MEDIA_INFORMATION_REJECT_CAUSE_RESERVED:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_RESERVED");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_SHIFT:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_SHIFT");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_SKEW:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_SKEW");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_EXSKEW:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_EXSKEW");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_LONG:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_LONG");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_SPACING:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_SPACING");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_INTERVAL:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_INTERVAL");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_DOUBLE:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_DOUBLE");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_DIMENSTION_ERR:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_DIMENSTION_ERR");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_DENO_UNIDENTIFIED:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_DENO_UNIDENTIFIED");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_VERIFICATION:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_VERIFICATION");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_UNFIT:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_UNFIT");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_BV_OTHERS:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_BV_OTHERS");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_DIFF_DENO:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_DIFF_DENO");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_EXCESS:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_EXCESS");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_FACTOR1:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_FACTOR1");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_FACTOR2:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_FACTOR2");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_NO_VALIDATION:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_NO_VALIDATION");
            break;
        case MEDIA_INFORMATION_REJECT_CAUSE_BV_FORMAT_ERR:
            strTemp.sprintf("MEDIA_INFORMATION_REJECT_CAUSE_BV_FORMAT_ERR");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "eNoteEdition")
    {
        switch (iValue)
        {
        case eNoteEdition_1999_100:
            strTemp.sprintf("eNoteEdition_1999");
            break;
        case eNoteEdition_2005_100:
            strTemp.sprintf("eNoteEdition_2005");
            break;
        case eNoteEdition_2015_100:
            strTemp.sprintf("eNoteEdition_2015");
            break;
        case eNoteEdition_unknown:
            strTemp.sprintf("eNoteEdition_unknown");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "USER_MEMORY_TARGET")
    {
        switch (iValue)
        {
        case USER_MEMORY_TARGET_RESERVED:
            strTemp.sprintf("USER_MEMORY_TARGET_RESERVED");
            break;
        case USER_MEMORY_TARGET_HCM2:
            strTemp.sprintf("USER_MEMORY_TARGET_HCM2");
            break;
        case USER_MEMORY_TARGET_CASS1:
            strTemp.sprintf("USER_MEMORY_TARGET_CASS1");
            break;
        case USER_MEMORY_TARGET_CASS2:
            strTemp.sprintf("USER_MEMORY_TARGET_CASS2");
            break;
        case USER_MEMORY_TARGET_CASS3:
            strTemp.sprintf("USER_MEMORY_TARGET_CASS3");
            break;
        case USER_MEMORY_TARGET_CASS4:
            strTemp.sprintf("USER_MEMORY_TARGET_CASS4");
            break;
        case USER_MEMORY_TARGET_CASS5:
            strTemp.sprintf("USER_MEMORY_TARGET_CASS5");
            break;
        case USER_MEMORY_TARGET_ALLCASS:
            strTemp.sprintf("USER_MEMORY_TARGET_ALLCASS");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "HCM2SHUTTER_STATUS")
    {
        switch (iValue)
        {
        case HCM2SHUTTER_STATUS_OPEN:
            strTemp.sprintf("HCM2SHUTTER_STATUS_OPEN");
            break;
        case HCM2SHUTTER_STATUS_CLOSED:
            strTemp.sprintf("HCM2SHUTTER_STATUS_CLOSED");
            break;
        case HCM2SHUTTER_STATUS_OTHERS:
            strTemp.sprintf("HCM2SHUTTER_STATUS_OTHERS");
            break;
        case HCM2SHUTTER_STATUS_UNKNOWN:
            strTemp.sprintf("HCM2SHUTTER_STATUS_UNKNOWN");
            break;
        default:
            break;
        }
    }
    else if (strSrc == "LOG_INFO_TYPE")
    {
        switch (iValue)
        {
        case LOG_INFO_STATISTICAL_TOTAL:
            strTemp.sprintf("LOG_STATISTICAL_TOTAL");
            break;
        case LOG_INFO_STATISTICAL_SPECIFIC:
            strTemp.sprintf("LOG_STATISTICAL_SPECIFIC");
            break;
        case LOG_INFO_ERRCODE:
            strTemp.sprintf("LOG_ERRCODE");
            break;
        case LOG_INFO_WARNINGCODE:
            strTemp.sprintf("LOG_WARNINGCODE");
            break;
        case LOG_INFO_ERRANALYSIS_GENERAL:
            strTemp.sprintf("LOG_ERRANALYSIS_GENERAL");
            break;
        case LOG_INFO_OPERATIONAL:
            strTemp.sprintf("LOG_OPERATIONAL");
            break;
        case LOG_INFO_NEAREST_OPERATION:
            strTemp.sprintf("LOG_NEAREST_OPERATION");
            break;
        case LOG_INFO_ERRANALYSIS_INDIVIDUAL:
            strTemp.sprintf("LOG_ERRANALYSIS_INDIVIDUAL");
            break;
        case LOG_INFO_SENSOR:
            strTemp.sprintf("LOG_SENSOR");
            break;
        case LOG_INFO_INTERNAL_COMMAND:
            strTemp.sprintf("LOG_INTERNAL_COMMAND");
            break;
        case LOG_INFO_NOTES_HANDLING:
            strTemp.sprintf("LOG_NOTES_HANDLING");
            break;
        case LOG_INFO_MOTOR_CONTROL:
            strTemp.sprintf("LOG_MOTOR_CONTROL");
            break;
        case LOG_INFO_SENSORLEVEL_LATEST:
            strTemp.sprintf("LOG_SENSORLEVEL_LATEST");
            break;
        case LOG_INFO_SENSORLEVEL_SPECIFIC:
            strTemp.sprintf("LOG_SENSORLEVEL_SPECIFIC");
            break;
        default:
            break;
        }
    }

    return strTemp;
}

void TestUR2Driver::TextEditLimit()
{
    QString textContent = ui->textEdit_UserMemoryWrite_szUserMemoryData->toPlainText();
    int length = textContent.count();
    int maxLength = MAX_USER_MEMORY_DATA_LENGTH; // 最大字符数
    if (length > maxLength)
    {
        int position = ui->textEdit_UserMemoryWrite_szUserMemoryData->textCursor().position();
        QTextCursor textCursor = ui->textEdit_UserMemoryWrite_szUserMemoryData->textCursor();
        textContent.remove(position - (length - maxLength), length - maxLength);
        ui->textEdit_UserMemoryWrite_szUserMemoryData->setText(textContent);
        textCursor.setPosition(position - (length - maxLength));
        ui->textEdit_UserMemoryWrite_szUserMemoryData->setTextCursor(textCursor);
        ui->lineEdit_UserMemoryWrite_usDataLen->setText("128");
        return ;
    }
    char szCount[10];
    sprintf(szCount, "%i", length);
    ui->lineEdit_UserMemoryWrite_usDataLen->setText(szCount);
}

//---------------------------获取控件值-----------------------------------
void TestUR2Driver::GetSTOperationalInfo(ST_OPERATIONAL_INFO &stOperationalInfo)
{
    stOperationalInfo.bArticle6Support = ui->checkBox_SetUnitInfo_bArticle6Support->isChecked();
    stOperationalInfo.bActiveVerificationLevel = ui->checkBox_SetUnitInfo_bActiveVerificationLevel->isChecked();
    stOperationalInfo.bRejectUnfitNotesStore = ui->checkBox_SetUnitInfo_bRejectUnfitNotesStore->isChecked();
    stOperationalInfo.bReportUnacceptDeno = ui->checkBox_SetUnitInfo_bReportUnacceptDeno->isChecked();
    stOperationalInfo.bCashCountErrAsWarning = ui->checkBox_SetUnitInfo_bCashCountErrAsWarning->isChecked();
    stOperationalInfo.bUseCorrectionFunction = ui->checkBox_SetUnitInfo_bUseCorrectionFunction->isChecked();
    stOperationalInfo.bDispErrAsWarning = ui->checkBox_SetUnitInfo_bDispErrAsWarning->isChecked();
    stOperationalInfo.bShutterCheckBeforeDispense = ui->checkBox_SetUnitInfo_bShutterCheckBeforeDispense->isChecked();
    stOperationalInfo.bUseSNImageReadFunction = ui->checkBox_SetUnitInfo_bUseSNImageReadFunction->isChecked();
    stOperationalInfo.bStorDiffSizeNotesInDeposit = ui->checkBox_SetUnitInfo_bStorDiffSizeNotesInDeposit->isChecked();
    stOperationalInfo.bUseSNReadFuncton = ui->checkBox_SetUnitInfo_bUseSNReadFuncton->isChecked();
    stOperationalInfo.bCassMemoryOperation = ui->checkBox_SetUnitInfo_bCassMemoryOperation->isChecked();
}

void TestUR2Driver::GetCassetteInfo(ST_CASSETTE_INFO(&stCassType)[MAX_CASSETTE_NUM])
{
    char pTempCtrlName[MAX_SIZE] = {0};

    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        memset(pTempCtrlName, 0, MAX_SIZE);
        sprintf(pTempCtrlName, "comboBox_SetUnitInfo_Cassette%d_iCassNO", i + 1);
        stCassType[i].iCassNO =
        (CASSETTE_NUMBER)StringToEnum("CASSETTE_NUMBER", findChild<QComboBox *>(pTempCtrlName)->currentText());

        memset(pTempCtrlName, 0, MAX_SIZE);
        sprintf(pTempCtrlName, "comboBox_SetUnitInfo_Cassette%d_iCassType", i + 1);
        stCassType[i].iCassType =
        (CASSETTE_TYPE)StringToEnum("CASSETTE_TYPE", findChild<QComboBox *>(pTempCtrlName)->currentText());

        memset(pTempCtrlName, 0, MAX_SIZE);
        sprintf(pTempCtrlName, "comboBox_SetUnitInfo_Cassette%d_iDenoCode", i + 1);
        stCassType[i].iDenoCode =
        (DENOMINATION_CODE)StringToEnum("DENOMINATION_CODE", findChild<QComboBox *>(pTempCtrlName)->currentText());

        memset(pTempCtrlName, 0, MAX_SIZE);
        sprintf(pTempCtrlName, "comboBox_SetUnitInfo_Cassette%d_iCassOper", i + 1);
        stCassType[i].iCassOper =
        (CASSETTE_OPERATION)StringToEnum("CASSETTE_OPERATION", findChild<QComboBox *>(pTempCtrlName)->currentText());

        memset(pTempCtrlName, 0, MAX_SIZE);
        sprintf(pTempCtrlName, "comboBox_SetUnitInfo_Cassette%d_iCurrencyCode", i + 1);
        stCassType[i].iCurrencyCode =
        (CURRENCY_CODE)StringToEnum("CURRENCY_CODE", findChild<QComboBox *>(pTempCtrlName)->currentText());

        memset(pTempCtrlName, 0, MAX_SIZE);
        sprintf(pTempCtrlName, "comboBox_SetUnitInfo_Cassette%d_iCassNoteHandInfo", i + 1);
        stCassType[i].iCassNoteHandInfo =
        (DESTINATION_REJCET)StringToEnum("DESTINATION_REJCET", findChild<QComboBox *>(pTempCtrlName)->currentText());

        memset(pTempCtrlName, 0, MAX_SIZE);
        sprintf(pTempCtrlName, "lineEdit_SetUnitInfo_Cassette%d_iCashValue", i + 1);
        stCassType[i].iCashValue = findChild<QLineEdit *>(pTempCtrlName)->text().toInt();

        memset(pTempCtrlName, 0, MAX_SIZE);
        sprintf(pTempCtrlName, "lineEdit_SetUnitInfo_Cassette%d_ucIssuingBank", i + 1);
        stCassType[i].ucIssuingBank = findChild<QLineEdit *>(pTempCtrlName)->text().toInt();

        memset(pTempCtrlName, 0, MAX_SIZE);
        sprintf(pTempCtrlName, "lineEdit_SetUnitInfo_Cassette%d_ucVersion", i + 1);
        stCassType[i].ucVersion = findChild<QLineEdit *>(pTempCtrlName)->text().toInt();

    }

}

//------------------------------END-----------------------------------


