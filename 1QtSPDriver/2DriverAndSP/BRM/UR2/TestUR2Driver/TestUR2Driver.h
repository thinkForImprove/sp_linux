#ifndef _UR2DRIVERTEST_H
#define _UR2DRIVERTEST_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QCloseEvent>
#include <QMessageBox>
#include <QAction>
#include <QListWidget>
#include <QFile>
#include <stdio.h>
#include <QDir>
#include <QTextCodec>
#include <sys/time.h>
#include <QFileDialog>
#include <QTableWidget>

#include "ReadDriverConfig.h"
#include "IURDevice.h"
#include "DeviceSOLoader.h"
#include "ui_ur2drivertest.h"

using namespace std;

#define TAB_STR                 "  "
#define LOG_DIR                 "UR2DriverTestLog"
#define DEFAULT_IMAGE           ":/Resource/Default.bmp"

#define FW_BATCH_DATA_SIZE      7168
#define MAX_SIZE                1024
#define MAX_DENO_LENGTH         10
#define MAX_MDEIA_DATALENGTH    10240

//command define
#define InvalidCommand            -1                         //无效的指令

//初始化设置指令组
#define getFWVersion              0                          //固件版本信息
#define setVerificationLevel      1                          //设置校验级别，减少废钞率
#define getBanknoteInfo           2                          //查询BV序列号及配置表
#define setDenominationCode       3                          //设置面额配置代码
#define getCassetteInfo           4                          //查询钞箱信息
#define setUnitInfo               5                          //设置HCM钞箱配置信息
#define getUnitInfo               6                          //获取HCM钞箱配置信息
#define initReset                 7                          //复位

//查询类指令组
#define queryBVStatusSense        8                          //查询BV模块信息
#define getDevStatusInfo          9                          //获得设备状态信息
#define getNoteSeiralInfo         10                         //获取冠字码相关信息
#define getMediaInformation       11                         //获取钞票流向面额信息

//控制指令组
#define cashCount                 12                         //点钞
#define cashCountRetract          13                         //点钞回收
#define storeMoney                14                         //存入钞票
#define cashRollback              15                         //取消存款
#define dispense                  16                         //挖钞
#define rejcet                    17                         //出钞后回收取款拒钞
#define retractESC                18                         //回收ESC中的钞票
#define closeShutter              19                         //关闭Shutter门
#define openShutter               20                         //打开Shutter门
#define bvCommStart               21                         //PC与ZERO BV通信开始
#define bvCommEnd                 22                         //PC与ZERO BV通信结束

//维护指令组
#define reboot                    23                         //断开连接并重启
#define startTransaction          24                         //开始存取款交易
#define preNextTransaction        25                         //准备进入一下笔交易
#define setStatusChangeListener   26                         //设置状态改变侦听接口 (暂不实现该接口)
#define userMemoryWrite           27                         //写数据到用户内存区域
#define userMemoryRead            28                         //读取用户内存的数据

//其他指令组
#define getLogData                29                          //请求日志数据
#define eraseAllLogData           30                          //清除日志数据
#define closeUSBConn              31                          //关闭连接
#define updatePDL                 32                          //更新固件

#define getAllSersorLight   33              //读取所有感应器的亮灭状态

typedef struct _ProgramFileInfo
{
    UINT uCTLID;
    UINT uLoadAdress;
    UINT uSUM;
    ULONG ulMaxPacketSize;
    char szFilePath[MAX_PATH];
    char szFWType[32];
    char szFWVersion[32];
    _ProgramFileInfo()
    {
        uCTLID = 0;
        uLoadAdress = 0;
        uSUM = 0;
        ulMaxPacketSize = 0;
        ZeroMemory(szFilePath, sizeof(szFilePath));
        ZeroMemory(szFWType, sizeof(szFWType));
        ZeroMemory(szFWVersion, sizeof(szFWVersion));
    }
} ProgramFileInfo;

typedef struct
{
    int iDenoValue;
    UCHAR ucVersion;
} DenoInfo;

namespace Ui
{
class UR2DriverTest;
}

class TestUR2Driver : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestUR2Driver(QWidget *parent = 0);
    ~TestUR2Driver();
    void Release();

protected:
    //日志记录函数
    //日志函数调用顺序如下：
    //1. 调用LogBeforeExecute记录执行命令前的时间、命令名、输入参数等信息
    //2. 执行命令
    //3. 调用LogAfterExecute记录执行命令的完成时间、函数返回值、错误描述等信息，并“输出参数:”
    //4. 调用LogOutput函数和其他Log函数记录输出参数
    //5. 最后调用LogEnd记录空行
    //记录一条输出，iTabNum指定前面加TAB个数
    void LogOutput(const char *pName, const QString Value, int iTabNum = 1);
    //记录结构名
    void LogOutputStructName(const char *pStructName, int iTabNum = 1);
    //执行命令前记录
    void LogBeforeExecute();
    //执行命令后记录
    void LogAfterExecute(int nError, bool bParam = false);//bHasOutPut
    //记录尾行
    void LogEnd(int nError);
    //通用LOG函数，记录一行日志信息
    void Log(QString log_content);

    //ST_DEV_STATUS_INFO
    void LogDevStatusInfo(const ST_DEV_STATUS_INFO &stDevStatusInfo);
    //ST_BV_INFO
    void LogBVInfo(const ST_BV_INFO &stBVInfo, int iTabNum = 1);
    //ST_DENO_INFO
    void LogDenoInfo(const ST_DENO_INFO &stDenoInfo, int iNum = -1, int iTabNum = 1);
    //ST_CASSETTE_INFO
    void LogCassetteInfo(const ST_CASSETTE_INFO &stCassetteInfo, int iNum = -1, int iTabNum = 1);
    //ST_OPERATIONAL_INFO
    void LogOperationalInfo(const ST_OPERATIONAL_INFO &stOperationalInfo, int iTabNum = 1);
    //ST_HW_CONFIG
    void LogHWConfig(const ST_HW_CONFIG &stHWConfig, int iTabNum = 1);
    //ST_BV_VERIFICATION_LEVEL
    void LogBVVerificationLevel(const ST_BV_VERIFICATION_LEVEL &stBVVerificationLevel, int iTabNum = 1);
    //ST_MEDIA_INFORMATION_INFO
    void LogMediaInformationInfo(const ST_MEDIA_INFORMATION_INFO &totalMediaInfo, int iNum = -1, int iTabNum = 1);
    //ST_TOTAL_STACKE_NOTES_DENO_INFO
    void LogTotalStackeNotesDenoInfo(const ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo, const char *szType, int iTabNum = 1);
    //FWVersion
    void AnalysisFWVersion(const char *pszFWVersion);

    // 执行命令响应函数
    void OnGetFWVersion();
    void OnSetVerificationLevel();
    void OnGetBanknoteInfo();
    void OnSetDenominationCode();
    void OnGetCassetteInfo();
    void OnSetUnitInfo();
    void OnGetUnitInfo();
    void OnInitReset();

    //查询命令
    void OnQueryBVStatusSense();
    void OnGetDevStatusInfo();
    void OnGetNoteSeiralInfo();
    void OnGetMediaInformation();
    void OnGetAllSersorLight();

    //控制命令
    void OnCashCount();
    void OnCashCountRetract();
    void OnStoreMoney();
    void OnCashRollback();
    void OnDispense();
    void OnReject();
    void OnRetractESC();
    void OnCloseShutter();
    void OnOpenShutter();
    void OnBVCommStart();
    void OnBVCommEnd();

    //维护指令
    void OnReboot();
    void OnStartTransaction();
    void OnPreNextTransaction();
    void OnSetStatusChangeListener();
    void OnUserMemoryWrite();
    void OnUserMemoryRead();

    //其他指令
    void OnGetLogData();
    void OnEraseAllLogData();
    void OnCloseUSBConn();
    void OnUpdatePDL();

    //获取控件值
    void GetSTOperationalInfo(ST_OPERATIONAL_INFO &stOperationalInfo);
    void GetCassetteInfo(ST_CASSETTE_INFO(&stCassType)[MAX_CASSETTE_NUM]);

private:
    void closeEvent(QCloseEvent *event);
    void InitTreeWidget();
    void ReadInitData();
    int CreateSerialImage(SNOTESERIALINFO &lpNoteSerialInfo, int iNumber);
    int CreateSerialImage(SNOTESERIALINFOFULLIMAGE &lpNoteSerialInfoFullImage, int iNumber);
    int CreateDefaultImage(int iNumber);
    void FindDevDifference(const ST_DEV_STATUS_INFO &stDevStatusInfo);
    void SaveDevStatus(const ST_DEV_STATUS_INFO &stDevStatusInfo);
    //功能：通过枚举成员值获取枚举成员
    QString EnumToString(QString strSrc, int iValue);
    //功能：通过枚举成员获取枚举成员值
    int StringToEnum(QString strEnumName, QString strEnumValue);

private slots:
    void show_widget(QTreeWidgetItem *item);
    bool SetCtrlValue(QString, QString, QString);
    void TextEditLimit();
    void on_action_connect_triggered();
    void on_action_close_triggered();
    void on_pushButton_exec_clicked();
    void ClearInfo();
    void SelectPDLFile();
    void GetLocalFW();
    void GetFileFW();

    //打开可选参数控制
    void EnableSetUnitInfoPriority(int iStatus);
    void EnableSetUnitInfoDenoCode(int iStatus);
    void EnableDispenseArrDispDeno(int iStatus);
    void EnableDispenseArrDispRoom(int iStatus);
    void EnableDispenseBVMode(int iStatus);
    void EnableCashCountAcceptDeno(int iStatus);
    void EnableCashCountBVMode(int iStatus);
    void EnableCashCountRetractBVMode(int iStatus);
    void EnableStoreMoneyBVMode(int iStatus);
    void EnableRejectBVMode(int iStatus);
    void EnableRetractESCBVMode(int iStatus);

protected:
    bool m_bUpdate;
    bool m_bGetLocalFWFlag;
    bool m_bGetFileFWFlag;

    int m_iStartTime, m_iEndTime;//记录命令执行时间
    int m_iCommandType;
    int m_iBVCount;//通过BV的钞票总数，用于控制获取冠字码信息的Index值

    QString m_strFilePathPDL;//更新固件文件路径
    QStringList m_strLocalFWVersion;//保存当前固件版本
    QString m_strSerName;//对应根节点名字
    QString m_strCurrencyCommand;//对应当前命令名
    QString m_strLogDir;

    ST_DEV_STATUS_INFO m_stDevStatusInfo;

    DeviceSOLoader<IURDevice> m_UR2Device;

    ReadDriverConfig m_ConfigData;//配置文件

    map<USHORT, ProgramFileInfo> m_ProFilesInfo;
    map<int, DenoInfo> m_DenoInfo;

    QLineEdit *m_lineEdit[MAX_DENOMINATION_NUM];//作为表格中的代理

private:
    Ui::UR2DriverTest *ui;
    QWidget *mainWidget;

    //控件成员变量
    QListWidget *m_list_log;
    char *m_currency_path;//程序当前路径
};

#endif // HCM2DRIVERTEST_H
